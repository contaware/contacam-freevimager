/******************************************************************************
|* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
|* ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
|* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
|* PARTICULAR PURPOSE.
|* 
|* Copyright 1995-2005 Nero AG. All Rights Reserved.
|*-----------------------------------------------------------------------------
|* PROJECT: NeroBurn NeroAPI Example
|*
|* FILE: NeroBurnDlg.cpp
|*
|* PURPOSE: Implementation of a dialog for interaction with the user.
******************************************************************************/


// NeroBurnDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NeroBurn.h"
#include "NeroBurnDlg.h"
#include "IsoTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
//{{AFX_MSG_MAP(CAboutDlg)
	// No message handlers
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNeroBurnDlg dialog

CNeroBurnDlg::CNeroBurnDlg(CWnd* pParent /*=NULL*/)
  : CDialog(CNeroBurnDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNeroBurnDlg)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bBurning = FALSE;
	m_bBurningCD = TRUE;
}

void CNeroBurnDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNeroBurnDlg)
	DDX_Control(pDX, IDOK, mOK);
	DDX_Control(pDX, IDC_ABORT, mbtnAbort);
	DDX_Control(pDX, IDC_PROGRESS1, mpgsProgress);
	DDX_Control(pDX, IDC_MESSAGES, medtMessages);
	DDX_Control(pDX, IDC_DEVICES, mcbxDevices);
	DDX_Control(pDX, IDC_BURN_CD, mbtnBurnCD);
	DDX_Control(pDX, IDC_BURN_DVD, mbtnBurnDVD);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNeroBurnDlg, CDialog)
	//{{AFX_MSG_MAP(CNeroBurnDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BURN_CD, OnBurnCD)
	ON_BN_CLICKED(IDC_BURN_DVD, OnBurnDVD)
	ON_BN_CLICKED(IDC_ABORT, OnAbort)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNeroBurnDlg message handlers

BOOL CNeroBurnDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);     // Set big icon
	SetIcon(m_hIcon, FALSE);    // Set small icon

	// For DVD Burn -> Disable Burn CD Button 
	if (((CNeroBurnApp*)::AfxGetApp())->m_bDoBurnDVD)
		mbtnBurnCD.EnableWindow(FALSE);

	NeroAPIInit();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CNeroBurnDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CNeroBurnDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNeroBurnDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CNeroBurnDlg::Burn(BOOL bBurnCD) 
{
	NERO_WRITE_CD writeCD;
	NERO_ISO_ITEM* pItem = NULL;
	EXITCODE code;
	CIsoTree IsoTree;

	// Check whether a path has been set
	ASSERT(((CNeroBurnApp*)::AfxGetApp())->m_sPathName != "");

	// Make the Iso Tree
	CString sIsoTreeRoot = ((CNeroBurnApp*)::AfxGetApp())->m_sPathName;
	if (sIsoTreeRoot[sIsoTreeRoot.GetLength() - 1] == '\\')
		sIsoTreeRoot.Delete(sIsoTreeRoot.GetLength() - 1);
	sIsoTreeRoot += "\\*.*";
	code = IsoTree.CreateIsoTree(true, sIsoTreeRoot, &pItem);
	if (code != EXITCODE_OK)
	{
		// Free the Iso Tree
		if (pItem)
		{
			IsoTree.DeleteIsoItemTree(pItem);
			pItem = NULL;
		}

		AppendString("Failed While Creating the ISO Tree");
		::AfxMessageBox("Failed While Creating the ISO Tree", MB_ICONSTOP);
		return;
	}

	// No CD stamp, artist or title required
	writeCD.nwcdpCDStamp = NULL;
	writeCD.nwcdArtist = NULL;
	writeCD.nwcdTitle = NULL;

	// No CD Extra information available
	writeCD.nwcdCDExtra = FALSE;

	// We have no Audio tracks
	writeCD.nwcdNumTracks = 0;

	// Do we want to write to a CD or DVD?
	if (bBurnCD)
	{
		m_bBurningCD = TRUE;
		writeCD.nwcdMediaType = MEDIA_CD;
	}
	else
	{
		m_bBurningCD = FALSE;
		writeCD.nwcdMediaType = MEDIA_DVD_ANY;
	}

	// Retrieve the NERO_SCSI_DEVICE_INFO pointer for the selected device
	// and assign it to a local variable
	NERO_SCSI_DEVICE_INFO* nsdiDevice = (NERO_SCSI_DEVICE_INFO*)mcbxDevices.GetItemDataPtr(mcbxDevices.GetCurSel());

	// Try to open the selected device
	ndhDeviceHandle = NeroOpenDevice(nsdiDevice);
	if (!ndhDeviceHandle)
	{
		// Free the Iso Tree
		if (pItem)
		{
			IsoTree.DeleteIsoItemTree(pItem);
			pItem = NULL;
		}

		// No handle available; tell the user what happened
		AppendString("Device could not be opened: " + (CString)nsdiDevice->nsdiDeviceName);
		::AfxMessageBox("Device could not be opened: " + (CString)nsdiDevice->nsdiDeviceName, MB_ICONSTOP);
		return;
	}

	// While burning the "Abort" button needs to be enabled
	// all the other buttons and controls have to be disabled
	mbtnAbort.EnableWindow(TRUE);
	mOK.EnableWindow(FALSE);
	mcbxDevices.EnableWindow(FALSE);
	if (!((CNeroBurnApp*)::AfxGetApp())->m_bDoBurnDVD)
		mbtnBurnCD.EnableWindow(FALSE);
	mbtnBurnDVD.EnableWindow(FALSE);
	m_bBurning = TRUE;

	// Set the range for the progress control, we will display percent
	mpgsProgress.SetRange(0,100);

	// Create An ISO track
	/*
	NERO_CITE_ARGS citeArgs;
	memset(&citeArgs, 0, sizeof (citeArgs));
	citeArgs.dwBurnOptions = NCITEF_CREATE_ISO_FS | NCITEF_USE_JOLIET;	// NCITEF_USE_MODE2,
																		// NCITEF_USE_ROCKRIDGE,
																		// NCITEF_CREATE_UDF_FS,
																		// NCITEF_USE_ALLSPACE,
																		// NCITEF_RELAX_JOLIET
	citeArgs.name = "Slideshow";
	citeArgs.firstRootItem = pItem;
	citeArgs.abstract = ;
	citeArgs.application = ;
	citeArgs.bibliographic = ;
	citeArgs.copyright = ;
	citeArgs.dataPreparer = ;
	citeArgs.publisher = ;
	citeArgs.systemIdentifier = ;
	citeArgs.volumeSet = ;
	writeCD.nwcdIsoTrack = NeroCreateIsoTrackEx(NULL,
												(const char *)&citeArgs,
												NCITEF_USE_STRUCT);
	*/
	writeCD.nwcdIsoTrack = NeroCreateIsoTrackEx(pItem,
												"Slideshow",
												NCITEF_CREATE_ISO_FS | NCITEF_USE_JOLIET);

	// Start the burn process by calling NeroBurn
	//
	// aDeviceHandle	= ndhDeviceHandle, the handle we got from NeroOpenDevice()
	// CDFormat			= NERO_ISO_AUDIO_CD
	// pWriteCD			= writeCD
	// dwFlags			= NBF_WRITE, do not simulate - burn!
	// dwSpeedInX		= 0, use maximum speed
	// pNeroProgress	= npProgress, filled during NeroAPIInit()
	int iRes = NeroBurn(ndhDeviceHandle,
						NERO_ISO_AUDIO_CD,
						&writeCD,
						NBF_WRITE | NBF_DETECT_NON_EMPTY_CDRW, // NBF_DISABLE_EJECT
						0,
						&npProgress);

	// Free memory that was allocated for the track
	NeroFreeIsoTrack(writeCD.nwcdIsoTrack);
	writeCD.nwcdIsoTrack = NULL;

	// Free the Iso Tree
	if (pItem)
	{
		IsoTree.DeleteIsoItemTree(pItem);
		pItem = NULL;
	}

	// Close the device
	NeroCloseDevice(ndhDeviceHandle);

	// Burning is finished, disable "Abort" activate all other controls
	m_bBurning = FALSE;
	mbtnAbort.EnableWindow(FALSE);
	mOK.EnableWindow(TRUE);
	mcbxDevices.EnableWindow(TRUE);
	if (!((CNeroBurnApp*)::AfxGetApp())->m_bDoBurnDVD)
		mbtnBurnCD.EnableWindow(TRUE);
	mbtnBurnDVD.EnableWindow(TRUE);

	// Clear the progress bar
	mpgsProgress.SetPos(0);

	// Make sure that aborted flag is not set if "Burn" button is pressed again
	mbAborted = false;

	// Retrieve the error log
	char* Log = NeroGetErrorLog();

	// Display the error log contents
#ifdef _DEBUG
	AppendString(Log);
#endif

	// Free the log
	NeroFreeMem(Log);

	// Tell the user how the burn process was finished
	switch(iRes)
	{
		case NEROAPI_BURN_OK:
			AppendString ("Burn Successful!\r\n");
			::AfxMessageBox("Burn Successful!");
			OnOK();
			break;
		case NEROAPI_BURN_UNKNOWN_CD_FORMAT:
			AppendString ("Unknown CD format\r\n");
			::AfxMessageBox("Unknown CD Format", MB_ICONSTOP);
			break;
		case NEROAPI_BURN_INVALID_DRIVE:
			AppendString ("Invalid Drive\r\n");
			::AfxMessageBox("Invalid Drive", MB_ICONSTOP);
			break;
		case NEROAPI_BURN_FAILED:
			AppendString ("Burn Failed\r\n");
			::AfxMessageBox("Burn Failed", MB_ICONSTOP);
			break;
		case NEROAPI_BURN_FUNCTION_NOT_ALLOWED:
			AppendString ("Function Not Allowed\r\n");
			::AfxMessageBox("Function Not Allowed", MB_ICONSTOP);
			break;
		case NEROAPI_BURN_DRIVE_NOT_ALLOWED:
			AppendString ("Drive Not Allowed\r\n");
			::AfxMessageBox("Drive Not Allowed", MB_ICONSTOP);
			break;
		case NEROAPI_BURN_USER_ABORT:
			AppendString ("User Aborted\r\n");
			::AfxMessageBox("User Aborted", MB_ICONSTOP);
			break;
		case NEROAPI_BURN_BAD_MESSAGE_FILE:
			AppendString ("Bad Message File\r\n");
			::AfxMessageBox("Bad Message File", MB_ICONSTOP);
			break;
		default:
			AppendString ("Unknown Error\r\n");
			::AfxMessageBox("Unknown Error", MB_ICONSTOP);
			break;
	}
}

void CNeroBurnDlg::OnBurnCD() 
{
	Burn(TRUE);
}

void CNeroBurnDlg::OnBurnDVD() 
{
	Burn(FALSE);
}

BOOL NERO_CALLBACK_ATTR CNeroBurnDlg::IdleCallback(void *pUserData)
{
	// idle callback is called frequently by NeroAPI

	// make sure that messages from other controls can be handled
	static MSG msg;
	while (!(((CNeroBurnDlg*)pUserData)->mbAborted) && ::PeekMessage(&msg,NULL,NULL,NULL,PM_NOREMOVE))
	{
		if (!AfxGetThread()->PumpMessage())
			break;
	}

	// aborted-flag serves as function result
	return ((CNeroBurnDlg*)pUserData)->mbAborted;
}

void CNeroBurnDlg::NeroAPIInit()
{
	bool bResult = true;
	mbAborted = false;

	// Try to open the NeroAPI DLL
	if (!NeroAPIGlueConnect(NULL))
	{
		AppendString("Cannot open NeroAPI.DLL");
		::AfxMessageBox("Cannot open NeroAPI.DLL", MB_ICONSTOP);
		CDialog::OnCancel();
		return;
	}

	// The NeroAPI DLL could be openend, get version information
#ifdef _DEBUG
	AppendString("Retrieving version information.");
#endif
	WORD majhi, majlo, minhi, minlo;
	NeroGetAPIVersionEx(&majhi, &majlo, &minhi, &minlo, NULL);

	// Format and display the version information
	CString strVersion;
	strVersion.Format("Using NeroAPI version %d.%d.%d.%d",
					majhi, majlo, minhi, minlo);
	AppendString(strVersion);

	// Setup of structures that the NeroAPI needs
#ifdef _DEBUG
	AppendString("Filling NERO_SETTINGS structure");
#endif

	// Information for registry access
	strcpy(pcNeroFilesPath, "NeroFiles");
	strcpy(pcVendor, "nero");
	strcpy(pcSoftware, "Nero - Burning Rom");

	// Use the US-English error message file
	strcpy(pcLanguageFile, "Nero.txt");

	nsSettings.nstNeroFilesPath = pcNeroFilesPath;
	nsSettings.nstVendor = pcVendor;

	// Set pointers to various callback functions
	nsSettings.nstIdle.ncCallbackFunction = IdleCallback;      

	// This pointer is required to access non-static variables from callback functions
	nsSettings.nstIdle.ncUserData = this;
	nsSettings.nstSoftware = pcSoftware;
	nsSettings.nstUserDialog.ncCallbackFunction = UserDialog; 
	nsSettings.nstUserDialog.ncUserData = this;
	nsSettings.nstLanguageFile =pcLanguageFile;

	// npProgress will be used during the burn process
	npProgress.npAbortedCallback = AbortedCallback;
	npProgress.npAddLogLineCallback = AddLogLine;
	npProgress.npDisableAbortCallback = NULL;
	npProgress.npProgressCallback = ProgressCallback;
	npProgress.npSetPhaseCallback = SetPhaseCallback;
	npProgress.npUserData = this;
	npProgress.npSetMajorPhaseCallback=NULL;	
	npProgress.npSubTaskProgressCallback=NULL;

	// No devices available yet
	pndiDeviceInfos = NULL;

	// Initialize the NeroAPI with nsSettings and the 
	// Serial Number that we got from the Registry
	NEROAPI_INIT_ERROR initErr;
	initErr = NeroInit(&nsSettings, NULL);

	// Display the result of NeroInit()
	switch (initErr)
	{
		case NEROAPI_INIT_OK:
#ifdef _DEBUG
			AppendString("Initialization of the NeroAPI Successful.");
#endif
			break;
		case NEROAPI_INIT_INVALID_ARGS:
			AppendString("The arguments are not valid.");
			::AfxMessageBox("NeroAPI Initialization Failed", MB_ICONSTOP);
			bResult = false;
			break;
		case NEROAPI_INIT_INVALID_SERIAL_NUM:
			AppendString("The Serial Number is not valid.");
			::AfxMessageBox("NeroAPI Initialization Failed\nThe Serial Number Is Not Valid", MB_ICONSTOP);
			bResult = false;
			break;
		default:
			AppendString("An error occured. The type of error cannot be determined.");
			::AfxMessageBox("NeroAPI Initialization Failed", MB_ICONSTOP);
			bResult = false;
			break;
	}

	// Get a list of available drives
	pndiDeviceInfos = NeroGetAvailableDrivesEx(MEDIA_CD, NULL);

	// Check whether any devices have been found 
	if (!pndiDeviceInfos)
	{
		AppendString("NeroGetAvailableDrivesEx() returned no available devices.");
		::AfxMessageBox("No Recording Devices", MB_ICONSTOP);
		bResult = false;
	}
	else
	{
		// Check the number of available devices to be sure
		if (pndiDeviceInfos->nsdisNumDevInfos > 0)
		{
			// Fill the ComboBox
#ifdef _DEBUG
			AppendString("Found the following devices:");
#endif
			for (DWORD dDeviceCounter = 0; dDeviceCounter < pndiDeviceInfos->nsdisNumDevInfos; dDeviceCounter++)
			{
#ifdef _DEBUG
				AppendString(pndiDeviceInfos->nsdisDevInfos[dDeviceCounter].nsdiDeviceName);
#endif

				// Add the device name to the ComboBox and get the index number
				int i = mcbxDevices.AddString(pndiDeviceInfos->nsdisDevInfos[dDeviceCounter].nsdiDeviceName);

				// Use the index number to access the corresponding entry
				// connect the entry's ItemData pointer to a NERO_DEVICE_INFO structure
				mcbxDevices.SetItemDataPtr(i, &pndiDeviceInfos->nsdisDevInfos[dDeviceCounter]);
			}

			// Select the first ComboBox entry
			mcbxDevices.SelectString(-1, pndiDeviceInfos->nsdisDevInfos[0].nsdiDeviceName);
		}
		else
		{
			AppendString("The number of available devices is 0.");
			::AfxMessageBox("No Recording Devices", MB_ICONSTOP);
			bResult = false;
		}
	}

	// Exit
	if (!bResult)
		OnCancel();
}

void CNeroBurnDlg::AppendString(CString str)
{
	// Store Last Appended String
	m_sLastAppended = str;

	// Make Sure the String Contains no \n without \r!
	int pos = 0;
	while ((pos = str.Find('\n', pos)) >= 0)
	{	
		str.Insert(pos, '\r');
		pos += 2;
	}

	// A CString for temporary use
	CString strBuffer;

	// Retrieve the content of the EditControl we use for messages
	medtMessages.GetWindowText(strBuffer);

	// Add a new line if the EditControl is not empty
	if (!strBuffer.IsEmpty())
	{
	 strBuffer += "\r\n";
	}
	// Append the string the function got as a parameter
	strBuffer += str;

	// Update the EditControl with the new content
	medtMessages.SetWindowText(strBuffer);

	// Scroll the edit control to the end
	medtMessages.LineScroll(medtMessages.GetLineCount(), 0);
}

NeroUserDlgInOut NERO_CALLBACK_ATTR CNeroBurnDlg::UserDialog(void *pUserData, NeroUserDlgInOut type, void *data)
{
  // handling of messages that require the user to perform an action
  // for reasons of brevity we only deal with the messages that 
  // are absolutely mandatory for this application

  switch (type)
  {
    case DLG_AUTO_INSERT:
      return DLG_RETURN_CONTINUE;
      break;
    case DLG_DISCONNECT_RESTART:
      return DLG_RETURN_ON_RESTART;
      break;
    case DLG_DISCONNECT:
      return DLG_RETURN_CONTINUE;
      break;
    case DLG_AUTO_INSERT_RESTART:
      return DLG_RETURN_EXIT;
      break;
    case DLG_RESTART:
      return DLG_RETURN_EXIT;
      break;
    case DLG_SETTINGS_RESTART:
      return DLG_RETURN_CONTINUE;
      break;
    case DLG_OVERBURN:
      return DLG_RETURN_TRUE;
      break;
    case DLG_AUDIO_PROBLEMS:
      return DLG_RETURN_EXIT;
      break;
	case DLG_NON_EMPTY_CDRW:
	{
		// Returning DLG_RETURN_EXIT will stop the burn process
		// Returning DLG_RETURN_CONTINUE will continue the burn process
		// Returning DLG_RETURN_RESTART will ask the user for an other CD

		// Returns estimated blanking time for loaded CD-RW in seconds, 
		// -1 if no CD inserted, 
		// -2 if recorder doesn't support CDRW
		// -3 if the inserted media is not rewritable
		int nTime = NeroGetCDRWErasingTime(((CNeroBurnDlg*)pUserData)->ndhDeviceHandle, NEROAPI_ERASE_QUICK);
		if (nTime >= 0)
		{
			CString sMsg;
			sMsg.Format("ReWritable Media is not empty. Do you want to format it?\nThis will take about %i seconds.", nTime);
			if (::AfxMessageBox(sMsg, MB_YESNO) == IDYES)
			{
				sMsg.Format("Erasing ReWritable Disc...Please Be Patient For %i Seconds.", nTime);
				((CNeroBurnDlg*)pUserData)->AppendString(sMsg);
				((CNeroBurnDlg*)pUserData)->BeginWaitCursor();
				int nRet = NeroEraseDisc(((CNeroBurnDlg*)pUserData)->ndhDeviceHandle, NEROAPI_ERASE_QUICK, 0, NULL);
				((CNeroBurnDlg*)pUserData)->EndWaitCursor();
				if (nRet == 0)
					return DLG_RETURN_CONTINUE;
				else
					return DLG_RETURN_EXIT;
			}
			else
				return DLG_RETURN_EXIT;
		}
		else if (nTime == -1)
		{
			::AfxMessageBox("No Media Inserted", MB_ICONSTOP);
			return DLG_RETURN_EXIT;
		}
		else if (nTime == -2)
		{
			::AfxMessageBox("ReWritable Medias not Supported", MB_ICONSTOP);
			return DLG_RETURN_EXIT;
		}
		else if (nTime == -3)
		{
			::AfxMessageBox("The Inserted Media is not ReWritable", MB_ICONSTOP);
			return DLG_RETURN_EXIT;
		}
		else
			return DLG_RETURN_EXIT;
		break;
	}
    case DLG_FILESEL_IMAGE:
      {
        // create filter for image files

        static char BASED_CODE szFilter[] = "Image Files (*.nrg)|*.nrg|All Files (*.*)|*.*||";

        // create a CFileDialog object. 
        // usage : CFileDialog( BOOL bOpenFileDialog, LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFileName = NULL,
        //                      DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCTSTR lpszFilter = NULL,
        //                      CWnd* pParentWnd = NULL );
        //
        // bOpenFileDialog = TRUE, create a File Open dialog 
        // lpszDefExt = NULL, do not automatically append a file extension
        // dwFlags = OFN_OVERWRITEPROMPT, makes no sense during file open,
        //           just in case we decide to use File Save later
        // szFilter = "Image Files (*.nrg)|*.nrg|All Files (*.*)|*.*||"
        // pParentWnd = ((CNeroBurnDlg*)pUserData), our current Dialog window is the parent

        CFileDialog dlgOpen(TRUE, NULL, "test.nrg", OFN_OVERWRITEPROMPT, szFilter, ((CNeroBurnDlg*)pUserData));

        // check how the dialog was ended

        if (dlgOpen.DoModal() == IDOK)
        {
          // user pressed "OK", copy the file name to the data parameter

          strcpy((char*)data,dlgOpen.GetPathName());

          // proceed with the burn process

          return DLG_RETURN_TRUE; 
        }
        else
        {
          // user canceled, do not proceed with the burn process

          return DLG_BURNIMAGE_CANCEL;
        }
      }
      break;
    case DLG_WAITCD:
    {
      NERO_WAITCD_TYPE waitcdType = (NERO_WAITCD_TYPE) (int)data;
	  
	  if (waitcdType == NERO_WAITCD_WRONG_MEDIUM)
	  {
			CString sWrongMediumText;
			if (((CNeroBurnDlg*)pUserData)->m_bBurningCD)
				sWrongMediumText = "Please Insert a CD!";
			else
				sWrongMediumText = "Please Insert a DVD!";

		  // Do Not Repeat the same Message!
		  if (((CNeroBurnDlg*)pUserData)->m_sLastAppended != sWrongMediumText)
			 ((CNeroBurnDlg*)pUserData)->AppendString(sWrongMediumText);
	  }
	  else
	  {
		  char *waitcdString = NeroGetLocalizedWaitCDTexts(waitcdType);
		  
		  // Do Not Repeat the same Message!
		  if (((CNeroBurnDlg*)pUserData)->m_sLastAppended != CString(waitcdString))
			 ((CNeroBurnDlg*)pUserData)->AppendString(waitcdString);

		  NeroFreeMem(waitcdString);
	  }
      
	  return DLG_RETURN_EXIT;
      break;
    }
    default:
      break;
  }

  // default return value, in case we forgot to handle a request
  
  return DLG_RETURN_EXIT;
}

BOOL NERO_CALLBACK_ATTR CNeroBurnDlg::ProgressCallback(void *pUserData, DWORD dwProgressInPercent)
{
	// the NeroAPI updates the current progress counter

	// set the progress bar to the percentage value that was passed to this function

	((CNeroBurnDlg*)pUserData)->mpgsProgress.SetPos(dwProgressInPercent);

	return true;
}

BOOL NERO_CALLBACK_ATTR CNeroBurnDlg::AbortedCallback(void *pUserData)
{
	// do not ask the user if he really wants to abort
	// just return the aborted flag

	return ((CNeroBurnDlg*)pUserData)->mbAborted;
}

void NERO_CALLBACK_ATTR CNeroBurnDlg::AddLogLine(void *pUserData, NERO_TEXT_TYPE type, const char *text)
{
	// Add the text that was passed to this function to the message log
#ifdef _DEBUG
	CString csTemp(text);
	((CNeroBurnDlg*)pUserData)->AppendString("Log line:" + csTemp);
#endif
	return;
}

void NERO_CALLBACK_ATTR CNeroBurnDlg::SetPhaseCallback(void *pUserData, const char *text)
{
	// display the current phase the burn process is currently going through
	CString csTemp(text);
	((CNeroBurnDlg*)pUserData)->AppendString("Phase: " + csTemp);
	return;
}

void CNeroBurnDlg::NeroAPIFree()
{
	// free the resources that have been used

	// make sure there is something to free so we do not run into an exception
	if (pndiDeviceInfos)
	{
		NeroFreeMem(pndiDeviceInfos);
	}

	// nothing to check before calling these functions
	NeroClearErrors();
	if (NeroDone())
	{
		::AfxMessageBox("Detected memory leaks in NeroBurn");
	}

	NeroAPIGlueDone();

	return;
}

void CNeroBurnDlg::OnOK() 
{
	if (!m_bBurning)
	{
		NeroAPIFree();
		CDialog::OnOK();
	}
}

void CNeroBurnDlg::OnCancel() 
{
	// Avoid exiting when the User presses
	// the Closing Cross while burning!
	if (!m_bBurning)
	{
		NeroAPIFree();
		CDialog::OnCancel();
	}
}

void CNeroBurnDlg::OnAbort() 
{
	mbAborted = true;
}


