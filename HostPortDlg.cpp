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
	m_sHost = _T("");
	m_nPort = DEFAULT_TCP_PORT;
	m_nDeviceTypeMode = 0;
}

BEGIN_MESSAGE_MAP(CHostPortDlg, CDialog)
	//{{AFX_MSG_MAP(CHostPortDlg)
	ON_CBN_SELCHANGE(IDC_COMBO_HOST, OnSelchangeComboHost)
	ON_CBN_EDITCHANGE(IDC_COMBO_HOST, OnEditchangeComboHost)
	ON_EN_CHANGE(IDC_EDIT_PORT, OnChangeEditPort)
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICETYPEMODE, OnSelchangeComboDeviceTypeMode)
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

	CDialog::OnInitDialog();

	// Load Settings
	LoadSettings();
	
	// Init
	CComboBox* pComboBoxHost = (CComboBox*)GetDlgItem(IDC_COMBO_HOST);
	if (m_HostsHistory.GetSize() <= 0) // if empty add an item!
	{
		m_HostsHistory.InsertAt(0, _T(""));
		m_PortsHistory.InsertAt(0, (DWORD)DEFAULT_TCP_PORT);
		m_DeviceTypeModesHistory.InsertAt(0, (DWORD)0);
	}
	for (int i = 0 ; i < m_HostsHistory.GetSize() ; i++)
	{
		// Add Hosts
		pComboBoxHost->AddString(m_HostsHistory[i]);
		
		// Set Host, Port and Device Type Mode of first entry
		if (i == 0)
		{
			// Host
			m_sHost = m_HostsHistory[0];

			// Port
			m_nPort = m_PortsHistory[0];
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
			CString sPort;
			sPort.Format(_T("%i"), m_nPort);
			pEdit->SetWindowText(sPort);

			// Device Type Mode
			if (m_DeviceTypeModesHistory[0] >= 0 && (int)m_DeviceTypeModesHistory[0] < pComboBoxDevTypeMode->GetCount())
				m_nDeviceTypeMode = m_DeviceTypeModesHistory[0];
			pComboBoxDevTypeMode->SetCurSel(m_nDeviceTypeMode);
		}
	}
	pComboBoxHost->SetCurSel(0); // pComboBoxHost is never empty!

	// Update Controls
	EnableDisableCtrls();
	LoadCredentials();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHostPortDlg::ParseUrl(CString& sGetFrameVideoHost,
							int& nGetFrameVideoPort,
							int& nNetworkDeviceTypeMode,
							CString& sHttpGetFrameLocation)
{
	// Init Vars
	int nPos, nPosEnd;
	BOOL bUrl = FALSE;
	int nUrlPort = 80; // default url port is always 80
	sGetFrameVideoHost = m_sHost;
	CString sGetFrameVideoHostLowerCase(sGetFrameVideoHost);
	sGetFrameVideoHostLowerCase.MakeLower();

	// Numeric IP6 with format http://[ip6%interfacenum]:port/framelocation
	if ((nPos = sGetFrameVideoHostLowerCase.Find(_T("http://["))) >= 0)
	{
		// Set flag
		bUrl = TRUE;

		// Remove leading http://[ from url
		sGetFrameVideoHost = sGetFrameVideoHost.Right(sGetFrameVideoHost.GetLength() - 8 - nPos);

		// Has Port?
		if ((nPos = sGetFrameVideoHost.Find(_T("]:"))) >= 0)
		{
			CString sPort;
			if ((nPosEnd = sGetFrameVideoHost.Find(_T('/'), nPos)) >= 0)
			{
				sPort = sGetFrameVideoHost.Mid(nPos + 2, nPosEnd - nPos - 2);
				sGetFrameVideoHost.Delete(nPos, nPosEnd - nPos);
			}
			else
			{
				sPort = sGetFrameVideoHost.Mid(nPos + 2, sGetFrameVideoHost.GetLength() - nPos - 2);
				sGetFrameVideoHost.Delete(nPos, sGetFrameVideoHost.GetLength() - nPos);
			}
			sPort.TrimLeft();
			sPort.TrimRight();
			int nPort = _tcstol(sPort.GetBuffer(0), NULL, 10);
			sPort.ReleaseBuffer();
			if (nPort > 0 && nPort <= 65535) // Port 0 is Reserved
				nUrlPort = nPort;
		}
		else if ((nPos = sGetFrameVideoHost.Find(_T("]"))) >= 0)
			sGetFrameVideoHost.Delete(nPos);
		else
			nPos = sGetFrameVideoHost.GetLength(); // Just in case ] is missing

		// Split
		CString sLocation = sGetFrameVideoHost.Right(sGetFrameVideoHost.GetLength() - nPos);
		sGetFrameVideoHost = sGetFrameVideoHost.Left(nPos);

		// Get Location which is set as first automatic camera type detection query string
		nPos = sLocation.Find(_T('/'));
		if (nPos >= 0)
		{	
			sHttpGetFrameLocation = sLocation.Right(sLocation.GetLength() - nPos);
			sHttpGetFrameLocation.TrimLeft();
			sHttpGetFrameLocation.TrimRight();
		}
		else
			sHttpGetFrameLocation = _T("/");
	}
	// Numeric IP4 or hostname with format http://host:port/framelocation
	else if ((nPos = sGetFrameVideoHostLowerCase.Find(_T("http://"))) >= 0)
	{
		// Set flag
		bUrl = TRUE;

		// Remove leading http:// from url
		sGetFrameVideoHost = sGetFrameVideoHost.Right(sGetFrameVideoHost.GetLength() - 7 - nPos);

		// Has Port?
		if ((nPos = sGetFrameVideoHost.Find(_T(":"))) >= 0)
		{
			CString sPort;
			if ((nPosEnd = sGetFrameVideoHost.Find(_T('/'), nPos)) >= 0)
			{
				sPort = sGetFrameVideoHost.Mid(nPos + 1, nPosEnd - nPos - 1);
				sGetFrameVideoHost.Delete(nPos, nPosEnd - nPos);
			}
			else
			{
				sPort = sGetFrameVideoHost.Mid(nPos + 1, sGetFrameVideoHost.GetLength() - nPos - 1);
				sGetFrameVideoHost.Delete(nPos, sGetFrameVideoHost.GetLength() - nPos);
			}
			sPort.TrimLeft();
			sPort.TrimRight();
			int nPort = _tcstol(sPort.GetBuffer(0), NULL, 10);
			sPort.ReleaseBuffer();
			if (nPort > 0 && nPort <= 65535) // Port 0 is Reserved
				nUrlPort = nPort;
		}

		// Get Location which is set as first automatic camera type detection query string
		nPos = sGetFrameVideoHost.Find(_T('/'));
		if (nPos >= 0)
		{	
			sHttpGetFrameLocation = sGetFrameVideoHost.Right(sGetFrameVideoHost.GetLength() - nPos);
			sGetFrameVideoHost = sGetFrameVideoHost.Left(nPos);
			sHttpGetFrameLocation.TrimLeft();
			sHttpGetFrameLocation.TrimRight();
		}
		else
			sHttpGetFrameLocation = _T("/");
	}
	else
		sHttpGetFrameLocation = _T("/");

	// Set vars
	sGetFrameVideoHost.TrimLeft();
	sGetFrameVideoHost.TrimRight();
	nGetFrameVideoPort = bUrl ? nUrlPort : m_nPort;
	nNetworkDeviceTypeMode = bUrl ? CVideoDeviceDoc::OTHERONE_CP : m_nDeviceTypeMode;
}

/*
From: http://msdn.microsoft.com/en-us/library/windows/desktop/aa511459.aspx
Don't disable group boxes. To indicate that a group of controls doesn't currently
apply, disable all the controls within the group box, but not the group box itself.
This approach is more accessible and can be supported consistently by all UI frameworks.
*/
void CHostPortDlg::EnableDisableCtrls()
{
	CString sHostLowerCase(m_sHost);
	sHostLowerCase.MakeLower();

	// Disable / Enable
	CEdit* pEditPort = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
	CComboBox* pComboBoxDevTypeMode = (CComboBox*)GetDlgItem(IDC_COMBO_DEVICETYPEMODE);
	CStatic* pStaticServerPush = (CStatic*)GetDlgItem(IDC_STATIC_SERVERPUSH);
	CStatic* pStaticClientPoll = (CStatic*)GetDlgItem(IDC_STATIC_CLIENTPOLL);
	if (sHostLowerCase.Find(_T("http://")) >= 0)
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
	CComboBox* pComboBoxHost = (CComboBox*)GetDlgItem(IDC_COMBO_HOST);
	pComboBoxHost->GetWindowText(m_sHost);
	EnableDisableCtrls();
	LoadCredentials();
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
	// Host
	CComboBox* pComboBoxHost = (CComboBox*)GetDlgItem(IDC_COMBO_HOST);
	pComboBoxHost->GetLBText(pComboBoxHost->GetCurSel(), m_sHost);

	// Port
	int nSel = pComboBoxHost->GetCurSel();
	if (nSel >= 0 && nSel < m_PortsHistory.GetSize())
	{
		m_nPort = m_PortsHistory[nSel];
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
		CString sPort;
		sPort.Format(_T("%i"), m_nPort);
		pEdit->SetWindowText(sPort);
	}

	// Device Type Mode
	CComboBox* pComboBoxDevTypeMode = (CComboBox*)GetDlgItem(IDC_COMBO_DEVICETYPEMODE);
	if (nSel >= 0 && nSel < m_DeviceTypeModesHistory.GetSize())
	{
		if (m_DeviceTypeModesHistory[nSel] >= 0 && (int)m_DeviceTypeModesHistory[nSel] < pComboBoxDevTypeMode->GetCount())
		{
			m_nDeviceTypeMode = m_DeviceTypeModesHistory[nSel];
			pComboBoxDevTypeMode->SetCurSel(m_nDeviceTypeMode);
		}
	}

	// Update Controls
	EnableDisableCtrls();
	LoadCredentials();
}

void CHostPortDlg::OnChangeEditPort()
{
	CString sPort;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
	pEdit->GetWindowText(sPort);
	int nPort = _tcstol(sPort.GetBuffer(0), NULL, 10);
	sPort.ReleaseBuffer();
	if (nPort > 0 && nPort <= 65535) // Port 0 is Reserved
		m_nPort = nPort;
	else
		m_nPort = DEFAULT_TCP_PORT;
	LoadCredentials();
}

void CHostPortDlg::OnSelchangeComboDeviceTypeMode()
{
	CComboBox* pComboBoxDevTypeMode = (CComboBox*)GetDlgItem(IDC_COMBO_DEVICETYPEMODE);
	m_nDeviceTypeMode = pComboBoxDevTypeMode->GetCurSel();
	LoadCredentials();
}

void CHostPortDlg::OnOK() 
{
	SaveSettings();
	SaveCredentials();
	CDialog::OnOK();
}

void CHostPortDlg::LoadCredentials()
{
	// Get device path name
	CString sGetFrameVideoHost;
	int nGetFrameVideoPort;
	int nNetworkDeviceTypeMode;
	CString sHttpGetFrameLocation;
	ParseUrl(sGetFrameVideoHost, nGetFrameVideoPort, nNetworkDeviceTypeMode, sHttpGetFrameLocation);
	CString sDevice;
	sDevice.Format(_T("%s:%d:%s:%d"), sGetFrameVideoHost, nGetFrameVideoPort, sHttpGetFrameLocation, nNetworkDeviceTypeMode);
	sDevice.Replace(_T('\\'), _T('/')); // Registry keys cannot begin with a backslash and should not contain backslashes otherwise subkeys are created!
	
	// Load & display
	CString	sUsername = ((CUImagerApp*)::AfxGetApp())->GetSecureProfileString(sDevice, _T("HTTPGetFrameUsername"), _T(""));
	CString	sPassword = ((CUImagerApp*)::AfxGetApp())->GetSecureProfileString(sDevice, _T("HTTPGetFramePassword"), _T(""));
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->SetWindowText(sUsername);
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->SetWindowText(sPassword);
}

void CHostPortDlg::SaveCredentials()
{
	// Get device path name
	CString sGetFrameVideoHost;
	int nGetFrameVideoPort;
	int nNetworkDeviceTypeMode;
	CString sHttpGetFrameLocation;
	ParseUrl(sGetFrameVideoHost, nGetFrameVideoPort, nNetworkDeviceTypeMode, sHttpGetFrameLocation);
	CString sDevice;
	sDevice.Format(_T("%s:%d:%s:%d"), sGetFrameVideoHost, nGetFrameVideoPort, sHttpGetFrameLocation, nNetworkDeviceTypeMode);
	sDevice.Replace(_T('\\'), _T('/')); // Registry keys cannot begin with a backslash and should not contain backslashes otherwise subkeys are created!
	
	// Store
	CString sText;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->GetWindowText(sText);
	((CUImagerApp*)::AfxGetApp())->WriteSecureProfileString(sDevice, _T("HTTPGetFrameUsername"), sText);
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->GetWindowText(sText);
	((CUImagerApp*)::AfxGetApp())->WriteSecureProfileString(sDevice, _T("HTTPGetFramePassword"), sText);
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