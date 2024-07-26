#include "stdafx.h"
#include "uImager.h"
#include "MainFrm.h"
#include "VideoDeviceDoc.h"
#include "PostDelayedMessage.h"
#include "VideoDeviceView.h"
#include "CameraAdvancedSettingsDlg.h"
#include "DxCapture.h"
#include "MyMemDC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

IMPLEMENT_DYNCREATE(CVideoDeviceView, CUImagerView)

BEGIN_MESSAGE_MAP(CVideoDeviceView, CUImagerView)
	//{{AFX_MSG_MAP(CVideoDeviceView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_VIEW_FULLSCREEN, OnViewFullscreen)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FULLSCREEN, OnUpdateViewFullscreen)
	ON_COMMAND(ID_EDIT_ACTIVATE_ALLZONES, OnEditActivateAllZones)
	ON_COMMAND(ID_EDIT_DEACTIVATE_ALLZONES, OnEditDeactivateAllZones)
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_FONTSIZE_4, OnFrameTimeFontSize4)
	ON_UPDATE_COMMAND_UI(ID_FONTSIZE_4, OnUpdateFrameTimeFontSize4)
	ON_COMMAND(ID_FONTSIZE_5, OnFrameTimeFontSize5)
	ON_UPDATE_COMMAND_UI(ID_FONTSIZE_5, OnUpdateFrameTimeFontSize5)
	ON_COMMAND(ID_FONTSIZE_6, OnFrameTimeFontSize6)
	ON_UPDATE_COMMAND_UI(ID_FONTSIZE_6, OnUpdateFrameTimeFontSize6)
	ON_COMMAND(ID_FONTSIZE_7, OnFrameTimeFontSize7)
	ON_UPDATE_COMMAND_UI(ID_FONTSIZE_7, OnUpdateFrameTimeFontSize7)
	ON_COMMAND(ID_FONTSIZE_8, OnFrameTimeFontSize8)
	ON_UPDATE_COMMAND_UI(ID_FONTSIZE_8, OnUpdateFrameTimeFontSize8)
	ON_COMMAND(ID_FONTSIZE_9, OnFrameTimeFontSize9)
	ON_UPDATE_COMMAND_UI(ID_FONTSIZE_9, OnUpdateFrameTimeFontSize9)
	ON_COMMAND(ID_FONTSIZE_10, OnFrameTimeFontSize10)
	ON_UPDATE_COMMAND_UI(ID_FONTSIZE_10, OnUpdateFrameTimeFontSize10)
	ON_COMMAND(ID_FONTSIZE_11, OnFrameTimeFontSize11)
	ON_UPDATE_COMMAND_UI(ID_FONTSIZE_11, OnUpdateFrameTimeFontSize11)
	ON_COMMAND(ID_FONTSIZE_12, OnFrameTimeFontSize12)
	ON_UPDATE_COMMAND_UI(ID_FONTSIZE_12, OnUpdateFrameTimeFontSize12)
	ON_COMMAND(ID_FONTSIZE_14, OnFrameTimeFontSize14)
	ON_UPDATE_COMMAND_UI(ID_FONTSIZE_14, OnUpdateFrameTimeFontSize14)
	ON_COMMAND(ID_FONTSIZE_16, OnFrameTimeFontSize16)
	ON_UPDATE_COMMAND_UI(ID_FONTSIZE_16, OnUpdateFrameTimeFontSize16)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_THREADSAFE_CAPTURECAMERABASICSETTINGS, OnThreadSafeCaptureCameraBasicSettings)
	ON_MESSAGE(WM_THREADSAFE_UPDATE_PHPPARAMS, OnThreadSafeUpdatePhpParams)
	ON_MESSAGE(WM_THREADSAFE_SAVE_SAVESCOUNT, OnThreadSafeSaveSavesCount)
	ON_MESSAGE(WM_THREADSAFE_DVCHANGEVIDEOFORMAT, OnThreadSafeDVChangeVideoFormat)
	ON_MESSAGE(WM_THREADSAFE_INIT_MOVDET, OnThreadSafeInitMovDet)
	ON_MESSAGE(WM_DIRECTSHOW_GRAPHNOTIFY, OnDirectShowGraphNotify)
END_MESSAGE_MAP()

	
CVideoDeviceView::CVideoDeviceView()
{
	// Init vars
	m_MovDetSingleZoneSensitivity = 1;
	m_bMovDetUnsupportedVideoSize = FALSE;
	m_bMovDetUnsupportedZonesSize = FALSE;
}

CVideoDeviceView::~CVideoDeviceView()
{
	
}

#ifdef _DEBUG
void CVideoDeviceView::AssertValid() const
{
	CUImagerView::AssertValid();
}

void CVideoDeviceView::Dump(CDumpContext& dc) const
{
	CUImagerView::Dump(dc);
}

CVideoDeviceDoc* CVideoDeviceView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CVideoDeviceDoc)));
	return (CVideoDeviceDoc*)m_pDocument;
}
#endif //_DEBUG

LONG CVideoDeviceView::OnThreadSafeDVChangeVideoFormat(WPARAM wparam, LPARAM lparam)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	int nPrevTotalDelay = (int)lparam;
	if (pDoc)
	{
		if (nPrevTotalDelay > PROCESSFRAME_MAX_RETRY_TIME || pDoc->IsProcessFrameStopped(PROCESSFRAME_DVFORMATDIALOG))
		{
			if (!pDoc->m_bClosing)
			{
				// Stop watchdog thread
				pDoc->m_WatchdogThread.Kill();

				// Stop
				pDoc->m_pDxCapture->Stop();

				// Show dialog
				pDoc->m_pDxCapture->ShowDVFormatDlg();

				// Update
				pDoc->m_bSizeToDoc = TRUE;
				pDoc->OnChangeDxVideoFormat();

				// Reset vars
				pDoc->m_dwFrameCountUp = 0U;
				pDoc->m_llNextSnapshotUpTime = (LONGLONG)::GetTickCount64();
				pDoc->m_llCurrentInitUpTime = pDoc->m_llNextSnapshotUpTime;

				// Restart
				if (pDoc->m_pDxCapture->Run())
				{
					// Restart process frame
					pDoc->StartProcessFrame(PROCESSFRAME_DVFORMATDIALOG);
				}

				// Restart watchdog thread
				pDoc->m_WatchdogThread.Start();
			}
			pDoc->m_bStopAndChangeDVFormat = FALSE;

			return 1;
		}
		else
		{
			double dFrameRate = pDoc->m_dEffectiveFrameRate;
			int delay;
			if (dFrameRate >= 1.0)
				delay = Round(1000.0 / dFrameRate); // In ms
			else
				delay = 1000;
			CPostDelayedMessageThread::PostDelayedMessage(	GetSafeHwnd(),
															WM_THREADSAFE_DVCHANGEVIDEOFORMAT,
															delay, 0, nPrevTotalDelay + delay);
			return 0;
		}
	}
	else
		return 0;
}

LONG CVideoDeviceView::OnThreadSafeCaptureCameraBasicSettings(WPARAM wparam, LPARAM lparam)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc && !pDoc->m_bClosing)
	{
		pDoc->CaptureCameraBasicSettings();
		return 1;
	}
	else
		return 0;
}

LONG CVideoDeviceView::OnThreadSafeUpdatePhpParams(WPARAM wparam, LPARAM lparam)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc && !pDoc->m_bClosing)
	{
		CString sWidth, sHeight;
		sWidth.Format(_T("%d"), pDoc->m_DocRect.right);
		sHeight.Format(_T("%d"), pDoc->m_DocRect.bottom);
		pDoc->PhpConfigFileSetParam(PHPCONFIG_WIDTH, sWidth);
		pDoc->PhpConfigFileSetParam(PHPCONFIG_HEIGHT, sHeight);
		return 1;
	}
	else
		return 0;
}

LONG CVideoDeviceView::OnThreadSafeSaveSavesCount(WPARAM wparam, LPARAM lparam)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc && !pDoc->m_bClosing)
	{
		pDoc->SaveSavesCount();
		return 1;
	}
	else
		return 0;
}

LONG CVideoDeviceView::OnThreadSafeInitMovDet(WPARAM wparam, LPARAM lparam)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	int i;
	int nMovDetXZonesCount = 0;
	int nMovDetYZonesCount = 0;

	// Video size check
	if (pDoc->m_DocRect.Width() <= 0 || pDoc->m_DocRect.Height() <= 0)
		return 0;
	
	// Maximum zones size for X and Y direction
	// and minimum zones count for X and Y direction
	int nMaxEdgePix = MAX(pDoc->m_DocRect.Width(), pDoc->m_DocRect.Height());
	int nMaxSizePix;
	switch (pDoc->m_nDetectionZoneSize)
	{
		// Big
		case 0 :	
			if (nMaxEdgePix <= 352)
				nMaxSizePix = 32;
			else
				nMaxSizePix = 72;
			break;

		// Medium
		case 1 :	
			if (nMaxEdgePix <= 352)
				nMaxSizePix = 16;
			else
				nMaxSizePix = 32;
			break;

		// Small
		default :	
			if (nMaxEdgePix <= 352)
				nMaxSizePix = 8;
			else
				nMaxSizePix = 16;
			break;
	}
	int nMinEdgePix = MIN(pDoc->m_DocRect.Width(), pDoc->m_DocRect.Height());
	int nMovDetMinXZonesCount, nMovDetMinYZonesCount;
	if (nMinEdgePix <= 352)
		nMovDetMinXZonesCount = nMovDetMinYZonesCount = MOVDET_MIN_ZONES_XORY;
	else
		nMovDetMinXZonesCount = nMovDetMinYZonesCount = 2 * MOVDET_MIN_ZONES_XORY;
	double dRatio = (double)(pDoc->m_DocRect.Width()) / (double)(pDoc->m_DocRect.Height());
	if (dRatio >= 1.0)
		nMovDetMinXZonesCount = (int)(nMovDetMinYZonesCount * dRatio);
	else
		nMovDetMinYZonesCount = (int)(nMovDetMinXZonesCount / dRatio);
	
	// Calc. zones count in X direction
	for (i = nMaxSizePix ; i > 0 ; i -= 8)
	{
		if ((pDoc->m_DocRect.Width() % i) == 0 &&
			(pDoc->m_DocRect.Width() / i) >= nMovDetMinXZonesCount)
		{
			nMovDetXZonesCount = pDoc->m_DocRect.Width() / i;
			break;
		}
	}

	// Calc. zones count in Y direction
	for (i = nMaxSizePix ; i > 0 ; i -= 8)
	{
		if ((pDoc->m_DocRect.Height() % i) == 0 &&
			(pDoc->m_DocRect.Height() / i) >= nMovDetMinYZonesCount)
		{
			nMovDetYZonesCount = pDoc->m_DocRect.Height() / i;
			break;
		}
	}

	// Total number of zones
	int nMovDetTotalZones = nMovDetXZonesCount * nMovDetYZonesCount;

	// Check and update doc vars
	if (nMovDetTotalZones == 0 || nMovDetTotalZones > MOVDET_MAX_ZONES)
	{
		if (nMovDetTotalZones == 0)
		{
			if (!m_bMovDetUnsupportedVideoSize && !((CUImagerApp*)::AfxGetApp())->m_bServiceProcess)
				::AfxGetMainFrame()->PopupNotificationWnd(APPNAME_NOEXT, ML_STRING(1836, "Cannot record, unsupported video size!"), 0);
			m_bMovDetUnsupportedVideoSize = TRUE;
			m_bMovDetUnsupportedZonesSize = FALSE;
		}
		else
		{
			if (!m_bMovDetUnsupportedZonesSize && !((CUImagerApp*)::AfxGetApp())->m_bServiceProcess)
				::AfxGetMainFrame()->PopupNotificationWnd(APPNAME_NOEXT, ML_STRING(1837, "Cannot record, choose a bigger zones size!"), 0);
			m_bMovDetUnsupportedZonesSize = TRUE;
			m_bMovDetUnsupportedVideoSize = FALSE;
		}
		pDoc->m_nMovDetTotalZones = 0;
		return 0;
	}
	else
	{
		if ((m_bMovDetUnsupportedVideoSize || m_bMovDetUnsupportedZonesSize) && !((CUImagerApp*)::AfxGetApp())->m_bServiceProcess)
			::AfxGetMainFrame()->CloseNotificationWnd();
		m_bMovDetUnsupportedVideoSize = m_bMovDetUnsupportedZonesSize = FALSE;
		pDoc->m_nMovDetXZonesCount = nMovDetXZonesCount;
		pDoc->m_nMovDetYZonesCount = nMovDetYZonesCount;
		pDoc->m_nMovDetTotalZones = nMovDetTotalZones;
	}

	// Update the old detection zone size so that this function is not called again
	if (pDoc->m_nOldDetectionZoneSize != pDoc->m_nDetectionZoneSize)
		pDoc->m_nOldDetectionZoneSize = pDoc->m_nDetectionZoneSize;

	return 1;
}

int CVideoDeviceView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	CToolBarChildFrame* pFrame = (CToolBarChildFrame*)GetParentFrame();
	if ((pFrame != NULL) && (pFrame->IsKindOf(RUNTIME_CLASS(CToolBarChildFrame))))
		pFrame->SetToolBar(&m_VideoDeviceToolBar);

	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

__forceinline void CVideoDeviceView::DrawZoneSensitivity(int i, HDC hDC, const RECT& rcDetZone, int n, int m,
		HBRUSH hBkgndBrush, HBRUSH hBrush5, HBRUSH hBrush10, HBRUSH hBrush25, HBRUSH hBrush50, HBRUSH hBrush100)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Center
	RECT rc;
	POINT ptCenter;
	ptCenter.x = rcDetZone.left + (rcDetZone.right - rcDetZone.left) / 2;
	ptCenter.y = rcDetZone.top + (rcDetZone.bottom - rcDetZone.top) / 2;

	// Fill
	// - size n:    can be 0, 1, 2, 3, ...
	// - margin m:  can be 0, 1, 2, 3, ...
	// - FillRect() includes left and top, but excludes right and bottom.
	if (pDoc->m_DoMovementDetection[i] >= 1)
	{
		rc.left = ptCenter.x - (3+n+m);
		rc.top = ptCenter.y - (3+n+m);
		rc.right = ptCenter.x + (4+n+m);
		rc.bottom = ptCenter.y + (4+n+m);
		::FillRect(hDC, &rc, hBkgndBrush);
		rc.left = ptCenter.x - (2+n);
		rc.top = ptCenter.y - (2+n);
		rc.right = ptCenter.x + (3+n);
		rc.bottom = ptCenter.y + (3+n);
	}
	if (pDoc->m_DoMovementDetection[i] >= 20)		// 5 %
		::FillRect(hDC, &rc, hBrush5);
	else if (pDoc->m_DoMovementDetection[i] >= 10)	// 10 %
		::FillRect(hDC, &rc, hBrush10);
	else if (pDoc->m_DoMovementDetection[i] >= 4)	// 25 %
		::FillRect(hDC, &rc, hBrush25);
	else if (pDoc->m_DoMovementDetection[i] >= 2)	// 50 %
		::FillRect(hDC, &rc, hBrush50);
	else if (pDoc->m_DoMovementDetection[i] >= 1)	// 100 %
		::FillRect(hDC, &rc, hBrush100);
}

void CVideoDeviceView::DrawZones(HDC hDC, const CRect& rcClient)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->m_nMovDetTotalZones > 0)
	{
		RECT rcDetZone;
		double dZoneWidth = (double)rcClient.Width() / (double)pDoc->m_nMovDetXZonesCount;
		double dZoneHeight = (double)rcClient.Height() / (double)pDoc->m_nMovDetYZonesCount;
		int nSensitivityRectSize;
		int nSensitivityRectMargin;
		if (dZoneWidth <= 16.0 || dZoneHeight <= 16.0)
		{
			nSensitivityRectSize = 0;
			nSensitivityRectMargin = 0;
		}
		else if (dZoneWidth <= 32.0 || dZoneHeight <= 32.0)
		{
			nSensitivityRectSize = 1;
			nSensitivityRectMargin = 0;
		}
		else if (dZoneWidth <= 64.0 || dZoneHeight <= 64.0)
		{
			nSensitivityRectSize = 2;
			nSensitivityRectMargin = 1;
		}
		else if (dZoneWidth <= 96.0 || dZoneHeight <= 96.0)
		{
			nSensitivityRectSize = 4;
			nSensitivityRectMargin = 2;
		}
		else if (dZoneWidth <= 128.0 || dZoneHeight <= 128.0)
		{
			nSensitivityRectSize = 6;
			nSensitivityRectMargin = 2;
		}
		else
		{
			nSensitivityRectSize = 8;
			nSensitivityRectMargin = 3;
		}

		if (pDoc->m_nShowEditDetectionZones)
		{
			// Common
			HBRUSH hSensitivityBkgndBrush = ::CreateSolidBrush(MOVDET_SENSITIVITY_BKGCOLOR);
			HBRUSH hSensitivityBrush5 = ::CreateSolidBrush(MOVDET_SENSITIVITY_COLOR5);
			HBRUSH hSensitivityBrush10 = ::CreateSolidBrush(MOVDET_SENSITIVITY_COLOR10);
			HBRUSH hSensitivityBrush25 = ::CreateSolidBrush(MOVDET_SENSITIVITY_COLOR25);
			HBRUSH hSensitivityBrush50 = ::CreateSolidBrush(MOVDET_SENSITIVITY_COLOR50);
			HBRUSH hSensitivityBrush100 = ::CreateSolidBrush(MOVDET_SENSITIVITY_COLOR100);
			HGDIOBJ hOldBrush = ::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));
			int nLastBottomEdge;

			// Draw Zones where Detection is enabled
			HPEN hPenSelectedZones = ::CreatePen(PS_SOLID, 1, MOVDET_SELECTED_ZONES_COLOR);
			HGDIOBJ hOldPenSelectedZones = ::SelectObject(hDC, hPenSelectedZones);
			nLastBottomEdge = 1;
			for (int y = 0; y < pDoc->m_nMovDetYZonesCount; y++)
			{
				int nLastRightEdge = 1;
				for (int x = 0; x < pDoc->m_nMovDetXZonesCount; x++)
				{
					int nZoneOffsetX = (int)(x * dZoneWidth);
					int nZoneOffsetY = (int)(y * dZoneHeight);
					rcDetZone.left = nLastRightEdge - 1;
					rcDetZone.top = nLastBottomEdge - 1;
					rcDetZone.right = (x == (pDoc->m_nMovDetXZonesCount - 1) ? rcClient.right : (int)(nZoneOffsetX + dZoneWidth));
					rcDetZone.bottom = (y == (pDoc->m_nMovDetYZonesCount - 1) ? rcClient.bottom : (int)(nZoneOffsetY + dZoneHeight));
					int i = x + y * pDoc->m_nMovDetXZonesCount;
					if (pDoc->m_DoMovementDetection[i])
					{
						DrawZoneSensitivity(i, hDC, rcDetZone, nSensitivityRectSize, nSensitivityRectMargin,
							hSensitivityBkgndBrush, hSensitivityBrush5, hSensitivityBrush10, 
							hSensitivityBrush25, hSensitivityBrush50, hSensitivityBrush100);
						::Rectangle(hDC, rcDetZone.left, rcDetZone.top, rcDetZone.right, rcDetZone.bottom);
					}
					nLastRightEdge = rcDetZone.right;
				}
				nLastBottomEdge = rcDetZone.bottom;
			}
			::SelectObject(hDC, hOldPenSelectedZones);
			::DeleteObject(hPenSelectedZones);

			// Draw Detected Zones
			HPEN hPenDetectingZones = ::CreatePen(PS_SOLID, 1, MOVDET_DETECTING_ZONES_COLOR);
			HGDIOBJ hOldPenDetectingZones = ::SelectObject(hDC, hPenDetectingZones);
			nLastBottomEdge = 1;
			for (int y = 0; y < pDoc->m_nMovDetYZonesCount; y++)
			{
				int nLastRightEdge = 1;
				for (int x = 0; x < pDoc->m_nMovDetXZonesCount; x++)
				{
					int nZoneOffsetX = (int)(x * dZoneWidth);
					int nZoneOffsetY = (int)(y * dZoneHeight);
					rcDetZone.left = nLastRightEdge - 1;
					rcDetZone.top = nLastBottomEdge - 1;
					rcDetZone.right = (x == (pDoc->m_nMovDetXZonesCount - 1) ? rcClient.right : (int)(nZoneOffsetX + dZoneWidth));
					rcDetZone.bottom = (y == (pDoc->m_nMovDetYZonesCount - 1) ? rcClient.bottom : (int)(nZoneOffsetY + dZoneHeight));
					int i = x + y * pDoc->m_nMovDetXZonesCount;
					if (pDoc->m_MovementDetections[i])
					{
						DrawZoneSensitivity(i, hDC, rcDetZone, nSensitivityRectSize, nSensitivityRectMargin,
							hSensitivityBkgndBrush, hSensitivityBrush5, hSensitivityBrush10, 
							hSensitivityBrush25, hSensitivityBrush50, hSensitivityBrush100);
						::Rectangle(hDC, rcDetZone.left, rcDetZone.top, rcDetZone.right, rcDetZone.bottom);
						::MoveToEx(hDC, rcDetZone.left + (rcDetZone.right - rcDetZone.left) / 4, rcDetZone.top, NULL);
						::LineTo(hDC, rcDetZone.left, rcDetZone.top + (rcDetZone.right - rcDetZone.left) / 4);
					}
					nLastRightEdge = rcDetZone.right;
				}
				nLastBottomEdge = rcDetZone.bottom;
			}
			::SelectObject(hDC, hOldPenDetectingZones);
			::DeleteObject(hPenDetectingZones);

			// Clean-up common
			::SelectObject(hDC, hOldBrush);
			::DeleteObject(hSensitivityBrush100);
			::DeleteObject(hSensitivityBrush50);
			::DeleteObject(hSensitivityBrush25);
			::DeleteObject(hSensitivityBrush10);
			::DeleteObject(hSensitivityBrush5);
			::DeleteObject(hSensitivityBkgndBrush);
		}
	}
}

BOOL CVideoDeviceView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CVideoDeviceView::DrawRecDot(HDC hDC, const CRect& rcClient)
{
	int nDiameter = ::SystemDPIScale(REC_DOT_DIAMETER);
	int nMarginRight = ::SystemDPIScale(REC_DOT_MARGIN_RIGHT);
	int nMarginBottom = ::SystemDPIScale(REC_DOT_MARGIN_BOTTOM);
	int nUnit = ::SystemDPIScale(1);

	// Create pen and brush
	HPEN hPen = ::CreatePen(PS_SOLID, nUnit, REC_DOT_OUTLINE_COLOR);
	HBRUSH hBrush = ::CreateSolidBrush(REC_DOT_COLOR);

	// Draw circle
	HGDIOBJ hOldPen = ::SelectObject(hDC, hPen);
	HGDIOBJ hOldBrush = ::SelectObject(hDC, hBrush);
	::Ellipse(	hDC,
				rcClient.right - nDiameter - nMarginRight,
				rcClient.bottom - nDiameter - nMarginBottom,
				rcClient.right - nMarginRight,
				rcClient.bottom - nMarginBottom);

	// Clean-up
	::SelectObject(hDC, hOldBrush);
	::SelectObject(hDC, hOldPen);
	::DeleteObject(hPen);
	::DeleteObject(hBrush);
}

void CVideoDeviceView::DrawSaveProgress(HDC hDC, const CRect& rcClient, BOOL bRecDotVisible, int nProgress)
{
	int nWidth = ::SystemDPIScale(REC_PROGRESS_WIDTH);
	int nHeight = ::SystemDPIScale(REC_PROGRESS_HEIGHT);
	int nMarginRight = ::SystemDPIScale(bRecDotVisible ? REC_DOT_DIAMETER + 2*REC_DOT_MARGIN_RIGHT : REC_DOT_MARGIN_RIGHT);
	int nMarginBottom = ::SystemDPIScale(REC_PROGRESS_MARGIN_BOTTOM);
	int nUnit = ::SystemDPIScale(1);

	// Create brushes
	HBRUSH hBrushOutline = ::CreateSolidBrush(REC_PROGRESS_OUTLINE_COLOR);
	HBRUSH hBrushContainer = ::CreateSolidBrush(REC_PROGRESS_CONTAINER_COLOR);
	HBRUSH hBrushBar = ::CreateSolidBrush(DRAW_PROGRESS_COLOR);

	// Draw outline
	CRect rcDraw(rcClient.right - nWidth - nMarginRight,
				rcClient.bottom - nHeight - nMarginBottom,
				rcClient.right - nMarginRight,
				rcClient.bottom - nMarginBottom);
	::FillRect(hDC, rcDraw, hBrushOutline);

	// Draw container
	rcDraw.DeflateRect(nUnit, nUnit);
	::FillRect(hDC, rcDraw, hBrushContainer);

	// Draw bar
	int nBarSize = nProgress * rcDraw.Width() / 100;
	rcDraw.right = rcDraw.left + nBarSize;
	::FillRect(hDC, rcDraw, hBrushBar);

	// Clean-up
	::DeleteObject(hBrushOutline);
	::DeleteObject(hBrushContainer);
	::DeleteObject(hBrushBar);
}

void CVideoDeviceView::OnDraw(CDC* pDC)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Flicker free drawing
	CRect rcClient;
	GetClientRect(&rcClient);
	CMyMemDC MemDC(pDC, &rcClient);

	// Draw
	if (pDoc->m_bCaptureStarted && !pDoc->m_bWatchDogVideoAlarm)
	{
		// Enter CS
		::EnterCriticalSection(&pDoc->m_csDib);

		// Draw Frame
		pDoc->m_pDrawDibRGB32->Paint(	MemDC.GetSafeHdc(),
										&rcClient,
										CRect(0, 0, pDoc->m_pDrawDibRGB32->GetWidth(), pDoc->m_pDrawDibRGB32->GetHeight()),
										FALSE);

		// Leave CS
		::LeaveCriticalSection(&pDoc->m_csDib);

		// Draw Zones
		if (pDoc->m_nShowEditDetectionZones)
			DrawZones(MemDC.GetSafeHdc(), rcClient);

		// Draw REC Dot Symbol
		BOOL bDetectingMinLengthMovement = pDoc->m_bDetectingMinLengthMovement;
		if (bDetectingMinLengthMovement)
			DrawRecDot(MemDC.GetSafeHdc(), rcClient);

		// Draw Save Progress
		int nSaveProgress = pDoc->m_SaveFrameListThread.GetSaveProgress();
		if (nSaveProgress < 100)
			DrawSaveProgress(MemDC.GetSafeHdc(), rcClient, bDetectingMinLengthMovement, nSaveProgress);
	}
	else
	{
		//  Erase Background
		CBrush br;
		br.CreateSolidBrush(DRAW_BKG_COLOR);	
		MemDC.FillRect(&rcClient, &br);

		// Create font
		if (!(HFONT)m_GDIDrawFont)
		{
			LOGFONT lf;
			memset(&lf, 0, sizeof(lf));
			_tcscpy(lf.lfFaceName, g_szDefaultFontFace);
			lf.lfHeight = -MulDiv(11, pDC->GetDeviceCaps(LOGPIXELSY), 72);
			lf.lfWeight = FW_NORMAL;
			lf.lfItalic = 0;
			lf.lfUnderline = 0;
			m_GDIDrawFont.CreateFontIndirect(&lf);
		}

		// Set colors and font
		COLORREF crOldTextColor = MemDC.SetTextColor(DRAW_MESSAGE_COLOR);
		int nOldBkMode = MemDC.SetBkMode(OPAQUE);
		COLORREF crOldBkColor = MemDC.SetBkColor(DRAW_BKG_COLOR);
		CFont* pOldFont = MemDC.SelectObject(&m_GDIDrawFont);

		// Display message
		CString sMsg(ML_STRING(1567, "Connecting..."));
		::EnterCriticalSection(&pDoc->m_csConnectionError);
		if (!pDoc->m_sLastConnectionError.IsEmpty())
			sMsg = pDoc->m_sLastConnectionError;
		::LeaveCriticalSection(&pDoc->m_csConnectionError);
		MemDC.DrawText(sMsg, -1, &rcClient, (DT_CENTER | DT_VCENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
		TEXTMETRIC TextMetrics;
		MemDC.GetTextMetrics(&TextMetrics);
		int nBoxLength = TextMetrics.tmHeight / 4;
		CPoint ptCenter(rcClient.CenterPoint());
		ptCenter.y += TextMetrics.tmHeight;
		CRect rcBoxMiddle(	ptCenter.x - nBoxLength, ptCenter.y - nBoxLength,
							ptCenter.x + nBoxLength, ptCenter.y + nBoxLength);
		CRect rcBoxLeft(rcBoxMiddle);
		rcBoxLeft.OffsetRect(-3*nBoxLength, 0);
		CRect rcBoxLeftLeft(rcBoxLeft);
		rcBoxLeftLeft.OffsetRect(-3*nBoxLength, 0);
		CRect rcBoxRight(rcBoxMiddle);
		rcBoxRight.OffsetRect(3*nBoxLength, 0);
		CRect rcBoxRightRight(rcBoxRight);
		rcBoxRightRight.OffsetRect(3*nBoxLength, 0);
		int nCount = (int)((::GetTickCount() / 1000U) % 5U);
		MemDC.FillSolidRect(rcBoxLeftLeft, DRAW_PROGRESS_COLOR);
		if (nCount >= 1)
			MemDC.FillSolidRect(rcBoxLeft, DRAW_PROGRESS_COLOR);
		if (nCount >= 2)
			MemDC.FillSolidRect(rcBoxMiddle, DRAW_PROGRESS_COLOR);
		if (nCount >= 3)
			MemDC.FillSolidRect(rcBoxRight, DRAW_PROGRESS_COLOR);
		if (nCount == 4)
			MemDC.FillSolidRect(rcBoxRightRight, DRAW_PROGRESS_COLOR);

		// Clean-up
		MemDC.SetBkMode(nOldBkMode);
		MemDC.SetBkColor(crOldBkColor);
		MemDC.SetTextColor(crOldTextColor);
		MemDC.SelectObject(pOldFont);
	}
}

void CVideoDeviceView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	ForceCursor();

	if (pDoc->m_nShowEditDetectionZones)
	{
		CMenu menu;
		if (pDoc->m_nShowEditDetectionZones == 1)
			VERIFY(menu.LoadMenu(IDR_CONTEXT_ACTIVATE_ZONES));
		else if (pDoc->m_nShowEditDetectionZones == 2)
			VERIFY(menu.LoadMenu(IDR_CONTEXT_DEACTIVATE_ZONES));
		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);
		SetForegroundWindow();
		ClientToScreen(&point);
		pPopup->TrackPopupMenu(TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, ::AfxGetMainFrame());
	}
	else
	{
		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_CONTEXT_VIDEO_DEVICE));
		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);
		SetForegroundWindow();
		ClientToScreen(&point);
		pPopup->TrackPopupMenu(TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, ::AfxGetMainFrame());
	}

	CUImagerView::OnRButtonDown(nFlags, point);

	ForceCursor(FALSE);
}

void CVideoDeviceView::OnInitialUpdate() 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	// Set Pointers
	pDoc->SetView(this);
	pDoc->SetFrame((CVideoDeviceChildFrame*)GetParentFrame());

	// Set Initial Tab Title
	pDoc->m_sTabTitle = pDoc->GetTitle();

	// Call Base Class Implementation
	CUImagerView::OnInitialUpdate();
}

void CVideoDeviceView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// Necessary to init m_ZoomRect, called only once after OnInitialUpdate()
	// this because we are not issuing UpdateAllViews() from the doc.
	// Note: default CView implementation only calls Invalidate
	UpdateWindowSizes(TRUE, FALSE, FALSE);
}

void CVideoDeviceView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	EnableCursor();

	CUImagerView::OnLButtonDown(nFlags, point);

	if (pDoc->m_nShowEditDetectionZones && pDoc->m_nMovDetTotalZones > 0)
	{
		// Get client rectangle
		CRect rcClient;
		GetClientRect(&rcClient);

		// Make sure point is inside the frame
		if (point.x < 0 ||
			point.y < 0 ||
			point.x >= rcClient.Width() ||
			point.y >= rcClient.Height())
			return;

		// Calc. x and y offsets
		int x = 0;
		if (rcClient.Width() > 0)
			x = pDoc->m_nMovDetXZonesCount * point.x / rcClient.Width(); // note: point.x < rcClient.Width()  -> x < pDoc->m_nMovDetXZonesCount
		int y = 0;
		if (rcClient.Height() > 0)
			y = pDoc->m_nMovDetYZonesCount * point.y / rcClient.Height();// note: point.y < rcClient.Height() -> y < pDoc->m_nMovDetYZonesCount

		// The Selected Zone Index
		int nZone = x + y * pDoc->m_nMovDetXZonesCount;

		// Add / Remove Zone Value
		if (nZone >= 0 && nZone < MOVDET_MAX_ZONES)
		{
			int nNewDoMovementDetectionValue = (pDoc->m_nShowEditDetectionZones == 1 ? m_MovDetSingleZoneSensitivity : 0);
			if (nNewDoMovementDetectionValue != pDoc->m_DoMovementDetection[nZone])
			{
				pDoc->m_DoMovementDetection[nZone] = nNewDoMovementDetectionValue;
				pDoc->SaveZonesBlockSettings(nZone / MOVDET_MAX_ZONES_BLOCK_SIZE, pDoc->GetDevicePathName());
				Invalidate(FALSE);
			}
		}
	}
}

void CVideoDeviceView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// No fullscreen enter/exit when editing zones!
	if (pDoc->m_nShowEditDetectionZones)
	{
		EnableCursor();
		CScrollView::OnLButtonDblClk(nFlags, point);
	}
	else
		CUImagerView::OnLButtonDblClk(nFlags, point);
}

void CVideoDeviceView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	EnableCursor();
	CUImagerView::OnMButtonDown(nFlags, point);
}

void CVideoDeviceView::OnTimer(UINT nIDEvent) 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	switch (nIDEvent)
	{
		case ID_TIMER_RELOAD :
		{
			CString sRecordAutoSaveDir = pDoc->m_sRecordAutoSaveDir;
			sRecordAutoSaveDir.TrimRight(_T('\\'));

			// Load video source obscuration state
			pDoc->m_bObscureSource = ::IsExistingFile(sRecordAutoSaveDir + _T("\\") + CAMERA_IS_OBSCURED_FILENAME);

			// Load detection level
			int nDetectionLevel;
			if ((nDetectionLevel = CVideoDeviceDoc::ReadDetectionLevelFromFile(sRecordAutoSaveDir)) >= 0)
				pDoc->m_nDetectionLevel = CVideoDeviceDoc::ValidateDetectionLevel(nDetectionLevel);

			break;
		}
		default:
			break;
	}

	CUImagerView::OnTimer(nIDEvent);
}

void CVideoDeviceView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CMenu menu;
	CMenu* pPopup = NULL;
	CPoint Point;
	Point.x = 6;
	Point.y = 6;

	switch (nChar)
	{
		case VK_CONTROL:
			// Switch from Add to Remove
			if (pDoc->m_nShowEditDetectionZones == 1)
			{
				// Bit14: previous key state (1 if the key is down before the call, 0 if the key is up)
				if ((nFlags & (1 << 14)) == 0)
				{
					pDoc->m_nShowEditDetectionZones = 2;
					UpdateCursor();
					Invalidate(FALSE);
				}
			}
			// Switch from Remove to Add
			else if (pDoc->m_nShowEditDetectionZones == 2)
			{
				// Bit14: previous key state (1 if the key is down before the call, 0 if the key is up)
				if ((nFlags & (1 << 14)) == 0)
				{
					pDoc->m_nShowEditDetectionZones = 1;
					UpdateCursor();
					Invalidate(FALSE);
				}
			}
			break;

		case VK_ESCAPE :
			if (pDoc->m_pCameraAdvancedSettingsDlg && pDoc->m_pCameraAdvancedSettingsDlg->IsWindowVisible())
				pDoc->m_pCameraAdvancedSettingsDlg->Hide();
			else if (m_bFullScreenMode)
				::AfxGetMainFrame()->EnterExitFullscreen();	// Exit Full-Screen Mode
			else if (pDoc->m_nShowEditDetectionZones)
				pDoc->HideDetectionZones();
			break;

		case VK_INSERT :
			pDoc->m_bDoEditSnapshot = TRUE;
			break;

		case VK_APPS :
			ForceCursor();
			if (pDoc->m_nShowEditDetectionZones)
			{
				if (pDoc->m_nShowEditDetectionZones == 1)
					VERIFY(menu.LoadMenu(IDR_CONTEXT_ACTIVATE_ZONES));
				else if (pDoc->m_nShowEditDetectionZones == 2)
					VERIFY(menu.LoadMenu(IDR_CONTEXT_DEACTIVATE_ZONES));
				pPopup = menu.GetSubMenu(0);
				ASSERT(pPopup != NULL);
				SetForegroundWindow();
				ClientToScreen(&Point);
				pPopup->TrackPopupMenu(TPM_LEFTBUTTON|TPM_RIGHTBUTTON, Point.x, Point.y, ::AfxGetMainFrame());
			}
			else
			{
				VERIFY(menu.LoadMenu(IDR_CONTEXT_VIDEO_DEVICE));
				pPopup = menu.GetSubMenu(0);
				ASSERT(pPopup != NULL);
				SetForegroundWindow();
				ClientToScreen(&Point);
				pPopup->TrackPopupMenu(TPM_LEFTBUTTON|TPM_RIGHTBUTTON, Point.x, Point.y, ::AfxGetMainFrame());
			}
			ForceCursor(FALSE);
			break;

		default : 
			break;
	}
	
	CUImagerView::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CVideoDeviceView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	HCURSOR hCursor;
	
	// If Wait Cursor leave it!
	if (((CUImagerApp*)::AfxGetApp())->IsWaitCursor())
	{
		RestoreWaitCursor();
		return TRUE;
	}
	else if (pDoc->m_nShowEditDetectionZones == 1)
	{
		hCursor = ::AfxGetApp()->LoadCursor(IDC_ZONEPLUS_CURSOR);
		ASSERT(hCursor);
		::SetCursor(hCursor);
		return TRUE;
	}
	else if (pDoc->m_nShowEditDetectionZones == 2)
	{
		hCursor = ::AfxGetApp()->LoadCursor(IDC_ZONEMINUS_CURSOR);
		ASSERT(hCursor);
		::SetCursor(hCursor);
		return TRUE;
	}
	else
		return CUImagerView::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CVideoDeviceView::ReOpenDxDevice()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->m_pDxCapture)
	{
		// Reset vars
		pDoc->m_dwFrameCountUp = 0U;
		pDoc->m_llNextSnapshotUpTime = (LONGLONG)::GetTickCount64();
		pDoc->m_llCurrentInitUpTime = pDoc->m_llNextSnapshotUpTime;

		// Re-Open
		if (pDoc->m_pDxCapture->Open(	GetSafeHwnd(),
										-1,	// Re-open previous one
										pDoc->m_dFrameRate,
										pDoc->m_nDeviceFormatId,
										pDoc->m_nDeviceFormatWidth,
										pDoc->m_nDeviceFormatHeight,
										pDoc->m_pDxCapture->GetOpenMediaSubType()))
		{
			// Update format
			pDoc->OnChangeDxVideoFormat();
				
			// Start capturing video data
			if (pDoc->m_pDxCapture->Run())
			{
				// Select Input Id for Capture Devices with multiple inputs (S-Video, TV-Tuner,...)
				if (pDoc->m_nDeviceInputId >= 0 && pDoc->m_nDeviceInputId < pDoc->m_pDxCapture->GetInputsCount())
				{
					if (!pDoc->m_pDxCapture->SetCurrentInput(pDoc->m_nDeviceInputId))
						pDoc->m_nDeviceInputId = -1;
				}
				else
					pDoc->m_nDeviceInputId = pDoc->m_pDxCapture->SetDefaultInput();

				return TRUE;
			}
		}
	}

	return FALSE;
}

LONG CVideoDeviceView::OnDirectShowGraphNotify(WPARAM wparam, LPARAM lparam)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (pDoc->m_pDxCapture)
	{
		LONG evCode, evParam1, evParam2;
		while (pDoc->m_pDxCapture->GetEvent(&evCode,
											(LONG_PTR*)&evParam1, 
											(LONG_PTR*)&evParam2,
											0)) // Wait Timeout of 0ms
		{
			// Free
			pDoc->m_pDxCapture->FreeEvent(evCode, evParam1, evParam2);

			// Device unplugged or replugged
			if (evCode == EC_DEVICE_LOST)
			{
				// Device was removed
				if (evParam2 == 0)
				{
					pDoc->ConnectErr(ML_STRING(1568, "Unplugged"), pDoc->GetDeviceName());
                    break;
				}
				// Device is available again
				else if (evParam2 == 1)
				{
					// Re-Open
					pDoc->StopProcessFrame(PROCESSFRAME_DXREPLUGGED);
					if (ReOpenDxDevice())
					{
						pDoc->ClearConnectErr();
						pDoc->StartProcessFrame(PROCESSFRAME_DXREPLUGGED);
					}
					break;
				}
			}
		}
		return S_OK;
	}
	else
		return E_POINTER;
}

void CVideoDeviceView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->m_nShowEditDetectionZones)
		::AfxGetMainFrame()->StatusText(ML_STRING(1483, "*** Red=100% Orange=50% Yellow=25% Green=10% Blue=5% ***"));
	else
		::AfxGetMainFrame()->StatusText();

	if (pDoc->m_nShowEditDetectionZones	&&
		(nFlags & MK_LBUTTON)			&&
		pDoc->m_nMovDetTotalZones > 0)
	{
		// Get client rectangle
		CRect rcClient;
		GetClientRect(&rcClient);

		// Make sure point is inside the frame
		if (point.x < 0 ||
			point.y < 0 ||
			point.x >= rcClient.Width() ||
			point.y >= rcClient.Height())
			return;

		// Calc. x and y offsets
		int x = 0;
		if (rcClient.Width() > 0)
			x = pDoc->m_nMovDetXZonesCount * point.x / rcClient.Width(); // note: point.x < rcClient.Width()  -> x < pDoc->m_nMovDetXZonesCount
		int y = 0;
		if (rcClient.Height() > 0)
			y = pDoc->m_nMovDetYZonesCount * point.y / rcClient.Height();// note: point.y < rcClient.Height() -> y < pDoc->m_nMovDetYZonesCount

		// The Selected Zone Index
		int nZone = x + y * pDoc->m_nMovDetXZonesCount;

		// Add / Remove Zone Value
		if (nZone >= 0 && nZone < MOVDET_MAX_ZONES)
		{
			int nNewDoMovementDetectionValue = (pDoc->m_nShowEditDetectionZones == 1 ? m_MovDetSingleZoneSensitivity : 0);
			if (nNewDoMovementDetectionValue != pDoc->m_DoMovementDetection[nZone])
			{
				pDoc->m_DoMovementDetection[nZone] = nNewDoMovementDetectionValue;
				pDoc->SaveZonesBlockSettings(nZone / MOVDET_MAX_ZONES_BLOCK_SIZE, pDoc->GetDevicePathName());
				Invalidate(FALSE);
			}
		}
	}
	
	CUImagerView::OnMouseMove(nFlags, point);
}

void CVideoDeviceView::OnViewFullscreen() 
{
	::AfxGetMainFrame()->EnterExitFullscreen();
}

void CVideoDeviceView::OnUpdateViewFullscreen(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bFullScreenMode ? 1 : 0);	
}

void CVideoDeviceView::OnEditActivateAllZones()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	memset(pDoc->m_DoMovementDetection, m_MovDetSingleZoneSensitivity, MOVDET_MAX_ZONES);
	pDoc->SaveZonesSettings(pDoc->GetDevicePathName());
	Invalidate(FALSE);
}

void CVideoDeviceView::OnEditDeactivateAllZones()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	memset(pDoc->m_DoMovementDetection, 0, MOVDET_MAX_ZONES);
	pDoc->SaveZonesSettings(pDoc->GetDevicePathName());
	Invalidate(FALSE);
}

void CVideoDeviceView::OnFrameTimeFontSize4()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->m_nRefFontSize = 4;
	::AfxGetApp()->WriteProfileInt(pDoc->GetDevicePathName(), _T("RefFontSize"), pDoc->m_nRefFontSize);
}

void CVideoDeviceView::OnUpdateFrameTimeFontSize4(CCmdUI* pCmdUI)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->SetCheck(pDoc->m_nRefFontSize == 4 ? 1 : 0);
}

void CVideoDeviceView::OnFrameTimeFontSize5()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->m_nRefFontSize = 5;
	::AfxGetApp()->WriteProfileInt(pDoc->GetDevicePathName(), _T("RefFontSize"), pDoc->m_nRefFontSize);
}

void CVideoDeviceView::OnUpdateFrameTimeFontSize5(CCmdUI* pCmdUI)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->SetCheck(pDoc->m_nRefFontSize == 5 ? 1 : 0);
}

void CVideoDeviceView::OnFrameTimeFontSize6()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->m_nRefFontSize = 6;
	::AfxGetApp()->WriteProfileInt(pDoc->GetDevicePathName(), _T("RefFontSize"), pDoc->m_nRefFontSize);
}

void CVideoDeviceView::OnUpdateFrameTimeFontSize6(CCmdUI* pCmdUI)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->SetCheck(pDoc->m_nRefFontSize == 6 ? 1 : 0);
}

void CVideoDeviceView::OnFrameTimeFontSize7()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->m_nRefFontSize = 7;
	::AfxGetApp()->WriteProfileInt(pDoc->GetDevicePathName(), _T("RefFontSize"), pDoc->m_nRefFontSize);
}

void CVideoDeviceView::OnUpdateFrameTimeFontSize7(CCmdUI* pCmdUI)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->SetCheck(pDoc->m_nRefFontSize == 7 ? 1 : 0);
}

void CVideoDeviceView::OnFrameTimeFontSize8()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->m_nRefFontSize = 8;
	::AfxGetApp()->WriteProfileInt(pDoc->GetDevicePathName(), _T("RefFontSize"), pDoc->m_nRefFontSize);
}

void CVideoDeviceView::OnUpdateFrameTimeFontSize8(CCmdUI* pCmdUI)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->SetCheck(pDoc->m_nRefFontSize == 8 ? 1 : 0);
}

void CVideoDeviceView::OnFrameTimeFontSize9()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->m_nRefFontSize = 9;
	::AfxGetApp()->WriteProfileInt(pDoc->GetDevicePathName(), _T("RefFontSize"), pDoc->m_nRefFontSize);
}

void CVideoDeviceView::OnUpdateFrameTimeFontSize9(CCmdUI* pCmdUI)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->SetCheck(pDoc->m_nRefFontSize == 9 ? 1 : 0);
}

void CVideoDeviceView::OnFrameTimeFontSize10()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->m_nRefFontSize = 10;
	::AfxGetApp()->WriteProfileInt(pDoc->GetDevicePathName(), _T("RefFontSize"), pDoc->m_nRefFontSize);
}

void CVideoDeviceView::OnUpdateFrameTimeFontSize10(CCmdUI* pCmdUI)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->SetCheck(pDoc->m_nRefFontSize == 10 ? 1 : 0);
}

void CVideoDeviceView::OnFrameTimeFontSize11()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->m_nRefFontSize = 11;
	::AfxGetApp()->WriteProfileInt(pDoc->GetDevicePathName(), _T("RefFontSize"), pDoc->m_nRefFontSize);
}

void CVideoDeviceView::OnUpdateFrameTimeFontSize11(CCmdUI* pCmdUI)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->SetCheck(pDoc->m_nRefFontSize == 11 ? 1 : 0);
}

void CVideoDeviceView::OnFrameTimeFontSize12()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->m_nRefFontSize = 12;
	::AfxGetApp()->WriteProfileInt(pDoc->GetDevicePathName(), _T("RefFontSize"), pDoc->m_nRefFontSize);
}

void CVideoDeviceView::OnUpdateFrameTimeFontSize12(CCmdUI* pCmdUI)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->SetCheck(pDoc->m_nRefFontSize == 12 ? 1 : 0);
}

void CVideoDeviceView::OnFrameTimeFontSize14()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->m_nRefFontSize = 14;
	::AfxGetApp()->WriteProfileInt(pDoc->GetDevicePathName(), _T("RefFontSize"), pDoc->m_nRefFontSize);
}

void CVideoDeviceView::OnUpdateFrameTimeFontSize14(CCmdUI* pCmdUI)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->SetCheck(pDoc->m_nRefFontSize == 14 ? 1 : 0);
}

void CVideoDeviceView::OnFrameTimeFontSize16()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->m_nRefFontSize = 16;
	::AfxGetApp()->WriteProfileInt(pDoc->GetDevicePathName(), _T("RefFontSize"), pDoc->m_nRefFontSize);
}

void CVideoDeviceView::OnUpdateFrameTimeFontSize16(CCmdUI* pCmdUI)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->SetCheck(pDoc->m_nRefFontSize == 16 ? 1 : 0);
}

BOOL CVideoDeviceView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	EnableCursor();	
	return CUImagerView::OnMouseWheel(nFlags, zDelta, pt);
}

#endif
