@echo off
for /F "tokens=3" %%V in (CurrentVersion.nsh) do set CURRENTVERSION=%%~V
copy ..\bin\FreeVimagerw\FreeVimagerw.exe .\FreeVimagerUnicode-%CURRENTVERSION%.exe
upx --compress-icons=0 --ultra-brute FreeVimagerUnicode-%CURRENTVERSION%.exe