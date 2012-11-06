// ConnectErrMsgBoxDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "Helpers.h"
#include "ConnectErrMsgBoxDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

// Message margins
#define ERRMSGTEXT_LEFT_MARGIN		10
#define ERRMSGTEXT_RIGHT_MARGIN		16

/////////////////////////////////////////////////////////////////////////////
// CConnectErrMsgBoxDlg dialog

CConnectErrMsgBoxDlg::CConnectErrMsgBoxDlg(LPCTSTR lpszText, CWnd* pParent /*=NULL*/)
	: CDialog(CConnectErrMsgBoxDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConnectErrMsgBoxDlg)
	m_bAutorun = TRUE;
	//}}AFX_DATA_INIT
	
	// Store message text
	m_sMsgText = lpszText;

	// Create message font
#if (WINVER >= 0x0600)
    NONCLIENTMETRICS ncm = {sizeof(NONCLIENTMETRICS) - sizeof(int)}; // to make it work with all OSs leave it like this!
#else
	NONCLIENTMETRICS ncm = {sizeof(NONCLIENTMETRICS)};
#endif
    VERIFY(::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0));
    m_MsgFont.CreateFontIndirect(&(ncm.lfMessageFont));

	// Create vista style colors and brushes
	if (g_bWinVistaOrHigher)
	{
		m_crTop = ::GetSysColor(COLOR_WINDOW);
		m_crBottom = ::GetSysColor(COLOR_BTNFACE);
		m_TopBrush.CreateSolidBrush(m_crTop);
		m_BottomBrush.CreateSolidBrush(m_crBottom);
	}
}

void CConnectErrMsgBoxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConnectErrMsgBoxDlg)
	DDX_Check(pDX, IDC_CHECK_AUTORUN, m_bAutorun);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CConnectErrMsgBoxDlg, CDialog)
	//{{AFX_MSG_MAP(CConnectErrMsgBoxDlg)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_CHECK_AUTORUN, OnCheckAutorun)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConnectErrMsgBoxDlg message handlers

BOOL CConnectErrMsgBoxDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Set error icon
	CStatic* pIcon = (CStatic*)GetDlgItem(IDC_CONNECTERR_ICON);
	pIcon->SetIcon(::LoadIcon(NULL, IDI_ERROR));

	// Set dialog title
	SetWindowText(::AfxGetAppName());

	// Play error sound
	::MessageBeep(MB_ICONERROR);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConnectErrMsgBoxDlg::OnCheckAutorun() 
{
	// If closing the dialog with ESC or X button
	// the m_bAutorun dialog variable has the selected value
	UpdateData(TRUE);
}

BOOL CConnectErrMsgBoxDlg::OnEraseBkgnd(CDC* pDC) 
{
	if (g_bWinVistaOrHigher)
	{
		// Get client rect
		CRect rect;
		GetClientRect(rect);

		// Get icon rect
		CStatic* pIcon = (CStatic*)GetDlgItem(IDC_CONNECTERR_ICON);
		CRect rcIcon;
		pIcon->GetWindowRect(&rcIcon);
		ScreenToClient(&rcIcon);

		// Get ok button rect
		CButton* pOK = (CButton*)GetDlgItem(IDOK);
		CRect rcOK;
		pOK->GetWindowRect(&rcOK);
		ScreenToClient(&rcOK);
		
		// Calc. cut line between top and bottom region
		int nCutLine = rcIcon.bottom + 2 * (rcOK.top - rcIcon.bottom) / 3;

		// Fill top rect
		CRect rcTop(rect);
		rcTop.bottom = nCutLine;
		pDC->FillRect(&rcTop, &m_TopBrush);

		// Fill bottom rect
		CRect rcBottom(rect);
		rcBottom.top = nCutLine;
		pDC->FillRect(&rcBottom, &m_BottomBrush);

		return TRUE;
	}
	else
		return CDialog::OnEraseBkgnd(pDC);
}

HBRUSH CConnectErrMsgBoxDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if (g_bWinVistaOrHigher)
	{
		// For checkbox and OK button return bottom brush
		TCHAR classname[MAX_PATH];
		if (::GetClassName(pWnd->m_hWnd, classname, MAX_PATH) == 0)
			return hbr; // on error
		if (_tcsicmp(classname, _T("BUTTON")) == 0)
		{
			pDC->SetBkColor(m_crBottom); // checkbox text background
			return (HBRUSH)m_BottomBrush;
		}

		// For icon return top brush
		return (HBRUSH)m_TopBrush;
	}
	else
		return hbr;
}

void CConnectErrMsgBoxDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	// Calc. message text container rect
	CRect rect;
	GetClientRect(rect);
	CStatic* pIcon = (CStatic*)GetDlgItem(IDC_CONNECTERR_ICON);
	CRect rcIcon;
	pIcon->GetWindowRect(&rcIcon);
	ScreenToClient(&rcIcon);
	rect.top = rcIcon.top;
	rect.bottom = rcIcon.bottom;
	rect.left = rcIcon.right + ERRMSGTEXT_LEFT_MARGIN;
	rect.right -= ERRMSGTEXT_RIGHT_MARGIN;

	// Set text font and color
	CFont* pOldFont = dc.SelectObject(&m_MsgFont);
	int nOldMode = dc.SetBkMode(TRANSPARENT);
	COLORREF crOldText = dc.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));

	// Break long strings that have no spaces
	CString s(m_sMsgText);
	CRect rcText(rect);
	rcText.bottom = 0;
	dc.DrawText(s, s.GetLength(), rcText, DT_WORDBREAK | DT_CALCRECT);
	while (rcText.Width() > rect.Width())
	{
		for (int nCount = s.GetLength() ; nCount > 0 ; nCount--)
		{
			CRect rc(rect);
			rc.bottom = 0;
			dc.DrawText(s, nCount, rc, DT_WORDBREAK | DT_CALCRECT);
			if (rc.Width() <= rect.Width())
			{
				s.Insert(nCount, _T("\r\n"));
				break;
			}
		}
		rcText = rect;
		rcText.bottom = 0;
		dc.DrawText(s, s.GetLength(), rcText, DT_WORDBREAK | DT_CALCRECT);
	}

	// Center vertically
	rcText.OffsetRect(0, (rect.Height() - rcText.Height()) / 2);

	// Draw text
	dc.DrawText(s, s.GetLength(), rcText, DT_WORDBREAK);
	
	// Clean-up
	dc.SetTextColor(crOldText);
	dc.SetBkMode(nOldMode);
	dc.SelectObject(pOldFont);
	
	// Do not call CDialog::OnPaint() for painting messages
}

#endif
