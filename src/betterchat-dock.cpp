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
			QStringLiteral("Vincula tu cuenta para añadir el chat a OBS y ver si estás en directo."),
			page);
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

		m_addSourceBtn = new QPushButton(QStringLiteral("Añadir chat a la escena"), page);
		m_addSourceBtn->setObjectName(QStringLiteral("primary"));
		connect(m_addSourceBtn, &QPushButton::clicked, this, &BetterChatDock::onAddChatSource);
		v->addWidget(m_addSourceBtn);

		m_actionStatus = new QLabel(page);
		m_actionStatus->setObjectName(QStringLiteral("muted"));
		m_actionStatus->setWordWrap(true);
		v->addWidget(m_actionStatus);

		v->addStretch(1);

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
}

// ---- Crear la fuente del chat en la escena activa ----

void BetterChatDock::onAddChatSource()
{
	QString url = m_api->overlayUrl();
	if (url.isEmpty()) {
		m_actionStatus->setText(QStringLiteral("Aún no tengo tu URL de overlay. Prueba de nuevo en unos segundos."));
		m_api->refreshStatus();
		return;
	}
	addBrowserSourceToCurrentScene(url);
}

void BetterChatDock::addBrowserSourceToCurrentScene(const QString &url)
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

	// Ajustes del browser source: URL del overlay + tamaño 1920x1080 (OBS canvas).
	obs_data_t *settings = obs_data_create();
	obs_data_set_string(settings, "url", url.toUtf8().constData());
	obs_data_set_int(settings, "width", 1920);
	obs_data_set_int(settings, "height", 1080);
	// Refrescar al activar la escena, para que reconecte al chat.
	obs_data_set_bool(settings, "reroute_audio", false);
	obs_data_set_bool(settings, "restart_when_active", true);

	const char *sourceName = "BetterChatTV Chat";
	obs_source_t *source = obs_source_create("browser_source", sourceName, settings, nullptr);
	obs_data_release(settings);

	if (!source) {
		obs_source_release(sceneSource);
		m_actionStatus->setText(QStringLiteral("No se pudo crear la fuente de navegador. ¿Tienes el navegador integrado de OBS?"));
		return;
	}

	obs_scene_add(scene, source);
	obs_source_release(source);
	obs_source_release(sceneSource);

	m_actionStatus->setText(QStringLiteral("Listo: se añadió \"%1\" a tu escena actual.").arg(sourceName));
	obs_log(LOG_INFO, "[betterchat] browser source added to current scene");
}

void BetterChatDock::onLogout()
{
	m_api->logout();
}

// ---- Registro del dock en OBS (puente C) ----
// Se llama desde obs_module_post_load(). Crea el widget y lo añade como dock
// acoplable usando la API del frontend de OBS.
extern "C" void betterchat_register_dock(void)
{
	auto *dock = new BetterChatDock();
	dock->setMinimumWidth(240);
	// id estable + título visible. La API moderna (OBS 30+) acopla el widget
	// y OBS gestiona su QDockWidget contenedor.
	obs_frontend_add_dock_by_id("betterchat-dock", "BetterChatTV", dock);
}
