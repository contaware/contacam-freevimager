#include "stdafx.h"
#include "uImager.h"
#include "VideoDevicePropertySheet.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

IMPLEMENT_DYNAMIC(CVideoDevicePropertySheet, CPropertySheet)

BEGIN_MESSAGE_MAP(CVideoDevicePropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CVideoDevicePropertySheet)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CVideoDevicePropertySheet::CVideoDevicePropertySheet(CVideoDeviceDoc* pDoc) :
CPropertySheet(MakeTitle(pDoc), NULL)
{	
	// Set doc pointer
	ASSERT_VALID(pDoc);
	m_pDoc = pDoc;
	
	// Init pages
	m_AssistantPropertyPage.SetDoc(pDoc);
	m_GeneralPropertyPage.SetDoc(pDoc);
	m_SnapshotPropertyPage.SetDoc(pDoc);
	m_MovementDetectionPropertyPage.SetDoc(pDoc);
	m_NetworkPropertyPage.SetDoc(pDoc);

	// Add pages
	AddPage(&m_AssistantPropertyPage);
	AddPage(&m_GeneralPropertyPage);
	AddPage(&m_SnapshotPropertyPage);
	AddPage(&m_MovementDetectionPropertyPage);
	AddPage(&m_NetworkPropertyPage);
}

CVideoDevicePropertySheet::~CVideoDevicePropertySheet()
{
	
}

void CVideoDevicePropertySheet::UpdateTitle()
{
	SetTitle(MakeTitle(m_pDoc));
}

CString CVideoDevicePropertySheet::MakeTitle(CVideoDeviceDoc* pDoc)
{
	ASSERT_VALID(pDoc);
	CString sTitle;
	if (pDoc->GetDeviceName() != pDoc->GetAssignedDeviceName())
		sTitle = pDoc->GetAssignedDeviceName() + _T(" (") + pDoc->GetDeviceName() + _T(")");
	else
		sTitle = pDoc->GetDeviceName();
	if (pDoc->m_pGetFrameNetCom)
	{
		switch(pDoc->m_nNetworkDeviceTypeMode)
		{
			case CVideoDeviceDoc::INTERNAL_UDP	: sTitle += CString(_T(" , ")) + ML_STRING(1547, "Internal UDP Server"); break;
			case CVideoDeviceDoc::OTHERONE		:
				if (pDoc->m_HttpGetFrameLocations[0] == _T("/"))
					sTitle += CString(_T(" , ")) + ML_STRING(1548, "Other HTTP Device");
				else
					sTitle += CString(_T(" , ")) + pDoc->m_HttpGetFrameLocations[0];
				break;
			case CVideoDeviceDoc::AXIS_SP		: sTitle += CString(_T(" , ")) + ML_STRING(1549, "Axis (Server Push Mode)"); break;
			case CVideoDeviceDoc::AXIS_CP		: sTitle += CString(_T(" , ")) + ML_STRING(1550, "Axis (Client Poll Mode)"); break;
			case CVideoDeviceDoc::PANASONIC_SP	: sTitle += CString(_T(" , ")) + ML_STRING(1551, "Panasonic (Server Push Mode)"); break;
			case CVideoDeviceDoc::PANASONIC_CP	: sTitle += CString(_T(" , ")) + ML_STRING(1552, "Panasonic (Client Poll Mode)"); break;
			case CVideoDeviceDoc::PIXORD_SP		: sTitle += CString(_T(" , ")) + ML_STRING(1553, "Pixord or NetComm (Server Push Mode)"); break;
			case CVideoDeviceDoc::PIXORD_CP		: sTitle += CString(_T(" , ")) + ML_STRING(1554, "Pixord or NetComm (Client Poll Mode)"); break;
			case CVideoDeviceDoc::EDIMAX_SP		: sTitle += CString(_T(" , ")) + ML_STRING(1789, "Edimax (Server Push Mode)"); break;
			case CVideoDeviceDoc::EDIMAX_CP		: sTitle += CString(_T(" , ")) + ML_STRING(1839, "Edimax (Client Poll Mode)"); break;
			case CVideoDeviceDoc::TPLINK_SP		: sTitle += CString(_T(" , ")) + ML_STRING(1840, "TP-Link (Server Push Mode)"); break;
			case CVideoDeviceDoc::TPLINK_CP		: sTitle += CString(_T(" , ")) + ML_STRING(1841, "TP-Link (Client Poll Mode)"); break;
			default : break;
		}
		if (pDoc->m_pGetFrameNetCom->GetSocketFamily() == AF_INET6)
			sTitle += _T(" , IPv6");
	}
	return sTitle;
}

void CVideoDevicePropertySheet::Toggle()
{
	if (IsVisible())
		Hide();
	else
		Show();
}

BOOL CVideoDevicePropertySheet::IsVisible()
{
	return IsWindowVisible();
}

void CVideoDevicePropertySheet::Show()
{
	m_pDoc->GetView()->ForceCursor();
	ShowWindow(SW_SHOW);
}

void CVideoDevicePropertySheet::Hide()
{
	ShowWindow(SW_HIDE);
	m_pDoc->GetView()->ForceCursor(FALSE);
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		m_pDoc->SaveSettings();
}

void CVideoDevicePropertySheet::PostNcDestroy() 
{
	m_pDoc->m_pVideoDevicePropertySheet = NULL;
	delete this;
	CPropertySheet::PostNcDestroy();	// This Does nothing
}

void CVideoDevicePropertySheet::OnClose() 
{
	// Instead of closing the modeless
	// property sheet, just hide it
	Hide();
}

void CVideoDevicePropertySheet::Close()
{
	if (IsVisible())
		m_pDoc->GetView()->ForceCursor(FALSE);
	DestroyWindow();
}

BOOL CVideoDevicePropertySheet::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		int nVirtKey = (int)pMsg->wParam;
		if (nVirtKey == VK_ESCAPE)
		{
			Hide();
			return TRUE;
		}
	}
	return CPropertySheet::PreTranslateMessage(pMsg);
}

#endif
