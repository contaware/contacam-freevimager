#if !defined(AFX_NETFRAMEHDR_H__94956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_)
#define AFX_NETFRAMEHDR_H__94956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DEFAULT_UDP_PORT				8800

#include "NetCom.h"

//
// Networking Frame Header Type
//
// Note:
// NETFRAME_TYPE_FRAME_REQ And NETFRAME_TYPE_FRAME_CONF may be combined!
// 
#define NETFRAME_TYPE_FRAME_REQ			0x01		// Client Polls For Frames, telling the server that it is alive
#define NETFRAME_TYPE_FRAME_ANS			0x02		// Server Sends Frame Fragment, sets current Up-Time
#define NETFRAME_TYPE_FRAME_CONF		0x04		// Client Received Frame, it replies Up-Time of frame
#define NETFRAME_TYPE_FRAME_RESEND		0x08		// Client is missing a Frame, it requests the frame by the sequence number
#define NETFRAME_TYPE_PING_REQ			0x10		// Ping Request
#define NETFRAME_TYPE_PING_ANS			0x20		// Ping reply, up-time of request packet is sent back
#define NETFRAME_TYPE_FRAMES_LOST		0x40		// Client informs the Server that the given count frames never arrived
													// (There is no re-send for them, client seeks to next keyframe)
#define NETFRAME_TYPE_FRAME_REQ_AUTH	0x80		// Authentication

//
// Networking Frame Header Flags
//
#define NETFRAME_FLAG_VIDEO				0x01		// This is a video fragment
#define NETFRAME_FLAG_AUDIO				0x02		// This is an audio fragment
#define NETFRAME_FLAG_RES0				0x04		// Reserved
#define NETFRAME_FLAG_RES1				0x08		// ...
#define NETFRAME_FLAG_RES2				0x10
#define NETFRAME_FLAG_RES3				0x20
#define NETFRAME_FLAG_AUTH_FAILED		0x40		// Last supplied username + password was wrong
#define NETFRAME_FLAG_KEYFRAME			0x80		// This is a key-frame fragment

// Max Ping RT Time
#define	NETFRAME_MAX_PING_RT			3000U		// ms

// Max defined UDP Fragments per frame
#define NETFRAME_MAX_FRAGMENTS			1024U

// Number of frames to queue
#define NETFRAME_MAX_FRAMES				256U
#define NETFRAME_MIN_FRAMES				8U
#define NETFRAME_DEFAULT_FRAMES			32U

// Resend size is 4 x NETFRAME_MAX_FRAMES
#define NETFRAME_RESEND_ARRAY_SIZE		1024
#define NETFRAME_RESEND_ARRAY_MASK		0x3FF

// 10 Mbps constant
#define NETFRAME_10MBPS_BANDWIDTH		1250000		// bytes / sec

// Frame Times
#define NETFRAME_MIN_FRAME_TIME			30U			// ms
#define NETFRAME_MAX_FRAME_TIME			30000U		// ms
#define NETFRAME_DEFAULT_FRAME_TIME		200U		// ms

//
// Networking Frame Headers, minimum 12 bytes long
//

// Frame Data Fragment Header
struct NetFrameHdrStruct
{
	DWORD dwUpTime;
	WORD FragmentNum;		// 0 .. (TotalFragments - 1)
	WORD TotalFragments;	// Max is NETFRAME_MAX_FRAGMENTS
	WORD wSeq;				// Inc. with each new frame
	BYTE Type;
	BYTE Flags;
};

// Frame Confirmation or Re-Send Header
struct NetFrameHdrStructConf
{
	DWORD dwUpTime;
	DWORD dwFrameSize;
	WORD wSeq;
	BYTE Type;
	BYTE Flags;
};

// Ping Header or Authentication Header
struct NetFrameHdrPingAuth
{
	DWORD dwUpTime;
	DWORD dwExtraSize;
	WORD wSeq;
	BYTE Type;
	BYTE Flags;
};

// Frames Lost Information Header
struct NetFrameHdrStructFramesLost
{
	DWORD dwUpTime;
	WORD wFirstLostSeq;
	WORD wLastLostSeq;
	WORD wLostCount; // wLastLostSeq - wFirstLostSeq + 1 
	BYTE Type;
	BYTE Flags;
};

__forceinline BOOL SendUDPPing(CNetCom* pNetCom, DWORD dwForceSize)
{
	// Header
	NetFrameHdrPingAuth PingHdr;
	PingHdr.wSeq = 0;
	PingHdr.Type = NETFRAME_TYPE_PING_REQ;
	PingHdr.Flags = 0;

	// Send
	if (dwForceSize <= sizeof(PingHdr))
	{
		PingHdr.dwExtraSize = 0;
		return (pNetCom->WriteDatagram(	(LPBYTE)&PingHdr,
										sizeof(NetFrameHdrPingAuth),
										NULL,
										0,
										TRUE) > 0);
	}
	else
	{
		PingHdr.dwExtraSize = dwForceSize - sizeof(PingHdr);
		LPBYTE buf = new BYTE[PingHdr.dwExtraSize];
		memset(buf, 0x80, PingHdr.dwExtraSize);
		BOOL res = pNetCom->WriteDatagram(	(LPBYTE)&PingHdr,
											sizeof(NetFrameHdrPingAuth),
											buf,
											PingHdr.dwExtraSize,
											TRUE) > 0;
		delete [] buf;
		return res;
	}
}

__forceinline BOOL SendUDPPingTo(CNetCom* pNetCom, sockaddr_in* pAddr, DWORD dwForceSize)
{
	// Header
	NetFrameHdrPingAuth PingHdr;
	PingHdr.wSeq = 0;
	PingHdr.Type = NETFRAME_TYPE_PING_REQ;
	PingHdr.Flags = 0;

	// Send
	if (dwForceSize <= sizeof(PingHdr))
	{
		PingHdr.dwExtraSize = 0;
		return (pNetCom->WriteDatagramTo(pAddr,
										(LPBYTE)&PingHdr,
										sizeof(NetFrameHdrPingAuth),
										NULL,
										0,
										TRUE) > 0);
	}
	else
	{
		PingHdr.dwExtraSize = dwForceSize - sizeof(PingHdr);
		LPBYTE buf = new BYTE[PingHdr.dwExtraSize];
		memset(buf, 0x80, PingHdr.dwExtraSize);
		BOOL res = pNetCom->WriteDatagramTo(pAddr,
											(LPBYTE)&PingHdr,
											sizeof(NetFrameHdrPingAuth),
											buf,
											PingHdr.dwExtraSize,
											TRUE) > 0;
		delete [] buf;
		return res;
	}
}

#endif // !defined(AFX_NETFRAMEHDR_H__94956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_)
