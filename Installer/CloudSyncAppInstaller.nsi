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
  !define MUI_PAGE_CUSTOMFUNCTION_PRE SkipWelcome
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Vars

Var prodName
Var fileDesc
Var mycheckbox
Var mycheckboxState
Var update
;--------------------------------
 ; GetParameters
 ; input, none
 ; output, top of stack (replaces, with e.g. whatever)
 ; modifies no other variables.

Function GetParameters

  Push $R0
  Push $R1
  Push $R2
  Push $R3

  StrCpy $R2 1
  StrLen $R3 $CMDLINE

  ;Check for quote or space
  StrCpy $R0 $CMDLINE $R2
  StrCmp $R0 '"' 0 +3
    StrCpy $R1 '"'
    Goto loop
  StrCpy $R1 " "

  loop:
    IntOp $R2 $R2 + 1
    StrCpy $R0 $CMDLINE 1 $R2
    StrCmp $R0 $R1 get
    StrCmp $R2 $R3 get
    Goto loop

  get:
    IntOp $R2 $R2 + 1
    StrCpy $R0 $CMDLINE 1 $R2
    StrCmp $R0 " " get
    StrCpy $R0 $CMDLINE "" $R2

  Pop $R3
  Pop $R2
  Pop $R1
  Exch $R0

FunctionEnd

Function SkipWelcome

  StrCmp $update "1" skip
  goto done
skip:
    Abort
    
done:
FunctionEnd

;--------------------------------
!insertmacro MUI_LANGUAGE "English"

; ################################################################
; appends \ to the path if missing
; example: !insertmacro GetCleanDir "c:\blabla"
; Pop $0 => "c:\blabla\"
!macro GetCleanDir INPUTDIR
  ; ATTENTION: USE ON YOUR OWN RISK!
  ; Please report bugs here: http://stefan.bertels.org/
  !define Index_GetCleanDir 'GetCleanDir_Line${__LINE__}'
  Push $R0
  Push $R1
  StrCpy $R0 "${INPUTDIR}"
  StrCmp $R0 "" ${Index_GetCleanDir}-finish
  StrCpy $R1 "$R0" "" -1
  StrCmp "$R1" "\" ${Index_GetCleanDir}-finish
  StrCpy $R0 "$R0\"
${Index_GetCleanDir}-finish:
  Pop $R1
  Exch $R0
  !undef Index_GetCleanDir
!macroend

; ################################################################
; similar to "RMDIR /r DIRECTORY", but does not remove DIRECTORY itself
; example: !insertmacro RemoveFilesAndSubDirs "$INSTDIR"
!macro RemoveFilesAndSubDirs DIRECTORY
  ; ATTENTION: USE ON YOUR OWN RISK!
  ; Please report bugs here: http://stefan.bertels.org/
  !define Index_RemoveFilesAndSubDirs 'RemoveFilesAndSubDirs_${__LINE__}'

  Push $R0
  Push $R1
  Push $R2

  !insertmacro GetCleanDir "${DIRECTORY}"
  Pop $R2
  FindFirst $R0 $R1 "$R2*.*"
${Index_RemoveFilesAndSubDirs}-loop:
  StrCmp $R1 "" ${Index_RemoveFilesAndSubDirs}-done
  StrCmp $R1 "." ${Index_RemoveFilesAndSubDirs}-next
  StrCmp $R1 ".." ${Index_RemoveFilesAndSubDirs}-next
  IfFileExists "$R2$R1\*.*" ${Index_RemoveFilesAndSubDirs}-directory
  ; file
  Delete "$R2$R1"
  goto ${Index_RemoveFilesAndSubDirs}-next
${Index_RemoveFilesAndSubDirs}-directory:
  ; directory
  RMDir /r "$R2$R1"
${Index_RemoveFilesAndSubDirs}-next:
  FindNext $R0 $R1
  Goto ${Index_RemoveFilesAndSubDirs}-loop
${Index_RemoveFilesAndSubDirs}-done:
  FindClose $R0

  Pop $R2
  Pop $R1
  Pop $R0
  !undef Index_RemoveFilesAndSubDirs
!macroend

;--------------------------------
;Installer Sections

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
    
    Call GetParameters
    Pop $2
    
    ;MessageBox MB_OK $2
    
    StrCmp $2 "-update" need_update
    GoTo done
    need_update:
    StrCpy $update "1"
    done:
    
FunctionEnd

Function un.onInit
    StrCpy $prodName "CloudSyncApp"
FunctionEnd

Section "CloudSyncApp" SecDummy  # installation
  
  StrCmp $update "1" update_install
  CreateDirectory "$PROGRAMFILES\$prodName"
  goto continue_install
  
update_install:
  Sleep 1500
  !insertmacro RemoveFilesAndSubDirs "$PROGRAMFILES\$prodName"
  
  continue_install:
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
	
    ;Delete "$PROGRAMFILES\$prodName\CloudSyncApp.exe"
    ;Delete "$PROGRAMFILES\$prodName\app.ico"
    ;Delete "$PROGRAMFILES\$prodName\Uninstall.exe"
    ;Delete "$PROGRAMFILES\$prodName\libssh2.dll"
    ;Delete "$PROGRAMFILES\$prodName\libeay32.dll"
    ;Delete "$PROGRAMFILES\$prodName\ssleay32.dll"
    ;Delete "$PROGRAMFILES\$prodName\zlib1.dll"
    ;Delete "$PROGRAMFILES\$prodName\sqlite3.dll"
    
    !insertmacro RemoveFilesAndSubDirs "$SMPROGRAMS\$prodName"
    
    Delete "$DESKTOP\CloudSyncApp.lnk"
    
    RMDir /r "$SMPROGRAMS\$prodName"
    RMDir /r "$PROGRAMFILES\$prodName"
    DeleteRegKey HKLM "Software\$prodName"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\$prodName"
SectionEnd
