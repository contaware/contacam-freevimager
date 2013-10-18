#include "stdafx.h"
#include "uImager.h"
#include "PictureDoc.h"
#include "VideoAviDoc.h"
#include "VideoDeviceDoc.h"
#include "AudioMCIDoc.h"
#include "CDAudioDoc.h"
#include "PictureView.h"
#include "VideoAviView.h"
#include "VideoDeviceView.h"
#include "MainFrm.h"
#include "DxCapture.h"
#include "GeneralPage.h"
#include "dbt.h"
#include "PicturePrintPreviewView.h"
#include "OutVolDlg.h"
#include "PlayerToolBarDlg.h"
#include "IMAPI2Dlg.h"
#include "BatchProcDlg.h"
#include "CPUCount.h"
#include "CPUSpeed.h"
#include "PostDelayedMessage.h"
#include "Dib.h"
#include "Quantizer.h"
#include "Tiff2Pdf.h"
#include "SaveFileDlg.h"
#include "NoVistaFileDlg.h"
#include "AVRec.h"

#ifdef _INC_WINDOWSX
// The following names from WINDOWSX.H collide with names in this file
#undef SubclassWindow
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib, "winmm.lib")

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_FILE_SELECTTWAINSOURCE, OnFileSelecttwainsource)
	ON_COMMAND(ID_FILE_ACQUIRE, OnFileAcquire)
	ON_WM_MENUSELECT()
	ON_COMMAND(ID_VIEW_ALL_PREVIOUS_PICTURE, OnViewAllPreviousPicture)
	ON_COMMAND(ID_VIEW_ALL_NEXT_PICTURE, OnViewAllNextPicture)
	ON_COMMAND(ID_VIEW_ALL_FIRST_PICTURE, OnViewAllFirstPicture)
	ON_COMMAND(ID_VIEW_ALL_LAST_PICTURE, OnViewAllLastPicture)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_COMMAND(ID_RESTORE, OnRestore)
	ON_UPDATE_COMMAND_UI(ID_RESTORE, OnUpdateRestore)
	ON_COMMAND(ID_MINIMIZE, OnMinimize)
	ON_UPDATE_COMMAND_UI(ID_MINIMIZE, OnUpdateMinimize)
	ON_COMMAND(ID_MAXIMIZE, OnMaximize)
	ON_UPDATE_COMMAND_UI(ID_MAXIMIZE, OnUpdateMaximize)
	ON_COMMAND(ID_MAINMONITOR, OnMainmonitor)
	ON_WM_MOVE()
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_FILE_ACQUIRE_TO_TIFF, OnFileAcquireToTiff)
	ON_COMMAND(ID_FILE_ACQUIRE_TO_PDF, OnFileAcquireToPdf)
	ON_COMMAND(ID_FILE_ACQUIRE_AND_EMAIL, OnFileAcquireAndEmail)
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_OPEN_FROM_TRAY, OnOpenFromTray)
	ON_WM_SYSCOMMAND()
	ON_WM_QUERYENDSESSION()
	ON_WM_ENDSESSION()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_INDICATOR_XCOORDINATE, OnXCoordinatesDoubleClick)
	ON_COMMAND(ID_INDICATOR_YCOORDINATE, OnYCoordinatesDoubleClick)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_XCOORDINATE, OnUpdateIndicatorXCoordinate)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_YCOORDINATE, OnUpdateIndicatorYCoordinate)
	ON_MESSAGE(WM_PROGRESS, OnProgress)
	ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
	ON_MESSAGE(WM_THREADSAFE_OPEN_DOC, OnThreadSafeOpenDoc)
	ON_MESSAGE(WM_SHRINKDOC_TERMINATED, OnShrinkDocTerminated)
	ON_MESSAGE(WM_TASKBAR_BUTTON, OnTaskBarButton)
	ON_MESSAGE(WM_RESTORE_FRAME, OnRestoreAllFrames)
	ON_MESSAGE(WM_ALL_CLOSED, OnAllClosed)
	ON_MESSAGE(WM_VIDEOAVI_FULLSCREEN_MODE_ON, OnVideoAviFullScreenModeOn)
	ON_MESSAGE(WM_VIDEOAVI_FULLSCREEN_MODE_OFF, OnVideoAviFullScreenModeOff)
	ON_MESSAGE(WM_SCANANDEMAIL, OnScanAndEmail)
	ON_MESSAGE(WM_TRAY_NOTIFICATION, OnTrayNotification)
	ON_MESSAGE(WM_COPYDATA, OnCopyData)
	ON_MESSAGE(WM_WTSSESSION_CHANGE, OnSessionChange)
	ON_MESSAGE(WM_TWAIN_CLOSED, OnTwainClosed)
#ifdef VIDEODEVICEDOC
	ON_MESSAGE(WM_AUTORUN_VIDEODEVICES, OnAutorunVideoDevices)
	ON_COMMAND(ID_VIEW_WEB, OnViewWeb)
	ON_COMMAND(ID_VIEW_FILES, OnViewFiles)
#endif
END_MESSAGE_MAP()

static TCHAR sba_CoordinateHelp[MAX_PATH];
static SBACTPANEINFO sba_indicators[] = 
{
	{ ID_SEPARATOR, _T(""), SBACTF_NORMAL },		// status line indicator
	{ ID_INDICATOR_PROGRESS, _T(""), SBACTF_NORMAL },
	{ ID_INDICATOR_XCOORDINATE, sba_CoordinateHelp, SBACTF_AUTOFIT | SBACTF_COMMAND | SBACTF_HANDCURSOR },
	{ ID_INDICATOR_YCOORDINATE, sba_CoordinateHelp, SBACTF_AUTOFIT | SBACTF_COMMAND | SBACTF_HANDCURSOR }, 
	{ ID_INDICATOR_CAPS, _T(""), SBACTF_NORMAL }, 
	{ ID_INDICATOR_NUM, _T(""), SBACTF_NORMAL }, 
	{ ID_INDICATOR_SCRL, _T(""), SBACTF_NORMAL }, 
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame() : m_TrayIcon(IDR_TRAYICON) // Menu ID
{
	m_bFullScreenMode = false;
	m_nFileMenuPos = -2;
	m_nEditMenuPos = -2;
	m_nViewMenuPos = -2;
	m_nCaptureMenuPos = -2;
	m_nPlayMenuPos = -2;
	m_nToolsMenuPos = -2;
	m_nWindowsPos = -2;
	m_nHelpMenuPos = -2;
	m_nHelpMenuItemsCount = -1;
	m_hMenu = NULL;
	m_bChildMax = false;
	m_bChildMin = false;
	m_bStatusBarWasVisible = false;
	m_bToolBarWasVisible = false;
	m_bChildToolBarWasVisible = false;
	m_dChildZoomFactor = 1.0;
	m_ptChildScrollPosition = CPoint(0,0);
	m_bScreenSaverWasActive = FALSE;
	m_SessionChangeTime = CTime::GetCurrentTime() - CTimeSpan(0, 0, 0, SESSIONCHANGE_WAIT_SEC);
	m_lSessionDisconnectedLockedCount = 0;
	m_sStatusBarString = _T("");
	m_bProgressIndicatorCreated = FALSE;
	m_sPlayGifMenuItem = _T("");
	m_sStopGifMenuItem = _T("");
	m_sNextPageFrameMenuItem = _T("");
	m_sPrevPageFrameMenuItem = _T("");
	m_hJPEGAdvancedMenu = NULL;
	m_sJPEGAdvancedMenuItem = _T("");
	m_TiffScan = NULL;
	m_bScanAndEmail = FALSE;
	m_pBatchProcDlg = NULL;
	m_pIMAPI2Dlg = NULL;
}

CMainFrame::~CMainFrame()
{

}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	if (CMDIFrameWnd::OnCreateClient(lpcs, pContext))
	{
		m_MDIClientWnd.SubclassWindow(m_hWndMDIClient);
		return TRUE;
	}
	else
		return FALSE;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED) ||
#ifdef VIDEODEVICEDOC
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME_VIDEODEVICEDOC))
#else
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
#endif
	{
		TRACE(_T("Failed to create toolbar\n"));
		return -1;      // fail to create
	}

	_tcsncpy(sba_CoordinateHelp, ML_STRING(1768, "Double-click to change unit"), MAX_PATH);
	sba_CoordinateHelp[MAX_PATH - 1] = _T('\0');
	if (!m_wndStatusBar.Create(this) || 
		!m_wndStatusBar.SetPanes(sba_indicators,
		  sizeof(sba_indicators)/sizeof(SBACTPANEINFO)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetHandCursor(IDC_HAND_CURSOR);

	// Toolbar
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	// Enable Drag'n'Drop
	DragAcceptFiles(TRUE);
	
#ifdef VIDEODEVICEDOC
	// Poll Timer
	SetTimer(ID_TIMER_ONESEC_POLL, ONESEC_POLL_TIMER_MS, NULL);
#endif

	// Init Menu Positions
	InitMenuPositions();
	
	// Set top most
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
	{
		((CUImagerApp*)::AfxGetApp())->m_bTopMost = (BOOL)((CUImagerApp*)::AfxGetApp())->GetProfileInt(_T("GeneralApp"), _T("TopMost"), FALSE);
		if (((CUImagerApp*)::AfxGetApp())->m_bTopMost)
			SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	// Allocate Dlg Objects
	((CUImagerApp*)::AfxGetApp())->m_pXmpImportDlg = new CXmpDlg(this, IDD_XMP_IMPORT);
	((CUImagerApp*)::AfxGetApp())->m_pXmpDlg = new CXmpDlg(this, IDD_XMP);

	// Setup Tray Icon
	if (((CUImagerApp*)::AfxGetApp())->m_bTrayIcon)
	{
		TrayIcon(TRUE);
		if (!((CUImagerApp*)::AfxGetApp())->m_bFirstRun && !((CUImagerApp*)::AfxGetApp())->m_bStartFullScreenMode)
			m_TrayIcon.MinimizeToTray();
	}

	// Register session change notification
#ifdef VIDEODEVICEDOC
	if (!((CUImagerApp*)::AfxGetApp())->m_bServiceProcess)
#endif
	{
		typedef BOOL (WINAPI * FPWTSREGISTERSESSIONNOTIFICATION)(HWND hWnd, DWORD dwFlags);
		HINSTANCE h = ::LoadLibrary(_T("Wtsapi32.dll"));
		if (h)
		{
			FPWTSREGISTERSESSIONNOTIFICATION fpWTSRegisterSessionNotification = (FPWTSREGISTERSESSIONNOTIFICATION)::GetProcAddress(h, "WTSRegisterSessionNotification");
			if (fpWTSRegisterSessionNotification)
				fpWTSRegisterSessionNotification(GetSafeHwnd(), NOTIFY_FOR_THIS_SESSION);
			::FreeLibrary(h);
		}
	}

	return 0;
}

void CMainFrame::OnXCoordinatesDoubleClick()
{
	ChangeCoordinatesUnit();
}

void CMainFrame::OnYCoordinatesDoubleClick()
{	
	ChangeCoordinatesUnit();
}

void CMainFrame::ChangeCoordinatesUnit()
{
	// Next Unit
	(((CUImagerApp*)::AfxGetApp())->m_nCoordinateUnit)++;
	if (((CUImagerApp*)::AfxGetApp())->m_nCoordinateUnit > COORDINATES_CM)
		((CUImagerApp*)::AfxGetApp())->m_nCoordinateUnit = COORDINATES_PIX;
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		((CUImagerApp*)::AfxGetApp())->WriteProfileInt(_T("GeneralApp"), _T("CoordinateUnit"), ((CUImagerApp*)::AfxGetApp())->m_nCoordinateUnit);

	// Update Active View's Pane and Status Text
	CMDIChildWnd* pChild = MDIGetActive();
	if (pChild)
	{
		CView* pView = pChild->GetActiveView();
		if (pView)
		{
			if (pView->IsKindOf(RUNTIME_CLASS(CUImagerView)))
				((CUImagerView*)pView)->UpdatePaneText();
			if (pView->IsKindOf(RUNTIME_CLASS(CPictureView)))
				((CPictureView*)pView)->UpdateCropStatusText();
			if (pView->IsKindOf(RUNTIME_CLASS(CPicturePrintPreviewView)))
			{
				((CPicturePrintPreviewView*)pView)->UpdatePaneText();
				((CPicturePrintPreviewView*)pView)->UpdateStatusText();
			}
		}
	}
}

void CMainFrame::TrayIcon(BOOL bEnable)
{
	if (bEnable)
	{
		m_TrayIcon.SetWnd(this, WM_TRAY_NOTIFICATION);
		m_TrayIcon.SetIcon(IDR_MAINFRAME);
	}
	else
	{
		if (m_TrayIcon.IsMinimizedToTray())
		{
			m_TrayIcon.MaximizeFromTray();
			ShowOwnedWindows(TRUE);
			((CUImagerApp*)::AfxGetApp())->PaintDocTitles();
		}
		m_TrayIcon.SetIcon(0);
	}
}

void CMainFrame::OnDestroy() 
{
	// Kill Timer
#ifdef VIDEODEVICEDOC
	KillTimer(ID_TIMER_ONESEC_POLL);
#endif

	// Base class
	CMDIFrameWnd::OnDestroy();

	// Free Dlg Objects
	if (((CUImagerApp*)::AfxGetApp())->m_pXmpImportDlg)
	{
		delete ((CUImagerApp*)::AfxGetApp())->m_pXmpImportDlg;
		((CUImagerApp*)::AfxGetApp())->m_pXmpImportDlg = NULL;
	}
	if (((CUImagerApp*)::AfxGetApp())->m_pXmpDlg)
	{
		delete ((CUImagerApp*)::AfxGetApp())->m_pXmpDlg;
		((CUImagerApp*)::AfxGetApp())->m_pXmpDlg = NULL;
	}
}

////////////////////////////////////////////////////////////////
// Handle notification from tray icon
//
LONG CMainFrame::OnTrayNotification(WPARAM uID, LPARAM lEvent)
{
	// Let tray icon do default stuff
    if (m_TrayIcon.OnTrayNotification(uID, lEvent) == 0)
        return 0;
	else 
	{
	    // If there's a resource menu with the same ID as the icon, use it as 
	    // the right-button popup menu. We will interpret the first
	    // item in the menu as the default command for WM_LBUTTONDBLCLK
	    CMenu menu;
	    if (!menu.LoadMenu(uID))
		    return 0;
	    CMenu* pSubMenu = menu.GetSubMenu(0);
	    if (!pSubMenu) 
		    return 0;

        /*CString string;
        string.LoadString(m_bEnabled ? IDI_DISABLE : IDI_ENABLE);
        pSubMenu->ModifyMenu(IDM_TRAY_ENABLE, MF_BYCOMMAND | MF_STRING, IDM_TRAY_ENABLE, string);*/

        if (lEvent == WM_RBUTTONUP)
		{

		    // Make first menu item the default (bold font)
		    ::SetMenuDefaultItem(pSubMenu->m_hMenu, 0, TRUE);

		    // Display the menu at the current mouse location. There's a "bug"
		    // (Microsoft calls it a feature) in Windows 95 that requires calling
		    // SetForegroundWindow. To find out more, search for Q135788 in MSDN.
		    CPoint mouse;
			::GetCursorPos(&mouse);
		    ::SetForegroundWindow(m_hWnd);	
		    ::TrackPopupMenu(pSubMenu->m_hMenu, 0, mouse.x, mouse.y, 0,
			    m_hWnd, NULL);

        }
		else // Double click: execute first menu item
		{
		    SendMessage(WM_COMMAND, pSubMenu->GetMenuItemID(0), 0);
        }
    }
    return 1;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

LONG CMainFrame::OnScanAndEmail(WPARAM wparam, LPARAM lparam)
{
	// Send
	((CUImagerApp*)::AfxGetApp())->SendMail(((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName);

	// Delete temporary directory containing tiff file and pdf file
	::DeleteDir(::GetDirName(((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName));

	return 1;
}

LONG CMainFrame::OnTwainClosed(WPARAM wparam, LPARAM lparam)
{
	if (m_TiffScan)
	{
		if (::AfxMessageBox(ML_STRING(1861, "Scan more pages?"),
							MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			// Acquire
			TwainAcquire(TWCPP_ANYCOUNT);
		}
		else
		{
			// Close TIFF
			::TIFFClose(m_TiffScan);
			m_TiffScan = NULL;

			// E-Mail it?
			if (m_bScanAndEmail)
			{
				BeginWaitCursor();
				if (::Tiff2Pdf(	((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName,
								((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName,
								_T("Fit"),
								TRUE,		// Fit Window
								TRUE,		// Interpolate
								((CUImagerApp*)::AfxGetApp())->m_nPdfScanCompressionQuality))
				{
					EndWaitCursor();
					PostMessage(WM_SCANANDEMAIL, 0, 0);
				}
				else
					EndWaitCursor();
			}
			else
			{
				// Pdf Wanted?
				if (((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName != _T(""))
				{
					BeginWaitCursor();
					if (::Tiff2Pdf(	((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName,
									((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName,
									_T("Fit"),
									TRUE,		// Fit Window
									TRUE,		// Interpolate
									((CUImagerApp*)::AfxGetApp())->m_nPdfScanCompressionQuality))
					{
						// End Wait Cursor
						EndWaitCursor();

						// Delete Tiff
						::DeleteFile(((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName);

						// Open Pdf with external program
						if (!((CUImagerApp*)::AfxGetApp())->m_bDisableExtProg)
							::ShellExecute(NULL, _T("open"), ((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName, NULL, NULL, SW_SHOWNORMAL);
					}
					else
						EndWaitCursor();
				}
				// Open Multi-Page Tiff
				else
					::AfxGetApp()->OpenDocumentFile(((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName);
			}
		}
	}

	return 1;
}

void CMainFrame::TwainCopyImage(HANDLE hBitmap, TW_IMAGEINFO& info)
{
	if (((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName != _T(""))
	{
		CDib Dib;

		// Copy Bitmap
		Dib.CopyFromHandle(hBitmap);

		// Count the Unique Colors
		int nNumColors = Dib.CountUniqueColors(this, TRUE);

		// Choose Right Compression Type
		int nCompression;
		if (nNumColors <= 2)
			nCompression = COMPRESSION_CCITTFAX4;
		else if (nNumColors <= 32)
		{
			// Convert to 4bpp
			if (info.BitsPerPixel != 4)
			{
				CQuantizer Quantizer(16, 8);
				Quantizer.ProcessImage(&Dib, this, TRUE);
				RGBQUAD* pColors = (RGBQUAD*)new RGBQUAD[16];
				Quantizer.SetColorTable(pColors);
				Dib.CreatePaletteFromColors(16, pColors);
				Dib.ConvertTo4bitsErrDiff(Dib.GetPalette(), this, TRUE);
				delete [] pColors;
			}
			nCompression = COMPRESSION_LZW;
		}
		else if (nNumColors <= 512)
		{
			// Convert to 8bpp
			if (info.BitsPerPixel != 8)
			{
				CQuantizer Quantizer(256, 8);
				Quantizer.ProcessImage(&Dib, this, TRUE);
				RGBQUAD* pColors = (RGBQUAD*)new RGBQUAD[256];
				Quantizer.SetColorTable(pColors);
				Dib.CreatePaletteFromColors(256, pColors);
				Dib.ConvertTo8bitsErrDiff(Dib.GetPalette(), this, TRUE);
				delete [] pColors;
			}

			// Choose Compression
			if (((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName != _T(""))
			{
				Dib.UpdateGrayscaleFlag();
				if (Dib.IsGrayscale())
					nCompression = COMPRESSION_JPEG;
				else
					nCompression = COMPRESSION_LZW;
			}
			else
				nCompression = COMPRESSION_LZW;
		}
		else
		{
			if (((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName != _T(""))
				nCompression = COMPRESSION_JPEG;
			else
				nCompression = COMPRESSION_LZW;
		}

		// First Page?
		if (m_TiffScan == NULL)
		{
			Dib.SaveFirstTIFF(	((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName,
								&m_TiffScan,
								0, // We do not know how many pages
								nCompression,
								((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName == _T("") ? DEFAULT_JPEGCOMPRESSION :
								((CUImagerApp*)::AfxGetApp())->m_nPdfScanCompressionQuality,
								this,
								TRUE);
		}
		else
		{
			Dib.SaveNextTIFF(	m_TiffScan,
								0, // We do not know the page number
								0, // We do not know how many pages
								nCompression,
								((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName == _T("") ? DEFAULT_JPEGCOMPRESSION :
								((CUImagerApp*)::AfxGetApp())->m_nPdfScanCompressionQuality,
								this,
								TRUE);
		}
	}
	else
	{
		// Make New Document
		CPictureDoc* pDoc = (CPictureDoc*)((CUImagerApp*)::AfxGetApp())->GetPictureDocTemplate()->OpenDocumentFile(NULL);
		if (pDoc)
		{
			// Check
			if (!pDoc->m_pDib)
			{
				pDoc->CloseDocumentForce();
				return;
			}

			// Get Bits
			pDoc->m_pDib->CopyFromHandle(hBitmap);
			pDoc->SetModifiedFlag();

			// Init Vars
			pDoc->m_DocRect.top = 0;
			pDoc->m_DocRect.left = 0;
			pDoc->m_DocRect.right = pDoc->m_pDib->GetWidth();
			pDoc->m_DocRect.bottom = pDoc->m_pDib->GetHeight();
			pDoc->m_nPixelAlignX = 1;
			pDoc->m_nPixelAlignY = 1;
			pDoc->SetDocumentTitle();

			// Zoom
			CZoomComboBox* pZoomCB = &(((CPictureToolBar*)((CToolBarChildFrame*)(pDoc->GetFrame()))->GetToolBar())->m_ZoomComboBox);
			pZoomCB->SetCurSel(pDoc->m_nZoomComboBoxIndex = 1); // Fit Big
			pZoomCB->OnChangeZoomFactor(*((double*)(pZoomCB->GetItemDataPtr(1))));

			// Fit to document
			if (!pDoc->GetFrame()->IsZoomed())
			{
				pDoc->GetView()->GetParentFrame()->SetWindowPos(NULL,
																0, 0, 0, 0,
																SWP_NOSIZE |
																SWP_NOZORDER);
				pDoc->GetView()->UpdateWindowSizes(FALSE, FALSE, TRUE);
			}
			else
				pDoc->GetView()->UpdateWindowSizes(TRUE, TRUE, FALSE);
		}
	}
}

LONG CMainFrame::OnAllClosed(WPARAM wparam, LPARAM lparam)
{
	if (((CUImagerApp*)::AfxGetApp())->m_bShuttingDownApplication)
		DestroyWindow();
	return 0;
}

void CMainFrame::OnClose() 
{
	// Already Exiting?
	if (((CUImagerApp*)::AfxGetApp())->m_bShuttingDownApplication)
		return;

	// From CFrameWnd::OnClose():

	if (m_lpfnCloseProc != NULL)
		(*m_lpfnCloseProc)(this);

	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();
	if (pApp != NULL && pApp->m_pMainWnd == this)
	{
		// Attempt to save all documents
		if (!pApp->SaveAllModified())
			return;     // don't close it

		// Let the user close the dialog(s),
		// some processing may still be running!
		if (m_pBatchProcDlg && m_pIMAPI2Dlg)
		{
			::MessageBeep(0xFFFFFFFF);
			m_pIMAPI2Dlg->SetActiveWindow();
			m_pIMAPI2Dlg->SetFocus();
			return;		// don't close it
		}
		else if (m_pBatchProcDlg)
		{
			::MessageBeep(0xFFFFFFFF);
			m_pBatchProcDlg->SetActiveWindow();
			m_pBatchProcDlg->SetFocus();
			return;		// don't close it
		}
		else if (m_pIMAPI2Dlg)
		{
			::MessageBeep(0xFFFFFFFF);
			m_pIMAPI2Dlg->SetActiveWindow();
			m_pIMAPI2Dlg->SetFocus();
			return;		// don't close it
		}

		// Unregister session change notification
#ifdef VIDEODEVICEDOC
		if (!((CUImagerApp*)::AfxGetApp())->m_bServiceProcess)
#endif
		{
			typedef BOOL (WINAPI * FPWTSUNREGISTERSESSIONNOTIFICATION)(HWND hWnd);
			HINSTANCE h = ::LoadLibrary(_T("Wtsapi32.dll"));
			if (h)
			{
				FPWTSUNREGISTERSESSIONNOTIFICATION fpWTSUnRegisterSessionNotification = (FPWTSUNREGISTERSESSIONNOTIFICATION)::GetProcAddress(h, "WTSUnRegisterSessionNotification");
				if (fpWTSUnRegisterSessionNotification)
					fpWTSUnRegisterSessionNotification(GetSafeHwnd());
				::FreeLibrary(h);
			}
		}

		// Stop All Threads used for the PostDelayedMessage() Function
		CPostDelayedMessageThread::Exit();

		// Save Settings
		if (pApp->m_bUseSettings)
			pApp->SaveSettings();

		// Release Twain
		ReleaseTwain();

		// Hide the application's windows before closing all the documents
		pApp->HideApplication();

		// Set Flag
		pApp->m_bShuttingDownApplication = TRUE;

		// Do Not Call pApp->CloseAllDocuments because it calls
		// CDocument::OnCloseDocument(), which destroys the attached views.
		// Instead Post WM_CLOSE Messages!
		//pApp->CloseAllDocuments(FALSE);
		pApp->CloseAll();

		// Don't exit if there are outstanding component objects
		if (!AfxOleCanExitApp())
		{
			// take user out of control of the app
			AfxOleSetUserCtrl(FALSE);

			// don't destroy the main window and close down just yet
			//  (there are outstanding component (OLE) objects)
			return;
		}

		// There are cases where destroying the documents may destroy the
		// main window of the application.
		if (!afxContextIsDLL && pApp->m_pMainWnd == NULL)
		{
			AfxPostQuitMessage(0);
			return;
		}
	}	
}

void CMainFrame::OnFileAcquire() 
{
	// Init and check
	if (!TwainIsValidDriver())
	{
		if (!InitTwain(GetSafeHwnd()))
		{
			::AfxMessageBox(ML_STRING(1223, "No twain driver found."), MB_OK | MB_ICONSTOP);
			return;
		}
		TwainSelectDefaultSource();
	}
	if (!TwainIsSourceSelected())
	{
		::AfxMessageBox(ML_STRING(1224, "No twain source found."), MB_OK | MB_ICONINFORMATION);
		return;
	}

	// Reset All
	m_bScanAndEmail = FALSE;
	((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName = _T("");
	((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName = _T("");

	// Acquire
	TwainAcquire(TWCPP_ANYCOUNT);
}

void CMainFrame::OnFileAcquireToTiff() 
{
	// Init and check
	if (!TwainIsValidDriver())
	{
		if (!InitTwain(GetSafeHwnd()))
		{
			::AfxMessageBox(ML_STRING(1223, "No twain driver found."), MB_OK | MB_ICONSTOP);
			return;
		}
		TwainSelectDefaultSource();
	}
	if (!TwainIsSourceSelected())
	{
		::AfxMessageBox(ML_STRING(1224, "No twain source found."), MB_OK | MB_ICONINFORMATION);
		return;
	}

	// Reset Flag
	m_bScanAndEmail = FALSE;

	// Display the Save As Dialog
	TCHAR szFileName[MAX_PATH];
	CNoVistaFileDlg dlgFile(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
	{
		((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName = ::AfxGetApp()->GetProfileString(_T("GeneralApp"), _T("TiffScanFileName"), _T("scan.tif"));
		if (!::IsExistingDir(::GetDriveAndDirName(((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName)))
			((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName = ::GetShortFileName(((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName);
	}
	else
		((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName = _T("scan.tif");
	_tcscpy(szFileName, ((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName);
	dlgFile.m_ofn.lpstrFile = szFileName;
	dlgFile.m_ofn.nMaxFile = MAX_PATH;
	dlgFile.m_ofn.lpstrCustomFilter = NULL;
	dlgFile.m_ofn.lpstrFilter = _T("Tag Image File Format (*.tif;*.tiff;*.jfx)\0*.tif;*.tiff;*.jfx\0");
	dlgFile.m_ofn.lpstrDefExt = _T("tif");
	if (dlgFile.DoModal() == IDOK)
	{
		((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName = szFileName;
		if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
			::AfxGetApp()->WriteProfileString(_T("GeneralApp"), _T("TiffScanFileName"), ((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName);
		((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName = _T("");
		TwainAcquire(TWCPP_ANYCOUNT);
	}
	else
	{
		((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName = _T("");
		((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName = _T("");
	}
}

void CMainFrame::OnFileAcquireToPdf() 
{
	// Init and check
	if (!TwainIsValidDriver())
	{
		if (!InitTwain(GetSafeHwnd()))
		{
			::AfxMessageBox(ML_STRING(1223, "No twain driver found."), MB_OK | MB_ICONSTOP);
			return;
		}
		TwainSelectDefaultSource();
	}
	if (!TwainIsSourceSelected())
	{
		::AfxMessageBox(ML_STRING(1224, "No twain source found."), MB_OK | MB_ICONINFORMATION);
		return;
	}

	// Reset Flag
	m_bScanAndEmail = FALSE;

	// Display the Save As Dialog
	TCHAR szFileName[MAX_PATH];
	CSaveFileDlg dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
	{
		((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName = ::AfxGetApp()->GetProfileString(_T("GeneralApp"), _T("PdfScanFileName"), _T("scan.pdf"));
		if (!::IsExistingDir(::GetDriveAndDirName(((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName)))
			((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName = ::GetShortFileName(((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName);
	}
	else
		((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName = _T("scan.pdf");
	_tcscpy(szFileName, ((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName);
	dlgFile.m_ofn.lpstrFile = szFileName;
	dlgFile.m_ofn.nMaxFile = MAX_PATH;
	dlgFile.m_ofn.lpstrCustomFilter = NULL;
	dlgFile.m_ofn.lpstrFilter = _T("Pdf Document (*.pdf)\0*.pdf\0");
	dlgFile.m_ofn.lpstrDefExt = _T("pdf");
	if (dlgFile.DoModal() == IDOK)
	{
		((CUImagerApp*)::AfxGetApp())->m_nPdfScanCompressionQuality = dlgFile.GetJpegCompressionQuality();
		((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName = szFileName;
		if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
			::AfxGetApp()->WriteProfileString(_T("GeneralApp"), _T("PdfScanFileName"), ((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName);
		((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName = ::MakeTempFileName(	((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),
																					::GetFileNameNoExt(((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName) + _T(".tif"));

		// Acquire
		TwainAcquire(TWCPP_ANYCOUNT);
	}
	else
	{
		((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName = _T("");
		((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName = _T("");
	}
}

void CMainFrame::OnFileAcquireAndEmail() 
{
	// Init and check
	if (!TwainIsValidDriver())
	{
		if (!InitTwain(GetSafeHwnd()))
		{
			::AfxMessageBox(ML_STRING(1223, "No twain driver found."), MB_OK | MB_ICONSTOP);
			return;
		}
		TwainSelectDefaultSource();
	}
	if (!TwainIsSourceSelected())
	{
		::AfxMessageBox(ML_STRING(1224, "No twain source found."), MB_OK | MB_ICONINFORMATION);
		return;
	}
	if (!((CUImagerApp*)::AfxGetApp())->m_bMailAvailable)
	{
		::AfxMessageBox(ML_STRING(1175, "No Email Program Installed."), MB_OK | MB_ICONINFORMATION);
		return;
	}

	// Create & Empty Temp Dir
	CString sScanAndEmailDir;
	sScanAndEmailDir.Format(_T("ScanAndEmail%X"), ::GetCurrentProcessId());
	sScanAndEmailDir = ((CUImagerApp*)::AfxGetApp())->GetAppTempDir() + sScanAndEmailDir;
	if (!::IsExistingDir(sScanAndEmailDir))
	{
		if (!::CreateDir(sScanAndEmailDir))
		{
			::ShowLastError(TRUE);
			return;
		}
	}
	else
	{
		if (!::DeleteDirContent(sScanAndEmailDir))
		{
			::AfxMessageBox(ML_STRING(1225, "Error While Deleting The Temporary Folder."), MB_OK | MB_ICONSTOP);
			return;
		}
	}

	// Set Temp File Names
	((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName = sScanAndEmailDir + _T("\\Document.pdf");
	((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName = sScanAndEmailDir + _T("\\Document.tif");

	// Set Scan & Email Flag
	m_bScanAndEmail = TRUE;

	// Set the default compression
	((CUImagerApp*)::AfxGetApp())->m_nPdfScanCompressionQuality = DEFAULT_JPEGCOMPRESSION;
		
	// Acquire
	TwainAcquire(TWCPP_ANYCOUNT);
}

void CMainFrame::OnFileSelecttwainsource() 
{
	if (!TwainIsValidDriver())
	{
		if (!InitTwain(GetSafeHwnd()))
		{
			::AfxMessageBox(ML_STRING(1223, "No twain driver found."), MB_OK | MB_ICONSTOP);
			return;
		}
	}
	TwainSelectSource();
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
	// TWAIN
	if (TwainProcessMessage(pMsg))
	{
		//TRACE(_T("Twain Msg: 0x%08X , wparam=%i , lparam=%i , hwnd=0x%08X , this hwnd=0x%08X\n"), pMsg->message, pMsg->wParam, pMsg->lParam, pMsg->hwnd, GetSafeHwnd());
		return TRUE; // TWAIN message processed
	}

	// For Print Preview Mouse Scroll
	if (pMsg->message == WM_MOUSEWHEEL)
	{
		CMDIChildWnd* pChild = MDIGetActive();
		if (pChild)
		{
			CView* pView = pChild->GetActiveView();
			if (pView && pView->IsKindOf(RUNTIME_CLASS(CPicturePrintPreviewView)))
			{
				if (((CPicturePrintPreviewView*)pView)->m_pScaleEdit && 
					(CWnd::GetFocus() == (CWnd*)(((CPicturePrintPreviewView*)pView)->m_pScaleEdit)))
				{
					pView->SetFocus(); // To Remove Focus From CDialogBar's Edit Box in Print Preview Mode
					pView->SendMessage(WM_MOUSEWHEEL, pMsg->wParam, pMsg->lParam);
					return TRUE; // Do Not Dispatch
				}
				else
					pView->SendMessage(WM_MOUSEWHEEL, pMsg->wParam, pMsg->lParam);
			}
		}
	}
	// To Remove Focus From CDialogBar's Edit Box in Print Preview Mode
	else if (	pMsg->message == WM_LBUTTONDBLCLK ||
				pMsg->message == WM_LBUTTONDOWN)
	{
		CMDIChildWnd* pChild = MDIGetActive();
		if (pChild)
		{
			CView* pView = pChild->GetActiveView();
			if (pView && pView->IsKindOf(RUNTIME_CLASS(CPicturePrintPreviewView)))
				pView->SetFocus();
		}
	}
	// App Exit Pressing ESC
	else if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		if (!((CUImagerApp*)::AfxGetApp())->AreDocsOpen())
			PostMessage(WM_CLOSE, 0, 0);
	}
	
	return CMDIFrameWnd::PreTranslateMessage(pMsg);
}

LONG CMainFrame::OnThreadSafeOpenDoc(WPARAM wparam, LPARAM lparam)
{
	CString* pFileName = (CString*)wparam;
	if (pFileName)
	{
		CDocument* pDoc = NULL;
		if (!m_bFullScreenMode)
			pDoc = ((CUImagerApp*)::AfxGetApp())->OpenDocumentFile(*pFileName);
		delete pFileName;
		if (pDoc)
			return 1;
		else
			return 0;
	}
	else
		return 0;
}

#ifdef VIDEODEVICEDOC
LONG CMainFrame::OnAutorunVideoDevices(WPARAM wparam, LPARAM lparam)
{
	((CUImagerApp*)::AfxGetApp())->AutorunVideoDevices((int)wparam);
	return 0;
}

void CMainFrame::OnViewWeb()
{
	CMDIChildWnd* pChild = MDIGetActive();
	if (pChild)
	{
		CVideoDeviceDoc* pDoc = (CVideoDeviceDoc*)pChild->GetActiveDocument();
		if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CVideoDeviceDoc)))
		{
			pDoc->ViewWeb();
			return;
		}
	}
	m_MDIClientWnd.ViewWeb(_T("localhost"));
}

void CMainFrame::OnViewFiles()
{
	CMDIChildWnd* pChild = MDIGetActive();
	if (pChild)
	{
		CVideoDeviceDoc* pDoc = (CVideoDeviceDoc*)pChild->GetActiveDocument();
		if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CVideoDeviceDoc)))
		{
			pDoc->ViewFiles();
			return;
		}
	}
	::ShellExecute(	NULL,
					_T("open"),
					((CUImagerApp*)::AfxGetApp())->m_sMicroApacheDocRoot,
					NULL,
					NULL,
					SW_SHOWNORMAL);
}
#endif				

LONG CMainFrame::OnShrinkDocTerminated(WPARAM wparam, LPARAM lparam)
{
	POSITION pos;
	CUImagerMultiDocTemplate* curTemplate = NULL;
	BOOL bOk = (BOOL)wparam;
	CVideoAviDoc* pDocFrom = (CVideoAviDoc*)lparam;

	// Message From a VideoAviDoc Processing Thread
	if (pDocFrom && ((CUImagerApp*)::AfxGetApp())->IsDoc(pDocFrom))
	{
		// Restore Frame
		if (pDocFrom->m_pAVIPlay &&
			pDocFrom->m_pAVIPlay->HasVideo() &&
			(pDocFrom->m_nActiveVideoStream >= 0))
		{
			pDocFrom->RestoreFrame(THREAD_SAFE_UPDATEWINDOWSIZES_DELAY);
		}
		else
		{
			CPostDelayedMessageThread::PostDelayedMessage(
									pDocFrom->GetView()->GetSafeHwnd(),
									WM_THREADSAFE_UPDATEWINDOWSIZES,
									THREAD_SAFE_UPDATEWINDOWSIZES_DELAY,
									(WPARAM)(UPDATEWINDOWSIZES_INVALIDATE),
									(LPARAM)0);
		}

		// Check whether all video avi processing threads have terminated
		BOOL bRunning = FALSE;
		curTemplate = ((CUImagerApp*)::AfxGetApp())->GetVideoAviDocTemplate();
		pos = curTemplate->GetFirstDocPosition();
		while (pos)
		{
			CVideoAviDoc* pDoc = (CVideoAviDoc*)curTemplate->GetNextDoc(pos);
			if ((pDoc != pDocFrom) && pDoc->m_ProcessingThread.IsRunning())
				return 0;
		}
		
		// Done
		if (((CUImagerApp*)::AfxGetApp())->m_bWaitingMailFinish)
			((CUImagerApp*)::AfxGetApp())->SendDocAsMailFinish(TRUE);

		return 1;
	}
	else
		return 0;
}

void CMainFrame::EnterExitFullscreen()
{
	// Available only if there is an active doc
	CMDIChildWnd* pChild = MDIGetActive();
	if (!pChild)
		return;

	// Doc
	CPictureDoc* pDoc = (CPictureDoc*)pChild->GetActiveDocument();
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)))
	{
		// Close Print Preview
		if (pDoc->GetView()->GetPicturePrintPreviewView())
			pDoc->GetView()->GetPicturePrintPreviewView()->Close();

		// Pause SlideShow?
		BOOL bSlideShowWasRunning = FALSE;
		if (pDoc->m_SlideShowThread.IsSlideshowRunning())
		{
			bSlideShowWasRunning = TRUE;
			pDoc->m_SlideShowThread.PauseSlideshow();
		}

		// Cancel Transitions
		pDoc->CancelTransition();
		pDoc->CancelLoadFullJpegTransition();

		// Enter / Exit Full-Screen Mode
		if (m_bFullScreenMode)
		{
			// Close OSD
			pDoc->ShowOsd(FALSE);
			
			// Full-Screen Mode Off 
			FullScreenModeOff();
		}
		else
		{
			// Lossless crop needs integer zoom factors!
			if (pDoc->m_bCrop && pDoc->m_bLosslessCrop)
				pDoc->CancelCrop();
			if (pDoc->m_bZoomTool)
				pDoc->CancelZoomTool();
			
			// Full-Screen Mode On 
			FullScreenModeOn();

			// Show OSD
			if (pDoc->m_bEnableOsd)
				pDoc->ShowOsd(TRUE);
		}

		// Restart SlideShow
		if (bSlideShowWasRunning)
			pDoc->m_SlideShowThread.RunSlideshow();
	}
	else
	{
		// Enter / Exit Full-Screen Mode
		if (m_bFullScreenMode)
			FullScreenModeOff();
		else
			FullScreenModeOn();
	}
}

BOOL CMainFrame::FullScreenTo(BOOL bNextMonitor)
{
	CRect rcMonitor;
	if (bNextMonitor)
		rcMonitor = GetNextMonitorFullRect();
	else
		rcMonitor = GetPreviousMonitorFullRect();
	return FullScreenTo(rcMonitor);
}

BOOL CMainFrame::FullScreenTo(const CRect& rcMonitor)
{
	// Available only if there is an active doc
	CMDIChildWnd* pChild = MDIGetActive();
	if (!pChild)
		return FALSE;

	// Move it
	CUImagerView* pView = (CUImagerView*)pChild->GetActiveView();
	if (pView && pView->IsKindOf(RUNTIME_CLASS(CUImagerView)))
	{
		int nMonitorWidth = rcMonitor.right - rcMonitor.left;
		int nMonitorHeight = rcMonitor.bottom - rcMonitor.top;
		WINDOWPLACEMENT FullScreenMainFrameWndPlacement;
		memcpy(&FullScreenMainFrameWndPlacement, &m_MainFrameWndPlacement, sizeof(WINDOWPLACEMENT));
		FullScreenMainFrameWndPlacement.showCmd = SW_RESTORE;
		FullScreenMainFrameWndPlacement.rcNormalPosition = rcMonitor;
		::SetWindowPlacement(GetSafeHwnd(), &FullScreenMainFrameWndPlacement);
		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		pView->SetWindowPos(NULL, -2, -2, nMonitorWidth + 4, nMonitorHeight + 4, SWP_NOZORDER);
		pView->UpdateWindowSizes(TRUE, TRUE, FALSE);
		return TRUE;
	}
	else
		return FALSE;
}

void CMainFrame::FullScreenModeOn(HWND hChildWndSafePaused/*=NULL*/)
{
	// Return if already in fullscreen mode
	if (m_bFullScreenMode)
		return;

	// Return if CMainFrame is not active
	CWnd* pActiveWnd = CWnd::GetActiveWindow();
	if (!(pActiveWnd && pActiveWnd->IsKindOf(RUNTIME_CLASS(CMainFrame))))
		return;

	// Get Child
	if (hChildWndSafePaused)
	{
		if (::IsWindow(hChildWndSafePaused))
			::SendMessage(m_hWndMDIClient, WM_MDIACTIVATE, (WPARAM)hChildWndSafePaused, 0); // Activate it
		else
			return; // We got an old message, child window has already been closed
	}
	CMDIChildWnd* pChild = MDIGetActive();
	if (!pChild)
		return;

	// View
	CView* pActiveView = pChild->GetActiveView();
	if (!(pActiveView && pActiveView->IsKindOf(RUNTIME_CLASS(CUImagerView))))
		return;
	CUImagerView* pView = (CUImagerView*)pActiveView;

	// Doc
	CUImagerDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->IsKindOf(RUNTIME_CLASS(CVideoAviDoc)))
	{
		// Safe Pause
		if (!hChildWndSafePaused)
		{
			DWORD dwSafePausedMsgTimeout = 0U;
			double dPlay = (((CVideoAviDoc*)pDoc)->m_PlayVideoFileThread.GetPlaySpeedPercent() * ((CVideoAviDoc*)pDoc)->GetPlayFrameRate());
			if (dPlay > 0.0)
				dwSafePausedMsgTimeout = (DWORD)Round(FULLSCREEN_SAFEPAUSED_FRAMES_TIMEOUT * 100000.0 / dPlay); 
			dwSafePausedMsgTimeout = MAX(FULLSCREEN_MIN_SAFEPAUSED_TIMEOUT, dwSafePausedMsgTimeout);
			((CVideoAviDoc*)pDoc)->m_PlayVideoFileThread.SafePauseDelayedRestart(	GetSafeHwnd(),
																					WM_VIDEOAVI_FULLSCREEN_MODE_ON,
																					(WPARAM)pChild->GetSafeHwnd(),
																					(LPARAM)0,
																					dwSafePausedMsgTimeout,
																					FULLSCREEN_DELAYEDRESTART_TIMEOUT,
																					FALSE);
			return;
		}

		// No drawing
		((CVideoAviDoc*)pDoc)->m_bNoDrawing = TRUE;

		// No Flip (do Blt) if some modeless dialogs are visible
		if (AreModelessDlgsVisible())
			((CVideoAviDoc*)pDoc)->m_PlayVideoFileThread.SetFullScreenBlt();
	}

	// Store placement
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		((CUImagerApp*)::AfxGetApp())->SaveSettings();

	// Get Current Monitor Rectangle
	m_rcEnterFullScreenMonitor = GetMonitorFullRect();

	// Disable Screensaver if it is Enabled
	::SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &m_bScreenSaverWasActive, 0);
	if (m_bScreenSaverWasActive)
		::SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, NULL, 0);

	// Store the MainFrame Placement
	GetWindowRect(&m_MainWndRect);
	memset(&m_MainFrameWndPlacement, 0, sizeof(m_MainFrameWndPlacement));
	m_MainFrameWndPlacement.length = sizeof(m_MainFrameWndPlacement);
	::GetWindowPlacement(GetSafeHwnd(), &m_MainFrameWndPlacement);

	// Store the Zoom Factor
	m_dChildZoomFactor = pDoc->m_dZoomFactor;

	// Store Child Placement
	memset(&m_ChildWndPlacement, 0, sizeof(m_ChildWndPlacement));
	m_ChildWndPlacement.length = sizeof(m_ChildWndPlacement);
	::GetWindowPlacement(pChild->GetSafeHwnd(), &m_ChildWndPlacement);
	
	// Store View Scroll Position
	if (pView->IsXOrYScroll())
		m_ptChildScrollPosition = pView->GetScrollPosition();

	// Store the Toolbars and Statusbar States and hide them
	m_bToolBarWasVisible = (m_wndToolBar.IsWindowVisible() != FALSE);
	m_wndToolBar.ShowWindow(SW_HIDE);
	CToolBar* pChildToolBar = ((CToolBarChildFrame*)pChild)->GetToolBar(); 
	if (pChildToolBar)
	{
		m_bChildToolBarWasVisible = (pChildToolBar->IsWindowVisible() != FALSE);
		pChildToolBar->ShowWindow(SW_HIDE);
	}
	m_bStatusBarWasVisible = (m_wndStatusBar.IsWindowVisible() != FALSE);
	m_wndStatusBar.ShowWindow(SW_HIDE);

	// Hide Menu
	m_hMenu = ::GetMenu(GetSafeHwnd());
	::SetMenu(GetSafeHwnd(), NULL);

	// Check the Child state and Maximize
	LONG style = ::GetWindowLong(pChild->m_hWnd, GWL_STYLE);
	if (style & WS_MAXIMIZE)
	{
		m_bChildMax = true;
		m_bChildMin = false;
	}
	else if (style & WS_MINIMIZE)
	{
		TRACE(_T("Child is minimized\n"));
		m_bChildMin = true;
		m_bChildMax = false;
		// Maximize the child window
		// (it will remove its caption)
		pChild->ShowWindow(SW_SHOWMAXIMIZED);
	}
	else
	{
		m_bChildMax = false;
		m_bChildMin = false;
		// Maximize the child window
		// (it will remove its caption)
		pChild->ShowWindow(SW_SHOWMAXIMIZED);
	}

	// Full Screen Mode Flags
	m_bFullScreenMode = true;
	pView->m_bFullScreenMode = true;

	// Change Styles For Full-Screen
	style = m_lOldStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
	style &= ~WS_CAPTION;
	style &= ~WS_SYSMENU;
	style &= ~WS_THICKFRAME;
	style &= ~WS_GROUP;
	style &= ~WS_TABSTOP;
	style &= ~WS_CLIPSIBLINGS;	
	::SetWindowLong(m_hWnd, GWL_STYLE, style);

	m_lOldChildExStyle = ::GetWindowLong(pChild->GetSafeHwnd(), GWL_EXSTYLE);
	style = m_lOldChildStyle =::GetWindowLong(pChild->GetSafeHwnd(), GWL_STYLE);
	//style &= ~WS_CAPTION; Leave this commented out otherwise Win Vista with Aero
	//						is stopping the repaint of the borders when exiting full-screen!!
	style &= ~WS_SYSMENU;
	style &= ~WS_THICKFRAME;
	style &= ~WS_GROUP;
	style &= ~WS_TABSTOP;
	style &= ~WS_CLIPSIBLINGS;
	::SetWindowLong(pChild->GetSafeHwnd(), GWL_STYLE, style);

	// Size MainFrame to Full-Screen and set as Top-Most
	// (In restored state because DirectDraw is not
	// working with a Maximized MainFrame!)
	CRect rcMonitor = GetMonitorFullRect();
	int nMonitorWidth = rcMonitor.right - rcMonitor.left;
	int nMonitorHeight = rcMonitor.bottom - rcMonitor.top;
	WINDOWPLACEMENT FullScreenMainFrameWndPlacement;
	memcpy(&FullScreenMainFrameWndPlacement, &m_MainFrameWndPlacement, sizeof(WINDOWPLACEMENT));
	FullScreenMainFrameWndPlacement.showCmd = SW_RESTORE;
	FullScreenMainFrameWndPlacement.rcNormalPosition = rcMonitor;
	::SetWindowPlacement(GetSafeHwnd(), &FullScreenMainFrameWndPlacement);
	SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	// Hack to Not Show the Child Borders
	pView->SetWindowPos(NULL, -2, -2, nMonitorWidth + 4, nMonitorHeight + 4, SWP_NOZORDER);

	// Start Full-Screen Timer for Cursor Hiding
	// and to call ResetFullScreenBlt() for CVideoAviDoc
	// if no modeless dialogs are visible
	pView->m_nMouseHideTimerCount = 0;
	pView->m_nMouseMoveCount = 0;
	pView->SetTimer(ID_TIMER_FULLSCREEN, FULLSCREEN_TIMER_MS, NULL);

	// Special Additional Commands for CVideoAviDoc
	if (pDoc->IsKindOf(RUNTIME_CLASS(CVideoAviDoc)))
	{
		// Restore previous m_UserZoomRect
		pView->m_UserZoomRect = ((CVideoAviDoc*)pDoc)->m_PrevUserZoomRect;

		// Get Video Stream
		CAVIPlay::CAVIVideoStream* pVideoStream = NULL;
		if (((CVideoAviDoc*)pDoc)->m_pAVIPlay)
			pVideoStream = ((CVideoAviDoc*)pDoc)->m_pAVIPlay->GetVideoStream(((CVideoAviDoc*)pDoc)->m_nActiveVideoStream);
		if (pVideoStream)
		{
			// Enter CS
			if (((CVideoAviDoc*)pDoc)->m_DxDraw.HasDxDraw())
				((CVideoAviDoc*)pDoc)->m_DxDraw.EnterCS();

			// Initialize the DirectDraw Interface
			if (((CVideoAviDoc*)pDoc)->m_DxDraw.IsInit() &&
				((CVideoAviDoc*)pDoc)->m_bUseDxDraw)
			{
				// Init DxDraw in Exclusive mode if we are the only open document
				BOOL bExclusive = (((CUImagerApp*)::AfxGetApp())->GetOpenDocsCount() == 1);
				((CVideoAviDoc*)pDoc)->m_DxDraw.InitFullScreen(	bExclusive ? GetSafeHwnd() : pView->GetSafeHwnd(),
																((CVideoAviDoc*)pDoc)->m_DocRect.right,
																((CVideoAviDoc*)pDoc)->m_DocRect.bottom,
																bExclusive,
																pVideoStream->GetFourCC(false),
																IDB_TELETEXT_DH_26);

				// Leave CS
				((CVideoAviDoc*)pDoc)->m_DxDraw.LeaveCS();
			}
			else
			{
				// Leave CS
				if (((CVideoAviDoc*)pDoc)->m_DxDraw.HasDxDraw())
					((CVideoAviDoc*)pDoc)->m_DxDraw.LeaveCS();
			}
		}

		// Calc. m_ZoomRect
		pView->UpdateZoomRect();

		// Set User Zoom Rect
		if (((CVideoAviDoc*)pDoc)->GetView()->m_UserZoomRect == CRect(0,0,0,0))
			((CVideoAviDoc*)pDoc)->GetView()->m_UserZoomRect = pView->m_ZoomRect;

		// Do Draw
		((CVideoAviDoc*)pDoc)->m_bNoDrawing = FALSE;

		// Restart
		((CVideoAviDoc*)pDoc)->m_PlayVideoFileThread.SetSafePauseRestartEvent();
	}

	// Update View
	pView->UpdateWindowSizes(TRUE, TRUE, FALSE);
}

LONG CMainFrame::OnVideoAviFullScreenModeOn(WPARAM wparam, LPARAM lparam)
{
	FullScreenModeOn((HWND)wparam);
	return 1;
}

void CMainFrame::FullScreenModeOff(HWND hChildWndSafePaused/*=NULL*/)
{
	CUImagerView* pView = NULL;
	CUImagerDoc* pDoc = NULL;

	// Return if not in fullscreen mode
	if (!m_bFullScreenMode)
		return;

	// Re-enable Screensaver if it was Enabled
	if (m_bScreenSaverWasActive)
		::SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, NULL, 0);

	// Get Child
	if (hChildWndSafePaused)
	{
		if (::IsWindow(hChildWndSafePaused))
			::SendMessage(m_hWndMDIClient, WM_MDIACTIVATE, (WPARAM)hChildWndSafePaused, 0); // Activate it
		else
			return; // We got an old message, child window has already been closed
	}
	CMDIChildWnd* pChild = MDIGetActive();
	if (pChild)
	{		
		// View
		CView* pActiveView = pChild->GetActiveView();
		if (!(pActiveView && pActiveView->IsKindOf(RUNTIME_CLASS(CUImagerView))))
			return;
		pView = (CUImagerView*)pActiveView;

		// Doc
		pDoc = pView->GetDocument();
		ASSERT_VALID(pDoc);

		if (pDoc->IsKindOf(RUNTIME_CLASS(CVideoAviDoc)))
		{
			// Safe Pause
			if (!hChildWndSafePaused)
			{
				DWORD dwSafePausedMsgTimeout = 0U;
				double dPlay = (((CVideoAviDoc*)pDoc)->m_PlayVideoFileThread.GetPlaySpeedPercent() * ((CVideoAviDoc*)pDoc)->GetPlayFrameRate());
				if (dPlay > 0.0)
					dwSafePausedMsgTimeout = (DWORD)Round(FULLSCREEN_SAFEPAUSED_FRAMES_TIMEOUT * 100000.0 / dPlay); 
				dwSafePausedMsgTimeout = MAX(FULLSCREEN_MIN_SAFEPAUSED_TIMEOUT, dwSafePausedMsgTimeout);
				((CVideoAviDoc*)pDoc)->m_PlayVideoFileThread.SafePauseDelayedRestart(	GetSafeHwnd(),
																						WM_VIDEOAVI_FULLSCREEN_MODE_OFF,
																						(WPARAM)pChild->GetSafeHwnd(),
																						(LPARAM)0,
																						dwSafePausedMsgTimeout,
																						FULLSCREEN_DELAYEDRESTART_TIMEOUT,
																						FALSE);
				return;
			}

			// No drawing
			((CVideoAviDoc*)pDoc)->m_bNoDrawing = TRUE;
		}

		// Exiting?
		if (((CUImagerApp*)::AfxGetApp())->m_bShuttingDownApplication)
		{
			m_bFullScreenMode = false;
			pView->m_bFullScreenMode = false;
			return;
		}
		
		// Restore starting monitor
		if (m_rcEnterFullScreenMonitor != GetMonitorFullRect())
			FullScreenTo(m_rcEnterFullScreenMonitor);

		// Full Screen Mode Flags
		m_bFullScreenMode = false;
		pView->m_bFullScreenMode = false;

		// Stop Timer for Cursor Hiding
		pView->KillTimer(ID_TIMER_FULLSCREEN);
		if (!pView->IsCursorEnabled())
		{
			// Show Cursor
			pView->EnableCursor();
		}

		// Restore the Zoom Factor
		pDoc->m_dZoomFactor = m_dChildZoomFactor;

		// Restore Child Style
		if (m_bChildMax)
			m_lOldChildStyle &= ~WS_MAXIMIZE;
		else if (m_bChildMin)
			m_lOldChildStyle &= ~WS_MINIMIZE;
		::SetWindowLong(pChild->GetSafeHwnd(), GWL_STYLE, m_lOldChildStyle);
		::SetWindowLong(pChild->GetSafeHwnd(), GWL_EXSTYLE, m_lOldChildExStyle);

		// Restore the Child Toolbar
		CToolBar* pChildToolBar = ((CToolBarChildFrame*)pChild)->GetToolBar();
		if (pChildToolBar && m_bChildToolBarWasVisible)
			pChildToolBar->ShowWindow(SW_SHOW);

		// Restore Child Placement, if minimized restore it,
		// otherwise the minimized position is not correct!
		if (m_ChildWndPlacement.showCmd == SW_SHOWMINIMIZED)
		{
			WINDOWPLACEMENT ChildWndPlacement;
			memcpy(&ChildWndPlacement, &m_ChildWndPlacement, sizeof(WINDOWPLACEMENT));
			ChildWndPlacement.showCmd = SW_RESTORE;
			::SetWindowPlacement(pChild->GetSafeHwnd(), &ChildWndPlacement);
		}
		else
			::SetWindowPlacement(pChild->GetSafeHwnd(), &m_ChildWndPlacement);
	}
	else if (((CUImagerApp*)::AfxGetApp())->m_bShuttingDownApplication)
	{
		m_bFullScreenMode = false;
		return;
	}

	// Restore Main Window Style
	::SetWindowLong(m_hWnd, GWL_STYLE, m_lOldStyle);

	// Restore Main Window Position
	::SetWindowPlacement(GetSafeHwnd(), &m_MainFrameWndPlacement);
	if (!((CUImagerApp*)::AfxGetApp())->m_bTopMost)
		SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	// Restore the Toolbar and Statusbar States
	if (m_bToolBarWasVisible)
		m_wndToolBar.ShowWindow(SW_SHOW);
	if (m_bStatusBarWasVisible)
		m_wndStatusBar.ShowWindow(SW_SHOW);

	// Show Menu
	if (m_hMenu)
		::SetMenu(GetSafeHwnd(), m_hMenu);

	// Recalc Layout
	RecalcLayout();

	if (pView && pDoc)
	{
		// Restore View Scroll Position
		if (pView->IsXOrYScroll())
			pView->ScrollToPosition(m_ptChildScrollPosition);

		// Restore the Top-Most state for the Layered Dialog if it is enabled
		if (pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)) && ((CPictureDoc*)pDoc)->m_pLayeredDlg)
		{
			((CPictureDoc*)pDoc)->m_pLayeredDlg->SetWindowPos(	((CPictureDoc*)pDoc)->m_pLayeredDlg->IsWindowEnabled() ?
																&wndTopMost : &wndNoTopMost,
																0, 0, 0, 0,
																SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}

		// Special Additional Commands for CVideoAviDoc
		if (pDoc->IsKindOf(RUNTIME_CLASS(CVideoAviDoc)))
		{
			// Close Player ToolBar if Open
			if (((CVideoAviDoc*)pDoc)->m_pPlayerToolBarDlg)
				((CVideoAviDoc*)pDoc)->m_pPlayerToolBarDlg->Close();

			// Get Video Stream
			CAVIPlay::CAVIVideoStream* pVideoStream = NULL;
			if (((CVideoAviDoc*)pDoc)->m_pAVIPlay)
				pVideoStream = ((CVideoAviDoc*)pDoc)->m_pAVIPlay->GetVideoStream(((CVideoAviDoc*)pDoc)->m_nActiveVideoStream);
			if (pVideoStream)
			{
				// Enter CS
				if (((CVideoAviDoc*)pDoc)->m_DxDraw.HasDxDraw())
					((CVideoAviDoc*)pDoc)->m_DxDraw.EnterCS();

				// Exit DirectDraw FullScreen Mode
				if (((CVideoAviDoc*)pDoc)->m_DxDraw.IsInit() &&
					((CVideoAviDoc*)pDoc)->m_bUseDxDraw)
				{
					// Init DxDraw
					((CVideoAviDoc*)pDoc)->m_DxDraw.Init(pView->GetSafeHwnd(),
														((CVideoAviDoc*)pDoc)->m_DocRect.right,
														((CVideoAviDoc*)pDoc)->m_DocRect.bottom,
														pVideoStream->GetFourCC(false),
														IDB_BITSTREAM_VERA_11);

					// Leave CS
					((CVideoAviDoc*)pDoc)->m_DxDraw.LeaveCS();

					// Do Draw
					((CVideoAviDoc*)pDoc)->m_bNoDrawing = FALSE;

					// Restore All Frames of All Docs
					RestoreAllFrames();
				}
				else
				{
					// Leave CS
					if (((CVideoAviDoc*)pDoc)->m_DxDraw.HasDxDraw())
						((CVideoAviDoc*)pDoc)->m_DxDraw.LeaveCS();

					// Do Draw
					((CVideoAviDoc*)pDoc)->m_bNoDrawing = FALSE;
				}
			}
			else
				((CVideoAviDoc*)pDoc)->m_bNoDrawing = FALSE;

			// Store m_UserZoomRect
			((CVideoAviDoc*)pDoc)->m_PrevUserZoomRect = pView->m_UserZoomRect;

			// Restart
			((CVideoAviDoc*)pDoc)->m_PlayVideoFileThread.SetSafePauseRestartEvent();
		}

		// Update View
		pView->UpdateWindowSizes(TRUE, TRUE, FALSE);
	}
}

LONG CMainFrame::OnVideoAviFullScreenModeOff(WPARAM wparam, LPARAM lparam)
{
	FullScreenModeOff((HWND)wparam);
	return 1;
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	CDWordArray* p = (CDWordArray*)dwData;
	if (p)
	{
		p->Add((DWORD)hMonitor);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CMainFrame::EnumerateMonitors(CDWordArray* pMonitors)
{
	if (pMonitors)
	{
		pMonitors->RemoveAll();
		return ::EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)pMonitors);
	}
	else
		return FALSE;
}

HMONITOR CMainFrame::GetPreviousMonitor()
{
	CDWordArray Monitors;
	if (EnumerateMonitors(&Monitors))
	{
		HMONITOR hCurrentMonitor = GetCurrentMonitor();
		for (int i = 0 ; i < Monitors.GetSize() ; i++)
		{
			if (hCurrentMonitor == (HMONITOR)(Monitors[i]))
			{
				if (--i >= 0)
					return (HMONITOR)(Monitors[i]);
				else
					return (HMONITOR)(Monitors[Monitors.GetSize() - 1]);
			}
		}
	}
	return NULL;
}

HMONITOR CMainFrame::GetNextMonitor()
{
	CDWordArray Monitors;
	if (EnumerateMonitors(&Monitors))
	{
		HMONITOR hCurrentMonitor = GetCurrentMonitor();
		for (int i = 0 ; i < Monitors.GetSize() ; i++)
		{
			if (hCurrentMonitor == (HMONITOR)(Monitors[i]))
			{
				if (++i < Monitors.GetSize())
					return (HMONITOR)(Monitors[i]);
				else
					return (HMONITOR)(Monitors[0]);
			}
		}
	}
	return NULL;
}

HMONITOR CMainFrame::GetCurrentMonitor()
{
	return ::MonitorFromWindow(this->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
}

CRect CMainFrame::GetPreviousMonitorFullRect()
{
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HMONITOR hMonitor = GetPreviousMonitor();
	if (hMonitor && ::GetMonitorInfo(hMonitor, &monInfo))
		return CRect(monInfo.rcMonitor);
	else
		return CRect(0, 0, 0, 0);
}

CRect CMainFrame::GetNextMonitorFullRect()
{
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HMONITOR hMonitor = GetNextMonitor();
	if (hMonitor && ::GetMonitorInfo(hMonitor, &monInfo))
		return CRect(monInfo.rcMonitor);
	else
		return CRect(0, 0, 0, 0);
}

CSize CMainFrame::GetMonitorSize(CWnd* pWnd/*=NULL*/)
{
	int nMonitorWidth, nMonitorHeight;
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HMONITOR hMonitor;
	if (pWnd)
		hMonitor = ::MonitorFromWindow(pWnd->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
	else
		hMonitor = ::MonitorFromWindow(this->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
	if (!::GetMonitorInfo(hMonitor, &monInfo))
		return CSize(0, 0);
	nMonitorWidth = monInfo.rcMonitor.right - monInfo.rcMonitor.left;
	nMonitorHeight = monInfo.rcMonitor.bottom - monInfo.rcMonitor.top;
	return CSize(nMonitorWidth, nMonitorHeight);
}

CSize CMainFrame::GetMonitorSize(CPoint pt)
{
	int nMonitorWidth, nMonitorHeight;
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HMONITOR hMonitor = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
	if (!::GetMonitorInfo(hMonitor, &monInfo))
		return CSize(0, 0);
	nMonitorWidth = monInfo.rcMonitor.right - monInfo.rcMonitor.left;
	nMonitorHeight = monInfo.rcMonitor.bottom - monInfo.rcMonitor.top;
	return CSize(nMonitorWidth, nMonitorHeight);
}

void CMainFrame::ClipToWorkRect(CRect& rc, CWnd* pWnd/*=NULL*/)
{
	int w = rc.Width();
	int h = rc.Height();
	CRect rcWork = GetMonitorWorkRect(pWnd);

	// Clip
	rc.left = MAX(rcWork.left, MIN(rcWork.right - w, rc.left));
	rc.top = MAX(rcWork.top, MIN(rcWork.bottom - h, rc.top));
	rc.right = rc.left + w;
	rc.bottom = rc.top + h;
}

void CMainFrame::ClipToWorkRect(CRect& rc, CPoint pt)
{
	int w = rc.Width();
	int h = rc.Height();
	CRect rcWork = GetMonitorWorkRect(pt);

	// Clip
	rc.left = MAX(rcWork.left, MIN(rcWork.right - w, rc.left));
	rc.top = MAX(rcWork.top, MIN(rcWork.bottom - h, rc.top));
	rc.right = rc.left + w;
	rc.bottom = rc.top + h;
}

void CMainFrame::ClipToMDIRect(LPRECT lpRect) const
{
	// Check
	if (!lpRect)
		return;

	// Width and Height
	int w = lpRect->right - lpRect->left;
	int h = lpRect->bottom - lpRect->top;

	// Get MDI Client Rect
	CRect rcMainFrame;
	GetMDIClientRect(&rcMainFrame);

	// Clip
	lpRect->left = MAX(rcMainFrame.left, MIN(rcMainFrame.right - w, lpRect->left));
	lpRect->top = MAX(rcMainFrame.top, MIN(rcMainFrame.bottom - h, lpRect->top));
	lpRect->right = MIN(rcMainFrame.right, lpRect->left + w);
	lpRect->bottom = MIN(rcMainFrame.bottom, lpRect->top + h);
}

CRect CMainFrame::GetPrimaryMonitorWorkRect()
{
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HMONITOR hPrimaryMonitor = ::MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
	if (::GetMonitorInfo(hPrimaryMonitor, &monInfo))
		return CRect(monInfo.rcWork);
	else
		return CRect(0, 0, 0, 0);
}

CRect CMainFrame::GetMonitorWorkRect(CWnd* pWnd/*=NULL*/)
{
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HMONITOR hMonitor;
	if (pWnd)
		hMonitor = ::MonitorFromWindow(pWnd->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
	else
		hMonitor = ::MonitorFromWindow(this->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
	if (::GetMonitorInfo(hMonitor, &monInfo))
		return CRect(monInfo.rcWork);
	else
		return CRect(0, 0, 0, 0);
}

CRect CMainFrame::GetMonitorWorkRect(CPoint pt)
{
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HMONITOR hMonitor = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
	if (::GetMonitorInfo(hMonitor, &monInfo))
		return CRect(monInfo.rcWork);
	else
		return CRect(0, 0, 0, 0);
}

void CMainFrame::ClipToFullRect(CRect& rc, CWnd* pWnd/*=NULL*/)
{
	int w = rc.Width();
	int h = rc.Height();
	CRect rcFull = GetMonitorFullRect(pWnd);

	// Clip
	rc.left = MAX(rcFull.left, MIN(rcFull.right - w, rc.left));
	rc.top = MAX(rcFull.top, MIN(rcFull.bottom - h, rc.top));
	rc.right = rc.left + w;
	rc.bottom = rc.top + h;
}

void CMainFrame::ClipToFullRect(CRect& rc, CPoint pt)
{
	int w = rc.Width();
	int h = rc.Height();
	CRect rcFull = GetMonitorFullRect(pt);

	// Clip
	rc.left = MAX(rcFull.left, MIN(rcFull.right - w, rc.left));
	rc.top = MAX(rcFull.top, MIN(rcFull.bottom - h, rc.top));
	rc.right = rc.left + w;
	rc.bottom = rc.top + h;
}

CRect CMainFrame::GetPrimaryMonitorFullRect()
{
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HMONITOR hPrimaryMonitor = ::MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
	if (::GetMonitorInfo(hPrimaryMonitor, &monInfo))
		return CRect(monInfo.rcMonitor);
	else
		return CRect(0, 0, 0, 0);
}

CRect CMainFrame::GetMonitorFullRect(CWnd* pWnd/*=NULL*/)
{
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HMONITOR hMonitor;
	if (pWnd)
		hMonitor = ::MonitorFromWindow(pWnd->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
	else
		hMonitor = ::MonitorFromWindow(this->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
	if (::GetMonitorInfo(hMonitor, &monInfo))
		return CRect(monInfo.rcMonitor);
	else
		return CRect(0, 0, 0, 0);
}

CRect CMainFrame::GetMonitorFullRect(CPoint pt)
{
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HMONITOR hMonitor = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
	if (::GetMonitorInfo(hMonitor, &monInfo))
		return CRect(monInfo.rcMonitor);
	else
		return CRect(0, 0, 0, 0);
}

// Remember to delete it with ::DeleteDC() !!!
HDC CMainFrame::CreateMonitorDC(CWnd* pWnd/*=NULL*/)
{
	MONITORINFOEX monInfo;
	monInfo.cbSize = sizeof(MONITORINFOEX);
	HMONITOR hMonitor;
	if (pWnd)
		hMonitor = ::MonitorFromWindow(pWnd->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
	else
		hMonitor = ::MonitorFromWindow(this->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
	if (::GetMonitorInfo(hMonitor, &monInfo))
		return ::CreateDC(_T("DISPLAY"), monInfo.szDevice, NULL, NULL);
	else
		return NULL;
}

int CMainFrame::GetMonitorBpp(CWnd* pWnd/*=NULL*/)
{
	int nBpp;
	if (::GetSystemMetrics(SM_CMONITORS) <= 1)
	{
		HDC hDC = ::GetDC(NULL);
		nBpp = ::GetDeviceCaps(hDC, BITSPIXEL);
		::ReleaseDC(NULL, hDC);
	}
	else
	{
		if (::GetSystemMetrics(SM_SAMEDISPLAYFORMAT))
		{
			HDC hDC = ::GetDC(NULL);
			nBpp = ::GetDeviceCaps(hDC, BITSPIXEL);
			::ReleaseDC(NULL, hDC);
		}
		else
		{
			HDC hDC = CreateMonitorDC(pWnd);
			nBpp = ::GetDeviceCaps(hDC, BITSPIXEL);
			::DeleteDC(hDC);
		}
	}
	return nBpp;
}

BOOL CMainFrame::AreModelessDlgsVisible()
{
	HWND hWnd = ::GetWindow(::GetDesktopWindow(), GW_CHILD);
	while (hWnd != NULL)
	{
		CWnd* pWnd = CWnd::FromHandlePermanent(hWnd);
		if (pWnd != NULL && m_hWnd != hWnd && AfxIsDescendant(m_hWnd, hWnd))
		{
			DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
			if ((dwStyle & WS_VISIBLE) == WS_VISIBLE)
				return TRUE;
		}
		hWnd = ::GetWindow(hWnd, GW_HWNDNEXT);
	}
	return FALSE;
}

void CMainFrame::InitMenuPositions(CDocument* pDoc/*=NULL*/)
{
	CMenu* pMenu = GetMenu();
	if (pMenu)
	{
		// Main menu entries count
		int nCount = (int)pMenu->GetMenuItemCount();

		// Init menu positions and remove Capture and Tools menus
		if (pDoc == NULL) // MainFrame menu
		{
			m_nFileMenuPos = 0;
			m_nEditMenuPos = 1;
			m_nCaptureMenuPos = 2;
			m_nToolsMenuPos = 3;
			m_nHelpMenuPos = 4;
#ifndef VIDEODEVICEDOC
			if (nCount == 5)
				pMenu->DeleteMenu(m_nCaptureMenuPos, MF_BYPOSITION);
			m_nCaptureMenuPos = -2;
			m_nToolsMenuPos--;
			m_nHelpMenuPos--;
			if (nCount == 5)
				pMenu->DeleteMenu(m_nToolsMenuPos, MF_BYPOSITION);
			m_nToolsMenuPos = -2;
			m_nHelpMenuPos--;
#endif
		}
		else if (pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)))
		{
			((CPictureDoc*)pDoc)->m_nFileMenuPos = 0;
			((CPictureDoc*)pDoc)->m_nEditMenuPos = 1;
			((CPictureDoc*)pDoc)->m_nViewMenuPos = 2;
			((CPictureDoc*)pDoc)->m_nCaptureMenuPos = 3;
			((CPictureDoc*)pDoc)->m_nPlayMenuPos = 4;
			((CPictureDoc*)pDoc)->m_nToolsMenuPos = 5;
			((CPictureDoc*)pDoc)->m_nWindowsPos = 6;
			((CPictureDoc*)pDoc)->m_nHelpMenuPos = 7;
#ifndef VIDEODEVICEDOC
			if (nCount == 8) // On some OSs menus are re-used from one doc opening to the next!
				pMenu->DeleteMenu(((CPictureDoc*)pDoc)->m_nCaptureMenuPos, MF_BYPOSITION);
			((CPictureDoc*)pDoc)->m_nCaptureMenuPos = -2;
			((CPictureDoc*)pDoc)->m_nPlayMenuPos--;
			((CPictureDoc*)pDoc)->m_nToolsMenuPos--;
			((CPictureDoc*)pDoc)->m_nWindowsPos--;
			((CPictureDoc*)pDoc)->m_nHelpMenuPos--;
			if (nCount == 8) // On some OSs menus are re-used from one doc opening to the next!
				pMenu->DeleteMenu(((CPictureDoc*)pDoc)->m_nToolsMenuPos, MF_BYPOSITION);
			((CPictureDoc*)pDoc)->m_nToolsMenuPos = -2;
			((CPictureDoc*)pDoc)->m_nWindowsPos--;
			((CPictureDoc*)pDoc)->m_nHelpMenuPos--;
#endif
		}
		else if (pDoc->IsKindOf(RUNTIME_CLASS(CVideoAviDoc)))
		{
			((CVideoAviDoc*)pDoc)->m_nFileMenuPos = 0;
			((CVideoAviDoc*)pDoc)->m_nEditMenuPos = 1;
			((CVideoAviDoc*)pDoc)->m_nViewMenuPos = 2;
			((CVideoAviDoc*)pDoc)->m_nCaptureMenuPos = 3;
			((CVideoAviDoc*)pDoc)->m_nPlayMenuPos = 4;
			((CVideoAviDoc*)pDoc)->m_nToolsMenuPos = 5;
			((CVideoAviDoc*)pDoc)->m_nWindowsPos = 6;
			((CVideoAviDoc*)pDoc)->m_nHelpMenuPos = 7;
#ifndef VIDEODEVICEDOC
			if (nCount == 8) // On some OSs menus are re-used from one doc opening to the next!
				pMenu->DeleteMenu(((CVideoAviDoc*)pDoc)->m_nCaptureMenuPos, MF_BYPOSITION);
			((CVideoAviDoc*)pDoc)->m_nCaptureMenuPos = -2;
			((CVideoAviDoc*)pDoc)->m_nPlayMenuPos--;
			((CVideoAviDoc*)pDoc)->m_nToolsMenuPos--;
			((CVideoAviDoc*)pDoc)->m_nWindowsPos--;
			((CVideoAviDoc*)pDoc)->m_nHelpMenuPos--;
			if (nCount == 8) // On some OSs menus are re-used from one doc opening to the next!
				pMenu->DeleteMenu(((CVideoAviDoc*)pDoc)->m_nToolsMenuPos, MF_BYPOSITION);
			((CVideoAviDoc*)pDoc)->m_nToolsMenuPos = -2;
			((CVideoAviDoc*)pDoc)->m_nWindowsPos--;
			((CVideoAviDoc*)pDoc)->m_nHelpMenuPos--;
#endif
		}
#ifdef VIDEODEVICEDOC
		else if (pDoc->IsKindOf(RUNTIME_CLASS(CVideoDeviceDoc)))
		{
			((CVideoDeviceDoc*)pDoc)->m_nFileMenuPos = 0;
			((CVideoDeviceDoc*)pDoc)->m_nEditMenuPos = 1;
			((CVideoDeviceDoc*)pDoc)->m_nViewMenuPos = 2;
			((CVideoDeviceDoc*)pDoc)->m_nCaptureMenuPos = 3;
			((CVideoDeviceDoc*)pDoc)->m_nToolsMenuPos = 4;
			((CVideoDeviceDoc*)pDoc)->m_nWindowsPos = 5;
			((CVideoDeviceDoc*)pDoc)->m_nHelpMenuPos = 6;
		}
#endif
		else if (pDoc->IsKindOf(RUNTIME_CLASS(CCDAudioDoc)))
		{
			((CCDAudioDoc*)pDoc)->m_nFileMenuPos = 0;
			((CCDAudioDoc*)pDoc)->m_nEditMenuPos = 1;
			((CCDAudioDoc*)pDoc)->m_nCaptureMenuPos = 2;
			((CCDAudioDoc*)pDoc)->m_nToolsMenuPos = 3;
			((CCDAudioDoc*)pDoc)->m_nWindowsPos = 4;
			((CCDAudioDoc*)pDoc)->m_nHelpMenuPos = 5;
#ifndef VIDEODEVICEDOC
			if (nCount == 6) // On some OSs menus are re-used from one doc opening to the next!
				pMenu->DeleteMenu(((CCDAudioDoc*)pDoc)->m_nCaptureMenuPos, MF_BYPOSITION);
			((CCDAudioDoc*)pDoc)->m_nCaptureMenuPos = -2;
			((CCDAudioDoc*)pDoc)->m_nToolsMenuPos--;
			((CCDAudioDoc*)pDoc)->m_nWindowsPos--;
			((CCDAudioDoc*)pDoc)->m_nHelpMenuPos--;
			if (nCount == 6) // On some OSs menus are re-used from one doc opening to the next!
				pMenu->DeleteMenu(((CCDAudioDoc*)pDoc)->m_nToolsMenuPos, MF_BYPOSITION);
			((CCDAudioDoc*)pDoc)->m_nToolsMenuPos = -2;
			((CCDAudioDoc*)pDoc)->m_nWindowsPos--;
			((CCDAudioDoc*)pDoc)->m_nHelpMenuPos--;
#endif
		}
		else if (pDoc->IsKindOf(RUNTIME_CLASS(CAudioMCIDoc)))
		{
			((CAudioMCIDoc*)pDoc)->m_nFileMenuPos = 0;
			((CAudioMCIDoc*)pDoc)->m_nEditMenuPos = 1;
			((CAudioMCIDoc*)pDoc)->m_nCaptureMenuPos = 2;
			((CAudioMCIDoc*)pDoc)->m_nToolsMenuPos = 3;
			((CAudioMCIDoc*)pDoc)->m_nWindowsPos = 4;
			((CAudioMCIDoc*)pDoc)->m_nHelpMenuPos = 5;
#ifndef VIDEODEVICEDOC
			if (nCount == 6) // On some OSs menus are re-used from one doc opening to the next!
				pMenu->DeleteMenu(((CAudioMCIDoc*)pDoc)->m_nCaptureMenuPos, MF_BYPOSITION);
			((CAudioMCIDoc*)pDoc)->m_nCaptureMenuPos = -2;
			((CAudioMCIDoc*)pDoc)->m_nToolsMenuPos--;
			((CAudioMCIDoc*)pDoc)->m_nWindowsPos--;
			((CAudioMCIDoc*)pDoc)->m_nHelpMenuPos--;
			if (nCount == 6) // On some OSs menus are re-used from one doc opening to the next!
				pMenu->DeleteMenu(((CAudioMCIDoc*)pDoc)->m_nToolsMenuPos, MF_BYPOSITION);
			((CAudioMCIDoc*)pDoc)->m_nToolsMenuPos = -2;
			((CAudioMCIDoc*)pDoc)->m_nWindowsPos--;
			((CAudioMCIDoc*)pDoc)->m_nHelpMenuPos--;
#endif
		}
		else
		{
			ASSERT(FALSE);
		}

		// Redraw menus
		DrawMenuBar();
	}
}

void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	CMDIFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	// Get main menu and direct sub menu
	CMenu* pMainMenu = GetMenu();
	CMenu* pSubMenu = pMainMenu ? pMainMenu->GetSubMenu(nIndex) : NULL;

	// Only handle direct popups of the main menu
	int i;
	int idx = (int)nIndex;
	if (pMainMenu && pSubMenu && pPopupMenu && !bSysMenu &&
		pSubMenu->GetSafeHmenu() == pPopupMenu->GetSafeHmenu() && idx >= 0)
	{
		// A document loaded?
		CMDIChildWnd* pChild = MDIGetActive();
		if (pChild)
		{
			// Get view & doc
			CView* pView = pChild->GetActiveView();
			CDocument* pDoc = NULL;
			if (pView)
				pDoc = pView->GetDocument();
			if (pDoc)
			{
				// Check whether document is maximized
				CString sTest;
				pMainMenu->GetMenuString(0, sTest, MF_BYPOSITION);
				if (sTest == _T("")) // In maximized state the first position is the system icon with no string
					idx--;

				// Is it a Picture Doc?
				if (pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc))) 
				{
					if (idx == ((CPictureDoc*)pDoc)->m_nPlayMenuPos)
					{
						// Remove the menu items if the file is not a animated gif
						if (!((CPictureDoc*)pDoc)->m_GifAnimationThread.IsAlive())
						{
							// Only remove if not already done!
							if (m_sPlayGifMenuItem == _T("") &&
								m_sStopGifMenuItem == _T(""))
							{
								// Separator Pos
								i = (int)pPopupMenu->GetMenuItemCount() - 3;

								// Store Menu Strings
								pPopupMenu->GetMenuString(i+1, m_sPlayGifMenuItem, MF_BYPOSITION);
								pPopupMenu->GetMenuString(i+2, m_sStopGifMenuItem, MF_BYPOSITION);

								// Remove
								pPopupMenu->RemoveMenu(i, MF_BYPOSITION);
								pPopupMenu->RemoveMenu(i, MF_BYPOSITION);
								pPopupMenu->RemoveMenu(i, MF_BYPOSITION);
							}
						}
						// Restore the menu items if the file is a animated gif
						else
						{
							// Only restore if not already done!
							if (m_sPlayGifMenuItem != _T("") &&
								m_sStopGifMenuItem != _T(""))
							{
								// Append
								pPopupMenu->AppendMenu(MF_SEPARATOR);
								if (((CPictureDoc*)pDoc)->m_GifAnimationThread.IsRunning())
								{
									pPopupMenu->AppendMenu(MF_STRING | MF_CHECKED, ID_PLAY_ANIMATION, m_sPlayGifMenuItem);
									pPopupMenu->AppendMenu(MF_STRING | MF_UNCHECKED, ID_STOP_ANIMATION, m_sStopGifMenuItem);
								}
								else
								{
									pPopupMenu->AppendMenu(MF_STRING | MF_UNCHECKED, ID_PLAY_ANIMATION, m_sPlayGifMenuItem);
									pPopupMenu->AppendMenu(MF_STRING | MF_CHECKED, ID_STOP_ANIMATION, m_sStopGifMenuItem);
								}
								m_sPlayGifMenuItem = _T("");
								m_sStopGifMenuItem = _T("");
							}
						}
					}
					else if (idx == ((CPictureDoc*)pDoc)->m_nViewMenuPos)
					{
						// Remove the menu items if the file is not a animated gif
						// and not a multipage tiff
						if (!((CPictureDoc*)pDoc)->m_GifAnimationThread.IsAlive() &&
							!((CPictureDoc*)pDoc)->IsMultiPageTIFF())
						{
							// Only remove if not already done!
							if (m_sNextPageFrameMenuItem == _T("") &&
								m_sPrevPageFrameMenuItem == _T(""))
							{
								// Separator Pos
								i = (int)pPopupMenu->GetMenuItemCount() - 3;

								// Store Menu Strings
								pPopupMenu->GetMenuString(i+1, m_sNextPageFrameMenuItem, MF_BYPOSITION);
								pPopupMenu->GetMenuString(i+2, m_sPrevPageFrameMenuItem, MF_BYPOSITION);

								// Remove
								pPopupMenu->RemoveMenu(i, MF_BYPOSITION);
								pPopupMenu->RemoveMenu(i, MF_BYPOSITION);
								pPopupMenu->RemoveMenu(i, MF_BYPOSITION);
							}
						}
						// Restore the menu items if the file is a animated gif
						// or a multipage tiff
						else
						{
							// Only restore if not already done!
							if (m_sNextPageFrameMenuItem != _T("") &&
								m_sPrevPageFrameMenuItem != _T(""))
							{
								// Append
								pPopupMenu->AppendMenu(MF_SEPARATOR);
								pPopupMenu->AppendMenu(MF_STRING, ID_VIEW_NEXT_PAGE_FRAME, m_sNextPageFrameMenuItem);
								m_sNextPageFrameMenuItem = _T("");
								pPopupMenu->AppendMenu(MF_STRING, ID_VIEW_PREVIOUS_PAGE_FRAME, m_sPrevPageFrameMenuItem);
								m_sPrevPageFrameMenuItem = _T("");
							}
						}
					}
					else if (idx == ((CPictureDoc*)pDoc)->m_nEditMenuPos)
					{
						// Restore the JPEG Advanced menu item if the file is a jpeg
						if (::IsJPEG(((CPictureDoc*)pDoc)->m_sFileName))
						{
							// Only restore if not already done!	
							if (m_hJPEGAdvancedMenu)
							{
								// Append
								pPopupMenu->AppendMenu(MF_POPUP, (UINT)m_hJPEGAdvancedMenu, m_sJPEGAdvancedMenuItem);
								m_hJPEGAdvancedMenu = NULL;
								m_sJPEGAdvancedMenuItem = _T("");
							}
						}
						// Remove the JPEG Advanced menu item if the file is not a jpeg
						else
						{
							// Only remove if not already done!
							if (m_hJPEGAdvancedMenu == NULL)
							{
								// JPEG Advanced Menu Pos
								i = (int)pPopupMenu->GetMenuItemCount() - 1;

								// Store Menu Strings
								pPopupMenu->GetMenuString(i, m_sJPEGAdvancedMenuItem, MF_BYPOSITION);

								// Store Popup Menu Handle
								m_hJPEGAdvancedMenu = pPopupMenu->GetSubMenu(i)->GetSafeHmenu();

								// Remove (this will not delete the SubMenu!)
								pPopupMenu->RemoveMenu(i, MF_BYPOSITION);
							}
						}
					}
				}

				// Populate Help or Capture menus
				if (pDoc->IsKindOf(RUNTIME_CLASS(CUImagerDoc)))
				{
					if (idx == ((CUImagerDoc*)pDoc)->m_nHelpMenuPos)
						PopulateHelpMenu(pPopupMenu);
#ifdef VIDEODEVICEDOC
					else if (idx == ((CUImagerDoc*)pDoc)->m_nCaptureMenuPos)
						PopulateCaptureMenu(pPopupMenu);
#endif
				}
				else if (pDoc->IsKindOf(RUNTIME_CLASS(CCDAudioDoc)))
				{
					if (idx == ((CCDAudioDoc*)pDoc)->m_nHelpMenuPos)
						PopulateHelpMenu(pPopupMenu);
#ifdef VIDEODEVICEDOC
					else if (idx == ((CCDAudioDoc*)pDoc)->m_nCaptureMenuPos)
						PopulateCaptureMenu(pPopupMenu);
#endif
				}
				else if (pDoc->IsKindOf(RUNTIME_CLASS(CAudioMCIDoc)))
				{
					if (idx == ((CAudioMCIDoc*)pDoc)->m_nHelpMenuPos)
						PopulateHelpMenu(pPopupMenu);
#ifdef VIDEODEVICEDOC
					else if (idx == ((CAudioMCIDoc*)pDoc)->m_nCaptureMenuPos)
						PopulateCaptureMenu(pPopupMenu);
#endif
				}
				else
				{
					ASSERT(FALSE);
				}
			}
		}
		// No Document loaded
		else
		{
			// Populate Help or Capture menus
			if (idx == m_nHelpMenuPos)
				PopulateHelpMenu(pPopupMenu);
#ifdef VIDEODEVICEDOC
			else if (idx == m_nCaptureMenuPos)
				PopulateCaptureMenu(pPopupMenu);
#endif
		}	
	}
}

#ifdef VIDEODEVICEDOC
void CMainFrame::PopulateCaptureMenu(CMenu* pPopupMenu)
{
	// Enumerate DirectShow Video Devices
	CStringArray sDxDevicesName;
	CStringArray sDxDevicesPath;
	int nDxDevicesCount = CDxCapture::EnumDevices(sDxDevicesName, sDxDevicesPath);

	// We have a limited range of menu items
	if (nDxDevicesCount > (ID_DIRECTSHOW_VIDEODEV_LAST - ID_DIRECTSHOW_VIDEODEV_FIRST + 1))
		nDxDevicesCount = (ID_DIRECTSHOW_VIDEODEV_LAST - ID_DIRECTSHOW_VIDEODEV_FIRST + 1);

	// Update Menu Items
	CString sItem;
	for (int id = 0 ; id <= (ID_DIRECTSHOW_VIDEODEV_LAST - ID_DIRECTSHOW_VIDEODEV_FIRST) ; id++)
	{
		if (pPopupMenu->GetMenuString(ID_DIRECTSHOW_VIDEODEV_FIRST + id, sItem, MF_BYCOMMAND))
		{
			if (id < nDxDevicesCount)
			{
				if (sItem != sDxDevicesName[id])
				{
					pPopupMenu->RemoveMenu(ID_DIRECTSHOW_VIDEODEV_FIRST + id, MF_BYCOMMAND);
					pPopupMenu->InsertMenu(ID_CAPTURE_NETWORK, MF_BYCOMMAND, ID_DIRECTSHOW_VIDEODEV_FIRST + id, sDxDevicesName[id]);
				}
			}
			else
				pPopupMenu->RemoveMenu(ID_DIRECTSHOW_VIDEODEV_FIRST + id, MF_BYCOMMAND);
		}
		else if (id < nDxDevicesCount)
			pPopupMenu->InsertMenu(ID_CAPTURE_NETWORK, MF_BYCOMMAND, ID_DIRECTSHOW_VIDEODEV_FIRST + id, sDxDevicesName[id]);
	}
}
#endif

void CMainFrame::PopulateHelpMenu(CMenu* pPopupMenu)
{
	int i;

	// Original Items Count
	if (m_nHelpMenuItemsCount <= 0)
		m_nHelpMenuItemsCount = (int)pPopupMenu->GetMenuItemCount();

	// Tutorials Count
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szProgramName[MAX_PATH];
	if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
		return;
	_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
	CString sTutorialsPath = CString(szDrive) + CString(szDir);
	sTutorialsPath += _T("Tutorials");
	CSortableFileFind FileFind;
	FileFind.AddAllowedExtension(_T("htm"));
	int nTutorialsCount = 0;
	if (FileFind.Init(sTutorialsPath + _T("\\*")))
	{
		// Get Count
		 nTutorialsCount = FileFind.GetFilesCount();

		// We have a limited range of menu items
		if (nTutorialsCount > (ID_HELP_TUTORIAL_LAST - ID_HELP_TUTORIAL_FIRST + 1))
			nTutorialsCount = (ID_HELP_TUTORIAL_LAST - ID_HELP_TUTORIAL_FIRST + 1);
	}

	// Remove Tutorial Menu Items
	i = (int)pPopupMenu->GetMenuItemCount() - 1;
	while (i >= m_nHelpMenuItemsCount)
		pPopupMenu->DeleteMenu(i--, MF_BYPOSITION);

	// Append Tutorial Menu Items
	if (nTutorialsCount > 0)
	{
		// Append Separator
		pPopupMenu->AppendMenu(MF_SEPARATOR);
	
		// Append Tutorial Menu Items
		for (i = 0 ; i < nTutorialsCount ; i++)
		{
			CString sDisplayedTutorialName = ::GetShortFileNameNoExt(FileFind.GetFileName(i));
			sDisplayedTutorialName.Replace(_T('_'), _T(' ')); // Replace '_' with a space
			pPopupMenu->AppendMenu(	MF_BYCOMMAND,
									ID_HELP_TUTORIAL_FIRST + i,
									sDisplayedTutorialName);
		}
	}
}

void CMainFrame::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	POSITION pos;
	CUImagerMultiDocTemplate* curTemplate = NULL;
	curTemplate = ((CUImagerApp*)::AfxGetApp())->GetPictureDocTemplate();
	pos = curTemplate->GetFirstDocPosition();
	while (pos)
	{
		CPictureDoc* pDoc = (CPictureDoc*)curTemplate->GetNextDoc(pos);
		
		// Closing Menu (Called before the Selected Command is executed, if any)
		if ((nFlags == 0xFFFF) && (hSysMenu == NULL))
		{
			if (pDoc->m_bDoRestartSlideshow)
			{
				pDoc->m_bDoRestartSlideshow = FALSE;
				pDoc->m_SlideShowThread.RunSlideshow();
			}
			pDoc->m_bDoUpdateLayeredDlg = TRUE;
			if (pDoc->m_pLayeredDlg)
				pDoc->GetView()->Draw();
		}
		// Opening Menu
		else
		{
			if (!pDoc->m_bDoRestartSlideshow)
			{
				if (pDoc->m_SlideShowThread.IsSlideshowRunning())
				{
					pDoc->m_bDoRestartSlideshow = TRUE;
					pDoc->m_SlideShowThread.PauseSlideshow();
				}

				// Cancel Transitions
				pDoc->CancelTransition();
				pDoc->CancelLoadFullJpegTransition();
			}
			pDoc->m_bDoUpdateLayeredDlg = FALSE;
		}
	}
	CMDIFrameWnd::OnMenuSelect(nItemID, nFlags, hSysMenu);
}

void CMainFrame::OnViewAllFirstPicture() 
{
	CUImagerMultiDocTemplate* pPictureDocTemplate =
			((CUImagerApp*)::AfxGetApp())->GetPictureDocTemplate();
	POSITION pos = pPictureDocTemplate->GetFirstDocPosition();
	CPictureDoc* pDoc;

	while (pos)
	{
		pDoc = (CPictureDoc*)(pPictureDocTemplate->GetNextDoc(pos));
		if (!((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc))
			continue;
		pDoc->m_SlideShowThread.FirstPicture();
	}
}

void CMainFrame::OnViewAllLastPicture() 
{
	CUImagerMultiDocTemplate* pPictureDocTemplate =
			((CUImagerApp*)::AfxGetApp())->GetPictureDocTemplate();
	POSITION pos = pPictureDocTemplate->GetFirstDocPosition();
	CPictureDoc* pDoc;

	while (pos)
	{
		pDoc = (CPictureDoc*)(pPictureDocTemplate->GetNextDoc(pos));
		if (!((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc))
			continue;
		pDoc->m_SlideShowThread.LastPicture();
	}
}

void CMainFrame::OnViewAllPreviousPicture() 
{
	CUImagerMultiDocTemplate* pPictureDocTemplate =
			((CUImagerApp*)::AfxGetApp())->GetPictureDocTemplate();
	POSITION pos = pPictureDocTemplate->GetFirstDocPosition();
	CPictureDoc* pDoc;

	while (pos)
	{
		pDoc = (CPictureDoc*)(pPictureDocTemplate->GetNextDoc(pos));
		if (!((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc))
			continue;
		pDoc->m_SlideShowThread.PreviousPicture();
	}
}

void CMainFrame::OnViewAllNextPicture() 
{
	CUImagerMultiDocTemplate* pPictureDocTemplate =
			((CUImagerApp*)::AfxGetApp())->GetPictureDocTemplate();
	POSITION pos = pPictureDocTemplate->GetFirstDocPosition();
	CPictureDoc* pDoc;

	while (pos)
	{
		pDoc = (CPictureDoc*)(pPictureDocTemplate->GetNextDoc(pos));
		if (!((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc))
			continue;
		pDoc->m_SlideShowThread.NextPicture();
	}
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	CMDIFrameWnd::OnTimer(nIDEvent);

	if (nIDEvent == ID_TIMER_CLOSING_ALL)
	{
		((CUImagerApp*)::AfxGetApp())->CloseAll();
	}
#ifdef VIDEODEVICEDOC
	else if (nIDEvent == ID_TIMER_ONESEC_POLL)
	{
		// Current Time
		CTime timedate = CTime::GetCurrentTime();
		CTime timeonly(2000, 1, 1, timedate.GetHour(), timedate.GetMinute(), timedate.GetSecond());

		// Iterate Through the Scheduler List
		POSITION pos = ((CUImagerApp*)::AfxGetApp())->m_Scheduler.GetHeadPosition();
		while (pos)
		{
			CUImagerApp::CSchedulerEntry* pSchedulerEntry =
				((CUImagerApp*)::AfxGetApp())->m_Scheduler.GetNext(pos);
			if (pSchedulerEntry)
			{
				if (pSchedulerEntry->m_Type == CUImagerApp::CSchedulerEntry::ONCE)
				{
					if (timedate >= pSchedulerEntry->m_StopTime)
					{
						if (pSchedulerEntry->m_bRunning)
						{
							// Done?
							if (pSchedulerEntry->Stop())
							{
								// Clean-Up
								if (pSchedulerEntry->m_pDoc && pSchedulerEntry->m_pDoc->m_pGeneralPage)
									pSchedulerEntry->m_pDoc->m_pGeneralPage->ClearOnceScheduler();
								((CUImagerApp*)::AfxGetApp())->DeleteOnceSchedulerEntry(pSchedulerEntry->m_sDevicePathName);
							}
						}
					}
					else if (timedate >= pSchedulerEntry->m_StartTime)
					{
						if (!pSchedulerEntry->m_bRunning)
							pSchedulerEntry->Start();
					}
				}
				else if (pSchedulerEntry->m_Type == CUImagerApp::CSchedulerEntry::DAILY)
				{
					if (pSchedulerEntry->m_StartTime <= pSchedulerEntry->m_StopTime)
					{
						if (timeonly < pSchedulerEntry->m_StartTime || timeonly > pSchedulerEntry->m_StopTime)
						{
							if (pSchedulerEntry->m_bRunning)
								pSchedulerEntry->Stop();
						}
						else
						{
							if (!pSchedulerEntry->m_bRunning)
								pSchedulerEntry->Start();
						}
					}
					else
					{
						if (timeonly < pSchedulerEntry->m_StartTime && timeonly > pSchedulerEntry->m_StopTime)
						{
							if (pSchedulerEntry->m_bRunning)
								pSchedulerEntry->Stop();
						}
						else
						{
							if (!pSchedulerEntry->m_bRunning)
								pSchedulerEntry->Start();
						}
					}
				}
			}
		}
	}
#endif
}

void CMainFrame::StatusText(CString sText/*=_T("")*/)
{
	CString sCurrentText(sText);
	CWnd* pMessageBar = GetMessageBar();
	if (pMessageBar)
		pMessageBar->GetWindowText(sCurrentText);

	if (m_sStatusBarString == sText && sCurrentText == sText)
		return;

	m_sStatusBarString = sText;

	if (sText == _T(""))
		SetMessageText(AFX_IDS_IDLEMESSAGE);
	else
    	SetMessageText((LPCTSTR)m_sStatusBarString);
}

LRESULT CMainFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
	/* Original Version in CFrameWnd
	UINT nIDLast = m_nIDLastMessage;
	m_nFlags &= ~WF_NOPOPMSG;

	CWnd* pMessageBar = GetMessageBar();
	if (pMessageBar != NULL)
	{
		LPCTSTR lpsz = NULL;
		CString strMessage;

		// set the message bar text
		if (lParam != 0)
		{
			ASSERT(wParam == 0);    // can't have both an ID and a string
			lpsz = (LPCTSTR)lParam; // set an explicit string
		}
		else if (wParam != 0)
		{
			// map SC_CLOSE to PREVIEW_CLOSE when in print preview mode
			if (wParam == AFX_IDS_SCCLOSE && m_lpfnCloseProc != NULL)
				wParam = AFX_IDS_PREVIEW_CLOSE;

			// get message associated with the ID indicated by wParam
			GetMessageString(wParam, strMessage);
			lpsz = strMessage;
		}
		pMessageBar->SetWindowText(lpsz);

		// update owner of the bar in terms of last message selected
		CFrameWnd* pFrameWnd = pMessageBar->GetParentFrame();
		if (pFrameWnd != NULL)
		{
			pFrameWnd->m_nIDLastMessage = (UINT)wParam;
			pFrameWnd->m_nIDTracking = (UINT)wParam;
		}
	}

	m_nIDLastMessage = (UINT)wParam;    // new ID (or 0)
	m_nIDTracking = (UINT)wParam;       // so F1 on toolbar buttons work
	return nIDLast;
	*/

	UINT nIDLast = m_nIDLastMessage;
	m_nFlags &= ~WF_NOPOPMSG;

	CWnd* pMessageBar = GetMessageBar();
	if (pMessageBar != NULL)
	{
		CString sMsg;
		CString strMessage;

		// set the message bar text
		if (lParam != 0)
		{
			ASSERT(wParam == 0);    // can't have both an ID and a string
            m_sStatusBarString = (LPCTSTR)lParam;
            sMsg = m_sStatusBarString;
		}
		else if (wParam != 0)
		{
			// map SC_CLOSE to PREVIEW_CLOSE when in print preview mode
			if (wParam == AFX_IDS_SCCLOSE && m_lpfnCloseProc != NULL)
				wParam = AFX_IDS_PREVIEW_CLOSE;

			// do not show idle message if a status bar string has been set
            if (wParam == AFX_IDS_IDLEMESSAGE && m_sStatusBarString != _T(""))
				sMsg = m_sStatusBarString;
			// special handling for DirectShow devices
			else if (wParam >= ID_DIRECTSHOW_VIDEODEV_FIRST && wParam <= ID_DIRECTSHOW_VIDEODEV_LAST)
				sMsg = _T("DirectShow / WDM");
			// get message associated with the ID indicated by wParam
			else
			{
				GetMessageString(wParam, strMessage);
				sMsg = strMessage;
			}
		}
		pMessageBar->SetWindowText(sMsg);

		// update owner of the bar in terms of last message selected
		CFrameWnd* pFrameWnd = pMessageBar->GetParentFrame();
		ASSERT(pFrameWnd == this);
		if (pFrameWnd != NULL)
		{
			m_nIDLastMessage = (UINT)wParam;
			m_nIDTracking = (UINT)wParam;
		}
	}

	m_nIDLastMessage = (UINT)wParam;    	// new ID (or 0)
	m_nIDTracking = (UINT)wParam;       	// so F1 on toolbar buttons work
	return nIDLast;
}

void CMainFrame::OnUpdateIndicatorXCoordinate(CCmdUI* pCmdUI)
{
	if (!((CUImagerApp*)::AfxGetApp())->AreDocsOpen())
		pCmdUI->SetText(_T("X:         "));
}

void CMainFrame::OnUpdateIndicatorYCoordinate(CCmdUI* pCmdUI)
{
	if (!((CUImagerApp*)::AfxGetApp())->AreDocsOpen())
		pCmdUI->SetText(_T("Y:         "));
}

LONG CMainFrame::OnProgress(WPARAM wparam, LPARAM lparam)
{
	lparam;
	Progress((int)wparam);
	return 1;
}

void CMainFrame::Progress(int nPercent)
{
	if (nPercent < 0)
		nPercent = 0;
	else if (nPercent > 100)
		nPercent = 100;

	RECT Rect;

	m_wndStatusBar.GetItemRect(1, &Rect);

	if (!m_bProgressIndicatorCreated)
	{
		m_Progress.Create(WS_VISIBLE | WS_CHILD, Rect, &m_wndStatusBar, 1);
		m_Progress.SetRange(0, 100);
		m_Progress.SetStep(1);
		m_bProgressIndicatorCreated = TRUE;
	}

	if (nPercent == 100)
	{
		m_wndStatusBar.GetItemRect(1, &Rect);
		m_Progress.SetPos(0);
		m_Progress.SetWindowPos(&wndTop,
								Rect.left,
								Rect.top,
								Rect.right - Rect.left,
								Rect.bottom - Rect.top,
								SWP_HIDEWINDOW);
	}
	else
	{
		m_wndStatusBar.GetItemRect(1, &Rect);
		m_Progress.SetPos(nPercent);
		m_Progress.SetWindowPos(&wndTop,
								Rect.left,
								Rect.top,
								Rect.right - Rect.left,
								Rect.bottom - Rect.top,
								SWP_SHOWWINDOW);
	}
}

void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (((CUImagerApp*)::AfxGetApp())->m_bTrayIcon	&&
		!m_bFullScreenMode							&&
		((nID & 0xFFF0) == SC_MINIMIZE				||
		(nID & 0xFFF0) == SC_CLOSE))
	{
		m_TrayIcon.MinimizeToTray();
		ShowOwnedWindows(FALSE);
	}
	else
		CMDIFrameWnd::OnSysCommand(nID, lParam);
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{	
	CMDIFrameWnd::OnSize(nType, cx, cy);
	RECT Rect;
	m_wndStatusBar.GetItemRect(1, &Rect);

	// Reposition the progress control correctly!
	if (m_bProgressIndicatorCreated)
	{
		m_Progress.SetWindowPos(&wndTop,
								Rect.left,
								Rect.top,
								Rect.right - Rect.left,
								Rect.bottom - Rect.top,
								0);
	}
}

LONG CMainFrame::OnRestoreAllFrames(WPARAM wparam, LPARAM lparam)
{
	RestoreAllFrames();
	return 1;
}

void CMainFrame::OnMove(int x, int y) 
{
	CMDIFrameWnd::OnMove(x, y);
	RestoreAllFrames();
}

void CMainFrame::RestoreAllFrames()
{
	CUImagerMultiDocTemplate* pVideoAviDocTemplate = ((CUImagerApp*)::AfxGetApp())->GetVideoAviDocTemplate();
	POSITION posVideoAviDoc = pVideoAviDocTemplate->GetFirstDocPosition();
	CVideoAviDoc* pVideoAviDoc;	
	while (posVideoAviDoc)
	{
		pVideoAviDoc = (CVideoAviDoc*)(pVideoAviDocTemplate->GetNextDoc(posVideoAviDoc));
		if (pVideoAviDoc && ((CUImagerApp*)::AfxGetApp())->IsDoc(pVideoAviDoc))
			pVideoAviDoc->RestoreFrame();
	}
}

void CMainFrame::OnOpenFromTray() 
{
	if (((CUImagerApp*)::AfxGetApp())->m_bTrayIcon &&
		m_TrayIcon.IsMinimizedToTray())
	{
		m_TrayIcon.MaximizeFromTray();
		ShowOwnedWindows(TRUE);
		((CUImagerApp*)::AfxGetApp())->PaintDocTitles();
	}
}

// The WPARAM is unused (zero) and the LPARAM contains the
// mouse position in screen coordinates, in the usual format.
//
// Note: not working anymore on Windows 7 or higher. Could
// implement toolbar buttons in the task preview through
// the ITaskbarList3 interface (ThumbBarSetImageList and ThumbBarAddButtons)
LONG CMainFrame::OnTaskBarButton(WPARAM wparam, LPARAM lparam)
{
	// Only show right-click taskbar context menu
	// if no modal dialog running, if not resizing with FakeThread
	// and not printing with FakeThread (they all disable the MainFrame)
	if (IsWindowEnabled())
	{
		// Get active view and force cursor
		CUImagerView* pActiveView = NULL;
		if (m_bFullScreenMode)
		{
			CMDIChildWnd* pChild = MDIGetActive();
			if (pChild)
			{
				pActiveView = (CUImagerView*)pChild->GetActiveView();
				if (pActiveView && pActiveView->IsKindOf(RUNTIME_CLASS(CUImagerView)))
					pActiveView->ForceCursor();
				else
					pActiveView = NULL;
			}
		}

		CPoint point(lparam);
		CMenu menu;
		if (((CUImagerApp*)::AfxGetApp())->m_bSlideShowOnly)
			VERIFY(menu.LoadMenu(IDR_CONTEXT_TASKBAR_SLIDESHOW_ONLY));
		else if (m_bFullScreenMode)
			VERIFY(menu.LoadMenu(IDR_CONTEXT_TASKBAR_FULLSCREEN));
		else
			VERIFY(menu.LoadMenu(IDR_CONTEXT_TASKBAR));
		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);
		pPopup->TrackPopupMenu(TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this);

		if (pActiveView)
			pActiveView->ForceCursor(FALSE);

		return 1;
	}
	else
		return 0;
}

void CMainFrame::OnRestore() 
{
	if (m_bFullScreenMode)
		EnterExitFullscreen();
	ShowWindow(SW_RESTORE);
}

void CMainFrame::OnUpdateRestore(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsZoomed() || IsIconic());
}

void CMainFrame::OnMinimize() 
{
	if (m_bFullScreenMode)
		EnterExitFullscreen();
	if (((CUImagerApp*)::AfxGetApp())->m_bTrayIcon)
	{
		m_TrayIcon.MinimizeToTray();
		ShowOwnedWindows(FALSE);
	}
	else
		ShowWindow(SW_MINIMIZE);
}

void CMainFrame::OnUpdateMinimize(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!IsIconic());
}

void CMainFrame::OnMaximize() 
{
	if (m_bFullScreenMode)
		EnterExitFullscreen();
	ShowWindow(SW_MAXIMIZE);
}

void CMainFrame::OnUpdateMaximize(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!IsZoomed());
}

void CMainFrame::OnMainmonitor() 
{
	if (m_bFullScreenMode)
		EnterExitFullscreen();

	CRect rc;
	CRect rcPrimaryMon = GetPrimaryMonitorWorkRect();
	CPoint ptPrimaryMonCenter = rcPrimaryMon.CenterPoint();

	// Place Main Window to Primary Monitor
	WINDOWPLACEMENT wp;
	::ZeroMemory(&wp, sizeof(WINDOWPLACEMENT));
	wp.length = sizeof(WINDOWPLACEMENT);
	rc = rcPrimaryMon;
	rc.DeflateRect(60, 60, 60, 60);
	wp.rcNormalPosition = rc;
	wp.showCmd = SW_RESTORE;
	SetWindowPlacement(&wp);

	// Place Image Info Dialog(s) to Primary Monitor
	CUImagerMultiDocTemplate* pPictureDocTemplate = ((CUImagerApp*)::AfxGetApp())->GetPictureDocTemplate();
	POSITION posPictureDoc = pPictureDocTemplate->GetFirstDocPosition();
	CPictureDoc* pDoc;
	while (posPictureDoc)
	{
		pDoc = (CPictureDoc*)(pPictureDocTemplate->GetNextDoc(posPictureDoc));
		if (pDoc && pDoc->m_pImageInfoDlg)
		{
			pDoc->m_pImageInfoDlg->GetWindowRect(&rc);
			pDoc->m_pImageInfoDlg->SetWindowPos(NULL,
												ptPrimaryMonCenter.x - rc.Width() / 2,
												ptPrimaryMonCenter.y - rc.Height() / 2,
												0, 0,
												SWP_NOSIZE | SWP_NOZORDER);
		}
	}
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
	{
		HINSTANCE h = ::LoadLibrary(_T("user32.dll"));
		if (h)
		{
			CString strAutoPos;
			strAutoPos.Format(_T("ID=0x%08lx"), IDD_IMAGEINFO);
			CString sSection = cdxCDynamicWndEx::MakeFullProfile(cdxCDynamicWndEx::M_lpszAutoPosProfileSection, strAutoPos);
			::AfxGetApp()->WriteProfileInt(sSection, _T("(valid)"), 0);
			::FreeLibrary(h);
		}
	}
}

BOOL CMainFrame::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// If Wait Cursor leave it!
	if (((CUImagerApp*)::AfxGetApp())->IsWaitCursor())
	{
		RestoreWaitCursor();
		return TRUE;
	}
	else
		return CMDIFrameWnd::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CMainFrame::OnQueryEndSession() 
{
	return TRUE;
}

void CMainFrame::OnEndSession(BOOL bEnding) 
{
	if (bEnding)
	{
		((CUImagerApp*)::AfxGetApp())->m_bEndSession = TRUE;
		((CUImagerApp*)::AfxGetApp())->SaveOnEndSession();
	}
}

LONG CMainFrame::OnSessionChange(WPARAM wparam, LPARAM lparam)
{
	// Use a counter for disconnection and lock.
	// This because XP is handling it differently
	// than Vista and newer. XP sends an additional session lock with
	// the disconnection, when then connecting to another user
	// we get a session unlock and finally coming back we receive
	// the session connect.
	if (wparam == WTS_CONSOLE_DISCONNECT	||
		wparam == WTS_REMOTE_DISCONNECT		||
		wparam == WTS_SESSION_LOCK)
	{
		m_SessionChangeTime = CTime::GetCurrentTime();
		::InterlockedIncrement(&m_lSessionDisconnectedLockedCount);
#ifdef _DEBUG
		if (wparam == WTS_CONSOLE_DISCONNECT)
			TRACE(_T("m_lSessionDisconnectedLockedCount = %d (LOCAL SESSION DISCONNECTED , session id = %d)\n"), m_lSessionDisconnectedLockedCount, lparam);
		else if (wparam == WTS_REMOTE_DISCONNECT)
			TRACE(_T("m_lSessionDisconnectedLockedCount = %d (REMOTE SESSION DISCONNECTED , session id = %d)\n"), m_lSessionDisconnectedLockedCount, lparam);
		else
			TRACE(_T("m_lSessionDisconnectedLockedCount = %d (SESSION LOCKED , session id = %d)\n"), m_lSessionDisconnectedLockedCount, lparam);
#endif
	}
	else if (wparam == WTS_CONSOLE_CONNECT	||
			wparam == WTS_REMOTE_CONNECT	||
			wparam == WTS_SESSION_UNLOCK)
	{
		m_SessionChangeTime = CTime::GetCurrentTime();
		::InterlockedDecrement(&m_lSessionDisconnectedLockedCount);
#ifdef _DEBUG
		if (wparam == WTS_CONSOLE_CONNECT)
			TRACE(_T("m_lSessionDisconnectedLockedCount = %d (LOCAL SESSION CONNECTED , session id = %d)\n"), m_lSessionDisconnectedLockedCount, lparam);
		else if (wparam == WTS_REMOTE_CONNECT)
			TRACE(_T("m_lSessionDisconnectedLockedCount = %d (REMOTE SESSION CONNECTED , session id = %d)\n"), m_lSessionDisconnectedLockedCount, lparam);
		else
			TRACE(_T("m_lSessionDisconnectedLockedCount = %d (SESSION UNLOCKED , session id = %d)\n"), m_lSessionDisconnectedLockedCount, lparam);
#endif
	}
	return 1;
}

LRESULT CMainFrame::OnCopyData(WPARAM /*wParam*/, LPARAM lParam)
{
	COPYDATASTRUCT* pCDS = reinterpret_cast<COPYDATASTRUCT*>(lParam);
	TCHAR* pszFiles = static_cast<TCHAR*>(pCDS->lpData);
	int nShellCommand = (int)pCDS->dwData;
	if (pszFiles)
	{
		if (!m_bFullScreenMode)
		{
			OnOpenFromTray();
			CString sFiles(pszFiles);
			int nStartIndex = sFiles.Find(_T("\""));
			if (nStartIndex < 0)
			{
				// If the file is empty or not existing try to paste the clipboard into it
				if (::GetFileSize64(pszFiles).QuadPart == 0)
					((CUImagerApp*)::AfxGetApp())->PasteToFile(pszFiles);
				CDocument* pDoc = ((CUImagerApp*)::AfxGetApp())->OpenDocumentFile(pszFiles);
				if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)) && 
					nShellCommand == CCommandLineInfo::FilePrint)
					SendMessage(WM_COMMAND, ID_FILE_PRINT_PREVIEW);
			}
			else
			{
				while (nStartIndex >= 0)
				{
					int nEndIndex = sFiles.Find(_T("\""), nStartIndex + 1);
					if (nEndIndex < 0)
						break;
					CString sFile = sFiles.Mid(nStartIndex + 1, nEndIndex - nStartIndex - 1);
					CDocument* pDoc = ((CUImagerApp*)::AfxGetApp())->OpenDocumentFile(sFile);
					if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)) &&
						nShellCommand == CCommandLineInfo::FilePrint)
					{
						SendMessage(WM_COMMAND, ID_FILE_PRINT_PREVIEW);
						break; // Only one print preview, like in ProcessShellCommand()
					}
					nStartIndex = sFiles.Find(_T("\""), nEndIndex + 1);
				}
			}
		}
	}
	return TRUE;
}
