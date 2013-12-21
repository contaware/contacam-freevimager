// FTPUploadConfigurationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "VideoDeviceDoc.h"
#include "FTPUploadConfigurationDlg.h"
#include "FTPTransfer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CFTPUploadConfigurationDlg dialog

CFTPUploadConfigurationDlg::CFTPUploadConfigurationDlg(UINT idd)
	: CDialog(idd, NULL)
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

	// Passive Mode?
	CButton* pCheck = (CButton*)GetDlgItem(IDC_PASV);
	pCheck->SetCheck(m_FTPUploadConfiguration.m_bPasv ? 1 : 0);

	// Use Proxy Server?
	pCheck = (CButton*)GetDlgItem(IDC_PROXY);
	pCheck->SetCheck(m_FTPUploadConfiguration.m_bProxy ? 1 : 0);
	
	// Proxy Server Name
	pEdit = (CEdit*)GetDlgItem(IDC_PROXY_HOST_NAME);
	pEdit->SetWindowText(m_FTPUploadConfiguration.m_sProxy);

	// Username and Password
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->SetWindowText(m_FTPUploadConfiguration.m_sUsername);
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->SetWindowText(m_FTPUploadConfiguration.m_sPassword);
	
	// Files to upload (optional control)
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_FILES_TO_UPLOAD);
	if (pComboBox)
	{
		pComboBox->AddString(_T("AVI"));
		pComboBox->AddString(_T("GIF"));
		pComboBox->AddString(_T("SWF"));
		pComboBox->AddString(_T("AVI + GIF"));
		pComboBox->AddString(_T("SWF + GIF"));
		pComboBox->AddString(_T("AVI + SWF + GIF"));
		pComboBox->SetCurSel((int)m_FTPUploadConfiguration.m_FilesToUpload);
	}

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

	CButton* pCheck = (CButton*)GetDlgItem(IDC_PASV);
	m_FTPUploadConfiguration.m_bPasv = pCheck->GetCheck();

	pCheck = (CButton*)GetDlgItem(IDC_PROXY);
	m_FTPUploadConfiguration.m_bProxy = pCheck->GetCheck();

	pEdit = (CEdit*)GetDlgItem(IDC_PROXY_HOST_NAME);
	pEdit->GetWindowText(sText);
	m_FTPUploadConfiguration.m_sProxy = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->GetWindowText(sText);
	m_FTPUploadConfiguration.m_sUsername = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->GetWindowText(sText);
	m_FTPUploadConfiguration.m_sPassword = sText;

	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_FILES_TO_UPLOAD);
	if (pComboBox)
	{
		m_FTPUploadConfiguration.m_FilesToUpload =
			(CVideoDeviceDoc::FilesToUploadType)pComboBox->GetCurSel();
	}
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
		BeginWaitCursor();

		CFTPTransfer FTP(NULL);
		FTP.m_sRemoteFile = _T("");
		FTP.m_sLocalFile = _T("");
		FTP.m_sServer = m_FTPUploadConfiguration.m_sHost;
		FTP.m_nPort = m_FTPUploadConfiguration.m_nPort;
		FTP.m_bDownload = FALSE;
		FTP.m_bBinary = m_FTPUploadConfiguration.m_bBinary;
		FTP.m_bPromptOverwrite = FALSE;
		FTP.m_dBandwidthLimit = 0.0;// For BANDWIDTH throttling, the value in KBytes / Second to limit the connection to
		FTP.m_bPasv = m_FTPUploadConfiguration.m_bPasv;
		FTP.m_bUsePreconfig = TRUE;	// Should preconfigured settings be used i.e. take proxy settings etc from the control panel
		FTP.m_bUseProxy = m_FTPUploadConfiguration.m_bProxy;
		FTP.m_sProxy = m_FTPUploadConfiguration.m_sProxy;
		FTP.m_dwConnectionTimeout = m_FTPUploadConfiguration.m_dwConnectionTimeout;
		if (!m_FTPUploadConfiguration.m_sUsername.IsEmpty())
		{
			FTP.m_sUserName = m_FTPUploadConfiguration.m_sUsername;
			FTP.m_sPassword = m_FTPUploadConfiguration.m_sPassword;
		}

		// Test Connection
		if (FTP.Test())
		{
			EndWaitCursor();
			::AfxMessageBox(ML_STRING(1771, "Success: Host Reachable."), MB_ICONINFORMATION);
		}
		else
		{
			EndWaitCursor();
			::AfxMessageBox(FTP.m_sError, MB_ICONSTOP);
		}
	}	
}

#endif
