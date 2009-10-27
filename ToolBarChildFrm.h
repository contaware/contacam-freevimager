#if !defined(AFX_TOOLBARCHILDFRAME__INCLUDED_)
#define AFX_TOOLBARCHILDFRAME__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ToolBarChildFrm.h : header file
//

#include "ChildFrm.h"
#include "uxtheme.h"
#include "ColorButtonPicker.h"
#include "ToolbarSliderCtrl.h"

/*
When a toolbar is created in the Visual Studio toolbar editor,
any pixel that is light gray (RGB (192, 192, 192))
is replaced with the user's chosen button color
(::GetSysColor (COLOR_BTNFACE))at runtime,
making those pixels effectively transparent.
*/

#define TOOLBAR_BORDER_LEFT							2
#define TOOLBAR_BORDER_TOP							2
#define TOOLBAR_BORDER_RIGHT						2
#define TOOLBAR_BORDER_BOTTOM						1

#define PLAY_SLIDER_WIDTH_MIN						80

#define MIN_ZOOM_COMBOBOX_INDEX						0
#define MAX_ZOOM_COMBOBOX_INDEX						10

class CChildToolBar : public CToolBar
{
public:
	CChildToolBar();
	virtual BOOL Create(CWnd* pParentWnd);
	virtual void UpdateControls(void) = 0;

// Operations
public:
	int m_nMinToolbarWidth;
	int m_nMaxToolbarWidth;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildToolBar)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CChildToolBar)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CChildToolBar)
};

class CVideoAviToolBar : public CChildToolBar
{
// Construction
public:
	BOOL Create(CWnd* pParentWnd);
	BOOL Create(CWnd* pParentWnd, BOOL bFullScreen, int nMaxWidth);
	CToolbarSliderCtrl m_PlayerSlider;

protected:	
	int m_PlayerSliderIndex;
	int m_PlaySliderWidth;
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoAviToolBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	CVideoAviToolBar();
	virtual ~CVideoAviToolBar();
	virtual void UpdateControls(void);

	// Generated message map functions
protected:
	//{{AFX_MSG(CVideoAviToolBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CVideoAviToolBar)
};

#ifdef VIDEODEVICEDOC

class CVideoDeviceToolBar : public CChildToolBar
{
// Construction
public:
	BOOL Create(CWnd* pParentWnd);

protected:		
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoDeviceToolBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	CVideoDeviceToolBar();
	virtual ~CVideoDeviceToolBar();
	virtual void UpdateControls(void);

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

	//{{AFX_MSG(CZoomComboBox)
	afx_msg BOOL OnSelEndOk();
	afx_msg void OnSelendcancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CPictureToolBar : public CChildToolBar
{
// Construction
public:
	BOOL Create(CWnd* pParentWnd);

protected:		
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPictureToolBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	CPictureToolBar();
	virtual ~CPictureToolBar();
	virtual void UpdateControls(void);
	CZoomComboBox m_ZoomComboBox;
	CColorButtonPicker m_BkgColorButtonPicker;

	// Generated message map functions
protected:
	int m_ZoomComboBoxIndex;
	CRect m_rcLastZoomComboBox;
	int m_BkgColorButtonPickerIndex;
	CRect m_rcLastBkgColorButtonPicker;

	//{{AFX_MSG(CPictureToolBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg LONG OnSelEndOK(UINT lParam, LONG wParam);

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CPictureToolBar)
};

/////////////////////////////////////////////////////////////////////////////
// CToolBarChildFrame frame

class CToolBarChildFrame : public CChildFrame
{
	DECLARE_DYNCREATE(CToolBarChildFrame)

protected:
	CToolBarChildFrame();
	virtual ~CToolBarChildFrame();
	BOOL IsThemed();
	HTHEME m_hTheme;

// Attributes
public:
	CChildToolBar* m_pToolBar;
	void SetToolBar(CChildToolBar* pToolBar);
	CChildToolBar* GetToolBar();
	
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToolBarChildFrame)
	protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CToolBarChildFrame)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CAudioMCIChildFrame frame
class CAudioMCIChildFrame : public CChildFrame
{
	DECLARE_DYNCREATE(CAudioMCIChildFrame)
protected:
	CAudioMCIChildFrame();           // protected constructor used by dynamic creation
	void StartShutdown();
	void EndShutdown();
	BOOL IsShutdownDone();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioMCIChildFrame)
	public:
	virtual void ActivateFrame(int nCmdShow = -1);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CAudioMCIChildFrame)
	afx_msg void OnClose();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CCDAudioChildFrame frame
class CCDAudioChildFrame : public CChildFrame
{
	DECLARE_DYNCREATE(CCDAudioChildFrame)
protected:
	CCDAudioChildFrame();           // protected constructor used by dynamic creation
	void StartShutdown();
	void EndShutdown();
	BOOL IsShutdownDone();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCDAudioChildFrame)
	public:
	virtual void ActivateFrame(int nCmdShow = -1);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CCDAudioChildFrame)
	afx_msg void OnClose();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CVideoAviChildFrame frame
class CVideoAviChildFrame : public CToolBarChildFrame
{
	DECLARE_DYNCREATE(CVideoAviChildFrame)
protected:
	CVideoAviChildFrame();           // protected constructor used by dynamic creation
	void StartShutdown1();
	void StartShutdown2();
	void EndShutdown();
	BOOL IsShutdown1Done();
	BOOL IsShutdown2Done();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoAviChildFrame)
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL m_bShutdown2Started;
	// Generated message map functions
	//{{AFX_MSG(CVideoAviChildFrame)
	afx_msg void OnClose();
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CVideoDeviceChildFrame frame
#ifdef VIDEODEVICEDOC
class CVideoDeviceChildFrame : public CToolBarChildFrame
{
	DECLARE_DYNCREATE(CVideoDeviceChildFrame)
protected:
	CVideoDeviceChildFrame(); // protected constructor used by dynamic creation
	void StartShutdown1();
	void StartShutdown2();
	void EndShutdown();
	BOOL IsShutdown1Done();
	BOOL IsShutdown2Done();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoDeviceChildFrame)
	protected:
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL m_bShutdown2Started;
	// Generated message map functions
	//{{AFX_MSG(CVideoDeviceChildFrame)
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};
#endif
/////////////////////////////////////////////////////////////////////////////
// CPictureChildFrame frame

class CPictureChildFrame : public CToolBarChildFrame
{
	DECLARE_DYNCREATE(CPictureChildFrame)

protected:
	CPictureChildFrame();           // protected constructor used by dynamic creation
	void StartShutdown();
	BOOL IsShutdownDone();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPictureChildFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPictureChildFrame)
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CBigPictureChildFrame frame

class CBigPictureChildFrame : public CPictureChildFrame
{
	DECLARE_DYNCREATE(CBigPictureChildFrame)

protected:
	CBigPictureChildFrame();           // protected constructor used by dynamic creation

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBigPictureChildFrame)
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBigPictureChildFrame)
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLBARCHILDFRAME__INCLUDED_)
