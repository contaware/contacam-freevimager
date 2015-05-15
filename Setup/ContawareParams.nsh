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
Var AIF
Var AU
Var MID
Var MP3
Var WAV
Var WMA
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
  ${GetOptions} $R0 /aif= $AIF
  ${GetOptions} $R0 /au= $AU
  ${GetOptions} $R0 /mid= $MID
  ${GetOptions} $R0 /mp3= $MP3
  ${GetOptions} $R0 /wav= $WAV
  ${GetOptions} $R0 /wma= $WMA
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
  StrCmp $AIF "" 0 +2
  StrCpy $AIF "1"
  StrCmp $AU "" 0 +2
  StrCpy $AU "1"
  StrCmp $MID "" 0 +2
  StrCpy $MID "1"
  StrCmp $MP3 "" 0 +2
  StrCpy $MP3 "1"
  StrCmp $WAV "" 0 +2
  StrCpy $WAV "1"
  StrCmp $WMA "" 0 +2
  StrCpy $WMA "1"
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
  StrCmp $AIF "" 0 +2
  StrCpy $AIF "0"
  StrCmp $AU "" 0 +2
  StrCpy $AU "0"
  StrCmp $MID "" 0 +2
  StrCpy $MID "0"
  StrCmp $MP3 "" 0 +2
  StrCpy $MP3 "0"
  StrCmp $WAV "" 0 +2
  StrCpy $WAV "0"
  StrCmp $WMA "" 0 +2
  StrCpy $WMA "0"
all_end:
  
FunctionEnd