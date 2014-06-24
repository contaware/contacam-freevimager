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
	// Store initial dialog title
	GetWindowText(m_sInitialDlgTitle);

	// Init Device Type Mode Combo Box
	CComboBox* pComboBoxDevTypeMode = (CComboBox*)GetDlgItem(IDC_COMBO_DEVICETYPEMODE);
	pComboBoxDevTypeMode->AddString(ML_STRING(1548, "Other HTTP Camera") + _T(" (") + ML_STRING(1865, "Server Push Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(ML_STRING(1548, "Other HTTP Camera") + _T(" (") + ML_STRING(1866, "Client Poll Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Axis (") + ML_STRING(1865, "Server Push Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Axis (") + ML_STRING(1866, "Client Poll Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Panasonic (") + ML_STRING(1865, "Server Push Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Panasonic (") + ML_STRING(1866, "Client Poll Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Pixord/NetComm (") + ML_STRING(1865, "Server Push Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Pixord/NetComm (") + ML_STRING(1866, "Client Poll Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Edimax (") + ML_STRING(1865, "Server Push Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Edimax (") + ML_STRING(1866, "Client Poll Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("TP-Link (") + ML_STRING(1865, "Server Push Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("TP-Link (") + ML_STRING(1866, "Client Poll Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Foscam/Tenvis/Clones (") + ML_STRING(1865, "Server Push Mode") + _T(")"));
	pComboBoxDevTypeMode->AddString(_T("Foscam/Tenvis/Clones (") + ML_STRING(1866, "Client Poll Mode") + _T(")"));

	CDialog::OnInitDialog();

	// Load History
	LoadHistory(m_HostsHistory, m_PortsHistory, m_DeviceTypeModesHistory);
	
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
	LoadCredentialsAndTitle();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHostPortDlg::ParseUrl(const CString& sInHost,
							int nInPort,
							int nInDeviceTypeMode,
							CString& sOutGetFrameVideoHost,
							int& nOutGetFrameVideoPort,
							CString& sOutHttpGetFrameLocation,
							int& nOutDeviceTypeMode)
{
	// Init Vars
	int nPos, nPosEnd;
	BOOL bUrl = FALSE;
	int nUrlPort = 80; // default url port is always 80
	sOutGetFrameVideoHost = sInHost;
	CString sGetFrameVideoHostLowerCase(sOutGetFrameVideoHost);
	sGetFrameVideoHostLowerCase.MakeLower();

	// Numeric IP6 with format http://[ip6%interfacenum]:port/framelocation
	if ((nPos = sGetFrameVideoHostLowerCase.Find(_T("http://["))) >= 0)
	{
		// Set flag
		bUrl = TRUE;

		// Remove leading http://[ from url
		sOutGetFrameVideoHost = sOutGetFrameVideoHost.Right(sOutGetFrameVideoHost.GetLength() - 8 - nPos);

		// Has Port?
		if ((nPos = sOutGetFrameVideoHost.Find(_T("]:"))) >= 0)
		{
			CString sPort;
			if ((nPosEnd = sOutGetFrameVideoHost.Find(_T('/'), nPos)) >= 0)
			{
				sPort = sOutGetFrameVideoHost.Mid(nPos + 2, nPosEnd - nPos - 2);
				sOutGetFrameVideoHost.Delete(nPos, nPosEnd - nPos);
			}
			else
			{
				sPort = sOutGetFrameVideoHost.Mid(nPos + 2, sOutGetFrameVideoHost.GetLength() - nPos - 2);
				sOutGetFrameVideoHost.Delete(nPos, sOutGetFrameVideoHost.GetLength() - nPos);
			}
			sPort.TrimLeft();
			sPort.TrimRight();
			int nPort = _tcstol(sPort.GetBuffer(0), NULL, 10);
			sPort.ReleaseBuffer();
			if (nPort > 0 && nPort <= 65535) // Port 0 is Reserved
				nUrlPort = nPort;
		}
		else if ((nPos = sOutGetFrameVideoHost.Find(_T("]"))) >= 0)
			sOutGetFrameVideoHost.Delete(nPos);
		else
			nPos = sOutGetFrameVideoHost.GetLength(); // Just in case ] is missing

		// Split
		CString sLocation = sOutGetFrameVideoHost.Right(sOutGetFrameVideoHost.GetLength() - nPos);
		sOutGetFrameVideoHost = sOutGetFrameVideoHost.Left(nPos);

		// Get Location which is set as first automatic camera type detection query string
		nPos = sLocation.Find(_T('/'));
		if (nPos >= 0)
		{	
			sOutHttpGetFrameLocation = sLocation.Right(sLocation.GetLength() - nPos);
			sOutHttpGetFrameLocation.TrimLeft();
			sOutHttpGetFrameLocation.TrimRight();
		}
		else
			sOutHttpGetFrameLocation = _T("/");
	}
	// Numeric IP4 or hostname with format http://host:port/framelocation
	else if ((nPos = sGetFrameVideoHostLowerCase.Find(_T("http://"))) >= 0)
	{
		// Set flag
		bUrl = TRUE;

		// Remove leading http:// from url
		sOutGetFrameVideoHost = sOutGetFrameVideoHost.Right(sOutGetFrameVideoHost.GetLength() - 7 - nPos);

		// Has Port?
		if ((nPos = sOutGetFrameVideoHost.Find(_T(":"))) >= 0)
		{
			CString sPort;
			if ((nPosEnd = sOutGetFrameVideoHost.Find(_T('/'), nPos)) >= 0)
			{
				sPort = sOutGetFrameVideoHost.Mid(nPos + 1, nPosEnd - nPos - 1);
				sOutGetFrameVideoHost.Delete(nPos, nPosEnd - nPos);
			}
			else
			{
				sPort = sOutGetFrameVideoHost.Mid(nPos + 1, sOutGetFrameVideoHost.GetLength() - nPos - 1);
				sOutGetFrameVideoHost.Delete(nPos, sOutGetFrameVideoHost.GetLength() - nPos);
			}
			sPort.TrimLeft();
			sPort.TrimRight();
			int nPort = _tcstol(sPort.GetBuffer(0), NULL, 10);
			sPort.ReleaseBuffer();
			if (nPort > 0 && nPort <= 65535) // Port 0 is Reserved
				nUrlPort = nPort;
		}

		// Get Location which is set as first automatic camera type detection query string
		nPos = sOutGetFrameVideoHost.Find(_T('/'));
		if (nPos >= 0)
		{	
			sOutHttpGetFrameLocation = sOutGetFrameVideoHost.Right(sOutGetFrameVideoHost.GetLength() - nPos);
			sOutGetFrameVideoHost = sOutGetFrameVideoHost.Left(nPos);
			sOutHttpGetFrameLocation.TrimLeft();
			sOutHttpGetFrameLocation.TrimRight();
		}
		else
			sOutHttpGetFrameLocation = _T("/");
	}
	else
		sOutHttpGetFrameLocation = _T("/");

	// Set vars
	sOutGetFrameVideoHost.TrimLeft();
	sOutGetFrameVideoHost.TrimRight();
	nOutGetFrameVideoPort = bUrl ? nUrlPort : nInPort;
	nOutDeviceTypeMode = bUrl ? CVideoDeviceDoc::OTHERONE_CP : nInDeviceTypeMode;
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
	LoadCredentialsAndTitle();
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
	LoadCredentialsAndTitle();
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
	LoadCredentialsAndTitle();
}

void CHostPortDlg::OnSelchangeComboDeviceTypeMode()
{
	CComboBox* pComboBoxDevTypeMode = (CComboBox*)GetDlgItem(IDC_COMBO_DEVICETYPEMODE);
	m_nDeviceTypeMode = pComboBoxDevTypeMode->GetCurSel();
	LoadCredentialsAndTitle();
}

void CHostPortDlg::OnOK() 
{
	SaveHistory(m_sHost, m_nPort, m_nDeviceTypeMode,
				m_HostsHistory, m_PortsHistory, m_DeviceTypeModesHistory);
	SaveCredentials();
	CDialog::OnOK();
}

CString CHostPortDlg::MakeDevicePathName(const CString& sInHost, int nInPort, int nInDeviceTypeMode)
{
	CString sOutGetFrameVideoHost;
	int nOutGetFrameVideoPort;
	CString sOutHttpGetFrameLocation;
	int nOutDeviceTypeMode;
	ParseUrl(sInHost, nInPort, nInDeviceTypeMode,
			sOutGetFrameVideoHost, nOutGetFrameVideoPort,
			sOutHttpGetFrameLocation, nOutDeviceTypeMode);
	return CVideoDeviceDoc::GetNetworkDevicePathName(	sOutGetFrameVideoHost, nOutGetFrameVideoPort,
														sOutHttpGetFrameLocation, nOutDeviceTypeMode);
}

void CHostPortDlg::LoadCredentialsAndTitle()
{
	// Get device path name
	CString sDevicePathName = MakeDevicePathName(m_sHost, m_nPort, m_nDeviceTypeMode);

	// Load and display credentials
	CString	sUsername = ((CUImagerApp*)::AfxGetApp())->GetSecureProfileString(sDevicePathName, _T("HTTPGetFrameUsername"), _T(""));
	CString	sPassword = ((CUImagerApp*)::AfxGetApp())->GetSecureProfileString(sDevicePathName, _T("HTTPGetFramePassword"), _T(""));
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->SetWindowText(sUsername);
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->SetWindowText(sPassword);

	// Update dialog title
	CString	sName = ((CUImagerApp*)::AfxGetApp())->GetProfileString(sDevicePathName, _T("RecordAutoSaveDir"), _T(""));
	sName.TrimRight(_T('\\'));
	int index = sName.ReverseFind(_T('\\'));
	if (index >= 0)
		sName = sName.Mid(index + 1);
	if (sName.IsEmpty())
		SetWindowText(m_sInitialDlgTitle);
	else
		SetWindowText(m_sInitialDlgTitle + _T(" ") + sName);
}

void CHostPortDlg::SaveCredentials()
{
	// Get device path name
	CString sDevicePathName = MakeDevicePathName(m_sHost, m_nPort, m_nDeviceTypeMode);

	// Store
	CString sText;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->GetWindowText(sText);
	((CUImagerApp*)::AfxGetApp())->WriteSecureProfileString(sDevicePathName, _T("HTTPGetFrameUsername"), sText);
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->GetWindowText(sText);
	((CUImagerApp*)::AfxGetApp())->WriteSecureProfileString(sDevicePathName, _T("HTTPGetFramePassword"), sText);
}

void CHostPortDlg::LoadHistory(	CStringArray& HostsHistory,
								CDWordArray& PortsHistory,
								CDWordArray& DeviceTypeModesHistory)
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
			HostsHistory.Add(sHost);

			// Port
			CString sPortEntry;
			sPortEntry.Format(_T("PortHistory%d"), i);
			dwPort = (DWORD) pApp->GetProfileInt(sSection, sPortEntry, 0xFFFFFFFF);
			if (dwPort == 0 || dwPort > 65535) // Port 0 is Reserved
				dwPort = DEFAULT_TCP_PORT;
			PortsHistory.Add(dwPort);

			// Device Type and Mode
			CString sDeviceTypeModeEntry;
			sDeviceTypeModeEntry.Format(_T("DeviceTypeModeHistory%d"), i);
			dwDeviceTypeMode = (DWORD) pApp->GetProfileInt(sSection, sDeviceTypeModeEntry, 0);
			DeviceTypeModesHistory.Add(dwDeviceTypeMode);
		}
	}
}

void CHostPortDlg::SaveHistory(	const CString& sHost,
								int nPort,
								int nDeviceTypeMode,
								CStringArray& HostsHistory,
								CDWordArray& PortsHistory,
								CDWordArray& DeviceTypeModesHistory)
{
	int i;
	CWinApp* pApp = ::AfxGetApp();
	CString sSection(_T("HostPortDlg"));

	// Remove duplicates
	i = 0;
	while (i < HostsHistory.GetSize())
	{
		if (HostsHistory[i] == sHost)
		{
			HostsHistory.RemoveAt(i);
			PortsHistory.RemoveAt(i);
			DeviceTypeModesHistory.RemoveAt(i);
			i = 0; // restart to check
		}
		else
			i++;
	}

	// Insert new one at the beginning
	if (!sHost.IsEmpty())
	{
		HostsHistory.InsertAt(0, sHost);
		PortsHistory.InsertAt(0, (DWORD)nPort);
		DeviceTypeModesHistory.InsertAt(0, (DWORD)nDeviceTypeMode);
	}

	// Shrink to MAX_HOST_PORT_HISTORY_SIZE
	while (HostsHistory.GetSize() > MAX_HOST_PORT_HISTORY_SIZE)
	{
		HostsHistory.RemoveAt(HostsHistory.GetUpperBound());
		PortsHistory.RemoveAt(PortsHistory.GetUpperBound());
		DeviceTypeModesHistory.RemoveAt(DeviceTypeModesHistory.GetUpperBound());
	}

	// Write them
	for (i = 0 ; i < MAX_HOST_PORT_HISTORY_SIZE ; i++)
	{
		CString sHostEntry;
		sHostEntry.Format(_T("HostHistory%d"), i);
		CString sPortEntry;
		sPortEntry.Format(_T("PortHistory%d"), i);
		CString sDeviceTypeModeEntry;
		sDeviceTypeModeEntry.Format(_T("DeviceTypeModeHistory%d"), i);
		if (i < HostsHistory.GetSize())
		{
			pApp->WriteProfileString(sSection, sHostEntry, HostsHistory[i]);
			pApp->WriteProfileInt(sSection, sPortEntry, (int)PortsHistory[i]);
			pApp->WriteProfileInt(sSection, sDeviceTypeModeEntry, (int)DeviceTypeModesHistory[i]);
		}
		else
		{
			if (::AfxGetApp()->m_pszRegistryKey)
			{
				::DeleteRegistryValue(HKEY_CURRENT_USER, CString(_T("Software\\")) + MYCOMPANY + _T("\\") + APPNAME_NOEXT + _T("\\") + sSection, sHostEntry);
				::DeleteRegistryValue(HKEY_CURRENT_USER, CString(_T("Software\\")) + MYCOMPANY + _T("\\") + APPNAME_NOEXT + _T("\\") + sSection, sPortEntry);
				::DeleteRegistryValue(HKEY_CURRENT_USER, CString(_T("Software\\")) + MYCOMPANY + _T("\\") + APPNAME_NOEXT + _T("\\") + sSection, sDeviceTypeModeEntry);
			}
			else
			{
				::WritePrivateProfileString(sSection, sHostEntry, NULL, ::AfxGetApp()->m_pszProfileName);
				::WritePrivateProfileString(sSection, sPortEntry, NULL, ::AfxGetApp()->m_pszProfileName);
				::WritePrivateProfileString(sSection, sDeviceTypeModeEntry, NULL, ::AfxGetApp()->m_pszProfileName);
			}
		}
	}
}

void CHostPortDlg::DeleteHistory(const CString& sDevicePathName)
{
	// Load History
	CStringArray HostsHistory;
	CDWordArray PortsHistory;
	CDWordArray DeviceTypeModesHistory;
	LoadHistory(HostsHistory, PortsHistory, DeviceTypeModesHistory);

	// Remove given Device Path Name
	int i = 0;
	while (i < HostsHistory.GetSize())
	{
		if (MakeDevicePathName(HostsHistory[i], PortsHistory[i], DeviceTypeModesHistory[i]) == sDevicePathName)
		{
			HostsHistory.RemoveAt(i);
			PortsHistory.RemoveAt(i);
			DeviceTypeModesHistory.RemoveAt(i);
			i = 0; // restart to check
		}
		else
			i++;
	}

	// Save History
	SaveHistory(_T(""), 0, 0, HostsHistory, PortsHistory, DeviceTypeModesHistory);
}

#endif