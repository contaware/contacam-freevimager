#include "stdafx.h"
#include "NetCom.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

#pragma comment(lib, "ws2_32.lib")

// Defines
#define MY_IN6ADDR_ANY_INIT { 0 }
#define MY_IN6ADDR_LOOPBACK_INIT { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 }
const struct in6_addr my_in6addr_any = MY_IN6ADDR_ANY_INIT;
const struct in6_addr my_in6addr_loopback = MY_IN6ADDR_LOOPBACK_INIT;
#ifndef IPPROTO_IPV6
#define IPPROTO_IPV6		41
#endif
#ifndef IPV6_V6ONLY
#define IPV6_V6ONLY			27
#endif

///////////////////////////////////////////////////////////////////////////////
// Buffer Class
///////////////////////////////////////////////////////////////////////////////

CNetCom::CBuf::CBuf()
{
	m_Buf = NULL;
	m_BufSize = 0;
	m_MsgSize = 0;
}

CNetCom::CBuf::CBuf(unsigned int Size)
{
	m_Buf = new char[Size];
	m_BufSize = Size;
	m_MsgSize = 0;
}

CNetCom::CBuf::~CBuf()
{
	if (m_Buf)
		delete [] m_Buf;
}

CNetCom::CBuf::CBuf(const CNetCom::CBuf& b) // Copy Constructor (CBuf b1 = b2 or CBuf b1(b2))
{
	m_MsgSize = b.m_MsgSize;
	m_Buf = new char[m_BufSize = b.m_BufSize];
	ASSERT(m_MsgSize <= m_BufSize);
	if (m_Buf && b.m_Buf)
		memcpy(m_Buf, b.m_Buf, m_MsgSize);
}

CNetCom::CBuf& CNetCom::CBuf::operator=(const CNetCom::CBuf& b) // Copy Assignment (CBuf b3; b3 = b1)
{
	if (this != &b) // beware of self-assignment!
	{
		m_MsgSize = b.m_MsgSize;
		if (m_BufSize < m_MsgSize || !m_Buf)
		{
			if (m_Buf)
				delete [] m_Buf;
			m_Buf = new char[m_BufSize = b.m_BufSize];
		}
		ASSERT(m_MsgSize <= m_BufSize);
		ASSERT(m_MsgSize <= b.m_BufSize);
		if (m_Buf && b.m_Buf)
			memcpy(m_Buf, b.m_Buf, m_MsgSize);
	}
	return *this;
}


///////////////////////////////////////////////////////////////////////////////
// The Parser & Processor Base Class
///////////////////////////////////////////////////////////////////////////////

void CNetCom::CParseProcess::Process(unsigned char* pLinBuf, int nSize)
{
	pLinBuf;
	nSize;
}

void CNetCom::CParseProcess::NewData(BOOL bLastCall)
{
	// Check
	if (!m_pNetCom)
		return;
	
	int nAvailableRxBytes = m_pNetCom->GetAvailableReadBytes();
	m_nProcessOffset = 0;
	m_nProcessSize = nAvailableRxBytes;
	if (nAvailableRxBytes > 0 && Parse(m_pNetCom, bLastCall))
	{
		// Adjust Process Offset and Size
		// (just in case it has not been correctly
		// done inside the parse function)
		if (m_nProcessOffset < 0)
			m_nProcessOffset = 0;
		else if (m_nProcessOffset > nAvailableRxBytes)
			m_nProcessOffset = nAvailableRxBytes;
		if (m_nProcessSize < 0 || (m_nProcessSize > nAvailableRxBytes - m_nProcessOffset))
			m_nProcessSize = nAvailableRxBytes - m_nProcessOffset;

		// Read the data
		int nReadSize = 0;
		unsigned char* pBuf = (unsigned char*)av_malloc(m_nProcessSize + FF_INPUT_BUFFER_PADDING_SIZE);
		if (pBuf)
		{
			memset(pBuf + m_nProcessSize, 0, FF_INPUT_BUFFER_PADDING_SIZE); // zero the padding!
			if (m_nProcessOffset == 0)
			{	
				// Copy data from queue to linear buffer
				// (queue is automatically freed)
				nReadSize = m_pNetCom->Read(pBuf, m_nProcessSize);
			}
			else
			{
				// Temp linear buffer
				unsigned char* pTemp = new unsigned char[m_nProcessOffset + m_nProcessSize];
				if (pTemp)
				{
					// Copy data from queue to linear buffer
					// (queue is automatically freed)
					nReadSize = m_pNetCom->Read(pTemp, m_nProcessOffset + m_nProcessSize) - m_nProcessOffset;
					memcpy(pBuf, pTemp + m_nProcessOffset, nReadSize);
					delete [] pTemp;
				}
			}

			// Call the Process function and free the buffer
			Process(pBuf, nReadSize);
			av_free(pBuf);
		}

		// Do Parse and maybe Process more data if it's not the last call,
		// if the current call processed data and if there are bytes available
		if (!bLastCall && nReadSize > 0 && m_pNetCom->GetAvailableReadBytes() > 0)
			NewData(FALSE);
	}
}


///////////////////////////////////////////////////////////////////////////////
// Message Thread
///////////////////////////////////////////////////////////////////////////////
int CNetCom::CMsgThread::CloseSocket()
{
	m_pNetCom->m_pTxThread->Kill(NETCOM_BLOCKING_TIMEOUT);
	m_pNetCom->m_pRxThread->Kill(NETCOM_BLOCKING_TIMEOUT);
	if (m_pNetCom->m_hSocket != INVALID_SOCKET)
	{
		::closesocket(m_pNetCom->m_hSocket);
		m_pNetCom->m_hSocket = INVALID_SOCKET;
	}
	if (g_nLogLevel > 1)
		::LogLine(_T("%s MsgThread ended (ID = 0x%08X)"), m_pNetCom->GetName(), GetId());
	return 0;
}

int CNetCom::CMsgThread::Work()
{
	if (g_nLogLevel > 1)
		::LogLine(_T("%s MsgThread started (ID = 0x%08X)"), m_pNetCom->GetName(), GetId());
	BOOL bShutdownInited = FALSE;
	
	for(;;)
	{
		DWORD Event = ::WaitForMultipleObjects(3, m_pNetCom->m_hMsgEventArray, FALSE, bShutdownInited ? NETCOM_CONNECTION_SHUTDOWN_TIMEOUT : INFINITE);
		switch (Event)
		{
			// Thread Shutdown Event
			case WAIT_OBJECT_0 :
				if (g_nLogLevel > 0)
					::LogLine(_T("%s MsgThread ended killed (ID = 0x%08X)"), m_pNetCom->GetName(), GetId());
				return 0;

			// Start Connection Shutdown Event
			case WAIT_OBJECT_0 + 1 :
				::ResetEvent(m_pNetCom->m_hStartConnectionShutdownEvent);
				if (!bShutdownInited)
				{
					if (g_nLogLevel > 1)
						::LogLine(_T("%s We are starting shutdown"), m_pNetCom->GetName());
					m_pNetCom->m_pTxThread->Kill(NETCOM_BLOCKING_TIMEOUT); // Stop the Tx and Rx Threads because after shutdown with SD_BOTH,
					m_pNetCom->m_pRxThread->Kill(NETCOM_BLOCKING_TIMEOUT); // transmitting and receiving is not allowed!
					if (m_pNetCom->m_hSocket != INVALID_SOCKET)
					{
						::WSAEventSelect(m_pNetCom->m_hSocket, (WSAEVENT)m_pNetCom->m_hNetEvent, FD_CLOSE); // enable only FD_CLOSE events
						::shutdown(m_pNetCom->m_hSocket, SD_BOTH);
					}
					bShutdownInited = TRUE;
				}
				break;

			// Net Event
			case WAIT_OBJECT_0 + 2 :
				if (m_pNetCom->m_hSocket == INVALID_SOCKET)
					::WSAResetEvent(m_pNetCom->m_hNetEvent);
				else
				{
					WSANETWORKEVENTS NetworkEvents;
					memset(&NetworkEvents, 0, sizeof(WSANETWORKEVENTS));
					::WSAEnumNetworkEvents(m_pNetCom->m_hSocket, m_pNetCom->m_hNetEvent, &NetworkEvents);
					if (NetworkEvents.lNetworkEvents & FD_CONNECT)
					{
						if (NetworkEvents.iErrorCode[FD_CONNECT_BIT] == 0)
						{
							if (g_nLogLevel > 1)
								::LogLine(_T("%s Normal connection establishment to %s"), m_pNetCom->GetName(), m_pNetCom->GetPeerSockIP());
							m_pNetCom->m_pRxThread->Start();
							m_pNetCom->m_pTxThread->Start();
							if (m_pNetCom->m_hConnectEvent)
								::SetEvent(m_pNetCom->m_hConnectEvent);			// trigger Connect Event
						}
						else
						{
							if (g_nLogLevel > 0)
								::LogLine(_T("%s Failed to connect"), m_pNetCom->GetName());
							if (m_pNetCom->m_hConnectFailedEvent)
								::SetEvent(m_pNetCom->m_hConnectFailedEvent);	// trigger Connect Failed Event
							return CloseSocket();
						}
					}
					if (NetworkEvents.lNetworkEvents & FD_READ)
					{
						if (m_pNetCom->m_hReadEvent)
							::SetEvent(m_pNetCom->m_hReadEvent);				// trigger Read Event
						::SetEvent(m_pNetCom->m_hRxEvent);						// trigger the RX Thread
					}
					if (NetworkEvents.lNetworkEvents & FD_CLOSE)
					{
						if (bShutdownInited)
							return CloseSocket();
						else
						{
							m_pNetCom->m_pTxThread->Kill(NETCOM_BLOCKING_TIMEOUT); // stop the Tx Thread because after shutdown with SD_SEND transmitting is not allowed
							if (m_pNetCom->m_hSocket != INVALID_SOCKET)
							{
								::WSAEventSelect(m_pNetCom->m_hSocket, (WSAEVENT)m_pNetCom->m_hNetEvent, FD_READ); // enable only FD_READ events
								::shutdown(m_pNetCom->m_hSocket, SD_SEND);
							}
							bShutdownInited = TRUE; // next step is entering the case WAIT_TIMEOUT, this is to let finish reading remaining data
						}
					}
				}
				break;

			// Timeout
			case WAIT_TIMEOUT :
				if (bShutdownInited)
					return CloseSocket();
				break;
		}
	}
	ASSERT(FALSE); // should never end up here...
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Rx Thread
///////////////////////////////////////////////////////////////////////////////
int CNetCom::CRxThread::Work() 
{
	if (g_nLogLevel > 1)
		::LogLine(_T("%s RxThread started (ID = 0x%08X)"), m_pNetCom->GetName(), GetId());
	DWORD Flags;
	DWORD NumberOfBytesReceived = 0;
	BOOL  bResult;

	for(;;)
	{
		DWORD Event = ::WaitForMultipleObjects(3, m_pNetCom->m_hRxEventArray, FALSE, INFINITE);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :
				if (m_pCurrentBuf)
				{
					delete m_pCurrentBuf;
					m_pCurrentBuf = NULL;
				}
				if (m_pNetCom->m_pParseProcess)
				{
					// Call the Parser last time
					m_pNetCom->m_pParseProcess->NewData(TRUE);
				}
				if (g_nLogLevel > 1)
					::LogLine(_T("%s RxThread ended (ID = 0x%08X)"), m_pNetCom->GetName(), GetId());
				return 0;
		
			// Overlapped Event
			case WAIT_OBJECT_0 + 1 :
				bResult = ::WSAGetOverlappedResult(m_pNetCom->m_hSocket, &m_pNetCom->m_ovRx, &NumberOfBytesReceived, TRUE, &Flags);
				::WSAResetEvent(m_pNetCom->m_ovRx.hEvent);
				if (!bResult)
				{
					if (m_pCurrentBuf)
					{
						delete m_pCurrentBuf;
						m_pCurrentBuf = NULL;
					}
				}

				// Update the message size
				// Note: even if WSARecv received all the bytes and returned no error,
				// the Overlapped event is also triggered! 
				if (m_pCurrentBuf && (m_pCurrentBuf->GetMsgSize() == 0))
				{
					if (NumberOfBytesReceived == 0)
					{
						delete m_pCurrentBuf;
						m_pCurrentBuf = NULL;
					}
					else
						m_pCurrentBuf->SetMsgSize(NumberOfBytesReceived);
				}
				break;

			// Read Event
			case WAIT_OBJECT_0 + 2 :
				::ResetEvent(m_pNetCom->m_hRxEvent);
				if ((m_pNetCom->m_hSocket != INVALID_SOCKET) && (m_pCurrentBuf == NULL))
					Read();
				break;

			default :
				break;
		}

		// Store and parse & process received data
		if (m_pCurrentBuf && (m_pCurrentBuf->GetMsgSize() > 0))
		{
			// Add to Fifo
			::EnterCriticalSection(&m_pNetCom->m_csRxFifoSync);
			m_pNetCom->m_RxFifo.AddTail(m_pCurrentBuf);
			::LeaveCriticalSection(&m_pNetCom->m_csRxFifoSync);

			// Set to NULL!
			m_pCurrentBuf = NULL;

			// Call the Parser
			if (m_pNetCom->m_pParseProcess)
				m_pNetCom->m_pParseProcess->NewData(FALSE);
		}
	}
	ASSERT(FALSE); // should never end up here...
	return 0;
}

__forceinline void CNetCom::CRxThread::Read()
{
	DWORD Flags = 0;
	WSABUF WSABuffer;
	m_pCurrentBuf = new CBuf(NETCOM_MAX_RX_BUFFER_SIZE);
	WSABuffer.len = m_pCurrentBuf->GetBufSize();
	WSABuffer.buf = m_pCurrentBuf->GetBuf();
	DWORD NumberOfBytesReceived = 0;
	int nRecvRes = ::WSARecv(m_pNetCom->m_hSocket,
							&WSABuffer,
							1,
							&NumberOfBytesReceived,
							&Flags,
							&m_pNetCom->m_ovRx,
							NULL);
	if (nRecvRes == SOCKET_ERROR)
	{
		int res = ::WSAGetLastError();
		if (res != WSAEWOULDBLOCK && res != WSA_IO_PENDING)
		{
			delete m_pCurrentBuf;
			m_pCurrentBuf = NULL;
		}	
	}
	// An overlapped event is also triggered if no error has been returned
	// -> the message size is set by the overlapped handler
}


///////////////////////////////////////////////////////////////////////////////
// Tx Thread
///////////////////////////////////////////////////////////////////////////////
int CNetCom::CTxThread::Work() 
{
	if (g_nLogLevel > 1)
		::LogLine(_T("%s TxThread started (ID = 0x%08X)"), m_pNetCom->GetName(), GetId());
	DWORD Flags;
	BOOL  bResult;
	DWORD NumberOfBytesSent = 0;

	for (;;)
	{
		DWORD Event = ::WaitForMultipleObjects(3, m_pNetCom->m_hTxEventArray, FALSE, INFINITE);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :
				if (m_pCurrentBuf)
				{
					delete m_pCurrentBuf;
					m_pCurrentBuf = NULL;
				}
				if (g_nLogLevel > 1)
					::LogLine(_T("%s TxThread ended (ID = 0x%08X)"), m_pNetCom->GetName(), GetId());
				return 0;

			// Overlapped Event
			case WAIT_OBJECT_0 + 1 :
				bResult = ::WSAGetOverlappedResult(m_pNetCom->m_hSocket, &m_pNetCom->m_ovTx, &NumberOfBytesSent, TRUE, &Flags);
				::WSAResetEvent(m_pNetCom->m_ovTx.hEvent);
				if (!bResult)
				{
					if (m_pCurrentBuf)
					{
						delete m_pCurrentBuf;
						m_pCurrentBuf = NULL;
					}
				}

				// Free the message buffer and trigger another tx event (if data available)
				// Note: even if WSASend sent all the bytes and returned no error,
				// the Overlapped event is triggered! 
				if (m_pCurrentBuf && (m_pCurrentBuf->GetMsgSize() > 0))
				{
					// Free
					delete m_pCurrentBuf;
					m_pCurrentBuf = NULL;
					
					// Trigger another tx event?
					if (m_nCurrentTxFifoSize > 1) // this is the fifo size before the current Write()
						::SetEvent(m_pNetCom->m_hTxEvent);
				}
				break;
		
			// Tx Event
			case WAIT_OBJECT_0 + 2 :
				::ResetEvent(m_pNetCom->m_hTxEvent);
				if (m_pCurrentBuf == NULL) // If not sending
					Write();
				break;

			default :
				break;
		}
	}
	ASSERT(FALSE); // should never end up here...
	return 0;
}

__forceinline void CNetCom::CTxThread::Write()
{
	WSABUF WSABuffer;
	DWORD NumberOfBytesSent = 0;
	if (m_pNetCom->m_hSocket != INVALID_SOCKET)
	{
		::EnterCriticalSection(&m_pNetCom->m_csTxFifoSync);
		m_nCurrentTxFifoSize = m_pNetCom->m_TxFifo.GetCount(); 
		if (m_nCurrentTxFifoSize == 0)
		{
			::LeaveCriticalSection(&m_pNetCom->m_csTxFifoSync);
			return;
		}
		m_pCurrentBuf = m_pNetCom->m_TxFifo.RemoveHead();
		::LeaveCriticalSection(&m_pNetCom->m_csTxFifoSync);

		// Init the WSABuffer
		WSABuffer.buf = m_pCurrentBuf->GetBuf();
		WSABuffer.len = m_pCurrentBuf->GetMsgSize();

		// Send out
		if ((WSABuffer.len > 0) && (WSABuffer.buf != NULL))
		{
			int nSendRes = ::WSASend(m_pNetCom->m_hSocket,
									&WSABuffer,
									1,
									&NumberOfBytesSent,
									0,
									&m_pNetCom->m_ovTx,
									NULL);
			if (nSendRes == SOCKET_ERROR)
			{
				int res = ::WSAGetLastError();
				if (res != WSAEWOULDBLOCK && res != WSA_IO_PENDING)
				{
					delete m_pCurrentBuf;
					m_pCurrentBuf = NULL;
				}
			}
			// An overlapped event is also triggered if no error has been returned
			// -> the message buffer is freed and another tx event is trigger
			//    (if data available) by the overlapped handler
		}
		else
		{
			if (m_pCurrentBuf)
			{
				delete m_pCurrentBuf;
				m_pCurrentBuf = NULL;
			}
		}
	}
	else
	{
		if (m_pCurrentBuf)
		{
			delete m_pCurrentBuf;
			m_pCurrentBuf = NULL;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// CNetCom
///////////////////////////////////////////////////////////////////////////////
CNetCom::CNetCom()
{
	// Init WinSock 2.2
	WSADATA wsadata;
	if (::WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
		::LogLine(_T("WSAStartup() failed"));

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2)
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		::WSACleanup();
		::LogLine(_T("No usable WinSock DLL found")); 
	}

	// Handle
	m_hSocket = INVALID_SOCKET;

	// Pointers
	m_pParseProcess	= NULL;
	m_pMsgThread = new CMsgThread;
	m_pRxThread = new CRxThread;
	m_pTxThread = new CTxThread;
	if (m_pMsgThread)
		m_pMsgThread->SetNetComPointer(this);
	if (m_pRxThread)
		m_pRxThread->SetNetComPointer(this);
	if (m_pTxThread)
		m_pTxThread->SetNetComPointer(this);

	// Initialize overlapped structure members to zero
	m_ovRx.Offset		= 0;
	m_ovRx.OffsetHigh	= 0;
	m_ovTx.Offset		= 0;
	m_ovTx.OffsetHigh	= 0;

	// Create Events
	m_ovRx.hEvent					= ::WSACreateEvent();
	m_ovTx.hEvent					= ::WSACreateEvent();
	m_hNetEvent						= ::WSACreateEvent();
	m_hTxEvent						= ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hRxEvent						= ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hStartConnectionShutdownEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hConnectEvent					= NULL;
	m_hConnectFailedEvent			= NULL;
	m_hReadEvent					= NULL;

	// Initialize the Event Object Arrays
	m_hMsgEventArray[0]	= m_pMsgThread ? m_pMsgThread->GetKillEvent() : NULL;
	m_hMsgEventArray[1]	= m_hStartConnectionShutdownEvent;
	m_hMsgEventArray[2]	= m_hNetEvent;
	m_hRxEventArray[0]	= m_pRxThread ? m_pRxThread->GetKillEvent() : NULL;
	m_hRxEventArray[1]	= m_ovRx.hEvent;
	m_hRxEventArray[2]	= m_hRxEvent;
	m_hTxEventArray[0]	= m_pTxThread ? m_pTxThread->GetKillEvent() : NULL;
	m_hTxEventArray[1]	= m_ovTx.hEvent;
	m_hTxEventArray[2]	= m_hTxEvent;

	// Initialize critical sections
	::InitializeCriticalSection(&m_csRxFifoSync);
	::InitializeCriticalSection(&m_csTxFifoSync);

	// Address
	m_sPeerAddress = _T("");
	
	// Port
	m_uiPeerPort = 0;

	// Socket Family
	m_nSocketFamily = AF_UNSPEC;

	// Reset init time
	m_InitTime = CTime(0);
}

CNetCom::~CNetCom()
{
	// Close connection
	Close();

	// Free the threads (attention both base class and derived class
	// destructors call Kill() which locks indefinitely)
	delete m_pMsgThread;
	delete m_pRxThread;
	delete m_pTxThread;

	// Close the events
	if (m_ovRx.hEvent != NULL)
	{
		::WSAResetEvent(m_ovRx.hEvent);
		::WSACloseEvent(m_ovRx.hEvent);
		m_ovRx.hEvent = NULL;
	}
	if (m_ovTx.hEvent != NULL)
	{
		::WSAResetEvent(m_ovTx.hEvent);
		::WSACloseEvent(m_ovTx.hEvent);
		m_ovTx.hEvent = NULL;
	}
	if (m_hNetEvent != NULL)
	{
		::WSAResetEvent(m_hNetEvent);
		::WSACloseEvent(m_hNetEvent);
		m_hNetEvent = NULL;
	}
	if (m_hTxEvent != NULL)
	{
		::ResetEvent(m_hTxEvent);
		::CloseHandle(m_hTxEvent);
		m_hTxEvent = NULL;
	}
	if (m_hRxEvent != NULL)
	{
		::ResetEvent(m_hRxEvent);
		::CloseHandle(m_hRxEvent);
		m_hRxEvent = NULL;
	}
	if (m_hStartConnectionShutdownEvent != NULL)
	{
		::ResetEvent(m_hStartConnectionShutdownEvent);
		::CloseHandle(m_hStartConnectionShutdownEvent);
		m_hStartConnectionShutdownEvent = NULL;
	}

	// Delete critical sections
	::DeleteCriticalSection(&m_csRxFifoSync);
	::DeleteCriticalSection(&m_csTxFifoSync);

	// WinSock cleanup
	::WSACleanup();
}

BOOL CNetCom::InitAddr(volatile int& nSocketFamily, const CString& sAddress, UINT uiPort, sockaddr* paddr)
{
	if (!paddr)
		return FALSE;
	((sockaddr_in*)paddr)->sin_port = htons((unsigned short)uiPort);
	if (sAddress != _T(""))
	{
		// Port string
		CString sPort;
		sPort.Format(_T("%u"), uiPort);

		// Priority to IPv6
		if (nSocketFamily == AF_INET6)
		{
			if (!StringToAddress(sAddress, sPort, paddr, AF_INET6))
			{
				if (!StringToAddress(sAddress, sPort, paddr, AF_INET))
					return FALSE;
				nSocketFamily = AF_INET;
			}
			else
				nSocketFamily = AF_INET6;				
		}
		// Priority to IPv4
		else
		{
			if (!StringToAddress(sAddress, sPort, paddr, AF_INET))
			{
				if (!StringToAddress(sAddress, sPort, paddr, AF_INET6))
					return FALSE;
				nSocketFamily = AF_INET6;
			}
			else
				nSocketFamily = AF_INET;
		}
	}
	paddr->sa_family = (unsigned short)nSocketFamily;
	return TRUE;
}

BOOL CNetCom::Init(	CParseProcess* pParseProcess,		// Parser & Processor
					CString sPeerAddress,				// Peer Address (IP or Host Name)
					UINT uiPeerPort,					// Peer Port
					HANDLE hConnectEvent,				// Handle to an Event Object that will get Connect Events
					HANDLE hConnectFailedEvent,			// Handle to an Event Object that will get Connect Failed Events
					HANDLE hReadEvent,					// Handle to an Event Object that will get Read Events
					int nSocketFamily)					// Socket family priority: AF_INET for IPv4, AF_INET6 for IPv6
{
	// First close
	Close();

	// Set init time
	m_InitTime = CTime::GetCurrentTime();

	// Init Address
	sockaddr_in6 peer_addr6;
	sockaddr* ppeer_addr = (sockaddr*)&peer_addr6;
	memset(&peer_addr6, 0, sizeof(peer_addr6)); // Init to any address
	if (!InitAddr(nSocketFamily, sPeerAddress, uiPeerPort, ppeer_addr)) // This can change nSocketFamily
		return FALSE;
	if (nSocketFamily == AF_UNSPEC)
		nSocketFamily = AF_INET; // Default to IP4
	ppeer_addr->sa_family = (unsigned short)nSocketFamily;
	m_nSocketFamily = nSocketFamily;
	m_sPeerAddress = sPeerAddress;
	m_uiPeerPort = uiPeerPort;

	// Init the Events
	m_hConnectEvent = hConnectEvent;
	m_hConnectFailedEvent = hConnectFailedEvent;
	m_hReadEvent = hReadEvent;

	// Init the Parser
	if (pParseProcess)
		pParseProcess->Init(this);
	m_pParseProcess = pParseProcess;

	// Create an Overlapped (=Asynchronous) Socket
	if ((m_hSocket = ::WSASocket(m_nSocketFamily, SOCK_STREAM,
								IPPROTO_TCP,
								NULL, 0, WSA_FLAG_OVERLAPPED)) != INVALID_SOCKET)
	{
		// Turn on the used network events
		::WSAEventSelect(m_hSocket, (WSAEVENT)m_hNetEvent, FD_READ | FD_CONNECT | FD_CLOSE);

		// Start Message Thread now so that FD_CONNECT can be handled
		// Note: the Reset event has to be here and not at the beginning of
		// the message thread because if calling two consecutive
		// Init() the first message thread start could reset the
		// event after the second ShutdownConnection_NoBlocking() call
		::ResetEvent(m_hStartConnectionShutdownEvent);
		m_pMsgThread->Start();
				
		// Connect
		if (!SOCKADDRANY(ppeer_addr))
		{
			if (::WSAConnect(m_hSocket, ppeer_addr, SOCKADDRSIZE(ppeer_addr), NULL, NULL, NULL, NULL) != SOCKET_ERROR)
				return TRUE;
			else
			{
				int nErrorCode = ::WSAGetLastError();
				if (nErrorCode == WSAEWOULDBLOCK)
					return TRUE;
				else
					return FALSE;
			}
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}

void CNetCom::Close()
{
	// Shutdown connection
	ShutdownConnection_NoBlocking();
	int nWaitMS = NETCOM_BLOCKING_TIMEOUT;
	while (nWaitMS > 0)
	{
		if (IsShutdown())
			break;
		Sleep(100);
		nWaitMS -= 100;
	}

	// Is connection really closed?
	// Note: if the above IsShutdown() returned TRUE  
	// it doesn't harm to go through the following code
	if (m_hSocket != INVALID_SOCKET)
		::closesocket(m_hSocket);
	m_pTxThread->Kill_NoBlocking();
	m_pRxThread->Kill_NoBlocking();
	m_pMsgThread->Kill_NoBlocking();
	nWaitMS = NETCOM_BLOCKING_TIMEOUT;
	while (nWaitMS > 0)
	{
		if (IsShutdown())
			break;	
		Sleep(100);
		nWaitMS -= 100;
	}
	m_hSocket = INVALID_SOCKET;

	// Empty fifos
	::EnterCriticalSection(&m_csRxFifoSync);
	while (!m_RxFifo.IsEmpty())
	{
		CBuf* pBuf = m_RxFifo.RemoveHead();
		if (pBuf)
			delete pBuf;
	}
	::LeaveCriticalSection(&m_csRxFifoSync);
	::EnterCriticalSection(&m_csTxFifoSync);
	while (!m_TxFifo.IsEmpty())
	{
		CBuf* pBuf = m_TxFifo.RemoveHead();
		if (pBuf)
			delete pBuf;
	}
	::LeaveCriticalSection(&m_csTxFifoSync);

	// Reset init time
	m_InitTime = CTime(0);
}

CString CNetCom::GetPeerSockIP()
{
	if (m_hSocket != INVALID_SOCKET)
	{
		if (m_nSocketFamily == AF_INET6)
		{
			sockaddr_in6 addr6;
			memset(&addr6, 0, sizeof(addr6));
			int addr6len = sizeof(addr6);
			if (::getpeername(m_hSocket, (sockaddr*)&addr6, &addr6len) == SOCKET_ERROR)
				return _T("");
			else
			{
				CString sPeerAddress; 
				sPeerAddress.Format(_T("%x:%x:%x:%x:%x:%x:%x:%x"),
						ntohs(addr6.sin6_addr.u.Word[0]),
						ntohs(addr6.sin6_addr.u.Word[1]),
						ntohs(addr6.sin6_addr.u.Word[2]),
						ntohs(addr6.sin6_addr.u.Word[3]),
						ntohs(addr6.sin6_addr.u.Word[4]),
						ntohs(addr6.sin6_addr.u.Word[5]),
						ntohs(addr6.sin6_addr.u.Word[6]),
						ntohs(addr6.sin6_addr.u.Word[7]));
				return sPeerAddress;
			}
		}
		else
		{
			sockaddr_in addr;
			memset(&addr, 0, sizeof(addr));
			int addrlen = sizeof(addr);
			if (::getpeername(m_hSocket, (sockaddr*)&addr, &addrlen) == SOCKET_ERROR)
				return _T("");
			else
			{
				CString sPeerAddress; 
				sPeerAddress.Format(_T("%d.%d.%d.%d"),
						addr.sin_addr.S_un.S_un_b.s_b1,
						addr.sin_addr.S_un.S_un_b.s_b2,
						addr.sin_addr.S_un.S_un_b.s_b3,
						addr.sin_addr.S_un.S_un_b.s_b4);
				return sPeerAddress;
			}
		}
	}
	else
		return _T("");
}

int CNetCom::GetAvailableReadBytes()
{
	int nSize = 0;
	::EnterCriticalSection(&m_csRxFifoSync);
	POSITION pos = m_RxFifo.GetHeadPosition();
	while (pos)
	{
		CBuf* pBuf = m_RxFifo.GetNext(pos);
		if (pBuf)
			nSize += pBuf->GetMsgSize();
	}
	::LeaveCriticalSection(&m_csRxFifoSync);
	return nSize;
}

int CNetCom::Read(BYTE* Data/*=NULL*/, int BufSize/*=0*/)
{
	unsigned int i = 0;
	int pos = 0;

	// Empty Read
	if (Data == NULL && BufSize <= 0)
	{
		::EnterCriticalSection(&m_csRxFifoSync);
		while (!m_RxFifo.IsEmpty())
		{
			CBuf* pBuf = m_RxFifo.RemoveHead();
			if (pBuf)
			{
				i += pBuf->GetMsgSize();
				delete pBuf;
			}
		}
		::LeaveCriticalSection(&m_csRxFifoSync);
		return i;
	}

	// Zero or wrong size
	if (BufSize <= 0)
		return 0;

	::EnterCriticalSection(&m_csRxFifoSync);
	while (!m_RxFifo.IsEmpty())
	{
		CBuf* pBuf = m_RxFifo.GetHead();
		::LeaveCriticalSection(&m_csRxFifoSync);
		i = 0;
		while ((pos < BufSize) && (i < pBuf->GetMsgSize()))
		{
			if (Data)
				Data[pos] = (pBuf->GetBuf())[i];
			pos++;
			i++;
		}
	
		if (pos == BufSize)
		{
			if (i < pBuf->GetMsgSize()) // -> the remaining part of the buffer has to be moved!
			{
				pBuf->SetMsgSize(pBuf->GetMsgSize() - i);
				memmove((void*)(pBuf->GetBuf()), (void*)(pBuf->GetBuf() + i), pBuf->GetMsgSize());
			}
			else // if i == pBuf->GetMsgSize()
			{
				::EnterCriticalSection(&m_csRxFifoSync);
				m_RxFifo.RemoveHead();
				delete pBuf;
				::LeaveCriticalSection(&m_csRxFifoSync);
			}
			return pos;
		}

		::EnterCriticalSection(&m_csRxFifoSync);
		m_RxFifo.RemoveHead();
		delete pBuf;
	}
	::LeaveCriticalSection(&m_csRxFifoSync);
	return pos;
}

int CNetCom::Write(const BYTE* Data, int Size)
{
	if ((Size == 0) || (Data == NULL))
		return 0;

	if (m_hSocket != INVALID_SOCKET)
	{
		int SentSize = 0;
		while (SentSize < Size)
		{
			CBuf* pBuf;
			if ((Size - SentSize) > (int)NETCOM_MAX_TX_BUFFER_SIZE)
			{
				pBuf = new CBuf(NETCOM_MAX_TX_BUFFER_SIZE);
				memcpy((void*)(pBuf->GetBuf()), (void*)(Data+SentSize), NETCOM_MAX_TX_BUFFER_SIZE);
				pBuf->SetMsgSize(NETCOM_MAX_TX_BUFFER_SIZE);
				SentSize += NETCOM_MAX_TX_BUFFER_SIZE;
			}
			else
			{
				pBuf = new CBuf(Size - SentSize);
				memcpy((void*)(pBuf->GetBuf()), (void*)(Data+SentSize), Size - SentSize);
				pBuf->SetMsgSize(Size - SentSize);
				SentSize = Size;
			}
			::EnterCriticalSection(&m_csTxFifoSync);
			m_TxFifo.AddTail(pBuf);
			::LeaveCriticalSection(&m_csTxFifoSync);
			::SetEvent(m_hTxEvent);
		}
		return SentSize;
	}
	else
		return 0;
}

int CNetCom::WriteStr(LPCTSTR str)
{
	CStringA sAnsiText(str);
	int res = Write((const BYTE*)sAnsiText.GetBuffer(), sAnsiText.GetLength());
    sAnsiText.ReleaseBuffer();
	return res;
}

BOOL CNetCom::StringToAddress(const TCHAR* sHost, const TCHAR* sPort, sockaddr* psockaddr, int nSocketFamily/*=AF_UNSPEC*/)
{
	// Check addr
	if (!psockaddr)
		return FALSE;

	// Init
	int nPort = 0;
	if (sPort)
	{
		nPort = _tcstol(sPort, NULL, 10);
		if (nPort < 0 || nPort > 65535)
			nPort = 0;
	}
	memset(psockaddr, 0, nSocketFamily == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6));
	psockaddr->sa_family = (unsigned short)nSocketFamily;
    ((sockaddr_in*)psockaddr)->sin_port = htons((unsigned short)nPort);

	// Check host
	if ((sHost == NULL) || (*sHost == _T('\0')))
		return FALSE;

	BOOL res = FALSE;
	HINSTANCE hInstLib = ::LoadLibraryFromSystem32(_T("Ws2_32.dll"));
	if (hInstLib)
	{
		INT (WSAAPI *lpfGetAddrInfo)(PCWSTR pNodeName, PCWSTR pServiceName, const ADDRINFOW* pHints, PADDRINFOW* ppResult);
		VOID (WSAAPI *lpfFreeAddrInfo)(PADDRINFOW pAddrInfo);
		lpfGetAddrInfo = (INT(WSAAPI*)(PCWSTR, PCWSTR, const ADDRINFOW*, PADDRINFOW*))::GetProcAddress(hInstLib, "GetAddrInfoW");
		lpfFreeAddrInfo = (VOID(WSAAPI*)(PADDRINFOW))::GetProcAddress(hInstLib, "FreeAddrInfoW");
		if (lpfGetAddrInfo && lpfFreeAddrInfo)
		{
			ADDRINFOW aiHints;
			ADDRINFOW* aiList = NULL;
			memset(&aiHints, 0, sizeof(aiHints));
			aiHints.ai_family = nSocketFamily;
			aiHints.ai_flags = AI_PASSIVE;
			if (lpfGetAddrInfo(sHost, sPort, &aiHints, &aiList) == 0)
			{
				ADDRINFOW* walk;
				for (walk = aiList ; walk != NULL ; walk = walk->ai_next)
				{
					if ((walk->ai_family == nSocketFamily || nSocketFamily == AF_UNSPEC) &&
						(walk->ai_family == AF_INET || walk->ai_family == AF_INET6))
					{
						memcpy(psockaddr, walk->ai_addr, walk->ai_addrlen);	
						res = TRUE;
						break;
					}
				}

				// Free
				lpfFreeAddrInfo(aiList);
			}
		}
		else if (nSocketFamily != AF_INET6)
		{
			psockaddr->sa_family = AF_INET;
			size_t host_size = _tcslen(sHost);
			char* pHost = (char*)new char[host_size+1];
			wcstombs(pHost, sHost, host_size+1);
			// Check for Dotted IP Address String
			((sockaddr_in*)psockaddr)->sin_addr.S_un.S_addr = inet_addr(pHost);

			// If not a Dotted IP Address String try to resolve it as host name
			if ((((sockaddr_in*)psockaddr)->sin_addr.S_un.S_addr == INADDR_NONE) &&
				(_tcscmp(sHost, _T("255.255.255.255"))))
			{
				LPHOSTENT pHostent = ::gethostbyname(pHost);

				// Check if successfull
				if (pHostent)
				{
					((sockaddr_in*)psockaddr)->sin_addr.S_un.S_addr = *((unsigned long*)pHostent->h_addr);
					res = TRUE;
				}
				else
					((sockaddr_in*)psockaddr)->sin_addr.S_un.S_addr = INADDR_ANY;
			}
			else
				res = TRUE;

			// Clean-Up
			delete [] pHost;
		}

		// Free lib
		::FreeLibrary(hInstLib);
	}

	return res;
}

#endif