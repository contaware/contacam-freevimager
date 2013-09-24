#if !defined(AFX_AVIINFODLG_H__DAB93E92_63DD_4B2F_8139_BF1D64A05F74__INCLUDED_)
#define AFX_AVIINFODLG_H__DAB93E92_63DD_4B2F_8139_BF1D64A05F74__INCLUDED_

#pragma once

// AviInfoDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAviInfoDlg dialog

class CAviInfoDlg : public CDialog
{
// Construction
public:
	CAviInfoDlg(CWnd* pParent);
	void UpdateDisplay();
	void Close();

// Dialog Data
	//{{AFX_DATA(CAviInfoDlg)
	enum { IDD = IDD_AVIINFO };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAviInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAviInfoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AVIINFODLG_H__DAB93E92_63DD_4B2F_8139_BF1D64A05F74__INCLUDED_)
