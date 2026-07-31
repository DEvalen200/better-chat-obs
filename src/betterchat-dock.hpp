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
class QListWidget;

// Dock acoplable de BetterChatTV dentro de OBS. Dos vistas:
//  - Deslogueado: botón para vincular la cuenta (device-flow).
//  - Logueado: identidad + directo, lista de instancias de chat (cada una su
//    propia fuente con su propio tamaño) y acciones para crear/añadir/quitar.
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
	void onCreateChat();       // crea una instancia NUEVA en la escena actual
	void onAddSelectedToScene(); // añade la seleccionada a la escena actual (referencia)
	void onRemoveSelected();   // elimina la instancia seleccionada
	void onLogout();

private:
	void buildUi();
	void refreshChatList();
	// Crea una fuente de chat con nombre único y la añade a la escena activa.
	void createChatInCurrentScene(const QString &url);

	BetterChatApi *m_api = nullptr;

	QStackedWidget *m_stack = nullptr;

	// Vista login.
	QPushButton *m_loginBtn = nullptr;
	QLabel *m_pairInfo = nullptr;
	QLabel *m_loginStatus = nullptr;

	// Vista logueado.
	QLabel *m_userLabel = nullptr;
	QLabel *m_liveBadge = nullptr;
	QListWidget *m_chatList = nullptr;
	QPushButton *m_createBtn = nullptr;
	QPushButton *m_addSelBtn = nullptr;
	QPushButton *m_removeSelBtn = nullptr;
	QPushButton *m_logoutBtn = nullptr;
	QLabel *m_actionStatus = nullptr;
};
