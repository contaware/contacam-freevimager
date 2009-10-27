# Microsoft Developer Studio Project File - Name="Zip" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Zip - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zip.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zip.mak" CFG="Zip - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Zip - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Zip - Win32 ContaCam" (based on "Win32 (x86) Static Library")
!MESSAGE "Zip - Win32 FreeVimager" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Zip - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Bin/Debug"
# PROP Intermediate_Dir "../Obj/Debug"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /ZI /Od /I "..\zlib" /I "..\\" /D "_DEBUG" /D "DEBUG" /D "_AFXDLL" /D "_WINDOWS" /D "_MBCS" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x415 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Zip - Win32 ContaCam"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Zip___Win32_ContaCam"
# PROP BASE Intermediate_Dir "Zip___Win32_ContaCam"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Bin/ContaCam"
# PROP Intermediate_Dir "../Obj/ContaCam"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MT /W4 /GX /O2 /I "..\zlib" /I "..\\" /D "_WINDOWS" /D "_MBCS" /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MT /W4 /GX /O2 /I "..\zlib" /I "..\\" /D "_WINDOWS" /D "_MBCS" /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Zip - Win32 FreeVimager"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Zip___Win32_FreeVimager"
# PROP BASE Intermediate_Dir "Zip___Win32_FreeVimager"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Bin/FreeVimager"
# PROP Intermediate_Dir "../Obj/FreeVimager"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MT /W4 /GX /O2 /I "..\zlib" /I "..\\" /D "_WINDOWS" /D "_MBCS" /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MT /W4 /GX /O2 /I "..\zlib" /I "..\\" /D "_WINDOWS" /D "_MBCS" /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "Zip - Win32 Debug"
# Name "Zip - Win32 ContaCam"
# Name "Zip - Win32 FreeVimager"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AutoBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\BigFile.cpp
# End Source File
# Begin Source File

SOURCE=.\CentralDir.cpp
# End Source File
# Begin Source File

SOURCE=.\FileHeader.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipArchive.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipException.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipInternalInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipStorage.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AutoBuffer.h
# End Source File
# Begin Source File

SOURCE=.\BigFile.h
# End Source File
# Begin Source File

SOURCE=.\CentralDir.h
# End Source File
# Begin Source File

SOURCE=.\FileHeader.h
# End Source File
# Begin Source File

SOURCE=.\ZipArchive.h
# End Source File
# Begin Source File

SOURCE=.\ZipException.h
# End Source File
# Begin Source File

SOURCE=.\ZipInternalInfo.h
# End Source File
# Begin Source File

SOURCE=.\ZipStorage.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Appnote.txt
# End Source File
# End Target
# End Project
# Section Zip : {427B9281-D5E8-11D3-B7C7-E75054E13747}
# 	2:12:AutoBuffer.h:AutoBuffer.h
# 	2:18:CLASS: CAutoBuffer:CAutoBuffer
# 	2:19:Application Include:Zip.h
# 	2:14:AutoBuffer.cpp:AutoBuffer.cpp
# End Section
