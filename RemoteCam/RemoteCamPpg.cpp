// RemoteCamPpg.cpp : Implementation of the CRemoteCamPropPage property page class.

#include "stdafx.h"
#include "RemoteCam.h"
#include "RemoteCamPpg.h"
#include "NetFrameHdr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CRemoteCamPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CRemoteCamPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CRemoteCamPropPage)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CRemoteCamPropPage, "REMOTECAM.RemoteCamPropPage.1",
	0xf443c3b5, 0x1da3, 0x491b, 0xb1, 0x3f, 0xc9, 0x19, 0xea, 0x28, 0x2, 0x94)


/////////////////////////////////////////////////////////////////////////////
// CRemoteCamPropPage::CRemoteCamPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CRemoteCamPropPage

BOOL CRemoteCamPropPage::CRemoteCamPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_REMOTECAM_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CRemoteCamPropPage::CRemoteCamPropPage - Constructor

CRemoteCamPropPage::CRemoteCamPropPage() :
	COlePropertyPage(IDD, IDS_REMOTECAM_PPG_CAPTION)
{
	//{{AFX_DATA_INIT(CRemoteCamPropPage)
	m_sHost = _T("localhost");
	m_lPort = DEFAULT_UDP_PORT;
	m_lMaxFrames = NETFRAME_DEFAULT_FRAMES;
	m_bDisableResend = FALSE;
	m_sGetFrameUsername = _T("");
	m_sGetFramePassword = _T("");
	m_bIPv6 = FALSE;
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CRemoteCamPropPage::DoDataExchange - Moves data between page and properties

void CRemoteCamPropPage::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CRemoteCamPropPage)
	DDP_Text(pDX, IDC_HOST, m_sHost, _T("Host") );
	DDX_Text(pDX, IDC_HOST, m_sHost);
	DDP_Text(pDX, IDC_PORT, m_lPort, _T("Port") );
	DDX_Text(pDX, IDC_PORT, m_lPort);
	DDP_Text(pDX, IDC_MAXFRAMES, m_lMaxFrames, _T("MaxFrames") );
	DDX_Text(pDX, IDC_MAXFRAMES, m_lMaxFrames);
	DDP_Check(pDX, IDC_DISABLERESEND, m_bDisableResend, _T("DisableResend") );
	DDX_Check(pDX, IDC_DISABLERESEND, m_bDisableResend);
	DDP_Text(pDX, IDC_USERNAME, m_sGetFrameUsername, _T("Username") );
	DDX_Text(pDX, IDC_USERNAME, m_sGetFrameUsername);
	DDP_Text(pDX, IDC_PASSWORD, m_sGetFramePassword, _T("Password") );
	DDX_Text(pDX, IDC_PASSWORD, m_sGetFramePassword);
	DDP_Check(pDX, IDC_IPV6, m_bIPv6, _T("IPv6") );
	DDX_Check(pDX, IDC_IPV6, m_bIPv6);
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CRemoteCamPropPage message handlers
