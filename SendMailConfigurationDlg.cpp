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

CSendMailConfigurationDlg::CSendMailConfigurationDlg(const CString& sName)
	: CDialog(CSendMailConfigurationDlg::IDD, NULL)
{
	//{{AFX_DATA_INIT(CSendMailConfigurationDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_sName = sName;
	m_hMailer = NULL;
	m_nRetryTimeMs = 0;
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

	CString s;

	// To Email
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_RECEIVER_MAIL);
	pEdit->SetWindowText(m_SendMailConfiguration.m_sTo);

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
	s.Format(_T("%i"), m_SendMailConfiguration.m_nPort);
	pEdit->SetWindowText(s);

	// Username and Password
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->SetWindowText(m_SendMailConfiguration.m_sUsername);
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->SetWindowText(m_SendMailConfiguration.m_sPassword);

	// Connection Type
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_CONNECTIONTYPE);
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
	int nNum;
	CString sText;

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_RECEIVER_MAIL);
	pEdit->GetWindowText(sText);
	m_SendMailConfiguration.m_sTo = sText;

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
	nNum = _tcstol(sText.GetBuffer(0), NULL, 10);
	sText.ReleaseBuffer();
	if (nNum > 0 && nNum <= 65535) // Port 0 is Reserved
		m_SendMailConfiguration.m_nPort = nNum;
	else
		m_SendMailConfiguration.m_nPort = 25;

	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->GetWindowText(sText);
	m_SendMailConfiguration.m_sUsername = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->GetWindowText(sText);
	m_SendMailConfiguration.m_sPassword = sText;

	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_CONNECTIONTYPE);
	m_SendMailConfiguration.m_ConnectionType = (ConnectionType)pComboBox->GetCurSel();
}

void CSendMailConfigurationDlg::OnOK() 
{
	CopyToStruct();
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
		m_hMailer = CVideoDeviceDoc::SendMail(m_SendMailConfiguration, m_sName, CTime::GetCurrentTime(), _T(""), TRUE, &m_sLogFileName);
		if (!m_hMailer)
		{
			EndWaitCursor();
			SendMessage(WM_SETCURSOR, (WPARAM)GetSafeHwnd(), MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
		}
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
