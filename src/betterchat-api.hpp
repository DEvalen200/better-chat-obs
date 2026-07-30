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

signals:
	// Emitido con el user_code y la verify_url para mostrarlos en el dock.
	void pairingStarted(const QString &userCode, const QString &verifyUrl);
	void pairingFailed(const QString &message);
	void loggedIn(); // token obtenido y validado
	void loggedOut();
	void statusUpdated(); // username/overlayUrl/live actualizados
	void statusError(const QString &message);

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

	// Estado del emparejamiento en curso.
	QString m_deviceCode;
	QTimer m_pairTimer;
	QTimer m_statusTimer;
	int m_pairElapsed = 0;
	int m_pairExpires = 0;
};
