@echo off
for /F "tokens=3" %%V in (CurrentVersion.nsh) do set CURRENTVERSION=%%~V
copy ..\Translation\FreeVimagerwRUS.exe .\FreeVimagerRus-%CURRENTVERSION%.exe
upx --compress-icons=0 --ultra-brute FreeVimagerRus-%CURRENTVERSION%.exe