#if !defined(AFX_DECREASEBPPDLG_H__28383D87_CC27_4CED_80D6_26AF94AA4584__INCLUDED_)
#define AFX_DECREASEBPPDLG_H__28383D87_CC27_4CED_80D6_26AF94AA4584__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DecreaseBppDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDecreaseBppDlg dialog

class CDecreaseBppDlg : public CDialog
{
// Construction
public:
	CDecreaseBppDlg(UINT uiMaxColorsMin, UINT uiMaxColorsMax, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CDecreaseBppDlg)
	enum { IDD = IDD_DECREASE_BPP };
	UINT	m_uiMaxColors;
	BOOL	m_bDitherColorConversion;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDecreaseBppDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	UINT m_uiMaxColorsMin;
	UINT m_uiMaxColorsMax;

	// Generated message map functions
	//{{AFX_MSG(CDecreaseBppDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DECREASEBPPDLG_H__28383D87_CC27_4CED_80D6_26AF94AA4584__INCLUDED_)
