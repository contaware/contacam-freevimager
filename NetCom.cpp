#include "stdafx.h"
#include "NetCom.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib, "mpr.lib")
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

BOOL CNetCom::CParseProcess::Process(unsigned char* pLinBuf, int nSize)
{
	pLinBuf;
	nSize;
	return TRUE; // Auto-delete pLinBuf
}

void CNetCom::CParseProcess::NewData(BOOL bLastCall)
{
	if (!m_pNetCom) return;
	
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
		unsigned char* pBuf = new unsigned char[m_nProcessSize + PROCESSOR_BUFFER_PADDING_SIZE];
		if (pBuf)
		{
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

			// Call the Process function
			if (Process(pBuf, nReadSize))
				delete [] pBuf; // Auto-delete if return value is TRUE
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Message Thread
///////////////////////////////////////////////////////////////////////////////
void CNetCom::CMsgThread::SignalClosing()
{
	// Close Event
	if (m_pNetCom->m_hCloseEvent)
		::SetEvent(m_pNetCom->m_hCloseEvent);
}

int CNetCom::CMsgThread::Work()
{
	DWORD Event;
	if (m_pNetCom->m_pMsgOut)
		m_pNetCom->Notice(m_pNetCom->GetName() + _T(" MsgThread started (ID = 0x%08X)"), GetId());
	BOOL bInitConnectionShutdown = FALSE; // Did this Socket Initiate the Connection Shutdown

	for(;;)
	{
		// Main wait function
		if (m_bClosing && !bInitConnectionShutdown)
			Event = ::WaitForMultipleObjects(3, m_pNetCom->m_hMsgEventArray, FALSE, NETCOM_PEER_CONNECTION_CLOSE_TIMEOUT);
		else if (bInitConnectionShutdown)
			Event = ::WaitForMultipleObjects(3, m_pNetCom->m_hMsgEventArray, FALSE, NETCOM_CONNECTION_SHUTDOWN_TIMEOUT);
		else
			Event = ::WaitForMultipleObjects(3, m_pNetCom->m_hMsgEventArray, FALSE, INFINITE);

		switch (Event)
		{
			// Thread Shutdown Event
			case WAIT_OBJECT_0 :
				m_pNetCom->m_bClientConnected = FALSE;
				if (m_pNetCom->m_pMsgOut)
					m_pNetCom->Warning(m_pNetCom->GetName() + _T(" MsgThread ended killed (ID = 0x%08X)"), GetId());
				return 0;

			// Start Connection Shutdown Event
			case WAIT_OBJECT_0 + 1 :
				::ResetEvent(m_pNetCom->m_hStartConnectionShutdownEvent);
				if (m_pNetCom->m_pMsgOut)
					m_pNetCom->Notice(m_pNetCom->GetName() + _T(" We are starting shutdown"));
				if (m_pNetCom->m_hSocket != INVALID_SOCKET)
				{
					// Stop the Tx and Rx Threads because after shutdown with SD_BOTH,
					// transmitting and receiving is not allowed!
					m_pNetCom->ShutdownTxThread();
					m_pNetCom->ShutdownRxThread();

					// Set flag
					bInitConnectionShutdown = TRUE;

					// Enable only FD_CLOSE events at this point
					if (::WSAEventSelect(m_pNetCom->m_hSocket, (WSAEVENT)m_pNetCom->m_hNetEvent, FD_CLOSE) == SOCKET_ERROR)
						m_pNetCom->ProcessWSAError(m_pNetCom->GetName() + _T(" WSAEventSelect()"));

					// Shutdown SD_BOTH
					if (::shutdown(m_pNetCom->m_hSocket, SD_BOTH) == SOCKET_ERROR)
						m_pNetCom->ProcessWSAError(m_pNetCom->GetName() + _T(" shutdown()"));
				}
				break;

			// Net Event
			case WAIT_OBJECT_0 + 2 :
				WSANETWORKEVENTS NetworkEvents;
				if (m_pNetCom->m_hSocket != INVALID_SOCKET)
				{
					if (::WSAEnumNetworkEvents(	m_pNetCom->m_hSocket, 
												m_pNetCom->m_hNetEvent,
												&NetworkEvents) == SOCKET_ERROR)
					{
						m_pNetCom->ProcessWSAError(m_pNetCom->GetName() + _T(" WSAEnumNetworkEvents()"));
						m_pNetCom->m_bClientConnected = FALSE;
						m_pNetCom->ShutdownTxThread();
						m_pNetCom->ShutdownRxThread();
						if (::closesocket(m_pNetCom->m_hSocket) == SOCKET_ERROR)
							m_pNetCom->ProcessWSAError(m_pNetCom->GetName() + _T(" closesocket()"));
						else
							m_pNetCom->m_hSocket = INVALID_SOCKET;
						if (m_pNetCom->m_pMsgOut)
							m_pNetCom->Notice(m_pNetCom->GetName() + _T(" MsgThread ended (ID = 0x%08X)"), GetId());
						return 0;
					}
					else
					{
						if (NetworkEvents.lNetworkEvents & FD_CONNECT)
						{
							if (m_pNetCom->m_pMsgOut)
								m_pNetCom->Notice(m_pNetCom->GetName() + _T(" Net Event FD_CONNECT"));

							switch (NetworkEvents.iErrorCode[FD_CONNECT_BIT])
							{
								case WSAEAFNOSUPPORT :
									if (m_pNetCom->m_pMsgOut)
										m_pNetCom->Error(m_pNetCom->GetName() + _T(" FD_CONNECT Addresses in the specified family cannot be used with this socket"));
									break;
								case WSAECONNREFUSED :
									if (m_pNetCom->m_pMsgOut)
										m_pNetCom->Error(m_pNetCom->GetName() + _T(" FD_CONNECT The attempt to connect was forcefully rejected"));
									break;
								case WSAENETUNREACH :
									if (m_pNetCom->m_pMsgOut)
										m_pNetCom->Error(m_pNetCom->GetName() + _T(" FD_CONNECT The network cannot be reached from this host at this time"));
									break;
								case WSAENOBUFS :
									if (m_pNetCom->m_pMsgOut)
										m_pNetCom->Error(m_pNetCom->GetName() + _T(" FD_CONNECT No buffer space is available. The socket cannot be connected"));
									break;
								case WSAETIMEDOUT :
									if (m_pNetCom->m_pMsgOut)
										m_pNetCom->Error(m_pNetCom->GetName() + _T(" FD_CONNECT Attempt to connect timed out without establishing a connection"));
									break;
								case WSAEADDRNOTAVAIL :
									if (m_pNetCom->m_pMsgOut)
										m_pNetCom->Error(m_pNetCom->GetName() + _T(" FD_CONNECT Cannot assign requested address")); 
									break;
								case 0 : // no error
									if (m_pNetCom->m_pMsgOut)
									{
										m_pNetCom->Notice(	m_pNetCom->GetName() +
															_T(" Normal connection establishment to ") +
															m_pNetCom->GetPeerSockIP());
									}
									break;
								default :
									if (m_pNetCom->m_pMsgOut)
										m_pNetCom->Error(m_pNetCom->GetName() + _T(" FD_CONNECT Unknown Error Code"));
									break;
							}
							if (NetworkEvents.iErrorCode[FD_CONNECT_BIT] != 0)
							{
								// Connect Failed Event
								if (m_pNetCom->m_hConnectFailedEvent)
									::SetEvent(m_pNetCom->m_hConnectFailedEvent);

								// Note that connection may fail while closing, thus we have
								// to make sure that the tx and rx threads are not running!
								m_pNetCom->ShutdownTxThread();
								m_pNetCom->ShutdownRxThread();
								if (::closesocket(m_pNetCom->m_hSocket) == SOCKET_ERROR)
									m_pNetCom->ProcessWSAError(m_pNetCom->GetName() + _T(" closesocket()"));
								else
									m_pNetCom->m_hSocket = INVALID_SOCKET;
								if (m_bClosing)
									SignalClosing();
								if (m_pNetCom->m_pMsgOut)
									m_pNetCom->Notice(m_pNetCom->GetName() + _T(" MsgThread ended (ID = 0x%08X)"), GetId());
								return 0;
							}
							else
							{
								m_pNetCom->m_bClientConnected = TRUE;

								// Start Rx and Tx Threads
								m_pNetCom->StartRxThread();
								m_pNetCom->StartTxThread();

								// Connect Event
								if (m_pNetCom->m_hConnectEvent)
									::SetEvent(m_pNetCom->m_hConnectEvent);
							}
						}
						if (NetworkEvents.lNetworkEvents & FD_READ)
						{
							if (m_pNetCom->m_pMsgOut)
								m_pNetCom->Debug(m_pNetCom->GetName() + _T(" Net Event FD_READ"));

							// Read Event
							if (m_pNetCom->m_hReadEvent)
								::SetEvent(m_pNetCom->m_hReadEvent);

							// Trigger the RX Thread
							::SetEvent(m_pNetCom->m_hRxEvent);
						}
						if (NetworkEvents.lNetworkEvents & FD_CLOSE)
						{
							m_bClosing = TRUE;
							m_pNetCom->m_bClientConnected = FALSE;
							if (m_pNetCom->m_pMsgOut)
								m_pNetCom->Notice(m_pNetCom->GetName() + _T(" Net Event FD_CLOSE"));

							switch (NetworkEvents.iErrorCode[FD_CLOSE_BIT])
							{
								case WSAENETDOWN :
									if (m_pNetCom->m_pMsgOut)
										m_pNetCom->Error(m_pNetCom->GetName() + _T(" FD_CLOSE The network subsystem has failed"));
									break;
								case WSAECONNRESET :
									if (m_pNetCom->m_pMsgOut)
										m_pNetCom->Error(m_pNetCom->GetName() + _T(" FD_CLOSE The connection was reset by the remote side"));
									break;
								case WSAECONNABORTED : // this error may happen when we close the connection and there is still something to read
									if (m_pNetCom->m_pMsgOut)
										m_pNetCom->Warning(m_pNetCom->GetName() + _T(" FD_CLOSE The connection was interrupted due to a time-out or other reason"));
									break;
								case 0 : // no error
									if (m_pNetCom->m_pMsgOut)
										m_pNetCom->Notice(m_pNetCom->GetName() + _T(" FD_CLOSE Normal connection termination"));
									break;
								default :
									if (m_pNetCom->m_pMsgOut)
										m_pNetCom->Error(m_pNetCom->GetName() + _T(" FD_CLOSE Unknown Error Code"));
									break;
							}

							if (bInitConnectionShutdown)
							{
								// Make sure Rx and Tx Threads are stopped!
								m_pNetCom->ShutdownTxThread();
								m_pNetCom->ShutdownRxThread();
								if (::closesocket(m_pNetCom->m_hSocket) == SOCKET_ERROR)
									m_pNetCom->ProcessWSAError(m_pNetCom->GetName() + _T(" closesocket()"));
								else
									m_pNetCom->m_hSocket = INVALID_SOCKET;
								SignalClosing();
								if (m_pNetCom->m_pMsgOut)
									m_pNetCom->Notice(m_pNetCom->GetName() + _T(" MsgThread ended, we started shutdown (ID = 0x%08X)"), GetId());
								return 0;
							}
							else
							{
								// Stop the Tx Thread because after shutdown with SD_SEND,
								// transmitting is not allowed!
								m_pNetCom->ShutdownTxThread();

								// Enable only FD_READ events at this point; on Wine that's needed because
								// another FD_CLOSE event is fired when calling shutdown() and this would
								// create an infinite loop!
								if (::WSAEventSelect(m_pNetCom->m_hSocket, (WSAEVENT)m_pNetCom->m_hNetEvent, FD_READ) == SOCKET_ERROR)
									m_pNetCom->ProcessWSAError(m_pNetCom->GetName() + _T(" WSAEventSelect()"));

								// Shutdown SD_SEND
								if (::shutdown(m_pNetCom->m_hSocket, SD_SEND) == SOCKET_ERROR)
									m_pNetCom->ProcessWSAError(m_pNetCom->GetName() + _T(" shutdown()"));

								// Next Step is entering the case WAIT_TIMEOUT.
								// This is to let finish reading remaining data!
							}
						}
					}
				}
				else
					::WSAResetEvent(m_pNetCom->m_hNetEvent);
				break;

			// Timeout
			case WAIT_TIMEOUT :
				if (m_bClosing && !bInitConnectionShutdown)
				{
					// Make sure Rx and Tx Threads are stopped!
					m_pNetCom->ShutdownTxThread();
					m_pNetCom->ShutdownRxThread();
					if (::closesocket(m_pNetCom->m_hSocket) == SOCKET_ERROR)
						m_pNetCom->ProcessWSAError(m_pNetCom->GetName() + _T(" closesocket()"));
					else
						m_pNetCom->m_hSocket = INVALID_SOCKET;
					SignalClosing();
					if (m_pNetCom->m_pMsgOut)
						m_pNetCom->Notice(m_pNetCom->GetName() + _T(" MsgThread ended, peer started shutdown (ID = 0x%08X)"), GetId());
					return 0;
				}
				else if (bInitConnectionShutdown)
				{
					// Make sure Rx and Tx Threads are stopped!
					m_pNetCom->ShutdownTxThread();
					m_pNetCom->ShutdownRxThread();
					if (::closesocket(m_pNetCom->m_hSocket) == SOCKET_ERROR)
						m_pNetCom->ProcessWSAError(m_pNetCom->GetName() + _T(" closesocket()"));
					else
						m_pNetCom->m_hSocket = INVALID_SOCKET;
					SignalClosing();
					if (m_pNetCom->m_pMsgOut)
						m_pNetCom->Warning(m_pNetCom->GetName() + _T(" MsgThread ended, we started shutdown, peer did not answer (ID = 0x%08X)"), GetId());
					return 0;
				}
				break;
		}
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Rx Thread
///////////////////////////////////////////////////////////////////////////////
int CNetCom::CRxThread::Work() 
{
	DWORD Event;
	DWORD Flags;
	DWORD NumberOfBytesReceived = 0;
	BOOL  bResult;

	if (m_pNetCom->m_pMsgOut)
		m_pNetCom->Notice(m_pNetCom->GetName() + _T(" RxThread started (ID = 0x%08X)"), GetId());

	for(;;)
	{
		// Main wait function
		Event = ::WaitForMultipleObjects(3, m_pNetCom->m_hRxEventArray, FALSE, INFINITE);
		
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :
				if (m_pCurrentBuf)
				{
					delete m_pCurrentBuf;
					m_pCurrentBuf = NULL;
				}
				// Call the Parser last time
				if (m_pNetCom->m_pParseProcess)
					m_pNetCom->m_pParseProcess->NewData(TRUE);
				if (m_pNetCom->m_pMsgOut)
					m_pNetCom->Notice(m_pNetCom->GetName() + _T(" RxThread ended (ID = 0x%08X)"), GetId());
				return 0;
		
			// Overlapped Event
			case WAIT_OBJECT_0 + 1 :
				bResult = ::WSAGetOverlappedResult(m_pNetCom->m_hSocket, &m_pNetCom->m_ovRx, &NumberOfBytesReceived, TRUE, &Flags);
				::WSAResetEvent(m_pNetCom->m_ovRx.hEvent);

				// Error Handling
				if (!bResult)
				{
					m_pNetCom->ProcessWSAError(m_pNetCom->GetName() + _T(" WSAGetOverlappedResult() of WSARecv() or WSARecvFrom()"));
					if (m_pCurrentBuf)
					{
						delete m_pCurrentBuf;
						m_pCurrentBuf = NULL;
					}
				}
				else
				{
					if (m_pNetCom->m_pMsgOut)
						m_pNetCom->Debug(m_pNetCom->GetName() + _T(" Got WSAGetOverlappedResult() of WSARecv() or WSARecvFrom() with %d bytes"), NumberOfBytesReceived);
				}
				// Strange: Even if WSARecv received all the bytes and returned no error,
				// the Overlapped event is triggered! 
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

		if (m_pCurrentBuf && (m_pCurrentBuf->GetMsgSize() > 0))
		{
			// Add Message to Fifo
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
	return 0;
}

__forceinline void CNetCom::CRxThread::Read()
{
	// Do not use the MSG_PARTIAL flag, it indicates that the receive operation
	// should complete even if only part of a message has been received.
	// (This Flag is only for Datagram Packets)
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
		if (res == WSAEWOULDBLOCK || res == WSA_IO_PENDING)
		{
			if (m_pNetCom->m_pMsgOut)
			{
				if (res == WSAEWOULDBLOCK)
					m_pNetCom->Debug(m_pNetCom->GetName() + _T(" Received with WSAEWOULDBLOCK %d bytes"), NumberOfBytesReceived);
				else
					m_pNetCom->Debug(m_pNetCom->GetName() + _T(" Received with WSA_IO_PENDING %d bytes"), NumberOfBytesReceived);
			}
		}
		else
		{
			m_pNetCom->ProcessWSAError(m_pNetCom->GetName() + _T(" WSARecv()"));
			delete m_pCurrentBuf;
			m_pCurrentBuf = NULL;
		}	
	}
	else
	{
		if (m_pNetCom->m_pMsgOut)
			m_pNetCom->Debug(m_pNetCom->GetName() + _T(" Received %d bytes"), NumberOfBytesReceived);

		// Overlapped Event is also called if no error has been returned.
		// -> the following code is not necessary!
		/* m_pCurrentBuf->SetMsgSize(NumberOfBytesReceived);*/
	}
}


///////////////////////////////////////////////////////////////////////////////
// Tx Thread
///////////////////////////////////////////////////////////////////////////////
int CNetCom::CTxThread::Work() 
{
	DWORD Event;
	DWORD Flags;
	BOOL  bResult;
	DWORD NumberOfBytesSent = 0;

	if (m_pNetCom->m_pMsgOut)
		m_pNetCom->Notice(m_pNetCom->GetName() + _T(" TxThread started (ID = 0x%08X)"), GetId());

	for (;;)
	{
		// Main wait function
		Event = ::WaitForMultipleObjects(3, m_pNetCom->m_hTxEventArray, FALSE, INFINITE);
		
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :
				if (m_pCurrentBuf)
				{
					delete m_pCurrentBuf;
					m_pCurrentBuf = NULL;
				}
				if (m_pNetCom->m_pMsgOut)
					m_pNetCom->Notice(m_pNetCom->GetName() + _T(" TxThread ended (ID = 0x%08X)"), GetId());
				return 0;

			// Overlapped Event
			case WAIT_OBJECT_0 + 1 :
				bResult = ::WSAGetOverlappedResult(m_pNetCom->m_hSocket, &m_pNetCom->m_ovTx, &NumberOfBytesSent, TRUE, &Flags);
				::WSAResetEvent(m_pNetCom->m_ovTx.hEvent);

				// Error Handling
				if (!bResult)
				{
					m_pNetCom->ProcessWSAError(m_pNetCom->GetName() + _T(" WSAGetOverlappedResult() of WSASend() or WSASendTo()"));
					if (m_pCurrentBuf)
					{
						delete m_pCurrentBuf;
						m_pCurrentBuf = NULL;
					}
				}
				else
				{
					if (m_pNetCom->m_pMsgOut)
						m_pNetCom->Debug(m_pNetCom->GetName() + _T(" Got WSAGetOverlappedResult() of WSASend() or WSASendTo() with %d bytes"), NumberOfBytesSent);
				}

				// Strange: Even if WSASend sent all the bytes and returned no error,
				// the Overlapped event is triggered! 
				if (m_pCurrentBuf && (m_pCurrentBuf->GetMsgSize() > 0))
				{
					delete m_pCurrentBuf;
					m_pCurrentBuf = NULL;
					
					// Statistics
					if (m_nCurrentTxFifoSize > 1) ::SetEvent(m_pNetCom->m_hTxEvent);
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
	return 0;
}

void CNetCom::CTxThread::Write()
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
		m_pCurrentBuf = m_pNetCom->m_TxFifo.GetHead();
		m_pNetCom->m_TxFifo.RemoveHead();
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
				if (res == WSAEWOULDBLOCK || res == WSA_IO_PENDING)
				{
					if (m_pNetCom->m_pMsgOut)
					{
						if (res == WSAEWOULDBLOCK)
							m_pNetCom->Debug(m_pNetCom->GetName() + _T(" Sent with WSAEWOULDBLOCK %d bytes"), NumberOfBytesSent);
						else
							m_pNetCom->Debug(m_pNetCom->GetName() + _T(" Sent with WSA_IO_PENDING %d bytes"), NumberOfBytesSent);
					}
				}
				else
				{
					m_pNetCom->ProcessWSAError(m_pNetCom->GetName() + _T(" WSASend()"));
					delete m_pCurrentBuf;
					m_pCurrentBuf = NULL;
				}
			}
			else
			{
				if (m_pNetCom->m_pMsgOut)
					m_pNetCom->Debug(m_pNetCom->GetName() + _T(" Sent %d bytes"), NumberOfBytesSent);

				// Overlapped Event is also called if no error has been returned.
				// -> the following code is not necessary!
				/*
				delete m_pCurrentBuf;
				m_pCurrentBuf = NULL;

				// Statistics
				if (m_nCurrentTxFifoSize > 1) ::SetEvent(m_pNetCom->m_hTxEvent);
				*/
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
	if(::WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
		TRACE(_T("WSAStartup() failed\n"));

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
		TRACE(_T("No usable WinSock DLL found\n")); 
	}

	// Handle
	m_hSocket					= INVALID_SOCKET;

	// Pointers
	m_pParseProcess				= NULL;
	m_pMsgOut					= NULL;
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
	m_ovRx.Offset				= 0;
	m_ovRx.OffsetHigh			= 0;
	m_ovTx.Offset				= 0;
	m_ovTx.OffsetHigh			= 0;

	// Create Events
	m_ovRx.hEvent					= ::WSACreateEvent();
	m_ovTx.hEvent					= ::WSACreateEvent();
	m_hNetEvent						= ::WSACreateEvent();
	m_hTxEvent						= ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hRxEvent						= ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hStartConnectionShutdownEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hConnectEvent					= NULL;
	m_hConnectFailedEvent			= NULL;
	m_hCloseEvent					= NULL;
	m_hReadEvent					= NULL;

	// Initialize the Event Object Arrays
	m_hMsgEventArray[0]			= m_pMsgThread ? m_pMsgThread->GetKillEvent() : NULL;	// highest priority
	m_hMsgEventArray[1]			= m_hStartConnectionShutdownEvent;
	m_hMsgEventArray[2]			= m_hNetEvent;
	m_hRxEventArray[0]			= m_pRxThread ? m_pRxThread->GetKillEvent() : NULL;		// highest priority
	m_hRxEventArray[1]			= m_ovRx.hEvent;
	m_hRxEventArray[2]			= m_hRxEvent;
	m_hTxEventArray[0]			= m_pTxThread ? m_pTxThread->GetKillEvent() : NULL;		// highest priority
	m_hTxEventArray[1]			= m_ovTx.hEvent;
	m_hTxEventArray[2]			= m_hTxEvent;

	// Initialize critical sections
	::InitializeCriticalSection(&m_csRxFifoSync);
	::InitializeCriticalSection(&m_csTxFifoSync);

	// Is the Client Connected?
	m_bClientConnected			= FALSE;

	// Must this Class Instance free or not
	m_bFreeMsgOut				= FALSE;

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
	Close();

	// Close the Events
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

	// Free the Threads
	delete m_pMsgThread;
	delete m_pRxThread;
	delete m_pTxThread;

	// Delete critical sections
	::DeleteCriticalSection(&m_csRxFifoSync);
	::DeleteCriticalSection(&m_csTxFifoSync);

	// WinSock Cleanup
	::WSACleanup();
}

BOOL CNetCom::InitAddr(volatile int& nSocketFamily, const CString& sAddress, UINT uiPort, sockaddr* paddr)
{
	if (!paddr)
		return FALSE;
	((sockaddr_in*)paddr)->sin_port = htons((unsigned short)uiPort); // If 0 -> Win Selects a Port
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
					HANDLE hConnectEvent,				// Handle to an Event Object that will get Connect Events.
					HANDLE hConnectFailedEvent,			// Handle to an Event Object that will get Connect Failed Events.
					HANDLE hCloseEvent,					// Handle to an Event Object that will get Close Events.
					HANDLE hReadEvent,					// Handle to an Event Object that will get Read Events.
					CMsgOut* pMsgOut,					// Message Class for Debug, Notice, Warning, Error and Critical Visualization.
					int nSocketFamily)					// Socket family
{
	// First close
	Close();

	// Set init time
	m_InitTime = CTime::GetCurrentTime();

	// Init socket family
	m_nSocketFamily = nSocketFamily;

	// Addresses
	sockaddr_in6 peer_addr6;
	sockaddr* ppeer_addr = (sockaddr*)&peer_addr6;
	memset(&peer_addr6, 0, sizeof(peer_addr6));		// Init to any address
	sockaddr_in6 local_addr6;
	sockaddr* plocal_addr = (sockaddr*)&local_addr6;
	memset(&local_addr6, 0, sizeof(local_addr6));	// Init to any address
	if (!InitAddr(m_nSocketFamily, sPeerAddress, uiPeerPort, ppeer_addr)) // This can change m_nSocketFamily
		return FALSE;
	if (m_nSocketFamily == AF_UNSPEC)
		m_nSocketFamily = AF_INET; // Default to IP4
	ppeer_addr->sa_family = plocal_addr->sa_family = (unsigned short)m_nSocketFamily; // Init family

	// Init Message Out if not supplied
	if (pMsgOut == NULL)
	{
#if defined(_DEBUG) || defined(TRACELOGFILE)
		pMsgOut = (CMsgOut*)new CMsgOut;
		m_bFreeMsgOut = TRUE;
#endif
	}

	// Initialize the Member Variables
	InitVars(	pParseProcess,
				sPeerAddress, uiPeerPort,
				hConnectEvent, hConnectFailedEvent, hCloseEvent, hReadEvent,
				pMsgOut);

	// Init the Parser
	if (m_pParseProcess)
		m_pParseProcess->Init(this);

	if (m_hSocket == INVALID_SOCKET)
	{
		// Create an Overlapped (=Asynchronous) Socket
		if ((m_hSocket = ::WSASocket(m_nSocketFamily, SOCK_STREAM,
									IPPROTO_TCP,
									NULL, 0, WSA_FLAG_OVERLAPPED)) != INVALID_SOCKET)
		{
			// Turn On all Network Events
			if (InitEvents() == FALSE)
				return FALSE;

			// Start Message Thread now so that FD_CONNECT can be handled
			StartMsgThread();
				
			// Connect
			if (!SOCKADDRANY(ppeer_addr))
			{
				if (::WSAConnect(m_hSocket, ppeer_addr, SOCKADDRSIZE(ppeer_addr), NULL, NULL, NULL, NULL) != SOCKET_ERROR)
				{
					if (m_pMsgOut)
						Notice(GetName() + _T(" Connect (%s port %d)"), sPeerAddress, uiPeerPort);
					return TRUE;
				}
				else
				{
					int nErrorCode = ::WSAGetLastError();
					if (nErrorCode == WSAEWOULDBLOCK)
					{
						if (m_pMsgOut)
							Notice(GetName() + _T(" Connect with WSAEWOULDBLOCK (%s port %d)"), sPeerAddress, uiPeerPort);
						return TRUE;
					}
					ProcessWSAError(GetName() + _T(" WSAConnect()"));
					return FALSE;
				}
			}
			else
			{
				if (m_pMsgOut)
					Error(GetName() + _T(" Cannot Connect To A Empty Address!"));
				return FALSE;
			}
		}
		else
		{
			ProcessWSAError(GetName() + _T(" WSASocket()"));
			return FALSE;
		}
	}
	else
		return FALSE;
}

void CNetCom::Close()
{
	// Shutdown connection
	ShutdownConnection_NoBlocking();
	WaitTillShutdown_Blocking();
		
	// Close the socket in case MsgThread was forced to shutdown after timeout
	if (m_hSocket != INVALID_SOCKET)
	{
		if (::closesocket(m_hSocket) == SOCKET_ERROR)
			ProcessWSAError(GetName() + _T(" closesocket()"));
		m_hSocket = INVALID_SOCKET;
	}

	// Empty fifos
	while (!m_RxFifo.IsEmpty())
	{
		CBuf* pBuf = m_RxFifo.GetHead();
		m_RxFifo.RemoveHead();
		delete pBuf;
	}
	while (!m_TxFifo.IsEmpty())
	{
		CBuf* pBuf = m_TxFifo.GetHead();
		m_TxFifo.RemoveHead();
		delete pBuf;
	}

	// Free msg out
	if (m_bFreeMsgOut && m_pMsgOut)
	{
		delete m_pMsgOut;
		m_pMsgOut = NULL;
		m_bFreeMsgOut = FALSE;
	}

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
			{
				ProcessWSAError(GetName() + _T(" getpeername()"));
				return _T("");
			}
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
			{
				ProcessWSAError(GetName() + _T(" getpeername()"));
				return _T("");
			}
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

DWORD CNetCom::EnumLAN(CStringArray* pHosts)
{
	// Check
	if (!pHosts)
		return 0;

	// Free
	pHosts->RemoveAll();

	// Enum
	HANDLE hEnum;
	DWORD res = ::WNetOpenEnum(RESOURCE_CONTEXT, RESOURCETYPE_ANY, NULL, NULL, &hEnum);
	if (res == NO_ERROR)
	{		
		if (hEnum)	
		{
			const int items_at_a_time = 256;
			NETRESOURCE net_resource[items_at_a_time];
			DWORD net_resource_count;
			
			do 
			{
				DWORD count = 0xFFFFFFFF;
				net_resource_count = sizeof(NETRESOURCE)*items_at_a_time;
				memset(net_resource, 0, net_resource_count);
				
				res = ::WNetEnumResource(hEnum, &count, (LPVOID)net_resource, &net_resource_count);				
				if ((res != NO_ERROR) && (res != ERROR_MORE_DATA))
					break;
				
				for (DWORD i = 0 ; i < count ; i++)
				{	
					if (net_resource[i].lpRemoteName)
					{
						const TCHAR* name = net_resource[i].lpRemoteName;
						if (lstrlen(name) >= 2 && name[0] == _T('\\') && name[1] == _T('\\'))
							name += 2;
						pHosts->Add(CString(name));
					}
				}
			} 
			while((res == NO_ERROR) || (res == ERROR_MORE_DATA));
			
			::WNetCloseEnum(hEnum);

			return pHosts->GetSize();
		}
		else
			return 0;
	}
	else
		return 0;
}
 
BOOL CNetCom::StartMsgThread()
{
	if (!m_pMsgThread->IsRunning())
	{
		// Reset eventually set vars from an old closed connection
		// before starting the message thread. Resetting them at
		// the beginning of the message thread could lock at
		// WaitTillShutdown_Blocking() in the Close() function
		// called two times by two consecutive Init()!
		m_pMsgThread->m_bClosing = FALSE;
		::ResetEvent(m_hStartConnectionShutdownEvent);
		if (m_pMsgThread->Start() == true) // this function sets the m_bRunning flag
		{
			return TRUE;
		}
		else
		{
			if (m_pMsgOut)
				Critical(GetName() + _T(" MsgThread start failed (ID = 0x%08X)"), m_pMsgThread->GetId());
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CNetCom::StartRxThread()
{
	if (!m_pRxThread->IsRunning())
	{
		if (m_pRxThread->Start() == true) // this function sets the m_bRunning flag
		{
			return TRUE;
		}
		else
		{
			if (m_pMsgOut)
				Critical(GetName() + _T(" RxThread start failed (ID = 0x%08X)"), m_pRxThread->GetId());
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CNetCom::StartTxThread()
{	
	if (!m_pTxThread->IsRunning())
	{
		if (m_pTxThread->Start() == true) // this function sets the m_bRunning flag
		{
			return TRUE;
		}
		else
		{
			if (m_pMsgOut)
				Critical(GetName() + _T(" TxThread start failed (ID = 0x%08X)"), m_pTxThread->GetId());
			return FALSE;
		}
	}
	return TRUE;
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
			CBuf* pBuf = m_RxFifo.GetHead();
			if (pBuf)
			{
				i += pBuf->GetMsgSize();
				delete pBuf;
			}
			m_RxFifo.RemoveHead();
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

CString CNetCom::GetName()
{
	if (m_hSocket != INVALID_SOCKET)
		return _T("Net_Client");
	else
	{
		return _T("Net");
	}
}

void CNetCom::InitVars(	CParseProcess* pParseProcess,
						CString sPeerAddress,
						UINT uiPeerPort,
						HANDLE hConnectEvent,
						HANDLE hConnectFailedEvent,
						HANDLE hCloseEvent,
						HANDLE hReadEvent,
						CMsgOut* pMsgOut)
{
	// Init member vars
	m_pParseProcess = pParseProcess;
	m_sPeerAddress = sPeerAddress;
	m_uiPeerPort = uiPeerPort;
	m_hConnectEvent = hConnectEvent;
	m_hConnectFailedEvent = hConnectFailedEvent;
	m_hCloseEvent = hCloseEvent;
	m_hReadEvent = hReadEvent;
	m_pMsgOut = pMsgOut;
}

BOOL CNetCom::InitEvents()
{
	if (m_hSocket != INVALID_SOCKET)
	{
		if (::WSAEventSelect(m_hSocket, (WSAEVENT)m_hNetEvent,
						FD_READ | FD_CONNECT | FD_CLOSE) == SOCKET_ERROR)
		{
			ProcessWSAError(GetName() + _T(" WSAEventSelect()"));
			return FALSE;
		}
		else
			return TRUE;
	}
	else
		return FALSE;
}

void CNetCom::ShutdownConnection_NoBlocking()
{
	if (m_pMsgThread->IsRunning() && !m_pMsgThread->m_bClosing)
	{
		m_bClientConnected = FALSE;
		m_pMsgThread->m_bClosing = TRUE;
		::SetEvent(m_hStartConnectionShutdownEvent);
	}
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
	HINSTANCE hInstLib = ::LoadLibrary(_T("Ws2_32.dll"));
	if (hInstLib)
	{
		INT (WSAAPI *lpfGetAddrInfo)(PCWSTR pNodeName, PCWSTR pServiceName, const ADDRINFOW* pHints, PADDRINFOW* ppResult);
		VOID (WSAAPI *lpfFreeAddrInfo)(PADDRINFOW pAddrInfo);
		lpfGetAddrInfo = (INT(WSAAPI*)(PCWSTR, PCWSTR, const ADDRINFOW*, PADDRINFOW*))::GetProcAddress(hInstLib, "GetAddrInfoW");
		lpfFreeAddrInfo = (VOID(WSAAPI*)(PADDRINFOW))::GetProcAddress(hInstLib, "FreeAddrInfoW");
		if (lpfGetAddrInfo && lpfFreeAddrInfo)
		{
			// Do not use ADDRINFOT because it is defined as addrinfo
			// if _WIN32_WINNT < 0x0502 in the atlsocket.h file!
			ADDRINFOW aiHints;
			ADDRINFOW* aiList = NULL;
			memset(&aiHints, 0, sizeof(aiHints));
			aiHints.ai_family = nSocketFamily;
			aiHints.ai_flags = AI_PASSIVE;
			if (lpfGetAddrInfo(sHost, sPort, &aiHints, &aiList) == 0)
			{
				// Do not use ADDRINFOT because it is defined as addrinfo
				// if _WIN32_WINNT < 0x0502 in the atlsocket.h file!
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

void CNetCom::ProcessWSAError(const CString& sErrorText)
{
	if (m_pMsgOut)
	{
		CString sText;
		LPVOID lpMsgBuf = NULL;

		int nLastError = ::WSAGetLastError();
		if (::FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			nLastError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR)&lpMsgBuf,
			0,
			NULL) && lpMsgBuf)
		{
			// Init
			sText = (LPCTSTR)lpMsgBuf;
			
			// Remove terminating CR and LF
			sText.TrimRight(_T("\r\n"));

			// Replace eventual CRs or LFs in the middle of the string with a space
			sText.Replace(_T('\r'), _T(' '));
			sText.Replace(_T('\n'), _T(' '));

			// Format message
			sText = sErrorText + _T(" failed with the following error:\n") + sText;
		}
		else
			sText.Format(_T("%s failed with the following error code: %d"),
								sErrorText, nLastError);

		// Free buffer
		if (lpMsgBuf)
			::LocalFree(lpMsgBuf);

		// Call Error
		Error(_T("%s"), sText);
	}
}

void CNetCom::Critical(const TCHAR* pFormat, ...)
{
	if (pFormat && m_pMsgOut)
	{
		// Get Msg
		CString s;
		va_list arguments;
		va_start(arguments, pFormat);	
		s.FormatV(pFormat, arguments);
		va_end(arguments);

		// Display It
		::EnterCriticalSection(&m_pMsgOut->m_csMessageOut);
		m_pMsgOut->MessageOut(CMsgOut::CRITICAL_MSG, s);
		::LeaveCriticalSection(&m_pMsgOut->m_csMessageOut);
	}
}

void CNetCom::Error(const TCHAR* pFormat, ...)
{
	if (pFormat && m_pMsgOut)
	{
		// Get Msg
		CString s;
		va_list arguments;
		va_start(arguments, pFormat);	
		s.FormatV(pFormat, arguments);
		va_end(arguments);

		// Display It
		::EnterCriticalSection(&m_pMsgOut->m_csMessageOut);
		m_pMsgOut->MessageOut(CMsgOut::ERROR_MSG, s);
		::LeaveCriticalSection(&m_pMsgOut->m_csMessageOut);
	}
}

void CNetCom::Warning(const TCHAR* pFormat, ...)
{
	if (pFormat && m_pMsgOut)
	{
		// Get Msg
		CString s;
		va_list arguments;
		va_start(arguments, pFormat);	
		s.FormatV(pFormat, arguments);
		va_end(arguments);

		// Display It
		::EnterCriticalSection(&m_pMsgOut->m_csMessageOut);
		m_pMsgOut->MessageOut(CMsgOut::WARNING_MSG, s);
		::LeaveCriticalSection(&m_pMsgOut->m_csMessageOut);
	}
}

void CNetCom::Notice(const TCHAR* pFormat, ...)
{
	if (pFormat && m_pMsgOut)
	{
		// Get Msg
		CString s;
		va_list arguments;
		va_start(arguments, pFormat);	
		s.FormatV(pFormat, arguments);
		va_end(arguments);

		// Display It
		::EnterCriticalSection(&m_pMsgOut->m_csMessageOut);
		m_pMsgOut->MessageOut(CMsgOut::NOTICE_MSG, s);
		::LeaveCriticalSection(&m_pMsgOut->m_csMessageOut);
	}
}

void CNetCom::Debug(const TCHAR* pFormat, ...)
{
#if defined(_DEBUG) || defined(TRACELOGFILE)
	if (pFormat && m_pMsgOut)
	{
		// Get Msg
		CString s;
		va_list arguments;
		va_start(arguments, pFormat);	
		s.FormatV(pFormat, arguments);
		va_end(arguments);

		// Display It
		::EnterCriticalSection(&m_pMsgOut->m_csMessageOut);
		m_pMsgOut->MessageOut(CMsgOut::DEBUG_MSG, s);
		::LeaveCriticalSection(&m_pMsgOut->m_csMessageOut);
	}
#endif
}

void CNetCom::CMsgOut::MessageOut(MsgOutCode code, const TCHAR* pMsg)
{
	if (pMsg)
	{
		switch (code)
		{
			case CMsgOut::CRITICAL_MSG :
				TRACE(_T("NETCOM CRITICAL %s\n"), pMsg);
				break;
			case CMsgOut::ERROR_MSG :
				TRACE(_T("NETCOM ERROR %s\n"), pMsg);
				break;
			case CMsgOut::WARNING_MSG :
				TRACE(_T("NETCOM WARNING %s\n"), pMsg);
				break;
			case CMsgOut::NOTICE_MSG :
				TRACE(_T("NETCOM NOTICE %s\n"), pMsg);
				break;
			case CMsgOut::DEBUG_MSG :
				break;
			default :
				break;
		}
	}
}