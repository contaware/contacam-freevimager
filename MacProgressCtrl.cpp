#include "stdafx.h"
#include "MacProgressCtrl.h"
#include "XThemeHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CColorProgressCtrl::CColorProgressCtrl()
{
	m_crColor = ::GetSysColor(COLOR_HIGHLIGHT);
	CalcColorBorder();
	CreatePenBorder();
}

CColorProgressCtrl::~CColorProgressCtrl()
{
	DeletePenBorder();
}

BEGIN_MESSAGE_MAP(CColorProgressCtrl, CProgressCtrl)
	//{{AFX_MSG_MAP(CColorProgressCtrl)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CColorProgressCtrl::IsThemed()
{
	return (ThemeHelper.IsAppThemed() &&
			ThemeHelper.IsThemeComCtl32());
}

void CColorProgressCtrl::OnPaint() 
{
	// Init vars
	CPaintDC dcPaint(this);
	CRect rect, rectClient;
	GetClientRect(rectClient);
	rect = rectClient;
	BOOL bVertical = GetStyle() & PBS_VERTICAL;

	// Create a memory DC for drawing
	CDC dc;
	dc.CreateCompatibleDC(&dcPaint);
 	int nSavedDC = dc.SaveDC();
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dcPaint, rect.Width(), rect.Height());
	CBitmap* pOldBmp = dc.SelectObject(&bmp);
	
	// Draw background
	if (IsThemed())
		ThemeHelper.DrawThemeParentBackground(GetSafeHwnd(), dc.GetSafeHdc(), &rect);
	else
	{
		CBrush brBackground(::GetSysColor(COLOR_3DFACE));
		dc.FillRect(rect, &brBackground);
	}

	// Determine the size of the bar and draw it
	int nLower, nUpper;
	GetRange(nLower, nUpper);
	if (bVertical)
	{
		rect.top = rect.bottom - int(((float)rect.Height() * float(GetPos() - nLower)) / float(nUpper - nLower));
		if (rect.Height() > 0)
		{
			CBrush br(m_crColor);
			CBrush* pOldBrush = dc.SelectObject(&br);
			CPen* pOldPen = dc.SelectObject(&m_PenBorder);
			dc.Rectangle(rect);
			dc.SelectObject(pOldPen);
			dc.SelectObject(pOldBrush);
		}
	}
	else
	{
		rect.right = int(((float)rect.Width() * float(GetPos() - nLower)) / float(nUpper - nLower));
		if (rect.Width() > 0)
		{
			CBrush br(m_crColor);
			CBrush* pOldBrush = dc.SelectObject(&br);
			CPen* pOldPen = dc.SelectObject(&m_PenBorder);
			dc.Rectangle(rect);
			dc.SelectObject(pOldPen);
			dc.SelectObject(pOldBrush);
		}
	}
	dcPaint.BitBlt(	rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), 
					&dc, rectClient.left, rectClient.top, SRCCOPY);

	// Clean-up
	dc.SelectObject(pOldBmp);
	dc.RestoreDC(nSavedDC);
	dc.DeleteDC();
}

BOOL CColorProgressCtrl::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

void CColorProgressCtrl::CalcColorBorder()
{
	const BYTE byReduceVal = 51;

	BYTE byRed = GetRValue(m_crColor);
	BYTE byGreen = GetGValue(m_crColor);
	BYTE byBlue = GetBValue(m_crColor);

	if (byRed > byReduceVal)
		byRed = BYTE(byRed - byReduceVal);
	else
		byRed = 0;
	if (byGreen > byReduceVal)
		byGreen = BYTE(byGreen - byReduceVal);
	else
		byGreen = 0;
	if (byBlue > byReduceVal)
		byBlue = BYTE(byBlue - byReduceVal);
	else
		byBlue = 0;

	m_crColorBorder = RGB(byRed, byGreen, byBlue);
}

void CColorProgressCtrl::SetColor(COLORREF crColor)
{
	m_crColor = crColor;
	CalcColorBorder();
	CreatePenBorder();
	RedrawWindow();
}

COLORREF CColorProgressCtrl::GetColor() const
{
	return m_crColor;
}

void CColorProgressCtrl::CreatePenBorder()
{
	DeletePenBorder();
	m_PenBorder.CreatePen(PS_SOLID, 1, m_crColorBorder);
}

void CColorProgressCtrl::DeletePenBorder()
{
	if (m_PenBorder.m_hObject)
		m_PenBorder.DeleteObject();
}