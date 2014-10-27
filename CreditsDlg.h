#if !defined(AFX_CREDITSDLG_H__442BFF16_8433_4F58_87D9_BBCA1C196C5F__INCLUDED_)
#define AFX_CREDITSDLG_H__442BFF16_8433_4F58_87D9_BBCA1C196C5F__INCLUDED_

#pragma once

// CreditsDlg.h : header file
//

#include "RichEditCtrl2.h"

/////////////////////////////////////////////////////////////////////////////
// CCreditsDlg dialog

class CCreditsDlg : public CDialog
{
// Construction
public:
	CCreditsDlg(CWnd* pParent = NULL);   // standard constructor
	CRichEditCtrl2 m_Credits;

// Dialog Data
	//{{AFX_DATA(CCreditsDlg)
	enum { IDD = IDD_CREDITS };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreditsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CCreditsDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CREDITSDLG_H__442BFF16_8433_4F58_87D9_BBCA1C196C5F__INCLUDED_)
