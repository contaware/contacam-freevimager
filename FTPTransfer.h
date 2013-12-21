#ifndef __FTPTRANSFER_H__
#define __FTPTRANSFER_H__

#pragma once

#ifndef _WININET_
#include <afxinet.h>    // MFC Internet support
#pragma message("To avoid this message, please put afxinet.h in your pre compiled header (normally stdafx.h)")
#endif

#include "WorkerThread.h"

#ifdef VIDEODEVICEDOC

#define FTP_CONNECTION_TIMEOUT_MS		15000U

// RFC 959
class CFTPTransfer
{
public:
	// The Make Connection Thread Class
	class CMakeConnectionThread : public CWorkerThread
	{
		public:
			CMakeConnectionThread(CFTPTransfer* pFTPTransfer){m_pFTPTransfer = pFTPTransfer;};
			virtual ~CMakeConnectionThread(){Kill();};

		protected:
			int Work();
			CFTPTransfer* m_pFTPTransfer;
	};
	friend CMakeConnectionThread;

	// Public Member Functions
	CFTPTransfer(CWorkerThread* pThread);
	virtual ~CFTPTransfer();
	virtual void OnTransferProgress(DWORD dwPercentage) {;};
	__forceinline DWORD GetPercentage() const {return m_dwPercentage;};
	int Transfer();
	void Close();
	BOOL Test();

	// Public Member Variables
	CString			m_sError;					// Returns error message if Transfer() or Test() failed
	BOOL			m_bDownload;				// TRUE if it's a download, FALSE if an upload
	CString			m_sServer;					// e.g. "ftp.microsoft.com"
	CString			m_sRemoteFile;				// e.g. "/pub/somefile.ext"
	CString			m_sLocalFile;				// e.g. "c:\\temp\\somefile.txt"
	CString			m_sUserName;				// Username to login to the server
	CString			m_sPassword;				// Password to login to the server
	INTERNET_PORT	m_nPort;					// If you want to change the port to make access on, by default it will be 21
	BOOL			m_bBinary;					// TRUE if binary transfer, FALSE for ascii transfer
	BOOL			m_bPromptOverwrite;			// Should the user be prompted to overwrite files
	BOOL			m_bPasv;					// Should a PASV connection be used
	BOOL			m_bUsePreconfig;			// Should preconfigured settings be used i.e. take proxy settings etc from the control panel
	BOOL			m_bUseProxy;				// Should a proxy be used
	CString			m_sProxy;					// The proxy connect string to use if "m_bUseProxy" is TRUE
	double			m_dBandwidthLimit;			// For BANDWIDTH throttling, the value in KBytes / Second to limit the connection to (disable that with a value of 0.0)
	DWORD			m_dwStartPos;				// Offset to resume the transfer at   
	DWORD			m_dwConnectionTimeout;		// The connection timeout to use (in milliseconds)

protected:
	void UpdatePercentage(DWORD dwTotalBytesDone, DWORD dwFileSize);
	static void CALLBACK _OnStatusCallBack(HINTERNET hInternet, DWORD dwContext, DWORD dwInternetStatus, 
										LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);
	void OnStatusCallBack(HINTERNET hInternet, DWORD dwInternetStatus, 
						LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);
	BOOL ResumeTransfer();
	BOOL OpenLocalFile();
	BOOL CreateRemoteDir(CString sDirName);

	DWORD			m_dwPercentage;
	DWORD			m_dwLastPercentage;
	HINTERNET		m_hInternetSession;
	HINTERNET		m_hFTPConnection;
	HINTERNET		m_hFTPFile;
	CFile			m_LocalFile;
	CWorkerThread*	m_pThread;
};

#endif

#endif //__FTPTRANSFERDLG_H__
