/*
BetterChatTV para OBS
Copyright (C) 2026 DEValen

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

// Port del script Lua betterchattv-resize.lua al plugin nativo.
//
// En OBS, arrastrar las esquinas de una fuente de navegador solo cambia su
// ESCALA (zoom); la resolucion de la pagina sigue siendo la de los ajustes.
// Este vigilante revisa cada 250 ms las fuentes de navegador del overlay de
// BetterChatTV y, cuando la escala queda estable y distinta de 1 (has soltado
// el arraston), la convierte en resolucion real (ancho x alto) y devuelve la
// escala a 1. El tamano en pantalla no cambia: ahora es el tamano real de la
// pagina y el chat se recoloca en vez de verse ampliado.

#include "betterchat-resize.hpp"

#include <obs.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>

#include <QObject>
#include <QTimer>
#include <QString>

#include <map>
#include <string>
#include <cmath>

namespace {

// Igual que en el script: filtro de URL, tolerancias y ticks estables.
constexpr const char *kUrlFilter = "betterchat";
constexpr int kStableTicks = 3; // lecturas iguales (a 250 ms) antes de convertir
constexpr double kEps = 0.01;

struct TrackState {
	float sx;
	float sy;
	int ticks;
};

// Estado por elemento de escena: "escena#idItem" -> { sx, sy, ticks }.
std::map<std::string, TrackState> g_tracked;

// Procesa un sceneitem: replica process_item() del Lua.
bool enumItem(obs_scene_t *, obs_sceneitem_t *item, void *param)
{
	const char *sceneName = static_cast<const char *>(param);

	obs_source_t *source = obs_sceneitem_get_source(item);
	if (!source)
		return true;

	const char *unversioned = obs_source_get_unversioned_id(source);
	if (!unversioned || std::string(unversioned) != "browser_source")
		return true;

	obs_data_t *settings = obs_source_get_settings(source);
	std::string url = obs_data_get_string(settings, "url") ? obs_data_get_string(settings, "url") : "";
	bool isLocal = obs_data_get_bool(settings, "is_local_file");

	std::string urlLower = url;
	for (auto &c : urlLower)
		c = (char)tolower((unsigned char)c);

	if (isLocal || urlLower.find(kUrlFilter) == std::string::npos) {
		obs_data_release(settings);
		return true;
	}

	// Solo transformaciones simples: con bounding box la escala no representa
	// el tamano y no debemos tocar nada.
	if (obs_sceneitem_get_bounds_type(item) != OBS_BOUNDS_NONE) {
		obs_data_release(settings);
		return true;
	}

	vec2 scale;
	obs_sceneitem_get_scale(item, &scale);

	std::string key = std::string(sceneName ? sceneName : "") + "#" +
			  std::to_string(obs_sceneitem_get_id(item));

	// Escala 1 (o volteada): nada que hacer.
	if (scale.x <= 0 || scale.y <= 0 ||
	    (std::fabs(scale.x - 1) < kEps && std::fabs(scale.y - 1) < kEps)) {
		g_tracked.erase(key);
		obs_data_release(settings);
		return true;
	}

	auto it = g_tracked.find(key);
	if (it != g_tracked.end() && std::fabs(it->second.sx - scale.x) < 1e-6 &&
	    std::fabs(it->second.sy - scale.y) < 1e-6) {
		it->second.ticks += 1;
	} else {
		g_tracked[key] = TrackState{scale.x, scale.y, 1};
		it = g_tracked.find(key);
	}

	if (it->second.ticks >= kStableTicks) {
		int w = (int)obs_data_get_int(settings, "width");
		int h = (int)obs_data_get_int(settings, "height");
		int nw = std::max(1, (int)std::floor(w * scale.x + 0.5));
		int nh = std::max(1, (int)std::floor(h * scale.y + 0.5));
		obs_data_set_int(settings, "width", nw);
		obs_data_set_int(settings, "height", nh);
		obs_source_update(source, settings);

		vec2 one;
		one.x = 1.0f;
		one.y = 1.0f;
		obs_sceneitem_set_scale(item, &one);

		g_tracked.erase(key);
		obs_log(LOG_INFO, "[betterchat] '%s': resolucion ajustada a %dx%d",
			obs_source_get_name(source), nw, nh);
	}

	obs_data_release(settings);
	return true;
}

// Un tick: replica tick() del Lua, recorriendo todas las escenas.
void resizeTick()
{
	struct obs_frontend_source_list scenes = {};
	obs_frontend_get_scenes(&scenes);
	for (size_t i = 0; i < scenes.sources.num; i++) {
		obs_source_t *sceneSrc = scenes.sources.array[i];
		obs_scene_t *scene = obs_scene_from_source(sceneSrc);
		if (!scene)
			continue;
		const char *sceneName = obs_source_get_name(sceneSrc);
		obs_scene_enum_items(scene, enumItem, (void *)sceneName);
	}
	obs_frontend_source_list_free(&scenes);
}

// Objeto duenno del QTimer. Vive mientras el vigilante esta activo.
class ResizeWatcher : public QObject {
public:
	ResizeWatcher()
	{
		connect(&m_timer, &QTimer::timeout, this, []() { resizeTick(); });
		m_timer.start(250);
	}
	~ResizeWatcher() override { m_timer.stop(); }

private:
	QTimer m_timer;
};

ResizeWatcher *g_watcher = nullptr;

} // namespace

extern "C" void betterchat_start_resize_watcher(void)
{
	if (!g_watcher)
		g_watcher = new ResizeWatcher();
}

extern "C" void betterchat_stop_resize_watcher(void)
{
	delete g_watcher;
	g_watcher = nullptr;
	g_tracked.clear();
}
