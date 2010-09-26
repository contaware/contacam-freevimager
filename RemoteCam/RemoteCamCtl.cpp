// RemoteCamCtl.cpp : Implementation of the CRemoteCamCtrl ActiveX Control class.

#include "stdafx.h"
#include "RemoteCam.h"
#include "RemoteCamCtl.h"
#include "RemoteCamPpg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CRemoteCamCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CRemoteCamCtrl, COleControl)
	//{{AFX_MSG_MAP(CRemoteCamCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
	ON_MESSAGE(WM_DOINVALIDATE_CTRL, OnDoInvalidateCtrl)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CRemoteCamCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CRemoteCamCtrl)
	DISP_PROPERTY_NOTIFY(CRemoteCamCtrl, "Host", m_sHost, OnHostChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CRemoteCamCtrl, "Port", m_lPort, OnPortChanged, VT_I4)
	DISP_PROPERTY_NOTIFY(CRemoteCamCtrl, "MaxFrames", m_lMaxFrames, OnMaxFramesChanged, VT_I4)
	DISP_PROPERTY_NOTIFY(CRemoteCamCtrl, "DisableResend", m_bDisableResend, OnDisableResendChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(CRemoteCamCtrl, "Username", m_sGetFrameUsername, OnUsernameChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CRemoteCamCtrl, "Password", m_sGetFramePassword, OnPasswordChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CRemoteCamCtrl, "IPv6", m_bIPv6, OnIPv6Changed, VT_BOOL)
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CRemoteCamCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CRemoteCamCtrl, COleControl)
	//{{AFX_EVENT_MAP(CRemoteCamCtrl)
	// NOTE - ClassWizard will add and remove event map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CRemoteCamCtrl, 1)
	PROPPAGEID(CRemoteCamPropPage::guid)
END_PROPPAGEIDS(CRemoteCamCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CRemoteCamCtrl, "REMOTECAM.RemoteCamCtrl.1",
	0x8cf6679, 0x5bb3, 0x4ef4, 0xa7, 0x73, 0xc4, 0x10, 0xdc, 0x56, 0x53, 0x1c)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CRemoteCamCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DRemoteCam =
		{ 0x7aa2fd65, 0xbb4d, 0x490d, { 0x89, 0x26, 0x68, 0x5, 0xbb, 0x40, 0x24, 0x67 } };
const IID BASED_CODE IID_DRemoteCamEvents =
		{ 0xce0d4885, 0xd7cf, 0x40a8, { 0x80, 0x65, 0x61, 0x4a, 0x45, 0xa, 0xf9, 0x57 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwRemoteCamOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CRemoteCamCtrl, IDS_REMOTECAM, _dwRemoteCamOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CRemoteCamCtrl::CRemoteCamCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CRemoteCamCtrl

BOOL CRemoteCamCtrl::CRemoteCamCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegApartmentThreading to 0.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_REMOTECAM,
			IDB_REMOTECAM,
			afxRegApartmentThreading,
			_dwRemoteCamOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CRemoteCamCtrl::CRemoteCamCtrl - Constructor

CRemoteCamCtrl::CRemoteCamCtrl()
{
	InitializeIIDs(&IID_DRemoteCam, &IID_DRemoteCamEvents);
	::InitializeCriticalSection(&m_csDib);
	m_GetFrameParseProcess.SetCtrl(this);
	m_bFirstConnected = FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CRemoteCamCtrl::~CRemoteCamCtrl - Destructor

CRemoteCamCtrl::~CRemoteCamCtrl()
{
	::DeleteCriticalSection(&m_csDib);
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamCtrl::OnDraw - Drawing function
LONG CRemoteCamCtrl::OnDoInvalidateCtrl(WPARAM wparam, LPARAM lparam)
{
	InvalidateControl();
	return 0;
}
void CRemoteCamCtrl::OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	::EnterCriticalSection(&m_csDib);
	CRect rcDib;
	rcDib.left = 0;
	rcDib.top = 0;
	rcDib.bottom = m_Dib.GetHeight();
	rcDib.right = m_Dib.GetWidth();
	if (m_Dib.IsValid())
	{
		CRect rcOldContainer, rcNewContainer;
		GetRectInContainer(&rcOldContainer);
		rcNewContainer = rcOldContainer;
		rcNewContainer.right = rcNewContainer.left + rcDib.right;
		rcNewContainer.bottom = rcNewContainer.top + rcDib.bottom;
		if (rcOldContainer != rcNewContainer)
			SetRectInContainer(&rcNewContainer);

		m_Dib.Paint(pdc->GetSafeHdc(), &CRect(rcBounds), rcDib);
		::LeaveCriticalSection(&m_csDib);
	}
	else
	{
		::LeaveCriticalSection(&m_csDib);
		CBrush backBrush(GetSysColor(COLOR_3DFACE));
		CBrush* pOldBrush = pdc->SelectObject(&backBrush);
		pdc->PatBlt(rcBounds.left, rcBounds.top, rcBounds.Width(), rcBounds.Height(), PATCOPY);
		pdc->SelectObject(pOldBrush);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CRemoteCamCtrl::DoPropExchange - Persistence support

void CRemoteCamCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	PX_String(pPX, _T("Host"), m_sHost, _T("localhost"));
	PX_Long(pPX, _T("Port"), m_lPort, DEFAULT_UDP_PORT);
	PX_Long(pPX, _T("MaxFrames"), m_lMaxFrames, NETFRAME_DEFAULT_FRAMES);
	PX_Bool(pPX, _T("DisableResend"), m_bDisableResend, FALSE);
	PX_String(pPX, _T("Username"), m_sGetFrameUsername, _T(""));
	PX_String(pPX, _T("Password"), m_sGetFramePassword, _T(""));
	PX_Bool(pPX, _T("IPv6"), m_bIPv6, FALSE);
}


/////////////////////////////////////////////////////////////////////////////
// CRemoteCamCtrl::OnResetState - Reset control to default state

void CRemoteCamCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CRemoteCamCtrl::AboutBox - Display an "About" box to the user

void CRemoteCamCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_REMOTECAM);
	dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CRemoteCamCtrl message handlers


BOOL CRemoteCamCtrl::ConnectGetFrameUDP(LPCTSTR pszHostName, int nPort)
{
	// Set Thread Priority
	m_GetFrameNetCom.SetThreadsPriority(THREAD_PRIORITY_HIGHEST);

	// Init
	if (!m_GetFrameNetCom.Init(
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
					&m_GetFrameParseProcess,// Parser
					&m_GetFrameGenerator,	// Generator
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
					1000,/*uiTxPacketTimeout*/// After this timeout a Packet is sent
											// even if no Write Event Happened (A zero meens INFINITE Timeout).
											// This is also the Generator rate,
											// if set to zero the Generator is never called!
					NULL,					// Message Class for Notice, Warning and Error Visualization.
					DoIPv6() ? AF_INET6 : AF_INET)) // Socket family
		return FALSE;
	else
	{
		m_GetFrameNetCom.EnableIdleGenerator(TRUE);
		return TRUE;
	}
}

int CRemoteCamCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// In design mode we never reach here
	ConnectGetFrameUDP(m_sHost, m_lPort);
	m_bFirstConnected = TRUE;
	
	return 0;
}

void CRemoteCamCtrl::OnDestroy() 
{
	COleControl::OnDestroy();
	
	// Important to close all the threads,
	// before destroying the Generator & Parser Objects
	// (which is done by the destructor)
	m_GetFrameNetCom.Close();
}

void CRemoteCamCtrl::OnHostChanged() 
{
	SetModifiedFlag();
	if (m_bFirstConnected)	// if not in design mode
		ConnectGetFrameUDP(m_sHost, m_lPort);
}

void CRemoteCamCtrl::OnPortChanged() 
{
	SetModifiedFlag();
	if (m_bFirstConnected)	// if not in design mode
		ConnectGetFrameUDP(m_sHost, m_lPort);
}

void CRemoteCamCtrl::OnMaxFramesChanged() 
{
	SetModifiedFlag();
}

void CRemoteCamCtrl::OnDisableResendChanged() 
{
	SetModifiedFlag();
}

void CRemoteCamCtrl::OnUsernameChanged() 
{
	SetModifiedFlag();
}

void CRemoteCamCtrl::OnPasswordChanged() 
{
	SetModifiedFlag();
}

void CRemoteCamCtrl::OnIPv6Changed() 
{
	SetModifiedFlag();
	if (m_bFirstConnected)	// if not in design mode
		ConnectGetFrameUDP(m_sHost, m_lPort);
}