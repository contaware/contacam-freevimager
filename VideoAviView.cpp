#include "stdafx.h"
#include "uImager.h"
#include "MainFrm.h"
#include "AviPlay.h"
#include "VideoAviDoc.h"
#include "VideoAviView.h"
#include "AviInfoDlg.h"
#include "OutVolDlg.h"
#include "AudioVideoShiftDlg.h"
#include "PlayerToolBarDlg.h"
#include "PostDelayedMessage.h"
#include "MyMemDC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CVideoAviView, CUImagerView)

BEGIN_MESSAGE_MAP(CVideoAviView, CUImagerView)
	//{{AFX_MSG_MAP(CVideoAviView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_HSCROLL()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_VIEW_FULLSCREEN, OnViewFullscreen)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FULLSCREEN, OnUpdateViewFullscreen)
	ON_WM_SETCURSOR()
	ON_COMMAND(ID_VIEW_FIT, OnViewFit)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FIT, OnUpdateViewFit)
	ON_COMMAND(ID_PLAY_INC, OnPlayInc)
	ON_UPDATE_COMMAND_UI(ID_PLAY_INC, OnUpdatePlayInc)
	ON_COMMAND(ID_PLAY_DEC, OnPlayDec)
	ON_UPDATE_COMMAND_UI(ID_PLAY_DEC, OnUpdatePlayDec)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_THREADSAFE_LOAD_AVI, OnThreadSafeLoadAVI)
	ON_MESSAGE(WM_THREADSAFE_UPDATEPLAYSLIDER, OnThreadSafeUpdatePlaySlider)
	ON_MESSAGE(WM_AVIFILE_PROGRESS, OnAviFileProgress)
	ON_MESSAGE(WM_ENABLE_CURSOR, OnEnableCursor)
	ON_MESSAGE(WM_END_THUMBTRACK, OnEndThumbTrack)
END_MESSAGE_MAP()

#ifdef _DEBUG
void CVideoAviView::AssertValid() const
{
	CUImagerView::AssertValid();
}

void CVideoAviView::Dump(CDumpContext& dc) const
{
	CUImagerView::Dump(dc);
}

CVideoAviDoc* CVideoAviView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CVideoAviDoc)));
	return (CVideoAviDoc*)m_pDocument;
}
#endif //_DEBUG

CVideoAviView::CVideoAviView()
{
	m_hResizeCursor = NULL;
	m_bUserTopLeft = FALSE;
	m_bUserBottomRight = FALSE;
	m_bUserBottomLeft = FALSE;
	m_bUserTopRight = FALSE;
	m_bUserTop = FALSE;
	m_bUserBottom = FALSE;
	m_bUserLeft = FALSE;
	m_bUserRight = FALSE;
	m_bUserClickClipping = FALSE;
	m_ptUserClick = CPoint(0,0);
	m_UserClickZoomRect = CRect(0,0,0,0);
	m_hFont = NULL;
	m_bBigFont = FALSE;
	m_bThumbTrack = FALSE;
	m_bThumbTrackDone = TRUE;
	m_nThumbTrackPos = 0;
	m_nLastThumbTrackPos = 0;
	m_nPreviewThumbTrackPos = 0;
	m_dwThumbTrackSeq = 0U;
	m_bWasPlayingBeforeThumbTrack = FALSE;
}	

CVideoAviView::~CVideoAviView()
{	
	if (m_hFont)
		::DeleteObject(m_hFont);
}

int CVideoAviView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	CToolBarChildFrame* pFrame = (CToolBarChildFrame*)GetParentFrame();
	if ((pFrame != NULL) && (pFrame->IsKindOf(RUNTIME_CLASS(CToolBarChildFrame))))
		pFrame->SetToolBar(&m_VideoAviToolBar);

	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

LONG CVideoAviView::OnThreadSafeLoadAVI(WPARAM wparam, LPARAM lparam)
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	CString* pFileName = (CString*)wparam;
	DWORD dwFramePos = (DWORD)lparam;
	if (pFileName)
	{
		if (pDoc && !pDoc->m_bClosing)
		{	
			BOOL res = pDoc->LoadAVI(*pFileName, dwFramePos, FALSE);
			delete pFileName;
			if (!res)
				pDoc->CloseDocumentForce();
			return res;
		}
		else
		{
			delete pFileName;
			return 0;
		}
	}
	else
		return 0;
}

LONG CVideoAviView::OnThreadSafeUpdatePlaySlider(WPARAM wparam, LPARAM lparam)
{
	wparam;
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	if (pDoc &&
		!pDoc->m_bClosing)
	{
		UpdatePlaySlider();
		return 1;
	}
	else
		return 0;
}

void CVideoAviView::UpdatePlaySlider()
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (!pDoc->m_pAVIPlay || !m_bThumbTrackDone)
		return;

	// Normal-Screen Doc ToolBar Update
	if (::IsWindow(m_VideoAviToolBar.GetSafeHwnd()) && 
		::IsWindow(m_VideoAviToolBar.m_PlayerSlider.GetSafeHwnd()))
	{
		CAVIPlay::CAVIVideoStream* pVideoStream =
					pDoc->m_pAVIPlay->GetVideoStream(pDoc->m_nActiveVideoStream);
		if (pVideoStream)
		{
			int nCurrentFramePos = pVideoStream->GetCurrentFramePos();
			if (nCurrentFramePos < 0)
				nCurrentFramePos = 0;
			if (pVideoStream->GetTotalFrames() > 1)
				m_VideoAviToolBar.m_PlayerSlider.SetPos(Round(	(double)nCurrentFramePos /
																(double)((int)pVideoStream->GetTotalFrames() - 1) *
																(double)(m_VideoAviToolBar.m_PlayerSlider.GetRangeMax())));
			else
				m_VideoAviToolBar.m_PlayerSlider.SetPos(0);
		}
		else 
		{	
			CAVIPlay::CAVIAudioStream* pAudioStream =
					pDoc->m_pAVIPlay->GetAudioStream(pDoc->m_nActiveAudioStream);
			if (pAudioStream)
			{
				LONGLONG llCurrentSamplePos = pDoc->m_PlayAudioFileThread.GetCurrentSamplePos();
				if (llCurrentSamplePos < 0)
					llCurrentSamplePos = 0;
				if (pDoc->m_PlayAudioFileThread.GetTotalSamples() > 1)
					m_VideoAviToolBar.m_PlayerSlider.SetPos(Round(	(double)llCurrentSamplePos /
																	(double)(pDoc->m_PlayAudioFileThread.GetTotalSamples() - 1) *
																	(double)(m_VideoAviToolBar.m_PlayerSlider.GetRangeMax())));
			}
		}
	}
	
	// Full-Screen ToolBar Dialog Update
	if (pDoc->m_pPlayerToolBarDlg &&
		::IsWindow(pDoc->m_pPlayerToolBarDlg->GetSafeHwnd()) && 
		::IsWindow(pDoc->m_pPlayerToolBarDlg->m_VideoAviToolBar.m_PlayerSlider.GetSafeHwnd()))
	{
		CAVIPlay::CAVIVideoStream* pVideoStream =
					pDoc->m_pAVIPlay->GetVideoStream(pDoc->m_nActiveVideoStream);
		if (pVideoStream)
		{
			pDoc->m_pPlayerToolBarDlg->m_VideoAviToolBar.m_PlayerSlider.SetPos(	Round((double)pVideoStream->GetCurrentFramePos() /
																				(double)((int)pVideoStream->GetTotalFrames() - 1) *
																				(double)(pDoc->m_pPlayerToolBarDlg->m_VideoAviToolBar.m_PlayerSlider.GetRangeMax())));
		}
		else
		{
			CAVIPlay::CAVIAudioStream* pAudioStream =
					pDoc->m_pAVIPlay->GetAudioStream(pDoc->m_nActiveAudioStream);
			if (pAudioStream)
			{
				pDoc->m_pPlayerToolBarDlg->m_VideoAviToolBar.m_PlayerSlider.SetPos(	Round((double)pDoc->m_PlayAudioFileThread.GetCurrentSamplePos() /
																					(double)(pDoc->m_PlayAudioFileThread.GetTotalSamples() - 1) *
																					(double)(pDoc->m_pPlayerToolBarDlg->m_VideoAviToolBar.m_PlayerSlider.GetRangeMax())));
			}
		}
	}
}

LONG CVideoAviView::OnEnableCursor(WPARAM wparam, LPARAM lparam)
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	BOOL bEnable = (BOOL)wparam;

	if (bEnable)
	{
		if (!m_bCursor)
		{
			::ShowCursor(TRUE);
			m_bCursor = TRUE;
			if ((pDoc->m_PlayVideoFileThread.IsAlive() &&	// Is Waiting Audio to finish
				pDoc->m_PlayVideoFileThread.IsWaitingForStart())
				||
				!pDoc->m_PlayVideoFileThread.IsAlive())		// Is Not Playing
				UpdateWindowSizes(TRUE, FALSE, FALSE);
		}
	}
	else
	{
		if (m_bCursor)
		{
			::ShowCursor(FALSE);
			m_bCursor = FALSE;
			if ((pDoc->m_PlayVideoFileThread.IsAlive() &&	// Is Waiting Audio to finish
				pDoc->m_PlayVideoFileThread.IsWaitingForStart())
				||
				!pDoc->m_PlayVideoFileThread.IsAlive())		// Is Not Playing
				UpdateWindowSizes(TRUE, FALSE, FALSE);
		}
	}

	return 0;
}

LONG CVideoAviView::OnEndThumbTrack(WPARAM wparam, LPARAM lparam)
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// End Poll?
	if ((--pDoc->m_nEndThumbTrackRetryCountDown <= 0)	||
		(m_dwThumbTrackSeq != (DWORD)wparam))
		return 1;

	// Repost?
	if (pDoc->m_PlayVideoFileThread.IsAlive() ||
		pDoc->m_PlayAudioFileThread.IsAlive())
	{
		CPostDelayedMessageThread::PostDelayedMessage(	GetSafeHwnd(),
														WM_END_THUMBTRACK,	
														END_TRUMBTRACK_RETRY_DELAY,
														wparam,
														lparam);
		TRACE(_T("Post Delayed: OnEndThumbTrack()\n"));
	}
	else
	{
		// Set Pos
		CAVIPlay::CAVIVideoStream* pVideoStream = pDoc->m_pAVIPlay->GetVideoStream(pDoc->m_nActiveVideoStream);
		if (pVideoStream)
			pVideoStream->SetCurrentFramePos(m_nThumbTrackPos);
		else if (pDoc->m_pAVIPlay->HasAudio() && (pDoc->m_nActiveAudioStream >= 0))
			pDoc->m_PlayAudioFileThread.SetCurrentSamplePos((LONGLONG)m_nThumbTrackPos);

		// Restart Playing
		if (m_bWasPlayingBeforeThumbTrack)
		{
			m_bWasPlayingBeforeThumbTrack = FALSE;
			
			// At The End?
			BOOL bLastPos = TRUE;
			if (pVideoStream)
				bLastPos = m_nThumbTrackPos >= ((int)pVideoStream->GetTotalFrames() - 1);
			else if (pDoc->m_pAVIPlay->HasAudio() && (pDoc->m_nActiveAudioStream >= 0))
				bLastPos = m_nThumbTrackPos >= ((int)pDoc->m_PlayAudioFileThread.GetTotalSamples() - 1);
			
			// Play AVI
			if (pDoc->IsLoop() || !bLastPos)
				pDoc->PlayAVI();
		}
		else if (pVideoStream)
		{
			pDoc->DisplayFrame(m_nThumbTrackPos);
			pDoc->m_PlayAudioFileThread.PlaySyncAudioFromVideo();
		}

		// Set Done Flag
		m_bThumbTrackDone = TRUE;
	}
	
	return 1;
}

BOOL CVideoAviView::UpdateCurrentFrame(CAVIPlay::CAVIVideoStream* pVideoStream)
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	::EnterCriticalSection(&pDoc->m_csDib);
	if (pVideoStream->GetCurrentFramePos() == 0)
	{
		if (pVideoStream->GetFrameAt(pDoc->m_pDib, 0))
		{			
			::LeaveCriticalSection(&pDoc->m_csDib);
			return TRUE;
		}
		else
		{
			::LeaveCriticalSection(&pDoc->m_csDib);
			return FALSE;
		}
	}
	else
	{
		// Note: Some codecs are not working if we do not first
		//       feed them with the starting frame!
		int nCurrentFramePos = pVideoStream->GetCurrentFramePos();
		if (pVideoStream->GetFrameAt(pDoc->m_pDib, 0) &&
			pVideoStream->GetFrameAt(pDoc->m_pDib, nCurrentFramePos))
		{
			::LeaveCriticalSection(&pDoc->m_csDib);
			return TRUE;
		}
		else
		{
			::LeaveCriticalSection(&pDoc->m_csDib);
			return FALSE;
		}
	}
}

void CVideoAviView::OnViewFit() 
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	if (m_bFullScreenMode)
	{
		// Reset User Zoom Rect
		m_UserZoomRect = m_ZoomRect;

		// Invalidate View
		pDoc->InvalidateView();
	}
	else
	{
		if (pDoc->GetFrame()->IsZoomed())
		{
			// Restore mainframe if maximized
			if (::AfxGetMainFrame()->IsZoomed())
				::AfxGetMainFrame()->ShowWindow(SW_RESTORE);

			// First resize in x direction,
			// because the menu may get taller,
			// then resize in y direction!
			CRect rcClient;
			GetClientRect(&rcClient);
			int delta = pDoc->m_DocRect.Width() - rcClient.Width();
			CRect rcMainWnd;
			::AfxGetMainFrame()->GetWindowRect(&rcMainWnd);
			::AfxGetMainFrame()->SetWindowPos(	NULL, 0, 0,
												rcMainWnd.Width() + delta,
												rcMainWnd.Height(),
												SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOMOVE);
			GetClientRect(&rcClient);
			::AfxGetMainFrame()->GetWindowRect(&rcMainWnd);
			delta = pDoc->m_DocRect.Height() - rcClient.Height();
			::AfxGetMainFrame()->SetWindowPos(	NULL, 0, 0,
												rcMainWnd.Width(),
												rcMainWnd.Height() + delta,
												SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOMOVE);

		}
		else
			UpdateWindowSizes(FALSE, FALSE, TRUE);
	}
}

void CVideoAviView::OnUpdateViewFit(CCmdUI* pCmdUI) 
{
	if (m_bFullScreenMode)
		pCmdUI->Enable(m_UserZoomRect != m_ZoomRect);
	else
	{
		CVideoAviDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);

		CRect rcClient;
		GetClientRect(&rcClient);
		pCmdUI->Enable(	(rcClient != pDoc->m_DocRect)	&&
						!pDoc->GetFrame()->IsIconic());
	}
}

void CVideoAviView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Pop-Up Menu
	CMenu menu;
	CMenu* pPopup = NULL;
	CPoint point;
	point.x = 6;
	point.y = 6;

	switch (nChar)
	{
		case VK_ESCAPE :
			if (((CUImagerApp*)::AfxGetApp())->m_bEscExit)
				::AfxGetMainFrame()->PostMessage(WM_CLOSE, 0, 0);
			else
			{
				if (pDoc->m_pAviInfoDlg			||
					pDoc->m_pOutVolDlg			||
					pDoc->m_pAudioVideoShiftDlg	||
					pDoc->m_pPlayerToolBarDlg)
				{
					if (pDoc->m_pAviInfoDlg)
						pDoc->m_pAviInfoDlg->Close();
					if (pDoc->m_pOutVolDlg)
						pDoc->m_pOutVolDlg->Close();
					if (pDoc->m_pAudioVideoShiftDlg)
						pDoc->m_pAudioVideoShiftDlg->Close();
					if (pDoc->m_pPlayerToolBarDlg)
						pDoc->m_pPlayerToolBarDlg->Close();
				}
				else if (m_bFullScreenMode)
					::AfxGetMainFrame()->EnterExitFullscreen();	// Exit Full-Screen Mode
				else if (pDoc->m_ProcessingThread.IsRunning())
					pDoc->m_ProcessingThread.Kill_NoBlocking();
				else
					pDoc->CloseDocument();
			}
			break;

		case VK_RIGHT :	// Right Arrow
			if (!pDoc->IsProcessing())
				pDoc->FrameFrontFast();
			break;

		case VK_LEFT :	// Left Arrow
			if (!pDoc->IsProcessing())
				pDoc->FrameBackFast();
			break;
		
		case VK_DOWN :	// Down Arrow
		case VK_NEXT :	// Page Down
			if (!pDoc->IsProcessing())
				pDoc->FrameFront();
			break;

		case VK_UP :	// Up Arrow
		case VK_PRIOR : // Page Up
			if (!pDoc->IsProcessing())
				pDoc->FrameBack();
			break;

		case VK_PAUSE : // Pause
			if (!pDoc->IsProcessing())
				pDoc->StopAVI();
			break;

		case VK_RETURN : // Enter
		case VK_SPACE :
			if (!pDoc->IsProcessing())
			{
				if (!pDoc->m_PlayVideoFileThread.IsAlive() &&
					!pDoc->m_PlayAudioFileThread.IsAlive())
					pDoc->PlayAVI();
				else
					pDoc->StopAVI();
			}
			break;

		case VK_ADD :
			if (pDoc->m_pAVIPlay->HasVideo()	&&
				pDoc->m_nActiveVideoStream >= 0	&&
				pDoc->m_PlayVideoFileThread.IsAlive())
				ChangePlaySpeed(TRUE);
			break;

		case VK_SUBTRACT :
			if (pDoc->m_pAVIPlay->HasVideo()	&&
				pDoc->m_nActiveVideoStream >= 0	&&
				pDoc->m_PlayVideoFileThread.IsAlive())
				ChangePlaySpeed(FALSE);
			break;

		case VK_HOME :
			if (!pDoc->IsProcessing())
				pDoc->JumpToFirstFrame();
			break;

		case VK_END :
			if (!pDoc->IsProcessing())
				pDoc->JumpToLastFrame();
			break;

		case VK_INSERT :
			if (pDoc->m_pAVIPlay						&&
				pDoc->m_pAVIPlay->HasVideo()			&&
				(pDoc->m_nActiveVideoStream >= 0)		&&
				!pDoc->IsProcessing()					&&
				!pDoc->m_PlayVideoFileThread.IsAlive()	&&
				!pDoc->m_PlayAudioFileThread.IsAlive())
				pDoc->EditSnapshot();
			break;

		case VK_DELETE : // Delete Doc
			if (!pDoc->IsModified()						&&
				!pDoc->IsProcessing()					&&
				!m_bFullScreenMode						&&
				!pDoc->m_PlayVideoFileThread.IsAlive()	&&
				!pDoc->m_PlayAudioFileThread.IsAlive())
			{
				if (::GetKeyState(VK_CONTROL) < 0)
					pDoc->EditDelete(FALSE);// Delete without prompting
				else
					pDoc->EditDelete(TRUE);	// Delete with prompting
			}
			break;

		case VK_APPS :
			ForceCursor();
			VERIFY(menu.LoadMenu(IDR_CONTEXT_VIDEO_AVI));
			pPopup = menu.GetSubMenu(0);
			ASSERT(pPopup != NULL);
			SetForegroundWindow();
			ClientToScreen(&point);
			if (m_bFullScreenMode)
				pDoc->PlayerToolBarDlg(point);
			else
				pPopup->TrackPopupMenu(TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, ::AfxGetMainFrame());
			ForceCursor(FALSE);
			break;
	
		default : break;
	}
	
	CUImagerView::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CVideoAviView::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

void CVideoAviView::EraseBkgnd(HDC hDC)
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Client Rect
	CRect rcClient;
	GetClientRect(rcClient);

	// Zoom Rect
	CRect ZoomRect;
	if ((m_UserZoomRect.Width() != 0) &&
		(m_UserZoomRect.Height() != 0) &&
		m_bFullScreenMode)
	{
		ZoomRect = m_UserZoomRect;
		if (ZoomRect.left < 0)
			ZoomRect.left = 0;
		if (ZoomRect.top < 0)
			ZoomRect.top = 0;
		if (ZoomRect.right > rcClient.Width())
			ZoomRect.right = rcClient.Width();
		if (ZoomRect.bottom > rcClient.Height())
			ZoomRect.bottom = rcClient.Height();
	}
	else
		ZoomRect = m_ZoomRect;

	// Erase Bkg
	CRect rcTop(rcClient.left, rcClient.top, rcClient.right, ZoomRect.top);
	CRect rcLeft(rcClient.left, ZoomRect.top, ZoomRect.left, ZoomRect.bottom);
	CRect rcRight = CRect(ZoomRect.right, ZoomRect.top, rcClient.right, ZoomRect.bottom);
	CRect rcBottom = CRect(rcClient.left, ZoomRect.bottom, rcClient.right, rcClient.bottom);

	// Has video?
	BOOL bHasVideo =	pDoc->m_pAVIPlay				&&
						pDoc->m_pAVIPlay->HasVideo()	&&
						pDoc->m_nActiveVideoStream >= 0;

	CBrush br;
	br.CreateSolidBrush(pDoc->m_crBackgroundColor);

	// If Audio or Processing
	if (!bHasVideo || pDoc->IsProcessing())
		::FillRect(hDC, &rcClient, (HBRUSH)br);
	else
	{
		if ((rcTop.Width() > 0) && (rcTop.Height() > 0))
			::FillRect(hDC, &rcTop, (HBRUSH)br);
		if ((rcLeft.Width() > 0) && (rcLeft.Height() > 0))
			::FillRect(hDC, &rcLeft, (HBRUSH)br);
		if ((rcRight.Width() > 0) && (rcRight.Height() > 0))
			::FillRect(hDC, &rcRight, (HBRUSH)br);
		if ((rcBottom.Width() > 0) && (rcBottom.Height() > 0))
			::FillRect(hDC, &rcBottom, (HBRUSH)br);
	}

	br.DeleteObject();
}

BOOL CVideoAviView::MakeFont(BOOL bBig)
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (bBig)	
	{
		// Create Big Font
		LOGFONT lf;
		memset(&lf, 0, sizeof(LOGFONT));
		_tcscpy(lf.lfFaceName, DEFAULT_FONTFACE_NARROW);
		HDC hDC = ::GetDC(GetSafeHwnd());
		lf.lfHeight = -MulDiv(36, GetDeviceCaps(hDC, LOGPIXELSY), 72);
		::ReleaseDC(GetSafeHwnd(), hDC);
		lf.lfWeight = FW_LIGHT;
		lf.lfItalic = 0;
		lf.lfUnderline = 0;
		if (m_hFont)
			::DeleteObject(m_hFont);
		m_hFont = ::CreateFontIndirect(&lf);
		m_bBigFont = TRUE;
		return (m_hFont != NULL);
	}
	else
	{
		// Create Small Font
		LOGFONT lf;
		memset(&lf, 0, sizeof(LOGFONT));
		_tcscpy(lf.lfFaceName, DEFAULT_FONTFACE);
		HDC hDC = ::GetDC(GetSafeHwnd());
		lf.lfHeight = -MulDiv(11, GetDeviceCaps(hDC, LOGPIXELSY), 72);
		::ReleaseDC(GetSafeHwnd(), hDC);
		lf.lfWeight = FW_NORMAL;
		lf.lfItalic = 0;
		lf.lfUnderline = 0;
		if (m_hFont)
			::DeleteObject(m_hFont);
		m_hFont = ::CreateFontIndirect(&lf);
		m_bBigFont = FALSE;
		return (m_hFont != NULL);
	}
}

void CVideoAviView::OnDraw(CDC* pDC) 
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Flicker free drawing
	CRect rcClient;
	GetClientRect(&rcClient);
	CMyMemDC MemDC(pDC, &rcClient);

	// Draw
	Draw(MemDC.GetSafeHdc());
}

void CVideoAviView::Draw(HDC hDC) 
{
	// Vars
	RECT Rect;
	COLORREF OldTextColor;
	int OldBkMode;
	HFONT hOldFont;
	TCHAR sText[MAX_STATISTICS_TEXT_SIZE];
	sText[0] = _T('\0');
	RECT rcClient;
	RECT ZoomRect;
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Client Rect
	GetClientRect(&rcClient);

	// Has video?
	BOOL bHasVideo =	pDoc->m_pAVIPlay				&&
						pDoc->m_pAVIPlay->HasVideo()	&&
						pDoc->m_nActiveVideoStream >= 0;

	// Draw Text?
	BOOL bDrawText = pDoc->m_bTimePositionShow;	
	if (bHasVideo)
	{
		if (!pDoc->m_PlayVideoFileThread.IsAlive()		||
			(m_bFullScreenMode && IsCursorEnabled())	||
			(pDoc->m_PlayVideoFileThread.GetPlaySpeedPercent() != 100))
			bDrawText = TRUE;
	}
	else
		bDrawText = TRUE;

	// Zoom Rect
	if (m_bFullScreenMode)
		ZoomRect = m_UserZoomRect;
	else
		ZoomRect = m_ZoomRect;

	// Enter CS
	::EnterCriticalSection(&pDoc->m_csDib);

	// EraseBkgnd
	EraseBkgnd(hDC);

	// Processing Progress
	if (pDoc->IsProcessing())
	{
		// Init Vars
		int nCurrentLine = 0;
		int nGDITextHeight = 0;

		// Leave CS
		::LeaveCriticalSection(&pDoc->m_csDib);

		// Init Font and Text Mode
		if (m_bBigFont)
			MakeFont(FALSE);
		OldTextColor = ::SetTextColor(hDC, RGB(0x0,0xff,0x0));
		OldBkMode = ::SetBkMode(hDC, TRANSPARENT);
		hOldFont = (HFONT)::SelectObject(hDC, m_hFont);

		// Video Percent Done
		if (pDoc->m_nVideoPercentDone >= 0)
		{
			_stprintf(sText, ML_STRING(1450, "Video Processing Progress: %i%%"), pDoc->m_nVideoPercentDone);
			Rect = rcClient;
			nGDITextHeight = ::DrawText(hDC, sText, -1, &Rect,
							(DT_LEFT | DT_TOP | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
			nCurrentLine++;
		}

		// Audio Percent Done
		if (pDoc->m_nAudioPercentDone >= 0)
		{	
			_stprintf(sText, ML_STRING(1451, "Audio Processing Progress: %i%%"), pDoc->m_nAudioPercentDone);
			Rect.left = rcClient.left;
			Rect.top = rcClient.top + nCurrentLine * nGDITextHeight;
			Rect.right = rcClient.left;
			Rect.bottom = rcClient.top + nCurrentLine * nGDITextHeight;
			nGDITextHeight = ::DrawText(hDC, sText, -1, &Rect,
							(DT_LEFT | DT_TOP | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
			nCurrentLine++;
		}

		// Processing Error Message
		if (pDoc->m_sProcessingError != _T(""))
		{
			Rect.left = rcClient.left;
			Rect.top = rcClient.top + nCurrentLine * nGDITextHeight;
			Rect.right = rcClient.left;
			Rect.bottom = rcClient.top + nCurrentLine * nGDITextHeight;
			nGDITextHeight = ::DrawText(hDC, pDoc->m_sProcessingError, -1, &Rect,
							(DT_LEFT | DT_TOP | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
			nCurrentLine++;
		}

		// Clean-Up
		::SetBkMode(hDC, OldBkMode);
		::SetTextColor(hDC, OldTextColor);
		::SelectObject(hDC, hOldFont);
	}
	// Frame Display
	else
	{
		// Make Font
		if (m_bFullScreenMode && !m_bBigFont)
			MakeFont(TRUE);
		else if (!m_bFullScreenMode && m_bBigFont)
			MakeFont(FALSE);

		// Paint
		if (pDoc->m_pDib)
		{
			// Valid Frame?
			if (pDoc->m_pDib->IsValid())
			{
				// Paint
				pDoc->m_pDib->Paint(hDC,
									&ZoomRect,
									pDoc->m_DocRect,
									FALSE);	// Force Stretch?

				// Leave CS
				::LeaveCriticalSection(&pDoc->m_csDib);

				// Update Slider?
				if (m_bThumbTrackDone						&&
					((pDoc->m_PlayVideoFileThread.IsAlive()	&&		// Is Waiting Audio to finish
					pDoc->m_PlayVideoFileThread.IsWaitingForStart())
															||
					!pDoc->m_PlayVideoFileThread.IsAlive()	||		// Is Not Playing
					((pDoc->m_dwFrameCountUp %
					MAX(1, Round(pDoc->GetPlayFrameRate()))) == 0)))// Update Slider Every Second if playing
				{
					UpdatePlaySlider();
				}
			}
			// Empty Frame
			else
			{
				// Leave CS
				::LeaveCriticalSection(&pDoc->m_csDib);

				// Fill Black
				CBrush br;
				br.CreateSolidBrush(RGB(0,0,0));
				::FillRect(hDC, &ZoomRect, (HBRUSH)br);
				br.DeleteObject();

				// Update Slider?
				if (m_bThumbTrackDone						&&
					((pDoc->m_PlayVideoFileThread.IsAlive()	&&		// Is Waiting Audio to finish
					pDoc->m_PlayVideoFileThread.IsWaitingForStart())
															||
					!pDoc->m_PlayVideoFileThread.IsAlive()	||		// Is Not Playing
					((pDoc->m_dwFrameCountUp %
					MAX(1, Round(pDoc->GetPlayFrameRate()))) == 0)))// Update Slider Every Second if playing
				{
					UpdatePlaySlider();
				}
			}
		}
		else
			::LeaveCriticalSection(&pDoc->m_csDib); // Leave CS

		// Info Text Show
		if (pDoc->m_pAVIPlay && bDrawText)
		{
			DrawInfo(	hDC,
						rcClient,
						ZoomRect);
		}
	}
}

__forceinline void CVideoAviView::DrawInfo(	HDC hDC,
											RECT& rcClient,
											RECT& ZoomRect)
{
	RECT Rect;
	COLORREF OldTextColor;
	int OldBkMode;
	COLORREF crOldBkColor;
	HFONT hOldFont;
	TCHAR sText[MAX_STATISTICS_TEXT_SIZE];
	sText[0] = _T('\0');
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Has video?
	BOOL bHasVideo = pDoc->m_pAVIPlay->HasVideo() && pDoc->m_nActiveVideoStream >= 0;

	// Set Font & Color
	OldTextColor = ::SetTextColor(hDC, RGB(0x0,0xFF,0x0));
	OldBkMode = ::SetBkMode(hDC, OPAQUE);
	crOldBkColor = ::SetBkColor(hDC, RGB(0,0,0));
	hOldFont = (HFONT)::SelectObject(hDC, m_hFont);

	// Avg Milliseconds Correction, Timer Delay and Dropped Frames Count Display
	if (((CUImagerApp*)::AfxGetApp())->m_bVideoAviInfo && bHasVideo)
	{
		_stprintf(sText, _T("Corr % 3i,Timer % 2u,Drop % 1u"),	
												pDoc->m_PlayVideoFileThread.GetMilliSecondsCorrectionAvg(),
												pDoc->m_PlayVideoFileThread.GetTimerDelay(),
												pDoc->m_PlayVideoFileThread.GetDroppedFramesCount());
		if (m_bFullScreenMode)
			Rect = rcClient;
		else
			Rect = ZoomRect;
		::DrawText(hDC, sText, -1, &Rect, (DT_LEFT | DT_BOTTOM | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
	}
	
	// Time Position Text
	if (bHasVideo)
	{
		// Get Video Stream
		CAVIPlay::CAVIVideoStream* pVideoStream = pDoc->m_pAVIPlay->GetVideoStream(pDoc->m_nActiveVideoStream);
		if (pVideoStream)
		{
			int nFramePos; 
			if (m_bThumbTrackDone)
				nFramePos = pVideoStream->GetCurrentFramePos();
			else
				nFramePos = m_nThumbTrackPos;
			bool bKeyFrame	= pVideoStream->IsKeyFrame(nFramePos);
			FramePosText(	sText,
							nFramePos,
							pVideoStream->GetFrameRate(),
							bKeyFrame,
							pDoc->m_PlayAudioFileThread.GetCurrentSamplePos());
		}	
	}
	else
	{
		LONGLONG llSamplePos; 
		if (m_bThumbTrackDone)
			llSamplePos = pDoc->m_PlayAudioFileThread.GetCurrentSamplePos();
		else
			llSamplePos = m_nThumbTrackPos;
		SamplePosText(	sText,
						llSamplePos,
						pDoc->m_PlayAudioFileThread.GetSampleRate());
	}
	if (bHasVideo)
	{
		if (m_bFullScreenMode)
			Rect = rcClient;
		else
			Rect = ZoomRect;
		::DrawText(hDC, sText, -1, &Rect, (DT_RIGHT | DT_TOP | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
	}
	else
	{
		Rect = rcClient;
		::DrawText(hDC, sText, -1, &Rect, (DT_RIGHT | DT_TOP | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
	}

	// Playback Speed
	if (bHasVideo								&&
		pDoc->m_PlayVideoFileThread.IsAlive()	&&
		pDoc->m_PlayVideoFileThread.GetPlaySpeedPercent() != 100)
	{
		_stprintf(sText, _T("%4.1fX"),
						pDoc->m_PlayVideoFileThread.GetPlaySpeedPercent() / 100.0);
		if (m_bFullScreenMode)
			Rect = rcClient;
		else
			Rect = ZoomRect;
		::DrawText(hDC, sText, -1, &Rect, (DT_RIGHT | DT_BOTTOM | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
	}

	// Clean-Up
	::SetBkMode(hDC, OldBkMode);
	::SetBkColor(hDC, crOldBkColor);
	::SetTextColor(hDC, OldTextColor);
	::SelectObject(hDC, hOldFont);
}

__forceinline void CVideoAviView::FramePosText(	TCHAR* sText,
												int nFramePos,
												double dFrameRate,
												bool bKeyFrame,
												LONGLONG llSamplePos)
{
	double dPos		= (double)nFramePos / dFrameRate;				// Position in Seconds
	int nPosHour	= (int)(dPos / 3600.0);							// Hours Part
	int nPosMin		= (int)((dPos - nPosHour * 3600.0) / 60.0);		// Minutes Part
	double dPosSec	= dPos - nPosHour * 3600.0 - nPosMin * 60.0;	// Seconds Part
#ifdef _DEBUG
	_stprintf(sText, _T("%02d:%02d:%06.3f%s"), nPosHour, nPosMin, dPosSec, bKeyFrame ? _T("(K)") : _T("(D)"));
#else
	_stprintf(sText, _T("%02d:%02d:%06.3f"), nPosHour, nPosMin, dPosSec);
#endif
}

__forceinline void CVideoAviView::SamplePosText(TCHAR* sText,
												LONGLONG llSamplePos,
												DWORD dwSampleRate)
{
	if (llSamplePos < 0)
		llSamplePos = 0;
	double dPos		= (double)llSamplePos / (double)dwSampleRate;	// Position in Seconds								
	int nPosHour	= (int)(dPos / 3600.0);							// Hours Part
	int nPosMin		= (int)((dPos - nPosHour * 3600.0) / 60.0);		// Minutes Part
	double dPosSec	= dPos - nPosHour * 3600.0 - nPosMin * 60.0;	// Seconds Part
#ifdef _DEBUG
	_stprintf(sText, _T("%02d:%02d:%06.3f (%I64d)"),nPosHour, nPosMin, dPosSec, llSamplePos);
#else
	_stprintf(sText, _T("%02d:%02d:%06.3f"),nPosHour, nPosMin, dPosSec);
#endif
}

void CVideoAviView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pScrollBar && !pDoc->IsProcessing())
	{
		if ((SB_THUMBTRACK == nSBCode) ||	// Dragging Slider
			(SB_THUMBPOSITION == nSBCode))	// Wheel On Mouse And End Of Dragging Slider
		{
			// Get video stream
			CAVIPlay::CAVIVideoStream* pVideoStream = pDoc->m_pAVIPlay->GetVideoStream(pDoc->m_nActiveVideoStream);

			// Init Thumbtracking?
			if (!m_bThumbTrack && (SB_THUMBTRACK == nSBCode))
			{
				// Inc. Seq.
				m_dwThumbTrackSeq++;

				// Reset Done Flag
				m_bThumbTrackDone = FALSE;

				// Set Thumb Tracking Flag
				m_bThumbTrack = TRUE;

				// Stop Playing?
				if (!m_bWasPlayingBeforeThumbTrack			&&
					(pDoc->m_PlayVideoFileThread.IsAlive()	||
					pDoc->m_PlayAudioFileThread.IsAlive()))
				{
					pDoc->StopAVI();
					m_bWasPlayingBeforeThumbTrack = TRUE;
				}

				// Init frame preview timer
				if (pVideoStream)
					SetTimer(ID_TIMER_AVIPLAYSLIDER, AVIPLAYSLIDER_TIMER_MS, NULL);
			}

			// End Thumbtracking?
			if (m_bThumbTrack && (SB_THUMBPOSITION == nSBCode))
			{
				// Reset Thumb Tracking Flag
				m_bThumbTrack = FALSE;

				// Kill timer
				if (pVideoStream)
					KillTimer(ID_TIMER_AVIPLAYSLIDER);

				// End Thumb Tracking
				pDoc->EndThumbTrack();

				return;
			}

			// Slider Pointer
			CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;
			
			// Update thumb track pos
			if (pVideoStream)
			{
				m_nThumbTrackPos = Round((double)pSlider->GetPos() /
										(double)pSlider->GetRangeMax() *
										(double)((int)pVideoStream->GetTotalFrames() - 1));
			}
			else
			{
				m_nThumbTrackPos =	Round((double)pSlider->GetPos() /
										(double)pSlider->GetRangeMax() *
										(double)(pDoc->m_PlayAudioFileThread.GetTotalSamples() - 1));
			}

			// Update Window
			UpdateWindowSizes(TRUE, FALSE, FALSE);
		}
	}
}

void CVideoAviView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	UpdateWindowSizes(TRUE, FALSE, FALSE);
}

void CVideoAviView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Reset Status Text
	::AfxGetMainFrame()->StatusText();	
	CUImagerView::OnMouseMove(nFlags, point);

	if (m_bFullScreenMode)
	{
		int nRectSizeXInside = USER_RECT_X_INSIDE;
		int nRectSizeYInside = USER_RECT_Y_INSIDE;
		int nRectSizeXOutside = USER_RECT_X_OUTSIDE;
		int nRectSizeYOutside = USER_RECT_Y_OUTSIDE;

		int excess;
		int dx, dy;
		CRect rcClient;
		GetClientRect(rcClient);
		CRect UserZoomClippedRect = m_UserZoomRect;
		if (UserZoomClippedRect.left < 0)
			UserZoomClippedRect.left = 0;
		if (UserZoomClippedRect.right > rcClient.right)
			UserZoomClippedRect.right = rcClient.right;
		if (UserZoomClippedRect.top < 0)
			UserZoomClippedRect.top = 0;
		if (UserZoomClippedRect.bottom > rcClient.bottom)
			UserZoomClippedRect.bottom = rcClient.bottom;

		CRect rcTop(	UserZoomClippedRect.left + nRectSizeXInside,
						UserZoomClippedRect.top - nRectSizeYOutside,
						UserZoomClippedRect.right - nRectSizeXInside,
						UserZoomClippedRect.top + nRectSizeYInside);

		CRect rcBottom(	UserZoomClippedRect.left + nRectSizeXInside,
						UserZoomClippedRect.bottom - nRectSizeYInside,
						UserZoomClippedRect.right - nRectSizeXInside,
						UserZoomClippedRect.bottom + nRectSizeYOutside);

		CRect rcLeft(	UserZoomClippedRect.left - nRectSizeXOutside,
						UserZoomClippedRect.top + nRectSizeYInside,
						UserZoomClippedRect.left + nRectSizeXInside,
						UserZoomClippedRect.bottom - nRectSizeYInside);

		CRect rcRight(	UserZoomClippedRect.right - nRectSizeXInside,
						UserZoomClippedRect.top + nRectSizeYInside,
						UserZoomClippedRect.right + nRectSizeXOutside,
						UserZoomClippedRect.bottom - nRectSizeYInside);

		CRect rcTopLeft(UserZoomClippedRect.left - nRectSizeXOutside,
						UserZoomClippedRect.top - nRectSizeYOutside,
						UserZoomClippedRect.left + nRectSizeXInside,
						UserZoomClippedRect.top + nRectSizeYInside);

		CRect rcBottomLeft(	UserZoomClippedRect.left - nRectSizeXOutside,
							UserZoomClippedRect.bottom - nRectSizeYInside,
							UserZoomClippedRect.left + nRectSizeXInside,
							UserZoomClippedRect.bottom + nRectSizeYOutside);

		CRect rcTopRight(	UserZoomClippedRect.right - nRectSizeXInside,
							UserZoomClippedRect.top - nRectSizeYOutside,
							UserZoomClippedRect.right + nRectSizeXOutside,
							UserZoomClippedRect.top + nRectSizeYInside);

		CRect rcBottomRight(UserZoomClippedRect.right - nRectSizeXInside,
							UserZoomClippedRect.bottom - nRectSizeYInside,
							UserZoomClippedRect.right + nRectSizeXOutside,
							UserZoomClippedRect.bottom + nRectSizeYOutside);

		if (rcTopLeft.PtInRect(point))
			m_hResizeCursor = ::LoadCursor(NULL, IDC_SIZENWSE);
		else if (rcBottomRight.PtInRect(point))
			m_hResizeCursor = ::LoadCursor(NULL, IDC_SIZENWSE);
		else if (rcBottomLeft.PtInRect(point))
			m_hResizeCursor = ::LoadCursor(NULL, IDC_SIZENESW);
		else if (rcTopRight.PtInRect(point))
			m_hResizeCursor = ::LoadCursor(NULL, IDC_SIZENESW);
		else if (rcTop.PtInRect(point))
			m_hResizeCursor = ::LoadCursor(NULL, IDC_SIZENS);
		else if (rcBottom.PtInRect(point))
			m_hResizeCursor = ::LoadCursor(NULL, IDC_SIZENS);
		else if (rcLeft.PtInRect(point))
			m_hResizeCursor = ::LoadCursor(NULL, IDC_SIZEWE);
		else if (rcRight.PtInRect(point))
			m_hResizeCursor = ::LoadCursor(NULL, IDC_SIZEWE);
		else
			m_hResizeCursor = NULL;

		if (nFlags & MK_LBUTTON)
		{
			BOOL bSymmetrical =	!(nFlags & MK_SHIFT) &&
								!(nFlags & MK_CONTROL);

			CRect UserZoomRect = m_UserZoomRect;
			
			if (m_bUserTopLeft)
			{
				// Clipping?
				if (m_bUserClickClipping)
				{
					UserZoomRect.left = m_UserClickZoomRect.left + (point.x - m_ptUserClick.x);
					UserZoomRect.top = m_UserClickZoomRect.top + (point.y - m_ptUserClick.y);
				}
				else
				{
					UserZoomRect.left = point.x;
					UserZoomRect.top = point.y;
				}

				// Update Also Bottom-Right?
				if (bSymmetrical)
				{
					dx = rcClient.left - UserZoomRect.left;
					UserZoomRect.right = rcClient.right + dx;
					dy = rcClient.top - UserZoomRect.top;
					UserZoomRect.bottom = rcClient.bottom + dy;
				}

				// Min Size
				if (UserZoomRect.Width() < USER_ZOOM_MIN_WIDTH)
				{
					if (bSymmetrical)
					{
						excess = (USER_ZOOM_MIN_WIDTH - UserZoomRect.Width()) / 2;
						UserZoomRect.left -= excess;
						UserZoomRect.right += excess;
					}
					else
						UserZoomRect.left = UserZoomRect.right - USER_ZOOM_MIN_WIDTH;
				}
				if (UserZoomRect.Height() < USER_ZOOM_MIN_HEIGHT)
				{
					if (bSymmetrical)
					{
						excess = (USER_ZOOM_MIN_HEIGHT - UserZoomRect.Height()) / 2;
						UserZoomRect.top -= excess;
						UserZoomRect.bottom += excess;
					}
					else
						UserZoomRect.top = UserZoomRect.bottom - USER_ZOOM_MIN_HEIGHT;
				}
			}
			else if (m_bUserBottomRight)
			{
				// Clipping?
				if (m_bUserClickClipping)
				{
					UserZoomRect.bottom = m_UserClickZoomRect.bottom + (point.y - m_ptUserClick.y);
					UserZoomRect.right = m_UserClickZoomRect.right + (point.x - m_ptUserClick.x);
				}
				else
				{
					UserZoomRect.bottom = point.y + 1;
					UserZoomRect.right = point.x + 1;
				}

				// Update Also Top-Left?
				if (bSymmetrical)
				{
					dx = rcClient.right - UserZoomRect.right;
					UserZoomRect.left = rcClient.left + dx;
					dy = rcClient.bottom - UserZoomRect.bottom;
					UserZoomRect.top = rcClient.top + dy;
				}

				// Min Size
				if (UserZoomRect.Width() < USER_ZOOM_MIN_WIDTH)
				{
					if (bSymmetrical)
					{
						excess = (USER_ZOOM_MIN_WIDTH - UserZoomRect.Width()) / 2;
						UserZoomRect.left -= excess;
						UserZoomRect.right += excess;
					}
					else
						UserZoomRect.right = UserZoomRect.left + USER_ZOOM_MIN_WIDTH;
				}
				if (UserZoomRect.Height() < USER_ZOOM_MIN_HEIGHT)
				{
					if (bSymmetrical)
					{
						excess = (USER_ZOOM_MIN_HEIGHT - UserZoomRect.Height()) / 2;
						UserZoomRect.top -= excess;
						UserZoomRect.bottom += excess;
					}
					else
						UserZoomRect.bottom = UserZoomRect.top + USER_ZOOM_MIN_HEIGHT;
				}
			}
			else if (m_bUserBottomLeft)
			{
				// Clipping?
				if (m_bUserClickClipping)
				{
					UserZoomRect.bottom = m_UserClickZoomRect.bottom + (point.y - m_ptUserClick.y);
					UserZoomRect.left = m_UserClickZoomRect.left + (point.x - m_ptUserClick.x);
				}
				else
				{
					UserZoomRect.bottom = point.y + 1;
					UserZoomRect.left = point.x;
				}

				// Update Also Top-Right?
				if (bSymmetrical)
				{
					dx = rcClient.left - UserZoomRect.left;
					UserZoomRect.right = rcClient.right + dx;
					dy = rcClient.bottom - UserZoomRect.bottom;
					UserZoomRect.top = rcClient.top + dy;
				}	

				// Min Size
				if (UserZoomRect.Width() < USER_ZOOM_MIN_WIDTH)
				{
					if (bSymmetrical)
					{
						excess = (USER_ZOOM_MIN_WIDTH - UserZoomRect.Width()) / 2;
						UserZoomRect.left -= excess;
						UserZoomRect.right += excess;
					}
					else
						UserZoomRect.left = UserZoomRect.right - USER_ZOOM_MIN_WIDTH;
				}
				if (UserZoomRect.Height() < USER_ZOOM_MIN_HEIGHT)
				{
					if (bSymmetrical)
					{
						excess = (USER_ZOOM_MIN_HEIGHT - UserZoomRect.Height()) / 2;
						UserZoomRect.top -= excess;
						UserZoomRect.bottom += excess;
					}
					else
						UserZoomRect.bottom = UserZoomRect.top + USER_ZOOM_MIN_HEIGHT;
				}
			}
			else if (m_bUserTopRight)
			{
				// Clipping?
				if (m_bUserClickClipping)
				{
					UserZoomRect.top = m_UserClickZoomRect.top + (point.y - m_ptUserClick.y);
					UserZoomRect.right = m_UserClickZoomRect.right + (point.x - m_ptUserClick.x);
				}
				else
				{
					UserZoomRect.top = point.y;
					UserZoomRect.right = point.x + 1;
				}

				// Update Also Bottom-Left?
				if (bSymmetrical)
				{
					dx = rcClient.right - UserZoomRect.right;
					UserZoomRect.left = rcClient.left + dx;
					dy = rcClient.top - UserZoomRect.top;
					UserZoomRect.bottom = rcClient.bottom + dy;
				}

				// Min Size
				if (UserZoomRect.Width() < USER_ZOOM_MIN_WIDTH)
				{
					if (bSymmetrical)
					{
						excess = (USER_ZOOM_MIN_WIDTH - UserZoomRect.Width()) / 2;
						UserZoomRect.left -= excess;
						UserZoomRect.right += excess;
					}
					else
						UserZoomRect.right = UserZoomRect.left + USER_ZOOM_MIN_WIDTH;
				}
				if (UserZoomRect.Height() < USER_ZOOM_MIN_HEIGHT)
				{
					if (bSymmetrical)
					{
						excess = (USER_ZOOM_MIN_HEIGHT - UserZoomRect.Height()) / 2;
						UserZoomRect.top -= excess;
						UserZoomRect.bottom += excess;
					}
					else
						UserZoomRect.top = UserZoomRect.bottom - USER_ZOOM_MIN_HEIGHT;
				}
			}
			else if (m_bUserTop)
			{
				// Clipping?
				if (m_bUserClickClipping)
					UserZoomRect.top = m_UserClickZoomRect.top + (point.y - m_ptUserClick.y);
				else
					UserZoomRect.top = point.y;

				// Update Also Bottom?
				if (bSymmetrical)
				{
					dy = rcClient.top - UserZoomRect.top;
					UserZoomRect.bottom = rcClient.bottom + dy;
				}

				// Min Size
				if (UserZoomRect.Height() < USER_ZOOM_MIN_HEIGHT)
				{
					if (bSymmetrical)
					{
						excess = (USER_ZOOM_MIN_HEIGHT - UserZoomRect.Height()) / 2;
						UserZoomRect.top -= excess;
						UserZoomRect.bottom += excess;
					}
					else
						UserZoomRect.top = UserZoomRect.bottom - USER_ZOOM_MIN_HEIGHT;
				}
			}
			else if (m_bUserBottom)
			{
				// Clipping?
				if (m_bUserClickClipping)
					UserZoomRect.bottom = m_UserClickZoomRect.bottom + (point.y - m_ptUserClick.y);
				else
					UserZoomRect.bottom = point.y + 1;

				// Update Also Top?
				if (bSymmetrical)
				{
					dy = rcClient.bottom - UserZoomRect.bottom;
					UserZoomRect.top = rcClient.top + dy;
				}

				// Min Size
				if (UserZoomRect.Height() < USER_ZOOM_MIN_HEIGHT)
				{
					if (bSymmetrical)
					{
						excess = (USER_ZOOM_MIN_HEIGHT - UserZoomRect.Height()) / 2;
						UserZoomRect.top -= excess;
						UserZoomRect.bottom += excess;
					}
					else
						UserZoomRect.bottom = UserZoomRect.top + USER_ZOOM_MIN_HEIGHT;
				}
			}
			else if (m_bUserLeft)
			{
				// Clipping?
				if (m_bUserClickClipping)
					UserZoomRect.left = m_UserClickZoomRect.left + (point.x - m_ptUserClick.x);
				else
					UserZoomRect.left = point.x;

				// Update Also Right?
				if (bSymmetrical)
				{
					dx = rcClient.left - UserZoomRect.left;
					UserZoomRect.right = rcClient.right + dx;
				}

				// Min Size
				if (UserZoomRect.Width() < USER_ZOOM_MIN_WIDTH)
				{
					if (bSymmetrical)
					{
						excess = (USER_ZOOM_MIN_WIDTH - UserZoomRect.Width()) / 2;
						UserZoomRect.left -= excess;
						UserZoomRect.right += excess;
					}
					else
						UserZoomRect.left = UserZoomRect.right - USER_ZOOM_MIN_WIDTH;
				}
			}
			else if (m_bUserRight)
			{
				// Clipping?
				if (m_bUserClickClipping)
					UserZoomRect.right = m_UserClickZoomRect.right + (point.x - m_ptUserClick.x);
				else
					UserZoomRect.right = point.x + 1;

				// Update Also Left?
				if (bSymmetrical)
				{
					dx = rcClient.right - UserZoomRect.right;
					UserZoomRect.left = rcClient.left + dx;
				}

				// Min Size
				if (UserZoomRect.Width() < USER_ZOOM_MIN_WIDTH)
				{
					if (bSymmetrical)
					{
						excess = (USER_ZOOM_MIN_WIDTH - UserZoomRect.Width()) / 2;
						UserZoomRect.left -= excess;
						UserZoomRect.right += excess;
					}
					else
						UserZoomRect.right = UserZoomRect.left + USER_ZOOM_MIN_WIDTH;
				}
			}

			// Set User Rect
			m_UserZoomRect = UserZoomRect;

			// Invalidate View
			pDoc->InvalidateView();
		}
	}
	else
		m_hResizeCursor = NULL;
}

void CVideoAviView::OnMButtonDown(UINT nFlags, CPoint point) 
{ 
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc->IsProcessing())
	{
		if (!pDoc->m_PlayVideoFileThread.IsAlive() &&
			!pDoc->m_PlayAudioFileThread.IsAlive())
			pDoc->PlayAVI();
		else
			pDoc->StopAVI();
	}
	CUImagerView::OnMButtonDown(nFlags, point);
}

void CVideoAviView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	EnableCursor();

	if (pDoc->m_pPlayerToolBarDlg)
		pDoc->m_pPlayerToolBarDlg->Close();	

	CUImagerView::OnLButtonDown(nFlags, point);

	if (m_bFullScreenMode)
	{
		int nRectSizeXInside = USER_RECT_X_INSIDE;
		int nRectSizeYInside = USER_RECT_Y_INSIDE;
		int nRectSizeXOutside = USER_RECT_X_OUTSIDE;
		int nRectSizeYOutside = USER_RECT_Y_OUTSIDE;

		CRect rcClient;
		GetClientRect(rcClient);
		CRect UserZoomClippedRect = m_UserZoomRect;
		if (UserZoomClippedRect.left < 0)
			UserZoomClippedRect.left = 0;
		if (UserZoomClippedRect.right > rcClient.right)
			UserZoomClippedRect.right = rcClient.right;
		if (UserZoomClippedRect.top < 0)
			UserZoomClippedRect.top = 0;
		if (UserZoomClippedRect.bottom > rcClient.bottom)
			UserZoomClippedRect.bottom = rcClient.bottom;

		CRect rcTop(	UserZoomClippedRect.left + nRectSizeXInside,
						UserZoomClippedRect.top - nRectSizeYOutside,
						UserZoomClippedRect.right - nRectSizeXInside,
						UserZoomClippedRect.top + nRectSizeYInside);

		CRect rcBottom(	UserZoomClippedRect.left + nRectSizeXInside,
						UserZoomClippedRect.bottom - nRectSizeYInside,
						UserZoomClippedRect.right - nRectSizeXInside,
						UserZoomClippedRect.bottom + nRectSizeYOutside);

		CRect rcLeft(	UserZoomClippedRect.left - nRectSizeXOutside,
						UserZoomClippedRect.top + nRectSizeYInside,
						UserZoomClippedRect.left + nRectSizeXInside,
						UserZoomClippedRect.bottom - nRectSizeYInside);

		CRect rcRight(	UserZoomClippedRect.right - nRectSizeXInside,
						UserZoomClippedRect.top + nRectSizeYInside,
						UserZoomClippedRect.right + nRectSizeXOutside,
						UserZoomClippedRect.bottom - nRectSizeYInside);

		CRect rcTopLeft(UserZoomClippedRect.left - nRectSizeXOutside,
						UserZoomClippedRect.top - nRectSizeYOutside,
						UserZoomClippedRect.left + nRectSizeXInside,
						UserZoomClippedRect.top + nRectSizeYInside);

		CRect rcBottomLeft(	UserZoomClippedRect.left - nRectSizeXOutside,
							UserZoomClippedRect.bottom - nRectSizeYInside,
							UserZoomClippedRect.left + nRectSizeXInside,
							UserZoomClippedRect.bottom + nRectSizeYOutside);

		CRect rcTopRight(	UserZoomClippedRect.right - nRectSizeXInside,
							UserZoomClippedRect.top - nRectSizeYOutside,
							UserZoomClippedRect.right + nRectSizeXOutside,
							UserZoomClippedRect.top + nRectSizeYInside);

		CRect rcBottomRight(UserZoomClippedRect.right - nRectSizeXInside,
							UserZoomClippedRect.bottom - nRectSizeYInside,
							UserZoomClippedRect.right + nRectSizeXOutside,
							UserZoomClippedRect.bottom + nRectSizeYOutside);

		// Set Mouse Capture
		SetCapture();

		// Store Click Point and Zoom Rect
		m_ptUserClick = point;
		m_UserClickZoomRect = m_UserZoomRect;
		
		if (rcTopLeft.PtInRect(point))
		{
			if (m_UserZoomRect.top <= 0 &&
				m_UserZoomRect.left <= 0)
				m_bUserClickClipping = TRUE;
			else
				m_bUserClickClipping = FALSE;
			m_bUserTopLeft = TRUE;
		}
		else if (rcBottomRight.PtInRect(point))
		{
			if (m_UserZoomRect.bottom >= rcClient.bottom &&
				m_UserZoomRect.right >= rcClient.right)
				m_bUserClickClipping = TRUE;
			else
				m_bUserClickClipping = FALSE;
			m_bUserBottomRight = TRUE;
		}
		else if (rcBottomLeft.PtInRect(point))
		{
			if (m_UserZoomRect.bottom >= rcClient.bottom &&
				m_UserZoomRect.left <= 0)
				m_bUserClickClipping = TRUE;
			else
				m_bUserClickClipping = FALSE;
			m_bUserBottomLeft = TRUE;
		}
		else if (rcTopRight.PtInRect(point))
		{
			if (m_UserZoomRect.top <= 0 &&
				m_UserZoomRect.right >= rcClient.right)
				m_bUserClickClipping = TRUE;
			else
				m_bUserClickClipping = FALSE;
			m_bUserTopRight = TRUE;
		}
		else if (rcTop.PtInRect(point))
		{
			if (m_UserZoomRect.top <= 0)
				m_bUserClickClipping = TRUE;
			else
				m_bUserClickClipping = FALSE;
			m_bUserTop = TRUE;
		}
		else if (rcBottom.PtInRect(point))
		{
			if (m_UserZoomRect.bottom >= rcClient.bottom)
				m_bUserClickClipping = TRUE;
			else
				m_bUserClickClipping = FALSE;
			m_bUserBottom = TRUE;
		}
		else if (rcLeft.PtInRect(point))
		{
			if (m_UserZoomRect.left <= 0)
				m_bUserClickClipping = TRUE;
			else
				m_bUserClickClipping = FALSE;
			m_bUserLeft = TRUE;
		}
		else if (rcRight.PtInRect(point))
		{
			if (m_UserZoomRect.right >= rcClient.right)
				m_bUserClickClipping = TRUE;
			else
				m_bUserClickClipping = FALSE;
			m_bUserRight = TRUE;
		}

		if (m_bUserTopLeft)
			m_hResizeCursor = ::LoadCursor(NULL, IDC_SIZENWSE);
		else if (m_bUserBottomRight)
			m_hResizeCursor = ::LoadCursor(NULL, IDC_SIZENWSE);
		else if (m_bUserBottomLeft)
			m_hResizeCursor = ::LoadCursor(NULL, IDC_SIZENESW);
		else if (m_bUserTopRight)
			m_hResizeCursor = ::LoadCursor(NULL, IDC_SIZENESW);
		else if (m_bUserTop)
			m_hResizeCursor = ::LoadCursor(NULL, IDC_SIZENS);
		else if (m_bUserBottom)
			m_hResizeCursor = ::LoadCursor(NULL, IDC_SIZENS);
		else if (m_bUserLeft)
			m_hResizeCursor = ::LoadCursor(NULL, IDC_SIZEWE);
		else if (m_bUserRight)
			m_hResizeCursor = ::LoadCursor(NULL, IDC_SIZEWE);
		else
			m_hResizeCursor = NULL;
	}
}

void CVideoAviView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pDoc->m_pPlayerToolBarDlg)
		pDoc->m_pPlayerToolBarDlg->Close();
	
	if (pDoc->m_pAVIPlay					&&
		pDoc->m_pAVIPlay->HasVideo()		&&
		(pDoc->m_nActiveVideoStream >= 0)	&&
		!pDoc->IsProcessing())
	{
		EnableCursor();
		::AfxGetMainFrame()->EnterExitFullscreen();
	}
}

void CVideoAviView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	ForceCursor();

	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_CONTEXT_VIDEO_AVI));
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	SetForegroundWindow();
	ClientToScreen(&point);

	if (m_bFullScreenMode)
		pDoc->PlayerToolBarDlg(point);
	else
		pPopup->TrackPopupMenu(TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, ::AfxGetMainFrame());

	ForceCursor(FALSE);

	CUImagerView::OnRButtonDown(nFlags, point);
}

void CVideoAviView::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	ForceCursor();

	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_CONTEXT_VIDEO_AVI));
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	SetForegroundWindow();
	ClientToScreen(&point);

	if (m_bFullScreenMode)
		pDoc->PlayerToolBarDlg(point);
	else
		pPopup->TrackPopupMenu(TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, ::AfxGetMainFrame());

	ForceCursor(FALSE);

	CUImagerView::OnRButtonDblClk(nFlags, point);
}

BOOL CVideoAviView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	EnableCursor();

	// Very Fast Frame Front/Back
	if (MK_SHIFT & nFlags)
	{
		if (zDelta > 0)
		{
			if (!pDoc->IsProcessing())
				pDoc->FrameBackFast(TRUE);
		}
		else
		{
			if (!pDoc->IsProcessing())
				pDoc->FrameFrontFast(TRUE);
		}
	}
	// Very Slow Frame Front/Back
	else if (MK_CONTROL & nFlags)
	{
		if (zDelta > 0)
		{
			if (!pDoc->IsProcessing())
				pDoc->FrameBack();
		}
		else
		{
			if (!pDoc->IsProcessing())
				pDoc->FrameFront();
		}
	}
	else
	{
		// Video play speed
		if (pDoc->m_pAVIPlay->HasVideo()	&&
			pDoc->m_nActiveVideoStream >= 0	&&
			pDoc->m_PlayVideoFileThread.IsAlive())
		{
			ChangePlaySpeed(zDelta < 0);
		}
		// Normal Frame Front/Back
		else
		{
			if (zDelta > 0)
			{
				if (!pDoc->IsProcessing())
					pDoc->FrameBackFast();
			}
			else
			{
				if (!pDoc->IsProcessing())
					pDoc->FrameFrontFast();
			}
		}
	}	

	return CUImagerView::OnMouseWheel(nFlags, zDelta, pt);
}

void CVideoAviView::OnPlayInc() 
{
	ChangePlaySpeed(TRUE);
}

void CVideoAviView::OnUpdatePlayInc(CCmdUI* pCmdUI) 
{
	CVideoAviDoc* pDoc = (CVideoAviDoc*)GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->Enable(	pDoc->m_pAVIPlay->HasVideo()	&&
					pDoc->m_nActiveVideoStream >= 0	&&
					pDoc->m_PlayVideoFileThread.IsAlive());
}

void CVideoAviView::OnPlayDec() 
{
	ChangePlaySpeed(FALSE);
}

void CVideoAviView::OnUpdatePlayDec(CCmdUI* pCmdUI) 
{
	CVideoAviDoc* pDoc = (CVideoAviDoc*)GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->Enable(	pDoc->m_pAVIPlay->HasVideo()	&&
					pDoc->m_nActiveVideoStream >= 0	&&
					pDoc->m_PlayVideoFileThread.IsAlive());
}

void CVideoAviView::ChangePlaySpeed(BOOL bIncrement)
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Kill audio thread and reset audio/video sync. handle
	if (pDoc->m_PlayAudioFileThread.IsAlive())
		pDoc->m_PlayAudioFileThread.Kill_NoBlocking();
	pDoc->m_PlayVideoFileThread.SetPlaySyncEvent(NULL);

	// Set video play speed
	int nSpeedPercent = pDoc->m_PlayVideoFileThread.GetPlaySpeedPercent();
	if (bIncrement)
	{
		if (nSpeedPercent >= 5000)
			nSpeedPercent += 1000;
		else if (nSpeedPercent >= 2000)
			nSpeedPercent += 500;
		else if (nSpeedPercent >= 1000)
			nSpeedPercent += 200;
		else if (nSpeedPercent >= 500)
			nSpeedPercent += 100;
		else if (nSpeedPercent >= 100)
			nSpeedPercent += 50;
		else if (nSpeedPercent >= 60)
			nSpeedPercent += 20;
		else
			nSpeedPercent += 10;
	}
	else
	{
		if (nSpeedPercent > 5000)
			nSpeedPercent -= 1000;
		else if (nSpeedPercent > 2000)
			nSpeedPercent -= 500;
		else if (nSpeedPercent > 1000)
			nSpeedPercent -= 200;
		else if (nSpeedPercent > 500)
			nSpeedPercent -= 100;
		else if (nSpeedPercent > 100)
			nSpeedPercent -= 50;
		else if (nSpeedPercent > 60)
			nSpeedPercent -= 20;
		else
			nSpeedPercent -= 10;
	}
	if (nSpeedPercent <= 0)
		nSpeedPercent = 10;
	else if (nSpeedPercent >= 9000)
		nSpeedPercent = 9000;
	pDoc->m_PlayVideoFileThread.SetPlaySpeedPercent(nSpeedPercent);
}

void CVideoAviView::OnInitialUpdate() 
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Set Pointers
	pDoc->SetView(this);
	pDoc->SetFrame((CVideoAviChildFrame*)GetParentFrame());

	// Create Font
	MakeFont(FALSE);

	// Call Base Class Implementation
	CUImagerView::OnInitialUpdate();
}

LONG CVideoAviView::OnAviFileProgress(WPARAM wparam, LPARAM lparam)
{
	CVideoAviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	
	DWORD fccType = (DWORD)wparam; // streamtypeVIDEO or streamtypeAUDIO

	if (fccType == streamtypeVIDEO)
		pDoc->m_nVideoPercentDone = (int)lparam;
	else if (fccType == streamtypeAUDIO)
		pDoc->m_nAudioPercentDone = (int)lparam;

	UpdateWindowSizes(TRUE, FALSE, FALSE);

	return 1;
}

void CVideoAviView::OnTimer(UINT nIDEvent) 
{
	CVideoAviDoc* pDoc = (CVideoAviDoc*)GetDocument();
	ASSERT_VALID(pDoc);

	switch (nIDEvent)
	{
		case ID_TIMER_AVIPLAYSLIDER :
		{
			if (m_bThumbTrack)
			{
				CAVIPlay::CAVIVideoStream* pVideoStream = pDoc->m_pAVIPlay->GetVideoStream(pDoc->m_nActiveVideoStream);
				if (pVideoStream)
				{
					if (m_nPreviewThumbTrackPos != m_nThumbTrackPos)
					{
						if (m_nLastThumbTrackPos == m_nThumbTrackPos)
							m_nPreviewThumbTrackPos = m_nThumbTrackPos;
						else
						{
							m_nLastThumbTrackPos = m_nThumbTrackPos;
							m_nPreviewThumbTrackPos = pVideoStream->GetPrevKeyFrame(m_nThumbTrackPos);
						}
						pVideoStream->SetCurrentFramePos(m_nPreviewThumbTrackPos);
						pDoc->DisplayFrame(m_nPreviewThumbTrackPos);
						pDoc->m_PlayAudioFileThread.PlaySyncAudioFromVideo();
					}
				}
			}
			break;
		}
		default:
			break;
	}

	CUImagerView::OnTimer(nIDEvent);
}

void CVideoAviView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// Reset Vars
	m_bUserTopLeft = FALSE;
	m_bUserBottomRight = FALSE;
	m_bUserBottomLeft = FALSE;
	m_bUserTopRight = FALSE;
	m_bUserTop = FALSE;
	m_bUserBottom = FALSE;
	m_bUserLeft = FALSE;
	m_bUserRight = FALSE;
	
	// Release Mouse Capture
	ReleaseCapture();
	
	CUImagerView::OnLButtonUp(nFlags, point);
}

void CVideoAviView::OnViewFullscreen() 
{
	CVideoAviDoc* pDoc = (CVideoAviDoc*)GetDocument();
	ASSERT_VALID(pDoc);
	if (pDoc->m_pAVIPlay					&&
		pDoc->m_pAVIPlay->HasVideo()		&&
		(pDoc->m_nActiveVideoStream >= 0)	&&
		!pDoc->IsProcessing())
		::AfxGetMainFrame()->EnterExitFullscreen();
}

void CVideoAviView::OnUpdateViewFullscreen(CCmdUI* pCmdUI) 
{
	CVideoAviDoc* pDoc = (CVideoAviDoc*)GetDocument();
	ASSERT_VALID(pDoc);
	pCmdUI->Enable(	pDoc->m_pAVIPlay					&&
					pDoc->m_pAVIPlay->HasVideo()		&&
					(pDoc->m_nActiveVideoStream >= 0)	&&
					!pDoc->IsProcessing());
	pCmdUI->SetCheck(m_bFullScreenMode ? 1 : 0);
}

BOOL CVideoAviView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// If Wait Cursor leave it!
	if (((CUImagerApp*)::AfxGetApp())->IsWaitCursor())
	{
		RestoreWaitCursor();
		return TRUE;
	}
	else if (m_hResizeCursor)
	{
		::SetCursor(m_hResizeCursor);
		return TRUE;
	}
	else
		return CUImagerView::OnSetCursor(pWnd, nHitTest, message);
}
