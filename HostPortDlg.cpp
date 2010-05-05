// HostPortDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "HostPortDlg.h"
#include "NetFrameHdr.h"
#include "VideoDeviceDoc.h"
#include "IniFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CHostPortDlg dialog


CHostPortDlg::CHostPortDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHostPortDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHostPortDlg)
	m_nDeviceTypeMode = 0;
	m_bDisableResend = FALSE;
	//}}AFX_DATA_INIT
	m_sHost = _T("localhost");
	m_nPort = DEFAULT_UDP_PORT;
	m_dwMaxFrames = NETFRAME_DEFAULT_FRAMES;
}


void CHostPortDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHostPortDlg)
	DDX_Control(pDX, IDC_COMBO_BUFSIZE, m_cbBufSize);
	DDX_CBIndex(pDX, IDC_COMBO_DEVICETYPEMODE, m_nDeviceTypeMode);
	DDX_Check(pDX, IDC_CHECK_DISABLE_RESEND, m_bDisableResend);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHostPortDlg, CDialog)
	//{{AFX_MSG_MAP(CHostPortDlg)
	ON_BN_CLICKED(IDC_SEARCH_SERVERS, OnSearchServers)
	ON_CBN_SELCHANGE(IDC_COMBO_HOST, OnSelchangeComboHost)
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICETYPEMODE, OnSelchangeComboDevicetypemode)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_HOST_OK, OnHostOk)
END_MESSAGE_MAP()

BOOL CHostPortDlg::OnInitDialog() 
{
	// Init Combo Box
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_DEVICETYPEMODE);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1547, "Internal UDP Server"));
		pComboBox->AddString(ML_STRING(1548, "Other HTTP Device"));
		pComboBox->AddString(ML_STRING(1549, "Axis (Server Push Mode)"));
		pComboBox->AddString(ML_STRING(1550, "Axis (Client Poll Mode)"));
		pComboBox->AddString(ML_STRING(1551, "Panasonic (Server Push Mode)"));
		pComboBox->AddString(ML_STRING(1552, "Panasonic (Client Poll Mode)"));
		pComboBox->AddString(ML_STRING(1553, "Pixord or NetComm (Server Push Mode)"));
		pComboBox->AddString(ML_STRING(1554, "Pixord or NetComm (Client Poll Mode)"));
		pComboBox->AddString(ML_STRING(1789, "Edimax (Server Push Mode)"));
	}

	CDialog::OnInitDialog();

	// Load Settings
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		LoadSettings();
	
	// Init
	if (m_HostsHistory.GetSize() > 0)
	{
		CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_HOST);
		for (int i = 0 ; i < m_HostsHistory.GetSize() ; i++)
		{
			// Host
			pComboBox->AddString(m_HostsHistory[i]);
			
			// Port, Device Type and Mode
			if (i == 0)
			{
				// Port
				CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
				CString sPort;
				sPort.Format(_T("%i"), m_PortsHistory[0]);
				pEdit->SetWindowText(sPort);

				// Device Type Mode
				CComboBox* pComboBoxDevTypeMode = (CComboBox*)GetDlgItem(IDC_COMBO_DEVICETYPEMODE);
				if (m_DeviceTypeModesHistory[0] >= 0 && (int)m_DeviceTypeModesHistory[0] < pComboBoxDevTypeMode->GetCount())
					pComboBoxDevTypeMode->SetCurSel(m_DeviceTypeModesHistory[0]);
				
				// Max Frames
				CString sMaxFrames;
				sMaxFrames.Format(_T("%u"), m_MaxFramesHistory[0]);
				m_cbBufSize.SelectString(-1, sMaxFrames);
				
				// Re-Send Check Box
				CButton* pCheckDisableResend = (CButton*)GetDlgItem(IDC_CHECK_DISABLE_RESEND);
				pCheckDisableResend->SetCheck(m_DisableResendHistory[0]);

				// Enable / Disable Windows
				m_cbBufSize.EnableWindow(m_DeviceTypeModesHistory[0] == 0);
				CButton* pButton = (CButton*)GetDlgItem(IDC_SEARCH_SERVERS);
				pButton->EnableWindow(m_DeviceTypeModesHistory[0] == 0);
				pCheckDisableResend->EnableWindow(m_DeviceTypeModesHistory[0] == 0);
				pEdit = (CEdit*)GetDlgItem(IDC_LABEL_BUFFERING);
				pEdit->EnableWindow(m_DeviceTypeModesHistory[0] == 0);
				pEdit = (CEdit*)GetDlgItem(IDC_LABEL_FRAMES);
				pEdit->EnableWindow(m_DeviceTypeModesHistory[0] == 0);
			}
		}
		pComboBox->SetCurSel(0);
	}
	else
	{
		// Empty Host
		CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_HOST);
		pComboBox->AddString(_T("localhost"));
		pComboBox->SetCurSel(0);
		
		// Default Tcp Port
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
		CString sPort;
		sPort.Format(_T("%i"), DEFAULT_UDP_PORT);
		pEdit->SetWindowText(sPort);

		// Default Max Frames
		CString sMaxFrames;
		sMaxFrames.Format(_T("%u"), NETFRAME_DEFAULT_FRAMES);
		m_cbBufSize.SelectString(-1, sMaxFrames);

		// Enable / Disable Windows
		m_cbBufSize.EnableWindow(m_nDeviceTypeMode == 0);
		CButton* pButton = (CButton*)GetDlgItem(IDC_SEARCH_SERVERS);
		pButton->EnableWindow(m_nDeviceTypeMode == 0);
		CButton* pCheckDisableResend = (CButton*)GetDlgItem(IDC_CHECK_DISABLE_RESEND);
		pCheckDisableResend->EnableWindow(m_nDeviceTypeMode == 0);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_BUFFERING);
		pEdit->EnableWindow(m_nDeviceTypeMode == 0);
		pEdit = (CEdit*)GetDlgItem(IDC_LABEL_FRAMES);
		pEdit->EnableWindow(m_nDeviceTypeMode == 0);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHostPortDlg::OnOK() 
{
	// Start Wait Cursor
	BeginWaitCursor();

	// Free
	Free();

	// Update Data
	UpdateData(TRUE);

	// Host
	CString sText;
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_HOST);
	pComboBox->GetWindowText(sText);
	m_sHost = sText;
		
	// Port
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
	pEdit->GetWindowText(sText);
	int nPort = _tcstol(sText.GetBuffer(0), NULL, 10);
	sText.ReleaseBuffer();
	if (nPort > 0 && nPort <= 65535) // Port 0 is Reserved
		m_nPort = nPort;
	else
		m_nPort = DEFAULT_UDP_PORT;

	// Max Frames
	CString sMaxFrames;
	m_cbBufSize.GetWindowText(sMaxFrames);
	m_dwMaxFrames = (DWORD)_ttoi(sMaxFrames);

	// Save Settings
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		SaveSettings();

	CDialog::OnOK();
}

void CHostPortDlg::OnCancel() 
{
	Free();
	CDialog::OnCancel();
}

void CHostPortDlg::OnSelchangeComboHost() 
{
	// Port
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_HOST);
	int nSel = pComboBox->GetCurSel();
	if (nSel >= 0 && nSel < m_PortsHistory.GetSize())
	{
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
		CString sPort;
		sPort.Format(_T("%i"), m_PortsHistory[nSel]);
		pEdit->SetWindowText(sPort);
	}

	// Device Type Mode
	CComboBox* pComboBoxDevTypeMode = (CComboBox*)GetDlgItem(IDC_COMBO_DEVICETYPEMODE);
	if (nSel >= 0 && nSel < m_DeviceTypeModesHistory.GetSize())
	{
		if (m_DeviceTypeModesHistory[nSel] >= 0 && (int)m_DeviceTypeModesHistory[nSel] < pComboBoxDevTypeMode->GetCount())
			pComboBoxDevTypeMode->SetCurSel(m_DeviceTypeModesHistory[nSel]);
	}

	// Max Frames
	CString sMaxFrames;
	sMaxFrames.Format(_T("%u"), m_MaxFramesHistory[nSel]);
	m_cbBufSize.SelectString(-1, sMaxFrames);

	// Re-Send Check Box
	CButton* pCheckDisableResend = (CButton*)GetDlgItem(IDC_CHECK_DISABLE_RESEND);
	pCheckDisableResend->SetCheck(m_DisableResendHistory[nSel]);

	UpdateData(TRUE);

	// Enable / Disable Windows
	m_cbBufSize.EnableWindow(m_nDeviceTypeMode == 0);
	CButton* pButton = (CButton*)GetDlgItem(IDC_SEARCH_SERVERS);
	pButton->EnableWindow(m_nDeviceTypeMode == 0);
	pCheckDisableResend->EnableWindow(m_nDeviceTypeMode == 0);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_LABEL_BUFFERING);
	pEdit->EnableWindow(m_nDeviceTypeMode == 0);
	pEdit = (CEdit*)GetDlgItem(IDC_LABEL_FRAMES);
	pEdit->EnableWindow(m_nDeviceTypeMode == 0);
}

void CHostPortDlg::OnSelchangeComboDevicetypemode() 
{
	UpdateData(TRUE);

	// Enable / Disable Windows
	m_cbBufSize.EnableWindow(m_nDeviceTypeMode == 0);
	CButton* pButton = (CButton*)GetDlgItem(IDC_SEARCH_SERVERS);
	pButton->EnableWindow(m_nDeviceTypeMode == 0);
	CButton* pCheckDisableResend = (CButton*)GetDlgItem(IDC_CHECK_DISABLE_RESEND);
	pCheckDisableResend->EnableWindow(m_nDeviceTypeMode == 0);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_LABEL_BUFFERING);
	pEdit->EnableWindow(m_nDeviceTypeMode == 0);
	pEdit = (CEdit*)GetDlgItem(IDC_LABEL_FRAMES);
	pEdit->EnableWindow(m_nDeviceTypeMode == 0);
}

void CHostPortDlg::LoadSettings()
{
	CWinApp* pApp = ::AfxGetApp();
	CString sSection(_T("HostPortDlg"));

	CString sHost;
	DWORD dwPort;
	DWORD dwDeviceTypeMode;
	DWORD dwMaxFrame;
	DWORD dwDisableResend;
	for (int i = 0 ; i < MAX_HOST_PORT_HISTORY_SIZE ; i++)
	{
		// Host
		CString sHostEntry;
		sHostEntry.Format(_T("HostHistory%d"), i);
		sHost = pApp->GetProfileString(sSection, sHostEntry, _T(""));
		if (sHost == _T(""))
			break;
		m_HostsHistory.Add(sHost);

		// Port
		CString sPortEntry;
		sPortEntry.Format(_T("PortHistory%d"), i);
		dwPort = (DWORD) pApp->GetProfileInt(sSection, sPortEntry, 0xFFFFFFFF);
		if (dwPort == 0 || dwPort > 65535) // Port 0 is Reserved
			dwPort = DEFAULT_UDP_PORT;
		m_PortsHistory.Add(dwPort);

		// Device Type and Mode
		CString sDeviceTypeModeEntry;
		sDeviceTypeModeEntry.Format(_T("DeviceTypeModeHistory%d"), i);
		dwDeviceTypeMode = (DWORD) pApp->GetProfileInt(sSection, sDeviceTypeModeEntry, 0);
		m_DeviceTypeModesHistory.Add(dwDeviceTypeMode);

		// Buf Size in Frames
		CString sMaxFrameEntry;
		sMaxFrameEntry.Format(_T("MaxFrameHistory%d"), i);
		dwMaxFrame = (DWORD) pApp->GetProfileInt(sSection, sMaxFrameEntry, NETFRAME_DEFAULT_FRAMES);
		m_MaxFramesHistory.Add(dwMaxFrame);

		// Re-Send Check Box
		CString sDisableResendEntry;
		sDisableResendEntry.Format(_T("DisableResendHistory%d"), i);
		dwDisableResend = (DWORD) pApp->GetProfileInt(sSection, sDisableResendEntry, 0);
		m_DisableResendHistory.Add(dwDisableResend);
	}
}

void CHostPortDlg::SaveSettings()
{
	int i;
	CWinApp* pApp = ::AfxGetApp();
	CString sSection(_T("HostPortDlg"));

	// Remove Duplicate
	for (i = 0 ; i < m_HostsHistory.GetSize() ; i++)
	{
		if (m_HostsHistory[i] == m_sHost)
		{
			m_HostsHistory.RemoveAt(i);
			m_PortsHistory.RemoveAt(i);
			m_DeviceTypeModesHistory.RemoveAt(i);
			m_MaxFramesHistory.RemoveAt(i);
			m_DisableResendHistory.RemoveAt(i);
			break;
		}
	}

	// Insert new ones at the beginning
	m_HostsHistory.InsertAt(0, m_sHost);
	m_PortsHistory.InsertAt(0, (DWORD)m_nPort);
	m_DeviceTypeModesHistory.InsertAt(0, (DWORD)m_nDeviceTypeMode);
	m_MaxFramesHistory.InsertAt(0, m_dwMaxFrames);
	m_DisableResendHistory.InsertAt(0, (DWORD)m_bDisableResend);

	// Shrink to MAX_HOST_PORT_HISTORY_SIZE
	while (m_HostsHistory.GetSize() > MAX_HOST_PORT_HISTORY_SIZE)
	{
		m_HostsHistory.RemoveAt(m_HostsHistory.GetUpperBound());
		m_PortsHistory.RemoveAt(m_PortsHistory.GetUpperBound());
		m_DeviceTypeModesHistory.RemoveAt(m_DeviceTypeModesHistory.GetUpperBound());
		m_MaxFramesHistory.RemoveAt(m_MaxFramesHistory.GetUpperBound());
		m_DisableResendHistory.RemoveAt(m_DisableResendHistory.GetUpperBound());
	}

	// Write to Registry
	if (((CUImagerApp*)::AfxGetApp())->m_bUseRegistry)
	{
		for (i = 0 ; i < MAX_HOST_PORT_HISTORY_SIZE && i < m_HostsHistory.GetSize() ; i++)
		{
			CString sHostEntry;
			sHostEntry.Format(_T("HostHistory%d"), i);
			pApp->WriteProfileString(sSection, sHostEntry, m_HostsHistory[i]);

			CString sPortEntry;
			sPortEntry.Format(_T("PortHistory%d"), i);
			pApp->WriteProfileInt(sSection, sPortEntry, (int)m_PortsHistory[i]);

			// Device Type and Mode
			CString sDeviceTypeModeEntry;
			sDeviceTypeModeEntry.Format(_T("DeviceTypeModeHistory%d"), i);
			pApp->WriteProfileInt(sSection, sDeviceTypeModeEntry, (int)m_DeviceTypeModesHistory[i]);

			// Buf Size in Frames
			CString sMaxFrameEntry;
			sMaxFrameEntry.Format(_T("MaxFrameHistory%d"), i);
			pApp->WriteProfileInt(sSection, sMaxFrameEntry, (int)m_MaxFramesHistory[i]);

			// Re-Send Check Box
			CString sDisableResendEntry;
			sDisableResendEntry.Format(_T("DisableResendHistory%d"), i);
			pApp->WriteProfileInt(sSection, sDisableResendEntry, (int)m_DisableResendHistory[i]);
		}
	}
	// Write to INI File
	else
	{
		// Make a temporary copy because writing to memory sticks is so slow! 
		CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), pApp->m_pszProfileName);
		::WritePrivateProfileString(NULL, NULL, NULL, pApp->m_pszProfileName); // recache
		::CopyFile(pApp->m_pszProfileName, sTempFileName, FALSE);
		for (i = 0 ; i < MAX_HOST_PORT_HISTORY_SIZE && i < m_HostsHistory.GetSize() ; i++)
		{
			CString sHostEntry;
			sHostEntry.Format(_T("HostHistory%d"), i);
			::WriteProfileIniString(sSection, sHostEntry, m_HostsHistory[i], sTempFileName);

			CString sPortEntry;
			sPortEntry.Format(_T("PortHistory%d"), i);
			::WriteProfileIniInt(sSection, sPortEntry, (int)m_PortsHistory[i], sTempFileName);

			// Device Type and Mode
			CString sDeviceTypeModeEntry;
			sDeviceTypeModeEntry.Format(_T("DeviceTypeModeHistory%d"), i);
			::WriteProfileIniInt(sSection, sDeviceTypeModeEntry, (int)m_DeviceTypeModesHistory[i], sTempFileName);

			// Buf Size in Frames
			CString sMaxFrameEntry;
			sMaxFrameEntry.Format(_T("MaxFrameHistory%d"), i);
			::WriteProfileIniInt(sSection, sMaxFrameEntry, (int)m_MaxFramesHistory[i], sTempFileName);

			// Re-Send Check Box
			CString sDisableResendEntry;
			sDisableResendEntry.Format(_T("DisableResendHistory%d"), i);
			::WriteProfileIniInt(sSection, sDisableResendEntry, (int)m_DisableResendHistory[i], sTempFileName);
		}

		// Move it
		::DeleteFile(pApp->m_pszProfileName);
		::WritePrivateProfileString(NULL, NULL, NULL, sTempFileName); // recache
		::MoveFile(sTempFileName, pApp->m_pszProfileName);
	}
}

void CHostPortDlg::Free() 
{
	for (int i = 0 ; i < m_Hosts.GetSize() ; i++)
	{
		if (m_Connections[i])
			delete m_Connections[i];
		if (m_Parsers[i])
			delete m_Parsers[i];
		if (m_Generators[i])
			delete m_Generators[i];
	}
	m_Connections.RemoveAll();
	m_Parsers.RemoveAll();
	m_Generators.RemoveAll();
}

void CHostPortDlg::OnSearchServers()
{
	CString sText;
	int i;

	BeginWaitCursor();

	Free();

	// Get Selected Port
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
	pEdit->GetWindowText(sText);
	int nPort = _tcstol(sText.GetBuffer(0), NULL, 10);
	sText.ReleaseBuffer();
	if (nPort > 0 && nPort <= 65535) // Port 0 is Reserved
		m_nPort = nPort;
	else
		m_nPort = DEFAULT_UDP_PORT;

	// Enum Hosts in LAN
	CNetCom NetCom;
	NetCom.EnumLAN(&m_Hosts);

	// Start Pings
	for (i = 0 ; i < m_Hosts.GetSize() ; i++)
	{
		CNetCom* pNetCom = (CNetCom*)new CNetCom;
		CPingParseProcess* pPingParseProcess = (CPingParseProcess*)new CPingParseProcess(this, i);
		CPingGenerator* pPingGenerator = (CPingGenerator*)new CPingGenerator(this);

		m_Connections.Add(pNetCom);
		m_Parsers.Add(pPingParseProcess);
		m_Generators.Add(pPingGenerator);
		
		pPingParseProcess->ResetPingReceived();
		Connect(pNetCom,
				pPingParseProcess,
				pPingGenerator,
				m_Hosts[i].m_sName,
				m_nPort);
	}

	EndWaitCursor();
}

BOOL CHostPortDlg::Connect(	CNetCom* pNetCom,
							CPingParseProcess* pPingParseProcess,
							CPingGenerator* pPingGenerator,
							LPCTSTR pszHostName, int nPort)
{
	// Check
	if (!pNetCom)
		return FALSE;

	if (CString(pszHostName) == _T(""))
		return FALSE;

	// Init
	if (!pNetCom->Init(
					FALSE,					// No Meaning For Datagram
					NULL,					// The Optional Owner Window to which send the Network Events.
					NULL,					// The lParam to send with the Messages
					NULL,					// The Optional Rx Buffer.
					NULL,					// The Optional Critical Section for the Rx Buffer.
					NULL,					// The Optional Rx Fifo.
					NULL,					// The Optional Critical Section fot the Rx Fifo.
					NULL,					// The Optional Tx Buffer.
					NULL,					// The Optional Critical Section for the Tx Buffer.
					NULL,					// The Optional Tx Fifo.
					NULL,					// The Optional Critical Section for the Tx Fifo.
					pPingParseProcess,		// Parser
					pPingGenerator,			// Generator
					SOCK_DGRAM,				// SOCK_DGRAM (UDP)
					_T(""),					// Local Address (IP or Host Name).
					0,						// Local Port, let the OS choose one
					pszHostName,			// Peer Address (IP or Host Name).
					nPort,					// Peer Port.
					NULL,					// Handle to an Event Object that will get Accept Events.
					NULL,					// Handle to an Event Object that will get Connect Events.
					NULL,					// Handle to an Event Object that will get Connect Failed Events.
					NULL,					// Handle to an Event Object that will get Close Events.
					NULL,					// Handle to an Event Object that will get Read Events.
					NULL,					// Handle to an Event Object that will get Write Events.
					NULL,					// Handle to an Event Object that will get OOB Events.
					NULL,					// Handle to an Event Object that will get an event when 
											// all connection of a server have been closed.
					0,						// A combination of network events:
											// FD_ACCEPT | FD_CONNECT | FD_CONNECTFAILED | FD_CLOSE | FD_READ | FD_WRITE | FD_OOB | FD_ALLCLOSE.
											// A set value means that instead of setting an event it is reset.
					0,						// A combination of network events:
											// FD_ACCEPT | FD_CONNECT | FD_CONNECTFAILED | FD_CLOSE | FD_READ | FD_WRITE | FD_OOB | FD_ALLCLOSE.
											// The Following messages will be sent to the pOwnerWnd (if pOwnerWnd != NULL):
											// WM_NETCOM_ACCEPT_EVENT -> Notification of incoming connections.
											// WM_NETCOM_CONNECT_EVENT -> Notification of completed connection or multipoint "join" operation.
											// WM_NETCOM_CONNECTFAILED_EVENT -> Notification of connection failure.
											// WM_NETCOM_CLOSE_EVENT -> Notification of socket closure.
											// WM_NETCOM_READ_EVENT -> Notification of readiness for reading.
											// WM_NETCOM_WRITE_EVENT -> Notification of readiness for writing.
											// WM_NETCOM_OOB_EVENT -> Notification of the arrival of out-of-band data.
											// WM_NETCOM_ALLCLOSE_EVENT -> Notification that all connection have been closed.
					0,/*=uiRxMsgTrigger*/	// The number of bytes that triggers an hRxMsgTriggerEvent 
											// (if hRxMsgTriggerEvent != NULL).
											// And/Or the number of bytes that triggers a WM_NETCOM_RX Message
											// (if pOwnerWnd != NULL).
					NULL,/*hRxMsgTriggerEvent*/	// Handle to an Event Object that will get an Event
											// each time uiRxMsgTrigger bytes arrived.
					0,/*uiMaxTxPacketSize*/	// The maximum size for transmitted packets,
											// upper bound for this value is NETCOM_MAX_TX_BUFFER_SIZE.
					0,/*uiRxPacketTimeout*/	// After this timeout a Packet is returned
											// even if the uiRxMsgTrigger size is not reached (A zero meens INFINITE Timeout).
					1000,/*uiTxPacketTimeout*/	// After this timeout a Packet is sent
											// even if no Write Event Happened (A zero meens INFINITE Timeout).
											// This is also the Generator rate,
											// if set to zero the Generator is never called!
					NULL))					// Optional Message Class for Notice, Warning and Error Visualization.
		return FALSE;
	else
	{
		pNetCom->EnableIdleGenerator(TRUE);
		return TRUE;
	}
}

BOOL CHostPortDlg::CPingGenerator::Generate(CNetCom* pNetCom)
{
	::SendUDPPing(pNetCom, 0);

	return TRUE; // Call the Generator again with the next tx timeout
}

BOOL CHostPortDlg::CPingParseProcess::Parse(CNetCom* pNetCom)
{
	// The Received Datagram
	CNetCom::CBuf* pBuf = pNetCom->GetReadHeadBuf();
	
	// Check Packet Family
	if (pBuf->GetAddrPtr()->sin_family != AF_INET)
	{
		delete pBuf;
		pNetCom->RemoveReadHeadBuf();
		return FALSE;
	}

	// Get Header
	NetFrameHdrPingAuth Hdr;
	if (pBuf->GetMsgSize() < sizeof(NetFrameHdrPingAuth))
	{
		delete pBuf;
		pNetCom->RemoveReadHeadBuf();
		return FALSE;
	}
	memcpy(&Hdr, pBuf->GetBuf(), sizeof(NetFrameHdrPingAuth));

	// Ping Answer?
	if (Hdr.Type & NETFRAME_TYPE_PING_ANS)
	{
		pNetCom->EnableIdleGenerator(FALSE);
		m_pDlg->PostMessage(WM_HOST_OK, m_nHostIndex, 0);
	}

	delete pBuf;
	pNetCom->RemoveReadHeadBuf();
	
	return FALSE; // Never Call Processor!
}

LONG CHostPortDlg::OnHostOk(WPARAM wparam, LPARAM lparam)
{
	int nHostIndex = (int)wparam;
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_HOST);
	if (!pComboBox)
		return 0;

	// Insert new ones at the beginning
	m_HostsHistory.InsertAt(0, m_Hosts[nHostIndex].m_sName);
	pComboBox->InsertString(0, m_Hosts[nHostIndex].m_sName);
	m_PortsHistory.InsertAt(0, (DWORD)m_nPort);
	m_DeviceTypeModesHistory.InsertAt(0, 0, 1);
	m_MaxFramesHistory.InsertAt(0, m_dwMaxFrames);
	m_DisableResendHistory.InsertAt(0, (DWORD)m_bDisableResend);

	// Shrink to MAX_HOST_PORT_HISTORY_SIZE
	while (m_HostsHistory.GetSize() > MAX_HOST_PORT_HISTORY_SIZE)
	{
		m_HostsHistory.RemoveAt(m_HostsHistory.GetUpperBound());
		pComboBox->DeleteString(pComboBox->GetCount() - 1);
		m_PortsHistory.RemoveAt(m_PortsHistory.GetUpperBound());
		m_DeviceTypeModesHistory.RemoveAt(m_DeviceTypeModesHistory.GetUpperBound());
		m_MaxFramesHistory.RemoveAt(m_MaxFramesHistory.GetUpperBound());
		m_DisableResendHistory.RemoveAt(m_DisableResendHistory.GetUpperBound());
	}

	// Select last found
	pComboBox->SetCurSel(0);
	
	return 1;
}

#endif
