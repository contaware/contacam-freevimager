#if !defined(AFX_UIMAGERVIEW_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_)
#define AFX_UIMAGERVIEW_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "uImagerDoc.h"

// Window Message IDs
#define WM_FORCE_CURSOR							WM_USER + 400 // Posted by ForceCursor()
#define WM_ENABLE_CURSOR						WM_USER + 401 // Posted by EnableCursor()
#define WM_THREADSAFE_SETDOCUMENTTITLE			WM_USER + 402
#define WM_THREADSAFE_UPDATEWINDOWSIZES			WM_USER + 403

// The wparam of OnThreadSafeUpdateWindowSizes may be a combination of:
#define UPDATEWINDOWSIZES_INVALIDATE			0x1
#define UPDATEWINDOWSIZES_ERASEBKG				0x2
#define UPDATEWINDOWSIZES_SIZETODOC				0x4

// Message Delays in ms
#define THREAD_SAFE_UPDATEWINDOWSIZES_DELAY		300

// Border in pixels while scrollbars on
#define MIN_BORDER								16

// Border in percent while in fit mode
#define FIT_ZOOMFACTOR_MARGIN					0.95

// In full-screen mode hide the cursor after 3 seconds
#define MOUSE_HIDE_AFTER_SEC					3

// In Full-screen mode show cursor after 8 mouse
// movements in one second
#define MOUSE_HIDE_MOVECOUNT_THRESHOLD			8

// The Base View Class
class CUImagerView : public CScrollView
{
protected: // create from serialization only
	DECLARE_DYNCREATE(CUImagerView)
	CUImagerView();
	virtual ~CUImagerView();

public:
	CUImagerDoc* GetDocument();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUImagerView)
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual void OnDraw(CDC* pDC);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

public:
	// Cursor Visible or Not in Full-Screen Mode?
	void ForceCursor(BOOL bForce = TRUE);
	BOOL IsForceCursor() const {return m_bForceCursor;};
	void EnableCursor(BOOL bEnable = TRUE);
	BOOL IsCursorEnabled() const {return m_bCursor;};

	// ScrollBars Available?
	BOOL IsXAndYScroll();
	BOOL IsXOrYScroll();
	BOOL IsXScroll();
	BOOL IsYScroll();

	// Post a WM_SETCURSOR Message if Cursor inside Client Area
	void UpdateCursor();

	// Updates m_ZoomRect from the client rect,
	// doc rect and the zoom factor.
	virtual void UpdateZoomRect();

	// Update Window Sizes in different ways,
	// depending from the parameters
	BOOL UpdateWindowSizes(	BOOL bInvalidate,
							BOOL bEraseBkg,
							BOOL bSizeToDoc);
	
	// Do Realize Palette only for old 256
	// colors or less Graphic cards
	int DoRealizePalette(BOOL bForceBackground);

	// Calc. the doc's m_dZoomFactor so that the image
	// fits inside the View. Note that this function
	// removes the Scrolls to calculate the right fitting!
	void FitZoomFactor();

	// Calc. the doc's m_dZoomFactor so that big images
	// fit inside the View. Small images are not enlarged!
	// Note that this function
	// removes the Scrolls to calculate the right fitting!
	void FitBigZoomFactor();

	// Update Pane Text
	void UpdatePaneText();

	// Incremented inside OnMouseMove() to avoid
	// showing the mouse pointer in full-screen
	// mode with each small mouse move
	// (implements some sort of noise filter
	// for the mouse movement)
	int m_nMouseMoveCount;

	// Last Point Of Movement
	CPoint m_ptMouseMoveLastPoint;

	// Count-up to hide the cursor after
	// MOUSE_HIDE_AFTER_SEC seconds
	int m_nMouseHideTimerCount;
	
	// The rectangle where the dib is painted
	CRect m_ZoomRect;

	// The Video View Rectangle the user
	// selects in Full-Screen Mode.
	// CRect(0,0,0,0) means use the default m_ZoomRect;
	CRect m_UserZoomRect;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// SetScrollSizes from the client rect,
	// doc rect and the zoom factor
	virtual void UpdateScrollSize();

	// Cursor Visible
	BOOL m_bCursor;

	// Cursor Visible because m_nForceCursorCount > 0
	BOOL m_bForceCursor;

	// ForceCursor(TRUE) increments this var,
	// ForceCursor(FALSE) decrements it
	int m_nForceCursorCount;
	
// Generated message map functions
protected:
	//{{AFX_MSG(CUImagerView)
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnViewFullscreen();
	afx_msg void OnUpdateViewFullscreen(CCmdUI* pCmdUI);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnAppAbout();
	afx_msg void OnAppLicense();
	//}}AFX_MSG
	afx_msg LONG OnThreadSafeUpdateWindowSizes(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafeSetDocumentTitle(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnForceCursor(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnEnableCursor(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnProgress(WPARAM wparam, LPARAM lparam);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in uImagerView.cpp
inline CUImagerDoc* CUImagerView::GetDocument()
   { return (CUImagerDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UIMAGERVIEW_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_)
