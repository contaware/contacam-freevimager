@echo off
for /F "tokens=3" %%V in (CurrentVersion.nsh) do set CURRENTVERSION=%%~V
copy ..\bin\FreeVimager\FreeVimager.exe .\FreeVimager-%CURRENTVERSION%.exe
upx --compress-icons=0 --ultra-brute FreeVimager-%CURRENTVERSION%.exe