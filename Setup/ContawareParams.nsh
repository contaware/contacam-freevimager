!include "FileFunc.nsh"
!insertmacro GetParameters
!insertmacro GetOptions
Var BMP
Var JPG
Var PCX
Var EMF
Var PNG
Var TIF
Var GIF
Var ALL
Var KILL
Function InitParams
  ${GetParameters} $R0
  ClearErrors
  ${GetOptions} $R0 /bmp= $BMP
  ${GetOptions} $R0 /jpg= $JPG
  ${GetOptions} $R0 /pcx= $PCX
  ${GetOptions} $R0 /emf= $EMF
  ${GetOptions} $R0 /png= $PNG
  ${GetOptions} $R0 /tif= $TIF
  ${GetOptions} $R0 /gif= $GIF
  ${GetOptions} $R0 /all= $ALL
  ${GetOptions} $R0 /kill= $KILL
  StrCmp $ALL "1" 0 all_off
  StrCmp $BMP "" 0 +2
  StrCpy $BMP "1"
  StrCmp $JPG "" 0 +2
  StrCpy $JPG "1"
  StrCmp $PCX "" 0 +2
  StrCpy $PCX "1"
  StrCmp $EMF "" 0 +2
  StrCpy $EMF "1"
  StrCmp $PNG "" 0 +2
  StrCpy $PNG "1"
  StrCmp $TIF "" 0 +2
  StrCpy $TIF "1"
  StrCmp $GIF "" 0 +2
  StrCpy $GIF "1"
  goto all_end
all_off:
  StrCmp $ALL "0" 0 all_end
  StrCmp $BMP "" 0 +2
  StrCpy $BMP "0"
  StrCmp $JPG "" 0 +2
  StrCpy $JPG "0"
  StrCmp $PCX "" 0 +2
  StrCpy $PCX "0"
  StrCmp $EMF "" 0 +2
  StrCpy $EMF "0"
  StrCmp $PNG "" 0 +2
  StrCpy $PNG "0"
  StrCmp $TIF "" 0 +2
  StrCpy $TIF "0"
  StrCmp $GIF "" 0 +2
  StrCpy $GIF "0"
all_end:
  
FunctionEnd