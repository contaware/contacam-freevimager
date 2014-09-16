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
	BOOL DxDraw(DWORD dwCurrentUpTime, const CString& sOSDMessage, COLORREF crOSDMessageColor);

protected:
	DECLARE_DYNCREATE(CVideoDeviceView)
	BOOL InitDxDraw(int nWidth, int nHeight);
	void DxDrawText(int nWidth, int nHeight, const CString& sOSDMessage, COLORREF crOSDMessageColor);
	__forceinline void DxDrawZoneSensitivity(int i, HDC hDC, const RECT& rcDetZone, int n);
	void DxDrawZones(int nWidth, int nHeight);
	__forceinline void EraseDxDrawBkgnd(BOOL bFullErase);
	BOOL ReOpenDxDevice();
	BYTE m_MovDetSingleZoneSensitivity;
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
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEditSelectall();
	afx_msg void OnEditSelectnone();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnEditZoneSensitivity100();
	afx_msg void OnEditZoneSensitivity50();
	afx_msg void OnEditZoneSensitivity25();
	afx_msg void OnEditZoneSensitivity10();
	afx_msg void OnUpdateEditZoneSensitivity100(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditZoneSensitivity50(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditZoneSensitivity25(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditZoneSensitivity10(CCmdUI* pCmdUI);
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
	volatile DWORD m_dwDxDrawUpTime;
	volatile BOOL m_bDxDrawInitFailed;
	volatile BOOL m_bDxDrawFirstInitOk;
	CFont m_GDIDrawFont;
	CDib* volatile m_pDxDrawDib; // DirectDraw Helper Dib
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
