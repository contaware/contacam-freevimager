// ColourPopup.cpp : implementation file
//
// Written by Chris Maunder (chrismaunder@codeguru.com)
// Extended by Alexander Bischofberger (bischofb@informatik.tu-muenchen.de)
// Copyright (c) 1998.
//
// Updated 30 May 1998 to allow any number of colours, and to
//                     make the appearance closer to Office 97. 
//                     Also added "Default" text area.         (CJM)
//
//         13 June 1998 Fixed change of focus bug (CJM)
//         30 June 1998 Fixed bug caused by focus bug fix (D'oh!!)
//                      Solution suggested by Paul Wilkerson.
//
// ColourPopup is a helper class for the colour picker control
// CColourPicker. Check out the header file or the accompanying 
// HTML doc file for details.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Expect bugs.
// 
// Please use and enjoy. Please let me know of any bugs/mods/improvements 
// that you have found/implemented and I will fix/incorporate them into this
// file. 

#include "stdafx.h"
#include <math.h>
#include "uImager.h"
#include "ColourPopup.h"
#include "ColorButtonPicker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef SPI_GETFLATMENU
#define SPI_GETFLATMENU			0x1022
#endif

#ifndef SPI_GETDROPSHADOW
#define SPI_GETDROPSHADOW		0x1024
#endif

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW			0x00020000
#endif

#ifndef COLOR_MENUHILIGHT
#define COLOR_MENUHILIGHT		29
#endif

#define DEFAULT_BOX_VALUE		-3
#define CUSTOM_BOX_VALUE		-2
#define INVALID_COLOUR			-1

#define MAX_COLOURS				100


ColourTableEntry CColourPopup::m_crColours[] = 
{
    { RGB(0x00, 0x00, 0x00),    _T("Black")             },
    { RGB(0xA5, 0x2A, 0x00),    _T("Brown")             },
    { RGB(0x00, 0x40, 0x40),    _T("Dark Olive Green")  },
    { RGB(0x00, 0x55, 0x00),    _T("Dark Green")        },
    { RGB(0x00, 0x00, 0x5E),    _T("Dark Teal")         },
    { RGB(0x00, 0x00, 0x8B),    _T("Dark blue")         },
    { RGB(0x4B, 0x00, 0x82),    _T("Indigo")            },
    { RGB(0x28, 0x28, 0x28),    _T("Dark grey")         },

    { RGB(0x8B, 0x00, 0x00),    _T("Dark red")          },
    { RGB(0xFF, 0x68, 0x20),    _T("Orange")            },
    { RGB(0x8B, 0x8B, 0x00),    _T("Dark yellow")       },
    { RGB(0x00, 0x93, 0x00),    _T("Green")             },
    { RGB(0x38, 0x8E, 0x8E),    _T("Teal")              },
    { RGB(0x00, 0x00, 0xFF),    _T("Blue")              },
    { RGB(0x7B, 0x7B, 0xC0),    _T("Blue-grey")         },
    { RGB(0x66, 0x66, 0x66),    _T("Grey - 40")         },

    { RGB(0xFF, 0x00, 0x00),    _T("Red")               },
    { RGB(0xFF, 0xAD, 0x5B),    _T("Light orange")      },
    { RGB(0x32, 0xCD, 0x32),    _T("Lime")              }, 
    { RGB(0x3C, 0xB3, 0x71),    _T("Sea green")         },
    { RGB(0x7F, 0xFF, 0xD4),    _T("Aqua")              },
    { RGB(0x7D, 0x9E, 0xC0),    _T("Light blue")        },
    { RGB(0x80, 0x00, 0x80),    _T("Violet")            },
    { RGB(0x7F, 0x7F, 0x7F),    _T("Grey - 50")         },

    { RGB(0xFF, 0xC0, 0xCB),    _T("Pink")              },
    { RGB(0xFF, 0xD7, 0x00),    _T("Gold")              },
    { RGB(0xFF, 0xFF, 0x00),    _T("Yellow")            },    
    { RGB(0x00, 0xFF, 0x00),    _T("Bright green")      },
    { RGB(0x40, 0xE0, 0xD0),    _T("Turquoise")         },
    { RGB(0xC0, 0xFF, 0xFF),    _T("Skyblue")           },
    { RGB(0x48, 0x00, 0x48),    _T("Plum")              },
    { RGB(0xC0, 0xC0, 0xC0),    _T("Light grey")        },

    { RGB(0xFF, 0xE4, 0xE1),    _T("Rose")              },
    { RGB(0xD2, 0xB4, 0x8C),    _T("Tan")               },
    { RGB(0xFF, 0xFF, 0xE0),    _T("Light yellow")      },
    { RGB(0x98, 0xFB, 0x98),    _T("Pale green ")       },
    { RGB(0xAF, 0xEE, 0xEE),    _T("Pale turquoise")    },
    { RGB(0x68, 0x83, 0x8B),    _T("Pale blue")         },
    { RGB(0xE6, 0xE6, 0xFA),    _T("Lavender")          },
    { RGB(0xFF, 0xFF, 0xFF),    _T("White")             }
};

//
// Sizing definitions
//
static const CSize s_sizeTextHiBorder (3, 3);
static const CSize s_sizeTextMargin (2, 2);
static const CSize s_sizeBoxHiBorder (2, 2);
static const CSize s_sizeBoxMargin (0, 0);
static const CSize s_sizeBoxCore (14, 14);

/////////////////////////////////////////////////////////////////////////////
// CColourPopup

CColourPopup::CColourPopup()
{
	m_pParent           = NULL;
	m_crColour          = m_crInitialColour = RGB(0,0,0);
	m_strDefaultText = _T("");
    m_strCustomText  = _T("");
	m_strProfileSection  = _T("");

    Initialise();
}

CColourPopup::CColourPopup(CPoint p,
						   COLORREF crColour,
						   CWnd* pParentWnd,
                           LPCTSTR szDefaultText /* = NULL */,
                           LPCTSTR szCustomText  /* = NULL */,
						   LPCTSTR szProfileSection  /* = NULL */)
{
    m_pParent        = pParentWnd;
	m_crColour       = m_crInitialColour = crColour;
    m_strDefaultText = (szDefaultText)	? szDefaultText	: _T("");
    m_strCustomText  = (szCustomText)	? szCustomText	: _T("");
	m_strProfileSection  = (szProfileSection)	? szProfileSection	: _T("");

	Initialise();

    CColourPopup::Create(p, crColour, pParentWnd, szDefaultText, szCustomText);
}

void CColourPopup::Initialise()
{
	// Init Vars
    m_nNumColumns       = 0;
    m_nNumRows          = 0;
    m_nBoxSize          = 18;
    m_nMargin           = ::GetSystemMetrics(SM_CXEDGE);
    m_nCurrentSel       = INVALID_COLOUR;
    m_nChosenColourSel  = INVALID_COLOUR;
    m_bChildWindowVisible = FALSE;
	m_nNumColours       = sizeof(m_crColours) / sizeof(ColourTableEntry);
    ASSERT(m_nNumColours <= MAX_COLOURS);
    if (m_nNumColours > MAX_COLOURS)
        m_nNumColours = MAX_COLOURS;

	// Flat Menus?
	if (m_pParent)
	{
		m_bThemed = ((CColorButtonPicker*)m_pParent)->IsThemed();
		m_bFlatmenus = FALSE;
 		if (m_bThemed)
			::SystemParametersInfo (SPI_GETFLATMENU, 0, &m_bFlatmenus, FALSE);
	}
	else
	{
		m_bThemed = FALSE;
		m_bFlatmenus = FALSE;
	}

	// Get all the colors I need
	int nAlpha = 48;
	m_clrBackground = ::GetSysColor(COLOR_MENU);
	m_clrHiLightBorder = ::GetSysColor(COLOR_HIGHLIGHT);
	m_clrHiLight = m_clrHiLightBorder;
	if (m_bThemed)
		m_clrHiLight = ::GetSysColor(COLOR_MENUHILIGHT);
	m_clrHiLightText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	m_clrText = ::GetSysColor(COLOR_MENUTEXT);
	m_clrLoLight = RGB (
		(GetRValue (m_clrBackground) * (255 - nAlpha) + 
			GetRValue (m_clrHiLightBorder) * nAlpha) >> 8,
		(GetGValue (m_clrBackground) * (255 - nAlpha) + 
			GetGValue (m_clrHiLightBorder) * nAlpha) >> 8,
		(GetBValue (m_clrBackground) * (255 - nAlpha) + 
			GetBValue (m_clrHiLightBorder) * nAlpha) >> 8);

    // Idiot check: Make sure the colour square is at least 5 x 5;
    if (m_nBoxSize - 2*m_nMargin - 2 < 5)
		m_nBoxSize = 5 + 2*m_nMargin + 2;

    // Create the font
#if (WINVER >= 0x0600)
    NONCLIENTMETRICS ncm = {sizeof(NONCLIENTMETRICS) - sizeof(int)}; // to make it work with all OSs leave it like this!
#else
	NONCLIENTMETRICS ncm = {sizeof(NONCLIENTMETRICS)};
#endif
    VERIFY(::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0));
    m_Font.CreateFontIndirect(&(ncm.lfMessageFont));

    // Create the palette
    struct {
        LOGPALETTE    LogPalette;
        PALETTEENTRY  PalEntry[MAX_COLOURS];
    } pal;
    LOGPALETTE* pLogPalette = (LOGPALETTE*) &pal;
    pLogPalette->palVersion    = 0x300;
    pLogPalette->palNumEntries = (WORD) m_nNumColours; 
    for (int i = 0; i < m_nNumColours; i++)
    {
        pLogPalette->palPalEntry[i].peRed   = GetRValue(m_crColours[i].crColour);
        pLogPalette->palPalEntry[i].peGreen = GetGValue(m_crColours[i].crColour);
        pLogPalette->palPalEntry[i].peBlue  = GetBValue(m_crColours[i].crColour);
        pLogPalette->palPalEntry[i].peFlags = 0;
    }
    m_Palette.CreatePalette(pLogPalette);
}

CColourPopup::~CColourPopup()
{
    m_Font.DeleteObject();
    m_Palette.DeleteObject();
}

BOOL CColourPopup::Create(CPoint p, COLORREF crColour, CWnd* pParentWnd,
                          LPCTSTR szDefaultText /* = NULL */,
                          LPCTSTR szCustomText  /* = NULL */)
{
    ASSERT(pParentWnd && ::IsWindow(pParentWnd->GetSafeHwnd()));
  
    m_pParent  = pParentWnd;
    m_crColour = m_crInitialColour = crColour;

	// Get the class name and create the window
	UINT nClassStyle = CS_CLASSDC|CS_SAVEBITS|CS_HREDRAW|CS_VREDRAW;
	if (m_bThemed)
	{
		BOOL bDropShadow;
		::SystemParametersInfo(SPI_GETDROPSHADOW, 0, &bDropShadow, FALSE);
		if (bDropShadow)
			nClassStyle |= CS_DROPSHADOW;
	}
    CString szClassName = AfxRegisterWndClass(nClassStyle,
                                              0,
                                              (HBRUSH) (COLOR_MENU+1), 
                                              0);

    // Finding the window's topmost parent
	CWnd* pTopParent = pParentWnd->GetParentOwner();
	if (pTopParent != NULL)
		pTopParent->SetRedraw( FALSE );

	BOOL bCreate = CWnd::CreateEx(0, szClassName, _T(""), WS_VISIBLE|WS_POPUP, 
                        p.x, p.y, 100, 100, // size updated soon
                        pParentWnd->GetSafeHwnd(), 0, NULL);
	if (pTopParent != NULL)
	{
		pTopParent->SendMessage(WM_NCACTIVATE, TRUE);
		pTopParent->SetRedraw(TRUE);
	}
	if (!bCreate)
		return FALSE;

    // Store the Custom text
    if (szCustomText != NULL) 
        m_strCustomText = szCustomText;

    // Store the Default Area text
    if (szDefaultText != NULL) 
        m_strDefaultText = szDefaultText;
        
    // Set the window size
    SetWindowSize();

    // Create the tooltips
    CreateToolTips();

    // Find which cell (if any) corresponds to the initial colour
    FindCellFromColour(crColour);

    // Capture all mouse events for the life of this window
    SetCapture();

    return TRUE;
}

BEGIN_MESSAGE_MAP(CColourPopup, CWnd)
    //{{AFX_MSG_MAP(CColourPopup)
    ON_WM_NCDESTROY()
    ON_WM_LBUTTONUP()
    ON_WM_PAINT()
    ON_WM_MOUSEMOVE()
    ON_WM_KEYDOWN()
    ON_WM_QUERYNEWPALETTE()
    ON_WM_PALETTECHANGED()
	ON_WM_KILLFOCUS()
	ON_WM_ACTIVATEAPP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColourPopup message handlers

// For tooltips
BOOL CColourPopup::PreTranslateMessage(MSG* pMsg) 
{
    m_ToolTip.RelayEvent(pMsg);

    // Fix (Adrian Roman): Sometimes if the picker loses focus it is never destroyed
    if (GetCapture()->GetSafeHwnd() != m_hWnd)
        SetCapture(); 

    return CWnd::PreTranslateMessage(pMsg);
}

// If an arrow key is pressed, then move the selection
void CColourPopup::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    int row = GetRow(m_nCurrentSel),
        col = GetColumn(m_nCurrentSel);

    if (nChar == VK_DOWN) 
    {
        if (row == DEFAULT_BOX_VALUE) 
            row = col = 0; 
        else if (row == CUSTOM_BOX_VALUE)
        {
            if (m_strDefaultText.GetLength())
                row = col = DEFAULT_BOX_VALUE;
            else
                row = col = 0;
        }
        else
        {
            row++;
            if (GetIndex(row,col) < 0)
            {
                if (m_strCustomText.GetLength())
                    row = col = CUSTOM_BOX_VALUE;
                else if (m_strDefaultText.GetLength())
                    row = col = DEFAULT_BOX_VALUE;
                else
                    row = col = 0;
            }
        }
        ChangeSelection(GetIndex(row, col));
    }

    if (nChar == VK_UP) 
    {
        if (row == DEFAULT_BOX_VALUE)
        {
            if (m_strCustomText.GetLength())
                row = col = CUSTOM_BOX_VALUE;
            else
           { 
                row = GetRow(m_nNumColours-1); 
                col = GetColumn(m_nNumColours-1); 
            }
        }
        else if (row == CUSTOM_BOX_VALUE)
        { 
            row = GetRow(m_nNumColours-1); 
            col = GetColumn(m_nNumColours-1); 
        }
        else if (row > 0) row--;
        else /* row == 0 */
        {
            if (m_strDefaultText.GetLength())
                row = col = DEFAULT_BOX_VALUE;
            else if (m_strCustomText.GetLength())
                row = col = CUSTOM_BOX_VALUE;
            else
            { 
                row = GetRow(m_nNumColours-1); 
                col = GetColumn(m_nNumColours-1); 
            }
        }
        ChangeSelection(GetIndex(row, col));
    }

    if (nChar == VK_RIGHT) 
    {
        if (row == DEFAULT_BOX_VALUE) 
            row = col = 0; 
        else if (row == CUSTOM_BOX_VALUE)
        {
            if (m_strDefaultText.GetLength())
                row = col = DEFAULT_BOX_VALUE;
            else
                row = col = 0;
        }
        else if (col < m_nNumColumns-1) 
            col++;
        else 
        { 
            col = 0; row++;
        }

        if (GetIndex(row,col) == INVALID_COLOUR)
        {
            if (m_strCustomText.GetLength())
                row = col = CUSTOM_BOX_VALUE;
            else if (m_strDefaultText.GetLength())
                row = col = DEFAULT_BOX_VALUE;
            else
                row = col = 0;
        }

        ChangeSelection(GetIndex(row, col));
    }

    if (nChar == VK_LEFT) 
    {
        if (row == DEFAULT_BOX_VALUE)
        {
            if (m_strCustomText.GetLength())
                row = col = CUSTOM_BOX_VALUE;
            else
           { 
                row = GetRow(m_nNumColours-1); 
                col = GetColumn(m_nNumColours-1); 
            }
        }
        else if (row == CUSTOM_BOX_VALUE)
        { 
            row = GetRow(m_nNumColours-1); 
            col = GetColumn(m_nNumColours-1); 
        }
        else if (col > 0) col--;
        else /* col == 0 */
        {
            if (row > 0) { row--; col = m_nNumColumns-1; }
            else 
            {
                if (m_strDefaultText.GetLength())
                    row = col = DEFAULT_BOX_VALUE;
                else if (m_strCustomText.GetLength())
                    row = col = CUSTOM_BOX_VALUE;
                else
                { 
                    row = GetRow(m_nNumColours-1); 
                    col = GetColumn(m_nNumColours-1); 
                }
            }
        }
        ChangeSelection(GetIndex(row, col));
    }

    if (nChar == VK_ESCAPE) 
    {
        m_crColour = m_crInitialColour;
        EndSelection(CPN_SELENDCANCEL);
        return;
    }

    if (nChar == VK_RETURN || nChar == VK_SPACE)
    {
        EndSelection(CPN_SELENDOK);
        return;
    }

    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

// auto-deletion
void CColourPopup::OnNcDestroy() 
{
    CWnd::OnNcDestroy();
    delete this;
}

void CColourPopup::OnPaint() 
{
    CPaintDC dc(this); // device context for painting

    // Draw the Default Area text
    if (m_strDefaultText.GetLength())
        DrawCell(&dc, DEFAULT_BOX_VALUE);
 
    // Draw colour cells
    for (int i = 0; i < m_nNumColours; i++)
        DrawCell(&dc, i);
    
    // Draw custom text
    if (m_strCustomText.GetLength())
        DrawCell(&dc, CUSTOM_BOX_VALUE);

    // Draw raised window edge (ex-window style WS_EX_WINDOWEDGE is sposed to do this,
    // but for some reason isn't
    CRect rect;
    GetClientRect(rect);
	DrawBorder(&dc, rect, EDGE_RAISED, BF_RECT);
}

void CColourPopup::OnMouseMove(UINT nFlags, CPoint point) 
{
    int nNewSelection = INVALID_COLOUR;

    // Translate points to be relative raised window edge
    point.x -= m_nMargin;
    point.y -= m_nMargin;

    // First check we aren't in text box
    if (m_strCustomText.GetLength() && m_CustomTextRect.PtInRect(point))
        nNewSelection = CUSTOM_BOX_VALUE;
    else if (m_strDefaultText.GetLength() && m_DefaultTextRect.PtInRect(point))
        nNewSelection = DEFAULT_BOX_VALUE;
	else if (!m_BoxesRect.PtInRect(point))
		nNewSelection = INVALID_COLOUR;
    else
    {
        // Take into account text box
        if (m_strDefaultText.GetLength()) 
            point.y -= m_DefaultTextRect.Height();  

        // Get the row and column
        nNewSelection = GetIndex(point.y / m_nBoxSize, point.x / m_nBoxSize);

        // In range? If not, default and exit
        if (nNewSelection < 0 || nNewSelection >= m_nNumColours)
        {
            //CWnd::OnMouseMove(nFlags, point);
            //return;
			nNewSelection = INVALID_COLOUR;
        }
    }

    // OK - we have the row and column of the current selection (may be CUSTOM_BOX_VALUE)
    // Has the row/col selection changed? If yes, then redraw old and new cells.
    if (nNewSelection != m_nCurrentSel)
        ChangeSelection(nNewSelection);

    CWnd::OnMouseMove(nFlags, point);
}

// End selection on LButtonUp
void CColourPopup::OnLButtonUp(UINT nFlags, CPoint point) 
{    
    CWnd::OnLButtonUp(nFlags, point);

    DWORD pos = GetMessagePos();
    point = CPoint(LOWORD(pos), HIWORD(pos));

    if (m_WindowRect.PtInRect(point))
        EndSelection(CPN_SELENDOK);
    else
        EndSelection(CPN_SELENDCANCEL);
}

/////////////////////////////////////////////////////////////////////////////
// CColourPopup implementation

int CColourPopup::GetIndex(int row, int col) const
{ 
    if ((row == CUSTOM_BOX_VALUE || col == CUSTOM_BOX_VALUE) && m_strCustomText.GetLength())
        return CUSTOM_BOX_VALUE;
    else if ((row == DEFAULT_BOX_VALUE || col == DEFAULT_BOX_VALUE) && m_strDefaultText.GetLength())
        return DEFAULT_BOX_VALUE;
    else if (row < 0 || col < 0 || row >= m_nNumRows || col >= m_nNumColumns)
        return INVALID_COLOUR;
    else
    {
        if (row*m_nNumColumns + col >= m_nNumColours)
            return INVALID_COLOUR;
        else
            return row*m_nNumColumns + col;
    }
}

int CColourPopup::GetRow(int nIndex) const               
{ 
    if (nIndex == CUSTOM_BOX_VALUE && m_strCustomText.GetLength())
        return CUSTOM_BOX_VALUE;
    else if (nIndex == DEFAULT_BOX_VALUE && m_strDefaultText.GetLength())
        return DEFAULT_BOX_VALUE;
    else if (nIndex < 0 || nIndex >= m_nNumColours)
        return INVALID_COLOUR;
    else
        return nIndex / m_nNumColumns; 
}

int CColourPopup::GetColumn(int nIndex) const            
{ 
    if (nIndex == CUSTOM_BOX_VALUE && m_strCustomText.GetLength())
        return CUSTOM_BOX_VALUE;
    else if (nIndex == DEFAULT_BOX_VALUE && m_strDefaultText.GetLength())
        return DEFAULT_BOX_VALUE;
    else if (nIndex < 0 || nIndex >= m_nNumColours)
        return INVALID_COLOUR;
    else
        return nIndex % m_nNumColumns; 
}

void CColourPopup::FindCellFromColour(COLORREF crColour)
{
    if (crColour == CLR_DEFAULT && m_strDefaultText.GetLength())
    {
        m_nChosenColourSel = DEFAULT_BOX_VALUE;
        return;
    }

    for (int i = 0; i < m_nNumColours; i++)
    {
        if (GetColour(i) == crColour)
        {
            m_nChosenColourSel = i;
            return;
        }
    }

    if (m_strCustomText.GetLength())
        m_nChosenColourSel = CUSTOM_BOX_VALUE;
    else
        m_nChosenColourSel = INVALID_COLOUR;
}

// Gets the dimensions of the colour cell given by (row,col)
BOOL CColourPopup::GetCellRect(int nIndex, const LPRECT& rect)
{
    if (nIndex == CUSTOM_BOX_VALUE)
    {
        ::SetRect(rect, 
                  m_CustomTextRect.left,  m_CustomTextRect.top,
                  m_CustomTextRect.right, m_CustomTextRect.bottom);
        return TRUE;
    }
    else if (nIndex == DEFAULT_BOX_VALUE)
    {
        ::SetRect(rect, 
                  m_DefaultTextRect.left,  m_DefaultTextRect.top,
                  m_DefaultTextRect.right, m_DefaultTextRect.bottom);
        return TRUE;
    }

    if (nIndex < 0 || nIndex >= m_nNumColours)
        return FALSE;

    rect->left = GetColumn(nIndex) * m_nBoxSize + m_nMargin;
    rect->top  = GetRow(nIndex) * m_nBoxSize + m_nMargin;

    // Move everything down if we are displaying a default text area
    if (m_strDefaultText.GetLength()) 
        rect->top += (m_nMargin + m_DefaultTextRect.Height());

    rect->right = rect->left + m_nBoxSize;
    rect->bottom = rect->top + m_nBoxSize;

    return TRUE;
}

// Works out an appropriate size and position of this window
void CColourPopup::SetWindowSize()
{
    CSize TextSize;

    // If we are showing a custom or default text area, get the font and text size.
    if (m_strCustomText.GetLength() || m_strDefaultText.GetLength())
    {
        CClientDC dc(this);
        CFont* pOldFont = (CFont*) dc.SelectObject(&m_Font);

        // Get the size of the custom text (if there IS custom text)
        TextSize = CSize(0,0);
        if (m_strCustomText.GetLength())
            TextSize = dc.GetTextExtent(m_strCustomText);

        // Get the size of the default text (if there IS default text)
        if (m_strDefaultText.GetLength())
        {
            CSize DefaultSize = dc.GetTextExtent(m_strDefaultText);
            if (DefaultSize.cx > TextSize.cx) TextSize.cx = DefaultSize.cx;
            if (DefaultSize.cy > TextSize.cy) TextSize.cy = DefaultSize.cy;
        }

        dc.SelectObject(pOldFont);
        TextSize += CSize(2*m_nMargin,2*m_nMargin);

        // Add even more space to draw the horizontal line
        TextSize.cy += 2*m_nMargin + 2;
    }

    // Get the number of columns and rows
    //m_nNumColumns = (int) sqrt((double)m_nNumColours);    // for a square window (yuk)
    m_nNumColumns = 8;
    m_nNumRows = m_nNumColours / m_nNumColumns;
    if (m_nNumColours % m_nNumColumns) m_nNumRows++;

    // Get the current window position, and set the new size
    CRect rect;
    GetWindowRect(rect);

    m_WindowRect.SetRect(rect.left, rect.top, 
                         rect.left + m_nNumColumns*m_nBoxSize + 2*m_nMargin,
                         rect.top  + m_nNumRows*m_nBoxSize + 2*m_nMargin);

    // if custom text, then expand window if necessary, and set text width as
    // window width
    if (m_strDefaultText.GetLength()) 
    {
        if (TextSize.cx > m_WindowRect.Width())
            m_WindowRect.right = m_WindowRect.left + TextSize.cx;
        TextSize.cx = m_WindowRect.Width()-2*m_nMargin;

        // Work out the text area
        m_DefaultTextRect.SetRect(m_nMargin, m_nMargin, 
                                  m_nMargin+TextSize.cx, 2*m_nMargin+TextSize.cy);
        m_WindowRect.bottom += m_DefaultTextRect.Height() + 2*m_nMargin;
    }
	else
		m_DefaultTextRect.SetRectEmpty();

    // if custom text, then expand window if necessary, and set text width as
    // window width
    if (m_strCustomText.GetLength()) 
    {
        if (TextSize.cx > m_WindowRect.Width())
            m_WindowRect.right = m_WindowRect.left + TextSize.cx;
        TextSize.cx = m_WindowRect.Width()-2*m_nMargin;

        // Work out the text area
        m_CustomTextRect.SetRect(m_nMargin, m_WindowRect.Height(), 
                                 m_nMargin+TextSize.cx, 
                                 m_WindowRect.Height()+m_nMargin+TextSize.cy);
        m_WindowRect.bottom += m_CustomTextRect.Height() + m_nMargin;
	}
	else
		m_CustomTextRect.SetRectEmpty();

	//
	// Compute the min width
	//
	int nBoxTotalWidth = m_nNumColumns * m_nBoxSize;
	int nMinWidth = nBoxTotalWidth;
	if (nMinWidth < TextSize.cx)
		nMinWidth = TextSize.cx;

	//
	// Initialize the color box rectangle
	//
	m_BoxesRect = CRect(
						CPoint ((nMinWidth - nBoxTotalWidth) / 2, m_DefaultTextRect.bottom), 
						CSize (nBoxTotalWidth, m_nNumRows * m_nBoxSize)
						);
	
	// Clip the window rectangle to the nearest monitor
	HMONITOR hMonitor;
	MONITORINFO mi;
	hMonitor = ::MonitorFromWindow(m_pParent->m_hWnd, MONITOR_DEFAULTTONEAREST);
	mi.cbSize = sizeof(mi);
	::GetMonitorInfo(hMonitor, &mi);
	CRect rc = mi.rcWork;
	int w = m_WindowRect.Width();
	int h = m_WindowRect.Height();
	m_WindowRect.left = max(rc.left, min(rc.right-w, m_WindowRect.left));
	m_WindowRect.top = max(rc.top, min(rc.bottom-h, m_WindowRect.top));
	m_WindowRect.right = m_WindowRect.left + w;
	m_WindowRect.bottom = m_WindowRect.top + h;

    // Set the window size and position
    MoveWindow(m_WindowRect, TRUE);
}

void CColourPopup::CreateToolTips()
{
    // Create the tool tip
    if (!m_ToolTip.Create(this)) return;

    // Add a tool for each cell
    for (int i = 0; i < m_nNumColours; i++)
    {
        CRect rect;
        if (!GetCellRect(i, rect)) continue;
            m_ToolTip.AddTool(this, GetColourName(i), rect, 1);
    }
}

void CColourPopup::ChangeSelection(int nIndex)
{
    CClientDC dc(this);        // device context for drawing

    if (nIndex > m_nNumColours)
        nIndex = CUSTOM_BOX_VALUE; 

    if ((m_nCurrentSel >= 0 && m_nCurrentSel < m_nNumColours) ||
        m_nCurrentSel == CUSTOM_BOX_VALUE || m_nCurrentSel == DEFAULT_BOX_VALUE)
    {
        // Set Current selection as invalid and redraw old selection (this way
        // the old selection will be drawn unselected)
        int OldSel = m_nCurrentSel;
        m_nCurrentSel = INVALID_COLOUR;
        DrawCell(&dc, OldSel);
    }

    // Set the current selection as row/col and draw (it will be drawn selected)
    m_nCurrentSel = nIndex;
    DrawCell(&dc, m_nCurrentSel);

    // Store the current colour
    if (m_nCurrentSel == CUSTOM_BOX_VALUE)
        m_pParent->SendMessage(CPN_SELCHANGE, (WPARAM) m_crInitialColour, 0);
    else if (m_nCurrentSel == DEFAULT_BOX_VALUE)
    {
        m_crColour = CLR_DEFAULT;
        m_pParent->SendMessage(CPN_SELCHANGE, (WPARAM) CLR_DEFAULT, 0);
    }
	else if (m_nCurrentSel == INVALID_COLOUR)
	{
		m_crColour = INVALID_COLOUR;
        m_pParent->SendMessage(CPN_SELCHANGE, (WPARAM) m_crInitialColour, 0);
	}
    else
    {
        m_crColour = GetColour(m_nCurrentSel);
        m_pParent->SendMessage(CPN_SELCHANGE, (WPARAM) m_crColour, 0);
    }
}

void CColourPopup::EndSelection(int nMessage)
{
    ReleaseCapture();

    // If custom text selected, perform a custom colour selection
    if (nMessage != CPN_SELENDCANCEL && m_nCurrentSel == CUSTOM_BOX_VALUE)
    {
        m_bChildWindowVisible = TRUE;

        CColorDialog dlg(m_crInitialColour, CC_FULLOPEN | CC_ANYCOLOR, this);
		COLORREF clCustomColors[16];
		CString szTemp;
		for (int i = 0 ; i < 16 ; i++)
		{
			if (m_strProfileSection.GetLength() > 0)
			{
				szTemp.Format(_T("BKG_CUSTOM_COLOR_%02d"), i);
				clCustomColors[i] = (COLORREF)::AfxGetApp()->GetProfileInt(m_strProfileSection, szTemp, RGB(255,255,255));
			}
			else
				clCustomColors[i] = RGB(255,255,255);
		}
		dlg.m_cc.lpCustColors = clCustomColors;

        if (dlg.DoModal() == IDOK)
            m_crColour = dlg.GetColor();
        else
            nMessage = CPN_SELENDCANCEL;

		if (m_strProfileSection.GetLength() > 0)
		{
			for (int i = 0 ; i < 16 ; i++)
			{
				szTemp.Format(_T("BKG_CUSTOM_COLOR_%02d"), i);
				::AfxGetApp()->WriteProfileInt(m_strProfileSection, szTemp, clCustomColors[i]);
			}
		}

        m_bChildWindowVisible = FALSE;
    } 

    if (nMessage == CPN_SELENDCANCEL)
        m_crColour = m_crInitialColour;

    m_pParent->SendMessage(nMessage, (WPARAM) m_crColour, 0);
    
    // Kill focus bug fixed by Martin Wawrusch
    if (!m_bChildWindowVisible)
        DestroyWindow();
}

void CColourPopup::DrawCell(CDC* pDC, int nIndex)
{
    //
	// Get the drawing rect
	//

	CRect rect;
	if (!GetCellRect(nIndex, &rect)) 
		return;

	//
	// Get the text pointer and colors
	//

	CString szText;
	COLORREF clrBox;
	CSize sizeMargin;
	CSize sizeHiBorder;
	UINT nBorder = 0;
	switch(nIndex)
	{
	case CUSTOM_BOX_VALUE:
		szText = m_strCustomText;
		sizeMargin = s_sizeTextMargin;
		sizeHiBorder = s_sizeTextHiBorder;
		nBorder = BF_TOP;
		break;

	case DEFAULT_BOX_VALUE:
		szText = m_strDefaultText;
		sizeMargin = s_sizeTextMargin;
		sizeHiBorder = s_sizeTextHiBorder;
		nBorder = BF_BOTTOM;
		break;

	default:
		szText = _T("");
		clrBox = GetColour(nIndex);
		sizeMargin = s_sizeBoxMargin;
		sizeHiBorder = s_sizeBoxHiBorder;
	}

	//
	// Based on the selectons, get our colors
	//

	COLORREF clrHiLight;
	COLORREF clrText;
	bool bSelected;
	COLORREF clr3dTopLeft, clr3dBottomRight;
	if (m_nCurrentSel == nIndex)
	{
		bSelected = true;
		clrHiLight = m_clrHiLight;
		clrText = (m_bFlatmenus ? m_clrHiLightText : m_clrText);
		clr3dTopLeft = ::GetSysColor(COLOR_3DHIGHLIGHT);
		clr3dBottomRight = ::GetSysColor(COLOR_3DSHADOW);
	}
	else if (m_nChosenColourSel == nIndex)
	{
		bSelected = true;
		clrHiLight = m_clrLoLight;
		clrText = m_clrText;
		clr3dTopLeft = ::GetSysColor(COLOR_3DSHADOW);
		clr3dBottomRight = ::GetSysColor(COLOR_3DHIGHLIGHT);
	}
	else
	{
		bSelected = false;
		clrHiLight = m_clrLoLight;
		clrText = m_clrText;
	}

	//
	// Select and realize the palette
	//

	CPalette* pOldPalette = NULL;
	if (szText == _T(""))
	{
		if (pDC->GetDeviceCaps(RASTERCAPS) & RC_PALETTE)
		{
			pOldPalette = pDC->SelectPalette(&m_Palette, FALSE);
			pDC->RealizePalette();
		}
	}

	//
	// If we are currently selected
	//

	if (bSelected)
	{

		//
		// If we have a background margin, then draw that
		//

		if (sizeMargin.cx > 0 || sizeMargin.cy > 0)
		{
			pDC->FillSolidRect(&rect, m_clrBackground);
			rect.DeflateRect(sizeMargin.cx, sizeMargin.cy);
		}

		//
		// Draw the selection rectagle
		//

		if(m_bFlatmenus)
		{
			CPen pen(PS_SOLID, 1,m_clrHiLightBorder);
			CPen *pOldPen = pDC->SelectObject(&pen);

			CBrush brush(clrHiLight);
			CBrush *pOldBrush = pDC->SelectObject(&brush);

			pDC->Rectangle(&rect);

			rect.DeflateRect(sizeHiBorder.cx - 1, sizeHiBorder.cy - 1);

			// restore DC
			pDC->SelectObject(pOldPen);
			pDC->SelectObject(pOldBrush);
		}
		else
		{
			pDC->FillSolidRect(&rect, m_clrBackground);
			pDC->Draw3dRect(&rect, clr3dTopLeft, clr3dBottomRight);
			rect.DeflateRect(sizeHiBorder.cx, sizeHiBorder.cy);
		}
	}

	//
	// Otherwise, we are not selected
	//

	else
	{
		// Draw the background

		pDC->FillSolidRect(&rect, m_clrBackground);
		rect.DeflateRect(sizeMargin.cx + sizeHiBorder.cx, sizeMargin.cy + sizeHiBorder.cy);
	}

	//
	// Draw custom text
	//

	if (szText != _T(""))
	{
        CFont *pOldFont = (CFont*) pDC->SelectObject(&m_Font);
		pDC->SetTextColor(clrText);
        pDC->SetBkMode(TRANSPARENT);
        pDC->DrawText(szText, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        pDC->SelectObject(pOldFont);
    }        

	//
	// Otherwise, draw color
	//

	else
	{

		//
		// Draw color
		//
		CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW));
		CPen* pOldPen = pDC->SelectObject(&pen);

		CBrush brush(PALETTERGB(GetRValue(GetColour(nIndex)), 
								GetGValue(GetColour(nIndex)), 
								GetBValue(GetColour(nIndex)) ));
		CBrush* pOldBrush = pDC->SelectObject(&brush);

		// Draw the cell colour
		rect.DeflateRect(1, 1);
		pDC->Rectangle(&rect);

		// Restore DC
		pDC->SelectObject(pOldPen);
		pDC->SelectObject(pOldBrush);
	}

	//
	// Draw border
	//

	if (nBorder)
	{
		CRect r;
		GetCellRect(nIndex, &r);
		r.InflateRect(-2, 1);
		DrawBorder(pDC, r, EDGE_ETCHED, nBorder);
	}

    // cleanup
    if (pOldPalette && pDC->GetDeviceCaps(RASTERCAPS) & RC_PALETTE)
        pDC->SelectPalette(pOldPalette, FALSE);
}

void CColourPopup::DrawBorder(CDC* pDC, CRect rect, UINT nEdge, UINT nBorder)
{
	if (m_bFlatmenus)
	{
		CPen pen(PS_SOLID, 1, ::GetSysColor(COLOR_GRAYTEXT));
		CPen *pOldPen = pDC->SelectObject(&pen);

		rect.DeflateRect(0, 0, 1, 1);

		if(nBorder & BF_TOP)
		{
			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.right+1, rect.top);
		}

		if(nBorder & BF_BOTTOM)
		{
			pDC->MoveTo(rect.left, rect.bottom);
			pDC->LineTo(rect.right+1, rect.bottom);
		}

		if(nBorder & BF_LEFT)
		{
			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.left, rect.bottom+1);
		}

		if(nBorder & BF_RIGHT)
		{
			pDC->MoveTo(rect.right, rect.top);
			pDC->LineTo(rect.right, rect.bottom+1);
		}

		pDC->SelectObject(pOldPen);
	}
	else
		pDC->DrawEdge(rect, nEdge, nBorder);
}

BOOL CColourPopup::OnQueryNewPalette() 
{
    Invalidate(FALSE);    
    return CWnd::OnQueryNewPalette();
}

void CColourPopup::OnPaletteChanged(CWnd* pFocusWnd) 
{
    CWnd::OnPaletteChanged(pFocusWnd);

    if (pFocusWnd->GetSafeHwnd() != GetSafeHwnd())
        Invalidate(FALSE);
}

void CColourPopup::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);

    ReleaseCapture();
    //DestroyWindow(); - causes crash when Custom colour dialog appears.
}

// KillFocus problem fix suggested by Paul Wilkerson.
void CColourPopup::OnActivateApp(BOOL bActive, DWORD dwThreadID) 
{
	CWnd::OnActivateApp(bActive, dwThreadID);

	// If Deactivating App, cancel this selection
	if (!bActive)
		 EndSelection(CPN_SELENDCANCEL);
}
