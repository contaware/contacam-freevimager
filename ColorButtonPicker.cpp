//***************************************************************************
//
// AUTHOR:  James White (feel free to remove or otherwise mangle any part)
//
//***************************************************************************
#include "stdafx.h"
#include "ColorButtonPicker.h"
#include "XThemeHelper.h"

//***********************************************************************
//**                         MFC Debug Symbols                         **
//***********************************************************************
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//***********************************************************************
//**                            Some Defines                           **
//***********************************************************************

#ifndef ODS_HOTLIGHT
#define ODS_HOTLIGHT        0x0040
#endif

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED     0x031A
#endif

//***********************************************************************
//**                            DDX Method                            **
//***********************************************************************

void AFXAPI DDX_ColorButtonPicker(CDataExchange *pDX, int nIDC, COLORREF& crColour)
{
    HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
    ASSERT (hWndCtrl != NULL);                
    
    CColorButtonPicker* pColourButton = (CColorButtonPicker*) CWnd::FromHandle(hWndCtrl);
    if (pDX->m_bSaveAndValidate)
    {
		crColour = pColourButton->Color;
    }
    else // initializing
    {
		pColourButton->Color = crColour;
    }
}

//***********************************************************************
//**                             Constants                             **
//***********************************************************************
const int g_ciArrowSizeX = 4 ;
const int g_ciArrowSizeY = 2 ;

//***********************************************************************
//**                            MFC Macros                            **
//***********************************************************************
IMPLEMENT_DYNCREATE(CColorButtonPicker, COddButton)

//***********************************************************************
// Method:	CColorButtonPicker::CColorButtonPicker(void)
// Notes:	Default Constructor.
//***********************************************************************
CColorButtonPicker::CColorButtonPicker(void):
	m_Color(CLR_DEFAULT),
	m_DefaultColor(::GetSysColor(COLOR_APPWORKSPACE)),
	m_strDefaultText(_T("Automatic")),
	m_strCustomText(_T("More Colors...")),
	m_bPopupActive(FALSE),
	m_bTrackSelection(FALSE),
	m_bToolbarButton(FALSE),
	m_strProfileSection(_T("")),
	m_bTracking(FALSE),
	m_bHover(FALSE),
	m_bFirstTime(TRUE),
	m_hTheme(NULL)
{
}

BOOL CColorButtonPicker::IsThemed()
{
	return (m_hTheme &&
			ThemeHelper.IsAppThemed() &&
			ThemeHelper.IsThemeComCtl32());
}

//***********************************************************************
// Method:	CColorButtonPicker::~CColorButtonPicker(void)
// Notes:	Destructor.
//***********************************************************************
CColorButtonPicker::~CColorButtonPicker(void)
{
	
}

//***********************************************************************
// Method:	CColorButtonPicker::GetColor()
// Notes:	None.
//***********************************************************************
COLORREF CColorButtonPicker::GetColor(void) const
{
	return m_Color;
}


//***********************************************************************
// Method:	CColorButtonPicker::SetColor()
// Notes:	None.
//***********************************************************************
void CColorButtonPicker::SetColor(COLORREF Color)
{
	m_Color = Color;

	if (::IsWindow(m_hWnd)) 
        RedrawWindow();
}


//***********************************************************************
// Method:	CColorButtonPicker::GetDefaultColor()
// Notes:	None.
//***********************************************************************
COLORREF CColorButtonPicker::GetDefaultColor(void) const
{
	return m_DefaultColor;
}

//***********************************************************************
// Method:	CColorButtonPicker::SetDefaultColor()
// Notes:	None.
//***********************************************************************
void CColorButtonPicker::SetDefaultColor(COLORREF Color)
{
	m_DefaultColor = Color;
}

//***********************************************************************
// Method:	CColorButtonPicker::SetCustomText()
// Notes:	None.
//***********************************************************************
void CColorButtonPicker::SetCustomText(LPCTSTR tszText)
{
	m_strCustomText = tszText;
}

//***********************************************************************
// Method:	CColorButtonPicker::SetDefaultText()
// Notes:	None.
//***********************************************************************
void CColorButtonPicker::SetDefaultText(LPCTSTR tszText)
{
	m_strDefaultText = tszText;
}


//***********************************************************************
// Method:	CColorButtonPicker::SetTrackSelection()
// Notes:	None.
//***********************************************************************
void CColorButtonPicker::SetTrackSelection(BOOL bTrack)
{
	m_bTrackSelection = bTrack;
}

//***********************************************************************
// Method:	CColorButtonPicker::GetTrackSelection()
// Notes:	None.
//***********************************************************************
BOOL CColorButtonPicker::GetTrackSelection(void) const
{
	return m_bTrackSelection;
}


//***********************************************************************
// Method:	CColorButtonPicker::SetDrawFocus()
// Notes:	None.
//***********************************************************************
void CColorButtonPicker::SetToolbarButton(BOOL bToolbarButton)
{
	m_bToolbarButton = bToolbarButton;

	if (::IsWindow(m_hWnd)) 
        RedrawWindow();
}

//***********************************************************************
// Method:	CColorButtonPicker::GetDrawFocus()
// Notes:	None.
//***********************************************************************
BOOL CColorButtonPicker::GetToolbarButton(void) const
{
	return m_bToolbarButton;
}

//***********************************************************************
// Method:	CColorButtonPicker::SetProfileSection()
// Notes:	None.
//***********************************************************************
void CColorButtonPicker::SetProfileSection(LPCTSTR tszProfileSection /*= _T("")*/)
{
	m_strProfileSection = tszProfileSection;
}

//***********************************************************************
//**                         Message Handlers                         **
//***********************************************************************
BEGIN_MESSAGE_MAP(CColorButtonPicker, COddButton)
    //{{AFX_MSG_MAP(CColorButtonPicker)
    ON_CONTROL_REFLECT_EX(BN_CLICKED, OnClicked)
    ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
    //}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
    ON_MESSAGE(CPN_SELENDOK,     OnSelEndOK)
    ON_MESSAGE(CPN_SELENDCANCEL, OnSelEndCancel)
    ON_MESSAGE(CPN_SELCHANGE,    OnSelChange)
END_MESSAGE_MAP()


//***********************************************************************
// Method:	CColorButtonPicker::OnSelEndOK()
// Notes:	None.
//***********************************************************************
LONG CColorButtonPicker::OnSelEndOK(UINT lParam, LONG /*wParam*/)
{
	m_bPopupActive = FALSE;

    COLORREF OldColor = m_Color;
	
	Color = (COLORREF)lParam;

    CWnd *pParent = GetParent();

    if (pParent) 
	{
        pParent->SendMessage(CPN_CLOSEUP, lParam, (WPARAM) GetDlgCtrlID());
        pParent->SendMessage(CPN_SELENDOK, lParam, (WPARAM) GetDlgCtrlID());
    }

    if (OldColor != m_Color)
        if (pParent) pParent->SendMessage(CPN_SELCHANGE, m_Color, (WPARAM) GetDlgCtrlID());

    return TRUE;
}


//***********************************************************************
// Method:	CColorButtonPicker::OnSelEndCancel()
// Notes:	None.
//***********************************************************************
LONG CColorButtonPicker::OnSelEndCancel(UINT lParam, LONG /*wParam*/)
{
	m_bPopupActive = FALSE;
	
	Color = (COLORREF)lParam;

    CWnd *pParent = GetParent();

    if (pParent) 
	{
        pParent->SendMessage(CPN_CLOSEUP, lParam, (WPARAM) GetDlgCtrlID());
        pParent->SendMessage(CPN_SELENDCANCEL, lParam, (WPARAM) GetDlgCtrlID());
    }

    return TRUE;
}

//***********************************************************************
// Method:	CColorButtonPicker::OnSelChange()
// Notes:	None.
//***********************************************************************
LONG CColorButtonPicker::OnSelChange(UINT lParam, LONG /*wParam*/)
{
    if (m_bTrackSelection) 
		Color = (COLORREF)lParam;

    CWnd *pParent = GetParent();

    if (pParent) pParent->SendMessage(CPN_SELCHANGE, lParam, (WPARAM) GetDlgCtrlID());

    return TRUE;
}

//***********************************************************************
// Method:	CColorButtonPicker::OnCreate()
// Notes:	None.
//***********************************************************************
int CColorButtonPicker::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (COddButton::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

//***********************************************************************
// Method:	CColorButtonPicker::OnClicked()
// Notes:	None.
//***********************************************************************
BOOL CColorButtonPicker::OnClicked()
{
	m_bPopupActive = TRUE;

    CRect rDraw;
    GetWindowRect(rDraw);

    new CColourPopup(	CPoint(rDraw.left, rDraw.bottom),		// Point to display popup
						m_Color,								// Selected colour
						this,									// parent
						m_strDefaultText,						// "Default" text area
						m_strCustomText,						// Custom Text
						m_strProfileSection);					// Profile Section for Custom Colours

    CWnd *pParent = GetParent();

    if (pParent)
        pParent->SendMessage(CPN_DROPDOWN, (LPARAM)m_Color, (WPARAM) GetDlgCtrlID());

    return TRUE;
}

//***********************************************************************
// Method:	CColorButtonPicker::OnTimer()
// Notes:	None.
//***********************************************************************
void CColorButtonPicker::OnTimer(UINT nIDEvent) 
{
	KillTimer(UNSELECT_TIMER_ID);
	if (!m_bPopupActive)
		Invalidate(FALSE);
}

//***********************************************************************
// Method:	CColorButtonPicker::DrawItem()
// Notes:	None.
//***********************************************************************
void CColorButtonPicker::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// Init Themes
	if (m_bFirstTime)
	{
		m_bFirstTime = FALSE;

		if (ThemeHelper.IsThemeLibAvailable())
		{
			if (m_hTheme)
				ThemeHelper.CloseThemeData(m_hTheme);
			m_hTheme = NULL;

			if (m_bToolbarButton)
				m_hTheme = ThemeHelper.OpenThemeData(m_hWnd, _T("Toolbar"));
			else
				m_hTheme = ThemeHelper.OpenThemeData(m_hWnd, _T("Button"));
		}
	}

	CDC*    pDC      = CDC::FromHandle(lpDrawItemStruct->hDC);
	UINT    state    = lpDrawItemStruct->itemState;
    CRect   rDraw    = lpDrawItemStruct->rcItem;
	CRect	rArrow;

	if (m_bPopupActive)
		state |= (ODS_SELECTED|ODS_FOCUS);

	//******************************************************
	//**                  Draw Outer Edge
	//******************************************************

	// After a selected draw an unwanted none
	// selected draw follows, the following
	// hack avoids the pushed/not pushed
	// flickering of the button.
	if ((state & ODS_SELECTED) == 0)
	{
		if (lpDrawItemStruct->itemAction & ODA_SELECT)
		{
			state |= (ODS_SELECTED|ODS_FOCUS);

			// But if the Popup is never showed
			// the button would remain in the pushed
			// state -> Start a timer that checks
			// that in 100 ms.
			SetTimer(	UNSELECT_TIMER_ID,
						UNSELECT_TIME_MS,
						NULL);
		}
	}

	// Has Themes?
	if (IsThemed())
	{	
		// Draw Background Color
		if (m_bToolbarButton)
		{
			COLORREF crBackground;
			ThemeHelper.GetThemeColor(	m_hTheme,
										TP_BUTTON,
										TS_NORMAL,
										TMT_FILLCOLOR,
										&crBackground);
			pDC->FillSolidRect(&rDraw, crBackground);
		}

		// Button State
		int nFrameState = 0;
		if ((state & ODS_SELECTED) != 0 || m_bPopupActive)
			nFrameState |= (m_bToolbarButton ? TS_PRESSED : PBS_PRESSED);
		if ((state & ODS_DISABLED) != 0)
			nFrameState |= (m_bToolbarButton ? TS_DISABLED : PBS_DISABLED);
		if ((state & ODS_HOTLIGHT) != 0 || m_bHover)
			nFrameState |= (m_bToolbarButton ? TS_HOT : PBS_HOT);
		else if ((state & ODS_DEFAULT) != 0)
			nFrameState |= PBS_DEFAULTED;

		// Draw Button
		if (m_bToolbarButton)
		{
			ThemeHelper.DrawThemeBackground(m_hTheme, pDC->GetSafeHdc(), TP_BUTTON, nFrameState, &rDraw, NULL);
			ThemeHelper.GetThemeBackgroundContentRect(m_hTheme, pDC->GetSafeHdc(), TP_BUTTON, nFrameState, &rDraw, &rDraw);
			rDraw.InflateRect(-2,-2,-2,-2);
		}
		else
		{
			if (ThemeHelper.IsThemeBackgroundPartiallyTransparent(m_hTheme, BP_PUSHBUTTON, nFrameState))
				ThemeHelper.DrawThemeParentBackground(GetSafeHwnd(), pDC->GetSafeHdc(), &rDraw);
			ThemeHelper.DrawThemeBackground(m_hTheme, pDC->GetSafeHdc(), BP_PUSHBUTTON, nFrameState, &rDraw, NULL);
			ThemeHelper.GetThemeBackgroundContentRect(m_hTheme, pDC->GetSafeHdc(), BP_PUSHBUTTON, nFrameState, &rDraw, &rDraw);
		}	

		// Offset if Selected
		if (state & ODS_SELECTED)
			rDraw.OffsetRect(1,0);
	}
	else
	{
		if (m_bToolbarButton)
		{
			BOOL bInflated = FALSE;

			// Draw Background Color
			COLORREF crBackground = ::GetSysColor(COLOR_BTNFACE);
			pDC->FillSolidRect(&rDraw, crBackground);

			// Draw Toolbar Button
			if (state & ODS_SELECTED)
			{
				DrawLine(pDC, rDraw.left, rDraw.top, rDraw.right-1, rDraw.top, ::GetSysColor(COLOR_BTNSHADOW));
				DrawLine(pDC, rDraw.left, rDraw.top, rDraw.left, rDraw.bottom-1, ::GetSysColor(COLOR_BTNSHADOW));
				DrawLine(pDC, rDraw.right-1, rDraw.bottom-1, rDraw.right-1, rDraw.top-1, RGB(255,255,255));
				DrawLine(pDC, rDraw.right-1, rDraw.bottom-1, rDraw.left-1, rDraw.bottom-1, RGB(255,255,255));
			}
			else if (state & ODS_DISABLED)
			{
				rDraw.InflateRect(-1,-1,-1,-1);
				// COLOR_GRAYTEXT is usually the same color as COLOR_BTNSHADOW
				CBrush br(::GetSysColor(COLOR_GRAYTEXT));
				::FrameRect(pDC->m_hDC,
							&rDraw,
							(HBRUSH)br);
				rDraw.InflateRect(-1,-1,-1,-1);
				bInflated = TRUE;
			}
			else if (m_bHover)
			{
				DrawLine(pDC, rDraw.left, rDraw.top, rDraw.right-1, rDraw.top, RGB(255,255,255));
				DrawLine(pDC, rDraw.left, rDraw.top, rDraw.left, rDraw.bottom-1, RGB(255,255,255));
				DrawLine(pDC, rDraw.right-1, rDraw.bottom-1, rDraw.right-1, rDraw.top-1, ::GetSysColor(COLOR_BTNSHADOW));
				DrawLine(pDC, rDraw.right-1, rDraw.bottom-1, rDraw.left-1, rDraw.bottom-1, ::GetSysColor(COLOR_BTNSHADOW));
			}
		
			if (!bInflated)
				rDraw.InflateRect(-2,-2,-2,-2);
		}
		else
		{
			// Button State
			UINT uFrameState = DFCS_BUTTONPUSH | DFCS_ADJUSTRECT;
			if (state & ODS_SELECTED)
				uFrameState |= DFCS_PUSHED;
			if (state & ODS_DISABLED)
				uFrameState |= DFCS_INACTIVE;

			// Draw Button
			pDC->DrawFrameControl(&rDraw,
								  DFC_BUTTON,
								  uFrameState);
		}

		// Offset if Selected
		if (state & ODS_SELECTED)
			rDraw.OffsetRect(1,1);
	}

	//******************************************************
	//**                     Draw Focus
	//******************************************************
	if (!m_bToolbarButton && (state & ODS_FOCUS)) 
    {
		RECT rFocus = {rDraw.left,
					   rDraw.top,
					   rDraw.right - 1,
					   rDraw.bottom};
  
        pDC->DrawFocusRect(&rFocus);
    }

	rDraw.DeflateRect(::GetSystemMetrics(SM_CXEDGE),
					  ::GetSystemMetrics(SM_CYEDGE));

	//******************************************************
	//**                     Draw Arrow
	//******************************************************
	rArrow.left		= rDraw.right - g_ciArrowSizeX - ::GetSystemMetrics(SM_CXEDGE) /2;
	rArrow.right	= rArrow.left + g_ciArrowSizeX;
	rArrow.top		= (rDraw.bottom + rDraw.top)/2 - g_ciArrowSizeY / 2;
	rArrow.bottom	= (rDraw.bottom + rDraw.top)/2 + g_ciArrowSizeY / 2;

	DrawArrow(pDC,
			  &rArrow,
			  0,
			  (state & ODS_DISABLED) 
			  ? ::GetSysColor(COLOR_GRAYTEXT) 
			  : RGB(0,0,0));


	rDraw.right = rArrow.left - ::GetSystemMetrics(SM_CXEDGE)/2;

	//******************************************************
	//**                   Draw Separator
	//******************************************************
	pDC->DrawEdge(&rDraw,
				  EDGE_ETCHED,
				  BF_RIGHT);

	rDraw.right -= (::GetSystemMetrics(SM_CXEDGE) * 2) + 1 ;
				  
	//******************************************************
	//**                     Draw Color
	//******************************************************
	if (state & ODS_DISABLED)
	{
		CBrush br(::GetSysColor(COLOR_GRAYTEXT));
		::FrameRect(pDC->m_hDC,
					&rDraw,
					(HBRUSH)br);
	}
	else
	{
		pDC->FillSolidRect(&rDraw,
						   (m_Color == CLR_DEFAULT)
						   ? m_DefaultColor
						   : m_Color);

		::FrameRect(pDC->m_hDC,
					&rDraw,
					(HBRUSH)::GetStockObject(BLACK_BRUSH));
	}
}


//***********************************************************************
//**                          Static Methods                          **
//***********************************************************************

//***********************************************************************
// Method:	CColorButtonPicker::DrawArrow()
// Notes:	None.
//***********************************************************************
void CColorButtonPicker::DrawArrow(CDC* pDC, 
							 RECT* pRect, 
							 int iDirection,
							 COLORREF clrArrow /*= RGB(0,0,0)*/)
{
	POINT ptsArrow[3];

	switch (iDirection)
	{
		case 0 : // Down
		{
			ptsArrow[0].x = pRect->left;
			ptsArrow[0].y = pRect->top;
			ptsArrow[1].x = pRect->right;
			ptsArrow[1].y = pRect->top;
			ptsArrow[2].x = (pRect->left + pRect->right)/2;
			ptsArrow[2].y = pRect->bottom;
			break;
		}

		case 1 : // Up
		{
			ptsArrow[0].x = pRect->left;
			ptsArrow[0].y = pRect->bottom;
			ptsArrow[1].x = pRect->right;
			ptsArrow[1].y = pRect->bottom;
			ptsArrow[2].x = (pRect->left + pRect->right)/2;
			ptsArrow[2].y = pRect->top;
			break;
		}

		case 2 : // Left
		{
			ptsArrow[0].x = pRect->right;
			ptsArrow[0].y = pRect->top;
			ptsArrow[1].x = pRect->right;
			ptsArrow[1].y = pRect->bottom;
			ptsArrow[2].x = pRect->left;
			ptsArrow[2].y = (pRect->top + pRect->bottom)/2;
			break;
		}

		case 3 : // Right
		{
			ptsArrow[0].x = pRect->left;
			ptsArrow[0].y = pRect->top;
			ptsArrow[1].x = pRect->left;
			ptsArrow[1].y = pRect->bottom;
			ptsArrow[2].x = pRect->right;
			ptsArrow[2].y = (pRect->top + pRect->bottom)/2;
			break;
		}
	}
	
	CBrush brsArrow(clrArrow);
	CPen penArrow(PS_SOLID, 1 , clrArrow);

	CBrush* pOldBrush = pDC->SelectObject(&brsArrow);
	CPen*   pOldPen   = pDC->SelectObject(&penArrow);
	
	pDC->SetPolyFillMode(WINDING);
	pDC->Polygon(ptsArrow, 3);

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
}

void CColorButtonPicker::DrawLine(CDC *pDC, long sx, long sy, long ex, long ey, COLORREF color)
{
	CPen newPen;
	CPen *oldPen;
	
	newPen.CreatePen(PS_SOLID, 1, color);
	oldPen = pDC->SelectObject(&newPen);

	pDC->MoveTo(sx, sy);
	pDC->LineTo(ex, ey);
	pDC->SelectObject(oldPen);

    newPen.DeleteObject();
}

void CColorButtonPicker::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (!m_bTracking)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE|TME_HOVER;
		tme.dwHoverTime = 1;

		// _TrackMouseEvent tries to use the window manager's
		// implementation of TrackMouseEvent if it is present
		// (it is present starting from Win98 & NT4),
		// otherwise it emulates it on win95.
		// (Needs newer version of comctl32.dll on win95!)
		m_bTracking = _TrackMouseEvent(&tme);
	}

	COddButton::OnMouseMove(nFlags, point);
}

LRESULT CColorButtonPicker::OnMouseHover(WPARAM wparam, LPARAM lparam) 
{
	m_bHover = TRUE;
	Invalidate(FALSE);

	return 0;
}

LRESULT CColorButtonPicker::OnMouseLeave(WPARAM wparam, LPARAM lparam)
{
	m_bTracking = FALSE;
	m_bHover = FALSE;
	Invalidate(FALSE);
	
	return 0;
}

LRESULT CColorButtonPicker::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
		case WM_THEMECHANGED:
		{
			if (IsThemed())
			{
				// when user changes themes, close current theme
				ThemeHelper.CloseThemeData(m_hTheme);
				m_hTheme = NULL;
				m_bFirstTime = TRUE;
			}
		}
		break;
	}

	return COddButton::DefWindowProc(message, wParam, lParam);
}

// OnEraseBkgnd - to prevent flicker
BOOL CColorButtonPicker::OnEraseBkgnd(CDC* pDC) 
{	
	return TRUE;
}
