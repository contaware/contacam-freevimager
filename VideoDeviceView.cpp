#include "stdafx.h"
#include "uImager.h"
#include "MainFrm.h"
#include "VideoDeviceDoc.h"
#include "PostDelayedMessage.h"
#include "VideoDeviceView.h"
#include "VideoDevicePropertySheet.h"
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
	ON_WM_MBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYUP()
	ON_COMMAND(ID_EDIT_SELECTALL, OnEditSelectall)
	ON_COMMAND(ID_EDIT_SELECTNONE, OnEditSelectnone)
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_THREADSAFE_CAPTURESETTINGS, OnThreadSafeCaptureSettings)
	ON_MESSAGE(WM_THREADSAFE_UPDATE_PHPPARAMS, OnThreadSafeUpdatePhpParams)
	ON_MESSAGE(WM_THREADSAFE_CHANGEVIDEOFORMAT, OnThreadSafeChangeVideoFormat)
	ON_MESSAGE(WM_THREADSAFE_STOP_AND_CHANGEVIDEOFORMAT, OnThreadSafeStopAndChangeVideoFormat)
	ON_MESSAGE(WM_ENABLE_DISABLE_CRITICAL_CONTROLS, OnEnableDisableCriticalControls)
	ON_MESSAGE(WM_THREADSAFE_INIT_MOVDET, OnThreadSafeInitMovDet)
	ON_MESSAGE(WM_DIRECTSHOW_GRAPHNOTIFY, OnDirectShowGraphNotify)
	ON_MESSAGE(WM_THREADSAFE_AUTORUNREMOVEDEVICE_CLOSEDOC, OnThreadSafeAutorunRemoveDeviceCloseDoc)
	ON_MESSAGE(WM_THREADSAFE_SENDFRAME_MSG, OnThreadSafeSendFrameMsg)
END_MESSAGE_MAP()

	
CVideoDeviceView::CVideoDeviceView()
{
	// Init vars
	m_bDxDrawInitFailed = FALSE;
	m_bDxDrawFirstInitOk = FALSE;
	m_bDxDrawInfoText = FALSE;
	m_dwDxDrawUpTime = ::timeGetTime();
	m_nCriticalControlsCount = 1;
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

LONG CVideoDeviceView::OnThreadSafeStopAndChangeVideoFormat(WPARAM wparam, LPARAM lparam)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	int nPrevTotalDelay = (int)lparam;
	if (pDoc)
	{
		if (nPrevTotalDelay > MAX_DX_DIALOGS_RETRY_TIME || pDoc->IsProcessFrameStopped())
		{
			if (!pDoc->m_bClosing)
			{
				// Stop
				if (pDoc->m_pDxCapture->Stop())
					pDoc->m_bCapture = FALSE;

				// Show dialog
				pDoc->m_pDxCapture->ShowDVFormatDlg();

				// Update
				pDoc->m_bSizeToDoc = TRUE;
				OnThreadSafeChangeVideoFormat(0, 0);

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

					// Set flag
					pDoc->m_bCapture = TRUE;

					// Restart process frame
					pDoc->ReStartProcessFrame();
				}
			}
			pDoc->m_bStopAndChangeFormat = FALSE;
			
			// Enable Critical Controls
			::SendMessage(	GetSafeHwnd(),
							WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
							(WPARAM)TRUE,	// Enable Them
							(LPARAM)0);

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
															WM_THREADSAFE_STOP_AND_CHANGEVIDEOFORMAT,
															delay, 0, nPrevTotalDelay + delay);
			return 0;
		}
	}
	else
		return 0;
}

LONG CVideoDeviceView::OnThreadSafeCaptureSettings(WPARAM wparam, LPARAM lparam)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc && !pDoc->m_bClosing)
	{
		pDoc->CaptureSettings();
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

LONG CVideoDeviceView::OnThreadSafeChangeVideoFormat(WPARAM wparam, LPARAM lparam)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc && !pDoc->m_bClosing)
	{
		pDoc->OnChangeVideoFormat();
		return 1;
	}
	else
		return 0;
}

LONG CVideoDeviceView::OnEnableDisableCriticalControls(WPARAM wparam, LPARAM lparam)
{
	BOOL bEnable = (BOOL)wparam;
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (bEnable)
		m_nCriticalControlsCount++;
	else
		m_nCriticalControlsCount--;

	if (pDoc->m_pGeneralPage)
		pDoc->m_pGeneralPage->EnableDisableCriticalControls(m_nCriticalControlsCount > 0);

	return 1;
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
				pDoc->m_pMovementDetectionPage->UpdateUnsupportedWarning();
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
				pDoc->m_pMovementDetectionPage->UpdateUnsupportedWarning();
		}
		::InterlockedExchange(&pDoc->m_lMovDetXZonesCount, lMovDetXZonesCount);
		::InterlockedExchange(&pDoc->m_lMovDetYZonesCount, lMovDetYZonesCount);
		::InterlockedExchange(&pDoc->m_lMovDetTotalZones, lMovDetTotalZones);
	}

	// Load/Store Settings
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
	{
		CWinApp* pApp = ::AfxGetApp();
		CString sSection(pDoc->GetDevicePathName());
		if (pDoc->m_lMovDetTotalZones == pApp->GetProfileInt(sSection, _T("MovDetTotalZones"), 0))
		{
			for (i = 0 ; i < pDoc->m_lMovDetTotalZones ; i++)
			{
				CString sZone;
				sZone.Format(MOVDET_ZONE_FORMAT, i);
				pDoc->m_DoMovementDetection[i] = (BOOL)pApp->GetProfileInt(sSection, sZone, TRUE);
			}
		}
		else
		{
			// Enable All Zones and Store Settings
			((CUImagerApp*)::AfxGetApp())->WriteProfileInt(sSection, _T("MovDetTotalZones"), pDoc->m_lMovDetTotalZones);
			for (i = 0 ; i < pDoc->m_lMovDetTotalZones ; i++)
			{
				pDoc->m_DoMovementDetection[i] = TRUE;
				CString sZone;
				sZone.Format(MOVDET_ZONE_FORMAT, i);
				((CUImagerApp*)::AfxGetApp())->WriteProfileInt(sSection, sZone, pDoc->m_DoMovementDetection[i]);
			}
		}
	}
	else
	{
		for (i = 0 ; i < pDoc->m_lMovDetTotalZones ; i++)
			pDoc->m_DoMovementDetection[i] = TRUE;
	}

	// Update current detection zone size var
	pDoc->m_nCurrentDetectionZoneSize = pDoc->m_nDetectionZoneSize;

	return 1;
}

afx_msg LONG CVideoDeviceView::OnThreadSafeAutorunRemoveDeviceCloseDoc(WPARAM wparam, LPARAM lparam)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CVideoDeviceDoc::AutorunRemoveDevice(pDoc->GetDevicePathName());
	pDoc->CloseDocument();
	return 1;
}

afx_msg LONG CVideoDeviceView::OnThreadSafeSendFrameMsg(WPARAM wparam, LPARAM lparam)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (wparam)
	{
		CString* pStr = (CString*)wparam;
		pDoc->m_sSendFrameMsg = *pStr;
		delete pStr;
		pDoc->ShowSendFrameMsg();
		return 1;
	}
	else
		return 0;
}

BOOL CVideoDeviceView::InitDxDraw(int nWidth, int nHeight, DWORD dwFourCC)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc); crashing because called from non UI thread!
	CTimeSpan ElapsedTimeSinceLastSessionChange = CTime::GetCurrentTime() - ::AfxGetMainFrame()->m_SessionChangeTime;
	if (nWidth > 0 && nHeight > 0 &&
		(ElapsedTimeSinceLastSessionChange.GetTotalSeconds() >= SESSIONCHANGE_WAIT_SEC ||
		ElapsedTimeSinceLastSessionChange.GetTotalSeconds() < 0))
	{
		if (pDoc->m_pDxDraw->Init(	GetSafeHwnd(),
									nWidth,
									nHeight,
									dwFourCC,
									IDB_BITSTREAM_VERA_11))
		{
			m_bDxDrawInitFailed = FALSE;
			m_bDxDrawFirstInitOk = TRUE;
			return TRUE;
		}
		else
		{
			pDoc->m_bDecodeFramesForPreview = TRUE;
			if (!m_bDxDrawInitFailed && dwFourCC == 0U) // if also BI_RGB failed, display "DirectX failed" in OnDraw()
			{
				m_bDxDrawInitFailed = TRUE;
				::PostMessage(	GetSafeHwnd(),
								WM_THREADSAFE_UPDATEWINDOWSIZES,
								(WPARAM)UPDATEWINDOWSIZES_INVALIDATE,
								(LPARAM)0);
			}
			return FALSE;
		}
	}
	else
		return FALSE;
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

__forceinline void CVideoDeviceView::EraseDxBkgnd(BOOL bFullErase)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc); crashing because called from non UI thread!

	// Erase Full-Screen Borders
	if (m_bFullScreenMode)
	{
		// Client Rect
		CRect rcClient;
		GetClientRect(rcClient);

		// Erase Bkg Rectangles
		CRect rcTop(rcClient.left, rcClient.top, rcClient.right, m_ZoomRect.top);
		CRect rcLeft(rcClient.left, m_ZoomRect.top, m_ZoomRect.left, m_ZoomRect.bottom);
		CRect rcRight = CRect(m_ZoomRect.right, m_ZoomRect.top, rcClient.right, m_ZoomRect.bottom);
		CRect rcBottom = CRect(rcClient.left, m_ZoomRect.bottom, rcClient.right, rcClient.bottom);
		
		// Clear Front
		if ((rcTop.Width() > 0) && (rcTop.Height() > 0))
			pDoc->m_pDxDraw->ClearFront(&rcTop);
		if ((rcLeft.Width() > 0) && (rcLeft.Height() > 0))
			pDoc->m_pDxDraw->ClearFront(&rcLeft);
		if ((rcRight.Width() > 0) && (rcRight.Height() > 0))
			pDoc->m_pDxDraw->ClearFront(&rcRight);
		if ((rcBottom.Width() > 0) && (rcBottom.Height() > 0))
			pDoc->m_pDxDraw->ClearFront(&rcBottom);
	}

	// Erase All
	if (bFullErase)
		pDoc->m_pDxDraw->ClearBack();
}

__forceinline BOOL CVideoDeviceView::IsDxCompressionDifferent()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc); crashing because called from non UI thread!

	// YUY2 Format Equivalents
	if (pDoc->m_pDxDraw->GetCurrentSrcFourCC() == FCC('YUY2')	&&
		(pDoc->m_pDib->GetCompression() == FCC('YUNV')			||
		pDoc->m_pDib->GetCompression() == FCC('VYUY')			||
		pDoc->m_pDib->GetCompression() == FCC('V422')			||
		pDoc->m_pDib->GetCompression() == FCC('YUYV')))
		return FALSE;
	// Special Handling for YV12 Format
	else if (pDoc->m_pDxDraw->GetCurrentSrcFourCC() == FCC('YV12'))
	{
		if (!pDoc->m_pDxDraw->GetCurrentSrcFlipUV()			&&
			pDoc->m_pDib->GetCompression() == FCC('YV12'))
			return FALSE;
		else if (pDoc->m_pDxDraw->GetCurrentSrcFlipUV()		&&
			(pDoc->m_pDib->GetCompression() == FCC('I420')	||
			pDoc->m_pDib->GetCompression() == FCC('IYUV')))
			return FALSE;
		else
			return TRUE;
	}
	// RGB Format Equivalents
	else if (pDoc->m_pDxDraw->GetCurrentSrcFourCC() == BI_RGB	&&
			(pDoc->m_pDib->GetCompression() == BI_BITFIELDS	||
			pDoc->m_pDib->GetCompression() == BI_RLE4		||
			pDoc->m_pDib->GetCompression() == FCC('RLE4')	||
			pDoc->m_pDib->GetCompression() == BI_RLE8		||
			pDoc->m_pDib->GetCompression() == FCC('RLE8')))
		return FALSE;
	// Remaining Formats
	else
		return (pDoc->m_pDib->GetCompression() != pDoc->m_pDxDraw->GetCurrentSrcFourCC());
}

BOOL CVideoDeviceView::DxDraw()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc); crashing because called from non UI thread!
	ASSERT(pDoc->m_pDxDraw);

	// Enter CS
	::EnterCriticalSection(&pDoc->m_csDib);

	// No drawing possible at this moment
	if (!pDoc->m_pDib || !pDoc->m_pDib->IsValid())
	{
		::LeaveCriticalSection(&pDoc->m_csDib);
		return FALSE;
	}

	// Init local vars
	DWORD dwCurrentUpTime = ::timeGetTime();
	BOOL bVideoView = pDoc->m_bVideoView;
	BOOL bStopAndChangeFormat = pDoc->m_bStopAndChangeFormat;
	BOOL bDxDrawInfoText = m_bDxDrawInfoText;
	BOOL bWatchDogAlarm = pDoc->m_bWatchDogAlarm;

	// Preview Off?
	if (!bVideoView)
	{
		if (!pDoc->m_pDxDraw->IsInit()										||
			(dwCurrentUpTime - m_dwDxDrawUpTime > DXDRAW_REINIT_TIMEOUT)	||
			pDoc->m_pDib->GetWidth() != pDoc->m_pDxDraw->GetSrcWidth()		||
			pDoc->m_pDib->GetHeight() != pDoc->m_pDxDraw->GetSrcHeight()	||				
			pDoc->m_pDxDraw->GetCurrentSrcFourCC() != BI_RGB)
		{
			m_dwDxDrawUpTime = dwCurrentUpTime;
			if (!InitDxDraw(pDoc->m_pDib->GetWidth(), pDoc->m_pDib->GetHeight(), BI_RGB))
			{
				::LeaveCriticalSection(&pDoc->m_csDib);
				return FALSE;
			}
		}	
	}
	else
	{
		if (!pDoc->m_pDxDraw->IsInit()										||
			(dwCurrentUpTime - m_dwDxDrawUpTime > DXDRAW_REINIT_TIMEOUT)	||
			pDoc->m_pDib->GetWidth() != pDoc->m_pDxDraw->GetSrcWidth()		||
			pDoc->m_pDib->GetHeight() != pDoc->m_pDxDraw->GetSrcHeight()	||				
			IsDxCompressionDifferent())
		{
			m_dwDxDrawUpTime = dwCurrentUpTime;
			if (!InitDxDraw(pDoc->m_pDib->GetWidth(), pDoc->m_pDib->GetHeight(), pDoc->m_pDib->GetCompression()))
			{
				::LeaveCriticalSection(&pDoc->m_csDib);
				return FALSE;
			}
		}
	}

	// Draw if initialized
	if (pDoc->m_pDxDraw->IsInit())
	{
		// Update Current Device
		pDoc->m_pDxDraw->UpdateCurrentDevice();

		// Erase Background, full erase if drawing a message
		EraseDxBkgnd(bStopAndChangeFormat || bDxDrawInfoText || bWatchDogAlarm || !bVideoView);

		// Display: Change Size
		if (bStopAndChangeFormat)
			pDoc->m_pDxDraw->DrawText(ML_STRING(1569, "Change Size"), 0, 0, DRAWTEXT_TOPLEFT);	// Only ASCII string supported!
		// Display: Info Text
		else if (bDxDrawInfoText)
			DxDrawInfoText();
		// Display: No Frames
		else if (bWatchDogAlarm)
			pDoc->m_pDxDraw->DrawText(ML_STRING(1570, "No Frames"), 0, 0, DRAWTEXT_TOPLEFT);	// Only ASCII string supported!
		// Draw Frame + Text + DC
		else if (bVideoView)
		{
			// Draw Frame
			pDoc->m_pDxDraw->RenderDib(pDoc->m_pDib, m_ZoomRect);

			// Text Drawing
			DxDrawText();
			
			// DC Drawing
			if (pDoc->m_bDetectingMovement		||
				pDoc->m_bShowEditDetectionZones	||
				((pDoc->m_dwVideoProcessorMode & SOFTWARE_MOVEMENT_DETECTOR) &&
				pDoc->m_bShowMovementDetections))
				DxDrawDC();
		}
		// Display: Preview Off
		else
			pDoc->m_pDxDraw->DrawText(ML_STRING(1571, "Preview Off"), 0, 0, DRAWTEXT_TOPLEFT);	// Only ASCII string supported!

		// Blt
		if (pDoc->m_pDxDraw->Blt(m_ZoomRect, CRect(0, 0, pDoc->m_pDib->GetWidth(), pDoc->m_pDib->GetHeight())))
			m_dwDxDrawUpTime = dwCurrentUpTime;
	}

	// Leave CS
	::LeaveCriticalSection(&pDoc->m_csDib);

	return TRUE;
}

__forceinline void CVideoDeviceView::DxDrawInfoText()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc); crashing because called from non UI thread!

	// Vars
	CSize szFontSize = pDoc->m_pDxDraw->GetFontSize();
	int nMaxChars = pDoc->m_pDib->GetWidth() / szFontSize.cx;
	int nMaxLines = pDoc->m_pDib->GetHeight() / szFontSize.cy;
	CStringArray sMessages;
	CString sInfoMsg;
	
	// Snapshots
	sInfoMsg = _T("");
	if (pDoc->m_bSnapshotLiveJpeg)
		sInfoMsg += _T("Live");
	if (pDoc->m_bSnapshotHistoryJpeg)
	{
		if (sInfoMsg.IsEmpty())
			sInfoMsg += _T("Jpegs");
		else
			sInfoMsg += _T(",Jpegs");
	}
	if (pDoc->m_bSnapshotHistorySwf)
	{
		if (sInfoMsg.IsEmpty())
			sInfoMsg += _T("Swf");
		else
			sInfoMsg += _T(",Swf");
	}
	if (sInfoMsg.IsEmpty())
	{
		sInfoMsg = _T("Shot: Off");
		sMessages.Add(sInfoMsg);
	}
	else
	{
		sInfoMsg = _T("Shot: ") + sInfoMsg;
		sMessages.Add(sInfoMsg);
		if (pDoc->m_bSnapshotStartStop)
		{
			sInfoMsg.Format(_T("      %02d:%02d:%02d-%02d:%02d:%02d"),
						pDoc->m_SnapshotStartTime.GetHour(),
						pDoc->m_SnapshotStartTime.GetMinute(),
						pDoc->m_SnapshotStartTime.GetSecond(),
						pDoc->m_SnapshotStopTime.GetHour(),
						pDoc->m_SnapshotStopTime.GetMinute(),
						pDoc->m_SnapshotStopTime.GetSecond());
			sMessages.Add(sInfoMsg);
		}
	}

	// Motion Detection
	sInfoMsg = _T("Det:  ");
	switch (pDoc->m_dwVideoProcessorMode)
	{
		case NO_DETECTOR :						sInfoMsg += _T("Off"); break;
		case TRIGGER_FILE_DETECTOR :			sInfoMsg += _T("Trigger"); break;
		case SOFTWARE_MOVEMENT_DETECTOR :		sInfoMsg += _T("Soft"); break;
		case (	TRIGGER_FILE_DETECTOR |
				SOFTWARE_MOVEMENT_DETECTOR):	sInfoMsg += _T("Trigger+Soft"); break;
		default: break;
	}
	sMessages.Add(sInfoMsg);
	if (pDoc->m_bDetectionStartStop)
	{
		sInfoMsg.Format(_T("      %02d:%02d:%02d-%02d:%02d:%02d"),
					pDoc->m_DetectionStartTime.GetHour(),
					pDoc->m_DetectionStartTime.GetMinute(),
					pDoc->m_DetectionStartTime.GetSecond(),
					pDoc->m_DetectionStopTime.GetHour(),
					pDoc->m_DetectionStopTime.GetMinute(),
					pDoc->m_DetectionStopTime.GetSecond());
		sMessages.Add(sInfoMsg);
	}

	// UDP Network Server
	if (pDoc->m_bSendVideoFrame)
		sInfoMsg.Format(_T("Net:  On(%d)"), pDoc->m_nSendFrameVideoPort);
	else
		sInfoMsg.Format(_T("Net:  Off"));
	sMessages.Add(sInfoMsg);

	// Network Device
	if (pDoc->m_pGetFrameNetCom)
	{
		if (pDoc->m_pGetFrameNetCom->IsClient())
		{
			if (pDoc->m_pHttpGetFrameParseProcess)
			{
				switch(pDoc->m_nNetworkDeviceTypeMode)
				{
					case CVideoDeviceDoc::OTHERONE :
						if (pDoc->m_pHttpGetFrameParseProcess->m_FormatType == CVideoDeviceDoc::CHttpGetFrameParseProcess::FORMATMJPEG)
						{
							sInfoMsg = _T("Cam:  Server Push");
							sMessages.Add(sInfoMsg);
							if (pDoc->m_nHttpGetFrameLocationPos < pDoc->m_HttpGetFrameLocations.GetSize())
								sInfoMsg = _T("      ") + pDoc->m_HttpGetFrameLocations[pDoc->m_nHttpGetFrameLocationPos];
							else
								sInfoMsg = _T("      ") + pDoc->m_HttpGetFrameLocations[0];
							sMessages.Add(sInfoMsg);
						}
						else if (pDoc->m_pHttpGetFrameParseProcess->m_FormatType == CVideoDeviceDoc::CHttpGetFrameParseProcess::FORMATJPEG)
						{
							sInfoMsg = _T("Cam:  Client Poll");
							sMessages.Add(sInfoMsg);
							if (pDoc->m_nHttpGetFrameLocationPos < pDoc->m_HttpGetFrameLocations.GetSize())
								sInfoMsg = _T("      ") + pDoc->m_HttpGetFrameLocations[pDoc->m_nHttpGetFrameLocationPos];
							else
								sInfoMsg = _T("      ") + pDoc->m_HttpGetFrameLocations[0];
							sMessages.Add(sInfoMsg);
						}
						break;
					case CVideoDeviceDoc::AXIS_SP :		sInfoMsg = _T("Cam:  Axis"); sMessages.Add(sInfoMsg);
														sInfoMsg = _T("      Server Push"); sMessages.Add(sInfoMsg); break;
					case CVideoDeviceDoc::AXIS_CP :		sInfoMsg = _T("Cam:  Axis"); sMessages.Add(sInfoMsg);
														sInfoMsg = _T("      Client Poll"); sMessages.Add(sInfoMsg); break;
					case CVideoDeviceDoc::PANASONIC_SP :sInfoMsg = _T("Cam:  Panasonic"); sMessages.Add(sInfoMsg);
														sInfoMsg = _T("      Server Push"); sMessages.Add(sInfoMsg); break;
					case CVideoDeviceDoc::PANASONIC_CP :sInfoMsg = _T("Cam:  Panasonic"); sMessages.Add(sInfoMsg);
														sInfoMsg = _T("      Client Poll"); sMessages.Add(sInfoMsg); break;
					case CVideoDeviceDoc::PIXORD_SP :	sInfoMsg = _T("Cam:  Pixord"); sMessages.Add(sInfoMsg);
														sInfoMsg = _T("      Server Push"); sMessages.Add(sInfoMsg); break;
					case CVideoDeviceDoc::PIXORD_CP :	sInfoMsg = _T("Cam:  Pixord"); sMessages.Add(sInfoMsg);
														sInfoMsg = _T("      Client Poll"); sMessages.Add(sInfoMsg); break;
					case CVideoDeviceDoc::EDIMAX_SP :	sInfoMsg = _T("Cam:  Edimax"); sMessages.Add(sInfoMsg);
														sInfoMsg = _T("      Server Push"); sMessages.Add(sInfoMsg); break;
					case CVideoDeviceDoc::EDIMAX_CP :	sInfoMsg = _T("Cam:  Edimax"); sMessages.Add(sInfoMsg);
														sInfoMsg = _T("      Client Poll"); sMessages.Add(sInfoMsg); break;
					case CVideoDeviceDoc::TPLINK_SP :	sInfoMsg = _T("Cam:  TP-Link"); sMessages.Add(sInfoMsg);
														sInfoMsg = _T("      Server Push"); sMessages.Add(sInfoMsg); break;
					case CVideoDeviceDoc::TPLINK_CP :	sInfoMsg = _T("Cam:  TP-Link"); sMessages.Add(sInfoMsg);
														sInfoMsg = _T("      Client Poll"); sMessages.Add(sInfoMsg); break;
					default : break;
				}
			}
		}
		else if (pDoc->m_pGetFrameNetCom->IsDatagram())
		{
			sInfoMsg = _T("Cam:  Internal UDP");
			sMessages.Add(sInfoMsg);
		}
		if (pDoc->m_pGetFrameNetCom->GetSocketFamily() == AF_INET6)
			sInfoMsg = _T("      IPv6");
		else
			sInfoMsg = _T("      IPv4");
		sMessages.Add(sInfoMsg);
	}
	
	// Draw text, only ASCII strings supported!
	for (int y = 0 ; y < MIN(nMaxLines, sMessages.GetSize()) ; y++)
	{
		sInfoMsg = _T("");
		for (int x = 0 ; x < MIN(nMaxChars, sMessages[y].GetLength()) ; x++)
			sInfoMsg += sMessages[y][x];
		pDoc->m_pDxDraw->DrawText(sInfoMsg, 0, y*szFontSize.cy, DRAWTEXT_TOPLEFT);
	}
}

__forceinline void CVideoDeviceView::DxDrawText()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc); crashing because called from non UI thread!

	// Progress Display
	if (pDoc->m_SaveFrameListThread.IsWorking())
	{
		CString sProgress(_T(""));
		if (pDoc->m_SaveFrameListThread.GetFTPUploadProgress() < 100)
			sProgress.Format(_T("FTP: %d%%"), pDoc->m_SaveFrameListThread.GetFTPUploadProgress());
		else if (pDoc->m_SaveFrameListThread.GetSendMailProgress() < 100)
			sProgress.Format(_T("Email: %d%%"), pDoc->m_SaveFrameListThread.GetSendMailProgress());
		if (sProgress != _T(""))
			pDoc->m_pDxDraw->DrawText(sProgress, pDoc->m_pDib->GetWidth() - 1, 0, DRAWTEXT_TOPRIGHT);
	}

	// Recording
	if (pDoc->m_SaveFrameListThread.IsWorking())
		pDoc->m_pDxDraw->DrawText(_T("REC"), pDoc->m_pDib->GetWidth() - 1, pDoc->m_pDib->GetHeight() - 1, DRAWTEXT_BOTTOMRIGHT);
}

__forceinline void CVideoDeviceView::DxDrawDC()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc); crashing because called from non UI thread!

	// Get Back DC
	HDC hDC = pDoc->m_pDxDraw->GetBackDC();
	if (hDC)
	{
		// Draw Zones where Detection is enabled
		if (pDoc->m_bShowEditDetectionZones)
		{
			for (int i = 0 ; i < pDoc->m_lMovDetTotalZones ; i++)
			{
				if (pDoc->m_DoMovementDetection[i])
				{
					RECT rcDetZone;
					int nZoneWidth = pDoc->m_pDib->GetWidth() / pDoc->m_lMovDetXZonesCount;
					int nZoneHeight = pDoc->m_pDib->GetHeight() / pDoc->m_lMovDetYZonesCount;
					int nZoneOffsetX = i%pDoc->m_lMovDetXZonesCount * nZoneWidth;
					int nZoneOffsetY = i/pDoc->m_lMovDetXZonesCount * nZoneHeight;
					rcDetZone.left = MAX(0, nZoneOffsetX - 1);
					rcDetZone.top = MAX(0, nZoneOffsetY - 1);
					rcDetZone.right = nZoneOffsetX + nZoneWidth;
					rcDetZone.bottom = nZoneOffsetY + nZoneHeight;

					HPEN hPen = ::CreatePen(PS_SOLID, 1, MOVDET_SELECTED_ZONES_COLOR);
					HGDIOBJ hOldPen = ::SelectObject(hDC, hPen);
					HGDIOBJ hOldBrush = ::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));
					::Rectangle(hDC, rcDetZone.left, rcDetZone.top, rcDetZone.right, rcDetZone.bottom);
					::MoveToEx(hDC, rcDetZone.left + (rcDetZone.right -  rcDetZone.left) / 4, rcDetZone.top, NULL);
					::LineTo(hDC, rcDetZone.left, rcDetZone.top + (rcDetZone.right -  rcDetZone.left) / 4);
					::SelectObject(hDC, hOldBrush);
					::SelectObject(hDC, hOldPen);
					::DeleteObject(hPen);
				}
			}
		}

		// Draw Detected Zones
		if ((pDoc->m_dwVideoProcessorMode & SOFTWARE_MOVEMENT_DETECTOR) &&
			pDoc->m_bShowMovementDetections)
		{
			DWORD dwCurrentTime = ::timeGetTime();
			for (int i = 0 ; i < pDoc->m_lMovDetTotalZones ; i++)
			{
				if (pDoc->m_MovementDetections[i])
				{
					RECT rcDetZone;
					int nZoneWidth = pDoc->m_pDib->GetWidth() / pDoc->m_lMovDetXZonesCount;
					int nZoneHeight = pDoc->m_pDib->GetHeight() / pDoc->m_lMovDetYZonesCount;
					int nZoneOffsetX = i%pDoc->m_lMovDetXZonesCount * nZoneWidth;
					int nZoneOffsetY = i/pDoc->m_lMovDetXZonesCount * nZoneHeight;
					rcDetZone.left = MAX(0, nZoneOffsetX - 1);
					rcDetZone.top = MAX(0, nZoneOffsetY - 1);
					rcDetZone.right = nZoneOffsetX + nZoneWidth;
					rcDetZone.bottom = nZoneOffsetY + nZoneHeight;

					HPEN hPen = ::CreatePen(PS_SOLID, 1, MOVDET_DETECTING_ZONES_COLOR);
					HGDIOBJ hOldPen = ::SelectObject(hDC, hPen);
					HGDIOBJ hOldBrush = ::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));
					::Rectangle(hDC, rcDetZone.left, rcDetZone.top, rcDetZone.right, rcDetZone.bottom);
					::MoveToEx(hDC, rcDetZone.left + (rcDetZone.right -  rcDetZone.left) / 4, rcDetZone.top, NULL);
					::LineTo(hDC, rcDetZone.left, rcDetZone.top + (rcDetZone.right -  rcDetZone.left) / 4);
					::SelectObject(hDC, hOldBrush);
					::SelectObject(hDC, hOldPen);
					::DeleteObject(hPen);
				}
			}
		}
	}

	// Release Back DC
	pDoc->m_pDxDraw->ReleaseBackDC(hDC);
}

BOOL CVideoDeviceView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CVideoDeviceView::OnDraw(CDC* pDC) 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!m_bDxDrawFirstInitOk)
	{
		// Flicker free drawing
		CRect rcClient;
		GetClientRect(&rcClient);
		CMyMemDC MemDC(pDC, &rcClient);

		//  Erase Background
		CBrush br;
		br.CreateSolidBrush(pDoc->m_crBackgroundColor);	
		MemDC.FillRect(&rcClient, &br);

		// Create font
		if (!(HFONT)m_GDIDrawFont)
		{
			LOGFONT lf;
			memset(&lf, 0, sizeof(LOGFONT));
			_tcscpy(lf.lfFaceName, _T("Arial"));
			lf.lfHeight = -MulDiv(11, pDC->GetDeviceCaps(LOGPIXELSY), 72);
			lf.lfWeight = FW_NORMAL;
			lf.lfItalic = 0;
			lf.lfUnderline = 0;
			m_GDIDrawFont.CreateFontIndirect(&lf);
		}

		// Set colors
		COLORREF crOldTextColor = MemDC.SetTextColor(DXDRAW_MESSAGE_COLOR);
		int nOldBkMode = MemDC.SetBkMode(OPAQUE);
		COLORREF crOldBkColor = MemDC.SetBkColor(pDoc->m_crBackgroundColor);
		CFont* pOldFont = MemDC.SelectObject(&m_GDIDrawFont);

		// Draw
		if (m_bDxDrawInitFailed)
		{
			MemDC.DrawText(	ML_STRING(1747, "DirectX failed"),
							-1,
							&rcClient,
							(DT_CENTER | DT_VCENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
		}
		else
		{
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
		}

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

	CUImagerView::OnLButtonDown(nFlags, point);

	if (pDoc->m_bShowEditDetectionZones)
	{
		if (pDoc->m_lMovDetTotalZones > 0)
		{
			// Width & Height
			int nZoneWidth = m_ZoomRect.Width() / pDoc->m_lMovDetXZonesCount;
			int nZoneHeight = m_ZoomRect.Height() / pDoc->m_lMovDetYZonesCount;
			if (nZoneWidth <= 0 || nZoneHeight <= 0)
				return;

			// Offset Remove
			point.x -= m_ZoomRect.left;
			point.y -= m_ZoomRect.top;

			// Check if inside Frame
			if (point.x < 0 ||
				point.y < 0 ||
				point.x > m_ZoomRect.Width() ||
				point.y > m_ZoomRect.Height())
				return;

			int x = point.x / nZoneWidth;
			int y = point.y / nZoneHeight;

			// The Selected Zone Index
			int nZone = x + y * pDoc->m_lMovDetXZonesCount;

			// Reset Zone State
			if ((nFlags & MK_SHIFT) ||
				(nFlags & MK_CONTROL))
				pDoc->m_DoMovementDetection[nZone] = FALSE;
			// Set Zone State
			else
				pDoc->m_DoMovementDetection[nZone] = TRUE;

			// Store Settings
			if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
			{
				CString sSection(pDoc->GetDevicePathName());
				CString sZone;
				sZone.Format(MOVDET_ZONE_FORMAT, nZone);
				((CUImagerApp*)::AfxGetApp())->WriteProfileInt(sSection, sZone, pDoc->m_DoMovementDetection[nZone]);
			}
		}
	}
	else
	{
		ForceCursor();
		SetCapture();
		m_bDxDrawInfoText = TRUE;
	}
}

void CVideoDeviceView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CUImagerView::OnLButtonUp(nFlags, point);
	if (m_bDxDrawInfoText)
	{
		ForceCursor(FALSE);
		m_bDxDrawInfoText = FALSE;
		ReleaseCapture();
	}
}

void CVideoDeviceView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	EnableCursor();
	CUImagerView::OnMButtonDown(nFlags, point);
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
		case _T('F') :
			::AfxGetMainFrame()->EnterExitFullscreen();
			break;

		case _T('S') :
			if (pDoc->m_pDxCapture															||
				((CUImagerApp*)::AfxGetApp())->IsDoc((CUImagerDoc*)(pDoc->m_pVideoAviDoc))	||
				pDoc->m_pGetFrameNetCom)
				pDoc->CaptureSettings();
			break;

		case _T('P') :
			if (pDoc->m_bCapture)
				pDoc->ViewVideo();
			break;

		case _T('T') :
			if (pDoc->m_bCapture)
				pDoc->m_bShowFrameTime = !pDoc->m_bShowFrameTime;
			break;

		case _T('R') :
			if (pDoc->m_bCapture					&&
				!pDoc->m_bAboutToStopRec			&&
				!pDoc->m_bAboutToStartRec)
				pDoc->CaptureRecord();
			break;

		case VK_PAUSE :
			if (pDoc->m_pAVRec						&&
				!pDoc->m_bAboutToStopRec			&&
				!pDoc->m_bAboutToStartRec)
				pDoc->CaptureRecordPause();
			break;

		case VK_ESCAPE :
			if (::GetKeyState(VK_SHIFT) < 0)
				::AfxGetMainFrame()->PostMessage(WM_CLOSE, 0, 0);
			else
			{
				if (pDoc->m_pVideoDevicePropertySheet && pDoc->m_pVideoDevicePropertySheet->IsVisible())
					pDoc->m_pVideoDevicePropertySheet->Hide();
				else if (m_bFullScreenMode)
				{
					if (((CUImagerApp*)::AfxGetApp())->m_bEscExit)
						::AfxGetMainFrame()->PostMessage(WM_CLOSE, 0, 0);
					else
						::AfxGetMainFrame()->EnterExitFullscreen();	// Exit Full-Screen Mode
				}
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
					CString sMsg;
					sMsg.Format(_T("%s unplugged\n"), pDoc->GetDeviceName());
					TRACE(sMsg);
					::LogLine(sMsg);
                    break;
				}
				// Device is available again
				else if (evParam2 == 1)
				{
					// I used to re-open here but some devices lock when
					// stopping their graph, other devices generate a blue-screen
					// and/or a reboot! The best solution is to do nothing,
					// leave everything open and in CVideoDeviceChildFrame::EndShutdown()
					// do not delete m_pDxCapture if m_bWatchDogAlarm is set. We get a
					// small memory leak but that's by far better than a full computer crash!
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
		// Width & Height
		int nZoneWidth = m_ZoomRect.Width() / pDoc->m_lMovDetXZonesCount;
		int nZoneHeight = m_ZoomRect.Height() / pDoc->m_lMovDetYZonesCount;
		if (nZoneWidth <= 0 || nZoneHeight <= 0)
			return;

		// Offset Remove
		point.x -= m_ZoomRect.left;
		point.y -= m_ZoomRect.top;

		// Check if inside Frame
		if (point.x < 0 ||
			point.y < 0 ||
			point.x > m_ZoomRect.Width() ||
			point.y > m_ZoomRect.Height())
			return;

		int x = point.x / nZoneWidth;
		int y = point.y / nZoneHeight;

		// The Selected Zone Index
		int nZone = x + y * pDoc->m_lMovDetXZonesCount;

		// Reset Zone State
		if ((nFlags & MK_SHIFT) ||
			(nFlags & MK_CONTROL))
			pDoc->m_DoMovementDetection[nZone] = FALSE;
		// Set Zone State
		else
			pDoc->m_DoMovementDetection[nZone] = TRUE;

		// Store Settings
		if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		{
			CString sSection(pDoc->GetDevicePathName());
			CString sZone;
			sZone.Format(MOVDET_ZONE_FORMAT, nZone);
			((CUImagerApp*)::AfxGetApp())->WriteProfileInt(sSection, sZone, pDoc->m_DoMovementDetection[nZone]);
		}
	}
	
	CUImagerView::OnMouseMove(nFlags, point);
}

void CVideoDeviceView::OnEditSelectall() 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	int i;

	for (i = 0 ; i < pDoc->m_lMovDetTotalZones ; i++)
		pDoc->m_DoMovementDetection[i] = TRUE;

	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
	{
		CString sSection(pDoc->GetDevicePathName());
		for (i = 0 ; i < pDoc->m_lMovDetTotalZones ; i++)
		{
			CString sZone;
			sZone.Format(MOVDET_ZONE_FORMAT, i);
			((CUImagerApp*)::AfxGetApp())->WriteProfileInt(sSection, sZone, pDoc->m_DoMovementDetection[i]);
		}
	}
}

void CVideoDeviceView::OnEditSelectnone() 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	int i;

	for (i = 0 ; i < pDoc->m_lMovDetTotalZones ; i++)
		pDoc->m_DoMovementDetection[i] = FALSE;

	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
	{
		CString sSection(pDoc->GetDevicePathName());
		for (i = 0 ; i < pDoc->m_lMovDetTotalZones ; i++)
		{
			CString sZone;
			sZone.Format(MOVDET_ZONE_FORMAT, i);
			((CUImagerApp*)::AfxGetApp())->WriteProfileInt(sSection, sZone, pDoc->m_DoMovementDetection[i]);
		}
	}
}

BOOL CVideoDeviceView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	EnableCursor();	
	return CUImagerView::OnMouseWheel(nFlags, zDelta, pt);
}

#endif
