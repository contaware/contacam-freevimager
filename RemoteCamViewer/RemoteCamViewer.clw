; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CSettingsDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "remotecamviewer.h"
LastPage=0

ClassCount=6
Class1=CMainFrame
Class2=CRemoteCamViewerApp
Class3=CAboutDlg
Class4=CRemoteCamViewerDoc
Class5=CRemoteCamViewerView

ResourceCount=8
Resource1=IDR_MAINFRAME
Resource2=IDD_REMOTECAMVIEWER_FORM
Resource3=IDD_ABOUTBOX
Class6=CSettingsDlg
Resource4=IDD_SETTINGS
Resource5=IDD_SETTINGS (English (U.S.))
Resource6=IDD_ABOUTBOX (English (U.S.))
Resource7=IDD_REMOTECAMVIEWER_FORM (English (U.S.))
Resource8=IDR_MAINFRAME (English (U.S.))

[CLS:CMainFrame]
Type=0
BaseClass=CFrameWnd
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp

[CLS:CRemoteCamViewerApp]
Type=0
BaseClass=CWinApp
HeaderFile=RemoteCamViewer.h
ImplementationFile=RemoteCamViewer.cpp
LastObject=ID_TOOLS_SHOWPROP

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=RemoteCamViewer.cpp
ImplementationFile=RemoteCamViewer.cpp
LastObject=ID_TOOLS_REGISTER

[CLS:CRemoteCamViewerDoc]
Type=0
BaseClass=CDocument
HeaderFile=RemoteCamViewerDoc.h
ImplementationFile=RemoteCamViewerDoc.cpp
Filter=N
VirtualFilter=DC
LastObject=CRemoteCamViewerDoc

[CLS:CRemoteCamViewerView]
Type=0
BaseClass=CFormView
HeaderFile=RemoteCamViewerView.h
ImplementationFile=RemoteCamViewerView.cpp
Filter=D
VirtualFilter=VWC
LastObject=ID_FILE_SETTINGS

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDOK,button,1342373889
Control4=IDC_STATIC2141,static,1342308352

[DLG:IDD_REMOTECAMVIEWER_FORM]
Type=1
Class=CRemoteCamViewerView
ControlCount=4
Control1=IDC_REMOTECAMCTRL0,{08CF6679-5BB3-4EF4-A773-C410DC56531C},1342177280
Control2=IDC_REMOTECAMCTRL1,{08CF6679-5BB3-4EF4-A773-C410DC56531C},1342177280
Control3=IDC_REMOTECAMCTRL2,{08CF6679-5BB3-4EF4-A773-C410DC56531C},1342177280
Control4=IDC_REMOTECAMCTRL3,{08CF6679-5BB3-4EF4-A773-C410DC56531C},1342177280

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
CommandCount=8

[MNU:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_SETTINGS
Command2=ID_FILE_SHOWPROP0
Command3=ID_FILE_SHOWPROP1
Command4=ID_FILE_SHOWPROP2
Command5=ID_FILE_SHOWPROP3
Command6=ID_APP_EXIT
Command7=ID_TOOLS_REGISTER
Command8=ID_TOOLS_UNREGISTER
Command9=ID_APP_ABOUT
Command10=ID_HELP_ABOUTCTRL
CommandCount=10

[DLG:IDD_SETTINGS]
Type=1
Class=CSettingsDlg
ControlCount=12
Control1=IDOK,button,1342373889
Control2=IDC_STATIC,static,1342308352
Control3=IDC_EDIT_TITLE,edit,1350631552
Control4=IDC_STATIC,static,1342308352
Control5=IDC_EDIT_LABEL0,edit,1350631552
Control6=IDC_STATIC,static,1342308352
Control7=IDC_EDIT_LABEL1,edit,1350631552
Control8=IDC_STATIC,static,1342308352
Control9=IDC_EDIT_LABEL2,edit,1350631552
Control10=IDC_STATIC,static,1342308352
Control11=IDC_EDIT_LABEL3,edit,1350631552
Control12=IDCANCEL,button,1342242816

[CLS:CSettingsDlg]
Type=0
HeaderFile=SettingsDlg.h
ImplementationFile=SettingsDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CSettingsDlg

[TB:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
CommandCount=8

[MNU:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=?
Command1=ID_FILE_SETTINGS
Command2=ID_FILE_SHOWPROP0
Command3=ID_FILE_SHOWPROP1
Command4=ID_FILE_SHOWPROP2
Command5=ID_FILE_SHOWPROP3
Command6=ID_APP_EXIT
Command7=ID_TOOLS_REGISTER
Command8=ID_TOOLS_UNREGISTER
Command9=ID_APP_ABOUT
Command10=ID_HELP_ABOUTCTRL
CommandCount=10

[DLG:IDD_ABOUTBOX (English (U.S.))]
Type=1
Class=?
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDOK,button,1342373889
Control4=IDC_STATIC,static,1342308352

[DLG:IDD_REMOTECAMVIEWER_FORM (English (U.S.))]
Type=1
Class=?
ControlCount=4
Control1=IDC_REMOTECAMCTRL0,{08CF6679-5BB3-4EF4-A773-C410DC56531C},1342177280
Control2=IDC_REMOTECAMCTRL1,{08CF6679-5BB3-4EF4-A773-C410DC56531C},1342177280
Control3=IDC_REMOTECAMCTRL2,{08CF6679-5BB3-4EF4-A773-C410DC56531C},1342177280
Control4=IDC_REMOTECAMCTRL3,{08CF6679-5BB3-4EF4-A773-C410DC56531C},1342177280

[DLG:IDD_SETTINGS (English (U.S.))]
Type=1
Class=?
ControlCount=12
Control1=IDOK,button,1342373889
Control2=IDC_STATIC,static,1342308352
Control3=IDC_EDIT_TITLE,edit,1350631552
Control4=IDC_STATIC,static,1342308352
Control5=IDC_EDIT_LABEL0,edit,1350631552
Control6=IDC_STATIC,static,1342308352
Control7=IDC_EDIT_LABEL1,edit,1350631552
Control8=IDC_STATIC,static,1342308352
Control9=IDC_EDIT_LABEL2,edit,1350631552
Control10=IDC_STATIC,static,1342308352
Control11=IDC_EDIT_LABEL3,edit,1350631552
Control12=IDCANCEL,button,1342242816

