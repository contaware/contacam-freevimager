// ColorButton.cpp : implementation file
//
// Written by Marius Bancila (mbancila@yahoo.com)
// Copyright (c) 2004.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. If 
// the source code in  this file is used in any commercial application 
// then acknowledgement must be made to the author of this file 
// (in whatever form you wish).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
// IN THE SOFTWARE.
// 
// Please use and enjoy. Please let me know of any bugs/mods/improvements 
// that you have found/implemented and I will fix/incorporate them into this
// file. 

#include "stdafx.h"
#include "ColorButton.h"
#include "XThemeHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef ODS_NOFOCUSRECT
#define ODS_NOFOCUSRECT     0x0200		// from WinUser.h
#endif

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED     0x031A
#endif

namespace clr
{
	const COLORREF CLR_BTN_WHITE  = RGB(255, 255, 255);
	const COLORREF CLR_BTN_BLACK  = RGB(0, 0, 0);
	const COLORREF CLR_BTN_DGREY  = RGB(128, 128, 128);
	const COLORREF CLR_BTN_GREY   = RGB(192, 192, 192);
	const COLORREF CLR_BTN_LLGREY = RGB(223, 223, 223);
}

/////////////////////////////////////////////////////////////////////////////
// CColorButton
CColorButton::CColorButton()
{
	m_hTheme = NULL;
	m_bFirstTime = TRUE;
	m_bToolbarButton = FALSE;
	m_bTracking = FALSE;
	m_bHover = FALSE;
	SetColorToWindowsDefault();
}

CColorButton::CColorButton(COLORREF text, COLORREF bkgnd)
{
	m_hTheme				= NULL;
	m_bFirstTime			= TRUE;
	m_bToolbarButton		= FALSE;
	m_bTracking				= FALSE;
	m_bHover				= FALSE;
	m_TextColor				= text;
	m_BkgndColor			= bkgnd;
	m_bUseDefaultColor		= FALSE;
	m_DisabledBkgndColor	= GetSysColor(COLOR_BTNFACE);
	m_Light					= GetSysColor(COLOR_3DLIGHT);
	m_Highlight				= GetSysColor(COLOR_BTNHIGHLIGHT);
	m_Shadow				= GetSysColor(COLOR_BTNSHADOW);
	m_DarkShadow			= GetSysColor(COLOR_3DDKSHADOW);	
}

CColorButton::CColorButton(COLORREF text, COLORREF bkgnd, COLORREF disabled)
{
	m_hTheme				= NULL;
	m_bFirstTime			= TRUE;
	m_bToolbarButton		= FALSE;
	m_bTracking				= FALSE;
	m_bHover				= FALSE;
	m_TextColor				= text;
	m_BkgndColor			= bkgnd;
	m_bUseDefaultColor		= FALSE;
	m_DisabledBkgndColor	= disabled;
	m_Light					= GetSysColor(COLOR_3DLIGHT);
	m_Highlight				= GetSysColor(COLOR_BTNHIGHLIGHT);
	m_Shadow				= GetSysColor(COLOR_BTNSHADOW);
	m_DarkShadow			= GetSysColor(COLOR_3DDKSHADOW);	
}

CColorButton::CColorButton(COLORREF text, COLORREF bkgnd, COLORREF disabled, COLORREF light, COLORREF highlight, COLORREF shadow, COLORREF darkShadow)
{
	m_hTheme				= NULL;
	m_bFirstTime			= TRUE;
	m_bToolbarButton		= FALSE;
	m_bTracking				= FALSE;
	m_bHover				= FALSE;
	m_TextColor				= text;
	m_BkgndColor			= bkgnd;
	m_bUseDefaultColor		= FALSE;
	m_DisabledBkgndColor	= disabled;
	m_Light					= light;
	m_Highlight				= highlight;
	m_Shadow				= shadow;
	m_DarkShadow			= darkShadow;
}

CColorButton::~CColorButton()
{
	if (m_hTheme)
		ThemeHelper.CloseThemeData(m_hTheme);
	m_hTheme = NULL;
}


BEGIN_MESSAGE_MAP(CColorButton, COddButton)
	//{{AFX_MSG_MAP(CColorButton)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorButton message handlers

BOOL CColorButton::IsThemed()
{
	return (m_hTheme &&
			ThemeHelper.IsAppThemed() &&
			ThemeHelper.IsThemeComCtl32());
}

void CColorButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
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

	CDC		*pDC;
	CRect	rcFocus, rcButton, rcText, rcOffsetText, rcColorBkg;
	pDC    = CDC::FromHandle(lpDrawItemStruct->hDC);
	int nOldBkMode = pDC->SetBkMode(TRANSPARENT);

	UINT state			= lpDrawItemStruct->itemState;
	BOOL bIsPressed     = state & ODS_SELECTED;
	BOOL bIsFocused     = state & ODS_FOCUS;
	BOOL bIsDisabled    = state & ODS_DISABLED;
	BOOL bDrawFocusRect = !(state & ODS_NOFOCUSRECT);
	
	// Rectangles
	rcFocus.CopyRect(&lpDrawItemStruct->rcItem); 
	rcButton.CopyRect(&lpDrawItemStruct->rcItem); 
	rcColorBkg.CopyRect(&lpDrawItemStruct->rcItem); 
	
	// Text Rect
	rcText = rcButton;
	if (!IsThemed())
		rcText.OffsetRect(-1, -1);
	rcOffsetText = rcText;
	rcOffsetText.OffsetRect(1, 1);

	// Set the focus rectangle to just past the border decoration
	rcFocus.left   += 3;
    rcFocus.right  -= 3;
    rcFocus.top    += 3;
    rcFocus.bottom -= 3;
	
	// Retrieve the button's caption
	CString strCaption;
	GetWindowText(strCaption);
	BOOL bHasText = !strCaption.IsEmpty();
	
	if (IsThemed())
	{
		// Draw Background
		DWORD buttonstate = bIsPressed ? PBS_PRESSED : PBS_NORMAL;
		if (buttonstate == PBS_NORMAL)
		{
			if (bIsFocused)
				buttonstate = PBS_DEFAULTED;
			if (m_bHover)
				buttonstate = PBS_HOT;
			if (bIsDisabled)
				buttonstate = PBS_DISABLED;
		}
		DWORD part = BP_PUSHBUTTON;
		if (ThemeHelper.IsThemeBackgroundPartiallyTransparent(m_hTheme, part, buttonstate))
			ThemeHelper.DrawThemeParentBackground(GetSafeHwnd(), pDC->GetSafeHdc(), &rcButton);
		ThemeHelper.DrawThemeBackground(m_hTheme, pDC->GetSafeHdc(), part, buttonstate, &rcButton, NULL);

		// Draw Color
		if (!m_bUseDefaultColor)
		{
			rcColorBkg.DeflateRect(4,4,4,4);
			if (bIsDisabled)
			{
				DrawFilledRect(pDC, rcColorBkg, m_DisabledBkgndColor);
			}
			else
			{
				DrawFilledRect(pDC, rcColorBkg, m_BkgndColor);
			}
		}

		// Draw Text
		/* Only Working For Single Line Text!
		if (bHasText)
		{
			UINT uTextAlignment = DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_WORDBREAK;
			ThemeHelper.DrawThemeText(	m_hTheme,
										pDC->GetSafeHdc(),
										part,
										bIsDisabled ? PBS_DISABLED : PBS_NORMAL,
										strCaption,
										uTextAlignment,
										0,
										&rcButton);
		}
		*/
	}
	else
	{
		if (bIsDisabled)
		{
			DrawFilledRect(pDC, rcColorBkg, m_bUseDefaultColor ? GetSysColor(COLOR_BTNFACE) : m_DisabledBkgndColor);
		}
		else
		{
			DrawFilledRect(pDC, rcColorBkg, m_bUseDefaultColor ? GetSysColor(COLOR_BTNFACE) : m_BkgndColor);
		}
		
		if (bIsPressed)
		{ 
			DrawFrame(pDC, rcButton, BUTTON_IN);
		}
		else
		{
			if ((state & ODS_DEFAULT) || bIsFocused)
			{
				DrawFrame(pDC, rcButton, BUTTON_OUT | BUTTON_BLACK_BORDER);			
			}
			else
			{
				DrawFrame(pDC, rcButton, BUTTON_OUT);
			}
		}
	}

	if (bHasText)
	{
		if (bIsDisabled)
		{
			DrawButtonText(pDC, rcOffsetText, strCaption, clr::CLR_BTN_WHITE);
			DrawButtonText(pDC, rcText,	strCaption, clr::CLR_BTN_DGREY);
		}
		else
		{
			if (!IsThemed())
			{
				if (bIsPressed)
				{
					DrawButtonText(pDC, rcOffsetText, strCaption, m_bUseDefaultColor ? GetSysColor(COLOR_BTNTEXT) : m_TextColor);
				}
				else
				{
					DrawButtonText(pDC, rcText, strCaption, m_bUseDefaultColor ? GetSysColor(COLOR_BTNTEXT) : m_TextColor);
				}
			}
			else
				DrawButtonText(pDC, rcText, strCaption, m_bUseDefaultColor ? GetSysColor(COLOR_BTNTEXT) : m_TextColor);
		}
	}

	if (bIsFocused && bDrawFocusRect)
	{
		DrawFocusRect(lpDrawItemStruct->hDC, (LPRECT)&rcFocus);
	}

	pDC->SetBkMode(nOldBkMode);
}

void CColorButton::DrawFrame(CDC *pDC, CRect rc, int state)
{
	COLORREF color;
	
	if (state & BUTTON_BLACK_BORDER)
	{
		color = clr::CLR_BTN_BLACK;
		
		DrawLine(pDC, rc.left, rc.top, rc.right, rc.top,    color); // Across top
		DrawLine(pDC, rc.left, rc.top, rc.left,  rc.bottom, color); // Down left
		
		DrawLine(pDC, rc.left,      rc.bottom - 1, rc.right,     rc.bottom - 1, color); // Across bottom
		DrawLine(pDC, rc.right - 1, rc.top,        rc.right - 1, rc.bottom,     color); // Down right
		
		rc.InflateRect(-1, -1);
	}
	
	if (state & BUTTON_OUT)
	{
		color = m_Highlight;
		
		DrawLine(pDC, rc.left, rc.top, rc.right, rc.top,    color); // Across top
		DrawLine(pDC, rc.left, rc.top, rc.left,  rc.bottom, color); // Down left
		
		color = m_DarkShadow;
		
		DrawLine(pDC, rc.left,      rc.bottom - 1, rc.right,     rc.bottom - 1, color); // Across bottom
		DrawLine(pDC, rc.right - 1, rc.top,        rc.right - 1, rc.bottom,     color); // Down right
		
		rc.InflateRect(-1, -1);
		
		color = m_Light;
		
		DrawLine(pDC, rc.left, rc.top, rc.right, rc.top,    color); // Across top
		DrawLine(pDC, rc.left, rc.top, rc.left,  rc.bottom, color); // Down left
		
		color = m_Shadow;
		
		DrawLine(pDC, rc.left,      rc.bottom - 1, rc.right,     rc.bottom - 1, color); // Across bottom
		DrawLine(pDC, rc.right - 1, rc.top,        rc.right - 1, rc.bottom,     color); // Down right
	}
	
	if (state & BUTTON_IN)
	{
		color = m_DarkShadow;
		
		DrawLine(pDC, rc.left, rc.top, rc.right, rc.top,    color); // Across top
		DrawLine(pDC, rc.left, rc.top, rc.left,  rc.bottom, color); // Down left
		DrawLine(pDC, rc.left,      rc.bottom - 1, rc.right,     rc.bottom - 1, color); // Across bottom
		DrawLine(pDC, rc.right - 1, rc.top,        rc.right - 1, rc.bottom,     color); // Down right
		
		rc.InflateRect(-1, -1);
		
		color = m_Shadow;
		
		DrawLine(pDC, rc.left, rc.top, rc.right, rc.top,    color); // Across top
		DrawLine(pDC, rc.left, rc.top, rc.left,  rc.bottom, color); // Down left
		DrawLine(pDC, rc.left,      rc.bottom - 1, rc.right,     rc.bottom - 1, color); // Across bottom
		DrawLine(pDC, rc.right - 1, rc.top,        rc.right - 1, rc.bottom,     color); // Down right
	}
}

void CColorButton::DrawFilledRect(CDC *pDC, CRect rc, COLORREF color)
{
	CBrush brSolid;
	
	brSolid.CreateSolidBrush(color);
	pDC->FillRect(rc, &brSolid);
}

void CColorButton::DrawLine(CDC *pDC, long sx, long sy, long ex, long ey, COLORREF color)
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

void CColorButton::DrawButtonText(CDC *pDC, CRect rc, CString strCaption, COLORREF textcolor)
{
	int i;
	DWORD uStyle = GetWindowLong(this->m_hWnd,GWL_STYLE);

	// Multi Line String Split-Up
	CArray<CString, CString> arLines;
	if ((uStyle & BS_MULTILINE) == BS_MULTILINE)
	{
		int nIndex = 0;
		while(nIndex != -1)
		{
			nIndex = strCaption.Find('\n');
			if(nIndex>-1)
			{
				CString line = strCaption.Left(nIndex);
				arLines.Add(line);
				strCaption.Delete(0,nIndex+1);
			}
			else
				arLines.Add(strCaption);
		}
	}
	else
	{
		arLines.Add(strCaption);
	}

	// Calc. Text Max Extent
	CSize sizeText(0,0);
	for (i = 0 ; i < arLines.GetSize() ; i++)
	{
		CString line = arLines.GetAt(i);
		CSize szTextExtent = pDC->GetOutputTextExtent(line);
		if (szTextExtent.cx > sizeText.cx)
			sizeText.cx = szTextExtent.cx;
		if (szTextExtent.cy > sizeText.cy)
			sizeText.cy = szTextExtent.cy;
	}
	
	// Set Color and Transparent Mode
    COLORREF oldColour = pDC->SetTextColor(textcolor);
	int nOldBkMode = pDC->SetBkMode(TRANSPARENT);

	// Vertical Alignment
	int nStartPos =	rc.top + (rc.Height() -
					arLines.GetSize()*sizeText.cy)/2;		// Default: Vertical Center
	if ((uStyle & BS_TOP) == BS_TOP)						// Top Align
		nStartPos = rc.top+2;
	if ((uStyle & BS_BOTTOM) == BS_BOTTOM)					// Bottom Align
		nStartPos = rc.bottom- arLines.GetSize()*sizeText.cy-2;
	if ((uStyle & BS_VCENTER) == BS_VCENTER)
		nStartPos = rc.top + (rc.Height() -
					arLines.GetSize()*sizeText.cy)/2;		// Vertical Center
	
	// Horizontal Alignment
	UINT uDrawStyles = DT_CENTER | DT_TOP | DT_SINGLELINE;	// Default: Horizontal Center
	if ((uStyle & BS_LEFT) == BS_LEFT)
		uDrawStyles = DT_LEFT | DT_TOP | DT_SINGLELINE;		// Left Align
	if ((uStyle & BS_RIGHT) == BS_RIGHT)
		uDrawStyles = DT_RIGHT | DT_TOP | DT_SINGLELINE;	// Right Align
	if ((uStyle & BS_CENTER) == BS_CENTER)
		uDrawStyles = DT_CENTER | DT_TOP | DT_SINGLELINE;	// Horizontal Center
	
	// Draw Text
	for (i = 0 ; i < arLines.GetSize() ; i++)
	{
		CRect textrc = rc;
		textrc.DeflateRect(3,0,3,0);
		textrc.top = nStartPos + sizeText.cy*i;
		textrc.bottom = textrc.top + sizeText.cy;
		CString line = arLines.GetAt(i);
		pDC->DrawText(line, line.GetLength(), textrc, uDrawStyles);
	}

	// Restore Old Values
	pDC->SetTextColor(oldColour);
	pDC->SetBkMode(nOldBkMode);
}

void CColorButton::SetColor(COLORREF text, COLORREF bkgnd)
{
	m_TextColor				= text;
	m_BkgndColor			= bkgnd; 

	if (m_TextColor != GetSysColor(COLOR_BTNTEXT) ||
		m_BkgndColor != GetSysColor(COLOR_BTNFACE))
		m_bUseDefaultColor = FALSE;

	if(m_hWnd != NULL)
		Invalidate();
}

void CColorButton::SetColor(COLORREF text, COLORREF bkgnd, COLORREF disabled)
{
	m_TextColor				= text;
	m_BkgndColor			= bkgnd; 
	m_DisabledBkgndColor	= disabled;

	if (m_TextColor != GetSysColor(COLOR_BTNTEXT)	||
		m_BkgndColor != GetSysColor(COLOR_BTNFACE)	||
		m_DisabledBkgndColor != GetSysColor(COLOR_BTNFACE))
		m_bUseDefaultColor = FALSE;

	if(m_hWnd != NULL)
		Invalidate();
}

void CColorButton::SetColor(COLORREF text,
							COLORREF bkgnd,
							COLORREF disabled,
							COLORREF light,
							COLORREF highlight,
							COLORREF shadow,
							COLORREF darkShadow)
{
	m_TextColor				= text;
	m_BkgndColor			= bkgnd; 
	m_DisabledBkgndColor	= disabled;
	m_Light					= light;
	m_Highlight				= highlight;
	m_Shadow				= shadow;
	m_DarkShadow			= darkShadow;

	if (m_TextColor != GetSysColor(COLOR_BTNTEXT)			||
		m_BkgndColor != GetSysColor(COLOR_BTNFACE)			||
		m_DisabledBkgndColor != GetSysColor(COLOR_BTNFACE)	||
		m_Light != GetSysColor(COLOR_3DLIGHT)				||
		m_Highlight != GetSysColor(COLOR_BTNHIGHLIGHT)		||
		m_Shadow != GetSysColor(COLOR_BTNSHADOW)			||
		m_DarkShadow != GetSysColor(COLOR_3DDKSHADOW))
		m_bUseDefaultColor = FALSE;

	if(m_hWnd != NULL)
		Invalidate();
}

void CColorButton::SetColorToWindowsDefault()
{
	m_TextColor				= GetSysColor(COLOR_BTNTEXT);
	m_BkgndColor			= GetSysColor(COLOR_BTNFACE); 
	m_DisabledBkgndColor	= GetSysColor(COLOR_BTNFACE);
	m_Light					= GetSysColor(COLOR_3DLIGHT);
	m_Highlight				= GetSysColor(COLOR_BTNHIGHLIGHT);
	m_Shadow				= GetSysColor(COLOR_BTNSHADOW);
	m_DarkShadow			= GetSysColor(COLOR_3DDKSHADOW);
	m_bUseDefaultColor		= TRUE;
	if(m_hWnd != NULL)
		Invalidate();
}

int CColorButton::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	lpCreateStruct->dwExStyle |= BS_OWNERDRAW;

	if (COddButton::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}

// OnEraseBkgnd - to prevent flicker
BOOL CColorButton::OnEraseBkgnd(CDC* pDC) 
{	
	return TRUE;
}

void CColorButton::OnMouseMove(UINT nFlags, CPoint point) 
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

LRESULT CColorButton::OnMouseHover(WPARAM wparam, LPARAM lparam) 
{
	m_bHover = TRUE;
	Invalidate(FALSE);

	return 0;
}

LRESULT CColorButton::OnMouseLeave(WPARAM wparam, LPARAM lparam)
{
	m_bTracking = FALSE;
	m_bHover = FALSE;
	Invalidate(FALSE);
	
	return 0;
}

LRESULT CColorButton::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
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
