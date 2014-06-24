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
	m_GeneralPropertyPage.SetDoc(pDoc);
	m_SnapshotPropertyPage.SetDoc(pDoc);
	m_MovementDetectionPropertyPage.SetDoc(pDoc);

	// Add pages
	AddPage(&m_GeneralPropertyPage);
	AddPage(&m_SnapshotPropertyPage);
	AddPage(&m_MovementDetectionPropertyPage);
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
	if (pDoc->m_pGetFrameNetCom)
	{
		switch(pDoc->m_nNetworkDeviceTypeMode)
		{
			case CVideoDeviceDoc::OTHERONE_SP	:
			case CVideoDeviceDoc::OTHERONE_CP	:
			{
				int nHttpGetFrameLocationPos = pDoc->m_nHttpGetFrameLocationPos; // make a local copy because it is updated by another thread
				if (nHttpGetFrameLocationPos > 0 && nHttpGetFrameLocationPos < pDoc->m_HttpGetFrameLocations.GetSize())
					sTitle += CString(_T(" , ")) + pDoc->m_HttpGetFrameLocations[nHttpGetFrameLocationPos];
				else if (pDoc->m_HttpGetFrameLocations[0] == _T("/"))
					sTitle += CString(_T(" , ")) + ML_STRING(1548, "Other HTTP Device");
				else
					sTitle += CString(_T(" , ")) + pDoc->m_HttpGetFrameLocations[0];
				if (pDoc->m_pHttpGetFrameParseProcess)
				{
					if (pDoc->m_pHttpGetFrameParseProcess->m_FormatType == CVideoDeviceDoc::CHttpGetFrameParseProcess::FORMATMJPEG)
						sTitle += _T(" (") + ML_STRING(1865, "Server Push Mode") + _T(")");
					else if (pDoc->m_pHttpGetFrameParseProcess->m_FormatType == CVideoDeviceDoc::CHttpGetFrameParseProcess::FORMATJPEG)
						sTitle += _T(" (") + ML_STRING(1866, "Client Poll Mode") + _T(")");
				}
				break;
			}
			case CVideoDeviceDoc::AXIS_SP		: sTitle += CString(_T(" , ")) + _T("Axis (") + ML_STRING(1865, "Server Push Mode") + _T(")"); break;
			case CVideoDeviceDoc::AXIS_CP		: sTitle += CString(_T(" , ")) + _T("Axis (") + ML_STRING(1866, "Client Poll Mode") + _T(")"); break;
			case CVideoDeviceDoc::PANASONIC_SP	: sTitle += CString(_T(" , ")) + _T("Panasonic (") + ML_STRING(1865, "Server Push Mode") + _T(")"); break;
			case CVideoDeviceDoc::PANASONIC_CP	: sTitle += CString(_T(" , ")) + _T("Panasonic (") + ML_STRING(1866, "Client Poll Mode") + _T(")"); break;
			case CVideoDeviceDoc::PIXORD_SP		: sTitle += CString(_T(" , ")) + _T("Pixord/NetComm (") + ML_STRING(1865, "Server Push Mode") + _T(")"); break;
			case CVideoDeviceDoc::PIXORD_CP		: sTitle += CString(_T(" , ")) + _T("Pixord/NetComm (") + ML_STRING(1866, "Client Poll Mode") + _T(")"); break;
			case CVideoDeviceDoc::EDIMAX_SP		: sTitle += CString(_T(" , ")) + _T("Edimax (") + ML_STRING(1865, "Server Push Mode") + _T(")"); break;
			case CVideoDeviceDoc::EDIMAX_CP		: sTitle += CString(_T(" , ")) + _T("Edimax (") + ML_STRING(1866, "Client Poll Mode") + _T(")"); break;
			case CVideoDeviceDoc::TPLINK_SP		: sTitle += CString(_T(" , ")) + _T("TP-Link (") + ML_STRING(1865, "Server Push Mode") + _T(")"); break;
			case CVideoDeviceDoc::TPLINK_CP		: sTitle += CString(_T(" , ")) + _T("TP-Link (") + ML_STRING(1866, "Client Poll Mode") + _T(")"); break;
			case CVideoDeviceDoc::FOSCAM_SP		: sTitle += CString(_T(" , ")) + _T("Foscam/Tenvis/Clones (") + ML_STRING(1865, "Server Push Mode") + _T(")"); break;
			case CVideoDeviceDoc::FOSCAM_CP		: sTitle += CString(_T(" , ")) + _T("Foscam/Tenvis/Clones (") + ML_STRING(1866, "Client Poll Mode") + _T(")"); break;
			default : break;
		}
		if (pDoc->m_pGetFrameNetCom->GetSocketFamily() == AF_INET6)
			sTitle += _T(" , IPv6");
	}
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
