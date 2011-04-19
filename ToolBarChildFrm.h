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

#define PLAY_SLIDER_WIDTH_MIN						80
#define MIN_ZOOM_COMBOBOX_INDEX						0
#define MAX_ZOOM_COMBOBOX_INDEX						10
#define ZOOMCOMBOBOX_WIDTH							92

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

/////////////////////////////////////////////////////////////////////////////
// CVideoAviToolBar

class CVideoAviToolBar : public CChildToolBar
{
public:
	CVideoAviToolBar();
	virtual ~CVideoAviToolBar();
	virtual void UpdateControls(void);
	BOOL Create(CWnd* pParentWnd);
	BOOL Create(CWnd* pParentWnd, BOOL bFullScreen, int nMaxWidth);
	CToolbarSliderCtrl m_PlayerSlider;

protected:	
	int m_PlayerSliderIndex;
	int m_PlaySliderWidth;
	
	// Overrides
protected:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoAviToolBar)
	//}}AFX_VIRTUAL

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

/////////////////////////////////////////////////////////////////////////////
// CVideoDeviceToolBar

class CVideoDeviceToolBar : public CChildToolBar
{
public:
	CVideoDeviceToolBar();
	virtual ~CVideoDeviceToolBar();
	virtual void UpdateControls(void);
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
	afx_msg BOOL OnSelEndOk();
	afx_msg void OnSelendcancel();
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
	BOOL Create(CWnd* pParentWnd);
	CZoomComboBox m_ZoomComboBox;
	CColorButtonPicker m_BkgColorButtonPicker;

protected:
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
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CToolBarChildFrame)
};

/////////////////////////////////////////////////////////////////////////////
// CAudioMCIChildFrame

class CAudioMCIChildFrame : public CChildFrame
{
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

	// Generated message map functions
protected:
	//{{AFX_MSG(CAudioMCIChildFrame)
	afx_msg void OnClose();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CAudioMCIChildFrame)
};

/////////////////////////////////////////////////////////////////////////////
// CCDAudioChildFrame

class CCDAudioChildFrame : public CChildFrame
{
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

	// Generated message map functions
protected:
	//{{AFX_MSG(CCDAudioChildFrame)
	afx_msg void OnClose();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CCDAudioChildFrame)
};

/////////////////////////////////////////////////////////////////////////////
// CVideoAviChildFrame

class CVideoAviChildFrame : public CToolBarChildFrame
{
protected:
	CVideoAviChildFrame();           // protected constructor used by dynamic creation
	void StartShutdown1();
	void StartShutdown2();
	void EndShutdown();
	BOOL IsShutdown1Done();
	BOOL IsShutdown2Done();

protected:
	BOOL m_bShutdown2Started;

	// Overrides
protected:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoAviChildFrame)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CVideoAviChildFrame)
	afx_msg void OnClose();
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CVideoAviChildFrame)
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
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT nIDEvent);
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
// CBigPictureChildFrame

class CBigPictureChildFrame : public CPictureChildFrame
{
protected:
	CBigPictureChildFrame();           // protected constructor used by dynamic creation

	// Overrides
protected:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBigPictureChildFrame)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CBigPictureChildFrame)
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CBigPictureChildFrame)
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLBARCHILDFRAME__INCLUDED_)
