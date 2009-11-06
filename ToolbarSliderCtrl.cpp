#include "stdafx.h"
#include "ToolbarSliderCtrl.h"
#include "XThemeHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED     0x031A
#endif

CToolbarSliderCtrl::CToolbarSliderCtrl()
{
	m_hTheme = NULL;
}

CToolbarSliderCtrl::~CToolbarSliderCtrl()
{
	if (m_hTheme)
		ThemeHelper.CloseThemeData(m_hTheme);
	m_hTheme = NULL;
}

int CToolbarSliderCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CSliderCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Open Thema Data
	if (ThemeHelper.IsThemeLibAvailable())
		m_hTheme = ThemeHelper.OpenThemeData(m_hWnd, _T("Toolbar"));

	return 0;
}

BEGIN_MESSAGE_MAP(CToolbarSliderCtrl, CSliderCtrl)
	//{{AFX_MSG_MAP(CToolbarSliderCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CToolbarSliderCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) 
{
	/*
	// itemState flags
	#define CDIS_SELECTED       0x0001
	#define CDIS_GRAYED         0x0002
	#define CDIS_DISABLED       0x0004
	#define CDIS_CHECKED        0x0008
	#define CDIS_FOCUS          0x0010
	#define CDIS_DEFAULT        0x0020
	#define CDIS_HOT            0x0040
	#define CDIS_MARKED         0x0080
	#define CDIS_INDETERMINATE  0x0100
	*/

	LPNMCUSTOMDRAW lpcd = (LPNMCUSTOMDRAW)pNMHDR;
	UINT drawStage = lpcd->dwDrawStage;
    UINT itemSpec = lpcd->dwItemSpec;
	CRect rcClient;
	GetClientRect(rcClient);

    switch (drawStage)
    {
		case CDDS_PREPAINT:
			// Before the paint cycle begins.
			// This is the most important of the drawing stages, where we
			// must return CDRF_NOTIFYITEMDRAW or else we will not get further 
			// NM_CUSTOMDRAW notifications for this drawing cycle
			// we also return CDRF_NOTIFYPOSTPAINT so that we will get post-paint
			// notifications
			*pResult = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;	// This Calls Will CDDS_POSTPAINT
			break;

		case CDDS_PREERASE:      // Before the erase cycle begins
		case CDDS_POSTERASE:     // After the erase cycle is complete
		case CDDS_ITEMPREERASE:  // Before an item is erased
		case CDDS_ITEMPOSTERASE: // After an item has been erased
			*pResult = CDRF_DODEFAULT;
			// Not Called By Sliders...
			break;

		case CDDS_ITEMPREPAINT:		// Before an item is drawn
		{
			CDC* pDC = CDC::FromHandle(lpcd->hdc);

			switch (itemSpec)
			{
				case TBCD_CHANNEL:
				{	
					// Erase Bkg
					COLORREF crBackground;
					if (IsThemed())
					{
						ThemeHelper.GetThemeColor(	m_hTheme,
													TP_BUTTON,
													TS_NORMAL,
													TMT_FILLCOLOR,
													&crBackground);
					}
					else
						crBackground = ::GetSysColor(COLOR_BTNFACE);
					pDC->FillSolidRect(&rcClient, crBackground);

					*pResult = CDRF_DODEFAULT;
					break;
				}

				case TBCD_TICS:
				{
					*pResult = CDRF_SKIPDEFAULT;
					break;
				}

				case TBCD_THUMB:
				{
					*pResult = CDRF_DODEFAULT;
					break;
				}

				default:
					// all of a slider's items have been listed, so we shouldn't get here
					ASSERT(FALSE);
					break;
			};

			break;
		}

		case CDDS_ITEMPOSTPAINT:    // After an item has been drawn
		{
			switch (itemSpec)
			{
				case TBCD_CHANNEL:
				{
					*pResult = CDRF_DODEFAULT;
					break;
				}

				case TBCD_TICS:
				{
					*pResult = CDRF_DODEFAULT;
					break;
				}

				case TBCD_THUMB:
				{
					*pResult = CDRF_DODEFAULT;
					break;
				}

				default:
					// all of a slider's items have been listed, so we shouldn't get here
					ASSERT(FALSE);
					break;
			};

			break;
		}

		case CDDS_POSTPAINT:
		{
			// After the paint cycle is complete.
			// This is the post-paint for the entire control, and it's possible to 
			// add to whatever is now visible on the control.
			*pResult = CDRF_DODEFAULT;
			break;
		}

		default:
			// all drawing stages are listed, so we shouldn't get here
			ASSERT(FALSE);
			break;
    }
}

BOOL CToolbarSliderCtrl::IsThemed()
{
	return (m_hTheme &&
			ThemeHelper.IsAppThemed() &&
			ThemeHelper.IsThemeComCtl32());
}

BOOL CToolbarSliderCtrl::OnEraseBkgnd(CDC* pDC) 
{	
	return TRUE;
}

LRESULT CToolbarSliderCtrl::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
		case WM_THEMECHANGED:
		{
			if (IsThemed())
			{
				// when user changes themes, close current theme and re-open
				ThemeHelper.CloseThemeData(m_hTheme);
				m_hTheme = ThemeHelper.OpenThemeData(m_hWnd, _T("Toolbar"));
			}
		}
		break;
	}

	return CSliderCtrl::DefWindowProc(message, wParam, lParam);
}
