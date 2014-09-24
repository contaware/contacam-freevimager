#include "stdafx.h"
#include "uImager.h"
#include "MainFrm.h"
#include "VideoDeviceDoc.h"
#include "PostDelayedMessage.h"
#include "VideoDeviceView.h"
#include "CameraAdvancedSettingsPropertySheet.h"
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
	ON_WM_KEYUP()
	ON_COMMAND(ID_VIEW_FULLSCREEN, OnViewFullscreen)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FULLSCREEN, OnUpdateViewFullscreen)
	ON_COMMAND(ID_EDIT_SELECTALL, OnEditSelectall)
	ON_COMMAND(ID_EDIT_SELECTNONE, OnEditSelectnone)
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_EDIT_ZONE_SENSITIVITY_100, OnEditZoneSensitivity100)
	ON_COMMAND(ID_EDIT_ZONE_SENSITIVITY_50, OnEditZoneSensitivity50)
	ON_COMMAND(ID_EDIT_ZONE_SENSITIVITY_25, OnEditZoneSensitivity25)
	ON_COMMAND(ID_EDIT_ZONE_SENSITIVITY_10, OnEditZoneSensitivity10)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ZONE_SENSITIVITY_100, OnUpdateEditZoneSensitivity100)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ZONE_SENSITIVITY_50, OnUpdateEditZoneSensitivity50)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ZONE_SENSITIVITY_25, OnUpdateEditZoneSensitivity25)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ZONE_SENSITIVITY_10, OnUpdateEditZoneSensitivity10)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_THREADSAFE_CAPTURECAMERABASICSETTINGS, OnThreadSafeCaptureCameraBasicSettings)
	ON_MESSAGE(WM_THREADSAFE_UPDATE_PHPPARAMS, OnThreadSafeUpdatePhpParams)
	ON_MESSAGE(WM_THREADSAFE_DVCHANGEVIDEOFORMAT, OnThreadSafeDVChangeVideoFormat)
	ON_MESSAGE(WM_THREADSAFE_INIT_MOVDET, OnThreadSafeInitMovDet)
	ON_MESSAGE(WM_DIRECTSHOW_GRAPHNOTIFY, OnDirectShowGraphNotify)
END_MESSAGE_MAP()

	
CVideoDeviceView::CVideoDeviceView()
{
	// Init vars
	m_MovDetSingleZoneSensitivity = 1;
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
				// Stop
				pDoc->m_pDxCapture->Stop();

				// Show dialog
				pDoc->m_pDxCapture->ShowDVFormatDlg();

				// Update
				pDoc->m_bSizeToDoc = TRUE;
				pDoc->OnChangeDxVideoFormat();

				// Reset vars
				pDoc->m_dwFrameCountUp = 0U;
				pDoc->m_dwNextSnapshotUpTime = ::timeGetTime();
				::InterlockedExchange(&pDoc->m_lCurrentInitUpTime, (LONG)pDoc->m_dwNextSnapshotUpTime);

				// Restart
				if (pDoc->m_pDxCapture->Run())
				{
					// Some devices need that...
					// Process frame must still be stopped when calling Dx Stop()!
					pDoc->m_pDxCapture->Stop();
					pDoc->m_pDxCapture->Run();

					// Restart process frame
					pDoc->StartProcessFrame(PROCESSFRAME_DVFORMATDIALOG);
				}
			}
			pDoc->m_bStopAndChangeFormat = FALSE;

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

	if (pDoc && pDoc->m_bCaptureStarted	&& !pDoc->m_bClosing)
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

LONG CVideoDeviceView::OnThreadSafeInitMovDet(WPARAM wparam, LPARAM lparam)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	int i;
	LONG lMovDetXZonesCount = 0;
	LONG lMovDetYZonesCount = 0;

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
			lMovDetXZonesCount = pDoc->m_DocRect.Width() / i;
			break;
		}
	}

	// Calc. zones count in Y direction
	for (i = nMaxSizePix ; i > 0 ; i -= 8)
	{
		if ((pDoc->m_DocRect.Height() % i) == 0 &&
			(pDoc->m_DocRect.Height() / i) >= nMovDetMinYZonesCount)
		{
			lMovDetYZonesCount = pDoc->m_DocRect.Height() / i;
			break;
		}
	}

	// Total number of zones
	LONG lMovDetTotalZones = lMovDetXZonesCount * lMovDetYZonesCount;

	// Check and update doc vars
	if (lMovDetTotalZones == 0 || lMovDetTotalZones > MOVDET_MAX_ZONES)
	{
		if (!pDoc->m_bUnsupportedVideoSizeForMovDet)
		{
			pDoc->m_bUnsupportedVideoSizeForMovDet = TRUE;
			if (pDoc->m_pMovementDetectionPage)
				pDoc->m_pMovementDetectionPage->UpdateDetectionState();
		}
		::InterlockedExchange(&pDoc->m_lMovDetTotalZones, 0);
		return 0;
	}
	else
	{
		if (pDoc->m_bUnsupportedVideoSizeForMovDet)
		{
			pDoc->m_bUnsupportedVideoSizeForMovDet = FALSE;
			if (pDoc->m_pMovementDetectionPage)
				pDoc->m_pMovementDetectionPage->UpdateDetectionState();
		}
		::InterlockedExchange(&pDoc->m_lMovDetXZonesCount, lMovDetXZonesCount);
		::InterlockedExchange(&pDoc->m_lMovDetYZonesCount, lMovDetYZonesCount);
		::InterlockedExchange(&pDoc->m_lMovDetTotalZones, lMovDetTotalZones);
	}

	// Load zones settings
	CString sSection(pDoc->GetDevicePathName());
	BOOL bZonesLoaded = FALSE;
	if (pDoc->m_lMovDetTotalZones == ::AfxGetApp()->GetProfileInt(sSection, _T("MovDetTotalZones"), 0))
		bZonesLoaded = pDoc->LoadZonesSettings();
	
	// If not loaded enable all zones and save it
	if (!bZonesLoaded)
	{
		memset(pDoc->m_DoMovementDetection, 1, MOVDET_MAX_ZONES);
		::AfxGetApp()->WriteProfileInt(sSection, _T("MovDetTotalZones"), pDoc->m_lMovDetTotalZones);
		pDoc->SaveZonesSettings();
	}

	// Update current detection zone size var
	pDoc->m_nCurrentDetectionZoneSize = pDoc->m_nDetectionZoneSize;

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

void CVideoDeviceView::Draw(HDC hDC)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Init
	BOOL bVideoView = pDoc->m_bVideoView;
	BOOL bStopAndChangeFormat = pDoc->m_bStopAndChangeFormat;
	BOOL bDxDeviceUnplugged = pDoc->m_bDxDeviceUnplugged;
	BOOL bWatchDogAlarm = pDoc->m_bWatchDogAlarm;

	// Draw Rect
	CRect rcClient;
	GetClientRect(&rcClient);

	// Erase Background
	if (bStopAndChangeFormat || bDxDeviceUnplugged || bWatchDogAlarm || !bVideoView)
	{
		CBrush br;
		br.CreateSolidBrush(DRAW_BKG_COLOR);	
		::FillRect(hDC, &rcClient, (HBRUSH)br.GetSafeHandle());
	}

	// Display: Change Size
	if (bStopAndChangeFormat)
	{
		::DrawBigText(	hDC, rcClient,
						ML_STRING(1569, "Change Size"),
						DRAW_MESSAGE_COLOR, 72, DT_CENTER | DT_VCENTER,
						OPAQUE, DRAW_BKG_COLOR); // faster drawing with opaque!
	}
	// Display: Unplugged
	else if (bDxDeviceUnplugged)
	{
		::DrawBigText(	hDC, rcClient,
						ML_STRING(1568, "Unplugged"),
						DRAW_MESSAGE_ERROR_COLOR, 72, DT_CENTER | DT_VCENTER,
						OPAQUE, DRAW_BKG_COLOR); // faster drawing with opaque!
	}
	// Display: No Frames
	else if (bWatchDogAlarm)
	{
		::DrawBigText(	hDC, rcClient,
						ML_STRING(1570, "No Frames"),
						DRAW_MESSAGE_ERROR_COLOR, 72, DT_CENTER | DT_VCENTER,
						OPAQUE, DRAW_BKG_COLOR); // faster drawing with opaque!
	}
	// Draw
	else if (bVideoView)
	{
		// Enter CS
		::EnterCriticalSection(&pDoc->m_csDib);

		// Draw Frame
		pDoc->m_pDrawDibRGB32->Paint(hDC,
									&rcClient,
									CRect(0, 0, pDoc->m_pDrawDibRGB32->GetWidth(), pDoc->m_pDrawDibRGB32->GetHeight()),
									FALSE, TRUE);

		// Leave CS
		::LeaveCriticalSection(&pDoc->m_csDib);

		// Draw Zones
		if (pDoc->m_bShowEditDetectionZones ||
			((pDoc->m_dwVideoProcessorMode & SOFTWARE_MOVEMENT_DETECTOR) && pDoc->m_bShowMovementDetections))
			DrawZones(hDC);

		// Draw Text
		CString sOSDMessage;
		COLORREF crOSDMessageColor = DRAW_MESSAGE_SUCCESS_COLOR;
		::EnterCriticalSection(&pDoc->m_csOSDMessage);
		DWORD dwCurrentUpTime = ::timeGetTime(); // uptime measurement must be inside the cs!
		if ((dwCurrentUpTime - pDoc->m_dwOSDMessageUpTime) <= DRAW_MESSAGE_SHOWTIME)
		{
			sOSDMessage = pDoc->m_sOSDMessage;
			crOSDMessageColor = pDoc->m_crOSDMessageColor;
		}
		else
			pDoc->m_sOSDMessage = _T("");
		::LeaveCriticalSection(&pDoc->m_csOSDMessage);
		if (pDoc->m_bDetectingMinLengthMovement		||
			pDoc->m_SaveFrameListThread.IsWorking() ||
			!sOSDMessage.IsEmpty())
			DrawTextMsg(hDC, sOSDMessage, crOSDMessageColor);
	}
	// Display: Preview Off
	else
	{
		::DrawBigText(	hDC, rcClient,
						ML_STRING(1571, "Preview Off"),
						DRAW_MESSAGE_SUCCESS_COLOR, 72, DT_CENTER | DT_VCENTER,
						OPAQUE, DRAW_BKG_COLOR); // faster drawing with opaque!
	}
}

void CVideoDeviceView::DrawTextMsg(HDC hDC, const CString& sOSDMessage, COLORREF crOSDMessageColor)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CRect rcClient;
	GetClientRect(&rcClient);

	// Calc. font size
	int nMaxFontSize = ::ScaleFont(	rcClient.Width(), rcClient.Height(),
									pDoc->m_nRefFontSize, FRAMETAG_REFWIDTH, FRAMETAG_REFHEIGHT);

	// Motion Detection
	if (pDoc->m_bDetectingMinLengthMovement)
	{
		::DrawBigText(	hDC, CRect(0, 0, rcClient.Width(), rcClient.Height()),
						ML_STRING(1844, "Det"), DRAW_MESSAGE_SUCCESS_COLOR, nMaxFontSize, DT_BOTTOM | DT_RIGHT,
						OPAQUE, DRAW_BKG_COLOR);
	}

	// Save / Email / FTP progress display
	if (pDoc->m_SaveFrameListThread.IsWorking())
	{
		CString sProgress(_T(""));
		if (pDoc->m_SaveFrameListThread.GetSaveProgress() < 100)
			sProgress.Format(ML_STRING(1877, "Save: %d%%"), pDoc->m_SaveFrameListThread.GetSaveProgress());
		else if (pDoc->m_SaveFrameListThread.GetSendMailProgress() < 100)
			sProgress.Format(ML_STRING(1878, "Email: %d%%"), pDoc->m_SaveFrameListThread.GetSendMailProgress());
		else if (pDoc->m_SaveFrameListThread.GetFTPUploadProgress() < 100)
			sProgress.Format(ML_STRING(1879, "FTP: %d%%"), pDoc->m_SaveFrameListThread.GetFTPUploadProgress());
		if (sProgress != _T(""))
		{
			::DrawBigText(	hDC, CRect(0, 0, rcClient.Width(), rcClient.Height()),
							sProgress, DRAW_MESSAGE_COLOR, nMaxFontSize, DT_TOP | DT_RIGHT,
							OPAQUE, DRAW_BKG_COLOR);
		}
	}

	// Draw OSD message
	if (!sOSDMessage.IsEmpty())
	{
		::DrawBigText(	hDC, CRect(0, 0, rcClient.Width(), rcClient.Height()),
						sOSDMessage, crOSDMessageColor, 72, DT_CENTER | DT_VCENTER,
						OPAQUE, DRAW_MESSAGE_BKG_COLOR);
	}
}

__forceinline void CVideoDeviceView::DrawZoneSensitivity(int i, HDC hDC, const RECT& rcDetZone, int n)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	POINT ptCenter;
	ptCenter.x = rcDetZone.left + (rcDetZone.right - rcDetZone.left) / 2;
	ptCenter.y = rcDetZone.top + (rcDetZone.bottom - rcDetZone.top) / 2;

	// 10 %
	if (pDoc->m_DoMovementDetection[i] >= 10)
	{
		// Draw a 1
		::MoveToEx(hDC, ptCenter.x - (2+n/2), ptCenter.y - (1+n/2), NULL);
		::LineTo(hDC, ptCenter.x - 1, ptCenter.y - (2+n));
		::LineTo(hDC, ptCenter.x - 1, ptCenter.y + (3+n));

		// Draw a 0
		::MoveToEx(hDC, ptCenter.x + (2+n), ptCenter.y - (2+n), NULL);
		::LineTo(hDC, ptCenter.x + 1, ptCenter.y - (2+n));
		::LineTo(hDC, ptCenter.x + 1, ptCenter.y + (2+n));
		::LineTo(hDC, ptCenter.x + (2+n), ptCenter.y + (2+n));
		::LineTo(hDC, ptCenter.x + (2+n), ptCenter.y - (2+n));
	}
	// 25 %
	else if (pDoc->m_DoMovementDetection[i] >= 4)
	{
		// Draw a 2
		::MoveToEx(hDC, ptCenter.x - (2+n), ptCenter.y - (2+n), NULL);
		::LineTo(hDC, ptCenter.x - 1, ptCenter.y - (2+n));
		::LineTo(hDC, ptCenter.x - 1, ptCenter.y);
		::LineTo(hDC, ptCenter.x - (2+n), ptCenter.y);
		::LineTo(hDC, ptCenter.x - (2+n), ptCenter.y + (2+n));
		::LineTo(hDC, ptCenter.x, ptCenter.y + (2+n));

		// Draw a 5
		::MoveToEx(hDC, ptCenter.x + (2+n), ptCenter.y - (2+n), NULL);
		::LineTo(hDC, ptCenter.x + 1, ptCenter.y - (2+n));
		::LineTo(hDC, ptCenter.x + 1, ptCenter.y);
		::LineTo(hDC, ptCenter.x + (2+n), ptCenter.y);
		::LineTo(hDC, ptCenter.x + (2+n), ptCenter.y + (2+n));
		::LineTo(hDC, ptCenter.x, ptCenter.y + (2+n));
	}
	// 50 %
	else if (pDoc->m_DoMovementDetection[i] >= 2)
	{
		// Draw a 5
		::MoveToEx(hDC, ptCenter.x - 1, ptCenter.y - (2+n), NULL);
		::LineTo(hDC, ptCenter.x - (2+n), ptCenter.y - (2+n));
		::LineTo(hDC, ptCenter.x - (2+n), ptCenter.y);
		::LineTo(hDC, ptCenter.x - 1 , ptCenter.y);
		::LineTo(hDC, ptCenter.x - 1, ptCenter.y + (2+n));
		::LineTo(hDC, ptCenter.x - (3+n), ptCenter.y + (2+n));

		// Draw a 0
		::MoveToEx(hDC, ptCenter.x + (2+n), ptCenter.y - (2+n), NULL);
		::LineTo(hDC, ptCenter.x + 1, ptCenter.y - (2+n));
		::LineTo(hDC, ptCenter.x + 1, ptCenter.y + (2+n));
		::LineTo(hDC, ptCenter.x + (2+n), ptCenter.y + (2+n));
		::LineTo(hDC, ptCenter.x + (2+n), ptCenter.y - (2+n));
	}
}

void CVideoDeviceView::DrawZones(HDC hDC)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->m_lMovDetTotalZones > 0)
	{
		CRect rcClient;
		GetClientRect(&rcClient);

		RECT rcDetZone;
		double dZoneWidth = (double)rcClient.Width() / (double)pDoc->m_lMovDetXZonesCount;
		double dZoneHeight = (double)rcClient.Height() / (double)pDoc->m_lMovDetYZonesCount;
		int nSensitivityTextSize;
		if (dZoneWidth <= 8.0 || dZoneHeight <= 8.0)
			nSensitivityTextSize = 0;
		else if (dZoneWidth <= 16.0 || dZoneHeight <= 16.0)
			nSensitivityTextSize = 2;
		else
			nSensitivityTextSize = 4;

		// Draw Zones where Detection is enabled
		if (pDoc->m_bShowEditDetectionZones)
		{
			HPEN hPen = ::CreatePen(PS_SOLID, 1, MOVDET_SELECTED_ZONES_COLOR);
			HGDIOBJ hOldPen = ::SelectObject(hDC, hPen);
			HGDIOBJ hOldBrush = ::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));
			int nLastBottomEdge = 1;
			for (int y = 0 ; y < pDoc->m_lMovDetYZonesCount ; y++)
			{
				int nLastRightEdge = 1;
				for (int x = 0 ; x < pDoc->m_lMovDetXZonesCount ; x++)
				{
					int nZoneOffsetX = (int)(x * dZoneWidth);
					int nZoneOffsetY = (int)(y * dZoneHeight);
					rcDetZone.left = nLastRightEdge - 1;
					rcDetZone.top = nLastBottomEdge - 1;
					rcDetZone.right = (x == (pDoc->m_lMovDetXZonesCount - 1) ? rcClient.right : (int)(nZoneOffsetX + dZoneWidth));
					rcDetZone.bottom = (y == (pDoc->m_lMovDetYZonesCount - 1) ? rcClient.bottom : (int)(nZoneOffsetY + dZoneHeight));
					int i = x + y*pDoc->m_lMovDetXZonesCount;
					if (pDoc->m_DoMovementDetection[i])
					{
						DrawZoneSensitivity(i, hDC, rcDetZone, nSensitivityTextSize);
						::Rectangle(hDC, rcDetZone.left, rcDetZone.top, rcDetZone.right, rcDetZone.bottom);
						::MoveToEx(hDC, rcDetZone.left + (rcDetZone.right -  rcDetZone.left) / 4, rcDetZone.top, NULL);
						::LineTo(hDC, rcDetZone.left, rcDetZone.top + (rcDetZone.right -  rcDetZone.left) / 4);
					}
					nLastRightEdge = rcDetZone.right;
				}
				nLastBottomEdge = rcDetZone.bottom;
			}
			::SelectObject(hDC, hOldBrush);
			::SelectObject(hDC, hOldPen);
			::DeleteObject(hPen);
		}

		// Draw Detected Zones
		if ((pDoc->m_dwVideoProcessorMode & SOFTWARE_MOVEMENT_DETECTOR) && pDoc->m_bShowMovementDetections)
		{
			HPEN hPen = ::CreatePen(PS_SOLID, 1, MOVDET_DETECTING_ZONES_COLOR);
			HGDIOBJ hOldPen = ::SelectObject(hDC, hPen);
			HGDIOBJ hOldBrush = ::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));
			int nLastBottomEdge = 1;
			for (int y = 0 ; y < pDoc->m_lMovDetYZonesCount ; y++)
			{
				int nLastRightEdge = 1;
				for (int x = 0 ; x < pDoc->m_lMovDetXZonesCount ; x++)
				{
					int nZoneOffsetX = (int)(x * dZoneWidth);
					int nZoneOffsetY = (int)(y * dZoneHeight);
					rcDetZone.left = nLastRightEdge - 1;
					rcDetZone.top = nLastBottomEdge - 1;
					rcDetZone.right = (x == (pDoc->m_lMovDetXZonesCount - 1) ? rcClient.right : (int)(nZoneOffsetX + dZoneWidth));
					rcDetZone.bottom = (y == (pDoc->m_lMovDetYZonesCount - 1) ? rcClient.bottom : (int)(nZoneOffsetY + dZoneHeight));
					int i = x + y*pDoc->m_lMovDetXZonesCount;
					if (pDoc->m_MovementDetections[i])
					{
						DrawZoneSensitivity(i, hDC, rcDetZone, nSensitivityTextSize);
						::Rectangle(hDC, rcDetZone.left, rcDetZone.top, rcDetZone.right, rcDetZone.bottom);
						::MoveToEx(hDC, rcDetZone.left + (rcDetZone.right -  rcDetZone.left) / 4, rcDetZone.top, NULL);
						::LineTo(hDC, rcDetZone.left, rcDetZone.top + (rcDetZone.right -  rcDetZone.left) / 4);
					}
					nLastRightEdge = rcDetZone.right;
				}
				nLastBottomEdge = rcDetZone.bottom;
			}
			::SelectObject(hDC, hOldBrush);
			::SelectObject(hDC, hOldPen);
			::DeleteObject(hPen);
		}
	}
}

BOOL CVideoDeviceView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
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
	if (pDoc->m_bCaptureStarted)
		Draw(MemDC.GetSafeHdc());
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
			_tcscpy(lf.lfFaceName, DEFAULT_FONTFACE);
			lf.lfHeight = -MulDiv(11, pDC->GetDeviceCaps(LOGPIXELSY), 72);
			lf.lfWeight = FW_NORMAL;
			lf.lfItalic = 0;
			lf.lfUnderline = 0;
			m_GDIDrawFont.CreateFontIndirect(&lf);
		}

		// Set colors
		COLORREF crOldTextColor = MemDC.SetTextColor(DRAW_MESSAGE_COLOR);
		int nOldBkMode = MemDC.SetBkMode(OPAQUE);
		COLORREF crOldBkColor = MemDC.SetBkColor(DRAW_BKG_COLOR);
		CFont* pOldFont = MemDC.SelectObject(&m_GDIDrawFont);

		// Draw
		MemDC.DrawText(	ML_STRING(1565, "Please wait..."),
										-1,
										&rcClient,
										(DT_CENTER | DT_VCENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
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
		int nCount = ((::GetTickCount() / 1000U) % 5U);
		MemDC.FillSolidRect(rcBoxLeftLeft, RGB(0,0xFF,0));
		if (nCount >= 1)
			MemDC.FillSolidRect(rcBoxLeft, RGB(0,0xFF,0));
		if (nCount >= 2)
			MemDC.FillSolidRect(rcBoxMiddle, RGB(0,0xFF,0));
		if (nCount >= 3)
			MemDC.FillSolidRect(rcBoxRight, RGB(0,0xFF,0));
		if (nCount == 4)
			MemDC.FillSolidRect(rcBoxRightRight, RGB(0,0xFF,0));

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

	if (pDoc->m_bShowEditDetectionZones)
	{
		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_CONTEXT_VIDEO_DEVICE_ZONES));
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

	if (pDoc->m_bShowEditDetectionZones && pDoc->m_lMovDetTotalZones > 0)
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
			x = pDoc->m_lMovDetXZonesCount * point.x / rcClient.Width(); // note: point.x < rcClient.Width()  -> x < pDoc->m_lMovDetXZonesCount
		int y = 0;
		if (rcClient.Height() > 0)
			y = pDoc->m_lMovDetYZonesCount * point.y / rcClient.Height();// note: point.y < rcClient.Height() -> y < pDoc->m_lMovDetYZonesCount

		// The Selected Zone Index
		int nZone = x + y * pDoc->m_lMovDetXZonesCount;

		// Reset Zone Value
		if ((nFlags & MK_SHIFT) ||
			(nFlags & MK_CONTROL))
			pDoc->m_DoMovementDetection[nZone] = 0;
		// Set Zone Value
		else
			pDoc->m_DoMovementDetection[nZone] = m_MovDetSingleZoneSensitivity;

		// Paint
		Invalidate(FALSE);
	}
}

void CVideoDeviceView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// No fullscreen enter/exit when editing zones!
	if (pDoc->m_bShowEditDetectionZones)
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
		case ID_TIMER_RELOAD_SETTINGS :
		{
			DWORD dwVideoProcessorMode = (DWORD) ::AfxGetApp()->GetProfileInt(pDoc->GetDevicePathName(), _T("VideoProcessorMode"), NO_DETECTOR);
			if (dwVideoProcessorMode != pDoc->m_dwVideoProcessorMode)
			{
				pDoc->m_dwVideoProcessorMode = dwVideoProcessorMode;
				if (pDoc->GetFrame() && pDoc->GetFrame()->GetToolBar())
					((CVideoDeviceToolBar*)(pDoc->GetFrame()->GetToolBar()))->m_DetComboBox.SetCurSel(pDoc->m_dwVideoProcessorMode);
				if (pDoc->m_pMovementDetectionPage)
					pDoc->m_pMovementDetectionPage->UpdateDetectionState();
			}
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
		case VK_RETURN : // Enter
			pDoc->CaptureRecord();
			break;

		case VK_ESCAPE :
			if (((CUImagerApp*)::AfxGetApp())->m_bEscExit)
				::AfxGetMainFrame()->PostMessage(WM_CLOSE, 0, 0);
			else
			{
				if (pDoc->m_pCameraAdvancedSettingsPropertySheet && pDoc->m_pCameraAdvancedSettingsPropertySheet->IsWindowVisible())
					pDoc->m_pCameraAdvancedSettingsPropertySheet->Hide(TRUE);
				else if (pDoc->m_bShowEditDetectionZones)
				{
					pDoc->HideDetectionZones();
					pDoc->SaveSettings();
				}
				else if (m_bFullScreenMode)
					::AfxGetMainFrame()->EnterExitFullscreen();	// Exit Full-Screen Mode
				else
					pDoc->CloseDocument();
			}
			break;

		case VK_INSERT :
			pDoc->m_bDoEditSnapshot = TRUE;
			break;

		case VK_CONTROL :
		case VK_SHIFT :
			if (pDoc->m_bShowEditDetectionZones)
			{
				pDoc->m_bShowEditDetectionZonesMinus = TRUE;
				UpdateCursor();
			}
			break;

		case VK_APPS :
			ForceCursor();
			if (pDoc->m_bShowEditDetectionZones)
			{
				VERIFY(menu.LoadMenu(IDR_CONTEXT_VIDEO_DEVICE_ZONES));
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

void CVideoDeviceView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	switch (nChar)
	{
		case VK_CONTROL :
		case VK_SHIFT :
			if (pDoc->m_bShowEditDetectionZones)
			{
				pDoc->m_bShowEditDetectionZonesMinus = FALSE;
				UpdateCursor();
			}
			break;

		case VK_MENU :	// Alt Gr returns this
			if (pDoc->m_bShowEditDetectionZones)
			{
				pDoc->m_bShowEditDetectionZonesMinus = FALSE;
				UpdateCursor();
			}
			break;

		default : 
			break;
	}
	
	CUImagerView::OnKeyUp(nChar, nRepCnt, nFlags);
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
	else if (pDoc->m_bShowEditDetectionZones)
	{
		if (pDoc->m_bShowEditDetectionZonesMinus)
			hCursor = ::AfxGetApp()->LoadCursor(IDC_ZONEMINUS_CURSOR);
		else
			hCursor = ::AfxGetApp()->LoadCursor(IDC_ZONEPLUS_CURSOR);
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
		pDoc->m_dwNextSnapshotUpTime = ::timeGetTime();
		::InterlockedExchange(&pDoc->m_lCurrentInitUpTime, (LONG)pDoc->m_dwNextSnapshotUpTime);

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

				// Some devices need that...
				// Process frame must still be stopped when calling Dx Stop()!
				pDoc->m_pDxCapture->Stop();
				pDoc->m_pDxCapture->Run();

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
					// Set Unplugged Flag
					pDoc->m_bDxDeviceUnplugged = TRUE;
					Invalidate(FALSE);
					::LogLine(_T("%s"), pDoc->GetAssignedDeviceName() + _T(" unplugged"));

                    break;
				}
				// Device is available again
				else if (evParam2 == 1)
				{
					// Re-Open
					pDoc->StopProcessFrame(PROCESSFRAME_DXREPLUGGED);
					if (ReOpenDxDevice())
					{
						// Reset Unplugged Flag
						pDoc->m_bDxDeviceUnplugged = FALSE;
						::LogLine(_T("%s"), pDoc->GetAssignedDeviceName() + _T(" replugged"));

						// Restart process frame
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

	// Status Text
	if (pDoc->m_bShowEditDetectionZones)
		::AfxGetMainFrame()->StatusText(ML_STRING(1483, "*** Click Inside The Capture Window to Add Zones. Press Ctrl (or Shift) to Remove Them ***"));
	else
		::AfxGetMainFrame()->StatusText();

	// In some cases releasing the shift or control key
	// is captured by another program. Check state here!
	if (pDoc->m_bShowEditDetectionZones)
	{
		if ((nFlags & MK_SHIFT) ||
			(nFlags & MK_CONTROL))
			pDoc->m_bShowEditDetectionZonesMinus = TRUE;
		else
			pDoc->m_bShowEditDetectionZonesMinus = FALSE;
	}

	if (pDoc->m_bShowEditDetectionZones	&&
		(nFlags & MK_LBUTTON)			&&
		pDoc->m_lMovDetTotalZones > 0)
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
			x = pDoc->m_lMovDetXZonesCount * point.x / rcClient.Width(); // note: point.x < rcClient.Width()  -> x < pDoc->m_lMovDetXZonesCount
		int y = 0;
		if (rcClient.Height() > 0)
			y = pDoc->m_lMovDetYZonesCount * point.y / rcClient.Height();// note: point.y < rcClient.Height() -> y < pDoc->m_lMovDetYZonesCount

		// The Selected Zone Index
		int nZone = x + y * pDoc->m_lMovDetXZonesCount;

		// Reset Zone Value
		if ((nFlags & MK_SHIFT) ||
			(nFlags & MK_CONTROL))
			pDoc->m_DoMovementDetection[nZone] = 0;
		// Set Zone Value
		else
			pDoc->m_DoMovementDetection[nZone] = m_MovDetSingleZoneSensitivity;

		// Paint
		Invalidate(FALSE);
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

void CVideoDeviceView::OnEditSelectall() 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	memset(pDoc->m_DoMovementDetection, 1, MOVDET_MAX_ZONES);
	Invalidate(FALSE);
}

void CVideoDeviceView::OnEditSelectnone() 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	memset(pDoc->m_DoMovementDetection, 0, MOVDET_MAX_ZONES);
	Invalidate(FALSE);
}

void CVideoDeviceView::OnEditZoneSensitivity100() 
{
	m_MovDetSingleZoneSensitivity = 1;
}

void CVideoDeviceView::OnUpdateEditZoneSensitivity100(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(m_MovDetSingleZoneSensitivity == 1);
}

void CVideoDeviceView::OnEditZoneSensitivity50() 
{
	m_MovDetSingleZoneSensitivity = 2;
}

void CVideoDeviceView::OnUpdateEditZoneSensitivity50(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(m_MovDetSingleZoneSensitivity == 2);
}

void CVideoDeviceView::OnEditZoneSensitivity25() 
{
	m_MovDetSingleZoneSensitivity = 4;
}

void CVideoDeviceView::OnUpdateEditZoneSensitivity25(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(m_MovDetSingleZoneSensitivity == 4);
}

void CVideoDeviceView::OnEditZoneSensitivity10() 
{
	m_MovDetSingleZoneSensitivity = 10;
}

void CVideoDeviceView::OnUpdateEditZoneSensitivity10(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(m_MovDetSingleZoneSensitivity == 10);
}

BOOL CVideoDeviceView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	EnableCursor();	
	return CUImagerView::OnMouseWheel(nFlags, zDelta, pt);
}

#endif
