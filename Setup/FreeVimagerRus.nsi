; Adds a user (no admin) manifest (for vista or higher)
RequestExecutionLevel user

; Type Of Installer: define both to be compatible with all systems!
;!define INSTALLER_WIN9X
!define INSTALLER_NT
;!define WITH_TUTORIALS

; Name Defines
!define APPNAME_EXT "FreeVimager.exe"
!define APPNAME_NOEXT "FreeVimager"
!define UNINSTNAME_EXT "uninstall.exe"
!define UNINSTNAME_LNK "Uninstall.lnk"

; Mutex Name Define
; Remenber to change the Application Mutex
; Name in the C++ Source!
!define APPMUTEXNAME "${APPNAME_NOEXT}AppMutex"
!define INSTALLERMUTEXNAME "${APPNAME_NOEXT}InstallerMutex"

; Application Version Define
!define APPVERSION "2.0.0"

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
OutFile "${APPNAME_NOEXT}-${APPVERSION}-Setup-Rus.exe"
!else
!ifdef INSTALLER_WIN9X
OutFile "${APPNAME_NOEXT}-${APPVERSION}-Setup-Rus.exe"
!else
OutFile "${APPNAME_NOEXT}-${APPVERSION}-Setup-Rus.exe"
!endif
!endif
!else
!ifdef INSTALLER_WIN9X & INSTALLER_NT
OutFile "${APPNAME_NOEXT}-${APPVERSION}-Setup-Rus-NoTutorials.exe"
!else
!ifdef INSTALLER_WIN9X
OutFile "${APPNAME_NOEXT}-${APPVERSION}-Setup-Rus-NoTutorials.exe"
!else
OutFile "${APPNAME_NOEXT}-${APPVERSION}-Setup-Rus-NoTutorials.exe"
!endif
!endif
!endif

; The default installation directory
InstallDir $PROGRAMFILES\${APPNAME_NOEXT}

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Contaware\${APPNAME_NOEXT}" "Install_Dir"

; Compressor
SetCompressor lzma

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
;!define MUI_ICON "icon.ico"
;!define MUI_UNICON "icon.ico"

;--------------------------------

; Page Modern UI
!insertmacro MUI_PAGE_LICENSE "..\License\LicenseFreeVimagerRus.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------

; Language File Modern UI,
; change also the Version Information language!
;!insertmacro MUI_LANGUAGE "English"
;!insertmacro MUI_LANGUAGE "Italian"
;!insertmacro MUI_LANGUAGE "French"
;!insertmacro MUI_LANGUAGE "German"
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
  
  ; Application Must not be Running
InstallCheckApplicationRunning:
  ClearErrors
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${APPMUTEXNAME}") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 lbl_check_exe_type
  StrCmp $INSTALLTYPE 'UNICODE' KillAppAsk 0 ; UNICODE is set for Win2000 or higher <-> KillProcesses works for Win2000 or higher
  MessageBox MB_OK|MB_ICONEXCLAMATION "Application is running. Close it and retry!" /SD IDOK
  goto KillAppAbort
KillAppAsk:
  StrCmp $KILL "1" KillApp 0      ; If param set -> kill without asking
  StrCmp $KILL "0" KillAppError 0 ; If param cleared -> do not kill
  ; If kill param not set -> ask (silent install answers no to the following question -> no killing):
  MessageBox MB_YESNO|MB_ICONQUESTION "Application is running.$\nDo you want me to close it and continue the installation?$\n(Choose No if you have some unsaved data left)" /SD IDNO IDYES KillApp
  goto KillAppAbort
KillApp:
  StrCpy $0 "${APPNAME_EXT}"
  KillProc::KillProcesses
  StrCmp $1 "-1" KillAppError
  Sleep 1500
  Goto lbl_check_exe_type
KillAppError:
  MessageBox MB_OK|MB_ICONEXCLAMATION "The Installer could not close the running application" /SD IDOK
KillAppAbort:
  ClearErrors
  Pop $R1
  Pop $R0
  ${UAC.Unload} ;Must call unload!
  Abort
    
  ; Check
lbl_check_exe_type:
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

Function FinishRunCB
UAC::Exec "" "$INSTDIR\${APPNAME_EXT}" "" ""
FunctionEnd

;--------------------------------

; The stuff to install
Section "${APPNAME_NOEXT} Program (required)"

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Section Read-Only (=User Cannot Change it's state)
  SectionIn RO
  
  ; Remove previous install files and directories
  Delete $INSTDIR\License.txt
  Delete $INSTDIR\History.txt
  Delete $INSTDIR\Start.exe
  Delete $INSTDIR\${APPNAME_EXT}
  Delete $INSTDIR\NeroBurn.exe
  Delete $INSTDIR\${UNINSTNAME_EXT}
  RMDir /r "$INSTDIR\Tutorials"
    
  ; Create Tutorials Directory
  CreateDirectory "$INSTDIR\Tutorials"
   
  ; Source Program File Path
!ifdef INSTALLER_WIN9X
  File "..\Bin\${APPNAME_NOEXT}\${APPNAME_EXT}"
!endif
!ifdef INSTALLER_NT
  File "..\Translation\${APPNAME_NOEXT}wRus.exe"
!endif
  File "/oname=License.txt" "..\License\LicenseFreeVimagerRus.txt"
  File "/oname=History.txt" "..\History\HistoryFreeVimager.txt"
  File "..\NeroBurn\Release\NeroBurn.exe"
!ifdef WITH_TUTORIALS
  File "/oname=Tutorials\Basics.htm" "..\Tutorials\Basics.htm"
  File "/oname=Tutorials\Basics.swf" "..\Tutorials\Basics.swf"
  File "/oname=Tutorials\Basics.js" "..\Tutorials\Basics.js"
  File "/oname=Tutorials\Batch_Processing.htm" "..\Tutorials\Batch_Processing.htm"
  File "/oname=Tutorials\Batch_Processing.swf" "..\Tutorials\Batch_Processing.swf"
  File "/oname=Tutorials\Batch_Processing.js" "..\Tutorials\Batch_Processing.js"
  File "/oname=Tutorials\CD_Slideshow.htm" "..\Tutorials\CD_Slideshow.htm"
  File "/oname=Tutorials\CD_Slideshow.swf" "..\Tutorials\CD_Slideshow.swf"
  File "/oname=Tutorials\CD_Slideshow.js" "..\Tutorials\CD_Slideshow.js"
  File "/oname=Tutorials\Compositions.htm" "..\Tutorials\Compositions.htm"
  File "/oname=Tutorials\Compositions.swf" "..\Tutorials\Compositions.swf"
  File "/oname=Tutorials\Compositions.js" "..\Tutorials\Compositions.js"
  File "/oname=Tutorials\Redeye_Remove.htm" "..\Tutorials\Redeye_Remove.htm"
  File "/oname=Tutorials\Redeye_Remove.swf" "..\Tutorials\Redeye_Remove.swf"
  File "/oname=Tutorials\Redeye_Remove.js" "..\Tutorials\Redeye_Remove.js"
!endif
  
  ; Install Unicode?
  StrCmp $INSTALLTYPE 'UNICODE' unicode
!ifdef INSTALLER_NT
    Delete "$INSTDIR\${APPNAME_NOEXT}wRus.exe"
!endif
    goto unicode_end
unicode:
!ifdef INSTALLER_WIN9X & INSTALLER_NT
    Rename "$INSTDIR\${APPNAME_EXT}" "$INSTDIR\Start.exe"
!endif
!ifdef INSTALLER_NT
    Rename "$INSTDIR\${APPNAME_NOEXT}wRus.exe" "$INSTDIR\${APPNAME_EXT}"
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
  
  ; Set First Run Flag only if not in silent mode
  IfSilent +3
  WriteRegDWORD HKCU "Software\Contaware\${APPNAME_NOEXT}\GeneralApp" "FirstRun" 1
  goto +2
  WriteRegDWORD HKCU "Software\Contaware\${APPNAME_NOEXT}\GeneralApp" "FirstRun" 0
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_NOEXT}" "DisplayName" "${APPNAME_NOEXT}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_NOEXT}" "UninstallString" '"$INSTDIR\${UNINSTNAME_EXT}"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_NOEXT}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_NOEXT}" "NoRepair" 1
  WriteUninstaller "${UNINSTNAME_EXT}"
  
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
  Pop $R0
  ${UAC.Unload} ;Must call unload!
  Abort
  
  ; Application Must not be Running
UninstallCheckApplicationRunning:
  ClearErrors
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${APPMUTEXNAME}") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 lbl_end
  StrCmp $INSTALLTYPE 'UNICODE' KillAppAsk 0 ; UNICODE is set for Win2000 or higher <-> KillProcesses works for Win2000 or higher
  MessageBox MB_OK|MB_ICONEXCLAMATION "Application is running. Close it and retry!" /SD IDOK
  goto KillAppAbort
KillAppAsk:
  MessageBox MB_YESNO|MB_ICONQUESTION "Application is running.$\nDo you want me to close it and continue the uninstallation?$\n(Choose No if you have some unsaved data left)" /SD IDNO IDYES KillApp
  goto KillAppAbort
KillApp:
  StrCpy $0 "${APPNAME_EXT}"
  KillProc::KillProcesses
  StrCmp $1 "-1" KillAppError
  Sleep 1500
  Goto lbl_end
KillAppError:
  MessageBox MB_OK|MB_ICONEXCLAMATION "The Installer could not close the running application" /SD IDOK
KillAppAbort:
  ClearErrors
  Pop $R0
  ${UAC.Unload} ;Must call unload!
  Abort
    
lbl_end:
  Pop $R0
  
FunctionEnd

!define SHCNE_ASSOCCHANGED 0x08000000
!define SHCNF_IDLIST 0
Function un.RefreshShellIcons
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v \
  (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
FunctionEnd

Section "Uninstall"
  
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
  
  ; Remove autostart registry value
  DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "${APPNAME_NOEXT}"
  
  ; Remove remaining registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME_NOEXT}"
  DeleteRegKey HKLM "Software\Contaware\${APPNAME_NOEXT}"
  DeleteRegKey HKCU "Software\Contaware\${APPNAME_NOEXT}"
  DeleteRegKey HKCR "Applications\${APPNAME_EXT}"
  
  ; Remove files and uninstaller
  Delete $INSTDIR\License.txt
  Delete $INSTDIR\History.txt
  Delete $INSTDIR\Start.exe
  Delete $INSTDIR\${APPNAME_EXT}
  Delete $INSTDIR\NeroBurn.exe
  Delete $INSTDIR\${UNINSTNAME_EXT}

  ; Removes Shortcuts from the Start Menu for All Users
  SetShellVarContext all
  Delete "$SMPROGRAMS\${APPNAME_NOEXT}\*.*"
  RMDir "$SMPROGRAMS\${APPNAME_NOEXT}"
  
  ; Remove directories used
  RMDir /r "$INSTDIR\Tutorials"
  RMDir "$INSTDIR"
  
  ; Refresh Icons
  call un.RefreshShellIcons

SectionEnd
