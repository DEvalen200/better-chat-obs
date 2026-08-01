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
	// Limpia el chat del overlay (POST /api/plugin/clear).
	void clearChat();
	// Abre/cierra el stream SSE del multichat en vivo (/api/plugin/chat-stream).
	// Emite chatMessage() por cada mensaje real recibido.
	void startChatStream();
	void stopChatStream();
	bool chatStreamActive() const { return m_sseReply != nullptr; }

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
	// Multichat en vivo (SSE): un mensaje real de chat.
	void chatMessage(const QString &platform, const QString &platformLabel,
			 const QString &author, const QString &text, const QString &color);
	// Evento destacado (bits/donacion, sub, regalo de sub, superchat, regalo TikTok...).
	void chatEvent(const QString &platform, const QString &platformLabel,
		       const QString &kind, const QString &actor, const QString &text,
		       int amount, const QString &unit);
	// Estado de conexión de una plataforma en el stream (connecting/connected/off/...).
	void platformStatus(const QString &platform, const QString &state, const QString &detail);
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

	// Stream SSE del multichat en vivo.
	QNetworkReply *m_sseReply = nullptr;
	QByteArray m_sseBuffer;
	QTimer m_sseRetryTimer;
	void handleSseData();
	void processSseEvent(const QByteArray &block);

	// Estado del emparejamiento en curso.
	QString m_deviceCode;
	QTimer m_pairTimer;
	QTimer m_statusTimer;
	int m_pairElapsed = 0;
	int m_pairExpires = 0;
};
