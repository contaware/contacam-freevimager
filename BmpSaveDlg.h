#if !defined(AFX_BMPSAVEDLG_H__616DEF53_75C7_42CB_A8EA_672D97A17908__INCLUDED_)
#define AFX_BMPSAVEDLG_H__616DEF53_75C7_42CB_A8EA_672D97A17908__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BmpSaveDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBmpSaveDlg dialog

class CBmpSaveDlg : public CDialog
{
// Construction
public:
	CBmpSaveDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBmpSaveDlg)
	enum { IDD = IDD_BMP_SAVE };
	BOOL	m_bRleEncode;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBmpSaveDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBmpSaveDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BMPSAVEDLG_H__616DEF53_75C7_42CB_A8EA_672D97A17908__INCLUDED_)
