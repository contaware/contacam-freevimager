#if !defined(AFX_PROGRESSDLG_H__B7B514DD_4620_4F2E_A4B1_58E607D02EB9__INCLUDED_)
#define AFX_PROGRESSDLG_H__B7B514DD_4620_4F2E_A4B1_58E607D02EB9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgressDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg dialog

class CProgressDlg : public CDialog
{
// Construction
public:
	CProgressDlg(HWND hMasterWnd, const CString& sTitle, DWORD dwStartTimeMs, DWORD dwWaitTimeMs);
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

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESSDLG_H__B7B514DD_4620_4F2E_A4B1_58E607D02EB9__INCLUDED_)
