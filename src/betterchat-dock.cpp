/*
BetterChatTV para OBS
Copyright (C) 2026 DEValen

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#include "betterchat-dock.hpp"
#include "betterchat-api.hpp"

#include <obs.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTextEdit>
#include <QTextCursor>
#include <QTextDocument>
#include <QImage>
#include <QPainterPath>
#include <QScrollBar>
#include <QTextOption>
#include <QAbstractItemView>
#include <QHash>
#include <QScrollArea>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QTimer>
#include <QtMath>
#include <QUrl>
#include <QUrlQuery>
#include <QDesktopServices>
#include <QIcon>
#include <QPixmap>
#include <QSize>
#include <QPainter>
#include <QPen>
#include <QColor>
#include <QPaintEvent>
#include <QSizePolicy>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDockWidget>
#include <QSignalBlocker>

// Estilo del rebrand: negro cálido rosáceo, plano, acento rosa/turquesa.
static const char *kStyle = R"CSS(
QWidget#bcRoot { background: #17110f; color: #f6eeea; font-family: 'Segoe UI', sans-serif; }
QLabel { color: #f6eeea; }
QLabel#muted { color: #b9a49c; font-size: 12px; }
QLabel#title { font-size: 15px; font-weight: 700; }
QPushButton#primary {
	background: #ff4d8d; color: #1a0c12; border: 0; border-radius: 8px;
	padding: 9px 14px; font-weight: 700;
}
QPushButton#primary:disabled { background: #6b3450; color: #b98aa0; }
QPushButton#ghost {
	background: transparent; color: #b9a49c; border: 1px solid #3a2a25;
	border-radius: 8px; padding: 7px 12px;
}
QPushButton#accent {
	background: #38d39f; color: #06231a; border: 0; border-radius: 8px;
	padding: 7px 12px; font-weight: 700;
}
QPushButton#accent:hover { background: #4fdcac; }
QPushButton#accent:disabled { background: #26514a; color: #6f8f87; }
QPushButton#danger {
	background: #ff5b6a; color: #2a0006; border: 0; border-radius: 8px;
	padding: 7px 12px; font-weight: 700;
}
QPushButton#danger:hover { background: #ff7280; }
QPushButton#danger:disabled { background: #5c2b30; color: #a97b80; }
QLabel#liveOn {
	background: #38d39f; color: #06231a; border-radius: 6px;
	padding: 3px 9px; font-weight: 700; font-size: 12px;
}
QLabel#liveOff {
	background: #2b1f1b; color: #b9a49c; border: 1px solid #3a2a25;
	border-radius: 6px; padding: 3px 9px; font-size: 12px;
}
QLabel#code {
	background: #2b1f1b; border: 1px solid #3a2a25; border-radius: 8px;
	padding: 10px; font-family: monospace; font-size: 20px; font-weight: 700;
	letter-spacing: 3px;
}
QFrame#card { background: #211815; border: 1px solid #3a2a25; border-radius: 12px; }
QListWidget#chatList {
	background: #211815; border: 1px solid #3a2a25; border-radius: 8px;
	color: #f6eeea; padding: 4px; outline: 0;
}
QListWidget#chatList::item { padding: 3px 2px; border-radius: 6px; }
QListWidget#chatList::item:selected { background: #ff4d8d; color: #1a0c12; }
QPushButton#ghost:disabled { color: #6b5a53; border-color: #2b1f1b; }
QPushButton#trash {
	background: transparent; border: 1px solid transparent; border-radius: 6px;
}
QPushButton#trash:hover { background: #3a1f28; border-color: #ff4d8d; }
QComboBox {
	background: #2b1f1b; color: #f6eeea; border: 1px solid #3a2a25;
	border-radius: 6px; padding: 4px 8px;
}
QComboBox QAbstractItemView {
	background: #2b1f1b; color: #f6eeea; selection-background-color: #ff4d8d;
	selection-color: #1a0c12; border: 1px solid #3a2a25;
}
QCheckBox { color: #f6eeea; font-size: 12px; spacing: 8px; }
QFrame#sep { color: #3a2a25; max-height: 1px; }
QWidget#navBar { background: #211815; border-radius: 9px; }
QPushButton#navTab {
	background: transparent; color: #b9a49c; border: 0;
	border-bottom: 2px solid transparent; border-radius: 0;
	padding: 8px 4px; font-size: 12px; font-weight: 600;
}
QPushButton#navTab:hover { color: #f6eeea; }
QPushButton#navTab:checked {
	color: #ff4d8d; border-bottom: 2px solid #ff4d8d;
}
QScrollArea#scrollPage { background: transparent; }
QScrollArea#scrollPage > QWidget > QWidget { background: transparent; }
QListWidget#multiList {
	background: #211815; border: 1px solid #3a2a25; border-radius: 8px;
	color: #f6eeea; padding: 4px; outline: 0;
}
QListWidget#multiList::item { border-bottom: 1px solid #2b1f1b; }
QTextEdit#multiChat {
	background: #211815; border: 1px solid #3a2a25; border-radius: 8px;
	color: #f6eeea; padding: 4px;
}
QSlider::groove:horizontal { height: 4px; background: #3a2a25; border-radius: 2px; }
QSlider::sub-page:horizontal { background: #ff4d8d; border-radius: 2px; }
QSlider::handle:horizontal {
	width: 14px; height: 14px; margin: -6px 0; border-radius: 7px;
	background: #ff4d8d; border: 2px solid #1a0c12;
}
QSpinBox {
	background: #2b1f1b; color: #f6eeea; border: 1px solid #3a2a25;
	border-radius: 6px; padding: 2px 4px; min-width: 54px;
}
)CSS";

// Interruptor deslizante (estilo de la web): track redondeado + círculo que se
// desliza, turquesa cuando está activo. Hereda de QCheckBox para conservar toda la
// lógica (toggled/setChecked/isChecked) intacta; solo cambia cómo se pinta.
class ToggleSwitch : public QCheckBox {
public:
	explicit ToggleSwitch(QWidget *parent = nullptr) : QCheckBox(parent)
	{
		setCursor(Qt::PointingHandCursor);
	}
	QSize sizeHint() const override { return QSize(46, 24); }

protected:
	void paintEvent(QPaintEvent *) override
	{
		const int w = 42, h = 22;
		const int y = (height() - h) / 2;
		QPainter p(this);
		p.setRenderHint(QPainter::Antialiasing, true);
		// Track.
		QColor track = isChecked() ? QColor("#38d39f") : QColor("#6b5850");
		if (!isEnabled())
			track.setAlpha(110);
		p.setPen(Qt::NoPen);
		p.setBrush(track);
		p.drawRoundedRect(0, y, w, h, h / 2.0, h / 2.0);
		// Perilla.
		int d = h - 6;
		int kx = isChecked() ? (w - d - 3) : 3;
		p.setBrush(QColor("#ffffff"));
		p.drawEllipse(kx, y + 3, d, d);
		// Texto a la derecha del switch.
		if (!text().isEmpty()) {
			p.setPen(QColor(isEnabled() ? "#f6eeea" : "#8a756c"));
			QRect tr(w + 8, 0, width() - w - 8, height());
			p.drawText(tr, Qt::AlignVCenter | Qt::AlignLeft, text());
		}
	}
};

BetterChatDock::BetterChatDock(QWidget *parent) : QWidget(parent)
{
	m_api = new BetterChatApi(this);

	buildUi();

	connect(m_api, &BetterChatApi::pairingStarted, this, &BetterChatDock::onPairingStarted);
	connect(m_api, &BetterChatApi::pairingFailed, this, &BetterChatDock::onPairingFailed);
	connect(m_api, &BetterChatApi::loggedIn, this, &BetterChatDock::onLoggedIn);
	connect(m_api, &BetterChatApi::loggedOut, this, &BetterChatDock::onLoggedOut);
	connect(m_api, &BetterChatApi::statusUpdated, this, &BetterChatDock::onStatusUpdated);
	connect(m_api, &BetterChatApi::chatMessage, this, &BetterChatDock::onChatMessage);
	connect(m_api, &BetterChatApi::chatEvent, this, &BetterChatDock::onChatEvent);
	connect(m_api, &BetterChatApi::platformStatus, this, &BetterChatDock::onPlatformStatus);
	connect(m_api, &BetterChatApi::chatStreamStateChanged, this, [this](bool connected) {
		if (m_multiStatus && m_tabStack && m_tabStack->currentIndex() == 1)
			m_multiStatus->setText(connected
				? QStringLiteral("Conectado. Esperando mensajes del chat en vivo...")
				: QStringLiteral("Reconectando al chat en vivo..."));
	});
	connect(m_api, &BetterChatApi::chatActionResult, this, [this](bool ok, const QString &msg) {
		if (!msg.isEmpty())
			m_actionStatus->setText(msg);
		// Si falla activar el auto (p.ej. sin overlay), reflejar que sigue apagado.
		if (!ok && m_autoTestCheck && m_autoTestCheck->isChecked()) {
			m_updatingPanel = true;
			m_autoTestCheck->setChecked(false);
			m_updatingPanel = false;
		}
	});

	if (m_api->isLoggedIn()) {
		m_stack->setCurrentIndex(1);
		if (m_navBar)
			m_navBar->setVisible(true);
		m_api->startStatusPolling();
		refreshChatList();
	} else {
		m_stack->setCurrentIndex(0);
	}

	// Escuchar eventos globales de OBS para refrescar la lista al instante
	// cuando se crean/eliminan fuentes desde OBS (no solo desde el plugin).
	connectObsSignals();
}

BetterChatDock::~BetterChatDock()
{
	disconnectObsSignals();
}

// Trampolín para los signals globales de OBS. Llega desde OTRO hilo, así que
// solo marshala un refresco al hilo de UI de Qt (nunca tocar widgets aquí).
void BetterChatDock::obsSignalTrampoline(void *data, calldata_t *)
{
	auto *self = static_cast<BetterChatDock *>(data);
	QMetaObject::invokeMethod(self, "refreshChatList", Qt::QueuedConnection);
}

void BetterChatDock::connectObsSignals()
{
	signal_handler_t *sh = obs_get_signal_handler();
	if (!sh)
		return;
	signal_handler_connect(sh, "source_create", &BetterChatDock::obsSignalTrampoline, this);
	signal_handler_connect(sh, "source_destroy", &BetterChatDock::obsSignalTrampoline, this);
	signal_handler_connect(sh, "source_remove", &BetterChatDock::obsSignalTrampoline, this);
}

void BetterChatDock::disconnectObsSignals()
{
	signal_handler_t *sh = obs_get_signal_handler();
	if (!sh)
		return;
	signal_handler_disconnect(sh, "source_create", &BetterChatDock::obsSignalTrampoline, this);
	signal_handler_disconnect(sh, "source_destroy", &BetterChatDock::obsSignalTrampoline, this);
	signal_handler_disconnect(sh, "source_remove", &BetterChatDock::obsSignalTrampoline, this);
}

void BetterChatDock::buildUi()
{
	setObjectName(QStringLiteral("bcRoot"));
	setStyleSheet(QString::fromUtf8(kStyle));

	auto *outer = new QVBoxLayout(this);
	outer->setContentsMargins(12, 12, 12, 12);
	outer->setSpacing(10);

	auto *title = new QLabel(QStringLiteral("BetterChatTV"), this);
	title->setObjectName(QStringLiteral("title"));
	outer->addWidget(title);

	// Nav bar de pestañas (solo visible cuando hay sesión). Cada botón cambia
	// m_tabStack. Se construye aquí pero se muestra desde onLoggedIn/onLoggedOut.
	m_navBar = new QWidget(this);
	m_navBar->setObjectName(QStringLiteral("navBar"));
	auto *navLayout = new QHBoxLayout(m_navBar);
	navLayout->setContentsMargins(6, 2, 6, 2);
	navLayout->setSpacing(2);
	const QStringList tabNames = {QStringLiteral("Ajustes de chat"), QStringLiteral("Multichat"),
				      QStringLiteral("Minijuegos/Apuestas")};
	for (int i = 0; i < tabNames.size(); i++) {
		auto *b = new QPushButton(tabNames[i], m_navBar);
		b->setObjectName(QStringLiteral("navTab"));
		b->setCheckable(true);
		b->setCursor(Qt::PointingHandCursor);
		b->setChecked(i == 0);
		connect(b, &QPushButton::clicked, this, [this, i]() { selectTab(i); });
		navLayout->addWidget(b, 1);
		m_tabButtons.append(b);
	}
	m_navBar->setVisible(false);
	outer->addWidget(m_navBar);

	m_stack = new QStackedWidget(this);
	outer->addWidget(m_stack, 1);

	// ---- Vista 0: login ----
	{
		auto *page = new QWidget(m_stack);
		auto *v = new QVBoxLayout(page);
		v->setContentsMargins(0, 0, 0, 0);
		v->setSpacing(10);

		auto *hint = new QLabel(
			QStringLiteral("Vincula tu cuenta para añadir el chat a OBS y ver si estás en directo."), page);
		hint->setObjectName(QStringLiteral("muted"));
		hint->setWordWrap(true);
		v->addWidget(hint);

		m_loginBtn = new QPushButton(QStringLiteral("Vincular mi cuenta"), page);
		m_loginBtn->setObjectName(QStringLiteral("primary"));
		connect(m_loginBtn, &QPushButton::clicked, this, &BetterChatDock::onStartLogin);
		v->addWidget(m_loginBtn);

		m_pairInfo = new QLabel(page);
		m_pairInfo->setObjectName(QStringLiteral("code"));
		m_pairInfo->setAlignment(Qt::AlignCenter);
		m_pairInfo->setVisible(false);
		v->addWidget(m_pairInfo);

		m_loginStatus = new QLabel(page);
		m_loginStatus->setObjectName(QStringLiteral("muted"));
		m_loginStatus->setWordWrap(true);
		v->addWidget(m_loginStatus);

		v->addStretch(1);
		m_stack->addWidget(page);
	}

	// ---- Vista 1: logueado ----
	{
		auto *page = new QWidget(m_stack);
		auto *outerV = new QVBoxLayout(page);
		outerV->setContentsMargins(0, 0, 0, 0);
		outerV->setSpacing(10);

		// Fila superior común a todas las pestañas: usuario + Dashboard + directo.
		auto *row = new QHBoxLayout();
		m_userLabel = new QLabel(page);
		m_userLabel->setObjectName(QStringLiteral("title"));
		row->addWidget(m_userLabel, 1);
		// Botón "Dashboard ↗" que abre el panel de BetterChatTV en el navegador.
		auto *dashBtn = new QPushButton(QStringLiteral("Dashboard ↗"), page);
		dashBtn->setObjectName(QStringLiteral("accent"));
		dashBtn->setCursor(Qt::PointingHandCursor);
		connect(dashBtn, &QPushButton::clicked, this, [this]() {
			QString base = m_api->baseUrl();
			if (base.isEmpty())
				base = QStringLiteral("https://betterchat.tv");
			QDesktopServices::openUrl(QUrl(base + QStringLiteral("/dashboard")));
		});
		row->addWidget(dashBtn, 0);
		m_liveBadge = new QLabel(page);
		m_liveBadge->setObjectName(QStringLiteral("liveOff"));
		m_liveBadge->setText(QStringLiteral("No estás en directo"));
		row->addWidget(m_liveBadge, 0, Qt::AlignRight);
		outerV->addLayout(row);

		// Stack de pestañas (lo cambia la nav bar).
		m_tabStack = new QStackedWidget(page);
		outerV->addWidget(m_tabStack, 1);

		// ===== Pestaña 0: Ajustes de chat =====
		auto *tabChat = new QWidget(m_tabStack);
		auto *v = new QVBoxLayout(tabChat);
		v->setContentsMargins(0, 0, 0, 16); // margen inferior (que "Cerrar sesión" respire)
		v->setSpacing(10);

		auto *listLabel = new QLabel(QStringLiteral("Tus chats en OBS"), page);
		listLabel->setObjectName(QStringLiteral("muted"));
		v->addWidget(listLabel);

		// Lista de instancias de chat existentes (cada una su propio tamaño).
		m_chatList = new QListWidget(page);
		m_chatList->setObjectName(QStringLiteral("chatList"));
		// Altura acotada: dentro del scroll de la página, evita que la lista se
		// estire sin límite y empuje los controles fuera de vista.
		m_chatList->setMinimumHeight(70);
		m_chatList->setMaximumHeight(180);
		connect(m_chatList, &QListWidget::itemSelectionChanged, this, [this]() {
			bool sel = m_chatList->currentItem() != nullptr;
			m_addSelBtn->setEnabled(sel);
			updateSettingsPanel();
		});
		// Solo al CLICAR el usuario (no en refrescos programáticos) saltamos a la
		// escena del chat y lo seleccionamos en las fuentes de OBS.
		connect(m_chatList, &QListWidget::itemClicked, this,
			[this](QListWidgetItem *) { focusSelectedChatInObs(); });
		v->addWidget(m_chatList, 1);

		// Acción sobre la seleccionada: añadirla a la escena actual. (Quitar va
		// como botón de papelera en cada fila de la lista.)
		auto *selRow = new QHBoxLayout();
		m_addSelBtn = new QPushButton(QStringLiteral("Añadir a esta escena"), page);
		m_addSelBtn->setObjectName(QStringLiteral("accent"));
		m_addSelBtn->setEnabled(false);
		connect(m_addSelBtn, &QPushButton::clicked, this, &BetterChatDock::onAddSelectedToScene);
		selRow->addWidget(m_addSelBtn, 1);
		v->addLayout(selRow);

		// Panel de ajustes de la instancia seleccionada (dirección / alineación).
		// Solo visible cuando hay un chat seleccionado; edita su URL (query params).
		m_settingsPanel = new QWidget(page);
		auto *sv = new QVBoxLayout(m_settingsPanel);
		sv->setContentsMargins(0, 4, 0, 0);
		sv->setSpacing(6);

		auto *dirRow = new QHBoxLayout();
		auto *dirLabel = new QLabel(QStringLiteral("Dirección"), m_settingsPanel);
		dirLabel->setObjectName(QStringLiteral("muted"));
		dirRow->addWidget(dirLabel, 1);
		m_dirCombo = new QComboBox(m_settingsPanel);
		m_dirCombo->addItem(QStringLiteral("Nuevos abajo"), QStringLiteral("down"));
		m_dirCombo->addItem(QStringLiteral("Nuevos arriba"), QStringLiteral("up"));
		connect(m_dirCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
			&BetterChatDock::onChatSettingChanged);
		dirRow->addWidget(m_dirCombo, 1);
		sv->addLayout(dirRow);

		auto *alignRow = new QHBoxLayout();
		auto *alignLabel = new QLabel(QStringLiteral("Alineación"), m_settingsPanel);
		alignLabel->setObjectName(QStringLiteral("muted"));
		alignRow->addWidget(alignLabel, 1);
		m_alignCombo = new QComboBox(m_settingsPanel);
		m_alignCombo->addItem(QStringLiteral("Izquierda"), QStringLiteral("left"));
		m_alignCombo->addItem(QStringLiteral("Centro"), QStringLiteral("center"));
		m_alignCombo->addItem(QStringLiteral("Derecha"), QStringLiteral("right"));
		connect(m_alignCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
			&BetterChatDock::onChatSettingChanged);
		alignRow->addWidget(m_alignCombo, 1);
		sv->addLayout(alignRow);

		auto *scaleRow = new QHBoxLayout();
		auto *scaleLabel = new QLabel(QStringLiteral("Escala"), m_settingsPanel);
		scaleLabel->setObjectName(QStringLiteral("muted"));
		scaleRow->addWidget(scaleLabel, 0);
		m_scaleSlider = new QSlider(Qt::Horizontal, m_settingsPanel);
		m_scaleSlider->setRange(30, 200); // coincide con el clamp del overlay
		m_scaleSlider->setValue(100);
		scaleRow->addWidget(m_scaleSlider, 1);
		m_scaleSpin = new QSpinBox(m_settingsPanel);
		m_scaleSpin->setRange(30, 200);
		m_scaleSpin->setValue(100);
		m_scaleSpin->setSuffix(QStringLiteral(" %"));
		scaleRow->addWidget(m_scaleSpin, 0);
		// Slider y campo van sincronizados. Mientras arrastras el slider solo se
		// actualiza el número (sin spamear peticiones); se aplica al SOLTAR. El campo
		// numérico aplica al confirmar su valor.
		connect(m_scaleSlider, &QSlider::valueChanged, this, [this](int v) {
			if (m_updatingPanel)
				return;
			m_updatingPanel = true;
			m_scaleSpin->setValue(v);
			m_updatingPanel = false;
		});
		connect(m_scaleSlider, &QSlider::sliderReleased, this,
			[this]() { onChatSettingChanged(); });
		connect(m_scaleSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int v) {
			if (m_updatingPanel)
				return;
			m_updatingPanel = true;
			m_scaleSlider->setValue(v);
			m_updatingPanel = false;
			onChatSettingChanged();
		});
		sv->addLayout(scaleRow);

		m_settingsPanel->setVisible(false);
		v->addWidget(m_settingsPanel);

		// Crear una instancia NUEVA e independiente en la escena actual.
		m_createBtn = new QPushButton(QStringLiteral("Crear chat nuevo en esta escena"), page);
		m_createBtn->setObjectName(QStringLiteral("primary"));
		connect(m_createBtn, &QPushButton::clicked, this, &BetterChatDock::onCreateChat);
		v->addWidget(m_createBtn);

		m_actionStatus = new QLabel(page);
		m_actionStatus->setObjectName(QStringLiteral("muted"));
		m_actionStatus->setWordWrap(true);
		v->addWidget(m_actionStatus);

		// Herramientas de prueba: separador + mensajes automáticos + limpiar.
		auto *sep = new QFrame(page);
		sep->setFrameShape(QFrame::HLine);
		sep->setObjectName(QStringLiteral("sep"));
		v->addWidget(sep);

		auto *toolsRow = new QHBoxLayout();
		m_autoTestCheck = new ToggleSwitch(page);
		m_autoTestCheck->setText(QStringLiteral("Mensajes de prueba automáticos"));
		connect(m_autoTestCheck, &QCheckBox::toggled, this, &BetterChatDock::onAutoTestToggled);
		toolsRow->addWidget(m_autoTestCheck, 1);
		m_clearBtn = new QPushButton(QStringLiteral("Limpiar chat"), page);
		m_clearBtn->setObjectName(QStringLiteral("danger"));
		connect(m_clearBtn, &QPushButton::clicked, this, &BetterChatDock::onClearChat);
		toolsRow->addWidget(m_clearBtn, 0);
		v->addLayout(toolsRow);

		m_logoutBtn = new QPushButton(QStringLiteral("Cerrar sesión"), page);
		m_logoutBtn->setObjectName(QStringLiteral("danger"));
		connect(m_logoutBtn, &QPushButton::clicked, this, &BetterChatDock::onLogout);
		v->addWidget(m_logoutBtn);

		// La pestaña "Ajustes de chat" va en un scroll area: al desplegar el panel
		// de ajustes el contenido crece y, sin scroll, el borde inferior quedaba
		// recortado.
		auto *scroll = new QScrollArea(m_tabStack);
		scroll->setWidget(tabChat);
		scroll->setWidgetResizable(true);
		scroll->setFrameShape(QFrame::NoFrame);
		scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scroll->setObjectName(QStringLiteral("scrollPage"));
		m_tabStack->addWidget(scroll); // índice 0

		// ===== Pestaña 1: Multichat (chat en vivo combinado) =====
		{
			auto *tab = new QWidget(m_tabStack);
			auto *tv = new QVBoxLayout(tab);
			tv->setContentsMargins(0, 0, 0, 0);
			tv->setSpacing(8);

			// Fila de chips: estado de conexión de cada plataforma.
			buildPlatformBar(tv);

			m_multiStatus = new QLabel(
				QStringLiteral("Esperando mensajes del chat en vivo..."), tab);
			m_multiStatus->setObjectName(QStringLiteral("muted"));
			m_multiStatus->setWordWrap(true);
			tv->addWidget(m_multiStatus);

			m_multiChat = new QTextEdit(tab);
			m_multiChat->setObjectName(QStringLiteral("multiChat"));
			m_multiChat->setReadOnly(true);
			m_multiChat->setFocusPolicy(Qt::NoFocus);
			m_multiChat->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
			// Descarte NATIVO de bloques antiguos: al superar el tope, Qt borra el
			// bloque más viejo automáticamente (mucho más barato que hacerlo con
			// cursor). 1000 mensajes de historial sin coste de acumulación.
			m_multiChat->document()->setMaximumBlockCount(1000);
			tv->addWidget(m_multiChat, 1);

			m_tabStack->addWidget(tab); // índice 1
		}

		// ===== Pestaña 2: Minijuegos/Apuestas (placeholder por ahora) =====
		{
			auto *tab = new QWidget(m_tabStack);
			auto *tv = new QVBoxLayout(tab);
			tv->setContentsMargins(0, 0, 0, 0);
			auto *lbl = new QLabel(QStringLiteral("Minijuegos y apuestas: próximamente."), tab);
			lbl->setObjectName(QStringLiteral("muted"));
			lbl->setWordWrap(true);
			tv->addWidget(lbl);
			tv->addStretch(1);
			m_tabStack->addWidget(tab); // índice 2
		}

		m_stack->addWidget(page);
	}
}

// Cambia la pestaña activa del dock (nav bar) y marca el botón correspondiente.
void BetterChatDock::selectTab(int index)
{
	if (!m_tabStack || index < 0 || index >= m_tabStack->count())
		return;
	m_tabStack->setCurrentIndex(index);
	for (int i = 0; i < m_tabButtons.size(); i++)
		m_tabButtons[i]->setChecked(i == index);
	// El stream del multichat solo corre mientras la pestaña está visible (ahorra
	// conexión y carga del servidor cuando no se está mirando).
	if (index == 1) {
		if (m_multiStatus)
			m_multiStatus->setText(QStringLiteral("Conectando al chat en vivo..."));
		m_api->startChatStream();
	} else {
		m_api->stopChatStream();
	}
}

// Construye la fila de chips de estado por plataforma (encima del multichat). Cada
// chip lleva el logo de la plataforma + un texto de estado; empieza en "off".
void BetterChatDock::buildPlatformBar(QVBoxLayout *parent)
{
	m_platBar = new QWidget();
	auto *h = new QHBoxLayout(m_platBar);
	h->setContentsMargins(0, 0, 0, 0);
	h->setSpacing(6);
	const char *order[] = {"twitch", "youtube", "kick", "tiktok"};
	for (const char *key : order) {
		auto *chip = new QLabel(m_platBar);
		chip->setObjectName(QStringLiteral("platChip"));
		chip->setTextFormat(Qt::RichText);
		m_platChips.insert(QString::fromLatin1(key), chip);
		h->addWidget(chip);
	}
	h->addStretch(1);
	parent->addWidget(m_platBar);
	// Estado inicial (se corrige en cuanto lleguen los status por SSE).
	for (auto it = m_platChips.constBegin(); it != m_platChips.constEnd(); ++it)
		onPlatformStatus(it.key(), QStringLiteral("off"), QString());
}

// Refresca el chip de una plataforma según su estado de conexión en el stream.
void BetterChatDock::onPlatformStatus(const QString &platform, const QString &state,
				      const QString &detail)
{
	Q_UNUSED(detail);
	QLabel *chip = m_platChips.value(platform.toLower(), nullptr);
	if (!chip)
		return;
	registerPlatformIcons();

	// Color del punto + texto según el estado.
	QString dot, label;
	if (state == QStringLiteral("connected")) {
		dot = QStringLiteral("#38d39f"); label = QStringLiteral("chat en vivo");
	} else if (state == QStringLiteral("connecting")) {
		dot = QStringLiteral("#ffb020"); label = QStringLiteral("conectando");
	} else if (state == QStringLiteral("error")) {
		dot = QStringLiteral("#ff5b6a"); label = QStringLiteral("error");
	} else if (state == QStringLiteral("disconnected")) {
		dot = QStringLiteral("#ff5b6a"); label = QStringLiteral("sin directo");
	} else { // off (no configurada)
		dot = QStringLiteral("#6b5a53"); label = QStringLiteral("no conectada");
	}

	const QString plat = platform.toLower();
	static const QHash<QString, bool> known = {
		{QStringLiteral("twitch"), true}, {QStringLiteral("youtube"), true},
		{QStringLiteral("kick"), true}, {QStringLiteral("tiktok"), true},
	};
	QString icon = known.value(plat, false)
		? QStringLiteral("<img src='bcplat://%1' width='13' height='13'>").arg(plat)
		: plat.toHtmlEscaped();

	// Chip: [logo] •estado, con punto de color según el estado.
	chip->setText(QStringLiteral(
			      "%1 <span style='color:%2;'>&#9679;</span> "
			      "<span style='color:#b9a49c; font-size:11px;'>%3</span>")
			      .arg(icon, dot, label));
	chip->setStyleSheet(state == QStringLiteral("off")
				    ? QStringLiteral("QLabel#platChip { background:#211815; border:1px solid #2b1f1b; "
						     "border-radius:6px; padding:3px 7px; }")
				    : QStringLiteral("QLabel#platChip { background:#2b1f1b; border:1px solid #3a2a25; "
						     "border-radius:6px; padding:3px 7px; }"));
}

// Iconos oficiales de cada plataforma, pre-rasterizados a PNG (con el renderer SVG
// de Qt en tiempo de build) y embebidos en base64. En runtime solo se carga el PNG
// (QImage), sin depender del modulo SVG de Qt que puede faltar en OBS Windows.
void BetterChatDock::registerPlatformIcons()
{
	if (m_iconsReady || !m_multiChat)
		return;
	m_iconsReady = true;
	struct P { const char *key; const char *png; };
	static const P plats[] = {
		{"twitch", "iVBORw0KGgoAAAANSUhEUgAAACQAAAAkCAYAAADhAJiYAAAACXBIWXMAAA7EAAAOxAGVKw4bAAACjklEQVRYhe2Yz2sTQRiGn5mNKRpaxUIVMYq/wEtIRQ+KWimWlggWRATvLVKQViroQfTuQSy2tPYQ/QdED/EQG6pFD+JBxKSC4kVq6kXRiqUtSbs7HjZ100WbzWaT7cH3NN/stzMP8+43uzsCm4baVESXdAtoB8JAyJ5ToeaArIKUZhDvHReTxRfFcmMwpuoMnQEBPcX9VZZSMCo1+vuSIvcHaDCm6pROEmitEYhdE0Ij1pcUOQlg6Az4CAPQqnRuA4ihNhUxJGlqZ9O/pKRBVOqS7jUAAyAMjS5ZqKa1IUVHALO0HalhC5y9BfVN7uZ7PgzpxKop4QAO95limE+vYParc5BgCPafdJQaCpQLA5B5DFOvnQNt2uYYCFkqob6pMpvKVUmg3YdLw4QPwKkbVl5wgxnva6kCkBM1bIW9x0wQAG2dGW/e6ROQl/oPVEqOyr6U3qfgw1MwFs144RcMnwal+wRk6EDx5Ar0vBXuOuJ8rKpbFu2E4xfgx2f4+MJnoGgnnLhowjy8Ags/S9/jyrKD56xXwew3eHnfbB86D42FvSdQB3uOmjCPrjqDcQ20vdlqf58ygVp6oPnMyrxlmPkZ52N7YplXMOBBlTXusGyqFAa8WCHbx28lMADiTrtSqyVoQQiuX9kXu2Y9R18y8OQmGEtmnJ8HfdEdDDiwTM/DQt7WV5h8OgOJ67CUcw9gl2vLptPew4C5QnOU+f+efQuZhPcwwJwEsuXe9eZBVWAAslJBqipDu5FgTGoGcWDVSquRFIq47B0XkwpG/aYB7l5KiXcSQGr0AxN+kSh4JjQuQ6Hs+5IiJzRiwAi1tU8BIzMbzbMh+Mupx1CbihgaXSg6qOKRHoIxqXPPfqT3G5gozWXc4RwqAAAAAElFTkSuQmCC"},
		{"kick", "iVBORw0KGgoAAAANSUhEUgAAACQAAAAkCAYAAADhAJiYAAAACXBIWXMAAA7EAAAOxAGVKw4bAAACNElEQVRYhd2YTWsTQRiAn5nNh/lQJCFW0IKHCF5K8SIlt1xa0mvpD5B68FRIfkgLHmoPkf4ADx5De3DxIuItKCg0h4BeqqZIbBK2MTMesmnTkNTozmbF57TMzM4+O+/s7MsrGGHdmVtQUj4CvYxgHkiMjvFIC80nEAdSqfLz6NG74U4xuCjobDTeO9kGHg+3+4wGdttWslgRNedMyJWpAPkZiYxit61koSJqjgRwVyYoGYB8rHeyBSDWnbkFZYkqswvTJLTs6UXZ38CBywAIJeWGBL0ctMk5eiXkftoTsXMNjt92x/ZlN+Msbl270FYtNak9aY8dn3oQJv86PflhgnmJ+XPGC4mQqZmqpSYA316NX81pMSY0KUx/iiehnz80nc89IyIDPAnV9zrU9zqmXACQRmczgLE9dBl3iwnSS2Eiqd+//0yE0kthbq1dmWqsb0IyDNfvhwGmWhnfhaI3rMtP5Qn8c5vaNyHnSw8718DONfhqn059n28hU13Ofsqnxyp4oWEab/pikZQkk48EL3S43eKQKdIP/rdNfedhjNV6htV6xpSPt5CFrgpity1TLv05TU2U3YwD/QTte/XvkzRjQoPculpqehKSQMuQkwlaYq178wOCe0GbAKD5KEEcBO1xjtiXUqky/SpE0GilVFm69ZndwG3g6Yvo0XsJ0LaSRcAOSkbAy46VLIF7UldEzWlbyYKGHWYbPq1hR1ipwoWC1TBuSW8D9IrPJb19qdSz0ZLeL2Qur3F9TkJXAAAAAElFTkSuQmCC"},
		{"youtube", "iVBORw0KGgoAAAANSUhEUgAAACQAAAAkCAYAAADhAJiYAAAACXBIWXMAAA7EAAAOxAGVKw4bAAAB8UlEQVRYhe2YMU/CUBDHf62AgyyaOCpuulRdnfgA+i100/hZcMAv4qSg0Wji5OAko8RR0cF2geo5tITawL1WCmXwn1zSpK93v/fu0l4PYhJwBE4EngRcAcnY3ND3iYATjx8FmRc4FfieAMQo+w5jzg+DuZoiSNyufkGFlHnB9K0OYIV5fASskfmcjgTYsoGDGYCBgGHfEngCNvKmCdWyBFxgIW+SUJ4lQe5mRnbeAHEVEq2qVmFtDZaXB7a4CKUSFItQCN34PvR60O3Cxwe8vg7s+Rlubsyx1HdDsSjSbEpmajYDn0pMHejoKDuYvg4PVSC9hnZ2EmU0lQw+daBKRXfuuvD1lQ7I4FMHWlnRnT88wPY2NBrJgVZX9ftqDXmeXg/X14O1u7sirZa5hlx3jKL2/eRAIFIoiBwfi3Q6o5/x/TGKOq0sa2B/VWYp29vLJGX6Cb29mXfkOEFRn53B+rp5faczxgnd3em7/fw011lct7djnFC7re+mXIa5OfOppPCpA93fpwuWRCafU/24NhrGj2uyBq1aDV750fZjaSloPYa1H70evL//bj/a7UTtx3/HaJINeHlDROTZwEveFBG92MBF3hQRnc/ar/Rm/2pmhg19oLzHMZcyYkZUl+kPrOoCJS2RjkBNJj/Sq8mQkd4Prg1E0y9/DacAAAAASUVORK5CYII="},
		{"tiktok", "iVBORw0KGgoAAAANSUhEUgAAACQAAAAkCAYAAADhAJiYAAAACXBIWXMAAA7EAAAOxAGVKw4bAAADJUlEQVRYhc2YT0gjdxTHP29UkrgXfwGPC1YR62FbRxRETwXZUEVBxGNR2AXrwUNvEg3m5LWQIrtGva9XD5Vd/HMQxZNYWmwhBJGCenEGoQuTgP56iLrOaDYx//R7mt+b9958+L3fG2ae4JFS6pWIvNVavwZeAi+8PkXqM/CviHzSWi/Ztv3n3Zty59qnlPoV+NljL6c08N627V+A1F0gn1JqDfihQiBebdm2/SOQqgJQSv0GjDwRDMA3gUAg6DjO76KUegX8QeXKlE0a+N4QkbfPAAZAROSNcd1Nz0Ja65BBprWLVjgc5vz8HMuysCyL1tbWQtK8NCjRe8YwDES+VL6jo6OQNC+MUsAALhiAsbExqqurH52nZEBemabJ5OTko+PKBgQQiURYXFykvr4+75iylexGw8PDHB4esrGxwdDQUOWAvqaqqipM06SzszOnb9mAtra2uLy8fHTco4EaGxsJh8OsrKywurpKf38/cL9kc3NzdHd3E4vFODo6yjt/3n3p9/uZnZ1lfHzcZV9bW8sak0gkiEajRKNRfD4ftbW1pQESEebn5x88lFrrB2O89lQqRSqVyvmsvEo2MTGRtUOurq4yiYzSHMecO1RXV8fMzIzLlkwmWVpa4vj4mGQyCWQ66a6y7VzRQKFQCL/f74Lp7e3l4uLC5ecFKlQ597mlpcW1jsfj92AqCuQ9G6enpw/6eYEcxykPUCKRcK3b29vv+YgIpmnertPp9O3ZKjnQ9vb2bSdBpuN6enpcMJFIhLa2tlvb7u4u6XS6ICBRSuVsh4WFBUZGvvyUaK3Z3Nzk7OyMrq4umpqaXP4DAwPs7OyUDygYDLK+vk5DQ0POhPF4nKmpqYJgIM8Xo2VZDA4Osr+/n9VHa00sFmN6erpgGMjs0H/k+V1dU1NDX18fo6OjNDc3EwwGOTk5YW9vj+XlZQ4ODoqCAT6LUupv4NtiM5VI/xgi8umpKW4kIh+f26/0d8b1fOb9E8MAvLNt+69nMY4RkU3Lsvq4GccAl47jfAgEAkGgg8oOrN7Ztv0TkOahB1+P9N5orUOUd6T3UWu97B3p/Q+1Uwt/WjrRTwAAAABJRU5ErkJggg=="},
	};
	for (const P &pl : plats) {
		QImage img;
		img.loadFromData(QByteArray::fromBase64(QByteArray(pl.png)), "PNG");
		m_multiChat->document()->addResource(QTextDocument::ImageResource,
						     QUrl(QStringLiteral("bcplat://") +
							  QString::fromLatin1(pl.key)),
						     QVariant(img));
	}
}


// Añade un mensaje del multichat en vivo: logo de plataforma + "autor:" + texto,
// cada uno en su propia línea (bloque).
void BetterChatDock::onChatMessage(const QString &platform, const QString &platformLabel,
				   const QString &author, const QString &text, const QString &color)
{
	if (!m_multiChat)
		return;
	registerPlatformIcons();
	if (m_multiStatus)
		m_multiStatus->setText(QStringLiteral("Chat en vivo de todas tus plataformas conectadas."));

	const QString plat = platform.toLower();
	static const QHash<QString, bool> known = {
		{QStringLiteral("twitch"), true}, {QStringLiteral("youtube"), true},
		{QStringLiteral("kick"), true}, {QStringLiteral("tiktok"), true},
	};
	const QString nameColor = color.isEmpty() ? QStringLiteral("#ff9ec4") : color;

	// Logo de la plataforma (si es conocida) o etiqueta de texto de reserva.
	QString icon;
	if (known.value(plat, false)) {
		icon = QStringLiteral("<img src='bcplat://%1' width='16' height='16'>").arg(plat);
	} else {
		const QString label = (platformLabel.isEmpty() ? platform : platformLabel).toHtmlEscaped();
		icon = QStringLiteral("<span style='background:#3a2a25; color:#f6eeea; font-size:10px; "
				      "padding:1px 5px; border-radius:4px;'>%1</span>")
			       .arg(label);
	}

	const QString html =
		QStringLiteral("%1 <b style='color:%2'>%3:</b> "
			       "<span style='color:#f6eeea'>%4</span>")
			.arg(icon, nameColor, author.toHtmlEscaped(), text.toHtmlEscaped());

	// Auto-scroll solo si ya estaba al fondo (no interrumpir si el usuario sube).
	QScrollBar *sb = m_multiChat->verticalScrollBar();
	bool atBottom = sb->value() >= sb->maximum() - 4;

	// Cada mensaje en su PROPIO bloque (línea) -> layout vertical. El descarte de
	// los mensajes antiguos lo hace el document (setMaximumBlockCount), sin coste.
	QTextCursor cur(m_multiChat->document());
	cur.movePosition(QTextCursor::End);
	if (!m_multiChat->document()->isEmpty())
		cur.insertBlock();
	cur.insertHtml(html);

	if (atBottom)
		sb->setValue(sb->maximum());
}

// Pinta un evento destacado (donación/sub/regalo...) como una línea resaltada con
// fondo de color, icono de plataforma y un texto legible según el tipo.
void BetterChatDock::onChatEvent(const QString &platform, const QString &platformLabel,
				 const QString &kind, const QString &actor, const QString &text,
				 int amount, const QString &unit)
{
	if (!m_multiChat)
		return;
	registerPlatformIcons();

	const QString plat = platform.toLower();
	static const QHash<QString, bool> known = {
		{QStringLiteral("twitch"), true}, {QStringLiteral("youtube"), true},
		{QStringLiteral("kick"), true}, {QStringLiteral("tiktok"), true},
	};
	QString icon;
	if (known.value(plat, false))
		icon = QStringLiteral("<img src='bcplat://%1' width='15' height='15'>").arg(plat);
	else
		icon = platformLabel.toHtmlEscaped();

	// Color e ícono-emoji semántico segun el tipo de evento.
	QString accent = QStringLiteral("#ff4d8d");
	QString glyph, desc;
	const QString A = QStringLiteral("<b>%1</b>").arg(actor.toHtmlEscaped());
	if (kind == QStringLiteral("cheer")) {
		accent = QStringLiteral("#9146ff"); glyph = QStringLiteral("BITS");
		desc = QStringLiteral("%1 donó %2 bits").arg(A).arg(amount);
	} else if (kind == QStringLiteral("superchat")) {
		accent = QStringLiteral("#00b0ff"); glyph = QStringLiteral("SUPER CHAT");
		desc = QStringLiteral("%1 donó %2").arg(A, unit.toHtmlEscaped());
		if (!text.isEmpty())
			desc += QStringLiteral(": %1").arg(text.toHtmlEscaped());
	} else if (kind == QStringLiteral("sub")) {
		accent = QStringLiteral("#38d39f"); glyph = QStringLiteral("SUB");
		desc = QStringLiteral("%1 se suscribió%2").arg(A,
			unit.isEmpty() ? QString() : QStringLiteral(" (%1)").arg(unit.toHtmlEscaped()));
	} else if (kind == QStringLiteral("resub")) {
		accent = QStringLiteral("#38d39f"); glyph = QStringLiteral("RESUB");
		desc = QStringLiteral("%1 renovó su sub").arg(A);
		if (amount > 0)
			desc += QStringLiteral(" (%1 meses)").arg(amount);
		if (!text.isEmpty())
			desc += QStringLiteral(": %1").arg(text.toHtmlEscaped());
	} else if (kind == QStringLiteral("subgift")) {
		accent = QStringLiteral("#ff9500"); glyph = QStringLiteral("REGALO");
		if (amount > 0)
			desc = QStringLiteral("%1 regaló %2 subs").arg(A).arg(amount);
		else if (!text.isEmpty())
			desc = QStringLiteral("%1 regaló una sub a <b>%2</b>").arg(A, text.toHtmlEscaped());
		else
			desc = QStringLiteral("%1 regaló una sub").arg(A);
	} else if (kind == QStringLiteral("gift")) {
		accent = QStringLiteral("#fe2c55"); glyph = QStringLiteral("REGALO");
		desc = QStringLiteral("%1 envió %2").arg(A, text.toHtmlEscaped());
		if (amount > 0)
			desc += QStringLiteral(" (%1 %2)").arg(amount).arg(unit.toHtmlEscaped());
	} else if (kind == QStringLiteral("member")) {
		accent = QStringLiteral("#0f9d58"); glyph = QStringLiteral("MIEMBRO");
		desc = QStringLiteral("%1 %2").arg(A, text.toHtmlEscaped());
	} else if (kind == QStringLiteral("raid")) {
		accent = QStringLiteral("#9146ff"); glyph = QStringLiteral("RAID");
		desc = QStringLiteral("%1 hizo raid con %2 viewers").arg(A).arg(amount);
	} else if (kind == QStringLiteral("follow")) {
		accent = QStringLiteral("#25f4ee"); glyph = QStringLiteral("SEGUIDOR");
		desc = QStringLiteral("%1 empezó a seguir").arg(A);
	} else {
		glyph = kind.toUpper(); desc = A;
	}

	// Línea destacada: fondo de color tenue con borde de acento a la izquierda.
	const QString html =
		QStringLiteral(
			"<table width='100%' cellspacing='0' cellpadding='4' "
			"style='background:%1; margin:2px 0;'><tr>"
			"<td style='border-left:3px solid %2;'>"
			"%3 <span style='color:%2; font-weight:700; font-size:10px;'>%4</span> "
			"<span style='color:#f6eeea;'>%5</span>"
			"</td></tr></table>")
			.arg(accent + QStringLiteral("22"), accent, icon, glyph, desc);

	QScrollBar *sb = m_multiChat->verticalScrollBar();
	bool atBottom = sb->value() >= sb->maximum() - 4;
	QTextCursor cur(m_multiChat->document());
	cur.movePosition(QTextCursor::End);
	if (!m_multiChat->document()->isEmpty())
		cur.insertBlock();
	cur.insertHtml(html);
	if (atBottom)
		sb->setValue(sb->maximum());
}

void BetterChatDock::onStartLogin()
{
	m_loginBtn->setEnabled(false);
	m_loginStatus->setText(QStringLiteral("Abriendo el navegador para confirmar…"));
	m_api->startPairing();
}

void BetterChatDock::onPairingStarted(const QString &userCode, const QString &verifyUrl)
{
	m_pairInfo->setText(userCode);
	m_pairInfo->setVisible(true);
	m_loginStatus->setText(
		QStringLiteral("Confirma este código en el navegador. Si no se abrió solo, entra en:\n%1")
			.arg(verifyUrl));
}

void BetterChatDock::onPairingFailed(const QString &message)
{
	m_loginBtn->setEnabled(true);
	m_pairInfo->setVisible(false);
	m_loginStatus->setText(message);
}

void BetterChatDock::onLoggedIn()
{
	m_pairInfo->setVisible(false);
	m_loginBtn->setEnabled(true);
	m_loginStatus->clear();
	m_stack->setCurrentIndex(1);
	if (m_navBar)
		m_navBar->setVisible(true);
	m_api->startStatusPolling();
	refreshChatList();
}

void BetterChatDock::onLoggedOut()
{
	if (m_autoTestCheck) {
		m_updatingPanel = true;
		m_autoTestCheck->setChecked(false);
		m_updatingPanel = false;
	}
	m_stack->setCurrentIndex(0);
	if (m_navBar)
		m_navBar->setVisible(false);
	if (m_tabStack)
		selectTab(0); // volver a la primera pestaña y parar el stream
	m_loginBtn->setEnabled(true);
	m_loginStatus->clear();
	m_pairInfo->setVisible(false);
}

void BetterChatDock::onStatusUpdated()
{
	QString name = m_api->username();
	m_userLabel->setText(name.isEmpty() ? QStringLiteral("Tu cuenta") : name);
	if (m_api->isLive()) {
		QString plat = m_api->platform();
		m_liveBadge->setObjectName(QStringLiteral("liveOn"));
		m_liveBadge->setText(plat.isEmpty() ? QStringLiteral("En directo ahora")
						    : QStringLiteral("En directo · %1").arg(plat));
	} else {
		m_liveBadge->setObjectName(QStringLiteral("liveOff"));
		m_liveBadge->setText(QStringLiteral("No estás en directo"));
	}
	// Re-aplicar el estilo tras cambiar objectName.
	m_liveBadge->setStyleSheet(QString::fromUtf8(kStyle));

	// Reflejar el estado sincronizado del auto-test (lo pudo cambiar la web u otro
	// cliente). m_updatingPanel evita que este ajuste dispare onAutoTestToggled.
	if (m_autoTestCheck && m_autoTestCheck->isChecked() != m_api->autoTestActive()) {
		m_updatingPanel = true;
		m_autoTestCheck->setChecked(m_api->autoTestActive());
		m_updatingPanel = false;
	}

	// Aprovechar el sondeo periodico para refrescar tamaños de la lista (el
	// auto-resize los cambia al arrastrar en OBS).
	refreshChatList();
}

// ---- Lista de instancias de chat ----

namespace {
// Marca propia en los settings para reconocer NUESTRAS fuentes de chat.
constexpr const char *kChatFlag = "betterchat_instance";

// Callback de enumeración: recoge nombre + tamaño + url de cada fuente de chat.
struct ChatInfo {
	QString name;
	int width;
	int height;
	QString url;
};

bool collectChatSources(void *param, obs_source_t *source)
{
	auto *out = static_cast<QList<ChatInfo> *>(param);
	const char *id = obs_source_get_id(source);
	if (!id || QString(id) != QStringLiteral("browser_source"))
		return true;
	obs_data_t *settings = obs_source_get_settings(source);

	// Es un chat de BetterChatTV si:
	//  a) lo creó el plugin (lleva el flag), o
	//  b) es un browser source cuya URL contiene "betterchat" (creado a mano).
	bool mine = obs_data_get_bool(settings, kChatFlag);
	if (!mine) {
		const char *urlC = obs_data_get_string(settings, "url");
		bool isLocal = obs_data_get_bool(settings, "is_local_file");
		QString url = urlC ? QString::fromUtf8(urlC) : QString();
		if (!isLocal && url.contains(QStringLiteral("betterchat"), Qt::CaseInsensitive)) {
			mine = true;
			// Adoptar: marcarla para futuras detecciones y persistir.
			obs_data_set_bool(settings, kChatFlag, true);
			obs_source_update(source, settings);
		}
	}

	if (mine) {
		ChatInfo info;
		info.name = QString::fromUtf8(obs_source_get_name(source));
		info.width = (int)obs_data_get_int(settings, "width");
		info.height = (int)obs_data_get_int(settings, "height");
		const char *u = obs_data_get_string(settings, "url");
		info.url = u ? QString::fromUtf8(u) : QString();
		out->append(info);
	}
	obs_data_release(settings);
	return true;
}
} // namespace

void BetterChatDock::refreshChatList()
{
	if (!m_chatList)
		return;
	QString prevSelected;
	if (m_chatList->currentItem())
		prevSelected = m_chatList->currentItem()->data(Qt::UserRole + 2).toString();

	m_chatList->clear();
	QList<ChatInfo> chats;
	obs_enum_sources(collectChatSources, &chats);
	for (const auto &c : chats) {
		QString label = QStringLiteral("%1  (%2×%3)").arg(c.name).arg(c.width).arg(c.height);
		auto *item = new QListWidgetItem(m_chatList);
		item->setData(Qt::UserRole, c.name);      // nombre real de la fuente
		item->setData(Qt::UserRole + 1, c.url);   // url actual (con sus query params)
		item->setData(Qt::UserRole + 2, label);   // etiqueta (para restaurar selección)

		// Widget de fila: etiqueta a la izquierda + botón papelera a la derecha.
		auto *rowW = new QWidget(m_chatList);
		auto *rowL = new QHBoxLayout(rowW);
		rowL->setContentsMargins(6, 2, 8, 2);
		rowL->setSpacing(6);
		auto *lbl = new QLabel(label, rowW);
		lbl->setMinimumWidth(0);
		lbl->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
		lbl->setToolTip(label); // el nombre completo, por si se recorta
		rowL->addWidget(lbl, 1);
		auto *delBtn = new QPushButton(rowW);
		delBtn->setObjectName(QStringLiteral("trash"));
		delBtn->setToolTip(QStringLiteral("Eliminar este chat"));
		delBtn->setCursor(Qt::PointingHandCursor);
		delBtn->setFixedSize(26, 26);
		// Icono papelera dibujado con QPainter (sin depender del plugin SVG de Qt,
		// que puede no venir en el OBS de Windows). Trazo estilo Lucide.
		{
			QPixmap pm(32, 32);
			pm.fill(Qt::transparent);
			QPainter p(&pm);
			p.setRenderHint(QPainter::Antialiasing, true);
			QPen pen(QColor("#b9a49c"));
			pen.setWidthF(2.2);
			pen.setCapStyle(Qt::RoundCap);
			pen.setJoinStyle(Qt::RoundJoin);
			p.setPen(pen);
			// Tapa + asa.
			p.drawLine(6, 9, 26, 9);
			p.drawLine(13, 9, 13, 6);
			p.drawLine(19, 9, 19, 6);
			p.drawLine(13, 6, 19, 6);
			// Cubo.
			p.drawLine(8, 9, 9, 26);
			p.drawLine(24, 9, 23, 26);
			p.drawLine(9, 26, 23, 26);
			// Rayas verticales.
			p.drawLine(13, 13, 13, 22);
			p.drawLine(19, 13, 19, 22);
			p.end();
			delBtn->setIcon(QIcon(pm));
			delBtn->setIconSize(QSize(16, 16));
		}
		QString chatName = c.name;
		connect(delBtn, &QPushButton::clicked, this, [this, chatName]() {
			obs_source_t *source = obs_get_source_by_name(chatName.toUtf8().constData());
			if (source) {
				obs_source_remove(source);
				obs_source_release(source);
				m_actionStatus->setText(QStringLiteral("\"%1\" eliminado.").arg(chatName));
			}
			refreshChatList();
		});
		rowL->addWidget(delBtn, 0);
		item->setSizeHint(rowW->sizeHint());
		m_chatList->setItemWidget(item, rowW);

		if (label == prevSelected)
			m_chatList->setCurrentItem(item);
	}
	if (chats.isEmpty())
		m_actionStatus->setText(QStringLiteral(
			"Aún no tienes ningún chat. Crea uno con el botón \"Crear chat nuevo en esta escena\"."));
	else
		m_actionStatus->clear();
	updateSettingsPanel();
}

// ---- Panel de ajustes por instancia (dirección / alineación) ----

void BetterChatDock::updateSettingsPanel()
{
	if (!m_settingsPanel)
		return;
	auto *item = m_chatList ? m_chatList->currentItem() : nullptr;
	if (!item) {
		m_settingsPanel->setVisible(false);
		return;
	}
	QString url = item->data(Qt::UserRole + 1).toString();
	QUrlQuery q(QUrl(url).query());
	QString dir = q.queryItemValue(QStringLiteral("dir"));
	QString align = q.queryItemValue(QStringLiteral("align"));
	QString scale = q.queryItemValue(QStringLiteral("scale"));

	// Rellenar los combos sin disparar onChatSettingChanged.
	m_updatingPanel = true;
	int dirIdx = (dir == QStringLiteral("up") || dir == QStringLiteral("top")) ? 1 : 0;
	m_dirCombo->setCurrentIndex(dirIdx);
	int alignIdx = align == QStringLiteral("center") ? 1 : align == QStringLiteral("right") ? 2 : 0;
	m_alignCombo->setCurrentIndex(alignIdx);
	// Escala: la URL guarda una fracción (1.0=100%); slider/campo usan enteros de %.
	int scalePct = 100;
	if (!scale.isEmpty()) {
		double f = scale.toDouble();
		if (f > 0)
			scalePct = (int)qRound(f * 100.0);
	}
	scalePct = qBound(30, scalePct, 200);
	m_scaleSlider->setValue(scalePct);
	m_scaleSpin->setValue(scalePct);
	m_updatingPanel = false;

	m_settingsPanel->setVisible(true);
}

void BetterChatDock::onChatSettingChanged()
{
	if (m_updatingPanel)
		return;
	setSelectedChatParam(QStringLiteral("dir"), m_dirCombo->currentData().toString());
	setSelectedChatParam(QStringLiteral("align"), m_alignCombo->currentData().toString());
	// Escala como fracción con hasta 2 decimales (100 -> "1", 150 -> "1.5").
	double f = m_scaleSpin->value() / 100.0;
	setSelectedChatParam(QStringLiteral("scale"),
			     QString::number(f, 'g', 4));
}

// Cambia a la primera escena que contiene la fuente seleccionada y la marca como
// seleccionada en el listado de fuentes de OBS. Comodo para editarla in situ.
void BetterChatDock::focusSelectedChatInObs()
{
	auto *item = m_chatList ? m_chatList->currentItem() : nullptr;
	if (!item)
		return;
	QString name = item->data(Qt::UserRole).toString();
	QByteArray nameUtf8 = name.toUtf8();

	struct obs_frontend_source_list scenes = {};
	obs_frontend_get_scenes(&scenes);
	for (size_t i = 0; i < scenes.sources.num; i++) {
		obs_source_t *sceneSrc = scenes.sources.array[i];
		obs_scene_t *scene = obs_scene_from_source(sceneSrc);
		if (!scene)
			continue;
		obs_sceneitem_t *found = obs_scene_find_source(scene, nameUtf8.constData());
		if (found) {
			// Cambiar a esa escena y seleccionar la fuente dentro de ella.
			obs_frontend_set_current_scene(sceneSrc);
			// Deseleccionar el resto y seleccionar solo la nuestra.
			obs_scene_enum_items(
				scene,
				[](obs_scene_t *, obs_sceneitem_t *it, void *param) -> bool {
					auto *target = static_cast<obs_sceneitem_t *>(param);
					obs_sceneitem_select(it, it == target);
					return true;
				},
				found);
			break;
		}
	}
	obs_frontend_source_list_free(&scenes);
}

void BetterChatDock::setSelectedChatParam(const QString &key, const QString &value)
{
	auto *item = m_chatList ? m_chatList->currentItem() : nullptr;
	if (!item)
		return;
	QString name = item->data(Qt::UserRole).toString();
	obs_source_t *source = obs_get_source_by_name(name.toUtf8().constData());
	if (!source)
		return;

	obs_data_t *settings = obs_source_get_settings(source);
	const char *urlC = obs_data_get_string(settings, "url");
	QUrl url(urlC ? QString::fromUtf8(urlC) : QString());
	QUrlQuery q(url.query());
	q.removeQueryItem(key);
	q.addQueryItem(key, value);
	url.setQuery(q);
	QString newUrl = url.toString();

	obs_data_set_string(settings, "url", newUrl.toUtf8().constData());
	obs_source_update(source, settings);
	obs_data_release(settings);
	obs_source_release(source);

	// Actualizar la url guardada en el item para no perder el estado.
	item->setData(Qt::UserRole + 1, newUrl);
	obs_log(LOG_INFO, "[betterchat] '%s' %s=%s", name.toUtf8().constData(),
		key.toUtf8().constData(), value.toUtf8().constData());
}

// ---- Crear una instancia NUEVA (fuente independiente, su propio tamaño) ----

void BetterChatDock::onCreateChat()
{
	QString url = m_api->overlayUrl();
	if (url.isEmpty()) {
		m_actionStatus->setText(
			QStringLiteral("Aún no tengo tu URL de overlay. Prueba de nuevo en unos segundos."));
		m_api->refreshStatus();
		return;
	}
	createChatInCurrentScene(url);
}

void BetterChatDock::createChatInCurrentScene(const QString &url)
{
	obs_source_t *sceneSource = obs_frontend_get_current_scene();
	if (!sceneSource) {
		m_actionStatus->setText(QStringLiteral("No hay ninguna escena activa en OBS."));
		return;
	}
	obs_scene_t *scene = obs_scene_from_source(sceneSource);
	if (!scene) {
		obs_source_release(sceneSource);
		m_actionStatus->setText(QStringLiteral("La escena activa no admite fuentes."));
		return;
	}

	// Nombre basado en la escena: "BetterChatTV Chat - <Escena>", y si ya hay uno
	// en esa escena, "... - <Escena> 2", "... 3", etc. Mas legible que enumerar.
	QString sceneName = QString::fromUtf8(obs_source_get_name(sceneSource));
	QString baseName = QStringLiteral("BetterChatTV Chat - %1").arg(sceneName);
	QString name = baseName;
	int n = 1;
	while (true) {
		obs_source_t *existing = obs_get_source_by_name(name.toUtf8().constData());
		if (!existing)
			break;
		obs_source_release(existing);
		n++;
		name = QStringLiteral("%1 %2").arg(baseName).arg(n);
	}

	obs_data_t *settings = obs_data_create();
	obs_data_set_string(settings, "url", url.toUtf8().constData());
	obs_data_set_int(settings, "width", 1920);
	obs_data_set_int(settings, "height", 1080);
	obs_data_set_bool(settings, "reroute_audio", false);
	obs_data_set_bool(settings, "restart_when_active", true);
	obs_data_set_bool(settings, kChatFlag, true); // marca de instancia nuestra

	obs_source_t *source = obs_source_create("browser_source", name.toUtf8().constData(), settings, nullptr);
	obs_data_release(settings);

	if (!source) {
		obs_source_release(sceneSource);
		m_actionStatus->setText(QStringLiteral(
			"No se pudo crear la fuente de navegador. ¿Tienes el navegador integrado de OBS?"));
		return;
	}

	obs_scene_add(scene, source);
	obs_source_release(source);
	obs_source_release(sceneSource);

	m_actionStatus->setText(QStringLiteral("Creado \"%1\" en la escena actual.").arg(name));
	obs_log(LOG_INFO, "[betterchat] nueva instancia de chat creada: %s", name.toUtf8().constData());
	refreshChatList();
}

// ---- Añadir la instancia SELECCIONADA a la escena actual (referencia) ----

void BetterChatDock::onAddSelectedToScene()
{
	auto *item = m_chatList->currentItem();
	if (!item)
		return;
	QString name = item->data(Qt::UserRole).toString();

	obs_source_t *source = obs_get_source_by_name(name.toUtf8().constData());
	if (!source) {
		m_actionStatus->setText(QStringLiteral("Ese chat ya no existe. Actualizo la lista."));
		refreshChatList();
		return;
	}
	obs_source_t *sceneSource = obs_frontend_get_current_scene();
	obs_scene_t *scene = sceneSource ? obs_scene_from_source(sceneSource) : nullptr;
	if (!scene) {
		if (sceneSource)
			obs_source_release(sceneSource);
		obs_source_release(source);
		m_actionStatus->setText(QStringLiteral("No hay ninguna escena activa en OBS."));
		return;
	}
	// Referencia del MISMO source en otra escena. Comparte contenido pero como
	// es la misma instancia, comparte tamaño (para tamaños distintos, crear otro).
	obs_scene_add(scene, source);
	obs_source_release(source);
	obs_source_release(sceneSource);
	m_actionStatus->setText(QStringLiteral("\"%1\" añadido a la escena actual.").arg(name));
}

void BetterChatDock::onLogout()
{
	m_api->logout();
}

// ---- Herramientas de prueba ----

void BetterChatDock::onAutoTestToggled(bool on)
{
	if (m_updatingPanel)
		return; // es un reflejo del estado del servidor, no una acción del usuario
	m_api->setAutoTest(on);
}

void BetterChatDock::onClearChat()
{
	m_api->clearChat();
}

// ---- Registro en OBS (puente C) ----
// Se llama desde obs_module_post_load(). En vez de un dock listado en "Paneles"
// (obs_frontend_add_dock_by_id), creamos un menú propio "BetterChatTV" en la
// barra de menús de OBS (junto a Herramientas / Ayuda) que muestra un panel
// acoplable gestionado por nosotros.
extern "C" void betterchat_register_dock(void)
{
	auto *mainWindow = static_cast<QMainWindow *>(obs_frontend_get_main_window());
	if (!mainWindow)
		return;

	auto *dock = new BetterChatDock();
	dock->setMinimumWidth(240);

	// Contenedor acoplable propio (NO via obs_frontend_add_dock_by_id, para que
	// no aparezca en el menú "Paneles"). Empieza flotante y oculto.
	auto *dockWidget = new QDockWidget(QStringLiteral("BetterChatTV"), mainWindow);
	dockWidget->setObjectName(QStringLiteral("BetterChatTVDock"));
	dockWidget->setWidget(dock);
	dockWidget->setFloating(true);
	dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
	mainWindow->addDockWidget(Qt::RightDockWidgetArea, dockWidget);
	dockWidget->hide();

	// Menú de nivel superior en la barra de OBS.
	QMenuBar *menuBar = mainWindow->menuBar();
	auto *menu = menuBar->addMenu(QStringLiteral("BetterChatTV"));

	auto *toggleAction = menu->addAction(QStringLiteral("Mostrar panel de BetterChatTV"));
	toggleAction->setCheckable(true);
	QObject::connect(toggleAction, &QAction::toggled, dockWidget,
			 [dockWidget](bool on) { dockWidget->setVisible(on); });
	// Mantener el check sincronizado si el usuario cierra el panel con la X.
	QObject::connect(dockWidget, &QDockWidget::visibilityChanged, toggleAction, [toggleAction](bool visible) {
		QSignalBlocker block(toggleAction);
		toggleAction->setChecked(visible);
	});
}
