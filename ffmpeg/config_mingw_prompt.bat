@echo off

rem call msys.bat trying common places
if exist "c:\mingw\msys\1.0\msys.bat" (
	call "c:\mingw\msys\1.0\msys.bat"
) else if exist "d:\mingw\msys\1.0\msys.bat" (
	call "d:\mingw\msys\1.0\msys.bat"
) else if exist "e:\mingw\msys\1.0\msys.bat" (
	call "e:\mingw\msys\1.0\msys.bat"
) else (
	echo ERROR: Could not find the msys.bat file
	pause
)