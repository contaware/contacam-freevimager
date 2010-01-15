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
#include "EnumGDIObjectsDlg.h"
#include "CPUCount.h"
#include "CPUSpeed.h"
#include "PostDelayedMessage.h"
#include "Dib.h"
#include "Quantizer.h"
#include "Tiff2Pdf.h"
#include "PdfSaveDlg.h"
#include "NoVistaFileDlg.h"
#ifdef SUPPORT_LIBAVCODEC
#include "AVRec.h"
#endif

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
	ON_COMMAND(ID_FILE_ACQUIRE_TO_PDF_DIRECT, OnFileAcquireToPdfDirect)
	ON_COMMAND(ID_FILE_LOCK_TWAINCLOSE, OnFileLockTwainclose)
	ON_UPDATE_COMMAND_UI(ID_FILE_LOCK_TWAINCLOSE, OnUpdateFileLockTwainclose)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_INDICATOR_XCOORDINATE, OnXCoordinatesDoubleClick)
	ON_COMMAND(ID_INDICATOR_YCOORDINATE, OnYCoordinatesDoubleClick)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_XCOORDINATE, OnUpdateIndicatorXCoordinate)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_YCOORDINATE, OnUpdateIndicatorYCoordinate)
	ON_MESSAGE(WM_PROGRESS, OnProgress)
	ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
	ON_MESSAGE(WM_THREADSAFE_STATUSTEXT, OnThreadSafeStatusText)
	ON_MESSAGE(WM_THREADSAFE_OPEN_DOC, OnThreadSafeOpenDoc)
	ON_MESSAGE(WM_THREADSAFE_AVISTART_SHRINKTO, OnThreadSafeAviStartShrinkTo)
	ON_MESSAGE(WM_SHRINKDOC_TERMINATED, OnShrinkDocTerminated)
	ON_MESSAGE(WM_TASKBAR_BUTTON, OnTaskBarButton)
	ON_MESSAGE(WM_RESTORE_FRAME, OnRestoreAllFrames)
	ON_MESSAGE(WM_ALL_CLOSED, OnAllClosed)
	ON_MESSAGE(WM_VIDEOAVI_FULLSCREEN_MODE_ON, OnVideoAviFullScreenModeOn)
	ON_MESSAGE(WM_VIDEOAVI_FULLSCREEN_MODE_OFF, OnVideoAviFullScreenModeOff)
	ON_MESSAGE(WM_SCANANDEMAIL, OnScanAndEmail)
	ON_MESSAGE(WM_TRAY_NOTIFICATION, OnTrayNotification)
	ON_MESSAGE(WM_COPYDATA, OnCopyData)
#ifdef VIDEODEVICEDOC
	ON_MESSAGE(WM_AUTORUN_VIDEODEVICES, OnAutorunVideoDevices)
#endif
END_MESSAGE_MAP()

static SBACTPANEINFO sba_indicators[] = 
{
	{ ID_SEPARATOR, _T(""), SBACTF_NORMAL },		// status line indicator
	{ ID_INDICATOR_PROGRESS, _T(""), SBACTF_NORMAL },
	{ ID_INDICATOR_XCOORDINATE, _T("Double-click to change unit"), SBACTF_AUTOFIT | SBACTF_COMMAND | SBACTF_HANDCURSOR },
	{ ID_INDICATOR_YCOORDINATE, _T("Double-click to change unit"), SBACTF_AUTOFIT | SBACTF_COMMAND | SBACTF_HANDCURSOR }, 
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
	m_sStatusBarString = _T("");
	m_bProgressIndicatorCreated = FALSE;
	m_pEnumGDIObjectsDlg = NULL;
	m_sPlayGifMenuItem = _T("");
	m_sStopGifMenuItem = _T("");
	m_sNextPageFrameMenuItem = _T("");
	m_sPrevPageFrameMenuItem = _T("");
	m_hJPEGAdvancedMenu = NULL;
	m_sJPEGAdvancedMenuItem = _T("");
	m_TiffScan = NULL;
	m_bScanAndEmail = FALSE;
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
 
	// Init Twain Close Locked Flag
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		m_bTwainCloseLocked = (BOOL)((CUImagerApp*)::AfxGetApp())->GetProfileInt(_T("GeneralApp"), _T("TwainCloseLocked"), FALSE);
	
#ifdef VIDEODEVICEDOC
	// Scheduler Timer
	SetTimer(ID_TIMER_SCHEDULER, SCHEDULER_TIMER_MS, NULL);
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

	// First Time that the App runs after Install (or Upgrade)
	// and first run ever or after a uninstall
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
	{
		// Get flags
		((CUImagerApp*)::AfxGetApp())->m_bFirstRun = (BOOL)((CUImagerApp*)::AfxGetApp())->GetProfileInt(_T("GeneralApp"), _T("FirstRun"), FALSE);
		((CUImagerApp*)::AfxGetApp())->m_bFirstRunEver = (BOOL)((CUImagerApp*)::AfxGetApp())->GetProfileInt(_T("GeneralApp"), _T("FirstRunEver"), TRUE);
		
		// Fix inconsistency
		if (((CUImagerApp*)::AfxGetApp())->m_bFirstRunEver && !((CUImagerApp*)::AfxGetApp())->m_bFirstRun)
			((CUImagerApp*)::AfxGetApp())->m_bFirstRun = TRUE;
	}

	// Setup Tray Icon if not hiding Mainframe
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings &&
		!((CUImagerApp*)::AfxGetApp())->m_bHideMainFrame)
	{
#ifdef VIDEODEVICEDOC
		if (((CUImagerApp*)::AfxGetApp())->m_bFirstRunEver)
			((CUImagerApp*)::AfxGetApp())->WriteProfileInt(_T("GeneralApp"), _T("TrayIcon"), TRUE);
#endif
		((CUImagerApp*)::AfxGetApp())->m_bTrayIcon = (BOOL)((CUImagerApp*)::AfxGetApp())->GetProfileInt(_T("GeneralApp"), _T("TrayIcon"), FALSE);
		if (((CUImagerApp*)::AfxGetApp())->m_bTrayIcon)
		{
			TrayIcon(TRUE);
			if (!((CUImagerApp*)::AfxGetApp())->m_bFirstRunEver)
				m_TrayIcon.MinimizeToTray();
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
	
	// Update Print Preview's Pane and Status Text for Old MFC
#if _MFC_VER < 0x0700
	CView* pView = GetActiveView();
	if (pView && pView->IsKindOf(RUNTIME_CLASS(CPicturePrintPreviewView)))
	{
		((CPicturePrintPreviewView*)pView)->UpdatePaneText();
		((CPicturePrintPreviewView*)pView)->UpdateStatusText();
		return;
	}
#endif

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
	KillTimer(ID_TIMER_SCHEDULER);
#endif

	// Close Eventually Open Dlgs
	if (m_pEnumGDIObjectsDlg)
	{
		// m_pEnumGDIObjectsDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		m_pEnumGDIObjectsDlg->Close();
	}

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

void CMainFrame::TwainCopyImage(HANDLE hBitmap, TW_IMAGEINFO& info)
{
	TwainSetImage((HBITMAP)hBitmap, info.ImageWidth, info.ImageLength, info.BitsPerPixel);
}

LONG CMainFrame::OnScanAndEmail(WPARAM wparam, LPARAM lparam)
{
	// Send
	((CUImagerApp*)::AfxGetApp())->SendMail(((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName);

	// Delete
	::DeleteFile(((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName);
	::DeleteFile(((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName);

	return 1;
}

BOOL CMainFrame::TwainCanClose()
{
	if (m_TiffScan)
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
							((CUImagerApp*)::AfxGetApp())->m_sPdfScanPaperSize,
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
								((CUImagerApp*)::AfxGetApp())->m_sPdfScanPaperSize,
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

	return TRUE;
}

void CMainFrame::TwainSetImage(HANDLE hDib, int width, int height, int bpp)
{
	if (!m_bFullScreenMode)
	{
		if (((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName != _T(""))
		{
			CDib Dib;

			// Copy Bitmap
			Dib.CopyFromHandle(hDib);

			// Count the Unique Colors
			int nNumColors = Dib.CountUniqueColors(this, TRUE);

			// Choose Right Compression Type
			int nCompression;
			if (nNumColors <= 2)
				nCompression = COMPRESSION_CCITTFAX4;
			else if (nNumColors <= 32)
			{
				// Convert to 4bpp
				if (bpp != 4)
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
				if (bpp != 8)
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

				DWORD dwSizeImage = DWALIGNEDWIDTHBYTES(width * bpp) * height;
				if (((CUImagerApp*)::AfxGetApp())->IsPictureSizeBig(dwSizeImage))
				{	
					// Create MM File
					if ((pDoc->m_sFileName = TwainSetImageMM(pDoc->m_pDib, width, height, bpp)) == _T(""))
					{
						pDoc->CloseDocumentForce();
						return;
					}
					else
						pDoc->SetPathName(pDoc->m_sFileName, TRUE);

					// Copy Bitmap to MM File
					pDoc->m_pDib->CopyFromHandle(hDib);

					// Create Preview Dib
					if (!CPictureDoc::CreatePreviewDib(pDoc->m_pDib))
					{
						pDoc->CloseDocumentForce();
						return;
					}

					// Set Big Picture Flag
					pDoc->m_bBigPicture = TRUE;

					// End Wait Cursor (Started inside SetImageMM)
					EndWaitCursor();
				}
				else
				{
					pDoc->m_pDib->CopyFromHandle(hDib);
					pDoc->SetModifiedFlag();
				}

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
}

CString CMainFrame::TwainSetImageMM(CDib* pDib, int width, int height, int bpp)
{
	ASSERT(pDib);

	// Display the Save As Dialog
	TCHAR szFileName[MAX_PATH];
	CNoVistaFileDlg dlgFile(FALSE);
	CString sFileName;
	sFileName.Format(_T("scan_%ux%u.bmp"), width, height);
	_tcscpy(szFileName, sFileName);
	dlgFile.m_ofn.lpstrFile = szFileName;
	dlgFile.m_ofn.nMaxFile = MAX_PATH;
	dlgFile.m_ofn.lpstrCustomFilter = NULL;
	dlgFile.m_ofn.Flags |= OFN_EXPLORER;
	dlgFile.m_ofn.lpstrFilter = _T("Windows Bitmap (*.bmp;*.dib)\0*.bmp;*.dib\0");
	dlgFile.m_ofn.lpstrDefExt = _T("bmp");
	if (dlgFile.DoModal() == IDOK)
	{
		// Begin Wait Cursor
		BeginWaitCursor();

		sFileName = szFileName;
		BITMAPINFOHEADER Bmih;
		Bmih.biSize = sizeof(BITMAPINFOHEADER); 
		Bmih.biWidth =  width; 
		Bmih.biHeight = height; 
		Bmih.biPlanes = 1; 
		Bmih.biBitCount = bpp;
		Bmih.biCompression = BI_RGB; 
		Bmih.biSizeImage = 0;
		Bmih.biXPelsPerMeter = 0; 
		Bmih.biYPelsPerMeter = 0; 
		Bmih.biClrUsed = 0; 
		Bmih.biClrImportant = 0;
		pDib->SetBMI((LPBITMAPINFO)&Bmih);
		if (!pDib->MMCreateBMP(szFileName))
			return _T("");
		else
			return sFileName;
	}
	else
		return _T("");
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

		// Start Micro Apache shutdown
#ifdef VIDEODEVICEDOC
		CVideoDeviceDoc::MicroApacheInitShutdown();
#endif

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
	CNoVistaFileDlg dlgFile(FALSE);
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
	dlgFile.m_ofn.Flags |= OFN_EXPLORER;
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
	CNoVistaFileDlg dlgFile(FALSE);
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
	dlgFile.m_ofn.Flags |= OFN_EXPLORER;
	dlgFile.m_ofn.lpstrFilter = _T("Pdf Document (*.pdf)\0*.pdf\0");
	dlgFile.m_ofn.lpstrDefExt = _T("pdf");
	if (dlgFile.DoModal() == IDOK)
	{
		((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName = szFileName;
		if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
			::AfxGetApp()->WriteProfileString(_T("GeneralApp"), _T("PdfScanFileName"), ((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName);
		((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName = ::MakeTempFileName(	((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),
																					::GetFileNameNoExt(((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName) + _T(".tif"));
		
		// Show the PDF Dialog
		CPdfSaveDlg dlg(this);
		dlg.m_nCompressionQuality = ((CUImagerApp*)::AfxGetApp())->m_nPdfScanCompressionQuality;
		dlg.m_sPdfScanPaperSize = ((CUImagerApp*)::AfxGetApp())->m_sPdfScanPaperSize;
		if (dlg.DoModal() == IDOK)
		{
			// Set Pdf Vars
			((CUImagerApp*)::AfxGetApp())->m_nPdfScanCompressionQuality = dlg.m_nCompressionQuality;
			((CUImagerApp*)::AfxGetApp())->m_sPdfScanPaperSize = dlg.m_sPdfScanPaperSize;
			if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
			{
				((CUImagerApp*)::AfxGetApp())->WriteProfileInt(	_T("GeneralApp"),
																_T("PdfScanCompressionQuality"),
																((CUImagerApp*)::AfxGetApp())->m_nPdfScanCompressionQuality);
				((CUImagerApp*)::AfxGetApp())->WriteProfileString(_T("GeneralApp"),
																_T("PdfScanPaperSize"),
																((CUImagerApp*)::AfxGetApp())->m_sPdfScanPaperSize);
			}
			
			// Acquire
			TwainAcquire(TWCPP_ANYCOUNT);
		}
		else
		{
			((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName = _T("");
			((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName = _T("");
		}
	}
	else
	{
		((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName = _T("");
		((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName = _T("");
	}
}

void CMainFrame::OnFileAcquireToPdfDirect() 
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
	CNoVistaFileDlg dlgFile(FALSE);
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
	dlgFile.m_ofn.Flags |= OFN_EXPLORER;
	dlgFile.m_ofn.lpstrFilter = _T("Pdf Document (*.pdf)\0*.pdf\0");
	dlgFile.m_ofn.lpstrDefExt = _T("pdf");
	if (dlgFile.DoModal() == IDOK)
	{
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
	if (!::IsExistingDir(((CUImagerApp*)::AfxGetApp())->GetAppTempDir() + TMP_SCAN_AND_EMAIL_DIR))
	{
		if (!::CreateDir(((CUImagerApp*)::AfxGetApp())->GetAppTempDir() + TMP_SCAN_AND_EMAIL_DIR))
		{
			::ShowLastError(TRUE);
			return;
		}
	}
	else
	{
		if (!::DeleteDirContent(((CUImagerApp*)::AfxGetApp())->GetAppTempDir() + TMP_SCAN_AND_EMAIL_DIR))
		{
			::AfxMessageBox(ML_STRING(1225, "Error While Deleting The Temporary Folder."), MB_OK | MB_ICONSTOP);
			return;
		}
	}

	// Set Temp File Names
	((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName = ((CUImagerApp*)::AfxGetApp())->GetAppTempDir() +
															TMP_SCAN_AND_EMAIL_DIR + _T("\\Document.pdf");
	((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName = ((CUImagerApp*)::AfxGetApp())->GetAppTempDir() +
															TMP_SCAN_AND_EMAIL_DIR + _T("\\Document.tif");

	// Show the PDF Dialog
	CPdfSaveDlg dlg(this);
	dlg.m_nCompressionQuality = ((CUImagerApp*)::AfxGetApp())->m_nPdfScanCompressionQuality;
	dlg.m_sPdfScanPaperSize = ((CUImagerApp*)::AfxGetApp())->m_sPdfScanPaperSize;
	if (dlg.DoModal() == IDOK)
	{
		// Set Scan & Email Flag
		m_bScanAndEmail = TRUE;
		
		// Set Pdf Vars
		((CUImagerApp*)::AfxGetApp())->m_nPdfScanCompressionQuality = dlg.m_nCompressionQuality;
		((CUImagerApp*)::AfxGetApp())->m_sPdfScanPaperSize = dlg.m_sPdfScanPaperSize;
		if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		{
			((CUImagerApp*)::AfxGetApp())->WriteProfileInt(	_T("GeneralApp"),
															_T("PdfScanCompressionQuality"),
															((CUImagerApp*)::AfxGetApp())->m_nPdfScanCompressionQuality);
			((CUImagerApp*)::AfxGetApp())->WriteProfileString(_T("GeneralApp"),
															_T("PdfScanPaperSize"),
															((CUImagerApp*)::AfxGetApp())->m_sPdfScanPaperSize);
		}
		
		// Acquire
		TwainAcquire(TWCPP_ANYCOUNT);
	}
	else
	{
		((CUImagerApp*)::AfxGetApp())->m_sScanToTiffFileName = _T("");
		((CUImagerApp*)::AfxGetApp())->m_sScanToPdfFileName = _T("");
	}
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

void CMainFrame::OnFileLockTwainclose() 
{
	m_bTwainCloseLocked = !m_bTwainCloseLocked;
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		((CUImagerApp*)::AfxGetApp())->WriteProfileInt(_T("GeneralApp"), _T("TwainCloseLocked"), m_bTwainCloseLocked);
}

void CMainFrame::OnUpdateFileLockTwainclose(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bTwainCloseLocked ? 1 : 0);
}

void CMainFrame::EnumGdiObjectsDlg() 
{
	// Close Enum GDI Dialog if already Open
	if (m_pEnumGDIObjectsDlg)
	{
		// m_pEnumGDIObjectsDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		m_pEnumGDIObjectsDlg->Close();
	}
	else
	{
		// Show Enum GDI Dialog
		m_pEnumGDIObjectsDlg = new CEnumGDIObjectsDlg(this);
		m_pEnumGDIObjectsDlg->ShowWindow(SW_RESTORE);
	}
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
#if _MFC_VER < 0x0700
		CView* pView = GetActiveView();
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
#else
		// With vs2003 and vs2005 GetActiveView() returns NULL
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
#endif
	}
	// To Remove Focus From CDialogBar's Edit Box in Print Preview Mode
	else if (	pMsg->message == WM_LBUTTONDBLCLK ||
				pMsg->message == WM_LBUTTONDOWN)
	{
#if _MFC_VER < 0x0700
		CView* pView = GetActiveView();
		if (pView && pView->IsKindOf(RUNTIME_CLASS(CPicturePrintPreviewView)))
			pView->SetFocus();
#else
		// With vs2003 and vs2005 GetActiveView() returns NULL
		CMDIChildWnd* pChild = MDIGetActive();
		if (pChild)
		{
			CView* pView = pChild->GetActiveView();
			if (pView && pView->IsKindOf(RUNTIME_CLASS(CPicturePrintPreviewView)))
				pView->SetFocus();
		}
#endif
	}
#if _MFC_VER < 0x0700
	// No menu in print preview mode -> we do not need menu activation keys
	// otherwise the pointer changes from hand to arrow!
	else if (pMsg->message == WM_SYSKEYDOWN)
	{
		if ((pMsg->wParam ==  VK_MENU		||	// ALT
			pMsg->wParam ==  VK_F10)		&&	// F10
			((CUImagerApp*)::AfxGetApp())->HasPicturePrintPreview())
			return TRUE;
	}
#endif
	// Note: with new MFC in full-screen print preview mode the menu
	// activation keys are de-activate automatically.
	else if (pMsg->message == WM_KEYDOWN)
	{
		// App Exit Pressing ESC
		if (pMsg->wParam ==  VK_ESCAPE)
		{
			if (!((CUImagerApp*)::AfxGetApp())->AreDocsOpen())
				PostMessage(WM_CLOSE, 0, 0);
		}

		// Secret Code
		#define MAX_CODE_LENGTH		13
		if (pMsg->wParam == VK_CONTROL)
			m_sSecretCodeSequence = _T(":");
		else if (m_sSecretCodeSequence.GetLength() > MAX_CODE_LENGTH)
			m_sSecretCodeSequence = _T("");
		else if (m_sSecretCodeSequence.GetLength() >= 1)
			m_sSecretCodeSequence += CString((char)pMsg->wParam);
		if (m_sSecretCodeSequence.CompareNoCase(_T(":gdidump")) == 0)	// Working Only For Win9x!
		{
			if (!g_bNT && !m_pEnumGDIObjectsDlg)
				EnumGdiObjectsDlg();
		}
		else if (m_sSecretCodeSequence.CompareNoCase(_T(":cpucount")) == 0)
		{
			::DisplayCpuCount();
		}
		else if (m_sSecretCodeSequence.CompareNoCase(_T(":aviinfo")) == 0)
		{
			((CUImagerApp*)::AfxGetApp())->m_bVideoAviInfo = !((CUImagerApp*)::AfxGetApp())->m_bVideoAviInfo;
			CUImagerMultiDocTemplate* pVideoAviDocTemplate = ((CUImagerApp*)::AfxGetApp())->GetVideoAviDocTemplate();
			POSITION posVideoAviDoc = pVideoAviDocTemplate->GetFirstDocPosition();
			while (posVideoAviDoc)
			{
				CVideoAviDoc* pVideoAviDoc = (CVideoAviDoc*)(pVideoAviDocTemplate->GetNextDoc(posVideoAviDoc));
				if (pVideoAviDoc)
					pVideoAviDoc->GetView()->Invalidate(FALSE);
			}
			if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
				((CUImagerApp*)::AfxGetApp())->WriteProfileInt(_T("GeneralApp"), _T("VideoAviInfo"), ((CUImagerApp*)::AfxGetApp())->m_bVideoAviInfo);
		}
		else if (m_sSecretCodeSequence.CompareNoCase(_T(":coldet")) == 0)
		{
			((CUImagerApp*)::AfxGetApp())->m_bColDet = !((CUImagerApp*)::AfxGetApp())->m_bColDet;
			if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
				((CUImagerApp*)::AfxGetApp())->WriteProfileInt(_T("GeneralApp"), _T("ColDet"), ((CUImagerApp*)::AfxGetApp())->m_bColDet);
		}
		else if (m_sSecretCodeSequence.CompareNoCase(_T(":osver")) == 0)
		{
			OSVERSIONINFO ovi = {0};
			ovi.dwOSVersionInfoSize = sizeof ovi;
			::GetVersionEx(&ovi);
			CString sVer;
			if (g_bReactOS)
			{
				sVer.Format(_T("ReactOS emulation of: PlatformId=%u , Maj=%u , Min=%u , Build=%u\n%s"),
												ovi.dwPlatformId,
												ovi.dwMajorVersion, 
												ovi.dwMinorVersion,
												ovi.dwBuildNumber,
												ovi.szCSDVersion);
			}
			else
			{
				sVer.Format(_T("PlatformId=%u , Maj=%u , Min=%u , Build=%u\n%s"),
												ovi.dwPlatformId,
												ovi.dwMajorVersion, 
												ovi.dwMinorVersion,
												ovi.dwBuildNumber,
												ovi.szCSDVersion);
			}
			::AfxMessageBox(sVer);
		}
	}
	
	return CMDIFrameWnd::PreTranslateMessage(pMsg);
}

LONG CMainFrame::OnThreadSafeStatusText(WPARAM wparam, LPARAM lparam)
{
	CString* pStatusText = (CString*)wparam;
	if (pStatusText)
	{
		StatusText(*pStatusText);
		delete pStatusText;
		return 1;
	}
	else
		return 0;
}

LONG CMainFrame::OnThreadSafeOpenDoc(WPARAM wparam, LPARAM lparam)
{
	CString* pFileName = (CString*)wparam;
#if defined (SUPPORT_LIBAVCODEC) && defined (VIDEODEVICEDOC)
	CPostRecParams* pPostRecParams = (CPostRecParams*)lparam;
#endif
	if (pFileName)
	{
		CDocument* pDoc = ((CUImagerApp*)::AfxGetApp())->OpenDocumentFile(*pFileName);
		delete pFileName;
		if (pDoc)
		{
#if defined (SUPPORT_LIBAVCODEC) && defined (VIDEODEVICEDOC)
			if (pDoc->IsKindOf(RUNTIME_CLASS(CVideoAviDoc)))
			{
				if (pPostRecParams)
				{
					((CVideoAviDoc*)pDoc)->PostRecProcessing(pPostRecParams->m_sSaveFileName,
															pPostRecParams->m_dwVideoCompressorFourCC,
															pPostRecParams->m_nVideoCompressorDataRate,
															pPostRecParams->m_nVideoCompressorKeyframesRate,
															pPostRecParams->m_fVideoCompressorQuality,
															pPostRecParams->m_nVideoCompressorQualityBitrate,
															pPostRecParams->m_bDeinterlace,
															pPostRecParams->m_bCloseWhenDone);
				}
			}
			if (pPostRecParams)
				delete pPostRecParams;
#endif
			return 1;
		}
		else
		{
#if defined (SUPPORT_LIBAVCODEC) && defined (VIDEODEVICEDOC)
			if (pPostRecParams)
				delete pPostRecParams;
#endif
			return 0;
		}
	}
	else
	{
#if defined (SUPPORT_LIBAVCODEC) && defined (VIDEODEVICEDOC)
		if (pPostRecParams)
			delete pPostRecParams;
#endif
		return 0;
	}
}

#ifdef VIDEODEVICEDOC
LONG CMainFrame::OnAutorunVideoDevices(WPARAM wparam, LPARAM lparam)
{
	((CUImagerApp*)::AfxGetApp())->AutorunVideoDevices((int)wparam);
	return 0;
}
#endif

LONG CMainFrame::OnThreadSafeAviStartShrinkTo(WPARAM wparam, LPARAM lparam)
{
	BOOL res = FALSE;
	CString* pSrcFileName = (CString*)wparam;
	CString* pDstFileName = (CString*)lparam;
	if (pSrcFileName && pDstFileName)
	{
		CVideoAviDoc* pDoc = (CVideoAviDoc*)((CUImagerApp*)::AfxGetApp())->OpenDocumentFile(*pSrcFileName);
		if (pDoc && pDoc->StartShrinkDocTo(*pDstFileName))
			res = TRUE;
	}

	if (pSrcFileName)
		delete pSrcFileName;

	if (pDstFileName)
		delete pDstFileName;

	return res;
}				

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

void CMainFrame::EnterExitFullscreen(BOOL bAdaptResolution/*=FALSE*/)
{
	// Available only if there is an active doc
	CMDIChildWnd* pChild = MDIGetActive();
	if (!pChild)
		return;

	// View
	CUImagerView* pView = (CUImagerView*)pChild->GetActiveView();
	ASSERT_VALID(pView);

	if (pView->IsKindOf(RUNTIME_CLASS(CPictureView)))
	{
		// Doc
		CPictureDoc* pDoc = ((CPictureView*)pView)->GetDocument();
		ASSERT_VALID(pDoc);

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
			FullScreenModeOn(bAdaptResolution);

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
			FullScreenModeOn(bAdaptResolution);
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

	// View
	CUImagerView* pView = (CUImagerView*)pChild->GetActiveView();
	ASSERT_VALID(pView);

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

void CMainFrame::FullScreenModeOn(BOOL bAdaptResolution/*=FALSE*/,
								  BOOL bSafePaused/*=FALSE*/)
{
	// Available only if there is an active doc
	CMDIChildWnd* pChild = MDIGetActive();
	if (!pChild)
		return;

	// View
	CUImagerView* pView = (CUImagerView*)pChild->GetActiveView();
	ASSERT_VALID(pView);

	// Doc
	CUImagerDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->IsKindOf(RUNTIME_CLASS(CVideoAviDoc)))
	{
		// Safe Pause & Do Not Draw
		if (!bSafePaused)
		{
			((CVideoAviDoc*)pDoc)->m_PlayVideoFileThread.SafePauseDelayedRestart(	GetSafeHwnd(),
																					WM_VIDEOAVI_FULLSCREEN_MODE_ON,
																					(WPARAM)bAdaptResolution,
																					(LPARAM)0,
																					(((CVideoAviDoc*)pDoc)->GetPlayFrameRate() > 0.0) ?
																					Round(FULLSCREENON_SAFEPAUSED_FRAMES_TIMEOUT * 1000.0 /
																					((CVideoAviDoc*)pDoc)->GetPlayFrameRate()) :
																					300,
																					FULLSCREENON_DELAYEDRESTART_TIMEOUT,
																					FALSE);
			((CVideoAviDoc*)pDoc)->m_bNoDrawing = TRUE;
			return;
		}

		// No Flip (do Blt) if some modeless dialogs are visible
		if (AreModelessDlgsVisible())
			((CVideoAviDoc*)pDoc)->m_PlayVideoFileThread.SetFullScreenBlt();
	}

	// Store placement
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		((CUImagerApp*)::AfxGetApp())->SaveSettings();

	// Get Current Monitor Rectangle
	m_rcEnterFullScreenMonitor = GetMonitorFullRect();

	// Disable Screensaver If It Is Enabled
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
	m_bToolBarWasVisible = (m_wndToolBar.IsWindowVisible() != 0);
	m_wndToolBar.ShowWindow(SW_HIDE);
	CToolBar* pChildToolBar = ((CToolBarChildFrame*)pChild)->GetToolBar(); 
	if (pChildToolBar)
	{
		m_bChildToolBarWasVisible = (pChildToolBar->IsWindowVisible() != 0);
		pChildToolBar->ShowWindow(SW_HIDE);
	}
	m_bStatusBarWasVisible = (m_wndStatusBar.IsWindowVisible() != 0);
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

	// Full Screen Mode Flag
	m_bFullScreenMode = true;

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
	// and call ResetFullScreenBlt() for CVideoAviDoc
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
		if (!pVideoStream)
		{
			((CVideoAviDoc*)pDoc)->m_bNoDrawing = FALSE;
			return;
		}

		// Enter CS
		if (((CVideoAviDoc*)pDoc)->m_DxDraw.HasDxDraw())
			((CVideoAviDoc*)pDoc)->m_DxDraw.EnterCS();

		// Initialize the DirectDraw Interface
		if (((CVideoAviDoc*)pDoc)->m_DxDraw.IsInit() &&
			((CVideoAviDoc*)pDoc)->m_bUseDxDraw)
		{
			// Init DxDraw
			((CVideoAviDoc*)pDoc)->m_DxDraw.InitFullScreen(	GetSafeHwnd(),
															((CVideoAviDoc*)pDoc)->m_DocRect.right,
															((CVideoAviDoc*)pDoc)->m_DocRect.bottom,
															bAdaptResolution,
															pVideoStream->GetFourCC(false),
															IDB_TELETEXT_DH_26);

			// Update Because of Possible Resolution Change
			if (bAdaptResolution)
			{
				rcMonitor = GetMonitorFullRect();
				nMonitorWidth = rcMonitor.right - rcMonitor.left;
				nMonitorHeight = rcMonitor.bottom - rcMonitor.top;
				pView->SetWindowPos(NULL, -2, -2, nMonitorWidth + 4, nMonitorHeight + 4, SWP_NOZORDER);
			}

			// Calc. m_ZoomRect
			pView->UpdateZoomRect();

			// Set User Zoom Rect to Doc Rectangle
			// if DirectX is in Emulated mode (Device 0)
			// because stretch is really slow in this mode!
			if (bAdaptResolution &&
				(((CVideoAviDoc*)pDoc)->m_DxDraw.GetCurrentDevice() == 0))
			{
				// Center User Zoom Rect in Screen
				((CVideoAviDoc*)pDoc)->GetView()->m_UserZoomRect = pView->m_ZoomRect;
				int nDeflateX =		(pView->m_ZoomRect.Width()	- pDoc->m_DocRect.Width())	/ 2;
				int nDeflateY =		(pView->m_ZoomRect.Height()	- pDoc->m_DocRect.Height())	/ 2;
				int nRemainderX =	(pView->m_ZoomRect.Width()	- pDoc->m_DocRect.Width())	% 2;
				int nRemainderY =	(pView->m_ZoomRect.Height()	- pDoc->m_DocRect.Height())	% 2;
				((CVideoAviDoc*)pDoc)->GetView()->m_UserZoomRect.DeflateRect(nDeflateX, nDeflateY);
				
				// If Odd Size Video
				if (nRemainderX)
					((CVideoAviDoc*)pDoc)->GetView()->m_UserZoomRect.DeflateRect(0, 0, 1, 0);
				if (nRemainderY)
					((CVideoAviDoc*)pDoc)->GetView()->m_UserZoomRect.DeflateRect(0, 0, 0, 1);

				// Check
				ASSERT(((CVideoAviDoc*)pDoc)->GetView()->m_UserZoomRect.Width() ==
						((CVideoAviDoc*)pDoc)->m_DocRect.Width());
				ASSERT(((CVideoAviDoc*)pDoc)->GetView()->m_UserZoomRect.Height() ==
						((CVideoAviDoc*)pDoc)->m_DocRect.Height());
			}
			else if (((CVideoAviDoc*)pDoc)->GetView()->m_UserZoomRect == CRect(0,0,0,0))
				((CVideoAviDoc*)pDoc)->GetView()->m_UserZoomRect = pView->m_ZoomRect;

			// Leave CS
			((CVideoAviDoc*)pDoc)->m_DxDraw.LeaveCS();
		}
		else
		{
			// Leave CS
			if (((CVideoAviDoc*)pDoc)->m_DxDraw.HasDxDraw())
				((CVideoAviDoc*)pDoc)->m_DxDraw.LeaveCS();

			// Calc. m_ZoomRect
			pView->UpdateZoomRect();

			// Set User Zoom Rect
			if (((CVideoAviDoc*)pDoc)->GetView()->m_UserZoomRect == CRect(0,0,0,0))
				((CVideoAviDoc*)pDoc)->GetView()->m_UserZoomRect = pView->m_ZoomRect;
		}

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
	FullScreenModeOn((BOOL)wparam, TRUE);
	return 1;
}

void CMainFrame::FullScreenModeOff(BOOL bSafePaused/*=FALSE*/)
{
	CUImagerView* pView = NULL;
	CUImagerDoc* pDoc = NULL;

	// Enable Screensaver If It Was Enabled
	if (m_bScreenSaverWasActive)
		::SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, NULL, 0);

	CMDIChildWnd* pChild = MDIGetActive();
	if (pChild)
	{
		// View
		pView = (CUImagerView*)pChild->GetActiveView();
		ASSERT_VALID(pView);

		// Doc
		pDoc = pView->GetDocument();
		ASSERT_VALID(pDoc);

		// Do Not Draw and Restore Display Mode
		if (pDoc->IsKindOf(RUNTIME_CLASS(CVideoAviDoc)))
		{
			if (!bSafePaused)
			{
				((CVideoAviDoc*)pDoc)->m_PlayVideoFileThread.SafePauseDelayedRestart(	GetSafeHwnd(),
																						WM_VIDEOAVI_FULLSCREEN_MODE_OFF,
																						(WPARAM)0,
																						(LPARAM)0,
																						(((CVideoAviDoc*)pDoc)->GetPlayFrameRate() > 0.0) ?
																						Round(FULLSCREENOFF_SAFEPAUSED_FRAMES_TIMEOUT * 1000.0 /
																						((CVideoAviDoc*)pDoc)->GetPlayFrameRate()) :
																						300,
																						FULLSCREENOFF_DELAYEDRESTART_TIMEOUT,
																						FALSE);
				((CVideoAviDoc*)pDoc)->m_bNoDrawing = TRUE;
				return;
			}
			else
			{
				// DirectDraw?
				if (((CVideoAviDoc*)pDoc)->m_DxDraw.HasDxDraw())
				{
					// Enter CS
					((CVideoAviDoc*)pDoc)->m_DxDraw.EnterCS();

					// Restore Display Mode
					if (((CVideoAviDoc*)pDoc)->m_bUseDxDraw &&
						((CVideoAviDoc*)pDoc)->m_DxDraw.IsInit())
						((CVideoAviDoc*)pDoc)->m_DxDraw.RestoreDisplayMode();

					// Leave CS
					((CVideoAviDoc*)pDoc)->m_DxDraw.LeaveCS();
				}
			}
		}

		// Exiting?
		if (((CUImagerApp*)::AfxGetApp())->m_bShuttingDownApplication)
		{
			m_bFullScreenMode = false;
			return;
		}
		
		// Restore starting monitor
		if (m_rcEnterFullScreenMonitor != GetMonitorFullRect())
			FullScreenTo(m_rcEnterFullScreenMonitor);

		// Full Screen Mode Flag
		m_bFullScreenMode = false;

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

	if (pView)
	{
		// Restore View Scroll Position
		if (pView->IsXOrYScroll())
			pView->ScrollToPosition(m_ptChildScrollPosition);

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
			if (!pVideoStream)
			{
				((CVideoAviDoc*)pDoc)->m_bNoDrawing = FALSE;
				return;
			}

			// Enter CS
			if (((CVideoAviDoc*)pDoc)->m_DxDraw.HasDxDraw())
				((CVideoAviDoc*)pDoc)->m_DxDraw.EnterCS();

			// Exit DirectDraw Exclusive FullScreen Mode
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

				// Restart
				((CVideoAviDoc*)pDoc)->m_PlayVideoFileThread.SetSafePauseRestartEvent();
			}
			else
			{
				// Leave CS
				if (((CVideoAviDoc*)pDoc)->m_DxDraw.HasDxDraw())
					((CVideoAviDoc*)pDoc)->m_DxDraw.LeaveCS();

				// Do Draw
				((CVideoAviDoc*)pDoc)->m_bNoDrawing = FALSE;

				// Restart
				((CVideoAviDoc*)pDoc)->m_PlayVideoFileThread.SetSafePauseRestartEvent();
			}

			// Store m_UserZoomRect
			((CVideoAviDoc*)pDoc)->m_PrevUserZoomRect = pView->m_UserZoomRect;
		}

		// Update View
		pView->UpdateWindowSizes(TRUE, TRUE, FALSE);
	}
	else
	{
		// Do Draw
		if (pDoc->IsKindOf(RUNTIME_CLASS(CVideoAviDoc)))
		{
			((CVideoAviDoc*)pDoc)->m_bNoDrawing = FALSE;
			((CVideoAviDoc*)pDoc)->m_PlayVideoFileThread.SetSafePauseRestartEvent();
		}
	}
}

LONG CMainFrame::OnVideoAviFullScreenModeOff(WPARAM wparam, LPARAM lparam)
{
	FullScreenModeOff(TRUE);
	return 1;
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor,LPRECT lprcMonitor, LPARAM dwData)
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
		HINSTANCE h = ::LoadLibrary(_T("user32.dll"));
		if (!h)
			return FALSE;
		FPENUMDISPLAYMONITORS fpEnumDisplayMonitors = (FPENUMDISPLAYMONITORS)::GetProcAddress(h, "EnumDisplayMonitors");
		if (fpEnumDisplayMonitors)
		{
			BOOL res = fpEnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)pMonitors);
			::FreeLibrary(h);
			return res;
		}
		else
		{
			::FreeLibrary(h);
			return FALSE;
		}
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
	HINSTANCE h = ::LoadLibrary(_T("user32.dll"));
	if (!h)
		return NULL;
	FPMONITORFROMWINDOW fpMonitorFromWindow = (FPMONITORFROMWINDOW)::GetProcAddress(h, "MonitorFromWindow");
	if (fpMonitorFromWindow)
	{
		HMONITOR hMonitor = fpMonitorFromWindow(this->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
		::FreeLibrary(h);
		return hMonitor;
	}
	else
	{
		::FreeLibrary(h);
		return NULL;
	}
}

CRect CMainFrame::GetPreviousMonitorFullRect()
{
	CRect rcMonitor;
	FPGETMONITORINFO fpGetMonitorInfo;
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HINSTANCE h = ::LoadLibrary(_T("user32.dll"));
	if (!h)
	{
		rcMonitor.left = 0;
		rcMonitor.top = 0;
		rcMonitor.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcMonitor.bottom = ::GetSystemMetrics(SM_CYSCREEN);
		return rcMonitor;
	}
#ifdef _UNICODE
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoW");
#else
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoA");
#endif
	if (fpGetMonitorInfo)
	{
		HMONITOR hMonitor = GetPreviousMonitor();
		if (hMonitor && fpGetMonitorInfo(hMonitor, &monInfo))
		{
			::FreeLibrary(h);
			rcMonitor = monInfo.rcMonitor;
		}
		else
		{
			::FreeLibrary(h);
			return CRect(0, 0, 0, 0);
		}
	}
	else
	{
		::FreeLibrary(h);
		rcMonitor.left = 0;
		rcMonitor.top = 0;
		rcMonitor.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcMonitor.bottom = ::GetSystemMetrics(SM_CYSCREEN);
	}

	return rcMonitor;
}

CRect CMainFrame::GetNextMonitorFullRect()
{
	CRect rcMonitor;
	FPGETMONITORINFO fpGetMonitorInfo;
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HINSTANCE h = ::LoadLibrary(_T("user32.dll"));
	if (!h)
	{
		rcMonitor.left = 0;
		rcMonitor.top = 0;
		rcMonitor.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcMonitor.bottom = ::GetSystemMetrics(SM_CYSCREEN);
		return rcMonitor;
	}
#ifdef _UNICODE
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoW");
#else
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoA");
#endif
	if (fpGetMonitorInfo)
	{
		HMONITOR hMonitor = GetNextMonitor();
		if (hMonitor && fpGetMonitorInfo(hMonitor, &monInfo))
		{
			::FreeLibrary(h);
			rcMonitor = monInfo.rcMonitor;
		}
		else
		{
			::FreeLibrary(h);
			return CRect(0, 0, 0, 0);
		}
	}
	else
	{
		::FreeLibrary(h);
		rcMonitor.left = 0;
		rcMonitor.top = 0;
		rcMonitor.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcMonitor.bottom = ::GetSystemMetrics(SM_CYSCREEN);
	}

	return rcMonitor;
}

CSize CMainFrame::GetMonitorSize(CWnd* pWnd/*=NULL*/)
{
	int nMonitorWidth, nMonitorHeight;
	FPMONITORFROMWINDOW fpMonitorFromWindow;
	FPGETMONITORINFO	fpGetMonitorInfo;
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HINSTANCE h = ::LoadLibrary(_T("user32.dll"));
	if (!h)
	{
		nMonitorWidth = ::GetSystemMetrics(SM_CXSCREEN);
		nMonitorHeight = ::GetSystemMetrics(SM_CYSCREEN);
		return CSize(nMonitorWidth, nMonitorHeight);
	}
	fpMonitorFromWindow = (FPMONITORFROMWINDOW)::GetProcAddress(h, "MonitorFromWindow");
#ifdef _UNICODE
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoW");
#else
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoA");
#endif
	if (fpMonitorFromWindow && fpGetMonitorInfo)
	{
		HMONITOR hMonitor;
		if (pWnd)
			hMonitor = fpMonitorFromWindow(pWnd->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
		else
			hMonitor = fpMonitorFromWindow(this->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
		if (!fpGetMonitorInfo(hMonitor, &monInfo))
		{
			::FreeLibrary(h);
			return CSize(0, 0);
		}
		::FreeLibrary(h);
		nMonitorWidth = monInfo.rcMonitor.right - monInfo.rcMonitor.left;
		nMonitorHeight = monInfo.rcMonitor.bottom - monInfo.rcMonitor.top;
	}
	else
	{
		::FreeLibrary(h);
		nMonitorWidth = ::GetSystemMetrics(SM_CXSCREEN);
		nMonitorHeight = ::GetSystemMetrics(SM_CYSCREEN);
	}

	return CSize(nMonitorWidth, nMonitorHeight);
}

CSize CMainFrame::GetMonitorSize(CPoint pt)
{
	int nMonitorWidth, nMonitorHeight;
	FPMONITORFROMPOINT fpMonitorFromPoint;
	FPGETMONITORINFO	fpGetMonitorInfo;
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HINSTANCE h = ::LoadLibrary(_T("user32.dll"));
	if (!h)
	{
		nMonitorWidth = ::GetSystemMetrics(SM_CXSCREEN);
		nMonitorHeight = ::GetSystemMetrics(SM_CYSCREEN);
		return CSize(nMonitorWidth, nMonitorHeight);
	}
	fpMonitorFromPoint = (FPMONITORFROMPOINT)::GetProcAddress(h, "MonitorFromPoint");
#ifdef _UNICODE
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoW");
#else
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoA");
#endif
	if (fpMonitorFromPoint && fpGetMonitorInfo)
	{
		HMONITOR hMonitor = fpMonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		if (!fpGetMonitorInfo(hMonitor, &monInfo))
		{
			::FreeLibrary(h);
			return CSize(0, 0);
		}
		::FreeLibrary(h);
		nMonitorWidth = monInfo.rcMonitor.right - monInfo.rcMonitor.left;
		nMonitorHeight = monInfo.rcMonitor.bottom - monInfo.rcMonitor.top;
	}
	else
	{
		::FreeLibrary(h);
		nMonitorWidth = ::GetSystemMetrics(SM_CXSCREEN);
		nMonitorHeight = ::GetSystemMetrics(SM_CYSCREEN);
	}

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
	CRect rcWork;
	FPMONITORFROMWINDOW fpMonitorFromWindow;
	FPGETMONITORINFO	fpGetMonitorInfo;
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HINSTANCE h = ::LoadLibrary(_T("user32.dll"));
	if (!h)
	{
		rcWork.left = 0;
		rcWork.top = 0;
		rcWork.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcWork.bottom = ::GetSystemMetrics(SM_CYSCREEN);
		return rcWork;
	}
	fpMonitorFromWindow = (FPMONITORFROMWINDOW)::GetProcAddress(h, "MonitorFromWindow");
#ifdef _UNICODE
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoW");
#else
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoA");
#endif
	if (fpMonitorFromWindow && fpGetMonitorInfo)
	{
		HMONITOR hPrimaryMonitor;
		hPrimaryMonitor = fpMonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
		if (!fpGetMonitorInfo(hPrimaryMonitor, &monInfo))
		{
			::FreeLibrary(h);
			return CRect(0, 0, 0, 0);
		}
		::FreeLibrary(h);
		rcWork = monInfo.rcWork;
	}
	else
	{
		::FreeLibrary(h);
		rcWork.left = 0;
		rcWork.top = 0;
		rcWork.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcWork.bottom = ::GetSystemMetrics(SM_CYSCREEN);
	}

	return rcWork;
}

CRect CMainFrame::GetMonitorWorkRect(CWnd* pWnd/*=NULL*/)
{
	CRect rcWork;
	FPMONITORFROMWINDOW fpMonitorFromWindow;
	FPGETMONITORINFO	fpGetMonitorInfo;
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HINSTANCE h = ::LoadLibrary(_T("user32.dll"));
	if (!h)
	{
		rcWork.left = 0;
		rcWork.top = 0;
		rcWork.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcWork.bottom = ::GetSystemMetrics(SM_CYSCREEN);
		return rcWork;
	}
	fpMonitorFromWindow = (FPMONITORFROMWINDOW)::GetProcAddress(h, "MonitorFromWindow");
#ifdef _UNICODE
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoW");
#else
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoA");
#endif
	if (fpMonitorFromWindow && fpGetMonitorInfo)
	{
		HMONITOR hMonitor;
		if (pWnd)
			hMonitor = fpMonitorFromWindow(pWnd->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
		else
			hMonitor = fpMonitorFromWindow(this->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
		if (!fpGetMonitorInfo(hMonitor, &monInfo))
		{
			::FreeLibrary(h);
			return CRect(0, 0, 0, 0);
		}
		::FreeLibrary(h);
		rcWork = monInfo.rcWork;
	}
	else
	{
		::FreeLibrary(h);
		rcWork.left = 0;
		rcWork.top = 0;
		rcWork.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcWork.bottom = ::GetSystemMetrics(SM_CYSCREEN);
	}

	return rcWork;
}

CRect CMainFrame::GetMonitorWorkRect(CPoint pt)
{
	CRect rcWork;
	FPMONITORFROMPOINT fpMonitorFromPoint;
	FPGETMONITORINFO	fpGetMonitorInfo;
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HINSTANCE h = ::LoadLibrary(_T("user32.dll"));
	if (!h)
	{
		rcWork.left = 0;
		rcWork.top = 0;
		rcWork.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcWork.bottom = ::GetSystemMetrics(SM_CYSCREEN);
		return rcWork;
	}
	fpMonitorFromPoint = (FPMONITORFROMPOINT)::GetProcAddress(h, "MonitorFromPoint");
#ifdef _UNICODE
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoW");
#else
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoA");
#endif
	if (fpMonitorFromPoint && fpGetMonitorInfo)
	{
		HMONITOR hMonitor = fpMonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		if (!fpGetMonitorInfo(hMonitor, &monInfo))
		{
			::FreeLibrary(h);
			return CRect(0, 0, 0, 0);
		}
		::FreeLibrary(h);
		rcWork = monInfo.rcWork;
	}
	else
	{
		::FreeLibrary(h);
		rcWork.left = 0;
		rcWork.top = 0;
		rcWork.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcWork.bottom = ::GetSystemMetrics(SM_CYSCREEN);
	}

	return rcWork;
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
	CRect rcMonitor;
	FPMONITORFROMWINDOW fpMonitorFromWindow;
	FPGETMONITORINFO	fpGetMonitorInfo;
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HINSTANCE h = ::LoadLibrary(_T("user32.dll"));
	if (!h)
	{
		rcMonitor.left = 0;
		rcMonitor.top = 0;
		rcMonitor.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcMonitor.bottom = ::GetSystemMetrics(SM_CYSCREEN);
		return rcMonitor;
	}
	fpMonitorFromWindow = (FPMONITORFROMWINDOW)::GetProcAddress(h, "MonitorFromWindow");
#ifdef _UNICODE
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoW");
#else
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoA");
#endif
	if (fpMonitorFromWindow && fpGetMonitorInfo)
	{
		HMONITOR hPrimaryMonitor;
		hPrimaryMonitor = fpMonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
		if (!fpGetMonitorInfo(hPrimaryMonitor, &monInfo))
		{
			::FreeLibrary(h);
			return CRect(0, 0, 0, 0);
		}
		::FreeLibrary(h);
		rcMonitor = monInfo.rcMonitor;
	}
	else
	{
		::FreeLibrary(h);
		rcMonitor.left = 0;
		rcMonitor.top = 0;
		rcMonitor.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcMonitor.bottom = ::GetSystemMetrics(SM_CYSCREEN);
	}

	return rcMonitor;
}

CRect CMainFrame::GetMonitorFullRect(CWnd* pWnd/*=NULL*/)
{
	CRect rcMonitor;
	FPMONITORFROMWINDOW fpMonitorFromWindow;
	FPGETMONITORINFO	fpGetMonitorInfo;
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HINSTANCE h = ::LoadLibrary(_T("user32.dll"));
	if (!h)
	{
		rcMonitor.left = 0;
		rcMonitor.top = 0;
		rcMonitor.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcMonitor.bottom = ::GetSystemMetrics(SM_CYSCREEN);
		return rcMonitor;
	}
	fpMonitorFromWindow = (FPMONITORFROMWINDOW)::GetProcAddress(h, "MonitorFromWindow");
#ifdef _UNICODE
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoW");
#else
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoA");
#endif
	if (fpMonitorFromWindow && fpGetMonitorInfo)
	{
		HMONITOR hMonitor;
		if (pWnd)
			hMonitor = fpMonitorFromWindow(pWnd->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
		else
			hMonitor = fpMonitorFromWindow(this->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
		if (!fpGetMonitorInfo(hMonitor, &monInfo))
		{
			::FreeLibrary(h);
			return CRect(0, 0, 0, 0);
		}
		::FreeLibrary(h);
		rcMonitor = monInfo.rcMonitor;
	}
	else
	{
		::FreeLibrary(h);
		rcMonitor.left = 0;
		rcMonitor.top = 0;
		rcMonitor.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcMonitor.bottom = ::GetSystemMetrics(SM_CYSCREEN);
	}

	return rcMonitor;
}

CRect CMainFrame::GetMonitorFullRect(CPoint pt)
{
	CRect rcMonitor;
	FPMONITORFROMPOINT fpMonitorFromPoint;
	FPGETMONITORINFO	fpGetMonitorInfo;
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	HINSTANCE h = ::LoadLibrary(_T("user32.dll"));
	if (!h)
	{
		rcMonitor.left = 0;
		rcMonitor.top = 0;
		rcMonitor.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcMonitor.bottom = ::GetSystemMetrics(SM_CYSCREEN);
		return rcMonitor;
	}
	fpMonitorFromPoint = (FPMONITORFROMPOINT)::GetProcAddress(h, "MonitorFromPoint");
#ifdef _UNICODE
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoW");
#else
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoA");
#endif
	if (fpMonitorFromPoint && fpGetMonitorInfo)
	{
		HMONITOR hMonitor = fpMonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		if (!fpGetMonitorInfo(hMonitor, &monInfo))
		{
			::FreeLibrary(h);
			return CRect(0, 0, 0, 0);
		}
		::FreeLibrary(h);
		rcMonitor = monInfo.rcMonitor;
	}
	else
	{
		::FreeLibrary(h);
		rcMonitor.left = 0;
		rcMonitor.top = 0;
		rcMonitor.right = ::GetSystemMetrics(SM_CXSCREEN);
		rcMonitor.bottom = ::GetSystemMetrics(SM_CYSCREEN);
	}

	return rcMonitor;
}

// Remember to delete it with ::DeleteDC() !!!
HDC CMainFrame::CreateMonitorDC(CWnd* pWnd/*=NULL*/)
{
	HDC hMonitorDC;
	FPMONITORFROMWINDOW fpMonitorFromWindow;
	FPGETMONITORINFO	fpGetMonitorInfo;
	MONITORINFOEX monInfo;
	monInfo.cbSize = sizeof(MONITORINFOEX);
	HINSTANCE h = ::LoadLibrary(_T("user32.dll"));
	if (!h)
	{
		hMonitorDC = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
		return hMonitorDC;
	}
	fpMonitorFromWindow = (FPMONITORFROMWINDOW)::GetProcAddress(h, "MonitorFromWindow");
#ifdef _UNICODE
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoW");
#else
	fpGetMonitorInfo = (FPGETMONITORINFO)::GetProcAddress(h, "GetMonitorInfoA");
#endif
	if (fpMonitorFromWindow && fpGetMonitorInfo)
	{
		HMONITOR hMonitor;
		if (pWnd)
			hMonitor = fpMonitorFromWindow(pWnd->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
		else
			hMonitor = fpMonitorFromWindow(this->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
		if (!fpGetMonitorInfo(hMonitor, &monInfo))
		{
			::FreeLibrary(h);
			return NULL;
		}
		::FreeLibrary(h);
		if (g_bNT)
			hMonitorDC = ::CreateDC(_T("DISPLAY"), monInfo.szDevice, NULL, NULL);
		else
			hMonitorDC = ::CreateDC(NULL, monInfo.szDevice, NULL, NULL);
	}
	else
	{
		::FreeLibrary(h);
		hMonitorDC = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	}

	return hMonitorDC;
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
#endif
#ifndef VIDEODEVICEDOC
			if (nCount == 5)
				pMenu->DeleteMenu(m_nToolsMenuPos, MF_BYPOSITION);
			m_nToolsMenuPos = -2;
			m_nHelpMenuPos--;
#endif
		}
		else if (pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)))
		{
			// Do not use IsKindOf(RUNTIME_CLASS(CPictureChildFrame)) because
			// CBigPictureChildFrame inherits from CPictureChildFrame this means that
			// IsKindOf(RUNTIME_CLASS(CPictureChildFrame)) would always be true. 
			// Do not use m_bBigPicture because it is not initialized yet!
			if (((CPictureDoc*)pDoc)->GetFrame()->IsKindOf(RUNTIME_CLASS(CBigPictureChildFrame)))
			{
				((CPictureDoc*)pDoc)->m_nFileMenuPos = 0;
				((CPictureDoc*)pDoc)->m_nEditMenuPos = 1;
				((CPictureDoc*)pDoc)->m_nViewMenuPos = 2;
				((CPictureDoc*)pDoc)->m_nCaptureMenuPos = 3;
				((CPictureDoc*)pDoc)->m_nToolsMenuPos = 4;
				((CPictureDoc*)pDoc)->m_nWindowsPos = 5;
				((CPictureDoc*)pDoc)->m_nHelpMenuPos = 6;
#ifndef VIDEODEVICEDOC
				if (nCount == 7) // On some OSs menus are re-used from one doc opening to the next!
					pMenu->DeleteMenu(((CPictureDoc*)pDoc)->m_nCaptureMenuPos, MF_BYPOSITION);
				((CPictureDoc*)pDoc)->m_nCaptureMenuPos = -2;
				((CPictureDoc*)pDoc)->m_nToolsMenuPos--;
				((CPictureDoc*)pDoc)->m_nWindowsPos--;
				((CPictureDoc*)pDoc)->m_nHelpMenuPos--;
#endif
#ifndef VIDEODEVICEDOC
				if (nCount == 7) // On some OSs menus are re-used from one doc opening to the next!
					pMenu->DeleteMenu(((CPictureDoc*)pDoc)->m_nToolsMenuPos, MF_BYPOSITION);
				((CPictureDoc*)pDoc)->m_nToolsMenuPos = -2;
				((CPictureDoc*)pDoc)->m_nWindowsPos--;
				((CPictureDoc*)pDoc)->m_nHelpMenuPos--;
#endif
			}
			else
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
#endif
#ifndef VIDEODEVICEDOC
				if (nCount == 8) // On some OSs menus are re-used from one doc opening to the next!
					pMenu->DeleteMenu(((CPictureDoc*)pDoc)->m_nToolsMenuPos, MF_BYPOSITION);
				((CPictureDoc*)pDoc)->m_nToolsMenuPos = -2;
				((CPictureDoc*)pDoc)->m_nWindowsPos--;
				((CPictureDoc*)pDoc)->m_nHelpMenuPos--;
#endif
			}
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
#ifndef SUPPORT_LIBAVCODEC
			if (nCount == 8) // On some OSs menus are re-used from one doc opening to the next!
			{
				CMenu* pPlayMenu = pMenu->GetSubMenu(((CVideoAviDoc*)pDoc)->m_nPlayMenuPos);
				if (pPlayMenu && (int)pPlayMenu->GetMenuItemCount() == 12)
				{
					// Delete the two Codec Priority Entries & Separator
					for (int j = 0 ; j < 3 ; j++)
					{
						pPlayMenu->DeleteMenu(	(int)pPlayMenu->GetMenuItemCount() - 1,
												MF_BYPOSITION);
					}
				}
			}
#endif
#ifndef VIDEODEVICEDOC
			if (nCount == 8) // On some OSs menus are re-used from one doc opening to the next!
				pMenu->DeleteMenu(((CVideoAviDoc*)pDoc)->m_nCaptureMenuPos, MF_BYPOSITION);
			((CVideoAviDoc*)pDoc)->m_nCaptureMenuPos = -2;
			((CVideoAviDoc*)pDoc)->m_nPlayMenuPos--;
			((CVideoAviDoc*)pDoc)->m_nToolsMenuPos--;
			((CVideoAviDoc*)pDoc)->m_nWindowsPos--;
			((CVideoAviDoc*)pDoc)->m_nHelpMenuPos--;
#endif
#ifndef VIDEODEVICEDOC
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
#ifndef VIDEODEVICEDOC
			if (nCount == 7) // On some OSs menus are re-used from one doc opening to the next!
				pMenu->DeleteMenu(((CVideoDeviceDoc*)pDoc)->m_nToolsMenuPos, MF_BYPOSITION);
			((CVideoDeviceDoc*)pDoc)->m_nToolsMenuPos = -2;
			((CVideoDeviceDoc*)pDoc)->m_nWindowsPos--;
			((CVideoDeviceDoc*)pDoc)->m_nHelpMenuPos--;
#endif
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
#endif
#ifndef VIDEODEVICEDOC
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
#endif
#ifndef VIDEODEVICEDOC
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

				// Is it a Picture Doc (not a Big Picture Doc!) ?
				//
				// Note: do not use IsKindOf(RUNTIME_CLASS(CPictureChildFrame)) because
				// CBigPictureChildFrame inherits from CPictureChildFrame this means that
				// IsKindOf(RUNTIME_CLASS(CPictureChildFrame)) would always be true. Do not
				// use m_bBigPicture because it may not be initialized yet!
				if (pDoc->IsKindOf(RUNTIME_CLASS(CPictureDoc)) &&
					!((CPictureDoc*)pDoc)->GetFrame()->IsKindOf(RUNTIME_CLASS(CBigPictureChildFrame))) 
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
						if (CPictureDoc::IsJPEG(((CPictureDoc*)pDoc)->m_sFileName))
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
					pPopupMenu->InsertMenu(ID_CAPTURE_VIDEO_DEVICE, MF_BYCOMMAND, ID_DIRECTSHOW_VIDEODEV_FIRST + id, sDxDevicesName[id]);
				}
			}
			else
				pPopupMenu->RemoveMenu(ID_DIRECTSHOW_VIDEODEV_FIRST + id, MF_BYCOMMAND);
		}
		else if (id < nDxDevicesCount)
			pPopupMenu->InsertMenu(ID_CAPTURE_VIDEO_DEVICE, MF_BYCOMMAND, ID_DIRECTSHOW_VIDEODEV_FIRST + id, sDxDevicesName[id]);
	}
}
#endif

void CMainFrame::PopulateHelpMenu(CMenu* pPopupMenu)
{
	// Remove Register... and separator menu items
	int i;
#ifndef VIDEODEVICEDOC
	for (i = 0 ; i < (int)pPopupMenu->GetMenuItemCount() ; i++)
	{
		if (pPopupMenu->GetMenuItemID(i) == ID_APP_REGISTRATION)
		{
			pPopupMenu->DeleteMenu(i, MF_BYPOSITION);
			pPopupMenu->DeleteMenu(i, MF_BYPOSITION);
			break;
		}
	}
#endif

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
	FileFind.AddAllowedExtension(_T(".htm"));
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
	else if (nIDEvent == ID_TIMER_SCHEDULER)
	{
		// Current Time
		CTime timedate = CTime::GetCurrentTime();
		CTime timeonly(2000, 1, 1, timedate.GetHour(), timedate.GetMinute(), timedate.GetSecond());

		// Debugger PostMessage breakpoint?
#ifdef CRACKCHECK
		if (::IsBPXv2((void*)::PostMessage))
		{
			// Throw exception
			int* p = NULL;
			int l = *p;

			// To avoid optimization removal
			::DeleteCriticalSection((LPCRITICAL_SECTION)&l);
		}
#endif

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

			// get message associated with the ID indicated by wParam
            if (wParam == AFX_IDS_IDLEMESSAGE && m_sStatusBarString != _T(""))
				sMsg = m_sStatusBarString;
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
LONG CMainFrame::OnTaskBarButton(WPARAM wparam, LPARAM lparam)
{
	// Only show right-click taskbar context menu
	// if no modal dialog running, if not resizing with FakeThread
	// and not printing with FakeThread (they all disable the MainFrame)
	if (IsWindowEnabled())
	{
		if (m_bFullScreenMode && MDIGetActive() &&
			(CUImagerView*)(MDIGetActive()->GetActiveView()))
			((CUImagerView*)(MDIGetActive()->GetActiveView()))->ForceCursor();

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

		if (m_bFullScreenMode && MDIGetActive() &&
			(CUImagerView*)(MDIGetActive()->GetActiveView()))
			((CUImagerView*)(MDIGetActive()->GetActiveView()))->ForceCursor(FALSE);

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
	CUImagerMultiDocTemplate* pBigPictureDocTemplate = ((CUImagerApp*)::AfxGetApp())->GetBigPictureDocTemplate();
	POSITION posBigPictureDoc = pBigPictureDocTemplate->GetFirstDocPosition();
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
	while (posBigPictureDoc)
	{
		pDoc = (CPictureDoc*)(pPictureDocTemplate->GetNextDoc(posBigPictureDoc));
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

LRESULT CMainFrame::OnCopyData(WPARAM /*wParam*/, LPARAM lParam)
{
	COPYDATASTRUCT* pCDS = reinterpret_cast<COPYDATASTRUCT*>(lParam);
	TCHAR* pszCmdLine = static_cast<TCHAR*>(pCDS->lpData);
	int nShellCommand = (int)pCDS->dwData;
	if (pszCmdLine)
	{
		if (!m_bFullScreenMode
#if _MFC_VER < 0x0700
			// With old MFC we should not open other files when in print preview
			&&
			!((CUImagerApp*)::AfxGetApp())->HasPicturePrintPreview()
#endif
			)
		{
			OnOpenFromTray();
			CDocument* pDoc = ((CUImagerApp*)::AfxGetApp())->OpenDocumentFile(pszCmdLine);
			if (pDoc && nShellCommand == CCommandLineInfo::FilePrint)
				SendMessage(WM_COMMAND, ID_FILE_PRINT_PREVIEW);
		}
	}
	return TRUE;
}
