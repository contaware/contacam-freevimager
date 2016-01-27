#include "stdafx.h"
#include "resource.h"
#include "FTPTransfer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

// Automatically link with wininet dll
#pragma comment(lib, "wininet.lib")

/*
KB224318 shows a workaround to the InternetSetOption API bug on setting timeout
values by creating a second thread (KB176420):
Create a worker thread to call the blocking WinInet APIs. If the connection takes
more time than the specified timeout value, the original thread will call
InternetCloseHandle to release the blocking WinInet function.
Note: this timeout bug is present up to IE6 (included)
*/
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
		m_pFTPTransfer->m_sError = ::ShowErrorMsg(::GetLastError(), FALSE);
	return 0;
}

CFTPTransfer::CFTPTransfer(CWorkerThread* pThread)
{
	m_dwLastPercentage = 100;
	m_pThread = pThread;
	m_hInternetSession = NULL;
	m_hFTPConnection = NULL;
	m_hFTPFile = NULL;
	m_bDownload = TRUE;
	m_nPort = INTERNET_DEFAULT_FTP_PORT;
	m_bPromptOverwrite = FALSE;
	m_dBandwidthLimit = 0.0;
	m_bPasv = FALSE;
	m_bUsePreconfig = TRUE;
	m_dwStartPos = 0;
}

CFTPTransfer::~CFTPTransfer()
{
	Close();
}

BOOL CFTPTransfer::OpenLocalFile() 
{
	TCHAR szCause[255];

	// Check to see if the file we are downloading exists and if
	// it does, then ask the user if he wants it overwritten
	// (if not resuming the download)
	CFileStatus fs;
	BOOL bDownloadFileExists = CFile::GetStatus(m_sLocalFile, fs);
	if (m_bDownload && bDownloadFileExists && m_bPromptOverwrite && m_dwStartPos == 0)
	{
		CString sMsg;
		sMsg.Format(ML_STRING(1388, "The file %s already exists.\nDo you want to replace it?"), m_sLocalFile);
		if (::AfxMessageBox(sMsg, MB_YESNO) == IDNO)
			return FALSE;
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
			m_sError.Format(ML_STRING(1772, "An error occurred while opening the file to be downloaded:\n%s\n"), szCause);
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
			m_sError.Format(ML_STRING(1773, "An error occurred while seeking to the end of the file to be downloaded:\n%s\n"), szCause);
			return FALSE;
		}
	}
	else
	{
		CFileException ex;
		if (!m_LocalFile.Open(m_sLocalFile, CFile::modeRead | CFile::shareDenyWrite, &ex))
		{
			ex.GetErrorMessage(szCause, 255);
			m_sError.Format(ML_STRING(1774, "An error occurred while opening the file to be uploaded:\n%s\n"), szCause);
			return FALSE;
		}

		// Seek to the resume point
		try
		{
			m_LocalFile.Seek(m_dwStartPos, CFile::begin);
		}
		catch (CFileException* pEx)                                         
		{
			pEx->GetErrorMessage(szCause, 255);
			pEx->Delete();
			m_sError.Format(ML_STRING(1789, "An error occurred while seeking to the resume point of the file to be uploaded:\n%s\n"), szCause);
			return FALSE;
		}
	}

	return TRUE;
}

/*
FTP Upload Resume
-----------------
The FTP client sends a SIZE command to get the current size of the 
partially uploaded file on the FTP server. If the REST FTP command is 
supported by the server, then the SIZE command should also be supported, 
see RFC 3659. The FTP client then sends a REST command with the current 
file size. The file data is then sent starting at the byte position 
previously indicated. 

FTP Download Resume
-------------------
This is the same as upload, except that no SIZE command is required. The 
FTP client simply knows the size of the partially downloaded file 
because it exists in the local filesystem on the client.
*/
BOOL CFTPTransfer::ResumeTransfer()
{
	// Form the resume request
	CString sRequest;
	sRequest.Format(_T("REST %d"), m_dwStartPos);

	// Send the resume request
	BOOL bSuccess = ::FtpCommand(m_hFTPConnection, FALSE, FTP_TRANSFER_TYPE_BINARY, sRequest, 0, NULL); 
	if (!bSuccess)
		m_sError = sRequest + _T(" -> ") + ::ShowErrorMsg(::GetLastError(), FALSE);
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
		if (!bSuccess)
			m_sError = sRequest + _T(" -> ") + pszResponse;

		// Tidy up the heap memory now that we have finished with it
		delete [] pszResponse;
	}

	return bSuccess;
}

void CFTPTransfer::UpdatePercentage(DWORD dwTotalBytesDone, DWORD dwFileSize)
{
	DWORD dwPercentage = 100;
	if (dwFileSize > 0)
		dwPercentage = (DWORD) (dwTotalBytesDone * 100.0 / dwFileSize);
	if (dwPercentage > m_dwLastPercentage)
	{
		OnTransferProgress(dwPercentage);
		m_dwLastPercentage = dwPercentage;
	}
}

// Return Values
// -1 : Do Exit Thread
// 0  : Error Uploading File (when user rejects file overwrite m_sError is empty)
// 1  : Ok
int CFTPTransfer::Transfer()
{
	TCHAR szCause[255];

	// Clear
	m_sError = _T("");
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
	if (!OpenLocalFile()) // this function sets m_sError on failure
		return 0;

	// Should we exit the thread
    if (m_pThread && m_pThread->DoExit())
	{
		m_LocalFile.Close();
		if (m_bDownload)
			::DeleteFile(m_sLocalFile);
		return -1;
	}

	// Create the Internet session handle
	if (m_bUsePreconfig)
		m_hInternetSession = ::InternetOpen(AfxGetAppName(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    else
		m_hInternetSession = ::InternetOpen(AfxGetAppName(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (m_hInternetSession == NULL)
    {
		m_sError = ::ShowErrorMsg(::GetLastError(), FALSE);
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

	// Setup the status callback function on the Internet session handle
	INTERNET_STATUS_CALLBACK pOldCallback = ::InternetSetStatusCallback(m_hInternetSession, _OnStatusCallBack);
	if (pOldCallback == INTERNET_INVALID_STATUS_CALLBACK)
	{
		m_sError = ::ShowErrorMsg(::GetLastError(), FALSE);
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

	// Make the connection to the FTP server, use a separate
	// thread because InternetConnect can block infinitely
	CMakeConnectionThread MakeConnectionThread(this);
	MakeConnectionThread.Start();

    // Wait for the thread to return
	if (m_pThread == NULL)
	{
		DWORD Event = ::WaitForSingleObject(MakeConnectionThread.GetHandle(), FTP_CONNECTION_TIMEOUT_MS);
		switch (Event)
		{
			// Make Connection Thread Terminated
			case WAIT_OBJECT_0 :		break;

			// Timeout
			case WAIT_TIMEOUT :			// Close the wininet session handle (which will cause the worker
										// thread to return from its blocking InternetConnect call)
										::InternetCloseHandle(m_hInternetSession);

										// Wait until the worker thread exits and then set the handle to NULL
										MakeConnectionThread.WaitDone_Blocking();
										m_hInternetSession = NULL;

										// Error string
										m_sError = ML_STRING(1775, "Failed connecting to the FTP server in a timely manner\n");

										// Close
										m_LocalFile.Close();
										if (m_bDownload)
											::DeleteFile(m_sLocalFile);
										Close();

										return 0;
			
			default:					break;
		}
	}
	else
	{
		HANDLE hEventArray[2];
		hEventArray[0] = m_pThread->GetKillEvent();
		hEventArray[1] = MakeConnectionThread.GetHandle();
		DWORD Event = ::WaitForMultipleObjects(2, hEventArray, FALSE, FTP_CONNECTION_TIMEOUT_MS);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :		// Close the wininet session handle (which will cause the worker
										// thread to return from its blocking InternetConnect call)
										::InternetCloseHandle(m_hInternetSession);

										// Wait until the worker thread exits and then set the handle to NULL
										MakeConnectionThread.WaitDone_Blocking();
										m_hInternetSession = NULL;

										// Clear error string set in thread
										m_sError = _T("");

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
										// thread to return from its blocking InternetConnect call)
										::InternetCloseHandle(m_hInternetSession);
										
										// Wait until the worker thread exits and then set the handle to NULL
										MakeConnectionThread.WaitDone_Blocking();
										m_hInternetSession = NULL;

										// Error string
										m_sError = ML_STRING(1775, "Failed connecting to the FTP server in a timely manner\n");

										// Close
										m_LocalFile.Close();
										if (m_bDownload)
											::DeleteFile(m_sLocalFile);
										Close();

										return 0;
			
			default:					break;
		}
	}

    // Check to see if the connection was successful
    if (m_hFTPConnection == NULL) // connection thread sets m_sError on failure
    {
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
	if (!m_bDownload && m_bPromptOverwrite && m_dwStartPos == 0)
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
			CString sMsg;
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
	if (m_dwStartPos && !ResumeTransfer()) // this function sets m_sError on failure
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
	if (m_bDownload)
	{
		m_hFTPFile = ::FtpOpenFile(m_hFTPConnection, m_sRemoteFile, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY | 
								INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE, (DWORD) this);
	}
	else
	{ 
		m_hFTPFile = ::FtpOpenFile(m_hFTPConnection, m_sRemoteFile, GENERIC_WRITE, FTP_TRANSFER_TYPE_BINARY | 
								INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE, (DWORD) this);
	}
	if (m_hFTPFile == NULL)
	{
		// Do We Have To Create A Directory?
		if (!m_bDownload)
		{
			CString sRemoteDir = m_sRemoteFile;
			int pos;
			if ((pos = sRemoteDir.ReverseFind(_T('/'))) > -1)
			{
				sRemoteDir = sRemoteDir.Left(pos);
				CreateRemoteDir(sRemoteDir);
				m_hFTPFile = ::FtpOpenFile(m_hFTPConnection, m_sRemoteFile, GENERIC_WRITE, FTP_TRANSFER_TYPE_BINARY | 
										INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE, (DWORD) this);
			}
		}

		// Ftp Open File Error?
		if (m_hFTPFile == NULL)
		{
			m_sError = ::ShowErrorMsg(::GetLastError(), FALSE);
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
				m_sError = ::ShowErrorMsg(::GetLastError(), FALSE);
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
					m_sError.Format(ML_STRING(1776, "An exception occurred while writing to the download file:\n%s\n"), szCause);
					m_LocalFile.Close();
					::DeleteFile(m_sLocalFile);
					if (m_hInternetSession && pOldCallback)
						::InternetSetStatusCallback(m_hInternetSession, pOldCallback);
					Close();
					pEx->Delete();
					return 0;
				}

				// For bandwidth throttling
				if (m_dBandwidthLimit > 0.0) 
				{
					double t = (double)(::GetTickCount() - dwStartTicks);
					q = (double)((double)dwTotalBytesRead / t);
					if (q > m_dBandwidthLimit)	 
						::Sleep((DWORD)((((q*t)/m_dBandwidthLimit)-t)));
				}

				// Increment the total number of bytes read
				dwTotalBytesRead += dwBytesRead;

				// Statistics
				if (bGotFileSize)
					UpdatePercentage(dwTotalBytesRead, dwFileSize - m_dwStartPos);
			}
		}
		else
		{
			// Read the data from the local file
			try
			{
				dwBytesRead = m_LocalFile.Read(szReadBuf, 1024);
			}
			catch (CFileException* pEx)
			{
				pEx->GetErrorMessage(szCause, 255);
				m_sError.Format(ML_STRING(1777, "An exception occurred while reading the local file:\n%s\n"), szCause);
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
					m_sError = ::ShowErrorMsg(::GetLastError(), FALSE);
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
						UpdatePercentage(dwTotalBytesWritten, dwFileSize - m_dwStartPos);
				}

				// For bandwidth throttling
				if (m_dBandwidthLimit > 0.0) 
				{
					double t = (double)(::GetTickCount() - dwStartTicks);
					q = (double)((double)dwTotalBytesWritten / t);
					if (q > m_dBandwidthLimit)	 
						::Sleep((DWORD)((((q*t)/m_dBandwidthLimit)-t)));
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
	// Clear
	m_sError = _T("");
	Close();

	// Create the Internet session handle
	if (m_bUsePreconfig)
		m_hInternetSession = ::InternetOpen(AfxGetAppName(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    else
		m_hInternetSession = ::InternetOpen(AfxGetAppName(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (m_hInternetSession == NULL)
    {
		m_sError = ::ShowErrorMsg(::GetLastError(), FALSE);
		return FALSE;
    }

	// Setup the status callback function on the Internet session handle
	INTERNET_STATUS_CALLBACK pOldCallback = ::InternetSetStatusCallback(m_hInternetSession, _OnStatusCallBack);
	if (pOldCallback == INTERNET_INVALID_STATUS_CALLBACK)
	{
		m_sError = ::ShowErrorMsg(::GetLastError(), FALSE);
		Close();
		return FALSE;
	}

	// Make the connection to the FTP server, use a separate
	// thread because InternetConnect can block infinitely
	CMakeConnectionThread MakeConnectionThread(this);
	MakeConnectionThread.Start();

    // Wait for the thread to return
	DWORD Event = ::WaitForSingleObject(MakeConnectionThread.GetHandle(), FTP_CONNECTION_TIMEOUT_MS);
    switch (Event)
	{
		// Make Connection Thread Terminated
		case WAIT_OBJECT_0 :		break;

		// Timeout
		case WAIT_TIMEOUT :			// Close the wininet session handle (which will cause the worker
									// thread to return from its blocking InternetConnect call)
									::InternetCloseHandle(m_hInternetSession);

									// Wait until the worker thread exits and then set the handle to NULL
									MakeConnectionThread.WaitDone_Blocking();
									m_hInternetSession = NULL;

									// Error string
									m_sError = ML_STRING(1775, "Failed connecting to the FTP server in a timely manner\n");

									// Close
									Close();

									return FALSE;
		
		default:					break;
	}

    // Check to see if the connection was successful
    if (m_hFTPConnection == NULL) // connection thread sets m_sError on failure
    {
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
