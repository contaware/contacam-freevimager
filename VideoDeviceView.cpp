#include "stdafx.h"
#include "uImager.h"
#include "MainFrm.h"
#include "VideoDeviceDoc.h"
#include "PostDelayedMessage.h"
#include "VideoDeviceView.h"
#include "ColorDetectionPage.h"
#include "VideoDevicePropertySheet.h"
#include "DxCapture.h"
#include "MemDC.h"

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
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_THREADSAFE_CAPTURESETTINGS, OnThreadSafeCaptureSettings)
	ON_MESSAGE(WM_THREADSAFE_UPDATE_PHPPARAMS, OnThreadSafeUpdatePhpParams)
	ON_MESSAGE(WM_THREADSAFE_CHANGEVIDEOFORMAT, OnThreadSafeChangeVideoFormat)
	ON_MESSAGE(WM_THREADSAFE_STOP_AND_CHANGEVIDEOFORMAT, OnThreadSafeStopAndChangeVideoFormat)
	ON_MESSAGE(WM_ENABLE_DISABLE_CRITICAL_CONTROLS, OnEnableDisableCriticalControls)
	ON_MESSAGE(WM_THREADSAFE_INIT_MOVDET, OnThreadSafeInitMovDet)
	ON_MESSAGE(WM_DIRECTSHOW_GRAPHNOTIFY, OnDirectShowGraphNotify)
	ON_MESSAGE(WM_THREADSAFE_OPENGETVIDEO, OnThreadSafeOpenGetVideo)
	ON_MESSAGE(WM_THREADSAFE_AUTORUNREMOVEDEVICE_CLOSEDOC, OnThreadSafeAutorunRemoveDeviceCloseDoc)
	ON_MESSAGE(WM_THREADSAFE_SENDFRAME_MSG, OnThreadSafeSendFrameMsg)
	ON_MESSAGE(WM_THREADSAFE_DXDRAW_INIT, OnThreadSafeDxDrawInit)
END_MESSAGE_MAP()

	
CVideoDeviceView::CVideoDeviceView()
{
	// Init vars
	m_bInitializingDxDraw = FALSE;
	m_bDxDrawInitFailed = FALSE;
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

	if (pDoc->m_pGeneralPage && !pDoc->m_bVfWDialogDisplaying)
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

	// X
	for (i = 72 ; i > 0 ; i -= 8)
	{
		if ((pDoc->m_DocRect.Width() % i) == 0 &&
			(pDoc->m_DocRect.Width() / i) >= MOVDET_MIN_ZONESX)
		{
			lMovDetXZonesCount = pDoc->m_DocRect.Width() / i;
			break;
		}
	}

	// Y
	for (i = 72 ; i > 0 ; i -= 8)
	{
		if ((pDoc->m_DocRect.Height() % i) == 0 &&
			(pDoc->m_DocRect.Height() / i) >= MOVDET_MIN_ZONESY)
		{
			lMovDetYZonesCount = pDoc->m_DocRect.Height() / i;
			break;
		}
	}

	// Total
	LONG lMovDetTotalZones = lMovDetXZonesCount * lMovDetYZonesCount;

	// Check
	if (lMovDetTotalZones == 0 || lMovDetTotalZones > MOVDET_MAX_ZONES)
	{
		if (!pDoc->m_bUnsupportedVideoSizeForMovDet)
		{
			if (pDoc->m_pMovementDetectionPage)
			{
				CEdit* pEdit = (CEdit*)pDoc->m_pMovementDetectionPage->GetDlgItem(IDC_WARNING);
				if (pEdit)
					pEdit->ShowWindow(TRUE);
			}
			pDoc->m_bUnsupportedVideoSizeForMovDet = TRUE;
		}
		return 0;
	}
	else
	{
		if (pDoc->m_bUnsupportedVideoSizeForMovDet)
		{
			if (pDoc->m_pMovementDetectionPage)
			{
				CEdit* pEdit = (CEdit*)pDoc->m_pMovementDetectionPage->GetDlgItem(IDC_WARNING);
				if (pEdit)
					pEdit->ShowWindow(FALSE);
			}
			pDoc->m_bUnsupportedVideoSizeForMovDet = FALSE;
		}
		::InterlockedExchange(&pDoc->m_lMovDetXZonesCount, lMovDetXZonesCount);
		::InterlockedExchange(&pDoc->m_lMovDetYZonesCount, lMovDetYZonesCount);
		::InterlockedExchange(&pDoc->m_lMovDetTotalZones, lMovDetTotalZones);
	}

	// Load Settings
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
	{
		CWinApp* pApp = ::AfxGetApp();
		CString sSection(pDoc->GetDevicePathName());
		if (pDoc->m_lMovDetTotalZones == pApp->GetProfileInt(sSection, _T("MovDetTotalZones"), 0))
		{
			for (i = 0 ; i < pDoc->m_lMovDetTotalZones ; i++)
			{
				CString sZone;
				sZone.Format(_T("DoMovementDetection%03i"), i);
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
				sZone.Format(_T("DoMovementDetection%03i"), i);
				((CUImagerApp*)::AfxGetApp())->WriteProfileInt(sSection, sZone, pDoc->m_DoMovementDetection[i]);
			}
		}
	}
	else
	{
		for (i = 0 ; i < pDoc->m_lMovDetTotalZones ; i++)
			pDoc->m_DoMovementDetection[i] = TRUE;
	}

	return 1;
}

afx_msg LONG CVideoDeviceView::OnThreadSafeOpenGetVideo(WPARAM wparam, LPARAM lparam)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (pDoc->OpenGetVideo())
		return 1;
	else
	{
		OnThreadSafeAutorunRemoveDeviceCloseDoc(0, 0);
		return 0;
	}
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

afx_msg LONG CVideoDeviceView::OnThreadSafeDxDrawInit(WPARAM wparam, LPARAM lparam)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	WORD wWidth = LOWORD(wparam);
	WORD wHeight = HIWORD(wparam);
	CTimeSpan ElapsedTimeSinceLastSessionChange = CTime::GetCurrentTime() - ::AfxGetMainFrame()->m_SessionChangeTime;
	if (wWidth > 0 && wHeight > 0									&&
		::AfxGetMainFrame()->m_lSessionDisconnectedLockedCount <= 0	&&
		ElapsedTimeSinceLastSessionChange.GetTotalSeconds() >= SESSIONCHANGE_WAIT_SEC)
	{
		if (pDoc->m_DxDraw.Init(	GetSafeHwnd(),
									(int)wWidth,
									(int)wHeight,
									(DWORD)lparam,
									IDB_BITSTREAM_VERA_11))
		{
			m_bDxDrawInitFailed = FALSE;
			m_bInitializingDxDraw = FALSE;
			return 1;
		}
		else
		{
			pDoc->m_bDecodeFramesForPreview = TRUE;
			m_bInitializingDxDraw = FALSE;
			if (!m_bDxDrawInitFailed && lparam == 0) // if also BI_RGB failed, display "DirectX failed" in OnDraw()
			{
				m_bDxDrawInitFailed = TRUE;
				Invalidate();
			}
			return 0;
		}
	}
	else
	{
		m_bInitializingDxDraw = FALSE;
		return 0;
	}
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

__forceinline void CVideoDeviceView::EraseBkgnd(BOOL bFullErase)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc); Crashing because called also from process thread!

	// Erase Full-Screen Borders
	if (::AfxGetMainFrame()->m_bFullScreenMode)
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
			pDoc->m_DxDraw.ClearFront(&rcTop);
		if ((rcLeft.Width() > 0) && (rcLeft.Height() > 0))
			pDoc->m_DxDraw.ClearFront(&rcLeft);
		if ((rcRight.Width() > 0) && (rcRight.Height() > 0))
			pDoc->m_DxDraw.ClearFront(&rcRight);
		if ((rcBottom.Width() > 0) && (rcBottom.Height() > 0))
			pDoc->m_DxDraw.ClearFront(&rcBottom);
	}

	// Erase All
	if (bFullErase)
		pDoc->m_DxDraw.ClearBack();
}

__forceinline BOOL CVideoDeviceView::IsCompressionDifferent()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc); Crashing because called also from process thread!

	// Check
	if (!pDoc->m_pDib)
		return FALSE;
	// YUY2 Format Equivalents
	else if (pDoc->m_DxDraw.GetCurrentSrcFourCC() == FCC('YUY2')	&&
			(pDoc->m_pDib->GetCompression() == FCC('YUNV')			||
			pDoc->m_pDib->GetCompression() == FCC('VYUY')			||
			pDoc->m_pDib->GetCompression() == FCC('V422')			||
			pDoc->m_pDib->GetCompression() == FCC('YUYV')))
		return FALSE;
	// Special Handling for YV12 Format
	else if (pDoc->m_DxDraw.GetCurrentSrcFourCC() == FCC('YV12'))
	{
		if (!pDoc->m_DxDraw.GetCurrentSrcFlipUV()			&&
			pDoc->m_pDib->GetCompression() == FCC('YV12'))
			return FALSE;
		else if (pDoc->m_DxDraw.GetCurrentSrcFlipUV()		&&
			(pDoc->m_pDib->GetCompression() == FCC('I420')	||
			pDoc->m_pDib->GetCompression() == FCC('IYUV')))
			return FALSE;
		else
			return TRUE;
	}
	// RGB Format Equivalents
	else if (pDoc->m_DxDraw.GetCurrentSrcFourCC() == BI_RGB	&&
			(pDoc->m_pDib->GetCompression() == BI_BITFIELDS	||
			pDoc->m_pDib->GetCompression() == BI_RLE4		||
			pDoc->m_pDib->GetCompression() == FCC('RLE4')	||
			pDoc->m_pDib->GetCompression() == BI_RLE8		||
			pDoc->m_pDib->GetCompression() == FCC('RLE8')))
		return FALSE;
	// Remaining Formats
	else
		return (pDoc->m_pDib->GetCompression() != pDoc->m_DxDraw.GetCurrentSrcFourCC());
}

void CVideoDeviceView::Draw()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc); Crashing because called also from process thread!

	// Nothing to draw as a service or if not connected
	if (((CUImagerApp*)::AfxGetApp())->m_bServiceProcess ||
		::AfxGetMainFrame()->m_lSessionDisconnectedLockedCount > 0)
		return;

	// Enter CS here, also m_bInitializingDxDraw must be under the cs
	// so that if two or more Draw() are called from different threads
	// m_bInitializingDxDraw can be set only by one of them!
	::EnterCriticalSection(&pDoc->m_csDib);

	// Main UI thread is initializing the DxDraw object for us
	// -> no drawing at this moment!
	if (!pDoc->m_pDib || m_bInitializingDxDraw)
	{
		::LeaveCriticalSection(&pDoc->m_csDib);
		return;
	}

	// Init local vars
	DWORD dwCurrentUpTime = ::timeGetTime();
	BOOL bVideoView = pDoc->m_bVideoView;
	BOOL bVfWVideoFormatApplyPressed = pDoc->m_bVfWVideoFormatApplyPressed;
	BOOL bStopAndChangeFormat = pDoc->m_bStopAndChangeFormat;
	BOOL bWatchDogAlarm = pDoc->m_bWatchDogAlarm;

	// Preview Off?
	if (!bVideoView)
	{
		if (!pDoc->m_DxDraw.IsInit()										||
			(dwCurrentUpTime - m_dwDxDrawUpTime > DXDRAW_REINIT_TIMEOUT)	||
			pDoc->m_pDib->GetWidth() != pDoc->m_DxDraw.GetSrcWidth()		||
			pDoc->m_pDib->GetHeight() != pDoc->m_DxDraw.GetSrcHeight()		||				
			pDoc->m_DxDraw.GetCurrentSrcFourCC() != BI_RGB)
		{
			// Dx draw must be init from the main UI thread,
			// otherwise it crashes on some machines with
			// certain graphic cards!
			// Init to RGB to be able to display
			// the messages on a black empty screen
			m_dwDxDrawUpTime = dwCurrentUpTime;
			m_bInitializingDxDraw = TRUE;
			::PostMessage(	GetSafeHwnd(),
							WM_THREADSAFE_DXDRAW_INIT,
							MAKEWPARAM((WORD)(pDoc->m_pDib->GetWidth()), (WORD)(pDoc->m_pDib->GetHeight())),
							(LPARAM)BI_RGB);
			::LeaveCriticalSection(&pDoc->m_csDib);
			return;
		}	
	}
	else
	{
		if (!pDoc->m_DxDraw.IsInit()										||
			(dwCurrentUpTime - m_dwDxDrawUpTime > DXDRAW_REINIT_TIMEOUT)	||
			pDoc->m_pDib->GetWidth() != pDoc->m_DxDraw.GetSrcWidth()		||
			pDoc->m_pDib->GetHeight() != pDoc->m_DxDraw.GetSrcHeight()		||				
			IsCompressionDifferent())
		{
			// Dx draw must be init from the main UI thread,
			// otherwise it crashes on some machines with
			// certain graphic cards!
			m_dwDxDrawUpTime = dwCurrentUpTime;
			m_bInitializingDxDraw = TRUE;
			::PostMessage(	GetSafeHwnd(),
							WM_THREADSAFE_DXDRAW_INIT,
							MAKEWPARAM((WORD)(pDoc->m_pDib->GetWidth()), (WORD)(pDoc->m_pDib->GetHeight())),
							(LPARAM)pDoc->m_pDib->GetCompression());
			::LeaveCriticalSection(&pDoc->m_csDib);
			return;
		}
	}

	// Draw if initialized
	if (pDoc->m_DxDraw.IsInit())
	{
		// Update Current Device
		pDoc->m_DxDraw.UpdateCurrentDevice();

		// Erase Background, full erase if drawing a message
		BOOL bDrawMsg = !bVideoView						||
						bVfWVideoFormatApplyPressed		||
						bStopAndChangeFormat			||
						bWatchDogAlarm;
		EraseBkgnd(bDrawMsg);

		// Display: OK or Cancel
		if (bVfWVideoFormatApplyPressed)
			pDoc->m_DxDraw.DrawText(ML_STRING(1567, "OK or Cancel"), 0, 0, DRAWTEXT_TOPLEFT);	// Only ASCII string supported!
		// Display: Change Size
		else if (bStopAndChangeFormat)
			pDoc->m_DxDraw.DrawText(ML_STRING(1569, "Change Size"), 0, 0, DRAWTEXT_TOPLEFT);	// Only ASCII string supported!
		// Display: No Frames
		else if (bWatchDogAlarm)
			pDoc->m_DxDraw.DrawText(ML_STRING(1570, "No Frames"), 0, 0, DRAWTEXT_TOPLEFT);		// Only ASCII string supported!
		// Draw Frame + Info
		else if (bVideoView)
		{
			// Draw Frame
			pDoc->m_DxDraw.RenderDib(pDoc->m_pDib, m_ZoomRect);

			// Text Drawing
			DrawText();
			
			// DC Drawing
			if (pDoc->m_bDetectingMovement		||
				pDoc->m_bShowEditDetectionZones	||
				((pDoc->m_VideoProcessorMode & MOVEMENT_DETECTOR) &&
				pDoc->m_bShowMovementDetections))
				DrawDC();
		}
		// Display: Preview Off
		else
			pDoc->m_DxDraw.DrawText(ML_STRING(1571, "Preview Off"), 0, 0, DRAWTEXT_TOPLEFT);	// Only ASCII string supported!
		
		// Blt
		if (pDoc->m_DxDraw.Blt(m_ZoomRect, CRect(0, 0, pDoc->m_pDib->GetWidth(), pDoc->m_pDib->GetHeight())))
			m_dwDxDrawUpTime = dwCurrentUpTime;
	}

	// Leave CS
	::LeaveCriticalSection(&pDoc->m_csDib);
}

__forceinline void CVideoDeviceView::DrawText()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc); Crashing because called also from process thread!

	// Progress Display
	if (pDoc->m_SaveFrameListThread.IsWorking())
	{
		CString sProgress(_T(""));
		if (pDoc->m_SaveFrameListThread.GetFTPUploadProgress() < 100)
			sProgress.Format(_T("FTP: %d%%"), pDoc->m_SaveFrameListThread.GetFTPUploadProgress());
		else if (pDoc->m_SaveFrameListThread.GetSendMailProgress() < 100)
			sProgress.Format(_T("Email: %d%%"), pDoc->m_SaveFrameListThread.GetSendMailProgress());
		if (sProgress != _T(""))
			pDoc->m_DxDraw.DrawText(sProgress, pDoc->m_pDib->GetWidth() - 1, 0, DRAWTEXT_TOPRIGHT);
	}

	// Recording
	if (pDoc->m_SaveFrameListThread.IsWorking())
		pDoc->m_DxDraw.DrawText(_T("REC"), pDoc->m_pDib->GetWidth() - 1, pDoc->m_pDib->GetHeight() - 1, DRAWTEXT_BOTTOMRIGHT);
	// Movement Detection Zones Count
	else if (pDoc->m_bDoFalseDetectionCheck)
	{
		if (pDoc->m_nBlueMovementDetectionsCount > 0 &&
			pDoc->m_nNoneBlueMovementDetectionsCount > 0)
		{
			CString sMovDetCount;
			sMovDetCount.Format(_T("Blue %d"), pDoc->m_nBlueMovementDetectionsCount);
			pDoc->m_DxDraw.DrawText(sMovDetCount, pDoc->m_pDib->GetWidth() - 1, pDoc->m_pDib->GetHeight() - 19, DRAWTEXT_BOTTOMRIGHT);
			sMovDetCount.Format(_T("!Blue %d"), pDoc->m_nNoneBlueMovementDetectionsCount);
			pDoc->m_DxDraw.DrawText(sMovDetCount, pDoc->m_pDib->GetWidth() - 1, pDoc->m_pDib->GetHeight() - 1, DRAWTEXT_BOTTOMRIGHT);
		}
		else if (pDoc->m_nBlueMovementDetectionsCount > 0)
		{
			CString sMovDetCount;
			sMovDetCount.Format(_T("Blue %d"), pDoc->m_nBlueMovementDetectionsCount);
			pDoc->m_DxDraw.DrawText(sMovDetCount, pDoc->m_pDib->GetWidth() - 1, pDoc->m_pDib->GetHeight() - 1, DRAWTEXT_BOTTOMRIGHT);
		}
		else if (pDoc->m_nNoneBlueMovementDetectionsCount > 0)
		{
			CString sMovDetCount;
			sMovDetCount.Format(_T("!Blue %d"), pDoc->m_nNoneBlueMovementDetectionsCount);
			pDoc->m_DxDraw.DrawText(sMovDetCount, pDoc->m_pDib->GetWidth() - 1, pDoc->m_pDib->GetHeight() - 1, DRAWTEXT_BOTTOMRIGHT);
		}
	}
}

__forceinline void CVideoDeviceView::DrawDC()
{
	CVideoDeviceDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc); Crashing because called also from process thread!

	// Get Back DC
	HDC hDC = pDoc->m_DxDraw.GetBackDC();
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
		if ((pDoc->m_VideoProcessorMode & MOVEMENT_DETECTOR) &&
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
	pDoc->m_DxDraw.ReleaseBackDC(hDC);
}

BOOL CVideoDeviceView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

// No GDI Drawing after that m_DxDraw is initialized!
void CVideoDeviceView::OnDraw(CDC* pDC) 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!m_bInitializingDxDraw && !pDoc->m_DxDraw.IsInit())
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
			MemDC.SetTextColor(DXDRAW_ERROR_COLOR);
			if (!pDoc->m_DxDraw.HasDxDraw())
			{
				MemDC.DrawText(	ML_STRING(1221, "DirectX 7.0 or higher is required!"),	
								-1,
								&rcClient,
								(DT_LEFT | DT_BOTTOM | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
			}
			else
			{
				TCHAR lpszErrorMessage[DXDRAW_ERRORMSG_BUFSIZE];
				lpszErrorMessage[0] = _T('\0');
				pDoc->m_DxDraw.GetLastErrorMessage(lpszErrorMessage, DXDRAW_ERRORMSG_BUFSIZE);
				MemDC.DrawText(	lpszErrorMessage,	
								-1,
								&rcClient,
								(DT_LEFT | DT_BOTTOM | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
			}
		}
		else
		{
			MemDC.DrawText(	ML_STRING(1565, "Please wait..."),
							-1,
							&rcClient,
							(DT_CENTER | DT_VCENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
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

	if (pDoc->m_nDoColorPickup != 0)
	{
		pDoc->m_nDoColorPickup = 0;
		m_HsvPickupArray.RemoveAll();
	}
	else if (pDoc->m_bShowEditDetectionZones)
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

void CVideoDeviceView::ColorPickup(UINT nFlags, CPoint point)
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Get the average of 9 pixels
	DWORD R = 0;
	DWORD G = 0;
	DWORD B = 0;
	int nPixelsCount = 0;
	HDC hDC = ::GetDC(NULL);
	ClientToScreen(&point);
	COLORREF crColor = ::GetPixel(hDC, point.x, point.y);
	for (int y = -1 ; y <= 1 ; y++)
	{
		for (int x = -1 ; x <= 1 ; x++)
		{
			crColor = ::GetPixel(hDC, point.x + x, point.y + y);
			if (crColor != CLR_INVALID)
			{
				R += GetRValue(crColor);
				G += GetGValue(crColor);
				B += GetBValue(crColor);
				++nPixelsCount;
			}
		}
	}
	::ReleaseDC(NULL, hDC);
	R /= nPixelsCount;
	G /= nPixelsCount;
	B /= nPixelsCount;
	crColor = RGB(R,G,B);

	// RGB -> HSV
	CVideoDeviceDoc::CColorDetection::HsvEntry e;
	e.hue = GetRValue(crColor);
	e.saturation = GetGValue(crColor);
	e.value = GetBValue(crColor);
	BOOL bChromatic = ::RGB2HSV((int*)&e.hue, (int*)&e.saturation, (int*)&e.value);
	if (!bChromatic)
	{
		ReleaseCapture();
		::AfxMessageBox(ML_STRING(1572, "Please click on a color which is not black, gray or white!"));
		SetCapture();
		HCURSOR hCursor = ::AfxGetApp()->LoadCursor(IDC_SELECTCOLOR_CURSOR);	
		ASSERT(hCursor);
		::SetCursor(hCursor);
	}
	else
	{
		// Add to Hsv Pickup Array
		m_HsvPickupArray.Add(e);
		
		// Done?
		if (!((nFlags & MK_SHIFT) ||
			(nFlags & MK_CONTROL)))
		{
			int nIndex = -1;
			if (pDoc->m_nDoColorPickup  == -1)
				nIndex = pDoc->m_ColorDetection.AppendColor(m_HsvPickupArray);
			else
			{
				if (pDoc->m_ColorDetection.ReplaceColor(pDoc->m_nDoColorPickup - 1, m_HsvPickupArray))
					nIndex = pDoc->m_nDoColorPickup - 1;
			}
			if (nIndex >= 0 && pDoc->m_pColorDetectionPage)
			{
				pDoc->m_pColorDetectionPage->SetHueRadius(nIndex, pDoc->m_ColorDetection.GetHueRadius(nIndex));
				pDoc->m_pColorDetectionPage->SetSaturationRadius(nIndex, pDoc->m_ColorDetection.GetSaturationRadius(nIndex));
				pDoc->m_pColorDetectionPage->SetValueRadius(nIndex, pDoc->m_ColorDetection.GetValueRadius(nIndex));
				pDoc->m_pColorDetectionPage->UpdateControls();
			}
			pDoc->m_nDoColorPickup = 0;
			ReleaseCapture();
			m_HsvPickupArray.RemoveAll();
		}
	}
}

void CVideoDeviceView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// m_nDoColorPickup:
	// -1 Add
	// 0 Do Nothing
	// 1 First Color
	// 2 Second Color
	// ...
	if (pDoc->m_nDoColorPickup != 0)
	{
		EnableCursor();
		ColorPickup(nFlags, point);
	}
	else
		CUImagerView::OnLButtonDblClk(nFlags, point);
}

void CVideoDeviceView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CVideoDeviceDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	EnableCursor();

	CUImagerView::OnLButtonDown(nFlags, point);

	// m_nDoColorPickup:
	// -1 Add
	// 0 Do Nothing
	// 1 First Color
	// 2 Second Color
	// ...
	if (pDoc->m_nDoColorPickup != 0)
		ColorPickup(nFlags, point);
	else if (pDoc->m_bShowEditDetectionZones && pDoc->m_lMovDetTotalZones > 0)
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
			((CUImagerApp*)::AfxGetApp())->WriteProfileInt(sSection, _T("MovDetTotalZones"), pDoc->m_lMovDetTotalZones);
			CString sZone;
			sZone.Format(_T("DoMovementDetection%03i"), nZone);
			((CUImagerApp*)::AfxGetApp())->WriteProfileInt(sSection, sZone, pDoc->m_DoMovementDetection[nZone]);
		}
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
				::IsWindow(pDoc->m_VfWCaptureVideoThread.m_hCapWnd)							||
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
				!pDoc->m_bVfWDialogDisplaying		&&
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
				else if (::AfxGetMainFrame()->m_bFullScreenMode)
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
			if (!pDoc->m_bDoEditCopy || !pDoc->m_bDoEditPaste)
			{
				pDoc->m_bDoEditCopy = TRUE;
				pDoc->m_bDoEditPaste = TRUE;
			}
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
	
	if (pDoc->m_nDoColorPickup != 0)
	{
		hCursor = ::AfxGetApp()->LoadCursor(IDC_SELECTCOLOR_CURSOR);	
		ASSERT(hCursor);
		::SetCursor(hCursor);
		return TRUE;
	}
	// If Wait Cursor leave it!
	else if (((CUImagerApp*)::AfxGetApp())->IsWaitCursor())
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

	// Make sure that inside the MainFrame we have the color picker cursor
	if (pDoc->m_nDoColorPickup != 0)
	{
		CRect rcMainFrame;
		::AfxGetMainFrame()->GetWindowRect(&rcMainFrame);
		CPoint ptColorPick = point;
		ClientToScreen(&ptColorPick);
		if (rcMainFrame.PtInRect(ptColorPick))
		{
			HCURSOR hCursor = ::AfxGetApp()->LoadCursor(IDC_SELECTCOLOR_CURSOR);	
			ASSERT(hCursor);
			::SetCursor(hCursor);
		}
	}

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
			((CUImagerApp*)::AfxGetApp())->WriteProfileInt(sSection, _T("MovDetTotalZones"), pDoc->m_lMovDetTotalZones);
			CString sZone;
			sZone.Format(_T("DoMovementDetection%03i"), nZone);
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
		((CUImagerApp*)::AfxGetApp())->WriteProfileInt(sSection, _T("MovDetTotalZones"), pDoc->m_lMovDetTotalZones);
		for (i = 0 ; i < pDoc->m_lMovDetTotalZones ; i++)
		{
			CString sZone;
			sZone.Format(_T("DoMovementDetection%03i"), i);
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
		((CUImagerApp*)::AfxGetApp())->WriteProfileInt(sSection, _T("MovDetTotalZones"), pDoc->m_lMovDetTotalZones);
		for (i = 0 ; i < pDoc->m_lMovDetTotalZones ; i++)
		{
			CString sZone;
			sZone.Format(_T("DoMovementDetection%03i"), i);
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
