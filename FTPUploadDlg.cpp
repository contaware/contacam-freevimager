// FTPUploadDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "FTPUploadDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CFTPUploadDlg dialog

CFTPUploadDlg::CFTPUploadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_FTP_UPLOAD, pParent)
	, m_sHost(_T(""))
	, m_sUsername(_T(""))
	, m_sPassword(_T(""))
{
	
}

CFTPUploadDlg::~CFTPUploadDlg()
{
}

void CFTPUploadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_HOST_NAME, m_sHost);
	DDX_Text(pDX, IDC_AUTH_USERNAME, m_sUsername);
	DDX_Text(pDX, IDC_AUTH_PASSWORD, m_sPassword);
}

BEGIN_MESSAGE_MAP(CFTPUploadDlg, CDialog)
	//{{AFX_MSG_MAP(CFTPUploadDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#endif
