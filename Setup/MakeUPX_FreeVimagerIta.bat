@echo off
for /F "tokens=3" %%V in (CurrentVersion.nsh) do set CURRENTVERSION=%%~V
copy ..\Translation\FreeVimagerwITA.exe .\FreeVimagerIta-%CURRENTVERSION%.exe
upx --compress-icons=0 --ultra-brute FreeVimagerIta-%CURRENTVERSION%.exe