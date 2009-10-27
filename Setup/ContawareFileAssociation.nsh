; The Extension to add or remove like: jpg, tif, gif, ...
Var FILEEXTENSION

; Internal vars used by the functions
Var CURRENT_GLOBALLEVEL_FILECLASSNAME
Var CURRENT_PROGIDUSERLEVEL
Var CURRENT_APPLICATIONUSERLEVEL
Var PREVIOUS_GLOBALLEVEL_FILECLASSNAME
Var PREVIOUS_PROGIDUSERLEVEL
Var PREVIOUS_APPLICATIONUSERLEVEL
Var MYFILECLASSNAME


; Add
Function AddFileAssociation
  ReadRegStr $CURRENT_GLOBALLEVEL_FILECLASSNAME HKCR ".$FILEEXTENSION" ""
  ReadRegStr $CURRENT_PROGIDUSERLEVEL HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.$FILEEXTENSION" "ProgID"
  ReadRegStr $CURRENT_APPLICATIONUSERLEVEL HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.$FILEEXTENSION" "Application"
  StrCpy $MYFILECLASSNAME "${APPNAME_NOEXT}.$FILEEXTENSION.1"
  
  ; Do We Own the extension at Global Level? 
  StrCmp $CURRENT_GLOBALLEVEL_FILECLASSNAME $MYFILECLASSNAME UserLevelProgID 0
    ; Store Previous File Association in my UninstallClassNameDefault for later Restore
    WriteRegStr HKCU "Software\Contaware\${APPNAME_NOEXT}\UninstallClassNameDefault" $FILEEXTENSION $CURRENT_GLOBALLEVEL_FILECLASSNAME
	
	; Register File Extension
	WriteRegStr HKCR ".$FILEEXTENSION" "" $MYFILECLASSNAME
	
	; Write the Old Value to the Open With ProgIDs,
	; this List contains alternate Class Names
	; that can open the file extension
	StrCmp $CURRENT_GLOBALLEVEL_FILECLASSNAME "" UserLevelProgID 0
	  WriteRegStr HKCR ".$FILEEXTENSION\OpenWithProgids" $CURRENT_GLOBALLEVEL_FILECLASSNAME ""
    
UserLevelProgID:
  StrCmp $CURRENT_PROGIDUSERLEVEL MYFILECLASSNAME UserLevelApplication 0
    ; Store Previous File Association in my UninstallUserProgID for Restore
    WriteRegStr HKCU "Software\Contaware\${APPNAME_NOEXT}\UninstallUserProgID" $FILEEXTENSION $CURRENT_PROGIDUSERLEVEL
  
    ; Register File Extension
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.$FILEEXTENSION" "ProgID" $MYFILECLASSNAME
	
	; Write the Old Value to the Open With ProgIDs,
	; this List contains alternate Class Names
	; that can open the file extension
	StrCmp $CURRENT_PROGIDUSERLEVEL "" UserLevelApplication 0
	  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.$FILEEXTENSION\OpenWithProgids" $CURRENT_PROGIDUSERLEVEL ""
      
UserLevelApplication:
  StrCmp $CURRENT_APPLICATIONUSERLEVEL "${APPNAME_EXT}" ShellCommands 0
    ; Store Previous File Association in my UninstallUserApplication for Restore
    WriteRegStr HKCU "Software\Contaware\${APPNAME_NOEXT}\UninstallUserApplication" $FILEEXTENSION $CURRENT_APPLICATIONUSERLEVEL
  
    ; Register File Extension
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.$FILEEXTENSION" "Application" "${APPNAME_EXT}"
  
ShellCommands:
    ; Description, Shown In Explorer
	WriteRegStr HKCR $MYFILECLASSNAME "" "${APPNAME_NOEXT} $FILEEXTENSION file"
	
	; Shell Open Command, Icon order
	;IDR_MAINFRAME			0
	;IDR_AUDIOMCI			1
	;IDR_CDAUDIO			2
	;IDR_VIDEOAVI			3
	;IDR_PICTURE			4
	;IDR_PICTURE_NOHQ		5
	;IDR_BIGPICTURE			6
	;IDR_BIGPICTURE_NOHQ	7
	;IDI_ZIP				8

; Zip
StrCmp $FILEEXTENSION "zip" 0 ZipExtensionEnd
  WriteRegStr HKCR "$MYFILECLASSNAME\DefaultIcon" "" "$INSTDIR\${APPNAME_EXT},8"
  WriteRegStr HKCR "$MYFILECLASSNAME\shell\open\command" "" '"$INSTDIR\${APPNAME_EXT}" "%1"'
  WriteRegStr HKCR "$MYFILECLASSNAME\shell\Extract Here\command" "" '"$INSTDIR\${APPNAME_EXT}" /extracthere "%1"'
  Return
ZipExtensionEnd:

; Audio
StrCmp $FILEEXTENSION "mp3" AudioExtension 0
StrCmp $FILEEXTENSION "wav" AudioExtension 0
StrCmp $FILEEXTENSION "wma" AudioExtension 0
StrCmp $FILEEXTENSION "mid" AudioExtension 0
StrCmp $FILEEXTENSION "rmi" AudioExtension 0
StrCmp $FILEEXTENSION "au" AudioExtension 0
StrCmp $FILEEXTENSION "aif" AudioExtension 0
StrCmp $FILEEXTENSION "aiff" AudioExtension AudioExtensionEnd
AudioExtension:
  WriteRegStr HKCR "$MYFILECLASSNAME\DefaultIcon" "" "$INSTDIR\${APPNAME_EXT},1"
  WriteRegStr HKCR "$MYFILECLASSNAME\shell\open\command" "" '"$INSTDIR\${APPNAME_EXT}" "%1"'
  Return
AudioExtensionEnd:

; Audio CD
StrCmp $FILEEXTENSION "cda" 0 AudioCDExtensionEnd
  WriteRegStr HKCR "$MYFILECLASSNAME\DefaultIcon" "" "$INSTDIR\${APPNAME_EXT},2"
  WriteRegStr HKCR "$MYFILECLASSNAME\shell\open\command" "" '"$INSTDIR\${APPNAME_EXT}" "%1"'
  Return
AudioCDExtensionEnd:

; Video
StrCmp $FILEEXTENSION "avi" VideoExtension 0
StrCmp $FILEEXTENSION "divx" VideoExtension VideoExtensionEnd
VideoExtension:
  WriteRegStr HKCR "$MYFILECLASSNAME\DefaultIcon" "" "$INSTDIR\${APPNAME_EXT},3"
  WriteRegStr HKCR "$MYFILECLASSNAME\shell\open\command" "" '"$INSTDIR\${APPNAME_EXT}" "%1"'
  Return
VideoExtensionEnd:

; Graphics
  WriteRegStr HKCR "$MYFILECLASSNAME\DefaultIcon" "" "$INSTDIR\${APPNAME_EXT},4"
  WriteRegStr HKCR "$MYFILECLASSNAME\shell\open\command" "" '"$INSTDIR\${APPNAME_EXT}" "%1"'
  WriteRegStr HKCR "$MYFILECLASSNAME\shell\edit\command" "" '"$INSTDIR\${APPNAME_EXT}" "%1"'
  WriteRegStr HKCR "$MYFILECLASSNAME\shell\print\command" "" '"$INSTDIR\${APPNAME_EXT}" /p "%1"'
  WriteRegStr HKCR "$MYFILECLASSNAME\shell\printto\command" "" '"$INSTDIR\${APPNAME_EXT}" /pt "%1"'
  WriteRegStr HKCR "$MYFILECLASSNAME\shell\preview\command" "" '"$INSTDIR\${APPNAME_EXT}" "%1"'
FunctionEnd


; Remove
Function RemoveFileAssociation
  ReadRegStr $CURRENT_GLOBALLEVEL_FILECLASSNAME HKCR ".$FILEEXTENSION" ""
  ReadRegStr $CURRENT_PROGIDUSERLEVEL HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.$FILEEXTENSION" "ProgID"
  ReadRegStr $CURRENT_APPLICATIONUSERLEVEL HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.$FILEEXTENSION" "Application"
  ReadRegStr $PREVIOUS_GLOBALLEVEL_FILECLASSNAME HKCU "Software\Contaware\${APPNAME_NOEXT}\UninstallClassNameDefault" $FILEEXTENSION
  ReadRegStr $PREVIOUS_PROGIDUSERLEVEL HKCU "Software\Contaware\${APPNAME_NOEXT}\UninstallUserProgID" $FILEEXTENSION
  ReadRegStr $PREVIOUS_APPLICATIONUSERLEVEL HKCU "Software\Contaware\${APPNAME_NOEXT}\UninstallUserApplication" $FILEEXTENSION
  StrCpy $MYFILECLASSNAME "${APPNAME_NOEXT}.$FILEEXTENSION.1"
  
  ; Do We Own the extension at Global Level? 
  StrCmp $CURRENT_GLOBALLEVEL_FILECLASSNAME $MYFILECLASSNAME 0 UserLevelProgID
    ; If Previous File Class Name Not Set -> Delete Key
    StrCmp $PREVIOUS_GLOBALLEVEL_FILECLASSNAME "" 0 RestoreGlobalLevelFileClassName
      DeleteRegKey HKCR ".$FILEEXTENSION"
      GOTO UserLevelProgID
    ; Else Restore Key
    RestoreGlobalLevelFileClassName:
      WriteRegStr HKCR ".$FILEEXTENSION" "" $PREVIOUS_GLOBALLEVEL_FILECLASSNAME
    
UserLevelProgID: ; ProgID and Class Name are to words to name the same thing... 
  ; Do We Own the extension at ProgID User Level? 
  StrCmp $CURRENT_PROGIDUSERLEVEL $MYFILECLASSNAME 0 UserLevelApplication
    ; If Previous ProgID Not Set -> Delete Key
    StrCmp $PREVIOUS_PROGIDUSERLEVEL "" 0 RestoreUserLevelProgID
      DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.$FILEEXTENSION" "ProgID"
      GOTO UserLevelApplication
    ; Else Restore Key
    RestoreUserLevelProgID:
      WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.$FILEEXTENSION" "ProgID" $PREVIOUS_PROGIDUSERLEVEL
      
UserLevelApplication:
  ; Do We Own the extension at Application User Level? 
  StrCmp $CURRENT_APPLICATIONUSERLEVEL "${APPNAME_EXT}" 0 DelMyClassName
    ; If Previous Application Name Not Set -> Delete Key
    StrCmp $PREVIOUS_APPLICATIONUSERLEVEL "" 0 RestoreUserLevelApplication
      DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.$FILEEXTENSION" "Application"
      GOTO DelMyClassName
    ; Else Restore Key
    RestoreUserLevelApplication:
      WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.$FILEEXTENSION" "Application" $PREVIOUS_APPLICATIONUSERLEVEL    
      
  ; Delete Our Class
DelMyClassName:
  DeleteRegKey HKCR $MYFILECLASSNAME 
FunctionEnd
Function un.RemoveFileAssociation
  ReadRegStr $CURRENT_GLOBALLEVEL_FILECLASSNAME HKCR ".$FILEEXTENSION" ""
  ReadRegStr $CURRENT_PROGIDUSERLEVEL HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.$FILEEXTENSION" "ProgID"
  ReadRegStr $CURRENT_APPLICATIONUSERLEVEL HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.$FILEEXTENSION" "Application"
  ReadRegStr $PREVIOUS_GLOBALLEVEL_FILECLASSNAME HKCU "Software\Contaware\${APPNAME_NOEXT}\UninstallClassNameDefault" $FILEEXTENSION
  ReadRegStr $PREVIOUS_PROGIDUSERLEVEL HKCU "Software\Contaware\${APPNAME_NOEXT}\UninstallUserProgID" $FILEEXTENSION
  ReadRegStr $PREVIOUS_APPLICATIONUSERLEVEL HKCU "Software\Contaware\${APPNAME_NOEXT}\UninstallUserApplication" $FILEEXTENSION
  StrCpy $MYFILECLASSNAME "${APPNAME_NOEXT}.$FILEEXTENSION.1"
  
  ; Do We Own the extension at Global Level? 
  StrCmp $CURRENT_GLOBALLEVEL_FILECLASSNAME $MYFILECLASSNAME 0 UserLevelProgID
    ; If Previous File Class Name Not Set -> Delete Key
    StrCmp $PREVIOUS_GLOBALLEVEL_FILECLASSNAME "" 0 RestoreGlobalLevelFileClassName
      DeleteRegKey HKCR ".$FILEEXTENSION"
      GOTO UserLevelProgID
    ; Else Restore Key
    RestoreGlobalLevelFileClassName:
      WriteRegStr HKCR ".$FILEEXTENSION" "" $PREVIOUS_GLOBALLEVEL_FILECLASSNAME
    
UserLevelProgID: ; ProgID and Class Name are to words to name the same thing... 
  ; Do We Own the extension at ProgID User Level? 
  StrCmp $CURRENT_PROGIDUSERLEVEL $MYFILECLASSNAME 0 UserLevelApplication
    ; If Previous ProgID Not Set -> Delete Key
    StrCmp $PREVIOUS_PROGIDUSERLEVEL "" 0 RestoreUserLevelProgID
      DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.$FILEEXTENSION" "ProgID"
      GOTO UserLevelApplication
    ; Else Restore Key
    RestoreUserLevelProgID:
      WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.$FILEEXTENSION" "ProgID" $PREVIOUS_PROGIDUSERLEVEL
      
UserLevelApplication:
  ; Do We Own the extension at Application User Level? 
  StrCmp $CURRENT_APPLICATIONUSERLEVEL "${APPNAME_EXT}" 0 DelMyClassName
    ; If Previous Application Name Not Set -> Delete Key
    StrCmp $PREVIOUS_APPLICATIONUSERLEVEL "" 0 RestoreUserLevelApplication
      DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.$FILEEXTENSION" "Application"
      GOTO DelMyClassName
    ; Else Restore Key
    RestoreUserLevelApplication:
      WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.$FILEEXTENSION" "Application" $PREVIOUS_APPLICATIONUSERLEVEL    
      
  ; Delete Our Class
DelMyClassName:
  DeleteRegKey HKCR $MYFILECLASSNAME 
FunctionEnd


!macro ADDREMOVE_FILEASSOCIATION paramvar paramext
  StrCmp ${paramvar} "1" 0 +4
  StrCpy $FILEEXTENSION "${paramext}"
  call AddFileAssociation
  goto +4
  StrCmp ${paramvar} "0" 0 +3 
  StrCpy $FILEEXTENSION "${paramext}"
  call RemoveFileAssociation
!macroend
