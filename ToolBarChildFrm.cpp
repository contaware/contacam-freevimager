#include "stdafx.h"
#include "MainFrm.h"
#include "uImager.h"
#include "ToolBarChildFrm.h"
#include "resource.h"
#include "uImagerDoc.h"
#include "VideoDeviceView.h"
#include "PictureView.h"
#include "PicturePrintPreviewView.h"
#include "mmsystem.h"
#include "XThemeHelper.h"
#include "CameraAdvancedSettingsPropertySheet.h"
#include "DxCapture.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED     0x031A
#endif

static const CRect TOOLBAR_BORDERRECT	= CRect(2,	// Left
												2,	// Top
												2,	// Right
												1);	// Bottom

/////////////////////////////////////////////////////////////////////////////
// CChildToolBar

IMPLEMENT_DYNAMIC(CChildToolBar, CToolBar)

CChildToolBar::CChildToolBar()
{
	m_nMinToolbarWidth = 0;
	m_nMaxToolbarWidth = 0;
	m_hTheme = NULL;
}

CChildToolBar::~CChildToolBar()
{
	if (m_hTheme)
		ThemeHelper.CloseThemeData(m_hTheme);
	m_hTheme = NULL;
}

BEGIN_MESSAGE_MAP(CChildToolBar, CToolBar)
	//{{AFX_MSG_MAP(CChildToolBar)
	ON_WM_NCPAINT()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CChildToolBar::Create(CWnd* pParentWnd)
{
	return CToolBar::CreateEx(	pParentWnd,
								TBSTYLE_FLAT, 
								(((CUImagerApp*)::AfxGetApp())->m_bShowToolbar ? WS_VISIBLE : 0) |
								WS_CHILD | CBRS_ALIGN_BOTTOM | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED,
								TOOLBAR_BORDERRECT);
}

int CChildToolBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CToolBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Open Thema Data
	if (ThemeHelper.IsThemeLibAvailable())
		m_hTheme = ThemeHelper.OpenThemeData(m_hWnd, _T("Toolbar"));
	
	return 0;
}

BOOL CChildToolBar::IsThemed()
{
	return (m_hTheme &&
			ThemeHelper.IsAppThemed() &&
			ThemeHelper.IsThemeComCtl32());
}

void CChildToolBar::OnNcPaint() 
{
	// Get window DC that is clipped to the non-client area
	CWindowDC dc(this);

	// Erase the toolbar background border
	
	// Get color
	COLORREF col = ::GetSysColor(COLOR_BTNFACE);
	if (IsThemed())
	{
		ThemeHelper.GetThemeColor(	m_hTheme,
									TP_BUTTON,
									TS_NORMAL,
									TMT_FILLCOLOR,
									&col);
	}

	// Set clip
	CRect rectClient;
	GetClientRect(rectClient);
	CRect rectWindow;
	GetWindowRect(rectWindow);
	ScreenToClient(rectWindow);
	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	dc.ExcludeClipRect(rectClient);

	// Fill with Brush
	rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);
	CBrush br;
	br.CreateSolidBrush(col);
	dc.FillRect(&rectWindow, &br);
	br.DeleteObject();

	// Erase parts not drawn
	dc.IntersectClipRect(rectWindow);
	
	// Do not call CToolBar::OnNcPaint() for painting messages
}

LRESULT CChildToolBar::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
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

	return CToolBar::DefWindowProc(message, wParam, lParam);
}

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CVideoDeviceToolBar

IMPLEMENT_DYNAMIC(CVideoDeviceToolBar, CChildToolBar)

CVideoDeviceToolBar::CVideoDeviceToolBar()
{

}

CVideoDeviceToolBar::~CVideoDeviceToolBar()
{

}

BEGIN_MESSAGE_MAP(CVideoDeviceToolBar, CChildToolBar)
	//{{AFX_MSG_MAP(CVideoDeviceToolBar)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CVideoDeviceToolBar::SwitchToolBar(int nDPI, BOOL bCallShowControlBar/*=TRUE*/)
{
	// NOTE
	// In the past there was a bug in MFC when switching toolbar,
	// but in new versions it seems to be solved:
	// http://www.verycomputer.com/418_a17ba2bef12732f0_1.htm

	// Load and set sizes
	if (nDPI > 192)
	{
		if (!LoadToolBar(IDR_VIDEO_DEVICE_TOOLBAR3X))
			return FALSE;
		SetSizes(	CSize(TOOLBAR_BUTTON_SIZE_3X, TOOLBAR_BUTTON_SIZE_3Y),
					CSize(TOOLBAR_IMAGE_SIZE_3X, TOOLBAR_IMAGE_SIZE_3Y));
	}
	else if (nDPI > 96)
	{
		if (!LoadToolBar(IDR_VIDEO_DEVICE_TOOLBAR2X))
			return FALSE;
		SetSizes(	CSize(TOOLBAR_BUTTON_SIZE_2X, TOOLBAR_BUTTON_SIZE_2Y),
					CSize(TOOLBAR_IMAGE_SIZE_2X, TOOLBAR_IMAGE_SIZE_2Y));
	}
	else
	{
		if (!LoadToolBar(IDR_VIDEO_DEVICE_TOOLBAR))
			return FALSE;
		SetSizes(	CSize(TOOLBAR_BUTTON_SIZE_X, TOOLBAR_BUTTON_SIZE_Y),
					CSize(TOOLBAR_IMAGE_SIZE_X, TOOLBAR_IMAGE_SIZE_Y));
	}

	// When a toolbar is already loaded then the ShowControlBar() call is necessary to correctly update it
	if (bCallShowControlBar)
		::AfxGetMainFrame()->ShowControlBar(this, IsWindowVisible(), TRUE);

	return TRUE;
}

BOOL CVideoDeviceToolBar::Create(CWnd* pParentWnd)
{
	// Create Toolbar
	if (!CChildToolBar::Create(pParentWnd))
		return FALSE;
	if (!SwitchToolBar(g_nSystemDPI, FALSE))
		return FALSE;

	// Add toolbar button dropdown arrows
	GetToolBarCtrl().SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);
	DWORD dwStyle = GetButtonStyle(CommandToIndex(ID_VIEW_FRAMETIME));
	dwStyle |= TBSTYLE_DROPDOWN;
	SetButtonStyle(CommandToIndex(ID_VIEW_FRAMETIME), dwStyle);
	dwStyle = GetButtonStyle(CommandToIndex(ID_CAPTURE_MOVDET));
	dwStyle |= TBSTYLE_DROPDOWN;
	SetButtonStyle(CommandToIndex(ID_CAPTURE_MOVDET), dwStyle);
	
	return TRUE;
}

void CVideoDeviceToolBar::UpdateControls(void)
{
	// Set Min Toolbar Width
	if (GetCount() > 0)
	{
		int nSeparatorCount = 0;
		int nButtonCount = 0;
		CRect rcSep(0,0,0,0);
		CRect rcButton(0,0,0,0);
		for (int i = 0 ; i < GetCount() ; i++)
		{
			if (GetButtonStyle(i) == TBBS_SEPARATOR)
			{
				nSeparatorCount++;
				if (rcSep.Width() == 0)
					GetItemRect(i, &rcSep);
			}
			else
			{
				CRect rc;
				GetItemRect(i, &rc);
				if (rc.Width() > 0)
				{
					nButtonCount++;
					if (rcButton.Width() == 0)
						rcButton = rc;
				}
			}
		}
		m_nMinToolbarWidth = 3 + rcButton.Width() * nButtonCount +
								rcSep.Width() * nSeparatorCount + 4;
	}
}

void CVideoDeviceToolBar::OnSize(UINT nType, int cx, int cy) 
{
	CToolBar::OnSize(nType, cx, cy);
	UpdateControls();
}

#endif

/////////////////////////////////////////////////////////////////////////////
// CPictureToolBar

IMPLEMENT_DYNAMIC(CPictureToolBar, CChildToolBar)

CPictureToolBar::CPictureToolBar()
{
	m_ZoomComboBoxIndex = -1;
	m_rcLastZoomComboBox = CRect(0,0,0,0);
	m_BkgColorButtonPickerIndex = -1;
	m_rcLastBkgColorButtonPicker = CRect(0,0,0,0);
}

CPictureToolBar::~CPictureToolBar()
{

}

BEGIN_MESSAGE_MAP(CPictureToolBar, CChildToolBar)
	//{{AFX_MSG_MAP(CPictureToolBar)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(CPN_SELENDOK, OnSelEndOK)
END_MESSAGE_MAP()

BOOL CPictureToolBar::SwitchToolBar(int nDPI, BOOL bCallShowControlBar/*=TRUE*/)
{
	// NOTE
	// In the past there was a bug in MFC when switching toolbar,
	// but in new versions it seems to be solved:
	// http://www.verycomputer.com/418_a17ba2bef12732f0_1.htm

	// Load and set sizes
	if (nDPI > 192)
	{
		if (!LoadToolBar(IDR_PICTURE_TOOLBAR3X))
			return FALSE;
		SetSizes(	CSize(TOOLBAR_BUTTON_SIZE_3X, TOOLBAR_BUTTON_SIZE_3Y),
					CSize(TOOLBAR_IMAGE_SIZE_3X, TOOLBAR_IMAGE_SIZE_3Y));
	}
	else if (nDPI > 96)
	{
		if (!LoadToolBar(IDR_PICTURE_TOOLBAR2X))
			return FALSE;
		SetSizes(	CSize(TOOLBAR_BUTTON_SIZE_2X, TOOLBAR_BUTTON_SIZE_2Y),
					CSize(TOOLBAR_IMAGE_SIZE_2X, TOOLBAR_IMAGE_SIZE_2Y));
	}
	else
	{
		if (!LoadToolBar(IDR_PICTURE_TOOLBAR))
			return FALSE;
		SetSizes(	CSize(TOOLBAR_BUTTON_SIZE_X, TOOLBAR_BUTTON_SIZE_Y),
					CSize(TOOLBAR_IMAGE_SIZE_X, TOOLBAR_IMAGE_SIZE_Y));
	}

	// When a toolbar is already loaded then the ShowControlBar() call is necessary to correctly update it
	if (bCallShowControlBar)
		::AfxGetMainFrame()->ShowControlBar(this, IsWindowVisible(), TRUE);

	return TRUE;
}

int CPictureToolBar::ScaleToolBar(int nDPI, int n)
{
	if (nDPI > 192)
		return 3*n;
	else if (nDPI > 96)
		return 2*n;
	else
		return n;
}

BOOL CPictureToolBar::Create(CWnd* pParentWnd)
{
	// Create Toolbar
	if (!CChildToolBar::Create(pParentWnd))
		return FALSE;
	if (!SwitchToolBar(g_nSystemDPI, FALSE))
		return FALSE;

	// Zoom Combo Box
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	_tcscpy(lf.lfFaceName, TOOLBAR_COMBOBOX_FONTFACENAME);
	HDC hDC = ::GetDC(GetSafeHwnd());
	lf.lfHeight = ScaleToolBar(g_nSystemDPI, TOOLBAR_COMBOBOX_FONTHEIGHT);
	::ReleaseDC(GetSafeHwnd(), hDC);
	lf.lfWeight = FW_NORMAL;
	m_ZoomComboBoxFont.CreateFontIndirect(&lf);
	m_ZoomComboBoxIndex = CommandToIndex(ID_ZOOM_COMBOX);
	if (m_ZoomComboBoxIndex != -1)
		if (!m_ZoomComboBox.Create(CBS_DROPDOWNLIST | WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this, ID_ZOOM_COMBOX))
			return FALSE;
	m_ZoomComboBox.SetFont(&m_ZoomComboBoxFont);
	m_ZoomComboBox.SetExtendedUI(TRUE);
	m_ZoomComboBox.Init();
	m_ZoomComboBox.EnableWindow(TRUE);

	// Color Button Picker
	m_BkgColorButtonPickerIndex = CommandToIndex(ID_BACKGROUND_COLOR);
	if (m_BkgColorButtonPickerIndex != -1)
		if (!m_BkgColorButtonPicker.Create(_T(""), WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this, ID_BACKGROUND_COLOR))
			return FALSE;
	m_BkgColorButtonPicker.SetToolbarButton(TRUE);
	m_BkgColorButtonPicker.Color			= ::GetSysColor(COLOR_WINDOW);
	m_BkgColorButtonPicker.DefaultColor		= ::GetSysColor(COLOR_WINDOW);
	m_BkgColorButtonPicker.TrackSelection	= TRUE;
	m_BkgColorButtonPicker.SetCustomText(ML_STRING(1576, "More Colors..."));
	m_BkgColorButtonPicker.SetDefaultText(ML_STRING(1273, "Default Background"));
	m_BkgColorButtonPicker.SetProfileSection(_T("GeneralApp"));
	m_BkgColorButtonPicker.EnableWindow(TRUE);

	return TRUE;
}

void CPictureToolBar::UpdateControls(void)
{
	CRect rect;

	// Place Zoom ComboBox
	if (::IsWindow(m_ZoomComboBox))
	{
		GetItemRect(m_ZoomComboBoxIndex, rect);
		rect.right = rect.left + ScaleToolBar(g_nSystemDPI, TOOLBAR_ZOOMCOMBOBOX_WIDTH);
		SetButtonInfo(	m_ZoomComboBoxIndex,
						ID_ZOOM_COMBOX,
						TBBS_SEPARATOR,
						rect.Width());
		rect.left += 2;
		(rect.right)--;
		m_ZoomComboBox.SetItemHeight(-1, rect.Height() - 4*::GetSystemMetrics(SM_CYEDGE) + 1); // height of the static-text control
		rect.bottom += 10; // set a minimum height for the dropdown to open and size itself

		// To Avoid Flickering Of The ComboBox
		if (m_rcLastZoomComboBox != rect)
		{
			m_ZoomComboBox.MoveWindow(&rect);
			m_rcLastZoomComboBox = rect;
		}
	}

	// Place Color Button Picker
	if (::IsWindow(m_BkgColorButtonPicker))
	{
		GetItemRect(m_BkgColorButtonPickerIndex, rect);
		rect.right = rect.left + ScaleToolBar(g_nSystemDPI, TOOLBAR_COLORBUTTONPICKER_WIDTH);
		SetButtonInfo(	m_BkgColorButtonPickerIndex,
						ID_BACKGROUND_COLOR,
						TBBS_SEPARATOR,
						rect.Width());
		rect.left += 2;
		(rect.right)--;
		
		// To Avoid Flickering Of The Color Button Picker
		if (m_rcLastBkgColorButtonPicker != rect)
		{
			m_BkgColorButtonPicker.MoveWindow(&rect);
			m_rcLastBkgColorButtonPicker = rect;
		}
	}

	// Set Min Toolbar Width
	if (GetCount() > 0)
	{
		int nSeparatorCount = 0;
		int nButtonCount = 0;
		CRect rcSep(0,0,0,0);
		CRect rcButton(0,0,0,0);
		for (int i = 0 ; i < GetCount() ; i++)
		{
			if (i != m_ZoomComboBoxIndex && i != m_BkgColorButtonPickerIndex)
			{
				if (GetButtonStyle(i) == TBBS_SEPARATOR)
				{
					nSeparatorCount++;
					if (rcSep.Width() == 0)
						GetItemRect(i, &rcSep);
				}
				else
				{
					GetItemRect(i, &rect);
					if (rect.Width() > 0)
					{
						nButtonCount++;
						if (rcButton.Width() == 0)
							rcButton = rect;
					}
				}
			}
		}
		m_nMinToolbarWidth =	m_rcLastZoomComboBox.Width() + 3 +
								m_rcLastBkgColorButtonPicker.Width() + 3 +
								rcButton.Width() * nButtonCount	+
								rcSep.Width() * nSeparatorCount + 4;
	}
}

void CPictureToolBar::OnSize(UINT nType, int cx, int cy) 
{
	CToolBar::OnSize(nType, cx, cy);
	UpdateControls();
}

LONG CPictureToolBar::OnSelEndOK(UINT /*lParam*/, LONG /*wParam*/)
{
	// Note: The CColorButtonPicker class
	//       inverted lParam with wParam!

    // Send WM_COMMAND
	GetParent()->SendMessage(WM_COMMAND,
							(WPARAM)ID_BACKGROUND_COLOR,
							(LPARAM)0);
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CToolBarChildFrame

IMPLEMENT_DYNCREATE(CToolBarChildFrame, CChildFrame)

CToolBarChildFrame::CToolBarChildFrame()
{
	m_pToolBar = NULL;
}

BEGIN_MESSAGE_MAP(CToolBarChildFrame, CChildFrame)
	//{{AFX_MSG_MAP(CToolBarChildFrame)
	ON_WM_SIZE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CToolBarChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CChildFrame::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create ToolBar
	if (m_pToolBar)
		if (!m_pToolBar->Create(this))
			return -1;

	return 0;
}
	
void CToolBarChildFrame::OnSize(UINT nType, int cx, int cy) 
{
	CChildFrame::OnSize(nType, cx, cy);

	CUImagerView* pView = (CUImagerView*)GetActiveView();
	if (!pView || !pView->IsKindOf(RUNTIME_CLASS(CUImagerView)))
		return;

	// Update ToolBar and m_ZoomRect
	pView->UpdateWindowSizes(FALSE, FALSE, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CZoomComboBox

const double dFit = 0.0;
const double dFitBig = -1.0;
const double dPercent6Quarter = 0.0625;
const double dPercent12Half = 0.125;
const double dPercent25 = 0.25;
const double dPercent50 = 0.5;
const double dPercent100 = 1.0;
const double dPercent200 = 2.0;
const double dPercent400 = 4.0;
const double dPercent800 = 8.0;
const double dPercent1600 = 16.0;

CZoomComboBox::CZoomComboBox()
{

}

BEGIN_MESSAGE_MAP(CZoomComboBox, CComboBox)
	//{{AFX_MSG_MAP(CZoomComboBox)
	ON_CONTROL_REFLECT(CBN_SELENDOK, OnSelEndOk)
	ON_CONTROL_REFLECT(CBN_CLOSEUP, OnCloseUp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CZoomComboBox::Init()
{
	SetItemDataPtr(AddString(ML_STRING(1822, "Fit")),	(void*)&dFit);
	SetItemDataPtr(AddString(ML_STRING(1823, "Fit Big")),(void*)&dFitBig);
	SetItemDataPtr(AddString(_T("6.25%")),	(void*)&dPercent6Quarter);
	SetItemDataPtr(AddString(_T("12.5%")),	(void*)&dPercent12Half);
	SetItemDataPtr(AddString(_T("25%")),	(void*)&dPercent25);
	SetItemDataPtr(AddString(_T("50%")),	(void*)&dPercent50);
	SetItemDataPtr(AddString(_T("100%")),	(void*)&dPercent100);
	SetItemDataPtr(AddString(_T("200%")),	(void*)&dPercent200);
	SetItemDataPtr(AddString(_T("400%")),	(void*)&dPercent400);
	SetItemDataPtr(AddString(_T("800%")),	(void*)&dPercent800);
	SetItemDataPtr(AddString(_T("1600%")),	(void*)&dPercent1600);

	// Set Fit
	SetCurSel(0);
}

void CZoomComboBox::OnChangeZoomFactor(double dZoomFactor)
{
	CPictureView* pView = (CPictureView*)(((CControlBar*)GetParent())->GetDockingFrame()->GetActiveView());
	ASSERT_VALID(pView);
	CPictureDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Avoid Transition While Changing Zoom Factor!
	pDoc->CancelTransition();
	pDoc->CancelLoadFullJpegTransition();

	if (dZoomFactor == 0.0) // Fit
	{
		// Lossless crop needs integer zoom factors!
		if (pDoc->m_bCrop && pDoc->m_bLosslessCrop)
			pDoc->CancelCrop();
		pView->FitZoomFactor();
	}
	else if (dZoomFactor == -1.0) // Fit Big
	{
		// Lossless crop needs integer zoom factors!
		if (pDoc->m_bCrop && pDoc->m_bLosslessCrop)
			pDoc->CancelCrop();
		pView->FitBigZoomFactor();
	}
	else
		pDoc->m_dZoomFactor = dZoomFactor;
	pView->CalcZoomedPixelAlign();
	pDoc->m_nZoomComboBoxIndex = GetCurSel();

	// Save Settings
	::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
									_T("ZoomComboBoxIndex"),
									pDoc->m_nZoomComboBoxIndex);
}

void CZoomComboBox::OnSelEndOk()
{
	int Index = GetCurSel();
	if (Index != CB_ERR)
	{
		CPictureView* pView = (CPictureView*)(((CControlBar*)GetParent())->GetDockingFrame()->GetActiveView());
		ASSERT_VALID(pView);
		CPictureDoc* pDoc = pView->GetDocument();
		ASSERT_VALID(pDoc);

		if ((int)GetItemDataPtr(Index) != -1)
		{
			OnChangeZoomFactor(*((double*)GetItemDataPtr(Index)));
			pDoc->SetDocumentTitle();
			pView->UpdateWindowSizes(TRUE, TRUE, FALSE);
			if (pDoc->m_bCrop)
			{
				if (pDoc->m_bNoBorders)
				{
					if (pView->IsXAndYScroll())
						pView->ScrollToPosition(CPoint(	pView->m_CropZoomRect.left,
														pView->m_CropZoomRect.top));
					else if (pView->IsXScroll())
						pView->ScrollToPosition(CPoint(	pView->m_CropZoomRect.left,
														0));
					else if (pView->IsYScroll())
						pView->ScrollToPosition(CPoint(	0,
														pView->m_CropZoomRect.top));
				}
				else
				{
					if (pView->IsXAndYScroll())
						pView->ScrollToPosition(CPoint(	pView->m_CropZoomRect.left - MIN_BORDER,
														pView->m_CropZoomRect.top - MIN_BORDER));
					else if (pView->IsXScroll())
						pView->ScrollToPosition(CPoint(	pView->m_CropZoomRect.left - MIN_BORDER,
														0));
					else if (pView->IsYScroll())
						pView->ScrollToPosition(CPoint(	0,
														pView->m_CropZoomRect.top - MIN_BORDER));
				}
			}
		}
	}
}

// The following restore code is necessary on some OSs.
// Do not use OnSelendcancel(), not working well on some
// OSs in conjunction with SetCurSel().
void CZoomComboBox::OnCloseUp()
{
	CPictureView* pView = (CPictureView*)(((CControlBar*)GetParent())->GetDockingFrame()->GetActiveView());
	ASSERT_VALID(pView);
	CPictureDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);
	SetCurSel(pDoc->m_nZoomComboBoxIndex);
	pView->SetFocus();
}

int CZoomComboBox::GetNextZoomIndex(double dZoomFactor)
{
	CPictureView* pView = (CPictureView*)(((CControlBar*)GetParent())->GetDockingFrame()->GetActiveView());
	ASSERT_VALID(pView);
	CPictureDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	if (dZoomFactor <= 0.0)
		return 2;	// Index of Smallest Zoom Factor
	else
	{
		// Zoom In
		for (int i = 2 ; i < GetCount() ; i++)
		{
			void* pData = GetItemDataPtr(i);
			if (*((double*)pData) > dZoomFactor)
				return i;
		}

		return (GetCount() - 1);	// Index of Biggest Zoom Factor
	}
}

int CZoomComboBox::GetPrevZoomIndex(double dZoomFactor)
{
	CPictureView* pView = (CPictureView*)(((CControlBar*)GetParent())->GetDockingFrame()->GetActiveView());
	ASSERT_VALID(pView);
	CPictureDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	if (dZoomFactor <= 0.0)
		return 2;	// Index of Smallest Zoom Factor
	else
	{
		// Zoom Out
		for (int i = (GetCount() - 1) ; i >= 2  ; i--)
		{
			void* pData = GetItemDataPtr(i);
			if (*((double*)pData) < dZoomFactor)
				return i;
		}

		return 2;	// Index of Smallest Zoom Factor
	}
}

BOOL CZoomComboBox::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CComboBox::Create(dwStyle | CBS_AUTOHSCROLL | WS_VSCROLL, rect, pParentWnd, nID); 
}

/////////////////////////////////////////////////////////////////////////////
// CPictureChildFrame

IMPLEMENT_DYNCREATE(CPictureChildFrame, CToolBarChildFrame)

CPictureChildFrame::CPictureChildFrame()
{

}

BEGIN_MESSAGE_MAP(CPictureChildFrame, CToolBarChildFrame)
	//{{AFX_MSG_MAP(CPictureChildFrame)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CPictureChildFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	return CChildFrame::PreCreateWindow(cs);
}

void CPictureChildFrame::OnTimer(UINT nIDEvent) 
{
	CToolBarChildFrame::OnTimer(nIDEvent);
	OnClose();
}

void CPictureChildFrame::OnClose() 
{
	CPictureDoc* pDoc = (CPictureDoc*)GetActiveDocument();
	ASSERT_VALID(pDoc);
	CPictureView* pView = pDoc->GetView();	// Get CPictureView this way, do not use GetActiveView()
	ASSERT_VALID(pView);					// because it can return CPicturePrintPreviewView!

	// Exit Full-Screen
	if (pView->m_bFullScreenMode)
		::AfxGetMainFrame()->EnterExitFullscreen();

	// Close Print Preview
	if (pView->GetPicturePrintPreviewView())
		pView->GetPicturePrintPreviewView()->Close();

	if (m_bFirstCloseAttempt)
	{
		// Start Closing only if Saved!
		if (pDoc->CanCloseFrame(this)) // This Calls CPictureDoc::SaveModified()
		{
			// Set Closing Flag
			::InterlockedExchange(&pDoc->m_bClosing, 1);

			// Clear Modified Flag
			pDoc->SetModifiedFlag(FALSE);

			// Cancel Pending Command?
			if (pDoc->m_dwIDAfterFullLoadCommand)
			{
				pDoc->m_dwIDAfterFullLoadCommand = 0;
				pView->EndWaitCursor();
				pView->ForceCursor(FALSE);
			}
			
			// Setup Timer?
			if (!((CUImagerApp*)::AfxGetApp())->m_bClosingAll)
				SetTimer(ID_TIMER_CLOSING_PICTUREDOC, CLOSING_CHECK_INTERVAL_TIMER_MS, NULL);

			// Start Shutdown
			m_bFirstCloseAttempt = FALSE;
			StartShutdown();
		}
	}
	else
	{
		if (IsShutdownDone() ||
			(::timeGetTime() - m_dwShutdownStartUpTime) >= MAX_PICTUREDOC_CLOSE_WAITTIME)
		{
			// Now that the layered thread exited we can close the dialog
			if (pDoc->m_pLayeredDlg)
			{
				// m_pLayeredDlg pointer is set to NULL
				// from the dialog class (selfdeletion)
				pDoc->m_pLayeredDlg->Close();
			}

			// Kill Timer?
			if (!((CUImagerApp*)::AfxGetApp())->m_bClosingAll)
				KillTimer(ID_TIMER_CLOSING_PICTUREDOC);

			// Destroy Window
			CToolBarChildFrame::OnClose();
		}
	}
}

void CPictureChildFrame::StartShutdown()
{
	CPictureDoc* pDoc = (CPictureDoc*)GetActiveDocument();
	ASSERT_VALID(pDoc);
	
	// Init timeout
	m_dwShutdownStartUpTime = ::timeGetTime();

	// Cancel Crop Tool
	pDoc->CancelCrop();

	// Do Not Draw Now!
	pDoc->m_bNoDrawing = TRUE;

	// Start Killing
	pDoc->m_SlideShowThread.Kill_NoBlocking();
	pDoc->m_bLoadFullJpegTransitionUI = FALSE;
	pDoc->m_bCancelLoadFullJpegTransition = FALSE;
	pDoc->m_JpegThread.Kill_NoBlocking();
	pDoc->m_LoadPicturesThread.Kill_NoBlocking();
	pDoc->m_LayeredDlgThread.Kill_NoBlocking();
	pDoc->m_GifAnimationThread.Kill_NoBlocking();
	pDoc->CancelTransition();
	
	// Close Eventually Open Dlgs
	
	if (pDoc->m_pOsdDlg)
	{
		// m_pOsdDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		pDoc->m_pOsdDlg->Close();
	}
	if (pDoc->m_pImageInfoDlg)
	{
		// m_pImageInfoDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		pDoc->m_pImageInfoDlg->Close();
	}
	if (pDoc->m_pHLSDlg)
	{
		// m_pHLSDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		pDoc->m_pHLSDlg->Close();
	}
	if (pDoc->m_pRedEyeDlg)
	{
		// m_pRedEyeDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		pDoc->m_pRedEyeDlg->Close();
	}
	if (pDoc->m_pMonochromeConversionDlg)
	{
		// m_pMonochromeConversionDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		pDoc->m_pMonochromeConversionDlg->Close();
	}
	if (pDoc->m_pSharpenDlg)
	{
		// m_pSharpenDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		pDoc->m_pSharpenDlg->Close();
	}
	if (pDoc->m_pSoftenDlg)
	{
		// m_pSoftenDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		pDoc->m_pSoftenDlg->Close();
	}
	if (pDoc->m_pSoftBordersDlg)
	{
		// m_pSoftBordersDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		pDoc->m_pSoftBordersDlg->Close();
	}
	if (pDoc->m_pRotationFlippingDlg)
	{
		// m_pRotationFlippingDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		pDoc->m_pRotationFlippingDlg->Close();
	}
	if (pDoc->m_pWndPalette)
	{
		// m_pWndPalette is selfdeleted and the
		// pointer is set to NULL in OnColorPickerClosed()
		pDoc->m_pWndPalette->Close();
	}
}

BOOL CPictureChildFrame::IsShutdownDone()
{
	CPictureDoc* pDoc = (CPictureDoc*)GetActiveDocument();
	ASSERT_VALID(pDoc);
	CPictureView* pView = pDoc->GetView();	// Get CPictureView this way, do not use GetActiveView()
	ASSERT_VALID(pView);					// because it can return CPicturePrintPreviewView!

	// Check whether we exited full-screen and
	// all Threads are Dead
	if (!pView->m_bFullScreenMode				&&
		!pDoc->m_SlideShowThread.IsAlive()		&&
		!pDoc->m_JpegThread.IsAlive()			&&
		!pDoc->m_LoadPicturesThread.IsAlive()	&&
		!pDoc->m_LayeredDlgThread.IsAlive()		&&
		!pDoc->m_GifAnimationThread.IsAlive()	&&
		!pDoc->m_TransitionThread.IsAlive())
		return TRUE;
	else
		return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CVideoDeviceChildFrame

#ifdef VIDEODEVICEDOC

IMPLEMENT_DYNCREATE(CVideoDeviceChildFrame, CToolBarChildFrame)

CVideoDeviceChildFrame::CVideoDeviceChildFrame()
{
	// Init vars
	m_bShutdown2Started = FALSE;
	m_bShutdown3Started = FALSE;
}

BEGIN_MESSAGE_MAP(CVideoDeviceChildFrame, CToolBarChildFrame)
	//{{AFX_MSG_MAP(CVideoDeviceChildFrame)
	ON_WM_SIZING()
	ON_NOTIFY(TBN_DROPDOWN, AFX_IDW_TOOLBAR, OnToolbarDropDown)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CVideoDeviceChildFrame::OnSizing(UINT fwSide, LPRECT pRect)
{
	CToolBarChildFrame::OnSizing(fwSide, pRect);

	if (::GetKeyState(VK_CONTROL) < 0)
	{
		CVideoDeviceView* pView = (CVideoDeviceView*)GetActiveView();
		ASSERT_VALID(pView);
		CVideoDeviceDoc* pDoc = pView->GetDocument();
		ASSERT_VALID(pDoc);

		// Get View Borders
		CRect rcw;
		pView->GetWindowRect(&rcw);
		CRect rcc;
		pView->GetClientRect(&rcc);
		CSize szViewBorder;
		szViewBorder.cx = rcw.Width() - rcc.Width();
		szViewBorder.cy = rcw.Height() - rcc.Height();

		// Get Frame Borders
		CRect rcw_frame;
		GetWindowRect(&rcw_frame);
		CRect rcc_frame;
		GetClientRect(&rcc_frame);
		CSize szFrameBorder;
		szFrameBorder.cx = rcw_frame.Width() - rcc_frame.Width();
		szFrameBorder.cy = rcw_frame.Height() - rcc_frame.Height();

		// Toolbar Rectangle
		CRect rcw_toolbar(0, 0, 0, 0);
		if (GetToolBar()->IsVisible())
			GetToolBar()->GetWindowRect(&rcw_toolbar);

		// Ratio
		double dRatio = 0.75;
		if (pDoc->m_DocRect.Height() > 0 && pDoc->m_DocRect.Width() > 0)
			dRatio = (double)pDoc->m_DocRect.Height() / (double)pDoc->m_DocRect.Width();

		/*
		Equations for the size calculation:

		pRect->right = pRect->left + W + szViewBorder.cx + szFrameBorder.cx;
		pRect->bottom = pRect->top + H + szViewBorder.cy + szFrameBorder.cy + rcw_toolbar.Height();

		W = pRect->right - pRect->left - szViewBorder.cx - szFrameBorder.cx;
		H = W * dRatio = pRect->bottom - pRect->top - szViewBorder.cy - szFrameBorder.cy - rcw_toolbar.Height();

		pRect->bottom - pRect->top - szViewBorder.cy - szFrameBorder.cy - rcw_toolbar.Height() = (pRect->right - pRect->left - szViewBorder.cx - szFrameBorder.cx) * dRatio
		pRect->right - pRect->left - szViewBorder.cx - szFrameBorder.cx = (pRect->bottom - pRect->top - szViewBorder.cy - szFrameBorder.cy - rcw_toolbar.Height()) / dRatio
		*/
		switch (fwSide)
		{
			case WMSZ_TOP:
			case WMSZ_BOTTOM:
				pRect->right = ::Round(pRect->left + szViewBorder.cx + szFrameBorder.cx + (pRect->bottom - pRect->top - szViewBorder.cy - szFrameBorder.cy - rcw_toolbar.Height()) / dRatio);
				break;
			case WMSZ_LEFT:
			case WMSZ_RIGHT:
			case WMSZ_BOTTOMLEFT:
			case WMSZ_BOTTOMRIGHT:
				pRect->bottom = ::Round(pRect->top + szViewBorder.cy + szFrameBorder.cy + rcw_toolbar.Height() + (pRect->right - pRect->left - szViewBorder.cx - szFrameBorder.cx) * dRatio);
				break;
			case WMSZ_TOPLEFT:
			case WMSZ_TOPRIGHT:
				pRect->top = ::Round(pRect->bottom - szViewBorder.cy - szFrameBorder.cy - rcw_toolbar.Height() - (pRect->right - pRect->left - szViewBorder.cx - szFrameBorder.cx) * dRatio);
				break;
			default:
				break;
		}
	}
}

void CVideoDeviceChildFrame::OnToolbarDropDown(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTOOLBAR pNMToolBar = reinterpret_cast<LPNMTOOLBAR>(pNMHDR);
	switch (pNMToolBar->iItem)
	{
		case ID_VIEW_FRAMETIME:
		{
			CMenu menu;
			VERIFY(menu.LoadMenu(IDR_CONTEXT_FONTSIZE));
			CMenu* pPopup = menu.GetSubMenu(0);
			ASSERT(pPopup != NULL);
			GetToolBar()->ClientToScreen(&(pNMToolBar->rcButton));
			pPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pNMToolBar->rcButton.left, pNMToolBar->rcButton.bottom, this);
			break;
		}
		case ID_CAPTURE_MOVDET:
		{
			CMenu menu;
			VERIFY(menu.LoadMenu(IDR_CONTEXT_SENSITIVITY));
			CMenu* pPopup = menu.GetSubMenu(0);
			ASSERT(pPopup != NULL);
			GetToolBar()->ClientToScreen(&(pNMToolBar->rcButton));
			pPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pNMToolBar->rcButton.left, pNMToolBar->rcButton.bottom, this);
			break;
		}
		default:
			return;
	}
}

void CVideoDeviceChildFrame::OnTimer(UINT nIDEvent) 
{
	CToolBarChildFrame::OnTimer(nIDEvent);
	OnClose();
}

void CVideoDeviceChildFrame::OnClose() 
{
	CVideoDeviceView* pView = (CVideoDeviceView*)GetActiveView();
	ASSERT_VALID(pView);
	CVideoDeviceDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Exit Full-Screen
	if (pView->m_bFullScreenMode)
		::AfxGetMainFrame()->EnterExitFullscreen();
	
	// Show closing progress in title bar
	pDoc->SetDocumentTitle();

	// If First Close Attempt
	if (m_bFirstCloseAttempt)
	{
		// Start closing
		if (pDoc->CanCloseFrame(this))
		{
			// Kill Timer
			pView->KillTimer(ID_TIMER_RELOAD_SETTINGS);

			// Hide detection zones
			if (pDoc->m_nShowEditDetectionZones)
				pDoc->HideDetectionZones();
	
			// Save Settings
			pDoc->SaveSettings();

			// Log the stopping
			if (pDoc->m_bCaptureStarted)
			{
				CTimeSpan TimeSpan = CTime::GetCurrentTime() - pDoc->m_CaptureStartTime;
				CString sMsg;
				if (TimeSpan.GetDays() > 0)
				{
					sMsg.Format(_T("%s stopping, running for %I64dday%s, %dhour%s, %dmin and %dsec"),
						pDoc->GetAssignedDeviceName(),
						TimeSpan.GetDays(),
						(TimeSpan.GetDays() == 1) ? _T("") : _T("s"),
						TimeSpan.GetHours(),
						(TimeSpan.GetHours() == 1) ? _T("") : _T("s"),
						TimeSpan.GetMinutes(),
						TimeSpan.GetSeconds());
				}
				else if (TimeSpan.GetTotalHours() > 0)
				{
					sMsg.Format(_T("%s stopping, running for %I64dhour%s, %dmin and %dsec"),
						pDoc->GetAssignedDeviceName(),
						(LONGLONG)TimeSpan.GetTotalHours(),
						(TimeSpan.GetTotalHours() == 1) ? _T("") : _T("s"),
						TimeSpan.GetMinutes(),
						TimeSpan.GetSeconds());
				}
				else if (TimeSpan.GetTotalMinutes() > 0)
				{
					sMsg.Format(_T("%s stopping, running for %I64dmin and %dsec"),
						pDoc->GetAssignedDeviceName(),
						(LONGLONG)TimeSpan.GetTotalMinutes(),
						TimeSpan.GetSeconds());
				}
				else if (TimeSpan.GetTotalSeconds() > 0)
				{
					sMsg.Format(_T("%s stopping, running for %I64dsec"),
						pDoc->GetAssignedDeviceName(),
						(LONGLONG)TimeSpan.GetTotalSeconds());
				}
				else
					sMsg.Format(_T("%s stopping"), pDoc->GetAssignedDeviceName());
				::LogLine(_T("%s"), sMsg);
			}

			// Set Closing Flag
			::InterlockedExchange(&pDoc->m_bClosing, 1);
			pDoc->SetDocumentTitle();

			// Clear Modified Flag
			pDoc->SetModifiedFlag(FALSE);

			// Setup Timer?
			if (!((CUImagerApp*)::AfxGetApp())->m_bClosingAll)
				SetTimer(ID_TIMER_CLOSING_VIDEODEVICEDOC, CLOSING_CHECK_INTERVAL_TIMER_MS, NULL);

			// Reset Flag and set shutdown start uptime
			m_bFirstCloseAttempt = FALSE;

			// Start Shutdown Process
			StartShutdown1();
		}
	}
	// StartShutdown2()?
	else if (!m_bShutdown2Started)
	{
		// The given wait time may be exceeded if a small framerate is set.
		// That's not a problem given that after PROCESSFRAME_MAX_RETRY_TIME milliseconds
		// that we called StopProcessFrame() we are not anymore inside ProcessI420Frame() and
		// because of the StopProcessFrame() call we cannot enter ProcessI420Frame() again!
		if (IsShutdown1Done() ||
			(::timeGetTime() - m_dwShutdownStartUpTime) >= PROCESSFRAME_MAX_RETRY_TIME)
			StartShutdown2();
	}
	// StartShutdown3()?
	else if (!m_bShutdown3Started)
	{
		if (IsShutdown2Done() ||
			(::timeGetTime() - m_dwShutdownStartUpTime) >= NETCOM_BLOCKING_TIMEOUT)
			StartShutdown3();
	}
	// Done?
	else if (IsShutdown3Done() ||
			(::timeGetTime() - m_dwShutdownStartUpTime) >= NETCOM_BLOCKING_TIMEOUT)
	{
		// Log the failure to close
		if (!IsShutdown2Done() || !IsShutdown3Done())
		{
			CString sMsg, t;
			sMsg.Format(_T("%s forced stopping"), pDoc->GetAssignedDeviceName());
			if (pDoc->m_HttpThread.IsAlive())
				t += _T(", http thread still alive");
			if (pDoc->m_RtspThread.IsAlive())
				t += _T(", rtsp thread still alive");
			if (pDoc->m_pVideoNetCom && !pDoc->m_pVideoNetCom->IsShutdown())
				t += _T(", netcom video threads still alive");
			if (pDoc->m_DeleteThread.IsAlive())
				t += _T(", delete thread still alive");
			if (pDoc->m_CaptureAudioThread.IsAlive())
				t += _T(", capture audio thread still alive");
			if (pDoc->m_SaveFrameListThread.IsAlive())
				t += _T(", save frame list thread still alive");
			if (pDoc->m_SaveSnapshotVideoThread.IsAlive())
				t += _T(", save snapshot video thread still alive");
			if (pDoc->m_SaveSnapshotThread.IsAlive())
				t += _T(", save snapshot thread still alive");
			::LogLine(_T("%s"), sMsg + t);
		}

		// Kill Timer?
		if (!((CUImagerApp*)::AfxGetApp())->m_bClosingAll)
			KillTimer(ID_TIMER_CLOSING_VIDEODEVICEDOC);

		// End Shutdown
		EndShutdown();

		// Destroy Window
		CToolBarChildFrame::OnClose();
	}
}

void CVideoDeviceChildFrame::StartShutdown1()
{
	CVideoDeviceView* pView = (CVideoDeviceView*)GetActiveView();
	ASSERT_VALID(pView);
	CVideoDeviceDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Init timeout
	m_dwShutdownStartUpTime = ::timeGetTime();

	// Kill it right now because of Save Frame List and HTTP Reconnect
	pDoc->m_WatchdogThread.Kill_NoBlocking();

	// Stop Processing Frames
	pDoc->StopProcessFrame(PROCESSFRAME_CLOSE);

	// Hide Window Property Sheet without saving
	// because already done in OnClose()
	if (pDoc->m_pCameraAdvancedSettingsPropertySheet &&
		pDoc->m_pCameraAdvancedSettingsPropertySheet->IsWindowVisible())
		pDoc->m_pCameraAdvancedSettingsPropertySheet->Hide(FALSE);
}

void CVideoDeviceChildFrame::StartShutdown2()
{
	CVideoDeviceView* pView = (CVideoDeviceView*)GetActiveView();
	ASSERT_VALID(pView);
	CVideoDeviceDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Init timeout
	m_dwShutdownStartUpTime = ::timeGetTime();

	// Set flag
	m_bShutdown2Started = TRUE;

	// Start killing threads
	pDoc->m_HttpThread.Kill_NoBlocking();
	pDoc->m_DeleteThread.Kill_NoBlocking();
	pDoc->m_CaptureAudioThread.Kill_NoBlocking();
	pDoc->m_SaveFrameListThread.Kill_NoBlocking();
	pDoc->m_SaveSnapshotVideoThread.Kill_NoBlocking();
	pDoc->m_SaveSnapshotThread.Kill_NoBlocking();
}

void CVideoDeviceChildFrame::StartShutdown3()
{
	CVideoDeviceView* pView = (CVideoDeviceView*)GetActiveView();
	ASSERT_VALID(pView);
	CVideoDeviceDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Init timeout
	m_dwShutdownStartUpTime = ::timeGetTime();

	// Set flag
	m_bShutdown3Started = TRUE;

	// Start killing rtsp thread
	pDoc->m_RtspThread.Kill_NoBlocking();

	// Start connection shutdown
	// (this must happen when m_HttpThread is not running
	// anymore, because inside this thread http connections
	// can be established)
	if (pDoc->m_pVideoNetCom)
		pDoc->m_pVideoNetCom->ShutdownConnection_NoBlocking();
}

void CVideoDeviceChildFrame::EndShutdown()
{
	CVideoDeviceView* pView = (CVideoDeviceView*)GetActiveView();
	ASSERT_VALID(pView);
	CVideoDeviceDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Network Clients Clean-Up
	if (pDoc->m_pVideoNetCom)
	{
		// This calls Close() which is not locking indefinitely,
		// but attention the destructors of the message threads
		// (both base class and derived class destructors) call
		// Kill() which locks indefinitely freezing the interface
		delete pDoc->m_pVideoNetCom;
		pDoc->m_pVideoNetCom = NULL;
	}

	// Delete DirectShow Capture Object
	if (pDoc->m_pDxCapture)
	{
		delete pDoc->m_pDxCapture;
		pDoc->m_pDxCapture = NULL;
	}

	// The next step must happen last, because it sets to NULL
	// the following pointers (used inside ProcessI420Frame()):
	//
	// m_pCameraAdvancedSettingsPropertySheet
	// m_pGeneralPage
	// m_pSnapshotPage
	// m_pMovementDetectionPage
	//
	if (pDoc->m_pCameraAdvancedSettingsPropertySheet)
	{
		// m_pCameraAdvancedSettingsPropertySheet pointer is set to NULL
		// from the sheet class (selfdeletion)
		// The other pointers are set to NULL
		// by the respective OnDestroy() functions
		pDoc->m_pCameraAdvancedSettingsPropertySheet->Close();
	}
}

BOOL CVideoDeviceChildFrame::IsShutdown1Done()
{
	CVideoDeviceView* pView = (CVideoDeviceView*)GetActiveView();
	ASSERT_VALID(pView);
	CVideoDeviceDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Check whether we exited full-screen, watchdog stopped
	// and we are not inside the processing function
	if (!pView->m_bFullScreenMode			&&
		!pDoc->m_WatchdogThread.IsAlive()	&&
		pDoc->IsProcessFrameStopped(PROCESSFRAME_CLOSE))
		return TRUE;
	else
		return FALSE;
}

BOOL CVideoDeviceChildFrame::IsShutdown2Done()
{
	CVideoDeviceView* pView = (CVideoDeviceView*)GetActiveView();
	ASSERT_VALID(pView);
	CVideoDeviceDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Check whether all Threads are dead
	if (!pDoc->m_HttpThread.IsAlive()				&&
		!pDoc->m_DeleteThread.IsAlive()				&&
		!pDoc->m_CaptureAudioThread.IsAlive()		&&
		!pDoc->m_SaveFrameListThread.IsAlive()		&&
		!pDoc->m_SaveSnapshotVideoThread.IsAlive()	&&
		!pDoc->m_SaveSnapshotThread.IsAlive())
		return TRUE;
	else
		return FALSE;
}

BOOL CVideoDeviceChildFrame::IsShutdown3Done()
{
	CVideoDeviceView* pView = (CVideoDeviceView*)GetActiveView();
	ASSERT_VALID(pView);
	CVideoDeviceDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Check whether the connections have been shutdown
	BOOL bVideoShutdown = pDoc->m_pVideoNetCom ? pDoc->m_pVideoNetCom->IsShutdown() : TRUE;
	if (!pDoc->m_RtspThread.IsAlive() && bVideoShutdown)
		return TRUE;
	else
		return FALSE;
}

#endif
