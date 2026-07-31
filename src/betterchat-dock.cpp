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
QListWidget#chatList::item { padding: 7px 8px; border-radius: 6px; }
QListWidget#chatList::item:selected { background: #ff4d8d; color: #1a0c12; }
QPushButton#ghost:disabled { color: #6b5a53; border-color: #2b1f1b; }
)CSS";

BetterChatDock::BetterChatDock(QWidget *parent) : QWidget(parent)
{
	m_api = new BetterChatApi(this);
	buildUi();

	connect(m_api, &BetterChatApi::pairingStarted, this, &BetterChatDock::onPairingStarted);
	connect(m_api, &BetterChatApi::pairingFailed, this, &BetterChatDock::onPairingFailed);
	connect(m_api, &BetterChatApi::loggedIn, this, &BetterChatDock::onLoggedIn);
	connect(m_api, &BetterChatApi::loggedOut, this, &BetterChatDock::onLoggedOut);
	connect(m_api, &BetterChatApi::statusUpdated, this, &BetterChatDock::onStatusUpdated);

	if (m_api->isLoggedIn()) {
		m_stack->setCurrentIndex(1);
		m_api->startStatusPolling();
		refreshChatList();
	} else {
		m_stack->setCurrentIndex(0);
	}
}

BetterChatDock::~BetterChatDock() = default;

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
		auto *v = new QVBoxLayout(page);
		v->setContentsMargins(0, 0, 0, 0);
		v->setSpacing(10);

		auto *row = new QHBoxLayout();
		m_userLabel = new QLabel(page);
		m_userLabel->setObjectName(QStringLiteral("title"));
		row->addWidget(m_userLabel, 1);
		m_liveBadge = new QLabel(page);
		m_liveBadge->setObjectName(QStringLiteral("liveOff"));
		m_liveBadge->setText(QStringLiteral("Sin directo"));
		row->addWidget(m_liveBadge, 0, Qt::AlignRight);
		v->addLayout(row);

		auto *listLabel = new QLabel(QStringLiteral("Tus chats en OBS"), page);
		listLabel->setObjectName(QStringLiteral("muted"));
		v->addWidget(listLabel);

		// Lista de instancias de chat existentes (cada una su propio tamaño).
		m_chatList = new QListWidget(page);
		m_chatList->setObjectName(QStringLiteral("chatList"));
		connect(m_chatList, &QListWidget::itemSelectionChanged, this, [this]() {
			bool sel = m_chatList->currentItem() != nullptr;
			m_addSelBtn->setEnabled(sel);
			m_removeSelBtn->setEnabled(sel);
		});
		v->addWidget(m_chatList, 1);

		// Acciones sobre la seleccionada: añadirla a la escena actual o quitarla.
		auto *selRow = new QHBoxLayout();
		m_addSelBtn = new QPushButton(QStringLiteral("Añadir a esta escena"), page);
		m_addSelBtn->setObjectName(QStringLiteral("ghost"));
		m_addSelBtn->setEnabled(false);
		connect(m_addSelBtn, &QPushButton::clicked, this, &BetterChatDock::onAddSelectedToScene);
		selRow->addWidget(m_addSelBtn, 1);
		m_removeSelBtn = new QPushButton(QStringLiteral("Quitar"), page);
		m_removeSelBtn->setObjectName(QStringLiteral("ghost"));
		m_removeSelBtn->setEnabled(false);
		connect(m_removeSelBtn, &QPushButton::clicked, this, &BetterChatDock::onRemoveSelected);
		selRow->addWidget(m_removeSelBtn, 0);
		v->addLayout(selRow);

		// Crear una instancia NUEVA e independiente en la escena actual.
		m_createBtn = new QPushButton(QStringLiteral("Crear chat nuevo en esta escena"), page);
		m_createBtn->setObjectName(QStringLiteral("primary"));
		connect(m_createBtn, &QPushButton::clicked, this, &BetterChatDock::onCreateChat);
		v->addWidget(m_createBtn);

		m_actionStatus = new QLabel(page);
		m_actionStatus->setObjectName(QStringLiteral("muted"));
		m_actionStatus->setWordWrap(true);
		v->addWidget(m_actionStatus);

		m_logoutBtn = new QPushButton(QStringLiteral("Cerrar sesión"), page);
		m_logoutBtn->setObjectName(QStringLiteral("ghost"));
		connect(m_logoutBtn, &QPushButton::clicked, this, &BetterChatDock::onLogout);
		v->addWidget(m_logoutBtn);

		m_stack->addWidget(page);
	}
}

// ---- Login ----

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
	m_api->startStatusPolling();
	refreshChatList();
}

void BetterChatDock::onLoggedOut()
{
	m_stack->setCurrentIndex(0);
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
		m_liveBadge->setText(plat.isEmpty() ? QStringLiteral("EN DIRECTO")
						    : QStringLiteral("EN DIRECTO · %1").arg(plat));
	} else {
		m_liveBadge->setObjectName(QStringLiteral("liveOff"));
		m_liveBadge->setText(QStringLiteral("Sin directo"));
	}
	// Re-aplicar el estilo tras cambiar objectName.
	m_liveBadge->setStyleSheet(QString::fromUtf8(kStyle));

	// Aprovechar el sondeo periodico para refrescar tamaños de la lista (el
	// auto-resize los cambia al arrastrar en OBS).
	refreshChatList();
}

// ---- Lista de instancias de chat ----

namespace {
// Marca propia en los settings para reconocer NUESTRAS fuentes de chat, sin
// depender de la URL (que puede variar). Las creadas por el plugin la llevan.
constexpr const char *kChatFlag = "betterchat_instance";

// Callback de enumeración: recoge nombre + tamaño de cada fuente de chat nuestra.
struct ChatInfo {
	QString name;
	int width;
	int height;
};

bool collectChatSources(void *param, obs_source_t *source)
{
	auto *out = static_cast<QList<ChatInfo> *>(param);
	const char *id = obs_source_get_id(source);
	if (!id || QString(id) != QStringLiteral("browser_source"))
		return true;
	obs_data_t *settings = obs_source_get_settings(source);
	bool mine = obs_data_get_bool(settings, kChatFlag);
	if (mine) {
		ChatInfo info;
		info.name = QString::fromUtf8(obs_source_get_name(source));
		info.width = (int)obs_data_get_int(settings, "width");
		info.height = (int)obs_data_get_int(settings, "height");
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
		prevSelected = m_chatList->currentItem()->text();

	m_chatList->clear();
	QList<ChatInfo> chats;
	obs_enum_sources(collectChatSources, &chats);
	for (const auto &c : chats) {
		QString label = QStringLiteral("%1  (%2×%3)").arg(c.name).arg(c.width).arg(c.height);
		auto *item = new QListWidgetItem(label, m_chatList);
		item->setData(Qt::UserRole, c.name); // nombre real de la fuente
		if (label == prevSelected)
			m_chatList->setCurrentItem(item);
	}
	if (chats.isEmpty())
		m_actionStatus->setText(QStringLiteral(
			"Aún no tienes ningún chat. Crea uno con el botón de abajo."));
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

// ---- Quitar la instancia seleccionada ----

void BetterChatDock::onRemoveSelected()
{
	auto *item = m_chatList->currentItem();
	if (!item)
		return;
	QString name = item->data(Qt::UserRole).toString();
	obs_source_t *source = obs_get_source_by_name(name.toUtf8().constData());
	if (source) {
		obs_source_remove(source); // lo saca de todas las escenas
		obs_source_release(source);
		m_actionStatus->setText(QStringLiteral("\"%1\" eliminado.").arg(name));
	}
	refreshChatList();
}

void BetterChatDock::onLogout()
{
	m_api->logout();
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
