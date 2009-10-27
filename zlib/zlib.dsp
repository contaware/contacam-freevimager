# Microsoft Developer Studio Project File - Name="zlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=zlib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zlib.mak" CFG="zlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "zlib - Win32 ContaCam" (based on "Win32 (x86) Static Library")
!MESSAGE "zlib - Win32 FreeVimager" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/u-blox/u-Center/zlib/msvc", VCCAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zlib - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W4 /Zi /Od /D "WIN32" /D "_DEBUG" /FR /FD /GZ /c
# ADD CPP /nologo /MDd /W2 /ZI /Od /D "_DEBUG" /D "DEBUG" /D WINVER=0x0400 /D "ASMV" /D "ASMINF" /D "WINDOWS" /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FR /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\win32\zlib\lib_dbg\zlibd.lib"
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib - Win32 ContaCam"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "zlib___Win32_ContaCam"
# PROP BASE Intermediate_Dir "zlib___Win32_ContaCam"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Bin/ContaCam"
# PROP Intermediate_Dir "../Obj/ContaCam"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MT /W2 /O2 /D "NDEBUG" /D WINVER=0x0400 /D "ASMV" /D "ASMINF" /D "WINDOWS" /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MT /W2 /O2 /D "NDEBUG" /D WINVER=0x0400 /D "ASMV" /D "ASMINF" /D "WINDOWS" /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib - Win32 FreeVimager"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "zlib___Win32_FreeVimager"
# PROP BASE Intermediate_Dir "zlib___Win32_FreeVimager"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Bin/FreeVimager"
# PROP Intermediate_Dir "../Obj/FreeVimager"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /MT /W2 /O2 /D "NDEBUG" /D WINVER=0x0400 /D "ASMV" /D "ASMINF" /D "WINDOWS" /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MT /W2 /O2 /D "NDEBUG" /D WINVER=0x0400 /D "ASMV" /D "ASMINF" /D "WINDOWS" /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FD /c
# SUBTRACT CPP /Fr
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

# Name "zlib - Win32 Debug"
# Name "zlib - Win32 ContaCam"
# Name "zlib - Win32 FreeVimager"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\adler32.c
# End Source File
# Begin Source File

SOURCE=.\compress.c
# End Source File
# Begin Source File

SOURCE=.\crc32.c
# End Source File
# Begin Source File

SOURCE=.\deflate.c
# End Source File
# Begin Source File

SOURCE=.\gvmat32.asm

!IF  "$(CFG)" == "zlib - Win32 Debug"

# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\../Obj/Debug
InputPath=.\gvmat32.asm
InputName=gvmat32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Zd /coff /nologo /Fo "$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "zlib - Win32 ContaCam"

# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\../Obj/ContaCam
InputPath=.\gvmat32.asm
InputName=gvmat32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Zd /coff /nologo /Fo "$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "zlib - Win32 FreeVimager"

# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\../Obj/FreeVimager
InputPath=.\gvmat32.asm
InputName=gvmat32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Zd /coff /nologo /Fo "$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gvmat32c.c

!IF  "$(CFG)" == "zlib - Win32 Debug"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - Win32 ContaCam"

!ELSEIF  "$(CFG)" == "zlib - Win32 FreeVimager"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gzio.c

!IF  "$(CFG)" == "zlib - Win32 Debug"

# ADD BASE CPP /Yc"zutil.h"
# SUBTRACT CPP /YX /Yc

!ELSEIF  "$(CFG)" == "zlib - Win32 ContaCam"

# ADD BASE CPP /Yc"zutil.h"
# SUBTRACT CPP /YX /Yc

!ELSEIF  "$(CFG)" == "zlib - Win32 FreeVimager"

# ADD BASE CPP /Yc"zutil.h"
# SUBTRACT CPP /YX /Yc

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\infback.c
# End Source File
# Begin Source File

SOURCE=.\inffas32.asm

!IF  "$(CFG)" == "zlib - Win32 Debug"

# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\../Obj/Debug
InputPath=.\inffas32.asm
InputName=inffas32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Zd /coff /nologo /Fo "$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "zlib - Win32 ContaCam"

# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\../Obj/ContaCam
InputPath=.\inffas32.asm
InputName=inffas32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Zd /coff /nologo /Fo "$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "zlib - Win32 FreeVimager"

# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\../Obj/FreeVimager
InputPath=.\inffas32.asm
InputName=inffas32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Zd /coff /nologo /Fo "$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\inffast.c

!IF  "$(CFG)" == "zlib - Win32 Debug"

# ADD BASE CPP /Yu"zutil.h"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "zlib - Win32 ContaCam"

# ADD BASE CPP /Yu"zutil.h"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "zlib - Win32 FreeVimager"

# ADD BASE CPP /Yu"zutil.h"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\inflate.c

!IF  "$(CFG)" == "zlib - Win32 Debug"

# ADD BASE CPP /Yu"zutil.h"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "zlib - Win32 ContaCam"

# ADD BASE CPP /Yu"zutil.h"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "zlib - Win32 FreeVimager"

# ADD BASE CPP /Yu"zutil.h"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\inftrees.c

!IF  "$(CFG)" == "zlib - Win32 Debug"

# ADD BASE CPP /Yu"zutil.h"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "zlib - Win32 ContaCam"

# ADD BASE CPP /Yu"zutil.h"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "zlib - Win32 FreeVimager"

# ADD BASE CPP /Yu"zutil.h"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\trees.c
# End Source File
# Begin Source File

SOURCE=.\uncompr.c
# End Source File
# Begin Source File

SOURCE=.\zutil.c

!IF  "$(CFG)" == "zlib - Win32 Debug"

# ADD BASE CPP /Yu"zutil.h"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "zlib - Win32 ContaCam"

# ADD BASE CPP /Yu"zutil.h"
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "zlib - Win32 FreeVimager"

# ADD BASE CPP /Yu"zutil.h"
# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\crc32.h
# End Source File
# Begin Source File

SOURCE=.\deflate.h
# End Source File
# Begin Source File

SOURCE=.\inffast.h
# End Source File
# Begin Source File

SOURCE=.\inffixed.h
# End Source File
# Begin Source File

SOURCE=.\inflate.h
# End Source File
# Begin Source File

SOURCE=.\inftrees.h
# End Source File
# Begin Source File

SOURCE=.\trees.h
# End Source File
# Begin Source File

SOURCE=.\zconf.h
# End Source File
# Begin Source File

SOURCE=.\zlib.h
# End Source File
# Begin Source File

SOURCE=.\zutil.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
