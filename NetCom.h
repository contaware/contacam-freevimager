#ifndef __NETCOM_H__
#define __NETCOM_H__

#ifdef VIDEODEVICEDOC

/*
Is Winsock thread-safe?
-----------------------

On modern Windows stacks, yes, it is, within limits: 

It is safe, for instance, to have one thread calling send() and another 
thread calling recv() on a single socket. 

By contrast, it's a bad idea for two threads to both be calling send() 
on a single socket. This is “thread-safe” in the limited sense that 
your program shouldn’t crash, and you certainly shouldn't be able to 
crash the kernel, which is handling these send() calls. The fact that it 
is “safe” doesn’t answer key questions about the actual effect of 
doing this. Which call's data goes out first on the connection? Does 
it get interleaved somehow? 

Having multiple threads receiving data from a socket is usually fine for 
UDP socket, but doesn't make much sense for TCP sockets most of the 
time. 
*/

// Includes
#ifndef __AFXTEMPL_H__
#pragma message("To avoid this message, put afxtempl.h in your PCH (usually stdafx.h)")
#include <afxtempl.h> // for CList
#endif
#include <winsock2.h>
#include <Ws2tcpip.h>
#include "WorkerThread.h"
extern "C"
{
#include "libavutil/opt.h"
#include "libavutil/mathematics.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

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

// The Maximum RX Buffer Size
#define NETCOM_MAX_RX_BUFFER_SIZE				4096

// The Maximum TX Buffer Size
#define NETCOM_MAX_TX_BUFFER_SIZE				1400

// Timeout before closing the socket
#define NETCOM_CONNECTION_SHUTDOWN_TIMEOUT		3000U

// Threads closing timeout
#define NETCOM_BLOCKING_TIMEOUT					15000U

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
			CBuf(const CBuf& b);			// Copy Constructor
			CBuf& operator=(const CBuf& b);	// Copy Assignment
			__forceinline void SetMsgSize(unsigned int MsgSize) {m_MsgSize = MsgSize;};
			__forceinline unsigned int GetMsgSize() const {return m_MsgSize;};
			__forceinline unsigned int GetBufSize() const {return m_BufSize;};
			__forceinline char* GetBuf() const {return m_Buf;};
		private:
			char* m_Buf;
			unsigned int m_MsgSize;
			unsigned int m_BufSize;
	};

	// Typedef
	typedef CList<CBuf*,CBuf*> BUFQUEUE;

	// The Message Thread Class
	class CMsgThread : public CWorkerThread
	{
		public:
			CMsgThread(){m_pNetCom = NULL;};
			virtual ~CMsgThread(){Kill();};
			__forceinline void SetNetComPointer(CNetCom* pNetCom) {m_pNetCom = pNetCom;};

		protected:
			int Work();
			int CloseSocket();
			CNetCom* m_pNetCom;
	};

	// The Reception Thread Class
	class CRxThread : public CWorkerThread
	{
		public:
			CRxThread(){m_pNetCom = NULL; m_pCurrentBuf = NULL;};
			virtual ~CRxThread(){Kill();};
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
			virtual ~CTxThread(){Kill();};
			__forceinline void SetNetComPointer(CNetCom* pNetCom) {m_pNetCom = pNetCom;};
			
		protected:
			int Work();
			__forceinline void Write();
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
			virtual void OnThreadStart();								// executed when the thread that calls Parse/Process is started
			virtual void OnThreadShutdown();							// executed when the thread that calls Parse/Process is shutdown
			virtual BOOL Parse(CNetCom* pNetCom, BOOL bLastCall) = 0;
			virtual void Process(unsigned char* pLinBuf, int nSize);	// pLinBuf is a correctly aligned buffer ending
																		// with FF_INPUT_BUFFER_PADDING_SIZE zero bytes
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
	BOOL Init(	CParseProcess* pParseProcess,	// Parser & Processor
				CString sPeerAddress,			// Peer Address (IP or Host Name)
				UINT uiPeerPort,				// Peer Port
				HANDLE hConnectEvent,			// Handle to an Event Object that will get Connect Events
				HANDLE hConnectFailedEvent,		// Handle to an Event Object that will get Connect Failed Events
				HANDLE hReadEvent,				// Handle to an Event Object that will get Read Events
				int nSocketFamily);				// Socket family

	// Close the Network Connection (this function is blocking a maximum of 2 * NETCOM_BLOCKING_TIMEOUT ms)
	void Close();

	// Start shutting down the connection, eventually do something else and finally
	// poll IsShutdown() or call Close() to make sure the connection has terminated
	__forceinline void ShutdownConnection_NoBlocking() {::SetEvent(m_hStartConnectionShutdownEvent);};

	// Is connection shutdown?
	__forceinline BOOL IsShutdown() {return !m_pMsgThread->IsAlive()	&&
											!m_pRxThread->IsAlive()		&&
											!m_pTxThread->IsAlive();};

	// Return the Peer Socket IP
	CString GetPeerSockIP();
	
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

	// Name of the CNetCom Object Instance
	CString GetName() {CString s; s.Format(_T("Net_0x%08IX"), (size_t)this); return s;};

	// Socket Family
	__forceinline int GetSocketFamily() {return m_nSocketFamily;};

	// Init() was called last time on
	CTime m_InitTime;

protected:
	// Init paddr from sAddress and uiPort (this function updates nSocketFamily)
	static BOOL InitAddr(volatile int& nSocketFamily, const CString& sAddress, UINT uiPort, sockaddr* paddr);

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
	HANDLE m_hMsgEventArray[3];	// m_pMsgThread->GetKillEvent()
								// m_hStartConnectionShutdownEvent
								// m_hNetEvent
	HANDLE m_hRxEventArray[3];	// m_pRxThread->GetKillEvent()
								// m_ovRx.hEvent
								// m_hRxEvent
	HANDLE m_hTxEventArray[3];	// m_pTxThread->GetKillEvent()
								// m_ovTx.hEvent
								// m_hTxEvent

	// Rx Fifo and Tx Fifo
	BUFQUEUE m_RxFifo;
	BUFQUEUE m_TxFifo;
	CRITICAL_SECTION m_csRxFifoSync;
	CRITICAL_SECTION m_csTxFifoSync;

	// Socket Family
	volatile int m_nSocketFamily;
};

#endif
#endif __NETCOM_H__
