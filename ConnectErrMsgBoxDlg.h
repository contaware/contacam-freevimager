#if !defined(AFX_CONNECTERRMSGBOXDLG_H__D2B1574B_1FDD_4095_8BAF_2F8AC46F019A__INCLUDED_)
#define AFX_CONNECTERRMSGBOXDLG_H__D2B1574B_1FDD_4095_8BAF_2F8AC46F019A__INCLUDED_

#pragma once

// ConnectErrMsgBoxDlg.h : header file
//

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CConnectErrMsgBoxDlg dialog

class CConnectErrMsgBoxDlg : public CDialog
{
// Construction
public:
	CConnectErrMsgBoxDlg(LPCTSTR lpszText, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CConnectErrMsgBoxDlg)
	enum { IDD = IDD_CONNECTERR_MSGBOX };
	BOOL	m_bAutorun;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConnectErrMsgBoxDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CFont m_MsgFont;
	CString m_sMsgText;
	COLORREF m_crTop;
	COLORREF m_crBottom;
	CBrush m_TopBrush;
	CBrush m_BottomBrush;
	// Generated message map functions
	//{{AFX_MSG(CConnectErrMsgBoxDlg)
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	afx_msg void OnCheckAutorun();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONNECTERRMSGBOXDLG_H__D2B1574B_1FDD_4095_8BAF_2F8AC46F019A__INCLUDED_)
