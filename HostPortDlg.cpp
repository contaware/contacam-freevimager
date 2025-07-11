// HostPortDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "HostPortDlg.h"
#include "VideoDeviceDoc.h"
#include "ToolBarChildFrm.h"
#include "SortableStringArray.h" // for the CompareNatural() function

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
	// Load last selected m_sHost, m_nPort, m_nDeviceTypeMode
	CWinApp* pApp = ::AfxGetApp();
	CString sSection(_T("HostPortDlg"));

	// Host
	m_sHost = pApp->GetProfileString(sSection, _T("HostHistory0"), _T(""));

	// Port
	DWORD dwPort = (DWORD)pApp->GetProfileInt(sSection, _T("PortHistory0"), 0xFFFFFFFF);
	if (dwPort == 0 || dwPort > 65535) // Port 0 is Reserved
		dwPort = RTSP_DEFAULT_PORT;
	m_nPort = dwPort;

	// Device Type and Mode
	m_nDeviceTypeMode = (DWORD)pApp->GetProfileInt(sSection, _T("DeviceTypeModeHistory0"), CVideoDeviceDoc::GENERIC_1_RTSP);
}

BEGIN_MESSAGE_MAP(CHostPortDlg, CDialog)
	//{{AFX_MSG_MAP(CHostPortDlg)
	ON_CBN_SELCHANGE(IDC_COMBO_HOST, OnSelchangeComboHost)
	ON_CBN_EDITCHANGE(IDC_COMBO_HOST, OnEditchangeComboHost)
	ON_EN_CHANGE(IDC_EDIT_PORT, OnChangeEditPort)
	ON_CBN_SELCHANGE(IDC_COMBO_DEVICETYPEMODE, OnSelchangeComboDeviceTypeMode)
	ON_BN_CLICKED(IDC_CHECK_PREFER_TCP_FOR_RTSP, OnCheckPreferTcpforRtsp)
	ON_BN_CLICKED(IDC_CHECK_UDP_MULTICAST_FOR_RTSP, OnCheckUdpMulticastforRtsp)
	ON_BN_CLICKED(IDC_BUTTON_HELP, OnButtonHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CHostPortDlg::InitDeviceTypeModeArrayForCB(CArray<CDeviceTypeModePair>& a)
{
	// Clear
	a.RemoveAll();

	// Freely order the following pairs
	a.Add(CDeviceTypeModePair(ML_STRING(1548, "Generic") + _T(" /11 (RTSP)"), (DWORD)CVideoDeviceDoc::GENERIC_1_RTSP));
	a.Add(CDeviceTypeModePair(ML_STRING(1548, "Generic") + _T(" /onvif1 (RTSP)"), (DWORD)CVideoDeviceDoc::GENERIC_2_RTSP));
	a.Add(CDeviceTypeModePair(ML_STRING(1548, "Generic") + _T(" /live/ch0 (RTSP)"), (DWORD)CVideoDeviceDoc::GENERIC_3_RTSP));
	a.Add(CDeviceTypeModePair(ML_STRING(1548, "Generic") + _T(" /ch01.264 (RTSP)"), (DWORD)CVideoDeviceDoc::GENERIC_4_RTSP));
	a.Add(CDeviceTypeModePair(ML_STRING(1548, "Generic") + _T(" /channel=1&stream=0.sdp (RTSP)"), (DWORD)CVideoDeviceDoc::GENERIC_5_RTSP));
	a.Add(CDeviceTypeModePair(ML_STRING(1548, "Generic") + _T(" /channel=1_stream=0.sdp (RTSP)"), (DWORD)CVideoDeviceDoc::GENERIC_6_RTSP));
	a.Add(CDeviceTypeModePair(ML_STRING(1548, "Generic") + _T(" (") + ML_STRING(1865, "HTTP motion jpeg") + _T(")"), (DWORD)CVideoDeviceDoc::GENERIC_SP));
	a.Add(CDeviceTypeModePair(ML_STRING(1548, "Generic") + _T(" (") + ML_STRING(1866, "HTTP jpeg snapshots") + _T(")"), (DWORD)CVideoDeviceDoc::GENERIC_CP));
	a.Add(CDeviceTypeModePair(_T("7Links (RTSP)"), (DWORD)CVideoDeviceDoc::SEVENLINKS_RTSP));
	a.Add(CDeviceTypeModePair(_T("Abus (RTSP)"), (DWORD)CVideoDeviceDoc::ABUS_RTSP));
	a.Add(CDeviceTypeModePair(_T("ACTi (RTSP)"), (DWORD)CVideoDeviceDoc::ACTI_RTSP));
	a.Add(CDeviceTypeModePair(_T("Amcrest (RTSP)"), (DWORD)CVideoDeviceDoc::AMCREST_RTSP));
	a.Add(CDeviceTypeModePair(_T("Arecont Vision (RTSP)"), (DWORD)CVideoDeviceDoc::ARECONT_RTSP));
	a.Add(CDeviceTypeModePair(_T("Axis (RTSP)"), (DWORD)CVideoDeviceDoc::AXIS_RTSP));
	a.Add(CDeviceTypeModePair(_T("Axis (") + ML_STRING(1865, "HTTP motion jpeg") + _T(")"), (DWORD)CVideoDeviceDoc::AXIS_SP));
	a.Add(CDeviceTypeModePair(_T("Axis (") + ML_STRING(1866, "HTTP jpeg snapshots") + _T(")"), (DWORD)CVideoDeviceDoc::AXIS_CP));
	a.Add(CDeviceTypeModePair(_T("Bosch (RTSP)"), (DWORD)CVideoDeviceDoc::BOSCH_RTSP));
	a.Add(CDeviceTypeModePair(_T("Canon (RTSP)"), (DWORD)CVideoDeviceDoc::CANON_RTSP));
	a.Add(CDeviceTypeModePair(_T("D-Link /live1.sdp (RTSP)"), (DWORD)CVideoDeviceDoc::DLINK_LIVE1_RTSP));
	a.Add(CDeviceTypeModePair(_T("D-Link /play1.sdp (RTSP)"), (DWORD)CVideoDeviceDoc::DLINK_PLAY1_RTSP));
	a.Add(CDeviceTypeModePair(_T("Dahua (RTSP)"), (DWORD)CVideoDeviceDoc::DAHUA_RTSP));
	a.Add(CDeviceTypeModePair(_T("Dekco (RTSP)"), (DWORD)CVideoDeviceDoc::DEKCO_RTSP));
	a.Add(CDeviceTypeModePair(_T("Dericam (RTSP)"), (DWORD)CVideoDeviceDoc::DERICAM_RTSP));
	a.Add(CDeviceTypeModePair(_T("Digoo (RTSP)"), (DWORD)CVideoDeviceDoc::DIGOO_RTSP));
	a.Add(CDeviceTypeModePair(_T("DroidCam(X) (") + ML_STRING(1865, "HTTP motion jpeg") + _T(")"), (DWORD)CVideoDeviceDoc::DROIDCAM_SP));
	a.Add(CDeviceTypeModePair(_T("Edimax H.264 (RTSP)"), (DWORD)CVideoDeviceDoc::EDIMAX_H264_RTSP));
	a.Add(CDeviceTypeModePair(_T("Edimax MPEG4 (RTSP)"), (DWORD)CVideoDeviceDoc::EDIMAX_MPEG4_RTSP));
	a.Add(CDeviceTypeModePair(_T("Edimax (") + ML_STRING(1865, "HTTP motion jpeg") + _T(")"), (DWORD)CVideoDeviceDoc::EDIMAX_SP));
	a.Add(CDeviceTypeModePair(_T("Edimax (") + ML_STRING(1866, "HTTP jpeg snapshots") + _T(")"), (DWORD)CVideoDeviceDoc::EDIMAX_CP));
	a.Add(CDeviceTypeModePair(_T("Escam (RTSP)"), (DWORD)CVideoDeviceDoc::ESCAM_RTSP));
	a.Add(CDeviceTypeModePair(_T("Ezviz /H.264 (RTSP)"), (DWORD)CVideoDeviceDoc::EZVIZ_H264_RTSP));
	a.Add(CDeviceTypeModePair(_T("Ezviz /h264_stream (RTSP)"), (DWORD)CVideoDeviceDoc::EZVIZ_H264STREAM_RTSP));
	a.Add(CDeviceTypeModePair(_T("FalconEye (RTSP)"), (DWORD)CVideoDeviceDoc::FALCONEYE_RTSP));
	a.Add(CDeviceTypeModePair(_T("Floureon (RTSP)"), (DWORD)CVideoDeviceDoc::FLOUREON_RTSP));
	a.Add(CDeviceTypeModePair(_T("Foscam (RTSP)"), (DWORD)CVideoDeviceDoc::FOSCAM_RTSP));
	a.Add(CDeviceTypeModePair(_T("Foscam (") + ML_STRING(1865, "HTTP motion jpeg") + _T(")"), (DWORD)CVideoDeviceDoc::FOSCAM_SP));
	a.Add(CDeviceTypeModePair(_T("Foscam (") + ML_STRING(1866, "HTTP jpeg snapshots") + _T(")"), (DWORD)CVideoDeviceDoc::FOSCAM_CP));
	a.Add(CDeviceTypeModePair(_T("Gembird (RTSP)"), (DWORD)CVideoDeviceDoc::GEMBIRD_RTSP));
	a.Add(CDeviceTypeModePair(_T("GeoVision (RTSP)"), (DWORD)CVideoDeviceDoc::GEOVISION_RTSP));
	a.Add(CDeviceTypeModePair(_T("HeimVision (RTSP)"), (DWORD)CVideoDeviceDoc::HEIMVISION_RTSP));
	a.Add(CDeviceTypeModePair(_T("HiKam (RTSP)"), (DWORD)CVideoDeviceDoc::HIKAM_RTSP));
	a.Add(CDeviceTypeModePair(_T("Hikvision (RTSP)"), (DWORD)CVideoDeviceDoc::HIKVISION_RTSP));
	a.Add(CDeviceTypeModePair(_T("Hikvision old (RTSP)"), (DWORD)CVideoDeviceDoc::HIKVISION_OLD_RTSP));
	a.Add(CDeviceTypeModePair(_T("IP Webcam (Pro) for Android (RTSP)"), (DWORD)CVideoDeviceDoc::IPWEBCAM_ANDROID_RTSP));
	a.Add(CDeviceTypeModePair(_T("JideTech (RTSP)"), (DWORD)CVideoDeviceDoc::JIDETECH_RTSP));
	a.Add(CDeviceTypeModePair(_T("Kucam (RTSP)"), (DWORD)CVideoDeviceDoc::KUCAM_RTSP));
	a.Add(CDeviceTypeModePair(_T("Linksys (RTSP)"), (DWORD)CVideoDeviceDoc::LINKSYS_RTSP));
	a.Add(CDeviceTypeModePair(_T("Logitech (RTSP)"), (DWORD)CVideoDeviceDoc::LOGITECH_RTSP));
	a.Add(CDeviceTypeModePair(_T("Lorex (RTSP)"), (DWORD)CVideoDeviceDoc::LOREX_RTSP));
	a.Add(CDeviceTypeModePair(_T("Monacor 3MP/4MP/5MP/.. Series (RTSP)"), (DWORD)CVideoDeviceDoc::MONACOR_RTSP));
	a.Add(CDeviceTypeModePair(_T("Monacor 2MP Series (RTSP)"), (DWORD)CVideoDeviceDoc::MONACOR_2MP_RTSP));
	a.Add(CDeviceTypeModePair(_T("NexGadget (RTSP)"), (DWORD)CVideoDeviceDoc::NEXGADGET_RTSP));
	a.Add(CDeviceTypeModePair(_T("Panasonic (RTSP)"), (DWORD)CVideoDeviceDoc::PANASONIC_RTSP));
	a.Add(CDeviceTypeModePair(_T("Panasonic (") + ML_STRING(1865, "HTTP motion jpeg") + _T(")"), (DWORD)CVideoDeviceDoc::PANASONIC_SP));
	a.Add(CDeviceTypeModePair(_T("Panasonic (") + ML_STRING(1866, "HTTP jpeg snapshots") + _T(")"), (DWORD)CVideoDeviceDoc::PANASONIC_CP));
	a.Add(CDeviceTypeModePair(_T("Pixord (RTSP)"), (DWORD)CVideoDeviceDoc::PIXORD_RTSP));
	a.Add(CDeviceTypeModePair(_T("Pixord (") + ML_STRING(1865, "HTTP motion jpeg") + _T(")"), (DWORD)CVideoDeviceDoc::PIXORD_SP));
	a.Add(CDeviceTypeModePair(_T("Pixord (") + ML_STRING(1866, "HTTP jpeg snapshots") + _T(")"), (DWORD)CVideoDeviceDoc::PIXORD_CP));
	a.Add(CDeviceTypeModePair(_T("Planet (RTSP)"), (DWORD)CVideoDeviceDoc::PLANET_RTSP));
	a.Add(CDeviceTypeModePair(_T("Reolink H.265 (RTSP)"), (DWORD)CVideoDeviceDoc::REOLINK_H265_RTSP));
	a.Add(CDeviceTypeModePair(_T("Reolink H.264 (RTSP)"), (DWORD)CVideoDeviceDoc::REOLINK_H264_RTSP));
	a.Add(CDeviceTypeModePair(_T("Samsung/Wisenet/Hanwha profile5 (RTSP)"), (DWORD)CVideoDeviceDoc::SAMSUNG_PROFILE5_RTSP));
	a.Add(CDeviceTypeModePair(_T("Samsung/Wisenet/Hanwha profile4 (RTSP)"), (DWORD)CVideoDeviceDoc::SAMSUNG_PROFILE4_RTSP));
	a.Add(CDeviceTypeModePair(_T("Samsung/Wisenet/Hanwha profile3 (RTSP)"), (DWORD)CVideoDeviceDoc::SAMSUNG_PROFILE3_RTSP));
	a.Add(CDeviceTypeModePair(_T("Samsung/Wisenet/Hanwha profile2 (RTSP)"), (DWORD)CVideoDeviceDoc::SAMSUNG_PROFILE2_RTSP));
	a.Add(CDeviceTypeModePair(_T("Samsung/Wisenet/Hanwha profile1 (RTSP)"), (DWORD)CVideoDeviceDoc::SAMSUNG_PROFILE1_RTSP));
	a.Add(CDeviceTypeModePair(_T("Sony (RTSP)"), (DWORD)CVideoDeviceDoc::SONY_RTSP));
	a.Add(CDeviceTypeModePair(_T("Sricam (RTSP)"), (DWORD)CVideoDeviceDoc::SRICAM_RTSP));
	a.Add(CDeviceTypeModePair(_T("Srihome (RTSP)"), (DWORD)CVideoDeviceDoc::SRIHOME_RTSP));
	a.Add(CDeviceTypeModePair(_T("Sumpple (RTSP)"), (DWORD)CVideoDeviceDoc::SUMPPLE_RTSP));
	a.Add(CDeviceTypeModePair(_T("Toshiba (RTSP)"), (DWORD)CVideoDeviceDoc::TOSHIBA_RTSP));
	a.Add(CDeviceTypeModePair(_T("TP-Link (RTSP)"), (DWORD)CVideoDeviceDoc::TPLINK_RTSP));
	a.Add(CDeviceTypeModePair(_T("TP-Link NC2XX/NC4XX (RTSP)"), (DWORD)CVideoDeviceDoc::TPLINK_NCXXX_RTSP));
	a.Add(CDeviceTypeModePair(_T("TP-Link Tapo (RTSP)"), (DWORD)CVideoDeviceDoc::TPLINK_TAPO_RTSP));
	a.Add(CDeviceTypeModePair(_T("TP-Link (") + ML_STRING(1865, "HTTP motion jpeg") + _T(")"), (DWORD)CVideoDeviceDoc::TPLINK_SP));
	a.Add(CDeviceTypeModePair(_T("TP-Link (") + ML_STRING(1866, "HTTP jpeg snapshots") + _T(")"), (DWORD)CVideoDeviceDoc::TPLINK_CP));
	a.Add(CDeviceTypeModePair(_T("TrendNet (RTSP)"), (DWORD)CVideoDeviceDoc::TRENDNET_RTSP));
	a.Add(CDeviceTypeModePair(_T("Ubiquiti (RTSP)"), (DWORD)CVideoDeviceDoc::UBIQUITI_RTSP));
	a.Add(CDeviceTypeModePair(_T("Uokoo (RTSP)"), (DWORD)CVideoDeviceDoc::UOKOO_RTSP));
	a.Add(CDeviceTypeModePair(_T("Vivotek (RTSP)"), (DWORD)CVideoDeviceDoc::VIVOTEK_RTSP));
	a.Add(CDeviceTypeModePair(_T("VStarCam (RTSP)"), (DWORD)CVideoDeviceDoc::VSTARCAM_RTSP));
	a.Add(CDeviceTypeModePair(_T("Wanscam (RTSP)"), (DWORD)CVideoDeviceDoc::WANSCAM_RTSP));
	a.Add(CDeviceTypeModePair(_T("Wansview (RTSP)"), (DWORD)CVideoDeviceDoc::WANSVIEW_RTSP));
	a.Add(CDeviceTypeModePair(_T("Wyze (RTSP)"), (DWORD)CVideoDeviceDoc::WYZE_RTSP));
	a.Add(CDeviceTypeModePair(_T("Xiaomi (RTSP)"), (DWORD)CVideoDeviceDoc::XIAOMI_RTSP));
	a.Add(CDeviceTypeModePair(_T("Y-cam (RTSP)"), (DWORD)CVideoDeviceDoc::YCAM_RTSP));
	a.Add(CDeviceTypeModePair(_T("Yudor (RTSP)"), (DWORD)CVideoDeviceDoc::YUDOR_RTSP));
	a.Add(CDeviceTypeModePair(_T("Zavio (RTSP)"), (DWORD)CVideoDeviceDoc::ZAVIO_RTSP));
	a.Add(CDeviceTypeModePair(_T("Zavio H.264 (RTSP)"), (DWORD)CVideoDeviceDoc::ZAVIO_H264_RTSP));
	a.Add(CDeviceTypeModePair(_T("Zavio MPEG4 (RTSP)"), (DWORD)CVideoDeviceDoc::ZAVIO_MPEG4_RTSP));
	a.Add(CDeviceTypeModePair(_T("Zmodo (RTSP)"), (DWORD)CVideoDeviceDoc::ZMODO_RTSP));
}

BOOL CHostPortDlg::OnInitDialog()
{
	// Store initial dialog title
	GetWindowText(m_sInitialDlgTitle);

	// Init Device Type Mode Combo Box
	CComboBox* pComboBoxDevTypeMode = (CComboBox*)GetDlgItem(IDC_COMBO_DEVICETYPEMODE);
	CArray<CDeviceTypeModePair> DeviceTypeModeArrayForCB;
	InitDeviceTypeModeArrayForCB(DeviceTypeModeArrayForCB);
	for (int nModeIndex = 0; nModeIndex < DeviceTypeModeArrayForCB.GetSize(); nModeIndex++)
	{
		CDeviceTypeModePair& pair = DeviceTypeModeArrayForCB.ElementAt(nModeIndex);
		pComboBoxDevTypeMode->SetItemData(pComboBoxDevTypeMode->AddString(pair.m_sMode), pair.m_dwMode);
	}

	CDialog::OnInitDialog();
	
	// Loop through all devices configured in registry or ini file
	CStringArray DevicePathNames;
	((CUImagerApp*)::AfxGetApp())->EnumConfiguredDevicePathNames(DevicePathNames);
	for (int nDeviceIndex = 0; nDeviceIndex < DevicePathNames.GetSize(); nDeviceIndex++)
	{
		CString sIntermediateGetFrameVideoHost;
		int nIntermediateGetFrameVideoPort;
		CString sIntermediateGetFrameLocation;
		CVideoDeviceDoc::NetworkDeviceTypeMode nIntermediateDeviceTypeMode;
		if (CVideoDeviceDoc::ParseNetworkDevicePathName(DevicePathNames[nDeviceIndex],
														sIntermediateGetFrameVideoHost,
														nIntermediateGetFrameVideoPort,
														sIntermediateGetFrameLocation,
														nIntermediateDeviceTypeMode))
		{
			CString sOutHost;
			int nOutPort;
			int nOutDeviceTypeMode;
			MakeUrl(sIntermediateGetFrameVideoHost,
					nIntermediateGetFrameVideoPort,
					sIntermediateGetFrameLocation,
					nIntermediateDeviceTypeMode,
					sOutHost,
					nOutPort,
					nOutDeviceTypeMode);
			if (!sOutHost.IsEmpty())
			{
				// Avoid duplicated hosts
				// (the same IP/hostname may occur multiple times
				//  with different port and/or device type/mode)
				// Attention: CComboBox does not support duplicated items, moreover
				//            a control with identical named items is not intuitive!
				BOOL bDoInsert = TRUE;
				for (int nHostIndex = 0; nHostIndex < m_Hosts.GetSize(); nHostIndex++)
				{
					if (m_Hosts[nHostIndex] == sOutHost)
					{
						// Prefer the one with the autorun flag set
						if (CVideoDeviceDoc::AutorunGetDeviceKey(DevicePathNames[nDeviceIndex]) != _T(""))
						{
							m_Hosts[nHostIndex] = sOutHost;
							m_Ports[nHostIndex] = nOutPort;
							m_DeviceTypeModes[nHostIndex] = nOutDeviceTypeMode;
						}
						bDoInsert = FALSE; // no insertion for duplicated hosts
						break;
					}
				}

				// Insertion sort
				if (bDoInsert)
				{
					for (int nHostIndex = 0; nHostIndex < m_Hosts.GetSize(); nHostIndex++)
					{
						if (::CompareNatural(&sOutHost, &m_Hosts[nHostIndex]) < 0)
						{
							m_Hosts.InsertAt(nHostIndex, sOutHost);
							m_Ports.InsertAt(nHostIndex, (DWORD)nOutPort);
							m_DeviceTypeModes.InsertAt(nHostIndex, (DWORD)nOutDeviceTypeMode);
							bDoInsert = FALSE;
							break;
						}
					}
					if (bDoInsert)
					{
						m_Hosts.Add(sOutHost);
						m_Ports.Add((DWORD)nOutPort);
						m_DeviceTypeModes.Add((DWORD)nOutDeviceTypeMode);
					}
				}
			}
		}
	}

	// If empty add the default item
	if (m_Hosts.GetSize() <= 0)
	{
		m_Hosts.InsertAt(0, _T(""));
		m_Ports.InsertAt(0, (DWORD)RTSP_DEFAULT_PORT);
		m_DeviceTypeModes.InsertAt(0, (DWORD)CVideoDeviceDoc::GENERIC_1_RTSP);
	}

	// Find the last selected index and populate the hosts combobox
	m_HostComboBox.SubclassDlgItem(IDC_COMBO_HOST, this);
	int nLastSel = 0; // if not found default to first item
	for (int nHostIndex = 0; nHostIndex < m_Hosts.GetSize(); nHostIndex++)
	{
		// Above the duplicated hosts are deleted, it can happen that the
		// chosen m_Ports[nHostIndex] and m_DeviceTypeModes[nHostIndex]
		// are not the ones that were selected last time, correct that here
		if (m_Hosts[nHostIndex] == m_sHost)
		{
			nLastSel = nHostIndex;
			m_Ports[nHostIndex] = m_nPort;
			m_DeviceTypeModes[nHostIndex] = m_nDeviceTypeMode;
		}

		// Fill the list box of the combobox (name is indexed by ItemData)
		m_HostComboBox.SetItemData(	m_HostComboBox.AddString(m_Hosts[nHostIndex]),
									m_HostComboBox.m_NameArray.Add(GetAssignedDeviceName(m_Hosts[nHostIndex],
																						m_Ports[nHostIndex],
																						m_DeviceTypeModes[nHostIndex])));
	}

	// Current Host
	m_sHost = m_Hosts[nLastSel];
	m_HostComboBox.SetCurSel(nLastSel);

	// Current Port
	m_nPort = m_Ports[nLastSel];
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
	CString sPort;
	sPort.Format(_T("%i"), m_nPort);
	pEdit->SetWindowText(sPort);

	// Current Device Type Mode
	m_nDeviceTypeMode = m_DeviceTypeModes[nLastSel];
	DeviceTypeModeToSelection(m_nDeviceTypeMode);

	// Rtsp and Http Port Hints
	CRichEditCtrl* pRichEditRtspPorts = (CRichEditCtrl*)GetDlgItem(IDC_RICHEDIT_RTSP_PORTS);
	CUImagerApp::RichEditCtrlAppendText(pRichEditRtspPorts, _T("RTSP: "), ::GetSysColor(COLOR_WINDOWTEXT));
	CUImagerApp::RichEditCtrlAppendText(pRichEditRtspPorts, _T("554"), ::GetSysColor(COLOR_WINDOWTEXT), TRUE);
	CUImagerApp::RichEditCtrlAppendText(pRichEditRtspPorts, _T(", 10554, 8554, 8080, 88, 80"), ::GetSysColor(COLOR_WINDOWTEXT));
	CRichEditCtrl* pRichEditHttpPorts = (CRichEditCtrl*)GetDlgItem(IDC_RICHEDIT_HTTP_PORTS);
	CUImagerApp::RichEditCtrlAppendText(pRichEditHttpPorts, _T("HTTP: "), ::GetSysColor(COLOR_WINDOWTEXT));
	CUImagerApp::RichEditCtrlAppendText(pRichEditHttpPorts, _T("80"), ::GetSysColor(COLOR_WINDOWTEXT), TRUE);
	CUImagerApp::RichEditCtrlAppendText(pRichEditHttpPorts, _T(", 88"), ::GetSysColor(COLOR_WINDOWTEXT));

	// Update Controls
	EnableDisableCtrls();
	LoadSettings();

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
		nUrlPort = HTTP_DEFAULT_PORT;

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
		nUrlPort = HTTP_DEFAULT_PORT;

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
		// TODO: does ffmpeg support numeric IP6? -> If yes add IP6 handling here!

		// Set flags
		bUrlRtsp = TRUE;
		nUrlPort = RTSP_DEFAULT_PORT;

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
		nOutDeviceTypeMode = CVideoDeviceDoc::GENERIC_CP;
	else if (bUrlRtsp)
		nOutDeviceTypeMode = CVideoDeviceDoc::URL_RTSP;
	else
		nOutDeviceTypeMode = nInDeviceTypeMode;
}

void CHostPortDlg::MakeUrl(	const CString& sInGetFrameVideoHost,
							int nInGetFrameVideoPort,
							const CString& sInGetFrameLocation,
							int nInDeviceTypeMode,
							CString& sOutHost,
							int& nOutPort,
							int& nOutDeviceTypeMode)
{
	// Url
	if (sInGetFrameLocation != _T("/"))
	{
		// RTSP
		if (nInDeviceTypeMode == CVideoDeviceDoc::URL_RTSP)
		{
			// TODO: does ffmpeg support numeric IP6? -> If yes add IP6 handling here!

			if (nInGetFrameVideoPort != RTSP_DEFAULT_PORT)
				sOutHost.Format(_T("rtsp://%s:%d%s"), sInGetFrameVideoHost, nInGetFrameVideoPort, sInGetFrameLocation);
			else
				sOutHost.Format(_T("rtsp://%s%s"), sInGetFrameVideoHost, sInGetFrameLocation);
			nOutPort = nInGetFrameVideoPort;
			nOutDeviceTypeMode = CVideoDeviceDoc::GENERIC_1_RTSP;
		}
		// HTTP
		else
		{
			// IP6
			if (sInGetFrameVideoHost.Find(_T(':')) >= 0)
			{
				if (nInGetFrameVideoPort != HTTP_DEFAULT_PORT)
					sOutHost.Format(_T("http://[%s]:%d%s"), sInGetFrameVideoHost, nInGetFrameVideoPort, sInGetFrameLocation);
				else
					sOutHost.Format(_T("http://[%s]%s"), sInGetFrameVideoHost, sInGetFrameLocation);
			}
			else
			{
				if (nInGetFrameVideoPort != HTTP_DEFAULT_PORT)
					sOutHost.Format(_T("http://%s:%d%s"), sInGetFrameVideoHost, nInGetFrameVideoPort, sInGetFrameLocation);
				else
					sOutHost.Format(_T("http://%s%s"), sInGetFrameVideoHost, sInGetFrameLocation);
			}
			nOutPort = nInGetFrameVideoPort;
			nOutDeviceTypeMode = CVideoDeviceDoc::GENERIC_CP;
		}
	}
	else
	{
		sOutHost = sInGetFrameVideoHost;
		nOutPort = nInGetFrameVideoPort;
		nOutDeviceTypeMode = nInDeviceTypeMode;
	}
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

void CHostPortDlg::EnableDisableCtrls()
{
	CString sHostLowerCase(m_sHost);
	sHostLowerCase.Trim();
	sHostLowerCase.MakeLower();

	// Disable / Enable
	CEdit* pEditPort = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
	CComboBox* pComboBoxDevTypeMode = (CComboBox*)GetDlgItem(IDC_COMBO_DEVICETYPEMODE);
	if (sHostLowerCase.Find(_T("http://")) >= 0 || sHostLowerCase.Find(_T("rtsp://")) >= 0)
	{
		pEditPort->EnableWindow(FALSE);
		pComboBoxDevTypeMode->EnableWindow(FALSE);
	}
	else
	{
		pEditPort->EnableWindow(TRUE);
		pComboBoxDevTypeMode->EnableWindow(TRUE);
	}
}

void CHostPortDlg::OnEditchangeComboHost()
{
	m_HostComboBox.GetWindowText(m_sHost);
	EnableDisableCtrls();
	LoadSettings();
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
	m_HostComboBox.GetLBText(m_HostComboBox.GetCurSel(), m_sHost);

	// Port
	int nSel = m_HostComboBox.GetCurSel();
	if (nSel >= 0 && nSel < m_Ports.GetSize())
	{
		m_nPort = m_Ports[nSel];
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PORT);
		CString sPort;
		sPort.Format(_T("%i"), m_nPort);
		pEdit->SetWindowText(sPort);
	}

	// Device Type Mode
	if (nSel >= 0 && nSel < m_DeviceTypeModes.GetSize())
	{
		m_nDeviceTypeMode = m_DeviceTypeModes[nSel];
		DeviceTypeModeToSelection(m_nDeviceTypeMode);
	}

	// Update Controls
	EnableDisableCtrls();
	LoadSettings();
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
		m_nPort = RTSP_DEFAULT_PORT;
	LoadSettings();
}

void CHostPortDlg::OnSelchangeComboDeviceTypeMode()
{
	m_nDeviceTypeMode = SelectionToDeviceTypeMode();
	LoadSettings();
}

void CHostPortDlg::OnCheckPreferTcpforRtsp()
{
	// Disallow the possibility to check both because when both are checked ffmpeg
	// chooses UDP Multicast and not TCP which is not what's intuitively expected
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_PREFER_TCP_FOR_RTSP);
	if (pCheck->GetCheck())
	{
		pCheck = (CButton*)GetDlgItem(IDC_CHECK_UDP_MULTICAST_FOR_RTSP);
		pCheck->SetCheck(0);
	}
}

void CHostPortDlg::OnCheckUdpMulticastforRtsp()
{
	// Disallow the possibility to check both because when both are checked ffmpeg
	// chooses UDP Multicast and not TCP which is not what's intuitively expected
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_UDP_MULTICAST_FOR_RTSP);
	if (pCheck->GetCheck())
	{
		pCheck = (CButton*)GetDlgItem(IDC_CHECK_PREFER_TCP_FOR_RTSP);
		pCheck->SetCheck(0);
	}
}

void CHostPortDlg::OnError()
{
	::SetFocus(m_HostComboBox.GetSafeHwnd());
	::AlertUser(GetSafeHwnd());
	::AfxGetMainFrame()->PopupNotificationWnd(APPNAME_NOEXT, ML_STRING(1867, "Enter an IP or a Hostname or an URL starting with rtsp:// or http:// (ATTENTION: User Name and Password must be provided under Camera Login, not in the URL)"), 0);
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

	// Do not continue if the device corresponding to the selected 
	// m_sHost, m_nPort, m_nDeviceTypeMode is already running
	if (IsRunning())
		return;

	// Close notification window in case it has been shown
	// by OnError() or by IsRunning()
	::AfxGetMainFrame()->CloseNotificationWnd();

	// Save selected m_sHost, m_nPort and m_nDeviceTypeMode
	// New ContaCam versions only use the first history
	// entry to store the last selected item
	// (old ContaCams stored the list of entered hosts separately,
	// new ContaCams build that list from the registry/inifile settings)
	CWinApp* pApp = ::AfxGetApp();
	CString sSection(_T("HostPortDlg"));
	pApp->WriteProfileString(sSection, _T("HostHistory0"), m_sHost);
	pApp->WriteProfileInt(sSection, _T("PortHistory0"), m_nPort);
	pApp->WriteProfileInt(sSection, _T("DeviceTypeModeHistory0"), m_nDeviceTypeMode);

	// Save Username, Password, PreferTcpforRtsp and UdpMulticastforRtsp flags
	// for the device given by m_sHost, m_nPort, m_nDeviceTypeMode
	SaveSettings();

	CDialog::OnOK();
}

void CHostPortDlg::OnCancel()
{
	// Close notification window in case it has been shown
	// by OnError() or by IsRunning()
	::AfxGetMainFrame()->CloseNotificationWnd();

	CDialog::OnCancel();
}

void CHostPortDlg::OnButtonHelp()
{
	::ShellExecute(NULL, _T("open"), IPCAM_HELP_ONLINE_PAGE, NULL, NULL, SW_SHOWNORMAL);
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
	return CVideoDeviceDoc::MakeNetworkDevicePathName(	sOutGetFrameVideoHost, nOutGetFrameVideoPort,
														sOutGetFrameLocation, (CVideoDeviceDoc::NetworkDeviceTypeMode)nOutDeviceTypeMode);
}

CString CHostPortDlg::GetAssignedDeviceName(const CString& sInHost, int nInPort, int nInDeviceTypeMode)
{
	// Get device path name
	CString sDevicePathName = MakeDevicePathName(sInHost, nInPort, nInDeviceTypeMode);

	// Check whether section exists because the get profile functions call GetSectionKey()
	// which creates the key if it doesn't exist. We do not want a new key for each typed char!
	BOOL bSectionExists = ((CUImagerApp*)::AfxGetApp())->IsExistingSection(sDevicePathName);

	CString	sName;
	if (bSectionExists)
	{
		sName = ((CUImagerApp*)::AfxGetApp())->GetProfileString(sDevicePathName, _T("RecordAutoSaveDir"), _T(""));
		sName.TrimRight(_T('\\'));
		int index = sName.ReverseFind(_T('\\'));
		if (index >= 0)
			sName = sName.Mid(index + 1);
	}
	return sName;
}

BOOL CHostPortDlg::IsRunning()
{
	// Get device path name
	CString sDevicePathName = MakeDevicePathName(m_sHost, m_nPort, m_nDeviceTypeMode);

	// Loop through the open documents to check whether a device with the given
	// combination of m_sHost, m_nPort, m_nDeviceTypeMode is already running
	CUImagerMultiDocTemplate* pVideoDeviceDocTemplate = ((CUImagerApp*)::AfxGetApp())->GetVideoDeviceDocTemplate();
	POSITION posVideoDeviceDoc = pVideoDeviceDocTemplate->GetFirstDocPosition();
	while (posVideoDeviceDoc)
	{
		CVideoDeviceDoc* pVideoDeviceDoc = (CVideoDeviceDoc*)pVideoDeviceDocTemplate->GetNextDoc(posVideoDeviceDoc);
		if (pVideoDeviceDoc && !pVideoDeviceDoc->m_pDxCapture &&
			pVideoDeviceDoc->GetDevicePathName() == sDevicePathName)
		{
			if (pVideoDeviceDoc->GetFrame()->IsIconic())
				pVideoDeviceDoc->GetFrame()->MDIRestore();
			else if (!pVideoDeviceDoc->GetFrame()->IsZoomed())
				pVideoDeviceDoc->GetFrame()->MDIActivate();
			::AfxGetMainFrame()->PopupNotificationWnd(APPNAME_NOEXT, ML_STRING(1464, "The camera is already running!"), 0);
			return TRUE;
		}
	}

	return FALSE;
}

void CHostPortDlg::LoadSettings()
{
	// Get device path name
	CString sDevicePathName = MakeDevicePathName(m_sHost, m_nPort, m_nDeviceTypeMode);

	// Check whether section exists because the get profile functions call GetSectionKey()
	// which creates the key if it doesn't exist. We do not want a new key for each typed char!
	BOOL bSectionExists = ((CUImagerApp*)::AfxGetApp())->IsExistingSection(sDevicePathName);

	// Load
	if (bSectionExists)
	{
		// Username
		CString	sUsername = ((CUImagerApp*)::AfxGetApp())->GetSecureProfileString(sDevicePathName, _T("HTTPGetFrameUsernameExportable"));
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
		pEdit->SetWindowText(sUsername);

		// Password
		CString	sPassword = ((CUImagerApp*)::AfxGetApp())->GetSecureProfileString(sDevicePathName, _T("HTTPGetFramePasswordExportable"));
		pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
		pEdit->SetWindowText(sPassword);

		// Prefer Tcp for Rtsp
		BOOL bPreferTcpforRtsp = (BOOL)((CUImagerApp*)::AfxGetApp())->GetProfileInt(sDevicePathName, _T("PreferTcpforRtsp"), FALSE);
		CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_PREFER_TCP_FOR_RTSP);
		pCheck->SetCheck(bPreferTcpforRtsp ? 1 : 0);

		// UDP Multicast for Rtsp
		BOOL bUdpMulticastforRtsp = (BOOL)((CUImagerApp*)::AfxGetApp())->GetProfileInt(sDevicePathName, _T("UdpMulticastforRtsp"), FALSE);
		pCheck = (CButton*)GetDlgItem(IDC_CHECK_UDP_MULTICAST_FOR_RTSP);
		pCheck->SetCheck(bUdpMulticastforRtsp ? 1 : 0);
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

void CHostPortDlg::SaveSettings()
{
	// Get device path name
	CString sDevicePathName = MakeDevicePathName(m_sHost, m_nPort, m_nDeviceTypeMode);
	CString sText;

	// Username
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_AUTH_USERNAME);
	pEdit->GetWindowText(sText);
	((CUImagerApp*)::AfxGetApp())->WriteSecureProfileString(sDevicePathName, _T("HTTPGetFrameUsernameExportable"), sText);

	// Password
	pEdit = (CEdit*)GetDlgItem(IDC_AUTH_PASSWORD);
	pEdit->GetWindowText(sText);
	((CUImagerApp*)::AfxGetApp())->WriteSecureProfileString(sDevicePathName, _T("HTTPGetFramePasswordExportable"), sText);

	// Prefer Tcp for Rtsp
	CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK_PREFER_TCP_FOR_RTSP);
	((CUImagerApp*)::AfxGetApp())->WriteProfileInt(sDevicePathName, _T("PreferTcpforRtsp"), pCheck->GetCheck());

	// UDP Multicast for Rtsp
	pCheck = (CButton*)GetDlgItem(IDC_CHECK_UDP_MULTICAST_FOR_RTSP);
	((CUImagerApp*)::AfxGetApp())->WriteProfileInt(sDevicePathName, _T("UdpMulticastforRtsp"), pCheck->GetCheck());
}

#endif
