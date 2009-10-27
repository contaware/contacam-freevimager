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


CSendMailConfigurationDlg::CSendMailConfigurationDlg(CVideoDeviceDoc* pDoc)
	: CDialog(CSendMailConfigurationDlg::IDD, NULL)
{
	//{{AFX_DATA_INIT(CSendMailConfigurationDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pDoc = pDoc;
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
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSendMailConfigurationDlg message handlers

BOOL CSendMailConfigurationDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_RECEIVER_MAIL);
	pEdit->SetWindowText(m_pDoc->m_MovDetSendMailConfiguration.m_sTo);

	CButton* pCheck = (CButton*)GetDlgItem(IDC_HTML);
	pCheck->SetCheck(m_pDoc->m_MovDetSendMailConfiguration.m_bHTML ? 1 : 0);

	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_ATTACHMENT);
	// Add the attachment possibilities
    pComboBox->AddString(_T("None"));
    pComboBox->AddString(_T("AVI Detection"));
    pComboBox->AddString(_T("Animated GIF Detection"));
    pComboBox->AddString(_T("AVI + Animated GIF Detections"));
	pComboBox->SetCurSel((int)m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType);

	pEdit = (CEdit*)GetDlgItem(IDC_SENDER_NAME);
	pEdit->SetWindowText(m_pDoc->m_MovDetSendMailConfiguration.m_sFromName);

	pEdit = (CEdit*)GetDlgItem(IDC_SENDER_MAIL);
	pEdit->SetWindowText(m_pDoc->m_MovDetSendMailConfiguration.m_sFrom);

	pEdit = (CEdit*)GetDlgItem(IDC_HOST_NAME);
	pEdit->SetWindowText(m_pDoc->m_MovDetSendMailConfiguration.m_sHost);

	pEdit = (CEdit*)GetDlgItem(IDC_HOST_PORT);
	CString sPort;
	sPort.Format(_T("%i"), m_pDoc->m_MovDetSendMailConfiguration.m_nPort);
	pEdit->SetWindowText(sPort);

	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->SetWindowText(m_pDoc->m_MovDetSendMailConfiguration.m_sUsername);

	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->SetWindowText(m_pDoc->m_MovDetSendMailConfiguration.m_sPassword);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSendMailConfigurationDlg::CopyToDoc()
{
	CString sText;

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_RECEIVER_MAIL);
	pEdit->GetWindowText(sText);
	m_pDoc->m_MovDetSendMailConfiguration.m_sTo = sText;

	CButton* pCheck = (CButton*)GetDlgItem(IDC_HTML);
	m_pDoc->m_MovDetSendMailConfiguration.m_bHTML = pCheck->GetCheck();

	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_ATTACHMENT);
	m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType =
			(CVideoDeviceDoc::AttachmentType)pComboBox->GetCurSel();

	pEdit = (CEdit*)GetDlgItem(IDC_SENDER_NAME);
	pEdit->GetWindowText(sText);
	m_pDoc->m_MovDetSendMailConfiguration.m_sFromName = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_SENDER_MAIL);
	pEdit->GetWindowText(sText);
	m_pDoc->m_MovDetSendMailConfiguration.m_sFrom = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_HOST_NAME);
	pEdit->GetWindowText(sText);
	m_pDoc->m_MovDetSendMailConfiguration.m_sHost = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_HOST_PORT);
	pEdit->GetWindowText(sText);
	int nPort = _tcstol(sText.GetBuffer(0), NULL, 10);
	sText.ReleaseBuffer();
	if (nPort > 0 && nPort <= 65535) // Port 0 is Reserved
		m_pDoc->m_MovDetSendMailConfiguration.m_nPort = nPort;
	else
		m_pDoc->m_MovDetSendMailConfiguration.m_nPort = 25;

	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->GetWindowText(sText);
	m_pDoc->m_MovDetSendMailConfiguration.m_sUsername = sText;

	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->GetWindowText(sText);
	m_pDoc->m_MovDetSendMailConfiguration.m_sPassword = sText;
}

void CSendMailConfigurationDlg::OnOK() 
{
	CopyToDoc();
	CDialog::OnOK();
}

void CSendMailConfigurationDlg::OnButtonTest() 
{
	CopyToDoc();

	int res = 0;

	if (m_pDoc->m_MovDetSendMailConfiguration.m_sHost.IsEmpty()||
		m_pDoc->m_MovDetSendMailConfiguration.m_sFrom.IsEmpty() ||
		m_pDoc->m_MovDetSendMailConfiguration.m_sTo.IsEmpty()) 
		::AfxMessageBox(ML_STRING(1406, "Please Enter A Host Name, a From and a To Address"));
	else 
	{
		BeginWaitCursor();

		CPJNSMTPMessage* pMessage = NULL;
		try
		{
			// Subject
			CTime Time = CTime::GetCurrentTime();
			CString sSubject(Time.Format(_T("Movement Detection on %A, %d %B %Y at %H:%M:%S")));
			m_pDoc->m_MovDetSendMailConfiguration.m_sSubject = sSubject;

			if (m_pDoc->m_MovDetSendMailConfiguration.m_bHTML == FALSE)
			{
				m_pDoc->m_MovDetSendMailConfiguration.m_bMime = FALSE;
				m_pDoc->m_MovDetSendMailConfiguration.m_sBody = _T("Movement Detection Test Email");

				// No Attachment(s)
				m_pDoc->m_MovDetSendMailConfiguration.m_sFiles = _T("");

				// Create the message
				pMessage = m_pDoc->CreateEmailMessage();
			}
			else
			{
				m_pDoc->m_MovDetSendMailConfiguration.m_bMime = TRUE;
				m_pDoc->m_MovDetSendMailConfiguration.m_sFiles = _T("");
				m_pDoc->m_MovDetSendMailConfiguration.m_sBody = _T("");

				// Create the message
				pMessage = m_pDoc->CreateEmailMessage();

				for (int i = 0 ; i < pMessage->GetNumberOfBodyParts() ; i++)
					pMessage->RemoveBodyPart(i);

				// Setup all the body parts we want
				CPJNSMTPBodyPart related;
				related.SetContentType(_T("multipart/related"));

				CPJNSMTPBodyPart html;
				html.SetText(_T("<p><b>Movement Detection Test Email</b></p>"));
				html.SetContentType(_T("text/html"));

				related.AddChildBodyPart(html);
				pMessage->AddBodyPart(related);
				pMessage->GetBodyPart(0)->SetContentLocation(_T("http://localhost"));
			}

			// Init Connection class
			CVideoDeviceDoc::CSaveFrameListSMTPConnection connection;

			// Auto connect to the internet?
			if (m_pDoc->m_MovDetSendMailConfiguration.m_bAutoDial)
				connection.ConnectToInternet();

			CString sHost;
			BOOL bSend = TRUE;
			if (m_pDoc->m_MovDetSendMailConfiguration.m_bDNSLookup)
			{
				if (pMessage->GetNumberOfRecipients() == 0)
				{
					EndWaitCursor();
					CString sMsg(ML_STRING(1410, "At least one recipient must be specified to use the DNS lookup option\n"));
					TRACE(sMsg);
					::AfxMessageBox(sMsg);
					bSend = FALSE;
				}
				else
				{
					CString sAddress = pMessage->GetRecipient(0)->m_sEmailAddress;
					int nAmpersand = sAddress.Find(_T("@"));
					if (nAmpersand == -1)
					{
						EndWaitCursor();
						CString sMsg;
						sMsg.Format(ML_STRING(1411, "Unable to determine the domain for the email address %s\n"), sAddress);
						TRACE(sMsg);
						::AfxMessageBox(sMsg);
						bSend = FALSE;
					}
					else
					{
						// We just pick the first MX record found, other implementations could ask the user
						// or automatically pick the lowest priority record
						CString sDomain(sAddress.Right(sAddress.GetLength() - nAmpersand - 1));
						CStringArray servers;
						CWordArray priorities;
						if (!connection.MXLookup(sDomain, servers, priorities))
						{
							EndWaitCursor();
							CString sMsg;
							sMsg.Format(ML_STRING(1413, "Unable to perform a DNS MX lookup for the domain %s, Error Code:%d\n"), sDomain, GetLastError());
							TRACE(sMsg);
							::AfxMessageBox(sMsg);
							bSend = FALSE;
						}
						else
							sHost = servers.GetAt(0);
					}
				}
			}
			else
				sHost = m_pDoc->m_MovDetSendMailConfiguration.m_sHost;

			// Connect and send the message
			if (bSend)
			{
				connection.SetBoundAddress(m_pDoc->m_MovDetSendMailConfiguration.m_sBoundIP);
				connection.Connect(	sHost,
									m_pDoc->m_MovDetSendMailConfiguration.m_Auth,
									m_pDoc->m_MovDetSendMailConfiguration.m_sUsername,
									m_pDoc->m_MovDetSendMailConfiguration.m_sPassword,
									m_pDoc->m_MovDetSendMailConfiguration.m_nPort);
				connection.SendMessage(*pMessage);
				EndWaitCursor();
				::AfxMessageBox(ML_STRING(1409, "Success: Email Sent."), MB_ICONINFORMATION);
			}

			// Auto disconnect from the internet
			if (m_pDoc->m_MovDetSendMailConfiguration.m_bAutoDial)
				connection.CloseInternetConnection();

			// Sending Interrupted?
			if (connection.m_bDoExit)
			{
				connection.Disconnect(FALSE);	// Disconnect no Gracefully,
												// otherwise the thread blocks
												// long time to get a answer!
				res = -1;
			}
			else
				res = 1;

			// Clean-up
			if (pMessage)
				delete pMessage;
		}
		catch (CPJNSMTPException* pEx)
		{
			// Clean-up
			if (pMessage)
				delete pMessage;

			// Display the error
			EndWaitCursor();
			CString sMsg;
			sMsg.Format(ML_STRING(1414, "An error occured sending the message, Error:%x\nDescription:%s\n"),
						pEx->m_hr,
						pEx->GetErrorMessage());
			TRACE(sMsg);
			::AfxMessageBox(sMsg, MB_ICONSTOP);
			pEx->Delete();
		}
	}
}

#endif
