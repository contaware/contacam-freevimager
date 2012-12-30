#include "stdafx.h"
#include "PaletteWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BTN_SIZE 16

static COLORREF ColorMap[16][16];

CPaletteWnd::CPaletteWnd()
{
	m_pParentWnd = NULL;
	m_bDrag = false;
	m_rc.SetRectEmpty();
	m_nIndex = -1;
	m_nColorMapWidth = -1;
	m_nColorMapHeight = -1;
}

CPaletteWnd::CPaletteWnd(CPoint pt, CWnd* pParentWnd, CPalette* pPalette)
{
	m_pParentWnd = NULL;
	m_bDrag = false;
	m_rc.SetRectEmpty();
	m_nIndex = -1;
	m_nColorMapWidth = -1;
	m_nColorMapHeight = -1;

	Create(pt, pParentWnd, pPalette);
}

BEGIN_MESSAGE_MAP(CPaletteWnd, CWnd)
	//{{AFX_MSG_MAP(CPaletteWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CPaletteWnd::Create(CPoint pt, CWnd* pParentWnd, CPalette* pPalette)
{
	// Store Parent Pointer
	m_pParentWnd = pParentWnd;

	// Init Palette
	UINT uiPaletteEntries = pPalette->GetEntryCount();
	if (uiPaletteEntries < 2)
		return FALSE;
	if (uiPaletteEntries == 2)
	{
		m_nColorMapWidth = 2;
		m_nColorMapHeight = 2;
	}
	else if (uiPaletteEntries <= 16)
	{
		m_nColorMapWidth = 4;
		m_nColorMapHeight = 4;
	}
	else
	{
		m_nColorMapWidth = 16;
		m_nColorMapHeight = 16;
	}
	PALETTEENTRY Entry;
	unsigned int i;
	for (i = 0 ; i < uiPaletteEntries ; i++)
	{
		if (pPalette->GetPaletteEntries(i, 1, &Entry) != 1)
			return FALSE;

		ColorMap[i/m_nColorMapHeight][i%m_nColorMapWidth] = RGB(Entry.peRed, Entry.peGreen, Entry.peBlue);  
	}
	for (i = uiPaletteEntries ; i < (unsigned int)(m_nColorMapWidth * m_nColorMapHeight) ; i++)
	{
		ColorMap[i/m_nColorMapHeight][i%m_nColorMapWidth] =
				ColorMap[(uiPaletteEntries - 1)/m_nColorMapHeight][(uiPaletteEntries - 1)%m_nColorMapWidth]; 
	}

    // Get the class name and create the window
    CString szClassName = ::AfxRegisterWndClass(CS_CLASSDC|CS_SAVEBITS|CS_HREDRAW|CS_VREDRAW);
	CRect rc;	
	rc.SetRect(pt.x, pt.y, 100, 100); // The Width & Height will be changed to the right size in OnCreate()
	BOOL res = CWnd::CreateEx(	WS_EX_TOOLWINDOW,
								szClassName, _T(""),
								WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
								rc, m_pParentWnd, 0, NULL);

	// Title
	if (uiPaletteEntries > 2)
		SetWindowText(_T("Palette"));

	return res;
}

void CPaletteWnd::OnClose() 
{
	DestroyWindow();
}

void CPaletteWnd::Close()
{
	OnClose();
}

void CPaletteWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect rc;

	GetClientRect(rc);

	CRect rcClient, rcWindow;
	GetClientRect(rcClient);
	GetWindowRect(rcWindow);
	int nDeltaWidth = rcWindow.Width() - rcClient.Width();
	int nDeltaHeight = rcWindow.Height() - rcClient.Height();


	dc.FillSolidRect(rc, RGB(128,128,128));

	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(0,0,0));
	CPen* pPen = dc.SelectObject(&pen);

	HBRUSH hBr = (HBRUSH ) GetCurrentObject(dc, OBJ_BRUSH);

	int nY = 1;

	for (int nColorRow = 0 ; nColorRow < m_nColorMapHeight ; nColorRow++)
	{
		int nX = 1;
		for (int nColorCol = 0 ; nColorCol < m_nColorMapWidth ; nColorCol++)
		{
			rc.SetRect(nX, nY, nX + BTN_SIZE, nY + BTN_SIZE);
			
			CBrush br;

			br.CreateSolidBrush(ColorMap[nColorRow][nColorCol]);
			CBrush* pBrush = dc.SelectObject(&br);
			dc.Rectangle(rc);
			dc.SelectObject(pBrush);


			nX += BTN_SIZE + 1;
		}

		nY += BTN_SIZE + 1;
	}

	// Draw 'Selected' item
	if (m_nIndex != -1)
	{
		CBrush br;
		br.CreateSolidBrush(RGB(255,255,255));
		dc.FrameRect(m_rc, &br);

		// Give select a rounded feel
		dc.SetPixel(m_rc.TopLeft(),RGB(128, 128, 128));
		dc.SetPixel(m_rc.BottomRight(),RGB(128, 128, 128));
		dc.SetPixel(CPoint(m_rc.right, m_rc.top), RGB(128, 128, 128));
		dc.SetPixel(CPoint(m_rc.left, m_rc.bottom) ,RGB(128, 128, 128));
	}

	dc.SelectObject(pPen);

	::SelectObject(dc, hBr);
}

void CPaletteWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonDown(nFlags, point);

	m_bDrag = true;
	SetCapture();

	SelectColor(point);
}

void CPaletteWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_bDrag = false;
	ReleaseCapture();
	CWnd::OnLButtonUp(nFlags, point);

	if (SelectColor(point))
	{
		m_pParentWnd->SendMessage(	WM_COLOR_PICKED,
									m_nIndex,
									(LPARAM)ColorMap[m_nIndex/m_nColorMapHeight][m_nIndex%m_nColorMapWidth]);
	}
}

void CPaletteWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	CWnd::OnMouseMove(nFlags, point);

	if (m_bDrag)
		SelectColor(point);
}

BOOL CPaletteWnd::SelectColor(CPoint& pt)
{
	CRect rc;
	GetClientRect(rc);

	if (!rc.PtInRect(pt))
		return FALSE;

	int nX = (pt.x / (BTN_SIZE+1));
	int nY = (pt.y / (BTN_SIZE+1));

	int nIndex = nX + (nY * m_nColorMapWidth);
	if (m_nIndex == nIndex)
		return TRUE;

	m_nIndex = -1;
	InvalidateRect(m_rc, TRUE);
	UpdateWindow();
	
	m_nIndex = nIndex;
	
	m_rc.SetRect((nX * (BTN_SIZE + 1) + 1), 
				 (nY * (BTN_SIZE + 1) + 1),
				 (nX * (BTN_SIZE + 1) + 1) + BTN_SIZE, 				 
				 (nY * (BTN_SIZE + 1) + 1) + BTN_SIZE);

	m_rc.InflateRect(1,1);
	InvalidateRect(m_rc, TRUE);
	UpdateWindow();
	
	return TRUE;

}

void CPaletteWnd::CreateToolTips()
{
    if (!m_wndToolTip.Create(this)) 
		return;

	int nY = 1;

	for (int nColorRow = 0 ; nColorRow < m_nColorMapHeight ; nColorRow++)
	{
		int nX = 1;
		for (int nColorCol = 0 ; nColorCol < m_nColorMapWidth ; nColorCol++)
		{
			CRect rc;
			rc.SetRect(nX, nY, nX + BTN_SIZE, nY + BTN_SIZE);


			CString strColor;
			strColor.Format(_T("RGB(%d,%d,%d)"), GetRValue(ColorMap[nColorRow][nColorCol]),
												 GetGValue(ColorMap[nColorRow][nColorCol]),
												 GetBValue(ColorMap[nColorRow][nColorCol]));

			m_wndToolTip.AddTool(this, strColor, rc, 1);	

			nX += BTN_SIZE + 1;
		}

		nY += BTN_SIZE + 1;
	}
}

int CPaletteWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rcClient;
	rcClient.left = 0;
	rcClient.top = 0;
	rcClient.right = ((BTN_SIZE + 1) * m_nColorMapWidth) + 1;
	rcClient.bottom = ((BTN_SIZE + 1) * m_nColorMapHeight) + 1;
	CalcWindowRect(&rcClient);// Calculates the Window Size From The Client Size
	SetWindowPos(NULL, 0, 0, rcClient.Width(), rcClient.Height(), SWP_NOMOVE);
	
	CreateToolTips();
	
	return 0;
}

BOOL CPaletteWnd::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		int nVirtKey = (int)pMsg->wParam;
		if (nVirtKey == VK_ESCAPE)
		{
			OnClose();
			return TRUE;
		}
	}

    m_wndToolTip.RelayEvent(pMsg);
	
	return CWnd::PreTranslateMessage(pMsg);
}

void  CPaletteWnd::SetCurrentColor(COLORREF crCurrent)
{
	int nIndex = 0;
	for (int nColorRow = 0 ; nColorRow < m_nColorMapHeight ; nColorRow++)
	{
		for (int nColorCol = 0 ; nColorCol < m_nColorMapWidth ; nColorCol++)
		{
			if (ColorMap[nColorRow][nColorCol] == crCurrent)
			{
				m_nIndex = nIndex;
				m_rc.SetRect((nColorCol * (BTN_SIZE + 1) + 1), 
							 (nColorRow * (BTN_SIZE + 1) + 1),
							 (nColorCol * (BTN_SIZE + 1) + 1) + BTN_SIZE, 				 
							 (nColorRow * (BTN_SIZE + 1) + 1) + BTN_SIZE);

				RedrawWindow();
				return;
			}
			nIndex++;
		}
	}
}

void CPaletteWnd::PostNcDestroy() 
{
	m_pParentWnd->SendMessage(WM_COLOR_PICKER_CLOSED, 0, 0);
	delete this;
	CWnd::PostNcDestroy();
}
