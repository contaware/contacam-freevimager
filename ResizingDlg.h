#if !defined(AFX_RESIZINGDLG_H__F1B48863_D6E1_4937_9740_2CDE8D26762F__INCLUDED_)
#define AFX_RESIZINGDLG_H__F1B48863_D6E1_4937_9740_2CDE8D26762F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResizingDlg.h : header file
//

#ifdef VIDEODEVICEDOC

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CResizingDlg dialog

class CResizingDlg : public CDialog
{
// Construction
public:
	CResizingDlg(	int nFullWidth, int nFullHeight,
					int nCurrentWidth, int nCurrentHeight,
					CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CResizingDlg)
	enum { IDD = IDD_RESIZING };
	int		m_nPixelsWidth;
	int		m_nPixelsHeight;
	int		m_nPercentWidth;
	int		m_nPercentHeight;
	BOOL	m_bRetainAspect;
	//}}AFX_DATA

	int m_nFullWidth;
	int m_nFullHeight;
	double m_dFullRatio;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResizingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL m_bDlgInitialized;
	BOOL m_bNormalValidate;
	double m_dMaxResizeFactor;

	// Generated message map functions
	//{{AFX_MSG(CResizingDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditPercentHeight();
	afx_msg void OnChangeEditPercentWidth();
	afx_msg void OnChangeEditPixelsHeight();
	afx_msg void OnChangeEditPixelsWidth();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESIZINGDLG_H__F1B48863_D6E1_4937_9740_2CDE8D26762F__INCLUDED_)
