#if !defined(AFX_TOOLBARCHILDFRAME__INCLUDED_)
#define AFX_TOOLBARCHILDFRAME__INCLUDED_

#pragma once

// ToolBarChildFrm.h : header file
//

#include "ChildFrm.h"
#include "uxtheme.h"
#include "ColorButtonPicker.h"

#define MIN_ZOOM_COMBOBOX_INDEX						0
#define MAX_ZOOM_COMBOBOX_INDEX						10

/////////////////////////////////////////////////////////////////////////////
// CChildToolBar

class CChildToolBar : public CToolBar
{
public:
	CChildToolBar();
	virtual ~CChildToolBar();
	virtual void UpdateControls(void) = 0;
	virtual BOOL Create(CWnd* pParentWnd);
	int m_nMinToolbarWidth;
	int m_nMaxToolbarWidth;

protected:
	BOOL IsThemed();
	HTHEME m_hTheme;

	// Overrides
protected:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildToolBar)
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CChildToolBar)
	afx_msg void OnNcPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CChildToolBar)
};

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CVideoDeviceToolBar

class CVideoDeviceToolBar : public CChildToolBar
{
public:
	CVideoDeviceToolBar();
	virtual ~CVideoDeviceToolBar();
	virtual void UpdateControls(void);
	BOOL SwitchToolBar(int nDPI, BOOL bCallShowControlBar = TRUE);
	BOOL Create(CWnd* pParentWnd);

	// Overrides
protected:		
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoDeviceToolBar)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CVideoDeviceToolBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CVideoDeviceToolBar)
};

#endif

/////////////////////////////////////////////////////////////////////////////
// CZoomComboBox

class CZoomComboBox : public CComboBox
{
public:
	CZoomComboBox();
	void Init();
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	void OnChangeZoomFactor(double dZoomFactor);
	int GetNextZoomIndex(double dZoomFactor);
	int GetPrevZoomIndex(double dZoomFactor);

	// Generated message map functions
protected:
	//{{AFX_MSG(CZoomComboBox)
	afx_msg void OnSelEndOk();
	afx_msg void OnCloseUp();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CPictureToolBar

class CPictureToolBar : public CChildToolBar
{
public:
	CPictureToolBar();
	virtual ~CPictureToolBar();
	virtual void UpdateControls(void);
	BOOL SwitchToolBar(int nDPI, BOOL bCallShowControlBar = TRUE);
	int ScaleToolBar(int nDPI, int n);
	BOOL Create(CWnd* pParentWnd);
	CZoomComboBox m_ZoomComboBox;
	CColorButtonPicker m_BkgColorButtonPicker;

protected:
	CFont m_ZoomComboBoxFont;
	int m_ZoomComboBoxIndex;
	CRect m_rcLastZoomComboBox;
	int m_BkgColorButtonPickerIndex;
	CRect m_rcLastBkgColorButtonPicker;

	// Overrides
protected:		
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPictureToolBar)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CPictureToolBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg LONG OnSelEndOK(UINT lParam, LONG wParam);

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CPictureToolBar)
};

/////////////////////////////////////////////////////////////////////////////
// CToolBarChildFrame

class CToolBarChildFrame : public CChildFrame
{
protected:
	CToolBarChildFrame();			// protected constructor used by dynamic creation

public:
	CChildToolBar* m_pToolBar;
	void SetToolBar(CChildToolBar* pToolBar) {m_pToolBar = pToolBar;};
	__forceinline CChildToolBar* GetToolBar() {return m_pToolBar;};

	// Overrides
protected:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToolBarChildFrame)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CToolBarChildFrame)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CToolBarChildFrame)
};

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CVideoDeviceChildFrame

class CVideoDeviceChildFrame : public CToolBarChildFrame
{
protected:
	CVideoDeviceChildFrame(); // protected constructor used by dynamic creation
	void StartShutdown1();
	void StartShutdown2();
	void StartShutdown3();
	void EndShutdown();
	BOOL IsShutdown1Done();
	BOOL IsShutdown2Done();
	BOOL IsShutdown3Done();

protected:
	BOOL m_bShutdown2Started;
	BOOL m_bShutdown3Started;

	// Overrides
protected:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoDeviceChildFrame)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CVideoDeviceChildFrame)
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnToolbarDropDown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClose();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CVideoDeviceChildFrame)
};

#endif

/////////////////////////////////////////////////////////////////////////////
// CPictureChildFrame

class CPictureChildFrame : public CToolBarChildFrame
{
protected:
	CPictureChildFrame();           // protected constructor used by dynamic creation
	void StartShutdown();
	BOOL IsShutdownDone();

	// Overrides
protected:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPictureChildFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CPictureChildFrame)
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CPictureChildFrame)
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLBARCHILDFRAME__INCLUDED_)
