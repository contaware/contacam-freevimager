# Microsoft Developer Studio Project File - Name="RemoteCamViewer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=RemoteCamViewer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "RemoteCamViewer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "RemoteCamViewer.mak" CFG="RemoteCamViewer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RemoteCamViewer - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "RemoteCamViewer - Win32 ContaCam" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RemoteCamViewer - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Bin/RemoteCamViewer/Debug"
# PROP Intermediate_Dir "../Obj/RemoteCamViewer/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "DEBUG" /D "_AFXDLL" /D "_MBCS" /D "_WINDOWS" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x810 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "RemoteCamViewer - Win32 ContaCam"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RemoteCamViewer___Win32_ContaCam"
# PROP BASE Intermediate_Dir "RemoteCamViewer___Win32_ContaCam"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Bin/RemoteCamViewer/ReleaseNoDLL"
# PROP Intermediate_Dir "../Obj/RemoteCamViewer/ReleaseNoDLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "_MBCS" /D "_WINDOWS" /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "_MBCS" /D "_WINDOWS" /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ENDIF 

# Begin Target

# Name "RemoteCamViewer - Win32 Debug"
# Name "RemoteCamViewer - Win32 ContaCam"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\remotecam.cpp
# End Source File
# Begin Source File

SOURCE=.\RemoteCamViewer.cpp
# End Source File
# Begin Source File

SOURCE=.\RemoteCamViewer.rc
# End Source File
# Begin Source File

SOURCE=.\RemoteCamViewerDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\RemoteCamViewerView.cpp
# End Source File
# Begin Source File

SOURCE=.\SettingsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\remotecam.h
# End Source File
# Begin Source File

SOURCE=.\RemoteCamViewer.h
# End Source File
# Begin Source File

SOURCE=.\RemoteCamViewerDoc.h
# End Source File
# Begin Source File

SOURCE=.\RemoteCamViewerView.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SettingsDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\RemoteCamViewer.ico
# End Source File
# Begin Source File

SOURCE=.\res\RemoteCamViewer.rc2
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
# Section RemoteCamViewer : {08CF6679-5BB3-4EF4-A773-C410DC56531C}
# 	2:21:DefaultSinkHeaderFile:remotecam.h
# 	2:16:DefaultSinkClass:CRemoteCam
# End Section
# Section RemoteCamViewer : {7AA2FD65-BB4D-490D-8926-6805BB402467}
# 	2:5:Class:CRemoteCam
# 	2:10:HeaderFile:remotecam.h
# 	2:8:ImplFile:remotecam.cpp
# End Section
