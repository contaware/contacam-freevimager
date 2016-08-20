#if !defined(AFX_PICTUREPRINTPREVIEWVIEW_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_)
#define AFX_PICTUREVIEW_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_

#pragma once

#include <afxpriv.h>
#include "PictureDoc.h"
#include <..\src\mfc\afximpl.h>

/*
#define ID_PAGES_1PAGE                  32771
#define ID_PAGES_2PAGES                 32772
#define ID_PAGES_3PAGES                 32773
#define ID_PAGES_4PAGES                 32774
#define ID_PAGES_6PAGES                 32775
#define ID_PAGES_9PAGES                 32776
*/

#define PREVIEW_MARGIN		8
#define PREVIEW_PAGEGAP		8
#define MIN_PRINT_SCALE		1.0
#define MAX_PRINT_SCALE		9999.0

class CNumSpinCtrl;
class CPrintPreviewScaleEdit;

// override MFC class
class CPicturePrintPreviewView : public CPreviewView
{
	DECLARE_DYNCREATE(CPicturePrintPreviewView)
public:
	CPictureDoc* GetDocument();
	CDialogBar* GetToolBar() {return m_pToolBar;};
	BOOL IsInchPaperFormat();
	void DisplayPrinterPaperDPI();
	void UpdateStatusText();
	void UpdatePaneText();
	void Close() {OnPreviewClose();};

protected:
				CPicturePrintPreviewView();
	virtual		~CPicturePrintPreviewView();

public:
	CPrintPreviewScaleEdit* m_pScaleEdit;
	CNumSpinCtrl* m_pScaleSpin;

private:
	// these variables are used in the page layout algorithm
	int			m_Across;				// number across the page, >= 1
	int			m_Down;					// number down the page, >= 1
	CPoint		m_PageOffset;			// amount to move the page from one position to the next
	BOOL		m_bGotScrollLines;		// Mouse Wheel Scroll Line Already gotten
	UINT		m_uCachedScrollLines;	// Mouse Wheel Scroll Lines Value
	CPoint		m_ptStartClickPosition;	// Start Mouse Click Position
	BOOL		m_bMouseCaptured;		// Mouse Capture Flag
	HCURSOR		m_hGrabClosedCursor;	// Closed Picture Move Cursor
	HCURSOR		m_hGrabNormalCursor;	// Normal Picture Move Cursor
	CBitmap		m_MemDCBitmap;			// Offscreen bitmap
	CRect		m_rcPrevMemDC;			// Previous MemDC Rectangle
	CPoint		m_ptMouseMoveLastPoint;	// Last Point Of Movement
	ULONGLONG	m_ullGestureZoomStart;	// Zoom gesture initial distance between the two points
	double		m_dGestureZoomStartPrintScale;	// Zoom gesture start print scale value
	CPoint		m_ptGesturePanStart;	// Pan gesture start position
	//{{AFX_VIRTUAL(CPicturePrintPreviewView)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL
	
protected:
	// Printing, orientation may be DMORIENT_PORTRAIT or DMORIENT_LANDSCAPE
	BOOL			SetPrintOrientation(int orientation) const;
	int				GetPrintOrientation() const;
	int				GetPaperSize() const;
	CString			PaperSizeToFormName(int nPaperSize) const;
	CString			GetPaperSizeName() const;
	void			CalcPhysPix(double& dX, double& dY);
	void			CalcPhysInch(double& dX, double& dY);
	void			CalcPhysCm(double& dX, double& dY);
	UINT			GetMouseScrollLines();
	virtual void	PositionPage(UINT nPage);
	void			SetScaledSize(UINT nPage);
	void			DoZoom(UINT nPage, CPoint point);
	void			SetZoomState(UINT nNewState, UINT nPage, CPoint point);
	CSize			CalcPageDisplaySize();
	void			SetupScrollbar();
	BOOL			FindPageRect(CPoint& point, UINT& nPage);
	void			FitSize();
	void			OnDisplayPageNumber(UINT nPage, UINT nPagesDisplayed);
	// note that if you change the maximum number of pages that can be displayed in preview mode
	// you have to increase the size of this array to make sure there is an entry available for every
	// possible page that can be shown at once.
	PAGE_INFO		m_pageInfoArray2[9]; // Embedded array for the default implementation - replaces MFC one of size 2!
	BOOL			m_bInchPaperFormat;

#ifdef _DEBUG
	virtual void	AssertValid() const;
	virtual void	Dump(CDumpContext& dc) const;
#endif
	
// Generated message map functions
protected:
	//{{AFX_MSG(CPicturePrintPreviewView)
	afx_msg int	OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNumPageChange();
	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();
	afx_msg void OnUpdateNumPageChange(CCmdUI* pCmdUI);
	afx_msg void OnUpdateZoomIn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateZoomOut(CCmdUI* pCmdUI);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSizeFit();
	afx_msg void OnPrintSetup();
	afx_msg void OnPortrait();
	afx_msg void OnLandscape();
	afx_msg void OnPrintMargin();
	//afx_msg void OnPreviewPages();
	afx_msg LRESULT OnGesture(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG
inline CPictureDoc* CPicturePrintPreviewView::GetDocument()
   { return (CPictureDoc*)m_pDocument; }
#endif

// the additional zoom level factors that wee support
#define ZOOM_IN_160     3
#define ZOOM_IN_180     4
#define ZOOM_IN_200     5


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PICTUREPRINTPREVIEWVIEW_H__CC2899D2_7CC9_45E8_A971_E6FC0E02639C__INCLUDED_)
