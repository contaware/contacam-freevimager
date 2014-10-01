// LicenseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "LicenseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLicenseDlg dialog


CLicenseDlg::CLicenseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLicenseDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLicenseDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLicenseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLicenseDlg)
	DDX_Control(pDX, IDC_EDIT_LICENSE, m_License);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLicenseDlg, CDialog)
	//{{AFX_MSG_MAP(CLicenseDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLicenseDlg message handlers

BOOL CLicenseDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Create Font
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	_tcscpy(lf.lfFaceName, _T("Courier New"));
	HDC hDC = ::GetDC(GetSafeHwnd());
	lf.lfHeight = -MulDiv(8, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	::ReleaseDC(GetSafeHwnd(), hDC);
	lf.lfWeight = FW_NORMAL;
	lf.lfItalic = 0;
	lf.lfUnderline = 0;
	m_Font.CreateFontIndirect(&lf);

	// Note:
	// After finishing using the font we do not have to call
	// DeleteObject, the destructor of CFont will do it for us!

	// Set Font
	m_License.SetFont(&m_Font);
	
	HRSRC hRes = ::FindResource(NULL, MAKEINTRESOURCE(IDR_LICENSE), _T("TEXT"));
	if (hRes == 0)
	{ 
		m_License.SetWindowText(_T("No License File Found, check the Author's Homepage\r\nfor the License Conditions."));
		return TRUE;
    }
	else
	{
		// Load To Mem
		HGLOBAL hMem = ::LoadResource(NULL, hRes);
		if (!hMem)
		{
			m_License.SetWindowText(_T("No License File Found, check the Author's Homepage\r\nfor the License Conditions."));
			return TRUE;
		}
        
        // Lock the resource getting a pointer to it
		BYTE* pData = (BYTE*)::LockResource(hMem);
		DWORD dwSizeRes = ::SizeofResource(NULL, hRes);

		// BOM
		const unsigned char BOM[3] = {0xEF, 0xBB, 0xBF};

		// From UTF-8 to CString
		CString s;
		if (dwSizeRes >= 3)
		{
			if (memcmp(pData, BOM, 3) == 0)
				s = ::FromUTF8(pData + 3, dwSizeRes - 3);
			else
				s = ::FromUTF8(pData, dwSizeRes);
		}
		else
		{
			m_License.SetWindowText(_T("No License File Found, check the Author's Homepage\r\nfor the License Conditions."));
			return TRUE;
		}

		// Display Text
		m_License.SetWindowText(s);

		// The UnlockResource and the FreeResource functions are obsolete.
		// These functions are provided only for compatibility
		// with 16-bit versions of Windows.
		// It is not necessary for Win32-based
		// applications to unlock and free resources. 
		// LoadResource is automatically freed when the module is was loaded
		// from is unloaded.
		//
		// Also calling the dialog several times is not a problem,
		// LoadResource is always returning the same portion of memory!
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CLicenseDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (HIWORD (wParam) == BN_CLICKED)
	{
		switch (LOWORD (wParam))
		{
			case IDOK:
				OnOK();
				return TRUE;
			case IDCANCEL:
				return TRUE;
			default:
				return CDialog::OnCommand(wParam, lParam);
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}
