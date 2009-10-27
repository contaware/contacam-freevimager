#if !defined(AFX_PNGSAVEDLG_H__FA64F7BE_125E_4E67_9071_B8F82E6B9337__INCLUDED_)
#define AFX_PNGSAVEDLG_H__FA64F7BE_125E_4E67_9071_B8F82E6B9337__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PngSaveDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPngSaveDlg dialog

class CPngSaveDlg : public CDialog
{
// Construction
public:
	CPngSaveDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPngSaveDlg)
	enum { IDD = IDD_PNG_SAVE };
	BOOL	m_bStoreBackgroundColor;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPngSaveDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPngSaveDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PNGSAVEDLG_H__FA64F7BE_125E_4E67_9071_B8F82E6B9337__INCLUDED_)
