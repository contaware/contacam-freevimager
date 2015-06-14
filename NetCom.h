#ifndef __NETCOM_H__
#define __NETCOM_H__

// Includes
#ifndef __AFXTEMPL_H__
#pragma message("To avoid this message, put afxtempl.h in your PCH (usually stdafx.h)")
#include <afxtempl.h>
#endif
#include <winsock2.h>
#include <Ws2tcpip.h>
#include "WorkerThread.h"

// Macros
#ifndef MAX
#define MAX(a,b)  ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

// Returns socket address structure size depending whether IP4 or IP6
#define SOCKADDRSIZE(pAddr)						(pAddr ? ((((sockaddr*)pAddr)->sa_family == AF_INET6) ? sizeof(sockaddr_in6) : sizeof(sockaddr_in)) : 0)

// Returns socket port
#define SOCKADDRPORT(pAddr)						(pAddr ? ((sockaddr_in*)pAddr)->sin_port : 0)

// Checks whether it is a any address
#define SOCKADDRANY(pAddr)						(pAddr ? ((((sockaddr*)pAddr)->sa_family == AF_INET6) ?\
												memcmp(&(((sockaddr_in6*)pAddr)->sin6_addr), &my_in6addr_any, sizeof(my_in6addr_any)) == 0 :\
												((sockaddr_in*)pAddr)->sin_addr.S_un.S_addr == INADDR_ANY) : 0)

// The safety margin for the processor linear buffer
#define PROCESSOR_BUFFER_PADDING_SIZE			32

// The Maximum RX Buffer Size
#define NETCOM_MAX_RX_BUFFER_SIZE				4096

// The Maximum TX Buffer Size
#define NETCOM_MAX_TX_BUFFER_SIZE				1400

// Timeout for waiting the FD_CLOSE replay if we started closing
#define NETCOM_CONNECTION_SHUTDOWN_TIMEOUT		3000U

// Timeout for letting the RX Thread finishing reading the data in the input queue
// (this is useful for http files download for example, in this case the peer
// closes the connection very fast)
#define NETCOM_PEER_CONNECTION_CLOSE_TIMEOUT	2000U

// Threads closing timeout, after this time the threads are forced to terminate
#define NETCOM_BLOCKING_TIMEOUT					15000U

////////////////////////////
// Event Masks From WinSock2
//
// #define FD_READ_BIT					0
// #define FD_READ						(1 << FD_READ_BIT)
// 
// #define FD_WRITE_BIT					1
// #define FD_WRITE						(1 << FD_WRITE_BIT)
//
// #define FD_OOB_BIT					2
// #define FD_OOB						(1 << FD_OOB_BIT)
//
// #define FD_ACCEPT_BIT				3
// #define FD_ACCEPT					(1 << FD_ACCEPT_BIT)
//
// #define FD_CONNECT_BIT				4
// #define FD_CONNECT					(1 << FD_CONNECT_BIT)
//
// #define FD_CLOSE_BIT					5
// #define FD_CLOSE						(1 << FD_CLOSE_BIT)
//
// #define FD_QOS_BIT					6
// #define FD_QOS						(1 << FD_QOS_BIT)
//
// #define FD_GROUP_QOS_BIT				7
// #define FD_GROUP_QOS					(1 << FD_GROUP_QOS_BIT)
//
// #define FD_ROUTING_INTERFACE_CHANGE_BIT 8
// #define FD_ROUTING_INTERFACE_CHANGE     (1 << FD_ROUTING_INTERFACE_CHANGE_BIT)
//
// #define FD_ADDRESS_LIST_CHANGE_BIT	9
// #define FD_ADDRESS_LIST_CHANGE		(1 << FD_ADDRESS_LIST_CHANGE_BIT)
//
// #define FD_MAX_EVENTS				10
// #define FD_ALL_EVENTS				((1 << FD_MAX_EVENTS) - 1)

#define FD_CONNECTFAILED_BIT			11
#define FD_CONNECTFAILED				(1 << FD_CONNECTFAILED_BIT)


// The Network Communication Class
class CNetCom
{		
public:
	// The Buffer Class
	class CBuf
	{
		public:
			CBuf();
			CBuf(unsigned int Size);
			virtual ~CBuf();
			CBuf(const CBuf& b); // Copy Constructor
			CBuf& operator=(const CBuf& b); // Copy Assignment
			__forceinline void SetMsgSize(unsigned int MsgSize) {m_MsgSize = MsgSize;};
			__forceinline unsigned int GetMsgSize() const {return m_MsgSize;};
			__forceinline unsigned int GetBufSize() const {return m_BufSize;};
			__forceinline char* GetBuf() const {return m_Buf;};
		private:
			char* m_Buf;
			unsigned int m_MsgSize;
			unsigned int m_BufSize;
	};

	// MFC Collection Class Typedef
	typedef CList<CBuf*,CBuf*> BUFQUEUE;

	// Output Message Class
	class CMsgOut
	{
		public:
			// Declare Friend Class
			friend class CNetCom;
			CMsgOut(){::InitializeCriticalSection(&m_csMessageOut);};
			virtual ~CMsgOut(){::DeleteCriticalSection(&m_csMessageOut);};
			enum MsgOutCode {CRITICAL_MSG, ERROR_MSG, WARNING_MSG, NOTICE_MSG, DEBUG_MSG};
			virtual void MessageOut(MsgOutCode code, const TCHAR* pMsg);
		protected:
			CRITICAL_SECTION m_csMessageOut;
	};

	// The Message Thread Class
	class CMsgThread : public CWorkerThread
	{
		public:
			CMsgThread(){m_bClosing = FALSE; m_pNetCom = NULL;};
			virtual ~CMsgThread(){Kill(NETCOM_BLOCKING_TIMEOUT);};
			__forceinline void SetNetComPointer(CNetCom* pNetCom) {m_pNetCom = pNetCom;};
			volatile BOOL m_bClosing; // About to Close

		protected:
			int Work();
			CNetCom* m_pNetCom;
	};

	// The Reception Thread Class
	class CRxThread : public CWorkerThread
	{
		public:
			CRxThread(){m_pNetCom = NULL; m_pCurrentBuf = NULL;};
			virtual ~CRxThread(){Kill(NETCOM_BLOCKING_TIMEOUT);};
			__forceinline void SetNetComPointer(CNetCom* pNetCom) {m_pNetCom = pNetCom;};

		protected:
			int Work();
			__forceinline void Read();
			CNetCom* m_pNetCom;
			CBuf* m_pCurrentBuf;
	};

	// The Transmission Thread Class
	class CTxThread : public CWorkerThread
	{
		public:
			CTxThread(){m_pNetCom = NULL;
						m_pCurrentBuf = NULL;
						m_nCurrentTxFifoSize = 0;};
			virtual ~CTxThread(){Kill(NETCOM_BLOCKING_TIMEOUT);};
			__forceinline void SetNetComPointer(CNetCom* pNetCom) {m_pNetCom = pNetCom;};
			
		protected:
			int Work();
			void Write();
			CNetCom* m_pNetCom;
			CBuf* m_pCurrentBuf;
			int m_nCurrentTxFifoSize;
	};

	// Base Parser Class
	class CParseProcess
	{
		friend class CNetCom;
		friend class CRxThread;
		public:
			CParseProcess(){m_pNetCom = NULL; m_nProcessOffset = m_nProcessSize = 0;};
			virtual ~CParseProcess(){;};
			virtual BOOL Parse(CNetCom* pNetCom, BOOL bLastCall) = 0;
			virtual BOOL Process(unsigned char* pLinBuf, int nSize);
			
		protected:
			void Init(CNetCom* pNetCom) {m_pNetCom = pNetCom;};
			void NewData(BOOL bLastCall);
			int m_nProcessOffset;
			int m_nProcessSize;
			CNetCom* m_pNetCom;
	};

	// Declare Friend Classes
	friend class CMsgThread;
	friend class CRxThread;
	friend class CTxThread;

	// Contruction and Destruction
	CNetCom();
	virtual	~CNetCom();

	// Host and Port to IP4 or IP6 address
	// Attention: if nSocketFamily is AF_UNSPEC or AF_INET6 pass a sockaddr_in6 structure pointer!
	static BOOL StringToAddress(const TCHAR* sHost, const TCHAR* sPort, sockaddr* psockaddr, int nSocketFamily = AF_UNSPEC);

	// Open a Network Connection								
	BOOL Init(		CParseProcess* pParseProcess,		// Parser & Processor
					CString sPeerAddress,				// Peer Address (IP or Host Name)
					UINT uiPeerPort,					// Peer Port
					HANDLE hConnectEvent,				// Handle to an Event Object that will get Connect Events
					HANDLE hConnectFailedEvent,			// Handle to an Event Object that will get Connect Failed Events
					HANDLE hReadEvent,					// Handle to an Event Object that will get Read Events
					CMsgOut* pMsgOut,					// Message Class for Debug, Notice, Warning, Error and Critical Visualization
					int nSocketFamily);					// Socket family

	// Close the Network Connection, this function is blocking
	void Close();

	// Start shutting down the connection,
	// eventually do something else and finally
	// use WaitTillShutdown_Blocking() to make
	// sure that we are done, or poll with the
	// IsShutdown() function, or just call Close()
	// (the destructor calls also Close() for us)
	void ShutdownConnection_NoBlocking();

	// Wait till all threads are dead
	__forceinline void WaitTillShutdown_Blocking() {
									if (!m_pTxThread->WaitDone_Blocking(NETCOM_BLOCKING_TIMEOUT))
									{
										if (m_pMsgOut)
											Critical(GetName() + _T(" TxThread has been forced to terminate by WaitTillShutdown_Blocking() after %u ms"), NETCOM_BLOCKING_TIMEOUT);
									}
									if (!m_pRxThread->WaitDone_Blocking(NETCOM_BLOCKING_TIMEOUT))
									{
										if (m_pMsgOut)
											Critical(GetName() + _T(" RxThread has been forced to terminate by WaitTillShutdown_Blocking() after %u ms"), NETCOM_BLOCKING_TIMEOUT);
									}
									if (!m_pMsgThread->WaitDone_Blocking(NETCOM_BLOCKING_TIMEOUT))
									{
										if (m_pMsgOut)
											Critical(GetName() + _T(" MsgThread has been forced to terminate by WaitTillShutdown_Blocking() after %u ms"), NETCOM_BLOCKING_TIMEOUT);
									}
								};

	// Is Shutdown?
	__forceinline BOOL IsShutdown() {return !m_pMsgThread->IsAlive()	&&
											!m_pRxThread->IsAlive()		&&
											!m_pTxThread->IsAlive();};

	// Return the Peer Socket IP
	CString GetPeerSockIP();

	// Start Communication Threads
	BOOL StartMsgThread();
	BOOL StartRxThread();
	BOOL StartTxThread();
	
	// Read Data from the Network
	int GetAvailableReadBytes(); // get the total available bytes count that can be read
	int Read(BYTE* Data = NULL, int BufSize = 0);	// Read() without params empties the RxFifo
													// Read() with NULL Data removes BufSize bytes 
													// Data must be preallocated with enough space if not NULL
													// Function returns the number of bytes read/removed
	__forceinline BUFQUEUE* GetRxFifo() {return &m_RxFifo;};					// instead of Read() we can access the fifo directly,
	__forceinline LPCRITICAL_SECTION GetRxFifoSync() {return &m_csRxFifoSync;};	// but in this case always use this critical section

	// Write Data to the Network (both functions return the number of written bytes)
	int Write(const BYTE* Data, int Size);	// If Size is bigger than NETCOM_MAX_TX_BUFFER_SIZE,
											// then the write is divided into multiple packets
	int WriteStr(LPCTSTR str);				// Write a NULL terminated string (the terminating NULL is not written to the net),
											// converting the given string with CStringA

	// Name of the CNetCom Object Instance:
	// "NetCom Client"
	CString GetName();

	// Socket Family
	__forceinline int GetSocketFamily() {return m_nSocketFamily;};

	// Init() was called last time on
	CTime m_InitTime;

protected:
	// Init paddr from sAddress and uiPort (this function updates nSocketFamily)
	BOOL InitAddr(volatile int& nSocketFamily, const CString& sAddress, UINT uiPort, sockaddr* paddr);

	// Shutdown Threads
	__forceinline void ShutdownMsgThread() {
										if (!m_pMsgThread->Kill(NETCOM_BLOCKING_TIMEOUT))
										{
											if (m_pMsgOut)
												Critical(GetName() + _T(" MsgThread has been forced to terminate by ShutdownMsgThread() after %u ms"), NETCOM_BLOCKING_TIMEOUT);
										}
									};
	__forceinline void ShutdownRxThread() {
										if (!m_pRxThread->Kill(NETCOM_BLOCKING_TIMEOUT))
										{
											if (m_pMsgOut)
												Critical(GetName() + _T(" RxThread has been forced to terminate by ShutdownRxThread() after %u ms"), NETCOM_BLOCKING_TIMEOUT);
										}
									};
	__forceinline void ShutdownTxThread() {
										if (!m_pTxThread->Kill(NETCOM_BLOCKING_TIMEOUT))
										{
											if (m_pMsgOut)
												Critical(GetName() + _T(" TxThread has been forced to terminate by ShutdownTxThread() after %u ms"), NETCOM_BLOCKING_TIMEOUT);
										}
									};

	// Critical, Error, Warning, Notice and Debug Functions
	void ProcessWSAError(const CString& sErrorText);
	void Critical(const TCHAR* pFormat, ...);
	void Error(const TCHAR* pFormat, ...);
	void Warning(const TCHAR* pFormat, ...);
	void Notice(const TCHAR* pFormat, ...);
	void Debug(const TCHAR* pFormat, ...);

	// The Parser & Processor
	CParseProcess* m_pParseProcess;

	// The Socket Handle
	SOCKET m_hSocket;
	
	// The Internet Address (IP or Host Name)
	CString m_sPeerAddress;

	// The Internet Port
	UINT m_uiPeerPort;

	// Threads
	CMsgThread* m_pMsgThread;
	CRxThread* m_pRxThread;
	CTxThread* m_pTxThread;

	// Overlapped Structures
	WSAOVERLAPPED m_ovRx;
	WSAOVERLAPPED m_ovTx;

	// Network Event (FD_READ, FD_CONNECT and FD_CLOSE)
	// Event is handled by the Message Thread
	WSAEVENT m_hNetEvent;

	// The Message Thread will send the Connect Events
	HANDLE m_hConnectEvent;

	// The Message Thread will send the Connect Failed Events
	HANDLE m_hConnectFailedEvent;

	// The Message Thread will send the Read Events
	HANDLE m_hReadEvent;

	// The Message Thread will send the Rx Event
	// when the FD_READ network event arrives
	// This Event triggers the RX Thread
	HANDLE m_hRxEvent;

	// The Write functions set this event,
	// which triggers the TX Thread
	HANDLE m_hTxEvent;

	// The m_hStartConnectionShutdownEvent is set by
	// ShutdownConnection_NoBlocking() function and handled
	// by the message thread
	HANDLE m_hStartConnectionShutdownEvent;

	// Event Arrays
	HANDLE m_hMsgEventArray[3];		// m_pMsgThread->GetKillEvent()
									// m_hStartConnectionShutdownEvent
									// m_hNetEvent
	HANDLE m_hRxEventArray[3];		// m_pRxThread->GetKillEvent()
									// m_ovRx.hEvent
									// m_hRxEvent
	HANDLE m_hTxEventArray[3];		// m_pTxThread->GetKillEvent()
									// m_ovTx.hEvent
									// m_hTxEvent

	// Rx Fifo and Tx Fifo
	BUFQUEUE m_RxFifo;
	BUFQUEUE m_TxFifo;
	CRITICAL_SECTION m_csRxFifoSync;
	CRITICAL_SECTION m_csTxFifoSync;

	// Message Output
	CMsgOut* m_pMsgOut;
	BOOL m_bFreeMsgOut;

	// Socket Family
	volatile int m_nSocketFamily;
};

#endif __NETCOM_H__
