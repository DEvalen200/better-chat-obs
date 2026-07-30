; Instalador de BetterChatTV para OBS (NSIS)
; Detecta la carpeta de OBS Studio e instala el plugin en las rutas correctas.

Unicode true
!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "x64.nsh"

!define PLUGIN_NAME "BetterChatTV para OBS"
!define PLUGIN_ID   "betterchat-obs"
!define PLUGIN_VER  "0.1.0"
!define PUBLISHER   "BetterChatTV"
!define WEBSITE     "https://betterchat.tv"

Name "${PLUGIN_NAME} ${PLUGIN_VER}"
OutFile "betterchat-obs-${PLUGIN_VER}-installer.exe"
RequestExecutionLevel admin
ShowInstDetails show
ShowUnInstDetails show

; Carpeta de instalacion por defecto: se resuelve en .onInit desde el registro.
InstallDir "$PROGRAMFILES64\obs-studio"

!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define MUI_ABORTWARNING

; ---- Paginas ----
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_LINK "Abrir betterchat.tv"
!define MUI_FINISHPAGE_LINK_LOCATION "${WEBSITE}"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "Spanish"

; ---- Detectar OBS en el registro ----
Function .onInit
  ${IfNot} ${RunningX64}
    MessageBox MB_OK|MB_ICONSTOP "Este plugin requiere OBS de 64 bits."
    Abort
  ${EndIf}
  SetRegView 64

  ; OBS guarda su ruta de instalacion aqui.
  ReadRegStr $0 HKLM "SOFTWARE\OBS Studio" ""
  ${If} $0 != ""
    ${If} ${FileExists} "$0\bin\64bit\obs64.exe"
      StrCpy $INSTDIR "$0"
      Return
    ${EndIf}
  ${EndIf}

  ; Fallback: rutas habituales.
  ${If} ${FileExists} "$PROGRAMFILES64\obs-studio\bin\64bit\obs64.exe"
    StrCpy $INSTDIR "$PROGRAMFILES64\obs-studio"
  ${EndIf}
FunctionEnd

; ---- Verificar que la carpeta elegida es OBS ----
Function .onVerifyInstDir
  ${IfNot} ${FileExists} "$INSTDIR\bin\64bit\obs64.exe"
    Abort
  ${EndIf}
FunctionEnd

; ---- Seccion principal ----
Section "Plugin BetterChatTV" SecMain
  SectionIn RO

  ; El .dll del plugin.
  SetOutPath "$INSTDIR\obs-plugins\64bit"
  File "payload\betterchat-obs.dll"

  ; Los datos (locale).
  SetOutPath "$INSTDIR\data\obs-plugins\${PLUGIN_ID}"
  File /r "payload\data\*.*"

  ; Registro para Agregar/Quitar programas + desinstalador.
  WriteUninstaller "$INSTDIR\obs-plugins\64bit\Uninstall-${PLUGIN_ID}.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_ID}" \
    "DisplayName" "${PLUGIN_NAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_ID}" \
    "DisplayVersion" "${PLUGIN_VER}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_ID}" \
    "Publisher" "${PUBLISHER}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_ID}" \
    "URLInfoAbout" "${WEBSITE}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_ID}" \
    "UninstallString" "$\"$INSTDIR\obs-plugins\64bit\Uninstall-${PLUGIN_ID}.exe$\""
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_ID}" \
    "InstallLocation" "$INSTDIR"
SectionEnd

; ---- Aviso final ----
Function .onInstSuccess
  MessageBox MB_OK|MB_ICONINFORMATION "Plugin instalado.$\r$\n$\r$\nSi OBS estaba abierto, cierralo y vuelve a abrirlo. Luego activa el panel 'BetterChatTV' desde el menu Acoplables (Docks)."
FunctionEnd

; ---- Desinstalador ----
Section "Uninstall"
  Delete "$INSTDIR\obs-plugins\64bit\betterchat-obs.dll"
  Delete "$INSTDIR\obs-plugins\64bit\Uninstall-${PLUGIN_ID}.exe"
  RMDir /r "$INSTDIR\data\obs-plugins\${PLUGIN_ID}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_ID}"
SectionEnd
