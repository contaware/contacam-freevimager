#if !defined(AFX_SOFTBORDERSDLG_H__FD749E98_D020_4588_9900_587E28CE3821__INCLUDED_)
#define AFX_SOFTBORDERSDLG_H__FD749E98_D020_4588_9900_587E28CE3821__INCLUDED_

#pragma once

// SoftBordersDlg.h : header file
//

#include "Dib.h"

/////////////////////////////////////////////////////////////////////////////
// CSoftBordersDlg dialog

class CSoftBordersDlg : public CDialog
{
// Construction
public:
	CSoftBordersDlg(CWnd* pParent);
	void Close();

// Dialog Data
	//{{AFX_DATA(CSoftBordersDlg)
	enum { IDD = IDD_SOFT_BORDERS };
	BOOL	m_bBlur;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSoftBordersDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	void DoIt();
	void Undo();
	CDib m_PreviewUndoDib;

	// Generated message map functions
	//{{AFX_MSG(CSoftBordersDlg)
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnCheckBlur();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOFTBORDERSDLG_H__FD749E98_D020_4588_9900_587E28CE3821__INCLUDED_)
