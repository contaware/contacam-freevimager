#if !defined(AFX_PICTUREVIEW_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_)
#define AFX_PICTUREVIEW_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ToolBarChildFrm.h"
#include "PictureDoc.h"
#include "uImagerView.h"

// Window Message IDs
#define WM_THREADSAFE_SLIDESHOW_LOAD_PICTURE	WM_USER + 125
#define WM_THREADSAFE_PLAYFIRST_BACKGROUNDMUSIC	WM_USER + 126
#define WM_THREADSAFE_STOP_BACKGROUNDMUSIC		WM_USER + 127
#define WM_THREADSAFE_UPDATEIMAGEINFO			WM_USER + 128
#define WM_THREADSAFE_RUNSLIDESHOW				WM_USER + 129
#define WM_THREADSAFE_PAUSESLIDESHOW			WM_USER + 130

// Message Delay in ms
#define THREAD_SAFE_UPDATEIMAGEINFO_DELAY		300

// Scroll Size For Mouse Wheel
#define SCROLLWHEEL_STEPS						12

// Visible Doc Drawing Safety Margin
#define VISIBLEDOC_INFLATE						32

// Crop Defines
#define CROP_MEMDC_USE_MAX_SIZE					4000
#define CROP_RECT_X_INSIDE						6
#define CROP_RECT_Y_INSIDE						6
#define CROP_RECT_X_OUTSIDE						14
#define CROP_RECT_Y_OUTSIDE						14
#define CROP_MARKER_RECT_WIDTH					10
#define CROP_MARKER_RECT_HEIGHT					10

// Crop Autoscroll Defines
#define AUTOSCROLL_BORDER						2
#define AUTOSCROLL_UP							1
#define AUTOSCROLL_DOWN							2
#define AUTOSCROLL_LEFT							4
#define AUTOSCROLL_RIGHT						8
#define AUTOSCROLL_STEPS						16

// Forward Declaration
class CPicturePrintPreviewView;

// The Picture View Class
class CPictureView : public CUImagerView
{
public:
	CPictureView();

	// Overriden here to add the
	// update of the m_CropZoomRect,
	// call also base implementation.
	virtual void UpdateZoomRect();

	// Draw function called by OnDraw()
	BOOL Draw(CDC* pDC = NULL);

	// Kill the Autoscroll Timer
	// (for Crop)
	void StopAutoScroll();

	// Zoom the passed point,
	// for ZoomTool and Mouse Wheel Zoom.
	// bZoomOut = TRUE: Zoom-Out
	// bZoomIn = FALSE: Zoom-In
	void ZoomPoint(CPoint point, BOOL bZoomOut);

	// Zoom In/Out for the +/- Keys
	void Zoom(BOOL bZoomOut); 

	// Calculate the Zoomed Pixel Align From the Zoom Factor and Doc Pixel Align
	void CalcZoomedPixelAlign();

	// Get the Print Preview View
	CPicturePrintPreviewView* GetPicturePrintPreviewView();
	
	// On End Printing
	void OnEndPrintingFromPrintPreview(CDC* pDC, CPrintInfo* pInfo);

	// Autoscroll Vars
	// (for Crop)
	int m_nAutoScroll;
	UINT m_uiAutoScrollTimerId;

	// These are the PixelAligns of the Document Class
	// multiplied by the Zoom Factor
	int m_nZoomedPixelAlignX;
	int m_nZoomedPixelAlignY;

	// The Print Rectangle to where the Dib has to be stretched
	CRect m_rcDibPrint;

	// Last right-click point
	CPoint m_ptLastRightClick;

	// Free Crop Mem DC & DibSection
	void FreeCropMemDCDrawing();

	// Crop Status Text
	void UpdateCropStatusText();

protected:
	DECLARE_DYNCREATE(CPictureView)
	virtual ~CPictureView();

	// Overriden here,
	// call also base implementation.
	virtual void UpdateScrollSize();

	// Scroll Functions
	CRect GetVisibleDocRect(CRect rcDocRect, double dZoomFactor);
	CRect GetVisibleZoomRect(CRect rcVisibleDocRect, double dZoomFactor);
	
	// Erase Bkg
	void EraseBkgnd(CDC* pDC);

	// Transition Drawing
	void DrawTransition(CDC* pDC, CDib* pDib);

	// Load Full Jpeg Transition
	void DrawLoadFullJpegTransition(CDC* pDC);

	// Set cursor position and send also a WM_MOUSEMOVE
	// - cursor position is kept inside the client area
	// - point is in view coordinates
	void SetCursorPosInsideClientRect(CPoint point);

	// Crop Functions
	void DrawCropTools(CDC* pDC);
	void DrawCropBkgImage(CDC* pDC, BOOL bUseMemDC);
	void CropLeft(CPoint point, int nHalfRectSizeX, BOOL bCropMaintainAspectRatio);
	void CropTop(CPoint point, int nHalfRectSizeY, BOOL bCropMaintainAspectRatio);
	void CropRight(CPoint point, int nHalfRectSizeX, BOOL bCropMaintainAspectRatio);
	void CropBottom(CPoint point, int nHalfRectSizeY, BOOL bCropMaintainAspectRatio);
	void CropCenter(CPoint delta, int nRectSizeXInside, int nRectSizeYInside);
	void CropClipLeft(int nPixelAlignX);
	void CropClipRight(int nPixelAlignX);
	void CropClipTop(int nPixelAlignY);
	void CropClipBottom(int nPixelAlignY);
	void DoAutoScroll();
	void UpdateCropRectangles();
	void StepLeftCropEdge(BOOL bDirectionLeft, BOOL bCursorTop);
	void StepRightCropEdge(BOOL bDirectionLeft, BOOL bCursorTop);
	void StepTopCropEdge(BOOL bDirectionUp, BOOL bCursorLeft);
	void StepBottomCropEdge(BOOL bDirectionUp, BOOL bCursorLeft);
	void StepCropCenter(CPoint delta);

	// Print Fit
	CRect PrintFit(int nWidth, int nHeight, double dXDpiRatio, double dYDpiRatio);

	// Scroll Move Vars
	CPoint m_ptStartScrollMovePos;
	CPoint m_ptStartScrollMoveClickPos;
	BOOL m_bDoScrollMove;

	// Crop Vars
	CRect m_CropZoomRect;
	BOOL m_bCropTopLeft;
	BOOL m_bCropBottomRight;
	BOOL m_bCropBottomLeft;
	BOOL m_bCropTopRight;
	BOOL m_bCropTop;
	BOOL m_bCropBottom;
	BOOL m_bCropLeft;
	BOOL m_bCropRight;
	BOOL m_bCropCenter;
	BOOL m_bCropDrag;
	CPoint m_ptCropClick;
	double m_dCropAspectRatio;
	BOOL m_bCropMaintainAspectRatio;
	int m_nAspectRatioPos;
	HCURSOR m_hCropCursor;
	BOOL m_bCropMouseCaptured;

	// Transition Vars
	volatile int m_nTransitionStep;
	int m_nCurrentTransition;
	CDib m_TransitionDib;
	HDC volatile m_hTransitionMemDC;
	HBITMAP volatile m_hTransitionOldBitmap;

	// Load Full Jpeg Transition Vars
	volatile int m_nLoadFullJpegTransitionStep;
	CDib m_LoadFullJpegTransitionDib;
	HDC volatile m_hLoadFullJpegTransitionMemDC;
	HBITMAP volatile m_hLoadFullJpegTransitionOldBitmap;

	// Mem DC Drawing Vars
	CDC m_MemDC;
	HBITMAP m_hMemDCDibSection;
	HBITMAP m_hOldMemDCBitmap;
	CRect m_rcPrevClient;
	LPBYTE m_pDibSectionBits;

	// The Toolbar
	CPictureToolBar m_PictureToolBar;

	// Used by OnMouseWheel() and DoAutoScroll()
	// for the Crop Autoscroll and the Crop Wheelscroll
	UINT m_uiOnMouseMoveLastFlag;
	CPoint m_OnMouseMoveLastPoint;

	// To Print Characters
	double m_dXFontPixelScale;
	double m_dYFontPixelScale;

	// Zoom Tool Flag
	BOOL m_bOldZoomToolMinus;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPictureView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnDraw(CDC* pDC);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnEndPrintPreview(CDC* pDC, CPrintInfo* pInfo, POINT point, CPreviewView* pView);
	//}}AFX_VIRTUAL

// Implementation
public:
	CPictureDoc* GetDocument();
	void OnPrintCall(CDC* pDC, CPrintInfo* pInfo) {OnPrint( pDC, pInfo);};
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CPictureView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintDirect();
	afx_msg void OnFilePrintPreview();
	afx_msg void OnFilePrintSetup();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnViewZoomTool();
	afx_msg void OnUpdateViewZoomTool(CCmdUI* pCmdUI);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnUpdateFilePrintPreview(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFilePrint(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFilePrintDirect(CCmdUI* pCmdUI);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnViewFullscreen();
	afx_msg void OnUpdateViewFullscreen(CCmdUI* pCmdUI);
	afx_msg void OnViewZoomFit();
	afx_msg void OnUpdateViewZoomFit(CCmdUI* pCmdUI);
	afx_msg void OnViewZoomFitbig();
	afx_msg void OnUpdateViewZoomFitbig(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom625();
	afx_msg void OnUpdateViewZoom625(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom125();
	afx_msg void OnUpdateViewZoom125(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom25();
	afx_msg void OnUpdateViewZoom25(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom50();
	afx_msg void OnUpdateViewZoom50(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom100();
	afx_msg void OnUpdateViewZoom100(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom200();
	afx_msg void OnUpdateViewZoom200(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom400();
	afx_msg void OnUpdateViewZoom400(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom800();
	afx_msg void OnUpdateViewZoom800(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom1600();
	afx_msg void OnUpdateViewZoom1600(CCmdUI* pCmdUI);
	afx_msg void OnViewNextMonitor();
	afx_msg void OnViewPreviousMonitor();
	//}}AFX_MSG
	afx_msg LONG OnThreadSafeSlideshowLoadPicture(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadUpdateImageInfo(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafePlayFirstBackgroundMusic(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafeStopBackgroundMusic(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafeRunSlideshow(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafePauseSlideshow(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnRecursiveFileFindDone(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnBackgroundMusicTrackDone(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnColorPicked(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnColorPickerClosed(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnApplicationCommand(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG
inline CPictureDoc* CPictureView::GetDocument()
   { return (CPictureDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PICTUREVIEW_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_)
