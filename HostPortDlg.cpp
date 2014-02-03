// HostPortDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "HostPortDlg.h"
#include "VideoDeviceDoc.h"

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
	//}}AFX_DATA_INIT
	m_sHost = _T("");
	m_nPort = DEFAULT_TCP_PORT;
}

void CHostPortDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHostPortDlg)
	DDX_CBIndex(pDX, IDC_COMBO_DEVICETYPEMODE, m_nDeviceTypeMode);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CHostPortDlg, CDialog)
	//{{AFX_MSG_MAP(CHostPortDlg)
	ON_CBN_SELCHANGE(IDC_COMBO_HOST, OnSelchangeComboHost)
	ON_CBN_EDITCHANGE(IDC_COMBO_HOST, OnEditchangeComboHost)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CHostPortDlg::OnInitDialog() 
{
	// Init Device Type Mode Combo Box
	CComboBox* pComboBoxDevTypeMode = (CComboBox*)GetDlgItem(IDC_COMBO_DEVICETYPEMODE);
	pComboBoxDevTypeMode->AddString(ML_STRING(1548, "Other HTTP Device") + _T(" (") + ML_STRING(1865, "Server Push Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(ML_STRING(1548, "Other HTTP Device") + _T(" (") + ML_STRING(1866, "Client Poll Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Axis (") + ML_STRING(1865, "Server Push Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Axis (") + ML_STRING(1866, "Client Poll Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Panasonic (") + ML_STRING(1865, "Server Push Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Panasonic (") + ML_STRING(1866, "Client Poll Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Pixord or NetComm (") + ML_STRING(1865, "Server Push Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Pixord or NetComm (") + ML_STRING(1866, "Client Poll Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Edimax (") + ML_STRING(1865, "Server Push Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Edimax (") + ML_STRING(1866, "Client Poll Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("TP-Link (") + ML_STRING(1865, "Server Push Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("TP-Link (") + ML_STRING(1866, "Client Poll Mode") + _T(")"));

	// This calls UpdateData(FALSE) -> vars to view
	CDialog::OnInitDialog();

	// Load Settings
	LoadSettings();
	
	// Init
	CComboBox* pComboBoxHost = (CComboBox*)GetDlgItem(IDC_COMBO_HOST);
	if (m_HostsHistory.GetSize() <= 0)
	{
		m_HostsHistory.InsertAt(0, _T(""));
		m_PortsHistory.InsertAt(0, (DWORD)DEFAULT_TCP_PORT);
		m_DeviceTypeModesHistory.InsertAt(0, (DWORD)0);
	}
	for (int i = 0 ; i < m_HostsHistory.GetSize() ; i++)
	{
		// Add Hosts
		pComboBoxHost->AddString(m_HostsHistory[i]);
		
		// Set Port and Device Type Mode of first Host
		if (i == 0)
		{
			// Port
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
			CString sPort;
			sPort.Format(_T("%i"), m_PortsHistory[0]);
			pEdit->SetWindowText(sPort);

			// Device Type Mode
			if (m_DeviceTypeModesHistory[0] >= 0 && (int)m_DeviceTypeModesHistory[0] < pComboBoxDevTypeMode->GetCount())
				pComboBoxDevTypeMode->SetCurSel(m_DeviceTypeModesHistory[0]);
		}
	}
	pComboBoxHost->SetCurSel(0);			// pComboBoxHost is never empty!
	EnableDisableCtrls(m_HostsHistory[0]);	// m_HostsHistory is never empty!

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHostPortDlg::OnOK() 
{
	// Update m_nDeviceTypeMode necessary by SaveSettings()
	UpdateData(TRUE);

	// Host
	CString sText;
	CComboBox* pComboBoxHost = (CComboBox*)GetDlgItem(IDC_COMBO_HOST);
	pComboBoxHost->GetWindowText(sText);
	m_sHost = sText;
		
	// Port
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
	pEdit->GetWindowText(sText);
	int nPort = _tcstol(sText.GetBuffer(0), NULL, 10);
	sText.ReleaseBuffer();
	if (nPort > 0 && nPort <= 65535) // Port 0 is Reserved
		m_nPort = nPort;
	else
		m_nPort = DEFAULT_TCP_PORT;

	// Save Settings
	SaveSettings();

	// This calls UpdateData(TRUE) -> view to vars
	CDialog::OnOK();
}

/*
From: http://msdn.microsoft.com/en-us/library/windows/desktop/aa511459.aspx
Don't disable group boxes. To indicate that a group of controls doesn't currently
apply, disable all the controls within the group box, but not the group box itself.
This approach is more accessible and can be supported consistently by all UI frameworks.
*/
void CHostPortDlg::EnableDisableCtrls(CString sHost)
{
	// Make lowercase
	sHost.MakeLower();

	// Disable / Enable
	CEdit* pEditPort = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
	CComboBox* pComboBoxDevTypeMode = (CComboBox*)GetDlgItem(IDC_COMBO_DEVICETYPEMODE);
	CStatic* pStaticServerPush = (CStatic*)GetDlgItem(IDC_STATIC_SERVERPUSH);
	CStatic* pStaticClientPoll = (CStatic*)GetDlgItem(IDC_STATIC_CLIENTPOLL);
	if (sHost.Find(_T("http://")) >= 0)
	{
		pEditPort->EnableWindow(FALSE);
		pComboBoxDevTypeMode->EnableWindow(FALSE);
		pStaticServerPush->EnableWindow(FALSE);
		pStaticClientPoll->EnableWindow(FALSE);
	}
	else
	{
		pEditPort->EnableWindow(TRUE);
		pComboBoxDevTypeMode->EnableWindow(TRUE);
		pStaticServerPush->EnableWindow(TRUE);
		pStaticClientPoll->EnableWindow(TRUE);
	}
}

void CHostPortDlg::OnEditchangeComboHost()
{
	// Enable / Disable Controls
	CString sHost;
	CComboBox* pComboBoxHost = (CComboBox*)GetDlgItem(IDC_COMBO_HOST);
	pComboBoxHost->GetWindowText(sHost);
	EnableDisableCtrls(sHost);
}

/*
According to KB66365
--------------------
When an application receives the CBN_SELCHANGE notification message,
the edit/static portion of the combo box has not been updated.
To obtain the new selection, send a CB_GETLBTEXT message to the
combo box control.
*/
void CHostPortDlg::OnSelchangeComboHost() 
{
	// Set Port
	CComboBox* pComboBoxHost = (CComboBox*)GetDlgItem(IDC_COMBO_HOST);
	int nSel = pComboBoxHost->GetCurSel();
	if (nSel >= 0 && nSel < m_PortsHistory.GetSize())
	{
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
		CString sPort;
		sPort.Format(_T("%i"), m_PortsHistory[nSel]);
		pEdit->SetWindowText(sPort);
	}

	// Set Device Type Mode
	CComboBox* pComboBoxDevTypeMode = (CComboBox*)GetDlgItem(IDC_COMBO_DEVICETYPEMODE);
	if (nSel >= 0 && nSel < m_DeviceTypeModesHistory.GetSize())
	{
		if (m_DeviceTypeModesHistory[nSel] >= 0 && (int)m_DeviceTypeModesHistory[nSel] < pComboBoxDevTypeMode->GetCount())
			pComboBoxDevTypeMode->SetCurSel(m_DeviceTypeModesHistory[nSel]);
	}

	// Enable / Disable Controls
	CString sHost;
	pComboBoxHost->GetLBText(pComboBoxHost->GetCurSel(), sHost);
	EnableDisableCtrls(sHost);
}

void CHostPortDlg::LoadSettings()
{
	CWinApp* pApp = ::AfxGetApp();
	CString sSection(_T("HostPortDlg"));

	CString sHost;
	DWORD dwPort;
	DWORD dwDeviceTypeMode;
	for (int i = 0 ; i < MAX_HOST_PORT_HISTORY_SIZE ; i++)
	{
		CString sHostEntry;
		sHostEntry.Format(_T("HostHistory%d"), i);
		sHost = pApp->GetProfileString(sSection, sHostEntry, _T(""));
		if (!sHost.IsEmpty())
		{
			// Host
			m_HostsHistory.Add(sHost);

			// Port
			CString sPortEntry;
			sPortEntry.Format(_T("PortHistory%d"), i);
			dwPort = (DWORD) pApp->GetProfileInt(sSection, sPortEntry, 0xFFFFFFFF);
			if (dwPort == 0 || dwPort > 65535) // Port 0 is Reserved
				dwPort = DEFAULT_TCP_PORT;
			m_PortsHistory.Add(dwPort);

			// Device Type and Mode
			CString sDeviceTypeModeEntry;
			sDeviceTypeModeEntry.Format(_T("DeviceTypeModeHistory%d"), i);
			dwDeviceTypeMode = (DWORD) pApp->GetProfileInt(sSection, sDeviceTypeModeEntry, 0);
			m_DeviceTypeModesHistory.Add(dwDeviceTypeMode);
		}
	}
}

void CHostPortDlg::SaveSettings()
{
	int i;
	CWinApp* pApp = ::AfxGetApp();
	CString sSection(_T("HostPortDlg"));

	// Remove duplicates
	i = 0;
	while (i < m_HostsHistory.GetSize())
	{
		if (m_HostsHistory[i] == m_sHost)
		{
			m_HostsHistory.RemoveAt(i);
			m_PortsHistory.RemoveAt(i);
			m_DeviceTypeModesHistory.RemoveAt(i);
			i = 0; // restart to check
		}
		else
			i++;
	}

	// Insert new one at the beginning
	if (!m_sHost.IsEmpty())
	{
		m_HostsHistory.InsertAt(0, m_sHost);
		m_PortsHistory.InsertAt(0, (DWORD)m_nPort);
		m_DeviceTypeModesHistory.InsertAt(0, (DWORD)m_nDeviceTypeMode);
	}

	// Shrink to MAX_HOST_PORT_HISTORY_SIZE
	while (m_HostsHistory.GetSize() > MAX_HOST_PORT_HISTORY_SIZE)
	{
		m_HostsHistory.RemoveAt(m_HostsHistory.GetUpperBound());
		m_PortsHistory.RemoveAt(m_PortsHistory.GetUpperBound());
		m_DeviceTypeModesHistory.RemoveAt(m_DeviceTypeModesHistory.GetUpperBound());
	}

	// Write them
	for (i = 0 ; i < MAX_HOST_PORT_HISTORY_SIZE && i < m_HostsHistory.GetSize() ; i++)
	{
		// Host
		CString sHostEntry;
		sHostEntry.Format(_T("HostHistory%d"), i);
		pApp->WriteProfileString(sSection, sHostEntry, m_HostsHistory[i]);
			
		// Port
		CString sPortEntry;
		sPortEntry.Format(_T("PortHistory%d"), i);
		pApp->WriteProfileInt(sSection, sPortEntry, (int)m_PortsHistory[i]);

		// Device Type and Mode
		CString sDeviceTypeModeEntry;
		sDeviceTypeModeEntry.Format(_T("DeviceTypeModeHistory%d"), i);
		pApp->WriteProfileInt(sSection, sDeviceTypeModeEntry, (int)m_DeviceTypeModesHistory[i]);
	}
}

#endif