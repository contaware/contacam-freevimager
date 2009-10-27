# Microsoft Developer Studio Project File - Name="directx_base" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=directx_base - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "directx_base.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "directx_base.mak" CFG="directx_base - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "directx_base - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "directx_base - Win32 ContaCam" (based on "Win32 (x86) Static Library")
!MESSAGE "directx_base - Win32 FreeVimager" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "directx_base - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Bin/Debug"
# PROP Intermediate_Dir "../Obj/Debug"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "DEBUG" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D DIRECT3D_VERSION=0x0900 /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x810 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "directx_base - Win32 ContaCam"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "directx_base___Win32_ContaCam"
# PROP BASE Intermediate_Dir "directx_base___Win32_ContaCam"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Bin/ContaCam"
# PROP Intermediate_Dir "../Obj/ContaCam"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D DIRECT3D_VERSION=0x0900 /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D DIRECT3D_VERSION=0x0900 /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "directx_base - Win32 FreeVimager"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "directx_base___Win32_FreeVimager"
# PROP BASE Intermediate_Dir "directx_base___Win32_FreeVimager"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Bin/FreeVimager"
# PROP Intermediate_Dir "../Obj/FreeVimager"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D DIRECT3D_VERSION=0x0700 /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D DIRECT3D_VERSION=0x0700 /D "NDEBUG" /D WINVER=0x0400 /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FD /c
# SUBTRACT CPP /YX
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

# Name "directx_base - Win32 Debug"
# Name "directx_base - Win32 ContaCam"
# Name "directx_base - Win32 FreeVimager"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\amextra.cpp
# End Source File
# Begin Source File

SOURCE=.\amfilter.cpp
# End Source File
# Begin Source File

SOURCE=.\amvideo.cpp
# End Source File
# Begin Source File

SOURCE=.\combase.cpp
# End Source File
# Begin Source File

SOURCE=.\cprop.cpp
# End Source File
# Begin Source File

SOURCE=.\crossbar.cpp
# End Source File
# Begin Source File

SOURCE=.\ctlutil.cpp
# End Source File
# Begin Source File

SOURCE=.\ddmm.cpp
# End Source File
# Begin Source File

SOURCE=.\dllentry.cpp
# End Source File
# Begin Source File

SOURCE=.\dllsetup.cpp
# End Source File
# Begin Source File

SOURCE=.\mtype.cpp
# End Source File
# Begin Source File

SOURCE=.\outputq.cpp
# End Source File
# Begin Source File

SOURCE=.\pstream.cpp
# End Source File
# Begin Source File

SOURCE=.\pullpin.cpp
# End Source File
# Begin Source File

SOURCE=.\refclock.cpp
# End Source File
# Begin Source File

SOURCE=.\renbase.cpp
# End Source File
# Begin Source File

SOURCE=.\schedule.cpp
# End Source File
# Begin Source File

SOURCE=.\seekpt.cpp
# End Source File
# Begin Source File

SOURCE=.\source.cpp
# End Source File
# Begin Source File

SOURCE=.\strmctl.cpp
# End Source File
# Begin Source File

SOURCE=.\sysclock.cpp
# End Source File
# Begin Source File

SOURCE=.\transfrm.cpp
# End Source File
# Begin Source File

SOURCE=.\transip.cpp
# End Source File
# Begin Source File

SOURCE=.\videoctl.cpp
# End Source File
# Begin Source File

SOURCE=.\vtrans.cpp
# End Source File
# Begin Source File

SOURCE=.\winctrl.cpp
# End Source File
# Begin Source File

SOURCE=.\winutil.cpp
# End Source File
# Begin Source File

SOURCE=.\wxdebug.cpp
# End Source File
# Begin Source File

SOURCE=.\wxlist.cpp
# End Source File
# Begin Source File

SOURCE=.\wxutil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\amextra.h
# End Source File
# Begin Source File

SOURCE=.\amfilter.h
# End Source File
# Begin Source File

SOURCE=.\cache.h
# End Source File
# Begin Source File

SOURCE=.\combase.h
# End Source File
# Begin Source File

SOURCE=.\cprop.h
# End Source File
# Begin Source File

SOURCE=.\crossbar.h
# End Source File
# Begin Source File

SOURCE=.\ctlutil.h
# End Source File
# Begin Source File

SOURCE=.\ddmm.h
# End Source File
# Begin Source File

SOURCE=.\dllsetup.h
# End Source File
# Begin Source File

SOURCE=.\dsschedule.h
# End Source File
# Begin Source File

SOURCE=.\fourcc.h
# End Source File
# Begin Source File

SOURCE=.\measure.h
# End Source File
# Begin Source File

SOURCE=.\msgthrd.h
# End Source File
# Begin Source File

SOURCE=.\mtype.h
# End Source File
# Begin Source File

SOURCE=.\outputq.h
# End Source File
# Begin Source File

SOURCE=.\pstream.h
# End Source File
# Begin Source File

SOURCE=.\pullpin.h
# End Source File
# Begin Source File

SOURCE=.\refclock.h
# End Source File
# Begin Source File

SOURCE=.\reftime.h
# End Source File
# Begin Source File

SOURCE=.\renbase.h
# End Source File
# Begin Source File

SOURCE=.\seekpt.h
# End Source File
# Begin Source File

SOURCE=.\source.h
# End Source File
# Begin Source File

SOURCE=.\streams.h
# End Source File
# Begin Source File

SOURCE=.\strmctl.h
# End Source File
# Begin Source File

SOURCE=.\sysclock.h
# End Source File
# Begin Source File

SOURCE=.\transfrm.h
# End Source File
# Begin Source File

SOURCE=.\transip.h
# End Source File
# Begin Source File

SOURCE=.\videoctl.h
# End Source File
# Begin Source File

SOURCE=.\vtrans.h
# End Source File
# Begin Source File

SOURCE=.\winctrl.h
# End Source File
# Begin Source File

SOURCE=.\winutil.h
# End Source File
# Begin Source File

SOURCE=.\wxdebug.h
# End Source File
# Begin Source File

SOURCE=.\wxlist.h
# End Source File
# Begin Source File

SOURCE=.\wxutil.h
# End Source File
# End Group
# End Target
# End Project
