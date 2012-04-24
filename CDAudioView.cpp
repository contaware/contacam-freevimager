// CDAudioView.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "CDAudioView.h"
#include "CDAudioDoc.h"
#include "CDAudio.h"
#include "MyMemDC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CDAUDIO_POLL_TIMER_DELAY	300		// ms

/////////////////////////////////////////////////////////////////////////////
// CCDAudioView

IMPLEMENT_DYNCREATE(CCDAudioView, CFormView)

CCDAudioView::CCDAudioView()
	: CFormView(CCDAudioView::IDD)
{
	//{{AFX_DATA_INIT(CCDAudioView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nLastTrackNum = 0;
}

CCDAudioView::~CCDAudioView()
{
}

void CCDAudioView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCDAudioView)
	DDX_Control(pDX, IDC_BUTTON_PAUSE, m_PauseButton);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_StopButton);
	DDX_Control(pDX, IDC_BUTTON_PLAY, m_PlayButton);
	DDX_Control(pDX, IDC_SLIDER_POS, m_SliderPos);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCDAudioView, CFormView)
	//{{AFX_MSG_MAP(CCDAudioView)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, OnButtonNext)
	ON_BN_CLICKED(IDC_BUTTON_PREVIOUS, OnButtonPrevious)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
	ON_WM_TIMER()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_TRACK, OnButtonTrack)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, OnButtonPause)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE((WM_USER + 1), (WM_USER + 200), OnTrackChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCDAudioView diagnostics

#ifdef _DEBUG
void CCDAudioView::AssertValid() const
{
	CFormView::AssertValid();
}

void CCDAudioView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CCDAudioDoc* CCDAudioView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCDAudioDoc)));
	return (CCDAudioDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCDAudioView message handlers

void CCDAudioView::OnInitialUpdate() 
{
	CCDAudioDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Set Pointers
	pDoc->SetView(this);
	pDoc->SetFrame((CCDAudioChildFrame*)GetParentFrame());

	CFormView::OnInitialUpdate();

	// Resize to dialog template size. Call ResizeParentToFit()
	// two times because if the mainframe is really small
	// ResizeParentToFit() is not working the first time...
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit(FALSE);
	ResizeParentToFit(FALSE);

	// Set Button Icons
	m_PlayButton.SetIcon(IDI_PLAY, CXButtonXP::LEFT);
	m_PauseButton.SetIcon(IDI_PAUSE, CXButtonXP::LEFT);
	m_StopButton.SetIcon(IDI_STOP, CXButtonXP::LEFT);

	// Set Timer
	SetTimer(ID_TIMER_CDAUDIO, CDAUDIO_POLL_TIMER_DELAY, NULL);
}

BOOL CCDAudioView::PreTranslateMessage(MSG* pMsg) 
{
	CCDAudioDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			if (::GetKeyState(VK_SHIFT) < 0)
				::AfxGetMainFrame()->PostMessage(WM_CLOSE, 0, 0);
			else
				GetParentFrame()->PostMessage(WM_CLOSE, 0, 0);
			return TRUE;
		}
	}
	else if (pMsg->message == WM_MOUSEWHEEL)
	{
		return TRUE;	// Do Not Dispatch to be consistent
						// with the Audio MCI interface!
	}

	return CFormView::PreTranslateMessage(pMsg);
}

void CCDAudioView::OnButtonPlay() 
{
	CCDAudioDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (pDoc->m_pCDAudio)
		pDoc->m_pCDAudio->Play();
}

void CCDAudioView::OnButtonNext() 
{
	CCDAudioDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (pDoc->m_pCDAudio)
		pDoc->m_pCDAudio->Forward();
}

void CCDAudioView::OnButtonPrevious() 
{
	CCDAudioDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (pDoc->m_pCDAudio)
		pDoc->m_pCDAudio->Backward();
}

void CCDAudioView::OnButtonPause() 
{
	CCDAudioDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (pDoc->m_pCDAudio)
		pDoc->m_pCDAudio->Pause();
}

void CCDAudioView::OnButtonStop() 
{
	CCDAudioDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (pDoc->m_pCDAudio)
		pDoc->m_pCDAudio->Stop();
}

void CCDAudioView::OnTimer(UINT nIDEvent) 
{
	CCDAudioDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (pDoc->m_pCDAudio)
	{	
		CString s, t;
		int nCurrentTrackNum = pDoc->m_pCDAudio->GetCurrentTrack();
		int nTotalTracks = pDoc->m_pCDAudio->GetTracksCount();
		int nCurrentTrackAbsolutePos = pDoc->m_pCDAudio->GetCurrentPos();
		int nTotalSeconds = pDoc->m_pCDAudio->GetLenghtAllTracks();
		int nCurrentTrackSeconds = pDoc->m_pCDAudio->GetTrackLength(nCurrentTrackNum);
		int nCurrentTrackBegin = pDoc->m_pCDAudio->GetTrackBeginTime(nCurrentTrackNum);
		int nCurrentTrackPos = nCurrentTrackAbsolutePos - nCurrentTrackBegin;

		// Stop
		if (nCurrentTrackAbsolutePos >= nTotalSeconds)
			pDoc->m_pCDAudio->Stop();

		// Set Slider Range, Position and
		// update the current file name
		if (nCurrentTrackNum != m_nLastTrackNum)
		{
			m_SliderPos.SetRange(1, nCurrentTrackSeconds, TRUE);
			m_nLastTrackNum = nCurrentTrackNum;
			CString sFileName;
			sFileName.Format(_T("%sTrack%02d.cda"),
						pDoc->m_pCDAudio->GetCurrentDrive(),
						nCurrentTrackNum);
			pDoc->SetPathName(sFileName, TRUE);
		}
		m_SliderPos.SetPos(nCurrentTrackPos);

		// Tracks
		s.Format(_T("%02d / %02d"),	nCurrentTrackNum,
									nTotalTracks);
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_TEXT_TRACKS);
		pEdit->GetWindowText(t);
		if (t != s)
			pEdit->SetWindowText(s);

		// Total Length
		s.Format(_T("Tot %02d:%02d"),
							nTotalSeconds / 60,
							nTotalSeconds % 60);
		pEdit = (CEdit*)GetDlgItem(IDC_TEXT_TOTAL);
		pEdit->GetWindowText(t);
		if (t != s)
			pEdit->SetWindowText(s);

		// Current Position
		s.Format(_T("%02d:%02d / %02d:%02d"),
							nCurrentTrackPos / 60,
							nCurrentTrackPos % 60,
							nCurrentTrackSeconds / 60,
							nCurrentTrackSeconds % 60);
		pEdit = (CEdit*)GetDlgItem(IDC_TEXT_CURRENT);
		pEdit->GetWindowText(t);
		if (t != s)
			pEdit->SetWindowText(s);
	}	
	CFormView::OnTimer(nIDEvent);
}

void CCDAudioView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CCDAudioDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (pScrollBar && pDoc->m_pCDAudio)
	{
		if ((SB_THUMBTRACK == nSBCode)		||	// Dragging Slider
			(SB_THUMBPOSITION == nSBCode)	||	// Wheel On Mouse And End Of Dragging Slider
			(SB_LINEUP  == nSBCode)			||	// Keyboard Arrow
			(SB_LINEDOWN  == nSBCode)		||	// Keyboard Arrow
			(SB_PAGEUP == nSBCode)			||	// Mouse Press Above Slider
			(SB_PAGEDOWN == nSBCode)		||	// Mouse Press Belove Slider
			(SB_LEFT == nSBCode)			||	// Home Button
			(SB_RIGHT == nSBCode))				// End Button  
		{
			int nCurrentTrackNum = pDoc->m_pCDAudio->GetCurrentTrack();
			int nCurrentTrackBegin = pDoc->m_pCDAudio->GetTrackBeginTime(nCurrentTrackNum);
			int nNewTrackAbsolutePos = m_SliderPos.GetPos() + nCurrentTrackBegin;
			pDoc->m_pCDAudio->Play(nNewTrackAbsolutePos);
		}
	}
	
	CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CCDAudioView::OnButtonTrack() 
{
	CCDAudioDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (pDoc->m_pCDAudio->IsReady() && pDoc->m_pCDAudio->IsMediaInsert())
	{
		// Get Button Rect
		CWnd* pBtn = GetDlgItem(IDC_BUTTON_TRACK);
		ASSERT(pBtn);
		CRect rect;
		pBtn->GetWindowRect(rect);	

		// Create Popup Menu
		CMenu menu;
		if (menu.CreatePopupMenu())
		{
			for (int nTrack = 1 ; nTrack <= pDoc->m_pCDAudio->GetTracksCount() ; nTrack++)
			{
				int nLength = pDoc->m_pCDAudio->GetTrackLength(nTrack);
				CString sTrack;
				sTrack.Format(_T("%02d\t%02d:%02d"), nTrack, (nLength / 60), (nLength % 60));
				menu.AppendMenu(MF_ENABLED | MF_STRING, (WM_USER + nTrack), sTrack);
			}
			menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, rect.left, rect.top, this);
		}
	}	
}

void CCDAudioView::OnTrackChange(UINT nID)
{
	CCDAudioDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (pDoc->m_pCDAudio)
	{
		int nTrack = (nID - WM_USER);
		int nCurrTrack = pDoc->m_pCDAudio->GetCurrentTrack();
		if (nCurrTrack == nTrack)
			return;
		pDoc->m_pCDAudio->Play(pDoc->m_pCDAudio->GetTrackBeginTime(nTrack));
	}
}
