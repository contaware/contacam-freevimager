#ifndef _TRAYICON_H
#define _TRAYICON_H

////////////////
// CTrayIcon manages an icon
// 
class CTrayIcon : public CCmdTarget
{
public:
	// Constructor / Destructor
	CTrayIcon(UINT uID);
	~CTrayIcon();

	// Set Wnd for tray notifications and minimize / maximize
	void SetWnd(CWnd* pNotifyWnd, UINT uCbMsg);

	// SetIcon functions. To remove the icon call SetIcon(0)
	BOOL SetIcon(UINT uID);
	BOOL SetIcon(HICON hicon, LPCTSTR lpTip);
	BOOL SetIcon(LPCTSTR lpResName, LPCTSTR lpTip)
		{ return SetIcon(lpResName ? 
			AfxGetApp()->LoadIcon(lpResName) : NULL, lpTip); }
	BOOL SetStandardIcon(LPCTSTR lpszIconName, LPCTSTR lpTip)
		{ return SetIcon(::LoadIcon(NULL, lpszIconName), lpTip); }

	// Notification
	virtual LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);

	// Minimize / Maximize
	BOOL IsMinimizedToTray() {return m_bMinimizedToTray;};
    void MinimizeToTray();
    void MaximizeFromTray();

protected:
	DECLARE_DYNAMIC(CTrayIcon)
	NOTIFYICONDATA m_nid;			// struct for Shell_NotifyIcon args
	volatile BOOL m_bMinimizedToTray;
	static HWND m_hWndInvisible;
	static BOOL RemoveTaskbarIcon(HWND hWnd);
};

#endif
