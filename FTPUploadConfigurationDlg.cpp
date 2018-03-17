// FTPUploadConfigurationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "VideoDeviceDoc.h"
#include "FTPUploadConfigurationDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CFTPUploadConfigurationDlg dialog

CFTPUploadConfigurationDlg::CFTPUploadConfigurationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFTPUploadConfigurationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFTPUploadConfigurationDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CFTPUploadConfigurationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFTPUploadConfigurationDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFTPUploadConfigurationDlg, CDialog)
	//{{AFX_MSG_MAP(CFTPUploadConfigurationDlg)
	ON_BN_CLICKED(IDC_BUTTON_TEST, OnButtonTest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFTPUploadConfigurationDlg message handlers

BOOL CFTPUploadConfigurationDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Server Name
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_HOST_NAME);
	pEdit->SetWindowText(m_FTPUploadConfiguration.m_sHost);

	// Remote Directory
	pEdit = (CEdit*)GetDlgItem(IDC_FTP_REMOTEDIR);
	pEdit->SetWindowText(m_FTPUploadConfiguration.m_sRemoteDir);
	
	// Server Port
	pEdit = (CEdit*)GetDlgItem(IDC_HOST_PORT);
	CString sPort;
	sPort.Format(_T("%i"), m_FTPUploadConfiguration.m_nPort);
	pEdit->SetWindowText(sPort);

	// Username and Password
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->SetWindowText(m_FTPUploadConfiguration.m_sUsername);
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->SetWindowText(m_FTPUploadConfiguration.m_sPassword);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFTPUploadConfigurationDlg::CopyToStruct()
{
	CString sText;

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_HOST_NAME);
	pEdit->GetWindowText(sText);
	m_FTPUploadConfiguration.m_sHost = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_FTP_REMOTEDIR);
	pEdit->GetWindowText(sText);
	m_FTPUploadConfiguration.m_sRemoteDir = sText;
	
	pEdit = (CEdit*)GetDlgItem(IDC_HOST_PORT);
	pEdit->GetWindowText(sText);
	int nPort = _tcstol(sText.GetBuffer(0), NULL, 10);
	sText.ReleaseBuffer();
	if (nPort > 0 && nPort <= 65535) // Port 0 is Reserved
		m_FTPUploadConfiguration.m_nPort = nPort;
	else
		m_FTPUploadConfiguration.m_nPort = 21;

	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->GetWindowText(sText);
	m_FTPUploadConfiguration.m_sUsername = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->GetWindowText(sText);
	m_FTPUploadConfiguration.m_sPassword = sText;
}

void CFTPUploadConfigurationDlg::OnOK() 
{
	CopyToStruct();
	CDialog::OnOK();
}

void CFTPUploadConfigurationDlg::OnButtonTest() 
{
	CopyToStruct();

	if (m_FTPUploadConfiguration.m_sHost.IsEmpty()) 
		::AfxMessageBox(ML_STRING(1770, "Please Enter A Host Name"));
	else 
	{
		// Set timeout and do not reconnect
		// Note: we set net:reconnect-interval-base to 1 sec because even if
		//       net:max-retries is configured to not retry in some circumstances one retry attempt is made
		CString sSets;
		sSets.Format(_T("set net:timeout %d; set net:max-retries 1; set net:reconnect-interval-base 1; "), FTPPROG_TIMEOUT_SEC);

		// Implicit FTPS
		CString sProto;
		if (m_FTPUploadConfiguration.m_nPort == 990)
		{
			sSets += _T("set ssl:verify-certificate no; ");
			sSets += _T("set ftp:passive-mode on; ");
			sProto = _T("ftps://");
		}
		// SSH File Transfer Protocol SFTP
		else if (m_FTPUploadConfiguration.m_nPort == 22)
		{
			sSets += _T("set sftp:auto-confirm yes; ");
			sSets += _T("set sftp:connect-program './ssh.exe'; "); // when executing lftp.exe current directory must contain ssh.exe
			sProto = _T("sftp://");
		}
		// Explicit FTPES or unencrypted (both on port 21)
		else
		{
			sSets += _T("set ssl:verify-certificate no; ");
			sSets += _T("set ftp:passive-mode on; ");
			sProto = _T("ftp://");
		}

		// Port
		CString sPort;
		sPort.Format(_T("-p %d "), m_FTPUploadConfiguration.m_nPort);

		// Username and password (lftp uses anonymous if nothing supplied)
		CString sUser;
		if (!m_FTPUploadConfiguration.m_sUsername.IsEmpty() && !m_FTPUploadConfiguration.m_sPassword.IsEmpty())
			sUser.Format(_T("-u \"%s,%s\" "), m_FTPUploadConfiguration.m_sUsername, m_FTPUploadConfiguration.m_sPassword);
		else if (!m_FTPUploadConfiguration.m_sUsername.IsEmpty() && m_FTPUploadConfiguration.m_sPassword.IsEmpty())
			sUser.Format(_T("-u \"%s\" "), m_FTPUploadConfiguration.m_sUsername);

		// FTP
		CString sOptions(CString(_T("-e \"")) + sSets + _T("cls -a -l -s --filesize -h --sort=name -I -D && echo '[OK]'; pwd\" ") +
						sPort + sUser + sProto + m_FTPUploadConfiguration.m_sHost);
		HANDLE hFTP = CVideoDeviceDoc::FTPCall(sOptions, TRUE);
		if (hFTP)
			CloseHandle(hFTP);
	}	
}

#endif
