#if !defined(AFX_VIDEODEVICEVIEW_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_)
#define AFX_VIDEODEVICEVIEW_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef VIDEODEVICEDOC

#include "ToolBarChildFrm.h"
#include "uImagerView.h"
#include "VideoDeviceDoc.h"

// Window Message IDs
#define WM_THREADSAFE_CAPTURESETTINGS				WM_USER + 600
#define WM_THREADSAFE_CHANGEVIDEOFORMAT				WM_USER + 601
#define WM_ENABLE_DISABLE_CRITICAL_CONTROLS			WM_USER + 602
#define WM_THREADSAFE_INIT_MOVDET					WM_USER + 603
#define WM_THREADSAFE_SENDFRAME_MSG					WM_USER + 604
#define WM_THREADSAFE_STOP_AND_CHANGEVIDEOFORMAT	WM_USER + 605
#define WM_THREADSAFE_AUTORUNREMOVEDEVICE_CLOSEDOC	WM_USER + 606
#define WM_THREADSAFE_UPDATE_PHPPARAMS				WM_USER + 607
		
class CVideoDeviceView : public CUImagerView
{
public:
	CVideoDeviceView();
	virtual ~CVideoDeviceView();
	CVideoDeviceDoc* GetDocument();
	BOOL DxDraw(DWORD dwCurrentUpTime, const CString& sOSDMessage, COLORREF crOSDMessageColor);
	BOOL AreCriticalControlsDisabled() {return m_nCriticalControlsCount <= 0;};

protected:
	DECLARE_DYNCREATE(CVideoDeviceView)
	BOOL InitDxDraw(int nWidth, int nHeight, DWORD dwFourCC);
	void DxDrawText(const CString& sOSDMessage, COLORREF crOSDMessageColor);
	void DxDrawZones();
	__forceinline void EraseDxDrawBkgnd(BOOL bFullErase);
	__forceinline BOOL IsDxDrawCompressionDifferent();

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
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEditSelectall();
	afx_msg void OnEditSelectnone();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	afx_msg LONG OnThreadSafeCaptureSettings(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafeUpdatePhpParams(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafeChangeVideoFormat(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafeStopAndChangeVideoFormat(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnEnableDisableCriticalControls(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafeInitMovDet(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafeAutorunRemoveDeviceCloseDoc(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnDirectShowGraphNotify(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafeSendFrameMsg(WPARAM wparam, LPARAM lparam);
	DECLARE_MESSAGE_MAP()

protected:
	volatile DWORD m_dwDxDrawUpTime;
	volatile BOOL m_bDxDrawInitFailed;
	volatile BOOL m_bDxDrawFirstInitOk;
	int m_nCriticalControlsCount;
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
