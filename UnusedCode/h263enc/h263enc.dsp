# Microsoft Developer Studio Project File - Name="h263enc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=h263enc - Win32 UnicodeFull
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "h263enc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "h263enc.mak" CFG="h263enc - Win32 UnicodeFull"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "h263enc - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "h263enc - Win32 ReleaseNoDLL" (based on "Win32 (x86) Static Library")
!MESSAGE "h263enc - Win32 ReleaseNoDLLFull" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "h263enc - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Bin/H263Enc/Debug"
# PROP Intermediate_Dir "../Obj/H263Enc/Debug"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\\" /I "..\Dib" /I "..\libpng" /I "..\libjpeg" /I "..\libtiff\libtiff" /I "..\zlib" /I "..\giflib\lib" /D "_DEBUG" /D "DEBUG" /D "_AFXDLL" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D "MFC_THREAD" /D WINVER=0x0400 /D "SUPPORT_BMP" /D "SUPPORT_MMBMP" /D "SUPPORT_LIBJPEG" /D "SUPPORT_LIBPNG" /D "SUPPORT_LIBTIFF" /D "SUPPORT_PCX" /D "SUPPORT_GIFLIB" /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "h263enc - Win32 ReleaseNoDLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseNoDLL"
# PROP BASE Intermediate_Dir "ReleaseNoDLL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Bin/H263Enc/ReleaseNoDLL"
# PROP Intermediate_Dir "../Obj/H263Enc/ReleaseNoDLL"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\\" /I "..\Dib" /I "..\libpng" /I "..\libjpeg" /I "..\libtiff\libtiff" /I "..\zlib" /I "..\giflib\lib" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D "MFC_THREAD" /D "SUPPORT_BMP" /D "SUPPORT_MMBMP" /D "SUPPORT_LIBJPEG" /D "SUPPORT_LIBPNG" /D "SUPPORT_LIBTIFF" /D "SUPPORT_PCX" /D "SUPPORT_GIFLIB" /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "h263enc - Win32 ReleaseNoDLLFull"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "h263enc___Win32_ReleaseNoDLLFull"
# PROP BASE Intermediate_Dir "h263enc___Win32_ReleaseNoDLLFull"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Bin/ReleaseNoDLLFull"
# PROP Intermediate_Dir "../Obj/ReleaseNoDLLFull"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\\" /I "..\Dib" /I "..\libpng" /I "..\libjpeg" /I "..\libtiff\libtiff" /I "..\zlib" /I "..\giflib\lib" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D "MFC_THREAD" /D "SUPPORT_BMP" /D "SUPPORT_MMBMP" /D "SUPPORT_LIBJPEG" /D "SUPPORT_LIBPNG" /D "SUPPORT_LIBTIFF" /D "SUPPORT_PCX" /D "SUPPORT_GIFLIB" /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\\" /I "..\Dib" /I "..\libpng" /I "..\libjpeg" /I "..\libtiff\libtiff" /I "..\zlib" /I "..\giflib\lib" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D "MFC_THREAD" /D "SUPPORT_BMP" /D "SUPPORT_MMBMP" /D "SUPPORT_LIBJPEG" /D "SUPPORT_LIBPNG" /D "SUPPORT_LIBTIFF" /D "SUPPORT_PCX" /D "SUPPORT_GIFLIB" /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /Yu"stdafx.h" /FD /c
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

# Name "h263enc - Win32 Debug"
# Name "h263enc - Win32 ReleaseNoDLL"
# Name "h263enc - Win32 ReleaseNoDLLFull"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\coder.cpp
# End Source File
# Begin Source File

SOURCE=.\countbit.cpp
# End Source File
# Begin Source File

SOURCE=.\dct.cpp
# End Source File
# Begin Source File

SOURCE=.\H263Enc.cpp
# End Source File
# Begin Source File

SOURCE=.\huffman.cpp
# End Source File
# Begin Source File

SOURCE=.\libr263.cpp
# End Source File
# Begin Source File

SOURCE=.\mot_est.cpp
# End Source File
# Begin Source File

SOURCE=.\pred.cpp
# End Source File
# Begin Source File

SOURCE=.\quant.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\stream.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\def.h
# End Source File
# Begin Source File

SOURCE=.\H263Enc.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# End Target
# End Project
