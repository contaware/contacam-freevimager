@echo off
REM Set --compress-exports=0 to be able to run it under Wine
upx --backup --ultra-brute --8mb-ram ..\bin\FreeVimagerw\FreeVimagerw.exe
pause