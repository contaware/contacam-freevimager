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
	m_nPort = DEFAULT_RTSP_PORT;
	m_nDeviceTypeMode = CVideoDeviceDoc::OTHERONE_RTSP;
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
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(ML_STRING(1548, "Other Camera") + _T(" (RTSP)")), (DWORD)CVideoDeviceDoc::OTHERONE_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(ML_STRING(1548, "Other Camera") + _T(" (") + ML_STRING(1865, "HTTP motion jpeg") + _T(")")), (DWORD)CVideoDeviceDoc::OTHERONE_SP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(ML_STRING(1548, "Other Camera") + _T(" (") + ML_STRING(1866, "HTTP jpeg snapshots") + _T(")")), (DWORD)CVideoDeviceDoc::OTHERONE_CP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("7Links (RTSP)")), (DWORD)CVideoDeviceDoc::SEVENLINKS_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Abus (RTSP)")), (DWORD)CVideoDeviceDoc::ABUS_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("ACTi (RTSP)")), (DWORD)CVideoDeviceDoc::ACTI_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Amcrest (RTSP)")), (DWORD)CVideoDeviceDoc::AMCREST_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Arecont Vision (RTSP)")), (DWORD)CVideoDeviceDoc::ARECONT_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Axis (RTSP)")), (DWORD)CVideoDeviceDoc::AXIS_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Axis (") + ML_STRING(1865, "HTTP motion jpeg") + _T(")")), (DWORD)CVideoDeviceDoc::AXIS_SP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Axis (") + ML_STRING(1866, "HTTP jpeg snapshots") + _T(")")), (DWORD)CVideoDeviceDoc::AXIS_CP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Bosch (RTSP)")), (DWORD)CVideoDeviceDoc::BOSCH_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Canon (RTSP)")), (DWORD)CVideoDeviceDoc::CANON_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("D-Link live1.sdp (RTSP)")), (DWORD)CVideoDeviceDoc::DLINK_LIVE1_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("D-Link play1.sdp (RTSP)")), (DWORD)CVideoDeviceDoc::DLINK_PLAY1_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Dahua (RTSP)")), (DWORD)CVideoDeviceDoc::DAHUA_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Digoo (RTSP)")), (DWORD)CVideoDeviceDoc::DIGOO_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Edimax (RTSP)")), (DWORD)CVideoDeviceDoc::EDIMAX_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Edimax (") + ML_STRING(1865, "HTTP motion jpeg") + _T(")")), (DWORD)CVideoDeviceDoc::EDIMAX_SP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Edimax (") + ML_STRING(1866, "HTTP jpeg snapshots") + _T(")")), (DWORD)CVideoDeviceDoc::EDIMAX_CP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("FalconEye (RTSP)")), (DWORD)CVideoDeviceDoc::FALCONEYE_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Foscam (RTSP)")), (DWORD)CVideoDeviceDoc::FOSCAM_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Foscam (") + ML_STRING(1865, "HTTP motion jpeg") + _T(")")), (DWORD)CVideoDeviceDoc::FOSCAM_SP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Foscam (") + ML_STRING(1866, "HTTP jpeg snapshots") + _T(")")), (DWORD)CVideoDeviceDoc::FOSCAM_CP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Gembird (RTSP)")), (DWORD)CVideoDeviceDoc::GEMBIRD_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("GeoVision (RTSP)")), (DWORD)CVideoDeviceDoc::GEOVISION_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("HiKam (RTSP)")), (DWORD)CVideoDeviceDoc::HIKAM_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Hikvision (RTSP)")), (DWORD)CVideoDeviceDoc::HIKVISION_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Kucam (RTSP)")), (DWORD)CVideoDeviceDoc::KUCAM_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Linksys (RTSP)")), (DWORD)CVideoDeviceDoc::LINKSYS_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Logitech (RTSP)")), (DWORD)CVideoDeviceDoc::LOGITECH_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Monacor 3MP/4MP/5MP/.. Series (RTSP)")), (DWORD)CVideoDeviceDoc::MONACOR_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Monacor 2MP Series (RTSP)")), (DWORD)CVideoDeviceDoc::MONACOR_2MP_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("NexGadget (RTSP)")), (DWORD)CVideoDeviceDoc::NEXGADGET_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Panasonic (RTSP)")), (DWORD)CVideoDeviceDoc::PANASONIC_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Panasonic (") + ML_STRING(1865, "HTTP motion jpeg") + _T(")")), (DWORD)CVideoDeviceDoc::PANASONIC_SP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Panasonic (") + ML_STRING(1866, "HTTP jpeg snapshots") + _T(")")), (DWORD)CVideoDeviceDoc::PANASONIC_CP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Pixord (RTSP)")), (DWORD)CVideoDeviceDoc::PIXORD_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Pixord (") + ML_STRING(1865, "HTTP motion jpeg") + _T(")")), (DWORD)CVideoDeviceDoc::PIXORD_SP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Pixord (") + ML_STRING(1866, "HTTP jpeg snapshots") + _T(")")), (DWORD)CVideoDeviceDoc::PIXORD_CP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Planet (RTSP)")), (DWORD)CVideoDeviceDoc::PLANET_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Samsung (RTSP)")), (DWORD)CVideoDeviceDoc::SAMSUNG_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Sony (RTSP)")), (DWORD)CVideoDeviceDoc::SONY_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Sricam (RTSP)")), (DWORD)CVideoDeviceDoc::SRICAM_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Toshiba (RTSP)")), (DWORD)CVideoDeviceDoc::TOSHIBA_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("TP-Link (RTSP)")), (DWORD)CVideoDeviceDoc::TPLINK_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("TP-Link (") + ML_STRING(1865, "HTTP motion jpeg") + _T(")")), (DWORD)CVideoDeviceDoc::TPLINK_SP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("TP-Link (") + ML_STRING(1866, "HTTP jpeg snapshots") + _T(")")), (DWORD)CVideoDeviceDoc::TPLINK_CP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("TrendNet (RTSP)")), (DWORD)CVideoDeviceDoc::TRENDNET_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Ubiquiti (RTSP)")), (DWORD)CVideoDeviceDoc::UBIQUITI_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Uokoo (RTSP)")), (DWORD)CVideoDeviceDoc::UOKOO_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Vivotek (RTSP)")), (DWORD)CVideoDeviceDoc::VIVOTEK_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("VStarCam (RTSP)")), (DWORD)CVideoDeviceDoc::VSTARCAM_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Wansview (RTSP)")), (DWORD)CVideoDeviceDoc::WANSVIEW_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Xiaomi (RTSP)")), (DWORD)CVideoDeviceDoc::XIAOMI_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Y-cam (RTSP)")), (DWORD)CVideoDeviceDoc::YCAM_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Zavio (RTSP)")), (DWORD)CVideoDeviceDoc::ZAVIO_RTSP);
	pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(_T("Zmodo (RTSP)")), (DWORD)CVideoDeviceDoc::ZMODO_RTSP);

	CDialog::OnInitDialog();
	
	// Init
	LoadHistoryAndSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHostPortDlg::ParseUrl(const CString& sInHost,
							int nInPort,
							int nInDeviceTypeMode,
							CString& sOutGetFrameVideoHost,
							int& nOutGetFrameVideoPort,
							CString& sOutGetFrameLocation,
							int& nOutDeviceTypeMode)
{
	// Init Vars
	int nPos, nPosEnd;
	BOOL bUrlHttp = FALSE;
	BOOL bUrlRtsp = FALSE;
	int nUrlPort;
	sOutGetFrameVideoHost = sInHost;
	CString sGetFrameVideoHostLowerCase(sOutGetFrameVideoHost);
	sGetFrameVideoHostLowerCase.MakeLower();

	// Numeric IP6 with format http://[ip6%interfacenum]:port/framelocation
	if ((nPos = sGetFrameVideoHostLowerCase.Find(_T("http://["))) >= 0)
	{
		// Set flag
		bUrlHttp = TRUE;
		nUrlPort = DEFAULT_HTTP_PORT;

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
			sOutGetFrameLocation = sLocation.Right(sLocation.GetLength() - nPos);
			sOutGetFrameLocation.TrimLeft();
			sOutGetFrameLocation.TrimRight();
		}
		else
			sOutGetFrameLocation = _T("/");
	}
	// Numeric IP4 or hostname with format http://host:port/framelocation
	else if ((nPos = sGetFrameVideoHostLowerCase.Find(_T("http://"))) >= 0)
	{
		// Set flag
		bUrlHttp = TRUE;
		nUrlPort = DEFAULT_HTTP_PORT;

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
			sOutGetFrameLocation = sOutGetFrameVideoHost.Right(sOutGetFrameVideoHost.GetLength() - nPos);
			sOutGetFrameVideoHost = sOutGetFrameVideoHost.Left(nPos);
			sOutGetFrameLocation.TrimLeft();
			sOutGetFrameLocation.TrimRight();
		}
		else
			sOutGetFrameLocation = _T("/");
	}
	else if ((nPos = sGetFrameVideoHostLowerCase.Find(_T("rtsp://"))) >= 0)
	{
		// Set flags
		bUrlRtsp = TRUE;
		nUrlPort = DEFAULT_RTSP_PORT;

		// Remove leading rtsp:// from url
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
			sOutGetFrameLocation = sOutGetFrameVideoHost.Right(sOutGetFrameVideoHost.GetLength() - nPos);
			sOutGetFrameVideoHost = sOutGetFrameVideoHost.Left(nPos);
			sOutGetFrameLocation.TrimLeft();
			sOutGetFrameLocation.TrimRight();
		}
		else
			sOutGetFrameLocation = _T("/");
	}
	else
		sOutGetFrameLocation = _T("/");

	// Set vars
	sOutGetFrameVideoHost.TrimLeft();
	sOutGetFrameVideoHost.TrimRight();
	nOutGetFrameVideoPort = (bUrlHttp || bUrlRtsp) ? nUrlPort : nInPort;
	if (bUrlHttp)
		nOutDeviceTypeMode = CVideoDeviceDoc::OTHERONE_CP;
	else if (bUrlRtsp)
		nOutDeviceTypeMode = CVideoDeviceDoc::URL_RTSP;
	else
		nOutDeviceTypeMode = nInDeviceTypeMode;
}

int CHostPortDlg::SelectionToDeviceTypeMode()
{
	CComboBox* pComboBoxDevTypeMode = (CComboBox*)GetDlgItem(IDC_COMBO_DEVICETYPEMODE);
	int nCurSel = pComboBoxDevTypeMode->GetCurSel();
	if (nCurSel == CB_ERR)
		nCurSel = 0; // fallback to first entry
	return pComboBoxDevTypeMode->GetItemData(nCurSel);
}

void CHostPortDlg::DeviceTypeModeToSelection(int nDeviceTypeMode)
{
	CComboBox* pComboBoxDevTypeMode = (CComboBox*)GetDlgItem(IDC_COMBO_DEVICETYPEMODE);
	for (int i = 0; i < pComboBoxDevTypeMode->GetCount(); i++)
	{
		if (nDeviceTypeMode == pComboBoxDevTypeMode->GetItemData(i))
		{
			pComboBoxDevTypeMode->SetCurSel(i);
			return;
		}
	}
	pComboBoxDevTypeMode->SetCurSel(0); // fallback to first entry
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
	sHostLowerCase.Trim();
	sHostLowerCase.MakeLower();

	// Disable / Enable
	CEdit* pEditPort = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
	CStatic* pStaticRtspPorts = (CStatic*)GetDlgItem(IDC_STATIC_RTSP_PORTS);
	CStatic* pStaticHttpPorts = (CStatic*)GetDlgItem(IDC_STATIC_HTTP_PORTS);
	CComboBox* pComboBoxDevTypeMode = (CComboBox*)GetDlgItem(IDC_COMBO_DEVICETYPEMODE);
	CStatic* pStaticRtsp = (CStatic*)GetDlgItem(IDC_STATIC_RTSP);
	CStatic* pStaticServerPush = (CStatic*)GetDlgItem(IDC_STATIC_SERVERPUSH);
	CStatic* pStaticClientPoll = (CStatic*)GetDlgItem(IDC_STATIC_CLIENTPOLL);
	if (sHostLowerCase.Find(_T("http://")) >= 0 || sHostLowerCase.Find(_T("rtsp://")) >= 0)
	{
		pEditPort->EnableWindow(FALSE);
		pStaticRtspPorts->EnableWindow(FALSE);
		pStaticHttpPorts->EnableWindow(FALSE);
		pComboBoxDevTypeMode->EnableWindow(FALSE);
		pStaticRtsp->EnableWindow(FALSE);
		pStaticServerPush->EnableWindow(FALSE);
		pStaticClientPoll->EnableWindow(FALSE);
	}
	else
	{
		pEditPort->EnableWindow(TRUE);
		pStaticRtspPorts->EnableWindow(TRUE);
		pStaticHttpPorts->EnableWindow(TRUE);
		pComboBoxDevTypeMode->EnableWindow(TRUE);
		pStaticRtsp->EnableWindow(TRUE);
		pStaticServerPush->EnableWindow(TRUE);
		pStaticClientPoll->EnableWindow(TRUE);
	}
}

void CHostPortDlg::OnEditchangeComboHost()
{
	CComboBox* pComboBoxHost = (CComboBox*)GetDlgItem(IDC_COMBO_HOST);
	pComboBoxHost->GetWindowText(m_sHost);
	EnableDisableCtrls();
	Load();
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
	if (nSel >= 0 && nSel < m_DeviceTypeModesHistory.GetSize())
	{
		m_nDeviceTypeMode = m_DeviceTypeModesHistory[nSel];
		DeviceTypeModeToSelection(m_nDeviceTypeMode);
	}

	// Update Controls
	EnableDisableCtrls();
	Load();
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
		m_nPort = DEFAULT_RTSP_PORT;
	Load();
}

void CHostPortDlg::OnSelchangeComboDeviceTypeMode()
{
	m_nDeviceTypeMode = SelectionToDeviceTypeMode();
	Load();
}

void CHostPortDlg::OnError()
{
	CComboBox* pComboBoxHost = (CComboBox*)GetDlgItem(IDC_COMBO_HOST);
	::SetFocus(pComboBoxHost->GetSafeHwnd());
	::AlertUser(GetSafeHwnd());
	::AfxGetMainFrame()->PopupToaster(APPNAME_NOEXT, ML_STRING(1867, "Enter an IP or a Hostname or an URL starting with rtsp:// or http:// (ATTENTION: User Name and Password must be provided under Camera Login, not in the URL)"), 0);
}

void CHostPortDlg::OnOK() 
{
	CString sHostLowerCase(m_sHost);
	sHostLowerCase.Trim();
	sHostLowerCase.MakeLower();
	
	// Empty?
	if (sHostLowerCase.IsEmpty())
		return OnError(); // empty!
	// Url?
	else if (sHostLowerCase.Find(_T("http://")) >= 0 || sHostLowerCase.Find(_T("rtsp://")) >= 0)
		
	{
		int nPos = sHostLowerCase.Find(_T("://"));
		nPos += 3;
		int nPosEnd = sHostLowerCase.Find(_T("/"), nPos);
		if (nPosEnd < 0)
			nPosEnd = sHostLowerCase.GetLength();
		CString sHostOnly(sHostLowerCase.Mid(nPos, nPosEnd - nPos));
		if (sHostOnly.Find(_T("@")) >= 0)
			return OnError(); // no credentials in URL!
	}
	// Not Url?
	else
	{
		if (sHostLowerCase.Find(_T(":")) >= 0 || sHostLowerCase.Find(_T("/")) >= 0)
			return OnError(); // missing protocol!
	}

	// OK
	SaveHistory(m_sHost, m_nPort, m_nDeviceTypeMode,
				m_HostsHistory, m_PortsHistory, m_DeviceTypeModesHistory);
	Save();
	CDialog::OnOK();
}

CString CHostPortDlg::MakeDevicePathName(const CString& sInHost, int nInPort, int nInDeviceTypeMode)
{
	CString sOutGetFrameVideoHost;
	int nOutGetFrameVideoPort;
	CString sOutGetFrameLocation;
	int nOutDeviceTypeMode;
	ParseUrl(sInHost, nInPort, nInDeviceTypeMode,
			sOutGetFrameVideoHost, nOutGetFrameVideoPort,
			sOutGetFrameLocation, nOutDeviceTypeMode);
	return CVideoDeviceDoc::GetNetworkDevicePathName(	sOutGetFrameVideoHost, nOutGetFrameVideoPort,
														sOutGetFrameLocation, nOutDeviceTypeMode);
}

void CHostPortDlg::Load()
{
	// Get device path name
	CString sDevicePathName = MakeDevicePathName(m_sHost, m_nPort, m_nDeviceTypeMode);

	// Check whether section exists because the get profile functions call GetSectionKey()
	// which creates the key if it doesn't exist. We do not want a new key for each typed char!
	BOOL bSectionExists = ((CUImagerApp*)::AfxGetApp())->IsExistingSection(sDevicePathName);

	// Load
	if (bSectionExists)
	{
		CString	sUsername = ((CUImagerApp*)::AfxGetApp())->GetSecureProfileString(sDevicePathName, _T("HTTPGetFrameUsernameExportable"));
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
		pEdit->SetWindowText(sUsername);
		CString	sPassword = ((CUImagerApp*)::AfxGetApp())->GetSecureProfileString(sDevicePathName, _T("HTTPGetFramePasswordExportable"));
		pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
		pEdit->SetWindowText(sPassword);
		BOOL bPreferTcpforRtsp = (BOOL)((CUImagerApp*)::AfxGetApp())->GetProfileInt(sDevicePathName, _T("PreferTcpforRtsp"), FALSE);
		CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_PREFER_TCP_FOR_RTSP);
		pCheck->SetCheck(bPreferTcpforRtsp ? 1 : 0);
	}

	// Update dialog title
	CString	sName;
	if (bSectionExists)
	{
		sName = ((CUImagerApp*)::AfxGetApp())->GetProfileString(sDevicePathName, _T("RecordAutoSaveDir"), _T(""));
		sName.TrimRight(_T('\\'));
		int index = sName.ReverseFind(_T('\\'));
		if (index >= 0)
			sName = sName.Mid(index + 1);
	}
	if (sName.IsEmpty())
		SetWindowText(m_sInitialDlgTitle);
	else
		SetWindowText(m_sInitialDlgTitle + _T(" ") + sName);
}

void CHostPortDlg::Save()
{
	// Get device path name
	CString sDevicePathName = MakeDevicePathName(m_sHost, m_nPort, m_nDeviceTypeMode);

	// Store
	CString sText;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->GetWindowText(sText);
	((CUImagerApp*)::AfxGetApp())->WriteSecureProfileString(sDevicePathName, _T("HTTPGetFrameUsernameExportable"), sText);
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->GetWindowText(sText);
	((CUImagerApp*)::AfxGetApp())->WriteSecureProfileString(sDevicePathName, _T("HTTPGetFramePasswordExportable"), sText);
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_PREFER_TCP_FOR_RTSP);
	((CUImagerApp*)::AfxGetApp())->WriteProfileInt(sDevicePathName, _T("PreferTcpforRtsp"), pCheck->GetCheck());
}

void CHostPortDlg::LoadHistoryAndSel(int nSel)
{
	// Load History
	m_HostsHistory.RemoveAll();
	m_PortsHistory.RemoveAll();
	m_DeviceTypeModesHistory.RemoveAll();
	LoadHistory(m_HostsHistory, m_PortsHistory, m_DeviceTypeModesHistory);

	// If empty add an item!
	if (m_HostsHistory.GetSize() <= 0)
	{
		m_HostsHistory.InsertAt(0, _T(""));
		m_PortsHistory.InsertAt(0, (DWORD)DEFAULT_RTSP_PORT);
		m_DeviceTypeModesHistory.InsertAt(0, (DWORD)CVideoDeviceDoc::OTHERONE_RTSP);
	}

	// Correct the selection
	if (nSel < 0)
		nSel = 0;
	else if (nSel >= m_HostsHistory.GetSize())
		nSel = m_HostsHistory.GetSize() - 1;

	// Populate hosts and select current
	CComboBox* pComboBoxHost = (CComboBox*)GetDlgItem(IDC_COMBO_HOST);
	pComboBoxHost->ResetContent();
	for (int i = 0; i < m_HostsHistory.GetSize(); i++)
		pComboBoxHost->AddString(m_HostsHistory[i]);
	pComboBoxHost->SetCurSel(nSel);

	// Current Host
	m_sHost = m_HostsHistory[nSel];

	// Current Port
	m_nPort = m_PortsHistory[nSel];
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
	CString sPort;
	sPort.Format(_T("%i"), m_nPort);
	pEdit->SetWindowText(sPort);

	// Current Device Type Mode
	m_nDeviceTypeMode = m_DeviceTypeModesHistory[nSel];
	DeviceTypeModeToSelection(m_nDeviceTypeMode);

	// Update Controls
	EnableDisableCtrls();
	Load();
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
			dwPort = (DWORD)pApp->GetProfileInt(sSection, sPortEntry, 0xFFFFFFFF);
			if (dwPort == 0 || dwPort > 65535) // Port 0 is Reserved
				dwPort = DEFAULT_RTSP_PORT;
			PortsHistory.Add(dwPort);

			// Device Type and Mode
			CString sDeviceTypeModeEntry;
			sDeviceTypeModeEntry.Format(_T("DeviceTypeModeHistory%d"), i);
			dwDeviceTypeMode = (DWORD)pApp->GetProfileInt(sSection, sDeviceTypeModeEntry, CVideoDeviceDoc::OTHERONE_RTSP);
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