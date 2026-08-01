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
#include <QList>
#include <QHash>

struct calldata;
typedef struct calldata calldata_t;

class BetterChatApi;
class QStackedWidget;
class QLabel;
class QPushButton;
class QListWidget;
class QTextEdit;
class QVBoxLayout;
class QComboBox;
class QWidget;
class QCheckBox;
class QTimer;
class QSlider;
class QSpinBox;

// Dock acoplable de BetterChatTV dentro de OBS. Dos vistas:
//  - Deslogueado: botón para vincular la cuenta (device-flow).
//  - Logueado: identidad + directo, lista de instancias de chat (cada una su
//    propia fuente con su propio tamaño) y acciones para crear/añadir/quitar.
class BetterChatDock : public QWidget {
	Q_OBJECT

public:
	explicit BetterChatDock(QWidget *parent = nullptr);
	~BetterChatDock() override;
	// Notifica el cambio de estado de transmisión de OBS al servidor (dispara la
	// auto-conexión de YouTube). La llama el callback del frontend de OBS.
	void onStreamingChanged(bool active);

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
	void onAutoTestToggled(bool on); // activa/desactiva mensajes de prueba automáticos
	void onClearChat();          // limpia el chat del overlay
	void onLogout();
	void refreshChatList();    // invocable desde los signals de OBS (por nombre)

private:
	void buildUi();
	void connectObsSignals();
	void disconnectObsSignals();
	void updateSettingsPanel(); // rellena los combos según la instancia seleccionada
	void selectTab(int index);  // cambia la pestaña activa de la nav bar
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

	// Nav bar de pestañas (visible solo logueado) + stack interno de pestañas.
	QWidget *m_navBar = nullptr;
	QStackedWidget *m_tabStack = nullptr;
	QList<QPushButton *> m_tabButtons;

	// Vista login.
	QPushButton *m_loginBtn = nullptr;
	QLabel *m_pairInfo = nullptr;
	QLabel *m_loginStatus = nullptr;

	// Vista logueado.
	QLabel *m_brandTitle = nullptr; // logo "BetterChatTV" (pasa a "+" si es plus)
	QLabel *m_userLabel = nullptr;
	QLabel *m_liveBadge = nullptr;
	QListWidget *m_chatList = nullptr;
	QPushButton *m_createBtn = nullptr;
	QPushButton *m_addSelBtn = nullptr;
	// Panel de ajustes por instancia (dirección / alineación / escala).
	QWidget *m_settingsPanel = nullptr;
	QComboBox *m_dirCombo = nullptr;
	QComboBox *m_alignCombo = nullptr;
	QSlider *m_scaleSlider = nullptr;
	QSpinBox *m_scaleSpin = nullptr;
	bool m_updatingPanel = false; // evita realimentar señales al rellenar los combos
	QPushButton *m_logoutBtn = nullptr;

	// Pestaña Multichat: chat en vivo combinado (SSE).
	QTextEdit *m_multiChat = nullptr;
	QLabel *m_multiStatus = nullptr;
	QWidget *m_platBar = nullptr;                 // fila de chips de estado por plataforma
	QHash<QString, QLabel *> m_platChips;         // platform -> chip
	bool m_iconsReady = false; // iconos de plataforma registrados en el documento
	QHash<QString, QString> m_platIconB64; // platform -> PNG base64 (para data URI en QLabel)
	void registerPlatformIcons();
	void buildPlatformBar(QVBoxLayout *parent);   // crea la fila de chips
	void onPlatformStatus(const QString &platform, const QString &state, const QString &detail);
	// Selector de directos de YouTube (para no-plus: elegir el directo a mano).
	QWidget *m_ytPicker = nullptr;      // panel plegable con la lista
	QVBoxLayout *m_ytList = nullptr;    // contenedor de los directos
	QLabel *m_ytPickerMsg = nullptr;    // estado/mensajes del selector
	QLabel *m_ytSearching = nullptr;    // indicador "buscando tu directo" (auto-conexión)
	QLabel *m_ytHelp = nullptr;         // ayuda contextual (plus: auto / standard: manual)
	QPushButton *m_ytPickBtn = nullptr; // botón que abre/refresca el selector
	void updateYouTubeHelp();           // refresca el texto de ayuda según isPlus/vinculado
	void toggleYouTubePicker();
	void onYouTubeLiveList(const QByteArray &json);
	void onYouTubeLiveError(const QString &message);
	void onChatMessage(const QString &platform, const QString &platformLabel,
			   const QString &author, const QString &text, const QString &color);
	void onChatEvent(const QString &platform, const QString &platformLabel,
			 const QString &kind, const QString &actor, const QString &text,
			 int amount, const QString &unit);
	QLabel *m_actionStatus = nullptr;
	// Herramientas de prueba (mensajes automáticos + limpiar).
	QCheckBox *m_autoTestCheck = nullptr;
	QPushButton *m_clearBtn = nullptr;
};
