@echo off
REM Set --compress-exports=0 to be able to run it under Wine
copy ..\bin\RemoteCam\ReleaseNoDLL\RemoteCam.ocx ..\ActiveX\RemoteCam.ocx
copy ..\bin\RemoteCamViewer\ReleaseNoDLL\RemoteCamViewer.exe ..\ActiveX\RemoteCamViewer.exe
upx --ultra-brute --8mb-ram ..\ActiveX\RemoteCam.ocx
pause