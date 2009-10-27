/*
=======================
UAC helper include file
.......................

Macros starting with UAC.I should only be called from the installer and vice versa for UAC.U macros.

*/
!ifndef UAC_HDR__INC
!define UAC_HDR__INC
/*!macro _UAC.BuildOnInitElevationFunc _funcprefix
Function ${_funcprefix}onInit
!ifmacrodef
FunctionEnd
!macroend*/

!macro _UAC.GenerateSimpleFunction _funcprefix _funcName _funcCode
Function ${_funcprefix}${_funcName}
${_funcCode}
FunctionEnd
!macroend

!macro _UAC.TryDef _d _v
!ifndef ${_d}
!define ${_d} "${_v}"
!endif
!macroend

!macro _UAC.InitStrings _modeprefix
!insertmacro _UAC.TryDef UACSTR.UnDataFile "UAC.dat"
!insertmacro _UAC.TryDef UACSTR.${_modeprefix}ElvWinErr "Unable to elevate , error $0"
!ifNdef __UNINSTALL__
	!insertmacro _UAC.TryDef UACSTR.${_modeprefix}ElvAbortReqAdmin "This installer requires admin access, aborting!"
	!insertmacro _UAC.TryDef UACSTR.${_modeprefix}ElvMustTryAgain "This installer requires admin access, try again"
	!else
	!insertmacro _UAC.TryDef UACSTR.${_modeprefix}ElvAbortReqAdmin "This uninstaller requires admin access, aborting!"
	!insertmacro _UAC.TryDef UACSTR.${_modeprefix}ElvMustTryAgain "This uninstaller requires admin access, try again"
	!endif
!macroend

!ifmacroNdef _UAC.GenerateUninstallerTango
!macro _UAC.GenerateUninstallerTango UninstallerFileName
!ifdef __GLOBAL__
	!error "UAC: Needs to be called inside a function"
	!endif
!ifNdef __UNINSTALL__
	!error "UAC: _UAC.GenerateUninstallerTango should only be called by uninstaller, see http://forums.winamp.com/showthread.php?threadid=280330"
	!endif
!include LogicLib.nsh
!insertmacro _UAC.InitStrings 'U.'
ReadIniStr $0 "$ExeDir\${UACSTR.UnDataFile}" UAC "Un.Ready"
${IF} $0 != 1
;TODO: ${AND} WinVer >= 6
	InitPluginsDir
	WriteIniStr "$PluginsDir\${UACSTR.UnDataFile}" UAC "Un.Ready" 1
	CopyFiles /SILENT "$EXEPATH" "$PluginsDir\${UninstallerFileName}"
	StrCpy $0 ""
	${IfThen} ${Silent} ${|} StrCpy $0 "/S " ${|}
	ExecWait '"$PluginsDir\${UninstallerFileName}" $0/NCRC _?=$INSTDIR' $0
	SetErrorLevel $0
	Quit
	${EndIf}
!macroend
!endif

!ifmacroNdef _UAC.GenerateOnInitElevationCode
!macro _UAC.GenerateOnInitElevationCode _modeprefix
!ifndef __FUNCTION__
	!error "UAC: Needs to be called inside a function"
	!endif
!insertmacro _UAC.InitStrings ${_modeprefix}
UAC_Elevate:
UAC::RunElevated 
StrCmp 1223 $0 UAC_ElevationAborted ; UAC dialog aborted by user?
StrCmp 0 $0 0 UAC_Err ; Error?
StrCmp 1 $1 0 UAC_Success ;Are we the real deal or just the wrapper?
Quit
UAC_Err:
MessageBox MB_OK|MB_ICONSTOP "${UACSTR.${_modeprefix}ElvWinErr}" /SD IDOK
Abort
UAC_ElevationAborted:
MessageBox MB_OK|MB_ICONSTOP "${UACSTR.${_modeprefix}ElvAbortReqAdmin}" /SD IDOK
Abort
UAC_Success:
# if $0==0 && $3==1, we are a member of the admin group (Any OS)
# if $0==0 && $1==0, UAC not supported (Probably <NT6), run as normal?
# if $0==0 && $1==3, we can try to elevate again
StrCmp 1 $3 /*+4*/ UAC_Done ;Admin?
StrCmp 3 $1 0 UAC_ElevationAborted ;Try again or abort?
MessageBox MB_OK|MB_ICONEXCLAMATION "${UACSTR.${_modeprefix}ElvMustTryAgain}" /SD IDOK
goto UAC_Elevate ;...lets try again
UAC_Done:
!macroend
!endif

!define UAC.Unload 'UAC::Unload'

!define UAC.I.Elevate.AdminOnly '!insertmacro UAC.I.Elevate.AdminOnly '
!macro UAC.I.Elevate.AdminOnly
!insertmacro _UAC.GenerateOnInitElevationCode 'I.'
!macroend

!define UAC.U.Elevate.AdminOnly '!insertmacro UAC.U.Elevate.AdminOnly '
!macro UAC.U.Elevate.AdminOnly _UninstallerName
!insertmacro _UAC.GenerateUninstallerTango "${_UninstallerName}"
!insertmacro _UAC.GenerateOnInitElevationCode 'U.'
!macroend

!define UAC.AutoCodeUnload '!insertmacro UAC.AutoCodeUnload '
!macro UAC.AutoCodeUnload _HasUninstaller
!insertmacro _UAC.GenerateSimpleFunction "" .OnInstFailed '${UAC.Unload}'
!insertmacro _UAC.GenerateSimpleFunction "" .OnInstSuccess '${UAC.Unload}'
!if "${_HasUninstaller}" != ""
	!insertmacro _UAC.GenerateSimpleFunction "un" .onUninstFailed '${UAC.Unload}'
	!insertmacro _UAC.GenerateSimpleFunction "un" .onUninstSuccess '${UAC.Unload}'
	!endif
!macroend

!define UAC.AutoCodeAdmin '!insertmacro UAC.AutoCodeAdmin '
!macro UAC.AutoCodeAdmin
!macroend

!define UAC.FastCallFunctionAsUser '!insertmacro UAC.FastCallFunctionAsUser '
!macro UAC.FastCallFunctionAsUser _func _var
GetFunctionAddress ${_var} ${_func}
UAC::ExecCodeSegment ${_var}
!macroend
!define UAC.CallFunctionAsUser '!insertmacro UAC.CallFunctionAsUser '
!macro UAC.CallFunctionAsUser _func
push $R9
!insertmacro UAC.FastCallFunctionAsUser ${_func} $R9
pop $R9
!macroend


!macro _UAC.DumpEx _disp _f _fp _v
${_f} ${_fp}
DetailPrint "${_disp}=${_v}"
!macroend
!macro _UAC.Dump _f _fp _v
!insertmacro _UAC.DumpEx `${_f}` `${_f}` `${_fp}` `${_v}`
!macroend
!macro _UAC.DbgDetailPrint
!insertmacro _UAC.DumpEx "User" System::Call "advapi32::GetUserName(t.r0,*i${NSIS_MAX_STRLEN})" $0
!insertmacro _UAC.DumpEx "CmdLine" "" "" "$CmdLine"
!insertmacro _UAC.Dump UAC::IsAdmin "" $0
!insertmacro _UAC.Dump UAC::SupportsUAC "" $0
!insertmacro _UAC.Dump UAC::GetElevationType "" $0
!macroend

!endif /* ifndef UAC_HDR__INC */