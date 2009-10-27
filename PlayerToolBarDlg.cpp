// PlayerToolBarDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "MainFrm.h"
#include "PlayerToolBarDlg.h"
#include "VideoAviView.h"
#include "VideoAviDoc.h"
#include "XThemeHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED     0x031A
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlayerToolBarDlg dialog


CPlayerToolBarDlg::CPlayerToolBarDlg(CWnd* pParent)
	: CDialog(CPlayerToolBarDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlayerToolBarDlg)
	//}}AFX_DATA_INIT
	m_hTheme = NULL;
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	pView->ForceCursor();
	CDialog::Create(CPlayerToolBarDlg::IDD, pParent);
}

CPlayerToolBarDlg::~CPlayerToolBarDlg()
{
	if (m_hTheme)
		ThemeHelper.CloseThemeData(m_hTheme);
	m_hTheme = NULL;
}

void CPlayerToolBarDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlayerToolBarDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPlayerToolBarDlg, CDialog)
	//{{AFX_MSG_MAP(CPlayerToolBarDlg)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlayerToolBarDlg message handlers

void CPlayerToolBarDlg::OnClose() 
{	
	DestroyWindow();
}

void CPlayerToolBarDlg::Close()
{
	OnClose();
}

BOOL CPlayerToolBarDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_RBUTTONDBLCLK ||
		pMsg->message == WM_RBUTTONDOWN)
	{
		OnClose();
		return TRUE; // Do Not Dispatch
	}
	else
		return CDialog::PreTranslateMessage(pMsg);
}

BOOL CPlayerToolBarDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (HIWORD (wParam) == BN_CLICKED)
	{
		switch (LOWORD (wParam))
		{
			case IDOK:
			case IDCANCEL:
				OnClose();
				return TRUE;
			case ID_PLAY_AVI:
			case ID_STOP_AVI:
			case ID_PLAY_VOL:
			case ID_PLAY_LOOP:
			case ID_FILE_INFO:
			case ID_VIEW_TIMEPOSITION:
			case ID_VIEW_FIT:
			case ID_PLAY_AUDIOVIDEOSHIFT:
				CDialog::OnCommand(wParam, lParam);
				OnClose();
				return TRUE;
			default:
				break;
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}

void CPlayerToolBarDlg::PostNcDestroy() 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();
	ASSERT_VALID(pDoc);

	pDoc->m_pPlayerToolBarDlg = NULL;
	pView->ForceCursor(FALSE);
	delete this;	
	CDialog::PostNcDestroy();
}

BOOL CPlayerToolBarDlg::OnInitDialog() 
{
	CVideoAviView* pView = (CVideoAviView*)m_pParentWnd;
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = (CVideoAviDoc*)pView->GetDocument();
	ASSERT_VALID(pDoc);

	CAVIPlay::CAVIVideoStream* pVideoStream = pDoc->m_pAVIPlay->GetVideoStream(pDoc->m_nActiveVideoStream);
	if (pVideoStream)
	{
		if (ThemeHelper.IsThemeLibAvailable())
			m_hTheme = ThemeHelper.OpenThemeData(m_hWnd, _T("Toolbar"));

		CDialog::OnInitDialog();
		
		CRect rcWork = ::AfxGetMainFrame()->GetMonitorWorkRect(this);
		int nToolBarWidth = (int)(rcWork.Width() / TOOLBAR_SCREENSIZE_FRACTION);
		nToolBarWidth = MAX(TOOLBAR_MIN_SIZE, nToolBarWidth);
		if (nToolBarWidth > rcWork.Width())
			nToolBarWidth = rcWork.Width();
		CRect rcDlg;
		GetWindowRect(&rcDlg);
		m_VideoAviToolBar.Create(this, TRUE, nToolBarWidth);
		CRect rcToolBar;
		m_VideoAviToolBar.GetWindowRect(&rcToolBar);
		m_VideoAviToolBar.SetWindowPos(	NULL,
										0,
										0,
										nToolBarWidth,
										25,
										SWP_NOZORDER);
		SetWindowPos(	NULL,
						0,
						0,
						nToolBarWidth,
						26,
						SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOZORDER);	// Will Be Shown Outside this Class!

		// Set Slider Position
		if (pDoc->m_pAVIPlay->HasVideo() && (pDoc->m_nActiveVideoStream >= 0))
		{
			if (pVideoStream->GetTotalFrames() > 1)
				m_VideoAviToolBar.m_PlayerSlider.SetPos(Round((double)pVideoStream->GetCurrentFramePos() /
															(double)(pVideoStream->GetTotalFrames() - 1) *
															(double)(m_VideoAviToolBar.m_PlayerSlider.GetRangeMax())));
			else
				m_VideoAviToolBar.m_PlayerSlider.SetPos(0);
		}
		else if (pDoc->m_pAVIPlay->HasAudio() && (pDoc->m_nActiveAudioStream >= 0))
		{
			if (pDoc->m_PlayAudioFileThread.GetTotalSamples() > 1)
				m_VideoAviToolBar.m_PlayerSlider.SetPos(Round((double)pDoc->m_PlayAudioFileThread.GetCurrentSamplePos() /
															(double)(pDoc->m_PlayAudioFileThread.GetTotalSamples() - 1) *
															(double)(m_VideoAviToolBar.m_PlayerSlider.GetRangeMax())));
			else
				m_VideoAviToolBar.m_PlayerSlider.SetPos(0);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPlayerToolBarDlg::IsThemed()
{
	return (m_hTheme &&
			ThemeHelper.IsAppThemed() &&
			ThemeHelper.IsThemeComCtl32());
}

void CPlayerToolBarDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rcDlg;
	GetClientRect(&rcDlg);
	
	COLORREF col = ::GetSysColor(COLOR_BTNFACE);
	if (IsThemed())
	{
		ThemeHelper.GetThemeColor(	m_hTheme,
									TP_BUTTON,
									TS_NORMAL,
									TMT_FILLCOLOR,
									&col);
	}

	CPen Pen;
	Pen.CreatePen(PS_SOLID, 1, col);
	CPen* pOldPen = dc.SelectObject(&Pen);
	int i;

	// Top Border
	for (i = 0 ; i < 2 ; i++)
	{
		dc.MoveTo(rcDlg.left, rcDlg.top+i);
		dc.LineTo(rcDlg.right, rcDlg.top+i);
	}
	
	// Left Border
	for (i = 0 ; i < 2 ; i++)
	{
		dc.MoveTo(rcDlg.left+i, rcDlg.top);
		dc.LineTo(rcDlg.left+i, rcDlg.bottom);
	}

	// Bottom Border
	for (i = 0 ; i < 2 ; i++)
	{
		dc.MoveTo(rcDlg.left, rcDlg.bottom-i-1);
		dc.LineTo(rcDlg.right, rcDlg.bottom-i-1);
	}

	// Right Border
	for (i = 0 ; i < 2 ; i++)
	{
		dc.MoveTo(rcDlg.right-i-1, rcDlg.top);
		dc.LineTo(rcDlg.right-i-1, rcDlg.bottom);
	}

	dc.SelectObject(pOldPen);
	Pen.DeleteObject();

	// Do not call CDialog::OnPaint() for painting messages
}

LRESULT CPlayerToolBarDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
		case WM_THEMECHANGED:
		{
			if (IsThemed())
			{
				if (m_hTheme)
				{
					// when user changes themes, close current theme and re-open
					ThemeHelper.CloseThemeData(m_hTheme);
					m_hTheme = ThemeHelper.OpenThemeData(m_hWnd, _T("Toolbar"));
				}
			}
		}
		break;
	}

	return CDialog::DefWindowProc(message, wParam, lParam);
}