# Microsoft Developer Studio Project File - Name="uImager" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=uImager - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "uImager.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "uImager.mak" CFG="uImager - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "uImager - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "uImager - Win32 ContaCam" (based on "Win32 (x86) Application")
!MESSAGE "uImager - Win32 FreeVimager" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/GPS/SOFTWARE/INTERNAL/uImager/Win32", FWCBAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "uImager - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Bin/Debug"
# PROP Intermediate_Dir "Obj/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".\ffmpeg\includes" /I ".\ffmpeg" /I ".\Dib" /I ".\icclib" /I ".\libpng" /I ".\libjpeg" /I ".\libtiff\libtiff" /I ".\zlib" /I ".\giflib\lib" /I ".\zip" /I ".\directx_base" /I ".\rsaeuro" /D "_DEBUG" /D "DEBUG" /D "_AFXDLL" /D "VIDEODEVICEDOC" /D "SUPPORT_LIBAVCODEC" /D "CPJNSMTP_NOSSL" /D "_MBCS" /D WINVER=0x0400 /D "_WINDOWS" /D "MFC_THREAD" /D DIRECT3D_VERSION=0x0700 /D "CPJNSMTP_NONTLM" /D "SUPPORT_BMP" /D "SUPPORT_MMBMP" /D "SUPPORT_LIBJPEG" /D "SUPPORT_LIBPNG" /D "SUPPORT_LIBTIFF" /D "SUPPORT_PCX" /D "SUPPORT_GIFLIB" /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x810 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL" /d "VIDEODEVICEDOC"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Bin/Debug/ContaCam.bsc"
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /out:"Bin/Debug/ContaCam.exe" /pdbtype:sept

!ELSEIF  "$(CFG)" == "uImager - Win32 ContaCam"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "uImager___Win32_ContaCam"
# PROP BASE Intermediate_Dir "uImager___Win32_ContaCam"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Bin/ContaCam"
# PROP Intermediate_Dir "Obj/ContaCam"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\libpng" /I "..\libjpeg" /I "..\libtiff" /I "..\zlib" /I ".\ffmpeg\includes" /I ".\ffmpeg" /I ".\Dib" /I ".\icclib" /I ".\libpng" /I ".\libjpeg" /I ".\libtiff\libtiff" /I ".\zlib" /I ".\giflib\lib" /I ".\zip" /I ".\directx_base" /D "VIDEODEVICEDOC" /D "NDEBUG" /D "SUPPORT_LIBAVCODEC" /D "CRACKCHECK" /D "CPJNSMTP_NOSSL" /D "_MBCS" /D WINVER=0x0400 /D "_WINDOWS" /D "MFC_THREAD" /D DIRECT3D_VERSION=0x0700 /D "CPJNSMTP_NONTLM" /D "SUPPORT_BMP" /D "SUPPORT_MMBMP" /D "SUPPORT_LIBJPEG" /D "SUPPORT_LIBPNG" /D "SUPPORT_LIBTIFF" /D "SUPPORT_PCX" /D "SUPPORT_GIFLIB" /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FR /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\libpng" /I "..\libjpeg" /I "..\libtiff" /I "..\zlib" /I ".\ffmpeg\includes" /I ".\ffmpeg" /I ".\Dib" /I ".\icclib" /I ".\libpng" /I ".\libjpeg" /I ".\libtiff\libtiff" /I ".\zlib" /I ".\giflib\lib" /I ".\zip" /I ".\directx_base" /I ".\rsaeuro" /D "VIDEODEVICEDOC" /D "NDEBUG" /D "SUPPORT_LIBAVCODEC" /D "CRACKCHECK" /D "CPJNSMTP_NOSSL" /D "_MBCS" /D WINVER=0x0400 /D "_WINDOWS" /D "MFC_THREAD" /D DIRECT3D_VERSION=0x0700 /D "CPJNSMTP_NONTLM" /D "SUPPORT_BMP" /D "SUPPORT_MMBMP" /D "SUPPORT_LIBJPEG" /D "SUPPORT_LIBPNG" /D "SUPPORT_LIBTIFF" /D "SUPPORT_PCX" /D "SUPPORT_GIFLIB" /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "VIDEODEVICEDOC"
# ADD RSC /l 0x409 /d "NDEBUG" /d "VIDEODEVICEDOC"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Bin/ContaCam/ContaCam.bsc"
# ADD BSC32 /nologo /o"Bin/ContaCam/ContaCam.bsc"
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386 /out:"Bin/ContaCam/ContaCam.exe"
# ADD LINK32 /nologo /subsystem:windows /machine:I386 /out:"Bin/ContaCam/ContaCam.exe"
# Begin Special Build Tool
WkspDir=.
SOURCE="$(InputPath)"
PostBuild_Cmds="$(WkspDir)\CheckSumPatcher\Release\CheckSumPatcher.exe" "$(WkspDir)\bin\ContaCam\ContaCam.exe"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "uImager - Win32 FreeVimager"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "uImager___Win32_FreeVimager"
# PROP BASE Intermediate_Dir "uImager___Win32_FreeVimager"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Bin/FreeVimager"
# PROP Intermediate_Dir "Obj/FreeVimager"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\libpng" /I "..\libjpeg" /I "..\libtiff" /I "..\zlib" /I ".\Dib" /I ".\icclib" /I ".\libpng" /I ".\libjpeg" /I ".\libtiff\libtiff" /I ".\zlib" /I ".\giflib\lib" /I ".\zip" /I ".\directx_base" /D "NDEBUG" /D "CPJNSMTP_NOSSL" /D "_MBCS" /D WINVER=0x0400 /D "_WINDOWS" /D "MFC_THREAD" /D DIRECT3D_VERSION=0x0700 /D "CPJNSMTP_NONTLM" /D "SUPPORT_BMP" /D "SUPPORT_MMBMP" /D "SUPPORT_LIBJPEG" /D "SUPPORT_LIBPNG" /D "SUPPORT_LIBTIFF" /D "SUPPORT_PCX" /D "SUPPORT_GIFLIB" /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FR /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\libpng" /I "..\libjpeg" /I "..\libtiff" /I "..\zlib" /I ".\ffmpeg\includes" /I ".\ffmpeg" /I ".\Dib" /I ".\icclib" /I ".\libpng" /I ".\libjpeg" /I ".\libtiff\libtiff" /I ".\zlib" /I ".\giflib\lib" /I ".\zip" /I ".\directx_base" /I ".\rsaeuro" /D "NDEBUG" /D "CPJNSMTP_NOSSL" /D "_MBCS" /D WINVER=0x0400 /D "_WINDOWS" /D "MFC_THREAD" /D DIRECT3D_VERSION=0x0700 /D "CPJNSMTP_NONTLM" /D "SUPPORT_LIBAVCODEC" /D "SUPPORT_BMP" /D "SUPPORT_MMBMP" /D "SUPPORT_LIBJPEG" /D "SUPPORT_LIBPNG" /D "SUPPORT_LIBTIFF" /D "SUPPORT_PCX" /D "SUPPORT_GIFLIB" /D "WIN32" /D _WIN32_IE=0x0400 /D "_CRT_NONSTDC_NO_DEPRECATE" /D "_CRT_SECURE_NO_DEPRECATE" /D "_SCL_SECURE_NO_DEPRECATE" /D "_ATL_SECURE_NO_DEPRECATE" /D "_AFX_SECURE_NO_DEPRECATE" /D "_CRT_NON_CONFORMING_SWPRINTFS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Bin/FreeVimager/FreeVimager.bsc"
# ADD BSC32 /nologo /o"Bin/FreeVimager/FreeVimager.bsc"
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386 /out:"Bin/FreeVimager/FreeVimager.exe"
# ADD LINK32 /nologo /subsystem:windows /machine:I386 /out:"Bin/FreeVimager/FreeVimager.exe"

!ENDIF 

# Begin Target

# Name "uImager - Win32 Debug"
# Name "uImager - Win32 ContaCam"
# Name "uImager - Win32 FreeVimager"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AddBordersDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimGifSaveDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimGifSaveSmallDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AssistantPage.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioFormatDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioInSourceDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioMCIDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioMCIView.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioOutDestinationDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioVideoShiftDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AuthenticationDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AviFile.cpp
# End Source File
# Begin Source File

SOURCE=.\AviFileMerge.cpp
# End Source File
# Begin Source File

SOURCE=.\AviInfoDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AviOpenStreamsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AviPlay.cpp
# End Source File
# Begin Source File

SOURCE=.\AviSaveAsStreamsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AVRec.cpp
# End Source File
# Begin Source File

SOURCE=.\AVRecHelpers.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\Base64.cpp
# End Source File
# Begin Source File

SOURCE=.\BatchProcDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\BatchProcGeneralTab.cpp
# End Source File
# Begin Source File

SOURCE=.\BatchProcJpegTab.cpp
# End Source File
# Begin Source File

SOURCE=.\BatchProcShrinkTab.cpp
# End Source File
# Begin Source File

SOURCE=.\BatchProcTiffTab.cpp
# End Source File
# Begin Source File

SOURCE=.\BkgColEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\BmpSaveDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\BrowseDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CaptureDeviceDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CDAudio.cpp
# End Source File
# Begin Source File

SOURCE=.\CDAudioDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\CDAudioView.cpp
# End Source File
# Begin Source File

SOURCE=.\cdxCDynamicDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\cdxCDynamicWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\cdxCDynamicWndEx.cpp
# End Source File
# Begin Source File

SOURCE=.\cdxCSizeIconCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\Cfile64.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\Cmallspy.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorButton.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorButtonPicker.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorDetectionPage.cpp
# End Source File
# Begin Source File

SOURCE=.\ColourPopup.cpp
# End Source File
# Begin Source File

SOURCE=.\ConvertUTF.cpp
# End Source File
# Begin Source File

SOURCE=.\CPUCount.cpp
# End Source File
# Begin Source File

SOURCE=.\CPUSpeed.cpp
# End Source File
# Begin Source File

SOURCE=.\Crc32Dynamic.cpp
# End Source File
# Begin Source File

SOURCE=.\CreditsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DecreaseBppDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DeletePageDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DIBStatic.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectX7Dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DiscFormatData.cpp
# End Source File
# Begin Source File

SOURCE=.\DiscFormatDataEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\DiscFormatErase.cpp
# End Source File
# Begin Source File

SOURCE=.\DiscFormatEraseEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\DiscMaster.cpp
# End Source File
# Begin Source File

SOURCE=.\DiscRecorder.cpp
# End Source File
# Begin Source File

SOURCE=.\DragDropListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\DxCapture.cpp
# End Source File
# Begin Source File

SOURCE=.\DxDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\DxTuner.cpp
# End Source File
# Begin Source File

SOURCE=.\DxVideoFormatDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DxVideoInputDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\EnumGDIObjectsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\EnumPrinters.cpp
# End Source File
# Begin Source File

SOURCE=.\Fourier.cpp
# End Source File
# Begin Source File

SOURCE=.\FTPTransfer.cpp
# End Source File
# Begin Source File

SOURCE=.\FTPUploadConfigurationDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GeneralPage.cpp
# End Source File
# Begin Source File

SOURCE=.\getdxver.cpp
# End Source File
# Begin Source File

SOURCE=.\GifAnimationThread.cpp
# End Source File
# Begin Source File

SOURCE=.\Helpers.cpp
# End Source File
# Begin Source File

SOURCE=.\HLSDlgModeless.cpp
# End Source File
# Begin Source File

SOURCE=.\HostPortDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\HScrollListBox.cpp
# End Source File
# Begin Source File

SOURCE=.\HttpVideoFormatDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ImageInfoDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\IMAPI2Dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\IMAPI2DownloadDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\IniFile.cpp
# End Source File
# Begin Source File

SOURCE=.\JpegCompressionQualityDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LayeredDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LicenseDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ListCtrlEx.cpp
# End Source File
# Begin Source File

SOURCE=.\MacProgressCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MDIClientWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\MlString.cpp
# End Source File
# Begin Source File

SOURCE=.\MonochromeConversionDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MotionDetHelpers.asm

!IF  "$(CFG)" == "uImager - Win32 Debug"

# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Obj/Debug
InputPath=.\MotionDetHelpers.asm
InputName=MotionDetHelpers

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Zd /coff /nologo /Fo "$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "uImager - Win32 ContaCam"

# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Obj/ContaCam
InputPath=.\MotionDetHelpers.asm
InputName=MotionDetHelpers

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Zd /coff /nologo /Fo "$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "uImager - Win32 FreeVimager"

# Begin Custom Build - Assembling $(InputPath)...
IntDir=.\Obj/FreeVimager
InputPath=.\MotionDetHelpers.asm
InputName=MotionDetHelpers

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Zd /coff /nologo /Fo "$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MovDetAnimGifConfigurationDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MovementDetectionPage.cpp
# End Source File
# Begin Source File

SOURCE=.\NetCom.cpp
# End Source File
# Begin Source File

SOURCE=.\NetworkPage.cpp
# End Source File
# Begin Source File

SOURCE=.\NewDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NoVistaFileDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NumEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\NumSpinCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\OCRLanguageDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OddButton.cpp
# End Source File
# Begin Source File

SOURCE=.\OsdDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OutVolDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PaletteWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\PdfSaveDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PeakMeterCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\PictureDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\PicturePrintPreviewView.cpp
# End Source File
# Begin Source File

SOURCE=.\PictureView.cpp
# End Source File
# Begin Source File

SOURCE=.\PJNSMTP.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerToolBarDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PngSaveDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PostDelayedMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\PreviewFileDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PrintPreviewScaleEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RedEyeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RegistrationDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizingDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizingDpiDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RotationFlippingDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SampleCGB.cpp
# End Source File
# Begin Source File

SOURCE=.\SaveFileDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SendMailConfigurationDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SendMailDocsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SepComboBox.cpp
# End Source File
# Begin Source File

SOURCE=.\SettingsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SettingsDlgVideoDeviceDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\SettingsXml.cpp
# End Source File
# Begin Source File

SOURCE=.\SharpenDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sinstance.cpp
# End Source File
# Begin Source File

SOURCE=.\SnapshotPage.cpp
# End Source File
# Begin Source File

SOURCE=.\SocMFC.cpp
# End Source File
# Begin Source File

SOURCE=.\SoftBordersDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SoftenDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SortableFileFind.cpp
# End Source File
# Begin Source File

SOURCE=.\SortableStringArray.cpp
# End Source File
# Begin Source File

SOURCE=.\StaticLink.cpp
# End Source File
# Begin Source File

SOURCE=.\StatusBarACT.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TabCtrlSSL.cpp
# End Source File
# Begin Source File

SOURCE=.\TabPageSSL.cpp
# End Source File
# Begin Source File

SOURCE=.\TiffSaveDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ToolBarChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\ToolbarSliderCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\TraceLogFile.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\TransAlpha.cpp
# End Source File
# Begin Source File

SOURCE=.\Trayicon.cpp
# End Source File
# Begin Source File

SOURCE=.\TryEnterCriticalSection.cpp
# End Source File
# Begin Source File

SOURCE=.\TwainCpp.cpp
# End Source File
# Begin Source File

SOURCE=.\uImager.cpp
# End Source File
# Begin Source File

SOURCE=.\uImager.rc
# End Source File
# Begin Source File

SOURCE=.\uImagerDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\uImagerView.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoAviDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoAviView.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoDeviceDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoDevicePropertySheet.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoDeviceView.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoFormatDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Winsock2MissingDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\WorkerThread.cpp
# End Source File
# Begin Source File

SOURCE=.\XButtonXP.cpp
# End Source File
# Begin Source File

SOURCE=.\XMLite.cpp
# End Source File
# Begin Source File

SOURCE=.\XmpDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\XThemeHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\YesNoAllCancelDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipProgressDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AddBordersDlg.h
# End Source File
# Begin Source File

SOURCE=.\AnimGifSaveDlg.h
# End Source File
# Begin Source File

SOURCE=.\AnimGifSaveSmallDlg.h
# End Source File
# Begin Source File

SOURCE=.\appconst.h
# End Source File
# Begin Source File

SOURCE=.\AssistantPage.h
# End Source File
# Begin Source File

SOURCE=.\AudioFormatDlg.h
# End Source File
# Begin Source File

SOURCE=.\AudioInSourceDlg.h
# End Source File
# Begin Source File

SOURCE=.\AudioMCIDoc.h
# End Source File
# Begin Source File

SOURCE=.\AudioMCIView.h
# End Source File
# Begin Source File

SOURCE=.\AudioOutDestinationDlg.h
# End Source File
# Begin Source File

SOURCE=.\AudioVideoShiftDlg.h
# End Source File
# Begin Source File

SOURCE=.\AuthenticationDlg.h
# End Source File
# Begin Source File

SOURCE=.\AviFile.h
# End Source File
# Begin Source File

SOURCE=.\AviFileMerge.h
# End Source File
# Begin Source File

SOURCE=.\AviInfoDlg.h
# End Source File
# Begin Source File

SOURCE=.\AviOpenStreamsDlg.h
# End Source File
# Begin Source File

SOURCE=.\AviPlay.h
# End Source File
# Begin Source File

SOURCE=.\AviSaveAsStreamsDlg.h
# End Source File
# Begin Source File

SOURCE=.\AVRec.h
# End Source File
# Begin Source File

SOURCE=.\Base64.h
# End Source File
# Begin Source File

SOURCE=.\BatchProcDlg.h
# End Source File
# Begin Source File

SOURCE=.\BatchProcGeneralTab.h
# End Source File
# Begin Source File

SOURCE=.\BatchProcJpegTab.h
# End Source File
# Begin Source File

SOURCE=.\BatchProcShrinkTab.h
# End Source File
# Begin Source File

SOURCE=.\BatchProcTiffTab.h
# End Source File
# Begin Source File

SOURCE=.\bigalloc.h
# End Source File
# Begin Source File

SOURCE=.\BkgColEdit.h
# End Source File
# Begin Source File

SOURCE=.\BmpSaveDlg.h
# End Source File
# Begin Source File

SOURCE=.\BrowseDlg.h
# End Source File
# Begin Source File

SOURCE=.\CaptureDeviceDlg.h
# End Source File
# Begin Source File

SOURCE=.\CDAudio.h
# End Source File
# Begin Source File

SOURCE=.\CDAudioDoc.h
# End Source File
# Begin Source File

SOURCE=.\CDAudioView.h
# End Source File
# Begin Source File

SOURCE=.\cdxCDynamicDialog.h
# End Source File
# Begin Source File

SOURCE=.\cdxCDynamicWnd.h
# End Source File
# Begin Source File

SOURCE=.\cdxCDynamicWndEx.h
# End Source File
# Begin Source File

SOURCE=.\cdxCSizeIconCtrl.h
# End Source File
# Begin Source File

SOURCE=.\Cfile64.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\Cmallspy.h
# End Source File
# Begin Source File

SOURCE=.\ColorButton.h
# End Source File
# Begin Source File

SOURCE=.\ColorButtonPicker.h
# End Source File
# Begin Source File

SOURCE=.\ColorDetectionPage.h
# End Source File
# Begin Source File

SOURCE=.\ColourPopup.h
# End Source File
# Begin Source File

SOURCE=.\ConvertUTF.h
# End Source File
# Begin Source File

SOURCE=.\CPUCount.h
# End Source File
# Begin Source File

SOURCE=.\CPUSpeed.h
# End Source File
# Begin Source File

SOURCE=.\Crc32Dynamic.h
# End Source File
# Begin Source File

SOURCE=.\CreditsDlg.h
# End Source File
# Begin Source File

SOURCE=.\DecreaseBppDlg.h
# End Source File
# Begin Source File

SOURCE=.\DeletePageDlg.h
# End Source File
# Begin Source File

SOURCE=.\DIBStatic.h
# End Source File
# Begin Source File

SOURCE=.\DirectX7Dlg.h
# End Source File
# Begin Source File

SOURCE=.\DiscFormatData.h
# End Source File
# Begin Source File

SOURCE=.\DiscFormatDataEvent.h
# End Source File
# Begin Source File

SOURCE=.\DiscFormatErase.h
# End Source File
# Begin Source File

SOURCE=.\DiscFormatEraseEvent.h
# End Source File
# Begin Source File

SOURCE=.\DiscMaster.h
# End Source File
# Begin Source File

SOURCE=.\DiscRecorder.h
# End Source File
# Begin Source File

SOURCE=.\DragDropListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\DxCapture.h
# End Source File
# Begin Source File

SOURCE=.\DxDraw.h
# End Source File
# Begin Source File

SOURCE=.\DxTuner.h
# End Source File
# Begin Source File

SOURCE=.\DxVideoFormatDlg.h
# End Source File
# Begin Source File

SOURCE=.\DxVideoInputDlg.h
# End Source File
# Begin Source File

SOURCE=.\EnumGDIObjectsDlg.h
# End Source File
# Begin Source File

SOURCE=.\EnumPrinters.h
# End Source File
# Begin Source File

SOURCE=.\Fourier.h
# End Source File
# Begin Source File

SOURCE=.\FTPTransfer.h
# End Source File
# Begin Source File

SOURCE=.\FTPUploadConfigurationDlg.h
# End Source File
# Begin Source File

SOURCE=.\GeneralPage.h
# End Source File
# Begin Source File

SOURCE=.\getdxver.h
# End Source File
# Begin Source File

SOURCE=.\GifAnimationThread.h
# End Source File
# Begin Source File

SOURCE=.\Helpers.h
# End Source File
# Begin Source File

SOURCE=.\HLSDlgModeless.h
# End Source File
# Begin Source File

SOURCE=.\HostPortDlg.h
# End Source File
# Begin Source File

SOURCE=.\HScrollListBox.h
# End Source File
# Begin Source File

SOURCE=.\HttpVideoFormatDlg.h
# End Source File
# Begin Source File

SOURCE=.\ImageInfoDlg.h
# End Source File
# Begin Source File

SOURCE=.\IMAPI2Dlg.h
# End Source File
# Begin Source File

SOURCE=.\IMAPI2DownloadDlg.h
# End Source File
# Begin Source File

SOURCE=.\Imapi2Small.h
# End Source File
# Begin Source File

SOURCE=.\IniFile.h
# End Source File
# Begin Source File

SOURCE=.\ISampleGrabber.h
# End Source File
# Begin Source File

SOURCE=.\JpegCompressionQualityDlg.h
# End Source File
# Begin Source File

SOURCE=.\LayeredDlg.h
# End Source File
# Begin Source File

SOURCE=.\LicenseDlg.h
# End Source File
# Begin Source File

SOURCE=.\ListCtrlEx.h
# End Source File
# Begin Source File

SOURCE=.\MacProgressCtrl.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MDIClientWnd.h
# End Source File
# Begin Source File

SOURCE=.\MDIVWCTL.tlh
# End Source File
# Begin Source File

SOURCE=.\MemDC.h
# End Source File
# Begin Source File

SOURCE=.\MlString.h
# End Source File
# Begin Source File

SOURCE=.\MonochromeConversionDlg.h
# End Source File
# Begin Source File

SOURCE=.\MotionDetHelpers.h
# End Source File
# Begin Source File

SOURCE=.\MovDetAnimGifConfigurationDlg.h
# End Source File
# Begin Source File

SOURCE=.\MovementDetectionPage.h
# End Source File
# Begin Source File

SOURCE=.\NetCom.h
# End Source File
# Begin Source File

SOURCE=.\NetFrameHdr.h
# End Source File
# Begin Source File

SOURCE=.\NetworkPage.h
# End Source File
# Begin Source File

SOURCE=.\NewDlg.h
# End Source File
# Begin Source File

SOURCE=.\NoVistaFileDlg.h
# End Source File
# Begin Source File

SOURCE=.\NumEdit.h
# End Source File
# Begin Source File

SOURCE=.\NumSpinCtrl.h
# End Source File
# Begin Source File

SOURCE=.\OCRLanguageDlg.h
# End Source File
# Begin Source File

SOURCE=.\OddButton.h
# End Source File
# Begin Source File

SOURCE=.\OsdDlg.h
# End Source File
# Begin Source File

SOURCE=.\OutVolDlg.h
# End Source File
# Begin Source File

SOURCE=.\PaletteWnd.h
# End Source File
# Begin Source File

SOURCE=.\PdfSaveDlg.h
# End Source File
# Begin Source File

SOURCE=.\PeakMeterCtrl.h
# End Source File
# Begin Source File

SOURCE=.\Performance.h
# End Source File
# Begin Source File

SOURCE=.\PictureDoc.h
# End Source File
# Begin Source File

SOURCE=.\PicturePrintPreviewView.h
# End Source File
# Begin Source File

SOURCE=.\PictureView.h
# End Source File
# Begin Source File

SOURCE=.\PJNMD5.h
# End Source File
# Begin Source File

SOURCE=.\PJNSMTP.h
# End Source File
# Begin Source File

SOURCE=.\PlayerToolBarDlg.h
# End Source File
# Begin Source File

SOURCE=.\PngSaveDlg.h
# End Source File
# Begin Source File

SOURCE=.\PostDelayedMessage.h
# End Source File
# Begin Source File

SOURCE=.\PreviewFileDlg.h
# End Source File
# Begin Source File

SOURCE=.\PrintPreviewScaleEdit.h
# End Source File
# Begin Source File

SOURCE=.\Progress.h
# End Source File
# Begin Source File

SOURCE=.\ProgressDlg.h
# End Source File
# Begin Source File

SOURCE=.\RedEyeDlg.h
# End Source File
# Begin Source File

SOURCE=.\RegistrationDlg.h
# End Source File
# Begin Source File

SOURCE=.\ResizingDlg.h
# End Source File
# Begin Source File

SOURCE=.\ResizingDpiDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\RotationFlippingDlg.h
# End Source File
# Begin Source File

SOURCE=.\Round.h
# End Source File
# Begin Source File

SOURCE=.\SampleCGB.h
# End Source File
# Begin Source File

SOURCE=.\SaveFileDlg.h
# End Source File
# Begin Source File

SOURCE=.\SendMailConfigurationDlg.h
# End Source File
# Begin Source File

SOURCE=.\SendMailDocsDlg.h
# End Source File
# Begin Source File

SOURCE=.\SepComboBox.h
# End Source File
# Begin Source File

SOURCE=.\SettingsDlg.h
# End Source File
# Begin Source File

SOURCE=.\SettingsDlgVideoDeviceDoc.h
# End Source File
# Begin Source File

SOURCE=.\SettingsXml.h
# End Source File
# Begin Source File

SOURCE=.\SharpenDlg.h
# End Source File
# Begin Source File

SOURCE=.\sinstance.h
# End Source File
# Begin Source File

SOURCE=.\SnapshotPage.h
# End Source File
# Begin Source File

SOURCE=.\SocMFC.h
# End Source File
# Begin Source File

SOURCE=.\SoftBordersDlg.h
# End Source File
# Begin Source File

SOURCE=.\SoftenDlg.h
# End Source File
# Begin Source File

SOURCE=.\SortableFileFind.h
# End Source File
# Begin Source File

SOURCE=.\SortableStringArray.h
# End Source File
# Begin Source File

SOURCE=.\StaticLink.h
# End Source File
# Begin Source File

SOURCE=.\StatusBarACT.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TabCtrlSSL.h
# End Source File
# Begin Source File

SOURCE=.\TabPageSSL.h
# End Source File
# Begin Source File

SOURCE=.\TiffSaveDlg.h
# End Source File
# Begin Source File

SOURCE=.\ToolBarChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\ToolbarSliderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\TraceLogFile.h
# End Source File
# Begin Source File

SOURCE=.\TransAlpha.h
# End Source File
# Begin Source File

SOURCE=.\Trayicon.h
# End Source File
# Begin Source File

SOURCE=.\TryEnterCriticalSection.h
# End Source File
# Begin Source File

SOURCE=.\twain.h
# End Source File
# Begin Source File

SOURCE=.\TwainCpp.h
# End Source File
# Begin Source File

SOURCE=.\uImager.h
# End Source File
# Begin Source File

SOURCE=.\uImagerDoc.h
# End Source File
# Begin Source File

SOURCE=.\uImagerView.h
# End Source File
# Begin Source File

SOURCE=.\VideoAviDoc.h
# End Source File
# Begin Source File

SOURCE=.\VideoAviView.h
# End Source File
# Begin Source File

SOURCE=.\VideoDeviceDoc.h
# End Source File
# Begin Source File

SOURCE=.\VideoDevicePropertySheet.h
# End Source File
# Begin Source File

SOURCE=.\VideoDeviceView.h
# End Source File
# Begin Source File

SOURCE=.\VideoFormatDlg.h
# End Source File
# Begin Source File

SOURCE=.\Winsock2MissingDlg.h
# End Source File
# Begin Source File

SOURCE=.\WorkerThread.h
# End Source File
# Begin Source File

SOURCE=.\XButtonXP.h
# End Source File
# Begin Source File

SOURCE=.\XMLite.h
# End Source File
# Begin Source File

SOURCE=.\XmpDlg.h
# End Source File
# Begin Source File

SOURCE=.\XThemeHelper.h
# End Source File
# Begin Source File

SOURCE=.\YesNoAllCancelDlg.h
# End Source File
# Begin Source File

SOURCE=.\ZipProgressDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\audiomcidoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\bigpicture.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bigpicturenohq.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bitstream_Vera_11.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp.ico
# End Source File
# Begin Source File

SOURCE=.\res\cdaudiodoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\device.ico
# End Source File
# Begin Source File

SOURCE=.\res\DragDropMulti.bmp
# End Source File
# Begin Source File

SOURCE=.\res\DragDropSingle.bmp
# End Source File
# Begin Source File

SOURCE=.\res\emf.ico
# End Source File
# Begin Source File

SOURCE=.\res\gif.ico
# End Source File
# Begin Source File

SOURCE=.\res\grab_closed.cur
# End Source File
# Begin Source File

SOURCE=.\res\grab_normal.cur
# End Source File
# Begin Source File

SOURCE=.\res\hand.cur
# End Source File
# Begin Source File

SOURCE=.\res\HeaderArrows.bmp
# End Source File
# Begin Source File

SOURCE=.\res\jpg.ico
# End Source File
# Begin Source File

SOURCE=.\res\magnifyminus.ico
# End Source File
# Begin Source File

SOURCE=.\res\magnifyp.ico
# End Source File
# Begin Source File

SOURCE=.\res\magnifyplus.ico
# End Source File
# Begin Source File

SOURCE=.\res\manifestcontacam.bin
# End Source File
# Begin Source File

SOURCE=.\res\manifestfreevimager.bin
# End Source File
# Begin Source File

SOURCE=.\res\pause.ico
# End Source File
# Begin Source File

SOURCE=.\res\pcx.ico
# End Source File
# Begin Source File

SOURCE=.\res\picture.bmp
# End Source File
# Begin Source File

SOURCE=.\res\picturedoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\picturenohq.bmp
# End Source File
# Begin Source File

SOURCE=.\res\play.ico
# End Source File
# Begin Source File

SOURCE=.\res\png.ico
# End Source File
# Begin Source File

SOURCE=.\res\selectcolor.cur
# End Source File
# Begin Source File

SOURCE=.\res\stop.ico
# End Source File
# Begin Source File

SOURCE=.\res\TeleTextDH_26.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tif.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbarvideodevicedoc.bmp
# End Source File
# Begin Source File

SOURCE=.\res\uimager.ico
# End Source File
# Begin Source File

SOURCE=.\res\uImager.rc2
# End Source File
# Begin Source File

SOURCE=.\res\uimager_videodevicedoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\videoavidoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\videodevice.bmp
# End Source File
# Begin Source File

SOURCE=.\res\videodevicedoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\videotoolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\videotoolbarfs.bmp
# End Source File
# Begin Source File

SOURCE=.\res\zip.ico
# End Source File
# Begin Source File

SOURCE=.\res\ZoneMinus.cur
# End Source File
# Begin Source File

SOURCE=.\res\ZonePlus.cur
# End Source File
# Begin Source File

SOURCE=.\res\ZoomMinus.cur
# End Source File
# Begin Source File

SOURCE=.\res\ZoomPlus.cur
# End Source File
# End Group
# End Target
# End Project
