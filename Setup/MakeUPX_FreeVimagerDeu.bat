@echo off
for /F "tokens=3" %%V in (CurrentVersion.nsh) do set CURRENTVERSION=%%~V
copy ..\Translation\FreeVimagerwDEU.exe .\FreeVimagerDeu-%CURRENTVERSION%.exe
upx --compress-icons=0 --ultra-brute FreeVimagerDeu-%CURRENTVERSION%.exe