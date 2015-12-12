#include "stdafx.h"
#include "StaticLink.h"
#include "resource.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CStaticLink, CStatic)


BEGIN_MESSAGE_MAP(CStaticLink, CStatic)
	//{{AFX_MSG_MAP(CStaticLink)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
	ON_CONTROL_REFLECT(STN_CLICKED, OnClicked)
END_MESSAGE_MAP()

CStaticLink::CStaticLink()
{
	m_crUnvisited = RGB(0, 0, 255);
	m_crVisited = RGB(128, 0, 128);
	m_bVisited = FALSE;
}

CStaticLink::~CStaticLink()
{
}

HBRUSH CStaticLink::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	nCtlColor;
	ASSERT(nCtlColor == CTLCOLOR_STATIC);
	DWORD dwStyle = GetStyle();
	
	// Notify Flag has to be set in resource editor
	ASSERT(dwStyle & SS_NOTIFY);

	HBRUSH hBrush = NULL;
	if ((dwStyle & 0xff) <= SS_RIGHT)
	{
		if (!(HFONT)m_Font)
		{
			LOGFONT lf;
			GetFont()->GetObject(sizeof(lf), &lf);
			lf.lfUnderline = TRUE;
			m_Font.CreateFontIndirect(&lf);
		}
	
		// Font & Color
		pDC->SelectObject(&m_Font);
		pDC->SetTextColor(m_bVisited ? m_crVisited : m_crUnvisited);
		pDC->SetBkMode(TRANSPARENT);
	
		hBrush = (HBRUSH)::GetStockObject(NULL_BRUSH);
	}

	return hBrush;
}

BOOL CStaticLink::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	pWnd;
	nHitTest;
	message;
	
	// IDC_HAND_CURSOR has to be in Resource
	HCURSOR hCursor = ::AfxGetApp()->LoadCursor(IDC_HAND_CURSOR);	
	ASSERT(hCursor);
	 
	::SetCursor(hCursor);
	
	return TRUE;
}

void CStaticLink::OnClicked()
{
	// Link from Static Control if m_sLink not set
	if (m_sLink.IsEmpty())
		GetWindowText(m_sLink);

	CString str;
	if (m_sLink.Find(_T('@')) != -1)
		str = _T("mailto:");
	str += m_sLink;

	// Execute Link
	HINSTANCE hInst = ::ShellExecute(NULL, _T("open"), str, NULL, NULL, SW_SHOWNORMAL);
	if (hInst > (HINSTANCE)32)
	{
		m_bVisited = TRUE;
		Invalidate();
	}
	else
		::LogLine(_T("Link Could Not Be Executed <%s>"), m_sLink);
}
