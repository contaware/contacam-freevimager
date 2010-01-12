#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <winbase.h>
#include <winsvc.h>
#include <process.h>
#include <ntsecapi.h>
#include <lm.h>

#pragma comment(lib, "netapi32.lib")

#define STRINGBUFSIZE				500
#define MAXPROCCOUNT				127
#define INTERFACE_WAIT_TIME			2000
#define ENDPROCESS_POLLTIME			100
#define ENDWORKERTHREAD_TIMEOUT		7000
HANDLE g_hKillEvent = NULL;
HANDLE g_hWorkerThread = NULL;
TCHAR g_pServiceName[STRINGBUFSIZE+1];
TCHAR g_pExeFile[STRINGBUFSIZE+1];
TCHAR g_pInitFile[STRINGBUFSIZE+1];
TCHAR g_pLogFile[STRINGBUFSIZE+1];
PROCESS_INFORMATION g_pProcInfo[MAXPROCCOUNT];
SERVICE_STATUS g_serviceStatus; 
SERVICE_STATUS_HANDLE g_hServiceStatusHandle; 
CRITICAL_SECTION g_myCS;
VOID WINAPI ContaCamServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
VOID WINAPI ContaCamServiceHandler(DWORD fdwControl);

void WriteLog(LPCTSTR pMsg)
{
	// write error or other information into log file
	EnterCriticalSection(&g_myCS);
	try
	{
		SYSTEMTIME oT;
		GetLocalTime(&oT);
		FILE* pLog = _tfopen(g_pLogFile, _T("a"));
		_ftprintf(pLog,_T("%02d/%02d/%04d, %02d:%02d:%02d\n    %s\n"),oT.wMonth,oT.wDay,oT.wYear,oT.wHour,oT.wMinute,oT.wSecond,pMsg); 
		fclose(pLog);
	} catch(...) {}
	LeaveCriticalSection(&g_myCS);
}

SERVICE_TABLE_ENTRY DispatchTable[] = 
{ 
	{g_pServiceName, ContaCamServiceMain}, 
	{NULL, NULL}
};

BOOL StartProcess(int nIndex) 
{
	STARTUPINFO startUpInfo = {sizeof(STARTUPINFO),NULL,_T(""),NULL,0,0,0,0,0,0,0,STARTF_USESHOWWINDOW,0,0,NULL,0,0,0};  
	TCHAR pItem[STRINGBUFSIZE+1];
	swprintf(pItem, STRINGBUFSIZE+1, _T("Process%d\0"), nIndex);
	TCHAR pCommandLine[STRINGBUFSIZE+1];
	GetPrivateProfileString(pItem, _T("CommandLine"), _T(""), pCommandLine, STRINGBUFSIZE, g_pInitFile);
	if (_tcslen(pCommandLine) > 4)
	{
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szName[_MAX_FNAME];
		TCHAR szExt[_MAX_EXT];
		TCHAR szProgramName[MAX_PATH];
		_tsplitpath(pCommandLine, szDrive, szDir, szName, szExt);
		if (szDrive[0] == _T('\0'))
		{
			if (GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
				return FALSE;
			_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
			TCHAR pTemp[STRINGBUFSIZE+1];
			_tcscpy(pTemp, pCommandLine);
			pCommandLine[0] = _T('\0');
			_tcscat(pCommandLine, szDrive);
			_tcscat(pCommandLine, szDir);
			_tcscat(pCommandLine, pTemp);
		}

		// set the correct desktop for the process to be started
		TCHAR CurrentDesktopName[512];
		HDESK hCurrentDesktop = GetThreadDesktop(GetCurrentThreadId());
		DWORD len;
		GetUserObjectInformation(hCurrentDesktop, UOI_NAME, CurrentDesktopName, MAX_PATH, &len);
		startUpInfo.wShowWindow = SW_HIDE;
		startUpInfo.lpDesktop = CurrentDesktopName;

		// create the process
		if (CreateProcess(NULL,pCommandLine,NULL,NULL,TRUE,NORMAL_PRIORITY_CLASS,NULL,NULL,&startUpInfo,&g_pProcInfo[nIndex]))
		{
			TCHAR pStartProcessWait[STRINGBUFSIZE+1];
			GetPrivateProfileString(pItem, _T("StartProcessWait"), _T("500"), pStartProcessWait, STRINGBUFSIZE, g_pInitFile);
			Sleep(_ttoi(pStartProcessWait));
			return TRUE;
		}
		else
		{
			long nError = GetLastError();
			TCHAR pTemp[121];
			swprintf(pTemp, 121, _T("Failed to start program '%s', error code = %d"), pCommandLine, nError); 
			WriteLog(pTemp);
			return FALSE;
		}
	}
	else
		return FALSE;
}

BOOL CALLBACK EnumChildWindowCallBack(HWND hwnd, LPARAM lParam) 
{ 
	BOOL* pbClosePosted = (BOOL*)lParam;
	TCHAR szTitle[MAX_PATH];
	GetWindowText(hwnd, szTitle, MAX_PATH);
	if (szTitle[0] != _T('\0')) 
	{
		*pbClosePosted = TRUE;
		PostMessage(hwnd, WM_CLOSE, 0, 0);
		return false; // Stop enumerating
	} 
	else
		return true; // Keep enumerating
}

BOOL CALLBACK EnumWindowCallBack(HWND hwnd, LPARAM lParam) 
{ 
	BOOL* pbClosePosted = (BOOL*)lParam;
	TCHAR szTitle[MAX_PATH];
	GetWindowText(hwnd, szTitle, MAX_PATH);
	if (szTitle[0] != _T('\0'))
	{
		*pbClosePosted = TRUE;
		PostMessage(hwnd, WM_CLOSE, 0, 0);
		return false; // Stop enumerating
	} 
	else
	{
		EnumChildWindows(hwnd, EnumChildWindowCallBack, lParam);
		if (*pbClosePosted)
			return false; // Stop enumerating
		else
			return true; // Keep enumerating
	}
}

BOOL PostCloseMessage(DWORD dwThreadId)
{
	BOOL bClosePosted = FALSE;
	EnumThreadWindows(dwThreadId, EnumWindowCallBack, (LPARAM)&bClosePosted);
	return bClosePosted;
}

void EndProcess(int nIndex) 
{	
	if (g_pProcInfo[nIndex].hProcess)
	{
		// Get the EndProcessTimeout param
		TCHAR pItem[STRINGBUFSIZE+1];
		swprintf(pItem, STRINGBUFSIZE+1, _T("Process%d\0"), nIndex);
		TCHAR pEndProcessTimeout[STRINGBUFSIZE+1];
		GetPrivateProfileString(pItem, _T("EndProcessTimeout"), _T("5000"), pEndProcessTimeout, STRINGBUFSIZE, g_pInitFile);
		int nEndProcessTimeout = _ttoi(pEndProcessTimeout);
		
		// Post a WM_CLOSE message, if it fails post a WM_QUIT
		if (!PostCloseMessage(g_pProcInfo[nIndex].dwThreadId))
			PostThreadMessage(g_pProcInfo[nIndex].dwThreadId, WM_QUIT, 0, 0);
		
		// Poll the exit code
		int nCountDown = nEndProcessTimeout / ENDPROCESS_POLLTIME;
		BOOL bProcessExited = FALSE;
		DWORD dwCode;
		while (GetExitCodeProcess(g_pProcInfo[nIndex].hProcess, &dwCode) && nCountDown >= 0)
		{
			if (dwCode != STILL_ACTIVE)
			{
				bProcessExited = TRUE;
				break;
			}
			Sleep(ENDPROCESS_POLLTIME);
			nCountDown--;
		}
		
		// Terminate the process by force
		if (!bProcessExited)
		{
			TCHAR pTemp[121];
			swprintf(pTemp, 121, _T("Forced process%d termination"), nIndex);
			WriteLog(pTemp);
			TerminateProcess(g_pProcInfo[nIndex].hProcess, 0);
		}

		// Close handles to avoid ERROR_NO_SYSTEM_RESOURCES
		try
		{
			CloseHandle(g_pProcInfo[nIndex].hThread);
			CloseHandle(g_pProcInfo[nIndex].hProcess);
		}
		catch(...) {}
		g_pProcInfo[nIndex].hProcess = 0;
		g_pProcInfo[nIndex].hThread = 0;
	}
}

////////////////////////////////////////////////////////////////////// 
//
// This routine gets used to start your service
//
VOID WINAPI ContaCamServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
	DWORD   status = 0; 
    DWORD   specificError = 0xfffffff; 
 
    g_serviceStatus.dwServiceType        = SERVICE_WIN32; 
    g_serviceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    g_serviceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE; 
    g_serviceStatus.dwWin32ExitCode      = 0; 
    g_serviceStatus.dwServiceSpecificExitCode = 0; 
    g_serviceStatus.dwCheckPoint         = 0; 
    g_serviceStatus.dwWaitHint           = 0; 
 
    g_hServiceStatusHandle = RegisterServiceCtrlHandler(g_pServiceName, ContaCamServiceHandler); 
    if (g_hServiceStatusHandle == 0) 
    {
		long nError = GetLastError();
		TCHAR pTemp[121];
		swprintf(pTemp, 121, _T("RegisterServiceCtrlHandler failed, error code = %d"), nError);
		WriteLog(pTemp);
        return; 
    } 
 
    // Initialization complete -> report running status 
    g_serviceStatus.dwCurrentState       = SERVICE_RUNNING; 
    g_serviceStatus.dwCheckPoint         = 0; 
    g_serviceStatus.dwWaitHint           = 0;  
    if (!SetServiceStatus(g_hServiceStatusHandle, &g_serviceStatus)) 
    { 
		long nError = GetLastError();
		TCHAR pTemp[121];
		swprintf(pTemp, 121, _T("SetServiceStatus failed, error code = %d"), nError);
		WriteLog(pTemp);
    } 

	// Start processes
	for (int i = 0 ; i < MAXPROCCOUNT ; i++)
	{
		g_pProcInfo[i].hProcess = 0;
		StartProcess(i);
	}
}

////////////////////////////////////////////////////////////////////// 
//
// This routine responds to events concerning your service, like start/stop
//
VOID WINAPI ContaCamServiceHandler(DWORD fdwControl)
{
	switch (fdwControl) 
	{
		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:
		{
			// Stop worker thread
			SetEvent(g_hKillEvent);
			if (WaitForSingleObject(g_hWorkerThread, ENDWORKERTHREAD_TIMEOUT) != WAIT_OBJECT_0)
			{
				// If it doesn't want to exit force the termination
				if (g_hWorkerThread)
				{
					WriteLog(_T("Forced worker thread termination"));
					TerminateThread(g_hWorkerThread, 0);
				}
			}
			if (g_hWorkerThread)
			{
				CloseHandle(g_hWorkerThread);
				g_hWorkerThread = NULL;
			}

			// Set status
			g_serviceStatus.dwWin32ExitCode = 0; 
			g_serviceStatus.dwCurrentState  = SERVICE_STOPPED; 
			g_serviceStatus.dwCheckPoint    = 0; 
			g_serviceStatus.dwWaitHint      = 0;

			// Terminate all processes started by this service before shutdown
			for (int i = MAXPROCCOUNT - 1 ; i >= 0 ; i--)
			{
				EndProcess(i);
			}			
			if (!SetServiceStatus(g_hServiceStatusHandle, &g_serviceStatus))
			{ 
				long nError = GetLastError();
				TCHAR pTemp[121];
				swprintf(pTemp, 121, _T("SetServiceStatus failed, error code = %d"), nError);
				WriteLog(pTemp);
			}

			return; 
		}
		case SERVICE_CONTROL_PAUSE:
			g_serviceStatus.dwCurrentState = SERVICE_PAUSED; 
			break;
		case SERVICE_CONTROL_CONTINUE:
			g_serviceStatus.dwCurrentState = SERVICE_RUNNING; 
			break;
		case SERVICE_CONTROL_INTERROGATE:
			break;
		default: 
			break;
	};
    if (!SetServiceStatus(g_hServiceStatusHandle, &g_serviceStatus)) 
	{ 
		long nError = GetLastError();
		TCHAR pTemp[121];
		swprintf(pTemp, 121, _T("SetServiceStatus failed, error code = %d"), nError);
		WriteLog(pTemp);
    } 
}

unsigned int __stdcall WorkerProc(void* lpParam)
{
	// Get CheckProcessSeconds
	TCHAR pCheckProcess[STRINGBUFSIZE+1];
	GetPrivateProfileString(_T("Settings"), _T("CheckProcessSeconds"), _T("60"), pCheckProcess, STRINGBUFSIZE, g_pInitFile);
	int nCheckProcessSeconds = _ttoi(pCheckProcess);
	if (nCheckProcessSeconds <= 0)
		return 0;

	// Main loop
	while (TRUE)
	{
		DWORD Event = WaitForSingleObject(g_hKillEvent, 1000 * nCheckProcessSeconds);
		switch (Event)
		{
			case WAIT_OBJECT_0 :
				ResetEvent(g_hKillEvent);
				return 0;
			case WAIT_TIMEOUT :
			{
				for (int i = 0 ; i < MAXPROCCOUNT ; i++)
				{
					if (g_pProcInfo[i].hProcess)
					{
						TCHAR pItem[STRINGBUFSIZE+1];
						swprintf(pItem, STRINGBUFSIZE+1, _T("Process%d\0"), i);
						TCHAR pRestart[STRINGBUFSIZE+1];
						GetPrivateProfileString(pItem, _T("Restart"), _T("No"), pRestart, STRINGBUFSIZE, g_pInitFile);
						if (pRestart[0] == _T('Y') || pRestart[0] == _T('y') || pRestart[0] == _T('1'))
						{
							DWORD dwCode;
							if (GetExitCodeProcess(g_pProcInfo[i].hProcess, &dwCode))
							{
								if (dwCode != STILL_ACTIVE)
								{
									try // close handles to avoid ERROR_NO_SYSTEM_RESOURCES
									{
										CloseHandle(g_pProcInfo[i].hThread);
										CloseHandle(g_pProcInfo[i].hProcess);
									}
									catch(...) {}
									if (StartProcess(i))
									{
										TCHAR pTemp[121];
										swprintf(pTemp, 121, _T("Restarted process%d"), i);
										WriteLog(pTemp);
									}
								}
							}
							else
							{
								long nError = GetLastError();
								TCHAR pTemp[121];
								swprintf(pTemp, 121, _T("GetExitCodeProcess failed, error code = %d"), nError);
								WriteLog(pTemp);
							}
						}
					}
				}
				break;
			}
			default:
				return 0;
		}
	}
}

// If not running returns:
// ERROR_SERVICE_NOT_ACTIVE
DWORD KillService(LPCTSTR pName) 
{ 
	DWORD dwError = ERROR_SUCCESS;
	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS); 
	if (schSCManager == 0) 
		dwError = GetLastError();
	else
	{
		SC_HANDLE schService = OpenService(schSCManager, pName, SERVICE_ALL_ACCESS);
		if (schService == 0)
			dwError = GetLastError();
		else
		{
			SERVICE_STATUS status;
			if (!ControlService(schService, SERVICE_CONTROL_STOP, &status))
				dwError = GetLastError();
			CloseServiceHandle(schService); 
		}
		CloseServiceHandle(schSCManager); 
	}
	return dwError;
}

// If already running returns:
// ERROR_SERVICE_ALREADY_RUNNING
DWORD RunService(LPCTSTR pName, int nArg, LPCTSTR* pArg) 
{
	DWORD dwError = ERROR_SUCCESS;
	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS); 
	if (schSCManager == 0) 
		dwError = GetLastError();
	else
	{
		SC_HANDLE schService = OpenService(schSCManager, pName, SERVICE_ALL_ACCESS);
		if (schService == 0)
			dwError = GetLastError();
		else
		{
			if (!StartService(schService, nArg, pArg))
				dwError = GetLastError();
			CloseServiceHandle(schService); 
		}
		CloseServiceHandle(schSCManager); 
	}
	return dwError;
}

// If the service is not installed it returns:
// ERROR_SERVICE_DOES_NOT_EXIST
DWORD Uninstall(LPCTSTR pName)
{
	DWORD dwError = ERROR_SUCCESS;
	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS); 
	if (schSCManager == 0) 
		dwError = GetLastError();
	else
	{
		SC_HANDLE schService = OpenService(schSCManager, pName, SERVICE_ALL_ACCESS);
		if (schService == 0)
			dwError = GetLastError();
		else
		{
			if (!DeleteService(schService)) 
				dwError = GetLastError();
			CloseServiceHandle(schService); 
		}
		CloseServiceHandle(schSCManager);	
	}
	return dwError;
}

// If pServiceStartName is not existing it returns:
// ERROR_INVALID_SERVICE_ACCOUNT
DWORD Install(LPCTSTR pPath, LPCTSTR pName, LPCTSTR pServiceStartName, LPCTSTR pServiceStartPassword) 
{
	DWORD dwError = ERROR_SUCCESS;
	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE); 
	if (schSCManager == 0) 
		dwError = GetLastError();
	else
	{
		SC_HANDLE schService = CreateService
		( 
			schSCManager,				// SCManager database
			pName,						// name of service
			pName,						// service name to display
			SERVICE_ALL_ACCESS,			// desired access
			SERVICE_WIN32_OWN_PROCESS | (pServiceStartName[0] == _T('\0') ? SERVICE_INTERACTIVE_PROCESS : 0),// service type
			SERVICE_AUTO_START,			// start type
			SERVICE_ERROR_NORMAL,		// error control type
			pPath,						// service's binary
			NULL,						// no load ordering group
			NULL,						// no tag identifier
			NULL,						// no dependencies
			pServiceStartName[0] != _T('\0') ? pServiceStartName : NULL, // set NULL for LocalSystem account
			pServiceStartPassword[0] != _T('\0') ? pServiceStartPassword : NULL  // set NULL for no password 
		);
		if (schService == 0)
			dwError = GetLastError();
		else
			CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
	}
	return dwError;
}

void InitLsaString(PLSA_UNICODE_STRING LsaString, LPWSTR String)
{
    USHORT StringLength;
	
    if (String == NULL)
    {
		LsaString->Buffer = NULL;
		LsaString->Length = 0;
		LsaString->MaximumLength = 0;
		return;
    }
    // Get the length of the string without the null terminator
    StringLength = wcslen(String);

    // Store the string
    LsaString->Buffer = String;
    LsaString->Length = StringLength * sizeof(WCHAR);
    LsaString->MaximumLength = (StringLength+1) * sizeof(WCHAR);
}

BOOL GetCurrentLoggedUser(TCHAR* pUserName)
{
	BOOL res = FALSE;
    WKSTA_USER_INFO_1* pUserInfo;
	NET_API_STATUS nets = NetWkstaUserGetInfo(NULL, 1, (LPBYTE*)&pUserInfo);
    if (nets == NERR_Success && pUserInfo)
    {
		swprintf(pUserName, STRINGBUFSIZE+1, _T("%s\\%s"), pUserInfo->wkui1_logon_domain, pUserInfo->wkui1_username);
		res = TRUE;
    }
    if (pUserInfo)
		NetApiBufferFree(pUserInfo);
	return res;
}

BOOL AddServiceLogonRight(LPCTSTR pUserName)
{
    // Open the policy object on the target machine
	LSA_OBJECT_ATTRIBUTES ObjectAttributes;
	LSA_HANDLE PolicyHandle;
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));
    NTSTATUS nts = LsaOpenPolicy(NULL, &ObjectAttributes, POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES, &PolicyHandle);
    if (nts != ERROR_SUCCESS) 
		return FALSE;

    // Obtain the SID of the user and set the privilege
	TCHAR DomName[MAX_PATH];
    SID* UserSid = (SID*)malloc(MAX_PATH);
	SID_NAME_USE SidUse;
	DWORD SidSize = MAX_PATH;
	DWORD DomSize = MAX_PATH;
	nts = 1; // set error
    if (LookupAccountName(NULL, pUserName, UserSid, &SidSize, DomName, &DomSize, &SidUse))
    {
		LSA_UNICODE_STRING lucPrivilege;
		InitLsaString(&lucPrivilege, L"SeServiceLogonRight");
		nts = LsaAddAccountRights(PolicyHandle, UserSid, &lucPrivilege, 1);
    }

	// Clean-up
    if (UserSid)
		free(UserSid);
    if (PolicyHandle)
		LsaClose(PolicyHandle);

	// Return
    if (nts == ERROR_SUCCESS)
		return TRUE;
    else 
		return FALSE;
}

TCHAR GetCharNoEcho(HANDLE hConsole)
{
	DWORD dwOldInputMode;
	DWORD NumberOfCharsRead = 0;
	TCHAR ch[4] = {0}; // Be safe!
	GetConsoleMode(hConsole, &dwOldInputMode);
	SetConsoleMode(hConsole, dwOldInputMode & ~ENABLE_LINE_INPUT & ~ENABLE_ECHO_INPUT);
	// ReadConsole is buggy when inputing while sleeping it then
	// returns 2 chars even if 1 is passed as param!
	ReadConsole(hConsole, ch, 1, &NumberOfCharsRead, NULL);
	SetConsoleMode(hConsole, dwOldInputMode);
	return ch[0];
}

void GetPw(TCHAR* sPw)
{
	HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	TCHAR ch;
	memset(sPw, 0, sizeof(TCHAR) * (STRINGBUFSIZE + 1));
	for (int i = 0 ; (i < STRINGBUFSIZE) && ((ch = GetCharNoEcho(hStdIn)) != _T('\r')) ; )
	{	
		if (ch == VK_BACK)
		{
			if (i > 0)
			{
				CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
				GetConsoleScreenBufferInfo(hStdOut, &ConsoleScreenBufferInfo);
				COORD coordScreen = ConsoleScreenBufferInfo.dwCursorPosition; 
				coordScreen.X -= 1;
				SetConsoleCursorPosition(hStdOut, coordScreen);
				DWORD dwCharsWritten;
				const TCHAR ClearOut[] = _T(" ");
				WriteConsole(hStdOut, (CONST VOID*)ClearOut, 1, &dwCharsWritten, NULL);
				SetConsoleCursorPosition(hStdOut, coordScreen);
				i--;
				sPw[i] = _T('\0');
			}
		}
		else
		{
			sPw[i] = ch;
			DWORD dwCharsWritten;
			const TCHAR HideOut[] = _T("*");
			WriteConsole(hStdOut, (CONST VOID*)HideOut, 1, &dwCharsWritten, NULL);
			i++;
		}
	}
	_tprintf(_T("\n"));
}

void _tmain(int argc, TCHAR* argv[])
{
	// initialize global critical section
	InitializeCriticalSection(&g_myCS);

	// initialize variables
	TCHAR pModuleFile[STRINGBUFSIZE+1];
	DWORD dwSize = GetModuleFileName(NULL, pModuleFile, STRINGBUFSIZE);
	pModuleFile[dwSize] = _T('\0');
	if (dwSize > 4 && pModuleFile[dwSize-4] == _T('.'))
	{
		_tsplitpath(pModuleFile, NULL, NULL, g_pServiceName, NULL);
		swprintf(g_pExeFile, STRINGBUFSIZE+1, _T("%s"), pModuleFile);
		pModuleFile[dwSize-4] = _T('\0');
		swprintf(g_pInitFile, STRINGBUFSIZE+1, _T("%s.ini"), pModuleFile);
		swprintf(g_pLogFile, STRINGBUFSIZE+1, _T("%s.log"), pModuleFile);
	}
	else
	{
		_tprintf(_T("Invalid module file name: %s\r\n"), pModuleFile);
		return;
	}

	// uninstall service
	if (argc==2&&_tcsicmp(_T("-u"),argv[1])==0)
	{
		_tprintf(_T("Uninstalling %s, please wait...\n\n"), g_pServiceName);
		KillService(g_pServiceName);
		int nRet = Uninstall(g_pServiceName);
		if (nRet == ERROR_SUCCESS)
			_tprintf(_T("%s uninstalled"), g_pServiceName);
		else if (nRet == ERROR_SERVICE_DOES_NOT_EXIST)
			_tprintf(_T("%s is not installed"), g_pServiceName);
		else
			_tprintf(_T("Failed to uninstall %s!"), g_pServiceName);
		Sleep(INTERFACE_WAIT_TIME);
	}
	// install service
	else if (argc==2&&_tcsicmp(_T("-i"),argv[1])==0)
	{
		_tprintf(_T("Installing %s, please wait...\n\n"), g_pServiceName);
		TCHAR pServiceStartName[STRINGBUFSIZE+1];
		TCHAR pServiceStartPassword[STRINGBUFSIZE+1];
		int nRet;
		while (TRUE)
		{
			KillService(g_pServiceName);
			Uninstall(g_pServiceName);
			if (GetCurrentLoggedUser(pServiceStartName))
				_tprintf(_T("Logon Username: %s\n"), pServiceStartName);
			else
			{
				TCHAR pTemp[STRINGBUFSIZE+1];
				_tprintf(_T("Logon Username: "));
				_getts_s(pTemp, STRINGBUFSIZE+1);
				_tcscpy(pServiceStartName, _T(".\\"));
				_tcscat(pServiceStartName, pTemp);
			}
			_tprintf(_T("Logon Password: "));
			GetPw(pServiceStartPassword);
			nRet = Install(g_pExeFile, g_pServiceName, pServiceStartName, pServiceStartPassword);
			if (nRet != ERROR_INVALID_SERVICE_ACCOUNT)
			{
				if (pServiceStartName[0] != _T('\0'))
					AddServiceLogonRight(pServiceStartName);
				if ((nRet = RunService(g_pServiceName,0,NULL)) != ERROR_SERVICE_LOGON_FAILED)
					break;
				else
				{
					if (pServiceStartName[0] != _T('\0') && pServiceStartPassword[0] == _T('\0'))
					{
						_tprintf(_T("\
Logon failed!\n\n\
Empty passwords may not be supported by your security policy\n\
-> disable Security Settings\\Local Policies\\Security Options\\Accounts:\n\
Limit local account use of blank passwords to console logon only\n\
-> or set LimitBlankPasswordUse to 0 under the registry key\n\
HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Lsa\n\n"));
					}
					else
						_tprintf(_T("Logon failed! (wrong password)\n\n"));
				}
			}
			else
				_tprintf(_T("Logon failed! (invalid account)\n\n"));
		}
		if (nRet == ERROR_SUCCESS)
			_tprintf(_T("\n%s installed and running"), g_pServiceName);
		else
			_tprintf(_T("\nFailed to install %s!"), g_pServiceName);
		Sleep(INTERFACE_WAIT_TIME);
	}
	// run service
	else if (argc==2&&_tcsicmp(_T("-r"),argv[1])==0)
	{			
		_tprintf(_T("Starting service, please wait...\n\n"));
		int nRet = RunService(g_pServiceName,0,NULL);
		if (nRet == ERROR_SUCCESS)
			_tprintf(_T("%s started"), g_pServiceName);
		else if (nRet == ERROR_SERVICE_ALREADY_RUNNING)
			_tprintf(_T("%s is already running"), g_pServiceName);
		else
			_tprintf(_T("Failed to start %s!"), g_pServiceName);
		Sleep(INTERFACE_WAIT_TIME);
	}
	// kill service
	else if (argc==2&&_tcsicmp(_T("-k"),argv[1])==0)
	{
		_tprintf(_T("Stopping service, please wait...\n\n"));
		int nRet = KillService(g_pServiceName);
		if (nRet == ERROR_SUCCESS)
			_tprintf(_T("%s stopped"), g_pServiceName);
		else if (nRet == ERROR_SERVICE_NOT_ACTIVE)
			_tprintf(_T("%s is already stopped"), g_pServiceName);
		else
			_tprintf(_T("Failed to stop %s!"), g_pServiceName);
		Sleep(INTERFACE_WAIT_TIME);
	}
	// assume user is starting this service 
	else 
	{
		// start a worker thread to check for dead programs (and restart if necessary)
		g_hKillEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if ((int)(g_hWorkerThread = (HANDLE)_beginthreadex(NULL, 0, WorkerProc, NULL, 0, NULL)) == 0)
		{
			long nError = GetLastError();
			TCHAR pTemp[121];
			swprintf(pTemp, 121, _T("_beginthreadex failed, error code = %d"), nError);
			WriteLog(pTemp);
		}
		// pass dispatch table to service controller
		if (!StartServiceCtrlDispatcher(DispatchTable))
		{
			long nError = GetLastError();
			TCHAR pTemp[121];
			swprintf(pTemp, 121, _T("StartServiceCtrlDispatcher failed, error code = %d"), nError);
			WriteLog(pTemp);
		}
		// you don't get here unless the service is shutdown
		if (g_hKillEvent)
			CloseHandle(g_hKillEvent);
	}

	// delete global critical section
	DeleteCriticalSection(&g_myCS);
}

