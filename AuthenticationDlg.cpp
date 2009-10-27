// AuthenticationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "AuthenticationDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAuthenticationDlg dialog


CAuthenticationDlg::CAuthenticationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAuthenticationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAuthenticationDlg)
	m_bSaveAuthenticationData = FALSE;
	m_sPassword = _T("");
	m_sUsername = _T("");
	//}}AFX_DATA_INIT
}


void CAuthenticationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAuthenticationDlg)
	DDX_Check(pDX, IDC_CHECK_AUTH_SAVE, m_bSaveAuthenticationData);
	DDX_Text(pDX, IDC_AUTH_PASSWORD, m_sPassword);
	DDX_Text(pDX, IDC_AUTH_USERNAME, m_sUsername);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAuthenticationDlg, CDialog)
	//{{AFX_MSG_MAP(CAuthenticationDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAuthenticationDlg message handlers
