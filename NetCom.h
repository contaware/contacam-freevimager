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
	BOOL Init(		CParseProcess* pParseProcess,		// Parser & Processor
					CString sPeerAddress,				// Peer Address (IP or Host Name)
					UINT uiPeerPort,					// Peer Port
					HANDLE hConnectEvent,				// Handle to an Event Object that will get Connect Events.
					HANDLE hConnectFailedEvent,			// Handle to an Event Object that will get Connect Failed Events.
					HANDLE hCloseEvent,					// Handle to an Event Object that will get Close Events.
					HANDLE hReadEvent,					// Handle to an Event Object that will get Read Events.
					UINT uiRxMsgTrigger,				// The number of bytes that triggers an hRxMsgTriggerEvent 
														// (if hRxMsgTriggerEvent != NULL).
														// Upper bound for this value is NETCOM_MAX_RX_BUFFER_SIZE.
					HANDLE hRxMsgTriggerEvent,			// Handle to an Event Object that will get an Event
														// each time uiRxMsgTrigger bytes arrived.
					UINT uiRxPacketTimeout,				// After this timeout a Packet is returned
														// even if the uiRxMsgTrigger size is not reached (A zero meens INFINITE Timeout).
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

	// Get The RxFifo and the TxFifo
	__forceinline BUFQUEUE* GetRxFifo() const {return m_pRxFifo;};
	__forceinline BUFQUEUE* GetTxFifo() const {return m_pTxFifo;};

	// Get The RxFifo and the TxFifo Critical Sections
	__forceinline LPCRITICAL_SECTION GetRxFifoSync() const {return m_pcsRxFifoSync;};
	__forceinline LPCRITICAL_SECTION GetTxFifoSync() const {return m_pcsTxFifoSync;};
	
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

	// Name of the CNetCom Object Instance:
	// "NetCom Client"
	CString GetName();

	// Socket Family
	__forceinline int GetSocketFamily() {return m_nSocketFamily;};

	// Set the new Rx message trigger
	void SetRxMsgTriggerSize(UINT uiNewSize);
	
	// Return the Rx message trigger value
	__forceinline UINT GetRxMsgTriggerSize() const {return m_uiRxMsgTrigger;};

	// Set the new Rx packet timeout
	UINT SetRxTimeout(UINT uiNewTimeout);

	// Return the Rx packet timeout
	__forceinline UINT GetRxTimeout() const {return (m_uiRxPacketTimeout == INFINITE) ? 0 : m_uiRxPacketTimeout;};

	// Init() was called last time on
	CTime m_InitTime;

protected:
	// Init paddr from sAddress
	BOOL InitAddr(volatile int& nSocketFamily, const CString& sAddress, UINT uiPort, sockaddr* paddr);

	// Initialize All User Parameters (Parameters from Init Function)
	void InitVars(CParseProcess* pParseProcess,
				CString sPeerAddress,
				UINT uiPeerPort,
				HANDLE hConnectEvent,
				HANDLE hConnectFailedEvent,
				HANDLE hCloseEvent,
				HANDLE hReadEvent,
				UINT uiRxMsgTrigger,
				HANDLE hRxMsgTriggerEvent,
				UINT uiRxPacketTimeout,
				CMsgOut* pMsgOut);
	
	// Initialize the Network Events FD_READ, FD_CONNECT and FD_CLOSE
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

	// The number of bytes that triggers an m_hRxMsgTriggerEvent 
	// (if m_hRxMsgTriggerEvent != NULL).
	// Upper bound for this value is NETCOM_MAX_RX_BUFFER_SIZE.
	UINT m_uiRxMsgTrigger;

	// After this timeout a Packet is returned
	// even if the m_uiRxMsgTrigger size is not reached
	// (A zero meens INFINITE Timeout).
	UINT m_uiRxPacketTimeout;

	// Is the Client Connected?
	volatile BOOL m_bClientConnected;
	
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

	// Event Handles

	// Network Event (FD_READ, FD_CONNECT and FD_CLOSE)
	// Event is handled by the Message Thread
	WSAEVENT m_hNetEvent;

	// The Message Thread will send the Connect Events
	HANDLE m_hConnectEvent;

	// The Message Thread will send the Connect Events
	HANDLE m_hConnectFailedEvent;

	// The Message Thread will send the Close Events
	HANDLE m_hCloseEvent;

	// The Message Thread will send the Read Events
	HANDLE m_hReadEvent;

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

	// The Rx Timeout has changed
	HANDLE m_hRxTimeoutChangeEvent;

	// Event Arrays
	HANDLE m_hMsgEventArray[3];		// m_MsgThread.GetKillEvent() -> (highest priority)
									// m_hStartConnectionShutdownEvent
									// m_hNetEvent
	HANDLE m_hRxEventArray[4];		// m_RxThread.GetKillEvent() -> (highest priority)
									// m_ovRx.hEvent
									// m_hRxEvent
									// m_hRxTimeoutChangeEvent
	HANDLE m_hTxEventArray[3];		// m_TxThread.GetKillEvent() -> (highest priority)
									// m_ovTx.hEvent
									// m_hTxEvent

	// Rx Fifo and Tx Fifo Pointers
	BUFQUEUE* m_pRxFifo;
	BUFQUEUE* m_pTxFifo;

	// Synchronisation Objects
	LPCRITICAL_SECTION	m_pcsRxFifoSync;
	LPCRITICAL_SECTION	m_pcsTxFifoSync;

	// Message Output
	CMsgOut* m_pMsgOut;
	BOOL m_bFreeMsgOut;

	// Socket Family
	volatile int m_nSocketFamily;
};

#endif __NETCOM_H__
