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

void BetterChatApi::logout()
{
	if (!m_token.isEmpty()) {
		// Best-effort: avisar al servidor para revocar el token.
		QNetworkReply *reply =
			m_net.post(apiRequest(QStringLiteral("/api/plugin/logout"), true), QByteArray("{}"));
		connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
	}
	stopStatusPolling();
	saveToken(QString());
	m_username.clear();
	m_overlayUrl.clear();
	m_live = false;
	m_platform.clear();
	emit loggedOut();
}
