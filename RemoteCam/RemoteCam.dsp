# Microsoft Developer Studio Project File - Name="RemoteCam" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=RemoteCam - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "RemoteCam.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "RemoteCam.mak" CFG="RemoteCam - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RemoteCam - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "RemoteCam - Win32 ContaCam" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RemoteCam - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Ext "ocx"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Bin/RemoteCam/Debug"
# PROP Intermediate_Dir "../Obj/RemoteCam/Debug"
# PROP Target_Ext "ocx"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\\" /I "..\ffmpeg\includes" /I "..\ffmpeg" /I "..\Dib" /I "..\libpng" /I "..\libjpeg" /I "..\libtiff\libtiff" /I "..\zip" /I "..\zlib" /I "..\giflib\lib" /D "_DEBUG" /D "DEBUG" /D "_AFXDLL" /D "_MBCS" /D "_WINDOWS" /D "_USRDLL" /D "_WINDLL" /D "MFC_THREAD" /D "SUPPORT_BMP" /D "SUPPORT_MMBMP" /D "SUPPORT_LIBJPEG" /D "SUPPORT_LIBPNG" /D "SUPPORT_LIBTIFF" /D "SUPPORT_PCX" /D "SUPPORT_GIFLIB" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x810 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# Begin Custom Build - Registering ActiveX Control...
OutDir=.\../Bin/RemoteCam/Debug
TargetPath=\uimager\Bin\RemoteCam\Debug\RemoteCam.ocx
InputPath=\uimager\Bin\RemoteCam\Debug\RemoteCam.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "RemoteCam - Win32 ContaCam"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RemoteCam___Win32_ContaCam"
# PROP BASE Intermediate_Dir "RemoteCam___Win32_ContaCam"
# PROP BASE Target_Ext "ocx"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Bin/RemoteCam/ReleaseNoDLL"
# PROP Intermediate_Dir "../Obj/RemoteCam/ReleaseNoDLL"
# PROP Target_Ext "ocx"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\\" /I "..\ffmpeg\includes" /I "..\ffmpeg" /I "..\Dib" /I "..\libpng" /I "..\libjpeg" /I "..\libtiff\libtiff" /I "..\zip" /I "..\zlib" /I "..\giflib\lib" /D "_MBCS" /D "_WINDOWS" /D "_USRDLL" /D "_WINDLL" /D "MFC_THREAD" /D "SUPPORT_BMP" /D "SUPPORT_MMBMP" /D "SUPPORT_LIBJPEG" /D "SUPPORT_LIBPNG" /D "SUPPORT_LIBTIFF" /D "SUPPORT_PCX" /D "SUPPORT_GIFLIB" /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\\" /I "..\ffmpeg\includes" /I "..\ffmpeg" /I "..\Dib" /I "..\libpng" /I "..\libjpeg" /I "..\libtiff\libtiff" /I "..\zip" /I "..\zlib" /I "..\giflib\lib" /D "_MBCS" /D "_WINDOWS" /D "_USRDLL" /D "_WINDLL" /D "MFC_THREAD" /D "SUPPORT_BMP" /D "SUPPORT_MMBMP" /D "SUPPORT_LIBJPEG" /D "SUPPORT_LIBPNG" /D "SUPPORT_LIBTIFF" /D "SUPPORT_PCX" /D "SUPPORT_GIFLIB" /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386
# Begin Custom Build - Registering ActiveX Control...
OutDir=.\../Bin/RemoteCam/ReleaseNoDLL
TargetPath=\uimager\Bin\RemoteCam\ReleaseNoDLL\RemoteCam.ocx
InputPath=\uimager\Bin\RemoteCam\ReleaseNoDLL\RemoteCam.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "RemoteCam - Win32 Debug"
# Name "RemoteCam - Win32 ContaCam"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AVCodecHelpers.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\ConvertUTF.cpp
# End Source File
# Begin Source File

SOURCE=..\Helpers.cpp
# End Source File
# Begin Source File

SOURCE=..\NetCom.cpp
# End Source File
# Begin Source File

SOURCE=.\NetGetFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\RemoteCam.cpp
# End Source File
# Begin Source File

SOURCE=.\RemoteCam.def
# End Source File
# Begin Source File

SOURCE=.\RemoteCam.odl
# End Source File
# Begin Source File

SOURCE=.\RemoteCam.rc
# End Source File
# Begin Source File

SOURCE=.\RemoteCamCtl.cpp
# End Source File
# Begin Source File

SOURCE=.\RemoteCamPpg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\WorkerThread.cpp
# End Source File
# Begin Source File

SOURCE=..\XMLite.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Helpers.h
# End Source File
# Begin Source File

SOURCE=..\NetCom.h
# End Source File
# Begin Source File

SOURCE=.\NetGetFrame.h
# End Source File
# Begin Source File

SOURCE=..\PJNMD5.h
# End Source File
# Begin Source File

SOURCE=.\RemoteCam.h
# End Source File
# Begin Source File

SOURCE=.\RemoteCamCtl.h
# End Source File
# Begin Source File

SOURCE=.\RemoteCamPpg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\WorkerThread.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\RemoteCam.ico
# End Source File
# Begin Source File

SOURCE=.\RemoteCamCtl.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
