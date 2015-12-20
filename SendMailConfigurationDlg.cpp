// SendMailConfigurationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "Helpers.h"
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

CSendMailConfigurationDlg::CSendMailConfigurationDlg(CVideoDeviceDoc* pDoc)
	: CDialog(CSendMailConfigurationDlg::IDD, NULL)
{
	//{{AFX_DATA_INIT(CSendMailConfigurationDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	ASSERT_VALID(pDoc);
	m_pDoc = pDoc;
	m_hMailer = NULL;
	m_nRetryTimeMs = 0;
	m_SendMailConfiguration = pDoc->m_MovDetSendMailConfiguration;
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
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSendMailConfigurationDlg message handlers

BOOL CSendMailConfigurationDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// To Email
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_RECEIVER_MAIL);
	pEdit->SetWindowText(m_SendMailConfiguration.m_sTo);

	// Attachment possibilities
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_ATTACHMENT);
    pComboBox->AddString(ML_STRING(1880, "None"));
    pComboBox->AddString(ML_STRING(1883, "Full Video"));
    pComboBox->AddString(ML_STRING(1882, "Small Video"));
    pComboBox->AddString(ML_STRING(1881, "Snapshots"));
	pComboBox->SetCurSel((int)m_SendMailConfiguration.m_AttachmentType);

	// From Name
	pEdit = (CEdit*)GetDlgItem(IDC_SENDER_NAME);
	pEdit->SetWindowText(m_SendMailConfiguration.m_sFromName);

	// From Email
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
	CString sPort;
	sPort.Format(_T("%i"), m_SendMailConfiguration.m_nPort);
	pEdit->SetWindowText(sPort);

	// Username and Password
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->SetWindowText(m_SendMailConfiguration.m_sUsername);
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->SetWindowText(m_SendMailConfiguration.m_sPassword);

	// Connection Type
	pComboBox = (CComboBox*)GetDlgItem(IDC_CONNECTIONTYPE);
	pComboBox->AddString(ML_STRING(1833, "Plain Text (Port 25 or 587)"));
    pComboBox->AddString(ML_STRING(1834, "SSL/TLS (Port 465)"));	
    pComboBox->AddString(ML_STRING(1835, "STARTTLS (Port 25 or 587)"));
	pComboBox->SetCurSel(m_SendMailConfiguration.m_ConnectionType);

	// Set Timer
	SetTimer(ID_TIMER_SENDMAILCONFIGURATIONDLG, SENDMAILCONFIGURATIONDLG_TIMER_MS, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSendMailConfigurationDlg::CopyToStruct()
{
	CString sText;

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_RECEIVER_MAIL);
	pEdit->GetWindowText(sText);
	m_SendMailConfiguration.m_sTo = sText;

	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_ATTACHMENT);
	m_SendMailConfiguration.m_AttachmentType =
			(CVideoDeviceDoc::AttachmentType)pComboBox->GetCurSel();

	pEdit = (CEdit*)GetDlgItem(IDC_SENDER_NAME);
	pEdit->GetWindowText(sText);
	m_SendMailConfiguration.m_sFromName = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_SENDER_MAIL);
	pEdit->GetWindowText(sText);
	m_SendMailConfiguration.m_sFrom = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_SUBJECT_LINE);
	pEdit->GetWindowText(sText);
	m_SendMailConfiguration.m_sSubject = sText;
	if (m_SendMailConfiguration.m_sSubject.IsEmpty())
		m_SendMailConfiguration.m_sSubject = MOVDET_DEFAULT_EMAIL_SUBJECT;

	pEdit = (CEdit*)GetDlgItem(IDC_HOST_NAME);
	pEdit->GetWindowText(sText);
	m_SendMailConfiguration.m_sHost = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_HOST_PORT);
	pEdit->GetWindowText(sText);
	int nPort = _tcstol(sText.GetBuffer(0), NULL, 10);
	sText.ReleaseBuffer();
	if (nPort > 0 && nPort <= 65535) // Port 0 is Reserved
		m_SendMailConfiguration.m_nPort = nPort;
	else
		m_SendMailConfiguration.m_nPort = 25;

	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->GetWindowText(sText);
	m_SendMailConfiguration.m_sUsername = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->GetWindowText(sText);
	m_SendMailConfiguration.m_sPassword = sText;

	pComboBox = (CComboBox*)GetDlgItem(IDC_CONNECTIONTYPE);
	m_SendMailConfiguration.m_ConnectionType =
			(CVideoDeviceDoc::ConnectionType)pComboBox->GetCurSel();
}

void CSendMailConfigurationDlg::OnOK() 
{
	CopyToStruct();
	m_pDoc->m_MovDetSendMailConfiguration = m_SendMailConfiguration;
	CDialog::OnOK();
}

void CSendMailConfigurationDlg::OnButtonTest() 
{
	CopyToStruct();
	if (m_SendMailConfiguration.m_sHost.IsEmpty()	||
		m_SendMailConfiguration.m_sFrom.IsEmpty()	||
		m_SendMailConfiguration.m_sTo.IsEmpty()) 
		::AfxMessageBox(ML_STRING(1406, "Please Enter A Host Name, a From and a To Address"));
	else 
	{
		// Reset
		if (m_hMailer)
		{
			::CloseHandle(m_hMailer);
			m_hMailer = NULL;
			EndWaitCursor();
		}
		m_nRetryTimeMs = 0;

		// Begin Wait Cursor
		BeginWaitCursor();
		SendMessage(WM_SETCURSOR, (WPARAM)GetSafeHwnd(), MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));

		// Mail
		CString sOptions;
		CString sConnectionTypeOption;
		CString sSubject = m_SendMailConfiguration.m_sSubject;
		CTime CurrentTime(CTime::GetCurrentTime());
		sSubject.Replace(_T("%name%"), m_pDoc->GetAssignedDeviceName());
		sSubject.Replace(_T("%date%"), ::MakeDateLocalFormat(CurrentTime));
		sSubject.Replace(_T("%time%"), ::MakeTimeLocalFormat(CurrentTime, TRUE));
		switch (m_SendMailConfiguration.m_ConnectionType)
		{
			case 0 : sConnectionTypeOption = _T(""); break;				// Plain Text
			case 1 : sConnectionTypeOption = _T("-ssl"); break;			// SSL and TLS
			default: sConnectionTypeOption = _T("-starttls"); break;	// STARTTLS
		}
		sOptions.Format(_T("-t \"%s\" -f %s %s %s -port %d %s -smtp %s -cs \"iso-8859-1\" -sub \"%s\" +cc +bc -user \"%s\" -pass \"%s\" -M \"%s\""),
						m_SendMailConfiguration.m_sTo,
						m_SendMailConfiguration.m_sFrom,
						m_SendMailConfiguration.m_sFromName.IsEmpty() ? _T("") : _T("-name \"") + m_SendMailConfiguration.m_sFromName + _T("\""),
						sConnectionTypeOption,
						m_SendMailConfiguration.m_nPort,
						(m_SendMailConfiguration.m_sUsername.IsEmpty() && m_SendMailConfiguration.m_sPassword.IsEmpty()) ? _T("") : _T("-auth"),
						m_SendMailConfiguration.m_sHost,
						sSubject,
						m_SendMailConfiguration.m_sUsername,
						m_SendMailConfiguration.m_sPassword,
						m_pDoc->GetAssignedDeviceName());
		m_hMailer = CVideoDeviceDoc::Mailer(sOptions, TRUE, &m_sLogFileName);
	}
}

void CSendMailConfigurationDlg::OnDestroy()
{
	// Kill timer
	KillTimer(ID_TIMER_SENDMAILCONFIGURATIONDLG);

	// Close
	if (m_hMailer)
	{
		::CloseHandle(m_hMailer);
		m_hMailer = NULL;
		EndWaitCursor();
	}

	// Base class
	CDialog::OnDestroy();
}

void CSendMailConfigurationDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (m_hMailer)
	{
		if (::WaitForSingleObject(m_hMailer, 0) == WAIT_OBJECT_0)
		{
			::CloseHandle(m_hMailer);
			m_hMailer = NULL;
			EndWaitCursor();
			if (::IsExistingFile(m_sLogFileName))
				::ShellExecute(NULL, _T("open"), m_sLogFileName, NULL, NULL, SW_SHOWNORMAL);
		}
		else if (m_nRetryTimeMs > MAILPROG_WAIT_TIMEOUT_MS)
		{
			::CloseHandle(m_hMailer);
			m_hMailer = NULL;
			EndWaitCursor();
		}
		else
			m_nRetryTimeMs += SENDMAILCONFIGURATIONDLG_TIMER_MS;
	}
	CDialog::OnTimer(nIDEvent);
}

BOOL CSendMailConfigurationDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_hMailer)
	{
		SetCursor(::AfxGetApp()->LoadStandardCursor(IDC_WAIT));
		return TRUE;
	}
	else
		return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

#endif
