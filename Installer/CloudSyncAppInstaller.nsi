;NSIS Modern User Interface
;Welcome/Finish Page Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"
  !include "InstallOptions.nsh"
;--------------------------------
;General

  ;Name and file
  Name "CloudSyncApp"
  OutFile "CloudSyncAppInstaller.exe"
  
  ;Request application privileges for Windows Vista
  !define MULTIUSER_EXECUTIONLEVEL Admin
;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !define MUI_PAGE_CUSTOMFUNCTION_SHOW ModifyWelcome
  !define MUI_PAGE_CUSTOMFUNCTION_LEAVE LeaveWelcome
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages
!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections
Var prodName
Var fileDesc
Var mycheckbox
Var mycheckboxState

Function ModifyWelcome
    ${NSD_CreateCheckbox} 120u -18u 50% 12u "Create Desktop and Program Menu shortcuts."
    Pop $mycheckbox
    SetCtlColors $mycheckbox "" ${MUI_BGCOLOR}
    ${NSD_Check} $mycheckbox ; Check it by default
FunctionEnd

Function LeaveWelcome
  ${NSD_GetState} $mycheckbox $0
  ${If} $0 <> 0
    StrCpy $mycheckboxState "1"
  ${Else}
    StrCpy $mycheckboxState "0"
  ${EndIf}
FunctionEnd

!define logData "!insertmacro logData"
!macro logData string

  FileOpen $fileDesc "$TEMP\nsisInstaller.log" a
  FileSeek $fileDesc 0 END
  
  StrCpy $0 "%Y-%m-%d %H:%M:%S"
  nsisdt::currentdate
  
  FileWrite $fileDesc "$0"
  FileWrite $fileDesc "$\t$\t${string}$\r$\n"
  
  FileClose $fileDesc
!macroend

Function .onInit
    StrCpy $prodName "CloudSyncApp"
FunctionEnd

Function un.onInit
    StrCpy $prodName "CloudSyncApp"
FunctionEnd

Section "CloudSyncApp" SecDummy  # installation
  
  CreateDirectory "$PROGRAMFILES\$prodName"
        
  setOutPath "$PROGRAMFILES\$prodName"

  ${logData} "write to PF Dir: $PROGRAMFILES\$prodName"
        
  File "libcurl.dll"
  File "libssh2.dll"
  File "libeay32.dll"
  File "ssleay32.dll"
  File "zlib1.dll"
  File "sqlite3.dll"
  File CloudSyncApp.exe
  File app.ico
  
  ${logData} "done write files"
  ;Create uninstaller
  WriteUninstaller "$PROGRAMFILES\$prodName\Uninstall.exe"

  WriteRegStr HKLM "Software\$prodName" "Version" "V1"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\$prodName" \
                 "DisplayName" "$prodName"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\$prodName" \
                 "UninstallString" "$\"$PROGRAMFILES\$prodName\Uninstall.exe$\""
  
  CreateDirectory "$SMPROGRAMS\$prodName"
  CreateShortCut "$SMPROGRAMS\$prodName\Uninstall.lnk" "$PROGRAMFILES\$prodName\Uninstall.exe" \
                   "" "$WINDIR\Uninstall.exe" 0 SW_SHOWNORMAL

  ${If} $mycheckboxState == "1"
    CreateShortCut "$SMPROGRAMS\$prodName\CloudSyncApp.lnk" "$PROGRAMFILES\$prodName\CloudSyncApp.exe" "" "$PROGRAMFILES\$prodName\app.ico"
    CreateShortCut "$DESKTOP\CloudSyncApp.lnk" "$PROGRAMFILES\$prodName\CloudSyncApp.exe" "" "$PROGRAMFILES\$prodName\app.ico"
  ${EndIf}

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecDummy ${LANG_ENGLISH} "CloudSyncApp"

  ;Assign language strings to sections
;  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
;  !insertmacro MUI_DESCRIPTION_TEXT ${SecDummy} $(DESC_SecDummy)
;  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"
    ${logData} "uninst local res"
	
    Delete "$PROGRAMFILES\$prodName\CloudSyncApp.exe"
    Delete "$PROGRAMFILES\$prodName\app.ico"
    Delete "$PROGRAMFILES\$prodName\Uninstall.exe"
    Delete "$PROGRAMFILES\$prodName\libssh2.dll"
    Delete "$PROGRAMFILES\$prodName\libeay32.dll"
    Delete "$PROGRAMFILES\$prodName\ssleay32.dll"
    Delete "$PROGRAMFILES\$prodName\zlib1.dll"
    Delete "$PROGRAMFILES\$prodName\sqlite3.dll"
    Delete "$DESKTOP\CloudSyncApp.lnk"
    
    RMDir /r "$SMPROGRAMS\$prodName"
    RMDir /r "$PROGRAMFILES\$prodName"
    DeleteRegKey HKLM "Software\$prodName"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\$prodName"
SectionEnd
