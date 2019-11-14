@echo off

REM Check parameter
if "%~1"=="" (
	echo.Please Provide a Parameter!
	echo.
	echo.off: camera source obscured
	echo.on:  camera source visible
	timeout /T 10
	goto :eof
)

REM Variable
set BatchDir=%~dp0

if /I "%~1"=="off" (
    echo Note: delete this file to re-enable the camera>"%BatchDir%CAMERA_IS_OBSCURED.txt"
) else (
    del "%BatchDir%CAMERA_IS_OBSCURED.txt" >nul 2>&1
)

timeout /T 3
