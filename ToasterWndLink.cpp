#include "stdafx.h"
#include "MainFrm.h"
#include "ToasterWndLink.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL CToasterNotificationLink::IsClickable(const CString& sText)
{
	return (::IsExistingFile(sText)			||
			::IsExistingDir(sText)			||
			sText.Find(_T("http://")) == 0	||
			sText.Find(_T("https://")) == 0	||
			sText.Find(ML_STRING(1570, "In Power Options disable: ")) == 0);
}

void CToasterNotificationLink::OnBodyTextClicked(CToasterWnd* pFrom)
{
	if (IsClickable(pFrom->m_sText))
	{
		if (pFrom->m_sText.Find(ML_STRING(1570, "In Power Options disable: ")) == 0)
			::ShellExecute(NULL, NULL, _T("control.exe"), _T("/name Microsoft.PowerOptions /page pagePlanSettings"), NULL, SW_SHOWNORMAL);
		else
			::ShellExecute(NULL, _T("open"), pFrom->m_sText, NULL, NULL, SW_SHOWNORMAL);

		::AfxGetMainFrame()->CloseToaster();
	}
}

void CToasterNotificationLink::OnTitleTextClicked(CToasterWnd* /*pFrom*/)
{
	TRACE(_T("CToasterNotificationLink::OnTitleTextClicked called\n"));
}

void CToasterNotificationLink::OnIconClicked(CToasterWnd* /*pFrom*/)
{
	TRACE(_T("CToasterNotificationLink::OnIconClicked called\n"));
}

void CToasterNotificationLink::OnClose(CToasterWnd* /*pFrom*/, BOOL /*bButton*/)
{
	TRACE(_T("CToasterNotificationLink::OnClose called\n"));
}