#include "stdafx.h"
#include "NetGetFrame.h"
#include "RemoteCamCtl.h"
#include "PJNMD5.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL CGetFrameGenerator::Generate(CNetCom* pNetCom)
{
	// Header
	NetFrameHdrPingAuth Hdr;
	Hdr.dwUpTime = ::timeGetTime();
	Hdr.wSeq = 0;
	Hdr.dwExtraSize = 0;
	Hdr.Type = NETFRAME_TYPE_FRAME_REQ | NETFRAME_TYPE_PING_REQ;
	Hdr.Flags = NETFRAME_FLAG_VIDEO;
	
	// Send
	pNetCom->WriteDatagram((LPBYTE)&Hdr,
							sizeof(NetFrameHdrStruct),
							NULL,
							0,
							TRUE);

	return TRUE; // Call the Generator again with the next tx timeout
}

__forceinline BYTE CGetFrameParseProcess::ReSendCountDown(	int nReSendCount,
															int nCount,
															int nCountOffset)
{
	int i;
	switch (nReSendCount)
	{
		case 0 : i = 2;   break;
		case 1 : i = 8;   break;
		case 2 : i = 34;  break;
		case 3 : i = 144; break;
		default: return 255;
	}
	int z;
	switch (m_pCtrl->GetMaxFrames())
	{
		case 8 :   z = 0; break;
		case 16 :  z = 1; break;
		case 32 :  z = 2; break;
		case 64 :  z = 3; break;
		case 96 :  z = 4; break;
		case 128 : z = 5; break;
		case 192 : z = 6; break;
		default:   z = 7; break;
	}
	return (BYTE)MIN(255, i + nCount + MAX(nCountOffset, z));
}

BOOL CGetFrameParseProcess::Parse(CNetCom* pNetCom)
{
	ASSERT(m_pCtrl);
	DWORD dwFrame;
	DWORD dwTimeDiff;
	DWORD dwMaxFragmentAge = m_pCtrl->GetMaxFrames() * 1000U; // ms

	// The Received Datagram
	CNetCom::CBuf* pBuf = pNetCom->GetReadHeadBuf();
	
	// Check Packet Family
	if (pBuf->GetAddrPtr()->sin_family != AF_INET)
	{
		pNetCom->RemoveReadHeadBuf();
		delete pBuf;
		return FALSE;
	}

	// Check Datagram Size
	ASSERT(pBuf->GetMsgSize() <= NETCOM_MAX_TX_BUFFER_SIZE);

	// Get Header
	NetFrameHdrStruct Hdr;
	if (pBuf->GetMsgSize() < sizeof(NetFrameHdrStruct))
	{
		pNetCom->RemoveReadHeadBuf();
		delete pBuf;
		return FALSE;
	}
	memcpy(&Hdr, pBuf->GetBuf(), sizeof(NetFrameHdrStruct));

	// Get Current Up-Time
	DWORD dwCurrentUpTime = ::timeGetTime();

	// Ping Request?
	if (Hdr.Type & NETFRAME_TYPE_PING_REQ)
	{
		// Header
		NetFrameHdrPingAuth* pReqPingHdr = (NetFrameHdrPingAuth*)&Hdr;
		NetFrameHdrPingAuth AnsPingHdr;
		AnsPingHdr.dwUpTime = pReqPingHdr->dwUpTime;
		AnsPingHdr.wSeq = pReqPingHdr->wSeq;
		AnsPingHdr.dwExtraSize = pReqPingHdr->dwExtraSize;
		AnsPingHdr.Type = NETFRAME_TYPE_PING_ANS;
		AnsPingHdr.Flags = 0;

		// Send
		pNetCom->WriteDatagram((LPBYTE)&AnsPingHdr,
								sizeof(NetFrameHdrPingAuth),
								NULL,
								0,
								TRUE);

		pNetCom->RemoveReadHeadBuf();
		delete pBuf;
		return FALSE;
	}
	// Ping Answer?
	else if (Hdr.Type & NETFRAME_TYPE_PING_ANS)
	{
		// Header
		NetFrameHdrPingAuth* pAnsPingHdr = (NetFrameHdrPingAuth*)&Hdr;
		if (m_dwPingRT == 0)
		{
			dwTimeDiff = dwCurrentUpTime - pAnsPingHdr->dwUpTime;
			if (dwTimeDiff <= NETFRAME_MAX_PING_RT)
				m_dwPingRT = MAX(1U, dwTimeDiff);
			else
				m_dwPingRT = NETFRAME_MAX_PING_RT;
		}
		TRACE(_T("Ping RT : %ums\n"), m_dwPingRT);
		pNetCom->RemoveReadHeadBuf();
		delete pBuf;
		return FALSE;
	}
	// Authentication Request?
	else if (Hdr.Type & NETFRAME_TYPE_FRAME_REQ_AUTH)
	{
		// Header
		NetFrameHdrPingAuth* pReqAuthHdr = (NetFrameHdrPingAuth*)&Hdr;
		NetFrameHdrPingAuth AuthHrd;
		AuthHrd.dwUpTime = ::timeGetTime();
		AuthHrd.wSeq = pReqAuthHdr->wSeq;
		AuthHrd.dwExtraSize = 16;
		AuthHrd.Type = pReqAuthHdr->Type;
		AuthHrd.Flags = pReqAuthHdr->Flags;

		// Calc. hash and send it back
		USES_CONVERSION;
		CPJNMD5 hmac;
		CPJNMD5Hash calc_hash;
		CString sNonce, sCNonce, sToHash;
		sNonce.Format(_T("%08x"), pReqAuthHdr->dwUpTime);
		sCNonce.Format(_T("%08x"), AuthHrd.dwUpTime);
		sToHash = m_pCtrl->GetFrameUsername() + sNonce + m_pCtrl->GetFramePassword() + sCNonce;
		char* psz = T2A(const_cast<LPTSTR>(sToHash.operator LPCTSTR()));
		if (hmac.Hash((const BYTE*)psz, (DWORD)strlen(psz), calc_hash))
		{
			pNetCom->WriteDatagram(	(LPBYTE)&AuthHrd,
									sizeof(NetFrameHdrPingAuth),
									calc_hash.m_byHash,
									16,
									TRUE);
		}
		pNetCom->RemoveReadHeadBuf();
		delete pBuf;
		return FALSE;
	}

	// Check Header Type and Flag
	if (!(Hdr.Type & NETFRAME_TYPE_FRAME_ANS) ||
		!(Hdr.Flags & NETFRAME_FLAG_VIDEO))
	{
		pNetCom->RemoveReadHeadBuf();
		delete pBuf;
		return FALSE;
	}

	// Server stopped and restarted?
	dwTimeDiff = Hdr.dwUpTime - m_dwLastFrameUpTime;
	if (dwTimeDiff > dwMaxFragmentAge && dwTimeDiff < 0x80000000U)
		m_bInitialized = FALSE;

	// If First Frame
	if (!m_bInitialized)
	{
		m_nTotalFragments[0] = Hdr.TotalFragments;
		m_wPrevSeq = m_wFrameSeq[0] = Hdr.wSeq;
		m_wPrevSeq = m_wPrevSeq - 1U;
		for (dwFrame = 0U ; dwFrame < m_pCtrl->GetMaxFrames() ; dwFrame++)
		{
			m_dwUpTime[dwFrame] = Hdr.dwUpTime;
			m_dwFrameSize[dwFrame] = 0U;
			m_bKeyFrame[dwFrame] = FALSE;
		}
		for (dwFrame = 1U ; dwFrame < m_pCtrl->GetMaxFrames() ; dwFrame++)
		{
			m_nTotalFragments[dwFrame] = 0;
			m_wFrameSeq[dwFrame] = 0U;
			m_dwUpTime[dwFrame] -= dwFrame;
		}
		memset(&m_ReSendCount, 0, NETFRAME_RESEND_ARRAY_SIZE);
		memset(&m_ReSendCountDown, 0, NETFRAME_RESEND_ARRAY_SIZE);
		m_bInitialized = TRUE;
		m_bFirstFrame = TRUE;
		m_bSeekToKeyFrame = TRUE;
		m_dwLastFrameUpTime = Hdr.dwUpTime;
		m_dwLastReSendUpTime = dwCurrentUpTime;
		m_dwLastPresentationUpTime = dwCurrentUpTime;
	}

	// Clean-up really old fragments
	for (dwFrame = 0U ; dwFrame < m_pCtrl->GetMaxFrames() ; dwFrame++)
	{
		dwTimeDiff = Hdr.dwUpTime - m_dwUpTime[dwFrame];
		if (dwTimeDiff > dwMaxFragmentAge && dwTimeDiff < 0x80000000U)
		{
			FreeFrameFragments(dwFrame);
			m_bKeyFrame[dwFrame] = FALSE;
			m_nTotalFragments[dwFrame] = 0;
			m_dwUpTime[dwFrame] = 0U;
			m_wFrameSeq[dwFrame] = 0U;
			m_dwFrameSize[dwFrame] = 0U;
		}
	}

	// Check whether fragment is part of a known frame
	BOOL bFragmentAdded = FALSE;
	for (dwFrame = 0U ; dwFrame < m_pCtrl->GetMaxFrames() ; dwFrame++)
	{
		if (m_wFrameSeq[dwFrame] == Hdr.wSeq)
		{
			// Set Vars
			bFragmentAdded = TRUE;
			if (m_Fragment[dwFrame][Hdr.FragmentNum])
				delete m_Fragment[dwFrame][Hdr.FragmentNum];
			m_Fragment[dwFrame][Hdr.FragmentNum] = pBuf;
			pNetCom->RemoveReadHeadBuf();
			m_bKeyFrame[dwFrame] = (Hdr.Flags & NETFRAME_FLAG_KEYFRAME) > 0 ? TRUE : FALSE;
			m_nTotalFragments[dwFrame] = Hdr.TotalFragments;
			m_dwUpTime[dwFrame] = Hdr.dwUpTime;
			if (GetReceivedFragmentsCount(dwFrame) == m_nTotalFragments[dwFrame])
			{
				m_dwFrameSize[dwFrame] = CalcFrameSize(dwFrame);
				SendConfirmation(pNetCom, Hdr.dwUpTime, m_dwFrameSize[dwFrame], Hdr.wSeq,
								(Hdr.wSeq & NETFRAME_FLAG_KEYFRAME) == NETFRAME_FLAG_KEYFRAME);
			}
			else
				m_dwFrameSize[dwFrame] = 0U;
			break;
		}
	}

	// Fragment is not part of a known frame
	if (!bFragmentAdded)
	{
		// Find empty space
		for (dwFrame = 0U ; dwFrame < m_pCtrl->GetMaxFrames() ; dwFrame++)
		{
			if (m_nTotalFragments[dwFrame] == 0)
			{
				// Set Vars
				bFragmentAdded = TRUE;
				if (m_Fragment[dwFrame][Hdr.FragmentNum])
					delete m_Fragment[dwFrame][Hdr.FragmentNum];
				m_Fragment[dwFrame][Hdr.FragmentNum] = pBuf;
				pNetCom->RemoveReadHeadBuf();
				m_bKeyFrame[dwFrame] = (Hdr.Flags & NETFRAME_FLAG_KEYFRAME) > 0 ? TRUE : FALSE;
				m_nTotalFragments[dwFrame] = Hdr.TotalFragments;
				m_dwUpTime[dwFrame] = Hdr.dwUpTime;
				m_wFrameSeq[dwFrame] = Hdr.wSeq;
				if (m_nTotalFragments[dwFrame] == 1)
				{
					m_dwFrameSize[dwFrame] = CalcFrameSize(dwFrame);
					SendConfirmation(pNetCom, Hdr.dwUpTime, m_dwFrameSize[dwFrame], Hdr.wSeq,
									(Hdr.wSeq & NETFRAME_FLAG_KEYFRAME) == NETFRAME_FLAG_KEYFRAME);
				}
				else
					m_dwFrameSize[dwFrame] = 0U;
				break;
			}
		}

		// Throw oldest because we could not find an empty space 
		if (!bFragmentAdded)
		{
			// Find oldest place
			WORD wOldestSeq = m_wFrameSeq[0];
			DWORD dwOldestIndex = 0U;
			for (dwFrame = 1U ; dwFrame < m_pCtrl->GetMaxFrames() ; dwFrame++)
			{
				if ((WORD)(m_wFrameSeq[dwFrame] - wOldestSeq) >= 0x8000U)
				{
					wOldestSeq = m_wFrameSeq[dwFrame];
					dwOldestIndex = dwFrame;
				}
			}

			// Current fragment is newer than oldest?
			if ((WORD)(Hdr.wSeq - wOldestSeq) < 0x8000U) 
			{
				// Free
				FreeFrameFragments(dwOldestIndex);
				
				// Set Vars
				bFragmentAdded = TRUE;
				if (m_Fragment[dwOldestIndex][Hdr.FragmentNum])
					delete m_Fragment[dwOldestIndex][Hdr.FragmentNum];
				m_Fragment[dwOldestIndex][Hdr.FragmentNum] = pBuf;
				pNetCom->RemoveReadHeadBuf();
				m_bKeyFrame[dwOldestIndex] = (Hdr.Flags & NETFRAME_FLAG_KEYFRAME) > 0 ? TRUE : FALSE;
				m_nTotalFragments[dwOldestIndex] = Hdr.TotalFragments;
				m_dwUpTime[dwOldestIndex] = Hdr.dwUpTime;
				m_wFrameSeq[dwOldestIndex] = Hdr.wSeq;
				if (m_nTotalFragments[dwOldestIndex] == 1)
				{
					m_dwFrameSize[dwOldestIndex] = CalcFrameSize(dwOldestIndex);
					SendConfirmation(pNetCom, Hdr.dwUpTime, m_dwFrameSize[dwOldestIndex], Hdr.wSeq,
									(Hdr.wSeq & NETFRAME_FLAG_KEYFRAME) == NETFRAME_FLAG_KEYFRAME);
				}
				else
					m_dwFrameSize[dwOldestIndex] = 0U;
			}
			else
			{
				pNetCom->RemoveReadHeadBuf();
				delete pBuf;
			}
		}
	}

	// Calc. ready frames count
	DWORD dwReadyFramesCount = 0U;
	for (dwFrame = 0U ; dwFrame < m_pCtrl->GetMaxFrames() ; dwFrame++)
	{
		if (m_dwFrameSize[dwFrame] > 0U)
			++dwReadyFramesCount;
	}

	// Frame Presentation
	DWORD dwFrameIndex;
	BOOL bFrameReady = FALSE;
	DWORD dwUpTimeDiff = dwCurrentUpTime - m_dwLastPresentationUpTime;
	DWORD dwUnit = m_pCtrl->GetMaxFrames() / NETFRAME_MIN_FRAMES;
	DWORD dwMaxFrames2 = m_pCtrl->GetMaxFrames() / 2U;
	if ((dwReadyFramesCount >= dwMaxFrames2 + 3U * dwUnit)
																		||
		(dwReadyFramesCount >= dwMaxFrames2 + dwUnit					&&
		dwUpTimeDiff >= m_dwAvgFrameTime / 2U)
																		||
		(dwReadyFramesCount >= dwMaxFrames2								&&
		dwUpTimeDiff >= 3U * m_dwAvgFrameTime / 4U)
																		||
		(dwReadyFramesCount >= dwMaxFrames2 - dwUnit					&&
		dwUpTimeDiff >= 5U * m_dwAvgFrameTime / 4U)						
																		||
		(dwReadyFramesCount >= dwMaxFrames2 - 3U * dwUnit				&&
		dwUpTimeDiff >= 2U * m_dwAvgFrameTime))
	{
		// Find successive frame in the frame seq.
		BOOL bNextFrameIncomplete = FALSE;
		for (dwFrame = 0U ; dwFrame < m_pCtrl->GetMaxFrames() ; dwFrame++)
		{
			if ((WORD)(m_wFrameSeq[dwFrame] - m_wPrevSeq) == 1U)
			{
				dwFrameIndex = dwFrame;
				if (m_dwFrameSize[dwFrame] > 0U)
				{
					bFrameReady = TRUE;
					dwFrameIndex = dwFrame;
					bNextFrameIncomplete = FALSE;
					break;
				}
				else
					bNextFrameIncomplete = TRUE;
			}
		}
		
		// No frame with the right sequence has been found
		// (the wanted one has been dropped)
		// -> take the next one and enable seek to key frame
		if (!bFrameReady)
		{
			// Warn
#ifdef _DEBUG
			if (bNextFrameIncomplete)
			{
				if (m_bKeyFrame[dwFrameIndex])
				{
					TRACE(_T("Seeking to next key-frame because we found an incomplete key-frame %u (available %d fragments of %d)\n"),
															m_wFrameSeq[dwFrameIndex],
															GetReceivedFragmentsCount(dwFrameIndex),
															m_nTotalFragments[dwFrameIndex]);
				}
				else
				{
					TRACE(_T("Seeking to next frame because we found an incomplete frame %u (available %d fragments of %d)\n"),
															m_wFrameSeq[dwFrameIndex],
															GetReceivedFragmentsCount(dwFrameIndex),
															m_nTotalFragments[dwFrameIndex]);
				}
				TraceIncompleteFrame(dwFrameIndex);
			}
#endif

			WORD wSeqDiff = 0x8000U;
			for (dwFrame = 0U ; dwFrame < m_pCtrl->GetMaxFrames() ; dwFrame++)
			{
				if (m_dwFrameSize[dwFrame] > 0U)
				{
					if ((WORD)(m_wFrameSeq[dwFrame] - m_wPrevSeq) < wSeqDiff)
					{
						bFrameReady = TRUE;
						m_bSeekToKeyFrame = TRUE;
						dwFrameIndex = dwFrame;
						wSeqDiff = m_wFrameSeq[dwFrame] - m_wPrevSeq;
					}
				}
			}	
		}
	}

	// Re-Send?
	if (!m_pCtrl->DoDisableResend())
	{
		if (dwCurrentUpTime - m_dwLastReSendUpTime > 3U * m_dwAvgFrameTime / 2U)
		{
			WORD wHighestReadySeqDiff = 0U;
			for (dwFrame = 0U ; dwFrame < m_pCtrl->GetMaxFrames() ; dwFrame++)
			{
				if (m_dwFrameSize[dwFrame] > 0U)
				{
					WORD wSeqDiff = m_wFrameSeq[dwFrame] - m_wPrevSeq;
					if (wSeqDiff > wHighestReadySeqDiff && wSeqDiff < 0x8000U)
						wHighestReadySeqDiff = wSeqDiff;
				}
			}
			WORD wSeqDiff = wHighestReadySeqDiff - (WORD)m_pCtrl->GetMaxFrames();
			if (wSeqDiff < 1U || wSeqDiff >= 0x8000U)
				wSeqDiff = 1U;
			int nCount = 0;
			int nCountOffset = 0;
			if (m_dwAvgFrameTime > 0U)
				nCountOffset = m_dwPingRT / m_dwAvgFrameTime;
			for ( ; wSeqDiff < wHighestReadySeqDiff ; wSeqDiff++)
			{
				WORD wReSendSeq = m_wPrevSeq + wSeqDiff;
				if (!IsFrameReady(wReSendSeq))
				{
					if (m_ReSendCountDown[NETFRAME_RESEND_ARRAY_MASK & wReSendSeq] == 0U)
					{
						ReSendFrame(pNetCom, wReSendSeq);
						m_ReSendCountDown[NETFRAME_RESEND_ARRAY_MASK & wReSendSeq] = ReSendCountDown(	m_ReSendCount[NETFRAME_RESEND_ARRAY_MASK & wReSendSeq],
																										nCount,
																										nCountOffset);
						m_ReSendCount[NETFRAME_RESEND_ARRAY_MASK & wReSendSeq]++;
						TRACE(_T("%u Re-Send Req for Frame with Seq : %u (next countdown : %u)\n"),
																		m_ReSendCount[NETFRAME_RESEND_ARRAY_MASK & wReSendSeq],
																		wReSendSeq,
																		m_ReSendCountDown[NETFRAME_RESEND_ARRAY_MASK & wReSendSeq]);
						if (++nCount >= 2)
							break;
					}
					else
						m_ReSendCountDown[NETFRAME_RESEND_ARRAY_MASK & wReSendSeq]--;
				}
			}
			m_dwLastReSendUpTime = dwCurrentUpTime;
		}
	}

	// Compose, decode, process and free ready frame
	if (bFrameReady)
	{
		// Reset Re-Send counts
		m_ReSendCount[NETFRAME_RESEND_ARRAY_MASK & m_wFrameSeq[dwFrameIndex]] = 0U;
		m_ReSendCountDown[NETFRAME_RESEND_ARRAY_MASK & m_wFrameSeq[dwFrameIndex]] = 0U;

		// Send Lost Count and reset lost Re-Send counts 
		WORD wHighestReadySeqDiff = m_wFrameSeq[dwFrameIndex] - m_wPrevSeq;
		if (wHighestReadySeqDiff > 1U && wHighestReadySeqDiff < 0x8000U)
		{
			TRACE(_T("SendLostCount: %u\n\n"), (WORD)(wHighestReadySeqDiff - 1U));
			SendLostCount(pNetCom, m_wPrevSeq + 1U, m_wFrameSeq[dwFrameIndex] - 1U, wHighestReadySeqDiff - 1U);
			for (WORD wSeqDiff = 1U ; wSeqDiff < wHighestReadySeqDiff ; wSeqDiff++)
			{
				WORD wLostSeq = m_wPrevSeq + wSeqDiff;
				m_ReSendCount[NETFRAME_RESEND_ARRAY_MASK & wLostSeq] = 0U;
				m_ReSendCountDown[NETFRAME_RESEND_ARRAY_MASK & wLostSeq] = 0U;
			}
		}

		// Calc. Avg. Frame Time
		m_wPrevSeq = m_wFrameSeq[dwFrameIndex];

		// Calc. Avg Frame Time
		if (wHighestReadySeqDiff == 1U)
		{
			m_dwAvgFrameTime = (3U * m_dwAvgFrameTime + (m_dwUpTime[dwFrameIndex] - m_dwLastFrameUpTime)) / 4U;
			if (m_dwAvgFrameTime < NETFRAME_MIN_FRAME_TIME)
				m_dwAvgFrameTime = NETFRAME_MIN_FRAME_TIME;
			else if (m_dwAvgFrameTime > NETFRAME_MAX_FRAME_TIME)
				m_dwAvgFrameTime = NETFRAME_DEFAULT_FRAME_TIME;
		}

		// Update Last Up-Times
		m_dwLastFrameUpTime = m_dwUpTime[dwFrameIndex];
		m_dwLastPresentationUpTime = dwCurrentUpTime;

		// Seek to KeyFrame?
		if (m_bSeekToKeyFrame && m_bKeyFrame[dwFrameIndex])
		{
			m_bSeekToKeyFrame = FALSE;
			if (m_pCodecCtx)
				avcodec_flush_buffers(m_pCodecCtx);
		}
		if (!m_bSeekToKeyFrame)
		{
			// Allocate
			LPBYTE pFrame = new BYTE [m_dwFrameSize[dwFrameIndex] + FF_INPUT_BUFFER_PADDING_SIZE];
			if (pFrame)
			{
				// Compose Fragments
				LPBYTE p = pFrame;
				for (int i = 0 ; i < m_nTotalFragments[dwFrameIndex] ; i++)
				{
					int nDataFragmentSize = m_Fragment[dwFrameIndex][i]->GetMsgSize() - sizeof(NetFrameHdrStruct);
					memcpy(p, m_Fragment[dwFrameIndex][i]->GetBuf() + sizeof(NetFrameHdrStruct), nDataFragmentSize);
					p += nDataFragmentSize;
				}

				// Decode and Invalidate for Painting
				Decode(pFrame, m_dwFrameSize[dwFrameIndex]);

				// Clean-Up
				delete [] pFrame;
			}
		}

		// Clean-Up
		FreeFrameFragments(dwFrameIndex);

		// Reset vars
		m_nTotalFragments[dwFrameIndex] = 0;
		m_dwFrameSize[dwFrameIndex] = 0U;
		m_bKeyFrame[dwFrameIndex] = FALSE;
	}

	return FALSE; // Never Call Processor!
}

BOOL CGetFrameParseProcess::SendConfirmation(	CNetCom* pNetCom,
												DWORD dwUpTime,
												DWORD dwFrameSize,
												WORD wSeq,
												BOOL bKeyFrame)
{
	// Header
	NetFrameHdrStructConf Hdr;
	Hdr.dwUpTime = dwUpTime;
	Hdr.dwFrameSize = dwFrameSize;
	Hdr.wSeq = wSeq;
	Hdr.Type = NETFRAME_TYPE_FRAME_CONF | NETFRAME_TYPE_FRAME_REQ;
	Hdr.Flags = NETFRAME_FLAG_VIDEO;
	if (bKeyFrame)
		Hdr.Flags |= NETFRAME_FLAG_KEYFRAME;
	
	// Send
	return (pNetCom->WriteDatagram(	(LPBYTE)&Hdr,
									sizeof(NetFrameHdrStructConf),
									NULL,
									0,
									FALSE) > 0);
}

BOOL CGetFrameParseProcess::SendLostCount(	CNetCom* pNetCom,
											WORD wFirstLostSeq,
											WORD wLastLostSeq,
											WORD wLostCount)
{
	// Header
	NetFrameHdrStructFramesLost Hdr;
	Hdr.wFirstLostSeq = wFirstLostSeq;
	Hdr.wLastLostSeq = wLastLostSeq;
	Hdr.wLostCount = wLostCount;
	Hdr.Type = NETFRAME_TYPE_FRAMES_LOST | NETFRAME_TYPE_FRAME_REQ;
	Hdr.Flags = NETFRAME_FLAG_VIDEO;
	
	// Send
	return (pNetCom->WriteDatagram(	(LPBYTE)&Hdr,
									sizeof(NetFrameHdrStructFramesLost),
									NULL,
									0,
									FALSE) > 0);
}

__forceinline BOOL CGetFrameParseProcess::ReSendFrame(	CNetCom* pNetCom,
														WORD wSeq)
{
	// Header
	NetFrameHdrStructConf Hdr;
	Hdr.wSeq = wSeq;
	Hdr.Type = NETFRAME_TYPE_FRAME_RESEND | NETFRAME_TYPE_FRAME_REQ;
	Hdr.Flags = NETFRAME_FLAG_VIDEO;
	
	// Send
	return (pNetCom->WriteDatagram(	(LPBYTE)&Hdr,
									sizeof(NetFrameHdrStructConf),
									NULL,
									0,
									FALSE) > 0);
}

__forceinline DWORD CGetFrameParseProcess::CalcFrameSize(DWORD dwFrame)
{
	DWORD dwSize = 0U;
	for (int i = 0 ; i < m_nTotalFragments[dwFrame] ; i++)
	{
		if (m_Fragment[dwFrame][i])
			dwSize += (m_Fragment[dwFrame][i]->GetMsgSize() - sizeof(NetFrameHdrStruct));
	}
	return dwSize;
}

__forceinline int CGetFrameParseProcess::GetReceivedFragmentsCount(DWORD dwFrame)
{
	int nCount = 0;
	for (int i = 0 ; i < m_nTotalFragments[dwFrame] ; i++)
	{
		if (m_Fragment[dwFrame][i])
			++nCount;
	}
	return nCount;
}

__forceinline BOOL CGetFrameParseProcess::IsFrameReady(WORD wSeq)
{
	for (DWORD dwFrame = 0U ; dwFrame < m_pCtrl->GetMaxFrames() ; dwFrame++)
		if (wSeq == m_wFrameSeq[dwFrame] && m_dwFrameSize[dwFrame])
			return TRUE;
	return FALSE;
}

#ifdef _DEBUG
void CGetFrameParseProcess::TraceIncompleteFrame(DWORD dwFrame)
{
	CString sMsg(_T("Missing fragment nums: "));
	CString t;
	for (int i = 0 ; i < m_nTotalFragments[dwFrame] ; i++)
	{
		if (m_Fragment[dwFrame][i] == NULL)
		{
			t.Format(_T("%d,"), i);
			sMsg += t;
		}
	}
	sMsg.Delete(sMsg.GetLength() - 1);
	sMsg += _T("\n");
	TRACE(sMsg);
}
#endif

BOOL CGetFrameParseProcess::OpenAVCodec(enum CodecID CodecId, int width, int height)
{
	// Free
	FreeAVCodec();

    // Find the decoder for the video stream
	m_pCodec = avcodec_find_decoder(CodecId);
    if (!m_pCodec)
        goto error_noclose;

	// Allocate Context
	m_pCodecCtx = avcodec_alloc_context();
	if (!m_pCodecCtx)
		goto error_noclose;

	// Width and Height, put the right width & height for SNOW otherwise it is not working!
	m_pCodecCtx->coded_width = width;
	m_pCodecCtx->coded_height = height;

	// Format
	m_pCodecCtx->pix_fmt = PIX_FMT_YUV420P;
	m_pCodecCtx->bits_per_coded_sample = 12;
	m_pCodecCtx->codec_type = CODEC_TYPE_VIDEO;

	// Set some other values
	m_pCodecCtx->error_concealment = 3;
	m_pCodecCtx->error_recognition = 1;

	// Extradata
	if (m_nExtradataSize > 0)
	{
		m_pCodecCtx->extradata = m_pExtradata;
		m_pCodecCtx->extradata_size = m_nExtradataSize;
	}

	// Open codec
    if (avcodec_open(m_pCodecCtx, m_pCodec) < 0)
        goto error_noclose;

	// Allocate video frames
    m_pFrame = avcodec_alloc_frame();
	if (!m_pFrame)
        goto error;
	m_pFrameRGB = avcodec_alloc_frame();
	if (!m_pFrameRGB)
        goto error;

	return TRUE;

error:
	FreeAVCodec();
	return FALSE;
error_noclose:
	FreeAVCodec(TRUE);
	return FALSE;
}

void CGetFrameParseProcess::FreeAVCodec(BOOL bNoClose/*=FALSE*/)
{
	if (m_pCodecCtx)
	{
		// Close
		if (!bNoClose)
			avcodec_close(m_pCodecCtx);

		// Free
		av_freep(&m_pCodecCtx);
		m_pCodec = NULL;
	}
	if (m_pFrame)
	{
		av_free(m_pFrame);
		m_pFrame = NULL;
	}
	if (m_pFrameRGB)
    {
		av_free(m_pFrameRGB);
		m_pFrameRGB = NULL;
	}
	if (m_pImgConvertCtx)
	{
		sws_freeContext(m_pImgConvertCtx);
		m_pImgConvertCtx = NULL;
	}
	if (m_pRGBBuf)
	{
		delete [] m_pRGBBuf;
		m_pRGBBuf = NULL;
	}
	m_dwRGBBufSize = 0;
	m_dwRGBImageSize = 0;
	if (m_pOutbuf)
	{
		delete [] m_pOutbuf;
		m_pOutbuf = NULL;
	}
	m_nOutbufSize = 0;
}

BOOL CGetFrameParseProcess::InitImgConvert()
{	
	// Free
	if (m_pImgConvertCtx)
	{
		sws_freeContext(m_pImgConvertCtx);
		m_pImgConvertCtx = NULL;
	}

	// Determine required buffer size and allocate buffer if necessary
	m_dwRGBImageSize = 4 * m_pCodecCtx->width * m_pCodecCtx->height;
	if ((int)(m_dwRGBBufSize) < m_dwRGBImageSize || m_pRGBBuf == NULL)
	{
		if (m_pRGBBuf)
			delete [] m_pRGBBuf;
		m_pRGBBuf = new BYTE[m_dwRGBImageSize + FF_INPUT_BUFFER_PADDING_SIZE];
		if (!m_pRGBBuf)
			return FALSE;
		m_dwRGBBufSize = m_dwRGBImageSize;
	}

	// Assign appropriate parts of buffer to image planes
	avpicture_fill((AVPicture*)m_pFrameRGB,
					(unsigned __int8 *)m_pRGBBuf,
					PIX_FMT_RGB32,
					m_pCodecCtx->width,
					m_pCodecCtx->height);

	// Prepare Image Conversion Context
	m_pImgConvertCtx = sws_getContext(	m_pCodecCtx->width,		// Source Width
										m_pCodecCtx->height,	// Source Height
										m_pCodecCtx->pix_fmt,	// Source Format
										m_pCodecCtx->width,		// Destination Width
										m_pCodecCtx->height,	// Destination Height
										PIX_FMT_RGB32,			// Destination Format
										SWS_BICUBIC,			// SWS_CPU_CAPS_MMX2, SWS_CPU_CAPS_MMX, SWS_CPU_CAPS_3DNOW
										NULL,					// No Src Filter
										NULL,					// No Dst Filter
										NULL);					// Param

	return (m_pImgConvertCtx != NULL);
}

BOOL CGetFrameParseProcess::Decode(LPBYTE pFrame, DWORD dwFrameSize)
{
	// There is a footer at the end, get it!
	DWORD dwInitFrameSize = dwFrameSize;
	dwFrameSize -= 4;
	m_nExtradataSize = *((DWORD*)&pFrame[dwFrameSize]);
	if (m_nExtradataSize > 0)
	{
		if (!m_pExtradata || m_nMaxExtradata < m_nExtradataSize)
		{
			if (m_pExtradata)
				av_freep(&m_pExtradata);
			m_nMaxExtradata = m_nExtradataSize;
			m_pExtradata = (uint8_t*)av_malloc(	m_nMaxExtradata +
												FF_INPUT_BUFFER_PADDING_SIZE);
			if (!m_pExtradata)
			{
				m_nMaxExtradata = 0;
				m_nExtradataSize = 0;
				return FALSE;
			}
		}
		dwFrameSize -= m_nExtradataSize;
		memcpy(	m_pExtradata,
				&pFrame[dwFrameSize],
				m_nExtradataSize);
	}
	dwFrameSize -= 4;
	m_dwEncryptionType = *((DWORD*)&pFrame[dwFrameSize]);
	dwFrameSize -= 2;
	int height = (int)(*((WORD*)&pFrame[dwFrameSize]));
	dwFrameSize -= 2;
	int width = (int)(*((WORD*)&pFrame[dwFrameSize]));
	dwFrameSize -= 4;
	enum CodecID CodecId = (enum CodecID)(*((DWORD*)&pFrame[dwFrameSize]));
	if (CodecId == CODEC_ID_H263P)
		CodecId = CODEC_ID_H263;
	memset(&pFrame[dwFrameSize], 0, dwInitFrameSize - dwFrameSize);

	// Encryption not yet supported...
	if (m_dwEncryptionType != 0) 
		return FALSE;
	
	// Re-init
	if (width != (int)m_dwPrevDibWidth || height != (int)m_dwPrevDibHeight)
		FreeAVCodec();

	// Init?
	if (!m_pCodecCtx)
	{
		if (!OpenAVCodec(CodecId, width, height))
			return FALSE;
	}

	// Decode
	int got_picture = 0;
	int len = avcodec_decode_video(	m_pCodecCtx,
									m_pFrame,
									&got_picture,
									(unsigned __int8 *)pFrame,
									(int)dwFrameSize);
	if (len > 0 && got_picture)
	{
		::EnterCriticalSection(&m_pCtrl->m_csDib);

		// Init Doc?
		if (m_bFirstFrame							||
			m_CodecId != CodecId					||
			m_dwPrevDibWidth != m_pCodecCtx->width	||
			m_dwPrevDibHeight != m_pCodecCtx->height)
		{
			if (!InitImgConvert())
			{
				::LeaveCriticalSection(&m_pCtrl->m_csDib);
				return FALSE;
			}
			if (!m_pCtrl->m_Dib.AllocateBitsFast(32, BI_RGB, m_pCodecCtx->width, m_pCodecCtx->height))
			{
				::LeaveCriticalSection(&m_pCtrl->m_csDib);
				return FALSE;
			}
			m_dwPrevDibWidth = m_pCtrl->m_Dib.GetWidth();
			m_dwPrevDibHeight = m_pCtrl->m_Dib.GetHeight();
			m_CodecId = CodecId;
			m_bFirstFrame = FALSE;
		}

		// Color Space Conversion
		if (m_pImgConvertCtx)
		{
			int sws_scale_res = sws_scale(	m_pImgConvertCtx,		// Image Convert Context
											m_pFrame->data,			// Source Data
											m_pFrame->linesize,		// Source Stride
											0,						// Source Slice Y
											m_pCodecCtx->height,	// Source Height
											m_pFrameRGB->data,		// Destination Data
											m_pFrameRGB->linesize);	// Destination Stride
#ifdef SUPPORT_LIBSWSCALE
			int res = sws_scale_res > 0 ? 1 : -1;
#else
			int res = sws_scale_res >= 0 ? 1 : -1;
#endif
			// Set Bits to Dib
			if (res == 1)
			{
				// Flip Vertically	
				DWORD dwDWAlignedLineSize = 4 * m_pCtrl->m_Dib.GetWidth();
				LPBYTE lpSrcBits = m_pRGBBuf;
				LPBYTE lpDstBits = m_pCtrl->m_Dib.GetBits() + (m_pCtrl->m_Dib.GetHeight() - 1) * dwDWAlignedLineSize;
				for (DWORD dwCurLine = 0 ; dwCurLine < m_pCtrl->m_Dib.GetHeight() ; dwCurLine++)
				{
					memcpy((void*)lpDstBits, (void*)lpSrcBits, dwDWAlignedLineSize); 
					lpSrcBits += dwDWAlignedLineSize;
					lpDstBits -= dwDWAlignedLineSize;
				}
			}	
		}

		::LeaveCriticalSection(&m_pCtrl->m_csDib);

		// Update View
		m_pCtrl->InvalidateControl();

		return TRUE;
	}
	else
		return FALSE;
}
