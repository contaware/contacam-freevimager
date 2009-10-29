// NetworkPage.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "NetworkPage.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "NetFrameHdr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CNetworkPage property page

IMPLEMENT_DYNCREATE(CNetworkPage, CPropertyPage)

CNetworkPage::CNetworkPage()
	: CPropertyPage(CNetworkPage::IDD)
{
	// OnInitDialog() is called when first pressing the tab
	// OnInitDialog() inits the property page pointer in the doc
	// -> Move all inits to OnInitDialog() because the assistant
	// may change the doc vars between construction and OnInitDialog() call
	//{{AFX_DATA_INIT(CNetworkPage)
	//}}AFX_DATA_INIT
	m_pDoc = NULL;
}

void CNetworkPage::SetDoc(CVideoDeviceDoc* pDoc)
{
	ASSERT(pDoc);
	m_pDoc = pDoc;
}

CNetworkPage::~CNetworkPage()
{
}

void CNetworkPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNetworkPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNetworkPage, CPropertyPage)
	//{{AFX_MSG_MAP(CNetworkPage)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_VIDEO_PORT_ENABLED, OnVideoPortEnabled)
	ON_EN_CHANGE(IDC_EDIT_PORT, OnChangeEditPort)
	ON_EN_CHANGE(IDC_EDIT_MTU, OnChangeEditMtu)
	ON_EN_CHANGE(IDC_EDIT_DATARATE, OnChangeEditDatarate)
	ON_CBN_SELCHANGE(IDC_COMBO_SIZE, OnSelchangeComboSize)
	ON_EN_CHANGE(IDC_EDIT_FREQDIV, OnChangeEditFreqdiv)
	ON_EN_CHANGE(IDC_EDIT_CONNECTIONS, OnChangeEditConnections)
	ON_EN_CHANGE(IDC_AUTH_USERNAME, OnChangeAuthUsername)
	ON_EN_CHANGE(IDC_AUTH_PASSWORD, OnChangeAuthPassword)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNetworkPage message handlers

BOOL CNetworkPage::OnInitDialog() 
{
	// Init Combo Box
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SIZE);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1555, "Full"));
		pComboBox->AddString(_T("1/2"));
		pComboBox->AddString(_T("1/4"));
		pComboBox->AddString(_T("1/8"));
	}

	// This calls UpdateData(FALSE)
	CPropertyPage::OnInitDialog();

	// Networking Messages
	m_pDoc->ShowSendFrameMsg();

	// Video Send Enable Check Box
	CButton* pCheckVideoPortEnable = (CButton*)GetDlgItem(IDC_VIDEO_PORT_ENABLED);
	pCheckVideoPortEnable->SetCheck(m_pDoc->m_bSendVideoFrame ? 1 : 0);
	
	// Video Send Port
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
	CString sPort;
	sPort.Format(_T("%i"), m_pDoc->m_nSendFrameVideoPort);
	pEdit->SetWindowText(sPort);
	pEdit->EnableWindow(!m_pDoc->m_bSendVideoFrame);

	// Max Connections
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONNECTIONS);
	CString sMaxConnections;
	sMaxConnections.Format(_T("%i"), m_pDoc->m_nSendFrameMaxConnections);
	pEdit->SetWindowText(sMaxConnections);
	pEdit->EnableWindow(!m_pDoc->m_bSendVideoFrame);

	// Mtu
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_MTU);
	CString sMtu;
	sMtu.Format(_T("%i"), m_pDoc->m_nSendFrameMTU);
	pEdit->SetWindowText(sMtu);

	// Data Rate
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DATARATE);
	CString sDataRate;
	sDataRate.Format(_T("%i"), m_pDoc->m_nSendFrameDataRate / 1000); // bits / sec -> kbps
	pEdit->SetWindowText(sDataRate);

	// Frequency Divider
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_FREQDIV);
	CString sFreqDiv;
	sFreqDiv.Format(_T("%i"), m_pDoc->m_nSendFrameFreqDiv);
	pEdit->SetWindowText(sFreqDiv);

	// Size Divider
	if (pComboBox)
		pComboBox->SetCurSel(m_pDoc->m_nSendFrameSizeDiv);

	// Username and Password
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->SetWindowText(m_pDoc->m_sSendFrameUsername);
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->SetWindowText(m_pDoc->m_sSendFramePassword);

	// Disable Critical Controls?
	if (m_pDoc->GetView()->AreCriticalControlsDisabled())
		EnableDisableCriticalControls(FALSE);

	// Set Page Pointer to this
	m_pDoc->m_pNetworkPage = this;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNetworkPage::OnDestroy() 
{
	CPropertyPage::OnDestroy();
	// Set Page Pointer to NULL
	m_pDoc->m_pNetworkPage = NULL;
}

void CNetworkPage::EnableDisableCriticalControls(BOOL bEnable)
{
	// Enable Check Box?
	CButton* pCheck = (CButton*)GetDlgItem(IDC_VIDEO_PORT_ENABLED);
	pCheck->EnableWindow(bEnable);
}

void CNetworkPage::OnVideoPortEnabled() 
{
	if (!m_pDoc->m_bSendVideoFrame)
	{
		::EnterCriticalSection(&m_pDoc->m_csSendFrameNetCom);

		// Free
		if (m_pDoc->m_pSendFrameNetCom)
		{
			delete m_pDoc->m_pSendFrameNetCom;
			m_pDoc->m_pSendFrameNetCom = NULL;
		}
		m_pDoc->m_pSendFrameParseProcess->FreeAVCodec();

		// Clear Table
		if (m_pDoc->m_pSendFrameParseProcess)
			m_pDoc->m_pSendFrameParseProcess->ClearTable();

		// Free Re-Send List
		m_pDoc->ClearReSendUDPFrameList();

		// Reset Vars
		m_pDoc->m_dwMaxSendFrameFragmentsPerFrame = 0U;
		m_pDoc->m_dwSendFrameTotalSentBytes = 0U;
		m_pDoc->m_dwSendFrameTotalLastSentBytes = 0U;
		m_pDoc->m_dwSendFrameOverallDatarate = 0U;
		m_pDoc->m_dwSendFrameDatarateCorrection = 0U;

		// Listen
		CNetCom* pNetCom = (CNetCom*)new CNetCom;
		if (m_pDoc->ConnectSendFrameUDP(pNetCom, m_pDoc->m_nSendFrameVideoPort))
		{
			m_pDoc->m_pSendFrameNetCom = pNetCom;
			m_pDoc->m_bSendVideoFrame = TRUE; // Set Flag
		}
		else
			delete pNetCom;

		::LeaveCriticalSection(&m_pDoc->m_csSendFrameNetCom);
	}
	else
	{
		::EnterCriticalSection(&m_pDoc->m_csSendFrameNetCom);
		
		// Reset Flag
		m_pDoc->m_bSendVideoFrame = FALSE;

		// Free
		if (m_pDoc->m_pSendFrameNetCom)
		{
			delete m_pDoc->m_pSendFrameNetCom;
			m_pDoc->m_pSendFrameNetCom = NULL;
		}
		m_pDoc->m_pSendFrameParseProcess->FreeAVCodec();

		// Clear Table
		if (m_pDoc->m_pSendFrameParseProcess)
			m_pDoc->m_pSendFrameParseProcess->ClearTable();

		// Free Re-Send List
		m_pDoc->ClearReSendUDPFrameList();

		// Reset Vars
		m_pDoc->m_dwMaxSendFrameFragmentsPerFrame = 0U;
		m_pDoc->m_dwSendFrameTotalSentBytes = 0U;
		m_pDoc->m_dwSendFrameTotalLastSentBytes = 0U;
		m_pDoc->m_dwSendFrameOverallDatarate = 0U;
		m_pDoc->m_dwSendFrameDatarateCorrection = 0U;

		// Clear Msg
		m_pDoc->m_sSendFrameMsg = _T("");
		m_pDoc->ShowSendFrameMsg();

		::LeaveCriticalSection(&m_pDoc->m_csSendFrameNetCom);
	}

	// Update Ctrls
	CButton* pCheckVideoPortEnable = (CButton*)GetDlgItem(IDC_VIDEO_PORT_ENABLED);
	pCheckVideoPortEnable->SetCheck(m_pDoc->m_bSendVideoFrame ? 1 : 0);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
	pEdit->EnableWindow(!m_pDoc->m_bSendVideoFrame);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONNECTIONS);
	pEdit->EnableWindow(!m_pDoc->m_bSendVideoFrame);
}

void CNetworkPage::OnChangeEditPort() 
{
	CString sText;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
	pEdit->GetWindowText(sText);
	int nPort = _tcstol(sText.GetBuffer(0), NULL, 10);
	sText.ReleaseBuffer();
	if (nPort > 0 && nPort <= 65535) // Port 0 is Reserved
		m_pDoc->m_nSendFrameVideoPort = nPort;
	else
		m_pDoc->m_nSendFrameVideoPort = DEFAULT_UDP_PORT;
}

void CNetworkPage::OnChangeEditConnections() 
{
	CString sText;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONNECTIONS);
	pEdit->GetWindowText(sText);
	int nConnections = _tcstol(sText.GetBuffer(0), NULL, 10);
	sText.ReleaseBuffer();
	if (nConnections >= SENDFRAME_MIN_CONNECTIONS && nConnections <= SENDFRAME_MAX_CONNECTIONS)
		m_pDoc->m_nSendFrameMaxConnections = nConnections;
	else
		m_pDoc->m_nSendFrameMaxConnections = DEFAULT_SENDFRAME_CONNECTIONS;
}

void CNetworkPage::OnChangeEditMtu() 
{
	CString sText;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_MTU);
	pEdit->GetWindowText(sText);
	int nMTU = _tcstol(sText.GetBuffer(0), NULL, 10);
	sText.ReleaseBuffer();
	if (nMTU < SENDFRAME_MIN_FRAGMENT_SIZE)
		nMTU = SENDFRAME_MIN_FRAGMENT_SIZE;
	else if (nMTU > SENDFRAME_MAX_FRAGMENT_SIZE)
		nMTU = SENDFRAME_MAX_FRAGMENT_SIZE;
	m_pDoc->m_nSendFrameMTU = nMTU;
	m_pDoc->m_dwMaxSendFrameFragmentsPerFrame = 0U;
	m_pDoc->m_dwSendFrameDatarateCorrection = 0U;
}

void CNetworkPage::OnChangeEditDatarate() 
{
	CString sText;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DATARATE);
	pEdit->GetWindowText(sText);
	int nDataRate = _tcstol(sText.GetBuffer(0), NULL, 10);
	sText.ReleaseBuffer();
	if (nDataRate < (SENDFRAME_MIN_DATARATE / 1000))
		nDataRate = SENDFRAME_MIN_DATARATE / 1000;
	else if (nDataRate > (SENDFRAME_MAX_DATARATE / 1000))
		nDataRate = SENDFRAME_MAX_DATARATE / 1000;
	m_pDoc->m_nSendFrameDataRate = nDataRate * 1000; // kbps -> bits / sec
	m_pDoc->m_dwMaxSendFrameFragmentsPerFrame = 0U;
	m_pDoc->m_dwSendFrameDatarateCorrection = 0U;
}

void CNetworkPage::OnChangeEditFreqdiv() 
{
	CString sText;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_FREQDIV);
	pEdit->GetWindowText(sText);
	int nFreqDiv = _tcstol(sText.GetBuffer(0), NULL, 10);
	sText.ReleaseBuffer();
	if (nFreqDiv < SENDFRAME_MIN_FREQDIV)
		nFreqDiv = SENDFRAME_MIN_FREQDIV;
	else if (nFreqDiv > SENDFRAME_MAX_FREQDIV)
		nFreqDiv = SENDFRAME_MAX_FREQDIV;
	m_pDoc->m_nSendFrameFreqDiv = nFreqDiv;
	m_pDoc->m_dwSendFrameDatarateCorrection = 0U;
}

void CNetworkPage::OnSelchangeComboSize() 
{
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_SIZE);
	m_pDoc->m_nSendFrameSizeDiv = pComboBox->GetCurSel();
	m_pDoc->m_dwMaxSendFrameFragmentsPerFrame = 0U;
	m_pDoc->m_dwSendFrameDatarateCorrection = 0U;
}

void CNetworkPage::OnChangeAuthUsername() 
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->GetWindowText(m_pDoc->m_sSendFrameUsername);
}

void CNetworkPage::OnChangeAuthPassword() 
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->GetWindowText(m_pDoc->m_sSendFramePassword);
}

#endif
