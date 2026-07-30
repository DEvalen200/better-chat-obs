/*
BetterChatTV para OBS
Copyright (C) 2026 DEValen

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

// Definida en betterchat-dock.cpp: crea el widget y lo registra como dock.
extern void betterchat_register_dock(void);
// Definidas en betterchat-resize.cpp: vigilante de redimensionado (escala -> resolucion).
extern void betterchat_start_resize_watcher(void);
extern void betterchat_stop_resize_watcher(void);

bool obs_module_load(void)
{
	obs_log(LOG_INFO, "BetterChatTV plugin cargado (version %s)", PLUGIN_VERSION);
	return true;
}

// El dock se registra cuando el frontend de OBS ya está listo.
void obs_module_post_load(void)
{
	betterchat_register_dock();
	betterchat_start_resize_watcher();
}

void obs_module_unload(void)
{
	betterchat_stop_resize_watcher();
	obs_log(LOG_INFO, "BetterChatTV plugin descargado");
}
