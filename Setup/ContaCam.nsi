; Adds a user (no admin) manifest (for vista or higher)
RequestExecutionLevel user

; Language defines
!ifndef INSTALLER_LANGUAGE
	!define INSTALLER_LANGUAGE "English"
!endif
!if ${INSTALLER_LANGUAGE} == "German"
	!define INSTALLER_LANGUAGE_ID ${LANG_GERMAN}
	!define INSTALLER_LANGUAGE_SUFFIX "Deu"
!else if ${INSTALLER_LANGUAGE} == "Italian"
	!define INSTALLER_LANGUAGE_ID ${LANG_ITALIAN}
	!define INSTALLER_LANGUAGE_SUFFIX "Ita"
!else if ${INSTALLER_LANGUAGE} == "Russian"
	!define INSTALLER_LANGUAGE_ID ${LANG_RUSSIAN}
	!define INSTALLER_LANGUAGE_SUFFIX "Rus"
!else
	!define INSTALLER_LANGUAGE_ID ${LANG_ENGLISH}
	!define INSTALLER_LANGUAGE_SUFFIX ""
!endif

; Name Defines
!include "CurrentVersion.nsh"
!define APPNAME_EXT "ContaCam.exe"
!define APPNAME_NOEXT "ContaCam"
!define UNINSTNAME_EXT "uninstall.exe"
!define UNINSTNAME_LNK "Uninstall.lnk"

; Mutex Name Define
; Remember to change the Application Mutex
; Name in the C++ Source!
!define APPMUTEXNAME "${APPNAME_NOEXT}AppMutex"
!define INSTALLERMUTEXNAME "${APPNAME_NOEXT}InstallerMutex"

; The name of the Program used in Title Bar and in
; Explaining Text when Referring to the Software
Name "${APPNAME_NOEXT} ${APPVERSION}"

; Includes
!include "MUI2.nsh"
!include "UAC.nsh"
!include "InitVersion.nsh"
!include "ContawareFileAssociation.nsh"
!include "ContawareParams.nsh"

; The file to write
!if ${INSTALLER_LANGUAGE} == "English"
	OutFile "${APPNAME_NOEXT}-${APPVERSION}-Setup.exe"
!else
	OutFile "${APPNAME_NOEXT}-${APPVERSION}-Setup-${INSTALLER_LANGUAGE_SUFFIX}.exe"
!endif

; The default installation directory
InstallDir "$PROGRAMFILES\${APPNAME_NOEXT}"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Contaware\${APPNAME_NOEXT}" "Install_Dir"

; Don't check CRC because we can have problems when installing from a net share!
CRCCheck off

; Compressor
SetCompressor /FINAL /SOLID lzma
SetCompressorDictSize 16

; Write Manifest to Installer and Uninstaller
xpstyle on

; Interface Settings for Modern UI
!define MUI_CUSTOMFUNCTION_ABORT MyOnUserAbort
!define MUI_CUSTOMFUNCTION_UNABORT un.MyOnUserAbort
!define MUI_ABORTWARNING
;!define MUI_UNABORTWARNING
!define MUI_WELCOMEPAGE
;!define MUI_FINISHPAGE_NOAUTOCLOSE ;Do not automatically jump to the finish page, to allow the user to check the log.
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_FUNCTION FinishRunCB
;!define MUI_FINISHPAGE_RUN_NOTCHECKED
;!define MUI_ICON "..\res\uimager_videodevicedoc.ico"
;!define MUI_UNICON "..\res\uimager_videodevicedoc.ico"

;--------------------------------

; Page Modern UI
!insertmacro MUI_PAGE_LICENSE "..\License\License${INSTALLER_LANGUAGE_SUFFIX}.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE PageFinLeave
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------

; Language File Modern UI
!insertmacro MUI_LANGUAGE ${INSTALLER_LANGUAGE}

; Multilingual Messages
!include "${INSTALLER_LANGUAGE}.nsh"

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
Function PageFinLeave
; Start service and start ContaCam through service if Run App not checked
nsExec::Exec '"$INSTDIR\ContaCamService.exe" -r'
SendMessage $mui.FinishPage.Run ${BM_GETCHECK} 0 0 $mui.FinishPage.ReturnValue
${if} $mui.FinishPage.ReturnValue = ${BST_UNCHECKED}
	nsExec::Exec '"$INSTDIR\ContaCamService.exe" -proc'
${endif}
FunctionEnd

;--------------------------------

Function .onInit

  Push $R0
  Push $R1
  ClearErrors
  
  ; Get Win Version
  call InitVersion
  
  ; Passed Parameters
  call InitParams
	
  ; UAC
  StrCmp $HAS_UAC 'FALSE' InstallCheckInstallerRunning
    ${UAC.I.Elevate.AdminOnly}
  
  ; Single Instance of Installer
InstallCheckInstallerRunning:
  System::Call 'kernel32::CreateMutex(i 0, i 0, t "${INSTALLERMUTEXNAME}") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 lbl_end
  MessageBox MB_OK|MB_ICONEXCLAMATION $(AlreadyRunning) /SD IDOK
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

Function KillApp

  Push $R0
  Push $R1
  ClearErrors
  
  ; Kill Application
  System::Call 'kernel32::CreateMutex(i 0, i 0, t "${APPMUTEXNAME}") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 lbl_end
  StrCmp $KILL "1" KillApp 0      ; If param set -> kill without asking
  StrCmp $KILL "0" KillAppError 0 ; If param cleared -> do not kill
  ; If kill param not set -> ask (silent install answers no to the following question -> no killing):
  MessageBox MB_YESNO|MB_ICONQUESTION $(CloseAppPrompt) /SD IDNO IDYES KillApp
  goto KillAppAbort
KillApp:
  KillProcDLL::KillProc "${APPNAME_EXT}"
  StrCmp $R0 "0" KillApp 0
  StrCmp $R0 "603" 0 KillAppError
  Sleep 1500
  Goto lbl_end
KillAppError:
  MessageBox MB_OK|MB_ICONEXCLAMATION $(CloseAppError) /SD IDOK
KillAppAbort:
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

Function KillMicroApache

  Push $R0
  Push $R1
  ClearErrors

KillMApache:
  KillProcDLL::KillProc "mapache.exe"
  StrCmp $R0 "0" KillMApache 0
  Sleep 1500
  
  Pop $R1
  Pop $R0
  
FunctionEnd

;--------------------------------

Function FinishRunCB
UAC::Exec "" "$INSTDIR\${APPNAME_EXT}" "" ""
FunctionEnd

!define SHCNE_ASSOCCHANGED 0x08000000
!define SHCNF_IDLIST 0
Function RefreshShellIcons
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v \
  (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
FunctionEnd

;--------------------------------

; The stuff to install
Section "${APPNAME_NOEXT} Program (required)"

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Section Read-Only (=User Cannot Change it's state)
  SectionIn RO
  
  ; Stop service and app
  DetailPrint $(StoppingServiceMessage)
  nsExec::Exec '"$INSTDIR\ContaCamService.exe" -k'
  DetailPrint $(StoppingApplicationMessage)
  call KillApp
  call KillMicroApache

  ; Remove previous install files and directories
  Delete $INSTDIR\License.txt
  Delete $INSTDIR\History.txt
  Delete $INSTDIR\FullscreenBrowser.exe
  Delete $INSTDIR\Start.exe
  Delete $INSTDIR\${APPNAME_EXT}
  Delete $INSTDIR\NeroBurn.exe
  Delete $INSTDIR\ContaCamService.exe
  Delete $INSTDIR\${UNINSTNAME_EXT}
  RMDir /r "$INSTDIR\ActiveX"
  RMDir /r "$INSTDIR\Tutorials"
  RMDir /r "$INSTDIR\microapache"
    
  ; Create Tutorials Directory
  CreateDirectory "$INSTDIR\Tutorials"
  
  ; Source Program File Path
!if ${INSTALLER_LANGUAGE} == "English"
  File "..\Bin\${APPNAME_NOEXT}\${APPNAME_EXT}"
!else
  File "/oname=${APPNAME_NOEXT}.exe" "..\Translation\${APPNAME_NOEXT}${INSTALLER_LANGUAGE_SUFFIX}.exe"
!endif
  File "/oname=License.txt" "..\License\License${INSTALLER_LANGUAGE_SUFFIX}.txt"
  File "/oname=History.txt" "..\History\HistoryContaCam.txt"
  File "..\FullscreenBrowser\Release\FullscreenBrowser.exe"
  File "..\ContaCamService\Release\ContaCamService.exe"
  SetOverwrite off
  File "..\ContaCamService\Release\ContaCamService.ini"
  SetOverwrite on
  File "/oname=Tutorials\Getting_Started.htm" "..\Tutorials\Getting_Started.htm"
  File "/oname=Tutorials\Getting_Started.swf" "..\Tutorials\Getting_Started.swf"
  File "/oname=Tutorials\Getting_Started.js" "..\Tutorials\Getting_Started.js"
  File "/oname=Tutorials\Device_Settings.htm" "..\Tutorials\Device_Settings.htm"
  File "/oname=Tutorials\Device_Settings.swf" "..\Tutorials\Device_Settings.swf"
  File "/oname=Tutorials\Device_Settings.js" "..\Tutorials\Device_Settings.js"
  File "/oname=Tutorials\Global_Settings.htm" "..\Tutorials\Global_Settings.htm"
  File "/oname=Tutorials\Global_Settings.swf" "..\Tutorials\Global_Settings.swf"
  File "/oname=Tutorials\Global_Settings.js" "..\Tutorials\Global_Settings.js"
  File "/oname=Tutorials\Publish_on_Internet.htm" "..\Tutorials\Publish_on_Internet.htm"
  File "/oname=Tutorials\Publish_on_Internet.swf" "..\Tutorials\Publish_on_Internet.swf"
  File "/oname=Tutorials\Publish_on_Internet.js" "..\Tutorials\Publish_on_Internet.js"
  SetOutPath $INSTDIR\microapache
  File /r /x .svn /x configuration*.* "..\microapache\*.*"
  SetOutPath $INSTDIR
  File "/oname=microapache\htdocs\configuration.php" "..\microapache\htdocs\configuration${INSTALLER_LANGUAGE_SUFFIX}.php"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM "Software\Contaware\${APPNAME_NOEXT}" "Install_Dir" "$INSTDIR"
  
  ; Write the Application Registration
  WriteRegStr HKCR "Applications\${APPNAME_EXT}\DefaultIcon" "" "$INSTDIR\${APPNAME_EXT},0"
  WriteRegStr HKCR "Applications\${APPNAME_EXT}\shell\open\command" "" '"$INSTDIR\${APPNAME_EXT}" "%1"'
  WriteRegStr HKCR "Applications\${APPNAME_EXT}\shell\edit\command" "" '"$INSTDIR\${APPNAME_EXT}" "%1"'
  WriteRegStr HKCR "Applications\${APPNAME_EXT}\shell\preview\command" "" '"$INSTDIR\${APPNAME_EXT}" "%1"'
  WriteRegStr HKCR "Applications\${APPNAME_EXT}\shell\print\command" "" '"$INSTDIR\${APPNAME_EXT}" /p "%1"'
  WriteRegStr HKCR "Applications\${APPNAME_EXT}\shell\printto\command" "" '"$INSTDIR\${APPNAME_EXT}" /pt "%1"'
  
  ; Set first run flag and silent install flag
  WriteRegDWORD HKCU "Software\Contaware\${APPNAME_NOEXT}\GeneralApp" "FirstRun" 1
  IfSilent 0 +2
  WriteRegDWORD HKCU "Software\Contaware\${APPNAME_NOEXT}\GeneralApp" "SilentInstall" 1
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_NOEXT}" "DisplayName" "${APPNAME_NOEXT}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_NOEXT}" "UninstallString" '"$INSTDIR\${UNINSTNAME_EXT}"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_NOEXT}" "QuietUninstallString" '"$INSTDIR\${UNINSTNAME_EXT}" /S'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_NOEXT}" "DisplayIcon" "$INSTDIR\${APPNAME_EXT},0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_NOEXT}" "DisplayVersion" "${APPVERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_NOEXT}" "Publisher" "Contaware.com"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_NOEXT}" "URLInfoAbout" "http://www.contaware.com"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_NOEXT}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_NOEXT}" "NoRepair" 1
  WriteUninstaller "${UNINSTNAME_EXT}"
  
  ; Firewall add to the authorized list
  nsisFirewall::AddAuthorizedApplication "$INSTDIR\${APPNAME_EXT}" "${APPNAME_NOEXT}"
  nsisFirewall::AddAuthorizedApplication "$INSTDIR\microapache\mapache.exe" "Micro Apache Server (used by ${APPNAME_NOEXT})"
  
  ; File Associations
  
  !insertmacro ADDREMOVE_FILEASSOCIATION $BMP bmp
  !insertmacro ADDREMOVE_FILEASSOCIATION $JPG jpg
  !insertmacro ADDREMOVE_FILEASSOCIATION $JPG jpeg
  !insertmacro ADDREMOVE_FILEASSOCIATION $JPG jpe
  !insertmacro ADDREMOVE_FILEASSOCIATION $JPG thm
  !insertmacro ADDREMOVE_FILEASSOCIATION $PCX pcx
  !insertmacro ADDREMOVE_FILEASSOCIATION $EMF emf
  !insertmacro ADDREMOVE_FILEASSOCIATION $PNG png
  !insertmacro ADDREMOVE_FILEASSOCIATION $TIF tif
  !insertmacro ADDREMOVE_FILEASSOCIATION $TIF tiff
  !insertmacro ADDREMOVE_FILEASSOCIATION $TIF jfx
  !insertmacro ADDREMOVE_FILEASSOCIATION $GIF gif
  
  !insertmacro ADDREMOVE_FILEASSOCIATION $AIF aif
  !insertmacro ADDREMOVE_FILEASSOCIATION $AIF aiff
  !insertmacro ADDREMOVE_FILEASSOCIATION $AU au
  !insertmacro ADDREMOVE_FILEASSOCIATION $MID mid
  !insertmacro ADDREMOVE_FILEASSOCIATION $MID rmi
  !insertmacro ADDREMOVE_FILEASSOCIATION $MP3 mp3
  !insertmacro ADDREMOVE_FILEASSOCIATION $WAV wav
  !insertmacro ADDREMOVE_FILEASSOCIATION $WMA wma
  !insertmacro ADDREMOVE_FILEASSOCIATION $CDA cda
  
  !insertmacro ADDREMOVE_FILEASSOCIATION $AVI avi
  !insertmacro ADDREMOVE_FILEASSOCIATION $AVI divx
  !insertmacro ADDREMOVE_FILEASSOCIATION $ZIP zip

  ; Refresh Icons
  call RefreshShellIcons

SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  ; Stores to Start Menu for All Users
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\${APPNAME_NOEXT}"
  CreateShortCut "$SMPROGRAMS\${APPNAME_NOEXT}\${UNINSTNAME_LNK}" "$INSTDIR\${UNINSTNAME_EXT}" "" "$INSTDIR\${UNINSTNAME_EXT}" 0
  CreateShortCut "$SMPROGRAMS\${APPNAME_NOEXT}\${APPNAME_NOEXT}.lnk" "$INSTDIR\${APPNAME_EXT}" "" "$INSTDIR\${APPNAME_EXT}" 0
  
SectionEnd

;--------------------------------

;Version Information

VIProductVersion "${APPVERSION}.0"
VIAddVersionKey /LANG=${INSTALLER_LANGUAGE_ID} "ProductName" "${APPNAME_NOEXT} Application"
VIAddVersionKey /LANG=${INSTALLER_LANGUAGE_ID} "Comments" "MM Application"
VIAddVersionKey /LANG=${INSTALLER_LANGUAGE_ID} "CompanyName" "Contaware.com"
VIAddVersionKey /LANG=${INSTALLER_LANGUAGE_ID} "LegalTrademarks" ""
VIAddVersionKey /LANG=${INSTALLER_LANGUAGE_ID} "LegalCopyright" "© Contaware.com"
VIAddVersionKey /LANG=${INSTALLER_LANGUAGE_ID} "FileDescription" "Installation Routine of ${APPNAME_NOEXT}"
VIAddVersionKey /LANG=${INSTALLER_LANGUAGE_ID} "FileVersion" "${APPVERSION}.0"
VIAddVersionKey /LANG=${INSTALLER_LANGUAGE_ID} "ProductVersion" "${APPVERSION}.0"

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
  System::Call 'kernel32::CreateMutex(i 0, i 0, t "${INSTALLERMUTEXNAME}") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 lbl_end
  MessageBox MB_OK|MB_ICONEXCLAMATION $(AlreadyRunning) /SD IDOK
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

Function un.KillApp

  Push $R0
  Push $R1
  ClearErrors
  
  ; Kill Application
  System::Call 'kernel32::CreateMutex(i 0, i 0, t "${APPMUTEXNAME}") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 lbl_end
  MessageBox MB_YESNO|MB_ICONQUESTION $(CloseAppPrompt) /SD IDNO IDYES KillApp
  goto KillAppAbort
KillApp:
  KillProcDLL::KillProc "${APPNAME_EXT}"
  StrCmp $R0 "0" KillApp 0
  StrCmp $R0 "603" 0 KillAppError
  Sleep 1500
  Goto lbl_end
KillAppError:
  MessageBox MB_OK|MB_ICONEXCLAMATION $(CloseAppError) /SD IDOK
KillAppAbort:
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

Function un.KillMicroApache

  Push $R0
  Push $R1
  ClearErrors

KillMApache:
  KillProcDLL::KillProc "mapache.exe"
  StrCmp $R0 "0" KillMApache 0
  Sleep 1500
  
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
  
  ; Uninstall service and stop apps
  DetailPrint $(UninstallingServiceMessage)
  nsExec::Exec '"$INSTDIR\ContaCamService.exe" -u'
  DetailPrint $(StoppingApplicationMessage)
  call un.KillApp
  call un.KillMicroApache
  
  ; Remove / Restore All File Associations
  
  StrCpy $FILEEXTENSION "bmp"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "jpg"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "jpeg"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "jpe"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "thm"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "pcx"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "emf"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "png"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "tif"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "tiff"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "jfx"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "gif"
  call un.RemoveFileAssociation
  
  StrCpy $FILEEXTENSION "aif"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "aiff"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "au"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "mid"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "rmi"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "mp3"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "wav"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "wma"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "cda"
  call un.RemoveFileAssociation
  
  StrCpy $FILEEXTENSION "avi"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "divx"
  call un.RemoveFileAssociation
  StrCpy $FILEEXTENSION "zip"
  call un.RemoveFileAssociation

  ; Firewall remove from the authorized list
  nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\${APPNAME_EXT}"
  nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\microapache\mapache.exe"

  ; Remove autostart registry value
  DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "${APPNAME_NOEXT}"
  
  ; Remove remaining registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_NOEXT}"
  DeleteRegKey HKLM "Software\Contaware\${APPNAME_NOEXT}"
  DeleteRegKey HKCU "Software\Contaware\${APPNAME_NOEXT}"
  DeleteRegKey HKCU "Software\Contaware\RemoteCamViewer"
  DeleteRegKey HKCR "Applications\${APPNAME_EXT}"
  
  ; Remove files and uninstaller
  Delete $INSTDIR\License.txt
  Delete $INSTDIR\History.txt
  Delete $INSTDIR\FullscreenBrowser.exe
  Delete $INSTDIR\Start.exe
  Delete $INSTDIR\${APPNAME_EXT}
  Delete $INSTDIR\NeroBurn.exe
  Delete $INSTDIR\ContaCamService.exe
  Delete $INSTDIR\ContaCamService.ini
  Delete $INSTDIR\ContaCamService.log
  Delete $INSTDIR\${UNINSTNAME_EXT}

  ; Removes Shortcuts from the Start Menu for All Users
  SetShellVarContext all
  Delete "$SMPROGRAMS\${APPNAME_NOEXT}\*.*"
  RMDir "$SMPROGRAMS\${APPNAME_NOEXT}"
  
  ; Remove directories used
  RMDir /r "$INSTDIR\ActiveX"
  RMDir /r "$INSTDIR\Tutorials"
  RMDir /r "$INSTDIR\microapache"
  RMDir "$INSTDIR"
  
  ; Remove application data directories for current user
  SetShellVarContext current
  RMDir /r "$APPDATA\Contaware\${APPNAME_NOEXT}"
  RMDir /r "$APPDATA\Contaware\FullscreenBrowser"
  RMDir "$APPDATA\Contaware"
  
  ; Refresh Icons
  call un.RefreshShellIcons

SectionEnd
