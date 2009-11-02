// RemoteCamViewerView.cpp : implementation of the CRemoteCamViewerView class
//

#include "stdafx.h"
#include "RemoteCamViewer.h"
#include "RemoteCamViewerDoc.h"
#include "RemoteCamViewerView.h"
#include "SettingsDlg.h"
#include "..\MemDC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#  define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

#define POLL_TIMER_ID			1
#define POLL_TIMER_MS			500
#define REG_SECTION				_T("GeneralApp")
#define MIN_SIZE				24
#define TITLE_MARGIN_TOP		10
#define TITLE_MARGIN_BOTTOM		2
#define TITLE_FONT_SIZE			36
#define TITLE_COLOR				RGB(0x60, 0x60, 0x60)
#define TITLE_FONT_FACE			_T("Helvetica")
#define LABEL_MARGIN			3
#define LABEL_FONT_SIZE			12
#define LABEL_COLOR				RGB(0x30, 0x30, 0x30)
#define LABEL_FONT_FACE			_T("Helvetica")

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerView

IMPLEMENT_DYNCREATE(CRemoteCamViewerView, CFormView)

BEGIN_MESSAGE_MAP(CRemoteCamViewerView, CFormView)
	//{{AFX_MSG_MAP(CRemoteCamViewerView)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_HELP_ABOUTCTRL, OnHelpAboutctrl)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_COMMAND(ID_FILE_SHOWPROP0, OnFileShowprop0)
	ON_COMMAND(ID_FILE_SHOWPROP1, OnFileShowprop1)
	ON_COMMAND(ID_FILE_SHOWPROP2, OnFileShowprop2)
	ON_COMMAND(ID_FILE_SHOWPROP3, OnFileShowprop3)
	ON_COMMAND(ID_FILE_SETTINGS, OnFileSettings)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CFormView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerView construction/destruction

CRemoteCamViewerView::CRemoteCamViewerView()
	: CFormView(CRemoteCamViewerView::IDD)
{
	//{{AFX_DATA_INIT(CRemoteCamViewerView)
	//}}AFX_DATA_INIT
	m_szRemoteCamSize0 = CSize(0, 0);
	m_szRemoteCamSize1 = CSize(0, 0);
	m_szRemoteCamSize2 = CSize(0, 0);
	m_szRemoteCamSize3 = CSize(0, 0);
	m_rc0 = CRect(0, 0, 0, 0);
	m_rc1 = CRect(0, 0, 0, 0);
	m_rc2 = CRect(0, 0, 0, 0);
	m_rc3 = CRect(0, 0, 0, 0);
	m_bInit0 = FALSE;
	m_bInit1 = FALSE;
	m_bInit2 = FALSE;
	m_bInit3 = FALSE;
	m_nTitleHight = 0;
}	

CRemoteCamViewerView::~CRemoteCamViewerView()
{
}

void CRemoteCamViewerView::DoDataExchange(CDataExchange* pDX)
{
	try
	{
		CFormView::DoDataExchange(pDX);
		//{{AFX_DATA_MAP(CRemoteCamViewerView)
		DDX_Control(pDX, IDC_REMOTECAMCTRL0, m_RemoteCam0);
		DDX_Control(pDX, IDC_REMOTECAMCTRL1, m_RemoteCam1);
		DDX_Control(pDX, IDC_REMOTECAMCTRL2, m_RemoteCam2);
		DDX_Control(pDX, IDC_REMOTECAMCTRL3, m_RemoteCam3);
		//}}AFX_DATA_MAP
	}
	catch (CException* e)
	{
		e->Delete();
		::AfxMessageBox(_T("ActiveX is not registered!"), MB_OK | MB_ICONERROR);
	}
}

void CRemoteCamViewerView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// Init
	CRemoteCamViewerApp* pApp = (CRemoteCamViewerApp*)::AfxGetApp();
	m_sTitle = pApp->GetProfileString(REG_SECTION, _T("Title"), _T("Set Title in Settings Dialog"));
	m_sLabel0 = pApp->GetProfileString(REG_SECTION, _T("Label0"), _T("Cam0"));
	m_sLabel1 = pApp->GetProfileString(REG_SECTION, _T("Label1"), _T("Cam1"));
	m_sLabel2 = pApp->GetProfileString(REG_SECTION, _T("Label2"), _T("Cam2"));
	m_sLabel3 = pApp->GetProfileString(REG_SECTION, _T("Label3"), _T("Cam3"));
	CRect rc;
	if (::IsWindow(m_RemoteCam0.m_hWnd))
	{
		m_RemoteCam0.SetHost(pApp->GetProfileString(REG_SECTION, _T("Host0"), m_RemoteCam0.GetHost()));
		m_RemoteCam0.SetPort(pApp->GetProfileInt(REG_SECTION, _T("Port0"), m_RemoteCam0.GetPort()));
		m_RemoteCam0.SetMaxFrames(pApp->GetProfileInt(REG_SECTION, _T("MaxFrames0"), m_RemoteCam0.GetMaxFrames()));
		m_RemoteCam0.SetUsername(pApp->GetSecureProfileString(REG_SECTION, _T("Username0"), m_RemoteCam0.GetUsername()));
		m_RemoteCam0.SetPassword(pApp->GetSecureProfileString(REG_SECTION, _T("Password0"), m_RemoteCam0.GetPassword()));
		m_RemoteCam0.SetDisableResend(pApp->GetProfileInt(REG_SECTION, _T("DisableResend0"), m_RemoteCam0.GetDisableResend()));
	}
	if (::IsWindow(m_RemoteCam1.m_hWnd))
	{
		m_RemoteCam1.SetHost(pApp->GetProfileString(REG_SECTION, _T("Host1"), m_RemoteCam1.GetHost()));
		m_RemoteCam1.SetPort(pApp->GetProfileInt(REG_SECTION, _T("Port1"), m_RemoteCam1.GetPort()));
		m_RemoteCam1.SetMaxFrames(pApp->GetProfileInt(REG_SECTION, _T("MaxFrames1"), m_RemoteCam1.GetMaxFrames()));
		m_RemoteCam1.SetUsername(pApp->GetSecureProfileString(REG_SECTION, _T("Username1"), m_RemoteCam1.GetUsername()));
		m_RemoteCam1.SetPassword(pApp->GetSecureProfileString(REG_SECTION, _T("Password1"), m_RemoteCam1.GetPassword()));
		m_RemoteCam1.SetDisableResend(pApp->GetProfileInt(REG_SECTION, _T("DisableResend1"), m_RemoteCam1.GetDisableResend()));
	}
	if (::IsWindow(m_RemoteCam2.m_hWnd))
	{
		m_RemoteCam2.SetHost(pApp->GetProfileString(REG_SECTION, _T("Host2"), m_RemoteCam2.GetHost()));
		m_RemoteCam2.SetPort(pApp->GetProfileInt(REG_SECTION, _T("Port2"), m_RemoteCam2.GetPort()));
		m_RemoteCam2.SetMaxFrames(pApp->GetProfileInt(REG_SECTION, _T("MaxFrames2"), m_RemoteCam2.GetMaxFrames()));
		m_RemoteCam2.SetUsername(pApp->GetSecureProfileString(REG_SECTION, _T("Username2"), m_RemoteCam2.GetUsername()));
		m_RemoteCam2.SetPassword(pApp->GetSecureProfileString(REG_SECTION, _T("Password2"), m_RemoteCam2.GetPassword()));
		m_RemoteCam2.SetDisableResend(pApp->GetProfileInt(REG_SECTION, _T("DisableResend2"), m_RemoteCam2.GetDisableResend()));
	}
	if (::IsWindow(m_RemoteCam3.m_hWnd))
	{
		m_RemoteCam3.SetHost(pApp->GetProfileString(REG_SECTION, _T("Host3"), m_RemoteCam3.GetHost()));
		m_RemoteCam3.SetPort(pApp->GetProfileInt(REG_SECTION, _T("Port3"), m_RemoteCam3.GetPort()));
		m_RemoteCam3.SetMaxFrames(pApp->GetProfileInt(REG_SECTION, _T("MaxFrames3"), m_RemoteCam3.GetMaxFrames()));
		m_RemoteCam3.SetUsername(pApp->GetSecureProfileString(REG_SECTION, _T("Username3"), m_RemoteCam3.GetUsername()));
		m_RemoteCam3.SetPassword(pApp->GetSecureProfileString(REG_SECTION, _T("Password3"), m_RemoteCam3.GetPassword()));
		m_RemoteCam3.SetDisableResend(pApp->GetProfileInt(REG_SECTION, _T("DisableResend3"), m_RemoteCam3.GetDisableResend()));
	}
	SetTimer(POLL_TIMER_ID, POLL_TIMER_MS, NULL);
	SetScrollSizes(MM_TEXT, CSize(0, 0));
	RepositionCams(); // Shifts down the 4 little ocx squares before they are init
	ModifyStyle(NULL, WS_CLIPCHILDREN, 0); // for flicker free window painting (resizing)
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerView message handlers

void CRemoteCamViewerView::OnFileSettings() 
{
	CSettingsDlg dlg;
	dlg.m_sTitle = m_sTitle;
	dlg.m_sLabel0 = m_sLabel0;
	dlg.m_sLabel1 = m_sLabel1;
	dlg.m_sLabel2 = m_sLabel2;
	dlg.m_sLabel3 = m_sLabel3;
	if (dlg.DoModal() == IDOK)
	{
		m_sTitle = dlg.m_sTitle;
		m_sLabel0 = dlg.m_sLabel0;
		m_sLabel1 = dlg.m_sLabel1;
		m_sLabel2 = dlg.m_sLabel2;
		m_sLabel3 = dlg.m_sLabel3;
		::AfxGetApp()->WriteProfileString(REG_SECTION, _T("Title"), m_sTitle);
		::AfxGetApp()->WriteProfileString(REG_SECTION, _T("Label0"), m_sLabel0);
		::AfxGetApp()->WriteProfileString(REG_SECTION, _T("Label1"), m_sLabel1);
		::AfxGetApp()->WriteProfileString(REG_SECTION, _T("Label2"), m_sLabel2);
		::AfxGetApp()->WriteProfileString(REG_SECTION, _T("Label3"), m_sLabel3);
		Invalidate(); // Draw new title and labels
	}
}

void CRemoteCamViewerView::OnFileShowprop0() 
{
	if (::IsWindow(m_RemoteCam0.m_hWnd))
	{
		m_RemoteCam0.PropertyPage(GetSafeHwnd());
		CRemoteCamViewerApp* pApp = (CRemoteCamViewerApp*)::AfxGetApp();
		pApp->WriteProfileString(REG_SECTION, _T("Host0"), m_RemoteCam0.GetHost());
		pApp->WriteProfileInt(REG_SECTION, _T("Port0"), m_RemoteCam0.GetPort());
		pApp->WriteProfileInt(REG_SECTION, _T("MaxFrames0"), m_RemoteCam0.GetMaxFrames());
		pApp->WriteSecureProfileString(REG_SECTION, _T("Username0"), m_RemoteCam0.GetUsername());
		pApp->WriteSecureProfileString(REG_SECTION, _T("Password0"), m_RemoteCam0.GetPassword());
		pApp->WriteProfileInt(REG_SECTION, _T("DisableResend0"), m_RemoteCam0.GetDisableResend());
	}
}

void CRemoteCamViewerView::OnFileShowprop1() 
{
	if (::IsWindow(m_RemoteCam1.m_hWnd))
	{
		m_RemoteCam1.PropertyPage(GetSafeHwnd());
		CRemoteCamViewerApp* pApp = (CRemoteCamViewerApp*)::AfxGetApp();
		pApp->WriteProfileString(REG_SECTION, _T("Host1"), m_RemoteCam1.GetHost());
		pApp->WriteProfileInt(REG_SECTION, _T("Port1"), m_RemoteCam1.GetPort());
		pApp->WriteProfileInt(REG_SECTION, _T("MaxFrames1"), m_RemoteCam1.GetMaxFrames());
		pApp->WriteSecureProfileString(REG_SECTION, _T("Username1"), m_RemoteCam1.GetUsername());
		pApp->WriteSecureProfileString(REG_SECTION, _T("Password1"), m_RemoteCam1.GetPassword());
		pApp->WriteProfileInt(REG_SECTION, _T("DisableResend1"), m_RemoteCam1.GetDisableResend());
	}
}

void CRemoteCamViewerView::OnFileShowprop2() 
{
	if (::IsWindow(m_RemoteCam2.m_hWnd))
	{
		m_RemoteCam2.PropertyPage(GetSafeHwnd());
		CRemoteCamViewerApp* pApp = (CRemoteCamViewerApp*)::AfxGetApp();
		pApp->WriteProfileString(REG_SECTION, _T("Host2"), m_RemoteCam2.GetHost());
		pApp->WriteProfileInt(REG_SECTION, _T("Port2"), m_RemoteCam2.GetPort());
		pApp->WriteProfileInt(REG_SECTION, _T("MaxFrames2"), m_RemoteCam2.GetMaxFrames());
		pApp->WriteSecureProfileString(REG_SECTION, _T("Username2"), m_RemoteCam2.GetUsername());
		pApp->WriteSecureProfileString(REG_SECTION, _T("Password2"), m_RemoteCam2.GetPassword());
		pApp->WriteProfileInt(REG_SECTION, _T("DisableResend2"), m_RemoteCam2.GetDisableResend());
	}
}

void CRemoteCamViewerView::OnFileShowprop3() 
{
	if (::IsWindow(m_RemoteCam3.m_hWnd))
	{
		m_RemoteCam3.PropertyPage(GetSafeHwnd());
		CRemoteCamViewerApp* pApp = (CRemoteCamViewerApp*)::AfxGetApp();
		pApp->WriteProfileString(REG_SECTION, _T("Host3"), m_RemoteCam3.GetHost());
		pApp->WriteProfileInt(REG_SECTION, _T("Port3"), m_RemoteCam3.GetPort());
		pApp->WriteProfileInt(REG_SECTION, _T("MaxFrames3"), m_RemoteCam3.GetMaxFrames());
		pApp->WriteSecureProfileString(REG_SECTION, _T("Username3"), m_RemoteCam3.GetUsername());
		pApp->WriteSecureProfileString(REG_SECTION, _T("Password3"), m_RemoteCam3.GetPassword());
		pApp->WriteProfileInt(REG_SECTION, _T("DisableResend3"), m_RemoteCam3.GetDisableResend());
	}
}

void CRemoteCamViewerView::OnHelpAboutctrl() 
{
	if (::IsWindow(m_RemoteCam0.m_hWnd))
		m_RemoteCam0.AboutBox();
}

void CRemoteCamViewerView::ClipToView(LPRECT lpRect) const
{
	// Check
	if (!lpRect)
		return;

	// Width and Height
	int w = lpRect->right - lpRect->left;
	int h = lpRect->bottom - lpRect->top;

	// Get Client Rect
	CRect rcView;
	GetClientRect(&rcView);
	rcView.top = m_nTitleHight;

	// Clip
	lpRect->left = MAX(rcView.left, MIN(rcView.right - w, lpRect->left));
	lpRect->top = MAX(rcView.top, MIN(rcView.bottom - h, lpRect->top));
	lpRect->right = MIN(rcView.right, lpRect->left + w);
	lpRect->bottom = MIN(rcView.bottom, lpRect->top + h);
}

void CRemoteCamViewerView::RepositionCams()
{
	if (::IsWindow(m_RemoteCam0.m_hWnd) && ::IsWindow(m_RemoteCam1.m_hWnd)	&&
		::IsWindow(m_RemoteCam2.m_hWnd) && ::IsWindow(m_RemoteCam3.m_hWnd))
	{
		// Client Rect
		CRect rcView;
		GetClientRect(&rcView);
		rcView.top = m_nTitleHight;

		// Necessary Sizes and Offsets
		int nNecessaryWidth = MAX(	MAX(m_szRemoteCamSize0.cx + m_szRemoteCamSize1.cx, m_szRemoteCamSize2.cx + m_szRemoteCamSize3.cx),
									MAX(m_szRemoteCamSize0.cx + m_szRemoteCamSize3.cx, m_szRemoteCamSize1.cx + m_szRemoteCamSize2.cx));
		int nNecessaryHeight = MAX(	MAX(m_szRemoteCamSize0.cy + m_szRemoteCamSize2.cy, m_szRemoteCamSize1.cy + m_szRemoteCamSize3.cy),
									MAX(m_szRemoteCamSize0.cy + m_szRemoteCamSize3.cy, m_szRemoteCamSize1.cy + m_szRemoteCamSize2.cy));
		int nLeftOffset = (rcView.Width() - nNecessaryWidth) / 2;
		if (nLeftOffset < 0)
			nLeftOffset = 0;
		int nTopOffset = (rcView.Height() - nNecessaryHeight) / 2;
		if (nTopOffset < 0)
			nTopOffset = 0;
		
		// Calc. Positions
		m_rc0.left = nLeftOffset;
		m_rc0.top = nTopOffset + rcView.top;
		m_rc0.right = m_rc0.left + m_szRemoteCamSize0.cx;
		m_rc0.bottom = m_rc0.top + m_szRemoteCamSize0.cy;
		m_rc1.left = nLeftOffset + MAX(m_szRemoteCamSize0.cx, m_szRemoteCamSize2.cx);
		m_rc1.top = m_rc0.top;
		m_rc1.right = m_rc1.left + m_szRemoteCamSize1.cx;
		m_rc1.bottom = m_rc1.top + m_szRemoteCamSize1.cy;
		m_rc2.left = m_rc0.left;
		m_rc2.top = m_rc0.top + MAX(m_szRemoteCamSize0.cy, m_szRemoteCamSize1.cy);
		m_rc2.right = m_rc2.left + m_szRemoteCamSize2.cx;
		m_rc2.bottom = m_rc2.top + m_szRemoteCamSize2.cy;
		m_rc3.left = m_rc1.left;
		m_rc3.top = m_rc2.top;
		m_rc3.right = m_rc3.left + m_szRemoteCamSize3.cx;
		m_rc3.bottom = m_rc3.top + m_szRemoteCamSize3.cy;
		
		// Clip
		ClipToView(&m_rc0);
		ClipToView(&m_rc1);
		ClipToView(&m_rc2);
		ClipToView(&m_rc3);

		// Set Window Positions
		if (m_rc0.Width() > MIN_SIZE && m_rc0.Height() > MIN_SIZE)
			m_RemoteCam0.SetWindowPos(&wndTop,		m_rc0.left, m_rc0.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		else
			m_RemoteCam0.SetWindowPos(&wndBottom,	m_rc0.left, m_rc0.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		if (m_rc1.Width() > MIN_SIZE && m_rc1.Height() > MIN_SIZE)
			m_RemoteCam1.SetWindowPos(&wndTop,		m_rc1.left, m_rc1.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		else
			m_RemoteCam1.SetWindowPos(&wndBottom,	m_rc1.left, m_rc1.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		if (m_rc2.Width() > MIN_SIZE && m_rc2.Height() > MIN_SIZE)
			m_RemoteCam2.SetWindowPos(&wndTop,		m_rc2.left, m_rc2.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		else
			m_RemoteCam2.SetWindowPos(&wndBottom,	m_rc2.left, m_rc2.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		if (m_rc3.Width() > MIN_SIZE && m_rc3.Height() > MIN_SIZE)
			m_RemoteCam3.SetWindowPos(&wndTop,		m_rc3.left, m_rc3.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		else
			m_RemoteCam3.SetWindowPos(&wndBottom,	m_rc3.left, m_rc3.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
	}
}

void CRemoteCamViewerView::OnTimer(UINT nIDEvent) 
{
	if (::IsWindow(m_RemoteCam0.m_hWnd) && ::IsWindow(m_RemoteCam1.m_hWnd)	&&
		::IsWindow(m_RemoteCam2.m_hWnd) && ::IsWindow(m_RemoteCam3.m_hWnd))
	{
		CRect rc;
		BOOL bChanged = FALSE;
		m_RemoteCam0.GetClientRect(&rc);
		if (rc.Width() > MIN_SIZE && rc.Height() > MIN_SIZE && rc.Size() != m_szRemoteCamSize0)
		{
			m_bInit0 = bChanged = TRUE;
			m_szRemoteCamSize0 = rc.Size();
		}
		m_RemoteCam1.GetClientRect(&rc);
		if (rc.Width() > MIN_SIZE && rc.Height() > MIN_SIZE && rc.Size() != m_szRemoteCamSize1)
		{
			m_bInit1 = bChanged = TRUE;
			m_szRemoteCamSize1 = rc.Size();
		}
		m_RemoteCam2.GetClientRect(&rc);
		if (rc.Width() > MIN_SIZE && rc.Height() > MIN_SIZE && rc.Size() != m_szRemoteCamSize2)
		{
			m_bInit2 = bChanged = TRUE;
			m_szRemoteCamSize2 = rc.Size();
		}
		m_RemoteCam3.GetClientRect(&rc);
		if (rc.Width() > MIN_SIZE && rc.Height() > MIN_SIZE && rc.Size() != m_szRemoteCamSize3)
		{
			m_bInit3 = bChanged = TRUE;
			m_szRemoteCamSize3 = rc.Size();
		}
		if (bChanged)
		{
			RepositionCams();
			Invalidate(); // Draw labels
		}
	}
	CFormView::OnTimer(nIDEvent);
}

BOOL CRemoteCamViewerView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CRemoteCamViewerView::OnDraw(CDC* pDC) 
{
	// Flicker free drawing
	CRect rc, rcClient;
	GetClientRect(&rcClient);
	CMemDC MemDC(pDC, &rcClient);

	// Erase Background
	MemDC.FillSolidRect(&rcClient, ::GetSysColor(COLOR_BTNFACE));

	// Create fonts
	if (!(HFONT)m_TitleFont)
	{
		LOGFONT lf;
		memset(&lf, 0, sizeof(LOGFONT));
		_tcscpy(lf.lfFaceName, TITLE_FONT_FACE);
		lf.lfHeight = -MulDiv(TITLE_FONT_SIZE, pDC->GetDeviceCaps(LOGPIXELSY), 72);
		lf.lfWeight = FW_BOLD;
		lf.lfItalic = 0;
		lf.lfUnderline = 0;
		m_TitleFont.CreateFontIndirect(&lf);
	}
	if (!(HFONT)m_LabelFont)
	{
		LOGFONT lf;
		memset(&lf, 0, sizeof(LOGFONT));
		_tcscpy(lf.lfFaceName, LABEL_FONT_FACE);
		lf.lfHeight = -MulDiv(LABEL_FONT_SIZE, pDC->GetDeviceCaps(LOGPIXELSY), 72);
		lf.lfWeight = FW_NORMAL;
		lf.lfItalic = 0;
		lf.lfUnderline = 0;
		m_LabelFont.CreateFontIndirect(&lf);
	}

	// Set background mode
	int nOldBkMode = MemDC.SetBkMode(TRANSPARENT);

	// Set title color and font
	COLORREF crOldTextColor = MemDC.SetTextColor(TITLE_COLOR);
	CFont* pOldFont = MemDC.SelectObject(&m_TitleFont);

	// Draw title
	int nNewTitleHight = 0;
	if (m_sTitle != _T(""))
	{
		rc = rcClient;
		rc.top = TITLE_MARGIN_TOP;
		nNewTitleHight = MemDC.DrawText(m_sTitle,
										-1,
										&rc,
										(DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK)) + TITLE_MARGIN_TOP + TITLE_MARGIN_BOTTOM;
	}
	if (m_nTitleHight != nNewTitleHight)
	{
		m_nTitleHight = nNewTitleHight;
		RepositionCams();
	}

	// Clean-up
	MemDC.SetTextColor(crOldTextColor);
	MemDC.SelectObject(pOldFont);

	// Set label color and font
	crOldTextColor = MemDC.SetTextColor(LABEL_COLOR);
	pOldFont = MemDC.SelectObject(&m_LabelFont);

	// Draw label0
	if (m_bInit0 && m_sLabel0 != _T(""))
	{
		rc = rcClient;
		rc.right = m_rc0.left - LABEL_MARGIN;
		rc.top = m_rc0.top;
		MemDC.DrawText(	m_sLabel0,
						-1,
						&rc,
						(DT_RIGHT | DT_TOP | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
	}

	// Draw label1
	if (m_bInit1 && m_sLabel1 != _T(""))
	{
		rc = rcClient;
		rc.left = m_rc1.right + LABEL_MARGIN;
		rc.top = m_rc1.top;
		MemDC.DrawText(	m_sLabel1,
						-1,
						&rc,
						(DT_LEFT | DT_TOP | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
	}

	// Draw label2
	if (m_bInit2 && m_sLabel2 != _T(""))
	{
		rc = rcClient;
		rc.right = m_rc2.left - LABEL_MARGIN;
		rc.bottom = m_rc2.bottom;
		MemDC.DrawText(	m_sLabel2,
						-1,
						&rc,
						(DT_RIGHT | DT_BOTTOM | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
	}

	// Draw label3
	if (m_bInit3 && m_sLabel3 != _T(""))
	{
		rc = rcClient;
		rc.left = m_rc3.right + LABEL_MARGIN;
		rc.bottom = m_rc3.bottom;
		MemDC.DrawText(	m_sLabel3,
						-1,
						&rc,
						(DT_LEFT | DT_BOTTOM | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
	}

	// Clean-up
	MemDC.SetTextColor(crOldTextColor);
	MemDC.SelectObject(pOldFont);

	// Clean-up background mode
	MemDC.SetBkMode(nOldBkMode);
}

void CRemoteCamViewerView::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	RepositionCams();
	// OnDraw() is called automatically
}

void CRemoteCamViewerView::OnDestroy() 
{
	CFormView::OnDestroy();
	KillTimer(POLL_TIMER_ID);	
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerView diagnostics

#ifdef _DEBUG
void CRemoteCamViewerView::AssertValid() const
{
	CFormView::AssertValid();
}

void CRemoteCamViewerView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CRemoteCamViewerDoc* CRemoteCamViewerView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRemoteCamViewerDoc)));
	return (CRemoteCamViewerDoc*)m_pDocument;
}
#endif //_DEBUG