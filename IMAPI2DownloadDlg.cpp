// IMAPI2DownloadDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "IMAPI2DownloadDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIMAPI2DownloadDlg dialog


CIMAPI2DownloadDlg::CIMAPI2DownloadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIMAPI2DownloadDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIMAPI2DownloadDlg)
	m_sTextRow1 = _T("");
	m_sTextRow2 = _T("");
	//}}AFX_DATA_INIT
}


void CIMAPI2DownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIMAPI2DownloadDlg)
	DDX_Text(pDX, IDC_EDIT_TEXTROW1, m_sTextRow1);
	DDX_Text(pDX, IDC_EDIT_TEXTROW2, m_sTextRow2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIMAPI2DownloadDlg, CDialog)
	//{{AFX_MSG_MAP(CIMAPI2DownloadDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIMAPI2DownloadDlg message handlers

BOOL CIMAPI2DownloadDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Title
	SetWindowText(APPNAME_NOEXT);

	// Subclass Link Edit Box
	m_TextLink.SubclassDlgItem(IDC_EDIT_LINK, this);
	m_TextLink.SetWindowText(m_sTextLink);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
