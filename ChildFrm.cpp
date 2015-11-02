// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "uImager.h"
#include "MainFrm.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_COMMAND(ID_VIEW_TOOLBAR, OnViewToolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, OnUpdateViewToolbar)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// Used For Closing in OnClose()
	m_bFirstCloseAttempt = TRUE;
	m_dwShutdownStartUpTime = 0;
}

CChildFrame::~CChildFrame()
{

}

void CChildFrame::OnViewToolbar()
{
	::AfxGetMainFrame()->ToggleToolbars();
}

void CChildFrame::OnUpdateViewToolbar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(((CUImagerApp*)::AfxGetApp())->m_bShowToolbar);
}

LRESULT CChildFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
	// Adapted version of CFrameWnd::OnSetMessageString() from winfrm.cpp
	// see also CMainFrame::OnSetMessageString()

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
			ASSERT(wParam == 0);	// can't have both an ID and a string
			sMsg = (LPCTSTR)lParam;	// set an explicit string
		}
		else if (wParam != 0)
		{
			// map SC_CLOSE to PREVIEW_CLOSE when in print preview mode
			if (wParam == AFX_IDS_SCCLOSE && m_lpfnCloseProc != NULL)
				wParam = AFX_IDS_PREVIEW_CLOSE;

			// do not show idle message if a status bar string has been set
            if (wParam == AFX_IDS_IDLEMESSAGE && ::AfxGetMainFrame()->GetStatusBarString() != _T(""))
				sMsg = ::AfxGetMainFrame()->GetStatusBarString();
			// special handling for DirectShow devices
			else if (wParam >= ID_DIRECTSHOW_VIDEODEV_FIRST && wParam <= ID_DIRECTSHOW_VIDEODEV_LAST)
				sMsg = _T("DirectShow / WDM");
			// get message associated with the ID indicated by wParam
			// NT64: Assume IDs are still 32-bit
			else
			{
				GetMessageString((UINT)wParam, strMessage);
				sMsg = strMessage;
			}
		}
		pMessageBar->SetWindowText(sMsg);

		// update owner of the bar in terms of last message selected
		::AfxGetMainFrame()->SetIDLastMessage((UINT)wParam);
		::AfxGetMainFrame()->SetIDTracking((UINT)wParam);
	}

	m_nIDLastMessage = (UINT)wParam;    	// new ID (or 0)
	m_nIDTracking = (UINT)wParam;       	// so F1 on toolbar buttons work
	return nIDLast;
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG