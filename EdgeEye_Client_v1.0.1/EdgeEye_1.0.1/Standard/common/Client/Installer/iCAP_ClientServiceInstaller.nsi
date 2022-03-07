;Include Modern UI
  !include "MUI2.nsh"

;General

  ;Name and file
  !define CORP "Innodisk"
  !define NAME "iCAP_ClientService"
  !define VERSION "1.0.1"

  Name "${NAME}${VERSION}"
  OutFile "${NAME}_win32_${VERSION}.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\${CORP}\${NAME}"

  !addplugindir "${NSISDIR}\Plugins\NSIS_Simple_Service_Plugin_1.30"

;--------------------------------
;Interface Settings
  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\win.bmp" ;
  !define MUI_ABORTWARNING
  ShowInstDetails show

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE EULA.txt
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "${NAME}" SecDummy

  SetOutPath "$INSTDIR"
  
  File "..\ClientService\iCAP_ClientService.exe"
  File "..\..\..\..\..\..\..\library\libsmart\libsmart.dll"
  File "..\..\..\..\..\..\..\library\EAPI\lib\windows\x86\shared\EAPI_Library.dll"
  File "..\..\..\..\..\..\..\library\lib_SysInfo\bin\lib_SysInfo.dll"
  File "..\..\..\..\..\..\..\library\libcrypto\bin\libcrypto.dll"
  File "..\..\..\..\..\..\..\library\libcrypto\Build-OpenSSL-MinGW-64\bin\libeay32.dll"
  File "..\..\..\..\..\..\..\library\libcrypto\Build-OpenSSL-MinGW-64\bin\ssleay32.dll"
  File "..\Dependencies\Windows\curl-7.44.0\lib\libcurl.dll"
  File "..\Dependencies\Windows\mingw\libgcc_s_dw2-1.dll"
  File "..\Dependencies\Windows\json-c\libjson-c-2.dll"
  File "..\Dependencies\Windows\mingw\libwinpthread-1.dll"
  File "..\Dependencies\Windows\GTK3\msvcr120.dll"
  File "..\Dependencies\Windows\GTK3\libstdc++-6.dll"
  File "..\..\..\..\..\..\..\library\eclipse-paho-mqtt-c-windows-1.1.0\lib\paho-mqtt3a.dll"
  File "..\..\..\..\..\..\..\library\libcpu\lib\libcpu.dll"
  File "..\..\..\..\..\..\..\library\libcpu\cpuz_bin\cpuidsdk.dll"
  File "..\..\..\..\..\..\..\library\libcpu\cpuz_bin\cpuidsdk64.dll"
  File "..\..\..\..\..\..\..\..\..\Inno\Trunk\EP\iRAID\dll\lib_ExSS.dll"
  File "..\..\..\..\..\..\..\..\..\Inno\Trunk\EP\iRAID\dll\lib_ExPS.dll"
  IfFileExists "$PROGRAMFILES\${CORP}\${NAME}\AsmIo64.sys" +3 0
  File "..\..\..\..\..\..\..\..\..\Inno\Trunk\EP\iRAID\release\pack\AsmIo.sys"
  File "..\..\..\..\..\..\..\..\..\Inno\Trunk\EP\iRAID\release\pack\AsmIo64.sys"
  File "..\..\..\..\..\..\..\..\..\Inno\Trunk\EP\iRAID\dll\asmiodll.dll"
  File "..\ClientService\iCAP_Client_Website.url"
  File "..\ClientService\icon.ico"
  File "..\Dependencies\Windows\sqlite3\sqlite3.dll"
  File "..\Dependencies\Windows\ismart\Flash.ini"
  File "..\Dependencies\Windows\ismart\mo.txt"
  File "..\Dependencies\Windows\ismart\mo.enc"


  CreateDirectory "$SYSDIR\${CORP}\${NAME}\wwwroot"
  CreateDirectory "$SYSDIR\${CORP}\${NAME}\wwwroot\dist"

  SetOutPath "$SYSDIR"

  File "..\Dependencies\Windows\ismart\Flash.ini"
  File "..\Dependencies\Windows\ismart\mo.txt"
  File "..\Dependencies\Windows\ismart\mo.enc"

  ;Store installation folder
  #WriteRegStr HKCU "Software\iCAP_ClientService" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  SetOutPath "$INSTDIR"
  CreateDirectory "$SMPROGRAMS\${CORP}"
  CreateDirectory "$SMPROGRAMS\${CORP}\${NAME}"
  CreateShortCut "$SMPROGRAMS\${CORP}\${NAME}\${NAME}.lnk" "$INSTDIR\iCAP_Client_Website.url"
  CreateShortCut "$SMPROGRAMS\${CORP}\${NAME}\Uninstall.lnk" "$INSTDIR\Uninstall.exe"

  SimpleSC::InstallService "iCAP" "iCAP Client Service" "16" "2" "$INSTDIR\iCAP_ClientService.exe" "" "" ""
  SimpleSC::SetServiceDescription "iCAP" "Innodisk iCAP client service"
  SimpleSC::SetServiceFailure "iCAP" "0" "" "" "1" "60000" "1" "60000" "1" "60000" 
  SimpleSC::SetServiceStartType "iCAP" "2"
  SimpleSC::SetServiceDelayedAutoStartInfo "iCAP" "1"

  SimpleSC::StartService "iCAP" "" 30
SectionEnd

Section "Create desktop shortcuts" CreateShortcuts

  SetOutPath "$INSTDIR"

  CreateShortCut "$DESKTOP\${NAME} Setting.lnk" "$INSTDIR\iCAP_Client_Website.url" "" "$INSTDIR\icon.ico" 0


  SetShellVarContext all
  SetOutPath "$SYSDIR\${CORP}\${NAME}\wwwroot\dist\"
  File /r "..\ClientService\wwwroot\dist\*"
  SetOutPath "$APPDATA\${CORP}\${NAME}"

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecDummy ${LANG_ENGLISH} "iCAP ClientService."
  LangString DESC_CreateShort ${LANG_ENGLISH} "Create desktop shortcut."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDummy} $(DESC_SecDummy)
    !insertmacro MUI_DESCRIPTION_TEXT ${CreateShortcuts} $(DESC_CreateShort)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...

  SimpleSC::StopService "iCAP" 1 30
  SimpleSC::RemoveService "iCAP"

  RMDir /r "$INSTDIR"
  RMDir /r "$PROGRAMFILES\${CORP}"
  RMDir /r "$SYSDIR\${CORP}"

  ;Delete "$SYSDIR\Flash.ini"
  ;Delete "$SYSDIR\mo.txt"

  Delete "$SMPROGRAMS\${CORP}\${NAME}\${NAME}.lnk"
  Delete "$SMPROGRAMS\${CORP}\${NAME}\Uninstall.lnk"
  Delete "$DESKTOP\${NAME} Setting.lnk"
  RMDir /r "$SMPROGRAMS\${CORP}\${NAME}"
  RMDir /r "$SMPROGRAMS\${CORP}"

SectionEnd