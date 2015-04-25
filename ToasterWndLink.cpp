#include "stdafx.h"
#include "ToasterWndLink.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL CToasterNotificationLink::IsClickable(const CString& sText)
{
	int nIndexHttp = sText.Find(_T("http://"));
	int nIndexHttps = sText.Find(_T("https://"));
	return (::IsExistingFile(sText) || ::IsExistingDir(sText) || nIndexHttp == 0 || nIndexHttps == 0);
}

void CToasterNotificationLink::OnBodyTextClicked(CToasterWnd* pFrom)
{
	if (IsClickable(pFrom->m_sText))
		::ShellExecute(NULL, _T("open"), pFrom->m_sText, NULL, NULL, SW_SHOWNORMAL);
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