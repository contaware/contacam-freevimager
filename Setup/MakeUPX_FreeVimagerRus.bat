@echo off
REM Set --compress-exports=0 to be able to run it under Wine
copy ..\Translation\FreeVimagerwRUS.exe .\FreeVimagerRus.exe
upx --compress-icons=0 --ultra-brute --8mb-ram FreeVimagerRus.exe