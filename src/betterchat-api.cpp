/*
BetterChatTV para OBS
Copyright (C) 2026 DEValen

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#include "betterchat-api.hpp"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <QSslSocket>

#include <obs.h>
#include <plugin-support.h>

// Traduce el error de red a un texto util para el usuario y lo registra.
static QString describeNetworkError(QNetworkReply *reply)
{
	int http = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	QString err = reply->errorString();
	obs_log(LOG_WARNING, "[betterchat] network error: qt=%d http=%d msg=%s", (int)reply->error(), http,
		err.toUtf8().constData());
	// OJO: supportsSsl() SOLO mira OpenSSL; ignora Schannel (TLS nativo de Windows).
	// El indicador fiable de "sin TLS" es que NO haya ningun backend disponible.
	if (QSslSocket::availableBackends().isEmpty()) {
		obs_log(LOG_WARNING, "[betterchat] Qt no tiene NINGUN backend TLS disponible");
		return QObject::tr("Tu OBS no tiene soporte TLS para HTTPS. Actualiza OBS o reinstala.");
	}
	if (http >= 400)
		return QObject::tr("El servidor respondió con un error (%1).").arg(http);
	return QObject::tr("No se pudo contactar con BetterChatTV: %1").arg(err);
}

// URL base del servicio. Configurable por variable de entorno para dev.
static QString resolveBaseUrl()
{
	QByteArray env = qgetenv("BETTERCHAT_BASE_URL");
	if (!env.isEmpty())
		return QString::fromUtf8(env);
	return QStringLiteral("https://betterchat.tv");
}

BetterChatApi::BetterChatApi(QObject *parent) : QObject(parent), m_baseUrl(resolveBaseUrl())
{
	loadToken();
	m_pairTimer.setInterval(3000);
	connect(&m_pairTimer, &QTimer::timeout, this, &BetterChatApi::pollPairing);
	connect(&m_statusTimer, &QTimer::timeout, this, &BetterChatApi::refreshStatus);
	obs_log(LOG_INFO, "[betterchat] api base=%s ssl_support=%s ssl_build=%s", m_baseUrl.toUtf8().constData(),
		QSslSocket::supportsSsl() ? "yes" : "NO",
		QSslSocket::sslLibraryBuildVersionString().toUtf8().constData());
	obs_log(LOG_INFO, "[betterchat] TLS backends disponibles: %s",
		QSslSocket::availableBackends().join(QStringLiteral(", ")).toUtf8().constData());
}

void BetterChatApi::loadToken()
{
	QSettings s(QStringLiteral("BetterChatTV"), QStringLiteral("obs-plugin"));
	m_token = s.value(QStringLiteral("pluginToken")).toString();
}

void BetterChatApi::saveToken(const QString &token)
{
	m_token = token;
	QSettings s(QStringLiteral("BetterChatTV"), QStringLiteral("obs-plugin"));
	if (token.isEmpty())
		s.remove(QStringLiteral("pluginToken"));
	else
		s.setValue(QStringLiteral("pluginToken"), token);
}

QNetworkRequest BetterChatApi::apiRequest(const QString &path, bool withAuth) const
{
	QNetworkRequest req(QUrl(m_baseUrl + path));
	req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
	req.setRawHeader("Accept", "application/json");
	if (withAuth && !m_token.isEmpty())
		req.setRawHeader("Authorization", QByteArray("Bearer ") + m_token.toUtf8());
	return req;
}

// ---- Emparejamiento (device-flow) ----

void BetterChatApi::startPairing()
{
	QNetworkReply *reply =
		m_net.post(apiRequest(QStringLiteral("/api/plugin/pair/start"), false), QByteArray("{}"));
	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		reply->deleteLater();
		if (reply->error() != QNetworkReply::NoError) {
			emit pairingFailed(describeNetworkError(reply));
			return;
		}
		QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
		m_deviceCode = obj.value(QStringLiteral("deviceCode")).toString();
		QString userCode = obj.value(QStringLiteral("userCode")).toString();
		QString verifyUrl = obj.value(QStringLiteral("verifyUrl")).toString();
		int interval = obj.value(QStringLiteral("interval")).toInt(3);
		m_pairExpires = obj.value(QStringLiteral("expiresIn")).toInt(600);
		m_pairElapsed = 0;
		if (m_deviceCode.isEmpty() || userCode.isEmpty()) {
			emit pairingFailed(tr("Respuesta inesperada del servidor."));
			return;
		}
		m_pairTimer.setInterval(interval * 1000);
		m_pairTimer.start();
		QDesktopServices::openUrl(QUrl(verifyUrl));
		emit pairingStarted(userCode, verifyUrl);
	});
}

void BetterChatApi::cancelPairing()
{
	m_pairTimer.stop();
	m_deviceCode.clear();
}

void BetterChatApi::pollPairing()
{
	if (m_deviceCode.isEmpty()) {
		m_pairTimer.stop();
		return;
	}
	m_pairElapsed += m_pairTimer.interval() / 1000;
	if (m_pairElapsed >= m_pairExpires) {
		m_pairTimer.stop();
		m_deviceCode.clear();
		emit pairingFailed(tr("El código ha caducado. Inténtalo de nuevo."));
		return;
	}

	QJsonObject body;
	body.insert(QStringLiteral("deviceCode"), m_deviceCode);
	QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
	QNetworkReply *reply = m_net.post(apiRequest(QStringLiteral("/api/plugin/pair/token"), false), payload);
	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		reply->deleteLater();
		QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
		QString status = obj.value(QStringLiteral("status")).toString();
		if (status == QStringLiteral("approved")) {
			m_pairTimer.stop();
			m_deviceCode.clear();
			saveToken(obj.value(QStringLiteral("token")).toString());
			emit loggedIn();
			refreshStatus();
		} else if (status == QStringLiteral("expired") || status == QStringLiteral("not_found")) {
			m_pairTimer.stop();
			m_deviceCode.clear();
			emit pairingFailed(tr("La vinculación caducó. Inténtalo de nuevo."));
		}
		// "pending": seguimos sondeando.
	});
}

// ---- Estado ----

void BetterChatApi::refreshStatus()
{
	if (m_token.isEmpty())
		return;
	QNetworkReply *reply = m_net.get(apiRequest(QStringLiteral("/api/plugin/status"), true));
	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		reply->deleteLater();
		int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		if (code == 401) {
			// Token revocado o invalido: forzar logout local.
			saveToken(QString());
			m_username.clear();
			m_overlayUrl.clear();
			m_live = false;
			emit loggedOut();
			return;
		}
		if (reply->error() != QNetworkReply::NoError) {
			emit statusError(tr("No se pudo consultar el estado."));
			return;
		}
		QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
		m_username = obj.value(QStringLiteral("username")).toString();
		m_overlayUrl = obj.value(QStringLiteral("overlayUrl")).toString();
		m_live = obj.value(QStringLiteral("live")).toBool();
		m_platform = obj.value(QStringLiteral("platform")).toString();
		m_autoTest = obj.value(QStringLiteral("autoTest")).toBool();
		m_isPlus = obj.value(QStringLiteral("isPlus")).toBool();
		m_youtubeLinked = obj.value(QStringLiteral("youtubeLinked")).toBool();
		emit statusUpdated();
	});
}

void BetterChatApi::startStatusPolling(int intervalMs)
{
	m_statusTimer.setInterval(intervalMs);
	m_statusTimer.start();
	refreshStatus();
}

void BetterChatApi::stopStatusPolling()
{
	m_statusTimer.stop();
}

void BetterChatApi::sendAutoTestMessage()
{
	if (m_token.isEmpty())
		return;
	QNetworkReply *reply = m_net.post(apiRequest(QStringLiteral("/api/plugin/test-message"), true),
					  QByteArray("{\"auto\":true}"));
	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		reply->deleteLater();
		int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		if (code == 409) {
			emit chatActionResult(false, tr("Abre el overlay del chat en OBS para ver los mensajes."));
		} else if (reply->error() != QNetworkReply::NoError) {
			emit chatActionResult(false, tr("No se pudo enviar el mensaje de prueba."));
		}
		// ok: no molestamos con mensaje (llegan a ritmo).
	});
}

// Activa/desactiva los mensajes automáticos en el SERVIDOR (sincronizado con la web
// y con el overlay). El servidor mantiene un solo timer por sala; aquí solo enviamos
// el estado deseado. El estado real vuelve por el sondeo de status (autoTest).
void BetterChatApi::setAutoTest(bool active)
{
	if (m_token.isEmpty())
		return;
	QByteArray body = active ? QByteArray("{\"active\":true}") : QByteArray("{\"active\":false}");
	QNetworkReply *reply =
		m_net.post(apiRequest(QStringLiteral("/api/plugin/auto-test"), true), body);
	connect(reply, &QNetworkReply::finished, this, [this, reply, active]() {
		reply->deleteLater();
		int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		if (code == 409) {
			emit chatActionResult(false, tr("Abre el overlay del chat en OBS primero."));
			return;
		}
		if (reply->error() != QNetworkReply::NoError) {
			emit chatActionResult(false, tr("No se pudo cambiar los mensajes automáticos."));
			return;
		}
		QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
		m_autoTest = obj.value(QStringLiteral("active")).toBool(active);
		emit statusUpdated(); // que el dock refleje el nuevo estado
	});
}

void BetterChatApi::clearChat()
{
	if (m_token.isEmpty())
		return;
	QNetworkReply *reply =
		m_net.post(apiRequest(QStringLiteral("/api/plugin/clear"), true), QByteArray("{}"));
	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		reply->deleteLater();
		int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		if (code == 409)
			emit chatActionResult(false, tr("Abre el overlay del chat en OBS primero."));
		else if (reply->error() != QNetworkReply::NoError)
			emit chatActionResult(false, tr("No se pudo limpiar el chat."));
		else
			emit chatActionResult(true, tr("Chat limpiado."));
	});
}

void BetterChatApi::fetchYouTubeLive()
{
	if (m_token.isEmpty()) {
		emit youtubeLiveError(tr("No has iniciado sesión."));
		return;
	}
	QNetworkReply *reply =
		m_net.get(apiRequest(QStringLiteral("/api/youtube/live-streams"), true));
	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		reply->deleteLater();
		const QByteArray body = reply->readAll();
		int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		if (reply->error() != QNetworkReply::NoError || code >= 400) {
			QString msg = QJsonDocument::fromJson(body).object()
					      .value(QStringLiteral("message")).toString();
			if (msg.isEmpty())
				msg = tr("No se pudieron cargar tus directos de YouTube.");
			emit youtubeLiveError(msg);
			return;
		}
		emit youtubeLiveList(body);
	});
}

void BetterChatApi::setYouTubeSource(const QString &url)
{
	if (m_token.isEmpty()) {
		emit youtubeSourceSet(false, tr("No has iniciado sesión."));
		return;
	}
	// Fija la fuente manual de YouTube (sin auto -> no choca con el gating de plus)
	// y activa su chat. Reutiliza el mismo endpoint que el panel web.
	QJsonObject yt;
	yt.insert(QStringLiteral("enabled"), true);
	yt.insert(QStringLiteral("mode"), QStringLiteral("manual"));
	yt.insert(QStringLiteral("auto"), false);
	yt.insert(QStringLiteral("source"), url);
	QJsonObject body;
	body.insert(QStringLiteral("youtube"), yt);
	QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
	QNetworkReply *reply = m_net.post(apiRequest(QStringLiteral("/api/config"), true), payload);
	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		reply->deleteLater();
		int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		if (reply->error() != QNetworkReply::NoError || code >= 400) {
			emit youtubeSourceSet(false, tr("No se pudo fijar el directo."));
			return;
		}
		emit youtubeSourceSet(true, tr("Directo de YouTube conectado."));
	});
}

void BetterChatApi::setStreaming(bool active)
{
	if (m_token.isEmpty())
		return;
	QJsonObject body;
	body.insert(QStringLiteral("active"), active);
	QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
	QNetworkReply *reply =
		m_net.post(apiRequest(QStringLiteral("/api/plugin/streaming"), true), payload);
	connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

void BetterChatApi::fetchControl()
{
	if (m_token.isEmpty())
		return;
	QNetworkReply *reply = m_net.get(apiRequest(QStringLiteral("/api/control"), true));
	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		reply->deleteLater();
		int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		if (reply->error() != QNetworkReply::NoError || code >= 400)
			return; // silencioso: es un poll periódico
		emit controlState(reply->readAll());
	});
}

void BetterChatApi::createBet(const QString &title, const QStringList &options, int durationSec)
{
	if (m_token.isEmpty())
		return;
	QJsonArray opts;
	for (const QString &label : options) {
		QJsonObject o;
		o.insert(QStringLiteral("label"), label);
		opts.append(o);
	}
	QJsonObject body;
	body.insert(QStringLiteral("title"), title);
	body.insert(QStringLiteral("options"), opts);
	if (durationSec > 0)
		body.insert(QStringLiteral("durationSec"), durationSec);
	QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
	QNetworkReply *reply =
		m_net.post(apiRequest(QStringLiteral("/api/control/bet"), true), payload);
	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		reply->deleteLater();
		int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		if (reply->error() != QNetworkReply::NoError || code >= 400) {
			QString msg = tr("No se pudo crear la apuesta.");
			const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
			if (obj.contains(QStringLiteral("message")))
				msg = obj.value(QStringLiteral("message")).toString();
			emit betActionResult(false, msg);
			return;
		}
		emit betActionResult(true, tr("Apuesta creada."));
		fetchControl();
	});
}

void BetterChatApi::betAction(const QString &action, const QString &winningOption)
{
	if (m_token.isEmpty())
		return;
	QJsonObject body;
	if (action == QStringLiteral("resolve"))
		body.insert(QStringLiteral("winningOption"), winningOption);
	QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);
	QNetworkReply *reply = m_net.post(
		apiRequest(QStringLiteral("/api/control/bet/") + action, true), payload);
	connect(reply, &QNetworkReply::finished, this, [this, reply]() {
		reply->deleteLater();
		int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		if (reply->error() != QNetworkReply::NoError || code >= 400) {
			QString msg = tr("No se pudo completar la acción.");
			const QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
			if (obj.contains(QStringLiteral("message")))
				msg = obj.value(QStringLiteral("message")).toString();
			emit betActionResult(false, msg);
			return;
		}
		emit betActionResult(true, QString());
		fetchControl();
	});
}

void BetterChatApi::logout()
{
	if (!m_token.isEmpty()) {
		// Best-effort: avisar al servidor para revocar el token.
		QNetworkReply *reply =
			m_net.post(apiRequest(QStringLiteral("/api/plugin/logout"), true), QByteArray("{}"));
		connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
	}
	stopStatusPolling();
	stopChatStream();
	saveToken(QString());
	m_username.clear();
	m_overlayUrl.clear();
	m_live = false;
	m_platform.clear();
	emit loggedOut();
}

// ---- Multichat en vivo (SSE) ----

void BetterChatApi::startChatStream()
{
	if (m_token.isEmpty() || m_sseReply)
		return;
	QNetworkRequest req = apiRequest(QStringLiteral("/api/plugin/chat-stream"), true);
	req.setRawHeader("Accept", "text/event-stream");
	// Sin timeout: es un stream persistente.
	req.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
	m_sseBuffer.clear();
	m_sseReply = m_net.get(req);
	connect(m_sseReply, &QNetworkReply::readyRead, this, &BetterChatApi::handleSseData);
	connect(m_sseReply, &QNetworkReply::finished, this, [this]() {
		bool wasActive = m_sseReply != nullptr;
		if (m_sseReply) {
			m_sseReply->deleteLater();
			m_sseReply = nullptr;
		}
		m_sseWatchdog.stop();
		emit chatStreamStateChanged(false);
		// Reconexión automática mientras siga logueado (backoff simple).
		if (wasActive && !m_token.isEmpty())
			m_sseRetryTimer.start(3000);
	});
	// Configurar el timer de reintento una sola vez.
	if (!m_sseRetryTimer.isSingleShot()) {
		m_sseRetryTimer.setSingleShot(true);
		connect(&m_sseRetryTimer, &QTimer::timeout, this, [this]() {
			if (!m_token.isEmpty() && !m_sseReply)
				startChatStream();
		});
	}
	// Watchdog de conexión medio-abierta: si el server reinicia (deploy) el
	// socket puede quedar colgado sin que Qt emita `finished`, y el reintento
	// nunca arranca. Si no llega NADA en 60s (el server hace ping cada 25s),
	// abortamos: eso dispara `finished` -> reconexión. Configurar una sola vez.
	if (!m_sseWatchdog.isSingleShot()) {
		m_sseWatchdog.setSingleShot(true);
		connect(&m_sseWatchdog, &QTimer::timeout, this, [this]() {
			if (m_sseReply) {
				obs_log(LOG_WARNING, "[betterchat] SSE watchdog: sin datos 60s, reconectando");
				m_sseReply->abort(); // -> finished -> reintento
			}
		});
	}
	m_sseWatchdog.start(60000);
	emit chatStreamStateChanged(true);
}

void BetterChatApi::stopChatStream()
{
	m_sseRetryTimer.stop();
	m_sseWatchdog.stop();
	if (m_sseReply) {
		QNetworkReply *r = m_sseReply;
		m_sseReply = nullptr;
		r->abort();
		r->deleteLater();
		emit chatStreamStateChanged(false);
	}
	m_sseBuffer.clear();
}

void BetterChatApi::handleSseData()
{
	if (!m_sseReply)
		return;
	// Llegaron datos (mensaje real o ping `:`): la conexión está viva, rearmar
	// el watchdog de 60s.
	m_sseWatchdog.start(60000);
	m_sseBuffer += m_sseReply->readAll();
	// Los eventos SSE se separan por línea en blanco (\n\n).
	int idx;
	while ((idx = m_sseBuffer.indexOf("\n\n")) != -1) {
		QByteArray block = m_sseBuffer.left(idx);
		m_sseBuffer.remove(0, idx + 2);
		processSseEvent(block);
	}
}

void BetterChatApi::processSseEvent(const QByteArray &block)
{
	// Extraer las líneas 'data:' (ignorar comentarios ':' y 'retry:').
	QByteArray payload;
	for (const QByteArray &line : block.split('\n')) {
		if (line.startsWith("data:"))
			payload += line.mid(5).trimmed();
	}
	if (payload.isEmpty())
		return;
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(payload, &err);
	if (err.error != QJsonParseError::NoError || !doc.isObject())
		return;
	QJsonObject o = doc.object();
	const QString type = o.value(QStringLiteral("type")).toString();
	if (type == QStringLiteral("status")) {
		QJsonObject s = o.value(QStringLiteral("status")).toObject();
		emit platformStatus(s.value(QStringLiteral("platform")).toString(),
				    s.value(QStringLiteral("state")).toString(),
				    s.value(QStringLiteral("detail")).toString());
		return;
	}
	if (type == QStringLiteral("event")) {
		QJsonObject e = o.value(QStringLiteral("event")).toObject();
		emit chatEvent(e.value(QStringLiteral("platform")).toString(),
			       e.value(QStringLiteral("platformLabel")).toString(),
			       e.value(QStringLiteral("kind")).toString(),
			       e.value(QStringLiteral("actor")).toString(),
			       e.value(QStringLiteral("text")).toString(),
			       (int)e.value(QStringLiteral("amount")).toDouble(),
			       e.value(QStringLiteral("unit")).toString());
		return;
	}
	if (type != QStringLiteral("chat"))
		return; // solo mensajes de chat y eventos (ignorar status/clear/etc.)
	QJsonObject m = o.value(QStringLiteral("message")).toObject();
	// El multichat muestra el chat REAL, no los mensajes de prueba.
	if (m.value(QStringLiteral("test")).toBool() || m.value(QStringLiteral("synthetic")).toBool())
		return;
	const QString platform = m.value(QStringLiteral("platform")).toString();
	const QString label = m.value(QStringLiteral("platformLabel")).toString();
	QString author = m.value(QStringLiteral("displayName")).toString();
	if (author.isEmpty())
		author = m.value(QStringLiteral("username")).toString();
	const QString text = m.value(QStringLiteral("text")).toString();
	const QString textHtml = m.value(QStringLiteral("textHtml")).toString();
	const QString color = m.value(QStringLiteral("color")).toString();
	emit chatMessage(platform, label, author, text, color, textHtml);
}
