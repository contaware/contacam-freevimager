// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__9A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
#define AFX_MAINFRM_H__9A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_

#pragma once

#include "TwainCpp.h"
#include "TryEnterCriticalSection.h"
#include "Progress.h"
#include "Trayicon.h"
#include "StatusBarACT.h"
#include "MDIClientWnd.h"
#include "ToasterWnd.h"
#include "ToasterWndLink.h"
extern "C"
{
#include "tiffiop.h"
}

// Window Message IDs
#define WM_TASKBAR_BUTTON							0x0313
#define WM_THREADSAFE_OPEN_DOC						WM_USER + 100
#define WM_THREADSAFE_POPUP_TOASTER					WM_USER + 101
#define WM_ALL_CLOSED								WM_USER + 102
#define WM_SCANANDEMAIL								WM_USER + 103
#define WM_TRAY_NOTIFICATION						WM_USER + 104
#ifdef VIDEODEVICEDOC
#define WM_THREADSAFE_CONNECTERR					WM_USER + 105
#define WM_AUTORUN_VIDEODEVICES						WM_USER + 106
#endif
														
#define ID_TIMER_FULLSCREEN							1
#define ID_TIMER_CROP_SCROLL						2
#define ID_TIMER_TRANSPARENCY						3
#define ID_TIMER_CLOSING_ALL						4
#define ID_TIMER_CLOSING_PICTUREDOC					5
#define ID_TIMER_CLOSING_VIDEODEVICEDOC				6
#define ID_TIMER_MCIAUDIO							7
#define ID_TIMER_PROGRESS							8
#define ID_TIMER_ONESEC_POLL						9
#ifdef VIDEODEVICEDOC
#define ID_TIMER_GENERALDLG  						10
#define ID_TIMER_CAMERABASICSETTINGSDLG				11
#define ID_TIMER_RELOAD_SETTINGS					12
#endif

#define FULLSCREEN_TIMER_MS							1000U	// ms
#define AUTOSCROLL_TIMER_MS							120U	// ms
#define CLOSING_CHECK_INTERVAL_TIMER_MS				300U	// ms
#define ONESEC_POLL_TIMER_MS						1000U	// ms

// Picture document closing wait time
#define MAX_PICTUREDOC_CLOSE_WAITTIME				15000U	// ms

// Forward Declarations
class CUImagerDoc;
class CBatchProcDlg;
class CIMAPI2Dlg;
class CDib;

class CMainFrame : public CMDIFrameWnd, public CTwain
{
	DECLARE_DYNAMIC(CMainFrame)

// Operations
public:
	CMainFrame();
	virtual ~CMainFrame();
	void TwainCopyImage(HANDLE hBitmap, TW_IMAGEINFO& info);
	void EnterExitFullscreen();
	void Progress(int nPercent);

	// Toaster window
	void PopupToaster(const CString& sTitle, const CString& sText, DWORD dwWaitTimeMs = 10000);	// call from any thread
	void CloseToaster();																		// call only from UI thread

	// Enable / Disable Tray Icon
	void TrayIcon(BOOL bEnable);

	//
	// Monitor Functions
	//
	// If pWnd == NULL the MainFrame window determines the monitor
	
	// Switch Fullscreen Monitor
	BOOL FullScreenTo(BOOL bNextMonitor); // if TRUE next Monitor, otherwise previous one
	BOOL FullScreenTo(const CRect& rcMonitor);

	// Enumerate all monitors
	BOOL EnumerateMonitors(CDWordArray* pMonitors);
	HMONITOR GetCurrentMonitor();
	HMONITOR GetNextMonitor();
	HMONITOR GetPreviousMonitor();

	// Bpp, DC and Size
	int GetMonitorBpp(CWnd* pWnd = NULL);
	HDC CreateMonitorDC(CWnd* pWnd = NULL); // Remember to delete it with ::DeleteDC()
	CSize GetMonitorSize(CWnd* pWnd = NULL);
	CSize GetMonitorSize(CPoint pt);
	
	// Work Area (monitor without Taskbar)
	void ClipToWorkRect(CRect& rc, CWnd* pWnd = NULL);
	void ClipToWorkRect(CRect& rc, CPoint pt);
	CRect GetPrimaryMonitorWorkRect();
	CRect GetMonitorWorkRect(CWnd* pWnd = NULL);
	CRect GetMonitorWorkRect(CPoint pt);
	
	// MDI client rect
	__forceinline void GetMDIClientRect(LPRECT lpRect) const {::GetClientRect(m_hWndMDIClient, lpRect);};
	void ClipToMDIRect(LPRECT lpRect) const;

	// Full Monitor Area
	void ClipToFullRect(CRect& rc, CWnd* pWnd = NULL);
	void ClipToFullRect(CRect& rc, CPoint pt);
	CRect GetPrimaryMonitorFullRect();
	CRect GetMonitorFullRect(CWnd* pWnd = NULL);
	CRect GetMonitorFullRect(CPoint pt);
	CRect GetPreviousMonitorFullRect();
	CRect GetNextMonitorFullRect();

	// Toolbar
	CToolBar* GetToolBar() {return &m_wndToolBar;};
	
	// Statusbar
	void StatusText(CString sText = _T("")); // if sText is _T("") the idle message is displayed
	CStatusBar* GetStatusBar() {return &m_wndStatusBar;};
	CString GetStatusBarString() const {return m_sStatusBarString;};
	void SetIDLastMessage(UINT nIDLastMessage) {m_nIDLastMessage = nIDLastMessage;};
	void SetIDTracking(UINT nIDTracking) {m_nIDTracking = nIDTracking;};

	// Menu Positions
	void InitMenuPositions(CDocument* pDoc = NULL);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	// Main frame menu positions
	int m_nFileMenuPos;
	int m_nEditMenuPos;
	int m_nViewMenuPos;
	int m_nCaptureMenuPos;
	int m_nPlayMenuPos;
	int m_nSettingsMenuPos;
	int m_nWindowsPos;
	int m_nHelpMenuPos;

	// Help Menu Items Count
	int m_nHelpMenuItemsCount;

	// Full screen flag
	volatile bool m_bFullScreenMode;

	// Progress control
	CProgressCtrl m_Progress;

	// Batch Processing dialog
	CBatchProcDlg* m_pBatchProcDlg;

	// IMAPI2 Burn dialog
	CIMAPI2Dlg* m_pIMAPI2Dlg;

	// Tray Icon
	CTrayIcon m_TrayIcon;

	// Main frame background window
	CMDIClientWnd m_MDIClientWnd;

protected:
	CStatusBarACT  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	HMENU		m_hMenu;
	LONG m_lOldChildStyle;
	LONG m_lOldChildExStyle;
	LONG m_lOldStyle;
	bool m_bStatusBarWasVisible, m_bToolBarWasVisible, m_bChildToolBarWasVisible;
	CRect m_MainWndRect;
	bool m_bChildMax;
	bool m_bChildMin;
	WINDOWPLACEMENT m_ChildWndPlacement;
	WINDOWPLACEMENT m_MainFrameWndPlacement;
	CRect m_rcEnterFullScreenMonitor;
	double m_dChildZoomFactor;
	CPoint m_ptChildScrollPosition;
	BOOL m_bScreenSaverWasActive;
	CString m_sStatusBarString;
	BOOL m_bProgressIndicatorCreated;
	CToasterWnd* m_pToaster;
	CToasterNotificationLink m_ToasterNotificationLink;

	// Scan Vars
	BOOL m_bScanAndEmail;
	TIFF* m_TiffScan;

protected:
	void FullScreenModeOn();
	void FullScreenModeOff();
	void ChangeCoordinatesUnit();
#ifdef VIDEODEVICEDOC
	void CleanupFileMenu(CMenu* pPopupMenu);
	void PopulateCaptureMenu(CMenu* pPopupMenu);
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnFileSelecttwainsource();
	afx_msg void OnFileAcquire();
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnViewAllPreviousPicture();
	afx_msg void OnViewAllNextPicture();
	afx_msg void OnViewAllFirstPicture();
	afx_msg void OnViewAllLastPicture();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRestore();
	afx_msg void OnUpdateRestore(CCmdUI* pCmdUI);
	afx_msg void OnMinimize();
	afx_msg void OnUpdateMinimize(CCmdUI* pCmdUI);
	afx_msg void OnMaximize();
	afx_msg void OnUpdateMaximize(CCmdUI* pCmdUI);
	afx_msg void OnMainmonitor();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnFileAcquireToTiff();
	afx_msg void OnFileAcquireToPdf();
	afx_msg void OnFileAcquireAndEmail();
	afx_msg void OnOpenFromTray();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnEndSession(BOOL bEnding);
	//}}AFX_MSG
	afx_msg void OnXCoordinatesDoubleClick();
	afx_msg void OnYCoordinatesDoubleClick();
	afx_msg LONG OnProgress(WPARAM wparam, LPARAM lparam);
	afx_msg void OnUpdateIndicatorXCoordinate(CCmdUI* pCmdUI);
	afx_msg void OnUpdateIndicatorYCoordinate(CCmdUI* pCmdUI);
	LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnThreadSafeOpenDoc(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnTaskBarButton(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnAllClosed(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnScanAndEmail(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnTrayNotification(WPARAM uID, LPARAM lEvent);
	afx_msg LRESULT OnCopyData(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnTwainClosed(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafePopupToaster(WPARAM wparam, LPARAM lparam);
#ifdef VIDEODEVICEDOC
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg LONG OnThreadSafeConnectErr(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnAutorunVideoDevices(WPARAM wparam, LPARAM lparam);
	afx_msg void OnViewWeb();
	afx_msg void OnViewFiles();
#endif
	DECLARE_MESSAGE_MAP()
};

// AfxGetMainWnd() returns the Main Window of the Current
// Thread and not of the Application!!!
inline CMainFrame* AfxGetMainFrame()
{
	CWinThread* pThread = (CWinThread*)::AfxGetApp();
	return (CMainFrame*)((pThread != NULL) ? pThread->GetMainWnd() : NULL);
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__9A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
