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

struct calldata;
typedef struct calldata calldata_t;

class BetterChatApi;
class QStackedWidget;
class QLabel;
class QPushButton;
class QListWidget;
class QComboBox;
class QWidget;

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
	void onChatSettingChanged(); // aplica dirección/alineación a la seleccionada
	void onLogout();
	void refreshChatList();    // invocable desde los signals de OBS (por nombre)

private:
	void buildUi();
	void connectObsSignals();
	void disconnectObsSignals();
	void updateSettingsPanel(); // rellena los combos según la instancia seleccionada
	// Cambia a la escena que contiene la fuente y la selecciona en las fuentes.
	void focusSelectedChatInObs();
	// Reescribe un query param en la URL de la fuente seleccionada (dir/align).
	void setSelectedChatParam(const QString &key, const QString &value);
	// Trampolín estático para los signals globales de OBS (source_create/destroy/remove).
	static void obsSignalTrampoline(void *data, calldata_t *cd);
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
	// Panel de ajustes por instancia (dirección / alineación).
	QWidget *m_settingsPanel = nullptr;
	QComboBox *m_dirCombo = nullptr;
	QComboBox *m_alignCombo = nullptr;
	bool m_updatingPanel = false; // evita realimentar señales al rellenar los combos
	QPushButton *m_logoutBtn = nullptr;
	QLabel *m_actionStatus = nullptr;
};
