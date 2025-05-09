#include "stdafx.h"
#include "uImager.h"
#include "PictureDoc.h"
#include "VideoDeviceDoc.h"
#include "PictureView.h"
#include "VideoDeviceView.h"
#include "MainFrm.h"
#include "LicenseHelper.h"
#include "DxCapture.h"
#include "dbt.h"
#include "PicturePrintPreviewView.h"
#include "BatchProcDlg.h"
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
	ON_COMMAND(ID_RESTORE, OnRestore)
	ON_UPDATE_COMMAND_UI(ID_RESTORE, OnUpdateRestore)
	ON_COMMAND(ID_MINIMIZE, OnMinimize)
	ON_UPDATE_COMMAND_UI(ID_MINIMIZE, OnUpdateMinimize)
	ON_COMMAND(ID_MAXIMIZE, OnMaximize)
	ON_UPDATE_COMMAND_UI(ID_MAXIMIZE, OnUpdateMaximize)
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_FILE_ACQUIRE_TO_TIFF, OnFileAcquireToTiff)
	ON_COMMAND(ID_FILE_ACQUIRE_TO_PDF, OnFileAcquireToPdf)
	ON_COMMAND(ID_FILE_ACQUIRE_AND_EMAIL, OnFileAcquireAndEmail)
	ON_COMMAND(ID_OPEN_FROM_TRAY, OnOpenFromTray)
	ON_WM_SYSCOMMAND()
	ON_WM_QUERYENDSESSION()
	ON_WM_ENDSESSION()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_PROGRESS, OnProgress)
	ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
	ON_MESSAGE(WM_ALL_CLOSED, OnAllClosed)
	ON_MESSAGE(WM_SCANANDEMAIL, OnScanAndEmail)
	ON_MESSAGE(WM_TRAY_NOTIFICATION, OnTrayNotification)
	ON_MESSAGE(WM_COPYDATA, OnCopyData)
	ON_MESSAGE(WM_TWAIN_CLOSED, OnTwainClosed)
	ON_MESSAGE(WM_THREADSAFE_POPUP_NOTIFICATIONWND, OnThreadSafePopupNotificationWnd)
#ifdef VIDEODEVICEDOC
	ON_WM_INITMENUPOPUP()
	ON_MESSAGE(WM_AUTORUN_VIDEODEVICES, OnAutorunVideoDevices)
	ON_COMMAND(ID_INDICATOR_REC_SPEED, OnRecSpeedClick)
#else
	ON_COMMAND(ID_INDICATOR_XCOORDINATE, OnXCoordinatesDoubleClick)
	ON_COMMAND(ID_INDICATOR_YCOORDINATE, OnYCoordinatesDoubleClick)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_XCOORDINATE, OnUpdateIndicatorXCoordinate)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_YCOORDINATE, OnUpdateIndicatorYCoordinate)
#endif
END_MESSAGE_MAP()

#ifdef VIDEODEVICEDOC
static TCHAR sba_RECSPEEDHelp[MAX_PATH];
#else
static TCHAR sba_CoordinateHelp[MAX_PATH];
#endif
static SBACTPANEINFO sba_indicators[] = 
{
	{ ID_SEPARATOR, _T(""), SBACTF_NORMAL },		// status line indicator
#ifdef VIDEODEVICEDOC
	{ ID_INDICATOR_REC_SPEED, sba_RECSPEEDHelp, SBACTF_AUTOFIT | SBACTF_COMMAND | SBACTF_SINGLECLICK | SBACTF_DOUBLECLICK | SBACTF_HANDCURSOR },
#else
	{ ID_INDICATOR_XCOORDINATE, sba_CoordinateHelp, SBACTF_AUTOFIT | SBACTF_COMMAND | SBACTF_SINGLECLICK | SBACTF_DOUBLECLICK | SBACTF_HANDCURSOR },
	{ ID_INDICATOR_YCOORDINATE, sba_CoordinateHelp, SBACTF_AUTOFIT | SBACTF_COMMAND | SBACTF_SINGLECLICK | SBACTF_DOUBLECLICK | SBACTF_HANDCURSOR },
#endif
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame() : m_TrayIcon(IDR_TRAYICON) // Menu ID
{
	m_bFullScreenMode = false;
	m_nFileMenuPos = -2;
	m_nEditMenuPos = -2;
	m_nCaptureMenuPos = -2;
	m_nSettingsMenuPos = -2;
	m_nHelpMenuPos = -2;
	m_hMenu = NULL;
	m_bChildMax = false;
	m_bChildMin = false;
	m_dChildZoomFactor = 1.0;
	m_ptChildScrollPosition = CPoint(0,0);
	m_bScreenSaverWasActive = FALSE;
	m_sStatusBarString = _T("");
	m_TiffScan = NULL;
	m_bScanAndEmail = FALSE;
	m_pBatchProcDlg = NULL;
	m_bLastNotificationWndDone = FALSE;
	m_pNotificationWnd = NULL;
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
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1) // this calls CMainFrame::OnCreateClient() and thus inits m_MDIClientWnd
		return -1;

	// Create Toolbar
#ifndef VIDEODEVICEDOC
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_VISIBLE | WS_CHILD | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED) ||
		!SwitchToolBar(g_nSystemDPI, FALSE))
	{
		TRACE(_T("Failed to create toolbar\n"));
		return -1;      // fail to create
	}
#endif

	// Create Statusbar
#ifdef VIDEODEVICEDOC
	_tcsncpy(sba_RECSPEEDHelp, ML_STRING(1762, "REC speed\n(click for more information)"), MAX_PATH);
	sba_RECSPEEDHelp[MAX_PATH - 1] = _T('\0');
#else
	_tcsncpy(sba_CoordinateHelp, ML_STRING(1768, "Click to change unit"), MAX_PATH);
	sba_CoordinateHelp[MAX_PATH - 1] = _T('\0');
#endif
	if (!m_wndStatusBar.Create(this, WS_VISIBLE | WS_CHILD | CBRS_BOTTOM, AFX_IDW_STATUS_BAR) || 
		!m_wndStatusBar.SetPanes(sba_indicators, sizeof(sba_indicators)/sizeof(SBACTPANEINFO)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetHandCursor(IDC_HAND_CURSOR);

	// Size Statusbar
	UINT nID;
	UINT nStyle;
	int cxWidth;
	m_wndStatusBar.GetPaneInfo(0, nID, nStyle, cxWidth);
	m_wndStatusBar.SetPaneInfo(0, nID, nStyle, ::SystemDPIScale(50));		// set minimum width (default is too big = 1/4 screen width)
	m_wndStatusBar.GetStatusBarCtrl().SetMinHeight(::SystemDPIScale(20));	// set minimum height (default is too small on high dpi devices)

	// MDI snapping width
	m_MDISnap.SetSnapWidth(::SystemDPIScale(8));

	// Create tab control
	m_wndMDITabs.Create(this, MT_TOP | MT_HIDEWLT2VIEWS); // add MT_IMAGES to display the document icons in the tabs

	// Enable Drag'n'Drop
	DragAcceptFiles(TRUE);

	// Init timer
	SetTimer(ID_TIMER_1SEC, 1000U, NULL);

	// Init Menu Positions
	InitMenuPositions();
	
	// Set top most
	((CUImagerApp*)::AfxGetApp())->m_bTopMost = (BOOL)((CUImagerApp*)::AfxGetApp())->GetProfileInt(_T("GeneralApp"), _T("TopMost"), FALSE);
	if (((CUImagerApp*)::AfxGetApp())->m_bTopMost)
		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	// Allocate Dlg Objects
	((CUImagerApp*)::AfxGetApp())->m_pXmpImportDlg = new CXmpDlg(this, IDD_XMP_IMPORT);
	((CUImagerApp*)::AfxGetApp())->m_pXmpDlg = new CXmpDlg(this, IDD_XMP);

	// Setup Tray Icon
	if (((CUImagerApp*)::AfxGetApp())->m_bTrayIcon)
	{
		TrayIcon(TRUE);
		if (!((CUImagerApp*)::AfxGetApp())->m_bFirstRun)
			m_TrayIcon.MinimizeToTray();
	}

	return 0;
}

#ifndef VIDEODEVICEDOC
void CMainFrame::OnXCoordinatesDoubleClick()
{
	ChangeCoordinatesUnit();
}

void CMainFrame::OnYCoordinatesDoubleClick()
{	
	ChangeCoordinatesUnit();
}

void CMainFrame::OnUpdateIndicatorXCoordinate(CCmdUI* pCmdUI)
{
	if (!((CUImagerApp*)::AfxGetApp())->AreDocsOpen())
	{
		CString sPaneText;
		sPaneText.LoadString(ID_INDICATOR_XCOORDINATE);
		pCmdUI->SetText(sPaneText);
	}
}

void CMainFrame::OnUpdateIndicatorYCoordinate(CCmdUI* pCmdUI)
{
	if (!((CUImagerApp*)::AfxGetApp())->AreDocsOpen())
	{
		CString sPaneText;
		sPaneText.LoadString(ID_INDICATOR_YCOORDINATE);
		pCmdUI->SetText(sPaneText);
	}
}

void CMainFrame::ChangeCoordinatesUnit()
{
	// Next Unit
	(((CUImagerApp*)::AfxGetApp())->m_nCoordinateUnit)++;
	if (((CUImagerApp*)::AfxGetApp())->m_nCoordinateUnit > COORDINATES_CM)
		((CUImagerApp*)::AfxGetApp())->m_nCoordinateUnit = COORDINATES_PIX;
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
#endif

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
	// Kill timer
	KillTimer(ID_TIMER_1SEC);

	// Close Notification Wnd
	CloseNotificationWnd(TRUE); // TRUE: do not allow further notification windows

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
			::GetSafeCursorPos(&mouse);
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
	::DeleteDir(::GetDriveAndDirName(((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName));

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

						// Show message
						PopupNotificationWnd(CString(APPNAME_NOEXT) + _T(" ") + ML_STRING(1849, "Saved"), ((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName);
					}
					else
					{
						EndWaitCursor();
						PopupNotificationWnd(APPNAME_NOEXT, ML_STRING(1850, "Save Failed!"), 0);
					}
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
			pZoomCB->SetCurSel(pDoc->m_nZoomComboBoxIndex);
			pZoomCB->OnChangeZoomFactor(*((double*)(pZoomCB->GetItemDataPtr(pDoc->m_nZoomComboBoxIndex))));

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

		// Let the user close the dialog, some processing may still be running
		if (m_pBatchProcDlg)
		{
			::MessageBeep(0xFFFFFFFF);
			m_pBatchProcDlg->SetActiveWindow();
			m_pBatchProcDlg->SetFocus();
			return;		// don't close it
		}

		// Stop All Threads used for the PostDelayedMessage() Function
		CPostDelayedMessageThread::Exit();

		// Save Placement
		pApp->SavePlacement();

		// Release Twain
		ReleaseTwain();

		// Hide the application's windows before closing all the documents
		pApp->HideApplication();

		// Set Flag
		pApp->m_bShuttingDownApplication = TRUE;

		// Show closing message
#ifdef VIDEODEVICEDOC
		if (((CUImagerApp*)::AfxGetApp())->m_bAutostartsExecuted &&
			!((CUImagerApp*)::AfxGetApp())->m_bServiceProcess)
		{
			PopupNotificationWnd(ML_STRING(1566, "Closing") + _T(" ") + APPNAME_NOEXT,
								ML_STRING(1565, "Please wait..."),
								0);
		}
#endif

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

void CMainFrame::PopupNotificationWnd(const CString& sTitle, const CString& sText, DWORD dwWaitTimeMs/*=10000*/)
{
	TCHAR* p = new TCHAR[sTitle.GetLength() + sText.GetLength() + 2];
	if (p)
	{
		_tcscpy(p, sTitle);
		p += sTitle.GetLength() + 1;
		_tcscpy(p, sText);
		p -= sTitle.GetLength() + 1;
		::PostMessage(	GetSafeHwnd(),
						WM_THREADSAFE_POPUP_NOTIFICATIONWND,
						(WPARAM)p,
						(LPARAM)dwWaitTimeMs);
	}
}

void CMainFrame::CloseNotificationWnd(BOOL bLastNotificationWndDone/*=FALSE*/)
{
	m_bLastNotificationWndDone = bLastNotificationWndDone;
	if (m_pNotificationWnd)
	{
		m_pNotificationWnd->Close(); // we do not need to delete m_pNotificationWnd because CNotificationWnd is self deleting
		m_pNotificationWnd = NULL;
	}
}

LONG CMainFrame::OnThreadSafePopupNotificationWnd(WPARAM wparam, LPARAM lparam)
{
	// Get params
	TCHAR* p = (TCHAR*)wparam;
	CString sTitle;
	CString sText;
	if (p)
	{
		sTitle = CString(p);
		p += sTitle.GetLength() + 1;
		sText = CString(p);
		p -= sTitle.GetLength() + 1;
		delete [] p;
	}
	DWORD dwWaitTimeMs = (DWORD)lparam;

	// Show Notification Wnd?
	if (!m_bLastNotificationWndDone)
	{
		// Close
		CloseNotificationWnd();

		// Create
		m_pNotificationWnd = new CNotificationWnd(sTitle, sText, 360, 95, dwWaitTimeMs);

		// Show
		if (m_pNotificationWnd && !m_pNotificationWnd->Show())
			m_pNotificationWnd = NULL; // we do not need to delete m_pNotificationWnd because CNotificationWnd is self deleting
	}

	return 0;
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
	((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName = ::AfxGetApp()->GetProfileString(_T("GeneralApp"), _T("TiffScanFileName"), _T("scan.tif"));
	if (!::IsExistingDir(::GetDriveAndDirName(((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName)))
		((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName = ::GetShortFileName(((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName);
	_tcscpy(szFileName, ((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName);
	dlgFile.m_ofn.lpstrFile = szFileName;
	dlgFile.m_ofn.nMaxFile = MAX_PATH;
	dlgFile.m_ofn.lpstrCustomFilter = NULL;
	dlgFile.m_ofn.lpstrFilter = _T("Tag Image File Format (*.tif;*.tiff;*.jfx)\0*.tif;*.tiff;*.jfx\0");
	dlgFile.m_ofn.lpstrDefExt = _T("tif");
	if (dlgFile.DoModal() == IDOK)
	{
		((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName = szFileName;
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
	((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName = ::AfxGetApp()->GetProfileString(_T("GeneralApp"), _T("PdfScanFileName"), _T("scan.pdf"));
	if (!::IsExistingDir(::GetDriveAndDirName(((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName)))
		((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName = ::GetShortFileName(((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName);
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

	// Create & Empty Temp Dir
	CString sScanAndEmailDir;
	sScanAndEmailDir.Format(_T("ScanAndEmail%X"), ::GetCurrentProcessId());
	sScanAndEmailDir = ((CUImagerApp*)::AfxGetApp())->GetAppTempDir() + sScanAndEmailDir;
	if (!::IsExistingDir(sScanAndEmailDir))
	{
		if (!::CreateDir(sScanAndEmailDir))
		{
			::ShowErrorMsg(GetLastError(), TRUE);
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

	// App Exit Pressing ESC
#ifndef VIDEODEVICEDOC
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		if (!((CUImagerApp*)::AfxGetApp())->AreDocsOpen())
			PostMessage(WM_CLOSE, 0, 0);
	}
#endif
	
	return CMDIFrameWnd::PreTranslateMessage(pMsg);
}

#ifdef VIDEODEVICEDOC

LONG CMainFrame::OnAutorunVideoDevices(WPARAM wparam, LPARAM lparam)
{
	if (!((CUImagerApp*)::AfxGetApp())->m_bServiceProcess)
		StatusText();
	for (unsigned int i = 0; i < MAX_DEVICE_AUTORUN_KEYS; i++)
	{
		CString sKey, sDevRegistry;
		sKey.Format(_T("%02u"), i);
		if ((sDevRegistry = ::AfxGetApp()->GetProfileString(_T("DeviceAutorun"), sKey, _T(""))) != _T(""))
		{
			CVideoDeviceDoc* pDoc = (CVideoDeviceDoc*)((CUImagerApp*)::AfxGetApp())->GetVideoDeviceDocTemplate()->OpenDocumentFile(NULL);
			if (pDoc)
			{
				if (!pDoc->OpenNetVideoDevice(sDevRegistry))
					pDoc->OpenDxVideoDevice(-1, sDevRegistry, ::AfxGetApp()->GetProfileString(sDevRegistry, _T("DeviceName"), _T("")));
			}
		}
	}
	return 0;
}

void CMainFrame::OnRecSpeedClick()
{
	::ShellExecute(NULL, _T("open"), LOAD_OPTIMIZATION_ONLINE_PAGE, NULL, NULL, SW_SHOWNORMAL);
}

#endif

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

void CMainFrame::FullScreenModeOn()
{
	// Return if already in fullscreen mode
	if (m_bFullScreenMode)
		return;

	// Return if CMainFrame is not active
	CWnd* pActiveWnd = CWnd::GetActiveWindow();
	if (!(pActiveWnd && pActiveWnd->IsKindOf(RUNTIME_CLASS(CMainFrame))))
		return;

	// Get Child
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

	// Save Placements
	((CUImagerApp*)::AfxGetApp())->SavePlacements();
	((CUImagerApp*)::AfxGetApp())->m_bCanSavePlacements = FALSE;

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

	// Hide Tabs
	m_wndMDITabs.SetMinViews(INT_MAX);

	// Hide Toolbar(s) and Statusbar
#ifndef VIDEODEVICEDOC
	m_wndToolBar.ShowWindow(SW_HIDE);
#endif
	CToolBar* pChildToolBar = ((CToolBarChildFrame*)pChild)->GetToolBar(); 
	if (pChildToolBar)
		pChildToolBar->ShowWindow(SW_HIDE);
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

	// Size MainFrame to Full-Screen and set as
	// Top-Most in restored state
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
	pView->m_nMouseHideTimerCount = 0;
	pView->m_nMouseMoveCount = 0;
	pView->SetTimer(ID_TIMER_FULLSCREEN, FULLSCREEN_TIMER_MS, NULL);

	// Update View
	pView->UpdateWindowSizes(TRUE, TRUE, FALSE);
}

void CMainFrame::FullScreenModeOff()
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

		// Show the Child Toolbar
		CToolBar* pChildToolBar = ((CToolBarChildFrame*)pChild)->GetToolBar();
		if (pChildToolBar)
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

	// Show Toolbar and Statusbar
#ifndef VIDEODEVICEDOC
	m_wndToolBar.ShowWindow(SW_SHOW);
#endif
	m_wndStatusBar.ShowWindow(SW_SHOW);

	// Show Menu
	if (m_hMenu)
		::SetMenu(GetSafeHwnd(), m_hMenu);

	// Show Tabs
	m_wndMDITabs.SetMinViews(2); // 2: because we created m_wndMDITabs with the MT_HIDEWLT2VIEWS flag

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

		// Update View
		pView->UpdateWindowSizes(TRUE, TRUE, FALSE);
	}

	// Now it's possible to save placements again
	((CUImagerApp*)::AfxGetApp())->m_bCanSavePlacements = TRUE;
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

void CMainFrame::InitMenuPositions(CDocument* pDoc/*=NULL*/)
{
	CMenu* pMenu = GetMenu();
	if (pMenu)
	{
		// Main menu entries count
		int nCount = (int)pMenu->GetMenuItemCount();

		// Init menu positions
		if (pDoc == NULL) // MainFrame menu
		{
			m_nFileMenuPos = 0;
			m_nEditMenuPos = 1;
			m_nCaptureMenuPos = 2;
			m_nSettingsMenuPos = 3;
			m_nHelpMenuPos = 4;
#ifdef VIDEODEVICEDOC
			if (nCount == 5)
				pMenu->DeleteMenu(m_nEditMenuPos, MF_BYPOSITION);
			m_nEditMenuPos = -2;
			m_nCaptureMenuPos--;
			m_nSettingsMenuPos--;
			m_nHelpMenuPos--;
#else
			if (nCount == 5)
				pMenu->DeleteMenu(m_nCaptureMenuPos, MF_BYPOSITION);
			m_nCaptureMenuPos = -2;
			m_nSettingsMenuPos--;
			m_nHelpMenuPos--;
#endif
		}
		else if (pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)))
		{
			((CPictureDoc*)pDoc)->m_nFileMenuPos = 0;
			((CPictureDoc*)pDoc)->m_nEditMenuPos = 1;
			((CPictureDoc*)pDoc)->m_nViewMenuPos = 2;
			((CPictureDoc*)pDoc)->m_nPlayMenuPos = 3;
			((CPictureDoc*)pDoc)->m_nSettingsMenuPos = 4;
			((CPictureDoc*)pDoc)->m_nWindowsPos = 5;
			((CPictureDoc*)pDoc)->m_nHelpMenuPos = 6;
		}
#ifdef VIDEODEVICEDOC
		else if (pDoc->IsKindOf(RUNTIME_CLASS(CVideoDeviceDoc)))
		{
			((CVideoDeviceDoc*)pDoc)->m_nFileMenuPos = 0;
			((CVideoDeviceDoc*)pDoc)->m_nCaptureMenuPos = 1;
			((CVideoDeviceDoc*)pDoc)->m_nSettingsMenuPos = 2;
			((CVideoDeviceDoc*)pDoc)->m_nWindowsPos = 3;
			((CVideoDeviceDoc*)pDoc)->m_nHelpMenuPos = 4;
		}
#endif
		else
		{
			ASSERT(FALSE);
		}

		// Redraw menus
		DrawMenuBar();
	}
}

#ifdef VIDEODEVICEDOC
void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	CMDIFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	// Get main menu and direct sub menu
	CMenu* pMainMenu = GetMenu();
	CMenu* pSubMenu = pMainMenu ? pMainMenu->GetSubMenu(nIndex) : NULL;

	// Only handle direct popups of the main menu
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

				if (pDoc->IsKindOf(RUNTIME_CLASS(CUImagerDoc)))
				{
					if (idx == ((CUImagerDoc*)pDoc)->m_nFileMenuPos)
						CleanupFileMenu(pPopupMenu);
					else if (idx == ((CUImagerDoc*)pDoc)->m_nCaptureMenuPos)
						PopulateCaptureMenu(pPopupMenu);
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
			if (idx == m_nFileMenuPos)
				CleanupFileMenu(pPopupMenu);
			else if (idx == m_nCaptureMenuPos)
				PopulateCaptureMenu(pPopupMenu);
		}	
	}
}

void CMainFrame::CleanupFileMenu(CMenu* pPopupMenu)
{
	int nPos = 0;
	while (nPos < pPopupMenu->GetMenuItemCount())
	{
		// Item IDs, GetMenuItemID() returns:
		// -1: pop-up menu or not existing, 0: separator, > 0: ID
		int nIDPrev = -1;
		if ((nPos - 1) >= 0)
			nIDPrev = (int)pPopupMenu->GetMenuItemID(nPos - 1);
		int nID = (int)pPopupMenu->GetMenuItemID(nPos);

		// Leave item?
		if ((nID == 0 && nIDPrev != 0)		||	// keep separator if not double
			nID == ID_FILE_CLOSE			||
			nID == ID_FILE_RELOAD			||
			nID == ID_FILE_SAVE				||
			nID == ID_FILE_SAVE_AS			||
			nID == ID_FILE_INFO				||
			nID == ID_FILE_PRINT_PREVIEW	||
			nID == ID_APP_EXIT)
			nPos++;
		else
			pPopupMenu->DeleteMenu(nPos, MF_BYPOSITION);
	}

	// Cleanup beginning separator
	if (pPopupMenu->GetMenuItemCount() > 0 && pPopupMenu->GetMenuItemID(0) == 0)
		pPopupMenu->DeleteMenu(0, MF_BYPOSITION);

	// Cleanup ending separator
	if (pPopupMenu->GetMenuItemCount() > 0 && pPopupMenu->GetMenuItemID(pPopupMenu->GetMenuItemCount() - 1) == 0)
		pPopupMenu->DeleteMenu(pPopupMenu->GetMenuItemCount() - 1, MF_BYPOSITION);
}

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

int CMainFrame::ScaleToolBar(int nDPI, int n)
{
	if (nDPI > 192)			// more than 200%
		return 3 * n;
	else if (nDPI > 120)	// more than 125%
		return 2 * n;
	else
		return n;
}

#ifndef VIDEODEVICEDOC
BOOL CMainFrame::SwitchToolBar(int nDPI, BOOL bCallShowControlBar/*=TRUE*/)
{
	// Load and set sizes
	int nScale = ScaleToolBar(nDPI, 1);
	if (nScale >= 3)
	{
		if (!m_wndToolBar.LoadToolBar(IDR_MAINFRAME3X))
			return FALSE;
		m_wndToolBar.SetSizes(	CSize(TOOLBAR_BUTTON_SIZE_3X, TOOLBAR_BUTTON_SIZE_3Y),
								CSize(TOOLBAR_IMAGE_SIZE_3X, TOOLBAR_IMAGE_SIZE_3Y));
	}
	else if (nScale == 2)
	{
		if (!m_wndToolBar.LoadToolBar(IDR_MAINFRAME2X))
			return FALSE;
		m_wndToolBar.SetSizes(	CSize(TOOLBAR_BUTTON_SIZE_2X, TOOLBAR_BUTTON_SIZE_2Y),
								CSize(TOOLBAR_IMAGE_SIZE_2X, TOOLBAR_IMAGE_SIZE_2Y));
	}
	else
	{
		if (!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
			return FALSE;
		m_wndToolBar.SetSizes(	CSize(TOOLBAR_BUTTON_SIZE_X, TOOLBAR_BUTTON_SIZE_Y),
								CSize(TOOLBAR_IMAGE_SIZE_X, TOOLBAR_IMAGE_SIZE_Y));
	}

	// When a toolbar is already loaded then the ShowControlBar() call is necessary to correctly update it
	if (bCallShowControlBar)
		ShowControlBar(&m_wndToolBar, m_wndToolBar.IsWindowVisible(), TRUE);

	return TRUE;
}
#endif

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

#ifdef VIDEODEVICEDOC
void CMainFrame::UpdateDetectionMaxMaxFrames()
{
	// Calculate
	LONGLONG llNum = g_nOSUsablePhysRamMB;
	llNum <<= 20; // RAM in bytes
	llNum *= MOVDET_MAX_MAX_FRAMES_RAM_PERCENT;
	llNum /= 100;
	LONGLONG llDen = 0;
	CUImagerMultiDocTemplate* pVideoDeviceDocTemplate = ((CUImagerApp*)::AfxGetApp())->GetVideoDeviceDocTemplate();
	POSITION posVideoDeviceDoc = pVideoDeviceDocTemplate->GetFirstDocPosition();
	while (posVideoDeviceDoc)
	{
		CVideoDeviceDoc* pVideoDeviceDoc = (CVideoDeviceDoc*)(pVideoDeviceDocTemplate->GetNextDoc(posVideoDeviceDoc));
		if (pVideoDeviceDoc)
			llDen += pVideoDeviceDoc->m_ProcessFrameBMI.bmiHeader.biSizeImage;
	}
	int nDetectionMaxMaxFrames;
	if (llDen > 0)
	{
		nDetectionMaxMaxFrames = (int)(llNum / llDen);
		if (nDetectionMaxMaxFrames > MOVDET_MAX_MAX_FRAMES_IN_LIST)
			nDetectionMaxMaxFrames = MOVDET_MAX_MAX_FRAMES_IN_LIST;
		else if (nDetectionMaxMaxFrames < 1)
			nDetectionMaxMaxFrames = 1;
	}
	else
		nDetectionMaxMaxFrames = MOVDET_MAX_MAX_FRAMES_IN_LIST;

	// Apply
	if (((CUImagerApp*)::AfxGetApp())->m_nDetectionMaxMaxFrames != nDetectionMaxMaxFrames)
		((CUImagerApp*)::AfxGetApp())->m_nDetectionMaxMaxFrames = nDetectionMaxMaxFrames;
}
#endif

CString CMainFrame::GetDiskStats(LPCTSTR lpszPath)
{
	CString sDiskStats;

	// - GetDiskFreeSpaceEx's lpDirectoryName must include a trailing backslash
	// - GetDiskFreeSpaceEx's lpDirectoryName does not have to specify the
	//   root directory (the function accepts any directory on a disk)
	CString sPath(lpszPath);
	sPath.TrimRight(_T('\\'));
	sPath += _T("\\");

	// Get the disk free space and the total disk size
	ULARGE_INTEGER FreeBytesAvailableToCaller;
	ULARGE_INTEGER TotalNumberOfBytesAvailableToCaller;
	if (::GetDiskFreeSpaceEx(sPath,
							&FreeBytesAvailableToCaller,
							&TotalNumberOfBytesAvailableToCaller,
							NULL))
	{
		// Format stats
		sDiskStats.Format(_T("HD: %I64u/%I64u") + ML_STRING(1826, "GB"),
						(TotalNumberOfBytesAvailableToCaller.QuadPart - FreeBytesAvailableToCaller.QuadPart) >> 30,
						TotalNumberOfBytesAvailableToCaller.QuadPart >> 30);
	}

	return sDiskStats;
}

BOOL CMainFrame::IsAddressInHeapRegion(LPVOID p, HEAPREGIONARRAY& Regions)
{
	for (int i = 0 ; i < Regions.GetCount() ; i++)
	{
		if (p >= Regions[i].lpFirstBlock && p < Regions[i].lpLastBlock)
			return TRUE;
	}
	return FALSE;
}

void CMainFrame::LogSysUsage()
{
	// Heap walk to temp binary file
	CString sHeapTempBinFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), HEAP_LOGNAME_EXT);
	HANDLE hHeapTempBinFile = ::CreateFile(	sHeapTempBinFileName,
											GENERIC_WRITE,
											FILE_SHARE_READ, NULL, CREATE_ALWAYS,
											FILE_ATTRIBUTE_NORMAL, NULL);
	if (hHeapTempBinFile == INVALID_HANDLE_VALUE)
		return;
	PROCESS_HEAP_ENTRY HeapEntry;
	::memset(&HeapEntry, 0, sizeof(HeapEntry));
	HANDLE heap = (HANDLE)::_get_heap_handle();
	if (::HeapLock(heap))
	{
		while (::HeapWalk(heap, &HeapEntry))
		{
			DWORD NumberOfBytesWritten;
			::WriteFile(hHeapTempBinFile, &HeapEntry, sizeof(HeapEntry), &NumberOfBytesWritten, NULL);
		}
		::HeapUnlock(heap);
	}
	::CloseHandle(hHeapTempBinFile);

	// Get Heap Usage
	hHeapTempBinFile = ::CreateFile(sHeapTempBinFileName,
									GENERIC_READ,
									FILE_SHARE_READ, NULL, OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL, NULL);
	if (hHeapTempBinFile == INVALID_HANDLE_VALUE)
	{
		::DeleteFile(sHeapTempBinFileName);
		return;
	}
	DWORD NumberOfBytesRead;
	HEAPREGIONARRAY HeapRegions;
	while (::ReadFile(hHeapTempBinFile, (LPVOID)&HeapEntry, sizeof(HeapEntry), &NumberOfBytesRead, NULL) && NumberOfBytesRead == sizeof(HeapEntry))
	{
		// A heap consists of one or more regions of virtual memory. 
		// HeapWalk returns an entry for each of those regions marking
		// them with the PROCESS_HEAP_REGION flag. Unfortunately we
		// cannot know the region index of a region because iRegionIndex
		// is always set to 0.
		if ((HeapEntry.wFlags & PROCESS_HEAP_REGION) != 0)
		{
			HEAPREGION r;
			r.lpFirstBlock = HeapEntry.Region.lpFirstBlock;	// pointer to the first valid memory block in this heap region
			r.lpLastBlock = HeapEntry.Region.lpLastBlock;	// pointer to the first invalid memory block in this heap region
			HeapRegions.Add(r);
		}	
	}
	ULONGLONG ullAllocatedData = 0;
	ULONGLONG ullAllocatedOverhead = 0;
	ULONGLONG ullAllocatedBigData = 0;
	ULONGLONG ullAllocatedBigOverhead = 0;
	ULONGLONG ullUncommittedFreeData = 0;
	ULONGLONG ullUncommittedFreeOverhead = 0;
	ULONGLONG ullCommittedFreeData = 0;
	ULONGLONG ullCommittedFreeOverhead = 0;
	::SetFilePointer(hHeapTempBinFile, 0, NULL, FILE_BEGIN);
	while (::ReadFile(hHeapTempBinFile, (LPVOID)&HeapEntry, sizeof(HeapEntry), &NumberOfBytesRead, NULL) && NumberOfBytesRead == sizeof(HeapEntry))
	{
		if ((HeapEntry.wFlags & PROCESS_HEAP_REGION) == 0 &&
			(HeapEntry.cbData > 0 || HeapEntry.cbOverhead > 0))
		{
			if ((HeapEntry.wFlags & PROCESS_HEAP_ENTRY_BUSY) != 0)
			{
				ullAllocatedData += HeapEntry.cbData;
				ullAllocatedOverhead += HeapEntry.cbOverhead;
				if (!IsAddressInHeapRegion((LPBYTE)HeapEntry.lpData + HeapEntry.cbData / 2, // take the middle and not the beginning because of possible alignment differences
										HeapRegions))
				{
					// Big heap blocks are not directly managed by the heap,
					// they are allocated with VirtualAlloc and their data pointer 
					// do not point to a PROCESS_HEAP_REGION
					ullAllocatedBigData += HeapEntry.cbData;
					ullAllocatedBigOverhead += HeapEntry.cbOverhead;
				}

				// Older unused flags for non-big blocks
				//
				// PROCESS_HEAP_ENTRY_MOVEABLE
				// A block can be moved when it has been allocated with
				// LMEM_MOVEABLE (LocalAlloc) or GMEM_MOVEABLE (GlocalAlloc).
				// Block.hMem is the handle of the moveable memory block.
				//
				// PROCESS_HEAP_ENTRY_DDESHARE
				// Unlike Windows version 3.x, this memory is not shared globally.
				// However, this flag is available for compatibility purposes.
				// It may be used by some applications to enhance the performance
				// of DDE operations and should, therefore, be specified if the memory
				// is to be used for DDE.
			}
			else if ((HeapEntry.wFlags & PROCESS_HEAP_UNCOMMITTED_RANGE) != 0)
			{
				ullUncommittedFreeData += HeapEntry.cbData;
				ullUncommittedFreeOverhead += HeapEntry.cbOverhead;
			}
			else
			{
				ullCommittedFreeData += HeapEntry.cbData;
				ullCommittedFreeOverhead += HeapEntry.cbOverhead;
			}
		}
	}
	::CloseHandle(hHeapTempBinFile);
	::DeleteFile(sHeapTempBinFileName);

	// Get Virtual Memory Usage
	ULONGLONG ullRegions; ULONGLONG ullFree; ULONGLONG ullReserved; ULONGLONG ullCommitted;
	ULONGLONG ullMaxFree; ULONGLONG ullMaxReserved; ULONGLONG ullMaxCommitted; double dFragmentation;
	::GetMemoryStats(&ullRegions, &ullFree, &ullReserved, &ullCommitted,
					&ullMaxFree, &ullMaxReserved, &ullMaxCommitted, &dFragmentation);

	// Get HD Usage
#ifdef VIDEODEVICEDOC
	CString sDiskStats(GetDiskStats(((CUImagerApp*)::AfxGetApp())->m_sMicroApacheDocRoot));
#else
	CString sDiskStats(GetDiskStats(((CUImagerApp*)::AfxGetApp())->GetAppTempDir()));
#endif

	// Message
	::LogLine(
		_T("%s | ")
		_T("CPU: %d thread | ")
		_T("RAM: %0.1f%s | ")
		_T("VMEM: used=%s(max %s) res=%s(max %s) free=%s(max %s) frag=%0.1f%% regions=%I64u | ")
		_T("HEAP: used=%s (%s big), free(committed)=%s, free(uncommitted)=%s"),
		sDiskStats,
		((CUImagerApp*)::AfxGetApp())->m_nThreadCount,
		(double)g_nOSUsablePhysRamMB / 1024.0, ML_STRING(1826, "GB"),
		::FormatBytes(ullCommitted), ::FormatBytes(ullMaxCommitted),
		::FormatBytes(ullReserved), ::FormatBytes(ullMaxReserved),
		::FormatBytes(ullFree), ::FormatBytes(ullMaxFree),
		dFragmentation, ullRegions,
		::FormatBytes(ullAllocatedData + ullAllocatedOverhead),
		::FormatBytes(ullAllocatedBigData + ullAllocatedBigOverhead),
		::FormatBytes(ullCommittedFreeData + ullCommittedFreeOverhead),
		::FormatBytes(ullUncommittedFreeData + ullUncommittedFreeOverhead));
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	CMDIFrameWnd::OnTimer(nIDEvent);

	if (nIDEvent == ID_TIMER_CLOSING_ALL)
	{
		((CUImagerApp*)::AfxGetApp())->CloseAll();
	}
	else if (nIDEvent == ID_TIMER_1SEC)
	{
		// If the No Donation flag changes trigger the drawing
		static BOOL bNoDonation = FALSE;
		if (g_DonorEmailValidateThread.m_bNoDonation != bNoDonation)
		{
			bNoDonation = g_DonorEmailValidateThread.m_bNoDonation;
			m_MDIClientWnd.Invalidate();
		}

		// System Usage
		static ULONGLONG ullCount = 0ULL;
		ullCount++;
		if (g_nLogLevel > 1)
		{
			// Log every 15 seconds
			if ((ullCount % 15) == 0)
				LogSysUsage();
		}
#ifdef VIDEODEVICEDOC
		else
		{
			// Single log 1 minute after cameras starting
			if (ullCount == (ULONGLONG)((((CUImagerApp*)::AfxGetApp())->m_dwFirstStartDelayMs / 1000U) + 60U))
				LogSysUsage();
		}

		// REC Speed
		static int nFlashState = 0;
		int nMinSaveFrameListSpeedPercent = -1;
		CString sMinSaveFrameListAssignedDeviceName;
		POSITION posTemplate = ((CUImagerApp*)::AfxGetApp())->GetFirstDocTemplatePosition();
		while (posTemplate)
		{
			CUImagerMultiDocTemplate* curTemplate = (CUImagerMultiDocTemplate*)((CUImagerApp*)::AfxGetApp())->GetNextDocTemplate(posTemplate);
			POSITION posDoc = curTemplate->GetFirstDocPosition();
			while (posDoc)
			{
				CDocument* pDoc = curTemplate->GetNextDoc(posDoc);
				if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CVideoDeviceDoc)))
				{
					int nSaveFrameListSpeedPercent = ((CVideoDeviceDoc*)pDoc)->m_nSaveFrameListSpeedPercent;
					if (nSaveFrameListSpeedPercent >= 0)
					{
						if (nMinSaveFrameListSpeedPercent == -1 || nSaveFrameListSpeedPercent < nMinSaveFrameListSpeedPercent)
						{
							nMinSaveFrameListSpeedPercent = nSaveFrameListSpeedPercent;
							sMinSaveFrameListAssignedDeviceName = ((CVideoDeviceDoc*)pDoc)->GetAssignedDeviceName();
						}
					}
				}
			}
		}
		CString sRecSpeedPaneText;
		sRecSpeedPaneText.LoadString(ID_INDICATOR_REC_SPEED); // this string has a leading and a trailing space
		if (nMinSaveFrameListSpeedPercent >= 0)
		{
			CString sMinSaveFrameListSpeed;
			sMinSaveFrameListSpeed.Format(_T("%s:%s%0.2fx"), sMinSaveFrameListAssignedDeviceName, sRecSpeedPaneText, (double)nMinSaveFrameListSpeedPercent / 100.0);
			if (nMinSaveFrameListSpeedPercent < 100)
			{
				if (nFlashState == 2)
					GetStatusBar()->SetPaneText(GetStatusBar()->CommandToIndex(ID_INDICATOR_REC_SPEED), _T(""));
				else
					GetStatusBar()->SetPaneText(GetStatusBar()->CommandToIndex(ID_INDICATOR_REC_SPEED), _T(" *** ") + sMinSaveFrameListSpeed + _T(" *** "));
			}
			else
				GetStatusBar()->SetPaneText(GetStatusBar()->CommandToIndex(ID_INDICATOR_REC_SPEED), _T(" ") + sMinSaveFrameListSpeed + _T(" "));
		}
		else
			GetStatusBar()->SetPaneText(GetStatusBar()->CommandToIndex(ID_INDICATOR_REC_SPEED), sRecSpeedPaneText);
		nFlashState = (nFlashState + 1) % 3;

		// Update m_nDetectionMaxMaxFrames which is the global upper boundary
		// for CVideoDeviceDoc::m_nDetectionMaxFrames 
		UpdateDetectionMaxMaxFrames();

		// Frames dropping logic
		LONGLONG llDropThresholdBytes = g_nOSUsablePhysRamMB;
		llDropThresholdBytes <<= 20; // RAM in bytes
		llDropThresholdBytes *= MOVDET_DROP_FRAMES_RAM_PERCENT;
		llDropThresholdBytes /= 100;
		if (CDib::m_llOverallSharedMemoryBytes > llDropThresholdBytes &&
			!((CUImagerApp*)::AfxGetApp())->m_bMovDetDropFrames)
		{
			// Disable frames storing
			((CUImagerApp*)::AfxGetApp())->m_bMovDetDropFrames = TRUE;
			
			// Log
			::LogLine(	_T("*** %s *** <-- BUF %0.1f%s > %d%% RAM"),
						ML_STRING(1815, "OUT OF MEMORY / OVERLOAD: dropping frames"),
						(double)(CDib::m_llOverallSharedMemoryBytes >> 20) / 1024.0, ML_STRING(1826, "GB"),
						MOVDET_DROP_FRAMES_RAM_PERCENT);
		}
		else if (((CUImagerApp*)::AfxGetApp())->m_bMovDetDropFrames &&
				CDib::m_llOverallSharedMemoryBytes == 0)
		{
			// Re-enable frames storing
			((CUImagerApp*)::AfxGetApp())->m_bMovDetDropFrames = FALSE;
		}
#endif
	}
}

void CMainFrame::StatusText(CString sText/*=_T("")*/)
{
	// Update variable
	m_sStatusBarString = sText;
	
	// Send Message handled in OnSetMessageString()
	if (m_sStatusBarString == _T(""))
		SetMessageText(AFX_IDS_IDLEMESSAGE);			// = SendMessage(WM_SETMESSAGESTRING, (WPARAM)AFX_IDS_IDLEMESSAGE);
	else
    	SetMessageText((LPCTSTR)m_sStatusBarString);	// = SendMessage(WM_SETMESSAGESTRING, 0, (LPARAM)(LPCTSTR)m_sStatusBarString);
}

LRESULT CMainFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
	// Adapted version of CFrameWnd::OnSetMessageString() from winfrm.cpp
	// see also CChildFrame::OnSetMessageString()

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
			ASSERT(wParam == 0);	// can't have both an ID and a string
			sMsg = (LPCTSTR)lParam;	// set an explicit string
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
			// NT64: Assume IDs are still 32-bit
			else
			{
				GetMessageString((UINT)wParam, strMessage);
				sMsg = strMessage;
			}
		}
		pMessageBar->SetWindowText(sMsg);

		// no need to update owner of the bar in terms of last message selected
		// because we are the owner and are updated below!
	}

	m_nIDLastMessage = (UINT)wParam;    	// new ID (or 0)
	m_nIDTracking = (UINT)wParam;       	// so F1 on toolbar buttons work
	return nIDLast;
}

void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	CMDIFrameWnd::OnUpdateFrameTitle(bAddToTitle);
	m_wndMDITabs.Update(); // sync the mdi tab control with all views
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
	if (nPercent == 100)
		StatusText();
	else
	{
		CString sProgress;
		sProgress.Format(_T("%d%%"), nPercent);
		StatusText(sProgress);
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
		((CUImagerApp*)::AfxGetApp())->SaveOnEndSession();
}

LRESULT CMainFrame::OnCopyData(WPARAM /*wParam*/, LPARAM lParam)
{
	COPYDATASTRUCT* pCDS = reinterpret_cast<COPYDATASTRUCT*>(lParam);
	TCHAR* pszFiles = static_cast<TCHAR*>(pCDS->lpData);
	int nShellCommand = (int)pCDS->dwData;
	if (pszFiles)
	{
		if (((CUImagerApp*)::AfxGetApp())->m_bShuttingDownApplication)
			::MessageBeep(0xFFFFFFFF);
		else if (!m_bFullScreenMode)
		{
			OnOpenFromTray(); // this restores us if minimized to tray
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
