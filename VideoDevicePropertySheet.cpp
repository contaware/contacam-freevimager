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
CPropertySheet(	pDoc->GetDeviceName() != pDoc->GetAssignedDeviceName() ?
				pDoc->GetAssignedDeviceName() + _T(" (") + pDoc->GetDeviceName() + _T(")") :
				pDoc->GetDeviceName(), NULL)
{	
	// Set doc pointer
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
