@echo off
copy ..\bin\RemoteCam\ReleaseNoDLL\RemoteCam.ocx ..\ActiveX\RemoteCam.ocx
copy ..\bin\RemoteCamViewer\ReleaseNoDLL\RemoteCamViewer.exe ..\ActiveX\RemoteCamViewer.exe
upx --ultra-brute ..\ActiveX\RemoteCam.ocx