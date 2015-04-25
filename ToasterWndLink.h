#pragma once

#ifndef __TOASTERWNDLINK_H__
#define __TOASTERWNDLINK_H__

#include "ToasterWnd.h"

class CToasterNotificationLink : public CToasterNotification
{
public:
	static BOOL IsClickable(const CString& sText);
	virtual void OnBodyTextClicked(CToasterWnd* pFrom);		// called when the user clicks the body text
	virtual void OnTitleTextClicked(CToasterWnd* pFrom);	// called when the user clicks the title text
	virtual void OnIconClicked(CToasterWnd* pFrom);			// called when the user clicks the title text
	virtual void OnClose(CToasterWnd* pFrom, BOOL bButton);	// called when the notification window is closing
};

#endif //__TOASTERWNDLINK_H__
