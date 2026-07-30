# BetterChatTV para OBS

Plugin nativo de OBS Studio para [BetterChatTV](https://betterchat.tv). Añade un
panel acoplable (dock) dentro de OBS con tres funciones:

1. **Vincular tu cuenta** sin copiar URLs ni tokens a mano (login por navegador,
   estilo *device-flow*: OBS te da un código, lo confirmas en la web ya logueado).
2. **Añadir el chat a la escena** con un botón: crea la fuente de navegador
   (Browser Source) apuntando a tu overlay, ya configurada.
3. **Saber si estás en directo**: un indicador que consulta el estado de tu
   sala (Twitch / YouTube / Kick / TikTok) y se pone verde cuando emites.

## Cómo funciona el login (device-flow)

No compartimos la cookie del navegador con la app nativa (no se puede de forma
segura). En su lugar:

```
OBS: POST /api/plugin/pair/start      -> { deviceCode, userCode, verifyUrl }
OBS: abre verifyUrl en el navegador   (el streamer ya tiene sesión y aprueba)
OBS: POST /api/plugin/pair/token      -> poll cada 3s hasta { status: "approved", token }
OBS: guarda el token (QSettings) y lo usa como  Authorization: Bearer <token>
OBS: GET  /api/plugin/status          -> { username, overlayUrl, live, platform }
```

El token es de larga vida y se puede revocar desde el propio plugin (Cerrar
sesión) o borrando la fila en `plugin_tokens`.

## Estructura

```
src/plugin-main.c        Entrada del módulo; registra el dock en post_load.
src/betterchat-api.*     Cliente HTTP (Qt Network): pairing, poll, status.
src/betterchat-dock.*    Dock Qt: login, botón de fuente, indicador de directo.
buildspec.json           Versiones de OBS/Qt/deps que descarga el CI.
.github/workflows/       CI multiplataforma (Windows, macOS, Linux).
```

## Compilar

El proyecto usa el sistema de build de la plantilla oficial de OBS. La forma
recomendada es dejar que **GitHub Actions** compile los tres sistemas al hacer
push (ver `.github/workflows/push.yaml`).

Para compilar en local necesitas las dependencias de OBS (libobs + obs-frontend-api
+ Qt6). En Linux con OBS instalado del sistema:

```bash
cmake --preset ubuntu-x86_64
cmake --build --preset ubuntu-x86_64
```

## Instalar (sin firma, cero fricción)

El plugin se distribuye como carpeta/zip que se copia a la carpeta de plugins de
OBS. No requiere firma de código: OBS carga plugins sin firmar y, al copiarse (no
descargarse como instalador), Windows no muestra avisos.

## Configuración para desarrollo

Por defecto apunta a `https://betterchat.tv`. Para pruebas contra otro entorno:

```bash
export BETTERCHAT_BASE_URL="http://localhost:3000"
```

## Licencia

GPL-2.0-or-later (igual que la plantilla de OBS).
