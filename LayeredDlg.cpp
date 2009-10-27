// LayeredDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "LayeredDlg.h"
#include "PictureDoc.h"
#include "PictureView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLayeredDlg dialog

CLayeredDlg::CLayeredDlg(CWnd* pParent)
: CDialog(CLayeredDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLayeredDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_ptLastRightClick = CPoint(0,0);
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	ASSERT_VALID(pView);
	pView->ForceCursor();
	CDialog::Create(CLayeredDlg::IDD, pParent);
}

void CLayeredDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLayeredDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLayeredDlg, CDialog)
	//{{AFX_MSG_MAP(CLayeredDlg)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLayeredDlg message handlers

LRESULT CLayeredDlg::OnNcHitTest(CPoint point) 
{
	return HTCAPTION; // Enable drag the window from any point
}

void CLayeredDlg::OnClose() 
{
	DestroyWindow();
}

void CLayeredDlg::Close()
{
	OnClose();
}

BOOL CLayeredDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (HIWORD (wParam) == BN_CLICKED)
	{
		switch (LOWORD (wParam))
		{
			case IDOK:
				return TRUE;
			case IDCANCEL:
				return TRUE;
			default:
				return CDialog::OnCommand(wParam, lParam);
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}

BOOL CLayeredDlg::PreTranslateMessage(MSG* pMsg) 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	if ((pMsg->message >= WM_KEYFIRST	&&
		pMsg->message <= WM_KEYLAST)	||
		pMsg->message == WM_MOUSEWHEEL)
	{
		pView->PostMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
		return TRUE;
	}
	else if (pMsg->message == WM_NCRBUTTONDOWN)
	{
		// Disable update
		pDoc->m_bDoUpdateLayeredDlg = FALSE;

		// Pause Slideshow
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

		// Pop-up context menu
		POINTS points = MAKEPOINTS(pMsg->lParam);   // position of cursor
		m_ptLastRightClick.x = points.x;
		m_ptLastRightClick.y = points.y;
		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_CONTEXT_LAYERED_DLG));	
		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);
		::AfxGetMainFrame()->SetForegroundWindow(); // Must be MainFrame otherwise menu may pop-under!
		pPopup->TrackPopupMenu(TPM_LEFTBUTTON|TPM_RIGHTBUTTON, m_ptLastRightClick.x, m_ptLastRightClick.y, pDoc->GetFrame()); // Send to this Frame and not MainFrame because another view may be active!

		// Note that the CPictureDoc::OnPlayStop function
		// (which in future may be called from inside the above TrackPopupMenu)
		// will set m_bDoRestartSlideshow to FALSE, so that the
		// Slideshow is not restarted if the Stop command is selected!
		if (pDoc->m_bDoRestartSlideshow)
		{
			pDoc->m_SlideShowThread.RunSlideshow();
			pDoc->m_bDoRestartSlideshow = FALSE;
		}

		// Re-enable updates and invalidate
		pDoc->m_bDoUpdateLayeredDlg = TRUE;
		pDoc->GetView()->Draw();

		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CLayeredDlg::PostNcDestroy() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	pDoc->m_pLayeredDlg = NULL;
	pView->ForceCursor(FALSE);
	delete this;
	CDialog::PostNcDestroy();
}
