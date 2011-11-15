#if !defined(AFX_CREDITSDLG_H__442BFF16_8433_4F58_87D9_BBCA1C196C5F__INCLUDED_)
#define AFX_CREDITSDLG_H__442BFF16_8433_4F58_87D9_BBCA1C196C5F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CreditsDlg.h : header file
//

#include "Staticlink.h"

/////////////////////////////////////////////////////////////////////////////
// CCreditsDlg dialog

class CCreditsDlg : public CDialog
{
// Construction
public:
	CCreditsDlg(CWnd* pParent = NULL);   // standard constructor

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
	CStaticLink m_FfmpegLink;
	CStaticLink m_IjgLink;
	CStaticLink m_LibTiffLink;
	CStaticLink m_LibPngLink;
	CStaticLink m_ZlibLink;
	CStaticLink m_GifLibLink;
	CStaticLink m_IccLibLink;
	CStaticLink m_WinkLink;
	CStaticLink m_NsisLink;

	// Generated message map functions
	//{{AFX_MSG(CCreditsDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CREDITSDLG_H__442BFF16_8433_4F58_87D9_BBCA1C196C5F__INCLUDED_)
