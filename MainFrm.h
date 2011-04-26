// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__9A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
#define AFX_MAINFRM_H__9A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TwainCpp.h"
#include "TryEnterCriticalSection.h"
#include "Progress.h"
#include "Trayicon.h"
#include "StatusBarACT.h"
#include "MDIClientWnd.h"
extern "C"
{
#include "tiffiop.h"
}

// Window Message IDs
#define WM_TASKBAR_BUTTON							0x0313
#define WM_THREADSAFE_STATUSTEXT					WM_USER + 100
#define WM_THREADSAFE_OPEN_DOC						WM_USER + 101
#define WM_THREADSAFE_AVISTART_SHRINKTO				WM_USER + 102
#define WM_SHRINKDOC_TERMINATED						WM_USER + 103
#define WM_ALL_CLOSED								WM_USER + 104
#define WM_VIDEOAVI_FULLSCREEN_MODE_ON				WM_USER + 105
#define WM_VIDEOAVI_FULLSCREEN_MODE_OFF				WM_USER + 106
#define WM_SCANANDEMAIL								WM_USER + 107
#define WM_TRAY_NOTIFICATION						WM_USER + 108
#ifdef VIDEODEVICEDOC
#define WM_AUTORUN_VIDEODEVICES						WM_USER + 109
#endif
														
#define ID_TIMER_FULLSCREEN							1
#define ID_TIMER_CROP_SCROLL						2
#define ID_TIMER_TRANSPARENCY						3
#define ID_TIMER_CLOSING_ALL						4
#define ID_TIMER_CLOSING_PICTUREDOC					5
#define ID_TIMER_CLOSING_VIDEOAVIDOC				6
#define ID_TIMER_CLOSING_VIDEODEVICEDOC				7
#define ID_TIMER_CDAUDIO							8
#define ID_TIMER_MCIAUDIO							9
#define ID_TIMER_PROGRESS							10
#define ID_TIMER_AVIPLAYSLIDER						11
#ifdef VIDEODEVICEDOC
#define ID_TIMER_GENERALDLG  						12
#define ID_TIMER_ONESEC_POLL						13
#define ID_TIMER_ASSISTANTDLG						14
#endif

#define FULLSCREEN_TIMER_MS							1000U	// ms
#define AUTOSCROLL_TIMER_MS							120U	// ms
#define CLOSING_CHECK_INTERVAL_TIMER_MS				100U	// ms
#define AVIPLAYSLIDER_TIMER_MS						300U	// ms
#ifdef VIDEODEVICEDOC
#define ONESEC_POLL_TIMER_MS						1000U	// ms
#endif

#ifdef VIDEODEVICEDOC
#define AUTORUN_VIDEODEVICES_MAX_RETRIES			6
#define AUTORUN_VIDEODEVICES_RETRY_DELAY			10000	// ms
#define MICROAPACHE_WATCHDOG_CHECK_TIME				10000U	// ms
#endif

#define FULLSCREEN_SAFEPAUSED_FRAMES_TIMEOUT		3
#define FULLSCREEN_MIN_SAFEPAUSED_TIMEOUT			1000U	// ms
#define FULLSCREEN_DELAYEDRESTART_TIMEOUT			3000U	// ms

// Closing wait time
#define MAX_CLOSE_CHILDFRAME_WAITTIME				15000U	// ms

// Temporary Scan & Email Directory
#define TMP_SCAN_AND_EMAIL_DIR						_T("ScanAndEmail")

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
	void TwainSetImage(HANDLE hDib, int width, int height, int bpp);
	void TwainCopyImage(HANDLE hBitmap,TW_IMAGEINFO& info);
	BOOL TwainCanClose();
	CString TwainSetImageMM(CDib* pDib, int width, int height, int bpp);
	void EnterExitFullscreen();
	void Progress(int nPercent);

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

	// Are Modeless Dialogs Visible?
	BOOL AreModelessDlgsVisible();

	// Toolbar and Statusbar
	CToolBar* GetToolBar() {return &m_wndToolBar;};
	CString* GetStatusBarString() {m_sStatusBarString;};
	void StatusText(CString sText = _T("")); // If sText is _T("") the Idle message is displayed
	CStatusBar* GetStatusBar() {return &m_wndStatusBar;};

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
	int m_nToolsMenuPos;
	int m_nWindowsPos;
	int m_nHelpMenuPos;

	// Help Menu Items Count
	int m_nHelpMenuItemsCount;

	// Full screen flag
	volatile bool m_bFullScreenMode;

	// Session disconnected or locked flag
	BOOL m_bSessionDisconnectedLocked;

	// Progress control
	CProgressCtrl m_Progress;

	// Batch Processing dialog
	CBatchProcDlg* m_pBatchProcDlg;

	// IMAPI2 Burn dialog
	CIMAPI2Dlg* m_pIMAPI2Dlg;

	// Menu Item Strings that have been deleted,
	// store them here for later restore
	CString m_sPlayGifMenuItem;
	CString m_sStopGifMenuItem;
	CString m_sNextPageFrameMenuItem;
	CString m_sPrevPageFrameMenuItem;
	HMENU m_hJPEGAdvancedMenu;
	CString m_sJPEGAdvancedMenuItem;

	// Tray Icon
	CTrayIcon m_TrayIcon;

	// Main frame background window
	CMDIClientWnd m_MDIClientWnd;

protected:
	CString m_sSecretCodeSequence;
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

	// Scan Vars
	BOOL m_bScanAndEmail;
	TIFF* m_TiffScan;

protected:
	void FullScreenModeOn(HWND hChildWndSafePaused = NULL);
	void FullScreenModeOff(HWND hChildWndSafePaused = NULL);
	void RestoreAllFrames();
	void ClearFrontAll();
	void ChangeCoordinatesUnit();
#ifdef VIDEODEVICEDOC
	void PopulateCaptureMenu(CMenu* pPopupMenu);
#endif
	void PopulateHelpMenu(CMenu* pPopupMenu);

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
	afx_msg void OnMove(int x, int y);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnFileAcquireToTiff();
	afx_msg void OnFileAcquireToPdf();
	afx_msg void OnFileAcquireAndEmail();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnOpenFromTray();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnEndSession(BOOL bEnding);
	afx_msg void OnFileAcquireToPdfDirect();
	afx_msg void OnFileLockTwainclose();
	afx_msg void OnUpdateFileLockTwainclose(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnXCoordinatesDoubleClick();
	afx_msg void OnYCoordinatesDoubleClick();
	afx_msg LONG OnProgress(WPARAM wparam, LPARAM lparam);
	afx_msg void OnUpdateIndicatorXCoordinate(CCmdUI* pCmdUI);
	afx_msg void OnUpdateIndicatorYCoordinate(CCmdUI* pCmdUI);
	LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnThreadSafeStatusText(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafeOpenDoc(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnThreadSafeAviStartShrinkTo(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnShrinkDocTerminated(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnTaskBarButton(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnRestoreAllFrames(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnAllClosed(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnVideoAviFullScreenModeOn(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnVideoAviFullScreenModeOff(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnScanAndEmail(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnTrayNotification(WPARAM uID, LPARAM lEvent);
	afx_msg LRESULT OnCopyData(WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnSessionChange(WPARAM wparam, LPARAM lparam);
#ifdef VIDEODEVICEDOC
	afx_msg LONG OnAutorunVideoDevices(WPARAM wparam, LPARAM lparam);
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
