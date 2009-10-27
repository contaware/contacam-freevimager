Var WINVER
Var INSTALLTYPE
Var HAS_UAC
Function InitVersion

  Push $R0
  Push $R1
  ClearErrors
  
  ReadRegStr $R0 HKLM \
  "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion
 
  IfErrors 0 lbl_winnt
  
  ; we are not NT
  ReadRegStr $R0 HKLM \
  "SOFTWARE\Microsoft\Windows\CurrentVersion" VersionNumber
 
  StrCpy $R1 $R0 1
  StrCmp $R1 '4' 0 lbl_error
 
  StrCpy $R1 $R0 3
 
  StrCmp $R1 '4.0' lbl_win32_95
  StrCmp $R1 '4.9' lbl_win32_ME lbl_win32_98
 
lbl_win32_95:
  StrCpy $WINVER '95'
  StrCpy $INSTALLTYPE 'ASCII'
  StrCpy $HAS_UAC 'FALSE'
  Goto lbl_done
 
lbl_win32_98:
  StrCpy $WINVER '98'
  StrCpy $INSTALLTYPE 'ASCII'
  StrCpy $HAS_UAC 'FALSE'
  Goto lbl_done
 
lbl_win32_ME:
  StrCpy $WINVER 'ME'
  StrCpy $INSTALLTYPE 'ASCII'
  StrCpy $HAS_UAC 'FALSE'
  Goto lbl_done
 
lbl_winnt:
 
  StrCpy $R1 $R0 1
 
  StrCmp $R1 '3' lbl_winnt_x
  StrCmp $R1 '4' lbl_winnt_x
  StrCmp $R1 '6' lbl_winnt_vista
 
  StrCpy $R1 $R0 3
 
  StrCmp $R1 '5.0' lbl_winnt_2000
  StrCmp $R1 '5.1' lbl_winnt_XP
  StrCmp $R1 '5.2' lbl_winnt_2003 lbl_winnt_new
 
lbl_winnt_x:
  StrCpy $WINVER "NT $R0" 6
  StrCpy $INSTALLTYPE 'ASCII'
  StrCpy $HAS_UAC 'FALSE'
  Goto lbl_done
 
lbl_winnt_2000:
  StrCpy $WINVER '2000'
  StrCpy $INSTALLTYPE 'UNICODE'
  StrCpy $HAS_UAC 'FALSE'
  Goto lbl_done
 
lbl_winnt_XP:
  StrCpy $WINVER 'XP'
  StrCpy $INSTALLTYPE 'UNICODE'
  StrCpy $HAS_UAC 'FALSE'
  Goto lbl_done
 
lbl_winnt_2003:
  StrCpy $WINVER '2003'
  StrCpy $INSTALLTYPE 'UNICODE'
  StrCpy $HAS_UAC 'FALSE'
  Goto lbl_done
 
lbl_winnt_vista:
  StrCpy $WINVER 'Vista'
  StrCpy $INSTALLTYPE 'UNICODE'
  StrCpy $HAS_UAC 'TRUE'
  Goto lbl_done
    
lbl_winnt_new:
  StrCpy $WINVER 'NewNT'
  StrCpy $INSTALLTYPE 'UNICODE'
  StrCpy $HAS_UAC 'TRUE'
  Goto lbl_done
    
lbl_error:
  StrCpy $WINVER 'Unknown'
  StrCpy $INSTALLTYPE 'ASCII'
  StrCpy $HAS_UAC 'FALSE'
  
lbl_done:
  Pop $R1
  Pop $R0
  
FunctionEnd


Function un.InitVersion

  Push $R0
  Push $R1
  ClearErrors
  
  ReadRegStr $R0 HKLM \
  "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion
 
  IfErrors 0 lbl_winnt
  
  ; we are not NT
  ReadRegStr $R0 HKLM \
  "SOFTWARE\Microsoft\Windows\CurrentVersion" VersionNumber
 
  StrCpy $R1 $R0 1
  StrCmp $R1 '4' 0 lbl_error
 
  StrCpy $R1 $R0 3
 
  StrCmp $R1 '4.0' lbl_win32_95
  StrCmp $R1 '4.9' lbl_win32_ME lbl_win32_98
 
lbl_win32_95:
  StrCpy $WINVER '95'
  StrCpy $INSTALLTYPE 'ASCII'
  StrCpy $HAS_UAC 'FALSE'
  Goto lbl_done
 
lbl_win32_98:
  StrCpy $WINVER '98'
  StrCpy $INSTALLTYPE 'ASCII'
  StrCpy $HAS_UAC 'FALSE'
  Goto lbl_done
 
lbl_win32_ME:
  StrCpy $WINVER 'ME'
  StrCpy $INSTALLTYPE 'ASCII'
  StrCpy $HAS_UAC 'FALSE'
  Goto lbl_done
 
lbl_winnt:
 
  StrCpy $R1 $R0 1
 
  StrCmp $R1 '3' lbl_winnt_x
  StrCmp $R1 '4' lbl_winnt_x
  StrCmp $R1 '6' lbl_winnt_vista
 
  StrCpy $R1 $R0 3
 
  StrCmp $R1 '5.0' lbl_winnt_2000
  StrCmp $R1 '5.1' lbl_winnt_XP
  StrCmp $R1 '5.2' lbl_winnt_2003 lbl_winnt_new
 
lbl_winnt_x:
  StrCpy $WINVER "NT $R0" 6
  StrCpy $INSTALLTYPE 'ASCII'
  StrCpy $HAS_UAC 'FALSE'
  Goto lbl_done
 
lbl_winnt_2000:
  StrCpy $WINVER '2000'
  StrCpy $INSTALLTYPE 'UNICODE'
  StrCpy $HAS_UAC 'FALSE'
  Goto lbl_done
 
lbl_winnt_XP:
  StrCpy $WINVER 'XP'
  StrCpy $INSTALLTYPE 'UNICODE'
  StrCpy $HAS_UAC 'FALSE'
  Goto lbl_done
 
lbl_winnt_2003:
  StrCpy $WINVER '2003'
  StrCpy $INSTALLTYPE 'UNICODE'
  StrCpy $HAS_UAC 'FALSE'
  Goto lbl_done
 
lbl_winnt_vista:
  StrCpy $WINVER 'Vista'
  StrCpy $INSTALLTYPE 'UNICODE'
  StrCpy $HAS_UAC 'TRUE'
  Goto lbl_done
    
lbl_winnt_new:
  StrCpy $WINVER 'NewNT'
  StrCpy $INSTALLTYPE 'UNICODE'
  StrCpy $HAS_UAC 'TRUE'
  Goto lbl_done
    
lbl_error:
  StrCpy $WINVER 'Unknown'
  StrCpy $INSTALLTYPE 'ASCII'
  StrCpy $HAS_UAC 'FALSE'
  
lbl_done:
  Pop $R1
  Pop $R0
  
FunctionEnd