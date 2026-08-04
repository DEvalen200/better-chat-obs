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
#include <QSet>
#include <QJsonArray>

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
class QLineEdit;
class QScrollArea;
class QFrame;
class QNetworkAccessManager;
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

protected:
	bool eventFilter(QObject *obj, QEvent *ev) override;

public:
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
	QWidget *m_topRow = nullptr;    // fila usuario + Dashboard + directo (se oculta en fullscreen)
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
	void buildYouTubePicker(QVBoxLayout *parent); // zona de elegir directo YT (al pie)
	// --- Pestaña Minijuegos: apuestas con fichas ---
	void buildBetsTab(QVBoxLayout *parent, QWidget *tab);
	// Intenta montar la pestaña de apuestas como WEB EMBEBIDA (QCef -> /control via
	// /plugin/bridge). Devuelve el widget embebido, o nullptr si obs-browser/CEF no
	// está disponible (entonces se usa buildBetsTab nativo como fallback).
	QWidget *buildBetsTabEmbedded();
	void loadBetsEmbedUrl();           // (re)carga la URL del bridge en el QCef al loguear
	QWidget *m_betsEmbed = nullptr;    // QCefWidget (como QWidget) si va embebido
	bool m_betsEmbedded = false;       // true si la pestaña de apuestas es web embebida
	QPushButton *m_embedReloadBtn = nullptr; // botón Recargar del pie (solo en apuestas)
	void onControlState(const QByteArray &json);   // refresca el estado de la apuesta
	void onCreateBetClicked();                      // crea la apuesta desde el formulario
	void refreshBets();                             // pide /api/control (poll)
	void updateBetView();                           // decide qué zona mostrar (grid/crear/activo)
	QWidget *m_betGrid = nullptr;      // galería de tipos de minijuego/apuesta
	QFrame *m_betsBlock = nullptr;     // bloque turquesa contenedor (estética web)
	QPushButton *m_betAddOptBtn = nullptr; // botón "+ Añadir opción" (tope 20)
	QString m_pickedType;              // tipo elegido en el grid ("" | "manual")
	bool m_lastIsPlus = false;         // último estado de control recibido
	bool m_lastHasBet = false;
	bool m_betDataReady = false;       // ya llegó el primer /api/control
	QWidget *m_betCreate = nullptr;    // formulario de nueva apuesta
	QWidget *m_betActive = nullptr;    // panel de la apuesta en curso
	QLabel *m_betGateMsg = nullptr;    // aviso para no-plus
	QLineEdit *m_betTitle = nullptr;   // título de la apuesta
	QVBoxLayout *m_betOptInputs = nullptr; // contenedor de los campos de opción (dinámico)
	QList<QLineEdit *> m_betOptFields;     // campos de opción actuales
	QWidget *m_betCreateForm = nullptr;    // widget padre de los campos (para nuevos hijos)
	void addBetOptionField(const QString &text = QString()); // añade un campo de opción
	void renumberBetOptions();                               // renumera placeholders "Opción N"
	void resetBetOptions();                                  // deja 2 opciones vacías
	void fillBetForm(const QString &title, const QStringList &options); // "repetir" del historial
	void markBetFieldError(QLineEdit *field);                // resalta un campo en rojo
	void clearBetFieldErrors();                              // quita el resaltado de error
	QComboBox *m_betDuration = nullptr;// duración (cierre automático)
	QPushButton *m_betCreateBtn = nullptr;
	QLabel *m_betMsg = nullptr;        // mensajes de resultado
	QWidget *m_betHistory = nullptr;   // tarjeta de historial de predicciones
	QVBoxLayout *m_betHistoryBox = nullptr; // filas del historial
	QJsonArray m_betHistoryData;       // historial completo recibido
	int m_betHistoryPage = 0;          // página actual (5 por página)
	QLabel *m_betHistoryPageLbl = nullptr;
	QPushButton *m_betHistPrev = nullptr;
	QPushButton *m_betHistNext = nullptr;
	void renderHistoryPage();          // pinta la página actual del historial
	int m_lastHistoryCount = 0;        // nº de predicciones en el historial
	QLabel *m_betActiveTitle = nullptr;// título de la apuesta activa
	QLabel *m_betActiveBadge = nullptr;// badge de estado (Abierta/Cerrada) en la cabecera
	QLabel *m_betPotPill = nullptr;    // pill del bote (fichas)
	QLabel *m_betTimePill = nullptr;   // pill de la cuenta atrás (icono + tiempo)
	QLabel *m_betActiveState = nullptr;// estado (abierta/bloqueada) + bote
	QVBoxLayout *m_betOptsBox = nullptr;// filas de opciones con barras
	QPushButton *m_betLockBtn = nullptr;
	QPushButton *m_betCancelBtn = nullptr;
	QTimer *m_betPollTimer = nullptr;  // refresco periódico del estado
	QString m_activeBetStatus;         // "", "open", "locked"
	QScrollArea *m_betsScroll = nullptr; // scroll de la pestaña de apuestas (para subir arriba al crear)
	QString m_lastCreatedBetId;        // id de la última apuesta que YO acabo de crear (para subir el scroll una sola vez)
	QString m_betLockAt;               // ISO del cierre automático de la apuesta activa (vacío = manual)
	QTimer *m_betCountdownTimer = nullptr; // refresca el "quedan Xs" cada segundo
	qint64 m_lastBote = 0;             // último bote conocido (para repintar el label sin re-parsear)
	// Pinta el label de estado de la apuesta (bote + abierta/cerrada + cuenta atrás).
	void refreshBetStateLabel(const QString &status, qint64 bote);
	QString m_clockIconB64;            // reloj Lucide oscuro (pill normal)
	QString m_clockIconB64White;       // reloj Lucide blanco (pill urgente rojo)
	QString clockIconB64(bool white = false); // lo genera la 1ª vez y lo cachea
	// --- Modo pantalla completa del multichat ---
	QPushButton *m_fsBtn = nullptr;   // botón flotante en la esquina del chat
	QWidget *m_ytZone = nullptr;      // contenedor de toda la zona YT (para ocultar)
	QLabel *m_verLabel = nullptr;     // etiqueta de versión (para ocultar en fullscreen)
	bool m_chatFullscreen = false;
	QIcon makeExpandIcon(bool expanded);
	QIcon makeTrashIcon();
	QIcon makeChartIcon();
	QIcon makeCopyIcon();
	// Icono "abrir en el navegador" (recuadro con flecha saliente, estilo Lucide
	// external-link). color = trazo; para botones Sala/Dashboard del dock.
	QIcon makeExternalLinkIcon(const QColor &color);
	QIcon makeReloadIcon(); // flecha circular (recargar panel embebido)
	void positionFsButton();
	void toggleChatFullscreen();
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
			   const QString &author, const QString &text, const QString &color,
			   const QString &textHtml);
	// Descarga los emotes (<img>) de un mensaje y los registra en el documento del
	// multichat para que se muestren; refresca el render al llegar cada imagen.
	void fetchEmotesFor(const QString &html);
	QNetworkAccessManager *m_emoteNet = nullptr;
	QSet<QString> m_emoteFetched; // URLs ya pedidas (no repetir)
	void onChatEvent(const QString &platform, const QString &platformLabel,
			 const QString &kind, const QString &actor, const QString &text,
			 int amount, const QString &unit);
	QLabel *m_actionStatus = nullptr;
	// Herramientas de prueba (mensajes automáticos + limpiar).
	QCheckBox *m_autoTestCheck = nullptr;
	QPushButton *m_clearBtn = nullptr;
};
