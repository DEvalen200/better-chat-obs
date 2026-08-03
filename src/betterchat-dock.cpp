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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QLayoutItem>
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
#include <QStyle>
#include <QLineEdit>
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
#include <QEvent>
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

// Estilo del rebrand: MISMA paleta que la web (bg #120b10, panel #1e141b,
// line #3a2833, texto #f6eef3, acento rosa #ff7ec8, turquesa #3ad9d9, ok/err).
static const char *kStyle = R"CSS(
QWidget#bcRoot { background: #120b10; color: #f6eef3; font-family: 'Segoe UI', sans-serif; }
QLabel { color: #f6eef3; }
QLabel#muted { color: #b3a1ac; font-size: 12px; }
QLabel#title { font-size: 15px; font-weight: 700; }
QPushButton#primary {
	background: #ff7ec8; color: #120b10; border: 0; border-radius: 8px;
	padding: 9px 14px; font-weight: 700;
}
QPushButton#primary:hover { background: #ff96d3; }
QPushButton#primary:disabled { background: #7a3f60; color: #c79ab3; }
QPushButton#ghost {
	background: transparent; color: #b3a1ac; border: 1px solid #3a2833;
	border-radius: 8px; padding: 7px 12px;
}
QPushButton#accent {
	background: #3ad9d9; color: #0b2b2b; border: 0; border-radius: 8px;
	padding: 7px 12px; font-weight: 700;
}
QPushButton#accent:hover { background: #57e2e2; }
QPushButton#accent:disabled { background: #275555; color: #6f9191; }
QPushButton#danger {
	background: #ff5d6c; color: #2a0006; border: 0; border-radius: 8px;
	padding: 7px 12px; font-weight: 700;
}
QPushButton#danger:hover { background: #ff7580; }
QPushButton#danger:disabled { background: #5c2b30; color: #a97b80; }
QLabel#liveOn {
	background: #3ad07a; color: #0b2b16; border-radius: 6px;
	padding: 3px 9px; font-weight: 700; font-size: 12px;
}
QLabel#liveOff {
	background: #291b24; color: #b3a1ac; border: 1px solid #3a2833;
	border-radius: 6px; padding: 3px 9px; font-size: 12px;
}
QLabel#code {
	background: #291b24; border: 1px solid #3a2833; border-radius: 8px;
	padding: 10px; font-family: monospace; font-size: 20px; font-weight: 700;
	letter-spacing: 3px;
}
QFrame#card { background: #1e141b; border: 1px solid #3a2833; border-radius: 12px; }
QListWidget#chatList {
	background: #1e141b; border: 1px solid #3a2833; border-radius: 8px;
	color: #f6eef3; padding: 4px; outline: 0;
}
QListWidget#chatList::item { padding: 3px 2px; border-radius: 6px; }
QListWidget#chatList::item:selected { background: #ff7ec8; color: #120b10; }
QPushButton#ghost:disabled { color: #6b5a63; border-color: #291b24; }
QPushButton#trash {
	background: transparent; border: 1px solid transparent; border-radius: 6px;
}
QPushButton#trash:hover { background: #3a1f2e; border-color: #ff7ec8; }
QComboBox {
	background: #291b24; color: #f6eef3; border: 1px solid #3a2833;
	border-radius: 6px; padding: 4px 8px;
}
QComboBox QAbstractItemView {
	background: #291b24; color: #f6eef3; selection-background-color: #ff7ec8;
	selection-color: #120b10; border: 1px solid #3a2833;
}
QCheckBox { color: #f6eef3; font-size: 12px; spacing: 8px; }
QFrame#sep { color: #3a2833; max-height: 1px; }
QWidget#navBar { background: #1e141b; border-radius: 9px; }
QPushButton#navTab {
	background: transparent; color: #b3a1ac; border: 0;
	border-bottom: 2px solid transparent; border-radius: 0;
	padding: 8px 4px; font-size: 12px; font-weight: 600;
}
QPushButton#navTab:hover { color: #f6eef3; }
QPushButton#navTab:checked {
	color: #ff7ec8; border-bottom: 2px solid #ff7ec8;
}
QScrollArea#scrollPage { background: transparent; }
QScrollArea#scrollPage > QWidget > QWidget { background: transparent; }
QListWidget#multiList {
	background: #1e141b; border: 1px solid #3a2833; border-radius: 8px;
	color: #f6eef3; padding: 4px; outline: 0;
}
QListWidget#multiList::item { border-bottom: 1px solid #291b24; }
QTextEdit#multiChat {
	background: #1e141b; border: 1px solid #3a2833; border-radius: 8px;
	color: #f6eef3; padding: 4px;
}
QPushButton#connBtn {
	background: #3ad9d9; color: #0b2b2b; border: 0; border-radius: 6px;
	padding: 3px 9px; font-size: 11px; font-weight: 700;
}
QPushButton#connBtn:hover { background: #57e2e2; }
QPushButton#ytPickBtn {
	background: #ff0000; color: #fff; border: 0; border-radius: 6px;
	padding: 3px 9px; font-size: 11px; font-weight: 700;
}
QPushButton#ytPickBtn:hover { background: #ff3333; }
QLabel#ytSearching { color: #ffb347; font-size: 11px; font-style: italic; }
QLabel#ytHelp { color: #b3a1ac; font-size: 11px; }
QLabel#verLabel { color: #6b5a63; font-size: 10px; padding: 2px 2px 0 0; }
QLineEdit {
	background: #291b24; color: #f6eef3; border: 1px solid #3a2833;
	border-radius: 6px; padding: 6px 8px; font-size: 12px;
}
QLineEdit:focus { border-color: #ff7ec8; }
/* ===== Pestaña Minijuegos: bloque turquesa fiel a la web (/control) ===== */
QFrame#betsBlock {
	background: #3ad9d9; border: 3px solid rgba(18,11,16,0.9); border-radius: 18px;
}
/* Textos e inputs sobre turquesa: tinta oscura */
QFrame#betsBlock QLabel#ytPickTitle { color: #120b10; font-size: 15px; font-weight: 800; }
QFrame#betsBlock QLabel#muted { color: #134a4a; font-size: 12px; }
QFrame#betsBlock QLabel#betNote { color: #134a4a; font-size: 11px; }
QLabel#betLockNote {
	background: rgba(18,11,16,0.1); border: 1px solid rgba(18,11,16,0.22);
	color: #120b10; border-radius: 11px; padding: 12px; font-size: 12px;
}
QFrame#betsBlock QLineEdit {
	background: #ffffff; color: #120b10; border: 1px solid rgba(18,11,16,0.2);
	border-radius: 7px; padding: 7px 9px; font-size: 12px;
}
QFrame#betsBlock QLineEdit:focus { border-color: #120b10; }
/* Campo con error de validación: borde rojo grueso */
QFrame#betsBlock QLineEdit[betError="true"] {
	border: 2px solid #ff5d6c; background: #fff2f3;
}
/* Botón de eliminar una opción (✕) */
QPushButton#betDelOpt {
	background: #ffffff; color: #ff5d6c; border: 1px solid #159a9a;
	border-radius: 7px; font-size: 13px; font-weight: 700;
}
QPushButton#betDelOpt:hover { background: #ff5d6c; color: #ffffff; border-color: #ff5d6c; }
/* Botón "Copiar ajustes" del historial (recuadro oscuro, fuera del turquesa) */
QFrame#histCard {
	background: #1e141b; border: 1px solid #3a2833; border-radius: 14px;
}
QLabel#histTitle {
	color: #b3a1ac; font-size: 11px; font-weight: 800; letter-spacing: 1px;
}
QPushButton#histRepeat {
	background: #291b24; color: #f6eef3; border: 1px solid #3a2833; border-radius: 7px;
	padding: 5px 11px; font-size: 11px; font-weight: 700;
}
QPushButton#histRepeat:hover { border-color: #ff7ec8; }
QPushButton#histSummary {
	background: #291b24; color: #f6eef3; border: 1px solid #3a2833; border-radius: 7px;
	padding: 5px 11px; font-size: 11px; font-weight: 700;
}
QPushButton#histSummary:hover { border-color: #3ad9d9; }
QLabel#histText { color: #f6eef3; font-size: 11px; }
QFrame#histItem {
	background: #291b24; border: 1px solid #3a2833; border-radius: 8px;
}
QFrame#betsBlock QComboBox {
	background: #ffffff; color: #120b10; border: 1px solid #159a9a;
	border-radius: 7px; padding: 5px 8px; font-size: 12px;
}
QFrame#betsBlock QComboBox::drop-down {
	subcontrol-origin: padding; subcontrol-position: center right;
	width: 22px; border: 0;
}
QFrame#betsBlock QComboBox::down-arrow { image: none; width: 0; height: 0; }
/* Botón "Crear apuesta" y acciones principales: tinta oscura sólida */
QFrame#betsBlock QPushButton#primary {
	background: #120b10; color: #ffffff; border: 0; border-radius: 8px;
	padding: 8px 14px; font-size: 12px; font-weight: 700;
}
QFrame#betsBlock QPushButton#primary:hover { background: #2a1a24; }
/* Botón fantasma (volver, añadir opción): translúcido oscuro */
QFrame#betsBlock QPushButton#betGhost, QFrame#betsBlock QPushButton#backBtn {
	background: rgba(18,11,16,0.08); color: #120b10;
	border: 1px solid rgba(18,11,16,0.2); border-radius: 7px;
	padding: 6px 11px; font-size: 12px; font-weight: 600;
}
QFrame#betsBlock QPushButton#betGhost:hover, QFrame#betsBlock QPushButton#backBtn:hover {
	border-color: rgba(18,11,16,0.9);
}
QFrame#betsBlock QPushButton#betGhost:disabled { color: #134a4a; background: rgba(18,11,16,0.04); }
/* Cerrar apuestas = verde ok; Cancelar = rojo err */
QFrame#betsBlock QPushButton#accent {
	background: #3ad07a; color: #0b0c10; border: 0; border-radius: 8px;
	padding: 8px 14px; font-size: 12px; font-weight: 700;
}
QFrame#betsBlock QPushButton#danger {
	background: #ff5d6c; color: #ffffff; border: 0; border-radius: 8px;
	padding: 8px 14px; font-size: 12px; font-weight: 700;
}
/* Pill del bote: verde */
QLabel#betPot {
	background: #3ad07a; color: #0b0c10; border-radius: 999px;
	padding: 3px 10px; font-size: 12px; font-weight: 700;
}
QLabel#betLockPill {
	background: rgba(18,11,16,0.12); color: #120b10; border-radius: 999px;
	padding: 3px 10px; font-size: 11px; font-weight: 700;
}
/* Fila de opción (pill blanca sólida con etiqueta + agregados) */
QFrame#betOpt {
	background: #ffffff; border: 1px solid #159a9a; border-radius: 9px;
}
QLabel#betOptLabel { color: #120b10; font-size: 13px; font-weight: 600; }
QLabel#betOptAgg { color: #134a4a; font-size: 11px; }
QPushButton#betWin {
	background: #3ad07a; color: #0b0c10; border: 0; border-radius: 7px;
	padding: 5px 10px; font-size: 11px; font-weight: 700;
}
/* Tarjetas de la galería: blancas sobre turquesa (bordes SÓLIDOS, no rgba: en Qt
   los bordes translúcidos redondeados se ven borrosos/lavados) */
QFrame#gameCard {
	background: #ffffff; border: 2px solid #159a9a; border-radius: 12px;
}
QFrame#gameCard:hover { border-color: #0b2b2b; }
QFrame#gameCardSoon {
	background: #d6f3f3; border: 2px solid #6fc7c7; border-radius: 12px;
}
QLabel#gameCardName { color: #120b10; font-size: 13px; font-weight: 800; }
QFrame#gameCardSoon QLabel#gameCardName { color: #2a5555; }
QLabel#gameCardDesc { color: #134a4a; font-size: 11px; }
QFrame#gameCardSoon QLabel#gameCardDesc { color: #3a6a6a; }
QPushButton#backBtn {
	background: transparent; color: #b3a1ac; border: 1px solid #3a2833;
	border-radius: 6px; padding: 4px 10px; font-size: 12px;
}
QPushButton#backBtn:hover { border-color: #ff7ec8; color: #f6eef3; }
QPushButton#fsBtn {
	background: rgba(33,24,21,0.82); border: 1px solid #3a2833; border-radius: 6px;
}
QPushButton#fsBtn:hover { background: #291b24; border-color: #ff7ec8; }
QWidget#ytPicker {
	background: #1e141b; border: 1px solid #3a2833; border-radius: 8px;
}
QLabel#ytPickTitle { color: #f6eef3; font-weight: 700; font-size: 12px; }
QPushButton#ytUseBtn {
	background: #ff7ec8; color: #120b10; border: 0; border-radius: 6px;
	padding: 4px 12px; font-size: 12px; font-weight: 700;
}
QPushButton#ytUseBtn:hover { background: #ff96d3; }
QSlider::groove:horizontal { height: 4px; background: #3a2833; border-radius: 2px; }
QSlider::sub-page:horizontal { background: #ff7ec8; border-radius: 2px; }
QSlider::handle:horizontal {
	width: 14px; height: 14px; margin: -6px 0; border-radius: 7px;
	background: #ff7ec8; border: 2px solid #120b10;
}
QSpinBox {
	background: #291b24; color: #f6eef3; border: 1px solid #3a2833;
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
		QColor track = isChecked() ? QColor("#ff7ec8") : QColor("#4a3a44");
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
			p.setPen(QColor(isEnabled() ? "#f6eef3" : "#8a7684"));
			QRect tr(w + 8, 0, width() - w - 8, height());
			p.drawText(tr, Qt::AlignVCenter | Qt::AlignLeft, text());
		}
	}
};

// QComboBox que dibuja su propia flecha con QPainter (Qt no rasteriza SVG en QSS
// y el PNG-en-QSS tampoco se ve en algunos OBS; QPainter es lo único fiable).
class ArrowComboBox : public QComboBox {
public:
	explicit ArrowComboBox(QWidget *parent = nullptr) : QComboBox(parent) {}
	void setArrowColor(const QColor &c) { m_arrow = c; }

protected:
	void paintEvent(QPaintEvent *ev) override
	{
		QComboBox::paintEvent(ev);
		QPainter p(this);
		p.setRenderHint(QPainter::Antialiasing, true);
		QPen pen(m_arrow);
		pen.setWidth(2);
		pen.setCapStyle(Qt::RoundCap);
		pen.setJoinStyle(Qt::RoundJoin);
		p.setPen(pen);
		// Chevron hacia abajo, centrado verticalmente y pegado a la derecha.
		const int cx = width() - 16;
		const int cy = height() / 2;
		p.drawLine(cx - 4, cy - 2, cx, cy + 2);
		p.drawLine(cx, cy + 2, cx + 4, cy - 2);
	}

private:
	QColor m_arrow{"#120b10"};
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
		// Sesión ya activa al arrancar (p. ej. OBS reabierto): arrancar también
		// el poll de apuestas y pedir el estado, igual que hace onLoggedIn.
		if (m_betPollTimer)
			m_betPollTimer->start();
		refreshBets();
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
	setAttribute(Qt::WA_StyledBackground, true); // pinta el fondo (que no se cuele el gris de OBS)
	setStyleSheet(QString::fromUtf8(kStyle));

	auto *outer = new QVBoxLayout(this);
	outer->setContentsMargins(12, 12, 12, 12);
	outer->setSpacing(10);

	auto *title = new QLabel(QStringLiteral("BetterChatTV"), this);
	title->setObjectName(QStringLiteral("title"));
	m_brandTitle = title;
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

	// Versión del plugin, en pequeño al pie del dock (para soporte).
	auto *ver = new QLabel(QStringLiteral("v%1").arg(QString::fromUtf8(PLUGIN_VERSION)), this);
	ver->setObjectName(QStringLiteral("verLabel"));
	m_verLabel = ver;
	ver->setAlignment(Qt::AlignRight);
	outer->addWidget(ver);

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
		// Se envuelve en un QWidget (m_topRow) para poder ocultarla en fullscreen.
		m_topRow = new QWidget(page);
		auto *row = new QHBoxLayout(m_topRow);
		row->setContentsMargins(0, 0, 0, 0);
		m_userLabel = new QLabel(m_topRow);
		m_userLabel->setObjectName(QStringLiteral("title"));
		row->addWidget(m_userLabel, 1);
		// Botón "Dashboard ↗" que abre el panel de BetterChatTV en el navegador.
		auto *dashBtn = new QPushButton(QStringLiteral("Dashboard ↗"), m_topRow);
		dashBtn->setObjectName(QStringLiteral("accent"));
		dashBtn->setCursor(Qt::PointingHandCursor);
		connect(dashBtn, &QPushButton::clicked, this, [this]() {
			QString base = m_api->baseUrl();
			if (base.isEmpty())
				base = QStringLiteral("https://betterchat.tv");
			QDesktopServices::openUrl(QUrl(base + QStringLiteral("/dashboard")));
		});
		row->addWidget(dashBtn, 0);
		m_liveBadge = new QLabel(m_topRow);
		m_liveBadge->setObjectName(QStringLiteral("liveOff"));
		m_liveBadge->setText(QStringLiteral("No estás en directo"));
		row->addWidget(m_liveBadge, 0, Qt::AlignRight);
		outerV->addWidget(m_topRow);

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

			m_multiChat = new QTextEdit(tab);
			m_multiChat->setObjectName(QStringLiteral("multiChat"));
			m_multiChat->setReadOnly(true);
			m_multiChat->setFocusPolicy(Qt::NoFocus);
			registerPlatformIcons(); // registra los iconos en el documento del multichat

			// Fila de chips: estado de conexión de cada plataforma.
			buildPlatformBar(tv);

			m_multiStatus = new QLabel(
				QStringLiteral("Esperando mensajes del chat en vivo..."), tab);
			m_multiStatus->setObjectName(QStringLiteral("muted"));
			m_multiStatus->setWordWrap(true);
			tv->addWidget(m_multiStatus);

			m_multiChat->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
			// Descarte NATIVO de bloques antiguos: al superar el tope, Qt borra el
			// bloque más viejo automáticamente (mucho más barato que hacerlo con
			// cursor). 1000 mensajes de historial sin coste de acumulación.
			m_multiChat->document()->setMaximumBlockCount(1000);
			tv->addWidget(m_multiChat, 1);

			// Botón flotante de pantalla completa en la esquina sup. der. del chat.
			// Al pulsarlo, el multichat ocupa toda la ventana (oculta el resto del
			// chrome); un segundo clic restaura. Se reposiciona con un event filter.
			m_fsBtn = new QPushButton(m_multiChat);
			m_fsBtn->setObjectName(QStringLiteral("fsBtn"));
			m_fsBtn->setCursor(Qt::PointingHandCursor);
			m_fsBtn->setFixedSize(26, 26);
			m_fsBtn->setToolTip(QStringLiteral("Ver el chat a pantalla completa"));
			m_fsBtn->setIcon(makeExpandIcon(false));
			m_fsBtn->setIconSize(QSize(15, 15));
			connect(m_fsBtn, &QPushButton::clicked, this, &BetterChatDock::toggleChatFullscreen);
			m_multiChat->installEventFilter(this); // para reposicionar el botón al redimensionar
			positionFsButton();

			// La conexión de YouTube es AUTOMÁTICA (se detecta el directo solo al
			// transmitir), así que ya no hay selector manual de directo al pie.

			m_tabStack->addWidget(tab); // índice 1
		}

		// ===== Pestaña 2: Minijuegos/Apuestas =====
		{
			auto *tab = new QWidget(m_tabStack);
			auto *tv = new QVBoxLayout(tab);
			tv->setContentsMargins(0, 0, 0, 0);
			tv->setSpacing(8);
			buildBetsTab(tv, tab);
			// En un scroll: la galería tiene varias tarjetas y sin scroll el dock
			// flotante se estiraba verticalmente rompiendo la disposición.
			auto *scroll = new QScrollArea(m_tabStack);
			scroll->setWidget(tab);
			scroll->setWidgetResizable(true);
			scroll->setFrameShape(QFrame::NoFrame);
			scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			scroll->setObjectName(QStringLiteral("scrollPage"));
			m_tabStack->addWidget(scroll); // índice 2
		}

		m_stack->addWidget(page);
	}
}

// ==== Pestaña Minijuegos: apuestas con fichas ====

// Construye la pestaña de apuestas: un formulario para crear y un panel para la
// apuesta en curso (se alternan según haya o no apuesta activa). Refresca por poll.
void BetterChatDock::buildBetsTab(QVBoxLayout *parent, QWidget *tab)
{
	// Bloque turquesa protagonista, fiel a la estética de la web (/control).
	m_betsBlock = new QFrame(tab);
	m_betsBlock->setObjectName(QStringLiteral("betsBlock"));
	m_betsBlock->setAttribute(Qt::WA_StyledBackground, true);
	auto *bp = new QVBoxLayout(m_betsBlock);
	bp->setContentsMargins(16, 16, 16, 16);
	bp->setSpacing(8);

	// Aviso para quien no tiene BetterChatTV+ (las apuestas son premium).
	m_betGateMsg = new QLabel(
		QStringLiteral("Las apuestas con fichas son parte de BetterChatTV+. "
			       "Hazte plus para crear apuestas en tu directo."),
		m_betsBlock);
	m_betGateMsg->setObjectName(QStringLiteral("betLockNote"));
	m_betGateMsg->setWordWrap(true);
	m_betGateMsg->setVisible(false);
	bp->addWidget(m_betGateMsg);

	// ---- Galería de tipos de minijuego/apuesta (como en la web) ----
	// Predicción está disponible; el resto se muestran como "Próximamente".
	m_betGrid = new QWidget(tab);
	auto *gv = new QVBoxLayout(m_betGrid);
	gv->setContentsMargins(0, 0, 0, 0);
	gv->setSpacing(6);
	auto *gt = new QLabel(QStringLiteral("Elige un minijuego o apuesta"), m_betGrid);
	gt->setObjectName(QStringLiteral("ytPickTitle"));
	gv->addWidget(gt);
	struct GameType {
		const char *id;
		const char *name;
		const char *desc;
		bool ready;
	};
	const GameType types[] = {
		{"manual", "🎲  Predicción",
		 "Tú escribes la pregunta y las opciones; el chat apuesta y tú eliges quién gana.", true},
		{"coinflip", "🎰  Cara o cruz", "Apuesta rápida a cara o cruz, resultado automático.", false},
		{"roulette", "🎡  Ruleta", "Rueda de opciones con giro y ganador al azar.", false},
		{"dice", "🎯  Dado", "Apuesta al número que saldrá.", false},
		{"rps", "✊  Piedra, papel o tijera", "Duelo del chat contra el streamer.", false},
		{"race", "🏁  Carrera", "Apuesta por el corredor ganador.", false},
	};
	for (const GameType &g : types) {
		// Tarjeta = contenedor clicable con título + descripción que AJUSTAN el
		// texto (un QPushButton no envuelve ni crece en alto: se cortaba).
		auto *card = new QFrame(m_betGrid);
		card->setObjectName(g.ready ? QStringLiteral("gameCard") : QStringLiteral("gameCardSoon"));
		card->setAttribute(Qt::WA_StyledBackground, true); // que el QSS pinte fondo/borde
		card->setCursor(g.ready ? Qt::PointingHandCursor : Qt::ArrowCursor);
		auto *cardV = new QVBoxLayout(card);
		cardV->setContentsMargins(12, 10, 12, 10);
		cardV->setSpacing(3);
		auto *nameLbl = new QLabel(QString::fromUtf8(g.name), card);
		nameLbl->setObjectName(QStringLiteral("gameCardName"));
		cardV->addWidget(nameLbl);
		auto *descLbl = new QLabel(
			g.ready ? QString::fromUtf8(g.desc)
				: QStringLiteral("%1  ·  Próximamente").arg(QString::fromUtf8(g.desc)),
			card);
		descLbl->setObjectName(QStringLiteral("gameCardDesc"));
		descLbl->setWordWrap(true);
		cardV->addWidget(descLbl);
		if (g.ready) {
			const QString id = QString::fromUtf8(g.id);
			// Capturamos el clic en toda la tarjeta con un event filter.
			card->installEventFilter(this);
			card->setProperty("gameType", id);
		}
		gv->addWidget(card);
	}
	bp->addWidget(m_betGrid);

	// ---- Formulario para crear una apuesta ----
	m_betCreate = new QWidget(tab);
	auto *cv = new QVBoxLayout(m_betCreate);
	cv->setContentsMargins(0, 0, 0, 0);
	cv->setSpacing(6);
	// Volver a la galería (solo visible en el formulario, antes de crear nada).
	auto *backBtn = new QPushButton(QStringLiteral("← Volver"), m_betCreate);
	backBtn->setObjectName(QStringLiteral("backBtn"));
	backBtn->setCursor(Qt::PointingHandCursor);
	connect(backBtn, &QPushButton::clicked, this, [this]() {
		m_pickedType.clear();
		m_betMsg->clear();
		updateBetView();
	});
	cv->addWidget(backBtn, 0, Qt::AlignLeft);
	auto *ct = new QLabel(QStringLiteral("Nueva predicción"), m_betCreate);
	ct->setObjectName(QStringLiteral("ytPickTitle"));
	cv->addWidget(ct);
	m_betTitle = new QLineEdit(m_betCreate);
	m_betTitle->setPlaceholderText(QStringLiteral("¿Sobre qué se apuesta?"));
	cv->addWidget(m_betTitle);
	// Campos de opción DINÁMICOS: arranca con 2, botón para añadir más (como la web).
	m_betCreateForm = m_betCreate;
	m_betOptInputs = new QVBoxLayout();
	m_betOptInputs->setSpacing(6);
	cv->addLayout(m_betOptInputs);
	addBetOptionField();
	addBetOptionField();
	m_betAddOptBtn = new QPushButton(QStringLiteral("+ Añadir opción"), m_betCreate);
	m_betAddOptBtn->setObjectName(QStringLiteral("betGhost"));
	m_betAddOptBtn->setCursor(Qt::PointingHandCursor);
	connect(m_betAddOptBtn, &QPushButton::clicked, this, [this]() { addBetOptionField(); });
	cv->addWidget(m_betAddOptBtn, 0, Qt::AlignLeft);

	auto *durRow = new QHBoxLayout();
	auto *durLbl = new QLabel(QStringLiteral("Cierre automático:"), m_betCreate);
	durLbl->setObjectName(QStringLiteral("muted"));
	durRow->addWidget(durLbl, 0);
	m_betDuration = new ArrowComboBox(m_betCreate);
	m_betDuration->addItem(QStringLiteral("Sin límite"), 0);
	m_betDuration->addItem(QStringLiteral("1 min"), 60);
	m_betDuration->addItem(QStringLiteral("2 min"), 120);
	m_betDuration->addItem(QStringLiteral("5 min"), 300);
	durRow->addWidget(m_betDuration, 1);
	cv->addLayout(durRow);

	m_betCreateBtn = new QPushButton(QStringLiteral("Crear apuesta"), m_betCreate);
	m_betCreateBtn->setObjectName(QStringLiteral("primary"));
	m_betCreateBtn->setCursor(Qt::PointingHandCursor);
	connect(m_betCreateBtn, &QPushButton::clicked, this, &BetterChatDock::onCreateBetClicked);
	cv->addWidget(m_betCreateBtn);
	m_betCreate->setVisible(false); // oculto hasta elegir un tipo (vista por pantallas)
	bp->addWidget(m_betCreate);

	// ---- Panel de la apuesta en curso ----
	m_betActive = new QWidget(tab);
	auto *av = new QVBoxLayout(m_betActive);
	av->setContentsMargins(0, 0, 0, 0);
	av->setSpacing(6);
	m_betActiveTitle = new QLabel(QString(), m_betActive);
	m_betActiveTitle->setObjectName(QStringLiteral("ytPickTitle"));
	m_betActiveTitle->setWordWrap(true);
	av->addWidget(m_betActiveTitle);
	m_betActiveState = new QLabel(QString(), m_betActive);
	m_betActiveState->setObjectName(QStringLiteral("muted"));
	av->addWidget(m_betActiveState);
	m_betOptsBox = new QVBoxLayout();
	m_betOptsBox->setSpacing(5);
	av->addLayout(m_betOptsBox);

	auto *actRow = new QHBoxLayout();
	m_betLockBtn = new QPushButton(QStringLiteral("Cerrar apuestas"), m_betActive);
	m_betLockBtn->setObjectName(QStringLiteral("accent"));
	m_betLockBtn->setCursor(Qt::PointingHandCursor);
	m_betLockBtn->setToolTip(QStringLiteral("Nadie más podrá apostar; luego marca el ganador"));
	connect(m_betLockBtn, &QPushButton::clicked, this, [this]() { m_api->betAction(QStringLiteral("lock")); });
	actRow->addWidget(m_betLockBtn, 1);
	m_betCancelBtn = new QPushButton(QStringLiteral("Cancelar y devolver"), m_betActive);
	m_betCancelBtn->setObjectName(QStringLiteral("danger"));
	m_betCancelBtn->setCursor(Qt::PointingHandCursor);
	m_betCancelBtn->setToolTip(QStringLiteral("Anula la apuesta y devuelve las fichas"));
	connect(m_betCancelBtn, &QPushButton::clicked, this, [this]() { m_api->betAction(QStringLiteral("cancel")); });
	actRow->addWidget(m_betCancelBtn, 0);
	av->addLayout(actRow);
	m_betActive->setVisible(false);
	bp->addWidget(m_betActive);

	m_betMsg = new QLabel(QString(), m_betsBlock);
	m_betMsg->setObjectName(QStringLiteral("betNote"));
	m_betMsg->setWordWrap(true);
	bp->addWidget(m_betMsg);

	parent->addWidget(m_betsBlock);

	// ---- Historial reciente: recuadro OSCURO PROPIO, fuera del turquesa (como la web) ----
	m_betHistory = new QFrame(tab);
	m_betHistory->setObjectName(QStringLiteral("histCard"));
	m_betHistory->setAttribute(Qt::WA_StyledBackground, true);
	auto *hv = new QVBoxLayout(m_betHistory);
	hv->setContentsMargins(16, 14, 16, 14);
	hv->setSpacing(8);
	auto *ht = new QLabel(QStringLiteral("Historial reciente"), m_betHistory);
	ht->setObjectName(QStringLiteral("histTitle"));
	hv->addWidget(ht);
	m_betHistoryBox = new QVBoxLayout();
	m_betHistoryBox->setSpacing(5);
	hv->addLayout(m_betHistoryBox);
	// Controles de paginación (5 por página): anterior / X de Y / siguiente.
	auto *pageRow = new QHBoxLayout();
	pageRow->setSpacing(6);
	m_betHistPrev = new QPushButton(QStringLiteral("‹ Anteriores"), m_betHistory);
	m_betHistPrev->setObjectName(QStringLiteral("ghost"));
	m_betHistPrev->setCursor(Qt::PointingHandCursor);
	connect(m_betHistPrev, &QPushButton::clicked, this, [this]() {
		if (m_betHistoryPage > 0) {
			--m_betHistoryPage;
			renderHistoryPage();
		}
	});
	pageRow->addWidget(m_betHistPrev, 0);
	m_betHistoryPageLbl = new QLabel(QString(), m_betHistory);
	m_betHistoryPageLbl->setObjectName(QStringLiteral("muted"));
	m_betHistoryPageLbl->setAlignment(Qt::AlignCenter);
	pageRow->addWidget(m_betHistoryPageLbl, 1);
	m_betHistNext = new QPushButton(QStringLiteral("Siguientes ›"), m_betHistory);
	m_betHistNext->setObjectName(QStringLiteral("ghost"));
	m_betHistNext->setCursor(Qt::PointingHandCursor);
	connect(m_betHistNext, &QPushButton::clicked, this, [this]() {
		const int pages = (m_betHistoryData.size() + 4) / 5;
		if (m_betHistoryPage < pages - 1) {
			++m_betHistoryPage;
			renderHistoryPage();
		}
	});
	pageRow->addWidget(m_betHistNext, 0);
	hv->addLayout(pageRow);
	m_betHistory->setVisible(false);
	parent->addWidget(m_betHistory);
	parent->addStretch(1);

	// Señales del API + poll periódico del estado (cada 4s mientras el dock está vivo).
	connect(m_api, &BetterChatApi::controlState, this, &BetterChatDock::onControlState);
	connect(m_api, &BetterChatApi::betActionResult, this, [this](bool ok, const QString &msg) {
		if (!msg.isEmpty())
			m_betMsg->setText(msg);
		else if (ok)
			m_betMsg->clear();
		if (ok) {
			// Al crear/resolver/cancelar, olvidar el tipo elegido: si ya no hay
			// apuesta activa se vuelve a la galería, no al formulario.
			m_pickedType.clear();
			// Limpiar el formulario para la próxima: título y opciones (dejar 2 vacías).
			if (m_betTitle) m_betTitle->clear();
			clearBetFieldErrors();
			resetBetOptions();
		}
	});
	m_betPollTimer = new QTimer(this);
	m_betPollTimer->setInterval(4000);
	connect(m_betPollTimer, &QTimer::timeout, this, &BetterChatDock::refreshBets);

	// Estado inicial de pantallas: galería visible, formulario y panel ocultos.
	updateBetView();
}

// Pide el estado del panel de control (apuesta activa) al servidor.
void BetterChatDock::refreshBets()
{
	m_api->fetchControl();
}

// Crea la apuesta a partir del formulario, validando los campos mínimos.
// Si falta algo, resalta en rojo el/los campos implicados.
void BetterChatDock::onCreateBetClicked()
{
	clearBetFieldErrors();
	const QString title = m_betTitle->text().trimmed();
	QList<QLineEdit *> emptyOpts;
	int filled = 0;
	for (QLineEdit *e : m_betOptFields) {
		if (e->text().trimmed().isEmpty())
			emptyOpts << e;
		else
			++filled;
	}
	bool bad = false;
	if (title.isEmpty()) {
		markBetFieldError(m_betTitle);
		bad = true;
	}
	if (filled < 2) {
		// Faltan opciones: marcar las vacías (al menos las 2 primeras).
		for (QLineEdit *e : emptyOpts)
			markBetFieldError(e);
		bad = true;
	}
	if (bad) {
		m_betMsg->setText(QStringLiteral("Rellena los campos marcados en rojo."));
		return;
	}
	QStringList opts;
	for (QLineEdit *e : m_betOptFields) {
		const QString v = e->text().trimmed();
		if (!v.isEmpty())
			opts << v;
	}
	m_betMsg->setText(QStringLiteral("Creando apuesta..."));
	m_api->createBet(title, opts, m_betDuration->currentData().toInt());
}

// Marca un campo como erróneo (borde rojo) hasta que el usuario lo edite.
void BetterChatDock::markBetFieldError(QLineEdit *field)
{
	if (!field)
		return;
	field->setProperty("betError", true);
	field->style()->unpolish(field);
	field->style()->polish(field);
	// Al empezar a escribir, quitar el estado de error.
	connect(field, &QLineEdit::textEdited, this, [this, field]() {
		if (field->property("betError").toBool()) {
			field->setProperty("betError", false);
			field->style()->unpolish(field);
			field->style()->polish(field);
		}
	});
}

// Limpia el resaltado de error de todos los campos del formulario.
void BetterChatDock::clearBetFieldErrors()
{
	QList<QLineEdit *> all = m_betOptFields;
	all << m_betTitle;
	for (QLineEdit *e : all) {
		if (e && e->property("betError").toBool()) {
			e->setProperty("betError", false);
			e->style()->unpolish(e);
			e->style()->polish(e);
		}
	}
}

// Añade un campo de opción al formulario (dinámico, como el "+ Añadir opción" web).
// Tope de 20 opciones. Las opciones más allá de las 2 primeras llevan botón de
// eliminar a la derecha (las 2 primeras son obligatorias, no se pueden quitar).
void BetterChatDock::addBetOptionField(const QString &text)
{
	if (!m_betOptInputs || !m_betCreateForm)
		return;
	if (m_betOptFields.size() >= 20)
		return;
	const int index = m_betOptFields.size();
	// Fila: campo + (opcional) botón eliminar.
	auto *row = new QWidget(m_betCreateForm);
	auto *rh = new QHBoxLayout(row);
	rh->setContentsMargins(0, 0, 0, 0);
	rh->setSpacing(6);
	auto *field = new QLineEdit(row);
	field->setPlaceholderText(QStringLiteral("Opción %1").arg(index + 1));
	if (!text.isEmpty())
		field->setText(text);
	rh->addWidget(field, 1);
	if (index >= 2) {
		auto *del = new QPushButton(row);
		del->setObjectName(QStringLiteral("betDelOpt"));
		del->setCursor(Qt::PointingHandCursor);
		del->setToolTip(QStringLiteral("Quitar esta opción"));
		del->setFixedWidth(34);
		del->setIcon(makeTrashIcon());
		del->setIconSize(QSize(18, 18));
		connect(del, &QPushButton::clicked, this, [this, row, field]() {
			m_betOptFields.removeOne(field);
			row->deleteLater();
			// Reactivar el botón de añadir y renumerar placeholders.
			if (m_betAddOptBtn) {
				m_betAddOptBtn->setEnabled(true);
				m_betAddOptBtn->setText(QStringLiteral("+ Añadir opción"));
			}
			renumberBetOptions();
		});
		rh->addWidget(del, 0);
	}
	m_betOptInputs->addWidget(row);
	m_betOptFields.append(field);
	if (m_betAddOptBtn) {
		const bool full = m_betOptFields.size() >= 20;
		m_betAddOptBtn->setEnabled(!full);
		m_betAddOptBtn->setText(full ? QStringLiteral("Máximo 20 opciones")
					     : QStringLiteral("+ Añadir opción"));
	}
}

// Renumera los placeholders "Opción N" tras eliminar una opción intermedia.
void BetterChatDock::renumberBetOptions()
{
	int i = 1;
	for (QLineEdit *e : m_betOptFields) {
		if (e->text().trimmed().isEmpty())
			e->setPlaceholderText(QStringLiteral("Opción %1").arg(i));
		++i;
	}
}

// Borra todas las filas de opción y deja el formulario con 2 opciones vacías.
void BetterChatDock::resetBetOptions()
{
	for (QLineEdit *e : m_betOptFields) {
		if (QWidget *rowW = e->parentWidget())
			rowW->deleteLater();
	}
	m_betOptFields.clear();
	if (m_betOptInputs) {
		addBetOptionField();
		addBetOptionField();
	}
}

// Rellena el formulario de predicción con una pregunta y sus opciones (para el
// botón "Repetir" del historial). Crea tantos campos como opciones haya.
void BetterChatDock::fillBetForm(const QString &title, const QStringList &options)
{
	if (m_betTitle)
		m_betTitle->setText(title);
	clearBetFieldErrors();
	// Vaciar y crear los campos justos para estas opciones (mínimo 2).
	for (QLineEdit *e : m_betOptFields) {
		if (QWidget *rowW = e->parentWidget())
			rowW->deleteLater();
	}
	m_betOptFields.clear();
	const int n = qMax(2, options.size());
	for (int i = 0; i < n; ++i)
		addBetOptionField(i < options.size() ? options.at(i) : QString());
}

// Refresca la UI con el estado de la apuesta activa (o el formulario si no hay).
void BetterChatDock::onControlState(const QByteArray &json)
{
	const QJsonObject root = QJsonDocument::fromJson(json).object();
	m_betDataReady = true;
	m_lastIsPlus = root.value(QStringLiteral("isPlus")).toBool();
	const QJsonValue betVal = root.value(QStringLiteral("bet"));
	m_lastHasBet = betVal.isObject();

	// Si hay apuesta activa, pintar su panel con los datos frescos.
	if (m_lastHasBet) {
		const QJsonObject bet = betVal.toObject();
		const QString status = bet.value(QStringLiteral("status")).toString();
		m_activeBetStatus = status;
		m_betActiveTitle->setText(bet.value(QStringLiteral("title")).toString());
		const qint64 bote = static_cast<qint64>(bet.value(QStringLiteral("bote")).toDouble());
		// Estado como pills (bote verde + estado), al estilo de la web.
		if (status == QStringLiteral("locked"))
			m_betActiveState->setText(
				QStringLiteral("%1 fichas en el bote   ·   Cerrada, elige ganador").arg(bote));
		else
			m_betActiveState->setText(
				QStringLiteral("%1 fichas en el bote   ·   Abierta").arg(bote));

		// Reconstruir las filas de opciones con su barra de reparto.
		while (QLayoutItem *it = m_betOptsBox->takeAt(0)) {
			if (it->widget())
				it->widget()->deleteLater();
			delete it;
		}
		const QJsonArray options = bet.value(QStringLiteral("options")).toArray();
		for (const QJsonValue &ov : options) {
			const QJsonObject o = ov.toObject();
			const QString oid = o.value(QStringLiteral("id")).toString();
			const QString label = o.value(QStringLiteral("label")).toString();
			const qint64 stake =
				static_cast<qint64>(o.value(QStringLiteral("totalStake")).toDouble());
			const int n = o.value(QStringLiteral("numWagers")).toInt();
			const int pct = bote > 0 ? int(stake * 100 / bote) : 0;

			// Pill blanca translúcida: etiqueta a la izquierda, agregados a la derecha.
			auto *row = new QFrame(m_betActive);
			row->setObjectName(QStringLiteral("betOpt"));
			row->setAttribute(Qt::WA_StyledBackground, true);
			auto *rh = new QHBoxLayout(row);
			rh->setContentsMargins(10, 7, 10, 7);
			rh->setSpacing(6);
			auto *lbl = new QLabel(label, row);
			lbl->setObjectName(QStringLiteral("betOptLabel"));
			lbl->setWordWrap(true);
			rh->addWidget(lbl, 1);
			if (status == QStringLiteral("locked")) {
				auto *win = new QPushButton(QStringLiteral("Marcar ganador"), row);
				win->setObjectName(QStringLiteral("betWin"));
				win->setCursor(Qt::PointingHandCursor);
				connect(win, &QPushButton::clicked, this,
					[this, oid]() { m_api->betAction(QStringLiteral("resolve"), oid); });
				rh->addWidget(win, 0);
			}
			auto *agg = new QLabel(
				QStringLiteral("%1 · %2 · %3%").arg(stake).arg(n).arg(pct), row);
			agg->setObjectName(QStringLiteral("betOptAgg"));
			rh->addWidget(agg, 0);
			m_betOptsBox->addWidget(row);
		}
		m_betLockBtn->setVisible(status == QStringLiteral("open"));
	} else {
		m_activeBetStatus.clear();
	}

	// Guardar el historial completo y pintar la página actual (5 por página).
	if (m_betHistoryBox) {
		const QJsonArray hist = root.value(QStringLiteral("history")).toArray();
		m_betHistoryData = hist;
		m_lastHistoryCount = hist.size();
		const int pages = (hist.size() + 4) / 5;
		if (m_betHistoryPage >= pages)
			m_betHistoryPage = pages > 0 ? pages - 1 : 0;
		renderHistoryPage();
	}

	updateBetView();
}

// Pinta la página actual del historial (5 predicciones por página).
void BetterChatDock::renderHistoryPage()
{
	if (!m_betHistoryBox)
		return;
	while (QLayoutItem *it = m_betHistoryBox->takeAt(0)) {
		if (it->widget())
			it->widget()->deleteLater();
		delete it;
	}
	const int total = m_betHistoryData.size();
	const int pages = (total + 4) / 5;
	const int start = m_betHistoryPage * 5;
	const int end = qMin(start + 5, total);
	for (int i = start; i < end; ++i) {
		const QJsonObject h = m_betHistoryData.at(i).toObject();
		const QString hid = h.value(QStringLiteral("id")).toString();
		const QString htitle = h.value(QStringLiteral("title")).toString();
		const bool cancelled = h.value(QStringLiteral("status")).toString()
				       == QStringLiteral("cancelled");
		const qint64 hbote = static_cast<qint64>(h.value(QStringLiteral("bote")).toDouble());
		const int hn = h.value(QStringLiteral("numWagers")).toInt();
		const QString win = h.value(QStringLiteral("winningLabel")).toString();
		// Resultado con color como la web: Cancelada en rojo, ganador en verde.
		QString resultHtml;
		if (cancelled)
			resultHtml = QStringLiteral("<span style='color:#ff5d6c;'>Cancelada</span>");
		else if (win.isEmpty())
			resultHtml = QStringLiteral("<span style='color:#b3a1ac;'>—</span>");
		else
			resultHtml = QStringLiteral("<span style='color:#3ad07a;'>✓ %1</span>")
					     .arg(win.toHtmlEscaped());
		QStringList optLabels;
		for (const QJsonValue &lv : h.value(QStringLiteral("optionLabels")).toArray())
			optLabels << lv.toString();

		auto *item = new QFrame(m_betHistory);
		item->setObjectName(QStringLiteral("histItem"));
		item->setAttribute(Qt::WA_StyledBackground, true);
		auto *iv = new QVBoxLayout(item);
		iv->setContentsMargins(10, 7, 10, 7);
		iv->setSpacing(5);
		// Línea 1: título + resultado (con color) + meta.
		auto *txt = new QLabel(item);
		txt->setObjectName(QStringLiteral("histText"));
		txt->setTextFormat(Qt::RichText);
		txt->setWordWrap(true);
		txt->setText(QStringLiteral("<b>%1</b><br>%2 &nbsp;·&nbsp; %3 fichas &nbsp;·&nbsp; %4 apostantes")
				     .arg(htitle.toHtmlEscaped(), resultHtml).arg(hbote).arg(hn));
		iv->addWidget(txt);
		// Línea 2: acciones (Ver resumen + Copiar ajustes).
		auto *actRow = new QHBoxLayout();
		actRow->setSpacing(6);
		actRow->addStretch(1);
		auto *sumBtn = new QPushButton(QStringLiteral(" Ver resumen"), item);
		sumBtn->setObjectName(QStringLiteral("histSummary"));
		sumBtn->setCursor(Qt::PointingHandCursor);
		sumBtn->setIcon(makeChartIcon());
		sumBtn->setIconSize(QSize(14, 14));
		sumBtn->setToolTip(QStringLiteral("Ver el resumen de resultados en el navegador"));
		connect(sumBtn, &QPushButton::clicked, this, [this, hid]() {
			QDesktopServices::openUrl(QUrl(m_api->baseUrl() +
						       QStringLiteral("/control?summary=") + hid));
		});
		actRow->addWidget(sumBtn, 0);
		if (optLabels.size() >= 2) {
			auto *rep = new QPushButton(QStringLiteral(" Copiar ajustes"), item);
			rep->setObjectName(QStringLiteral("histRepeat"));
			rep->setCursor(Qt::PointingHandCursor);
			rep->setIcon(makeCopyIcon());
			rep->setIconSize(QSize(14, 14));
			rep->setToolTip(QStringLiteral("Copiar esta pregunta y sus opciones al formulario"));
			connect(rep, &QPushButton::clicked, this, [this, htitle, optLabels]() {
				fillBetForm(htitle, optLabels);
				m_pickedType = QStringLiteral("manual");
				updateBetView();
				if (m_betMsg)
					m_betMsg->setText(QStringLiteral(
						"Datos copiados. Revisa el tiempo y pulsa Crear apuesta."));
			});
			actRow->addWidget(rep, 0);
		}
		iv->addLayout(actRow);
		m_betHistoryBox->addWidget(item);
	}
	// Etiqueta y botones de paginación.
	if (m_betHistoryPageLbl)
		m_betHistoryPageLbl->setText(
			pages > 0 ? QStringLiteral("Página %1 de %2").arg(m_betHistoryPage + 1).arg(pages)
				  : QString());
	const bool multi = pages > 1;
	if (m_betHistPrev) {
		m_betHistPrev->setVisible(multi);
		m_betHistPrev->setEnabled(m_betHistoryPage > 0);
	}
	if (m_betHistNext) {
		m_betHistNext->setVisible(multi);
		m_betHistNext->setEnabled(m_betHistoryPage < pages - 1);
	}
	if (m_betHistoryPageLbl)
		m_betHistoryPageLbl->setVisible(multi);
}

// Decide qué zona mostrar según el estado: aviso (no plus), panel activo (hay
// apuesta y no puedes salir hasta cerrarla/cancelarla), formulario (elegiste tipo)
// o la galería de tipos por defecto.
void BetterChatDock::updateBetView()
{
	// El historial (como en la web) se muestra SIEMPRE que: eres plus, hay
	// historial, y NO hay una apuesta activa en curso (en la galería y en el
	// formulario). No depende de la pantalla concreta.
	auto refreshHistoryVisibility = [this](bool allowed) {
		if (m_betHistory)
			m_betHistory->setVisible(allowed && m_lastHistoryCount > 0);
	};
	// Antes de recibir el primer estado del servidor, mostramos la galería (sin
	// aviso de gate) para no dar un falso "requiere plus"... salvo que el usuario
	// YA haya elegido un tipo: entonces mostramos su formulario igualmente.
	if (!m_betDataReady && m_pickedType.isEmpty()) {
		m_betGateMsg->setVisible(false);
		m_betGrid->setVisible(true);
		m_betCreate->setVisible(false);
		m_betActive->setVisible(false);
		refreshHistoryVisibility(false);
		return;
	}
	// Si aún no hay datos pero eligió un tipo, asumimos que puede usarlo (el
	// backend rechazará con plus_required si no es plus, y lo reflejará el poll).
	if (!m_betDataReady) {
		m_betGateMsg->setVisible(false);
		m_betGrid->setVisible(m_pickedType.isEmpty());
		m_betCreate->setVisible(m_pickedType == QStringLiteral("manual"));
		m_betActive->setVisible(false);
		refreshHistoryVisibility(m_pickedType == QStringLiteral("manual"));
		return;
	}
	const bool plus = m_lastIsPlus;
	m_betGateMsg->setVisible(!plus);
	if (!plus) {
		m_betGrid->setVisible(false);
		m_betCreate->setVisible(false);
		m_betActive->setVisible(false);
		refreshHistoryVisibility(false);
		return;
	}
	if (m_lastHasBet) {
		// Hay algo en marcha: el panel activo (no se puede salir) + el historial
		// debajo, igual que la web (recuadro propio, siempre visible si hay).
		m_betGrid->setVisible(false);
		m_betCreate->setVisible(false);
		m_betActive->setVisible(true);
		refreshHistoryVisibility(true);
		return;
	}
	if (m_pickedType == QStringLiteral("manual")) {
		// Elegiste Predicción: mostrar su formulario + historial debajo.
		m_betGrid->setVisible(false);
		m_betCreate->setVisible(true);
		m_betActive->setVisible(false);
		refreshHistoryVisibility(true);
		return;
	}
	// Por defecto: la galería de tipos (SIN historial; el historial es de
	// predicciones, solo se muestra dentro de Predicción / apuesta activa).
	m_betGrid->setVisible(true);
	m_betCreate->setVisible(false);
	m_betActive->setVisible(false);
	refreshHistoryVisibility(false);
}

// Cambia la pestaña activa del dock (nav bar) y marca el botón correspondiente.
void BetterChatDock::selectTab(int index)
{
	if (!m_tabStack || index < 0 || index >= m_tabStack->count())
		return;
	m_tabStack->setCurrentIndex(index);
	for (int i = 0; i < m_tabButtons.size(); i++)
		m_tabButtons[i]->setChecked(i == index);
	// El stream del multichat se mantiene VIVO mientras haya sesión (no se corta al
	// salir de la pestaña): así no se reconectan los conectores cada vez que entras
	// (evita el parpadeo "conectando -> chat en vivo"). Se arranca la 1a vez que se
	// entra a Multichat y sigue en marcha hasta cerrar sesión.
	if (index == 1 && !m_api->chatStreamActive()) {
		if (m_multiStatus)
			m_multiStatus->setText(QStringLiteral("Conectando al chat en vivo..."));
		m_api->startChatStream();
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
	// Botón de ajustes: abre la sección de conexiones de "Configura tu chat" en el
	// navegador (reutiliza toda la gestión de conexiones de la web).
	auto *cfgBtn = new QPushButton(QStringLiteral("Conexiones ↗"), m_platBar);
	cfgBtn->setObjectName(QStringLiteral("connBtn"));
	cfgBtn->setCursor(Qt::PointingHandCursor);
	cfgBtn->setToolTip(QStringLiteral("Gestionar las conexiones de plataformas"));
	connect(cfgBtn, &QPushButton::clicked, this, [this]() {
		QString base = m_api->baseUrl();
		if (base.isEmpty())
			base = QStringLiteral("https://betterchat.tv");
		QDesktopServices::openUrl(QUrl(base + QStringLiteral("/customize#conexiones")));
	});
	h->addWidget(cfgBtn, 0);
	parent->addWidget(m_platBar);

	// Estado inicial (se corrige en cuanto lleguen los status por SSE).
	for (auto it = m_platChips.constBegin(); it != m_platChips.constEnd(); ++it)
		onPlatformStatus(it.key(), QStringLiteral("off"), QString());
}

// Construye, al PIE de la pestaña Multichat, la zona de selección de directo de
// YouTube: línea de ayuda contextual + botón + panel plegable con la lista.
void BetterChatDock::buildYouTubePicker(QVBoxLayout *parent)
{
	// Todo dentro de un contenedor para poder ocultarlo de golpe (modo pantalla completa).
	m_ytZone = new QWidget();
	auto *zoneV = new QVBoxLayout(m_ytZone);
	zoneV->setContentsMargins(0, 0, 0, 0);
	zoneV->setSpacing(6);

	// Línea de ayuda contextual: explica si el directo de YouTube se conecta solo
	// (plus) o hay que elegirlo a mano (standard). Se rellena en onStatusUpdated.
	m_ytHelp = new QLabel(QString());
	m_ytHelp->setObjectName(QStringLiteral("ytHelp"));
	m_ytHelp->setWordWrap(true);
	zoneV->addWidget(m_ytHelp);

	// Fila: indicador "buscando" (izq) + botón para desplegar la lista (der).
	auto *ytRow = new QWidget(m_ytZone);
	auto *ytH = new QHBoxLayout(ytRow);
	ytH->setContentsMargins(0, 0, 0, 0);
	ytH->setSpacing(6);
	m_ytSearching = new QLabel(QStringLiteral("Buscando tu directo..."), ytRow);
	m_ytSearching->setObjectName(QStringLiteral("ytSearching"));
	m_ytSearching->setVisible(false);
	ytH->addWidget(m_ytSearching, 0);
	ytH->addStretch(1);
	m_ytPickBtn = new QPushButton(QStringLiteral("Elegir directo de YouTube"), ytRow);
	m_ytPickBtn->setObjectName(QStringLiteral("ytPickBtn"));
	m_ytPickBtn->setCursor(Qt::PointingHandCursor);
	m_ytPickBtn->setToolTip(QStringLiteral("Elegir tu directo de YouTube en curso"));
	connect(m_ytPickBtn, &QPushButton::clicked, this, &BetterChatDock::toggleYouTubePicker);
	ytH->addWidget(m_ytPickBtn, 0);
	zoneV->addWidget(ytRow);

	// Panel plegable del selector de directos de YouTube (oculto por defecto).
	m_ytPicker = new QWidget(m_ytZone);
	m_ytPicker->setObjectName(QStringLiteral("ytPicker"));
	auto *pv = new QVBoxLayout(m_ytPicker);
	pv->setContentsMargins(10, 8, 10, 8);
	pv->setSpacing(6);
	auto *ptitle = new QLabel(QStringLiteral("Tus directos de YouTube en curso"), m_ytPicker);
	ptitle->setObjectName(QStringLiteral("ytPickTitle"));
	pv->addWidget(ptitle);
	m_ytPickerMsg = new QLabel(QString(), m_ytPicker);
	m_ytPickerMsg->setObjectName(QStringLiteral("muted"));
	m_ytPickerMsg->setWordWrap(true);
	pv->addWidget(m_ytPickerMsg);
	m_ytList = new QVBoxLayout();
	m_ytList->setSpacing(5);
	pv->addLayout(m_ytList);
	m_ytPicker->setVisible(false);
	zoneV->addWidget(m_ytPicker);

	parent->addWidget(m_ytZone);

	// Conexiones del selector con el API.
	connect(m_api, &BetterChatApi::youtubeLiveList, this, &BetterChatDock::onYouTubeLiveList);
	connect(m_api, &BetterChatApi::youtubeLiveError, this, &BetterChatDock::onYouTubeLiveError);
	connect(m_api, &BetterChatApi::youtubeSourceSet, this, [this](bool ok, const QString &msg) {
		if (m_ytPickerMsg)
			m_ytPickerMsg->setText(msg);
		if (ok) {
			// Ocultar el panel tras elegir; el chat se reconectará solo.
			if (m_ytPicker)
				m_ytPicker->setVisible(false);
			if (m_ytPickBtn)
				m_ytPickBtn->setText(QStringLiteral("Elegir directo de YouTube"));
		}
	});
}

// Icono de pantalla completa dibujado con QPainter (nada de SVG en runtime).
// expanded=false -> flechas hacia fuera (entrar a fullscreen); true -> hacia dentro.
QIcon BetterChatDock::makeExpandIcon(bool expanded)
{
	QPixmap pm(30, 30);
	pm.fill(Qt::transparent);
	QPainter p(&pm);
	p.setRenderHint(QPainter::Antialiasing, true);
	QPen pen(QColor("#f6eef3"));
	pen.setWidth(2);
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	p.setPen(pen);
	const int a = 6, b = 24; // esquinas del recuadro
	const int L = 5;          // longitud de las patas
	// Cuatro esquinas, cada una con dos segmentos en "L".
	if (!expanded) {
		// Flechas hacia FUERA: esquinas apuntando a los bordes.
		p.drawLine(a, a + L, a, a); p.drawLine(a, a, a + L, a);          // sup-izq
		p.drawLine(b - L, a, b, a); p.drawLine(b, a, b, a + L);          // sup-der
		p.drawLine(a, b - L, a, b); p.drawLine(a, b, a + L, b);          // inf-izq
		p.drawLine(b - L, b, b, b); p.drawLine(b, b, b, b - L);          // inf-der
	} else {
		// Flechas hacia DENTRO: esquinas apuntando al centro.
		p.drawLine(a, a + L, a + L, a + L); p.drawLine(a + L, a + L, a + L, a);
		p.drawLine(b, a + L, b - L, a + L); p.drawLine(b - L, a + L, b - L, a);
		p.drawLine(a, b - L, a + L, b - L); p.drawLine(a + L, b - L, a + L, b);
		p.drawLine(b, b - L, b - L, b - L); p.drawLine(b - L, b - L, b - L, b);
	}
	p.end();
	return QIcon(pm);
}

// Icono de papelera dibujado con QPainter (color rojo err), para eliminar opción.
QIcon BetterChatDock::makeTrashIcon()
{
	QPixmap pm(28, 28);
	pm.fill(Qt::transparent);
	QPainter p(&pm);
	p.setRenderHint(QPainter::Antialiasing, true);
	QPen pen(QColor("#ff5d6c"));
	pen.setWidth(2);
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	p.setPen(pen);
	// Tapa + asa.
	p.drawLine(7, 9, 21, 9);          // borde de la tapa
	p.drawLine(11, 9, 12, 6);         // asa izq
	p.drawLine(12, 6, 16, 6);         // asa top
	p.drawLine(16, 6, 17, 9);         // asa der
	// Cubo (trapecio) + 2 rayas verticales.
	p.drawLine(9, 9, 10, 22);
	p.drawLine(19, 9, 18, 22);
	p.drawLine(10, 22, 18, 22);
	p.drawLine(12, 12, 12, 19);
	p.drawLine(16, 12, 16, 19);
	p.end();
	return QIcon(pm);
}

// Icono de gráfico de barras (Ver resumen), QPainter.
QIcon BetterChatDock::makeChartIcon()
{
	QPixmap pm(28, 28);
	pm.fill(Qt::transparent);
	QPainter p(&pm);
	p.setRenderHint(QPainter::Antialiasing, true);
	QPen pen(QColor("#f6eef3"));
	pen.setWidth(2);
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	p.setPen(pen);
	// Ejes.
	p.drawLine(6, 5, 6, 22);
	p.drawLine(6, 22, 23, 22);
	// Barras.
	p.drawLine(11, 22, 11, 15);
	p.drawLine(16, 22, 16, 10);
	p.drawLine(21, 22, 21, 17);
	p.end();
	return QIcon(pm);
}

// Icono de copiar (dos hojas superpuestas), QPainter.
QIcon BetterChatDock::makeCopyIcon()
{
	QPixmap pm(28, 28);
	pm.fill(Qt::transparent);
	QPainter p(&pm);
	p.setRenderHint(QPainter::Antialiasing, true);
	QPen pen(QColor("#f6eef3"));
	pen.setWidth(2);
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	p.setPen(pen);
	// Hoja delantera.
	p.drawRoundedRect(11, 11, 11, 12, 2, 2);
	// Hoja trasera (parte visible en forma de L).
	p.drawLine(8, 17, 6, 17);
	p.drawLine(6, 17, 6, 6);
	p.drawLine(6, 6, 16, 6);
	p.drawLine(16, 6, 16, 8);
	p.end();
	return QIcon(pm);
}

// Coloca el botón de fullscreen en la esquina superior derecha del área del chat.
void BetterChatDock::positionFsButton()
{
	if (!m_fsBtn || !m_multiChat)
		return;
	const int m = 6;
	m_fsBtn->move(m_multiChat->viewport()->width() - m_fsBtn->width() - m, m);
	m_fsBtn->raise();
}

// Alterna el modo pantalla completa del multichat: oculta/restaura todo el chrome
// del dock (título, nav bar, chips, zona YT, versión) dejando solo el chat.
void BetterChatDock::toggleChatFullscreen()
{
	m_chatFullscreen = !m_chatFullscreen;
	const bool fs = m_chatFullscreen;
	// Ocultar/mostrar todo lo que NO es el chat.
	if (m_brandTitle) m_brandTitle->setVisible(!fs);
	if (m_topRow) m_topRow->setVisible(!fs);
	if (m_navBar) m_navBar->setVisible(!fs);
	if (m_verLabel) m_verLabel->setVisible(!fs);
	if (m_platBar) m_platBar->setVisible(!fs);
	if (m_multiStatus) m_multiStatus->setVisible(!fs);
	if (m_ytZone) m_ytZone->setVisible(!fs);
	if (auto *lay = layout())
		lay->setContentsMargins(fs ? QMargins(0, 0, 0, 0) : QMargins(12, 12, 12, 12));
	if (m_fsBtn) {
		m_fsBtn->setIcon(makeExpandIcon(fs));
		m_fsBtn->setToolTip(fs ? QStringLiteral("Salir de pantalla completa")
				       : QStringLiteral("Ver el chat a pantalla completa"));
	}
	positionFsButton();
	// El layout recalcula el tamaño del chat DESPUÉS de este ciclo; reposicionar
	// el botón en el siguiente tick para que no quede descolocado al restaurar.
	QTimer::singleShot(0, this, [this]() { positionFsButton(); });
}

// Reposiciona el botón de fullscreen cuando el área del chat cambia de tamaño.
bool BetterChatDock::eventFilter(QObject *obj, QEvent *ev)
{
	if (obj == m_multiChat && (ev->type() == QEvent::Resize || ev->type() == QEvent::Show))
		positionFsButton();
	// Clic en una tarjeta de la galería de minijuegos -> elegir ese tipo.
	if (ev->type() == QEvent::MouseButtonRelease) {
		QWidget *w = qobject_cast<QWidget *>(obj);
		if (w) {
			const QVariant gt = w->property("gameType");
			if (gt.isValid()) {
				m_pickedType = gt.toString();
				if (m_betMsg)
					m_betMsg->clear();
				updateBetView();
				return true;
			}
		}
	}
	return QWidget::eventFilter(obj, ev);
}

// Abre/cierra el selector de directos de YouTube; al abrir, pide la lista.
void BetterChatDock::toggleYouTubePicker()
{
	if (!m_ytPicker)
		return;
	const bool show = !m_ytPicker->isVisible();
	m_ytPicker->setVisible(show);
	if (show) {
		m_ytPickBtn->setText(QStringLiteral("Ocultar directos"));
		// Limpiar la lista previa y pedir de nuevo.
		while (QLayoutItem *it = m_ytList->takeAt(0)) {
			if (it->widget())
				it->widget()->deleteLater();
			delete it;
		}
		if (m_ytPickerMsg)
			m_ytPickerMsg->setText(QStringLiteral("Buscando tus directos..."));
		m_api->fetchYouTubeLive();
	} else {
		updateYouTubeHelp(); // restaura el texto del botón según el plan
	}
}

// Pinta la lista de directos activos de YouTube, con un botón "Usar" por cada uno.
void BetterChatDock::onYouTubeLiveList(const QByteArray &json)
{
	if (!m_ytList)
		return;
	while (QLayoutItem *it = m_ytList->takeAt(0)) {
		if (it->widget())
			it->widget()->deleteLater();
		delete it;
	}
	QJsonObject obj = QJsonDocument::fromJson(json).object();
	QJsonArray streams = obj.value(QStringLiteral("streams")).toArray();
	if (streams.isEmpty()) {
		if (m_ytPickerMsg)
			m_ytPickerMsg->setText(QStringLiteral("No tienes directos activos ahora mismo."));
		return;
	}
	if (m_ytPickerMsg)
		m_ytPickerMsg->setText(QStringLiteral("Elige el directo cuyo chat quieres leer:"));
	static const QHash<QString, QString> privLabel = {
		{QStringLiteral("public"), QStringLiteral("Público")},
		{QStringLiteral("unlisted"), QStringLiteral("No listado")},
		{QStringLiteral("private"), QStringLiteral("Privado")},
	};
	for (const QJsonValue &v : streams) {
		QJsonObject s = v.toObject();
		const QString videoId = s.value(QStringLiteral("videoId")).toString();
		QString title = s.value(QStringLiteral("title")).toString();
		const QString privacy = s.value(QStringLiteral("privacy")).toString();
		if (title.isEmpty())
			title = QStringLiteral("(sin título)");

		auto *row = new QWidget(m_ytPicker);
		auto *rh = new QHBoxLayout(row);
		rh->setContentsMargins(0, 0, 0, 0);
		rh->setSpacing(8);
		auto *lbl = new QLabel(row);
		lbl->setTextFormat(Qt::RichText);
		lbl->setText(QStringLiteral("<b>%1</b> <span style='color:#b3a1ac; font-size:11px;'>%2</span>")
				     .arg(title.toHtmlEscaped(), privLabel.value(privacy, privacy)));
		lbl->setWordWrap(true);
		rh->addWidget(lbl, 1);
		auto *use = new QPushButton(QStringLiteral("Usar"), row);
		use->setObjectName(QStringLiteral("ytUseBtn"));
		use->setCursor(Qt::PointingHandCursor);
		const QString url = QStringLiteral("https://www.youtube.com/watch?v=") + videoId;
		connect(use, &QPushButton::clicked, this, [this, url]() {
			if (m_ytPickerMsg)
				m_ytPickerMsg->setText(QStringLiteral("Conectando..."));
			m_api->setYouTubeSource(url);
		});
		rh->addWidget(use, 0);
		m_ytList->addWidget(row);
	}
}

void BetterChatDock::onYouTubeLiveError(const QString &message)
{
	if (m_ytPickerMsg)
		m_ytPickerMsg->setText(message);
}

// Ajusta la ayuda y el botón de YouTube: la auto-conexión es GRATIS para todos
// (solo depende de tener YouTube vinculado). Vinculado -> se conecta solo al
// transmitir; sin vincular -> se invita a vincular (o elegir el directo a mano).
void BetterChatDock::updateYouTubeHelp()
{
	if (!m_ytHelp || !m_ytPickBtn)
		return;
	if (m_api->youtubeLinked()) {
		m_ytHelp->setText(QStringLiteral(
			"Tu directo de YouTube se conecta solo al iniciar la "
			"transmisión. No tienes que hacer nada."));
		m_ytPickBtn->setText(QStringLiteral("Elegir otro directo"));
	} else {
		m_ytHelp->setText(QStringLiteral(
			"Vincula tu canal de YouTube en Conexiones para que tu directo "
			"se conecte solo al transmitir. También puedes elegirlo a mano."));
		m_ytPickBtn->setText(QStringLiteral("Elegir directo de YouTube"));
	}
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
		dot = QStringLiteral("#3ad07a"); label = QStringLiteral("en vivo");
	} else if (state == QStringLiteral("connecting")) {
		dot = QStringLiteral("#ffb347"); label = QStringLiteral("...");
	} else if (state == QStringLiteral("searching")) {
		dot = QStringLiteral("#ffb347"); label = QStringLiteral("buscando");
	} else if (state == QStringLiteral("error")) {
		dot = QStringLiteral("#ff5d6c"); label = QStringLiteral("error");
	} else if (state == QStringLiteral("disconnected")) {
		dot = QStringLiteral("#ff5d6c"); label = QStringLiteral("sin directo");
	} else { // off (no configurada)
		dot = QStringLiteral("#6b5a63"); label = QStringLiteral("off");
	}

	const QString plat = platform.toLower();
	// Indicador "Buscando tu directo..." junto al botón de elegir directo de YouTube:
	// visible solo mientras la auto-conexión de YouTube está buscando.
	if (plat == QStringLiteral("youtube") && m_ytSearching)
		m_ytSearching->setVisible(state == QStringLiteral("searching"));
	QString icon = m_platIconB64.contains(plat)
		? QStringLiteral("<img src='data:image/png;base64,%1' width='13' height='13'>")
			  .arg(m_platIconB64.value(plat))
		: plat.toHtmlEscaped();

	// Chip: [logo] •estado, con punto de color según el estado.
	chip->setText(QStringLiteral(
			      "%1 <span style='color:%2;'>&#9679;</span> "
			      "<span style='color:#b3a1ac; font-size:11px;'>%3</span>")
			      .arg(icon, dot, label));
	chip->setStyleSheet(state == QStringLiteral("off")
				    ? QStringLiteral("QLabel#platChip { background:#1e141b; border:1px solid #291b24; "
						     "border-radius:6px; padding:3px 7px; }")
				    : QStringLiteral("QLabel#platChip { background:#291b24; border:1px solid #3a2833; "
						     "border-radius:6px; padding:3px 7px; }"));
}

// Iconos oficiales de cada plataforma, pre-rasterizados a PNG (con el renderer SVG
// de Qt en tiempo de build) y embebidos en base64. En runtime solo se carga el PNG
// (QImage), sin depender del modulo SVG de Qt que puede faltar en OBS Windows.
void BetterChatDock::registerPlatformIcons()
{
	if (m_iconsReady)
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
		const QString key = QString::fromLatin1(pl.key);
		m_platIconB64.insert(key, QString::fromLatin1(pl.png)); // para data URI en QLabel
		if (m_multiChat) {
			QImage img;
			img.loadFromData(QByteArray::fromBase64(QByteArray(pl.png)), "PNG");
			m_multiChat->document()->addResource(QTextDocument::ImageResource,
							     QUrl(QStringLiteral("bcplat://") + key),
							     QVariant(img));
		}
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
		icon = QStringLiteral("<span style='background:#3a2833; color:#f6eef3; font-size:10px; "
				      "padding:1px 5px; border-radius:4px;'>%1</span>")
			       .arg(label);
	}

	const QString html =
		QStringLiteral("%1 <b style='color:%2'>%3:</b> "
			       "<span style='color:#f6eef3'>%4</span>")
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
	QString accent = QStringLiteral("#ff7ec8");
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
		accent = QStringLiteral("#3ad9d9"); glyph = QStringLiteral("SUB");
		desc = QStringLiteral("%1 se suscribió%2").arg(A,
			unit.isEmpty() ? QString() : QStringLiteral(" (%1)").arg(unit.toHtmlEscaped()));
	} else if (kind == QStringLiteral("resub")) {
		accent = QStringLiteral("#3ad9d9"); glyph = QStringLiteral("RESUB");
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
			"<span style='color:#f6eef3;'>%5</span>"
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
	// Arrancar el poll de apuestas y pedir el estado ya.
	if (m_betPollTimer)
		m_betPollTimer->start();
	refreshBets();
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
	if (m_betPollTimer)
		m_betPollTimer->stop();
	m_loginBtn->setEnabled(true);
	m_loginStatus->clear();
	m_pairInfo->setVisible(false);
}

void BetterChatDock::onStatusUpdated()
{
	QString name = m_api->username();
	// El nombre de usuario se muestra con "@" al inicio (como en la web).
	m_userLabel->setText(name.isEmpty()
		? QStringLiteral("Tu cuenta")
		: QStringLiteral("@%1").arg(name.startsWith(QLatin1Char('@')) ? name.mid(1) : name));
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

	// Ayuda de YouTube según el plan (plus = auto / standard = manual).
	updateYouTubeHelp();

	// Logo: "BetterChatTV+" (con + rosa) si el dueño tiene el plan plus.
	if (m_brandTitle) {
		m_brandTitle->setTextFormat(Qt::RichText);
		m_brandTitle->setText(m_api->isPlus()
			? QStringLiteral("BetterChatTV<span style='color:#ff7ec8;'>+</span>")
			: QStringLiteral("BetterChatTV"));
	}

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
			QPen pen(QColor("#b3a1ac"));
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

// Notifica al servidor el cambio de estado de transmisión de OBS.
void BetterChatDock::onStreamingChanged(bool active)
{
	if (m_api)
		m_api->setStreaming(active);
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
	// Estilizar el contenedor del dock (barra de título + marco) con la paleta de
	// la web: por defecto OBS lo pinta gris. El widget interno ya es negro-vino.
	dockWidget->setStyleSheet(QStringLiteral(
		"QDockWidget { background: #120b10; color: #f6eef3; }"
		"QDockWidget::title { background: #1e141b; color: #f6eef3;"
		" padding: 6px 8px; text-align: left; border-bottom: 1px solid #3a2833; }"));
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

	// Escucha los eventos de TRANSMISIÓN de OBS: es la señal más fiable de "estoy
	// en directo". Al empezar/parar de transmitir, avisamos al servidor (que dispara
	// la auto-conexión de YouTube). Puntero al dock capturado por el callback C.
	static BetterChatDock *s_dock = dock;
	obs_frontend_add_event_callback(
		[](enum obs_frontend_event event, void *) {
			if (!s_dock)
				return;
			if (event == OBS_FRONTEND_EVENT_STREAMING_STARTED)
				QMetaObject::invokeMethod(s_dock, [] { s_dock->onStreamingChanged(true); });
			else if (event == OBS_FRONTEND_EVENT_STREAMING_STOPPED)
				QMetaObject::invokeMethod(s_dock, [] { s_dock->onStreamingChanged(false); });
		},
		nullptr);
}
