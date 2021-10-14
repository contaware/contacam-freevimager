#if !defined(AFX_VIDEODEVICEVIEW_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_)
#define AFX_VIDEODEVICEVIEW_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_

#pragma once

#ifdef VIDEODEVICEDOC

#include "ToolBarChildFrm.h"
#include "uImagerView.h"
#include "VideoDeviceDoc.h"

// Window Message IDs
#define WM_THREADSAFE_CAPTURECAMERABASICSETTINGS	WM_USER + 600
#define WM_THREADSAFE_INIT_MOVDET					WM_USER + 601
#define WM_THREADSAFE_DVCHANGEVIDEOFORMAT			WM_USER + 602
#define WM_THREADSAFE_UPDATE_PHPPARAMS				WM_USER + 603
		
class CVideoDeviceView : public CUImagerView
{
public:
	CVideoDeviceView();
	virtual ~CVideoDeviceView();
	CVideoDeviceDoc* GetDocument();

protected:
	DECLARE_DYNCREATE(CVideoDeviceView)
	__forceinline void DrawRecDot(HDC hDC, const CRect& rcClient);
	__forceinline void DrawSaveProgress(HDC hDC, const CRect& rcClient, BOOL bRecDotVisible, int nProgress);
	__forceinline void DrawZoneSensitivity(int i, HDC hDC, const RECT& rcDetZone, int n, int m, int s, HBRUSH hBkgndBrush);
	void DrawZones(HDC hDC, const CRect& rcClient);
	BOOL ReOpenDxDevice();
	BYTE m_MovDetSingleZoneSensitivity;
	BOOL m_bMovDetUnsupportedVideoSize;
	BOOL m_bMovDetUnsupportedZonesSize;
	CVideoDeviceToolBar m_VideoDeviceToolBar;

// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoDeviceView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Debug
#ifdef _DEBUG
public:
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	
// Generated message map functions
protected:
	//{{AFX_MSG(CVideoDeviceView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnEditAllZones();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnFrameTimeFontSize4();
	afx_msg void OnUpdateFrameTimeFontSize4(CCmdUI* pCmdUI);
	afx_msg void OnFrameTimeFontSize5();
	afx_msg void OnUpdateFrameTimeFontSize5(CCmdUI* pCmdUI);
	afx_msg void OnFrameTimeFontSize6();
	afx_msg void OnUpdateFrameTimeFontSize6(CCmdUI* pCmdUI);
	afx_msg void OnFrameTimeFontSize7();
	afx_msg void OnUpdateFrameTimeFontSize7(CCmdUI* pCmdUI);
	afx_msg void OnFrameTimeFontSize8();
	afx_msg void OnUpdateFrameTimeFontSize8(CCmdUI* pCmdUI);
	afx_msg void OnFrameTimeFontSize9();
	afx_msg void OnUpdateFrameTimeFontSize9(CCmdUI* pCmdUI);
	afx_msg void OnFrameTimeFontSize10();
	afx_msg void OnUpdateFrameTimeFontSize10(CCmdUI* pCmdUI);
	afx_msg void OnFrameTimeFontSize11();
	afx_msg void OnUpdateFrameTimeFontSize11(CCmdUI* pCmdUI);
	afx_msg void OnFrameTimeFontSize12();
	afx_msg void OnUpdateFrameTimeFontSize12(CCmdUI* pCmdUI);
	afx_msg void OnFrameTimeFontSize14();
	afx_msg void OnUpdateFrameTimeFontSize14(CCmdUI* pCmdUI);
	afx_msg void OnFrameTimeFontSize16();
	afx_msg void OnUpdateFrameTimeFontSize16(CCmdUI* pCmdUI);
	afx_msg void OnViewFullscreen();
	afx_msg void OnUpdateViewFullscreen(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg LONG OnThreadSafeCaptureCameraBasicSettings(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafeUpdatePhpParams(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafeDVChangeVideoFormat(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafeInitMovDet(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnDirectShowGraphNotify(WPARAM wparam, LPARAM lparam);
	DECLARE_MESSAGE_MAP()

protected:
	CFont m_GDIDrawFont;
};

#ifndef _DEBUG
inline CVideoDeviceDoc* CVideoDeviceView::GetDocument()
   { return (CVideoDeviceDoc*)m_pDocument; }
#endif

#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDEODEVICEVIEW_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_)
