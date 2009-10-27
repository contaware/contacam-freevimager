; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CRemoteCamViewerDoc
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "remotecamviewer.h"
LastPage=0

ClassCount=5
Class1=CMainFrame
Class2=CRemoteCamViewerApp
Class3=CAboutDlg
Class4=CRemoteCamViewerDoc
Class5=CRemoteCamViewerView

ResourceCount=3
Resource1=IDD_REMOTECAMVIEWER_FORM
Resource2=IDD_ABOUTBOX
Resource3=IDR_MAINFRAME

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
LastObject=CRemoteCamViewerView

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
ControlCount=1
Control1=IDC_REMOTECAMCTRL,{08CF6679-5BB3-4EF4-A773-C410DC56531C},1342242816

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
Command1=ID_APP_EXIT
Command2=ID_TOOLS_REGISTER
Command3=ID_TOOLS_UNREGISTER
Command4=ID_TOOLS_SHOWPROP
Command5=ID_APP_ABOUT
Command6=ID_HELP_ABOUTCTRL
CommandCount=6

