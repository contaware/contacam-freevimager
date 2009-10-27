#if !defined(AFX_OCRLANGUAGEDLG_H__6B770A9D_24EC_459A_B552_00F2583F1CAA__INCLUDED_)
#define AFX_OCRLANGUAGEDLG_H__6B770A9D_24EC_459A_B552_00F2583F1CAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OCRLanguageDlg.h : header file
//

#include "MDIVWCTL.tlh"

/////////////////////////////////////////////////////////////////////////////
// COCRLanguageDlg dialog

class COCRLanguageDlg : public CDialog
{
// Construction
public:
	COCRLanguageDlg(CWnd* pParent = NULL);   // standard constructor
	enum MiLANGUAGES m_lLangId;
// Dialog Data
	//{{AFX_DATA(COCRLanguageDlg)
	enum { IDD = IDD_OCR_LANGUAGE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COCRLanguageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SelToLangId();
	// Generated message map functions
	//{{AFX_MSG(COCRLanguageDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OCRLANGUAGEDLG_H__6B770A9D_24EC_459A_B552_00F2583F1CAA__INCLUDED_)
