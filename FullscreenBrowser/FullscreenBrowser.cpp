/////////////////////////////////////////////////////////////////////////////
// FullscreenBrowser.cpp: Full Screen Browser Application
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "SimpleBrowser.h"
#include "winuser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DESKTOPNAME			_T("FullscreenBrowserDesktop85672235")
#define FULLSCREENSTART		_T("FullscreenStart:")
#define EXITSTRING_ENTRY	_T("ExitString")
#define DEFAULT_EXITSTRING	_T("")
#define DEFAULT_WEBPAGE		_T("http://localhost:8800")
#define APPNAME_NOEXT		_T("FullscreenBrowser")
#define MYCOMPANY			_T("Contaware")
#define INI_NAME_EXT		_T("FullscreenBrowser.ini")
#define INI_FILE			(CString(MYCOMPANY) + CString(_T("\\")) + CString(APPNAME_NOEXT) + CString(_T("\\")) + CString(INI_NAME_EXT))

/////////////////////////////////////////////////////////////////////////////
// FullscreenBrowserApp declaration
/////////////////////////////////////////////////////////////////////////////

class FullscreenBrowserApp : public CWinApp

{

public:

	FullscreenBrowserApp();
	static BOOL IsExistingFile(LPCTSTR lpszFileName);
	static BOOL IsExistingDir(LPCTSTR lpszFileName);
	CString GetDriveAndDirName(const CString& sFullFilePath);
	static BOOL CreateDir(LPCTSTR szNewDir);
	static CString GetSpecialFolderPath(int nSpecialFolder);
	CString m_sExitString;
	CString m_sUrl;
	BOOL m_bNewDesktop;

	//{{AFX_VIRTUAL(FullscreenBrowserApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(FullscreenBrowserApp)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

protected:
#ifdef LOCK_BROWSER
	BOOL TaskManagerEnableDisable(BOOL bEnableDisable);
	int KeysEnableDisable(BOOL bEnableDisable);
#endif
	BOOL StartProcess(CString sDesktopName, CString sPath);
	void ProcessDesktop(LPCTSTR szDesktopName, LPCTSTR szPath);
};

/////////////////////////////////////////////////////////////////////////////
// FullscreenBrowser declaration
/////////////////////////////////////////////////////////////////////////////

class FullscreenBrowserDlg;

class FullscreenBrowser : public SimpleBrowser {

public:

	virtual bool OnBeforeNavigate2(CString URL,
	                               CString frame,
								   void    *post_data,int post_data_size,
								   CString headers);
    virtual void OnDocumentComplete(CString URL);
    virtual void OnDownloadBegin();
    virtual void OnProgressChange(int progress,int progress_max);
    virtual void OnDownloadComplete();
    virtual void OnNavigateComplete2(CString URL);
    virtual void OnStatusTextChange(CString text);
    virtual void OnTitleChange(CString text);

	FullscreenBrowserDlg* Container;

};

/////////////////////////////////////////////////////////////////////////////
// FullscreenBrowserDlg declaration
/////////////////////////////////////////////////////////////////////////////

class FullscreenBrowserDlg : public CDialog
{

public:
	FullscreenBrowserDlg(CWnd* pParent = NULL);
	void Event(CString string);
	void OnOK();
	void OnCancel();

	//{{AFX_DATA(FullscreenBrowserDlg)
	enum { IDD = IDD_DIALOG };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(FullscreenBrowserDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

protected:
	CString m_sSecretCodeSequence;
	CSize GetMonitorSize(CWnd* pWnd = NULL);
	
	//{{AFX_MSG(FullscreenBrowserDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

    afx_msg void OnBeforeNavigate2(NMHDR *pNMHDR,LRESULT *pResult);
    afx_msg void OnDocumentComplete(NMHDR *pNMHDR,LRESULT *pResult);
    afx_msg void OnDownloadBegin(NMHDR *pNMHDR,LRESULT *pResult);
    afx_msg void OnProgressChange(NMHDR *pNMHDR,LRESULT *pResult);
    afx_msg void OnDownloadComplete(NMHDR *pNMHDR,LRESULT *pResult);
    afx_msg void OnNavigateComplete2(NMHDR *pNMHDR,LRESULT *pResult);
    afx_msg void OnStatusTextChange(NMHDR *pNMHDR,LRESULT *pResult);
    afx_msg void OnTitleChange(NMHDR *pNMHDR,LRESULT *pResult);

	DECLARE_MESSAGE_MAP()

private:
	FullscreenBrowser _FullscreenBrowser;

};

/////////////////////////////////////////////////////////////////////////////
// FullscreenBrowserApp implementation
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(FullscreenBrowserApp, CWinApp)
	//{{AFX_MSG_MAP(FullscreenBrowserApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

FullscreenBrowserApp _FullscreenBrowserApp;

FullscreenBrowserApp::FullscreenBrowserApp()
{
	m_bNewDesktop = FALSE;
}

#ifdef LOCK_BROWSER
// Enable / Disable Task Manager and others of the
// security Screen popped-up with CTRL+ALT+DEL
// NOTE: many anti-virus are detecting the modification
// of the following registry entries as a virus...
BOOL FullscreenBrowserApp::TaskManagerEnableDisable(BOOL bEnableDisable)
{
    #define KEY_SYSTEM_POLICIES		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System")
	#define KEY_EXPLORER_POLICIES	_T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer")
	#define KEY_IMAGE_FILE_EXEC_OPT	_T("Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options")
    HKEY    hKey;
    DWORD   val = 1U;
	DWORD	dwDisposition;

	// System Policies at user level to disable: Change Password, Task Manager and Lock Workstation
    if (::RegOpenKeyEx(HKEY_CURRENT_USER, KEY_SYSTEM_POLICIES, 0, KEY_WRITE | KEY_READ, &hKey) != ERROR_SUCCESS)
        if (::RegCreateKeyEx(HKEY_CURRENT_USER, KEY_SYSTEM_POLICIES, 0, (LPTSTR)NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, &dwDisposition) != ERROR_SUCCESS)
			return FALSE;
    if (bEnableDisable)
    {
        ::RegDeleteValue(hKey, _T("DisableChangePassword"));
		::RegDeleteValue(hKey, _T("DisableTaskMgr"));
		::RegDeleteValue(hKey, _T("DisableLockWorkstation"));
    }
    else
    {
        ::RegSetValueEx(hKey, _T("DisableChangePassword"), 0, REG_DWORD, (BYTE*)&val, sizeof(val));
		::RegSetValueEx(hKey, _T("DisableTaskMgr"), 0, REG_DWORD, (BYTE*)&val, sizeof(val));
		::RegSetValueEx(hKey, _T("DisableLockWorkstation"), 0, REG_DWORD, (BYTE*)&val, sizeof(val));
    }
    ::RegCloseKey(hKey);

	// Explorer Policies at user level to disable: Logoff, Shutdown, Restart and Sleep
	if (::RegOpenKeyEx(HKEY_CURRENT_USER, KEY_EXPLORER_POLICIES, 0, KEY_WRITE | KEY_READ, &hKey) != ERROR_SUCCESS)
        if (::RegCreateKeyEx(HKEY_CURRENT_USER, KEY_EXPLORER_POLICIES, 0, (LPTSTR)NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, &dwDisposition) != ERROR_SUCCESS)
			return FALSE;
	if (bEnableDisable)
    {
		::RegDeleteValue(hKey, _T("NoLogoff"));
		::RegDeleteValue(hKey, _T("NoClose"));
	}
    else
    {
        ::RegSetValueEx(hKey, _T("NoLogoff"), 0, REG_DWORD, (BYTE*)&val, sizeof(val));
		::RegSetValueEx(hKey, _T("NoClose"), 0, REG_DWORD, (BYTE*)&val, sizeof(val));
    }
	::RegCloseKey(hKey);

	// System Policies at computer level to disable: Switch User
	if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEY_SYSTEM_POLICIES, 0, KEY_WRITE | KEY_READ, &hKey) != ERROR_SUCCESS)
        if (::RegCreateKeyEx(HKEY_LOCAL_MACHINE, KEY_SYSTEM_POLICIES, 0, (LPTSTR)NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, &dwDisposition) != ERROR_SUCCESS)
			return FALSE;
	if (bEnableDisable)
		::RegDeleteValue(hKey, _T("HideFastUserSwitching"));
    else
        ::RegSetValueEx(hKey, _T("HideFastUserSwitching"), 0, REG_DWORD, (BYTE*)&val, sizeof(val));
	::RegCloseKey(hKey);

	// To avoid popping-up the "Task Manager has been disabled by your administrator" dialog under XP
	if (bEnableDisable)
    {
		if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEY_IMAGE_FILE_EXEC_OPT, 0, KEY_WRITE | KEY_READ, &hKey) == ERROR_SUCCESS)
			::RegDeleteKey(hKey, _T("TaskMgr.exe"));
	}
    else
    {
		if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEY_IMAGE_FILE_EXEC_OPT + CString(_T("\\TaskMgr.exe")), 0, KEY_WRITE | KEY_READ, &hKey) != ERROR_SUCCESS)
			if (::RegCreateKeyEx(HKEY_LOCAL_MACHINE, KEY_IMAGE_FILE_EXEC_OPT + CString(_T("\\TaskMgr.exe")), 0, (LPTSTR)NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, &dwDisposition) != ERROR_SUCCESS)
				return FALSE;
		TCHAR szData[] = _T("blank");
		DWORD dwBufferLength = (lstrlen(szData) + 1) * sizeof(TCHAR);
        ::RegSetValueEx(hKey, _T("Debugger"), 0, REG_SZ, (LPBYTE)szData, dwBufferLength);
    }
	::RegCloseKey(hKey);

    return TRUE;
}


// Note: CTRL+ALT+DEL Unfortunately doesn't work and
// hooking works only when not in debug mode!
HHOOK hKeyboardHook; 
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
    PKBDLLHOOKSTRUCT p;

    if (nCode == HC_ACTION) 
    {
        p = (PKBDLLHOOKSTRUCT) lParam;

        if (
            // WIN key (for Start Menu) APPS key (for right-click context menu) and F1 Help
            ((p->vkCode == VK_LWIN) || (p->vkCode == VK_RWIN) || (p->vkCode == VK_APPS) || (p->vkCode == VK_SNAPSHOT) || (p->vkCode == VK_F1)) || 
			// SHIFT+F10 (for right-click context menu)
            ((p->vkCode == VK_F10) && ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)) ||
            // ALT+TAB
            (p->vkCode == VK_TAB && p->flags & LLKHF_ALTDOWN) ||   
			// ALT+RETURN
            (p->vkCode == VK_RETURN && p->flags & LLKHF_ALTDOWN) ||   
            // ALT+ESC
            (p->vkCode == VK_ESCAPE && p->flags & LLKHF_ALTDOWN) ||    
            // CTRL+ESC
            ((p->vkCode == VK_ESCAPE) && ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0)) ||
            // CTRL+SHIFT+ESC
            ((p->vkCode == VK_ESCAPE) && ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0) && ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0))
            )
            return 1;
   }
                    
   return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}
int FullscreenBrowserApp::KeysEnableDisable(BOOL bEnableDisable)
{
    if (!bEnableDisable)
    {
        if (!hKeyboardHook)
		{
            /*
			I wanted to avoid the use of an external library,
			so I set the global hook inside my own application
			(without an external library). This is accomplished
			by passing in the instance handle of the application
			(and not the library as the documentation states)
			as the 3rd parameter
			*/
            hKeyboardHook  = ::SetWindowsHookEx(	WH_KEYBOARD_LL, 
													LowLevelKeyboardProc, 
													AfxGetInstanceHandle(), 
													0);
			if (!hKeyboardHook)
				return 0;
		}
    }
    else
    {
        ::UnhookWindowsHookEx(hKeyboardHook);
        hKeyboardHook = NULL;
    }

    return 1;
}
#endif

BOOL FullscreenBrowserApp::StartProcess(CString sDesktopName, CString sPath)
{
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
	LPTSTR szDesktopName = sDesktopName.GetBuffer(sDesktopName.GetLength());
	LPTSTR szPath = sPath.GetBuffer(sPath.GetLength());

    // Zero these structs
    ::ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
	si.lpTitle = szDesktopName;
	si.lpDesktop = szDesktopName;
    ::ZeroMemory(&pi, sizeof(pi));

    // Start the child process
    if (!::CreateProcess(NULL,  // No module name (use command line). 
                       szPath,  // Command line. 
                       NULL,    // Process handle not inheritable. 
                       NULL,    // Thread handle not inheritable. 
                       FALSE,   // Set handle inheritance to FALSE. 
                       0,       // No creation flags. 
                       NULL,    // Use parent's environment block. 
                       NULL,    // Use parent's starting directory. 
                       &si,     // Pointer to STARTUPINFO structure.
                       &pi))    // Pointer to PROCESS_INFORMATION structure.
    {
		sDesktopName.ReleaseBuffer();
		sPath.ReleaseBuffer();
        return FALSE;
    }
	else
	{
		sDesktopName.ReleaseBuffer();
		sPath.ReleaseBuffer();
	}

	// Wait until process exits
	::WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles
    ::CloseHandle(pi.hProcess);
    ::CloseHandle(pi.hThread);

	return TRUE;
}

void FullscreenBrowserApp::ProcessDesktop(LPCTSTR szDesktopName, LPCTSTR szPath)
{
	HDESK	hOriginalThread;
	HDESK	hOriginalInput;
	HDESK	hNewDesktop;

	// Save original
	hOriginalThread = ::GetThreadDesktop(GetCurrentThreadId());
	hOriginalInput = ::OpenInputDesktop(0, FALSE, DESKTOP_SWITCHDESKTOP);

	// Create a new Desktop and switch to it
	hNewDesktop = ::CreateDesktop(szDesktopName, NULL, NULL, 0, GENERIC_ALL, NULL);
	::SetThreadDesktop(hNewDesktop);
	::SwitchDesktop(hNewDesktop);

	// Execute process in new desktop
	StartProcess(szDesktopName, szPath);

	// Restore original
	::SwitchDesktop(hOriginalInput);
	::SetThreadDesktop(hOriginalThread);

	// Close the Desktop
	::CloseDesktop(hNewDesktop);
}

BOOL FullscreenBrowserApp::IsExistingFile(LPCTSTR lpszFileName)
{
	DWORD dwAttrib = GetFileAttributes(lpszFileName);
	if (dwAttrib != 0xFFFFFFFF)
	{
		// Directory
		if (!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;
}

BOOL FullscreenBrowserApp::IsExistingDir(LPCTSTR lpszFileName)
{
	DWORD dwAttrib = GetFileAttributes(lpszFileName);
	if (dwAttrib != 0xFFFFFFFF)
	{
		// Directory
		if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;
}

CString FullscreenBrowserApp::GetDriveAndDirName(const CString& sFullFilePath)
{
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	_tsplitpath(sFullFilePath, szDrive, szDir, NULL, NULL);
	return CString(szDrive) + CString(szDir);
}

// Iterative Solution Should Be Faster than a Recursive One!
BOOL FullscreenBrowserApp::CreateDir(LPCTSTR szNewDir)
{
	TCHAR szFolder[MAX_PATH];
	_tcsncpy(szFolder, szNewDir, MAX_PATH);
	szFolder[MAX_PATH - 1] = _T('\0');
	TCHAR* pStart = szFolder;
	TCHAR* pEnd = pStart + _tcslen(szFolder);
	TCHAR* p = pEnd;

	// Try 64 times to create the directory
	for (int i = 0 ; i < 64 ; i++)
	{
		BOOL bOK = CreateDirectory(szFolder, NULL);
		DWORD dwLastError = GetLastError();
		if (!bOK && dwLastError == ERROR_PATH_NOT_FOUND)
		{
			while (*p != _T('\\'))
			{
				if (p <= pStart)
					return FALSE;
				p--;
			}
			*p = NULL;
		}
		else if (bOK || (ERROR_ALREADY_EXISTS == dwLastError))
		{
			if (p >= pEnd)
				return TRUE;

			*p = _T('\\');
			while (*p)
				p++;
		}
		else
			break;
	}

	return FALSE;
}

CString FullscreenBrowserApp::GetSpecialFolderPath(int nSpecialFolder)
{
	TCHAR path[MAX_PATH] = {0};
	SHGetSpecialFolderPath(NULL, path, nSpecialFolder, FALSE);
	return CString(path);
}

BOOL FullscreenBrowserApp::InitInstance()
{
	AfxEnableControlContainer();

	// Get Module Name and Split it
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szName[_MAX_FNAME];
	TCHAR szProgramName[MAX_PATH];
	if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
		return FALSE;
	_tsplitpath(szProgramName, szDrive, szDir, szName, NULL);
	CString sDrive(szDrive);
	CString sDir(szDir);
	CString sName(szName);
	CString sDriveDir = sDrive + sDir;

	// Get Command Line
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	CString s(FULLSCREENSTART);

	// First free the string allocated by MFC at CWinApp startup.
	// The string is allocated before InitInstance is called.
	free((void*)m_pszProfileName);

	// Change the name of the .INI file
	CString sProfileName = GetSpecialFolderPath(CSIDL_APPDATA);
	if (sProfileName == _T(""))
		sProfileName = sDriveDir + INI_NAME_EXT;
	else
		sProfileName += _T("\\") + INI_FILE;
	CString sProfileNamePath = GetDriveAndDirName(sProfileName);
	if (!IsExistingDir(sProfileNamePath))
		CreateDir(sProfileNamePath);

	// The CWinApp destructor will free the memory
	m_pszProfileName = _tcsdup(sProfileName);

	// Force a unicode ini file by writing the UTF16-LE BOM (FFFE)
	if (!IsExistingFile(m_pszProfileName))
	{
		const WORD wBOM = 0xFEFF;
		DWORD NumberOfBytesWritten;
		HANDLE hFile = ::CreateFile(m_pszProfileName,
									GENERIC_WRITE, 0, NULL,
									CREATE_NEW,
									FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			::WriteFile(hFile, &wBOM, sizeof(WORD), &NumberOfBytesWritten, NULL);
			::CloseHandle(hFile);
		}
	}

	// All varieties of Windows NT since 3.51 include the ability to create and run multiple desktops.
	// Normally, this feature isn't used, and all applications run within the "Default" desktop.
	// However, if you want to create a kiosk-style application which has a full-screen interface,
	// and prevents Ctrl+Alt+Del or any of the other standard Windows options from being accessed,
	// then this technique is the way to do it up to Vista. From Windows 7 on the task manager is
	// shown in the current desktop...
	m_bNewDesktop = (BOOL)GetProfileInt(_T("General"), _T("NewDesktop"), FALSE);
	WriteProfileInt(_T("General"), _T("NewDesktop"), m_bNewDesktop);
	if (!m_bNewDesktop)
		cmdInfo.m_strFileName = s + cmdInfo.m_strFileName;
	if (!m_bNewDesktop || cmdInfo.m_strFileName.Find(s) == 0)
	{
		// Get Exit string
		m_sExitString = GetProfileString(_T("General"), EXITSTRING_ENTRY, DEFAULT_EXITSTRING);
		WriteProfileString(_T("General"), EXITSTRING_ENTRY, m_sExitString);
		
		// Get Url
		if (cmdInfo.m_strFileName.GetLength() > s.GetLength()) 
			m_sUrl = cmdInfo.m_strFileName.Mid(s.GetLength());
		if (m_sUrl == _T(""))
			m_sUrl = GetProfileString(_T("General"), _T("WebPage"), DEFAULT_WEBPAGE);
		WriteProfileString(_T("General"), _T("WebPage"), m_sUrl);
		
#ifdef LOCK_BROWSER
		// Keys disable, not working if debugging!
#ifndef _DEBUG
		KeysEnableDisable(FALSE);
#endif

		// Task Manager Disable
		TaskManagerEnableDisable(FALSE);
#endif

		// Start Dlg
		FullscreenBrowserDlg dlg;
		m_pMainWnd = &dlg;
		dlg.DoModal();

#ifdef LOCK_BROWSER
		// Task Manager Enable
		TaskManagerEnableDisable(TRUE);

		// Keys enable, not working if debugging!
#ifndef _DEBUG
		KeysEnableDisable(TRUE);
#endif
#endif
	}
	else
	{
		ProcessDesktop(DESKTOPNAME, CString(szProgramName) + CString(_T(" ")) +
									s + cmdInfo.m_strFileName);
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// FullscreenBrowserDlg implementation
/////////////////////////////////////////////////////////////////////////////

FullscreenBrowserDlg::FullscreenBrowserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(FullscreenBrowserDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(FullscreenBrowserDlg)
	//}}AFX_DATA_INIT
	_FullscreenBrowser.Container = this;
}

void FullscreenBrowserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FullscreenBrowserDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(FullscreenBrowserDlg, CDialog)
	//{{AFX_MSG_MAP(FullscreenBrowserDlg)
	//}}AFX_MSG_MAP
	ON_NOTIFY(SimpleBrowser::BeforeNavigate2,IDC_BROWSER,OnBeforeNavigate2)
	ON_NOTIFY(SimpleBrowser::DocumentComplete,IDC_BROWSER,OnDocumentComplete)
	ON_NOTIFY(SimpleBrowser::DownloadBegin,IDC_BROWSER,OnDownloadBegin)
	ON_NOTIFY(SimpleBrowser::ProgressChange,IDC_BROWSER,OnProgressChange)
	ON_NOTIFY(SimpleBrowser::DownloadComplete,IDC_BROWSER,OnDownloadComplete)
	ON_NOTIFY(SimpleBrowser::NavigateComplete2,IDC_BROWSER,OnNavigateComplete2)
	ON_NOTIFY(SimpleBrowser::StatusTextChange,IDC_BROWSER,OnStatusTextChange)
	ON_NOTIFY(SimpleBrowser::TitleChange,IDC_BROWSER,OnTitleChange)
END_MESSAGE_MAP()

CSize FullscreenBrowserDlg::GetMonitorSize(CWnd* pWnd/*=NULL*/)
{
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);
	int nMonitorWidth, nMonitorHeight;
	HMONITOR hMonitor;
	if (pWnd)
		hMonitor = MonitorFromWindow(pWnd->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
	else
		hMonitor = MonitorFromWindow(this->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
	if (!GetMonitorInfo(hMonitor, &monInfo))
		return CSize(0, 0);
	nMonitorWidth = monInfo.rcMonitor.right - monInfo.rcMonitor.left;
	nMonitorHeight = monInfo.rcMonitor.bottom - monInfo.rcMonitor.top;
	return CSize(nMonitorWidth, nMonitorHeight);
}

BOOL FullscreenBrowserDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Create Browser Ctrl
	_FullscreenBrowser.CreateFromControl(this, IDC_BROWSER);
	_FullscreenBrowser.PutSilent(true); // Dialog boxes are not displayed
	if (((FullscreenBrowserApp*)::AfxGetApp())->m_sUrl != _T(""))
		_FullscreenBrowser.Navigate(((FullscreenBrowserApp*)::AfxGetApp())->m_sUrl);
	else
		_FullscreenBrowser.Navigate(DEFAULT_WEBPAGE);

	// Full-Screen
#ifndef _DEBUG
	CSize szMonitor = GetMonitorSize();
	SetWindowPos(&wndTopMost, 0, 0, szMonitor.cx, szMonitor.cy, SWP_SHOWWINDOW);
	CWnd* pWnd = (CWnd*)GetDlgItem(IDC_BROWSER);
	if (pWnd)
	{
		CRect rc;
		GetClientRect(&rc);
		pWnd->MoveWindow(-2, -2, szMonitor.cx + 4, szMonitor.cy + 4);
	}
#endif
	return TRUE;
}

BSTR ToBSTR(LPCTSTR szFileName)
{
    return ::SysAllocString(szFileName);
}

BOOL FullscreenBrowserDlg::PreTranslateMessage(MSG* pMsg) 
{
	// When starting-up the wheel scroll is not working
	// till we first click inside the body of the html document.
	// I want that the wheel scroll works right when starting-up
	// -> implement wheel scroll here:
	if (pMsg->message == WM_MOUSEWHEEL)
	{
		short zDelta = (short) HIWORD(pMsg->wParam);
		HRESULT hr;
		IHTMLDocument2* pHtmlDoc = _FullscreenBrowser.GetDocument();
		ASSERT(pHtmlDoc);
		IHTMLElement* pBody = NULL;
		hr = pHtmlDoc->get_body(&pBody);
		ASSERT(SUCCEEDED(hr));
		ASSERT(pBody);
		IHTMLElement2* pBody2 = NULL;
		hr = pBody->QueryInterface(IID_IHTMLElement2,(void**)&pBody2);
		ASSERT(SUCCEEDED(hr));
		ASSERT(pBody2);
		long height = 0;
		pBody2->get_scrollHeight(&height);
		long clientheight = 0;
		pBody2->get_clientHeight(&clientheight);
		// Only if it is the body that has to be scrolled do it,
		// (sometimes it is not the body that has to be scrolled,
		// in this case let the control do it)
		if (clientheight < height)
		{
			BSTR bstr;
			if (zDelta > 0)
				bstr = ToBSTR(_T("scrollbarUp"));
			else
				bstr = ToBSTR(_T("scrollbarDown"));
			VARIANT var;
			var.vt = VT_BSTR;
			var.bstrVal = bstr;
			hr = pBody2->doScroll(var);
			hr = pBody2->doScroll(var);
			hr = pBody2->doScroll(var);
			::SysFreeString(bstr);
			return TRUE;
		}
	}
	else if (pMsg->message == WM_KEYDOWN)
	{
		// Secret code for exiting
		if (pMsg->wParam == VK_ESCAPE)
			m_sSecretCodeSequence = _T(":");
		else if (m_sSecretCodeSequence.GetLength() > ((FullscreenBrowserApp*)::AfxGetApp())->m_sExitString.GetLength() + 1)
			m_sSecretCodeSequence = _T("");
		else if (m_sSecretCodeSequence.GetLength() >= 1)
			m_sSecretCodeSequence += CString((char)pMsg->wParam);
		if (m_sSecretCodeSequence.CompareNoCase(_T(":") + ((FullscreenBrowserApp*)::AfxGetApp())->m_sExitString) == 0)
			CDialog::OnCancel();
	}
	else if (pMsg->message == WM_RBUTTONDOWN ||
			pMsg->message == WM_RBUTTONDBLCLK)
		return TRUE;
	
	return CDialog::PreTranslateMessage(pMsg);
}

void FullscreenBrowserDlg::OnOK()
{

}

void FullscreenBrowserDlg::OnCancel()
{

}

// Event handler

void FullscreenBrowserDlg::Event(CString string)
{
	//TRACE(string + _T("\n"));
}

// Notification handlers

void FullscreenBrowserDlg::OnBeforeNavigate2(NMHDR *pNMHDR,LRESULT *pResult)
{
	*pResult = 0;
}

void FullscreenBrowserDlg::OnDocumentComplete(NMHDR *pNMHDR,LRESULT *pResult)
{
	*pResult = 0;
}

void FullscreenBrowserDlg::OnDownloadBegin(NMHDR *pNMHDR,LRESULT *pResult)
{
	*pResult = 0;
}

void FullscreenBrowserDlg::OnProgressChange(NMHDR *pNMHDR,LRESULT *pResult)
{
	*pResult = 0;
}

void FullscreenBrowserDlg::OnDownloadComplete(NMHDR *pNMHDR,LRESULT *pResult)
{
	*pResult = 0;
}

void FullscreenBrowserDlg::OnNavigateComplete2(NMHDR *pNMHDR,LRESULT *pResult)
{
	*pResult = 0;
}

void FullscreenBrowserDlg::OnStatusTextChange(NMHDR *pNMHDR,LRESULT *pResult)
{
	*pResult = 0;
}

void FullscreenBrowserDlg::OnTitleChange(NMHDR *pNMHDR,LRESULT *pResult)
{
	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
// FullscreenBrowser implementation
/////////////////////////////////////////////////////////////////////////////

bool FullscreenBrowser::OnBeforeNavigate2(CString URL,
                                              CString frame,
								              void    *post_data,int post_data_size,
								              CString headers)
{
	SimpleBrowser::OnBeforeNavigate2(URL,frame,post_data,post_data_size,headers);

	CString event;
	event.Format(_T("OnBeforeNavigation: URL=\"%s\", frame=\"%s\", post_data=[0x%08X,%d bytes], headers=\"%s\""),
	             URL,frame,post_data,post_data_size,headers);
	Container->Event(event);
	return false;
}

void FullscreenBrowser::OnDocumentComplete(CString URL)
{
	SimpleBrowser::OnDocumentComplete(URL);

	CString event;
	event.Format(_T("OnDocumentComplete: URL=\"%s\""),
	             URL);
	Container->Event(event);
}

void FullscreenBrowser::OnDownloadBegin()
{
	SimpleBrowser::OnDownloadBegin();
	
	Container->Event(_T("OnDownloadBegin"));
}

void FullscreenBrowser::OnProgressChange(int progress,int progress_max)
{
	SimpleBrowser::OnProgressChange(progress,progress_max);
	
	CString event;
	event.Format(_T("OnProgressChange: progress=%d, progress_max=%d"),
	             progress,progress_max);
	Container->Event(event);
}

void FullscreenBrowser::OnDownloadComplete()
{
	SimpleBrowser::OnDownloadComplete();
	
	Container->Event(_T("OnDownloadComplete"));
}

void FullscreenBrowser::OnNavigateComplete2(CString URL)
{
	SimpleBrowser::OnNavigateComplete2(URL);
	
	CString event;
	event.Format(_T("OnNavigationComplete2: URL=\"%s\""),URL);
	Container->Event(event);
}

void FullscreenBrowser::OnStatusTextChange(CString text)
{
	SimpleBrowser::OnStatusTextChange(text);
	
	CString event;
	event.Format(_T("OnStatusTextChange: text=\"%s\""),text);
	Container->Event(event);
}

void FullscreenBrowser::OnTitleChange(CString text)
{
	SimpleBrowser::OnTitleChange(text);
	
	CString event;
	event.Format(_T("OnTitleChange: text=\"%s\""),text);
	Container->Event(event);
}
