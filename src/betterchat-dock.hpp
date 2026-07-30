/*
BetterChatTV para OBS
Copyright (C) 2026 DEValen

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#pragma once

#include <QWidget>

class BetterChatApi;
class QStackedWidget;
class QLabel;
class QPushButton;

// Dock acoplable de BetterChatTV dentro de OBS. Dos vistas:
//  - Deslogueado: botón para vincular la cuenta (device-flow).
//  - Logueado: nombre de usuario, indicador de directo, botón "Añadir chat
//    a la escena" y "Cerrar sesión".
class BetterChatDock : public QWidget {
	Q_OBJECT

public:
	explicit BetterChatDock(QWidget *parent = nullptr);
	~BetterChatDock() override;

private slots:
	void onStartLogin();
	void onPairingStarted(const QString &userCode, const QString &verifyUrl);
	void onPairingFailed(const QString &message);
	void onLoggedIn();
	void onLoggedOut();
	void onStatusUpdated();
	void onAddChatSource();
	void onLogout();

private:
	void buildUi();
	void addBrowserSourceToCurrentScene(const QString &url);

	BetterChatApi *m_api = nullptr;

	QStackedWidget *m_stack = nullptr;

	// Vista login.
	QPushButton *m_loginBtn = nullptr;
	QLabel *m_pairInfo = nullptr;
	QLabel *m_loginStatus = nullptr;

	// Vista logueado.
	QLabel *m_userLabel = nullptr;
	QLabel *m_liveBadge = nullptr;
	QPushButton *m_addSourceBtn = nullptr;
	QPushButton *m_logoutBtn = nullptr;
	QLabel *m_actionStatus = nullptr;
};
