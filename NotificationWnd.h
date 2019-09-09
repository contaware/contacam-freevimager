#pragma once

#ifndef __NOTIFICATIONWND_H__
#define __NOTIFICATIONWND_H__

#ifndef _UXTHEME_H_
#include <uxtheme.h>
#endif

class CNotificationWnd : public CFrameWnd
{
public:
	CNotificationWnd(const CString& sTitle, const CString& sText, int nWidth, int nHeight, DWORD dwWaitTimeMs = 0U);
	virtual ~CNotificationWnd();

	BOOL Show();				// Create and popup the notification window
	void Close();				// Actually destroys the window
	DWORD m_dwTextStyle;		// The flags to use in the call to DrawText for the body text
								// default: DT_NOPREFIX | DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL

protected:
	HMODULE LoadLibraryFromSystem32(LPCTSTR lpFileName);
	virtual void DrawCloseButton(CDC* pDC);
	virtual void DrawThemeCloseButton(CDC* pDC);
	virtual void DrawLegacyCloseButton(CDC* pDC);
	virtual void CreateFonts();
	virtual void HandleClosing();
	virtual void PostNcDestroy();
	BOOL IsBodyTextClickable(const CString& sText);
	CRect CalculatePopupPosition();

	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	// Title
	CString m_sTitle;			// The title text to display in the notification window
	CFont m_fontTitle;			// The font to use for title text
	COLORREF m_colorTitle;		// The color to use for the title text
	HICON m_TitleIcon;			// The icon to display just before the Title text

	// Text
	CString m_sText;			// The body text to display in the notification window
	CFont m_fontText;			// The font to use for body text
	COLORREF m_colorText;		// The color to use for the body text
	COLORREF m_colorBackground;	// The color to use for the background
	BOOL m_bTextHot;			// Is the body text clickable
	HCURSOR m_cursorHand;		// The hand cursor

	// Size
	int m_nWidth;				// The width to use for the notification window
	int m_nHeight;				// The height to use for the notification window
	
	// Close timer
	DWORD m_dwWaitTimeMs;		// The time in milliseconds which the notification should be shown for, 0 means show infinitely
	UINT_PTR m_nTimerID;		// The timer ID we use for the auto-close
	
	// Rectangles
	CRect m_rectIcon;			// Client window location of the icon
	CRect m_rectClose;			// Client window location of the close button
	CRect m_rectTitle;			// Client window location of the title text
	CRect m_rectBodyText;		// Client window location of the body text
	
	// Flags
	BOOL m_bClosePressed;		// Has the closed button been pressed
	BOOL m_bCloseHot;			// Is the close button "hot"
	BOOL m_bBodyTextPressed;	// Has the body text been pressed
	BOOL m_bSafeToClose;		// Set when we want to destroy the window as opposed to just hiding it
	
	// Themes suport
	HMODULE m_hUXTheme;
	typedef HTHEME (WINAPI OPENTHEMEDATA)(HWND, LPCWSTR);
	typedef OPENTHEMEDATA* LPOPENTHEMEDATA;
	LPOPENTHEMEDATA m_lpfnOpenThemeData;
	typedef HRESULT (WINAPI DRAWTHEMEBACKGROUND)(HTHEME, HDC, int, int, const RECT*, const RECT*);
	typedef DRAWTHEMEBACKGROUND* LPDRAWTHEMEBACKGROUND;
	LPDRAWTHEMEBACKGROUND  m_lpfnDrawThemeBackground;
	typedef BOOL (WINAPI ISTHEMEACTIVE)();
	typedef HRESULT (WINAPI CLOSETHEMEDATA)(HTHEME);
	typedef CLOSETHEMEDATA* LPCLOSETHEMEDATA;
	LPCLOSETHEMEDATA m_lpfnCloseThemeData;
};

#endif
