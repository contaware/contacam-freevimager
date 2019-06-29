// TextEntryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TextEntryDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_EDITCTL			11

CTextEntryDlg::CTextEntryDlg()
{

}

CTextEntryDlg::~CTextEntryDlg()
{

}

BEGIN_MESSAGE_MAP(CTextEntryDlg, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MESSAGE_MAP()

int CTextEntryDlg::Show(CWnd* pParent, CWnd* pCenterIn, LPCTSTR pszTitle, BOOL bPassword/*=FALSE*/)
{
	// Register window class
	CString strClass = ::AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, LoadCursor(NULL, IDC_ARROW), 
											CBrush(::GetSysColor(COLOR_BTNFACE)));

	// Get parent window if not provided
	if (!pParent)
		pParent = ::AfxGetApp()->GetMainWnd();

	// Create window
    if (!CreateEx(	WS_EX_DLGMODALFRAME, strClass, pszTitle, WS_SYSMENU | WS_POPUP | 
					WS_BORDER | WS_CAPTION, 0, 0, ::SystemDPIScale(200), 4 * GetSystemMetrics(SM_CYCAPTION),
					pParent ? pParent->GetSafeHwnd() : NULL, NULL))
	{
		return 0;
	}

	// Centers inside the given window
	CenterWindow(pCenterIn);

	// Sizes
	CRect rcClient;
	GetClientRect(rcClient);
	int nMarginX = ::SystemDPIScale(8);
	int nMarginY = ::SystemDPIScale(4);
	int nEditHeight = ::SystemDPIScale(20);
	int nButtonWidth = rcClient.Width() / 4;
	int nButtonSpacing = rcClient.Width() / 16;

	// OK button
	typedef LPCWSTR(WINAPI * FPMB_GETSTRING)(int);
	HMODULE hMod = ::LoadLibrary(_T("user32.dll"));
	FPMB_GETSTRING fpMB_GetString = (FPMB_GETSTRING)::GetProcAddress(hMod, "MB_GetString");
	if (!m_ctlOK.Create(fpMB_GetString(0), WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON,
		CRect(	rcClient.Width() - nMarginX - nButtonWidth - nButtonSpacing - nButtonWidth,
				nMarginY + nEditHeight + nMarginY,
				rcClient.Width() - nMarginX - nButtonWidth - nButtonSpacing,
				rcClient.Height() - nMarginY),
		this, IDOK))
	{
		::FreeLibrary(hMod);
		return 0;
	}
	// Cancel button
	if (!m_ctlCancel.Create(fpMB_GetString(1), WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_PUSHBUTTON,
		CRect(	rcClient.Width() - nMarginX - nButtonWidth,
				nMarginY + nEditHeight + nMarginY,
				rcClient.Width() - nMarginX,
				rcClient.Height() - nMarginY),
		this, IDCANCEL))
	{
		::FreeLibrary(hMod);
		return 0;
	}
	::FreeLibrary(hMod);

	// Edit box
	DWORD dwEditStyles = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL;
	if (bPassword)
		dwEditStyles |= ES_PASSWORD;
	if (!m_ctlEdit.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), m_sText, dwEditStyles,
		CRect(	nMarginX,
				nMarginY,
				rcClient.Width() - nMarginX,
				nMarginY + nEditHeight),
		this, IDC_EDITCTL))
	{
		return 0;
	}

	// Font
	m_Font.CreatePointFont(80, g_szDefaultFontFace);
	m_ctlOK.SetFont(&m_Font, false);
	m_ctlCancel.SetFont(&m_Font, false);
	m_ctlEdit.SetFont(&m_Font, false);

	// Show
	ShowWindow(SW_SHOW);

	// Enter modal loop
	return LoopIt(pParent);
}

BOOL CTextEntryDlg::OnEraseBkgnd(CDC* pDC) 
{
	CBrush brBkgd(::GetSysColor(COLOR_BTNFACE));
	CRect rcFillArea;

	pDC->GetClipBox(&rcFillArea);
	pDC->FillRect(&rcFillArea, &brBkgd);
	
	return TRUE;
}

int CTextEntryDlg::LoopIt(CWnd *pParent)
{
	pParent->EnableWindow(FALSE);
	EnableWindow(TRUE);

	// Enter modal loop
	DWORD dwFlags = MLF_SHOWONIDLE;
	if (GetStyle() & DS_NOIDLEMSG)
		dwFlags |= MLF_NOIDLEMSG;

	m_ctlEdit.SetFocus();

	int nReturn = RunModalLoop(MLF_NOIDLEMSG);

	if (m_hWnd != NULL)
		SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);

	pParent->SetFocus();
	if (GetParent() != NULL && ::GetActiveWindow() == m_hWnd)
		::SetActiveWindow(pParent->m_hWnd);

	if (::IsWindow(m_hWnd))
		DestroyWindow();

	pParent->EnableWindow(TRUE);

	return nReturn;
}

void CTextEntryDlg::OnClose() 
{
	EndModalLoop(IDCANCEL);
}

void CTextEntryDlg::OnOK()
{
	m_ctlEdit.GetWindowText(m_sText);
	EndModalLoop(IDOK);
}

void CTextEntryDlg::OnCancel()
{
	EndModalLoop(IDCANCEL);
}

BOOL CTextEntryDlg::PreTranslateMessage(MSG* pMsg) 
{
	// Check
	ASSERT(m_hWnd != NULL);

	// Allow tooltip messages to be filtered
	if (CWnd::PreTranslateMessage(pMsg))
		return TRUE;

	// Don't translate dialog messages when in Shift+F1 help mode
	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	if (pFrameWnd != NULL && pFrameWnd->m_bHelpMode)
		return FALSE;

	// Fix around for VK_ESCAPE in a multiline Edit that is on a Dialog
	// that doesn't have a cancel or the cancel is disabled.
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_CANCEL) &&
		(::GetWindowLong(pMsg->hwnd, GWL_STYLE) & ES_MULTILINE))
	{
		HWND hItem = ::GetDlgItem(m_hWnd, IDCANCEL);
		EndModalLoop(IDCANCEL);
		return TRUE;
	}

	// Filter both messages to dialog and from children
	return PreTranslateInput(pMsg);
}
