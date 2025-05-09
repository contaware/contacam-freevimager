#if !defined(AFX_LAYEREDDLG_H__B0F2328B_122D_489E_8587_38D5BDB00278__INCLUDED_)
#define AFX_LAYEREDDLG_H__B0F2328B_122D_489E_8587_38D5BDB00278__INCLUDED_

#pragma once

// LayeredDlg.h : header file
//

#include "Dib.h"

// NOTE:
// IDD_LAYERED must only have the WS_POPUP style,
// otherwise it is not working correctly on all OSs!

/////////////////////////////////////////////////////////////////////////////
// CLayeredDlg dialog

class CLayeredDlg : public CDialog
{
// Construction
public:
	CLayeredDlg(CWnd* pParent);
	void Close();
	CDib m_CurrentLayeredDib;
	CPoint m_ptLastRightClick;

// Dialog Data
	//{{AFX_DATA(CLayeredDlg)
	enum { IDD = IDD_LAYERED };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLayeredDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CLayeredDlg)
	afx_msg void OnClose();
	//}}AFX_MSG
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnEnable(BOOL bEnable);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LAYEREDDLG_H__B0F2328B_122D_489E_8587_38D5BDB00278__INCLUDED_)
