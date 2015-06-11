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
#define PROCESSOR_BUFFER_PADDING_SIZE			16

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

// Tx Sleep in ms
#define NETCOM_TX_SLEEP							10U

// Network Communication Messages that may be sent
// to the given Owner Window
#define WM_NETCOM_READ_EVENT			WM_USER + 16	// Notification of readiness for reading 
#define WM_NETCOM_WRITE_EVENT			WM_USER + 17	// Notification of readiness for writing 
#define WM_NETCOM_OOB_EVENT				WM_USER + 18	// Notification of the arrival of out-of-band data 
#define WM_NETCOM_ACCEPT_EVENT			WM_USER + 19	// Notification of incoming connections 
#define WM_NETCOM_CONNECT_EVENT			WM_USER + 20	// Notification of completed connection or multipoint "join" operation
#define WM_NETCOM_CONNECTFAILED_EVENT	WM_USER + 21	// Notification of connection failure
#define WM_NETCOM_CLOSE_EVENT			WM_USER + 22	// Notification of socket closure 
#define WM_NETCOM_RX					WM_USER + 23	// m_uiRxMsgTrigger characters were received and placed in the input buffer.
														// The last received char is transmitted in the WPARAM of the message.
														// (This may be useful if m_uiRxMsgTrigger is set to 1)
#define WM_NETCOM_RXBUF_ADD				WM_USER + 24	// This Message is sent when a packet is added to the RxBuf
#define WM_NETCOM_TXBUF_ADD				WM_USER + 25	// This Message is sent when a packet is added to the TxBuf

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

	// The Host Class
	class CHost
	{
		public:
			CHost(){;};
			CHost(CString sName, CString sFullName, CString sIP, in_addr IP){m_sName = sName; m_sFullName = sFullName; m_sIP = sIP; m_IP = IP;};
			virtual ~CHost(){;};
			CString m_sName;
			CString m_sFullName;
			CString m_sIP;
			in_addr m_IP;
	};

	// MFC Collection Class Typedefs
	typedef CArray<CBuf*,CBuf*> BUFARRAY;
	typedef CList<CBuf*,CBuf*> BUFQUEUE;
	typedef CArray<CNetCom*,CNetCom*> NETCOMVECTOR;
	typedef CArray<CString,CString&> STRINGVECTOR;

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
			void SignalClosing();
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
			__forceinline void Read(UINT BufSize);
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

	// Enumerate the LAN
	static DWORD EnumLAN(CStringArray* pHosts);

	// Open a Network Connection								
	BOOL Init(		HWND hOwnerWnd,						// The Optional Owner Window to which send the Network Events.
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

	// Get Parser & Processor
	__forceinline CParseProcess* GetParseProcess() const {return m_pParseProcess;};

	// Return the Peer Socket IP
	CString GetPeerSockIP();

	// Start Communication Threads
	BOOL StartMsgThread();
	BOOL StartRxThread();
	BOOL StartTxThread();

	// Get The Tx Event Handle.
	// This handle is used to trigger the TX Thread
	// (The Write function shows how to use this handle)
	__forceinline HANDLE GetTxEventHandle() const {return m_hTxEvent;};

	// Get The RxFifo, RxBuf, TxBuf and the TxFifo
	__forceinline BUFQUEUE* GetRxFifo() const {return m_pRxFifo;};
	__forceinline BUFARRAY* GetRxBuf() const {return m_pRxBuf;};
	__forceinline BUFARRAY* GetTxBuf() const {return m_pTxBuf;};
	__forceinline BUFQUEUE* GetTxFifo() const {return m_pTxFifo;};

	// Get The RxFifo, RxBuf, TxBuf and the TxFifo Critical Sections
	__forceinline LPCRITICAL_SECTION GetRxFifoSync() const {return m_pcsRxFifoSync;};
	__forceinline LPCRITICAL_SECTION GetRxBufSync() const {return m_pcsRxBufSync;};
	__forceinline LPCRITICAL_SECTION GetTxBufSync() const {return m_pcsTxBufSync;};
	__forceinline LPCRITICAL_SECTION GetTxFifoSync() const {return m_pcsTxFifoSync;};

	// Enter / Leave RxBuf Critical Section
	__forceinline void StartUsingRxBuf() const {::EnterCriticalSection(m_pcsRxBufSync);};
	__forceinline void StopUsingRxBuf() const {::LeaveCriticalSection(m_pcsRxBufSync);};

	// Enter / Leave RxFifo Critical Section
	__forceinline void StartUsingRxFifo() const {::EnterCriticalSection(m_pcsRxFifoSync);};
	__forceinline void StopUsingRxFifo() const {::LeaveCriticalSection(m_pcsRxFifoSync);};
	
	// Enter / Leave TxBuf Critical Section
	__forceinline void StartUsingTxBuf() const {::EnterCriticalSection(m_pcsTxBufSync);};
	__forceinline void StopUsingTxBuf() const {::LeaveCriticalSection(m_pcsTxBufSync);};

	// Enter / Leave TxFifo Critical Section
	__forceinline void StartUsingTxFifo() const {::EnterCriticalSection(m_pcsTxFifoSync);};
	__forceinline void StopUsingTxFifo() const {::LeaveCriticalSection(m_pcsTxFifoSync);};
	
	// Get the available bytes count that can be read with Read()
	int GetAvailableReadBytes();

	// Read Data from the Network
	int Read(BYTE* Data = NULL, int BufSize = 0); // returns the number of bytes read 

	// Write Data to the Network

	// This Splits in multiple packets if they are to big
	int Write(BYTE* Data, int Size);

	// Get the Head Buffer from the Rx Queue and remove it from the Rx Queue
	// (Remember to delete the CBuf Object!)
	CBuf* ReadHeadBuf(void);

	// Get the Head Buffer from the Rx Queue, do not remove it from the Rx Queue
	// (Remember to delete the CBuf Object!)
	CBuf* GetReadHeadBuf(void);

	// Remove the Head Buffer from the Rx Queue
	// (Remember to delete the CBuf Object!)
	BOOL RemoveReadHeadBuf(void);

	// Returns the position in the rx fifo of the given character
	int FindByte(BYTE b);

	// Read a Line from the Input Buffer.
	// If no '\n' or no '\r' is found, 0 is returned.
	// The '\n' and/or the '\r' are also returned in the buffer.
	int ReadLine(BYTE* Data, int MaxSize);

	// Read a CR + LF Line.
	// CR and LF are also returned.
	int ReadCRLFLine(BYTE* Data, int MaxSize);

	// Write a NULL Terminated String
	// (The terminating NULL is not written to the net)
	// if bEscapeSeq is TRUE the string is parsed with
	// the StrToByte (see below)
	int WriteStr(LPCTSTR str, BOOL bEscapeSeq = FALSE);

	// Is Client Connected ?
	__forceinline BOOL IsClientConnected() const {return m_bClientConnected;}; 

	// Config
	void SetMaxRxFifoSize(UINT uiMaxRxFifoSize) {m_uiMaxRxFifoSize = uiMaxRxFifoSize;};	// Bytes

	// Statistics
	__forceinline UINT GetRxByteCount() const {return m_uiRxByteCount;};
	__forceinline UINT GetTxByteCount() const {return m_uiTxByteCount;};
	__forceinline UINT GetRxFifoSize() const {return (m_pRxFifo ? m_pRxFifo->GetCount() : 0);};
	__forceinline UINT GetTxFifoSize() const {return (m_pTxFifo ? m_pTxFifo->GetCount() : 0);};
	__forceinline UINT GetRxBufSize() const {return (m_pRxBuf ? m_pRxBuf->GetSize() : 0);};
	__forceinline UINT GetTxBufSize() const {return (m_pTxBuf ? m_pTxBuf->GetSize() : 0);};

	// Name of the CNetCom Object Instance:
	// "NetCom Client"
	CString GetName();

	// Socket Family
	__forceinline int GetSocketFamily() {return m_nSocketFamily;};

	// Logging
	void SetRxLogging(BOOL bLogging);
	void SetTxLogging(BOOL bLogging);
	BOOL IsRxLoggingEnabled() const {return m_bRxBufEnabled;};
	BOOL IsTxLoggingEnabled() const {return m_bTxBufEnabled;};
	BOOL IsRxLogging();
	BOOL IsTxLogging();

	// Set the new max Tx packet size
	void SetMaxTxPacketSize(UINT uiNewSize);
	
	// Return the max Tx packet size value
	__forceinline UINT GetMaxTxPacketSize() const {return m_uiMaxTxPacketSize;};

	// Set the new Rx message trigger
	void SetRxMsgTriggerSize(UINT uiNewSize);
	
	// Return the Rx message trigger value
	__forceinline UINT GetRxMsgTriggerSize() const {return m_uiRxMsgTrigger;};

	// Set the new Tx packet timeout
	UINT SetTxTimeout(UINT uiNewTimeout);

	// Return the Tx packet timeout
	__forceinline UINT GetTxTimeout() const {return (m_uiTxPacketTimeout == INFINITE) ? 0 : m_uiTxPacketTimeout;};

	// Set the new Rx packet timeout
	UINT SetRxTimeout(UINT uiNewTimeout);

	// Return the Rx packet timeout
	__forceinline UINT GetRxTimeout() const {return (m_uiRxPacketTimeout == INFINITE) ? 0 : m_uiRxPacketTimeout;};

	// Set this variables before calling the Init Function!
	// If they have a value that differs from 0,
	// then a WM_COMMAND with the specified ID is posted
	// to the Parent Window.
	WPARAM m_nIDAccept;
	WPARAM m_nIDConnect;
	WPARAM m_nIDConnectFailed;
	WPARAM m_nIDRead;
	WPARAM m_nIDWrite;
	WPARAM m_nIDOOB;
	WPARAM m_nIDClose;
	WPARAM m_nIDRx;

	// The Owner Window
	HWND m_hOwnerWnd;

	// Init() was called last time on
	CTime m_InitTime;

protected:
	// Init paddr from sAddress
	BOOL InitAddr(volatile int& nSocketFamily, const CString& sAddress, UINT uiPort, sockaddr* paddr);

	// Initialize All User Parameters (Parameters from Init Function)
	void InitVars(HWND hOwnerWnd,
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
				CMsgOut* pMsgOut);
	
	// Initialize all Network Events (FD_ACCEPT, FD_CONNECT, ...)
	BOOL InitEvents();

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

	/*
	String to Byte, returns the new size.
	input: null terminated string
	output: bytes with no null termination
	
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
	int StrToByte(char* str);

	// The Parser & Processor
	CParseProcess* m_pParseProcess;

	// The Socket Handle
	SOCKET m_hSocket;

	// The lParam to send with the Messages
	LPARAM m_lParam;

	// The Network Events that are to be sent to the Owner Window
	long m_lOwnerWndNetEvents;

	// The number of bytes that triggers an m_hRxMsgTriggerEvent 
	// (if m_hRxMsgTriggerEvent != NULL).
	// And/Or the number of bytes that triggers a WM_NETCOM_RX Message
	// (if m_pOwnerWnd != NULL).
	// Upper bound for this value is NETCOM_MAX_RX_BUFFER_SIZE.
	UINT m_uiRxMsgTrigger;

	// Maximum Tx Packet Size, 
	// upper bound for this value is NETCOM_MAX_TX_BUFFER_SIZE.
	UINT m_uiMaxTxPacketSize;

	// After this timeout a Packet is returned
	// even if the m_uiRxMsgTrigger size is not reached
	// (A zero meens INFINITE Timeout).
	UINT m_uiRxPacketTimeout;

	// After this timeout a Packet is sent
	// even if no Write Event Happened (A zero meens INFINITE Timeout).
	UINT m_uiTxPacketTimeout;

	// Is the Client Connected?
	volatile BOOL m_bClientConnected;

	// Must this Class Instance free or not the Buffers, Fifos and the Critical Sections
	BOOL m_bFreeRxBufSync;
	BOOL m_bFreeRxFifo;
	BOOL m_bFreeRxFifoSync;
	BOOL m_bFreeTxBufSync;
	BOOL m_bFreeTxFifo;
	BOOL m_bFreeTxFifoSync;
	
	// The Internet Addresses (IPs or Host Names)
	CString m_sLocalAddress;
	CString m_sPeerAddress;

	// The Internet Ports
	UINT m_uiLocalPort;
	UINT m_uiPeerPort;

	// Threads
	CMsgThread* m_pMsgThread;
	CRxThread* m_pRxThread;
	CTxThread* m_pTxThread;

	// Overlapped Structures
	WSAOVERLAPPED m_ovRx;
	WSAOVERLAPPED m_ovTx;

	// A set value means that instead of setting an event it is reset
	long m_lResetEventMask;

	// Event Handles

	// Network Event (FD_ACCEPT, FD_CONNECT, FD_CLOSE, ...)
	// Event is handled by the Message Thread
	WSAEVENT m_hNetEvent;

	// The Message Thread will send the Accept Events
	HANDLE m_hAcceptEvent;

	// The Message Thread will send the Connect Events
	HANDLE m_hConnectEvent;

	// The Message Thread will send the Connect Events
	HANDLE m_hConnectFailedEvent;

	// The Message Thread will send the Close Events
	HANDLE m_hCloseEvent;

	// The Message Thread will send the Read Events
	HANDLE m_hReadEvent;

	// The Message Thread will send the Write Events
	HANDLE m_hWriteEvent;
	
	// The Message Thread will send the OOB Events
	HANDLE m_hOOBEvent;

	// The Message Thread will send the Rx Event
	// when the FD_READ network event arrives
	// This Event triggers the RX Thread
	HANDLE m_hRxEvent;

	// The Rx Thread will set this event each time m_uiRxMsgTrigger
	// bytes have been received
	HANDLE m_hRxMsgTriggerEvent;

	// A User or the Write function set this event
	// The Event triggers the TX Thread
	HANDLE m_hTxEvent;

	// The m_hStartConnectionShutdownEvent is set by
	// ShutdownConnection_NoBlocking() function and handled
	// by the message thread
	HANDLE m_hStartConnectionShutdownEvent;
	
	// The Tx Timeout has changed
	HANDLE m_hTxTimeoutChangeEvent;

	// The Rx Timeout has changed
	HANDLE m_hRxTimeoutChangeEvent;

	// Event Arrays
	HANDLE m_hMsgEventArray[3];		// m_MsgThread.GetKillEvent() -> (highest priority)
									// m_hStartConnectionShutdownEvent
									// m_hNetEvent
	HANDLE m_hRxEventArray[4];		// m_RxThread.GetKillEvent() -> (highest priority)
									// m_hRxEvent
									// m_ovRx.hEvent
									// m_hRxTimeoutChangeEvent
	HANDLE m_hTxEventArray[4];		// m_TxThread.GetKillEvent() -> (highest priority)
									// m_hWriteEvent
									// m_ovTx.hEvent
									// m_hTxTimeoutChangeEvent

	// Rx Buf, Rx Fifo, Tx Buf and Tx Fifo Pointers
	BUFARRAY* m_pRxBuf;
	BUFQUEUE* m_pRxFifo;
	BUFARRAY* m_pTxBuf;
	BUFQUEUE* m_pTxFifo;

	// Synchronisation Objects
	LPCRITICAL_SECTION	m_pcsRxBufSync;
	LPCRITICAL_SECTION	m_pcsRxFifoSync;
	LPCRITICAL_SECTION	m_pcsTxBufSync;
	LPCRITICAL_SECTION	m_pcsTxFifoSync;

	// Message Output
	CMsgOut* m_pMsgOut;
	BOOL m_bFreeMsgOut;

	// Statistics
	volatile UINT m_uiRxByteCount;
	volatile UINT m_uiTxByteCount;

	// Enable / Disable use of the Rx and Tx Buffers
	BOOL m_bRxBufEnabled;
	BOOL m_bTxBufEnabled;

	// Max Rx Fifo Size
	UINT m_uiMaxRxFifoSize;

	// Socket Family
	volatile int m_nSocketFamily;
};

#endif __NETCOM_H__
