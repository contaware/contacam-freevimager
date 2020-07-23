// SendMailConfigurationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "VideoDeviceDoc.h"
#include "SendMailConfigurationDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CSendMailConfigurationDlg dialog

CSendMailConfigurationDlg::CSendMailConfigurationDlg(const CString& sName)
	: CDialog(CSendMailConfigurationDlg::IDD, NULL)
{
	//{{AFX_DATA_INIT(CSendMailConfigurationDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_sName = sName;
}

void CSendMailConfigurationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendMailConfigurationDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSendMailConfigurationDlg, CDialog)
	//{{AFX_MSG_MAP(CSendMailConfigurationDlg)
	ON_BN_CLICKED(IDC_BUTTON_TEST, OnButtonTest)
	ON_BN_CLICKED(IDC_CHECK_SHOW_PASSWORD, OnCheckShowPassword)
	ON_EN_CHANGE(IDC_SENDER_MAIL, OnChangeEditSenderMail)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSendMailConfigurationDlg message handlers

BOOL CSendMailConfigurationDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CString s;

	// To Email
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_RECEIVER_MAIL);
	pEdit->SetWindowText(m_SendMailConfiguration.m_sTo);

	// From Name
	pEdit = (CEdit*)GetDlgItem(IDC_SENDER_NAME);
	pEdit->SetWindowText(m_SendMailConfiguration.m_sFromName);

	// From Email
	// Note: SetWindowText() calls OnChangeEditSenderMail() which can init:
	// IDC_HOST_NAME, IDC_HOST_PORT, IDC_AUTH_USERNAME and IDC_CONNECTIONTYPE.
	// That's not a problem because those fields are overwritten below after
	// the mentioned OnChangeEditSenderMail() call!
	pEdit = (CEdit*)GetDlgItem(IDC_SENDER_MAIL);
	pEdit->SetWindowText(m_SendMailConfiguration.m_sFrom);

	// Subject Line
	pEdit = (CEdit*)GetDlgItem(IDC_SUBJECT_LINE);
	pEdit->SetWindowText(m_SendMailConfiguration.m_sSubject);

	// Server Name
	pEdit = (CEdit*)GetDlgItem(IDC_HOST_NAME);
	pEdit->SetWindowText(m_SendMailConfiguration.m_sHost);

	// Server Port
	pEdit = (CEdit*)GetDlgItem(IDC_HOST_PORT);
	s.Format(_T("%i"), m_SendMailConfiguration.m_nPort);
	pEdit->SetWindowText(s);

	// Username and Password
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->SetWindowText(m_SendMailConfiguration.m_sUsername);
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->SetWindowText(m_SendMailConfiguration.m_sPassword);
	m_cPasswordChar = pEdit->GetPasswordChar();

	// Connection Type
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_CONNECTIONTYPE);
	pComboBox->AddString(ML_STRING(1833, "Plain Text (Port 25 or 587)"));
    pComboBox->AddString(ML_STRING(1834, "SSL/TLS (Port 465)"));	
    pComboBox->AddString(ML_STRING(1835, "STARTTLS (Port 25 or 587)"));
	pComboBox->SetCurSel(m_SendMailConfiguration.m_ConnectionType);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSendMailConfigurationDlg::OnCheckShowPassword()
{
	CEdit* pEditPw = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	CButton* pCheckPw = (CButton*)GetDlgItem(IDC_CHECK_SHOW_PASSWORD);
	if (pCheckPw->GetCheck() > 0)
		pEditPw->SetPasswordChar(0);
	else
		pEditPw->SetPasswordChar(m_cPasswordChar);
	pEditPw->Invalidate();
}

void CSendMailConfigurationDlg::OnChangeEditSenderMail() 
{
	// Get sender mail
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_SENDER_MAIL);
	CString sSenderMail;
	pEdit->GetWindowText(sSenderMail);

	// Check for common sender domains
	// (only servers that support STARTTLS on port 587
	//  and use the email address as username)
	CString sDomain, sKnownSmtpServer;
	int i = sSenderMail.Find(_T('@'));
	if (i >= 0)
	{
		sDomain = sSenderMail.Mid(i + 1);
		if (sDomain == _T("aol.com"))
			sKnownSmtpServer = _T("smtp.aol.com");
		else if (sDomain == _T("att.net"))
			sKnownSmtpServer = _T("outbound.att.net");
		else if (sDomain == _T("comcast.net"))
			sKnownSmtpServer = _T("smtp.comcast.net");
		else if (sDomain == _T("gmail.com"))
			sKnownSmtpServer = _T("smtp.gmail.com");
		else if (sDomain == _T("googlemail.com"))
			sKnownSmtpServer = _T("smtp.googlemail.com");
		else if (sDomain == _T("gmx.net") || sDomain == _T("gmx.de") || sDomain == _T("gmx.ch") || sDomain == _T("gmx.at") || sDomain == _T("gmx.li"))
			sKnownSmtpServer = _T("mail.gmx.net");
		else if (sDomain == _T("gmx.com") || sDomain == _T("gmx.co.uk") || sDomain == _T("gmx.us") || sDomain == _T("gmx.fr") || sDomain == _T("gmx.es"))
			sKnownSmtpServer = _T("mail.gmx.com");
		else if (sDomain == _T("hotmail.com") || sDomain == _T("live.com") || sDomain == _T("outlook.com"))
			sKnownSmtpServer = _T("smtp-mail.outlook.com");
		else if (sDomain == _T("icloud.com") || sDomain == _T("me.com") || sDomain == _T("mac.com"))
			sKnownSmtpServer = _T("smtp.mail.me.com");
		else if (sDomain == _T("libero.it"))
			sKnownSmtpServer = _T("smtp.libero.it");
		else if (sDomain == _T("mail.com"))
			sKnownSmtpServer = _T("smtp.mail.com");
		else if (sDomain == _T("office365.com"))
			sKnownSmtpServer = _T("smtp.office365.com");
		else if (sDomain == _T("yahoo.com"))
			sKnownSmtpServer = _T("smtp.mail.yahoo.com");
		else if (sDomain == _T("yandex.com"))
			sKnownSmtpServer = _T("smtp.yandex.com");
		else if (sDomain == _T("yandex.ru"))
			sKnownSmtpServer = _T("smtp.yandex.ru");
		else if (sDomain == _T("zoho.com"))
			sKnownSmtpServer = _T("smtp.zoho.com");
	}

	// Found known smtp server?
	if (!sKnownSmtpServer.IsEmpty())
	{
		// Get current smtp server
		CString sCurrentSmtpServer;
		pEdit = (CEdit*)GetDlgItem(IDC_HOST_NAME);
		pEdit->GetWindowText(sCurrentSmtpServer);

		// Only update if empty!
		if (sCurrentSmtpServer.IsEmpty())
		{
			// Smtp hostname
			pEdit->SetWindowText(sKnownSmtpServer);

			// Host port
			pEdit = (CEdit*)GetDlgItem(IDC_HOST_PORT);
			pEdit->SetWindowText(_T("587"));

			// Username
			pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
			pEdit->SetWindowText(sSenderMail);

			// Connection type
			CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_CONNECTIONTYPE);
			pComboBox->SetCurSel(STARTTLS);
		}
	}
}

void CSendMailConfigurationDlg::CopyToStruct()
{
	int nNum;
	CString sText;

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_RECEIVER_MAIL);
	pEdit->GetWindowText(sText);
	sText.Trim();
	m_SendMailConfiguration.m_sTo = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_SENDER_NAME);
	pEdit->GetWindowText(sText);
	m_SendMailConfiguration.m_sFromName = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_SENDER_MAIL);
	pEdit->GetWindowText(sText);
	sText.Trim();
	m_SendMailConfiguration.m_sFrom = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_SUBJECT_LINE);
	pEdit->GetWindowText(sText);
	m_SendMailConfiguration.m_sSubject = sText;
	if (m_SendMailConfiguration.m_sSubject.IsEmpty())
		m_SendMailConfiguration.m_sSubject = DEFAULT_EMAIL_SUBJECT;

	pEdit = (CEdit*)GetDlgItem(IDC_HOST_NAME);
	pEdit->GetWindowText(sText);
	sText.Trim();
	m_SendMailConfiguration.m_sHost = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_HOST_PORT);
	pEdit->GetWindowText(sText);
	nNum = _tcstol(sText.GetBuffer(0), NULL, 10);
	sText.ReleaseBuffer();
	if (nNum > 0 && nNum <= 65535) // Port 0 is Reserved
		m_SendMailConfiguration.m_nPort = nNum;
	else
		m_SendMailConfiguration.m_nPort = 587;

	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->GetWindowText(sText);
	m_SendMailConfiguration.m_sUsername = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->GetWindowText(sText);
	m_SendMailConfiguration.m_sPassword = sText;

	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_CONNECTIONTYPE);
	m_SendMailConfiguration.m_ConnectionType = (ConnectionType)pComboBox->GetCurSel();
}

BOOL CSendMailConfigurationDlg::IsEmail(const CString& sEmail)
{
	// Min. lenght of 3 chars
	if (sEmail.GetLength() < 3)
		return FALSE;

	// Quite all chars are allowed in the local part
	// (also @ in double quotes), so we must reverse search @
	int nLocalPartLength = sEmail.ReverseFind(_T('@'));
	if (nLocalPartLength <= 0)
		return FALSE;
	int nDomainLength = sEmail.GetLength() - nLocalPartLength - 1;
	if (nDomainLength <= 0)
		return FALSE;

	return TRUE;
}

BOOL CSendMailConfigurationDlg::ValidateEmailsAndHost()
{
	if (!IsEmail(m_SendMailConfiguration.m_sTo))
	{
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_RECEIVER_MAIL);
		pEdit->SetFocus();
		pEdit->SetSel(0xFFFF0000);
		::AlertUser(GetSafeHwnd());
		return FALSE;
	}
	if (!IsEmail(m_SendMailConfiguration.m_sFrom))
	{
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_SENDER_MAIL);
		pEdit->SetFocus();
		pEdit->SetSel(0xFFFF0000);
		::AlertUser(GetSafeHwnd());
		return FALSE;
	}
	if (m_SendMailConfiguration.m_sHost.IsEmpty())
	{
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_HOST_NAME);
		pEdit->SetFocus();
		pEdit->SetSel(0xFFFF0000);
		::AlertUser(GetSafeHwnd());
		return FALSE;
	}

	return TRUE;
}

void CSendMailConfigurationDlg::OnOK() 
{
	CopyToStruct();
	CDialog::OnOK();
}

void CSendMailConfigurationDlg::OnButtonTest() 
{
	CopyToStruct();
	if (ValidateEmailsAndHost())
	{
		CVideoDeviceDoc::SendMail(	m_SendMailConfiguration,
									m_sName,
									CTime::GetCurrentTime(),
									_T("TEST"),
									_T(""),
									_T(""),
									MAILPROG_TEST_TIMEOUT_SEC,
									TRUE);
	}
}

#endif
