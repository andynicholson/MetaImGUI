; NSIS Installer Script for MetaImGUI
; Requires NSIS 3.0 or later
; Build with: makensis windows_installer.nsi

;--------------------------------
; Includes

!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "x64.nsh"

;--------------------------------
; Configuration

!define PRODUCT_NAME "MetaImGUI"
!define PRODUCT_VERSION "1.0.0"
!define PRODUCT_PUBLISHER "A P Nicholson"
!define PRODUCT_WEB_SITE "https://github.com/andynicholson/MetaImGUI"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"

;--------------------------------
; General

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "..\packaging\output\${PRODUCT_NAME}-${PRODUCT_VERSION}-Setup.exe"
InstallDir "$PROGRAMFILES64\${PRODUCT_NAME}"
InstallDirRegKey HKLM "Software\${PRODUCT_NAME}" "Install_Dir"
RequestExecutionLevel admin

;--------------------------------
; Interface Settings

!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

;--------------------------------
; Pages

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\README.md"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

;--------------------------------
; Languages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Version Information

VIProductVersion "${PRODUCT_VERSION}.0"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "ProductVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "CompanyName" "${PRODUCT_PUBLISHER}"
VIAddVersionKey "FileDescription" "${PRODUCT_NAME} Installer"
VIAddVersionKey "FileVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "LegalCopyright" "Copyright (C) 2025 ${PRODUCT_PUBLISHER}"

;--------------------------------
; Installer Sections

Section "Core Files (required)" SecCore
  SectionIn RO

  SetOutPath "$INSTDIR"

  ; Add files
  File "..\build\Release\MetaImGUI.exe"

  ; Add vcpkg DLLs (required dependencies)
  ; Note: These must be copied from vcpkg's bin directory
  IfFileExists "C:\vcpkg\installed\x64-windows\bin\glfw3.dll" 0 +2
    File "C:\vcpkg\installed\x64-windows\bin\glfw3.dll"

  IfFileExists "C:\vcpkg\installed\x64-windows\bin\libcurl.dll" 0 +2
    File "C:\vcpkg\installed\x64-windows\bin\libcurl.dll"

  ; libcurl dependencies
  File /nonfatal "C:\vcpkg\installed\x64-windows\bin\zlib1.dll"

  ; OpenSSL DLLs (try different versions, vcpkg may have renamed)
  File /nonfatal "C:\vcpkg\installed\x64-windows\bin\libssl-3-x64.dll"
  File /nonfatal "C:\vcpkg\installed\x64-windows\bin\libssl-3.dll"
  File /nonfatal "C:\vcpkg\installed\x64-windows\bin\libcrypto-3-x64.dll"
  File /nonfatal "C:\vcpkg\installed\x64-windows\bin\libcrypto-3.dll"

  ; VC++ Runtime DLLs (if not already installed system-wide)
  ; These might be at vcpkg\installed\x64-windows\bin or in System32
  IfFileExists "C:\vcpkg\installed\x64-windows\bin\msvcp140.dll" 0 +2
    File "C:\vcpkg\installed\x64-windows\bin\msvcp140.dll"

  IfFileExists "C:\vcpkg\installed\x64-windows\bin\vcruntime140.dll" 0 +2
    File "C:\vcpkg\installed\x64-windows\bin\vcruntime140.dll"

  IfFileExists "C:\vcpkg\installed\x64-windows\bin\vcruntime140_1.dll" 0 +2
    File "C:\vcpkg\installed\x64-windows\bin\vcruntime140_1.dll"

  ; Add resources if they exist
  IfFileExists "..\resources\*.*" 0 +3
    CreateDirectory "$INSTDIR\resources"
    File /r "..\resources\*.*"

  ; Write registry keys
  WriteRegStr HKLM "Software\${PRODUCT_NAME}" "Install_Dir" "$INSTDIR"

  ; Write uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; Write uninstall registry keys
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\MetaImGUI.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegDWORD HKLM "${PRODUCT_UNINST_KEY}" "NoModify" 1
  WriteRegDWORD HKLM "${PRODUCT_UNINST_KEY}" "NoRepair" 1

  ; Get installed size
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  WriteRegDWORD HKLM "${PRODUCT_UNINST_KEY}" "EstimatedSize" "$0"

SectionEnd

Section "Start Menu Shortcuts" SecStartMenu
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
  CreateShortcut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\MetaImGUI.exe" "" "$INSTDIR\MetaImGUI.exe" 0
  CreateShortcut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0
SectionEnd

Section "Desktop Shortcut" SecDesktop
  CreateShortcut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\MetaImGUI.exe" "" "$INSTDIR\MetaImGUI.exe" 0
SectionEnd

;--------------------------------
; Section Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "Core application files (required)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} "Add shortcuts to the Start Menu"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "Add shortcut to the Desktop"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
; Uninstaller Section

Section "Uninstall"
  ; Remove files
  Delete "$INSTDIR\MetaImGUI.exe"
  Delete "$INSTDIR\glfw3.dll"
  Delete "$INSTDIR\libcurl.dll"
  Delete "$INSTDIR\zlib1.dll"
  Delete "$INSTDIR\libssl-3-x64.dll"
  Delete "$INSTDIR\libcrypto-3-x64.dll"
  Delete "$INSTDIR\msvcp140.dll"
  Delete "$INSTDIR\vcruntime140.dll"
  Delete "$INSTDIR\vcruntime140_1.dll"
  Delete "$INSTDIR\Uninstall.exe"

  ; Remove resources
  RMDir /r "$INSTDIR\resources"

  ; Remove shortcuts
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\*.*"
  RMDir "$SMPROGRAMS\${PRODUCT_NAME}"
  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"

  ; Remove directories
  RMDir "$INSTDIR"

  ; Remove registry keys
  DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "Software\${PRODUCT_NAME}"

  ; Remove from Programs and Features
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"

SectionEnd

;--------------------------------
; Functions

Function .onInit
  ; Check if running on 64-bit Windows
  ${If} ${RunningX64}
    ; Good to go
  ${Else}
    MessageBox MB_OK|MB_ICONEXCLAMATION "This application requires a 64-bit version of Windows."
    Abort
  ${EndIf}

  ; Note: This application requires Visual C++ Redistributable 2015-2022
  ; If the application fails to start, install VC++ Redistributable from:
  ; https://aka.ms/vs/17/release/vc_redist.x64.exe
  ;
  ; To check if installed, look for:
  ; HKLM\SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64
FunctionEnd

