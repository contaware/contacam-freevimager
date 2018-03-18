#include "stdafx.h"
#include "uImager.h"
#include "CameraAdvancedSettingsPropertySheet.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

IMPLEMENT_DYNAMIC(CCameraAdvancedSettingsPropertySheet, CPropertySheet)

BEGIN_MESSAGE_MAP(CCameraAdvancedSettingsPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CCameraAdvancedSettingsPropertySheet)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CCameraAdvancedSettingsPropertySheet::CCameraAdvancedSettingsPropertySheet(CVideoDeviceDoc* pDoc) :
CPropertySheet(MakeTitle(pDoc), NULL)
{	
	// Set doc pointer
	ASSERT_VALID(pDoc);
	m_pDoc = pDoc;
	
	// Init pages
	m_VideoPropertyPage.SetDoc(pDoc);
	m_SnapshotPropertyPage.SetDoc(pDoc);

	// Add pages
	AddPage(&m_VideoPropertyPage);
	AddPage(&m_SnapshotPropertyPage);
}

CCameraAdvancedSettingsPropertySheet::~CCameraAdvancedSettingsPropertySheet()
{
	
}

void CCameraAdvancedSettingsPropertySheet::UpdateTitle()
{
	SetTitle(MakeTitle(m_pDoc));
}

CString CCameraAdvancedSettingsPropertySheet::MakeTitle(CVideoDeviceDoc* pDoc)
{
	ASSERT_VALID(pDoc);
	CString sTitle;
	if (pDoc->GetDeviceName() != pDoc->GetAssignedDeviceName())
		sTitle = pDoc->GetAssignedDeviceName() + _T(" (") + pDoc->GetDeviceName() + _T(")");
	else
		sTitle = pDoc->GetDeviceName();
	return sTitle;
}

void CCameraAdvancedSettingsPropertySheet::Show()
{
	if (!IsWindowVisible())
	{
		m_pDoc->GetView()->ForceCursor();
		ShowWindow(SW_SHOW);
	}
}

void CCameraAdvancedSettingsPropertySheet::Hide(BOOL bSaveSettingsOnHiding)
{
	if (IsWindowVisible())
	{
		if (bSaveSettingsOnHiding)
			m_pDoc->SaveSettings();
		ShowWindow(SW_HIDE);
		m_pDoc->GetView()->ForceCursor(FALSE);
	}
}

void CCameraAdvancedSettingsPropertySheet::PostNcDestroy() 
{
	m_pDoc->m_pCameraAdvancedSettingsPropertySheet = NULL;
	delete this;
	CPropertySheet::PostNcDestroy();	// This Does nothing
}

void CCameraAdvancedSettingsPropertySheet::OnClose() 
{
	// Instead of closing the modeless
	// property sheet, just hide it
	Hide(TRUE);
}

void CCameraAdvancedSettingsPropertySheet::Close()
{
	if (IsWindowVisible())
		m_pDoc->GetView()->ForceCursor(FALSE);
	DestroyWindow();
}

BOOL CCameraAdvancedSettingsPropertySheet::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		int nVirtKey = (int)pMsg->wParam;
		if (nVirtKey == VK_ESCAPE)
		{
			Hide(TRUE);
			return TRUE;
		}
	}
	return CPropertySheet::PreTranslateMessage(pMsg);
}

#endif
