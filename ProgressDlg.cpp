// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "ProgressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg dialog

CProgressDlg::CProgressDlg(HWND hMasterWnd, const CString& sTitle, DWORD dwStartTimeMs, DWORD dwWaitTimeMs)
	: CDialog(CProgressDlg::IDD, NULL)	// NULL -> Parent is Main Frame
{
	//{{AFX_DATA_INIT(CProgressDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_hMasterWnd = hMasterWnd;
	m_sTitle = sTitle;
	m_dwStartTimeMs = dwStartTimeMs;
	m_dwWaitTimeMs = dwWaitTimeMs;
	CDialog::Create(CProgressDlg::IDD, NULL);
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgressDlg)
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	//{{AFX_MSG_MAP(CProgressDlg)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg message handlers

BOOL CProgressDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	SetWindowText(m_sTitle);
	SetWindowPos(&this->wndTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
	m_Progress.SetRange32(0, m_dwWaitTimeMs);
	m_Progress.SetStep(1000);
	m_Progress.SetPos(m_dwStartTimeMs);
	SetTimer(ID_TIMER_PROGRESS, 1000U, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProgressDlg::OnClose() 
{
	DestroyWindow();
}

void CProgressDlg::Close()
{
	OnClose();
}

// Avoid closing dialog with ALT+F4
BOOL CProgressDlg::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message)
	{
		case WM_SYSKEYDOWN:
			if (pMsg->wParam == VK_F4)
				return TRUE;
			break;

		default:
			break;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

// Avoid closing dialog
BOOL CProgressDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (HIWORD (wParam) == BN_CLICKED)
	{
		switch (LOWORD (wParam))
		{
			case IDOK:
			case IDCANCEL:
				return TRUE;
			default:
				return CDialog::OnCommand(wParam, lParam);
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}

void CProgressDlg::OnTimer(UINT nIDEvent) 
{
	CDialog::OnTimer(nIDEvent);
	if (::IsWindow(m_hMasterWnd))
		m_Progress.StepIt();
	else
		Close(); // Close us if master window is closed!
}

void CProgressDlg::PostNcDestroy() 
{
	delete this;	
	CDialog::PostNcDestroy();
}

void CProgressDlg::OnDestroy() 
{
	KillTimer(ID_TIMER_PROGRESS);
	CDialog::OnDestroy();
}

BOOL CProgressDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// If Wait Cursor leave it!
	if (((CUImagerApp*)::AfxGetApp())->IsWaitCursor())
	{
		RestoreWaitCursor();
		return TRUE;
	}
	else
		return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

// Avoid moving dlg
void CProgressDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xfff0) == SC_MOVE ||
		(nID & 0xfff0) == SC_SIZE ||
		(nID & 0xfff0) == SC_RESTORE)
		return;
	CProgressDlg::OnSysCommand(nID, lParam);
}
