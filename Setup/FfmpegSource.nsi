; Adds a user (no admin) manifest (for vista or higher)
RequestExecutionLevel user

; Name Defines
!define APPNAME "ContaCam Ffmpeg Source"
!define APPNAME_NOSPACE "ContaCamFfmpegSource"
!define UNINSTNAME_EXT "uninstall.exe"
!define UNINSTNAME_LNK "Uninstall.lnk"

; Mutex Name Define
!define INSTALLERMUTEXNAME "${APPNAME_NOSPACE}InstallerMutex"

; Application Version Define
!define APPVERSION "2.0.7"

; The name of the Program used in Title Bar and in
; Explaining Text when Referring to the Software
Name "${APPNAME} ${APPVERSION} (based on rev. 15375)"

; Includes
!include "MUI2.nsh"
!include "UAC.nsh"
!include "InitVersion.nsh"

; The file to write
OutFile "${APPNAME_NOSPACE}-${APPVERSION}-Setup.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\${APPNAME}"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Contaware\${APPNAME}" "Install_Dir"

; Compressor
SetCompressor lzma

; Write Manifest to Installer and Uninstaller
xpstyle on

; Interface Settings for Modern UI
!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_FINISHPAGE_TITLE_3LINES
!define MUI_CUSTOMFUNCTION_ABORT MyOnUserAbort
!define MUI_CUSTOMFUNCTION_UNABORT un.MyOnUserAbort
!define MUI_ABORTWARNING
!define MUI_WELCOMEPAGE
;!define MUI_FINISHPAGE_NOAUTOCLOSE ;Do not automatically jump to the finish page, to allow the user to check the log.

;--------------------------------

; Page Modern UI
!insertmacro MUI_PAGE_LICENSE "..\License\lgpl-3.0.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------

; Language File Modern UI,
; change also the Version Information language!
!insertmacro MUI_LANGUAGE "English"

;--------------------------------

Function .OnInstFailed
${UAC.Unload} ;Must call unload!
FunctionEnd
Function MyOnUserAbort
${UAC.Unload} ;Must call unload!
FunctionEnd
Function .OnInstSuccess
${UAC.Unload} ;Must call unload!
FunctionEnd

;--------------------------------

Function .onInit

  Push $R0
  Push $R1
  ClearErrors
	
  ; Get Win Version
  call InitVersion
  
  ; UAC
  StrCmp $HAS_UAC 'FALSE' InstallCheckInstallerRunning
    ${UAC.I.Elevate.AdminOnly}
  
  ; Single Instance of Installer
InstallCheckInstallerRunning:
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${INSTALLERMUTEXNAME}") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 lbl_end
  MessageBox MB_OK|MB_ICONEXCLAMATION "The Installer is already running" /SD IDOK
  ClearErrors
  Pop $R1
  Pop $R0
  ${UAC.Unload} ;Must call unload!
  Abort

lbl_end:  
  Pop $R1
  Pop $R0
 
FunctionEnd
  
;--------------------------------

!define SHCNE_ASSOCCHANGED 0x08000000
!define SHCNF_IDLIST 0
Function RefreshShellIcons
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v \
  (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
FunctionEnd

;--------------------------------

; The stuff to install
Section "${APPNAME} Program (required)"

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Section Read-Only (=User Cannot Change it's state)
  SectionIn RO

  ; Remove previous install files and directories
  Delete $INSTDIR\${UNINSTNAME_EXT}
  RMDir /r "$INSTDIR\src"
    
  ; Create Directories
  CreateDirectory "$INSTDIR\src"
  
  ; Source Program File Path
  SetOutPath $INSTDIR\src
  File /r /x .svn /x *.o /x *.a /x *.d /x *.pc "..\ffmpeg\*.*"
  SetOutPath $INSTDIR
  
  ; Write the installation path into the registry
  WriteRegStr HKLM "Software\Contaware\${APPNAME}" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" '"$INSTDIR\${UNINSTNAME_EXT}"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "QuietUninstallString" '"$INSTDIR\${UNINSTNAME_EXT}" /S'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayVersion" "${APPVERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "Publisher" "Contaware.com"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "URLInfoAbout" "http://www.contaware.com"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoRepair" 1
  WriteUninstaller "${UNINSTNAME_EXT}"
  
  ; Refresh Icons
  call RefreshShellIcons

SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  ; Stores to Start Menu for All Users
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\${APPNAME}"
  CreateShortCut "$SMPROGRAMS\${APPNAME}\${UNINSTNAME_LNK}" "$INSTDIR\${UNINSTNAME_EXT}" "" "$INSTDIR\${UNINSTNAME_EXT}" 0
  
SectionEnd

;--------------------------------

;Version Information

VIProductVersion "${APPVERSION}.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${APPNAME}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" "Modified ffmpeg source based on rev. 15375"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "Contaware.com"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" "${APPNAME} is a trademark of Contaware.com"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "© Contaware.com"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "Installation Routine of ${APPNAME}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${APPVERSION}.0"

;--------------------------------

; Uninstaller

;--------------------------------

Function un.OnUnInstFailed
${UAC.Unload} ;Must call unload!
FunctionEnd
Function un.MyOnUserAbort
${UAC.Unload} ;Must call unload!
FunctionEnd
Function un.OnUnInstSuccess
${UAC.Unload} ;Must call unload!
FunctionEnd

;--------------------------------

Function un.onInit

  Push $R0
  Push $R1
  ClearErrors
  
  ; Get Win Version
  call un.InitVersion
  
  ; UAC
  StrCmp $HAS_UAC 'FALSE' UninstallCheckUninstallerRunning
    ${UAC.U.Elevate.AdminOnly} ${UNINSTNAME_EXT}
  
  ; Single Instance of Uninstaller
UninstallCheckUninstallerRunning:
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${INSTALLERMUTEXNAME}") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 lbl_end
  MessageBox MB_OK|MB_ICONEXCLAMATION "The Uninstaller is already running" /SD IDOK
  ClearErrors
  Pop $R1
  Pop $R0
  ${UAC.Unload} ;Must call unload!
  Abort
  
lbl_end:
  Pop $R1
  Pop $R0
  
FunctionEnd

;--------------------------------

Function un.RefreshShellIcons
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v \
  (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
FunctionEnd

;--------------------------------

; The stuff to uninstall
Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
  DeleteRegKey HKLM "Software\Contaware\${APPNAME}"
  
  ; Remove uninstaller
  Delete $INSTDIR\${UNINSTNAME_EXT}

  ; Removes Shortcuts from the Start Menu for All Users
  SetShellVarContext all
  Delete "$SMPROGRAMS\${APPNAME}\*.*"
  RMDir "$SMPROGRAMS\${APPNAME}"
  
  ; Remove directories used
  RMDir /r "$INSTDIR\src"
  RMDir "$INSTDIR"
  
  ; Refresh Icons
  call un.RefreshShellIcons

SectionEnd
