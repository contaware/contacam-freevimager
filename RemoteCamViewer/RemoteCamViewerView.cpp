// RemoteCamViewerView.cpp : implementation of the CRemoteCamViewerView class
//

#include "stdafx.h"
#include "RemoteCamViewer.h"
#include "RemoteCamViewerDoc.h"
#include "RemoteCamViewerView.h"

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

#define POLL_TIMER_ID		1
#define POLL_TIMER_MS		500
#define REG_SECTION			_T("GeneralApp")
#define MIN_SIZE			32

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerView

IMPLEMENT_DYNCREATE(CRemoteCamViewerView, CFormView)

BEGIN_MESSAGE_MAP(CRemoteCamViewerView, CFormView)
	//{{AFX_MSG_MAP(CRemoteCamViewerView)
	ON_COMMAND(ID_HELP_ABOUTCTRL, OnHelpAboutctrl)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_COMMAND(ID_TOOLS_SHOWPROP0, OnToolsShowprop0)
	ON_COMMAND(ID_TOOLS_SHOWPROP1, OnToolsShowprop1)
	ON_COMMAND(ID_TOOLS_SHOWPROP2, OnToolsShowprop2)
	ON_COMMAND(ID_TOOLS_SHOWPROP3, OnToolsShowprop3)
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

	// For flicker free window painting (resizing)
	ModifyStyle(NULL, WS_CLIPCHILDREN, 0);
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerView message handlers

void CRemoteCamViewerView::OnHelpAboutctrl() 
{
	if (::IsWindow(m_RemoteCam0.m_hWnd))
		m_RemoteCam0.AboutBox();
}

void CRemoteCamViewerView::OnToolsShowprop0() 
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

void CRemoteCamViewerView::OnToolsShowprop1() 
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

void CRemoteCamViewerView::OnToolsShowprop2() 
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

void CRemoteCamViewerView::OnToolsShowprop3() 
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
		CRect rc0, rc1, rc2, rc3;
		rc0.left = nLeftOffset;
		rc0.top = nTopOffset;
		rc0.right = rc0.left + m_szRemoteCamSize0.cx;
		rc0.bottom = rc0.top + m_szRemoteCamSize0.cy;
		rc1.left = nLeftOffset + MAX(m_szRemoteCamSize0.cx, m_szRemoteCamSize2.cx);
		rc1.top = rc0.top;
		rc1.right = rc1.left + m_szRemoteCamSize1.cx;
		rc1.bottom = rc1.top + m_szRemoteCamSize1.cy;
		rc2.left = rc0.left;
		rc2.top = nTopOffset + MAX(m_szRemoteCamSize0.cy, m_szRemoteCamSize1.cy);
		rc2.right = rc2.left + m_szRemoteCamSize2.cx;
		rc2.bottom = rc2.top + m_szRemoteCamSize2.cy;
		rc3.left = rc1.left;
		rc3.top = rc2.top;
		rc3.right = rc3.left + m_szRemoteCamSize3.cx;
		rc3.bottom = rc3.top + m_szRemoteCamSize3.cy;
		
		// Clip
		ClipToView(&rc0);
		ClipToView(&rc1);
		ClipToView(&rc2);
		ClipToView(&rc3);

		// Set Window Positions
		if (rc0.Width() > MIN_SIZE && rc0.Height() > MIN_SIZE)
			m_RemoteCam0.SetWindowPos(&wndTop, rc0.left, rc0.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		else
			m_RemoteCam0.SetWindowPos(&wndBottom, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		if (rc1.Width() > MIN_SIZE && rc1.Height() > MIN_SIZE)
			m_RemoteCam1.SetWindowPos(&wndTop, rc1.left, rc1.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		else
			m_RemoteCam1.SetWindowPos(&wndBottom, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		if (rc2.Width() > MIN_SIZE && rc2.Height() > MIN_SIZE)
			m_RemoteCam2.SetWindowPos(&wndTop, rc2.left, rc2.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		else
			m_RemoteCam2.SetWindowPos(&wndBottom, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		if (rc3.Width() > MIN_SIZE && rc3.Height() > MIN_SIZE)
			m_RemoteCam3.SetWindowPos(&wndTop, rc3.left, rc3.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		else
			m_RemoteCam3.SetWindowPos(&wndBottom, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
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
			bChanged = TRUE;
			m_szRemoteCamSize0 = rc.Size();
		}
		m_RemoteCam1.GetClientRect(&rc);
		if (rc.Width() > MIN_SIZE && rc.Height() > MIN_SIZE && rc.Size() != m_szRemoteCamSize1)
		{
			bChanged = TRUE;
			m_szRemoteCamSize1 = rc.Size();
		}
		m_RemoteCam2.GetClientRect(&rc);
		if (rc.Width() > MIN_SIZE && rc.Height() > MIN_SIZE && rc.Size() != m_szRemoteCamSize2)
		{
			bChanged = TRUE;
			m_szRemoteCamSize2 = rc.Size();
		}
		m_RemoteCam3.GetClientRect(&rc);
		if (rc.Width() > MIN_SIZE && rc.Height() > MIN_SIZE && rc.Size() != m_szRemoteCamSize3)
		{
			bChanged = TRUE;
			m_szRemoteCamSize3 = rc.Size();
		}
		if (bChanged)
			RepositionCams();
	}
	CFormView::OnTimer(nIDEvent);
}

void CRemoteCamViewerView::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	RepositionCams();
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