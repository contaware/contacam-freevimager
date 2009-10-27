// XButtonXP.h  Version 1.2
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// This software is released into the public domain.  You are free to use
// it in any way you like, except that you may not sell this source code.
//
// This software is provided "as is" with no expressed or implied warranty.
// I accept no liability for any damage or loss of business that this
// software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef  XBUTTONXP_H 
#define  XBUTTONXP_H 

#include "uxtheme.h"
#include "OddButton.h"

/////////////////////////////////////////////////////////////////////////////
// CXButtonXP window

#ifndef DFCS_HOT
#define DFCS_HOT                0x1000
#endif

class CXButtonXP : public COddButton
{
// Construction
public:
	CXButtonXP();
	virtual ~CXButtonXP();

// Attributes
public:
	enum ICON_ALIGNMENT
	{
		CENTER = 0,
		LEFT,
		RIGHT
	};

	void SetIconAlignment(ICON_ALIGNMENT ia) 
	{ 
		m_eIconAlignment = ia; 
		RedrawWindow();
	}
	BOOL GetToggle() { return m_bIsToggle; }
	void SetToggle(BOOL bIsToggle) 
	{ 
		m_bIsToggle = bIsToggle;
		m_bToggled  = FALSE;
		RedrawWindow();
	}
	BOOL GetToggleState() { return m_bToggled; }
	void SetToggleState(BOOL bToggled) { m_bToggled = bToggled; RedrawWindow(); }
	void EnableTheming(BOOL bEnable) 
	{ 
		m_bEnableTheming = bEnable; 
		if (m_bEnableTheming)
		{
			TRACE(_T("Theming will be enabled only on XP and only ")
				  _T("if the app is themed with a manifest file.\n"));
		}
		RedrawWindow();
	}
	BOOL IsThemed();
	BOOL GetDrawToolbar() { return m_bDrawToolbar; }
	void SetDrawToolbar(BOOL bDrawToolbar) 
	{ 
		m_bDrawToolbar = bDrawToolbar; 
		m_bFirstTime = TRUE;	// force OpenThemeData() call
		RedrawWindow();
	}


// Operations
public:
	void SetIcon(UINT nIDResource, ICON_ALIGNMENT ia = LEFT);
	void SetIcon(HICON hIcon, ICON_ALIGNMENT ia = LEFT);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXButtonXP)
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void DrawIcon(CDC *pDC, 
				  BOOL bHasText, 
				  RECT& rectItem, 
				  RECT& rectText, 
				  BOOL bIsPressed, 
				  BOOL bIsDisabled);

	void PrepareImageRect(BOOL bHasText, 
						  RECT& rectItem, 
						  RECT& rectText, 
						  BOOL bIsPressed, 
						  DWORD dwWidth, 
						  DWORD dwHeight, 
						  RECT& rectImage);


	BOOL			m_bFirstTime;
	BOOL			m_bTracking;
	BOOL			m_bMouseOverButton;
	HTHEME			m_hTheme;
	HICON			m_hIcon;
	ICON_ALIGNMENT	m_eIconAlignment;
	BOOL			m_bToggled;
	BOOL			m_bIsToggle;
	BOOL			m_bLButtonDown;
	BOOL			m_bSent;
	BOOL			m_bEnableTheming;
	BOOL			m_bDrawToolbar;

	// Generated message map functions
	//{{AFX_MSG(CXButtonXP)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnMouseHover(WPARAM wparam, LPARAM lparam);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // XBUTTONXP_H 
