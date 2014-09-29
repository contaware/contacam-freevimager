#if !defined(AFX_VIDEOAVIVIEW_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_)
#define AFX_VIDEOAVIVIEW_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_

#pragma once

#include "ToolBarChildFrm.h"
#include "VideoAviDoc.h"
#include "uImagerView.h"

#define USER_RECT_X_INSIDE						110
#define USER_RECT_Y_INSIDE						110
#define USER_RECT_X_OUTSIDE						110
#define USER_RECT_Y_OUTSIDE						110
#define USER_ZOOM_MIN_WIDTH						240
#define USER_ZOOM_MIN_HEIGHT					240
#define MAX_STATISTICS_TEXT_SIZE				128

// Window Message IDs
#define WM_THREADSAFE_LOAD_AVI					WM_USER + 175
#define WM_THREADSAFE_UPDATEPLAYSLIDER			WM_USER + 176
#define WM_END_THUMBTRACK						WM_USER + 177
#define WM_AVIFILE_PROGRESS						WM_USER + 178

// Message Delays in ms
#define THREAD_SAFE_LOAD_AVI_DELAY				500

// End Thumb-Track Retry Delay
// 100 ms * 40 = 4 sec retrying
#define END_TRUMBTRACK_RETRY_DELAY				100
#define END_TRUMBTRACK_RETRY_COUNTDOWN			40

class CVideoAviView : public CUImagerView
{
public:
	void Draw(HDC hDC);
	void EraseBkgnd(HDC hDC, const CRect& ZoomRect);

	// Update Play Slider
	void UpdatePlaySlider();

	// Settings
	void LoadSettings();
	void SaveSettings();

protected: // create from serialization only
	DECLARE_DYNCREATE(CVideoAviView)
	CVideoAviView();
	virtual ~CVideoAviView();
	
	// Drawing Helpers
	__forceinline void DrawInfo(HDC hDC,
								const CRect& rcClient,
								const CRect& ZoomRect);
	__forceinline void FramePosText(TCHAR* sText,
									int nFramePos,
									double dFrameRate,
									bool bKeyFrame,
									LONGLONG llSamplePos);
	__forceinline void SamplePosText(TCHAR* sText,
									 LONGLONG llSamplePos,
									 DWORD dwSampleRate);
	BOOL UpdateCurrentFrame(CAVIPlay::CAVIVideoStream* pVideoStream);

	// Change play speed
	void ChangePlaySpeed(BOOL bIncrement);

	// Video Avi ToolBar
	CVideoAviToolBar m_VideoAviToolBar;

	// User Zoom Rect Vars
	HCURSOR m_hResizeCursor;
	BOOL m_bUserTopLeft;
	BOOL m_bUserBottomRight;
	BOOL m_bUserBottomLeft;
	BOOL m_bUserTopRight;
	BOOL m_bUserTop;
	BOOL m_bUserBottom;
	BOOL m_bUserLeft;
	BOOL m_bUserRight;
	BOOL m_bUserClickClipping;
	CPoint m_ptUserClick;
	CRect m_UserClickZoomRect;

	// Font For Drawing On Display
	BOOL MakeFont(BOOL bBig);
	HFONT m_hFont;
	BOOL m_bBigFont;

	// Slider Thumb Tracking
	BOOL m_bThumbTrack;					// Set and reset in OnHScroll()
	volatile BOOL m_bThumbTrackDone;	// Set when the player has been restarted (if it was running)
										// and reset in OnHScroll() when initing thumb tracking
	volatile int m_nThumbTrackPos;		// Tracking Position in Frames (Samples for Audio-Only AVI)
	int m_nLastThumbTrackPos;			// Thumb Track Pos of last OnTimer() call
	int m_nPreviewThumbTrackPos;		// Thumb Track Pos of last previewed frame 
	BOOL m_bWasPlayingBeforeThumbTrack;	// Do restart player when thumb track is done
	DWORD m_dwThumbTrackSeq;			// Identifies a thumb track init/end pair

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoAviView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	CVideoAviDoc* GetDocument();
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	
// Generated message map functions
protected:
	//{{AFX_MSG(CVideoAviView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnViewFullscreen();
	afx_msg void OnUpdateViewFullscreen(CCmdUI* pCmdUI);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnViewFit();
	afx_msg void OnUpdateViewFit(CCmdUI* pCmdUI);
	afx_msg void OnPlayInc();
	afx_msg void OnUpdatePlayInc(CCmdUI* pCmdUI);
	afx_msg void OnPlayDec();
	afx_msg void OnUpdatePlayDec(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg LONG OnThreadSafeLoadAVI(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafeUpdatePlaySlider(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnAviFileProgress(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnEnableCursor(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnEndThumbTrack(WPARAM wparam, LPARAM lparam);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG
inline CVideoAviDoc* CVideoAviView::GetDocument()
   { return (CVideoAviDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDEOAVIVIEW_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_)
