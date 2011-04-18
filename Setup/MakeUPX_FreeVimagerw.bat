@echo off
REM Set --compress-exports=0 to be able to run it under Wine
copy ..\bin\FreeVimagerw\FreeVimagerw.exe .\FreeVimagerw.exe
upx --compress-icons=0 --ultra-brute --8mb-ram FreeVimagerw.exe