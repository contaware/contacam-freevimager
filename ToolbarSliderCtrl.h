#if !defined(AFX_TOOLBARSLIDERCTRL_H__021A6935_B2FE_11D3_90FC_0020AFBC499D__INCLUDED_)
#define AFX_TOOLBARSLIDERCTRL_H__021A6935_B2FE_11D3_90FC_0020AFBC499D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ToolbarSliderCtrl.h : header file

#include "uxtheme.h"

/////////////////////////////////////////////////////////////////////////////
// CToolbarSliderCtrl window

class CToolbarSliderCtrl : public CSliderCtrl
{
// Construction
public:
	CToolbarSliderCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToolbarSliderCtrl)
	protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CToolbarSliderCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CToolbarSliderCtrl)
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	BOOL IsThemed();
	HTHEME m_hTheme;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLBARSLIDERCTRL_H__021A6935_B2FE_11D3_90FC_0020AFBC499D__INCLUDED_)
