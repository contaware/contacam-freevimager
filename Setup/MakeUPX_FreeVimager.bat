@echo off
REM Set --compress-exports=0 to be able to run it under Wine
copy ..\bin\FreeVimager\FreeVimager.exe .\FreeVimager.exe
upx --compress-icons=0 --ultra-brute --8mb-ram FreeVimager.exe
pause