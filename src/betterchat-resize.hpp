/*
BetterChatTV para OBS
Copyright (C) 2026 DEValen

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#pragma once

// Arranca/para el vigilante de redimensionado. Convierte la escala (zoom) de las
// fuentes de navegador de BetterChatTV en resolucion real al soltar el arrastre,
// igual que hacia el script Lua betterchattv-resize.lua. Puente C para llamarlo
// desde plugin-main.c (obs_module_post_load / obs_module_unload).
#ifdef __cplusplus
extern "C" {
#endif

void betterchat_start_resize_watcher(void);
void betterchat_stop_resize_watcher(void);

#ifdef __cplusplus
}
#endif
