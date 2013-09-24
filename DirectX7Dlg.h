#if !defined(AFX_DIRECTX7DLG_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_)
#define AFX_DIRECTX7DLG_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_

#pragma once

// DirectX7Dlg.h : header file
//

#include "Staticlink.h"

/////////////////////////////////////////////////////////////////////////////
// CDirectX7Dlg dialog

class CDirectX7Dlg : public CDialog
{
// Construction
public:
	CDirectX7Dlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDirectX7Dlg)
	enum { IDD = IDD_DIRECTX7 };
	CString	m_sTextRow1;
	CString	m_sTextRow2;
	CString	m_sTextRow3;
	//}}AFX_DATA
	CString	m_sTextLink;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDirectX7Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CStaticLink m_TextLink;
	// Generated message map functions
	//{{AFX_MSG(CDirectX7Dlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIRECTX7DLG_H__9226541B_3040_4454_97FC_14E326FA7043__INCLUDED_)
