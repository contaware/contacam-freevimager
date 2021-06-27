@echo off

REM Check parameter
if "%~1"=="" (
	echo.Please Provide a Parameter!
	echo.
	echo.0: REC OFF
	echo.5,10,20,30..90: increasing motion sensitivity
	echo.100: continuous REC
	timeout /T 10
	goto :eof
)

REM Variables
set BatchDir=%~dp0
set /a Sensitivity=%~1
set /a Retry=5

:loop

REM Write sensitivity to file
echo.Writing sensitivity:
echo.%Sensitivity%
echo.%Sensitivity%>"%BatchDir%CAMERA_REC_SENSITIVITY.txt"

echo.

REM Check whether the sensitivity has been written
echo.Checking sensitivity:
set /a ReadSensitivity=-1
set /p ReadSensitivity=<"%BatchDir%CAMERA_REC_SENSITIVITY.txt"
echo.%ReadSensitivity%
if %ReadSensitivity%==%Sensitivity% goto end

REM Retry counter decrement
set /a Retry=%Retry%-1
if %Retry%==0 goto end
timeout /T 1 /NOBREAK
echo.
goto loop

:end

timeout /T 3
