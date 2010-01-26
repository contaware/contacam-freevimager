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

CProgressDlg::CProgressDlg(const CString& sTitle, DWORD dwStartTimeMs, DWORD dwWaitTimeMs)
: CDialog(CProgressDlg::IDD, CWnd::FromHandle(::GetDesktopWindow()))
{
	//{{AFX_DATA_INIT(CProgressDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bUseThread = TRUE;
	m_hMasterWnd = NULL;
	m_sTitle = sTitle;
	m_dwStartTimeMs = dwStartTimeMs;
	m_dwWaitTimeMs = dwWaitTimeMs;
	CDialog::Create(CProgressDlg::IDD, NULL);
}

CProgressDlg::CProgressDlg(HWND hMasterWnd, const CString& sTitle, DWORD dwStartTimeMs, DWORD dwWaitTimeMs)
	: CDialog(CProgressDlg::IDD, NULL)	// NULL -> Parent is Main Frame
{
	//{{AFX_DATA_INIT(CProgressDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bUseThread = FALSE;
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
	PostMessage(WM_CLOSE, 0, 0);
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
	if (::IsWindow(m_hMasterWnd) || m_bUseThread)
		m_Progress.StepIt();
	else
		Close(); // Close us if master window is closed!
}

void CProgressDlg::PostNcDestroy() 
{
	if (m_bUseThread)
		::AfxPostQuitMessage(0);
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


IMPLEMENT_DYNCREATE(CProgressDlgThread, CWinThread)

CProgressDlgThread::CProgressDlgThread()
{
	m_bAutoDelete = FALSE;
	m_pProgressDlg = NULL;
	m_sTitle = _T("Progress");
	m_dwStartTimeMs = 0;
	m_dwWaitTimeMs = 5000;
	m_hStartupDone = ::CreateEvent(NULL, TRUE, FALSE, NULL);
}

CProgressDlgThread::CProgressDlgThread(const CString& sTitle, DWORD dwStartTimeMs, DWORD dwWaitTimeMs)
{
	m_bAutoDelete = FALSE;
	m_pProgressDlg = NULL;
	m_sTitle = sTitle;
	m_dwStartTimeMs = dwStartTimeMs;
	m_dwWaitTimeMs = dwWaitTimeMs;
	m_hStartupDone = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	Start();
}

CProgressDlgThread::~CProgressDlgThread()
{
	Kill();
	if (m_hStartupDone)
		::CloseHandle(m_hStartupDone);
}

BOOL CProgressDlgThread::InitInstance()
{
 	m_pProgressDlg = new CProgressDlg(m_sTitle, m_dwStartTimeMs, m_dwWaitTimeMs);
	::SetEvent(m_hStartupDone);
	if (m_pProgressDlg)
		return TRUE;	// Start the message pump
	else
		return FALSE;	// Exit the thread
}

int CProgressDlgThread::ExitInstance()
{
	return CWinThread::ExitInstance();
}

void CProgressDlgThread::Start()
{
	if (m_hThread == NULL && CreateThread())
	{
		::WaitForSingleObject(m_hStartupDone, INFINITE);
		::ResetEvent(m_hStartupDone);
	}
}

void CProgressDlgThread::Kill(DWORD dwTimeout/*=INFINITE*/)
{
	if (m_pProgressDlg)
	{
		// Set the focus back to the Main Frame
		if (::AfxGetMainFrame())
		{
			::AttachThreadInput(::AfxGetApp()->m_nThreadID, m_nThreadID, TRUE);
			::AfxGetMainFrame()->SetFocus();
			::AfxGetMainFrame()->SetForegroundWindow();
			::AttachThreadInput(::AfxGetApp()->m_nThreadID, m_nThreadID, FALSE);
		}

		// Self-deletion of m_pProgressDlg
		m_pProgressDlg->Close();
		if (::WaitForSingleObject(m_hThread, dwTimeout) != WAIT_OBJECT_0)
		{
			// If it doesn't want to exit force the termination!
			if (m_hThread)
			{
				::TerminateThread(m_hThread, 0);
				TRACE(_T("Thread: %lu has been forced to terminate!\n"), m_nThreadID);
				ASSERT(FALSE);
			}
		}
		m_pProgressDlg = NULL;
		if (m_hThread)
		{
			CloseHandle(m_hThread);
			m_hThread = NULL;
			m_nThreadID = 0;
		}
	}
}

BEGIN_MESSAGE_MAP(CProgressDlgThread, CWinThread)
	//{{AFX_MSG_MAP(CProgressDlgThread)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()