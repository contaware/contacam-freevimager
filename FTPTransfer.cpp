#include "stdafx.h"
#include "resource.h"
#include "FTPTransfer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

const UINT WM_FTPTRANSFER_THREAD_FINISHED = WM_APP + 1;
const UINT WM_FTPTRANSFER_ASK_OVERWRITE_FILE = WM_APP + 2;

#pragma comment(lib, "wininet.lib") // Automatically link with wininet dll


// Class which handles dynamically calling function which must be constructed at run time
// since support for resuming FTP requires IE5 to be installed, which we do not
// want to limit the code to. To avoid the loader bringing up a message such as "Failed to 
// load due to missing export...", the function is constructed using GetProcAddress. The 
// CFTPTransfer function then checks to see if these function pointer is NULL and if it 
// returns FALSE and sets the last error to ERROR_CALL_NOT_IMPLEMENTED and fails the call

class _FTP_DOWNLOAD_DATA
{
public:
	_FTP_DOWNLOAD_DATA();
	~_FTP_DOWNLOAD_DATA();

	HINSTANCE m_hWininet;

	typedef BOOL (__stdcall FTPCOMMAND)(HINTERNET, BOOL, DWORD, LPCTSTR, DWORD, HINTERNET*);
	typedef FTPCOMMAND* LPFTPCOMMAND;

	LPFTPCOMMAND m_lpfnFtpCommand;
};

_FTP_DOWNLOAD_DATA::_FTP_DOWNLOAD_DATA()
{
	m_hWininet = ::GetModuleHandle(_T("WININET.DLL"));
	if (m_hWininet)
	{
#ifdef _UNICODE
		m_lpfnFtpCommand = (LPFTPCOMMAND)::GetProcAddress(m_hWininet, "FtpCommandW");
#else
		m_lpfnFtpCommand = (LPFTPCOMMAND)::GetProcAddress(m_hWininet, "FtpCommandA");
#endif
	}
}

_FTP_DOWNLOAD_DATA::~_FTP_DOWNLOAD_DATA()
{
	m_hWininet = NULL;
}

_FTP_DOWNLOAD_DATA _FtpDownloadData;

int CFTPTransfer::CMakeConnectionThread::Work() 
{
	ASSERT(m_pFTPTransfer->m_hFTPConnection == NULL);
	ASSERT(m_pFTPTransfer->m_sServer.GetLength());
	ASSERT(m_pFTPTransfer->m_hInternetSession);
	if (m_pFTPTransfer->m_sUserName.GetLength())
	{
		m_pFTPTransfer->m_hFTPConnection = ::InternetConnect(	m_pFTPTransfer->m_hInternetSession,
																m_pFTPTransfer->m_sServer,
																m_pFTPTransfer->m_nPort,
																m_pFTPTransfer->m_sUserName, 
																m_pFTPTransfer->m_sPassword,
																INTERNET_SERVICE_FTP,
																m_pFTPTransfer->m_bPasv ? INTERNET_FLAG_PASSIVE : 0,
																(DWORD)m_pFTPTransfer);
	}
	else
	{
		m_pFTPTransfer->m_hFTPConnection = ::InternetConnect(	m_pFTPTransfer->m_hInternetSession,
																m_pFTPTransfer->m_sServer,
																m_pFTPTransfer->m_nPort,
																NULL, 
																NULL,
																INTERNET_SERVICE_FTP,
																m_pFTPTransfer->m_bPasv ? INTERNET_FLAG_PASSIVE : 0,
																(DWORD)m_pFTPTransfer);
	}
	if (m_pFTPTransfer->m_hFTPConnection == NULL)
	{
		// Hive away this threads error code as we have use for it in the main thread
		m_pFTPTransfer->m_dwTempConnectionError = ::GetLastError();
	}
	return 0;
}

CFTPTransfer::CFTPTransfer(CWorkerThread* pThread)
{
	m_dwPercentage = 100;
	m_dwLastPercentage = 100;
	m_bShowMessageBoxOnError = TRUE;
	m_pThread = pThread;
	m_hInternetSession = NULL;
	m_hFTPConnection = NULL;
	m_hFTPFile = NULL;
	m_bSafeToClose = FALSE;
	m_bDownload = TRUE;
	m_nPort = INTERNET_DEFAULT_FTP_PORT;
	m_bBinary = TRUE;
	m_bPromptOverwrite = FALSE;
	m_dbLimit = 0.0;
	m_bPasv = FALSE;
	m_bUsePreconfig = TRUE;
	m_bUseProxy = FALSE;
	m_dwStartPos = 0;
	m_dwConnectionTimeout = FTP_CONNECTION_TIMEOUT_MS;
	m_dwTempConnectionError = ERROR_SUCCESS;
}

CFTPTransfer::~CFTPTransfer()
{
	Close();
}

BOOL CFTPTransfer::OpenLocalFile() 
{
	TCHAR szCause[255];
	CString sMsg;

	// Check to see if the file we are downloading exists and if
	// it does, then ask the user if they were it overwritten
	ASSERT(m_sLocalFile.GetLength());
	CFileStatus fs;
	BOOL bDownloadFileExists = CFile::GetStatus(m_sLocalFile, fs);
	if (m_bDownload && bDownloadFileExists && m_bPromptOverwrite)
	{
		sMsg.Format(ML_STRING(1388, "The file %s already exists.\nDo you want to replace it?"), m_sLocalFile);
		if (::AfxMessageBox(sMsg, MB_YESNO) != IDYES)
		{
			TRACE(_T("Failed to confirm file overwrite, download aborted\n"));
			return FALSE;
		}
	}

	// Try and open the file we will downloading into / uploading from
	if (m_bDownload)
	{
		DWORD dwFileFlags = 0;
		if (bDownloadFileExists)
			dwFileFlags = CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite | CFile::shareDenyWrite;
		else
			dwFileFlags = CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite;

		CFileException ex;
		if (!m_LocalFile.Open(m_sLocalFile, dwFileFlags, &ex))
		{
			ex.GetErrorMessage(szCause, 255);
			CString sError(szCause);
			sMsg.Format(ML_STRING(1772, "An error occured while opening the file to be downloaded:\n%s\n"), sError);
			TRACE(sMsg);
			if (m_bShowMessageBoxOnError)
				::AfxMessageBox(sMsg, MB_ICONSTOP);
			return FALSE;
		}

		// Seek to the end of the file
		try
		{
			m_LocalFile.Seek(m_dwStartPos, CFile::begin); 
			m_LocalFile.SetLength(m_dwStartPos);
		}
		catch (CFileException* pEx)                                         
		{
			pEx->GetErrorMessage(szCause, 255);
			pEx->Delete();
			CString sError(szCause);
			sMsg.Format(ML_STRING(1773, "An error occurred while seeking to the end of the file to be downloaded:\n%s\n"), sError);
			TRACE(sMsg);
			if (m_bShowMessageBoxOnError)
				::AfxMessageBox(sMsg, MB_ICONSTOP);
			return FALSE;
		}	
	}
	else
	{
		CFileException ex;
		if (!m_LocalFile.Open(m_sLocalFile, CFile::modeRead | CFile::shareDenyWrite, &ex))
		{
			ex.GetErrorMessage(szCause, 255);
			CString sError(szCause);
			sMsg.Format(ML_STRING(1774, "An error occured while opening the file to be uploaded:\n%s\n"), sError);
			TRACE(sMsg);
			if (m_bShowMessageBoxOnError)
				::AfxMessageBox(sMsg, MB_ICONSTOP);
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CFTPTransfer::ResumeTransfer(CString& sError)
{
	// Check that the FtpCommand function is available
	if (_FtpDownloadData.m_lpfnFtpCommand == NULL)
	{
		sError = _T("Error Command Missing\n");
		TRACE(_T("CFTPTransfer::ResumeTransfer, FtpCommand method not available in Wininet dll, FTP resumes are not supported\n"));
		return FALSE;
	}

	// Form the resume request
	CString sRequest;
	sRequest.Format(_T("REST %d"), m_dwStartPos);

	// Send the resume request
	BOOL bSuccess = _FtpDownloadData.m_lpfnFtpCommand(m_hFTPConnection, FALSE, FTP_TRANSFER_TYPE_BINARY, sRequest, 0, NULL); 
	if (!bSuccess)
	{
		DWORD dwLastError = ::GetLastError();
		TRACE(_T("CFTPTransfer::ResumeTransfer, Failed in call to FtpCommand, Error:%d\n"), dwLastError);
		CString sNumericError;
		sNumericError.Format(_T("%d"), dwLastError);
		sError.Format(_T("An error occurred while resuming the transfer, Error:%s"), sNumericError);
	}
	else
	{
		// Check the reponse to see if we get a "350" response code
		DWORD dwInetError;
		DWORD dwSize=0;
		::InternetGetLastResponseInfo(&dwInetError, NULL, &dwSize);
		TCHAR* pszResponse = new TCHAR[dwSize+1];
		::InternetGetLastResponseInfo(&dwInetError, pszResponse, &dwSize);
		pszResponse[dwSize] = _T('\0');

		// Check to see if there is a 350 response code in the extended error text anywhere
		bSuccess = (_ttoi(pszResponse) == 350);

		// Tidy up the heap memory now that we have finished with it
		delete [] pszResponse;

		if (!bSuccess)
		{
			CString sResponse(pszResponse);
			TRACE(_T("CFTPTransfer::ResumeTransfer, Resume not supported, Response: %s\n"), sResponse);
			sError.Format(_T("The server reports that resume is not supported, Error:%s"), sResponse);
		}
	}

	return bSuccess;
}

void CFTPTransfer::UpdatePercentage(DWORD dwTotalBytesDone, DWORD dwFileSize)
{
	if (dwFileSize > 0)
		m_dwPercentage = (DWORD) (dwTotalBytesDone * 100.0 / dwFileSize);
	else
		m_dwPercentage = 100;
	if (m_dwPercentage > m_dwLastPercentage)
	{
		OnTransferProgress(m_dwPercentage);
		m_dwLastPercentage = m_dwPercentage;
	}
}

// Return Values
// -1 : Do Exit Thread
// 0  : Error Uploading File
// 1  : Ok
int CFTPTransfer::Transfer()
{
	TCHAR szCause[255];
	CString sMsg;

	Close();

	// Adjust Remote File Name by replacing '\' with '/'
	int n = -1;
	do
	{
		n = m_sRemoteFile.Find(_T('\\'), ++n);
		if (n >= 0)
			m_sRemoteFile.SetAt(n, _T('/'));
	}
	while (n >= 0);
	m_sRemoteFile.TrimRight(_T('/'));

	// Open Local File
	if (!OpenLocalFile())
		return 0;

	// Should we exit the thread
    if (m_pThread && m_pThread->DoExit())
	{
		m_LocalFile.Close();
		if (m_bDownload)
			::DeleteFile(m_sLocalFile);
		return -1;
	}

	// Create the Internet session handle (if needed)
	ASSERT(m_hInternetSession == NULL);
    if (m_bUseProxy)
		m_hInternetSession = ::InternetOpen(AfxGetAppName(), INTERNET_OPEN_TYPE_PROXY, m_sProxy, NULL, 0);
	else if (m_bUsePreconfig)
		m_hInternetSession = ::InternetOpen(AfxGetAppName(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    else
		m_hInternetSession = ::InternetOpen(AfxGetAppName(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (m_hInternetSession == NULL)
    {
		::ShowLastError(m_bShowMessageBoxOnError);
		m_LocalFile.Close();
		if (m_bDownload)
			::DeleteFile(m_sLocalFile);
		return 0;
    }

    // Should we exit the thread
    if (m_pThread && m_pThread->DoExit())
	{
		m_LocalFile.Close();
		if (m_bDownload)
			::DeleteFile(m_sLocalFile);
		Close();
		return -1;
	}
 
	ASSERT(m_hInternetSession);

	// Setup the status callback function on the Internet session handle
	INTERNET_STATUS_CALLBACK pOldCallback = ::InternetSetStatusCallback(m_hInternetSession, _OnStatusCallBack);
	if (pOldCallback == INTERNET_INVALID_STATUS_CALLBACK)
	{
		::ShowLastError(m_bShowMessageBoxOnError);
		m_LocalFile.Close();
		if (m_bDownload)
			::DeleteFile(m_sLocalFile);
		Close();
		return 0;
	}

	// Should we exit the thread
    if (m_pThread && m_pThread->DoExit())
	{
		m_LocalFile.Close();
		if (m_bDownload)
			::DeleteFile(m_sLocalFile);
		if (m_hInternetSession && pOldCallback)
			::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
		Close();
		return -1;
	}  

	// Make the connection to the FTP server,
    // use a separate thread because InternetConnect()
	// is known to have a bug, which blocks the function infinitely!
	CMakeConnectionThread MakeConnectionThread(this);
    if (!MakeConnectionThread.Start())
    {
		TRACE(_T("Failed in create the connection thread\n"));
		m_LocalFile.Close();
		if (m_bDownload)
			::DeleteFile(m_sLocalFile);
		if (m_hInternetSession && pOldCallback)
			::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
		Close();
		return 0;
    }

    // Wait for the thread to return or a timeout occurs
	if (m_pThread == NULL)
	{
		DWORD Event = ::WaitForSingleObject(MakeConnectionThread.GetHandle(), m_dwConnectionTimeout);
		switch (Event)
		{
			// Make Connection Thread Terminated
			case WAIT_OBJECT_0 :		break;

			// Timeout
			case WAIT_TIMEOUT :			// Close the wininet session handle (which will cause the worker
										// thread to return from its blocking call which most likely will
										// be the call to InternetConnect)
										::InternetCloseHandle(m_hInternetSession);

										// Wait until the worker thread exits and then set the handle to NULL
										MakeConnectionThread.WaitDone_Blocking();
										m_hInternetSession = NULL;

										// Close
										m_LocalFile.Close();
										if (m_bDownload)
											::DeleteFile(m_sLocalFile);
										Close();

										sMsg = ML_STRING(1775, "Failed connecting to the FTP server in a timely manner\n");
										TRACE(sMsg);
										if (m_bShowMessageBoxOnError)
											::AfxMessageBox(sMsg, MB_ICONSTOP);

										return 0;
			
			default:					break;
		}
	}
	else
	{
		HANDLE hEventArray[2];
		hEventArray[0] = m_pThread->GetKillEvent();
		hEventArray[1] = MakeConnectionThread.GetHandle();
		DWORD Event = ::WaitForMultipleObjects(2, hEventArray, FALSE, m_dwConnectionTimeout);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :		// Close the wininet session handle (which will cause the worker
										// thread to return from its blocking call which most likely will
										// be the call to InternetConnect)
										::InternetCloseHandle(m_hInternetSession);

										// Wait until the worker thread exits and then set the handle to NULL
										MakeConnectionThread.WaitDone_Blocking();
										m_hInternetSession = NULL;

										// Close
										m_LocalFile.Close();
										if (m_bDownload)
											::DeleteFile(m_sLocalFile);
										Close();
										
										return -1;

			// Make Connection Thread Terminated
			case WAIT_OBJECT_0 + 1 :	break;

			// Timeout
			case WAIT_TIMEOUT :			// Close the wininet session handle (which will cause the worker
										// thread to return from its blocking call which most likely will
										// be the call to InternetConnect)
										::InternetCloseHandle(m_hInternetSession);
										
										// Wait until the worker thread exits and then set the handle to NULL
										MakeConnectionThread.WaitDone_Blocking();
										m_hInternetSession = NULL;

										// Close
										m_LocalFile.Close();
										if (m_bDownload)
											::DeleteFile(m_sLocalFile);
										Close();

										sMsg = ML_STRING(1775, "Failed connecting to the FTP server in a timely manner\n");
										TRACE(sMsg);
										if (m_bShowMessageBoxOnError)
											::AfxMessageBox(sMsg, MB_ICONSTOP);

										return 0;
			
			default:					break;
		}
	}

    // Check to see if the connection was successful
    if (m_hFTPConnection == NULL)
    {
		::ShowError(m_dwTempConnectionError, m_bShowMessageBoxOnError);
		m_LocalFile.Close();
		if (m_bDownload)
			::DeleteFile(m_sLocalFile);
		if (m_hInternetSession && pOldCallback)
			::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
		Close();
		return 0;
    }

    // Should we exit the thread
    if (m_pThread && m_pThread->DoExit())
	{
		m_LocalFile.Close();
		if (m_bDownload)
			::DeleteFile(m_sLocalFile);
		if (m_hInternetSession && pOldCallback)
			::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
		Close();
		return -1;
	}
 
	ASSERT(m_hFTPConnection);

	// Get the length of the file to transfer            
	DWORD dwFileSize = 0;
	BOOL bGotFileSize = FALSE;
	if (m_bDownload)
	{
		WIN32_FIND_DATA wfd;
		HINTERNET hFind = ::FtpFindFirstFile(m_hFTPConnection, m_sRemoteFile, &wfd, INTERNET_FLAG_RELOAD | 
											 INTERNET_FLAG_DONT_CACHE, (DWORD) this); 
		if (hFind)
		{
			bGotFileSize = TRUE;
			dwFileSize = (DWORD)wfd.nFileSizeLow;

			// Destroy the enumeration handle now that we are finished with it
			::InternetCloseHandle(hFind);
		}
	}
	else
	{
		bGotFileSize = TRUE;
		dwFileSize = (DWORD)m_LocalFile.GetLength();
	}

	// Should we exit the thread
    if (m_pThread && m_pThread->DoExit())
	{
		m_LocalFile.Close();
		if (m_bDownload)
			::DeleteFile(m_sLocalFile);
		if (m_hInternetSession && pOldCallback)
			::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
		Close();
		return -1;
	}

	// Check to see if the file already exists on the server  
	if (!m_bDownload && m_bPromptOverwrite)
	{
		WIN32_FIND_DATA wfd;
		HINTERNET hFind = ::FtpFindFirstFile(m_hFTPConnection, m_sRemoteFile, &wfd, INTERNET_FLAG_RELOAD | 
											 INTERNET_FLAG_DONT_CACHE, (DWORD) this); 
		BOOL bFound = FALSE;
		if (hFind)
		{
			bFound = TRUE;

			// Destroy the enumeration handle now that we are finished with it
			InternetCloseHandle(hFind);
		}

		if (bFound)  
		{
			sMsg.Format(ML_STRING(1388, "The file %s already exists.\nDo you want to replace it?"), m_sRemoteFile);
			if (::AfxMessageBox(sMsg, MB_YESNO) == IDNO)
			{
				m_LocalFile.Close();
				if (m_hInternetSession && pOldCallback)
					::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
				Close();
				return 0;
			}
		}
	}

	// Should we exit the thread
    if (m_pThread && m_pThread->DoExit())
	{
		m_LocalFile.Close();
		if (m_bDownload)
			::DeleteFile(m_sLocalFile);
		if (m_hInternetSession && pOldCallback)
			::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
		Close();
		return -1;
	}

	// Resume the transfer if requested to do so
	CString sError;
	if (m_dwStartPos && !ResumeTransfer(sError))
	{
		m_LocalFile.Close();
		if (m_bDownload)
			::DeleteFile(m_sLocalFile);
		if (m_hInternetSession && pOldCallback)
			::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
		Close();
		return 0;
	}

	// Open the remote file
	ASSERT(m_hFTPFile == NULL);
	if (m_bDownload)
	{
		if (m_bBinary)
			m_hFTPFile = ::FtpOpenFile(m_hFTPConnection, m_sRemoteFile, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY | 
								   INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE, (DWORD) this);
		else
			m_hFTPFile = ::FtpOpenFile(m_hFTPConnection, m_sRemoteFile, GENERIC_READ, FTP_TRANSFER_TYPE_ASCII | 
								   INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE, (DWORD) this);
	}
	else
	{
		if (m_bBinary)  
			m_hFTPFile = ::FtpOpenFile(m_hFTPConnection, m_sRemoteFile, GENERIC_WRITE, FTP_TRANSFER_TYPE_BINARY | 
								   INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE, (DWORD) this);
		else
			m_hFTPFile = ::FtpOpenFile(m_hFTPConnection, m_sRemoteFile, GENERIC_WRITE, FTP_TRANSFER_TYPE_ASCII | 
								   INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE, (DWORD) this);
	}
	if (m_hFTPFile == NULL)
	{
		BOOL bFtpOpenFileError = TRUE;

		// Do We Have To Create A Directory?
		if (!m_bDownload)
		{
			CString sRemoteDir = m_sRemoteFile;
			int pos;
			if ((pos = sRemoteDir.ReverseFind(_T('/'))) > -1)
			{
				sRemoteDir = sRemoteDir.Left(pos);
				if (CreateRemoteDir(sRemoteDir))
				{
					if (m_bBinary)  
						m_hFTPFile = ::FtpOpenFile(m_hFTPConnection, m_sRemoteFile, GENERIC_WRITE, FTP_TRANSFER_TYPE_BINARY | 
											   INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE, (DWORD) this);
					else
						m_hFTPFile = ::FtpOpenFile(m_hFTPConnection, m_sRemoteFile, GENERIC_WRITE, FTP_TRANSFER_TYPE_ASCII | 
											   INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE, (DWORD) this);

					if (m_hFTPFile != NULL)
						bFtpOpenFileError = FALSE;
				}
			}
		}

		// Ftp Open File Error?
		if (bFtpOpenFileError)
		{
			::ShowLastError(m_bShowMessageBoxOnError);
			m_LocalFile.Close();
			if (m_bDownload)
				::DeleteFile(m_sLocalFile);
			if (m_hInternetSession && pOldCallback)
				::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
			Close();
			return 0;
		}
	}

	// Should we exit the thread
    if (m_pThread && m_pThread->DoExit())
	{
		m_LocalFile.Close();
		if (m_bDownload)
			::DeleteFile(m_sLocalFile);
		if (m_hInternetSession && pOldCallback)
			::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
		Close();
		return -1;
	}

	// Now do the actual reading / writing of the file
	DWORD dwStartTicks = ::GetTickCount();
	DWORD dwBytesRead = 0;
	DWORD dwBytesWritten = 0;
	char szReadBuf[1024];
	DWORD dwBytesToRead = 1024;
	DWORD dwTotalBytesRead = 0;
	DWORD dwTotalBytesWritten = 0;
	m_dwLastPercentage = 0;
	double q = 0.0;
	do
	{
		if (m_bDownload)
		{
			// Read from the remote file
			if (!::InternetReadFile(m_hFTPFile, szReadBuf, dwBytesToRead, &dwBytesRead))
			{
				::ShowLastError(m_bShowMessageBoxOnError);
				m_LocalFile.Close();
				::DeleteFile(m_sLocalFile);
				if (m_hInternetSession && pOldCallback)
				  ::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
				Close();
				return 0;
			}
			else if (dwBytesRead)
			{
				// Write the data to file
				try
				{
					m_LocalFile.Write(szReadBuf, dwBytesRead);
				}
				catch (CFileException* pEx)
				{
					pEx->GetErrorMessage(szCause, 255);
					sError = CString(szCause);
					sMsg.Format(ML_STRING(1776, "An exception occured while writing to the download file:\n%s\n"), sError);
					TRACE(sMsg);
					if (m_bShowMessageBoxOnError)
						::AfxMessageBox(sMsg, MB_ICONSTOP);
					m_LocalFile.Close();
					::DeleteFile(m_sLocalFile);
					if (m_hInternetSession && pOldCallback)
						::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
					Close();
					pEx->Delete();
					return 0;
				}

				// For bandwidth throttling
				if (m_dbLimit > 0.0) 
				{
					double t = (double)(::GetTickCount() - dwStartTicks);
					q = (double)((double)dwTotalBytesRead / t);
					if (q > m_dbLimit)	 
						::Sleep((DWORD)((((q*t)/m_dbLimit)-t)));
				}

				// Increment the total number of bytes read
				dwTotalBytesRead += dwBytesRead;

				// Statistics
				if (bGotFileSize)
					UpdatePercentage(dwTotalBytesRead, dwFileSize);
			}
		}
		else
		{
			// Read the data from the local file
			try
			{
				dwBytesRead = m_LocalFile.Read(szReadBuf, 1024);
			}
			catch(CFileException* pEx)
			{
				pEx->GetErrorMessage(szCause, 255);
				sError = CString(szCause);
				sMsg.Format(ML_STRING(1777, "An exception occured while reading the local file:\n%s\n"), sError);
				TRACE(sMsg);
				if (m_bShowMessageBoxOnError)
					::AfxMessageBox(sMsg, MB_ICONSTOP);
				m_LocalFile.Close();
				if (m_hInternetSession && pOldCallback)
				  ::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
				Close();
				pEx->Delete();
				return 0;
			}

			// Write to the remote file
			if (dwBytesRead)
			{
				if (!::InternetWriteFile(m_hFTPFile, szReadBuf, dwBytesRead, &dwBytesWritten))
				{
					::ShowLastError(m_bShowMessageBoxOnError);
					m_LocalFile.Close();
					if (m_hInternetSession && pOldCallback)
						::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
					Close();
					return 0;
				}
				else if (dwBytesWritten)
				{
					// Increment the total number of bytes read
					dwTotalBytesWritten += dwBytesWritten;

					// Statistics
					if (bGotFileSize)
						UpdatePercentage(dwTotalBytesWritten, dwFileSize);
				}

				// For bandwidth throttling
				if (m_dbLimit > 0.0) 
				{
					double t = (double)(::GetTickCount() - dwStartTicks);
					q = (double)((double)dwTotalBytesWritten / t);
					if (q > m_dbLimit)	 
						::Sleep((DWORD)((((q*t)/m_dbLimit)-t)));
				}
			}
		}

		// Should we exit the thread
		if (m_pThread && m_pThread->DoExit())
		{
			m_LocalFile.Close();
			if (m_bDownload)
				::DeleteFile(m_sLocalFile);
			if (m_hInternetSession && pOldCallback)
				::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
			Close();
			return -1;
		}
	} 
	while (dwBytesRead);

	// Close
	m_LocalFile.Close();
	if (m_hInternetSession && pOldCallback)
		::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
	Close();

	return 1;
}

BOOL CFTPTransfer::Test()
{
	CString sMsg;

	Close();

	// Create the Internet session handle (if needed)
	ASSERT(m_hInternetSession == NULL);
    if (m_bUseProxy)
		m_hInternetSession = ::InternetOpen(AfxGetAppName(), INTERNET_OPEN_TYPE_PROXY, m_sProxy, NULL, 0);
	else if (m_bUsePreconfig)
		m_hInternetSession = ::InternetOpen(AfxGetAppName(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    else
		m_hInternetSession = ::InternetOpen(AfxGetAppName(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (m_hInternetSession == NULL)
    {
		::ShowLastError(m_bShowMessageBoxOnError);
		return FALSE;
    }

	// Setup the status callback function on the Internet session handle
	INTERNET_STATUS_CALLBACK pOldCallback = ::InternetSetStatusCallback(m_hInternetSession, _OnStatusCallBack);
	if (pOldCallback == INTERNET_INVALID_STATUS_CALLBACK)
	{
		::ShowLastError(m_bShowMessageBoxOnError);
		Close();
		return FALSE;
	}

	// Make the connection to the FTP server,
    // use a separate thread because InternetConnect()
	// is known to have a bug, which blocks the function infinitely!
	CMakeConnectionThread MakeConnectionThread(this);
    if (!MakeConnectionThread.Start())
    {
		TRACE(_T("Failed in create the connection thread\n"));
		if (m_hInternetSession && pOldCallback)
			::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
		Close();
		return FALSE;
    }

    // Wait for the thread to return or a timeout occurs
	DWORD Event = ::WaitForSingleObject(MakeConnectionThread.GetHandle(), m_dwConnectionTimeout);
    switch (Event)
	{
		// Make Connection Thread Terminated
		case WAIT_OBJECT_0 :		break;

		// Timeout
		case WAIT_TIMEOUT :			// Close the wininet session handle (which will cause the worker
									// thread to return from its blocking call which most likely will
									// be the call to InternetConnect)
									::InternetCloseHandle(m_hInternetSession);

									// Wait until the worker thread exits and then set the handle to NULL
									MakeConnectionThread.WaitDone_Blocking();
									m_hInternetSession = NULL;

									// Close
									Close();

									sMsg = ML_STRING(1775, "Failed connecting to the FTP server in a timely manner\n");
									TRACE(sMsg);
									if (m_bShowMessageBoxOnError)
										::AfxMessageBox(sMsg, MB_ICONSTOP);

									return FALSE;
		
		default:					break;
	}

    // Check to see if the connection was successful
    if (m_hFTPConnection == NULL)
    {
		::ShowError(m_dwTempConnectionError, m_bShowMessageBoxOnError);
		if (m_hInternetSession && pOldCallback)
			::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
		Close();
		return FALSE;
    }

	// Close
	if (m_hInternetSession && pOldCallback)
		::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
	Close();

	return TRUE;
}

BOOL CFTPTransfer::CreateRemoteDir(CString sDirName)
{
	if (::FtpCreateDirectory(m_hFTPConnection, sDirName))
		return TRUE;
	else
	{
		// First Create Parent Dir(s)
		int pos;
		if ((pos = sDirName.ReverseFind(_T('/'))) > -1)
		{
			if (CreateRemoteDir(sDirName.Left(pos)))
				return ::FtpCreateDirectory(m_hFTPConnection, sDirName);
			else
				return FALSE;
		}
		else
			return FALSE;
	}
}

void CALLBACK CFTPTransfer::_OnStatusCallBack(HINTERNET hInternet, DWORD dwContext, DWORD dwInternetStatus, 
                                                  LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
	// Convert from the SDK C world to the C++ world
	CFTPTransfer* pFTP = (CFTPTransfer*)dwContext;
	ASSERT(pFTP);
	pFTP->OnStatusCallBack(hInternet, dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
}

void CFTPTransfer::OnStatusCallBack(HINTERNET /*hInternet*/, DWORD dwInternetStatus, 
                                         LPVOID lpvStatusInformation, DWORD /*dwStatusInformationLength*/)
{
	switch (dwInternetStatus)
	{
		case INTERNET_STATUS_RESOLVING_NAME:
		{
			break;
		}
		case INTERNET_STATUS_NAME_RESOLVED:
		{
			break;
		}
		case INTERNET_STATUS_CONNECTING_TO_SERVER:
		{
			break;
		}
		case INTERNET_STATUS_CONNECTED_TO_SERVER:
		{
			break;
		}
		default:
		{
			break;
		}
	}
}

void CFTPTransfer::Close() 
{
	if (m_hFTPFile)
	{
		::InternetCloseHandle(m_hFTPFile);
		m_hFTPFile = NULL;
	}

	if (m_hFTPConnection)
	{
		::InternetCloseHandle(m_hFTPConnection);
		m_hFTPConnection = NULL;
	}

	if (m_hInternetSession)
	{
		::InternetCloseHandle(m_hInternetSession);
		m_hInternetSession = NULL;
	}
}

#endif
