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
	{	
		if (m_pNetCom->m_lResetEventMask & FD_CLOSE)
			::ResetEvent(m_pNetCom->m_hCloseEvent);
		else
			::SetEvent(m_pNetCom->m_hCloseEvent);
	}

	// Send the Close Message to the Parent Window
	if ((m_pNetCom->m_lOwnerWndNetEvents & FD_CLOSE) && m_pNetCom->m_hOwnerWnd)
	{
		::PostMessage(m_pNetCom->m_hOwnerWnd, WM_NETCOM_CLOSE_EVENT,
			(WPARAM)m_pNetCom,
			(LPARAM)m_pNetCom->m_lParam);

		if (m_pNetCom->m_nIDClose)
			::PostMessage(m_pNetCom->m_hOwnerWnd, WM_COMMAND,
				(WPARAM)m_pNetCom->m_nIDClose, (LPARAM)NULL);

	}
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
								{	
									if (m_pNetCom->m_lResetEventMask & FD_CONNECTFAILED)
										::ResetEvent(m_pNetCom->m_hConnectFailedEvent);
									else
										::SetEvent(m_pNetCom->m_hConnectFailedEvent);
								}

								// Send Connection failed Message to the Parent Window
								if ((m_pNetCom->m_lOwnerWndNetEvents & FD_CONNECTFAILED) && m_pNetCom->m_hOwnerWnd)
								{
									::PostMessage(m_pNetCom->m_hOwnerWnd, WM_NETCOM_CONNECTFAILED_EVENT,
												(WPARAM)m_pNetCom, (LPARAM)m_pNetCom->m_lParam);

									if (m_pNetCom->m_nIDConnectFailed)
										::PostMessage(m_pNetCom->m_hOwnerWnd, WM_COMMAND,
											(WPARAM)m_pNetCom->m_nIDConnectFailed, (LPARAM)NULL);
								}

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
								{	
									if (m_pNetCom->m_lResetEventMask & FD_CONNECT)
										::ResetEvent(m_pNetCom->m_hConnectEvent);
									else
										::SetEvent(m_pNetCom->m_hConnectEvent);
								}

								// Send Connect Message to the Parent Window
								if ((m_pNetCom->m_lOwnerWndNetEvents & FD_CONNECT) && m_pNetCom->m_hOwnerWnd)
								{
									::PostMessage(m_pNetCom->m_hOwnerWnd, WM_NETCOM_CONNECT_EVENT,
												(WPARAM)m_pNetCom, (LPARAM)m_pNetCom->m_lParam);

									if (m_pNetCom->m_nIDConnect)
										::PostMessage(m_pNetCom->m_hOwnerWnd, WM_COMMAND,
											(WPARAM)m_pNetCom->m_nIDConnect, (LPARAM)NULL);
								}
							}
						}
						if (NetworkEvents.lNetworkEvents & FD_READ)
						{
							if (m_pNetCom->m_pMsgOut)
								m_pNetCom->Debug(m_pNetCom->GetName() + _T(" Net Event FD_READ"));

							// Read Event
							if (m_pNetCom->m_hReadEvent)
							{	
								if (m_pNetCom->m_lResetEventMask & FD_READ)
									::ResetEvent(m_pNetCom->m_hReadEvent);
								else
									::SetEvent(m_pNetCom->m_hReadEvent);
							}
							
							// Send Read Message to the Parent Window
							if ((m_pNetCom->m_lOwnerWndNetEvents & FD_READ) && m_pNetCom->m_hOwnerWnd)
							{
								::PostMessage(m_pNetCom->m_hOwnerWnd, WM_NETCOM_READ_EVENT,
											(WPARAM)m_pNetCom, (LPARAM)m_pNetCom->m_lParam);
							
								if (m_pNetCom->m_nIDRead)
									::PostMessage(m_pNetCom->m_hOwnerWnd, WM_COMMAND,
										(WPARAM)m_pNetCom->m_nIDRead, (LPARAM)NULL);
							}

							// Trigger the RX Thread
							::SetEvent(m_pNetCom->m_hRxEvent);
						}
						if (NetworkEvents.lNetworkEvents & FD_WRITE)
						{
							if (m_pNetCom->m_pMsgOut)
								m_pNetCom->Debug(m_pNetCom->GetName() + _T(" Net Event FD_WRITE"));

							// Write Event
							if (m_pNetCom->m_hWriteEvent)
							{	
								if (m_pNetCom->m_lResetEventMask & FD_WRITE)
									::ResetEvent(m_pNetCom->m_hWriteEvent);
								else
									::SetEvent(m_pNetCom->m_hWriteEvent);
							}

							// Send Write Message to the Parent Window
							if ((m_pNetCom->m_lOwnerWndNetEvents & FD_WRITE) && m_pNetCom->m_hOwnerWnd)
							{
								::PostMessage(m_pNetCom->m_hOwnerWnd, WM_NETCOM_WRITE_EVENT,
											(WPARAM)m_pNetCom, (LPARAM)m_pNetCom->m_lParam);
							
								if (m_pNetCom->m_nIDWrite)
									::PostMessage(m_pNetCom->m_hOwnerWnd, WM_COMMAND,
										(WPARAM)m_pNetCom->m_nIDWrite, (LPARAM)NULL);
							}
						}
						if (NetworkEvents.lNetworkEvents & FD_OOB)
						{
							if (m_pNetCom->m_pMsgOut)
								m_pNetCom->Notice(m_pNetCom->GetName() + _T(" Net Event FD_OOB"));

							// OOB Event
							if (m_pNetCom->m_hOOBEvent)
							{	
								if (m_pNetCom->m_lResetEventMask & FD_OOB)
									::ResetEvent(m_pNetCom->m_hOOBEvent);
								else
									::SetEvent(m_pNetCom->m_hOOBEvent);
							}

							// Send OOB Message to the Parent Window
							if ((m_pNetCom->m_lOwnerWndNetEvents & FD_OOB) && m_pNetCom->m_hOwnerWnd)
							{
								::PostMessage(m_pNetCom->m_hOwnerWnd, WM_NETCOM_OOB_EVENT,
												(WPARAM)m_pNetCom, (LPARAM)m_pNetCom->m_lParam);

								if (m_pNetCom->m_nIDOOB)
									::PostMessage(m_pNetCom->m_hOwnerWnd, WM_COMMAND,
										(WPARAM)m_pNetCom->m_nIDOOB, (LPARAM)NULL);
							}
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
	DWORD TriggerBytesReceived = 0;
	DWORD Timeout = INFINITE;
	BOOL  bResult;
	UINT  BufSize;

	if (m_pNetCom->m_pMsgOut)
		m_pNetCom->Notice(m_pNetCom->GetName() + _T(" RxThread started (ID = 0x%08X)"), GetId());

	for(;;)
	{
		if (m_pNetCom->m_uiRxMsgTrigger == 0)
			BufSize = NETCOM_MAX_RX_BUFFER_SIZE;
		else
			BufSize = m_pNetCom->m_uiRxMsgTrigger;

		// Main wait function
		Event = ::WaitForMultipleObjects(4, m_pNetCom->m_hRxEventArray, FALSE, Timeout);
		
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
				Timeout = m_pNetCom->m_uiRxPacketTimeout; // Set Timeout
				if ((m_pNetCom->m_hSocket != INVALID_SOCKET) && (m_pCurrentBuf == NULL))
					Read(BufSize);
				break;

			// Timeout has been changed
			case WAIT_OBJECT_0 + 3 :
				::ResetEvent(m_pNetCom->m_hRxTimeoutChangeEvent);
				continue;

			// Timeout
			case WAIT_TIMEOUT :
				Timeout = INFINITE; // Reset Timeout
				if (m_pNetCom->m_hOwnerWnd)
					::PostMessage(m_pNetCom->m_hOwnerWnd, WM_NETCOM_RX,
								(WPARAM)m_pNetCom, (LPARAM)m_pNetCom->m_lParam);
				if (m_pNetCom->m_hRxMsgTriggerEvent != NULL)
					::SetEvent(m_pNetCom->m_hRxMsgTriggerEvent);
				break;
		}

		unsigned int uiCurrentMsgSize;
		if (m_pCurrentBuf && ((uiCurrentMsgSize = m_pCurrentBuf->GetMsgSize()) > 0))
		{
			// Statistics
			m_pNetCom->m_uiRxByteCount += uiCurrentMsgSize;

			// Make a Copy if RxBuf and RxFifo are both available
			if (m_pNetCom->m_pRxBuf && m_pNetCom->m_bRxBufEnabled && m_pNetCom->m_pRxFifo)
			{
				CBuf* pCopyBuf = new CBuf(uiCurrentMsgSize);
				memcpy((void*)(pCopyBuf->GetBuf()), (void*)(m_pCurrentBuf->GetBuf()), uiCurrentMsgSize);
				pCopyBuf->SetMsgSize(uiCurrentMsgSize);

				// Add Message to Buffer
				::EnterCriticalSection(m_pNetCom->m_pcsRxBufSync);
				m_pNetCom->m_pRxBuf->Add(pCopyBuf);
				::LeaveCriticalSection(m_pNetCom->m_pcsRxBufSync);

				// Add Message to Fifo
				::EnterCriticalSection(m_pNetCom->m_pcsRxFifoSync);
				m_pNetCom->m_pRxFifo->AddTail(m_pCurrentBuf);
				::LeaveCriticalSection(m_pNetCom->m_pcsRxFifoSync);

				// Notify Parent that a Buffer was added to the RxBuf
				if (m_pNetCom->m_hOwnerWnd)
					::PostMessage(m_pNetCom->m_hOwnerWnd, WM_NETCOM_RXBUF_ADD,
						(WPARAM)m_pNetCom, (LPARAM)m_pNetCom->m_lParam);
			}
			else
			{
				// Add Message to Fifo
				if (m_pNetCom->m_pRxFifo)
				{
					::EnterCriticalSection(m_pNetCom->m_pcsRxFifoSync);
					m_pNetCom->m_pRxFifo->AddTail(m_pCurrentBuf);
					::LeaveCriticalSection(m_pNetCom->m_pcsRxFifoSync);
				}
				// Add Message to Buffer
				else if (m_pNetCom->m_pRxBuf && m_pNetCom->m_bRxBufEnabled)
				{
					::EnterCriticalSection(m_pNetCom->m_pcsRxBufSync);
					m_pNetCom->m_pRxBuf->Add(m_pCurrentBuf);
					::LeaveCriticalSection(m_pNetCom->m_pcsRxBufSync);

					// Notify Parent that a Buffer was added to the RxBuf
					if (m_pNetCom->m_hOwnerWnd)
						::PostMessage(m_pNetCom->m_hOwnerWnd, WM_NETCOM_RXBUF_ADD,
							(WPARAM)m_pNetCom, (LPARAM)m_pNetCom->m_lParam);
				}
				else
					delete m_pCurrentBuf;
			}

			// Set to NULL!
			m_pCurrentBuf = NULL;

			// Check the Fifo Size
			::EnterCriticalSection(m_pNetCom->m_pcsRxFifoSync);
			if (m_pNetCom->m_pRxFifo				&&
				(m_pNetCom->m_uiMaxRxFifoSize > 0)	&&
				((UINT)m_pNetCom->m_pRxFifo->GetCount() > m_pNetCom->m_uiMaxRxFifoSize))
			{
				CBuf* pBuf = m_pNetCom->m_pRxFifo->GetHead();
				m_pNetCom->m_pRxFifo->RemoveHead();
				delete pBuf;
			}
			::LeaveCriticalSection(m_pNetCom->m_pcsRxFifoSync);

			// Call the Parser
			if (m_pNetCom->m_pParseProcess)
				m_pNetCom->m_pParseProcess->NewData(FALSE);

			// Notify parent and set the event if at least m_uiRxMsgTrigger were received,
			// or if the Rx Timeout elapsed.
			if (m_pNetCom->m_uiRxMsgTrigger)
			{
				TriggerBytesReceived += uiCurrentMsgSize;
				if (m_pNetCom->m_uiRxMsgTrigger <= TriggerBytesReceived)
				{
					Timeout = INFINITE; // Reset Timeout
					TriggerBytesReceived = 0;
					if (m_pNetCom->m_hOwnerWnd)
					{
						::PostMessage(m_pNetCom->m_hOwnerWnd, WM_NETCOM_RX,
							(WPARAM)m_pNetCom, (LPARAM)m_pNetCom->m_lParam);

						if (m_pNetCom->m_nIDRx)
							::PostMessage(m_pNetCom->m_hOwnerWnd, WM_COMMAND,
								(WPARAM)m_pNetCom->m_nIDRx, (LPARAM)NULL);
					}
					if (m_pNetCom->m_hRxMsgTriggerEvent != NULL)
						::SetEvent(m_pNetCom->m_hRxMsgTriggerEvent);		
				}
			}
		}
	}
	return 0;
}

__forceinline void CNetCom::CRxThread::Read(UINT BufSize)
{
	// Do not use the MSG_PARTIAL flag, it indicates that the receive operation
	// should complete even if only part of a message has been received.
	// (This Flag is only for Datagram Packets)
	DWORD Flags = 0;
	WSABUF WSABuffer;
	m_pCurrentBuf = new CBuf(BufSize);
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
		Event = ::WaitForMultipleObjects(4, m_pNetCom->m_hTxEventArray, FALSE, m_pNetCom->m_uiTxPacketTimeout);
		
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

				// Statistics
				// Strange: Even if WSASend sent all the bytes and returned no error,
				// the Overlapped event is triggered! 
				if (m_pCurrentBuf && (m_pCurrentBuf->GetMsgSize() > 0))
				{
					// Store into TxBuf if available
					if (m_pNetCom->m_pTxBuf && m_pNetCom->m_bTxBufEnabled)
					{
						::EnterCriticalSection(m_pNetCom->m_pcsTxBufSync);
						m_pNetCom->m_pTxBuf->Add(m_pCurrentBuf);
						::LeaveCriticalSection(m_pNetCom->m_pcsTxBufSync);

						// Notify Parent that a Buffer was added to the TxBuf
						if (m_pNetCom->m_hOwnerWnd)
							::PostMessage(m_pNetCom->m_hOwnerWnd, WM_NETCOM_TXBUF_ADD,
								(WPARAM)m_pNetCom, (LPARAM)m_pNetCom->m_lParam);
					}
					else
						delete m_pCurrentBuf;
					m_pCurrentBuf = NULL;
					
					// Statistics
					if (NumberOfBytesSent > 0) m_pNetCom->m_uiTxByteCount += NumberOfBytesSent;
					if (m_nCurrentTxFifoSize > 1) ::SetEvent(m_pNetCom->m_hTxEvent);
				}
				break;
		
			// Tx Event
			case WAIT_OBJECT_0 + 2 :
				::ResetEvent(m_pNetCom->m_hTxEvent);
				if (m_pCurrentBuf == NULL) // If not sending
					Write();
				break; 

			// Timeout has been changed
			case WAIT_OBJECT_0 + 3 :
				::ResetEvent(m_pNetCom->m_hTxTimeoutChangeEvent);
				break;

			// Timeout
			case WAIT_TIMEOUT :
				if (m_pCurrentBuf == NULL) // If not sending
					Write();
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
		::EnterCriticalSection(m_pNetCom->m_pcsTxFifoSync);
		m_nCurrentTxFifoSize = m_pNetCom->m_pTxFifo->GetCount(); 
		if (m_nCurrentTxFifoSize == 0)
		{
			::LeaveCriticalSection(m_pNetCom->m_pcsTxFifoSync);
			return;
		}
		m_pCurrentBuf = m_pNetCom->m_pTxFifo->GetHead();
		m_pNetCom->m_pTxFifo->RemoveHead();
		::LeaveCriticalSection(m_pNetCom->m_pcsTxFifoSync);

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
				// Store into TxBuf if available
				if (m_pNetCom->m_pTxBuf && m_pNetCom->m_bTxBufEnabled)
				{
					::EnterCriticalSection(m_pNetCom->m_pcsTxBufSync);
					m_pNetCom->m_pTxBuf->Add(m_pCurrentBuf);
					::LeaveCriticalSection(m_pNetCom->m_pcsTxBufSync);

					// Notify Parent that a Buffer was added to the TxBuf
					if (m_pNetCom->m_hOwnerWnd)
						::PostMessage(m_pNetCom->m_hOwnerWnd, WM_NETCOM_TXBUF_ADD,
							(WPARAM)m_pNetCom, (LPARAM)m_pNetCom->m_lParam);
				}
				else
					delete m_pCurrentBuf;
				m_pCurrentBuf = NULL;

				// Statistics
				if (NumberOfBytesSent > 0) m_pNetCom->m_uiTxByteCount += NumberOfBytesSent;
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

	// Handles
	m_hSocket					= INVALID_SOCKET;
	m_hRxMsgTriggerEvent		= NULL;
	m_hOwnerWnd					= NULL;

	// Pointers
	m_pParseProcess				= NULL;
	m_lParam					= 0;
	m_pRxBuf					= NULL;
	m_pRxFifo					= NULL;
	m_pTxBuf					= NULL;
	m_pTxFifo					= NULL;
	m_pcsRxBufSync				= NULL;
	m_pcsRxFifoSync				= NULL;
	m_pcsTxBufSync				= NULL;
	m_pcsTxFifoSync				= NULL;
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
	m_hTxTimeoutChangeEvent			= ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hRxTimeoutChangeEvent			= ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hAcceptEvent					= NULL;
	m_hConnectEvent					= NULL;
	m_hConnectFailedEvent			= NULL;
	m_hCloseEvent					= NULL;
	m_hReadEvent					= NULL;
	m_hWriteEvent					= NULL;
	m_hOOBEvent						= NULL;

	// Initialize the Event Object Arrays
	m_hMsgEventArray[0]			= m_pMsgThread ? m_pMsgThread->GetKillEvent() : NULL;	// highest priority
	m_hMsgEventArray[1]			= m_hStartConnectionShutdownEvent;
	m_hMsgEventArray[2]			= m_hNetEvent;
	m_hRxEventArray[0]			= m_pRxThread ? m_pRxThread->GetKillEvent() : NULL;		// highest priority
	m_hRxEventArray[1]			= m_ovRx.hEvent;
	m_hRxEventArray[2]			= m_hRxEvent;
	m_hRxEventArray[3]			= m_hRxTimeoutChangeEvent;
	m_hTxEventArray[0]			= m_pTxThread ? m_pTxThread->GetKillEvent() : NULL;		// highest priority
	m_hTxEventArray[1]			= m_ovTx.hEvent;
	m_hTxEventArray[2]			= m_hTxEvent;
	m_hTxEventArray[3]			= m_hTxTimeoutChangeEvent;

	// Is the Client Connected?
	m_bClientConnected			= FALSE;

	// Must this Class Instance free or not
	m_bFreeRxBufSync			= FALSE;
	m_bFreeRxFifo				= FALSE;
	m_bFreeRxFifoSync			= FALSE;
	m_bFreeTxBufSync			= FALSE;
	m_bFreeTxFifo				= FALSE;
	m_bFreeTxFifoSync			= FALSE;
	m_bFreeMsgOut				= FALSE;

	// Addresses
	m_sLocalAddress = _T("");
	m_sPeerAddress = _T("");
	
	// Ports
	m_uiLocalPort = 0;
	m_uiPeerPort = 0;

	// Number of Rx Bytes that trigger a WM_NETCOM_RX Msg
	m_uiRxMsgTrigger = 0;

	// Statistics
	m_uiRxByteCount = 0;
	m_uiTxByteCount = 0;

	m_uiMaxTxPacketSize = NETCOM_MAX_TX_BUFFER_SIZE;
	
	m_uiRxPacketTimeout = INFINITE;
	m_uiTxPacketTimeout = INFINITE;

	m_lResetEventMask = 0;
	m_lOwnerWndNetEvents = 0;

	// Disable use of the Rx and Tx Buffers
	m_bRxBufEnabled = FALSE;
	m_bTxBufEnabled = FALSE;

	// Configuration Variables
	m_uiMaxRxFifoSize = 0;	// No Rx size limit
	m_nIDAccept = 0;
	m_nIDConnect = 0;
	m_nIDConnectFailed = 0;
	m_nIDRead = 0;
	m_nIDWrite = 0;
	m_nIDOOB = 0;
	m_nIDClose = 0;
	m_nIDRx = 0;

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
	if (m_hTxTimeoutChangeEvent != NULL)
	{
		::ResetEvent(m_hTxTimeoutChangeEvent);
		::CloseHandle(m_hTxTimeoutChangeEvent);
		m_hTxTimeoutChangeEvent = NULL;
	}
	if (m_hRxTimeoutChangeEvent != NULL)
	{
		::ResetEvent(m_hRxTimeoutChangeEvent);
		::CloseHandle(m_hRxTimeoutChangeEvent);
		m_hRxTimeoutChangeEvent = NULL;
	}

	// Free the Threads
	delete m_pMsgThread;
	delete m_pRxThread;
	delete m_pTxThread;

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

BOOL CNetCom::Init(	HWND hOwnerWnd,						// The Optional Owner Window to which send the Network Events.
					LPARAM	lParam,						// The lParam to send with the Messages
					BUFARRAY* pRxBuf,					// The Optional Rx Buffer.
					LPCRITICAL_SECTION pcsRxBufSync,	// The Optional Critical Section for the Rx Buffer.
					BUFQUEUE* pRxFifo,					// The Optional Rx Fifo.
					LPCRITICAL_SECTION pcsRxFifoSync,	// The Optional Critical Section fot the Rx Fifo.
					BUFARRAY* pTxBuf,					// The Optional Tx Buffer.
					LPCRITICAL_SECTION pcsTxBufSync,	// The Optional Critical Section for the Tx Buffer.
					BUFQUEUE* pTxFifo,					// The Optional Tx Fifo.
					LPCRITICAL_SECTION pcsTxFifoSync,	// The Optional Critical Section for the Tx Fifo.
					CParseProcess* pParseProcess,		// Parser & Processor
					CString sLocalAddress,				// Local Address (IP or Host Name), if _T("") Any Address is ok
					UINT uiLocalPort,					// Local Port, if 0 -> Win Selects a Port
					CString sPeerAddress,				// Peer Address (IP or Host Name), if _T("") Any Address is ok
					UINT uiPeerPort,					// Peer Port, if 0 -> Win Selects a Port
					HANDLE hAcceptEvent,				// Handle to an Event Object that will get Accept Events.
					HANDLE hConnectEvent,				// Handle to an Event Object that will get Connect Events.
					HANDLE hConnectFailedEvent,			// Handle to an Event Object that will get Connect Failed Events.
					HANDLE hCloseEvent,					// Handle to an Event Object that will get Close Events.
					HANDLE hReadEvent,					// Handle to an Event Object that will get Read Events.
					HANDLE hWriteEvent,					// Handle to an Event Object that will get Write Events.
					HANDLE hOOBEvent,					// Handle to an Event Object that will get OOB Events.
					long lResetEventMask,				// A combination of network events:
														// FD_ACCEPT | FD_CONNECT | FD_CONNECTFAILED | FD_CLOSE | FD_READ | FD_WRITE | FD_OOB
														// A set value means that instead of setting an event it is reset.
					long lOwnerWndNetEvents,			// A combination of network events:
														// FD_ACCEPT | FD_CONNECT | FD_CONNECTFAILED | FD_CLOSE | FD_READ | FD_WRITE | FD_OOB
														// The Following messages will be sent to the pOwnerWnd (if pOwnerWnd != NULL):
														// WM_NETCOM_ACCEPT_EVENT -> Notification of incoming connections.
														// WM_NETCOM_CONNECT_EVENT -> Notification of completed connection or multipoint "join" operation.
														// WM_NETCOM_CONNECTFAILED_EVENT -> Notification of connection failure.
														// WM_NETCOM_CLOSE_EVENT -> Notification of socket closure.
														// WM_NETCOM_READ_EVENT -> Notification of readiness for reading.
														// WM_NETCOM_WRITE_EVENT -> Notification of readiness for writing.
														// WM_NETCOM_OOB_EVENT -> Notification of the arrival of out-of-band data. 
					UINT uiRxMsgTrigger,				// The number of bytes that triggers an hRxMsgTriggerEvent 
														// (if hRxMsgTriggerEvent != NULL).
														// And/Or the number of bytes that triggers a WM_NETCOM_RX Message
														// (if pOwnerWnd != NULL).
														// Upper bound for this value is NETCOM_MAX_RX_BUFFER_SIZE.
					HANDLE hRxMsgTriggerEvent,			// Handle to an Event Object that will get an Event
														// each time uiRxMsgTrigger bytes arrived.
					UINT uiMaxTxPacketSize,				// The maximum size for transmitted packets,
														// upper bound for this value is NETCOM_MAX_TX_BUFFER_SIZE.
					UINT uiRxPacketTimeout,				// After this timeout a Packet is returned
														// even if the uiRxMsgTrigger size is not reached (A zero meens INFINITE Timeout).
					UINT uiTxPacketTimeout,				// After this timeout a Packet is sent
														// even if no Write Event Happened (A zero meens INFINITE Timeout).
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

	// Limit the Maximum size of the sent packets
	if ((uiMaxTxPacketSize == 0) || (uiMaxTxPacketSize > NETCOM_MAX_TX_BUFFER_SIZE))
		uiMaxTxPacketSize = NETCOM_MAX_TX_BUFFER_SIZE;

	// Limit the Trigger Size
	if (uiRxMsgTrigger > NETCOM_MAX_RX_BUFFER_SIZE)
		uiRxMsgTrigger = NETCOM_MAX_RX_BUFFER_SIZE;

	// Timeouts
	if (uiRxPacketTimeout == 0)
		uiRxPacketTimeout = INFINITE;
	if (uiTxPacketTimeout == 0)
		uiTxPacketTimeout = INFINITE;

	// Init Message Out if not supplied
	if (pMsgOut == NULL)
	{
#if defined(_DEBUG) || defined(TRACELOGFILE)
		pMsgOut = (CMsgOut*)new CMsgOut;
		m_bFreeMsgOut = TRUE;
#endif
	}

	// Statistics Reset
	m_uiRxByteCount = 0;
	m_uiTxByteCount = 0;

	// Initialize the Member Variables
	InitVars(	hOwnerWnd, lParam, pRxBuf, pcsRxBufSync, pRxFifo, pcsRxFifoSync,
				pTxBuf, pcsTxBufSync, pTxFifo, pcsTxFifoSync, pParseProcess, sLocalAddress, uiLocalPort,
				sPeerAddress, uiPeerPort, hAcceptEvent, hConnectEvent, hConnectFailedEvent, hCloseEvent,
				hReadEvent, hWriteEvent, hOOBEvent, lResetEventMask, lOwnerWndNetEvents,
				uiRxMsgTrigger, hRxMsgTriggerEvent, uiMaxTxPacketSize, uiRxPacketTimeout, uiTxPacketTimeout, pMsgOut);

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

	m_hRxMsgTriggerEvent = NULL;

	if (m_bFreeRxBufSync && m_pcsRxBufSync)
	{
		::DeleteCriticalSection(m_pcsRxBufSync);
		delete m_pcsRxBufSync;
		m_pcsRxBufSync = NULL;
		m_bFreeRxBufSync = FALSE;
	}

	if (m_bFreeRxFifo && m_pRxFifo)
	{
		while (!m_pRxFifo->IsEmpty())
		{
			CBuf* pBuf = m_pRxFifo->GetHead();
			m_pRxFifo->RemoveHead();
			delete pBuf;
		}
		delete m_pRxFifo;
		m_pRxFifo = NULL;
		m_bFreeRxFifo = FALSE;
	}
	if (m_bFreeRxFifoSync && m_pcsRxFifoSync)
	{
		::DeleteCriticalSection(m_pcsRxFifoSync);
		delete m_pcsRxFifoSync;
		m_pcsRxFifoSync = NULL;
		m_bFreeRxFifoSync = FALSE;
	}

	if (m_bFreeTxBufSync && m_pcsTxBufSync)
	{
		::DeleteCriticalSection(m_pcsTxBufSync);
		delete m_pcsTxBufSync;
		m_pcsTxBufSync = NULL;
		m_bFreeTxBufSync = FALSE;
	}

	if (m_bFreeTxFifo && m_pTxFifo)
	{
		while (!m_pTxFifo->IsEmpty())
		{
			CBuf* pBuf = m_pTxFifo->GetHead();
			m_pTxFifo->RemoveHead();
			delete pBuf;
		}
		delete m_pTxFifo;
		m_pTxFifo = NULL;
		m_bFreeTxFifo = FALSE;
	}
	if (m_bFreeTxFifoSync && m_pcsTxFifoSync)
	{
		::DeleteCriticalSection(m_pcsTxFifoSync);
		delete m_pcsTxFifoSync;
		m_pcsTxFifoSync = NULL;
		m_bFreeTxFifoSync = FALSE;
	}

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
	if (m_pcsRxFifoSync && m_pRxFifo)
	{
		::EnterCriticalSection(m_pcsRxFifoSync);
		POSITION pos = m_pRxFifo->GetHeadPosition();
		while (pos)
		{
			CBuf* pBuf = m_pRxFifo->GetNext(pos);
			if (pBuf)
				nSize += pBuf->GetMsgSize();
		}
		::LeaveCriticalSection(m_pcsRxFifoSync);
	}
	return nSize;
}

int CNetCom::Read(BYTE* Data/*=NULL*/, int BufSize/*=0*/)
{
	unsigned int i = 0;
	int pos = 0;

	if (m_pcsRxFifoSync && m_pRxFifo)
	{
		// Empty Read
		if (Data == NULL && BufSize <= 0)
		{
			::EnterCriticalSection(m_pcsRxFifoSync);
			while (!m_pRxFifo->IsEmpty())
			{
				CBuf* pBuf = m_pRxFifo->GetHead();
				if (pBuf)
				{
					i += pBuf->GetMsgSize();
					delete pBuf;
				}
				m_pRxFifo->RemoveHead();
			}
			::LeaveCriticalSection(m_pcsRxFifoSync);
			return i;
		}

		// Zero or wrong size
		if (BufSize <= 0)
			return 0;

		::EnterCriticalSection(m_pcsRxFifoSync);
		while (!m_pRxFifo->IsEmpty())
		{
			CBuf* pBuf = m_pRxFifo->GetHead();
			::LeaveCriticalSection(m_pcsRxFifoSync);
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
					::EnterCriticalSection(m_pcsRxFifoSync);
					m_pRxFifo->RemoveHead();
					delete pBuf;
					::LeaveCriticalSection(m_pcsRxFifoSync);
				}
				return pos;
			}

			::EnterCriticalSection(m_pcsRxFifoSync);
			m_pRxFifo->RemoveHead();
			delete pBuf;
		}
		::LeaveCriticalSection(m_pcsRxFifoSync);
		return pos;
	}
	else
		return 0;
}

int CNetCom::Write(BYTE* Data, int Size)
{
	if ((Size == 0) || (Data == NULL))
		return 0;

	if ((m_hSocket != INVALID_SOCKET) && m_pcsTxFifoSync && m_pTxFifo)
	{
		int SentSize = 0;
		while (SentSize < Size)
		{
			CBuf* pBuf;
			if ((Size - SentSize) > (int)m_uiMaxTxPacketSize)
			{
				pBuf = new CBuf(m_uiMaxTxPacketSize);
				memcpy((void*)(pBuf->GetBuf()), (void*)(Data+SentSize), m_uiMaxTxPacketSize);
				pBuf->SetMsgSize(m_uiMaxTxPacketSize);
				SentSize += m_uiMaxTxPacketSize;
			}
			else
			{
				pBuf = new CBuf(Size - SentSize);
				memcpy((void*)(pBuf->GetBuf()), (void*)(Data+SentSize), Size - SentSize);
				pBuf->SetMsgSize(Size - SentSize);
				SentSize = Size;
			}
			::EnterCriticalSection(m_pcsTxFifoSync);
			m_pTxFifo->AddTail(pBuf);
			::LeaveCriticalSection(m_pcsTxFifoSync);
			::SetEvent(m_hTxEvent);
		}
		return SentSize;
	}
	else
		return 0;
}

CNetCom::CBuf* CNetCom::ReadHeadBuf()
{
	CNetCom::CBuf* pBuf = NULL;

	if (m_pcsRxFifoSync && m_pRxFifo)
	{
		::EnterCriticalSection(m_pcsRxFifoSync);
		if (!m_pRxFifo->IsEmpty())
		{
			pBuf = m_pRxFifo->GetHead();
			m_pRxFifo->RemoveHead();
		}
		::LeaveCriticalSection(m_pcsRxFifoSync);
	}
	
	return pBuf;
}

CNetCom::CBuf* CNetCom::GetReadHeadBuf()
{
	CNetCom::CBuf* pBuf = NULL;

	if (m_pcsRxFifoSync && m_pRxFifo)
	{
		::EnterCriticalSection(m_pcsRxFifoSync);
		if (!m_pRxFifo->IsEmpty())
			pBuf = m_pRxFifo->GetHead();
		::LeaveCriticalSection(m_pcsRxFifoSync);
	}
	
	return pBuf;
}

BOOL CNetCom::RemoveReadHeadBuf()
{
	BOOL bRes = FALSE;

	if (m_pcsRxFifoSync && m_pRxFifo)
	{
		::EnterCriticalSection(m_pcsRxFifoSync);
		if (!m_pRxFifo->IsEmpty())
		{
			m_pRxFifo->RemoveHead();
			bRes = TRUE;
		}
		::LeaveCriticalSection(m_pcsRxFifoSync);
	}
	
	return bRes;
}

int CNetCom::FindByte(BYTE b)
{
	int bytepos = 0;

	if (m_pcsRxFifoSync && m_pRxFifo)
	{
		::EnterCriticalSection(m_pcsRxFifoSync);
		POSITION position = m_pRxFifo->GetHeadPosition();
		::LeaveCriticalSection(m_pcsRxFifoSync);
		if (!position) return -1;
		while (position)
		{
			CBuf* pBuf;
			::EnterCriticalSection(m_pcsRxFifoSync);
			pBuf = m_pRxFifo->GetNext(position);
			::LeaveCriticalSection(m_pcsRxFifoSync);
			for (unsigned int i = 0 ; i < pBuf->GetMsgSize() ; i++)
			{
				if ((pBuf->GetBuf())[i] == b) return bytepos;
				bytepos++;
			}
		}
	}
	return -1;
}

int CNetCom::ReadLine(BYTE* Data, int MaxSize)
{
	int LFPos;
	int CRPos;
	
	LFPos = FindByte('\n');
	if (LFPos >= 0)
	{
		if ((LFPos+1) <= MaxSize)
			return Read(Data, LFPos+1);
		else
			return 0;
	}

	CRPos = FindByte('\r');
	if (CRPos >= 0)
	{
		if ((CRPos+1) <= MaxSize)
			return Read(Data, CRPos+1);
		else
			return 0;
	}
	
	return 0;
}

int CNetCom::ReadCRLFLine(BYTE* Data, int MaxSize)
{
	int LFPos;
	int CRPos;
	
	LFPos = FindByte('\n');
	CRPos = FindByte('\r');

	if ((LFPos >= 0) && (CRPos >= 0))
		if (CRPos == (LFPos-1))
			if ((LFPos+1) <= MaxSize)
				return Read(Data, LFPos+1);
	
	return 0;
}

int CNetCom::WriteStr(LPCTSTR str, BOOL bEscapeSeq/*=FALSE*/)
{
	int nSize = _tcslen(str);
#ifdef _UNICODE
	// Convert to Normal Bytes
	char* buf = new char[nSize+1]; // +1 for null termination
	if (::WideCharToMultiByte(CP_ACP, 0, str, nSize+1, buf, nSize+1, NULL, FALSE) != (nSize+1))
	{
		delete [] buf;
		return 0;
	}
	buf[nSize] = '\0';
	if (bEscapeSeq) 
		nSize = StrToByte(buf); // Convert Escape Sequences to Bytes
	int res = Write((BYTE*)buf, nSize);
	delete [] buf;
	return res;
#else
	if (bEscapeSeq)
	{
		char* buf = new char[nSize+1]; // +1 for null termination
		strcpy(buf, str);
		nSize = StrToByte(buf); // Convert Escape Sequences to Bytes
		int res = Write((BYTE*)buf, nSize);
		delete [] buf;
		return res;
	}
	else
		return Write((BYTE*)str, nSize);
#endif
}

/*
Escape Sequence Represents 
\a Bell
\b Backspace 
\f Formfeed 
\n New line 
\r Carriage return 
\t Horizontal tab 
\v Vertical tab 
\\ Backslash 
\ddd ASCII character in decimal notation 
\0ooo ASCII character in octal notation 
\xhh ASCII character in hexadecimal notation 
\0xhh ASCII character in hexadecimal notation 

Note: If a backslash precedes a character that does not appear above,
	  the undefined character is handled as the character itself.
	  For example, \x is treated as an x.
*/
int CNetCom::StrToByte(char* str)
{
	int size = strlen(str);
	char* pstopstring;
	char* buf = new char[size];
	int count = 0;
	for (int i = 0 ; i < size ; i++)
	{
		if (str[i] == '\\') // Escape Char
		{
			if (++i < size)
			{
				pstopstring = NULL;
				switch (str[i])
				{
					case 'A' :	
					case 'a' : buf[count++] = 0x07; break; // Bell
					case 'B' :
					case 'b' : buf[count++] = 0x08; break; // Backspace
					case 'F' :
					case 'f' : buf[count++] = 0x0C; break; // Formfeed
					case 'N' :
					case 'n' : buf[count++] = 0x0A; break; // New line
					case 'R' :
					case 'r' : buf[count++] = 0x0D; break; // Carriage return
					case 'T' :
					case 't' : buf[count++] = 0x09; break; // Horizontal tab
					case 'V' :
					case 'v' : buf[count++] = 0x0B; break; // Vertical tab
					case '\\': buf[count++] = '\\'; break; // Backslash
					case 'X' :
					case 'x' : if (++i < size) buf[count++] = (char)strtoul(str+i, &pstopstring, 16); break; // Base 16
					case '0' : buf[count++] = (char)strtoul(str+i, &pstopstring, 0); break; // Base 8 or base 16
					case '1' :
					case '2' :
					case '3' :
					case '4' :
					case '5' :
					case '6' :
					case '7' :
					case '8' :
					case '9' : buf[count++] = (char)strtoul(str+i, &pstopstring, 10); break; // Base 10
					default  : buf[count++] = str[i]; 
				}
				if ((unsigned int)pstopstring > (unsigned int)(str+i))
					i = pstopstring-str-1;
			}
		}
		else
			buf[count++] = str[i];
	}

	// Copy back
	memcpy(str, buf, count);
	delete [] buf;
	
	// Return the count
	return count;
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

void CNetCom::InitVars(	HWND hOwnerWnd,
						LPARAM	lParam,
						BUFARRAY* pRxBuf,
						LPCRITICAL_SECTION pcsRxBufSync,
						BUFQUEUE* pRxFifo,
						LPCRITICAL_SECTION pcsRxFifoSync,
						BUFARRAY* pTxBuf,
						LPCRITICAL_SECTION pcsTxBufSync,
						BUFQUEUE* pTxFifo,
						LPCRITICAL_SECTION pcsTxFifoSync,
						CParseProcess* pParseProcess,
						CString sLocalAddress,
						UINT uiLocalPort,
						CString sPeerAddress,
						UINT uiPeerPort,
						HANDLE hAcceptEvent,
						HANDLE hConnectEvent,
						HANDLE hConnectFailedEvent,
						HANDLE hCloseEvent,
						HANDLE hReadEvent,
						HANDLE hWriteEvent,
						HANDLE hOOBEvent,
						long lResetEventMask,
						long lOwnerWndNetEvents,
						UINT uiRxMsgTrigger,
						HANDLE hRxMsgTriggerEvent,
						UINT uiMaxTxPacketSize,
						UINT uiRxPacketTimeout,
						UINT uiTxPacketTimeout,
						CMsgOut* pMsgOut)
{
	// Init the Buffer and the Fifos
	if (pRxBuf)
	{
		m_pRxBuf = pRxBuf;

		if (pcsRxBufSync == NULL)
		{
			m_bFreeRxBufSync = TRUE;
			m_pcsRxBufSync = new CRITICAL_SECTION;
			::InitializeCriticalSection(m_pcsRxBufSync);
		}
		else
		{
			m_bFreeRxBufSync = FALSE;
			m_pcsRxBufSync = pcsRxBufSync;
		}
	}
	else
	{
		m_pRxBuf = NULL;
		m_bFreeRxBufSync = FALSE;
		m_pcsRxBufSync = NULL;
	}
	
	if (pRxFifo == NULL)
	{
		m_bFreeRxFifo = TRUE;
		m_pRxFifo = new BUFQUEUE;
	}
	else
	{
		m_bFreeRxFifo = FALSE;
		m_pRxFifo = pRxFifo;
	}
	if (pcsRxFifoSync == NULL)
	{
		m_bFreeRxFifoSync = TRUE;
		m_pcsRxFifoSync = new CRITICAL_SECTION;
		::InitializeCriticalSection(m_pcsRxFifoSync);
	}
	else
	{
		m_bFreeRxFifoSync = FALSE;
		m_pcsRxFifoSync = pcsRxFifoSync;
	}

	if (pTxBuf)
	{
		m_pTxBuf = pTxBuf;

		if (pcsTxBufSync == NULL)
		{
			m_bFreeTxBufSync = TRUE;
			m_pcsTxBufSync = new CRITICAL_SECTION;
			::InitializeCriticalSection(m_pcsTxBufSync);
		}
		else
		{
			m_bFreeTxBufSync = FALSE;
			m_pcsTxBufSync = pcsTxBufSync;
		}
	}
	else
	{
		m_pTxBuf = NULL;
		m_bFreeTxBufSync = FALSE;
		m_pcsTxBufSync = NULL;
	}

	if (pTxFifo == NULL)
	{
		m_bFreeTxFifo = TRUE;
		m_pTxFifo = new BUFQUEUE;
	}
	else
	{
		m_bFreeTxFifo = FALSE;
		m_pTxFifo = pTxFifo;
	}
	if (pcsTxFifoSync == NULL)
	{
		m_bFreeTxFifoSync = TRUE;
		m_pcsTxFifoSync = new CRITICAL_SECTION;
		::InitializeCriticalSection(m_pcsTxFifoSync);
	}
	else
	{
		m_bFreeTxFifoSync = FALSE;
		m_pcsTxFifoSync = pcsTxFifoSync;
	}

	// Init member vars
	m_pParseProcess = pParseProcess;
	m_hOwnerWnd = hOwnerWnd;
	m_lParam = lParam;
	m_sLocalAddress = sLocalAddress;
	m_uiLocalPort = uiLocalPort;
	m_sPeerAddress = sPeerAddress;
	m_uiPeerPort = uiPeerPort;
	m_hAcceptEvent = hAcceptEvent;
	m_hConnectEvent = hConnectEvent;
	m_hConnectFailedEvent = hConnectFailedEvent;
	m_hCloseEvent = hCloseEvent;
	m_hReadEvent = hReadEvent;
	m_hWriteEvent = hWriteEvent;
	m_hOOBEvent = hOOBEvent;
	m_lResetEventMask = lResetEventMask;
	m_lOwnerWndNetEvents = lOwnerWndNetEvents;
	m_uiRxMsgTrigger = uiRxMsgTrigger;
	m_hRxMsgTriggerEvent = hRxMsgTriggerEvent;
	m_uiMaxTxPacketSize = uiMaxTxPacketSize;
	m_uiRxPacketTimeout = uiRxPacketTimeout;
	m_uiTxPacketTimeout = uiTxPacketTimeout;
	m_pMsgOut = pMsgOut;
}

BOOL CNetCom::InitEvents()
{
	if (m_hSocket != INVALID_SOCKET)
	{
		if (::WSAEventSelect(m_hSocket, (WSAEVENT)m_hNetEvent,
						FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE) == SOCKET_ERROR)
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

void CNetCom::SetRxLogging(BOOL bLogging)
{
	m_bRxBufEnabled = bLogging;
}

void CNetCom::SetTxLogging(BOOL bLogging)
{
	m_bTxBufEnabled = bLogging;
}

void CNetCom::SetMaxTxPacketSize(UINT uiNewSize)
{
	// Limit the Maximum size of the sent packets
	if ((uiNewSize == 0) || (uiNewSize > NETCOM_MAX_TX_BUFFER_SIZE))
		uiNewSize = NETCOM_MAX_TX_BUFFER_SIZE;
	m_uiMaxTxPacketSize = uiNewSize;
}

BOOL CNetCom::IsRxLogging()
{
	return (m_pRxThread->IsRunning() && m_bRxBufEnabled);
}

BOOL CNetCom::IsTxLogging()
{
	return (m_pTxThread->IsRunning() && m_bTxBufEnabled);
}

void CNetCom::SetRxMsgTriggerSize(UINT uiNewSize)
{
	// Limit the Trigger Size
	if (uiNewSize > NETCOM_MAX_RX_BUFFER_SIZE)
		uiNewSize = NETCOM_MAX_RX_BUFFER_SIZE;
	m_uiRxMsgTrigger = uiNewSize;
}

UINT CNetCom::SetTxTimeout(UINT uiNewTimeout)
{
	if (uiNewTimeout == 0)
		uiNewTimeout = INFINITE;
	UINT uiOldTxTimeout = m_uiTxPacketTimeout;
	m_uiTxPacketTimeout = uiNewTimeout;
	::SetEvent(m_hTxTimeoutChangeEvent); // Release the Tx thread to change the timeout
	return uiOldTxTimeout;
}

UINT CNetCom::SetRxTimeout(UINT uiNewTimeout)
{
	if (uiNewTimeout == 0)
		uiNewTimeout = INFINITE;
	UINT uiOldRxTimeout = m_uiRxPacketTimeout;
	m_uiRxPacketTimeout = uiNewTimeout;
	::SetEvent(m_hRxTimeoutChangeEvent); // Release the Rx thread to change the timeout
	return uiOldRxTimeout;
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
#ifdef _UNICODE
		INT (WSAAPI *lpfGetAddrInfo)(PCWSTR pNodeName, PCWSTR pServiceName, const ADDRINFOW* pHints, PADDRINFOW* ppResult);
		VOID (WSAAPI *lpfFreeAddrInfo)(PADDRINFOW pAddrInfo);
		lpfGetAddrInfo = (INT(WSAAPI*)(PCWSTR, PCWSTR, const ADDRINFOW*, PADDRINFOW*))::GetProcAddress(hInstLib, "GetAddrInfoW");
		lpfFreeAddrInfo = (VOID(WSAAPI*)(PADDRINFOW))::GetProcAddress(hInstLib, "FreeAddrInfoW");
#else
		INT (WSAAPI *lpfGetAddrInfo)(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA* pHints, PADDRINFOA* ppResult);
		VOID (WSAAPI *lpfFreeAddrInfo)(PADDRINFOA pAddrInfo);
		lpfGetAddrInfo = (INT(WSAAPI*)(PCSTR, PCSTR, const ADDRINFOA*, PADDRINFOA*))::GetProcAddress(hInstLib, "getaddrinfo");
		lpfFreeAddrInfo = (VOID(WSAAPI*)(PADDRINFOA))::GetProcAddress(hInstLib, "freeaddrinfo");
#endif
		if (lpfGetAddrInfo && lpfFreeAddrInfo)
		{
			// Do not use ADDRINFOT because it is defined as addrinfo
			// if _WIN32_WINNT < 0x0502 in the atlsocket.h file!
#ifdef _UNICODE
			ADDRINFOW aiHints;
			ADDRINFOW* aiList = NULL;
#else
			ADDRINFOA aiHints;
			ADDRINFOA* aiList = NULL;
#endif
			memset(&aiHints, 0, sizeof(aiHints));
			aiHints.ai_family = nSocketFamily;
			aiHints.ai_flags = AI_PASSIVE;
			if (lpfGetAddrInfo(sHost, sPort, &aiHints, &aiList) == 0)
			{
				// Do not use ADDRINFOT because it is defined as addrinfo
				// if _WIN32_WINNT < 0x0502 in the atlsocket.h file!
#ifdef _UNICODE
				ADDRINFOW* walk;
#else
				ADDRINFOA* walk;
#endif
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
#ifdef _UNICODE
			size_t host_size = _tcslen(sHost);
			char* pHost = (char*)new char[host_size+1];
			wcstombs(pHost, sHost, host_size+1);
			// Check for Dotted IP Address String
			((sockaddr_in*)psockaddr)->sin_addr.S_un.S_addr = inet_addr(pHost);
#else
			// Check for Dotted IP Address String
			((sockaddr_in*)psockaddr)->sin_addr.S_un.S_addr = inet_addr(sHost);
#endif

			// If not a Dotted IP Address String try to resolve it as host name
			if ((((sockaddr_in*)psockaddr)->sin_addr.S_un.S_addr == INADDR_NONE) &&
				(_tcscmp(sHost, _T("255.255.255.255"))))
			{
#ifdef _UNICODE
				LPHOSTENT pHostent = ::gethostbyname(pHost);
#else
				LPHOSTENT pHostent = ::gethostbyname(sHost);
#endif

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
#ifdef _UNICODE
			delete [] pHost;
#endif
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