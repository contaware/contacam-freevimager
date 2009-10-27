# Microsoft Developer Studio Project File - Name="Dib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Dib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Dib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Dib.mak" CFG="Dib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Dib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Dib - Win32 ContaCam" (based on "Win32 (x86) Static Library")
!MESSAGE "Dib - Win32 FreeVimager" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Dib - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Bin/Debug/Dib"
# PROP Intermediate_Dir "../Obj/Debug/Dib"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\libpng" /I "..\libjpeg" /I "..\libtiff\libtiff" /I "..\zlib" /I "..\giflib\lib" /I "..\zip" /D "_DEBUG" /D "VIDEODEVICEDOC" /D "DEBUG" /D "_AFXDLL" /D "_MBCS" /D "_WINDOWS" /D WINVER=0x0400 /D "MFC_THREAD" /D "SUPPORT_BMP" /D "SUPPORT_MMBMP" /D "SUPPORT_LIBJPEG" /D "SUPPORT_LIBPNG" /D "SUPPORT_LIBTIFF" /D "SUPPORT_PCX" /D "SUPPORT_GIFLIB" /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x810 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Dib - Win32 ContaCam"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Dib___Win32_ContaCam"
# PROP BASE Intermediate_Dir "Dib___Win32_ContaCam"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Bin/ContaCam/Dib"
# PROP Intermediate_Dir "../Obj/ContaCam/Dib"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\libpng" /I "..\libjpeg" /I "..\libtiff\libtiff" /I "..\zlib" /I "..\giflib\lib" /I "..\zip" /D "_MBCS" /D "VIDEODEVICEDOC" /D "_WINDOWS" /D "MFC_THREAD" /D "SUPPORT_BMP" /D "SUPPORT_MMBMP" /D "SUPPORT_LIBJPEG" /D "SUPPORT_LIBPNG" /D "SUPPORT_LIBTIFF" /D "SUPPORT_PCX" /D "SUPPORT_GIFLIB" /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\libpng" /I "..\libjpeg" /I "..\libtiff\libtiff" /I "..\zlib" /I "..\giflib\lib" /I "..\zip" /D "_MBCS" /D "VIDEODEVICEDOC" /D "_WINDOWS" /D "MFC_THREAD" /D "SUPPORT_BMP" /D "SUPPORT_MMBMP" /D "SUPPORT_LIBJPEG" /D "SUPPORT_LIBPNG" /D "SUPPORT_LIBTIFF" /D "SUPPORT_PCX" /D "SUPPORT_GIFLIB" /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Dib - Win32 FreeVimager"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Dib___Win32_FreeVimager"
# PROP BASE Intermediate_Dir "Dib___Win32_FreeVimager"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Bin/FreeVimager/Dib"
# PROP Intermediate_Dir "../Obj/FreeVimager/Dib"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\libpng" /I "..\libjpeg" /I "..\libtiff\libtiff" /I "..\zlib" /I "..\giflib\lib" /I "..\zip" /D "_MBCS" /D "_WINDOWS" /D "MFC_THREAD" /D "SUPPORT_BMP" /D "SUPPORT_MMBMP" /D "SUPPORT_LIBJPEG" /D "SUPPORT_LIBPNG" /D "SUPPORT_LIBTIFF" /D "SUPPORT_PCX" /D "SUPPORT_GIFLIB" /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\libpng" /I "..\libjpeg" /I "..\libtiff\libtiff" /I "..\zlib" /I "..\giflib\lib" /I "..\zip" /D "_MBCS" /D "_WINDOWS" /D "MFC_THREAD" /D "SUPPORT_BMP" /D "SUPPORT_MMBMP" /D "SUPPORT_LIBJPEG" /D "SUPPORT_LIBPNG" /D "SUPPORT_LIBTIFF" /D "SUPPORT_PCX" /D "SUPPORT_GIFLIB" /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /Yu"stdafx.h" /FD /c
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

# Name "Dib - Win32 Debug"
# Name "Dib - Win32 ContaCam"
# Name "Dib - Win32 FreeVimager"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AnimGifSave.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorSpace.cpp
# End Source File
# Begin Source File

SOURCE=.\Dib.cpp
# End Source File
# Begin Source File

SOURCE=.\DibAdjColors.cpp
# End Source File
# Begin Source File

SOURCE=.\DibAlphaMixDiff.cpp
# End Source File
# Begin Source File

SOURCE=.\DibBmp.cpp
# End Source File
# Begin Source File

SOURCE=.\DibConvertBpp.cpp
# End Source File
# Begin Source File

SOURCE=.\DibFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\DibGif.cpp
# End Source File
# Begin Source File

SOURCE=.\DibJpeg.cpp
# End Source File
# Begin Source File

SOURCE=.\DibMetadata.cpp
# End Source File
# Begin Source File

SOURCE=.\DibMmBmp.cpp
# End Source File
# Begin Source File

SOURCE=.\DibPcx.cpp
# End Source File
# Begin Source File

SOURCE=.\DibPng.cpp
# End Source File
# Begin Source File

SOURCE=.\DibResize.cpp
# End Source File
# Begin Source File

SOURCE=.\DibRotate.cpp
# End Source File
# Begin Source File

SOURCE=.\DibText.cpp
# End Source File
# Begin Source File

SOURCE=.\DibTiff.cpp
# End Source File
# Begin Source File

SOURCE=.\Quantizer.cpp
# End Source File
# Begin Source File

SOURCE=.\RgbToYuv.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Tiff2Pdf.cpp
# End Source File
# Begin Source File

SOURCE=.\YuvToRgb.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AnimGifSave.h
# End Source File
# Begin Source File

SOURCE=.\ColorSpace.h
# End Source File
# Begin Source File

SOURCE=.\Dib.h
# End Source File
# Begin Source File

SOURCE=.\DibGif.h
# End Source File
# Begin Source File

SOURCE=.\DibInline.h
# End Source File
# Begin Source File

SOURCE=.\DibMetadata.h
# End Source File
# Begin Source File

SOURCE=.\Quantizer.h
# End Source File
# Begin Source File

SOURCE=.\RgbToYuv.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Tiff2Pdf.h
# End Source File
# Begin Source File

SOURCE=.\YuvToRgb.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Readme.txt
# End Source File
# End Target
# End Project
