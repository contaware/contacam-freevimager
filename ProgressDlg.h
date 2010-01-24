#if !defined(AFX_PROGRESSDLG_H__B7B514DD_4620_4F2E_A4B1_58E607D02EB9__INCLUDED_)
#define AFX_PROGRESSDLG_H__B7B514DD_4620_4F2E_A4B1_58E607D02EB9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/* Usage
   -----
new CProgressDlg(GetSafeHwnd(),
				_T("Long Op..."),,
				0,		// start progress bar at 0s
				15000);	// estimate 15sec
// ...some operations which do not block the UI
// when the passed master window closes, the progress dialog is closed and this object is self-deleted
*/
class CProgressDlg : public CDialog
{
// Construction
public:
	CProgressDlg(const CString& sTitle, DWORD dwStartTimeMs, DWORD dwWaitTimeMs);					// Used by CProgressDlgThread
	CProgressDlg(HWND hMasterWnd, const CString& sTitle, DWORD dwStartTimeMs, DWORD dwWaitTimeMs);	// Self deletion when hMasterWnd closes
	void Close();

// Dialog Data
	//{{AFX_DATA(CProgressDlg)
	enum { IDD = IDD_PROGRESS };
	CProgressCtrl m_Progress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgressDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL m_bUseThread;
	HWND m_hMasterWnd;
	CString m_sTitle;
	DWORD m_dwStartTimeMs;
	DWORD m_dwWaitTimeMs;
	// Generated message map functions
	//{{AFX_MSG(CProgressDlg)
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/* Usage 1
   -------
{
	CProgressDlgThread ProgressDlgThread(	_T("Long Op..."),
											0,		// start progress bar at 0s
											15000);	// estimate 15sec
	// ...some long operations that block the main UI thread
}
// exiting the scope will close the progress dialog and stop the thread
*/

/* Usage 2
   -------
CProgressDlgThread ProgressDlgThread;
ProgressDlgThread.m_sTitle = _T("Long Op...");
ProgressDlgThread.m_dwStartTimeMs = 0;		// start progress bar at 0s
ProgressDlgThread.m_dwWaitTimeMs = 15000;	// estimate 15sec
ProgressDlgThread.Start();
// ...some long operations that block the main UI thread
ProgressDlgThread.Kill();
*/
class CProgressDlgThread : public CWinThread
{
	DECLARE_DYNCREATE(CProgressDlgThread)
public:
	CProgressDlgThread();
	CProgressDlgThread(const CString& sTitle, DWORD dwStartTimeMs, DWORD dwWaitTimeMs);
	virtual ~CProgressDlgThread();
	void Start();
	void Kill(DWORD dwTimeout = INFINITE);
	CString m_sTitle;
	DWORD m_dwStartTimeMs;
	DWORD m_dwWaitTimeMs;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgressDlgThread)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
protected:
	CProgressDlg* volatile m_pProgressDlg;
	HANDLE volatile m_hStartupDone;
	
	// Generated message map functions
	//{{AFX_MSG(CProgressDlgThread)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESSDLG_H__B7B514DD_4620_4F2E_A4B1_58E607D02EB9__INCLUDED_)
