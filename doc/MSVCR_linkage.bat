@echo off

REM Static link:  /MT  -> libcmt.lib
REM  and Debug    /MTd -> libcmtd.lib
REM Dynamic link: /MD  -> msvcrt.lib  -> msvcrXXX.dll
REM  and Debug    /MDd -> msvcrtd.lib -> msvcrXXXd.dll
REM
REM Note that if you link your program from the command line without a compiler
REM option that specifies a C run-time library, the linker will use libcmt.lib.
REM
REM msvcrt.dll used to be the Visual C Run-Time Library for Visual C++ version 4.2 to 6.0,
REM versions of Visual C++ before 4.0 and since 7.0 have used differently named DLLs for
REM each version (MSVCR20.DLL, MSVCR70.DLL, MSVCR71.DLL, MSVCP110.DLL,...).
REM
REM The msvcrt.dll is now a "known DLL" meaning that it is a system component owned
REM and built by Windows. It is intended for future use only by system-level components
REM and is updated with each OS version. Some people succeeded in linking to it with
REM new Visual C++ versions:
REM http://www.syndicateofideas.com/posts/fighting-the-msvcrt-dll-hell
REM
REM To avoid any problems it is best to statically link to the C Run-Time Library.
REM The following command tests whether the passed file (.EXE, .DLL, .LIB)
REM is statically or dynamically linked.

REM Call Visual Studio Command Prompt
if exist "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" (
	call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" x86
) else if exist "%VS110COMNTOOLS%\..\..\VC\vcvarsall.bat" (
	call "%VS110COMNTOOLS%\..\..\VC\vcvarsall.bat" x86
) else if exist "%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat" (
	call "%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat" x86
) else (
	echo ERROR: Could not find environment variables for Visual Studio
	pause
	goto :EOF
)

REM Test
echo If dynamically linked you should see MSVCRT.dll, MSVCRTD.dll, MSVCRXXX.dll or MSVCRXXXd.dll:
dumpbin /all "%~f1" | find /i "msvcr"
pause