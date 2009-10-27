#include "stdafx.h"
#include "MainFrm.h"
#include "uImager.h"
#include "ToolBarChildFrm.h"
#include "resource.h"
#include "uImagerDoc.h"
#include "VideoAviView.h"
#include "VideoDeviceView.h"
#include "PictureView.h"
#include "AudioMCIView.h"
#include "PlayerToolBarDlg.h"
#include "mmsystem.h"
#include "XThemeHelper.h"
#include "AviInfoDlg.h"
#include "OutVolDlg.h"
#include "AudioVideoShiftDlg.h"
#include "VideoDevicePropertySheet.h"
#include "DxCapture.h"
#include "DxCaptureVMR9.h"
#include "ProgressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED     0x031A
#endif

static const CRect	TOOLBAR_BORDERRECT		= CRect(TOOLBAR_BORDER_LEFT,
													TOOLBAR_BORDER_TOP,
													TOOLBAR_BORDER_RIGHT,
													TOOLBAR_BORDER_BOTTOM);

/////////////////////////////////////////////////////////////////////////////
// CChildToolBar

IMPLEMENT_DYNAMIC(CChildToolBar, CToolBar)

CChildToolBar::CChildToolBar()
{
	m_nMinToolbarWidth = 0;
	m_nMaxToolbarWidth = 0;
}

BOOL CChildToolBar::Create(CWnd* pParentWnd)
{
	return CToolBar::CreateEx(pParentWnd,	TBSTYLE_FLAT, 
											WS_VISIBLE | WS_CHILD | CBRS_ALIGN_BOTTOM |
											CBRS_TOOLTIPS | CBRS_FLYBY |
											CBRS_SIZE_FIXED, TOOLBAR_BORDERRECT);
}

BEGIN_MESSAGE_MAP(CChildToolBar, CToolBar)
	//{{AFX_MSG_MAP(CChildToolBar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVideoAviToolBar

IMPLEMENT_DYNAMIC(CVideoAviToolBar, CChildToolBar)

CVideoAviToolBar::CVideoAviToolBar()
{
	m_PlayerSliderIndex = -1;
}

CVideoAviToolBar::~CVideoAviToolBar()
{

}

BOOL CVideoAviToolBar::Create(CWnd* pParentWnd, BOOL bFullScreen, int nMaxWidth)
{
	if (!CChildToolBar::Create(pParentWnd))
		return FALSE;
	
	if (!LoadToolBar(bFullScreen ? IDR_VIDEO_AVI_TOOLBAR_FULLSCREEN : IDR_VIDEO_AVI_TOOLBAR))
		return FALSE;

	// Set Var
	m_nMaxToolbarWidth = nMaxWidth;

	// Player Slider
	m_PlayerSliderIndex = CommandToIndex(ID_PLAY_SLIDER);
	if (!m_PlayerSlider.Create(WS_VISIBLE | WS_CHILD | TBS_HORZ | TBS_TOP | TBS_NOTICKS, CRect(0,0,0,0), this, ID_PLAY_SLIDER))
		return FALSE;
	m_PlayerSlider.ModifyStyleEx(0, WS_EX_NOPARENTNOTIFY);
	m_PlayerSlider.SetRange(0, 100, TRUE);
	m_PlayerSlider.SetPageSize(1);
	m_PlayerSlider.SetLineSize(1);

	ShowWindow(SW_SHOW);
	UpdateWindow();
	return TRUE;
}

BOOL CVideoAviToolBar::Create(CWnd* pParentWnd)
{
	if (!CChildToolBar::Create(pParentWnd))
		return FALSE;
	
	if (!LoadToolBar(IDR_VIDEO_AVI_TOOLBAR))
		return FALSE;

	// Player Slider
	m_PlayerSliderIndex = CommandToIndex(ID_PLAY_SLIDER);
	if (!m_PlayerSlider.Create(WS_VISIBLE | WS_CHILD | TBS_HORZ | TBS_TOP | TBS_NOTICKS, CRect(0,0,0,0), this, ID_PLAY_SLIDER))
		return FALSE;
	m_PlayerSlider.ModifyStyleEx(0, WS_EX_NOPARENTNOTIFY);
	m_PlayerSlider.SetRange(0, 100, TRUE);
	m_PlayerSlider.SetPageSize(0);
	m_PlayerSlider.SetLineSize(0);

	ShowWindow(SW_SHOW);
	UpdateWindow();
	return TRUE;
}

LRESULT CVideoAviToolBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	if (IsWindowVisible())
	{
		CFrameWnd* pParent = (CFrameWnd*)GetParent();
		if (pParent->IsKindOf(RUNTIME_CLASS(CFrameWnd)))
			return CToolBar::OnIdleUpdateCmdUI(wParam, lParam);
		else
		{
			if (pParent)
				OnUpdateCmdUI(pParent, (BOOL)wParam);
		}
	}

	return 0;
}

BEGIN_MESSAGE_MAP(CVideoAviToolBar, CChildToolBar)
	//{{AFX_MSG_MAP(CVideoAviToolBar)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CVideoAviToolBar::UpdateControls(void)
{
	// Player Slider
	if (IsWindow(m_PlayerSlider))
	{
		CRect rcSlider;
		GetItemRect(m_PlayerSliderIndex, rcSlider);
		int nCount = GetCount() - 1; // Slider Not Counted!
		CRect rcItem;
		GetItemRect(0, &rcItem);

		// Set Min Toolbar Width
		m_nMinToolbarWidth = PLAY_SLIDER_WIDTH_MIN + rcItem.Width() * nCount + 4;

		// Set Slider Width
		if (m_nMaxToolbarWidth > 0)
		{
			int MaxSize = m_nMaxToolbarWidth - (rcItem.Width() * nCount) - 4;
			if (MaxSize < PLAY_SLIDER_WIDTH_MIN)
				rcSlider.right = rcSlider.left + PLAY_SLIDER_WIDTH_MIN;
			else
				rcSlider.right = rcSlider.left + MaxSize;
		}
		else
			rcSlider.right = rcSlider.left + PLAY_SLIDER_WIDTH_MIN;
		rcSlider.top -= 2;
		SetButtonInfo(m_PlayerSliderIndex, ID_PLAY_SLIDER, TBBS_SEPARATOR, rcSlider.right - rcSlider.left);
		m_PlayerSlider.MoveWindow(&rcSlider);
	}
}

void CVideoAviToolBar::OnSize(UINT nType, int cx, int cy) 
{
	CToolBar::OnSize(nType, cx, cy);
	UpdateControls();
}

void CVideoAviToolBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	nPos;
	pScrollBar;
	nSBCode;

	// Available only if there is an active doc
	CMDIChildWnd* pChild = ::AfxGetMainFrame()->MDIGetActive();
	if (!pChild)
		return;

	// Send the WM_HSCROLL Message to the View
	CUImagerView* pView = (CUImagerView*)(pChild->GetActiveView());
	ASSERT_VALID(pView);
	pView->SetFocus();
	pView->SendMessage(WM_HSCROLL, GetCurrentMessage()->wParam, GetCurrentMessage()->lParam);
}

/////////////////////////////////////////////////////////////////////////////
// CVideoDeviceToolBar

#ifdef VIDEODEVICEDOC

IMPLEMENT_DYNAMIC(CVideoDeviceToolBar, CChildToolBar)

CVideoDeviceToolBar::CVideoDeviceToolBar()
{
	
}

CVideoDeviceToolBar::~CVideoDeviceToolBar()
{

}

BOOL CVideoDeviceToolBar::Create(CWnd* pParentWnd)
{
	if (!CChildToolBar::Create(pParentWnd))
		return FALSE;
	
	if (!LoadToolBar(IDR_VIDEO_DEVICE_TOOLBAR))
		return FALSE;

	ShowWindow(SW_SHOW);
	UpdateWindow();
	return TRUE;
}

BEGIN_MESSAGE_MAP(CVideoDeviceToolBar, CChildToolBar)
	//{{AFX_MSG_MAP(CVideoDeviceToolBar)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CVideoDeviceToolBar::UpdateControls(void)
{
	// Set Min Toolbar Width
	int nCount = GetCount();
	if (nCount > 0)
	{
		CRect rcItem;
		GetItemRect(0, &rcItem);
		m_nMinToolbarWidth = rcItem.Width() * nCount + 4;
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

BOOL CPictureToolBar::Create(CWnd* pParentWnd)
{
	if (!CChildToolBar::Create(pParentWnd))
		return FALSE;
	
	BOOL bBigPicture;
	if (pParentWnd->IsKindOf(RUNTIME_CLASS(CBigPictureChildFrame)))
		bBigPicture = TRUE;
	else
		bBigPicture = FALSE;
	
	if (g_bNT)
	{
		if (bBigPicture)
		{
			if (!LoadToolBar(IDR_BIGPICTURE_TOOLBAR))
				return FALSE;
		}
		else
		{
			if (!LoadToolBar(IDR_PICTURE_TOOLBAR))
				return FALSE;
		}
	}
	else
	{
		if (bBigPicture)
		{
			if (!LoadToolBar(IDR_BIGPICTURE_TOOLBAR_NOHQ))
				return FALSE;
		}
		else
		{
			if (!LoadToolBar(IDR_PICTURE_TOOLBAR_NOHQ))
				return FALSE;
		}
	}

	CFont m_Font;
	if (!m_Font.CreateStockObject(DEFAULT_GUI_FONT)) // ANSI_FIXED_FONT, DEFAULT_GUI_FONT, OEM_FIXED_FONT
		return FALSE;								// ANSI_VAR_FONT, SYSTEM_FONT, SYSTEM_FIXED_FONT

	// Zoom Combo Box
	m_ZoomComboBoxIndex = CommandToIndex(ID_ZOOM_COMBOX);
	if (m_ZoomComboBoxIndex != -1)
		if (!m_ZoomComboBox.Create(CBS_DROPDOWNLIST | WS_VISIBLE | WS_CHILD, CRect(0,0,0,0), this, ID_ZOOM_COMBOX))
			return FALSE;
	m_ZoomComboBox.SetFont(&m_Font);
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
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		m_BkgColorButtonPicker.SetProfileSection(_T("GeneralApp"));
	m_BkgColorButtonPicker.EnableWindow(TRUE);

	ShowWindow(SW_SHOW);
	UpdateWindow();

	return TRUE;
}

BEGIN_MESSAGE_MAP(CPictureToolBar, CChildToolBar)
	//{{AFX_MSG_MAP(CPictureToolBar)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(CPN_SELENDOK, OnSelEndOK)
END_MESSAGE_MAP()

void CPictureToolBar::UpdateControls(void)
{
	CRect rect;

	CPictureView* pView = NULL;
	CPictureDoc* pDoc = NULL;
	int nHiddenCount = 0;
	if (GetParent())
	{
		pView = (CPictureView*)(((CPictureChildFrame*)GetParent())->GetActiveView());
		if (pView)
		{
			ASSERT_VALID(pView);
			pDoc = pView->GetDocument();
			ASSERT_VALID(pDoc);
			if (pDoc->IsMultiPageTIFF() || pDoc->m_GifAnimationThread.IsAlive())
			{
				GetToolBarCtrl().HideButton(ID_VIEW_NEXT_PAGE_FRAME, FALSE);
				GetToolBarCtrl().HideButton(ID_VIEW_PREVIOUS_PAGE_FRAME, FALSE);
			}
			else
			{
				nHiddenCount = 2;
				GetToolBarCtrl().HideButton(ID_VIEW_NEXT_PAGE_FRAME, TRUE);
				GetToolBarCtrl().HideButton(ID_VIEW_PREVIOUS_PAGE_FRAME, TRUE);
			}
		}
	}

	if (IsWindow(m_ZoomComboBox))
	{
		GetItemRect(m_ZoomComboBoxIndex, rect);
		rect.right = rect.left + 64;
		SetButtonInfo(	m_ZoomComboBoxIndex,
						ID_ZOOM_COMBOX,
						TBBS_SEPARATOR,
						rect.Width());
		rect.left += 2;
		(rect.right)--;
		rect.bottom += 300;

		// To Avoid Flickering Of The ComboBox!
		if (m_rcLastZoomComboBox != rect)
		{
			m_ZoomComboBox.MoveWindow(&rect);
			m_rcLastZoomComboBox = rect;
		}
	}
	if (IsWindow(m_BkgColorButtonPicker))
	{
		GetItemRect(m_BkgColorButtonPickerIndex, rect);
		rect.right = rect.left + 36;
		SetButtonInfo(	m_BkgColorButtonPickerIndex,
						ID_BACKGROUND_COLOR,
						TBBS_SEPARATOR,
						rect.Width());
		rect.left += 2;
		(rect.right)--;
		
		// To Avoid Flickering Of The Color Button Picker
		if (m_rcLastBkgColorButtonPicker != rect)
			m_BkgColorButtonPicker.MoveWindow(&rect);
		m_rcLastBkgColorButtonPicker = rect;
	}

	// Set Min Toolbar Width
	int nCount = GetCount() - 3 - nHiddenCount; // Zoom ComboBox, Bkg Color Button Picker and separator Not Counted!
	if (nCount >= 0)
	{
		GetItemRect(0, &rect);
		m_nMinToolbarWidth =	m_rcLastZoomComboBox.Width() +
								m_rcLastBkgColorButtonPicker.Width() +
								rect.Width() * nCount;
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
	m_hTheme = NULL;
}

CToolBarChildFrame::~CToolBarChildFrame()
{
	if (m_hTheme)
		ThemeHelper.CloseThemeData(m_hTheme);
	m_hTheme = NULL;
}

BEGIN_MESSAGE_MAP(CToolBarChildFrame, CChildFrame)
	//{{AFX_MSG_MAP(CToolBarChildFrame)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToolBarChildFrame message handlers
int CToolBarChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CChildFrame::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Open Thema Data
	if (ThemeHelper.IsThemeLibAvailable())
		m_hTheme = ThemeHelper.OpenThemeData(m_hWnd, _T("Toolbar"));

	// Create ToolBar
	if (m_pToolBar)
		if (!m_pToolBar->Create(this))
			return -1;

	return 0;
}

void CToolBarChildFrame::SetToolBar(CChildToolBar* pToolBar)
{
	m_pToolBar = pToolBar;
}

CChildToolBar* CToolBarChildFrame::GetToolBar() 
{ 
	return m_pToolBar; 
}
	
void CToolBarChildFrame::OnSize(UINT nType, int cx, int cy) 
{
	CChildFrame::OnSize(nType, cx, cy);

	CUImagerView* pView = (CUImagerView*)GetActiveView();
	if (!pView || !pView->IsKindOf(RUNTIME_CLASS(CUImagerView)))
		return;

	pView->UpdateWindowSizes(FALSE, FALSE, FALSE); // Update ToolBar
}

BOOL CToolBarChildFrame::IsThemed()
{
	return (m_hTheme &&
			ThemeHelper.IsAppThemed() &&
			ThemeHelper.IsThemeComCtl32());
}

void CToolBarChildFrame::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CUImagerView* pView = (CUImagerView*)GetActiveView();
	if (!pView || !pView->IsKindOf(RUNTIME_CLASS(CUImagerView)))
		return;

	CRect rcFrameClient;
	GetClientRect(rcFrameClient);

	CRect rcViewClient;
	pView->GetClientRect(rcViewClient);

	CToolBar* pToolBar = (CToolBar*)GetToolBar();
	if (!pToolBar)
		return;

	CRect rcToolBar;
	pToolBar->GetClientRect(&rcToolBar); 

	// With Themes (Turned On or Off) We Have to Erase the
	// Toolbar Background Border!
	if ((TOOLBAR_BORDERRECT.top > 0 || TOOLBAR_BORDERRECT.left > 0 ||
		TOOLBAR_BORDERRECT.bottom > 0 || TOOLBAR_BORDERRECT.right > 0) &&
		(ThemeHelper.GetComCtl32Version() >= 6))
	{
		CRect rcToolBarBkg;
		rcToolBarBkg.top = rcFrameClient.bottom - rcToolBar.Height() -
						(TOOLBAR_BORDERRECT.top + TOOLBAR_BORDERRECT.bottom);
		rcToolBarBkg.left = rcFrameClient.left;
		rcToolBarBkg.right = rcFrameClient.right;
		rcToolBarBkg.bottom = rcFrameClient.bottom;
		
		COLORREF col = ::GetSysColor(COLOR_BTNFACE);
		if (IsThemed())
		{
			ThemeHelper.GetThemeColor(	m_hTheme,
										TP_BUTTON,
										TS_NORMAL,
										TMT_FILLCOLOR,
										&col);
		}
	
		CPen Pen;
		Pen.CreatePen(PS_SOLID, 1, col);
		CPen* pOldPen = dc.SelectObject(&Pen);
		int i;

		// Top Border
		for (i = 0 ; i < TOOLBAR_BORDERRECT.top ; i++)
		{
			dc.MoveTo(rcToolBarBkg.left, rcToolBarBkg.top+i);
			dc.LineTo(rcToolBarBkg.right, rcToolBarBkg.top+i);
		}
		
		// Left Border
		for (i = 0 ; i < TOOLBAR_BORDERRECT.left ; i++)
		{
			dc.MoveTo(rcToolBarBkg.left+i, rcToolBarBkg.top);
			dc.LineTo(rcToolBarBkg.left+i, rcToolBarBkg.bottom);
		}

		// Bottom Border
		for (i = 0 ; i < TOOLBAR_BORDERRECT.bottom ; i++)
		{
			dc.MoveTo(rcToolBarBkg.left, rcToolBarBkg.bottom-i-1);
			dc.LineTo(rcToolBarBkg.right, rcToolBarBkg.bottom-i-1);
		}

		// Right Border
		for (i = 0 ; i < TOOLBAR_BORDERRECT.right ; i++)
		{
			dc.MoveTo(rcToolBarBkg.right-i-1, rcToolBarBkg.top);
			dc.LineTo(rcToolBarBkg.right-i-1, rcToolBarBkg.bottom);
		}

		dc.SelectObject(pOldPen);
		Pen.DeleteObject();
	}

	// To erase the Background at the
	// bottom-right corner between the Scroll Bars!
	if (pView->IsXAndYScroll()) // Are both ScrollBars Visible?
	{
		CRect rcBottomRight = CRect(rcFrameClient.right - (rcFrameClient.Width() - rcViewClient.Width()) + 2,
								rcFrameClient.bottom - (rcFrameClient.Height() - rcViewClient.Height()) + 2,
								rcFrameClient.right - 2,
								rcFrameClient.bottom - rcToolBar.Height() -
								(TOOLBAR_BORDERRECT.top + TOOLBAR_BORDERRECT.bottom));

		CBrush br(::GetSysColor(COLOR_BTNFACE));
		dc.FillRect(rcBottomRight, &br);
	}

	// Do not call CChildFrame::OnPaint() for painting messages
}

/////////////////////////////////////////////////////////////////////////////
// CAudioMCIChildFrame

IMPLEMENT_DYNCREATE(CAudioMCIChildFrame, CChildFrame)

CAudioMCIChildFrame::CAudioMCIChildFrame()
{

}

BEGIN_MESSAGE_MAP(CAudioMCIChildFrame, CChildFrame)
	//{{AFX_MSG_MAP(CAudioMCIChildFrame)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CAudioMCIChildFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style &= ~WS_MAXIMIZEBOX;
	cs.style &= ~WS_SIZEBOX;
	return CChildFrame::PreCreateWindow(cs);
}

void CAudioMCIChildFrame::ActivateFrame(int nCmdShow) 
{
	nCmdShow = SW_NORMAL;	// Otherwise if current shown document is maximized
							// also this one is miximized, we do not want that!
	CChildFrame::ActivateFrame(nCmdShow);
}

void CAudioMCIChildFrame::OnClose() 
{
	CAudioMCIView* pView = (CAudioMCIView*)GetActiveView();
	ASSERT_VALID(pView);
	CAudioMCIDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Kill Timer
	if (pDoc->m_uiTimerId)
		pView->KillTimer(pDoc->m_uiTimerId);

	// Destroy Window
	CChildFrame::OnClose();
}

/////////////////////////////////////////////////////////////////////////////
// CCDAudioChildFrame

IMPLEMENT_DYNCREATE(CCDAudioChildFrame, CChildFrame)

CCDAudioChildFrame::CCDAudioChildFrame()
{

}

BEGIN_MESSAGE_MAP(CCDAudioChildFrame, CChildFrame)
	//{{AFX_MSG_MAP(CCDAudioChildFrame)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CCDAudioChildFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style &= ~WS_MAXIMIZEBOX;
	cs.style &= ~WS_SIZEBOX;
	return CChildFrame::PreCreateWindow(cs);
}

void CCDAudioChildFrame::ActivateFrame(int nCmdShow) 
{
	nCmdShow = SW_NORMAL;	// Otherwise if current shown document is maximized
							// also this one is miximized, we do not want that!
	CChildFrame::ActivateFrame(nCmdShow);
}

void CCDAudioChildFrame::OnClose() 
{
	CVideoAviView* pView = (CVideoAviView*)GetActiveView();
	ASSERT_VALID(pView);
	
	// Kill Timer
	pView->KillTimer(ID_TIMER_CDAUDIO);
	
	// Destroy Window
	CChildFrame::OnClose();
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

BEGIN_MESSAGE_MAP(CZoomComboBox, CComboBox)
	//{{AFX_MSG_MAP(CZoomComboBox)
	ON_CONTROL_REFLECT_EX(CBN_SELENDOK, OnSelEndOk)
	ON_CONTROL_REFLECT(CBN_SELENDCANCEL, OnSelendcancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CZoomComboBox::CZoomComboBox()
{
}

void CZoomComboBox::Init()
{
	SetItemDataPtr(AddString(_T("Fit")),	(void*)&dFit);
	SetItemDataPtr(AddString(_T("Fit Big")),(void*)&dFitBig);
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
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
	{
		::AfxGetApp()->WriteProfileInt(	_T("PictureDoc"),
										_T("ZoomComboBoxIndex"),
										pDoc->m_nZoomComboBoxIndex);
	}
}

BOOL CZoomComboBox::OnSelEndOk()
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
			pView->SetFocus();
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

	return TRUE;
}

void CZoomComboBox::OnSelendcancel() 
{
	CPictureView* pView = (CPictureView*)(((CControlBar*)GetParent())->GetDockingFrame()->GetActiveView());
	ASSERT_VALID(pView);
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
// CVideoAviChildFrame

IMPLEMENT_DYNCREATE(CVideoAviChildFrame, CToolBarChildFrame)

CVideoAviChildFrame::CVideoAviChildFrame()
{
	m_bShutdown2Started = FALSE;
}

BEGIN_MESSAGE_MAP(CVideoAviChildFrame, CToolBarChildFrame)
	//{{AFX_MSG_MAP(CVideoAviChildFrame)
	ON_WM_CLOSE()
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()

void CVideoAviChildFrame::OnTimer(UINT nIDEvent) 
{
	CToolBarChildFrame::OnTimer(nIDEvent);
	OnClose();
}

void CVideoAviChildFrame::OnClose() 
{
	CVideoAviView* pView = (CVideoAviView*)GetActiveView();
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	if (m_bFirstCloseAttempt)
	{
		// Start Closing only if Saved!
		if (pDoc->CanCloseFrame(this)) // This Calls CVideoAviDoc::SaveModified()
		{
			// Set Closing Flag
			::InterlockedExchange(&pDoc->m_bClosing, 1);

			// Clear Modified Flag
			pDoc->SetModifiedFlag(FALSE);

			// Setup Timer?
			if (!((CUImagerApp*)::AfxGetApp())->m_bClosingAll)
				SetTimer(ID_TIMER_CLOSING_VIDEOAVIDOC, CLOSING_CHECK_INTERVAL_TIMER_MS, NULL);

			// Start Shutdown
			m_bFirstCloseAttempt = FALSE;
			m_dwFirstCloseAttemptUpTime = ::timeGetTime();
			StartShutdown1();
		}
	}
	else if (IsShutdown1Done() && !m_bShutdown2Started)
		StartShutdown2();
	else if (IsShutdown2Done() ||
			(::timeGetTime() - m_dwFirstCloseAttemptUpTime) >= MAX_CLOSE_CHILDFRAME_WAITTIME)
	{
		// Kill Timer?
		if (!((CUImagerApp*)::AfxGetApp())->m_bClosingAll)
			KillTimer(ID_TIMER_CLOSING_VIDEOAVIDOC);

		// End Shutdown
		EndShutdown();

		// Destroy Window
		CToolBarChildFrame::OnClose();
	}
}

void CVideoAviChildFrame::StartShutdown1()
{
	// Exit Full-Screen
	if (::AfxGetMainFrame()->m_bFullScreenMode)
		::AfxGetMainFrame()->EnterExitFullscreen();
}

void CVideoAviChildFrame::StartShutdown2()
{
	CVideoAviView* pView = (CVideoAviView*)GetActiveView();
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);
	m_bShutdown2Started = TRUE;

	// Save Settings
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		pDoc->SaveSettings();

	// Do Not Draw Now!
	pDoc->m_bNoDrawing = TRUE;

	// Start Killing
	pDoc->m_PlayAudioFileThread.Kill_NoBlocking();
	pDoc->m_PlayVideoFileThread.Kill_NoBlocking();
	pDoc->m_ProcessingThread.Kill_NoBlocking();

	// Close Eventually Open Dlgs

	if (pDoc->m_pAviInfoDlg)
	{
		// m_pAviInfoDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		pDoc->m_pAviInfoDlg->Close();
	}
	if (pDoc->m_pOutVolDlg)
	{
		// m_pOutVolDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		pDoc->m_pOutVolDlg->Close();
	}
	if (pDoc->m_pAudioVideoShiftDlg)
	{
		// m_pAudioVideoShiftDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		pDoc->m_pAudioVideoShiftDlg->Close();
	}
	if (pDoc->m_pPlayerToolBarDlg)
	{
		// m_pPlayerToolBarDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		pDoc->m_pPlayerToolBarDlg->Close();
	}
}

void CVideoAviChildFrame::EndShutdown()
{
	CVideoAviView* pView = (CVideoAviView*)GetActiveView();
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

#ifdef VIDEODEVICEDOC
	// Close Avi File Capture Document
	if (((CUImagerApp*)::AfxGetApp())->IsDoc(pDoc->m_pVideoDeviceDoc))
	{
		// Close Video Device Doc
		pDoc->m_pVideoDeviceDoc->GetFrame()->PostMessage(WM_CLOSE,
														0, 0);
	}
#endif

	// Close DirectDraw
	pDoc->m_DxDraw.Free();
}

BOOL CVideoAviChildFrame::IsShutdown1Done()
{
	CVideoAviView* pView = (CVideoAviView*)GetActiveView();
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Check whether we exited full-screen
	if (!::AfxGetMainFrame()->m_bFullScreenMode)
		return TRUE;
	else
		return FALSE;
}

BOOL CVideoAviChildFrame::IsShutdown2Done()
{
	CVideoAviView* pView = (CVideoAviView*)GetActiveView();
	ASSERT_VALID(pView);
	CVideoAviDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Check whether all Threads are Dead
	if (!pDoc->m_PlayAudioFileThread.IsAlive()	&&
		!pDoc->m_PlayVideoFileThread.IsAlive()	&&
		!pDoc->m_ProcessingThread.IsAlive())
		return TRUE;
	else
		return FALSE;
}

void CVideoAviChildFrame::OnMove(int x, int y) 
{
	CToolBarChildFrame::OnMove(x, y);
	
	CVideoAviView* pView = (CVideoAviView*)GetActiveView();
	if (pView)
	{
		CVideoAviDoc* pDoc = pView->GetDocument();
		ASSERT_VALID(pDoc);
		pDoc->RestoreFrame();
	}
}

void CVideoAviChildFrame::OnSize(UINT nType, int cx, int cy) 
{
	CToolBarChildFrame::OnSize(nType, cx, cy);
	
	CVideoAviView* pView = (CVideoAviView*)GetActiveView();
	if (pView)
	{
		CVideoAviDoc* pDoc = pView->GetDocument();
		ASSERT_VALID(pDoc);
		pDoc->RestoreFrame();
	}
}

LRESULT CVideoAviChildFrame::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	CVideoAviView* pView = (CVideoAviView*)GetActiveView();
	if (pView)
	{
		CVideoAviDoc* pDoc = pView->GetDocument();
		if (pDoc && pDoc->m_pPlayerToolBarDlg)
			pDoc->m_pPlayerToolBarDlg->SendMessageToDescendants(WM_IDLEUPDATECMDUI,
																wParam,
																lParam);
	}
	CToolBarChildFrame::OnIdleUpdateCmdUI();
	return 0;
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
	CPictureView* pView = (CPictureView*)GetActiveView();
	ASSERT_VALID(pView);
	CPictureDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Exit Full-Screen
	if (::AfxGetMainFrame()->m_bFullScreenMode)
		::AfxGetMainFrame()->EnterExitFullscreen();

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
			m_dwFirstCloseAttemptUpTime = ::timeGetTime();
			StartShutdown();
		}
	}
	else
	{
		if (IsShutdownDone() ||
			(::timeGetTime() - m_dwFirstCloseAttemptUpTime) >= MAX_CLOSE_CHILDFRAME_WAITTIME)
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
	CPictureView* pView = (CPictureView*)GetActiveView();
	ASSERT_VALID(pView);
	CPictureDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Save The Settings
	if (!((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		pDoc->SaveSettingsXml();
	
	// Cancel Crop Tool
	pDoc->CancelCrop();

	// Do Not Draw Now!
	pDoc->m_bNoDrawing = TRUE;

	// Start Killing
	pDoc->m_ChangeNotificationThread.Kill_NoBlocking();
	pDoc->m_SlideShowThread.Kill_NoBlocking();
#ifdef SUPPORT_LIBJPEG
	pDoc->m_bLoadFullJpegTransitionUI = FALSE;
	pDoc->m_bCancelLoadFullJpegTransition = FALSE;
	pDoc->m_JpegThread.Kill_NoBlocking();
#endif
	pDoc->m_LoadPicturesThread.Kill_NoBlocking();
	pDoc->m_LayeredDlgThread.Kill_NoBlocking();
#ifdef SUPPORT_GIFLIB
	pDoc->m_GifAnimationThread.Kill_NoBlocking();
#endif
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
	CPictureView* pView = (CPictureView*)GetActiveView();
	ASSERT_VALID(pView);
	CPictureDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Check whether we exited full-screen and
	// all Threads are Dead
	if (!::AfxGetMainFrame()->m_bFullScreenMode		&&
		!pDoc->m_ChangeNotificationThread.IsAlive()	&&
		!pDoc->m_SlideShowThread.IsAlive()			&&
		!pDoc->m_JpegThread.IsAlive()				&&
		!pDoc->m_LoadPicturesThread.IsAlive()		&&
		!pDoc->m_LayeredDlgThread.IsAlive()			&&
#ifdef SUPPORT_GIFLIB
		!pDoc->m_GifAnimationThread.IsAlive()		&&
#endif
		!pDoc->m_TransitionThread.IsAlive())
		return TRUE;
	else
		return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CBigPictureChildFrame

IMPLEMENT_DYNCREATE(CBigPictureChildFrame, CPictureChildFrame)

CBigPictureChildFrame::CBigPictureChildFrame()
{

}

BEGIN_MESSAGE_MAP(CBigPictureChildFrame, CPictureChildFrame)
	//{{AFX_MSG_MAP(CBigPictureChildFrame)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CVideoDeviceChildFrame

#ifdef VIDEODEVICEDOC

IMPLEMENT_DYNCREATE(CVideoDeviceChildFrame, CToolBarChildFrame)

CVideoDeviceChildFrame::CVideoDeviceChildFrame()
{
	// Init var
	m_bShutdown2Started = FALSE;
	
	// Debugger MessageBox breakpoint?
#ifdef CRACKCHECK
	if (::IsBPXv1((void*)::MessageBox))
		::AfxGetMainFrame()->PostMessage(WM_CLOSE, 0, 0);
#endif
}

BEGIN_MESSAGE_MAP(CVideoDeviceChildFrame, CToolBarChildFrame)
	//{{AFX_MSG_MAP(CVideoDeviceChildFrame)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

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
	if (::AfxGetMainFrame()->m_bFullScreenMode)
		::AfxGetMainFrame()->EnterExitFullscreen();

	if (m_bFirstCloseAttempt)
	{
		// Start closing only if not displaying a VfW Dialog,
		// see SaveModified() called by CanCloseFrame()
		if (pDoc->CanCloseFrame(this))
		{
			// Show closing progress dialog
			if (!((CUImagerApp*)::AfxGetApp())->m_bTrayIcon ||
				!::AfxGetMainFrame()->m_TrayIcon.IsMinimizedToTray())
				new CProgressDlg(	this->GetSafeHwnd(),
									ML_STRING(1566, "Closing ") + pDoc->GetDeviceName() + _T("..."),
									0, MAX_CLOSE_CHILDFRAME_WAITTIME);

			// Remove wait cursor if trying to connect
			if (pDoc->m_pHttpGetFrameParseProcess &&
				pDoc->m_pHttpGetFrameParseProcess->m_bTryConnecting)
			{
				pDoc->m_pHttpGetFrameParseProcess->m_bTryConnecting = FALSE;
				pDoc->EndWaitCursor();
			}

			// Save Settings
			if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
				pDoc->SaveSettings();

			// Log the stopping
			// (no log for Video Avi mode)
			if (pDoc->m_bCaptureStarted && pDoc->m_pVideoAviDoc == NULL)
			{
				CTimeSpan TimeSpan = CTime::GetCurrentTime() - pDoc->m_CaptureStartTime;
				CString sMsg;
				if (TimeSpan.GetDays() > 0)
				{
					sMsg.Format(_T("%s stopping (running for %I64dday%s, %dhour%s, %dmin and %dsec)\n"),
						pDoc->GetDeviceName(),
						(LONGLONG)TimeSpan.GetDays(),
						(TimeSpan.GetDays() == 1) ? _T("") : _T("s"),
						TimeSpan.GetHours(),
						(TimeSpan.GetHours() == 1) ? _T("") : _T("s"),
						TimeSpan.GetMinutes(),
						TimeSpan.GetSeconds());
				}
				else if (TimeSpan.GetTotalHours() > 0)
				{
					sMsg.Format(_T("%s stopping (running for %I64dhour%s, %dmin and %dsec)\n"),
						pDoc->GetDeviceName(),
						(LONGLONG)TimeSpan.GetTotalHours(),
						(TimeSpan.GetTotalHours() == 1) ? _T("") : _T("s"),
						TimeSpan.GetMinutes(),
						TimeSpan.GetSeconds());
				}
				else if (TimeSpan.GetTotalMinutes() > 0)
				{
					sMsg.Format(_T("%s stopping (running for %I64dmin and %dsec)\n"),
						pDoc->GetDeviceName(),
						(LONGLONG)TimeSpan.GetTotalMinutes(),
						TimeSpan.GetSeconds());
				}
				else if (TimeSpan.GetTotalSeconds() > 0)
				{
					sMsg.Format(_T("%s stopping (running for %I64dsec)\n"),
						pDoc->GetDeviceName(),
						(LONGLONG)TimeSpan.GetTotalSeconds());
				}
				else
					sMsg.Format(_T("%s stopping\n"), pDoc->GetDeviceName());
				TRACE(sMsg);
				::LogLine(sMsg);
			}

			// Set Closing Flag
			::InterlockedExchange(&pDoc->m_bClosing, 1);

			// Clear Modified Flag
			pDoc->SetModifiedFlag(FALSE);

			// Setup Timer?
			if (!((CUImagerApp*)::AfxGetApp())->m_bClosingAll)
				SetTimer(ID_TIMER_CLOSING_VIDEODEVICEDOC, CLOSING_CHECK_INTERVAL_TIMER_MS, NULL);

			// Start Shutdown
			m_bFirstCloseAttempt = FALSE;
			m_dwFirstCloseAttemptUpTime = ::timeGetTime();

			// If we have a Video AVI as frame source
			// kill it so that frames stop to arrive!
			if (pDoc->m_pVideoAviDoc)
			{
				if (((CUImagerApp*)::AfxGetApp())->IsDoc(pDoc->m_pVideoAviDoc) &&
					pDoc->m_pVideoAviDoc->m_PlayVideoFileThread.IsAlive())
					pDoc->m_pVideoAviDoc->StopAVI();
			}
			else
				StartShutdown1();
		}
	}
	else if (pDoc->m_pVideoAviDoc)
	{
		if (!((CUImagerApp*)::AfxGetApp())->IsDoc(pDoc->m_pVideoAviDoc))
		{
			pDoc->m_pVideoAviDoc = NULL;
			pDoc->m_bCapture = FALSE;
			StartShutdown1();
		}
		else if (((CUImagerApp*)::AfxGetApp())->IsDoc(pDoc->m_pVideoAviDoc)	&&
				!pDoc->m_pVideoAviDoc->m_PlayVideoFileThread.IsAlive())
		{
			pDoc->m_pVideoAviDoc->m_pVideoDeviceDoc = NULL;
			pDoc->m_pVideoAviDoc = NULL;
			pDoc->m_bCapture = FALSE;
			StartShutdown1();
		}
	}
	else if (IsShutdown1Done() && !m_bShutdown2Started)
		StartShutdown2();
	else if (IsShutdown2Done() ||
			(::timeGetTime() - m_dwFirstCloseAttemptUpTime) >= MAX_CLOSE_CHILDFRAME_WAITTIME)
	{
		// Log the failure to close
		if (!IsShutdown2Done())
		{
			CString sMsg, t;
			sMsg.Format(_T("%s forced stopping"), pDoc->GetDeviceName());
			if (pDoc->m_HttpGetFrameThread.IsAlive())
				t += _T(", http get frame thread still alive");
			if (pDoc->m_pGetFrameNetCom && !pDoc->m_pGetFrameNetCom->IsShutdown())
				t += _T(", udp get frame threads still alive");
			if (pDoc->m_pSendFrameNetCom && !pDoc->m_pSendFrameNetCom->IsShutdown())
				t += _T(", udp send frame threads still alive");
			if (pDoc->m_DeleteThread.IsAlive())
				t += _T(", delete thread still alive");
			if (pDoc->m_CaptureAudioThread.IsAlive())
				t += _T(", capture audio thread still alive");
			if (pDoc->m_VfWCaptureVideoThread.IsAlive())
				t += _T(", vfw capture video thread still alive");
			if (pDoc->m_VMR9CaptureVideoThread.IsAlive())
				t += _T(", vmr9 capture video thread still alive");
			if (pDoc->m_SaveFrameListThread.IsAlive())
				t += _T(", save frame list thread still alive");
			if (pDoc->m_SaveSnapshotFTPThread.IsAlive())
				t += _T(", save snapshot ftp thread still alive");
			if (pDoc->m_SaveSnapshotThread.IsAlive())
				t += _T(", save snapshot thread still alive");
			sMsg = sMsg + t + _T("\n");
			TRACE(sMsg);
			::LogLine(sMsg);
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

	// Kill Watchdog (kill it right now because like the
	// Processing Frames function also the watchdog may be drawing)
	pDoc->m_WatchdogThread.Kill_NoBlocking();

	// Stop Processing Frames
	pDoc->StopProcessFrame();

	// Begin Wait Cursor
	BeginWaitCursor();

	// Hide Window Property Sheet
	if (pDoc->m_pVideoDevicePropertySheet &&
		pDoc->m_pVideoDevicePropertySheet->IsVisible())
		pDoc->m_pVideoDevicePropertySheet->Hide();
}

void CVideoDeviceChildFrame::StartShutdown2()
{
	CVideoDeviceView* pView = (CVideoDeviceView*)GetActiveView();
	ASSERT_VALID(pView);
	CVideoDeviceDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);
	m_bShutdown2Started = TRUE;

	// Start Killing
	pDoc->m_HttpGetFrameThread.Kill_NoBlocking();
	if (pDoc->m_pGetFrameNetCom)
		pDoc->m_pGetFrameNetCom->ShutdownConnection_NoBlocking();
	if (pDoc->m_pSendFrameNetCom)
		pDoc->m_pSendFrameNetCom->ShutdownConnection_NoBlocking();
	pDoc->m_DeleteThread.Kill_NoBlocking();
	pDoc->m_CaptureAudioThread.Kill_NoBlocking();
	pDoc->m_VfWCaptureVideoThread.Kill_NoBlocking();
	pDoc->m_VMR9CaptureVideoThread.Kill_NoBlocking();
	pDoc->m_SaveFrameListThread.Kill_NoBlocking();
	pDoc->m_SaveSnapshotFTPThread.Kill_NoBlocking();
	pDoc->m_SaveSnapshotThread.Kill_NoBlocking();
}

void CVideoDeviceChildFrame::EndShutdown()
{
	CVideoDeviceView* pView = (CVideoDeviceView*)GetActiveView();
	ASSERT_VALID(pView);
	CVideoDeviceDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	// If this is a Network Client Clean-Up
	// (Threads should already be stopped)
	if (pDoc->m_pGetFrameNetCom)
	{
		delete pDoc->m_pGetFrameNetCom;
		pDoc->m_pGetFrameNetCom = NULL;
	}

	// Clean-Up the Frame Stream-Server
	// (Threads already stopped)
	::EnterCriticalSection(&pDoc->m_csSendFrameNetCom);
	if (pDoc->m_pSendFrameNetCom)
	{
		delete pDoc->m_pSendFrameNetCom;
		pDoc->m_pSendFrameNetCom = NULL;
	}
	::LeaveCriticalSection(&pDoc->m_csSendFrameNetCom);

	// Close VfW Capture Thread
	pDoc->m_VfWCaptureVideoThread.Disconnect();
	pDoc->m_VfWCaptureVideoThread.DestroyCaptureWnd();

	// Delete DirectShow Capture Object
	if (pDoc->m_pDxCapture)
	{
		delete pDoc->m_pDxCapture;
		pDoc->m_pDxCapture = NULL;
		pDoc->m_bCapture = FALSE;
	}
	if (pDoc->m_pDxCaptureVMR9)
	{
		delete pDoc->m_pDxCaptureVMR9;
		pDoc->m_pDxCaptureVMR9 = NULL;
		pDoc->m_bCapture = FALSE;
	}

	// Close DirectDraw
	pDoc->m_DxDraw.Free();

	// The next step must happen last, because it sets to NULL
	// the following pointers (used inside ProcessFrame()
	// and the Audio Thread for the PeakMeter):
	//
	// m_pVideoDevicePropertySheet
	// m_pAssistantPage
	// m_pGeneralPage
	// m_pSnapshotPage
	// m_pColorDetectionPage
	// m_NetworkPage
	// m_MovementDetectionPage
	//
	if (pDoc->m_pVideoDevicePropertySheet)
	{
		// m_pVideoDevicePropertySheet pointer is set to NULL
		// from the sheet class (selfdeletion)
		// The other pointers are set to NULL
		// by the respective OnDestroy() functions
		pDoc->m_pVideoDevicePropertySheet->Close();
	}

	// End Wait Cursor
	EndWaitCursor();
}

BOOL CVideoDeviceChildFrame::IsShutdown1Done()
{
	CVideoDeviceView* pView = (CVideoDeviceView*)GetActiveView();
	ASSERT_VALID(pView);
	CVideoDeviceDoc* pDoc = pView->GetDocument();
	ASSERT_VALID(pDoc);

	// Check whether we exited full-screen, watchdog stopped
	// and we are not inside the processing function
	// (or frames where not arriving)
	if (!::AfxGetMainFrame()->m_bFullScreenMode		&&
		!pDoc->m_WatchdogThread.IsAlive()			&&
		(pDoc->IsProcessFrameStopped()				||
		!pDoc->m_bCapture							||
		pDoc->m_bWatchDogAlarm						||
		pDoc->m_bDxDeviceUnplugged))
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

	// Check whether all Threads are Dead
	if (!pDoc->m_HttpGetFrameThread.IsAlive()		&&
		(pDoc->m_pGetFrameNetCom ? pDoc->m_pGetFrameNetCom->IsShutdown() : TRUE)	&&
		(pDoc->m_pSendFrameNetCom ? pDoc->m_pSendFrameNetCom->IsShutdown() : TRUE)	&&
		!pDoc->m_DeleteThread.IsAlive()	&&
		!pDoc->m_CaptureAudioThread.IsAlive()		&&
		!pDoc->m_VfWCaptureVideoThread.IsAlive()	&&
		!pDoc->m_VMR9CaptureVideoThread.IsAlive()	&&
		!pDoc->m_SaveFrameListThread.IsAlive()		&&
		!pDoc->m_SaveSnapshotFTPThread.IsAlive()	&&
		!pDoc->m_SaveSnapshotThread.IsAlive())
		return TRUE;
	else
		return FALSE;
}

#endif

LRESULT CToolBarChildFrame::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
		case WM_THEMECHANGED:
		{
			if (IsThemed())
			{
				if (m_hTheme)
				{
					// when user changes themes, close current theme and re-open
					ThemeHelper.CloseThemeData(m_hTheme);
					m_hTheme = ThemeHelper.OpenThemeData(m_hWnd, _T("Toolbar"));
				}
			}
		}
		break;
	}

	return CChildFrame::DefWindowProc(message, wParam, lParam);
}
