#include "stdafx.h"
#include "uImager.h"
#include "MainFrm.h"
#include "PictureDoc.h"
#include "PictureView.h"
#include "RedEyeDlg.h"
#include "PicturePrintPreviewView.h"
#include "PrintPreviewScaleEdit.h"
#include "SortableFileFind.h"
#include "HelpersAudio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPictureView, CUImagerView)

BEGIN_MESSAGE_MAP(CPictureView, CUImagerView)
	//{{AFX_MSG_MAP(CPictureView)
	ON_WM_CREATE()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, OnFilePrintDirect)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_COMMAND(ID_FILE_PRINT_SETUP, OnFilePrintSetup)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_VIEW_ZOOM_TOOL, OnViewZoomTool)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM_TOOL, OnUpdateViewZoomTool)
	ON_WM_KEYUP()
	ON_WM_SIZE()
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_DIRECT, OnUpdateFilePrintDirect)
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_COMMAND(ID_VIEW_FULLSCREEN, OnViewFullscreen)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FULLSCREEN, OnUpdateViewFullscreen)
	ON_COMMAND(ID_VIEW_ZOOM_FIT, OnViewZoomFit)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM_FIT, OnUpdateViewZoomFit)
	ON_COMMAND(ID_VIEW_ZOOM_FITBIG, OnViewZoomFitbig)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM_FITBIG, OnUpdateViewZoomFitbig)
	ON_COMMAND(ID_VIEW_NEXT_MONITOR, OnViewNextMonitor)
	ON_COMMAND(ID_VIEW_PREVIOUS_MONITOR, OnViewPreviousMonitor)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
	ON_MESSAGE(WM_THREADSAFE_SLIDESHOW_LOAD_PICTURE, OnThreadSafeSlideshowLoadPicture)
	ON_MESSAGE(WM_THREADSAFE_UPDATEIMAGEINFO, OnThreadUpdateImageInfo)
	ON_MESSAGE(WM_THREADSAFE_PAUSESLIDESHOW, OnThreadSafePauseSlideshow)
	ON_MESSAGE(WM_RECURSIVEFILEFIND_DONE, OnRecursiveFileFindDone)
	ON_MESSAGE(WM_COLOR_PICKED, OnColorPicked)
	ON_MESSAGE(WM_COLOR_PICKER_CLOSED, OnColorPickerClosed)
	ON_MESSAGE(WM_APPCOMMAND, OnApplicationCommand)
	ON_MESSAGE(WM_GESTURE, OnGesture)
END_MESSAGE_MAP()

CPictureView::CPictureView()
{	
	// Transition Drawing
	m_nTransitionStep = 0;
	m_nCurrentTransition = 0;
	m_hTransitionMemDC = NULL;
	m_hTransitionOldBitmap = NULL;

	// Load Full Jpeg Transition Drawing
	m_nLoadFullJpegTransitionStep = 0;
	m_hLoadFullJpegTransitionMemDC = NULL;
	m_hLoadFullJpegTransitionOldBitmap = NULL;

	// Mem DC
	m_hMemDCDibSection = NULL;
	m_hOldMemDCBitmap = NULL;
	m_rcPrevClient = CRect(0,0,0,0);
	m_pDibSectionBits = NULL;

	// Scroll Move Vars
	m_ptStartScrollMovePos = CPoint(0,0);
	m_ptStartScrollMoveClickPos = CPoint(0,0);
	m_bDoScrollMove = FALSE;

	// Crop Vars
	m_nZoomedPixelAlignX = 1;
	m_nZoomedPixelAlignY = 1;
	m_CropZoomRect = CRect(0,0,0,0);
	m_bCropTopLeft = FALSE;
	m_bCropBottomRight = FALSE;
	m_bCropBottomLeft = FALSE;
	m_bCropTopRight = FALSE;
	m_bCropTop = FALSE;
	m_bCropBottom = FALSE;
	m_bCropLeft = FALSE;
	m_bCropRight = FALSE;
	m_bCropCenter = FALSE;
	m_bCropDrag = FALSE;
	m_ptCropClick = CPoint(0,0);
	m_dCropAspectRatio = 1.0;
	m_bCropMaintainAspectRatio = FALSE;
	m_nAspectRatioPos = 0;
	m_bCropMouseCaptured = FALSE;
	m_nAutoScroll = 0;
	m_uiAutoScrollTimerId = 0;
	m_hCropCursor = NULL;

	// Pixel Scale to Print Characters
	m_dXFontPixelScale = 1.0;
	m_dYFontPixelScale = 1.0;

	// Zoom Tool Flag
	m_bOldZoomToolMinus = FALSE;

	// The Print Rectangle to where the Dib has to be stretched
	m_rcDibPrint = CRect(0,0,0,0);

	// Last right-click point
	m_ptLastRightClick = CPoint(0,0);
}

CPictureView::~CPictureView()
{
	FreeCropMemDCDrawing();
}

void CPictureView::FreeCropMemDCDrawing()
{
	// Free Mem DC Drawing
	if (m_hOldMemDCBitmap)
	{
		::SelectObject(m_MemDC.GetSafeHdc(), m_hOldMemDCBitmap);
		m_hOldMemDCBitmap = NULL;
	}
	if (m_hMemDCDibSection)
	{
		::DeleteObject(m_hMemDCDibSection);
		m_hMemDCDibSection = NULL;
	}
	m_MemDC.DeleteDC();
}

int CPictureView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	CToolBarChildFrame* pFrame = (CToolBarChildFrame*)GetParentFrame();
	if ((pFrame != NULL) && (pFrame->IsKindOf(RUNTIME_CLASS(CToolBarChildFrame))))
		pFrame->SetToolBar(&m_PictureToolBar);

	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	typedef BOOL (WINAPI * FPSETGESTURECONFIG)(HWND hwnd, DWORD dwReserved, UINT cIDs, PGESTURECONFIG pGestureConfig, UINT cbSize);
	HINSTANCE h = ::LoadLibraryFromSystem32(_T("user32.dll"));
	if (h)
	{
		FPSETGESTURECONFIG fpSetGestureConfig = (FPSETGESTURECONFIG)::GetProcAddress(h, "SetGestureConfig");
		if (fpSetGestureConfig)
		{
			DWORD dwPanWant  = GC_PAN | GC_PAN_WITH_INERTIA | GC_PAN_WITH_SINGLE_FINGER_VERTICALLY | GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY;
			DWORD dwPanBlock = GC_PAN_WITH_GUTTER;
			GESTURECONFIG gc[] =	{{GID_ZOOM, GC_ZOOM, 0},				// Translated to legacy Ctrl + Mouse Wheel message which is handled in OnMouseWheel()
									{GID_ROTATE, 0, GC_ROTATE},				// Disabled
									{GID_PAN, dwPanWant , dwPanBlock},		// Handled in OnGesture()
									{GID_TWOFINGERTAP, 0, GC_TWOFINGERTAP},	// Disabled
									{GID_PRESSANDTAP, 0, GC_PRESSANDTAP}};	// Disabled
			fpSetGestureConfig(GetSafeHwnd(), 0, 5, gc, sizeof(GESTURECONFIG));
		}
		::FreeLibrary(h);
	}

	return 0;
}

LONG CPictureView::OnThreadSafePauseSlideshow(WPARAM wparam, LPARAM lparam)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pDoc->m_SlideShowThread.PauseSlideshow();

	return 1;
}

LONG CPictureView::OnThreadSafeSlideshowLoadPicture(WPARAM wparam, LPARAM lparam)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CString* pFileName = (CString*)wparam;
	BOOL bNext = (BOOL)lparam;

	if (pFileName && !pDoc->m_bClosing)
	{
		// On error skip picture
		if (!pDoc->LoadPicture(&pDoc->m_pDib, *pFileName))
		{
			if (bNext)
				pDoc->m_SlideShowThread.NextPicture();
			else
				pDoc->m_SlideShowThread.PreviousPicture();
		}
		else
		{
			// Sends a WM_PAINT message directly, bypassing the application queue
			UpdateWindow();
		}
	}

	// Delete
	if (pFileName)
		delete pFileName;

	// Signal that we are done with loading, also if we failed
	pDoc->m_SlideShowThread.m_bSlideshowLoadPictureDone = TRUE;

	return 0;
}

LONG CPictureView::OnThreadUpdateImageInfo(WPARAM wparam, LPARAM lparam)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	BOOL bUpdateFileInfoOnly = (BOOL)wparam;

	if (pDoc &&
		!pDoc->m_bClosing)
	{
		pDoc->UpdateImageInfo(bUpdateFileInfoOnly);
		return 1;
	}
	else
		return 0;
}

BOOL CPictureView::OnPreparePrinting(CPrintInfo* pInfo) 
{
	pInfo->SetMaxPage(1);
	return DoPreparePrinting(pInfo);
}

void CPictureView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CScrollView::OnBeginPrinting(pDC, pInfo);
}

void CPictureView::OnEndPrintingFromPrintPreview(CDC* pDC, CPrintInfo* pInfo) 
{
	CScrollView::OnEndPrinting(pDC, pInfo);
}

void CPictureView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	OnEndPrintingFromPrintPreview(pDC, pInfo);

	::PostMessage(	GetSafeHwnd(),
					WM_THREADSAFE_UPDATEWINDOWSIZES,
					(WPARAM)(UPDATEWINDOWSIZES_INVALIDATE |
					UPDATEWINDOWSIZES_ERASEBKG),
					(LPARAM)0);
}

void CPictureView::OnFilePrintDirect()
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Init printer and set the printer name if not set by the /pt command line
	((CUImagerApp*)::AfxGetApp())->InitPrinter();
	if (((CUImagerApp*)::AfxGetApp())->m_pCmdInfo->m_strPrinterName == _T(""))
	{
		int index = ((CUImagerApp*)::AfxGetApp())->GetCurrentPrinterIndex();
		CString sName = ((CUImagerApp*)::AfxGetApp())->m_PrinterControl.GetPrinterName(index);
		((CUImagerApp*)::AfxGetApp())->m_pCmdInfo->m_strPrinterName = sName;
	}

	// Stop Playing
	if (pDoc->m_SlideShowThread.IsSlideshowRunning() ||
							pDoc->m_bDoRestartSlideshow)
	{
		// See CMainFrame::OnMenuSelect() to understand the m_bDoRestartSlideshow Flag
		pDoc->m_bDoRestartSlideshow = FALSE;
		pDoc->m_SlideShowThread.PauseSlideshow();
	}

	// The Animation has a separate array of dibs, sync the document's
	// one with the current one of the animation array
	if (pDoc->m_GifAnimationThread.IsAlive())
	{
		::EnterCriticalSection(&pDoc->m_csDib);
		*pDoc->m_pDib = *(pDoc->m_GifAnimationThread.m_DibAnimationArray.GetAt
						(pDoc->m_GifAnimationThread.m_dwDibAnimationPos));
		::LeaveCriticalSection(&pDoc->m_csDib);
		if (pDoc->m_GifAnimationThread.m_AlphaRenderedDibAnimationArray.GetSize() > 0)
			pDoc->m_AlphaRenderedDib = *(pDoc->m_GifAnimationThread.m_AlphaRenderedDibAnimationArray.GetAt
										(pDoc->m_GifAnimationThread.m_dwDibAnimationPos));
		else
			pDoc->UpdateAlphaRenderedDib();
	}

	// Wait and schedule command if dib not fully loaded!
	if (!pDoc->IsDibReadyForCommand(ID_FILE_PRINT_DIRECT))
		return;
	
	// Print Direct
	pDoc->m_bPrinting = TRUE;
	ForceCursor();
	CView::OnFilePrint();
	ForceCursor(FALSE);
	pDoc->m_bPrinting = FALSE;
}

void CPictureView::OnUpdateFilePrintDirect(CCmdUI* pCmdUI) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pCmdUI->Enable(	pDoc->m_pDib												&&
					(pDoc->m_dwIDAfterFullLoadCommand == 0 ||
					pDoc->m_dwIDAfterFullLoadCommand == ID_FILE_PRINT_DIRECT)	&&
					!pDoc->m_pRotationFlippingDlg								&&
					!pDoc->m_pWndPalette										&&
					!pDoc->m_pHLSDlg											&&
					!pDoc->m_pRedEyeDlg											&&
					!pDoc->m_bDoRedEyeColorPickup								&&
					!pDoc->m_pMonochromeConversionDlg							&&
					!pDoc->m_pSharpenDlg										&&
					!pDoc->m_pSoftenDlg											&&
					!pDoc->m_pSoftBordersDlg									&&
					!pDoc->m_bCrop);
}

void CPictureView::OnFilePrint() 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Init
	((CUImagerApp*)::AfxGetApp())->InitPrinter();

	// Stop Playing
	if (pDoc->m_SlideShowThread.IsSlideshowRunning() ||
							pDoc->m_bDoRestartSlideshow)
	{
		// See CMainFrame::OnMenuSelect() to understand the m_bDoRestartSlideshow Flag
		pDoc->m_bDoRestartSlideshow = FALSE;
		pDoc->m_SlideShowThread.PauseSlideshow();
	}

	// The Animation has a separate array of dibs, sync the document's
	// one with the current one of the animation array
	if (pDoc->m_GifAnimationThread.IsAlive())
	{
		::EnterCriticalSection(&pDoc->m_csDib);
		*pDoc->m_pDib = *(pDoc->m_GifAnimationThread.m_DibAnimationArray.GetAt
						(pDoc->m_GifAnimationThread.m_dwDibAnimationPos));
		::LeaveCriticalSection(&pDoc->m_csDib);
		if (pDoc->m_GifAnimationThread.m_AlphaRenderedDibAnimationArray.GetSize() > 0)
			pDoc->m_AlphaRenderedDib = *(pDoc->m_GifAnimationThread.m_AlphaRenderedDibAnimationArray.GetAt
										(pDoc->m_GifAnimationThread.m_dwDibAnimationPos));
		else
			pDoc->UpdateAlphaRenderedDib();
	}

	// Wait and schedule command if dib not fully loaded!
	if (!pDoc->IsDibReadyForCommand(ID_FILE_PRINT))
		return;

	// Print Opening Printer Select Dialog
	pDoc->m_bPrinting = TRUE;
	ForceCursor();
	CView::OnFilePrint();
	ForceCursor(FALSE);
	pDoc->m_bPrinting = FALSE;
}

void CPictureView::OnUpdateFilePrint(CCmdUI* pCmdUI) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pCmdUI->Enable(	pDoc->m_pDib										&&
					(pDoc->m_dwIDAfterFullLoadCommand == 0 ||
					pDoc->m_dwIDAfterFullLoadCommand == ID_FILE_PRINT)	&&
					!pDoc->m_pRotationFlippingDlg						&&
					!pDoc->m_pWndPalette								&&
					!pDoc->m_pHLSDlg									&&
					!pDoc->m_pRedEyeDlg									&&
					!pDoc->m_bDoRedEyeColorPickup						&&
					!pDoc->m_pMonochromeConversionDlg					&&
					!pDoc->m_pSharpenDlg								&&
					!pDoc->m_pSoftenDlg									&&
					!pDoc->m_pSoftBordersDlg							&&
					!pDoc->m_bCrop);
}

void CPictureView::OnFilePrintPreview() 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Init
	((CUImagerApp*)::AfxGetApp())->InitPrinter();

	// Set Flag
	pDoc->m_bPrintPreviewMode = TRUE;

	// Stop Playing
	if (pDoc->m_SlideShowThread.IsSlideshowRunning() ||
							pDoc->m_bDoRestartSlideshow)
	{
		// See CMainFrame::OnMenuSelect() to understand the m_bDoRestartSlideshow Flag
		pDoc->m_bDoRestartSlideshow = FALSE;
		pDoc->m_SlideShowThread.PauseSlideshow();
	}

	// Close the Layered Dialog
	if (pDoc->m_pLayeredDlg)
		pDoc->ViewLayeredDlg();		

	// The Animation has a separate array of dibs, sync the document's
	// one with the current one of the animation array
	if (pDoc->m_GifAnimationThread.IsAlive())
	{
		::EnterCriticalSection(&pDoc->m_csDib);
		*pDoc->m_pDib = *(pDoc->m_GifAnimationThread.m_DibAnimationArray.GetAt
						(pDoc->m_GifAnimationThread.m_dwDibAnimationPos));
		::LeaveCriticalSection(&pDoc->m_csDib);
		if (pDoc->m_GifAnimationThread.m_AlphaRenderedDibAnimationArray.GetSize() > 0)
			pDoc->m_AlphaRenderedDib = *(pDoc->m_GifAnimationThread.m_AlphaRenderedDibAnimationArray.GetAt
										(pDoc->m_GifAnimationThread.m_dwDibAnimationPos));
		else
			pDoc->UpdateAlphaRenderedDib();
	}

	// Cancel Transitions
	pDoc->CancelTransition();
	pDoc->CancelLoadFullJpegTransition();

	// Wait and schedule command if dib not fully loaded!
	if (!pDoc->IsDibReadyForCommand(ID_FILE_PRINT_PREVIEW))
		return;

	// Disable OSD
	if (m_bFullScreenMode)
	{
		pDoc->m_bOSDWasEnabled = pDoc->m_bEnableOsd;
		if (pDoc->m_bEnableOsd)
		{
			pDoc->m_bEnableOsd = FALSE;
			pDoc->ShowOsd(pDoc->m_bEnableOsd);
		}
	}

	// Must not create this on the frame.  Must outlive this function
	CPrintPreviewState* pState = new CPrintPreviewState;
	
	// Init Force Cursor
	ForceCursor();

	// DoPrintPreview's return value does not necessarily indicate that
	// Print preview succeeded or failed, but rather what actions are necessary
	// at this point.  If DoPrintPreview returns TRUE, it means that
	// OnEndPrintPreview will be (or has already been) called and the
	// pState structure will be/has been deleted.
	// If DoPrintPreview returns FALSE, it means that OnEndPrintPreview
	// WILL NOT be called and that cleanup, including deleting pState
	// must be done here.
	if (!DoPrintPreview(IDD_PICTUREPRINTPREVIEW, this, RUNTIME_CLASS(CPicturePrintPreviewView), pState))
	{
		// In derived classes, reverse special window handling here for
		// Preview failure case
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		delete pState;      // preview failed to initialize, delete State now
		pState = NULL;

		// Reset Flag
		pDoc->m_bPrintPreviewMode = FALSE;
		
		// If in Full-Screen mode
		if (m_bFullScreenMode)
		{
			// Hack to Not Show the Child Borders
			CRect rcMonitor = ::AfxGetMainFrame()->GetMonitorFullRect();
			int nMonitorWidth = rcMonitor.right - rcMonitor.left;
			int nMonitorHeight = rcMonitor.bottom - rcMonitor.top;
			SetWindowPos(NULL, -2, -2, nMonitorWidth + 4, nMonitorHeight + 4, SWP_NOZORDER);

			// Re-Enable OSD
			if (pDoc->m_bOSDWasEnabled)
			{
				pDoc->m_bOSDWasEnabled = FALSE;
				pDoc->m_bEnableOsd = TRUE;
				pDoc->ShowOsd(pDoc->m_bEnableOsd);
			}
		}

		// End Force Cursor
		ForceCursor(FALSE);
	}
	else
	{
		// Init the Printer Name, the Paper Size Name and the DPI
		if (GetPicturePrintPreviewView())
			GetPicturePrintPreviewView()->DisplayPrinterPaperDPI();
	}
}

void CPictureView::OnUpdateFilePrintPreview(CCmdUI* pCmdUI) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pCmdUI->Enable(	pDoc->m_pDib												&&
					(pDoc->m_dwIDAfterFullLoadCommand == 0 ||
					pDoc->m_dwIDAfterFullLoadCommand == ID_FILE_PRINT_PREVIEW)	&&
					!pDoc->m_pRotationFlippingDlg								&&
					!pDoc->m_pWndPalette										&&
					!pDoc->m_pHLSDlg											&&
					!pDoc->m_pRedEyeDlg											&&
					!pDoc->m_bDoRedEyeColorPickup								&&
					!pDoc->m_pMonochromeConversionDlg							&&
					!pDoc->m_pSharpenDlg										&&
					!pDoc->m_pSoftenDlg											&&
					!pDoc->m_pSoftBordersDlg									&&
					!pDoc->m_bCrop												&&
					!(m_bFullScreenMode											&&
					((CUImagerApp*)::AfxGetApp())->m_PrinterControl.GetPrinterCount() == 0));
}

void CPictureView::OnEndPrintPreview(CDC* pDC, CPrintInfo* pInfo, POINT point, CPreviewView* pView) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CUImagerView::OnEndPrintPreview(pDC, pInfo, point, pView);
	
	// Reset Flag
	pDoc->m_bPrintPreviewMode = FALSE;

	// If in Full-Screen mode
	if (m_bFullScreenMode)
	{
		// Hack to Not Show the Child Borders
		CRect rcMonitor = ::AfxGetMainFrame()->GetMonitorFullRect();
		int nMonitorWidth = rcMonitor.right - rcMonitor.left;
		int nMonitorHeight = rcMonitor.bottom - rcMonitor.top;
		SetWindowPos(NULL, -2, -2, nMonitorWidth + 4, nMonitorHeight + 4, SWP_NOZORDER);

		// Re-Enable OSD
		if (pDoc->m_bOSDWasEnabled)
		{
			pDoc->m_bOSDWasEnabled = FALSE;
			pDoc->m_bEnableOsd = TRUE;
			pDoc->ShowOsd(pDoc->m_bEnableOsd);
		}
	}

	// End Force Cursor
	ForceCursor(FALSE);
}

void CPictureView::OnFilePrintSetup() 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Init
	((CUImagerApp*)::AfxGetApp())->InitPrinter();

	ForceCursor();

	// Stop Playing
	if (pDoc->m_SlideShowThread.IsSlideshowRunning() ||
							pDoc->m_bDoRestartSlideshow)
	{
		// See CMainFrame::OnMenuSelect() to understand the m_bDoRestartSlideshow Flag
		pDoc->m_bDoRestartSlideshow = FALSE;
		pDoc->m_SlideShowThread.PauseSlideshow();
	}

	CPrintDialog pd(TRUE);
	::AfxGetApp()->DoPrintDialog(&pd);
	
	ForceCursor(FALSE);
}

CPicturePrintPreviewView* CPictureView::GetPicturePrintPreviewView()
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	POSITION posView = pDoc->GetFirstViewPosition();
	while (posView != NULL)
	{
		CView* pView = (CView*)pDoc->GetNextView(posView);
		ASSERT_VALID(pView);
		if (pView && pView->IsKindOf(RUNTIME_CLASS(CPicturePrintPreviewView)))
			return (CPicturePrintPreviewView*)pView;		
	}
	return NULL;
}

CRect CPictureView::PrintFit(int nWidth, int nHeight, double dXDpiRatio, double dYDpiRatio)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CRect DibPrint(0,0,0,0);
	CRect DibRect;
	if (pDoc->m_pDib && pDoc->m_pDib->IsValid())
		DibRect = CRect(0, 0,	pDoc->m_pDib->GetWidth(),
								pDoc->m_pDib->GetHeight());
	else
		return DibPrint;

	double dZoomFactorX, dZoomFactorY;
	double dXDpiRatioNormed, dYDpiRatioNormed;

	if (dXDpiRatio > dYDpiRatio)
	{
		dXDpiRatioNormed = dXDpiRatio / dYDpiRatio;
		dYDpiRatioNormed = 1.0;
		dZoomFactorX = (double)nWidth / ((double)DibRect.Width() * dXDpiRatioNormed);
		dZoomFactorY = (double)nHeight / ((double)DibRect.Height());
	}
	else if (dXDpiRatio < dYDpiRatio)
	{
		dXDpiRatioNormed = 1.0;
		dYDpiRatioNormed = dYDpiRatio / dXDpiRatio;
		dZoomFactorX = (double)nWidth / ((double)DibRect.Width());
		dZoomFactorY = (double)nHeight / ((double)DibRect.Height()* dYDpiRatioNormed);
	}
	else
	{
		dXDpiRatioNormed = 1.0;
		dYDpiRatioNormed = 1.0;
		dZoomFactorX = (double)(nWidth) / (double)(DibRect.Width());
		dZoomFactorY = (double)(nHeight) / (double)(DibRect.Height());
	}

	if (dZoomFactorX > dZoomFactorY)
	{
		// Center X
		DibPrint.top = 0;
		DibPrint.bottom = Round(DibRect.Height() * dYDpiRatioNormed * dZoomFactorY);

		int nEffWidth = Round(DibRect.Width() * dXDpiRatioNormed * dZoomFactorY);
		DibPrint.left = (nWidth - nEffWidth) / 2;
		DibPrint.right = nEffWidth + DibPrint.left;

		// Update Print Scale
		CPicturePrintPreviewView* pPrintPreview = GetPicturePrintPreviewView();
		pDoc->m_dPrintScale = dYDpiRatioNormed * dZoomFactorY / dYDpiRatio;
		if (pPrintPreview && pPrintPreview->m_pScaleEdit)
			pPrintPreview->m_pScaleEdit->SetPrintScale(pDoc->m_dPrintScale);
	}
	else
	{
		// Center Y
		DibPrint.left = 0;
		DibPrint.right = Round(DibRect.Width() * dXDpiRatioNormed * dZoomFactorX);

		int nEffHeight = Round(DibRect.Height()* dYDpiRatioNormed * dZoomFactorX );
		DibPrint.top = (nHeight - nEffHeight) / 2;
		DibPrint.bottom = nEffHeight + DibPrint.top;

		// Update Print Scale
		CPicturePrintPreviewView* pPrintPreview = GetPicturePrintPreviewView();
		pDoc->m_dPrintScale = dXDpiRatioNormed * dZoomFactorX / dXDpiRatio;
		if (pPrintPreview && pPrintPreview->m_pScaleEdit)
			pPrintPreview->m_pScaleEdit->SetPrintScale(pDoc->m_dPrintScale);
	}

	return DibPrint;
}

void CPictureView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CRect rcClient;
	GetClientRect(&rcClient);

	// Dib Rect
	CRect DibRect;
	CRect PreviewDibRect;
	if (pDoc->m_pDib && pDoc->m_pDib->IsValid())
		DibRect = CRect(0, 0,	pDoc->m_pDib->GetWidth(),
								pDoc->m_pDib->GetHeight());
	else
		return;

	// Alpha
	CDib* pDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&(pDoc->m_AlphaRenderedDib) :
					pDoc->m_pDib;

	// Pixel Scaling For Character Printing (To Be Used In Future...)
	CClientDC dcView(this);
	int XScreenPixPerInch = dcView.GetDeviceCaps(LOGPIXELSX);
	int YScreenPixPerInch = dcView.GetDeviceCaps(LOGPIXELSY);
	int XPrinterPixPerInch = pDC->GetDeviceCaps(LOGPIXELSX);
	int YPrinterPixPerInch = pDC->GetDeviceCaps(LOGPIXELSY);
	m_dXFontPixelScale = (double)XPrinterPixPerInch / (double)XScreenPixPerInch;
	m_dYFontPixelScale = (double)YPrinterPixPerInch / (double)YScreenPixPerInch;

	// Get Dib DPIs
	int nXDpiDib = pDib->GetXDpi();
	int nYDpiDib = pDib->GetYDpi();
	if (nXDpiDib == 0 || nYDpiDib == 0)
	{
		nXDpiDib = DEFAULT_DPI;
		nYDpiDib = DEFAULT_DPI;
	}

	// DPI Ratio Between Printer and Dib
	double dXDpiRatio = (double)XPrinterPixPerInch / (double)nXDpiDib;
	double dYDpiRatio = (double)YPrinterPixPerInch / (double)nYDpiDib;

	// Reset Offsets
	if (pDoc->m_bPrintSizeFit)
		pDoc->m_ptPrintOffset = pDoc->m_ptLastPrintOffset = CPoint(0, 0);

	// Fit the image to the print margin
	if (pDoc->m_bPrintSizeFit && pDoc->m_bPrintMargin)
	{
		m_rcDibPrint = PrintFit(pInfo->m_rectDraw.Width(),
								pInfo->m_rectDraw.Height(),
								dXDpiRatio, dYDpiRatio);
	}
	else
	{
		// Fit the image to the print page
		if (pDoc->m_bPrintSizeFit && !pDoc->m_bPrintMargin)
		{
			// Get print rect and offset
			CSize szPageSize;
			VERIFY(pDC->Escape(GETPHYSPAGESIZE, 0, NULL, (LPVOID)&szPageSize));
			CSize szPageOffset;
			VERIFY(pDC->Escape(GETPRINTINGOFFSET, 0, NULL, (LPVOID)&szPageOffset));
			
			// Fit to page
			m_rcDibPrint = PrintFit(szPageSize.cx,
									szPageSize.cy,
									dXDpiRatio, dYDpiRatio);
			m_rcDibPrint.OffsetRect(-szPageOffset.cx, -szPageOffset.cy);

			// Calc. the m_ptPrintOffset and m_ptLastPrintOffset offsets using the new
			// m_dPrintScale so that when switching to no fit mode they are set correctly
			int nEffWidth = Round(dXDpiRatio * DibRect.Width() * pDoc->m_dPrintScale);
			int nEffHeight = Round(dYDpiRatio * DibRect.Height() * pDoc->m_dPrintScale);
			int nXOffset = (pInfo->m_rectDraw.Width() - nEffWidth) / 2;
			int nYOffset = (pInfo->m_rectDraw.Height() - nEffHeight) / 2;
			pDoc->m_ptPrintOffset = pDoc->m_ptLastPrintOffset =
					CPoint(m_rcDibPrint.left - nXOffset, m_rcDibPrint.top - nYOffset);
		}
		// No Fit
		else
		{
			// Print Rect
			int nEffWidth = Round(dXDpiRatio * DibRect.Width() * pDoc->m_dPrintScale);
			int nEffHeight = Round(dYDpiRatio * DibRect.Height() * pDoc->m_dPrintScale);
			int nXOffset = (pInfo->m_rectDraw.Width() - nEffWidth) / 2;
			int nYOffset = (pInfo->m_rectDraw.Height() - nEffHeight) / 2;
			m_rcDibPrint.left = nXOffset;
			m_rcDibPrint.right = m_rcDibPrint.left + nEffWidth;
			m_rcDibPrint.top = nYOffset;
			m_rcDibPrint.bottom = m_rcDibPrint.top + nEffHeight;

			// Offset Print Rect
			// (m_ptPrintOffset is changed when dragging with the mouse)
			m_rcDibPrint.OffsetRect(pDoc->m_ptPrintOffset);
		}

		// Clip Print Rect
		double dXRatio = dXDpiRatio * pDoc->m_dPrintScale;
		double dYRatio = dYDpiRatio * pDoc->m_dPrintScale;
		if (m_rcDibPrint.left < pInfo->m_rectDraw.left)
		{
			DibRect.left += Round((pInfo->m_rectDraw.left - m_rcDibPrint.left) / dXRatio);
			m_rcDibPrint.left = pInfo->m_rectDraw.left;
		}
		if (m_rcDibPrint.right > pInfo->m_rectDraw.right)
		{
			DibRect.right -= Round((m_rcDibPrint.right - pInfo->m_rectDraw.right) / dXRatio);
			m_rcDibPrint.right = pInfo->m_rectDraw.right;
		}
		if (m_rcDibPrint.top < pInfo->m_rectDraw.top)
		{
			DibRect.top += Round((pInfo->m_rectDraw.top - m_rcDibPrint.top) / dYRatio);
			m_rcDibPrint.top = pInfo->m_rectDraw.top;
		}
		if (m_rcDibPrint.bottom > pInfo->m_rectDraw.bottom)
		{
			DibRect.bottom -= Round((m_rcDibPrint.bottom - pInfo->m_rectDraw.bottom) / dYRatio);
			m_rcDibPrint.bottom = pInfo->m_rectDraw.bottom;
		}
	}

	// Set PreviewDibRect
	if (pDib->GetPreviewDib() && pDib->GetPreviewDib()->IsValid())
	{
		double dXPreviewRatio = (double)pDib->GetWidth() / 
								(double)pDib->GetPreviewDib()->GetWidth();
								
		double dYPreviewRatio = (double)pDib->GetHeight() / 
								(double)pDib->GetPreviewDib()->GetHeight();
		PreviewDibRect.left = Round(DibRect.left / dXPreviewRatio);
		PreviewDibRect.right = Round(DibRect.right / dXPreviewRatio);
		PreviewDibRect.top = Round(DibRect.top / dYPreviewRatio);
		PreviewDibRect.bottom = Round(DibRect.bottom / dYPreviewRatio);
	}

	// Print Bitmap
	if (!pInfo->m_bPreview)
	{
		BeginWaitCursor();

		// Make a Copy of the Dib
		CDib Dib(*pDib);

		// Clip the Dib by Cropping
		if (!Dib.CropBits(	DibRect.left,
							DibRect.top,
							DibRect.Width(),
							DibRect.Height(),
							NULL,
							pDoc->GetView(),
							TRUE))
		{
			// Restore Pixel Scaling
			m_dXFontPixelScale = 1.0;
			m_dYFontPixelScale = 1.0;
			EndWaitCursor();
			return;
		}

		// Convert low res to 24 bpp for faster and better stretching.
		// Convert also 32 bpp to use less memory and to remove the alpha
		// component which some printers are interpreting even if the BMI
		// is not a alpha version!
		if ((Dib.GetBitCount() <= 16 || Dib.GetBitCount() == 32) &&
			!Dib.ConvertTo24bits(pDoc->GetView(), TRUE))
		{
			// Restore Pixel Scaling
			m_dXFontPixelScale = 1.0;
			m_dYFontPixelScale = 1.0;
			EndWaitCursor();
			return;
		}

		// Resize the Dib, that may fail if we do not have enough memory
		// (for example in case of huge prints)
		if (!Dib.StretchBits(	(DWORD)(m_rcDibPrint.Width()),
								(DWORD)(m_rcDibPrint.Height()),
								NULL,
								pDoc->GetView(), TRUE))
		{
			// Re-copy and re-crop because if StretchBits
			// failed the Dib's bits may have been freed,
			// but could not be allocated!
			Dib = *pDib;
			if (!Dib.CropBits(	DibRect.left,
								DibRect.top,
								DibRect.Width(),
								DibRect.Height(),
								NULL,
								pDoc->GetView(),
								TRUE))
			{
				// Restore Pixel Scaling
				m_dXFontPixelScale = 1.0;
				m_dYFontPixelScale = 1.0;
				EndWaitCursor();
				return;
			}

			// Print with halftone stretch
			Dib.SetStretchMode(HALFTONE);
			if (!Dib.Paint(	pDC->m_hDC,
							m_rcDibPrint,
							CRect(0, 0, Dib.GetWidth(), Dib.GetHeight()),
							FALSE))
			{
				// Try with color on color stretch
				Dib.SetStretchMode(COLORONCOLOR);
				Dib.Paint(	pDC->m_hDC,
							m_rcDibPrint,
							CRect(0, 0, Dib.GetWidth(), Dib.GetHeight()),
							FALSE);
			}

			// Restore Pixel Scaling
			m_dXFontPixelScale = 1.0;
			m_dYFontPixelScale = 1.0;
			EndWaitCursor();
			return;
		}

		// Print (No Stretch)
		Dib.Paint(	pDC->m_hDC,
					m_rcDibPrint,
					CRect(0, 0, Dib.GetWidth(), Dib.GetHeight()),
					FALSE);

		EndWaitCursor();
	}
	else
	{
		if (pDib->GetPreviewDib() && pDib->GetPreviewDib()->IsValid())
		{
			pDib->GetPreviewDib()->SetStretchMode(COLORONCOLOR);
			pDib->GetPreviewDib()->Paint(	pDC->m_hDC,
											m_rcDibPrint,
											PreviewDibRect,
											TRUE); // Force Stretch Otherwise it will not be previewed correctly!
		}
		else
		{
			pDib->SetStretchMode(COLORONCOLOR);
			pDib->Paint(pDC->m_hDC,
						m_rcDibPrint,
						DibRect,
						TRUE); // Force Stretch Otherwise it will not be previewed correctly!
		}
	}

	// Draw the Printer Margins
	if (pInfo->m_bPreview && pDoc->m_bPrintMargin)
	{
		CPen Pen(PS_DOT, 1, RGB(0,0,0));
		CPen* pOldPen = pDC->SelectObject(&Pen);
		HBRUSH hOldBrush = (HBRUSH)::SelectObject(pDC->GetSafeHdc(), ::GetStockObject(NULL_BRUSH));
		pDC->Rectangle(pInfo->m_rectDraw);
		pDC->SelectObject(pOldPen);
		::SelectObject(pDC->GetSafeHdc(), hOldBrush);
		Pen.DeleteObject();
		// Note: It is not necessary (but it is not harmful)
		//       to delete stock objects by calling DeleteObject.
	}

	// Restore Pixel Scaling
	m_dXFontPixelScale = 1.0;
	m_dYFontPixelScale = 1.0;
}

void CPictureView::OnViewFullscreen() 
{
	::AfxGetMainFrame()->EnterExitFullscreen();
}

void CPictureView::OnUpdateViewFullscreen(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bFullScreenMode ? 1 : 0);	
}

void CPictureView::OnViewZoomFit() 
{
	CZoomComboBox* pZoomCB = &(((CPictureToolBar*)((CToolBarChildFrame*)GetParentFrame())->GetToolBar())->m_ZoomComboBox);
	if (pZoomCB)
	{
		void* pData = pZoomCB->GetItemDataPtr(0);
		if ((int)pData != -1)
		{
			pZoomCB->SetCurSel(0);
			pZoomCB->OnChangeZoomFactor(*((double*)pData));
			UpdateWindowSizes(TRUE, TRUE, FALSE);
		}
	}
}

void CPictureView::OnUpdateViewZoomFit(CCmdUI* pCmdUI) 
{
	CZoomComboBox* pZoomCB = &(((CPictureToolBar*)((CToolBarChildFrame*)GetParentFrame())->GetToolBar())->m_ZoomComboBox);
	if (pZoomCB)
		pCmdUI->SetCheck((pZoomCB->GetCurSel() == 0 || (m_bFullScreenMode && pZoomCB->GetCurSel() != 1)) ? 1 : 0);
}

void CPictureView::OnViewZoomFitbig() 
{
	CZoomComboBox* pZoomCB = &(((CPictureToolBar*)((CToolBarChildFrame*)GetParentFrame())->GetToolBar())->m_ZoomComboBox);
	if (pZoomCB)
	{
		void* pData = pZoomCB->GetItemDataPtr(1);
		if ((int)pData != -1)
		{
			pZoomCB->SetCurSel(1);
			pZoomCB->OnChangeZoomFactor(*((double*)pData));
			UpdateWindowSizes(TRUE, TRUE, FALSE);
		}
	}
}

void CPictureView::OnUpdateViewZoomFitbig(CCmdUI* pCmdUI) 
{
	CZoomComboBox* pZoomCB = &(((CPictureToolBar*)((CToolBarChildFrame*)GetParentFrame())->GetToolBar())->m_ZoomComboBox);
	if (pZoomCB)
		pCmdUI->SetCheck(pZoomCB->GetCurSel() == 1 ? 1 : 0);
}

void CPictureView::OnViewZoomTool() 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!m_bFullScreenMode)
	{
		if (pDoc->m_bZoomTool)
			pDoc->CancelZoomTool();
		else
			pDoc->ViewZoomTool();
	}
}

void CPictureView::OnUpdateViewZoomTool(CCmdUI* pCmdUI) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->Enable(!m_bFullScreenMode);
	pCmdUI->SetCheck(pDoc->m_bZoomTool ? 1 : 0);
}

void CPictureView::OnViewNextMonitor() 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (m_bFullScreenMode)
	{
		::AfxGetMainFrame()->FullScreenTo(TRUE);
		if (pDoc->m_pOsdDlg)
			pDoc->m_pOsdDlg->MonitorSwitch();
	}
}

void CPictureView::OnViewPreviousMonitor() 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (m_bFullScreenMode)
	{
		::AfxGetMainFrame()->FullScreenTo(FALSE);
		if (pDoc->m_pOsdDlg)
			pDoc->m_pOsdDlg->MonitorSwitch();
	}
}

BOOL CPictureView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// Do Nothing if Already Mouse Click Scrolling!
	if (m_bDoScrollMove)
		return TRUE;

	BOOL bDoMouseMove = FALSE;
	
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Next / Prev. Picture
	if ((m_bFullScreenMode || !(MK_CONTROL & nFlags))		&&
		!pDoc->m_bZoomTool									&&
		!IsXOrYScroll()										&&
		!pDoc->IsModified()									&&
		!pDoc->m_pRotationFlippingDlg						&&
		!pDoc->m_pWndPalette								&&
		!pDoc->m_pHLSDlg									&&
		!pDoc->m_pRedEyeDlg									&&
		!pDoc->m_bDoRedEyeColorPickup						&&
		!pDoc->m_pMonochromeConversionDlg					&&
		!pDoc->m_pSharpenDlg								&&
		!pDoc->m_pSoftenDlg									&&
		!pDoc->m_pSoftBordersDlg							&&
		!pDoc->m_bCrop)
	{
		// Prev.
		if (zDelta > 0)
			pDoc->m_SlideShowThread.PreviousPicture();
		// Next
		else
			pDoc->m_SlideShowThread.NextPicture();
	}
	// Zoom In / Out?
	// (for legacy support the zoom touch gesture triggers
	// this mouse wheel message + CTRL key) 
	else if ((MK_CONTROL & nFlags)	&&
			!m_bFullScreenMode		&&
			!pDoc->m_bZoomTool)
	{
		ScreenToClient(&pt);
		ZoomPoint(pt, zDelta < 0);
	}
	else 
	{
		// Scroll?
		BOOL bScrollHoriz = (MK_SHIFT & nFlags) ? TRUE : FALSE;
		if (!m_bFullScreenMode &&
			((bScrollHoriz && IsXScroll()) || (!bScrollHoriz && IsYScroll())))
		{
			CPoint pos = GetScrollPosition();
			CSize size = GetTotalSize();
			int nXStep = size.cx / SCROLLWHEEL_STEPS;
			int nYStep = size.cy / SCROLLWHEEL_STEPS;
			if (zDelta > 0)
			{
				if (bScrollHoriz)
					ScrollToPosition(CPoint(MAX(pos.x - nXStep, 0), pos.y));
				else
					ScrollToPosition(CPoint(pos.x, MAX(pos.y - nYStep, 0)));
			}
			else
			{
				if (bScrollHoriz)
					ScrollToPosition(CPoint(MIN(pos.x + nXStep, size.cx), pos.y));
				else
					ScrollToPosition(CPoint(pos.x, MIN(pos.y + nYStep, size.cy)));
			}

			// Scrolling is like moving, update Crop Rect!
			if (pDoc->m_bCrop && m_bCropMouseCaptured)
				bDoMouseMove = TRUE;
		}
	}
	
	// This function sends a WM_PAINT message directly,
	// bypassing the application queue. If the update
	// region is empty, WM_PAINT is not sent
	UpdateWindow();

	// Call this after UpdateWindow(),
	// because before the view has not been updated
	// with the new coordinates!
	if (bDoMouseMove)
		OnMouseMove(m_uiOnMouseMoveLastFlag, m_OnMouseMoveLastPoint);

	// Update Pane Text
#ifndef VIDEODEVICEDOC
	UpdatePaneText();
#endif

	return TRUE;
}

LRESULT CPictureView::OnColorPicked(WPARAM wParam, LPARAM lParam)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	int nIndex = (int)wParam;
	COLORREF crColor = (COLORREF)lParam;
	
	if (((CUImagerApp*)::AfxGetApp())->ShowColorDlg(crColor, pDoc->m_pWndPalette))
	{
		if (pDoc->m_pDib && pDoc->m_pDib->GetColors())
		{
			// A Duplicated Dib is created
			pDoc->AddUndo();

			// Edit Color
			if (nIndex >= pDoc->m_pDib->GetNumColors())
				nIndex = pDoc->m_pDib->GetNumColors() - 1;
			RGBQUAD* pColors = pDoc->m_pDib->GetColors();
			pColors[nIndex].rgbRed = GetRValue(crColor);
			pColors[nIndex].rgbGreen = GetGValue(crColor);
			pColors[nIndex].rgbBlue = GetBValue(crColor);
			pDoc->m_pDib->CreatePaletteFromBMI();

			// Re-Show new Palette
			CRect rcWnd;
			pDoc->m_pWndPalette->GetWindowRect(&rcWnd);
			pDoc->m_pWndPalette->Close();
			pDoc->m_pWndPalette = (CPaletteWnd*)new CPaletteWnd;
			if (!pDoc->m_pWndPalette)
				return 0;
			pDoc->m_pWndPalette->Create(rcWnd.TopLeft(),
										this,
										pDoc->m_pDib->GetPalette());
			ForceCursor();
			pDoc->m_pWndPalette->ShowWindow(SW_SHOW);

			// Set Modified Flag
			pDoc->SetModifiedFlag();
		
			// Set Document Title
			pDoc->SetDocumentTitle();

			// Invalidate
			pDoc->InvalidateAllViews(FALSE);

			// Update Image Info
			pDoc->UpdateImageInfo();
		}
	}

	return 0;
}

LRESULT CPictureView::OnColorPickerClosed(WPARAM wParam, LPARAM lParam)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->m_pWndPalette = NULL;
	ForceCursor(FALSE);
	return 0;
}

// Flicks handling for Vista and Windows 7 (flicks were removed in Windows 8)
// Note: if WM_TABLET_FLICK is not handled a WM_APPCOMMAND is fired
LRESULT CPictureView::OnApplicationCommand(WPARAM /*wParam*/, LPARAM lParam)
{
    CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
    const int cmd = GET_APPCOMMAND_LPARAM(lParam);  
    switch (cmd)
    {
        case APPCOMMAND_BROWSER_BACKWARD :
			if (((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc))
				pDoc->m_SlideShowThread.PreviousPicture();
            return TRUE;  // return TRUE to indicate that we processed the button

        case APPCOMMAND_BROWSER_FORWARD :
			if (((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc))
				pDoc->m_SlideShowThread.NextPicture();
			return TRUE;  // return TRUE to indicate that we processed the button

		default :
			break;
    }
    
    // This was a button we don't care about - return FALSE to indicate
    // that we didn't process the command
    return FALSE;
}

// Gesture handling for Windows 7 and higher
LRESULT CPictureView::OnGesture(WPARAM /*wParam*/, LPARAM lParam)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Get gesture function pointers
	typedef	BOOL (WINAPI *FPGETGESTUREINFO)(HGESTUREINFO_COMPATIBLE, PGESTUREINFO_COMPATIBLE);
	typedef	BOOL (WINAPI *FPCLOSEGESTUREINFOHANDLE)(HGESTUREINFO_COMPATIBLE);
	HINSTANCE h = ::LoadLibraryFromSystem32(_T("user32.dll"));
	if (!h)
		return Default();
	FPGETGESTUREINFO fpGetGestureInfo = (FPGETGESTUREINFO)::GetProcAddress(h, "GetGestureInfo");
	FPCLOSEGESTUREINFOHANDLE fpCloseGestureInfoHandle = (FPCLOSEGESTUREINFOHANDLE)::GetProcAddress(h, "CloseGestureInfoHandle");
	if (fpGetGestureInfo == NULL || fpCloseGestureInfoHandle == NULL)
	{
		::FreeLibrary(h);
		return Default();
	}

	// Get current gesture info
	GESTUREINFO_COMPATIBLE CurrentGestureInfo;
	memset(&CurrentGestureInfo, 0, sizeof(GESTUREINFO_COMPATIBLE));
	CurrentGestureInfo.cbSize = sizeof(GESTUREINFO_COMPATIBLE);
	if (!fpGetGestureInfo((HGESTUREINFO_COMPATIBLE)lParam, &CurrentGestureInfo) || CurrentGestureInfo.hwndTarget != GetSafeHwnd())
	{
		::FreeLibrary(h);
		return Default();
	}

	// Process gesture
	BOOL bDefaultProcessing = TRUE;
	switch (CurrentGestureInfo.dwID)
	{
		case GID_PAN :
		{
			// Get current position in client coordinates
			CPoint pt(CurrentGestureInfo.ptsLocation.x, CurrentGestureInfo.ptsLocation.y);
			ScreenToClient(&pt);

			if (CurrentGestureInfo.dwFlags & GF_BEGIN)
			{
				// Init vars
				m_ptGesturePanStart = pt;
				m_ptGesturePanStartScrollPos = GetScrollPosition();
				m_bGesturePanSlideDone = FALSE;

				// Init crop
				if (pDoc->m_bCrop)
				{
					pt += GetScrollPosition();
					int nCropWidth4 = m_CropZoomRect.Width() / 4;
					int nCropHeight4 = m_CropZoomRect.Height() / 4;
					if (pt.x < (m_CropZoomRect.left + nCropWidth4) && pt.y < (m_CropZoomRect.top + nCropHeight4))
						m_nGesturePanCropHandle = 0;	// top-left
					else if (pt.x > (m_CropZoomRect.right - nCropWidth4) && pt.y > (m_CropZoomRect.bottom - nCropHeight4))
						m_nGesturePanCropHandle = 1;	// bottom-right
					else if (pt.x < (m_CropZoomRect.left + nCropWidth4) && pt.y > (m_CropZoomRect.bottom - nCropHeight4))
						m_nGesturePanCropHandle = 2;	// bottom-left
					else if (pt.x > (m_CropZoomRect.right - nCropWidth4) && pt.y < (m_CropZoomRect.top + nCropHeight4))
						m_nGesturePanCropHandle = 3;	// top-right
					else if (pt.y < (m_CropZoomRect.top + nCropHeight4))
						m_nGesturePanCropHandle = 4;	// top
					else if (pt.y > (m_CropZoomRect.bottom - nCropHeight4))
						m_nGesturePanCropHandle = 5;	// bottom
					else if (pt.x < (m_CropZoomRect.left + nCropWidth4))
						m_nGesturePanCropHandle = 6;	// left
					else if (pt.x > (m_CropZoomRect.right - nCropWidth4))
						m_nGesturePanCropHandle = 7;	// right
					else
					{
						m_nGesturePanCropHandle = 8;	// center
						pDoc->m_rcCropCenter = pDoc->m_rcCropDelta;
					}
				}
			}
			else
			{
				// Calc. delta
				int dx = pt.x - m_ptGesturePanStart.x;
				int dy = pt.y - m_ptGesturePanStart.y;

				// Crop
				if (pDoc->m_bCrop)
				{
					int nRectSizeXInside = ::SystemDPIScale(CROP_RECT_X_INSIDE);
					int nRectSizeYInside = ::SystemDPIScale(CROP_RECT_Y_INSIDE);
					pt += GetScrollPosition();
					switch (m_nGesturePanCropHandle)
					{
						// top-left
						case 0 :	CropTop(pt, nRectSizeYInside, FALSE);
									CropLeft(pt, nRectSizeXInside, FALSE);
									break;
						// bottom-right
						case 1 :	CropBottom(pt, nRectSizeYInside, FALSE);
									CropRight(pt, nRectSizeXInside, FALSE);
									break;
						// bottom-left
						case 2 :	CropBottom(pt, nRectSizeYInside, FALSE);
									CropLeft(pt, nRectSizeXInside, FALSE);
									break;
						// top-right
						case 3 :	CropTop(pt, nRectSizeYInside, FALSE);
									CropRight(pt, nRectSizeXInside, FALSE);
									break;
						// top
						case 4 :	CropTop(pt, nRectSizeYInside, FALSE);
									break;
						// bottom
						case 5 :	CropBottom(pt, nRectSizeYInside, FALSE);
									break;
						// left
						case 6 :	CropLeft(pt, nRectSizeXInside, FALSE);
									break;
						// right
						case 7 :	CropRight(pt, nRectSizeXInside, FALSE);
									break;
						// center
						case 8 :	CropCenter(CPoint(dx, dy), nRectSizeXInside, nRectSizeYInside);
									break;
					}
					UpdateCropRectangles();
					UpdateCropStatusText();
				}
				// Scroll
				else if (IsXOrYScroll())
				{
					CPoint ptCurrentScrollPos = GetScrollPosition();
					CPoint ptScrollPos = m_ptGesturePanStartScrollPos;
					if (IsXScroll())
						ptScrollPos.x -= dx;
					else
						ptScrollPos.x = ptCurrentScrollPos.x;
					if (IsYScroll())
						ptScrollPos.y -= dy;
					else
						ptScrollPos.y = ptCurrentScrollPos.y;
					ScrollToPosition(ptScrollPos);
				}
				// Slide
				else if (((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc) && !m_bGesturePanSlideDone)
				{
					// Set flag
					m_bGesturePanSlideDone = TRUE;

					// Calc. ABS
					int dxABS = ABS(dx);
					int dyABS = ABS(dy);

					// Horizontal swipe
					if (dxABS > dyABS && dxABS > GESTURE_PAN_MIN_SWIPE_LENGTH)
					{
						// Left to right swipe
						if (dx > 0)
						{
							pDoc->m_SlideShowThread.PreviousPicture();
						}
						// Right to left swipe
						else
						{
							pDoc->m_SlideShowThread.NextPicture();
						}
					}
					// Vertical swipe
					else if (dyABS > dxABS && dyABS > GESTURE_PAN_MIN_SWIPE_LENGTH)
					{
						// Top to bottom swipe
						if (dy > 0)
						{
							if (!pDoc->ViewPreviousPageFrame())
								pDoc->m_SlideShowThread.PreviousPicture();
						}
						// Bottom to top swipe
						else
						{
							if (!pDoc->ViewNextPageFrame())
								pDoc->m_SlideShowThread.NextPicture();
						}
					}
				}
			}

			bDefaultProcessing = FALSE;
			break;
		}

		default :
			break; // do default processing
	}

	// Close info handle if processing stops here
	if (!bDefaultProcessing)
		fpCloseGestureInfoHandle((HGESTUREINFO_COMPATIBLE)lParam);

	// Free library
	::FreeLibrary(h);

	// return 0 if processing done here
	return bDefaultProcessing ? Default() : 0;
}

void CPictureView::UpdateCropRectangles()
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Calc. crop rectangles
	m_CropZoomRect = m_ZoomRect;
	pDoc->m_CropDocRect = pDoc->m_DocRect;
	m_CropZoomRect.DeflateRect(	Round(pDoc->m_rcCropDelta.left*pDoc->m_dZoomFactor),
								Round(pDoc->m_rcCropDelta.top*pDoc->m_dZoomFactor),
								Round(pDoc->m_rcCropDelta.right*pDoc->m_dZoomFactor),
								Round(pDoc->m_rcCropDelta.bottom*pDoc->m_dZoomFactor));
	m_CropZoomRect.NormalizeRect();
	pDoc->m_CropDocRect.DeflateRect(pDoc->m_rcCropDelta);
	pDoc->m_CropDocRect.NormalizeRect();

	// Update Window
	CRect rcClient; 
	GetClientRect(&rcClient);
	InvalidateRect(rcClient, FALSE);
}

void CPictureView::StepLeftCropEdge(BOOL bDirectionLeft, BOOL bCursorTop)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->m_bCrop && !m_bCropMouseCaptured)
	{
		// Simulate a left mouse movement
		int nZoomedPixelAlignX;
		if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
			nZoomedPixelAlignX = MAX(1, Round(pDoc->m_dZoomFactor));
		else
			nZoomedPixelAlignX = m_nZoomedPixelAlignX;
		if (bDirectionLeft)
			CropLeft(CPoint(m_CropZoomRect.left - nZoomedPixelAlignX, 0), ::SystemDPIScale(CROP_RECT_X_INSIDE), FALSE);
		else
			CropLeft(CPoint(m_CropZoomRect.left + nZoomedPixelAlignX, 0), ::SystemDPIScale(CROP_RECT_X_INSIDE), FALSE);

		// Update the crop rectangles
		UpdateCropRectangles();
		
		// Set cursor position
		CPoint point;
		point.x = m_CropZoomRect.left;
		point.y = bCursorTop ? m_CropZoomRect.top : m_CropZoomRect.bottom;
		SetCursorPosInsideClientRect(point);
	}
}

void CPictureView::StepRightCropEdge(BOOL bDirectionLeft, BOOL bCursorTop)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->m_bCrop && !m_bCropMouseCaptured)
	{
		// Simulate a right mouse movement
		int nZoomedPixelAlignX;
		if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
			nZoomedPixelAlignX = MAX(1, Round(pDoc->m_dZoomFactor));
		else
			nZoomedPixelAlignX = m_nZoomedPixelAlignX;
		if (bDirectionLeft)
			CropRight(CPoint(m_CropZoomRect.right - nZoomedPixelAlignX, 0), ::SystemDPIScale(CROP_RECT_X_INSIDE), FALSE);
		else
			CropRight(CPoint(m_CropZoomRect.right + nZoomedPixelAlignX, 0), ::SystemDPIScale(CROP_RECT_X_INSIDE), FALSE);

		// Update the crop rectangles
		UpdateCropRectangles();
		
		// Set cursor position
		CPoint point;
		point.x = m_CropZoomRect.right;
		point.y = bCursorTop ? m_CropZoomRect.top : m_CropZoomRect.bottom;
		SetCursorPosInsideClientRect(point);
	}
}

void CPictureView::StepTopCropEdge(BOOL bDirectionUp, BOOL bCursorLeft)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->m_bCrop && !m_bCropMouseCaptured)
	{
		// Simulate a left mouse movement
		int nZoomedPixelAlignY;
		if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
			nZoomedPixelAlignY = MAX(1, Round(pDoc->m_dZoomFactor));
		else
			nZoomedPixelAlignY = m_nZoomedPixelAlignY;
		if (bDirectionUp)
			CropTop(CPoint(0, m_CropZoomRect.top - nZoomedPixelAlignY), ::SystemDPIScale(CROP_RECT_Y_INSIDE), FALSE);
		else
			CropTop(CPoint(0, m_CropZoomRect.top + nZoomedPixelAlignY), ::SystemDPIScale(CROP_RECT_Y_INSIDE), FALSE);

		// Update the crop rectangles
		UpdateCropRectangles();
		
		// Set cursor position
		CPoint point;
		point.x = bCursorLeft ? m_CropZoomRect.left : m_CropZoomRect.right;
		point.y = m_CropZoomRect.top;
		SetCursorPosInsideClientRect(point);
	}
}

void CPictureView::StepBottomCropEdge(BOOL bDirectionUp, BOOL bCursorLeft)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->m_bCrop && !m_bCropMouseCaptured)
	{
		// Simulate a left mouse movement
		int nZoomedPixelAlignY;
		if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
			nZoomedPixelAlignY = MAX(1, Round(pDoc->m_dZoomFactor));
		else
			nZoomedPixelAlignY = m_nZoomedPixelAlignY;
		if (bDirectionUp)
			CropBottom(CPoint(0, m_CropZoomRect.bottom - nZoomedPixelAlignY), ::SystemDPIScale(CROP_RECT_Y_INSIDE), FALSE);
		else
			CropBottom(CPoint(0, m_CropZoomRect.bottom + nZoomedPixelAlignY), ::SystemDPIScale(CROP_RECT_Y_INSIDE), FALSE);

		// Update the crop rectangles
		UpdateCropRectangles();

		// Set cursor position
		CPoint point;
		point.x = bCursorLeft ? m_CropZoomRect.left : m_CropZoomRect.right;
		point.y = m_CropZoomRect.bottom;
		SetCursorPosInsideClientRect(point);
	}
}

void CPictureView::StepCropCenter(CPoint delta)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->m_bCrop && !m_bCropMouseCaptured)
	{
		// Simulate a centered mouse movement
		int nZoomedPixelAlignX;
		if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
			nZoomedPixelAlignX = MAX(1, Round(pDoc->m_dZoomFactor));
		else
			nZoomedPixelAlignX = m_nZoomedPixelAlignX;
		int nZoomedPixelAlignY;
		if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
			nZoomedPixelAlignY = MAX(1, Round(pDoc->m_dZoomFactor));
		else
			nZoomedPixelAlignY = m_nZoomedPixelAlignY;
		pDoc->m_rcCropCenter = pDoc->m_rcCropDelta;
		delta.x *= nZoomedPixelAlignX;
		delta.y *= nZoomedPixelAlignY;
		CropCenter(delta, ::SystemDPIScale(CROP_RECT_X_INSIDE), ::SystemDPIScale(CROP_RECT_Y_INSIDE));

		// Update the crop rectangles
		UpdateCropRectangles();
		
		// Set cursor position
		SetCursorPosInsideClientRect(m_CropZoomRect.CenterPoint());
	}
}

void CPictureView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CRect rcClient;
	GetClientRect(&rcClient);

	CMenu menu;
	CMenu* pPopup = NULL;
	CPoint Point;
	Point.x = 6;
	Point.y = 6;

	switch (nChar)
	{
		case VK_CONTROL :
		case VK_SHIFT :
			if (pDoc->m_bZoomTool)
			{
				pDoc->m_bZoomToolMinus = TRUE;
				UpdateCursor();
			}
			else if (pDoc->m_bCrop)
			{
				if (!m_bCropMaintainAspectRatio)
				{
					m_bCropMaintainAspectRatio = TRUE;
					m_dCropAspectRatio =	(double)pDoc->m_CropDocRect.Width() /
											(double)pDoc->m_CropDocRect.Height();
				}
			}
			break;

		case VK_TAB :
			if (pDoc->m_bCrop && !m_bCropMouseCaptured)
			{
				CPoint point;
				::GetSafeCursorPos(&point);
				ScreenToClient(&point);
				point += GetScrollPosition();
				CPoint center(m_CropZoomRect.CenterPoint());
				if (::GetKeyState(VK_SHIFT) < 0)
				{
					// Center -> Left-Top
					if (ABS(center.x - point.x) < m_CropZoomRect.Width() / 4 &&
						ABS(center.y - point.y) < m_CropZoomRect.Height() / 4)
					{
						point.x = m_CropZoomRect.left;
						point.y = m_CropZoomRect.top;
					}
					// Left -> Right-Bottom
					else if (ABS(m_CropZoomRect.left - point.x) < ABS(m_CropZoomRect.right - point.x))
					{
						point.x = m_CropZoomRect.right - 1;
						point.y = m_CropZoomRect.bottom - 1;
					}
					// Right -> Center
					else
					{
						point = center;
					}
				}
				else
				{
					// Center -> Right-Bottom
					if (ABS(center.x - point.x) < m_CropZoomRect.Width() / 4 &&
						ABS(center.y - point.y) < m_CropZoomRect.Height() / 4)
					{
						point.x = m_CropZoomRect.right - 1;
						point.y = m_CropZoomRect.bottom - 1;
					}
					// Left -> Center
					else if (ABS(m_CropZoomRect.left - point.x) < ABS(m_CropZoomRect.right - point.x))
					{
						point = center;
					}
					// Right -> Left-Top
					else
					{
						point.x = m_CropZoomRect.left;
						point.y = m_CropZoomRect.top;
					}
				}
				SetCursorPosInsideClientRect(point);
			}
			else if (m_bFullScreenMode)
			{
				if (::GetKeyState(VK_SHIFT) < 0)
					::AfxGetMainFrame()->FullScreenTo(FALSE);	// Previous Monitor
				else
					::AfxGetMainFrame()->FullScreenTo(TRUE);	// Next Monitor
				if (pDoc->m_pOsdDlg)
					pDoc->m_pOsdDlg->MonitorSwitch();
			}
			break;

		case VK_SUBTRACT :
			if (!m_bFullScreenMode)
				ZoomPoint(rcClient.CenterPoint(), TRUE); // Zoom-Out
			break;

		case VK_ADD :
			if (!m_bFullScreenMode)
				ZoomPoint(rcClient.CenterPoint(), FALSE); // Zoom-In
			break;

		case VK_DIVIDE :
			OnViewZoomFitbig();
			break;

		case VK_MULTIPLY :
			OnViewZoomFit();
			break;

		case VK_RETURN : // Enter
			if (pDoc->m_bCrop)
				pDoc->DoCropRect();
			else
			{
				if (!((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc, TRUE))
					break;
				if (pDoc->m_SlideShowThread.IsSlideshowRunning())
					pDoc->m_SlideShowThread.PauseSlideshow();
				else
					pDoc->m_SlideShowThread.RunSlideshow();
			}
			break;

		case VK_PAUSE : // Pause
			if (!((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc, TRUE))
				break;
			pDoc->m_SlideShowThread.PauseSlideshow();
			break;

		case VK_ESCAPE :
			if (pDoc->m_bZoomTool)
				pDoc->CancelZoomTool();
			else if (	pDoc->m_pRotationFlippingDlg		||
						pDoc->m_pWndPalette					||
						pDoc->m_pLayeredDlg					||
						pDoc->m_pImageInfoDlg				||
						pDoc->m_pHLSDlg						||
						pDoc->m_pRedEyeDlg					||
						pDoc->m_pMonochromeConversionDlg	||
						pDoc->m_pSharpenDlg					||
						pDoc->m_pSoftenDlg					||
						pDoc->m_pSoftBordersDlg				||
						pDoc->m_bDoRedEyeColorPickup)
			{
				if (pDoc->m_pRotationFlippingDlg)
					pDoc->m_pRotationFlippingDlg->Close();
				if (pDoc->m_pWndPalette)
					pDoc->m_pWndPalette->Close();
				if (pDoc->m_pImageInfoDlg)
					pDoc->m_pImageInfoDlg->Close();
				if (pDoc->m_pLayeredDlg)
				{
					pDoc->m_LayeredDlgThread.Kill();
					pDoc->m_pLayeredDlg->Close();
				}
				if (pDoc->m_pHLSDlg)
					pDoc->m_pHLSDlg->Close();
				if (pDoc->m_pMonochromeConversionDlg)
					pDoc->m_pMonochromeConversionDlg->Close();
				if (pDoc->m_pSharpenDlg)
					pDoc->m_pSharpenDlg->Close();
				if (pDoc->m_pSoftenDlg)
					pDoc->m_pSoftenDlg->Close();
				if (pDoc->m_pSoftBordersDlg)
					pDoc->m_pSoftBordersDlg->Close();
				if (pDoc->m_pRedEyeDlg)
					pDoc->m_pRedEyeDlg->Close();
				else if (pDoc->m_bDoRedEyeColorPickup)
				{
					pDoc->m_bDoRedEyeColorPickup = FALSE;
					ForceCursor(FALSE);
					::AfxGetMainFrame()->StatusText();
				}
			}
			else if (pDoc->m_bCrop)
				pDoc->CancelCrop();
			else if (m_bFullScreenMode)
				::AfxGetMainFrame()->EnterExitFullscreen();	// Exit Full-Screen Mode
			else
				pDoc->CloseDocument();
			break;

		case VK_DOWN :
			if (pDoc->m_bCrop)
			{
				CPoint point;
				::GetSafeCursorPos(&point);
				ScreenToClient(&point);
				point += GetScrollPosition();
				CPoint center(m_CropZoomRect.CenterPoint());
				if (ABS(center.x - point.x) < m_CropZoomRect.Width() / 4 &&
					ABS(center.y - point.y) < m_CropZoomRect.Height() / 4)
					StepCropCenter(CPoint(0, 1));
				else if (ABS(m_CropZoomRect.top - point.y) < ABS(m_CropZoomRect.bottom - point.y))
				{
					if (ABS(m_CropZoomRect.left - point.x) < ABS(m_CropZoomRect.right - point.x))
						StepTopCropEdge(FALSE, TRUE);		// Go down and cursor top-left
					else
						StepTopCropEdge(FALSE, FALSE);		// Go down and cursor top-right
				}
				else
				{
					if (ABS(m_CropZoomRect.left - point.x) < ABS(m_CropZoomRect.right - point.x))
						StepBottomCropEdge(FALSE, TRUE);	// Go down and cursor bottom-left
					else
						StepBottomCropEdge(FALSE, FALSE);	// Go down and cursor bottom-right
				}
			}
			else
			{
				if (!((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc, TRUE))
					break;
				pDoc->m_SlideShowThread.NextPicture();
			}
			break;

		case VK_RIGHT :
			if (pDoc->m_bCrop)
			{
				CPoint point;
				::GetSafeCursorPos(&point);
				ScreenToClient(&point);
				point += GetScrollPosition();
				CPoint center(m_CropZoomRect.CenterPoint());
				if (ABS(center.x - point.x) < m_CropZoomRect.Width() / 4 &&
					ABS(center.y - point.y) < m_CropZoomRect.Height() / 4)
					StepCropCenter(CPoint(1, 0));
				else if (ABS(m_CropZoomRect.left - point.x) < ABS(m_CropZoomRect.right - point.x))
				{
					if (ABS(m_CropZoomRect.top - point.y) < ABS(m_CropZoomRect.bottom - point.y))
						StepLeftCropEdge(FALSE, TRUE);		// Go right and cursor left-top
					else
						StepLeftCropEdge(FALSE, FALSE);		// Go right and cursor left-bottom
				}
				else
				{
					if (ABS(m_CropZoomRect.top - point.y) < ABS(m_CropZoomRect.bottom - point.y))
						StepRightCropEdge(FALSE, TRUE);		// Go right and cursor right-top
					else
						StepRightCropEdge(FALSE, FALSE);	// Go right and cursor right-bottom
				}
			}
			else
			{
				if (!((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc, TRUE))
					break;
				pDoc->m_SlideShowThread.NextPicture();
			}
			break;

		case VK_SPACE :
			if (!((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc, TRUE))
				break;
			pDoc->m_SlideShowThread.NextPicture();
			break;

		case VK_UP :
			if (pDoc->m_bCrop)
			{
				CPoint point;
				::GetSafeCursorPos(&point);
				ScreenToClient(&point);
				point += GetScrollPosition();
				CPoint center(m_CropZoomRect.CenterPoint());
				if (ABS(center.x - point.x) < m_CropZoomRect.Width() / 4 &&
					ABS(center.y - point.y) < m_CropZoomRect.Height() / 4)
					StepCropCenter(CPoint(0, -1));
				else if (ABS(m_CropZoomRect.top - point.y) < ABS(m_CropZoomRect.bottom - point.y))
				{
					if (ABS(m_CropZoomRect.left - point.x) < ABS(m_CropZoomRect.right - point.x))
						StepTopCropEdge(TRUE, TRUE);		// Go up and cursor top-left
					else
						StepTopCropEdge(TRUE, FALSE);		// Go up and cursor top-right
				}
				else
				{
					if (ABS(m_CropZoomRect.left - point.x) < ABS(m_CropZoomRect.right - point.x))
						StepBottomCropEdge(TRUE, TRUE);		// Go up and cursor bottom-left
					else
						StepBottomCropEdge(TRUE, FALSE);	// Go up and cursor bottom-right
				}
			}
			else
			{
				if (!((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc, TRUE))
					break;
				pDoc->m_SlideShowThread.PreviousPicture();
			}
			break;

		case VK_LEFT :
			if (pDoc->m_bCrop)
			{
				CPoint point;
				::GetSafeCursorPos(&point);
				ScreenToClient(&point);
				point += GetScrollPosition();
				CPoint center(m_CropZoomRect.CenterPoint());
				if (ABS(center.x - point.x) < m_CropZoomRect.Width() / 4 &&
					ABS(center.y - point.y) < m_CropZoomRect.Height() / 4)
					StepCropCenter(CPoint(-1, 0));
				else if (ABS(m_CropZoomRect.left - point.x) < ABS(m_CropZoomRect.right - point.x))
				{
					if (ABS(m_CropZoomRect.top - point.y) < ABS(m_CropZoomRect.bottom - point.y))
						StepLeftCropEdge(TRUE, TRUE);		// Go left and cursor left-top
					else
						StepLeftCropEdge(TRUE, FALSE);		// Go left and cursor left-bottom
				}
				else
				{
					if (ABS(m_CropZoomRect.top - point.y) < ABS(m_CropZoomRect.bottom - point.y))
						StepRightCropEdge(TRUE, TRUE);		// Go left and cursor right-top
					else
						StepRightCropEdge(TRUE, FALSE);		// Go left and cursor right-bottom
				}
			}
			else
			{
				if (!((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc, TRUE))
					break;
				pDoc->m_SlideShowThread.PreviousPicture();
			}
			break;

		case VK_BACK :
			if (!((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc, TRUE))
				break;
			pDoc->m_SlideShowThread.PreviousPicture();
			break;

		case VK_PRIOR : // Page Up
			if (!((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc, TRUE))
				break;
			if (!pDoc->ViewPreviousPageFrame())
				pDoc->m_SlideShowThread.PreviousPicture();
			break;

		case VK_NEXT : // Page Down
			if (!((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc, TRUE))
				break;
			if (!pDoc->ViewNextPageFrame())
				pDoc->m_SlideShowThread.NextPicture();
			break;

		case VK_HOME : // Load First Picture
			if (!((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc, TRUE))
				break;
			if (!pDoc->ViewFirstPageFrame())
				pDoc->m_SlideShowThread.FirstPicture();
			break;

		case VK_END : // Load Last Picture
			if (!((CUImagerApp*)::AfxGetApp())->IsDocReadyToSlide(pDoc, TRUE))
				break;
			if (!pDoc->ViewLastPageFrame())
				pDoc->m_SlideShowThread.LastPicture();
			break;

		case VK_DELETE : // Delete Doc
			if ((pDoc->m_dwIDAfterFullLoadCommand != 0) ||
				pDoc->IsModified() || pDoc->m_bMetadataModified ||
				pDoc->m_SlideShowThread.IsSlideshowRunning() ||
				pDoc->m_bDoRestartSlideshow || pDoc->m_pRotationFlippingDlg || pDoc->m_pHLSDlg || 
				pDoc->m_pWndPalette || pDoc->m_bDoRedEyeColorPickup || pDoc->m_pRedEyeDlg ||
				pDoc->m_pMonochromeConversionDlg || pDoc->m_pSharpenDlg || pDoc->m_pSoftenDlg ||
				pDoc->m_pSoftBordersDlg || pDoc->m_bCrop)
				break;
			pDoc->EditDelete();
			break;

		case VK_APPS :
			// Release Capture!
			if (m_bDoScrollMove)
			{
				ReleaseCapture();
				m_bDoScrollMove = FALSE;
			}

			ForceCursor();
			if (pDoc->m_SlideShowThread.IsSlideshowRunning())
			{
				pDoc->m_bDoRestartSlideshow = TRUE;
				pDoc->m_SlideShowThread.PauseSlideshow();
			}
			else
				pDoc->m_bDoRestartSlideshow = FALSE;

			// Cancel Transitions
			pDoc->CancelTransition();
			pDoc->CancelLoadFullJpegTransition();

			if (pDoc->m_bCrop)
				VERIFY(menu.LoadMenu(IDR_CONTEXT_CROP));
			else
			{
				if (m_bFullScreenMode)
					VERIFY(menu.LoadMenu(IDR_CONTEXT_PICTURE_FULLSCREEN));
				else
					VERIFY(menu.LoadMenu(IDR_CONTEXT_PICTURE));
			}
			
			pPopup = menu.GetSubMenu(0);
			ASSERT(pPopup != NULL);
			::AfxGetMainFrame()->SetForegroundWindow(); // Must be MainFrame otherwise menu may pop-under the OSD or the Preview Display in Full-screen!
			ClientToScreen(&Point);
			pDoc->m_bDoUpdateLayeredDlg = FALSE;
			pPopup->TrackPopupMenu(TPM_LEFTBUTTON|TPM_RIGHTBUTTON, Point.x, Point.y, ::AfxGetMainFrame());
			pDoc->m_bDoUpdateLayeredDlg = TRUE;
			if (pDoc->m_pLayeredDlg)
				Draw();

			// Note that the CPictureDoc::OnPlayStop function
			// (which may be called from inside the above TrackPopupMenu)
			// will set m_bDoRestartSlideshow to FALSE, so that the
			// Slideshow is not restarted if the Stop command is selected!
			if (pDoc->m_bDoRestartSlideshow)
			{
				pDoc->m_SlideShowThread.RunSlideshow();
				pDoc->m_bDoRestartSlideshow = FALSE;
			}
			ForceCursor(FALSE);
			break;
	
		default : break;
	}

	CUImagerView::OnKeyDown(nChar, nRepCnt, nFlags);
}

CRect CPictureView::GetVisibleDocRect(CRect rcDocRect, double dZoomFactor) 
{
	CRect rcVisibleDocRect;
	CRect rcClient;
	GetClientRect(&rcClient);

	// Calc. Offset of Visible Picture Area
	CPoint ptPos = GetScrollPosition();
	ptPos -= m_ZoomRect.TopLeft();
	ptPos.x = Round((double)ptPos.x / dZoomFactor);
	ptPos.y = Round((double)ptPos.y / dZoomFactor);
	rcVisibleDocRect = CRect(ptPos.x,
							ptPos.y,
							ptPos.x + Round((double)rcClient.Width() / dZoomFactor),
							ptPos.y + Round((double)rcClient.Height() / dZoomFactor));

	// Inflate
	rcVisibleDocRect.InflateRect(	VISIBLEDOC_INFLATE,
									VISIBLEDOC_INFLATE);

	// Clip
	rcVisibleDocRect.left	= MAX(rcVisibleDocRect.left, rcDocRect.left);
	rcVisibleDocRect.top	= MAX(rcVisibleDocRect.top, rcDocRect.top);
	rcVisibleDocRect.right	= MIN(rcVisibleDocRect.right, rcDocRect.right);
	rcVisibleDocRect.bottom	= MIN(rcVisibleDocRect.bottom, rcDocRect.bottom);

	return rcVisibleDocRect;
}

CRect CPictureView::GetVisibleZoomRect(CRect rcVisibleDocRect, double dZoomFactor) 
{
	rcVisibleDocRect.left = Round((double)rcVisibleDocRect.left * dZoomFactor);
	rcVisibleDocRect.top = Round((double)rcVisibleDocRect.top * dZoomFactor);
	rcVisibleDocRect.right = Round((double)rcVisibleDocRect.right * dZoomFactor);
	rcVisibleDocRect.bottom = Round((double)rcVisibleDocRect.bottom * dZoomFactor);
	rcVisibleDocRect += m_ZoomRect.TopLeft();
	return rcVisibleDocRect;
}

void CPictureView::OnDraw(CDC* pDC) 
{
	Draw(pDC);
}

BOOL CPictureView::Draw(CDC* pDC/*=NULL*/)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CDib* pDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&(pDoc->m_AlphaRenderedDib) :
					pDoc->m_pDib;

	// Do Not Draw?
	if (pDoc->m_bNoDrawing)
		return FALSE;

	// For Cropping a Memory Device Context
	// may be used to avoid flickering
	CDC* pTheDC = pDC;
	BOOL bUseMemDC = FALSE;
	CPoint ptOldWindowOrg;

	// Client Rect
	CRect rcClient;
	GetClientRect(&rcClient);

	// Fast Drawing with Memory DC for Cropping?
	if (pDoc->m_bCrop)
	{
		// Decide whether to use Mem DC
		int bpp = ::AfxGetMainFrame()->GetMonitorBpp();
		if (bpp < 8)
			bpp = 32;
		DWORD dwMemDCSize = DWALIGNEDWIDTHBYTES(rcClient.Width() * bpp) * rcClient.Height();
		if (((CUImagerApp*)::AfxGetApp())->IsPictureSizeBig(dwMemDCSize))
			bUseMemDC = FALSE;
		else
			bUseMemDC = TRUE;

		if (bUseMemDC &&
			(m_rcPrevClient != rcClient || (m_hMemDCDibSection == NULL)))
		{
			// Free
			FreeCropMemDCDrawing();

			if (m_MemDC.CreateCompatibleDC(pDC))
			{
				// Create the New DibSection for Mem DC
				BITMAPINFOHEADER Bmi;
				Bmi.biSize = sizeof(BITMAPINFOHEADER);
				Bmi.biWidth = rcClient.Width();
				Bmi.biHeight = rcClient.Height();
				Bmi.biPlanes = 1;
				Bmi.biBitCount = bpp;
				Bmi.biCompression = BI_RGB;
				Bmi.biSizeImage = dwMemDCSize;
				Bmi.biXPelsPerMeter = 0; 
				Bmi.biYPelsPerMeter = 0; 
				Bmi.biClrUsed = 0; 
				Bmi.biClrImportant = 0;
				m_hMemDCDibSection = ::CreateDIBSection(pDC->GetSafeHdc(),
														(const BITMAPINFO*)&Bmi,
														DIB_RGB_COLORS,
														(void**)&m_pDibSectionBits,
														NULL,
														0);
				if (m_hMemDCDibSection)
				{
					// Select DibSection into Memory DC
					m_hOldMemDCBitmap = (HBITMAP)::SelectObject(m_MemDC.GetSafeHdc(), m_hMemDCDibSection);
				}
				else
					bUseMemDC = FALSE;
			}
			else
				bUseMemDC = FALSE;
		}

		// Use Mem DC?
		if (bUseMemDC)
		{
			ptOldWindowOrg = m_MemDC.SetWindowOrg(GetScrollPosition());
			pTheDC = &m_MemDC;
		}
	}

	// Erase Bkg
	EraseBkgnd(pTheDC);

	if ((pDoc->m_GifAnimationThread.m_dwDibAnimationCount > 1) &&
		pDoc->m_GifAnimationThread.IsAlive())
	{
		CDib* pGifDib;
		if (pDoc->m_GifAnimationThread.m_AlphaRenderedDibAnimationArray.GetSize() > 0)
			pGifDib = pDoc->m_GifAnimationThread.m_AlphaRenderedDibAnimationArray.GetAt
							(pDoc->m_GifAnimationThread.m_dwDibAnimationPos);
		else
			pGifDib = pDoc->m_GifAnimationThread.m_DibAnimationArray.GetAt
							(pDoc->m_GifAnimationThread.m_dwDibAnimationPos);
		if (pDoc->m_bStretchModeHalftone)
			pGifDib->SetStretchMode(HALFTONE);
		else
			pGifDib->SetStretchMode(COLORONCOLOR);
		pGifDib->Paint(	pTheDC->GetSafeHdc(),
						m_ZoomRect,
						pDoc->m_DocRect,
						FALSE);

		// Update Layered Image
		pDoc->UpdateLayeredDlg(	pDoc->m_GifAnimationThread.m_DibAnimationArray.GetAt
								(pDoc->m_GifAnimationThread.m_dwDibAnimationPos));
	}
	else
	{
		if (pDib->GetPreviewDib() && pDib->GetPreviewDib()->IsValid())
		{	
			// Load and Draw Full Jpeg Transition
			// Note: it may happen that the transition thread
			// finishes and clears m_bLoadFullJpegTransitionWorker
			// but the UI thread couldn't reach this point ->
			// check the m_bLoadFullJpegTransitionWorkerDone flag!
			if (pDoc->m_bLoadFullJpegTransitionUI			&&	// (Set in JPEGGet())
				((pDoc->m_bFirstLoadFullJpegTransition		&&	// (Set in JPEGGet())
				pDoc->m_bLoadFullJpegTransitionWorkerDone)	||	// Transition already done
				pDoc->m_bLoadFullJpegTransitionWorker))			// JPEG thread making transition dib
			{
				DrawLoadFullJpegTransition(pTheDC);
			}
			// Transition
			else if (pDoc->m_bTransitionUI	&&
					pDoc->m_bTransitionWorker)
			{
				DrawTransition(pTheDC, pDib->GetPreviewDib());
			}
			// Crop
			else if (pDoc->m_bCrop)
			{
				// Darker-Outer Background Image
				DrawCropBkgImage(pTheDC, bUseMemDC);

				// Create the Crop Zoom Rectangle Region
				CRgn rgnCropZoomRect;
				CRect rcCropZoomRect;
				rcCropZoomRect = m_CropZoomRect;
				if (IsXOrYScroll())
					rcCropZoomRect.OffsetRect(-GetScrollPosition());
				rgnCropZoomRect.CreateRectRgnIndirect(&rcCropZoomRect);

				// Select Region into DC
				if (pTheDC)
					pTheDC->SelectClipRgn(&rgnCropZoomRect);

				// Preview Doc Rect
				CRect PreviewDocRect = CRect(0, 0,
											pDib->GetPreviewDib()->GetWidth(),
											pDib->GetPreviewDib()->GetHeight());

				// Set Stretch Mode
				if (pDoc->m_bStretchModeHalftone)
					pDib->GetPreviewDib()->SetStretchMode(HALFTONE);
				else
					pDib->GetPreviewDib()->SetStretchMode(COLORONCOLOR);

				// Draw Cropped Image
				if (!IsXOrYScroll())
				{
					pDib->GetPreviewDib()->Paint(	pTheDC->GetSafeHdc(),
													m_ZoomRect,
													PreviewDocRect,
													FALSE);
				}
				else
				{
					double dScale =	pDib->GetPreviewDibRatio();
					CRect rcVisibleDocRect = GetVisibleDocRect(	PreviewDocRect,
																pDoc->m_dZoomFactor * dScale);
					CRect rcVisibleZoomRect = GetVisibleZoomRect(rcVisibleDocRect,
																pDoc->m_dZoomFactor * dScale);
					pDib->GetPreviewDib()->Paint(	pTheDC->GetSafeHdc(),
													rcVisibleZoomRect,
													rcVisibleDocRect,
													FALSE);
				}

				// Remove Region from DC
				if (pTheDC)
					pTheDC->SelectClipRgn(NULL);

				// Draw Crop Tools
				DrawCropTools(pTheDC);

				// Update Layered Image
				pDoc->UpdateLayeredDlg(pDoc->m_pDib->GetPreviewDib());
			}
			// Normal Painting
			else
			{
				// Preview Doc Rect
				CRect PreviewDocRect = CRect(	0, 0,
												pDib->GetPreviewDib()->GetWidth(),
												pDib->GetPreviewDib()->GetHeight());

				// Set Stretch Mode
				if (pDoc->m_bStretchModeHalftone)
					pDib->GetPreviewDib()->SetStretchMode(HALFTONE);
				else
					pDib->GetPreviewDib()->SetStretchMode(COLORONCOLOR);

				// Paint Image
				if (!IsXOrYScroll())
				{
					pDib->GetPreviewDib()->Paint(pTheDC->GetSafeHdc(),
												m_ZoomRect,
												PreviewDocRect,
												FALSE);
				}
				else
				{
					double dScale = pDib->GetPreviewDibRatio();
					CRect rcVisibleDocRect = GetVisibleDocRect(		PreviewDocRect,
																	pDoc->m_dZoomFactor * dScale);
					CRect rcVisibleZoomRect = GetVisibleZoomRect(	rcVisibleDocRect,
																	pDoc->m_dZoomFactor * dScale);
					pDib->GetPreviewDib()->Paint(	pTheDC->GetSafeHdc(),
													rcVisibleZoomRect,
													rcVisibleDocRect,
													FALSE);
				}

				// Update Layered Image
				if (pDoc->m_pHLSDlg)
					pDoc->UpdateLayeredDlg(pDib->GetPreviewDib()); // 8 bpp HLS preview (alpha rendered if it has transparency)
				else
					pDoc->UpdateLayeredDlg(pDoc->m_pDib->GetPreviewDib());
			}
		}
		else if (pDib && pDib->IsValid())
		{
			// Load and Draw Full Jpeg Transition
			// Note: it may happen that the transition thread
			// finishes and clears m_bLoadFullJpegTransitionWorker
			// but the UI thread couldn't reach this point ->
			// check the m_bLoadFullJpegTransitionWorkerDone flag!
			if (pDoc->m_bLoadFullJpegTransitionUI			&&	// (Set in JPEGGet())
				((pDoc->m_bFirstLoadFullJpegTransition		&&	// (Set in JPEGGet())
				pDoc->m_bLoadFullJpegTransitionWorkerDone)	||	// Transition already done
				pDoc->m_bLoadFullJpegTransitionWorker))			// JPEG thread making transition dib
			{
				DrawLoadFullJpegTransition(pTheDC);
			}
			// Transition
			else if (pDoc->m_bTransitionUI &&
					pDoc->m_bTransitionWorker)
			{
				DrawTransition(pTheDC, pDib);
			}
			// Crop
			else if (pDoc->m_bCrop)
			{
				// Darker-Outer Background Image
				DrawCropBkgImage(pTheDC, bUseMemDC);

				// Create the Crop Zoom Rectangle Region
				CRgn rgnCropZoomRect;
				CRect rcCropZoomRect;
				rcCropZoomRect = m_CropZoomRect;
				if (IsXOrYScroll())
					rcCropZoomRect.OffsetRect(-GetScrollPosition());
				rgnCropZoomRect.CreateRectRgnIndirect(&rcCropZoomRect);

				// Select Region into DC
				if (pTheDC)
					pTheDC->SelectClipRgn(&rgnCropZoomRect);					

				// Set Stretch Mode
				if (pDoc->m_bStretchModeHalftone)
					pDib->SetStretchMode(HALFTONE);
				else
					pDib->SetStretchMode(COLORONCOLOR);

				// Draw Cropped Image
				if (!IsXOrYScroll())
				{
					pDib->Paint(pTheDC->GetSafeHdc(),
								m_ZoomRect,
								pDoc->m_DocRect,
								FALSE);
				}
				else
				{
					CRect rcVisibleDocRect = GetVisibleDocRect(pDoc->m_DocRect, pDoc->m_dZoomFactor);
					CRect rcVisibleZoomRect = GetVisibleZoomRect(rcVisibleDocRect, pDoc->m_dZoomFactor);
					pDib->Paint(pTheDC->GetSafeHdc(),
								rcVisibleZoomRect,
								rcVisibleDocRect,
								FALSE);
				}

				// Remove Region from DC
				if (pTheDC)
					pTheDC->SelectClipRgn(NULL);

				// Draw Crop Tools
				DrawCropTools(pTheDC);

				// Update Layered Image
				pDoc->UpdateLayeredDlg(pDoc->m_pDib);
			}
			// Normal Painting
			else
			{
				// Set Stretch Mode
				if (pDoc->m_bStretchModeHalftone)
					pDib->SetStretchMode(HALFTONE);
				else
					pDib->SetStretchMode(COLORONCOLOR);

				// Paint Image
				if (!IsXOrYScroll())
				{
					pDib->Paint(pTheDC->GetSafeHdc(),
								m_ZoomRect,
								pDoc->m_DocRect,
								FALSE);
				}
				else
				{
					CRect rcVisibleDocRect = GetVisibleDocRect(pDoc->m_DocRect, pDoc->m_dZoomFactor);
					CRect rcVisibleZoomRect = GetVisibleZoomRect(rcVisibleDocRect, pDoc->m_dZoomFactor);
					pDib->Paint(pTheDC->GetSafeHdc(),
								rcVisibleZoomRect,
								rcVisibleDocRect,
								FALSE);
				}

				// Update Layered Image
				pDoc->UpdateLayeredDlg(pDoc->m_pDib);
			}
		}
	}

	// BitBlt MemDC to Screen DC
	if (pDoc->m_bCrop && bUseMemDC)
	{	
		// Restore Window Origin
		m_MemDC.SetWindowOrg(ptOldWindowOrg);

		// Copy Visible Bits
		::BitBlt(pDC->GetSafeHdc(),
				GetScrollPosition().x,
				GetScrollPosition().y,
				rcClient.Width(), 
				rcClient.Height(),
				m_MemDC.GetSafeHdc(),
				0,
				0,
				SRCCOPY);

		m_rcPrevClient = rcClient;
	}
	else
		m_rcPrevClient = CRect(0,0,0,0);

	return TRUE;
}

void CPictureView::DrawTransition(CDC* pDC, CDib* pDib)
{
	// Check
	if (!pDC)
		return;

	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Check
	if (!pDib || !pDib->IsValid())
	{
		pDoc->CancelTransition();
		goto ReDrawTransition;
	}

	// Do not do Transitions if scrolls are present,
	// (This is also to avoid big DibSections,
	// if zooming for example)
	if (!IsXOrYScroll())
	{
		// Enter CS
		if (!pDoc->m_csTransition.EnterCriticalSection(CS_TRANSITION_TIMEOUT))
			goto ReDrawTransition;

		// Do Transition?
		if (pDoc->m_bTransitionUI		&&
			pDoc->m_bTransitionWorker	&&
			m_hTransitionMemDC)
		{
			// BitBlt
			::BitBlt(pDC->GetSafeHdc(),
					m_ZoomRect.left,
					m_ZoomRect.top,
					m_ZoomRect.Width(),
					m_ZoomRect.Height(),		
					m_hTransitionMemDC,
					0,
					0,
					SRCCOPY);
			
			// Flush the GDI batch queue 
			::GdiFlush();
		}

		// Leave CS
		pDoc->m_csTransition.LeaveCriticalSection();
	}
	else
		pDoc->CancelTransition();
		
		// If Not Finished -> return
	if (pDoc->m_bTransitionUI &&
		pDoc->m_bTransitionWorker)
		return;

	// Otherwise Re-Draw
ReDrawTransition:
	::PostMessage(	GetSafeHwnd(),
					WM_THREADSAFE_UPDATEWINDOWSIZES,
					(WPARAM)UPDATEWINDOWSIZES_INVALIDATE,
					(LPARAM)0);
	::Sleep(0); // Switch to Worker Thread
}

#ifdef _DEBUG
#define DRAWLOADFULLJPEGTRANS_CHANGEPOINTER	if (!pDoc->m_csLoadFullJpegDib.EnterCriticalSection(CS_LOADFULLJPEGDIB_TIMEOUT))\
												goto ReDrawLoadFullJpegTransition;\
											ASSERT(pDoc->m_pLoadFullJpegDib && pDoc->m_pLoadFullJpegDib->IsValid());\
											delete pDoc->m_pDib;\
											pDoc->m_pDib = pDoc->m_pLoadFullJpegDib;\
											pDoc->m_pLoadFullJpegDib = NULL;\
											pDoc->m_csLoadFullJpegDib.LeaveCriticalSection();\
											::PostMessage(	pDoc->GetView()->GetSafeHwnd(),\
															WM_THREADSAFE_SETDOCUMENTTITLE,\
															(WPARAM)0, (LPARAM)0)
#else
#define DRAWLOADFULLJPEGTRANS_CHANGEPOINTER	if (!pDoc->m_csLoadFullJpegDib.EnterCriticalSection(CS_LOADFULLJPEGDIB_TIMEOUT))\
												goto ReDrawLoadFullJpegTransition;\
											delete pDoc->m_pDib;\
											pDoc->m_pDib = pDoc->m_pLoadFullJpegDib;\
											pDoc->m_pLoadFullJpegDib = NULL;\
											pDoc->m_csLoadFullJpegDib.LeaveCriticalSection()
#endif
void CPictureView::DrawLoadFullJpegTransition(CDC* pDC)
{
	// Check
	if (!pDC)
		return;

	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// If Cancel Transition
	if (pDoc->m_bCancelLoadFullJpegTransition)
	{
		if (pDoc->m_bFirstLoadFullJpegTransition)
		{
			// Change Pointer
			DRAWLOADFULLJPEGTRANS_CHANGEPOINTER;

			// Reset Var
			pDoc->m_bFirstLoadFullJpegTransition = FALSE;

			// Start Pending Command?
			if (pDoc->m_dwIDAfterFullLoadCommand)
			{
				PostMessage(WM_COMMAND,
							pDoc->m_dwIDAfterFullLoadCommand);
				pDoc->m_dwIDAfterFullLoadCommand = 0;
				EndWaitCursor();
				ForceCursor(FALSE);
			}
		}
		pDoc->m_bLoadFullJpegTransitionUI = FALSE;
	}
	// Nothing to Do
	else if (	pDoc->m_pDib			&&
				pDoc->m_pDib->IsValid()	&&
				pDoc->m_bFirstLoadFullJpegTransition)
	{
		// Delete Pointer
		if (!pDoc->m_csLoadFullJpegDib.EnterCriticalSection(CS_LOADFULLJPEGDIB_TIMEOUT))
			goto ReDrawLoadFullJpegTransition;
		if (pDoc->m_pLoadFullJpegDib)
		{
			delete pDoc->m_pLoadFullJpegDib; 
			pDoc->m_pLoadFullJpegDib = NULL;
		}
		pDoc->m_csLoadFullJpegDib.LeaveCriticalSection();

		// Done
		pDoc->m_bFirstLoadFullJpegTransition = FALSE;
		pDoc->m_bLoadFullJpegTransitionUI = FALSE;

		// Start Pending Command?
		if (pDoc->m_dwIDAfterFullLoadCommand)
		{
			PostMessage(WM_COMMAND,
						pDoc->m_dwIDAfterFullLoadCommand);
			pDoc->m_dwIDAfterFullLoadCommand = 0;
			EndWaitCursor();
			ForceCursor(FALSE);
		}
	}
	else
	{
		// Do not do Transitions if scrolls are present!
		// (This is also to avoid big DibSections,
		// if zooming for example)
		if (!IsXOrYScroll())
		{	
			if (pDoc->m_bFirstLoadFullJpegTransition)
			{
				// Change Pointer
				DRAWLOADFULLJPEGTRANS_CHANGEPOINTER;

				// Reset Var
				pDoc->m_bFirstLoadFullJpegTransition = FALSE;

				// Start Pending Command?
				if (pDoc->m_dwIDAfterFullLoadCommand)
				{
					PostMessage(WM_COMMAND,
								pDoc->m_dwIDAfterFullLoadCommand);
					pDoc->m_dwIDAfterFullLoadCommand = 0;
					EndWaitCursor();
					ForceCursor(FALSE);
					pDoc->m_bLoadFullJpegTransitionUI = FALSE;
				}
			}
			
			// Enter CS
			if (!pDoc->m_csLoadFullJpegDib.EnterCriticalSection(CS_LOADFULLJPEGDIB_TIMEOUT))
				goto ReDrawLoadFullJpegTransition;

			// Do Full Jpeg Transition?
			if (pDoc->m_bLoadFullJpegTransitionUI		&&
				pDoc->m_bLoadFullJpegTransitionWorker	&&
				m_hLoadFullJpegTransitionMemDC)
			{
				// BitBlt
				::BitBlt(	pDC->GetSafeHdc(),
							m_ZoomRect.left,
							m_ZoomRect.top,
							m_ZoomRect.Width(),
							m_ZoomRect.Height(),		
							m_hLoadFullJpegTransitionMemDC,
							0,
							0,
							SRCCOPY);

				// Flush the GDI batch queue 
				::GdiFlush();
			}

			// Leave CS
			pDoc->m_csLoadFullJpegDib.LeaveCriticalSection();
		}
		else
		{
			if (pDoc->m_bFirstLoadFullJpegTransition)
			{
				// Change Pointer
				DRAWLOADFULLJPEGTRANS_CHANGEPOINTER;

				// Reset Var
				pDoc->m_bFirstLoadFullJpegTransition = FALSE;

				// Start Pending Command?
				if (pDoc->m_dwIDAfterFullLoadCommand)
				{
					PostMessage(WM_COMMAND,
								pDoc->m_dwIDAfterFullLoadCommand);
					pDoc->m_dwIDAfterFullLoadCommand = 0;
					EndWaitCursor();
					ForceCursor(FALSE);
				}
			}
			pDoc->m_bLoadFullJpegTransitionUI = FALSE;
		}
	}
	
	// If Not finished the jpeg thread posts messages
	if (pDoc->m_bLoadFullJpegTransitionUI &&
		pDoc->m_bLoadFullJpegTransitionWorker)
		return;

	// Otherwise Re-Draw
ReDrawLoadFullJpegTransition:
	::PostMessage(	GetSafeHwnd(),
					WM_THREADSAFE_UPDATEWINDOWSIZES,
					(WPARAM)UPDATEWINDOWSIZES_INVALIDATE,
					(LPARAM)0);
	::Sleep(0); // Switch to Worker Thread
}

void CPictureView::SetCursorPosInsideClientRect(CPoint point)
{
	// From view to client coordinates
	point -= GetScrollPosition();

	// Keep the cursor in the client area
	CRect rcClient; 
	GetClientRect(&rcClient);
	if (point.x >= rcClient.right)
		point.x = rcClient.right - 1;
	else if (point.x < rcClient.left)
		point.x = rcClient.left;
	if (point.y >= rcClient.bottom) 
		point.y = rcClient.bottom - 1; 
	else if (point.y < rcClient.top) 
		point.y = rcClient.top;

	// Set new cursor position
	ClientToScreen(&point); 
	::SetCursorPos(point.x, point.y);

	// Send WM_MOUSEMOVE
	ScreenToClient(&point);
	SendMessage(WM_MOUSEMOVE, 0, (LPARAM)MAKELONG(point.x, point.y));
}

void CPictureView::DrawCropTools(CDC* pDC)
{
	// Check
	if (!pDC)
		return;

	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Draw Original Image Rect
	if (m_CropZoomRect != m_ZoomRect)
	{
		CPen Pen;
		Pen.CreatePen(PS_DASH, 1, RGB(0x80,0x80,0x80));
		CPen* pOldPen = pDC->SelectObject(&Pen);
		CBrush* pOldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
		pDC->Rectangle(m_ZoomRect);
		pDC->SelectObject(pOldBrush);
		pDC->SelectObject(pOldPen);
		Pen.DeleteObject();
	}

	// Draw Crop Points
	int nCropMarkerRectWidth = ::SystemDPIScale(CROP_MARKER_RECT_WIDTH);
	int nCropMarkerRectHeight = ::SystemDPIScale(CROP_MARKER_RECT_HEIGHT);
	CBrush Brush;
	Brush.CreateSolidBrush(RGB(0x80,0x80,0x80));
	CBrush* pOldBrush = pDC->SelectObject(&Brush);
	CPen Pen;
	Pen.CreatePen(PS_SOLID, 1, RGB(0,0,0));
	CPen* pOldPen = pDC->SelectObject(&Pen);
	CRect rcTopLeft(		m_CropZoomRect.left - nCropMarkerRectWidth,
							m_CropZoomRect.top - nCropMarkerRectHeight,
							m_CropZoomRect.left,
							m_CropZoomRect.top);

	CRect rcBottomRight(	m_CropZoomRect.right,
							m_CropZoomRect.bottom,
							m_CropZoomRect.right + nCropMarkerRectWidth,
							m_CropZoomRect.bottom + nCropMarkerRectHeight);

	CRect rcBottomLeft(		m_CropZoomRect.left - nCropMarkerRectWidth,
							m_CropZoomRect.bottom,
							m_CropZoomRect.left,
							m_CropZoomRect.bottom + nCropMarkerRectHeight);

	CRect rcTopRight(		m_CropZoomRect.right,
							m_CropZoomRect.top - nCropMarkerRectHeight,
							m_CropZoomRect.right + nCropMarkerRectWidth,
							m_CropZoomRect.top);

	CRect rcLeft(			m_CropZoomRect.left - nCropMarkerRectWidth,
							m_CropZoomRect.top + (m_CropZoomRect.Height()/2) - (nCropMarkerRectHeight/2),
							m_CropZoomRect.left,
							m_CropZoomRect.top + (m_CropZoomRect.Height()/2) + (nCropMarkerRectHeight/2));

	CRect rcRight(			m_CropZoomRect.right,
							m_CropZoomRect.top + (m_CropZoomRect.Height()/2) - (nCropMarkerRectHeight/2),
							m_CropZoomRect.right + nCropMarkerRectWidth,
							m_CropZoomRect.top + (m_CropZoomRect.Height()/2) + (nCropMarkerRectHeight/2));
	
	CRect rcTop(			m_CropZoomRect.left + (m_CropZoomRect.Width()/2) - (nCropMarkerRectWidth/2),
							m_CropZoomRect.top - nCropMarkerRectHeight,
							m_CropZoomRect.left + (m_CropZoomRect.Width()/2) + (nCropMarkerRectWidth/2),
							m_CropZoomRect.top);

	CRect rcBottom(			m_CropZoomRect.left + (m_CropZoomRect.Width()/2) - (nCropMarkerRectWidth/2),
							m_CropZoomRect.bottom,
							m_CropZoomRect.left + (m_CropZoomRect.Width()/2) + (nCropMarkerRectWidth/2),
							m_CropZoomRect.bottom + nCropMarkerRectHeight);
	
	CRect rcCenter(			m_CropZoomRect.left + (m_CropZoomRect.Width()/2) - (nCropMarkerRectWidth/2),
							m_CropZoomRect.top + (m_CropZoomRect.Height()/2) - (nCropMarkerRectHeight/2),
							m_CropZoomRect.left + (m_CropZoomRect.Width()/2) + (nCropMarkerRectWidth/2),
							m_CropZoomRect.top + (m_CropZoomRect.Height()/2) + (nCropMarkerRectHeight/2));

	pDC->Rectangle(rcTopLeft);
	pDC->Rectangle(rcBottomRight);
	pDC->Rectangle(rcBottomLeft);
	pDC->Rectangle(rcTopRight);
	pDC->Rectangle(rcLeft);
	pDC->Rectangle(rcRight);
	pDC->Rectangle(rcTop);
	pDC->Rectangle(rcBottom);
	pDC->Rectangle(rcCenter);
	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
	Brush.DeleteObject();
	Pen.DeleteObject();

	// Frame Around Crop Rectangle
	Pen.CreatePen(PS_DASH, 1, RGB(0x80,0x80,0x80));
	pOldPen = pDC->SelectObject(&Pen);
	pOldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
	pDC->Rectangle(m_CropZoomRect);
	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
	Pen.DeleteObject();
}

void CPictureView::DrawCropBkgImage(CDC* pDC, BOOL bUseMemDC)
{
	// Check
	if (!pDC)
		return;

	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CDib* pDib =	(pDoc->m_pDib->HasAlpha() && pDoc->m_pDib->GetBitCount() == 32) ?
					&(pDoc->m_AlphaRenderedDib) :
					pDoc->m_pDib;

	if (m_CropZoomRect != m_ZoomRect)
	{
		// Rectangles
		CRect rcTop(m_ZoomRect.left, m_ZoomRect.top, m_ZoomRect.right, m_CropZoomRect.top);
		CRect rcLeft(m_ZoomRect.left, m_CropZoomRect.top, m_CropZoomRect.left, m_CropZoomRect.bottom);
		CRect rcRight(m_CropZoomRect.right, m_CropZoomRect.top, m_ZoomRect.right, m_CropZoomRect.bottom);
		CRect rcBottom(m_ZoomRect.left, m_CropZoomRect.bottom, m_ZoomRect.right, m_ZoomRect.bottom);
		CRect rcDocTop(pDoc->m_DocRect.left, pDoc->m_DocRect.top, pDoc->m_DocRect.right, pDoc->m_CropDocRect.top);
		CRect rcDocLeft(pDoc->m_DocRect.left, pDoc->m_CropDocRect.top, pDoc->m_CropDocRect.left, pDoc->m_CropDocRect.bottom);
		CRect rcDocRight(pDoc->m_CropDocRect.right, pDoc->m_CropDocRect.top, pDoc->m_DocRect.right, pDoc->m_CropDocRect.bottom);
		CRect rcDocBottom(pDoc->m_DocRect.left, pDoc->m_CropDocRect.bottom, pDoc->m_DocRect.right, pDoc->m_DocRect.bottom);

		// Offset
		if (IsXOrYScroll())
		{
			rcTop.OffsetRect(-GetScrollPosition());
			rcLeft.OffsetRect(-GetScrollPosition());
			rcRight.OffsetRect(-GetScrollPosition());
			rcBottom.OffsetRect(-GetScrollPosition());
		}

		// Create the 4 Regions (top, left, right and bottom)
		// and make a union of them
		CRgn rgnTop, rgnLeft, rgnRight, rgnBottom, rgnTopLeft, rgnTopLeftBottom, rgnTopLeftBottomRight;
		rgnTop.CreateRectRgnIndirect(&rcTop);
		rgnLeft.CreateRectRgnIndirect(&rcLeft);
		rgnRight.CreateRectRgnIndirect(&rcRight);
		rgnBottom.CreateRectRgnIndirect(&rcBottom);
		rgnTopLeft.CreateRectRgn(0,0,0,0);
		rgnTopLeftBottom.CreateRectRgn(0,0,0,0);
		rgnTopLeftBottomRight.CreateRectRgn(0,0,0,0);
		rgnTopLeft.CombineRgn(&rgnTop, &rgnLeft, RGN_OR);
		rgnTopLeftBottom.CombineRgn(&rgnTopLeft, &rgnBottom, RGN_OR);
		rgnTopLeftBottomRight.CombineRgn(&rgnTopLeftBottom, &rgnRight, RGN_OR);

		// Select Region into DC
		pDC->SelectClipRgn(&rgnTopLeftBottomRight);

		// Set Stretch Mode
		if (pDoc->m_bStretchModeHalftone)
			pDoc->m_pCropBkgDib->SetStretchMode(HALFTONE);
		else
			pDoc->m_pCropBkgDib->SetStretchMode(COLORONCOLOR);

		// Doc Rectangle
		CRect DocRect = CRect(	0,
								0,
								pDoc->m_pCropBkgDib->GetWidth(),
								pDoc->m_pCropBkgDib->GetHeight());

		// Draw
		if (!IsXOrYScroll())
		{
			pDoc->m_pCropBkgDib->Paint(	pDC->GetSafeHdc(),
										m_ZoomRect,
										DocRect,
										FALSE);
		}
		else
		{
			double dScale = 1.0;
			if (pDib->GetPreviewDib() &&
				pDib->GetPreviewDib()->IsValid())
				dScale = pDib->GetPreviewDibRatio();
			CRect rcVisibleDocRect = GetVisibleDocRect(		DocRect,
															pDoc->m_dZoomFactor * dScale);
			CRect rcVisibleZoomRect = GetVisibleZoomRect(	rcVisibleDocRect,
															pDoc->m_dZoomFactor * dScale);
			pDoc->m_pCropBkgDib->Paint(	pDC->GetSafeHdc(),
										rcVisibleZoomRect,
										rcVisibleDocRect,
										FALSE);
		}

		// Remove Region from DC
		pDC->SelectClipRgn(NULL);
	}
}

void CPictureView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	UpdateWindowSizes(TRUE, TRUE, FALSE);
}

void CPictureView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->m_bZoomTool)
	{
		m_bOldZoomToolMinus = pDoc->m_bZoomToolMinus;
		pDoc->m_bZoomToolMinus = TRUE;
		UpdateCursor();
		ZoomPoint(point, pDoc->m_bZoomToolMinus);
		if (IsXOrYScroll())
		{
			m_ptStartScrollMoveClickPos = point;
			m_ptStartScrollMovePos = GetScrollPosition();
			m_bDoScrollMove = TRUE;
			SetCapture();
		}
		return;
	}
	else
	{
		// Release Capture!
		if (m_bDoScrollMove)
		{
			ReleaseCapture();
			m_bDoScrollMove = FALSE;
		}

		if (pDoc->m_bDoRedEyeColorPickup)
		{	
			pDoc->m_bDoRedEyeColorPickup = FALSE;
			ForceCursor(FALSE);
			::AfxGetMainFrame()->StatusText();
			return;
		}
		else if (pDoc->m_bDoRotationColorPickup)
		{
			pDoc->m_bDoRotationColorPickup = FALSE;
			return;
		}
	}

	// Context Menu
	ForceCursor();

	// Store position used by OnEditPasteIntoTopLeft(), OnEditPasteIntoTopRight(), ...
	m_ptLastRightClick = point;

	if (pDoc->m_SlideShowThread.IsSlideshowRunning())
	{
		pDoc->m_bDoRestartSlideshow = TRUE;
		pDoc->m_SlideShowThread.PauseSlideshow();
	}
	else
		pDoc->m_bDoRestartSlideshow = FALSE;

	// Cancel Transitions
	pDoc->CancelTransition();
	pDoc->CancelLoadFullJpegTransition();

	CMenu menu;
	BOOL bPopupOsd = FALSE;
	if (pDoc->m_pOsdDlg)
	{
		CRect rcOsdDlg;
		CPoint pt = point;
		ClientToScreen(&pt);
		pDoc->m_pOsdDlg->GetWindowRect(&rcOsdDlg);
		bPopupOsd = rcOsdDlg.PtInRect(pt);
	}
	if (bPopupOsd)
	{
		pDoc->m_pOsdDlg->ForceShow();
		VERIFY(menu.LoadMenu(IDR_CONTEXT_OSD));
	}
	else if (pDoc->m_bCrop)
		VERIFY(menu.LoadMenu(IDR_CONTEXT_CROP));
	else if (m_bFullScreenMode)
		VERIFY(menu.LoadMenu(IDR_CONTEXT_PICTURE_FULLSCREEN));
	else
		VERIFY(menu.LoadMenu(IDR_CONTEXT_PICTURE));
		
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	::AfxGetMainFrame()->SetForegroundWindow(); // Must be MainFrame otherwise menu may pop-under the OSD or the Preview Display in Full-screen!
	ClientToScreen(&point);
	pDoc->m_bDoUpdateLayeredDlg = FALSE;
	pPopup->TrackPopupMenu(TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, ::AfxGetMainFrame());
	pDoc->m_bDoUpdateLayeredDlg = TRUE;
	if (pDoc->m_pLayeredDlg)
		Draw();

	CUImagerView::OnRButtonDown(nFlags, point);

	if (pDoc->m_pOsdDlg && bPopupOsd)
	{
		pDoc->m_pOsdDlg->ForceShow(FALSE);
		
		// Set Focus to Main Frame
		::SetActiveWindow(::AfxGetMainFrame()->GetSafeHwnd());
	}

	// Note that the CPictureDoc::OnPlayStop function
	// (which may be called from inside the above TrackPopupMenu)
	// will set m_bDoRestartSlideshow to FALSE, so that the
	// Slideshow is not restarted if the Stop command is selected!
	if (pDoc->m_bDoRestartSlideshow)
	{
		pDoc->m_SlideShowThread.RunSlideshow();
		pDoc->m_bDoRestartSlideshow = FALSE;
	}

	ForceCursor(FALSE);
}

void CPictureView::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->m_bZoomTool)
	{
		m_bOldZoomToolMinus = pDoc->m_bZoomToolMinus;
		pDoc->m_bZoomToolMinus = TRUE;
		UpdateCursor();
		ZoomPoint(point, pDoc->m_bZoomToolMinus);
		if (IsXOrYScroll())
		{
			m_ptStartScrollMoveClickPos = point;
			m_ptStartScrollMovePos = GetScrollPosition();
			m_bDoScrollMove = TRUE;
			SetCapture();
		}
	}
	
	CUImagerView::OnRButtonDblClk(nFlags, point);
}

void CPictureView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	if (m_bDoScrollMove)
	{
		ReleaseCapture();
		m_bDoScrollMove = FALSE;
	}

	if (pDoc->m_bZoomTool)
	{
		pDoc->m_bZoomToolMinus = m_bOldZoomToolMinus;
		UpdateCursor();
	}
	
	CUImagerView::OnRButtonUp(nFlags, point);
}

void CPictureView::OnTimer(UINT nIDEvent) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	switch (nIDEvent)
	{
		case ID_TIMER_CROP_SCROLL :
		{
			DoAutoScroll();
			break;
		}
		default:
			break;
	}

	CUImagerView::OnTimer(nIDEvent);
}

void CPictureView::OnInitialUpdate() 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Set Pointers
	pDoc->SetView(this);
	pDoc->SetFrame((CPictureChildFrame*)GetParentFrame());

	// Set Initial Tab Title
	pDoc->m_sTabTitle = pDoc->GetTitle();

	// Call Base Class Implementation
	CUImagerView::OnInitialUpdate();
	
	// Update Control
	CColorButtonPicker* pBkgColorButtonPicker =
		&(((CPictureToolBar*)((CToolBarChildFrame*)GetParentFrame())
							->GetToolBar())->m_BkgColorButtonPicker);
	pBkgColorButtonPicker->SetColor(pDoc->m_crBackgroundColor);
	pBkgColorButtonPicker->SetDefaultColor(::GetSysColor(COLOR_WINDOW));

	// Set Window Pointer
	pDoc->m_FileFind.SetWnd(GetSafeHwnd());
}

BOOL CPictureView::OnEraseBkgnd(CDC* pDC) 
{
	EraseBkgnd(pDC);
	return TRUE;
}

void CPictureView::EraseBkgnd(CDC* pDC)
{
	// Check
	if (!pDC)
		return;

	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CRect rcClient, rcFull;
	GetClientRect(&rcClient);
	CSize TotalSize = GetTotalSize();
	rcFull.left = 0;
	rcFull.top = 0;
	if (TotalSize.cx > rcClient.right)
	{
		if (pDoc->m_bNoBorders)
			rcFull.right = TotalSize.cx;
		else	
			rcFull.right = TotalSize.cx + 2 * ::SystemDPIScale(MIN_BORDER);
	}
	else
		rcFull.right = rcClient.right;
	if (TotalSize.cy > rcClient.bottom)
	{
		if (pDoc->m_bNoBorders)
			rcFull.bottom = TotalSize.cy;
		else
			rcFull.bottom = TotalSize.cy + 2 * ::SystemDPIScale(MIN_BORDER);
	}
	else
		rcFull.bottom = rcClient.bottom;

	// Erase Bkg
	//   The FillRect() includes the left and top borders,
	//   but excludes the right and bottom borders of the rectangle!
	CRect rcTop(rcFull.left, rcFull.top, rcFull.right, m_ZoomRect.top);
	CRect rcLeft(rcFull.left, m_ZoomRect.top, m_ZoomRect.left, m_ZoomRect.bottom);
	CRect rcRight = CRect(m_ZoomRect.right, m_ZoomRect.top, rcFull.right, m_ZoomRect.bottom);
	CRect rcBottom = CRect(rcFull.left, m_ZoomRect.bottom, rcFull.right, rcFull.bottom);
	CBrush br;
	if (pDoc->m_bImageBackgroundColor)
		br.CreateSolidBrush(pDoc->m_crImageBackgroundColor);
	else
		br.CreateSolidBrush(pDoc->m_crBackgroundColor);
	if ((rcTop.Width() > 0) && (rcTop.Height() > 0))
		pDC->FillRect(rcTop, &br);
	if ((rcLeft.Width() > 0) && (rcLeft.Height() > 0))
		pDC->FillRect(rcLeft, &br);
	if ((rcRight.Width() > 0) && (rcRight.Height() > 0))
		pDC->FillRect(rcRight, &br);
	if ((rcBottom.Width() > 0) && (rcBottom.Height() > 0))
		pDC->FillRect(rcBottom, &br);
	br.DeleteObject();
}

void CPictureView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CUImagerView::OnLButtonDown(nFlags, point);

	EnableCursor();

	if (pDoc->m_bZoomTool)
	{
		ZoomPoint(point, pDoc->m_bZoomToolMinus);
		if (IsXOrYScroll())
		{
			m_ptStartScrollMoveClickPos = point;
			m_ptStartScrollMovePos = GetScrollPosition();
			m_bDoScrollMove = TRUE;
			SetCapture();
		}
	}
	else if (pDoc->m_bCrop)
	{
		int nRectSizeXInside = ::SystemDPIScale(CROP_RECT_X_INSIDE);
		int nRectSizeYInside = ::SystemDPIScale(CROP_RECT_Y_INSIDE);
		int nRectSizeXOutside = ::SystemDPIScale(CROP_RECT_X_OUTSIDE);
		int nRectSizeYOutside = ::SystemDPIScale(CROP_RECT_Y_OUTSIDE);
		int nCropMarkerRectWidth = ::SystemDPIScale(CROP_MARKER_RECT_WIDTH);
		int nCropMarkerRectHeight = ::SystemDPIScale(CROP_MARKER_RECT_HEIGHT);

		// Adjust Point Offset
		point += GetScrollPosition();

		CRect rcTop(	m_CropZoomRect.left + nRectSizeXInside,
						m_CropZoomRect.top - nRectSizeYOutside,
						m_CropZoomRect.right - nRectSizeXInside,
						m_CropZoomRect.top + nRectSizeYInside);

		CRect rcBottom(	m_CropZoomRect.left + nRectSizeXInside,
						m_CropZoomRect.bottom - nRectSizeYInside,
						m_CropZoomRect.right - nRectSizeXInside,
						m_CropZoomRect.bottom + nRectSizeYOutside);

		CRect rcLeft(	m_CropZoomRect.left - nRectSizeXOutside,
						m_CropZoomRect.top + nRectSizeYInside,
						m_CropZoomRect.left + nRectSizeXInside,
						m_CropZoomRect.bottom - nRectSizeYInside);

		CRect rcRight(	m_CropZoomRect.right - nRectSizeXInside,
						m_CropZoomRect.top + nRectSizeYInside,
						m_CropZoomRect.right + nRectSizeXOutside,
						m_CropZoomRect.bottom - nRectSizeYInside);

		CRect rcTopLeft(m_CropZoomRect.left - nRectSizeXOutside,
						m_CropZoomRect.top - nRectSizeYOutside,
						m_CropZoomRect.left + nRectSizeXInside,
						m_CropZoomRect.top + nRectSizeYInside);

		CRect rcBottomLeft(	m_CropZoomRect.left - nRectSizeXOutside,
							m_CropZoomRect.bottom - nRectSizeYInside,
							m_CropZoomRect.left + nRectSizeXInside,
							m_CropZoomRect.bottom + nRectSizeYOutside);

		CRect rcTopRight(	m_CropZoomRect.right - nRectSizeXInside,
							m_CropZoomRect.top - nRectSizeYOutside,
							m_CropZoomRect.right + nRectSizeXOutside,
							m_CropZoomRect.top + nRectSizeYInside);

		CRect rcBottomRight(m_CropZoomRect.right - nRectSizeXInside,
							m_CropZoomRect.bottom - nRectSizeYInside,
							m_CropZoomRect.right + nRectSizeXOutside,
							m_CropZoomRect.bottom + nRectSizeYOutside);

		CRect rcCenter(		m_CropZoomRect.left + (m_CropZoomRect.Width()/2) - ((nCropMarkerRectWidth + nRectSizeXInside)/2),
							m_CropZoomRect.top + (m_CropZoomRect.Height()/2) - ((nCropMarkerRectHeight + nRectSizeYInside)/2),
							m_CropZoomRect.left + (m_CropZoomRect.Width()/2) + ((nCropMarkerRectWidth + nRectSizeXInside)/2),
							m_CropZoomRect.top + (m_CropZoomRect.Height()/2) + ((nCropMarkerRectHeight + nRectSizeYInside)/2));
		
		if (rcTopLeft.PtInRect(point))
		{
			m_bCropTopLeft = TRUE;
		}
		else if (rcBottomRight.PtInRect(point))
		{
			m_bCropBottomRight = TRUE;
		}
		else if (rcBottomLeft.PtInRect(point))
		{
			m_bCropBottomLeft = TRUE;
		}
		else if (rcTopRight.PtInRect(point))
		{
			m_bCropTopRight = TRUE;
		}
		else if (rcTop.PtInRect(point))
		{
			m_bCropTop = TRUE;
			if (point.x < (m_CropZoomRect.right - m_CropZoomRect.Width() / 2))
				m_nAspectRatioPos = 0;
			else
				m_nAspectRatioPos = 1;
		}
		else if (rcBottom.PtInRect(point))
		{
			m_bCropBottom = TRUE;
			if (point.x < (m_CropZoomRect.right - m_CropZoomRect.Width() / 2))
				m_nAspectRatioPos = 0;
			else
				m_nAspectRatioPos = 1;
		}
		else if (rcLeft.PtInRect(point))
		{
			m_bCropLeft = TRUE;
			if (point.y < (m_CropZoomRect.bottom - m_CropZoomRect.Height() / 2))
				m_nAspectRatioPos = 0;
			else
				m_nAspectRatioPos = 1;
		}
		else if (rcRight.PtInRect(point))
		{
			m_bCropRight = TRUE;
			if (point.y < (m_CropZoomRect.bottom - m_CropZoomRect.Height() / 2))
				m_nAspectRatioPos = 0;
			else
				m_nAspectRatioPos = 1;
		}
		else if (rcCenter.PtInRect(point))
		{
			m_ptCropClick = point;
			pDoc->m_rcCropCenter = pDoc->m_rcCropDelta;
			m_bCropCenter = TRUE;
		}
		else
		{
			// Set vars
			m_bCropDrag = TRUE;
			m_ptCropClick = point;

			// Reset crop rectangles
			pDoc->m_rcCropDelta = CRect(0,0,0,0);
			m_CropZoomRect = m_ZoomRect;
			pDoc->m_CropDocRect = pDoc->m_DocRect;

			// Middle point
			CPoint ptMiddle = CPoint((m_CropZoomRect.right + m_CropZoomRect.left) / 2, (m_CropZoomRect.bottom + m_CropZoomRect.top) / 2);

			// Point in Top-Left quadrant
			if (point.x <= ptMiddle.x && point.y <= ptMiddle.y)
			{
				CropTop(point, nRectSizeYInside, FALSE);
				CropLeft(point, nRectSizeXInside, FALSE);
				m_CropZoomRect = m_ZoomRect;
				pDoc->m_CropDocRect = pDoc->m_DocRect;
				m_CropZoomRect.DeflateRect(	Round(pDoc->m_rcCropDelta.left*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.top*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.right*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.bottom*pDoc->m_dZoomFactor));
				pDoc->m_CropDocRect.DeflateRect(pDoc->m_rcCropDelta);
				CropBottom(CPoint(m_CropZoomRect.left + nRectSizeXInside + 1, m_CropZoomRect.top + nRectSizeYInside + 1), nRectSizeYInside, FALSE);
				CropRight(CPoint(m_CropZoomRect.left + nRectSizeXInside + 1, m_CropZoomRect.top + nRectSizeYInside + 1), nRectSizeXInside, FALSE);
				m_CropZoomRect = m_ZoomRect;
				pDoc->m_CropDocRect = pDoc->m_DocRect;
				m_CropZoomRect.DeflateRect(	Round(pDoc->m_rcCropDelta.left*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.top*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.right*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.bottom*pDoc->m_dZoomFactor));
				pDoc->m_CropDocRect.DeflateRect(pDoc->m_rcCropDelta);
			}
			// Point in Top-Right quadrant
			else if (point.x > ptMiddle.x && point.y <= ptMiddle.y)
			{
				CropTop(point, nRectSizeYInside, FALSE);
				CropRight(point, nRectSizeXInside, FALSE);
				m_CropZoomRect = m_ZoomRect;
				pDoc->m_CropDocRect = pDoc->m_DocRect;
				m_CropZoomRect.DeflateRect(	Round(pDoc->m_rcCropDelta.left*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.top*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.right*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.bottom*pDoc->m_dZoomFactor));
				pDoc->m_CropDocRect.DeflateRect(pDoc->m_rcCropDelta);
				CropBottom(CPoint(m_CropZoomRect.right - nRectSizeXInside - 1, m_CropZoomRect.top + nRectSizeYInside + 1), nRectSizeYInside, FALSE);
				CropLeft(CPoint(m_CropZoomRect.right - nRectSizeXInside - 1, m_CropZoomRect.top + nRectSizeYInside + 1), nRectSizeXInside, FALSE);
				m_CropZoomRect = m_ZoomRect;
				pDoc->m_CropDocRect = pDoc->m_DocRect;
				m_CropZoomRect.DeflateRect(	Round(pDoc->m_rcCropDelta.left*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.top*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.right*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.bottom*pDoc->m_dZoomFactor));
				pDoc->m_CropDocRect.DeflateRect(pDoc->m_rcCropDelta);
			}
			// Point in Bottom-Right quadrant
			else if (point.x > ptMiddle.x && point.y > ptMiddle.y)
			{
				CropBottom(point, nRectSizeYInside, FALSE);
				CropRight(point, nRectSizeXInside, FALSE);
				m_CropZoomRect = m_ZoomRect;
				pDoc->m_CropDocRect = pDoc->m_DocRect;
				m_CropZoomRect.DeflateRect(	Round(pDoc->m_rcCropDelta.left*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.top*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.right*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.bottom*pDoc->m_dZoomFactor));
				pDoc->m_CropDocRect.DeflateRect(pDoc->m_rcCropDelta);
				CropTop(CPoint(m_CropZoomRect.right - nRectSizeXInside - 1, m_CropZoomRect.bottom - nRectSizeYInside - 1), nRectSizeYInside, FALSE);
				CropLeft(CPoint(m_CropZoomRect.right - nRectSizeXInside - 1, m_CropZoomRect.bottom - nRectSizeYInside - 1), nRectSizeXInside, FALSE);
				m_CropZoomRect = m_ZoomRect;
				pDoc->m_CropDocRect = pDoc->m_DocRect;
				m_CropZoomRect.DeflateRect(	Round(pDoc->m_rcCropDelta.left*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.top*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.right*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.bottom*pDoc->m_dZoomFactor));
				pDoc->m_CropDocRect.DeflateRect(pDoc->m_rcCropDelta);
			}
			// Point in Bottom-Left quadrant
			else
			{
				CropBottom(point, nRectSizeYInside, FALSE);
				CropLeft(point, nRectSizeXInside, FALSE);
				m_CropZoomRect = m_ZoomRect;
				pDoc->m_CropDocRect = pDoc->m_DocRect;
				m_CropZoomRect.DeflateRect(	Round(pDoc->m_rcCropDelta.left*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.top*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.right*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.bottom*pDoc->m_dZoomFactor));
				pDoc->m_CropDocRect.DeflateRect(pDoc->m_rcCropDelta);
				CropTop(CPoint(m_CropZoomRect.left + nRectSizeXInside + 1, m_CropZoomRect.bottom - nRectSizeYInside - 1), nRectSizeYInside, FALSE);
				CropRight(CPoint(m_CropZoomRect.left + nRectSizeXInside + 1, m_CropZoomRect.bottom - nRectSizeYInside - 1), nRectSizeXInside, FALSE);
				m_CropZoomRect = m_ZoomRect;
				pDoc->m_CropDocRect = pDoc->m_DocRect;
				m_CropZoomRect.DeflateRect(	Round(pDoc->m_rcCropDelta.left*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.top*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.right*pDoc->m_dZoomFactor),
											Round(pDoc->m_rcCropDelta.bottom*pDoc->m_dZoomFactor));
				pDoc->m_CropDocRect.DeflateRect(pDoc->m_rcCropDelta);
			}

			// Invalidate View and Update Status Text
			CRect rcClient; 
			GetClientRect(&rcClient);
			InvalidateRect(rcClient, FALSE);
			UpdateCropStatusText();
		}

		// Set Capture
		m_bCropMouseCaptured = TRUE;
		SetCapture();
	}
	else if (pDoc->m_bDoRotationColorPickup)
	{
		CClientDC dc(this);
		COLORREF crColor(dc.GetPixel(point.x, point.y));
		if (crColor != CLR_INVALID && pDoc->m_pRotationFlippingDlg)
		{
			pDoc->m_pRotationFlippingDlg->m_PickColorFromDlg.SetColorToWindowsDefault();
			pDoc->m_pRotationFlippingDlg->m_crBackgroundColor = crColor;
			pDoc->m_pRotationFlippingDlg->
			m_PickColorFromImage.SetColor(	CDib::HighlightColor(crColor),
											crColor);	
		}
		pDoc->m_bDoRotationColorPickup = FALSE;
		::AfxGetMainFrame()->StatusText();
	}
	else if (pDoc->m_bDoRedEyeColorPickup)
	{
		if (pDoc->IsClickPointRedEye(point, pDoc->m_pDib))
		{
			pDoc->m_pRedEyeDlg = new CRedEyeDlg(this, point);
			ClientToScreen(&point);
			CRect rcWork = ::AfxGetMainFrame()->GetMonitorWorkRect(point);
			CRect rcDlg;
			pDoc->m_pRedEyeDlg->GetWindowRect(&rcDlg);
			CPoint ptShowDlg;

			// Display the dialog near the click point
			if (point.x > (rcWork.left + rcWork.Width() / 2))
				ptShowDlg.x = point.x - rcDlg.Width() - rcDlg.Width() / 5;	// to the left of the click point
			else
				ptShowDlg.x = point.x + rcDlg.Width() / 5;					// to the right of the click point

			// In the Monitor's Work area Vertical Middle
			ptShowDlg.y = rcWork.top + (rcWork.Height() / 2) - (rcDlg.Height() / 2);

			// Show Dialog
			pDoc->m_pRedEyeDlg->SetWindowPos(	NULL,
												ptShowDlg.x,
												ptShowDlg.y,
												0, 0,
												SWP_SHOWWINDOW | SWP_NOSIZE);
		}
		else
		{
			::AfxMessageBox(ML_STRING(1227, "Please Click in the Middle of a Red-Eye!"), MB_ICONSTOP);
			ForceCursor(FALSE);
		}

		pDoc->m_bDoRedEyeColorPickup = FALSE;
		::AfxGetMainFrame()->StatusText();
	}
	else if (pDoc->m_pRedEyeDlg && ((nFlags & MK_CONTROL) || (nFlags & MK_SHIFT)))
	{
		if (nFlags & MK_CONTROL)
			pDoc->m_pRedEyeDlg->AddMaskPoint(point);
		else
			pDoc->m_pRedEyeDlg->RemoveMaskPoint(point);
	}
	else if (IsXOrYScroll())
	{
		m_ptStartScrollMoveClickPos = point;
		m_ptStartScrollMovePos = GetScrollPosition();
		m_bDoScrollMove = TRUE;
		SetCapture();
		HCURSOR hCursor;
		hCursor = ::AfxGetApp()->LoadCursor(IDC_GRAB_NORMAL);
		ASSERT(hCursor);
		::SetCursor(hCursor);
	}
}

void CPictureView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	EnableCursor();

	if (pDoc->m_bZoomTool)
	{
		ZoomPoint(point, pDoc->m_bZoomToolMinus);
		if (IsXOrYScroll())
		{
			m_ptStartScrollMoveClickPos = point;
			m_ptStartScrollMovePos = GetScrollPosition();
			m_bDoScrollMove = TRUE;
			SetCapture();
		}
	}
	else
		::AfxGetMainFrame()->EnterExitFullscreen();
}

void CPictureView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	m_bCropTopLeft = FALSE;
	m_bCropBottomRight = FALSE;
	m_bCropBottomLeft = FALSE;
	m_bCropTopRight = FALSE;
	m_bCropTop = FALSE;
	m_bCropBottom = FALSE;
	m_bCropLeft = FALSE;
	m_bCropRight = FALSE;
	m_bCropCenter = FALSE;
	m_bCropDrag = FALSE;
	m_ptCropClick = CPoint(0,0);
	m_dCropAspectRatio = 1.0;
	m_bCropMaintainAspectRatio = FALSE;
	pDoc->m_rcCropCenter = CRect(0,0,0,0);
	m_nAspectRatioPos = 0;

	if (m_bDoScrollMove)
	{
		ReleaseCapture();
		m_bDoScrollMove = FALSE;
	}
	else if (m_bCropMouseCaptured)
	{
		ReleaseCapture();
		m_bCropMouseCaptured = FALSE;
	}
	if (m_nAutoScroll)
		StopAutoScroll();
	
	CUImagerView::OnLButtonUp(nFlags, point);
}

void CPictureView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	EnableCursor();
	CUImagerView::OnMButtonDown(nFlags, point);
}

void CPictureView::ZoomPoint(CPoint point, BOOL bZoomOut) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Avoid Transition While Zooming!
	pDoc->CancelTransition();
	pDoc->CancelLoadFullJpegTransition();

	// Calculate the click coordinates:
	// - Origine is top-left of picture
	// - Measure unit is in picture pixels
	CPoint ptClick = point;
	ptClick += GetScrollPosition();
	ptClick -= m_ZoomRect.TopLeft();
	ptClick.x = (int)((double)ptClick.x / pDoc->m_dZoomFactor);
	ptClick.y = (int)((double)ptClick.y / pDoc->m_dZoomFactor);
	CZoomComboBox* pZoomCB = &(((CPictureToolBar*)((CToolBarChildFrame*)GetParentFrame())->GetToolBar())->m_ZoomComboBox);
	int Index = pZoomCB->GetCurSel();
	if (Index != CB_ERR)
	{
		void* pData;
		if (Index == 0 || Index == 1) // Zoom Fit or Fit Big
		{
			if (bZoomOut)
				Index = pZoomCB->GetPrevZoomIndex(pDoc->m_dZoomFactor); // Zoom Out
			else
				Index = pZoomCB->GetNextZoomIndex(pDoc->m_dZoomFactor); // Zoom In
			pData = pZoomCB->GetItemDataPtr(Index);
		}
		else
		{
			if (bZoomOut)
			{
				--Index; // Zoom Out
				if (Index < 2)
					Index = 2;
				pData = pZoomCB->GetItemDataPtr(Index);
			}
			else
			{
				++Index; // Zoom In
				if (Index > (pZoomCB->GetCount() - 1))
					Index = pZoomCB->GetCount() - 1;
				pData = pZoomCB->GetItemDataPtr(Index);
			}
		}
		if ((int)pData != -1)
		{
			pZoomCB->SetCurSel(Index);
			pZoomCB->OnChangeZoomFactor(*((double*)pData));
			UpdateWindowSizes(TRUE, TRUE, FALSE);
			if (Index > 1)
			{
				CPoint ptScrollTo;
				ptScrollTo.x = (int)((double)ptClick.x * pDoc->m_dZoomFactor);
				ptScrollTo.y = (int)((double)ptClick.y * pDoc->m_dZoomFactor);
				ptScrollTo += m_ZoomRect.TopLeft();
				ptScrollTo -= point;
				if (ptScrollTo.x < 0)
					ptScrollTo.x = 0;
				if (ptScrollTo.y < 0)
					ptScrollTo.y = 0;
				if (IsXAndYScroll())
					ScrollToPosition(ptScrollTo);
				else if (IsXScroll())
					ScrollToPosition(CPoint(	ptScrollTo.x,
												0));
				else if (IsYScroll())
					ScrollToPosition(CPoint(	0,
												ptScrollTo.y));
			}
		}
	}	
}

BOOL CPictureView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CPoint point;
	::GetSafeCursorPos(&point);
	ScreenToClient(&point); // Client coordinates of mouse position
	CRect rcClient;
	GetClientRect(&rcClient);

	// If Wait Cursor leave it!
	if (((CUImagerApp*)::AfxGetApp())->IsWaitCursor())
	{
		RestoreWaitCursor();
		return TRUE;
	}
	else if (rcClient.PtInRect(point))
	{
		if (pDoc->m_bZoomTool)
		{
			HCURSOR hCursor;
			if (pDoc->m_bZoomToolMinus)
				hCursor = ::AfxGetApp()->LoadCursor(IDC_ZOOMMINUS);	
			else
				hCursor = ::AfxGetApp()->LoadCursor(IDC_ZOOMPLUS);	
			ASSERT(hCursor);
			::SetCursor(hCursor);
			return TRUE;
		}
		else if (pDoc->m_bDoRotationColorPickup || pDoc->m_bDoRedEyeColorPickup)
		{
			HCURSOR hCursor = ::AfxGetApp()->LoadCursor(IDC_SELECTCOLOR_CURSOR);	
			ASSERT(hCursor);
			::SetCursor(hCursor);
			return TRUE;
		}
		else if (pDoc->m_bCrop)
		{
			if (m_hCropCursor)
			{
				::SetCursor(m_hCropCursor);
				return TRUE;
			}
			else
				return CUImagerView::OnSetCursor(pWnd, nHitTest, message);
		}
	}
	
	return CUImagerView::OnSetCursor(pWnd, nHitTest, message);
}

void CPictureView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CUImagerView::OnMouseMove(nFlags, point);

	// Store Last Flag and Point,
	// used by the Crop Autoscroll
	// and the Crop Wheelscroll
	m_uiOnMouseMoveLastFlag = nFlags;
	m_OnMouseMoveLastPoint = point;

	if (m_bDoScrollMove)
	{
		CPoint ptCurrentScrollPos = GetScrollPosition();
		CPoint ptScrollPos = m_ptStartScrollMovePos;
		int nDeltaX = m_ptStartScrollMoveClickPos.x - point.x; 
		int nDeltaY = m_ptStartScrollMoveClickPos.y - point.y;
		if (IsXScroll())
			ptScrollPos.x += nDeltaX;
		else
			ptScrollPos.x = ptCurrentScrollPos.x;
		if (IsYScroll())
			ptScrollPos.y += nDeltaY;
		else
			ptScrollPos.y = ptCurrentScrollPos.y;
		ScrollToPosition(ptScrollPos);

		if (pDoc->m_bZoomTool)
		{
			if ((nFlags & MK_SHIFT) ||
				(nFlags & MK_CONTROL)) 
				pDoc->m_bZoomToolMinus = TRUE;
			else
				pDoc->m_bZoomToolMinus = FALSE;
			::AfxGetMainFrame()->StatusText(ML_STRING(1226, "*** Right Click to Zoom Out ***"));
		}
	}
	else if (pDoc->m_bZoomTool)
	{
		if ((nFlags & MK_SHIFT) ||
			(nFlags & MK_CONTROL)) 
			pDoc->m_bZoomToolMinus = TRUE;
		else
			pDoc->m_bZoomToolMinus = FALSE;
		::AfxGetMainFrame()->StatusText(ML_STRING(1226, "*** Right Click to Zoom Out ***"));
	}
	else if (pDoc->m_bCrop)
	{
		CPoint ptOrigPoint = point;
		CRect rcClient; 
		GetClientRect(&rcClient);
		int nRectSizeXInside = ::SystemDPIScale(CROP_RECT_X_INSIDE);
		int nRectSizeYInside = ::SystemDPIScale(CROP_RECT_Y_INSIDE);
		int nRectSizeXOutside = ::SystemDPIScale(CROP_RECT_X_OUTSIDE);
		int nRectSizeYOutside = ::SystemDPIScale(CROP_RECT_Y_OUTSIDE);
		int nCropMarkerRectWidth = ::SystemDPIScale(CROP_MARKER_RECT_WIDTH);
		int nCropMarkerRectHeight = ::SystemDPIScale(CROP_MARKER_RECT_HEIGHT);
		CRect rcLastCropDelta = pDoc->m_rcCropDelta;

		// Adjust Point Offset
		point += GetScrollPosition();

		CRect rcTop(	m_CropZoomRect.left + nRectSizeXInside,
						m_CropZoomRect.top - nRectSizeYOutside,
						m_CropZoomRect.right - nRectSizeXInside,
						m_CropZoomRect.top + nRectSizeYInside);

		CRect rcBottom(	m_CropZoomRect.left + nRectSizeXInside,
						m_CropZoomRect.bottom - nRectSizeYInside,
						m_CropZoomRect.right - nRectSizeXInside,
						m_CropZoomRect.bottom + nRectSizeYOutside);

		CRect rcLeft(	m_CropZoomRect.left - nRectSizeXOutside,
						m_CropZoomRect.top + nRectSizeYInside,
						m_CropZoomRect.left + nRectSizeXInside,
						m_CropZoomRect.bottom - nRectSizeYInside);

		CRect rcRight(	m_CropZoomRect.right - nRectSizeXInside,
						m_CropZoomRect.top + nRectSizeYInside,
						m_CropZoomRect.right + nRectSizeXOutside,
						m_CropZoomRect.bottom - nRectSizeYInside);

		CRect rcTopLeft(m_CropZoomRect.left - nRectSizeXOutside,
						m_CropZoomRect.top - nRectSizeYOutside,
						m_CropZoomRect.left + nRectSizeXInside,
						m_CropZoomRect.top + nRectSizeYInside);

		CRect rcBottomLeft(	m_CropZoomRect.left - nRectSizeXOutside,
							m_CropZoomRect.bottom - nRectSizeYInside,
							m_CropZoomRect.left + nRectSizeXInside,
							m_CropZoomRect.bottom + nRectSizeYOutside);

		CRect rcTopRight(	m_CropZoomRect.right - nRectSizeXInside,
							m_CropZoomRect.top - nRectSizeYOutside,
							m_CropZoomRect.right + nRectSizeXOutside,
							m_CropZoomRect.top + nRectSizeYInside);

		CRect rcBottomRight(m_CropZoomRect.right - nRectSizeXInside,
							m_CropZoomRect.bottom - nRectSizeYInside,
							m_CropZoomRect.right + nRectSizeXOutside,
							m_CropZoomRect.bottom + nRectSizeYOutside);

		CRect rcCenter(		m_CropZoomRect.left + (m_CropZoomRect.Width()/2) - ((nCropMarkerRectWidth + nRectSizeXInside)/2),
							m_CropZoomRect.top + (m_CropZoomRect.Height()/2) - ((nCropMarkerRectHeight + nRectSizeYInside)/2),
							m_CropZoomRect.left + (m_CropZoomRect.Width()/2) + ((nCropMarkerRectWidth + nRectSizeXInside)/2),
							m_CropZoomRect.top + (m_CropZoomRect.Height()/2) + ((nCropMarkerRectHeight + nRectSizeYInside)/2));

		/*
		IDC_SIZENESW Double-pointed arrow pointing northeast and southwest 
		IDC_SIZENS Double-pointed arrow pointing north and south 
		IDC_SIZENWSE Double-pointed arrow pointing northwest and southeast 
		IDC_SIZEWE Double-pointed arrow pointing west and east
		IDC_SIZEALL 2 x Double-pointed arrows pointing in all directions
		*/
		if (nFlags & MK_LBUTTON)
		{
			// Auto-Scroll
			BOOL bDoAutoScroll = FALSE;
			if (ptOrigPoint.y < AUTOSCROLL_BORDER)
			{
				m_nAutoScroll = AUTOSCROLL_UP;
				bDoAutoScroll = TRUE;
			}
			else if (ptOrigPoint.y > (rcClient.Height() - AUTOSCROLL_BORDER))
			{
				m_nAutoScroll = AUTOSCROLL_DOWN;
				bDoAutoScroll = TRUE;
			}
			if (ptOrigPoint.x < AUTOSCROLL_BORDER)
			{
				m_nAutoScroll |= AUTOSCROLL_LEFT;
				bDoAutoScroll = TRUE;
			}
			else if (ptOrigPoint.x > (rcClient.Width() - AUTOSCROLL_BORDER))
			{
				m_nAutoScroll |= AUTOSCROLL_RIGHT;
				bDoAutoScroll = TRUE;
			}
			if (bDoAutoScroll)
			{
				if (m_uiAutoScrollTimerId == 0)
					m_uiAutoScrollTimerId = SetTimer(ID_TIMER_CROP_SCROLL, AUTOSCROLL_TIMER_MS, NULL);
			}
			else
			{
				if (m_nAutoScroll)
					StopAutoScroll();
			}
		}
		else
		{
			if (m_nAutoScroll)
				StopAutoScroll();

			if (rcTopLeft.PtInRect(point))
				m_hCropCursor = ::LoadCursor(NULL, IDC_SIZENWSE);
			else if (rcBottomRight.PtInRect(point))
				m_hCropCursor = ::LoadCursor(NULL, IDC_SIZENWSE);
			else if (rcBottomLeft.PtInRect(point))
				m_hCropCursor = ::LoadCursor(NULL, IDC_SIZENESW);
			else if (rcTopRight.PtInRect(point))
				m_hCropCursor = ::LoadCursor(NULL, IDC_SIZENESW);
			else if (rcTop.PtInRect(point))
				m_hCropCursor = ::LoadCursor(NULL, IDC_SIZENS);
			else if (rcBottom.PtInRect(point))
				m_hCropCursor = ::LoadCursor(NULL, IDC_SIZENS);
			else if (rcLeft.PtInRect(point))
				m_hCropCursor = ::LoadCursor(NULL, IDC_SIZEWE);
			else if (rcRight.PtInRect(point))
				m_hCropCursor = ::LoadCursor(NULL, IDC_SIZEWE);
			else if (rcCenter.PtInRect(point))
				m_hCropCursor = ::LoadCursor(NULL, IDC_SIZEALL);
			else
				m_hCropCursor = NULL;
		}

		if (m_bCropTopLeft)
		{
			if (m_bCropMaintainAspectRatio)
			{
				m_nAspectRatioPos = 0;
				if (m_dCropAspectRatio < 1.0)
					CropTop(point, nRectSizeYInside, TRUE);
				else
					CropLeft(point, nRectSizeXInside, TRUE);
			}
			else
			{
				CropTop(point, nRectSizeYInside, FALSE);
				CropLeft(point, nRectSizeXInside, FALSE);
			}
		}
		else if (m_bCropBottomRight)
		{
			if (m_bCropMaintainAspectRatio)
			{
				m_nAspectRatioPos = 1;
				if (m_dCropAspectRatio < 1.0)
					CropBottom(point, nRectSizeYInside, TRUE);
				else
					CropRight(point, nRectSizeXInside, TRUE);
			}
			else
			{
				CropBottom(point, nRectSizeYInside, FALSE);
				CropRight(point, nRectSizeXInside, FALSE);
			}
		}
		else if (m_bCropBottomLeft)
		{
			if (m_bCropMaintainAspectRatio)
			{
				if (m_dCropAspectRatio < 1.0)
				{
					m_nAspectRatioPos = 0;
					CropBottom(point, nRectSizeYInside, TRUE);
				}
				else
				{
					m_nAspectRatioPos = 1;
					CropLeft(point, nRectSizeXInside, TRUE);
				}
			}
			else
			{
				CropBottom(point, nRectSizeYInside, FALSE);
				CropLeft(point, nRectSizeXInside, FALSE);
			}
		}
		else if (m_bCropTopRight)
		{
			if (m_bCropMaintainAspectRatio)
			{
				if (m_dCropAspectRatio < 1.0)
				{
					m_nAspectRatioPos = 1;
					CropTop(point, nRectSizeYInside, TRUE);
				}
				else
				{
					m_nAspectRatioPos = 0;
					CropRight(point, nRectSizeXInside, TRUE);					
				}
			}
			else
			{
				CropTop(point, nRectSizeYInside, FALSE);
				CropRight(point, nRectSizeXInside, FALSE);
			}
		}
		else if (m_bCropTop)
		{
			CropTop(point, nRectSizeYInside, m_bCropMaintainAspectRatio);
		}
		else if (m_bCropBottom)
		{
			CropBottom(point, nRectSizeYInside, m_bCropMaintainAspectRatio);	
		}
		else if (m_bCropLeft)
		{
			CropLeft(point, nRectSizeXInside, m_bCropMaintainAspectRatio);
		}
		else if (m_bCropRight)
		{
			CropRight(point, nRectSizeXInside, m_bCropMaintainAspectRatio);
		}
		else if (m_bCropCenter)
		{
			CropCenter(point - m_ptCropClick, nRectSizeXInside, nRectSizeYInside);
		}
		else if (m_bCropDrag)
		{
			if (point.x > m_ptCropClick.x + nRectSizeXInside && point.y > m_ptCropClick.y + nRectSizeYInside)
			{
				m_bCropBottomRight = TRUE;
				m_bCropDrag = FALSE;
			}
			else if (point.x < m_ptCropClick.x - nRectSizeXInside && point.y > m_ptCropClick.y + nRectSizeYInside)
			{
				m_bCropBottomLeft = TRUE;
				m_bCropDrag = FALSE;
			}
			else if (point.x < m_ptCropClick.x - nRectSizeXInside && point.y < m_ptCropClick.y - nRectSizeYInside)
			{
				m_bCropTopLeft = TRUE;
				m_bCropDrag = FALSE;
			}
			else if (point.x > m_ptCropClick.x + nRectSizeXInside && point.y < m_ptCropClick.y - nRectSizeYInside)
			{
				m_bCropTopRight = TRUE;
				m_bCropDrag = FALSE;
			}
		}

		// Update the crop rectangles if dragging
		if ((nFlags & MK_LBUTTON)									&&
			(rcLastCropDelta.top != pDoc->m_rcCropDelta.top			||
			rcLastCropDelta.bottom != pDoc->m_rcCropDelta.bottom	||
			rcLastCropDelta.left != pDoc->m_rcCropDelta.left		||
			rcLastCropDelta.right != pDoc->m_rcCropDelta.right))
			UpdateCropRectangles();

		// Status Text
		UpdateCropStatusText();
	}
	// Set it again here, because some other open docs may
	// have changed the status message!
	else if (pDoc->m_bDoRedEyeColorPickup)
		::AfxGetMainFrame()->StatusText(ML_STRING(1228, "*** Click In The Middle Of A Red-Eye ***"));
	else if (pDoc->m_bDoRotationColorPickup)
		::AfxGetMainFrame()->StatusText(ML_STRING(1229, "*** Click Inside The Image To Get The Background Color ***"));
	else if (pDoc->m_pRedEyeDlg)
	{
		CClientDC dc(this);
		COLORREF crColor(dc.GetPixel(point.x, point.y));
		CColor c(crColor);
		CString s;
#ifdef _DEBUG
		float fRed = (float)c.GetRed();
		float fGreen = (float)c.GetGreen();
		if (fGreen == 0.0f)	
			fGreen = 1.0f;
		BOOL bRedEye = CRedEyeDlg::IsRedEyePixel(	crColor,
													(float)pDoc->m_pRedEyeDlg->m_uiHueStart,
													(float)pDoc->m_pRedEyeDlg->m_uiHueEnd,
													pDoc->m_pRedEyeDlg->GetCurrentRatio());
		if (bRedEye)
		{
			s.Format(	ML_STRING(1230, "Redeye with Hue: %0.1f° , Red/Green: %0.1f ") +
						ML_STRING(1231, "*** CTRL / SHIFT + Click to add / remove mask pixel ***"),
						c.GetHue(), fRed / fGreen);
		}
		else
		{
			s.Format(	ML_STRING(1232, "No Redeye with Hue: %0.1f° , Red/Green: %0.1f ") +
						ML_STRING(1231, "*** CTRL / SHIFT + Click to add / remove mask pixel ***"),
						c.GetHue(), fRed / fGreen);
		}
#else
		s.Format(	ML_STRING(1233, "Hue: %0.1f° ") +
					ML_STRING(1231, "*** CTRL / SHIFT + Click to add / remove mask pixel ***"),
					c.GetHue());
#endif
		::AfxGetMainFrame()->StatusText(s);

		if (nFlags & MK_LBUTTON)
		{
			if (nFlags & MK_CONTROL)
				pDoc->m_pRedEyeDlg->AddMaskPoint(point);
			else if (nFlags & MK_SHIFT)
				pDoc->m_pRedEyeDlg->RemoveMaskPoint(point);
		}
	}
	else
		::AfxGetMainFrame()->StatusText();
}

void CPictureView::UpdateCropStatusText()
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->m_bCrop)
	{
		CString sCropRect;
		if (((CUImagerApp*)::AfxGetApp())->m_nCoordinateUnit == COORDINATES_PIX)
		{
			sCropRect.Format(	_T("\u2194=%dpx  \u2195=%dpx  \u2194/\u2195=%0.2f  (\u2190=%d  \u2191=%d  \u2192=%d  \u2193=%d) ") +
								ML_STRING(1235, "*** CTRL Lock Ratio, ENTER Apply, ESC Cancel ***"),
								pDoc->m_CropDocRect.Width(),
								pDoc->m_CropDocRect.Height(),
								(double)pDoc->m_CropDocRect.Width() / (double)pDoc->m_CropDocRect.Height(),
								pDoc->m_rcCropDelta.left,
								pDoc->m_rcCropDelta.top,
								pDoc->m_rcCropDelta.right,
								pDoc->m_rcCropDelta.bottom);
		}
		else if (((CUImagerApp*)::AfxGetApp())->m_nCoordinateUnit == COORDINATES_INCH)
		{
			int nXDpi = 0;
			if (pDoc->m_pDib)
				nXDpi = pDoc->m_pDib->GetXDpi();
			if (nXDpi <= 0)
				nXDpi = DEFAULT_DPI;
			int nYDpi = 0;
			if (pDoc->m_pDib)
				nYDpi = pDoc->m_pDib->GetYDpi();
			if (nYDpi <= 0)
				nYDpi = DEFAULT_DPI;
			sCropRect.Format(	_T("\u2194=%0.2fin  \u2195=%0.2fin  \u2194/\u2195=%0.2f  (\u2190=%0.2f  \u2191=%0.2f  \u2192=%0.2f  \u2193=%0.2f) ") +
								ML_STRING(1235, "*** CTRL Lock Ratio, ENTER Apply, ESC Cancel ***"),
								(double)pDoc->m_CropDocRect.Width()		/ (double)nXDpi,
								(double)pDoc->m_CropDocRect.Height()	/ (double)nYDpi,
								(double)pDoc->m_CropDocRect.Width()		/ (double)pDoc->m_CropDocRect.Height(),
								(double)pDoc->m_rcCropDelta.left		/ (double)nXDpi,
								(double)pDoc->m_rcCropDelta.top			/ (double)nYDpi,
								(double)pDoc->m_rcCropDelta.right		/ (double)nXDpi,
								(double)pDoc->m_rcCropDelta.bottom		/ (double)nYDpi);
		}
		else
		{
			int nXDpi = 0;
			if (pDoc->m_pDib)
				nXDpi = pDoc->m_pDib->GetXDpi();
			if (nXDpi <= 0)
				nXDpi = DEFAULT_DPI;
			int nYDpi = 0;
			if (pDoc->m_pDib)
				nYDpi = pDoc->m_pDib->GetYDpi();
			if (nYDpi <= 0)
				nYDpi = DEFAULT_DPI;
			sCropRect.Format(	_T("\u2194=%0.2fcm  \u2195=%0.2fcm  \u2194/\u2195=%0.2f  (\u2190=%0.2f  \u2191=%0.2f  \u2192=%0.2f  \u2193=%0.2f) ") +
								ML_STRING(1235, "*** CTRL Lock Ratio, ENTER Apply, ESC Cancel ***"),
								(double)pDoc->m_CropDocRect.Width()	* 2.54	/ (double)nXDpi,
								(double)pDoc->m_CropDocRect.Height()* 2.54	/ (double)nYDpi,
								(double)pDoc->m_CropDocRect.Width()			/ (double)pDoc->m_CropDocRect.Height(),
								(double)pDoc->m_rcCropDelta.left	* 2.54	/ (double)nXDpi,
								(double)pDoc->m_rcCropDelta.top		* 2.54	/ (double)nYDpi,
								(double)pDoc->m_rcCropDelta.right	* 2.54	/ (double)nXDpi,
								(double)pDoc->m_rcCropDelta.bottom	* 2.54	/ (double)nYDpi);
		}
		::AfxGetMainFrame()->StatusText(sCropRect);
	}
}

void CPictureView::CalcZoomedPixelAlign()
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->m_dZoomFactor <= 1.0)
	{
		m_nZoomedPixelAlignX = Round(pDoc->m_nPixelAlignX * pDoc->m_dZoomFactor);
		m_nZoomedPixelAlignY = Round(pDoc->m_nPixelAlignY * pDoc->m_dZoomFactor);
		if (m_nZoomedPixelAlignX == 0)
			m_nZoomedPixelAlignX = 1;
		if (m_nZoomedPixelAlignY == 0)
			m_nZoomedPixelAlignY = 1;
	}
	else
	{
		m_nZoomedPixelAlignX = Round(pDoc->m_nPixelAlignX * pDoc->m_dZoomFactor);
		m_nZoomedPixelAlignY = Round(pDoc->m_nPixelAlignY * pDoc->m_dZoomFactor);
	}
}

void CPictureView::CropLeft(CPoint point, int nRectSizeXInside, BOOL bCropMaintainAspectRatio)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Set Pixel Align Vars
	int nPixelAlignX, nPixelAlignY, nZoomedPixelAlignX;
	if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
		nPixelAlignX = 1;
	else
		nPixelAlignX = pDoc->m_nPixelAlignX;
	if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
		nPixelAlignY = 1;
	else
		nPixelAlignY = pDoc->m_nPixelAlignY;
	if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
		nZoomedPixelAlignX = MAX(1, Round(pDoc->m_dZoomFactor));
	else
		nZoomedPixelAlignX = m_nZoomedPixelAlignX;

	// Inc / Dec Crop Delta
	if (point.x < (m_CropZoomRect.right - nRectSizeXInside))
	{
		pDoc->m_rcCropDelta.left +=	Round(	(point.x - m_CropZoomRect.left) /
											nZoomedPixelAlignX * nZoomedPixelAlignX /
											pDoc->m_dZoomFactor);
	}

	// Clip
	CropClipLeft(nPixelAlignX);

	// If Maintain Aspect Ratio Set Top Or Bottom Crop Edge
	if (bCropMaintainAspectRatio)
	{
		// New Crop Rect Width
		int w = pDoc->m_DocRect.Width() - (pDoc->m_rcCropDelta.left + pDoc->m_rcCropDelta.right);

		// Top Side Follows
		if (m_nAspectRatioPos == 0)
		{	
			int nCropDeltaTop = (int)(pDoc->m_DocRect.Height() - w / m_dCropAspectRatio - pDoc->m_rcCropDelta.bottom);
			pDoc->m_rcCropDelta.top = nCropDeltaTop / nPixelAlignY * nPixelAlignY;

			// Clip
			CropClipTop(nPixelAlignY);
		}
		// Bottom Side Follows
		else
		{
			int nCropDeltaBottom = (int)(pDoc->m_DocRect.Height() - w / m_dCropAspectRatio - pDoc->m_rcCropDelta.top);
			int nFirstBottomCrop = pDoc->m_DocRect.Height() % nPixelAlignY;
			if (nCropDeltaBottom >= nFirstBottomCrop)
			{
				pDoc->m_rcCropDelta.bottom =	nFirstBottomCrop;
				pDoc->m_rcCropDelta.bottom +=	(nCropDeltaBottom - nFirstBottomCrop) / nPixelAlignY * nPixelAlignY;
			}
			else
				pDoc->m_rcCropDelta.bottom = 0;

			// Clip
			CropClipBottom(nPixelAlignY);
		}
	}
}

void CPictureView::CropTop(CPoint point, int nRectSizeYInside, BOOL bCropMaintainAspectRatio)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Set Pixel Align Vars
	int nPixelAlignX, nPixelAlignY, nZoomedPixelAlignY;
	if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
		nPixelAlignX = 1;
	else
		nPixelAlignX = pDoc->m_nPixelAlignX;
	if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
		nPixelAlignY = 1;
	else
		nPixelAlignY = pDoc->m_nPixelAlignY;
	if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
		nZoomedPixelAlignY = MAX(1, Round(pDoc->m_dZoomFactor));
	else
		nZoomedPixelAlignY = m_nZoomedPixelAlignY;

	// Inc / Dec Crop Delta
	if (point.y < (m_CropZoomRect.bottom - nRectSizeYInside))
	{
		pDoc->m_rcCropDelta.top += Round(	(point.y - m_CropZoomRect.top) /
											nZoomedPixelAlignY * nZoomedPixelAlignY /
											pDoc->m_dZoomFactor);
	}

	// Clip
	CropClipTop(nPixelAlignY);

	// If Maintain Aspect Ratio Set Left Or Right Crop Delta
	if (bCropMaintainAspectRatio)
	{
		// New Crop Rect Height
		int h = pDoc->m_DocRect.Height() - (pDoc->m_rcCropDelta.top + pDoc->m_rcCropDelta.bottom);

		// Left Side Follows
		if (m_nAspectRatioPos == 0)
		{
			int nCropDeltaLeft = (int)(pDoc->m_DocRect.Width() - h * m_dCropAspectRatio - pDoc->m_rcCropDelta.right);
			pDoc->m_rcCropDelta.left = nCropDeltaLeft / nPixelAlignX * nPixelAlignX;

			// Clip
			CropClipLeft(nPixelAlignX);
		}
		// Right Side Follows
		else
		{
			int nCropDeltaRight = (int)(pDoc->m_DocRect.Width() - h * m_dCropAspectRatio - pDoc->m_rcCropDelta.left);
			int nFirstRightCrop = pDoc->m_DocRect.Width() % nPixelAlignX;
			if (nCropDeltaRight >= nFirstRightCrop)
			{
				pDoc->m_rcCropDelta.right =		nFirstRightCrop;
				pDoc->m_rcCropDelta.right +=	(nCropDeltaRight - nFirstRightCrop) / nPixelAlignX * nPixelAlignX;
			}
			else
				pDoc->m_rcCropDelta.right =	0;

			// Clip
			CropClipRight(nPixelAlignX);
		}
	}
}

void CPictureView::CropRight(CPoint point, int nRectSizeXInside, BOOL bCropMaintainAspectRatio)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Set Pixel Align Vars
	int nPixelAlignX, nPixelAlignY, nZoomedPixelAlignX;
	if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
	{
		nPixelAlignX = 1;
		nZoomedPixelAlignX = MAX(1, Round(pDoc->m_dZoomFactor));
	}
	else
	{
		nPixelAlignX = pDoc->m_nPixelAlignX;
		nZoomedPixelAlignX = m_nZoomedPixelAlignX;
	}
	if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
		nPixelAlignY = 1;
	else
		nPixelAlignY = pDoc->m_nPixelAlignY;

	// Inc / Dec Crop Delta
	if (point.x > (m_CropZoomRect.left + nRectSizeXInside))
	{
		if (pDoc->m_rcCropDelta.right == 0)
		{
			int nFirstRightCrop = pDoc->m_DocRect.Width() % nPixelAlignX;
			int nZoomedFirstRightCrop = m_ZoomRect.Width() % nZoomedPixelAlignX;
			if (nFirstRightCrop == 0)
				nZoomedFirstRightCrop = 0;
			if ((m_CropZoomRect.right - point.x) > nZoomedFirstRightCrop)
			{
				pDoc->m_rcCropDelta.right =		nFirstRightCrop;
				pDoc->m_rcCropDelta.right +=	Round(	(m_CropZoomRect.right - nZoomedFirstRightCrop - point.x) /
														nZoomedPixelAlignX * nZoomedPixelAlignX /
														pDoc->m_dZoomFactor);
			}
		}
		else if (point.x == m_ZoomRect.right)
			pDoc->m_rcCropDelta.right = 0;
		else
			pDoc->m_rcCropDelta.right +=	Round(	(m_CropZoomRect.right - point.x) /
													nZoomedPixelAlignX * nZoomedPixelAlignX /
													pDoc->m_dZoomFactor);
	}

	// Clip
	CropClipRight(nPixelAlignX);

	// If Maintain Aspect Ratio Set Top Or Bottom Crop Edge
	if (bCropMaintainAspectRatio)
	{
		// New Crop Rect Width
		int w = pDoc->m_DocRect.Width() - (pDoc->m_rcCropDelta.left + pDoc->m_rcCropDelta.right);

		// Top Side Follows
		if (m_nAspectRatioPos == 0)
		{	
			int nCropDeltaTop = (int)(pDoc->m_DocRect.Height() - w / m_dCropAspectRatio - pDoc->m_rcCropDelta.bottom);
			pDoc->m_rcCropDelta.top = nCropDeltaTop / nPixelAlignY * nPixelAlignY;

			// Clip
			CropClipTop(nPixelAlignY);
		}
		// Bottom Side Follows
		else
		{
			int nCropDeltaBottom = (int)(pDoc->m_DocRect.Height() - w / m_dCropAspectRatio - pDoc->m_rcCropDelta.top);
			int nFirstBottomCrop = pDoc->m_DocRect.Height() % nPixelAlignY;
			if (nCropDeltaBottom >= nFirstBottomCrop)
			{
				pDoc->m_rcCropDelta.bottom =	nFirstBottomCrop;
				pDoc->m_rcCropDelta.bottom +=	(nCropDeltaBottom - nFirstBottomCrop) / nPixelAlignY * nPixelAlignY;
			}
			else
				pDoc->m_rcCropDelta.bottom = 0;

			// Clip
			CropClipBottom(nPixelAlignY);
		}
	}
}

void CPictureView::CropBottom(CPoint point, int nRectSizeYInside, BOOL bCropMaintainAspectRatio)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Set Pixel Align Vars
	int nPixelAlignX, nPixelAlignY, nZoomedPixelAlignY;
	if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
		nPixelAlignX = 1;
	else
		nPixelAlignX = pDoc->m_nPixelAlignX;
	if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
	{
		nPixelAlignY = 1;
		nZoomedPixelAlignY = MAX(1, Round(pDoc->m_dZoomFactor));
	}
	else
	{
		nPixelAlignY = pDoc->m_nPixelAlignY;
		nZoomedPixelAlignY = m_nZoomedPixelAlignY;
	}

	// Inc / Dec Crop Delta
	if (point.y > (m_CropZoomRect.top + nRectSizeYInside))
	{
		if (pDoc->m_rcCropDelta.bottom == 0)
		{
			int nFirstBottomCrop = pDoc->m_DocRect.Height() % nPixelAlignY;
			int nZoomedFirstBottomCrop = m_ZoomRect.Height() % nZoomedPixelAlignY;
			if (nFirstBottomCrop == 0)
				nZoomedFirstBottomCrop = 0;
			if ((m_CropZoomRect.bottom - point.y) > nZoomedFirstBottomCrop)
			{
				pDoc->m_rcCropDelta.bottom =	nFirstBottomCrop;
				pDoc->m_rcCropDelta.bottom +=	Round(	(m_CropZoomRect.bottom - nZoomedFirstBottomCrop - point.y) /
														nZoomedPixelAlignY * nZoomedPixelAlignY /
														pDoc->m_dZoomFactor);
			}
		}
		else if (point.y == m_ZoomRect.bottom)
			pDoc->m_rcCropDelta.bottom = 0;
		else
			pDoc->m_rcCropDelta.bottom +=	Round(	(m_CropZoomRect.bottom - point.y) /
													nZoomedPixelAlignY * nZoomedPixelAlignY /
													pDoc->m_dZoomFactor);
	}

	// Clip
	CropClipBottom(nPixelAlignY);

	// If Maintain Aspect Ratio Set Left Or Right Crop Edge
	if (bCropMaintainAspectRatio)
	{
		// New Crop Rect Height
		int h = pDoc->m_DocRect.Height() - (pDoc->m_rcCropDelta.top + pDoc->m_rcCropDelta.bottom);

		// Left Side Follows
		if (m_nAspectRatioPos == 0)
		{
			int nCropDeltaLeft = (int)(pDoc->m_DocRect.Width() - h * m_dCropAspectRatio - pDoc->m_rcCropDelta.right);
			pDoc->m_rcCropDelta.left = nCropDeltaLeft / nPixelAlignX * nPixelAlignX;

			// Clip
			CropClipLeft(nPixelAlignX);
		}
		// Right Side Follows
		else
		{
			int nCropDeltaRight = (int)(pDoc->m_DocRect.Width() - h * m_dCropAspectRatio - pDoc->m_rcCropDelta.left);
			int nFirstRightCrop = pDoc->m_DocRect.Width() % nPixelAlignX;
			if (nCropDeltaRight >= nFirstRightCrop)
			{
				pDoc->m_rcCropDelta.right =		nFirstRightCrop;
				pDoc->m_rcCropDelta.right +=	(nCropDeltaRight - nFirstRightCrop) / nPixelAlignX * nPixelAlignX;
			}
			else
				pDoc->m_rcCropDelta.right =	0;

			// Clip
			CropClipRight(nPixelAlignX);
		}
	}
}

void CPictureView::CropCenter(CPoint delta, int nRectSizeXInside, int nRectSizeYInside)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Set Pixel Align Vars
	int nPixelAlignX, nPixelAlignY, nZoomedPixelAlignX, nZoomedPixelAlignY;
	if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
	{
		nPixelAlignX = 1;
		nZoomedPixelAlignX = MAX(1, Round(pDoc->m_dZoomFactor));
	}
	else
	{
		nPixelAlignX = pDoc->m_nPixelAlignX;
		nZoomedPixelAlignX = m_nZoomedPixelAlignX;
	}
	if (pDoc->IsModified() || !pDoc->m_bLosslessCrop)
	{
		nPixelAlignY = 1;
		nZoomedPixelAlignY = MAX(1, Round(pDoc->m_dZoomFactor));
	}
	else
	{
		nPixelAlignY = pDoc->m_nPixelAlignY;
		nZoomedPixelAlignY = m_nZoomedPixelAlignY;
	}

	// Reset Crop Delta to Value Stored When the L-Button Was Pressed
	pDoc->m_rcCropDelta = pDoc->m_rcCropCenter;

	// Set Left Crop Delta
	pDoc->m_rcCropDelta.left +=		Round(	delta.x / nZoomedPixelAlignX *
											nZoomedPixelAlignX /
											pDoc->m_dZoomFactor);

	// Set Top Crop Delta
	pDoc->m_rcCropDelta.top +=		Round(	delta.y / nZoomedPixelAlignY *
											nZoomedPixelAlignY /
											pDoc->m_dZoomFactor);

	// Set Right Crop Delta
	if (pDoc->m_rcCropDelta.right == 0)
	{
		int nFirstRightCrop = pDoc->m_DocRect.Width() % nPixelAlignX;
		int nZoomedFirstRightCrop = m_ZoomRect.Width() % nZoomedPixelAlignX;
		if (nFirstRightCrop == 0)
			nZoomedFirstRightCrop = 0;
		if (-delta.x > nZoomedFirstRightCrop)
		{
			pDoc->m_rcCropDelta.right =		nFirstRightCrop;
			pDoc->m_rcCropDelta.right +=	Round((	-delta.x - nZoomedFirstRightCrop) / nZoomedPixelAlignX *
													nZoomedPixelAlignX /
													pDoc->m_dZoomFactor);
		}
	}
	else
		pDoc->m_rcCropDelta.right +=		Round(	-delta.x / nZoomedPixelAlignX *
													nZoomedPixelAlignX /
													pDoc->m_dZoomFactor);

	// Set Bottom Crop Delta
	if (pDoc->m_rcCropDelta.bottom == 0)
	{
		int nFirstBottomCrop = pDoc->m_DocRect.Height() % nPixelAlignY;
		int nZoomedFirstBottomCrop = m_ZoomRect.Height() % nZoomedPixelAlignY;
		if (nFirstBottomCrop == 0)
			nZoomedFirstBottomCrop = 0;
		if (-delta.y > nZoomedFirstBottomCrop)
		{
			pDoc->m_rcCropDelta.bottom =	nFirstBottomCrop;
			pDoc->m_rcCropDelta.bottom +=	Round((	-delta.y - nZoomedFirstBottomCrop) / nZoomedPixelAlignY *
													nZoomedPixelAlignY /
													pDoc->m_dZoomFactor);
		}
	}
	else
		pDoc->m_rcCropDelta.bottom +=		Round(	-delta.y / nZoomedPixelAlignY *
													nZoomedPixelAlignY /
													pDoc->m_dZoomFactor);

	// Clip
	CropClipLeft(nPixelAlignX);
	CropClipRight(nPixelAlignX);
	CropClipTop(nPixelAlignY);
	CropClipBottom(nPixelAlignY);
}

void CPictureView::CropClipLeft(int nPixelAlignX)
{
	CPictureDoc* pDoc = (CPictureDoc*)GetDocument();
	ASSERT_VALID(pDoc);

	if (nPixelAlignX < 1)
		nPixelAlignX = 1;
	
	int nFirstRightCrop = pDoc->m_DocRect.Width() % nPixelAlignX;
	if (nFirstRightCrop == 0)
		nFirstRightCrop = nPixelAlignX;
	if (pDoc->m_rcCropDelta.left < 0)
		pDoc->m_rcCropDelta.left = 0;
	if (pDoc->m_rcCropDelta.left > (pDoc->m_DocRect.Width() - nFirstRightCrop))
		pDoc->m_rcCropDelta.left = (pDoc->m_DocRect.Width() - nFirstRightCrop);
}

void CPictureView::CropClipRight(int nPixelAlignX)
{
	CPictureDoc* pDoc = (CPictureDoc*)GetDocument();
	ASSERT_VALID(pDoc);

	if (nPixelAlignX < 1)
		nPixelAlignX = 1;

	if (pDoc->m_rcCropDelta.right < 0)
		pDoc->m_rcCropDelta.right = 0;
	if (pDoc->m_rcCropDelta.right > (pDoc->m_DocRect.Width() - nPixelAlignX))
		pDoc->m_rcCropDelta.right = (pDoc->m_DocRect.Width() - nPixelAlignX);
}

void CPictureView::CropClipTop(int nPixelAlignY)
{
	CPictureDoc* pDoc = (CPictureDoc*)GetDocument();
	ASSERT_VALID(pDoc);

	if (nPixelAlignY < 1)
		nPixelAlignY = 1;

	int nFirstBottomCrop = pDoc->m_DocRect.Height() % nPixelAlignY;
	if (nFirstBottomCrop == 0)
		nFirstBottomCrop = nPixelAlignY;
	if (pDoc->m_rcCropDelta.top < 0)
		pDoc->m_rcCropDelta.top = 0;
	if (pDoc->m_rcCropDelta.top > (pDoc->m_DocRect.Height() - nFirstBottomCrop))
		pDoc->m_rcCropDelta.top = (pDoc->m_DocRect.Height() - nFirstBottomCrop);
}

void CPictureView::CropClipBottom(int nPixelAlignY)
{
	CPictureDoc* pDoc = (CPictureDoc*)GetDocument();
	ASSERT_VALID(pDoc);

	if (nPixelAlignY < 1)
		nPixelAlignY = 1;
	
	if (pDoc->m_rcCropDelta.bottom < 0)
		pDoc->m_rcCropDelta.bottom = 0;
	if (pDoc->m_rcCropDelta.bottom > (pDoc->m_DocRect.Height() - nPixelAlignY))
		pDoc->m_rcCropDelta.bottom = (pDoc->m_DocRect.Height() - nPixelAlignY);
}

void CPictureView::DoAutoScroll()
{
	CPictureDoc* pDoc = (CPictureDoc*)GetDocument();
	ASSERT_VALID(pDoc);

	CPoint ptPos = GetScrollPosition();
	CPoint ptNewPos = ptPos;
	CSize ScrollSize = GetTotalSize(); 
	int nXStep = ScrollSize.cx / AUTOSCROLL_STEPS;
	int nYStep = ScrollSize.cy / AUTOSCROLL_STEPS;
	if (m_nAutoScroll & AUTOSCROLL_UP)
	{
		if (IsYScroll())
			ptNewPos = CPoint(ptNewPos.x, MAX(0, ptNewPos.y - nYStep));
	}
	else if (m_nAutoScroll & AUTOSCROLL_DOWN)
	{
		if (IsYScroll())
			ptNewPos = CPoint(ptNewPos.x, MIN(ScrollSize.cy - 1, ptNewPos.y + nYStep));
	}
	if (m_nAutoScroll & AUTOSCROLL_LEFT)
	{
		if (IsXScroll())
			ptNewPos = CPoint(MAX(0, ptNewPos.x - nXStep), ptNewPos.y);
	}
	else if (m_nAutoScroll & AUTOSCROLL_RIGHT)
	{
		if (IsXScroll())
			ptNewPos = CPoint(MIN(ScrollSize.cx - 1, ptNewPos.x + nXStep), ptNewPos.y);
	}
	if (ptNewPos != ptPos)
	{
		ScrollToPosition(ptNewPos);
		OnMouseMove(m_uiOnMouseMoveLastFlag,
					m_OnMouseMoveLastPoint);
	}
}

void CPictureView::StopAutoScroll()
{
	m_nAutoScroll = 0;
	KillTimer(m_uiAutoScrollTimerId);
	m_uiAutoScrollTimerId = 0;
}

void CPictureView::UpdateScrollSize()
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Zoom Fit or Fit Big?
	if (pDoc->m_nZoomComboBoxIndex == 0	||
		pDoc->m_nZoomComboBoxIndex == 1	||
		m_bFullScreenMode)
		SetScrollSizes(MM_TEXT, CSize(0, 0));
	else
	{
		CRect ClientRect;
		GetClientRect(&ClientRect);

		CSize ZoomedSize(	Round(pDoc->m_DocRect.Width() * pDoc->m_dZoomFactor),
							Round(pDoc->m_DocRect.Height() * pDoc->m_dZoomFactor));
		if ((ZoomedSize.cx < 0) || (ZoomedSize.cy < 0))
		{
			ZoomedSize.cx = 0;
			ZoomedSize.cy = 0;
		}
		if (!pDoc->m_bNoBorders)
		{
			ZoomedSize.cx += 2 * ::SystemDPIScale(MIN_BORDER);
			ZoomedSize.cy += 2 * ::SystemDPIScale(MIN_BORDER);
		}
		SetScrollSizes(MM_TEXT, ZoomedSize);
	}
}

void CPictureView::UpdateZoomRect()
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CRect ClientRect;
	GetClientRect(&ClientRect);

	// Fit Zoom Factor
	BOOL bFit = FALSE;
	if (pDoc->m_nZoomComboBoxIndex == 0)		// Fit
	{
		FitZoomFactor();
		bFit = TRUE;
	}
	else if (pDoc->m_nZoomComboBoxIndex == 1)	// Fit Big
	{
		FitBigZoomFactor();
		bFit = TRUE;
	}
	else if (m_bFullScreenMode)
	{
		FitZoomFactor();						// Fit
		bFit = TRUE;
	}

	// New Zoom Size
	CSize ZoomedSize(	Round(pDoc->m_DocRect.Width() * pDoc->m_dZoomFactor),
						Round(pDoc->m_DocRect.Height() * pDoc->m_dZoomFactor));
	
	// New Zoom Rect
	if (pDoc->m_bNoBorders)
	{
		if (bFit || ZoomedSize.cx <= ClientRect.Width())
		{
			m_ZoomRect.left = (ClientRect.Width() - ZoomedSize.cx) / 2;
			m_ZoomRect.right = m_ZoomRect.left + ZoomedSize.cx;
		}
		else
		{
			m_ZoomRect.left = 0;
			m_ZoomRect.right = ZoomedSize.cx;
		}

		if (bFit || ZoomedSize.cy <= ClientRect.Height())
		{
			m_ZoomRect.top = (ClientRect.Height() - ZoomedSize.cy) / 2;
			m_ZoomRect.bottom = m_ZoomRect.top + ZoomedSize.cy;
		}
		else
		{
			m_ZoomRect.top = 0;
			m_ZoomRect.bottom = ZoomedSize.cy;
		}
	}
	else
	{
		if (bFit || ZoomedSize.cx <= ClientRect.Width() - 2 * ::SystemDPIScale(MIN_BORDER))
		{
			m_ZoomRect.left = (ClientRect.Width() - ZoomedSize.cx) / 2;
			m_ZoomRect.right = m_ZoomRect.left + ZoomedSize.cx;
		}
		else
		{
			m_ZoomRect.left = ::SystemDPIScale(MIN_BORDER);
			m_ZoomRect.right = ZoomedSize.cx + ::SystemDPIScale(MIN_BORDER);
		}

		if (bFit || ZoomedSize.cy <= ClientRect.Height() - 2 * ::SystemDPIScale(MIN_BORDER))
		{
			m_ZoomRect.top = (ClientRect.Height() - ZoomedSize.cy) / 2;
			m_ZoomRect.bottom = m_ZoomRect.top + ZoomedSize.cy;
		}
		else
		{
			m_ZoomRect.top = ::SystemDPIScale(MIN_BORDER);
			m_ZoomRect.bottom = ZoomedSize.cy + ::SystemDPIScale(MIN_BORDER);
		}
	}

	// Calc. Zoomed Pixel Align
	CalcZoomedPixelAlign();

	// Crop Zoom Rect
	if (pDoc->m_bCrop)
	{
		m_CropZoomRect = m_ZoomRect;
		m_CropZoomRect.DeflateRect(	Round(pDoc->m_rcCropDelta.left*pDoc->m_dZoomFactor),
									Round(pDoc->m_rcCropDelta.top*pDoc->m_dZoomFactor),
									Round(pDoc->m_rcCropDelta.right*pDoc->m_dZoomFactor),
									Round(pDoc->m_rcCropDelta.bottom*pDoc->m_dZoomFactor));
		m_CropZoomRect.NormalizeRect();
	}
}

void CPictureView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	switch (nChar)
	{
		case VK_CONTROL :
		case VK_SHIFT :
			if (pDoc->m_bZoomTool)
			{
				pDoc->m_bZoomToolMinus = FALSE;
				UpdateCursor();
			}
			else if (pDoc->m_bCrop)
			{
				if (m_bCropMaintainAspectRatio)
				{
					m_bCropMaintainAspectRatio = FALSE;
					m_dCropAspectRatio = 1.0;
				}
			}
			break;

		case VK_MENU :	// Alt Gr returns this
			if (pDoc->m_bZoomTool)
			{
				pDoc->m_bZoomToolMinus = FALSE;
				UpdateCursor();
			}
			break;

		default : 
			break;
	}
	
	CUImagerView::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CPictureView::OnSize(UINT nType, int cx, int cy) 
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CUImagerView::OnSize(nType, cx, cy);
	pDoc->CancelTransition();
	pDoc->CancelLoadFullJpegTransition();
}

LONG CPictureView::OnRecursiveFileFindDone(WPARAM wparam, LPARAM lparam)
{
	CPictureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Update
	pDoc->SetDocumentTitle();
	pDoc->UpdateImageInfo();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CPictureView diagnostics

#ifdef _DEBUG
void CPictureView::AssertValid() const
{
	CUImagerView::AssertValid();
}

void CPictureView::Dump(CDumpContext& dc) const
{
	CUImagerView::Dump(dc);
}

CPictureDoc* CPictureView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPictureDoc)));
	return (CPictureDoc*)m_pDocument;
}
#endif //_DEBUG
