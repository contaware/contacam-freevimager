// Winsock2MissingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "Winsock2MissingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWinsock2MissingDlg dialog


CWinsock2MissingDlg::CWinsock2MissingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWinsock2MissingDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWinsock2MissingDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CWinsock2MissingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWinsock2MissingDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWinsock2MissingDlg, CDialog)
	//{{AFX_MSG_MAP(CWinsock2MissingDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinsock2MissingDlg message handlers

BOOL CWinsock2MissingDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Title
	SetWindowText(APPNAME_NOEXT);

	// Subclass Link Edit Box
	m_TextLink.SubclassDlgItem(IDC_WEB_LINK, this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
