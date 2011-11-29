; Adds a user (no admin) manifest (for vista or higher)
RequestExecutionLevel user

; Possible defines: INSTALLER_WIN9X, INSTALLER_NT, WITH_TUTORIALS
!ifndef INSTALLER_WIN9X & INSTALLER_NT
!define INSTALLER_NT
!endif

; Name Defines
!include "CurrentVersion.nsh"
!define APPNAME_EXT "ContaCam.exe"
!define APPNAME_NOEXT "ContaCam"
!define UNINSTNAME_EXT "uninstall.exe"
!define UNINSTNAME_LNK "Uninstall.lnk"

; Mutex Name Define
; Remenber to change the Application Mutex
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
!ifdef WITH_TUTORIALS
!ifdef INSTALLER_WIN9X & INSTALLER_NT
OutFile "${APPVERSION}\russian\${APPNAME_NOEXT}-${APPVERSION}-Setup-Rus.exe"
!else
!ifdef INSTALLER_WIN9X
OutFile "${APPVERSION}\russian\${APPNAME_NOEXT}-${APPVERSION}-Setup-Rus.exe"
!else
OutFile "${APPVERSION}\russian\${APPNAME_NOEXT}-${APPVERSION}-Setup-Rus.exe"
!endif
!endif
!else
!ifdef INSTALLER_WIN9X & INSTALLER_NT
OutFile "${APPVERSION}\russian\${APPNAME_NOEXT}-${APPVERSION}-Setup-Rus-NoTutorials.exe"
!else
!ifdef INSTALLER_WIN9X
OutFile "${APPVERSION}\russian\${APPNAME_NOEXT}-${APPVERSION}-Setup-Rus-NoTutorials.exe"
!else
OutFile "${APPVERSION}\russian\${APPNAME_NOEXT}-${APPVERSION}-Setup-Rus-NoTutorials.exe"
!endif
!endif
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
!insertmacro MUI_PAGE_LICENSE "..\License\License.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE PageFinLeave
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------

; Language File Modern UI,
; change also the Version Information language!
!insertmacro MUI_LANGUAGE "Russian"

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
; Start service and start ContaCam through service if Run App not checked (only for win2k or higher)
StrCmp $INSTALLTYPE 'UNICODE' startit
goto startend
startit:
nsExec::Exec '"$INSTDIR\ContaCamService.exe" -r'
SendMessage $mui.FinishPage.Run ${BM_GETCHECK} 0 0 $mui.FinishPage.ReturnValue
${if} $mui.FinishPage.ReturnValue = ${BST_UNCHECKED}
	nsExec::Exec '"$INSTDIR\ContaCamService.exe" -proc'
${endif}
startend:
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
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${INSTALLERMUTEXNAME}") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 InstallCheckApplicationRunning
  MessageBox MB_OK|MB_ICONEXCLAMATION "The Installer is already running" /SD IDOK
  ClearErrors
  Pop $R1
  Pop $R0
  ${UAC.Unload} ;Must call unload!
  Abort
  
  ; Application Running?
InstallCheckApplicationRunning:
  ClearErrors
  StrCmp $INSTALLTYPE 'UNICODE' InstallCheckApplicationRunningEnd 0 ; UNICODE is set for Win2000 or higher the later 
                                                                    ; executed KillApps works only for Win2000 or higher
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${APPMUTEXNAME}") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 InstallCheckApplicationRunningEnd													
  MessageBox MB_OK|MB_ICONEXCLAMATION "Application is running. Close it and retry!" /SD IDOK
  ClearErrors
  Pop $R1
  Pop $R0
  ${UAC.Unload} ;Must call unload!
  Abort
InstallCheckApplicationRunningEnd:
    
  ; OS version check
!ifndef INSTALLER_WIN9X
  StrCmp $INSTALLTYPE 'UNICODE' lbl_end
    MessageBox MB_OK|MB_ICONEXCLAMATION "This Installer works only on Win2000, XP and newer Systems" /SD IDOK
    Pop $R1
    Pop $R0
    ${UAC.Unload} ;Must call unload!
    Abort
lbl_end:
!endif
  
  Pop $R1
  Pop $R0
 
FunctionEnd

;--------------------------------
; Not working on systems older than Win2000

Function KillApps

  Push $R0
  Push $R1
  ClearErrors
  
  ; Kill Application
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${APPMUTEXNAME}") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 KillMicroApache
  StrCmp $KILL "1" KillApp 0      ; If param set -> kill without asking
  StrCmp $KILL "0" KillAppError 0 ; If param cleared -> do not kill
  ; If kill param not set -> ask (silent install answers no to the following question -> no killing):
  MessageBox MB_YESNO|MB_ICONQUESTION "Application is running.$\nDo you want me to close it and continue the installation?$\n(Choose No if you have some unsaved data left)" /SD IDNO IDYES KillApp
  goto KillAppAbort
KillApp:
  StrCpy $0 "${APPNAME_EXT}"
  KillProc::KillProcesses
  StrCmp $1 "-1" KillAppError
  Goto KillMicroApache
KillAppError:
  MessageBox MB_OK|MB_ICONEXCLAMATION "The Installer could not close the running application" /SD IDOK
KillAppAbort:
  ClearErrors
  Pop $R1
  Pop $R0
  ${UAC.Unload} ;Must call unload!
  Abort
  
  ; Kill Micro Apache
KillMicroApache:
  StrCpy $0 "mapache.exe"
  KillProc::KillProcesses
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
  
  ; Stop service and apps (only for win2k or higher)
  StrCmp $INSTALLTYPE 'UNICODE' stopit
    goto stopend
stopit:
  DetailPrint "Stopping service, please be patient..."
  nsExec::Exec '"$INSTDIR\ContaCamService.exe" -k'
  DetailPrint "Stopping application, please be patient..."
  call KillApps
stopend:

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
    
  ; Create Directories
  CreateDirectory "$INSTDIR\ActiveX"
  CreateDirectory "$INSTDIR\Tutorials"
  
  ; Source Program File Path
!ifdef INSTALLER_WIN9X
  File "..\Bin\${APPNAME_NOEXT}\${APPNAME_EXT}"
  File "/oname=FullscreenBrowser.exe" "..\FullscreenBrowser\Release\FullscreenBrowser.exe"
!endif
!ifdef INSTALLER_NT
  File "/oname=${APPNAME_NOEXT}w.exe" "..\Translation\${APPNAME_NOEXT}wRus.exe"
  File "/oname=FullscreenBrowserw.exe" "..\FullscreenBrowser\Release_Unicode\FullscreenBrowser.exe"
!endif
  File "/oname=License.txt" "..\License\License.txt"
  File "/oname=History.txt" "..\History\HistoryContaCam.txt"
  File "..\NeroBurn\Release\NeroBurn.exe"
  File "..\ContaCamService\Release\ContaCamService.exe"
  SetOverwrite off
  File "..\ContaCamService\Release\ContaCamService.ini"
  SetOverwrite on
  File "/oname=ActiveX\RemoteCam.htm" "..\ActiveX\RemoteCam.htm"
  File "/oname=ActiveX\RemoteCam.ocx" "..\ActiveX\RemoteCam.ocx"
  File "/oname=ActiveX\RemoteCamViewer.exe" "..\ActiveX\RemoteCamViewer.exe"
!ifdef WITH_TUTORIALS
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
!endif
  SetOutPath $INSTDIR\microapache
  File /r /x .svn /x configuration*.* "..\microapache\*.*"
  SetOutPath $INSTDIR
  File "/oname=microapache\htdocs\configuration.php" "..\microapache\htdocs\configuration_rus.php"
  
  ; Install Unicode?
  StrCmp $INSTALLTYPE 'UNICODE' unicode
!ifdef INSTALLER_NT
    Delete "$INSTDIR\${APPNAME_NOEXT}w.exe"
	Delete "$INSTDIR\FullscreenBrowserw.exe"
!endif
    goto unicode_end
unicode:
!ifdef INSTALLER_WIN9X & INSTALLER_NT
    Rename "$INSTDIR\${APPNAME_EXT}" "$INSTDIR\Start.exe"
	Delete "$INSTDIR\FullscreenBrowser.exe"
!endif
!ifdef INSTALLER_NT
    Rename "$INSTDIR\${APPNAME_NOEXT}w.exe" "$INSTDIR\${APPNAME_EXT}"
	Rename "$INSTDIR\FullscreenBrowserw.exe" "$INSTDIR\FullscreenBrowser.exe"
!endif
unicode_end:
  
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
  
  ; Register ocx
  RegDLL "$INSTDIR\ActiveX\RemoteCam.ocx"
  
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
VIAddVersionKey /LANG=${LANG_RUSSIAN} "ProductName" "${APPNAME_NOEXT} Application"
VIAddVersionKey /LANG=${LANG_RUSSIAN} "Comments" "MM Application"
VIAddVersionKey /LANG=${LANG_RUSSIAN} "CompanyName" "Contaware.com"
VIAddVersionKey /LANG=${LANG_RUSSIAN} "LegalTrademarks" "${APPNAME_NOEXT} is a trademark of Contaware.com"
VIAddVersionKey /LANG=${LANG_RUSSIAN} "LegalCopyright" "© Contaware.com"
VIAddVersionKey /LANG=${LANG_RUSSIAN} "FileDescription" "Installation Routine of ${APPNAME_NOEXT}"
VIAddVersionKey /LANG=${LANG_RUSSIAN} "FileVersion" "${APPVERSION}.0"

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
  StrCmp $R0 0 UninstallCheckApplicationRunning
  MessageBox MB_OK|MB_ICONEXCLAMATION "The Uninstaller is already running" /SD IDOK
  ClearErrors
  Pop $R1
  Pop $R0
  ${UAC.Unload} ;Must call unload!
  Abort
  
  ; Application Running?
UninstallCheckApplicationRunning:
  ClearErrors
  StrCmp $INSTALLTYPE 'UNICODE' UninstallCheckApplicationRunningEnd 0 ; UNICODE is set for Win2000 or higher the later 
                                                                      ; executed un.KillApps works only for Win2000 or higher
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${APPMUTEXNAME}") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 UninstallCheckApplicationRunningEnd
  MessageBox MB_OK|MB_ICONEXCLAMATION "Application is running. Close it and retry!" /SD IDOK
  ClearErrors
  Pop $R1
  Pop $R0
  ${UAC.Unload} ;Must call unload!
  Abort
UninstallCheckApplicationRunningEnd:
  
  Pop $R1
  Pop $R0
  
FunctionEnd

;--------------------------------
; Not working on systems older than Win2000

Function un.KillApps

  Push $R0
  Push $R1
  ClearErrors
  
  ; Kill Application
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${APPMUTEXNAME}") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 KillMicroApache
  MessageBox MB_YESNO|MB_ICONQUESTION "Application is running.$\nDo you want me to close it and continue the uninstallation?$\n(Choose No if you have some unsaved data left)" /SD IDNO IDYES KillApp
  goto KillAppAbort
KillApp:
  StrCpy $0 "${APPNAME_EXT}"
  KillProc::KillProcesses
  StrCmp $1 "-1" KillAppError
  Goto KillMicroApache
KillAppError:
  MessageBox MB_OK|MB_ICONEXCLAMATION "The Uninstaller could not close the running application" /SD IDOK
KillAppAbort:
  ClearErrors
  Pop $R1
  Pop $R0
  ${UAC.Unload} ;Must call unload!
  Abort
  
  ; Kill Micro Apache
KillMicroApache:
  StrCpy $0 "mapache.exe"
  KillProc::KillProcesses
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
  
  ; Uninstall service and stop apps (only for win2k or higher)
  StrCmp $INSTALLTYPE 'UNICODE' uninstallsrvandstopit
    goto uninstallsrvandstopitend
uninstallsrvandstopit:
  DetailPrint "Uninstalling service, please be patient..."
  nsExec::Exec '"$INSTDIR\ContaCamService.exe" -u'
  DetailPrint "Stopping application, please be patient..."
  call un.KillApps
uninstallsrvandstopitend:

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
  
  ; Unregister ocx
  UnRegDLL "$INSTDIR\ActiveX\RemoteCam.ocx"

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
