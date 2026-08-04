/*
BetterChatTV para OBS
Copyright (C) 2026 DEValen

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#pragma once

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QTimer>

// Cliente HTTP de la API de BetterChatTV. Encapsula el device-flow de login
// (pair/start -> abrir navegador -> poll pair/token) y el sondeo de estado
// (/api/plugin/status). Guarda el token del plugin en QSettings.
class BetterChatApi : public QObject {
	Q_OBJECT

public:
	explicit BetterChatApi(QObject *parent = nullptr);

	bool isLoggedIn() const { return !m_token.isEmpty(); }
	QString baseUrl() const { return m_baseUrl; }
	QString username() const { return m_username; }
	QString overlayUrl() const { return m_overlayUrl; }
	bool isLive() const { return m_live; }
	QString platform() const { return m_platform; }

	// Empieza el emparejamiento: pide un par device/user code y abre el navegador.
	void startPairing();
	// Cancela el poll de emparejamiento en curso.
	void cancelPairing();
	// Revoca el token en el servidor y lo borra en local.
	void logout();
	// Fuerza una consulta de estado inmediata.
	void refreshStatus();
	// Arranca/para el sondeo periodico de estado (directo/no directo).
	void startStatusPolling(int intervalMs = 20000);
	void stopStatusPolling();
	// Emite un mensaje de prueba automático en el overlay (POST /api/plugin/test-message).
	void sendAutoTestMessage();
	// Activa/desactiva los mensajes de prueba automáticos (sincronizado con la web).
	void setAutoTest(bool active);
	// Estado actual del auto-test (según el último status recibido).
	bool autoTestActive() const { return m_autoTest; }
	// ¿El dueño tiene BetterChatTV+? (branding "+" y apuestas premium).
	bool isPlus() const { return m_isPlus; }
	// ¿Tiene la cuenta de YouTube vinculada?
	bool youtubeLinked() const { return m_youtubeLinked; }
	// Limpia el chat del overlay (POST /api/plugin/clear).
	void clearChat();
	// Abre/cierra el stream SSE del multichat en vivo (/api/plugin/chat-stream).
	// Emite chatMessage() por cada mensaje real recibido.
	void startChatStream();
	void stopChatStream();
	bool chatStreamActive() const { return m_sseReply != nullptr; }
	// Lista los directos activos de YouTube del streamer (cuenta ya vinculada).
	// Emite youtubeLiveList() con [{videoId,title,privacy}] o youtubeLiveError().
	void fetchYouTubeLive();
	// Fija la fuente de YouTube (URL del directo elegido) y activa su chat.
	// Emite youtubeSourceSet(ok, message).
	void setYouTubeSource(const QString &url);
	// Informa al servidor de si OBS está transmitiendo (dispara la auto-conexión
	// de YouTube sin esperar a que Twitch/Kick reporten el en vivo).
	void setStreaming(bool active);
	// --- Apuestas (pestaña Minijuegos) ---
	// Pide el estado del panel de control (incluye la apuesta activa). Emite
	// controlState(QByteArray json) con el objeto completo de /api/control.
	void fetchControl();
	// Crea una apuesta. options = lista de etiquetas. durationSec 0 = sin cierre auto.
	// Emite betActionResult(ok, message) y refresca via controlState.
	void createBet(const QString &title, const QStringList &options, int durationSec);
	// Acción sobre la apuesta activa: "lock" | "resolve" | "cancel".
	// Para resolve, winningOption = id de la opción ganadora. Emite betActionResult.
	void betAction(const QString &action, const QString &winningOption = QString());

signals:
	// Emitido con el user_code y la verify_url para mostrarlos en el dock.
	void pairingStarted(const QString &userCode, const QString &verifyUrl);
	void pairingFailed(const QString &message);
	void loggedIn(); // token obtenido y validado
	void loggedOut();
	void statusUpdated(); // username/overlayUrl/live actualizados
	void statusError(const QString &message);
	// Resultado de una acción de chat (test/clear): ok o mensaje de error.
	void chatActionResult(bool ok, const QString &message);
	// Multichat en vivo (SSE): un mensaje real de chat. textHtml trae el texto
	// con emotes/emojis como <img> (para renderizar emojis en el multichat).
	void chatMessage(const QString &platform, const QString &platformLabel,
			 const QString &author, const QString &text, const QString &color,
			 const QString &textHtml);
	// Evento destacado (bits/donacion, sub, regalo de sub, superchat, regalo TikTok...).
	void chatEvent(const QString &platform, const QString &platformLabel,
		       const QString &kind, const QString &actor, const QString &text,
		       int amount, const QString &unit);
	// Estado de conexión de una plataforma en el stream (connecting/connected/off/...).
	void platformStatus(const QString &platform, const QString &state, const QString &detail);
	// Lista de directos activos de YouTube (JSON crudo de /api/youtube/live-streams).
	void youtubeLiveList(const QByteArray &json);
	void youtubeLiveError(const QString &message);
	// Resultado de fijar la fuente de YouTube.
	void youtubeSourceSet(bool ok, const QString &message);
	// Estado del panel de control (JSON de /api/control): apuesta activa, isPlus, etc.
	void controlState(const QByteArray &json);
	// Resultado de una acción de apuesta (crear/lock/resolve/cancel).
	void betActionResult(bool ok, const QString &message);
	void chatStreamStateChanged(bool connected);

private slots:
	void pollPairing();

private:
	void loadToken();
	void saveToken(const QString &token);
	QNetworkRequest apiRequest(const QString &path, bool withAuth) const;

	QNetworkAccessManager m_net;
	QString m_baseUrl;
	QString m_token;
	QString m_username;
	QString m_overlayUrl;
	QString m_platform;
	bool m_live = false;
	bool m_autoTest = false; // estado de los mensajes de prueba automáticos (sincronía)
	bool m_isPlus = false;        // BetterChatTV+ (branding "+" y apuestas premium)
	bool m_youtubeLinked = false; // cuenta de YouTube vinculada

	// Stream SSE del multichat en vivo.
	QNetworkReply *m_sseReply = nullptr;
	QByteArray m_sseBuffer;
	QTimer m_sseRetryTimer;
	// Watchdog: detecta conexiones SSE medio-abiertas (el server reinicia y el
	// socket queda colgado sin FIN/RST, por lo que `finished` nunca se dispara).
	// Se reinicia con cada byte recibido (el server manda `: ping` cada 25s); si
	// pasan >60s sin datos, se asume muerta y se fuerza reconexion.
	QTimer m_sseWatchdog;
	void handleSseData();
	void processSseEvent(const QByteArray &block);

	// Estado del emparejamiento en curso.
	QString m_deviceCode;
	QTimer m_pairTimer;
	QTimer m_statusTimer;
	int m_pairElapsed = 0;
	int m_pairExpires = 0;
};
