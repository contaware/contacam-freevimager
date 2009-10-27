#if !defined(AFX_BKGCOLEDIT_H__FEF70A48_84EF_4E70_9FA7_46D652532BCE__INCLUDED_)
#define AFX_BKGCOLEDIT_H__FEF70A48_84EF_4E70_9FA7_46D652532BCE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BkgColEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBkgColEdit window

class CBkgColEdit : public CEdit
{
// Construction
public:
	CBkgColEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBkgColEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBkgColEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBkgColEdit)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BKGCOLEDIT_H__FEF70A48_84EF_4E70_9FA7_46D652532BCE__INCLUDED_)
