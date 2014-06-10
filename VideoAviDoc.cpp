#include "stdafx.h"
#include "uImager.h"
#include "MainFrm.h"
#include "VideoAviDoc.h"
#include "VideoAviView.h"
#include "SaveFileDlg.h"
#include "ResizingDlg.h"
#include "AudioOutDestinationDlg.h"
#include "AnimGifSaveDlg.h"
#include "AviOpenStreamsDlg.h"
#include "AviInfoDlg.h"
#include "OutVolDlg.h"
#include "AudioVideoShiftDlg.h"
#include "PlayerToolBarDlg.h"
#include "RenameDlg.h"
#include "PostDelayedMessage.h"
#include "Quantizer.h"
#include "AnimGifSave.h"
#include "NoVistaFileDlg.h"
#include "VideoFormatDlg.h"
#include "AudioFormatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// VideoAviDoc

IMPLEMENT_DYNCREATE(CVideoAviDoc, CUImagerDoc)

BEGIN_MESSAGE_MAP(CVideoAviDoc, CUImagerDoc)
	//{{AFX_MSG_MAP(CVideoAviDoc)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
	ON_COMMAND(ID_PLAY_AVI, OnPlayAvi)
	ON_UPDATE_COMMAND_UI(ID_PLAY_AVI, OnUpdatePlayAvi)
	ON_COMMAND(ID_STOP_AVI, OnStopAvi)
	ON_UPDATE_COMMAND_UI(ID_STOP_AVI, OnUpdateStopAvi)
	ON_COMMAND(ID_PLAY_LOOP, OnPlayLoop)
	ON_UPDATE_COMMAND_UI(ID_PLAY_LOOP, OnUpdatePlayLoop)
	ON_COMMAND(ID_PLAY_FRAME_BACK, OnPlayFrameBack)
	ON_UPDATE_COMMAND_UI(ID_PLAY_FRAME_BACK, OnUpdatePlayFrameBack)
	ON_COMMAND(ID_PLAY_FRAME_FRONT, OnPlayFrameFront)
	ON_UPDATE_COMMAND_UI(ID_PLAY_FRAME_FRONT, OnUpdatePlayFrameFront)
	ON_COMMAND(ID_PLAY_FRAME_BACK_FAST, OnPlayFrameBackFast)
	ON_UPDATE_COMMAND_UI(ID_PLAY_FRAME_BACK_FAST, OnUpdatePlayFrameBackFast)
	ON_COMMAND(ID_PLAY_FRAME_FRONT_FAST, OnPlayFrameFrontFast)
	ON_UPDATE_COMMAND_UI(ID_PLAY_FRAME_FRONT_FAST, OnUpdatePlayFrameFrontFast)
	ON_COMMAND(ID_PLAY_FRAME_FIRST, OnPlayFrameFirst)
	ON_UPDATE_COMMAND_UI(ID_PLAY_FRAME_FIRST, OnUpdatePlayFrameFirst)
	ON_COMMAND(ID_PLAY_FRAME_LAST, OnPlayFrameLast)
	ON_UPDATE_COMMAND_UI(ID_PLAY_FRAME_LAST, OnUpdatePlayFrameLast)
	ON_COMMAND(ID_EDIT_STOP, OnEditStop)
	ON_UPDATE_COMMAND_UI(ID_EDIT_STOP, OnUpdateEditStop)
	ON_COMMAND(ID_FILE_INFO, OnFileInfo)
	ON_COMMAND(ID_VIEW_TIMEPOSITION, OnViewTimeposition)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TIMEPOSITION, OnUpdateViewTimeposition)
	ON_UPDATE_COMMAND_UI(ID_FILE_INFO, OnUpdateFileInfo)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_COMMAND(ID_PLAY_VOL, OnPlayVol)
	ON_UPDATE_COMMAND_UI(ID_PLAY_VOL, OnUpdatePlayVol)
	ON_COMMAND(ID_PLAY_AUDIOVIDEOSHIFT, OnPlayAudiovideoshift)
	ON_UPDATE_COMMAND_UI(ID_PLAY_AUDIOVIDEOSHIFT, OnUpdatePlayAudiovideoshift)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_PLAY_VFWCODECPRIORITY, OnPlayVfwcodecpriority)
	ON_UPDATE_COMMAND_UI(ID_PLAY_VFWCODECPRIORITY, OnUpdatePlayVfwcodecpriority)
	ON_COMMAND(ID_PLAY_INTERNALCODECPRIORITY, OnPlayInternalcodecpriority)
	ON_UPDATE_COMMAND_UI(ID_PLAY_INTERNALCODECPRIORITY, OnUpdatePlayInternalcodecpriority)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_COMMAND(ID_EDIT_SNAPSHOT, OnEditSnapshot)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SNAPSHOT, OnUpdateEditSnapshot)
	ON_COMMAND(ID_EDIT_RENAME, OnEditRename)
	ON_UPDATE_COMMAND_UI(ID_EDIT_RENAME, OnUpdateEditRename)
	ON_COMMAND(ID_FILE_EXTRACTFRAMES, OnFileExtractframes)
	ON_UPDATE_COMMAND_UI(ID_FILE_EXTRACTFRAMES, OnUpdateFileExtractframes)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CVideoAviDoc::CPlayAudioFileThread::CPlayAudioFileThread()
{
	m_pDoc = NULL;
	m_nPlaySpeedPercent = 100;
	memset(&m_WaveHeader[0], 0, sizeof(m_WaveHeader[0]));
	memset(&m_WaveHeader[1], 0, sizeof(m_WaveHeader[1]));
	m_pBuf[0] = NULL;
	m_pBuf[1] = NULL;

	// Create Output Event
	m_hWaveOutEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEventArray[0] = GetKillEvent();
	m_hEventArray[1] = m_hWaveOutEvent;
	m_hPlaySyncEventArray[0] = GetKillEvent();
	m_hPlaySyncEventArray[1] = NULL;
	m_hWaveOut = NULL;
	m_pWaveFormat = (WAVEFORMATEX*)new BYTE[sizeof(WAVEFORMATEX)];
	memset(m_pWaveFormat, 0, sizeof(WAVEFORMATEX));

	// Sync
	m_hPlaySyncEvent = NULL;
	m_bWaitingForStart = FALSE;
	m_bResetting = FALSE;
	m_bSleepingVideoLeadsAudio = TRUE;
}

CVideoAviDoc::CPlayAudioFileThread::~CPlayAudioFileThread() 
{
	Kill();
	CloseOutAudio();
	if (m_pWaveFormat)
	{
		delete [] m_pWaveFormat;
		m_pWaveFormat = NULL;
	}
	::CloseHandle(m_hWaveOutEvent);
	m_hWaveOutEvent = NULL;
	if (m_pBuf[0])
	{
		delete [] m_pBuf[0];
		m_pBuf[0] = NULL;
	}
	if (m_pBuf[1])
	{
		delete [] m_pBuf[1];
		m_pBuf[1] = NULL;
	}
}

// New playback rate setting. This setting is a multiplier indicating the current
// change in playback rate from the original authored setting.
// The playback rate multiplier must be a positive value. 
// The rate is specified as a fixed-point value. The high-order word contains
// the signed integer part of the number, and the low-order word contains the fractional part.
// A value of 0x8000 in the low-order word represents one-half, and 0x4000 represents one-quarter.
// For example, the value 0x00010000 specifies a multiplier of 1.0 (no playback rate change),
// and a value of 0x000F8000 specifies a multiplier of 15.5. 
BOOL CVideoAviDoc::CPlayAudioFileThread::SetPlaySpeedPercent(int nPlaySpeedPercent)
{
	if (m_hWaveOut == NULL)
	{
		if (nPlaySpeedPercent == 100)
			return TRUE;
		else
			return FALSE;
	}
	
	m_nPlaySpeedPercent = nPlaySpeedPercent;

	double dPlaySpeed = nPlaySpeedPercent / 100.0;
	WORD wPlaySpeedHiWord = (WORD)dPlaySpeed;

	// Convert From Floating Point To Fixed-Point
	//
	// dwRate Format:
	//
	// msbit                                                                        lsbit
	// 31     30    29    ..  16    15      14      13       12      ..    1        0
	// 2^15   2^14  2^13  ..  2^0   2^(-1)  2^(-2)  2^(-3)   2^(-4)  ..    2^(-15)  2^(-16)

	// Init:
	double dFractionalPart = dPlaySpeed - (double)wPlaySpeedHiWord;
	WORD wPlaySpeedLoWord = 0;
	for (int i = 15 ; i >= 0 ; i--) // From bit15 down to bit0
	{
		double dFractionalPartFirst = dFractionalPart;
		dFractionalPart *= 2.0;
		DWORD dwIntegerPart = (DWORD)dFractionalPart;
		dFractionalPart -= dwIntegerPart;	
		if ((dFractionalPartFirst - (dFractionalPart / 2.0)) * 2.0)
			wPlaySpeedLoWord |= (1<<i);
	}

	DWORD dwRate = wPlaySpeedHiWord<<16;
	dwRate |= (DWORD)wPlaySpeedLoWord;

	MMRESULT res = ::waveOutSetPlaybackRate(m_hWaveOut, dwRate);
	if (res != MMSYSERR_NOERROR)
	{
		switch (res)
		{
			case MMSYSERR_NOTSUPPORTED :	if (m_nPlaySpeedPercent == 100) return TRUE;
											::AfxMessageBox(ML_STRING(1422, "Sound Output Playback Speed Not Supported"));
											break;
			default :						::AfxMessageBox(ML_STRING(1423, "Sound Output Cannot Set Playback Speed"));
											break;
		}
		
		res = ::waveOutSetPitch(m_hWaveOut, dwRate);
		if (res != MMSYSERR_NOERROR)
		{
			switch (res)
			{
				case MMSYSERR_NOTSUPPORTED :	if (m_nPlaySpeedPercent == 100) return TRUE;
												::AfxMessageBox(ML_STRING(1424, "Sound Output Pitch Not Supported"));
												break;
				default :						::AfxMessageBox(ML_STRING(1425, "Sound Output Cannot Set Pitch"));
												break;
			}

			return FALSE;
		}

		return TRUE;
	}

	return TRUE;
}

void CVideoAviDoc::CPlayAudioFileThread::OnExit()
{
	::ResetEvent(m_hWaveOutEvent);
	::waveOutPause(m_hWaveOut);
	m_bResetting = TRUE;
	::waveOutReset(m_hWaveOut);
	::waveOutUnprepareHeader(m_hWaveOut, &m_WaveHeader[0], sizeof(WAVEHDR));
	::waveOutUnprepareHeader(m_hWaveOut, &m_WaveHeader[1], sizeof(WAVEHDR));
	memset(&m_WaveHeader[0], 0, sizeof(m_WaveHeader[0]));
	memset(&m_WaveHeader[1], 0, sizeof(m_WaveHeader[1]));
	if (m_pBuf[0])
	{
		delete [] m_pBuf[0];
		m_pBuf[0] = NULL;
	}
	if (m_pBuf[1])
	{
		delete [] m_pBuf[1];
		m_pBuf[1] = NULL;
	}
	::EnterCriticalSection(&m_pDoc->m_csPlayWaitingForStart);
	m_bWaitingForStart = FALSE;
	::LeaveCriticalSection(&m_pDoc->m_csPlayWaitingForStart);

	// Update View
	if ((!m_pDoc->m_pAVIPlay->HasVideo() || !(m_pDoc->m_nActiveVideoStream >= 0)))
	{
		CPostDelayedMessageThread::PostDelayedMessage(	m_pDoc->GetView()->GetSafeHwnd(),
														WM_THREADSAFE_UPDATEWINDOWSIZES,
														THREAD_SAFE_UPDATEWINDOWSIZES_DELAY,
														(WPARAM)(	UPDATEWINDOWSIZES_INVALIDATE |
																	UPDATEWINDOWSIZES_ERASEBKG),
														(LPARAM)0);
	}
	else
	{
		CPostDelayedMessageThread::PostDelayedMessage(	m_pDoc->GetView()->GetSafeHwnd(),
														WM_THREADSAFE_UPDATEWINDOWSIZES,
														THREAD_SAFE_UPDATEWINDOWSIZES_DELAY,
														(WPARAM)(UPDATEWINDOWSIZES_INVALIDATE),
														(LPARAM)0);
	}

	::CoUninitialize();
}

int CVideoAviDoc::CPlayAudioFileThread::Work() 
{
	DWORD Event;
	bool bExit;
	BOOL bDoRewind;

	if (!m_pDoc)
		return 0;
	
	if (!m_pDoc->m_pAVIPlay)
		return 0;

	if (!IsOpen())
		return 0;

	CAVIPlay::CAVIAudioStream* pAudioStream = m_pDoc->m_pAVIPlay->GetAudioStream(m_pDoc->m_nActiveAudioStream);
	if (!pAudioStream)
		return 0;

	// Get The Output (=Uncompressed) Buffer Size 
	DWORD dwBufSize = pAudioStream->GetBufSize();
	if (dwBufSize == 0)
		return 0;

	// Allocate 2 x Buffers
	if (m_pBuf[0])
		delete [] m_pBuf[0];
	if (m_pBuf[1])
		delete [] m_pBuf[1];
	m_pBuf[0] = new BYTE [dwBufSize];
	m_pBuf[1] = new BYTE [dwBufSize];

	::CoInitialize(NULL);

	// Wait For Sync.
	if (m_hPlaySyncEvent)
	{
		::EnterCriticalSection(&m_pDoc->m_csPlayWaitingForStart);
		if (!m_pDoc->m_PlayVideoFileThread.IsWaitingForStart())
		{
			m_bWaitingForStart = TRUE;
			::ResetEvent(m_hPlaySyncEvent);
		}
		::LeaveCriticalSection(&m_pDoc->m_csPlayWaitingForStart);
	}

	// No Rewind
	bDoRewind = FALSE;

	// Loop
	do
	{
		// Make Sure To Pause The Player
		if (::waveOutPause(m_hWaveOut) != MMSYSERR_NOERROR)
		{
			TRACE(_T("Sound Output Cannot Pause Device!\n"));
			OnExit();
			return 0;
		}

		// Reset Audio Event
		::ResetEvent(m_hWaveOutEvent);

		// Sync with Video
		m_bSleepingVideoLeadsAudio = TRUE;
		if (m_hPlaySyncEvent)
		{
			if (!m_bWaitingForStart)
				::SetEvent(m_hPlaySyncEvent);

			Event = ::WaitForMultipleObjects(2, m_hPlaySyncEventArray, FALSE, INFINITE);
			switch (Event)
			{
				// Shutdown Event
				case WAIT_OBJECT_0 :		OnExit();
											return 0;

				// Sync Event
				case WAIT_OBJECT_0 + 1 :	::EnterCriticalSection(&m_pDoc->m_csPlayWaitingForStart);
											m_bWaitingForStart = FALSE;
											::LeaveCriticalSection(&m_pDoc->m_csPlayWaitingForStart);
											break;

				default :					OnExit();
											return 0;
			}
		}

		// Rewind?
		if (bDoRewind)
			Rew();

		// Wait if Video Leads Audio at Start-Up
		LONGLONG llAudioPos = GetNextSamplePos() * 1000 / (LONGLONG)GetSampleRate();
		if (llAudioPos < -((LONGLONG)m_pDoc->m_nAudioLeadsVideoShift))
			::Sleep((DWORD)-((LONGLONG)m_pDoc->m_nAudioLeadsVideoShift - llAudioPos));
		m_bSleepingVideoLeadsAudio = FALSE;	// Now video thread can sync. with us!

		// We Are Starting Not Resetting!
		m_bResetting = FALSE;

		// Reset Exit Flag
		bExit = false;

		// Start Buffering
		if (pAudioStream)
		{
			m_nWaveOutToggle = 0;
			memcpy(m_pBuf[m_nWaveOutToggle], pAudioStream->GetBufData(), pAudioStream->GetBufBytesCount());
			
			// To avoid some codec's initial junk
			// generating clicks silent some samples.
			// Could also do a fade-in...
			int nSamplesToSilent = m_pWaveFormat->nSamplesPerSec / 10;	// Silent 100 ms,
			if (nSamplesToSilent < 2*1152)								// or a minimum of
				nSamplesToSilent = 2*1152;								// two mpeg audio frames
			DWORD dwBytesToSilent = (DWORD)(nSamplesToSilent * m_pWaveFormat->nBlockAlign);
			if (m_pWaveFormat->wBitsPerSample == 8)
				memset(m_pBuf[m_nWaveOutToggle], 0x80, MIN(dwBytesToSilent, pAudioStream->GetBufBytesCount()));
			else
				memset(m_pBuf[m_nWaveOutToggle], 0, MIN(dwBytesToSilent, pAudioStream->GetBufBytesCount()));
			
			if (!DataOutAudio(m_pBuf[m_nWaveOutToggle], pAudioStream->GetBufBytesCount()))
				bExit = true;
			if (!pAudioStream->GetNextChunksSamples())
				bExit = true;
			memcpy(m_pBuf[m_nWaveOutToggle], pAudioStream->GetBufData(), pAudioStream->GetBufBytesCount()); 
			if (!DataOutAudio(m_pBuf[m_nWaveOutToggle], pAudioStream->GetBufBytesCount()))
				bExit = true;
			if (!pAudioStream->GetNextChunksSamples())
				bExit = true;
		}

		// Start Playing
		::waveOutRestart(m_hWaveOut);

		// Update Slider & View
		if ((!m_pDoc->m_pAVIPlay->HasVideo() || !(m_pDoc->m_nActiveVideoStream >= 0)))
		{
			::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
							WM_THREADSAFE_UPDATEPLAYSLIDER,
							0, 0);
			::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
							WM_THREADSAFE_UPDATEWINDOWSIZES,
							(WPARAM)(	UPDATEWINDOWSIZES_INVALIDATE |
										UPDATEWINDOWSIZES_ERASEBKG),
							(LPARAM)0);
		}
		
		// Play
		while (!bExit)
		{		
			Event = ::WaitForMultipleObjects(2, m_hEventArray, FALSE, INFINITE);
			switch (Event)
			{
				// Shutdown Event
				case WAIT_OBJECT_0 :		OnExit();
											return 0;

				// Wave Out Event
				case WAIT_OBJECT_0 + 1 :	::ResetEvent(m_hWaveOutEvent);
											if (pAudioStream)
											{
												memcpy(m_pBuf[m_nWaveOutToggle], pAudioStream->GetBufData(), pAudioStream->GetBufBytesCount()); 
												if (!DataOutAudio(m_pBuf[m_nWaveOutToggle], pAudioStream->GetBufBytesCount()))
													bExit = true;
												if (!pAudioStream->GetNextChunksSamples())
													bExit = true;
											}

											// Update Slider & View
											if ((!m_pDoc->m_pAVIPlay->HasVideo() || !(m_pDoc->m_nActiveVideoStream >= 0)))
											{
												::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
																WM_THREADSAFE_UPDATEPLAYSLIDER,
																0, 0);
												::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
																WM_THREADSAFE_UPDATEWINDOWSIZES,
																(WPARAM)(	UPDATEWINDOWSIZES_INVALIDATE |
																			UPDATEWINDOWSIZES_ERASEBKG),
																(LPARAM)0);
											}

											break;

				default :					OnExit();
											return 0;					
			}
		}

		// Wait End Of Play
		m_bResetting = TRUE;
		if (!WaitEndOfPlayUnprepareHdr()) // Returns FALSE if exiting thread!
		{
			OnExit();
			return 0;
		}
	
		// Update Slider & View
		if ((!m_pDoc->m_pAVIPlay->HasVideo() || !(m_pDoc->m_nActiveVideoStream >= 0)))
		{
			::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
							WM_THREADSAFE_UPDATEPLAYSLIDER,
							0, 0);
			::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
							WM_THREADSAFE_UPDATEWINDOWSIZES,
							(WPARAM)(	UPDATEWINDOWSIZES_INVALIDATE |
										UPDATEWINDOWSIZES_ERASEBKG),
							(LPARAM)0);
		}

		// Clean-Up
		::ResetEvent(m_hWaveOutEvent);
		::waveOutReset(m_hWaveOut);
		memset(&m_WaveHeader[0], 0, sizeof(m_WaveHeader[0]));
		memset(&m_WaveHeader[1], 0, sizeof(m_WaveHeader[1]));

		::EnterCriticalSection(&m_pDoc->m_csPlayWaitingForStart);
		if (m_pDoc->IsLoop())
		{
			bDoRewind = TRUE;
			if (m_hPlaySyncEvent)
			{
				if (!m_pDoc->m_PlayVideoFileThread.IsWaitingForStart())
				{
					m_bWaitingForStart = TRUE;
					::ResetEvent(m_hPlaySyncEvent);
				}
			}
		}
		::LeaveCriticalSection(&m_pDoc->m_csPlayWaitingForStart);
	}
	while (m_pDoc->IsLoop());

	// Clean-Up
	if (m_pBuf[0])
	{
		delete [] m_pBuf[0];
		m_pBuf[0] = NULL;
	}
	if (m_pBuf[1])
	{
		delete [] m_pBuf[1];
		m_pBuf[1] = NULL;
	}
	::EnterCriticalSection(&m_pDoc->m_csPlayWaitingForStart);
	m_bWaitingForStart = FALSE;
	::LeaveCriticalSection(&m_pDoc->m_csPlayWaitingForStart);

	// Update View
	if ((!m_pDoc->m_pAVIPlay->HasVideo() || !(m_pDoc->m_nActiveVideoStream >= 0)))
	{
		CPostDelayedMessageThread::PostDelayedMessage(	m_pDoc->GetView()->GetSafeHwnd(),
														WM_THREADSAFE_UPDATEWINDOWSIZES,
														THREAD_SAFE_UPDATEWINDOWSIZES_DELAY,
														(WPARAM)(	UPDATEWINDOWSIZES_INVALIDATE |
																	UPDATEWINDOWSIZES_ERASEBKG),
														(LPARAM)0);
	}
	else
	{
		CPostDelayedMessageThread::PostDelayedMessage(	m_pDoc->GetView()->GetSafeHwnd(),
														WM_THREADSAFE_UPDATEWINDOWSIZES,
														THREAD_SAFE_UPDATEWINDOWSIZES_DELAY,
														(WPARAM)(UPDATEWINDOWSIZES_INVALIDATE),
														(LPARAM)0);
	}

	::CoUninitialize();

	return 0;
}

BOOL CVideoAviDoc::CPlayAudioFileThread::WaitEndOfPlayUnprepareHdr()
{
	DWORD Event;

	// Make Sure Something Has Been Played!
	if ((m_WaveHeader[0].dwFlags & WHDR_PREPARED) == WHDR_PREPARED)
	{
		// You must wait until the driver is finished with the buffer
		// before calling waveOutUnprepareHeader
		while ((m_WaveHeader[0].dwFlags & WHDR_DONE) != WHDR_DONE)
		{
			Event = ::WaitForMultipleObjects(2, m_hEventArray, FALSE, 50);
			switch (Event)
			{
				// Shutdown Event
				case WAIT_OBJECT_0 :		return FALSE;

				// Wave Out Event
				case WAIT_OBJECT_0 + 1 :	::ResetEvent(m_hWaveOutEvent);
											break;

				default :
					break;
			}
		}

		// This Loop is not necessary, but you never know...
		while (::waveOutUnprepareHeader(m_hWaveOut, &m_WaveHeader[0], sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
		{
			Event = ::WaitForMultipleObjects(2, m_hEventArray, FALSE, 50);
			switch (Event)
			{
				// Shutdown Event
				case WAIT_OBJECT_0 :		return FALSE;

				// Wave Out Event
				case WAIT_OBJECT_0 + 1 :	::ResetEvent(m_hWaveOutEvent);
											break;

				default :
					break;
			}
		}
	}

	// Make Sure Something Has Been Played!
	if ((m_WaveHeader[1].dwFlags & WHDR_PREPARED) == WHDR_PREPARED)
	{
		// You must wait until the driver is finished with the buffer
		// before calling waveOutUnprepareHeader
		while ((m_WaveHeader[1].dwFlags & WHDR_DONE) != WHDR_DONE)
		{
			Event = ::WaitForMultipleObjects(2, m_hEventArray, FALSE, 50);
			switch (Event)
			{
				// Shutdown Event
				case WAIT_OBJECT_0 :		return FALSE;

				// Wave Out Event
				case WAIT_OBJECT_0 + 1 :	::ResetEvent(m_hWaveOutEvent);
											break;

				default :
					break;
			}
		}

		// This Loop is not necessary, but you never know...
		while (::waveOutUnprepareHeader(m_hWaveOut, &m_WaveHeader[1], sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
		{
			Event = ::WaitForMultipleObjects(2, m_hEventArray, FALSE, 50);
			switch (Event)
			{
				// Shutdown Event
				case WAIT_OBJECT_0 :		return FALSE;

				// Wave Out Event
				case WAIT_OBJECT_0 + 1 :	::ResetEvent(m_hWaveOutEvent);
											break;

				default :
					break;
			}
		}
	}

	return TRUE;
}

void CVideoAviDoc::CPlayAudioFileThread::Rew()
{
	if (m_pDoc && m_pDoc->m_pAVIPlay)
	{
		CAVIPlay::CAVIAudioStream* pAudioStream = m_pDoc->m_pAVIPlay->GetAudioStream(m_pDoc->m_nActiveAudioStream);
		if (pAudioStream)
			pAudioStream->Rew();
	}
}

LONGLONG CVideoAviDoc::CPlayAudioFileThread::GetCurrentSamplePos() const
{
	if (m_pDoc && m_pDoc->m_pAVIPlay)
	{
		CAVIPlay::CAVIAudioStream* pAudioStream = m_pDoc->m_pAVIPlay->GetAudioStream(m_pDoc->m_nActiveAudioStream);
		if (pAudioStream)
		{
			if (pAudioStream->IsEnd())
				return pAudioStream->GetTotalSamples() - 1;
			else if (pAudioStream->IsStart())
				return -1;
			else
				return (pAudioStream->GetCurrentSamplePos() - 1);
		}
		else
			return -1;
	}
	else
		return -1;
}

LONGLONG CVideoAviDoc::CPlayAudioFileThread::GetNextSamplePos() const
{
	if (m_pDoc && m_pDoc->m_pAVIPlay)
	{
		CAVIPlay::CAVIAudioStream* pAudioStream = m_pDoc->m_pAVIPlay->GetAudioStream(m_pDoc->m_nActiveAudioStream);
		if (pAudioStream)
		{
			if (pAudioStream->IsEnd())
				return pAudioStream->GetTotalSamples();
			else if (pAudioStream->IsStart())
				return 0;
			else
				return (LONGLONG)pAudioStream->GetCurrentSamplePos();
		}
		else
			return 0;
	}
	else
		return 0;
}

BOOL CVideoAviDoc::CPlayAudioFileThread::SetCurrentSamplePos(LONGLONG llCurrentSamplePos)
{
	if ((llCurrentSamplePos < -1) || (llCurrentSamplePos >= GetTotalSamples()))
		return FALSE;

	if (m_pDoc && m_pDoc->m_pAVIPlay)
	{
		CAVIPlay::CAVIAudioStream* pAudioStream = m_pDoc->m_pAVIPlay->GetAudioStream(m_pDoc->m_nActiveAudioStream);
		if (pAudioStream)
			return pAudioStream->GetSamples(llCurrentSamplePos + 1);
		else
			return FALSE;
	}
	else
		return FALSE;
}

BOOL CVideoAviDoc::CPlayAudioFileThread::SetNextSamplePos(LONGLONG llNextSamplePos)
{
	if (llNextSamplePos > GetTotalSamples())
		return FALSE;

	if (m_pDoc && m_pDoc->m_pAVIPlay)
	{
		CAVIPlay::CAVIAudioStream* pAudioStream = m_pDoc->m_pAVIPlay->GetAudioStream(m_pDoc->m_nActiveAudioStream);
		if (pAudioStream)
			return pAudioStream->GetSamples(llNextSamplePos);
		else
			return FALSE;
	}
	else
		return FALSE;
}

LONGLONG CVideoAviDoc::CPlayAudioFileThread::GetTotalSamples() const
{
	if (m_pDoc && m_pDoc->m_pAVIPlay)
	{
		CAVIPlay::CAVIAudioStream* pAudioStream = m_pDoc->m_pAVIPlay->GetAudioStream(m_pDoc->m_nActiveAudioStream);
		if (pAudioStream)
			return pAudioStream->GetTotalSamples();
		else
			return 0;
	}
	else
		return 0;
}

DWORD CVideoAviDoc::CPlayAudioFileThread::GetSampleRate() const
{
	if (m_pDoc && m_pDoc->m_pAVIPlay)
	{
		CAVIPlay::CAVIAudioStream* pAudioStream = m_pDoc->m_pAVIPlay->GetAudioStream(m_pDoc->m_nActiveAudioStream);
		if (pAudioStream)
		{
			// Return Decompressed sample rate which may differ from
			// source sample rate for some codecs like AAC!
			return pAudioStream->GetSampleRate(false);
		}
		else
			return 0;
	}
	else
		return 0;
}

void CVideoAviDoc::CPlayAudioFileThread::PlaySyncAudioFromVideo()
{
	if (m_pDoc && m_pDoc->m_pAVIPlay)
	{
		CAVIPlay::CAVIVideoStream* pVideoStream = m_pDoc->m_pAVIPlay->GetVideoStream(m_pDoc->m_nActiveVideoStream);
		CAVIPlay::CAVIAudioStream* pAudioStream = m_pDoc->m_pAVIPlay->GetAudioStream(m_pDoc->m_nActiveAudioStream);
		if (pVideoStream && pAudioStream)
		{
			double dCurrentTimePos = 1.0 /	(double)m_pDoc->GetPlayFrameRate()		*
											(double)pVideoStream->GetNextFramePos()	+
											(double)m_pDoc->m_nAudioLeadsVideoShift;
			LONGLONG llSampleNum = (LONGLONG)(dCurrentTimePos * pAudioStream->GetSampleRate(false));
			if (llSampleNum < 0)
				llSampleNum = 0;
			pAudioStream->GetSamples(llSampleNum);
		}
	}
}

void CVideoAviDoc::CPlayAudioFileThread::AudioOutDestinationDialog()
{
	CAudioOutDestinationDlg dlg(m_pDoc->GetView(), m_pDoc->m_dwPlayAudioDeviceID);
	if (dlg.DoModal() == IDOK && dlg.m_uiDeviceID != m_pDoc->m_dwPlayAudioDeviceID)
	{
		m_pDoc->StopAVI();
		if (m_pDoc->m_pOutVolDlg)
		{
			// m_pOutVolDlg pointer is set to NULL
			// from the dialog class (selfdeletion)
			m_pDoc->m_pOutVolDlg->Close();
		}
		m_pDoc->m_PlayAudioFileThread.WaitDone_Blocking();
		CloseOutAudio();
		m_pDoc->m_dwPlayAudioDeviceID = dlg.m_uiDeviceID;
		if (m_pDoc && m_pDoc->m_pAVIPlay)
		{
			CAVIPlay::CAVIAudioStream* pAudioStream = m_pDoc->m_pAVIPlay->GetAudioStream(m_pDoc->m_nActiveAudioStream);
			if (pAudioStream)
				OpenOutAudio(pAudioStream->GetFormat(false));
		}
	}
}

BOOL CVideoAviDoc::CPlayAudioFileThread::OpenOutAudio(LPWAVEFORMATEX pWaveFormat/*=NULL*/)
{
	MMRESULT res;
	LPWAVEFORMATEX pUncompressedWaveFormat = NULL;

	// First Close
	CloseOutAudio();

	// Get Number of Audio Devices
	UINT num = ::waveOutGetNumDevs(); 
	if (num == 0)
	{
		TRACE(_T("No Sound Output Device.\n"));
		return FALSE;
	}

	// Check Wave Format Pointer
	if (pWaveFormat)
		SetWaveFormat(pWaveFormat);
	else
	{
		if (m_pWaveFormat == NULL)
			return FALSE;
	}

	// Choose the Right PCM Format if a Compression is Set
	if (m_pWaveFormat->wFormatTag != WAVE_FORMAT_PCM)
	{
		pUncompressedWaveFormat = new WAVEFORMATEX;
		::ZeroMemory(pUncompressedWaveFormat, sizeof(WAVEFORMATEX));
		pUncompressedWaveFormat->wFormatTag = WAVE_FORMAT_PCM;
		res = ::acmFormatSuggest(NULL, m_pWaveFormat, pUncompressedWaveFormat, sizeof(WAVEFORMATEX), ACM_FORMATSUGGESTF_WFORMATTAG);
		if (res != MMSYSERR_NOERROR)
		{
			delete pUncompressedWaveFormat;
			::AfxMessageBox(ML_STRING(1428, "Sound Output Cannot Choose The Right PCM Format!"));
			return FALSE;
		}
	}

	// Open First Output Device 
	res = ::waveOutOpen(&m_hWaveOut, m_pDoc->m_dwPlayAudioDeviceID, (pUncompressedWaveFormat != NULL) ? pUncompressedWaveFormat : m_pWaveFormat, (DWORD)m_hWaveOutEvent, NULL, CALLBACK_EVENT); 
	if (res != MMSYSERR_NOERROR)
	{
		if (pUncompressedWaveFormat)
			delete pUncompressedWaveFormat;
		::AfxMessageBox(ML_STRING(1429, "Sound Output Cannot Open Device!"));
	    return FALSE;
	}

	// Clean-up
	if (pUncompressedWaveFormat)
		delete pUncompressedWaveFormat;

	// Reset The Open Event
	::ResetEvent(m_hWaveOutEvent);

	// Pause Player Otherwise with the first write it starts to play
	res =::waveOutPause(m_hWaveOut);
	if (res != MMSYSERR_NOERROR)
	{
        ::AfxMessageBox(ML_STRING(1430, "Sound Output Cannot Pause Device!"));
	    return FALSE;
	}

	return TRUE;
}

void CVideoAviDoc::CPlayAudioFileThread::CloseOutAudio()
{
	if (m_hWaveOut)
	{
		::waveOutClose(m_hWaveOut);
		m_hWaveOut = NULL;
		// Reset The Close Event
		::ResetEvent(m_hWaveOutEvent);
	}
}

BOOL CVideoAviDoc::CPlayAudioFileThread::DataOutAudio(LPBYTE lpData, DWORD dwSize)
{
	MMRESULT res;

	if (lpData == NULL)
		return FALSE;

	if (dwSize == 0)
		return FALSE;

	// Obs.: waveOutUnprepareHeader in Win2000 fails if the WAVEHDR is empty (all zeros)
	// Make sure the buffer has been used (not first 2 buffers)
	if (m_WaveHeader[m_nWaveOutToggle].dwFlags & WHDR_DONE)
	{
		res = ::waveOutUnprepareHeader(m_hWaveOut, &m_WaveHeader[m_nWaveOutToggle], sizeof(WAVEHDR)); 
		if (res != MMSYSERR_NOERROR) 
		{
			TRACE(_T("Sound Output Cannot UnPrepareHeader!\n"));
			return FALSE;
		}
	}

    m_WaveHeader[m_nWaveOutToggle].lpData = (CHAR*)lpData;
    m_WaveHeader[m_nWaveOutToggle].dwBufferLength = dwSize;
	m_WaveHeader[m_nWaveOutToggle].dwLoops = 0;
	m_WaveHeader[m_nWaveOutToggle].dwFlags = 0;

    res = ::waveOutPrepareHeader(m_hWaveOut, &m_WaveHeader[m_nWaveOutToggle], sizeof(WAVEHDR)); 
	if ((res != MMSYSERR_NOERROR) || (m_WaveHeader[m_nWaveOutToggle].dwFlags != WHDR_PREPARED))
	{
		TRACE(_T("Sound Output Cannot PrepareHeader!\n"));
		return FALSE;
	}

	res = ::waveOutWrite(m_hWaveOut, &m_WaveHeader[m_nWaveOutToggle], sizeof(WAVEHDR));
	if (res != MMSYSERR_NOERROR) 
	{
		TRACE(_T("Sound Output Cannot Write To Buffer!\n"));
		return FALSE;
	}

	m_nWaveOutToggle = !m_nWaveOutToggle; 

	return TRUE;
}

void CVideoAviDoc::CPlayAudioFileThread::SetWaveFormat(LPWAVEFORMATEX pWaveFormat)
{
	if (m_pWaveFormat)
		delete [] m_pWaveFormat;

	if (pWaveFormat == NULL)
	{
		m_pWaveFormat = NULL;
		return;
	}

	int nWaveFormatSize;
	if (pWaveFormat->wFormatTag == WAVE_FORMAT_PCM)
		nWaveFormatSize = sizeof(WAVEFORMATEX);
	else
		nWaveFormatSize = sizeof(WAVEFORMATEX) + pWaveFormat->cbSize;
 
	m_pWaveFormat = (LPWAVEFORMATEX)new BYTE[nWaveFormatSize];
	memcpy(m_pWaveFormat, pWaveFormat, nWaveFormatSize);
}

CVideoAviDoc::CPlayAudioFileThread::CMixerOut::CMixerOut()
{
	m_hMixer =						NULL;
	m_hWndMixerCallback =			NULL;
	m_uiMixerID =					0xFFFFFFFF;

	m_dwMasterVolumeControlID =		0xFFFFFFFF;
	m_dwMasterVolumeControlMin =	0;
	m_dwMasterVolumeControlMax =	0;
	m_dwMasterMuteControlID		=	0xFFFFFFFF;
	m_dwMasterChannels =			0;

	m_dwWaveVolumeControlID =		0xFFFFFFFF;
	m_dwWaveVolumeControlMin =		0;
	m_dwWaveVolumeControlMax =		0;
	m_dwWaveMuteControlID =			0xFFFFFFFF;
	m_dwWaveChannels =				0;
}

CVideoAviDoc::CPlayAudioFileThread::CMixerOut::~CMixerOut()
{
	Close();
}

BOOL CVideoAviDoc::CPlayAudioFileThread::CMixerOut::Open(HWAVEOUT hWaveOut, HWND hWndCallBack/*=NULL*/)
{
	MMRESULT res;

	if (hWaveOut == NULL)
		return FALSE;

	res = ::mixerGetID((HMIXEROBJ)hWaveOut, &m_uiMixerID, MIXER_OBJECTF_HWAVEOUT);
	if (res != MMSYSERR_NOERROR)
	{
		TRACE(_T("Sound Output Mixer Is Not Available\n"));
		return FALSE;
	}

	MIXERCAPS MixerCaps;
	res = ::mixerGetDevCaps(m_uiMixerID, &MixerCaps, sizeof(MIXERCAPS));
	if (res != MMSYSERR_NOERROR)
	{
		TRACE(_T("Sound Output Mixer Cannot Get Mixer Capabilities\n"));
		return FALSE;
	}

	// Close It if Already Open
	if (m_hMixer)
		Close();

	// Open Mixer
	if (hWndCallBack == NULL)
		res = ::mixerOpen(&m_hMixer, m_uiMixerID, 0, 0L, MIXER_OBJECTF_MIXER | CALLBACK_NULL);
	else
		res = ::mixerOpen(&m_hMixer, m_uiMixerID, DWORD(hWndCallBack), 0L, MIXER_OBJECTF_MIXER | CALLBACK_WINDOW);
	if (res != MMSYSERR_NOERROR)
	{
		TRACE(_T("Sound Output Mixer Cannot Be Opened\n"));
		return FALSE;
	}

	// Store Callback Window Handle
	m_hWndMixerCallback = hWndCallBack;

	// Mixer Vars
	MIXERLINE mxl;
	MIXERLINECONTROLS mxlc;
	MIXERCONTROL mxc;
	mxl.cbStruct = sizeof(MIXERLINE);
	mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mxc.cbStruct = sizeof(MIXERCONTROL);

	// Find The Output Target
	mxl.dwSource = 0xFFFFFFF; // Not Used
	mxl.dwDestination = 0xFFFFFFF; // Not Used
	mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
	res = ::mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mxl, MIXER_GETLINEINFOF_COMPONENTTYPE);
	if (res != MMSYSERR_NOERROR)
	{
		TRACE(_T("Sound Output Cannot Get Line Information\n"));
		return FALSE;
	}
	DWORD dwDestination = mxl.dwDestination; 
	DWORD dwSourcesCount = mxl.cConnections;

	// Number Of Channels
	m_dwMasterChannels = mxl.cChannels;
	
	// Master Volume (Destination Control)
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(MIXERCONTROL);
	mxlc.pamxctrl = &mxc;
	res = ::mixerGetLineControls((HMIXEROBJ)m_hMixer, &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
	if (res == MMSYSERR_NOERROR)
	{
		m_dwMasterVolumeControlID = mxc.dwControlID;
		m_dwMasterVolumeControlMin = mxc.Bounds.dwMinimum;
		m_dwMasterVolumeControlMax = mxc.Bounds.dwMaximum;
	}

	// Master Mute (Destination Control)
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(MIXERCONTROL);
	mxlc.pamxctrl = &mxc;
	res = ::mixerGetLineControls((HMIXEROBJ)m_hMixer, &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
	if (res == MMSYSERR_NOERROR)
		m_dwMasterMuteControlID = mxc.dwControlID;

	for (int source = 0 ; source < (int)dwSourcesCount ; source++)
	{
		mxl.dwSource = source;
		mxl.dwDestination = dwDestination;
		res = ::mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mxl, MIXER_GETLINEINFOF_SOURCE);
		if (res != MMSYSERR_NOERROR)
		{
			TRACE(_T("Sound Input Cannot Get Line Information\n"));
			return FALSE;
		}

		if (mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT)
		{
			// Number Of Channels
			m_dwWaveChannels = mxl.cChannels;

			// Volume Source Control
			mxlc.dwLineID = mxl.dwLineID;
			mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
			mxlc.cControls = 1;
			mxlc.cbmxctrl = sizeof(MIXERCONTROL);
			mxlc.pamxctrl = &mxc;
			res = ::mixerGetLineControls((HMIXEROBJ)m_hMixer, &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
			if (res == MMSYSERR_NOERROR)
			{
				m_dwWaveVolumeControlID = mxc.dwControlID;
				m_dwWaveVolumeControlMin = mxc.Bounds.dwMinimum;
				m_dwWaveVolumeControlMax = mxc.Bounds.dwMaximum;
			}
			else
				return FALSE;

			// Mute Source Control
			mxlc.dwLineID = mxl.dwLineID;
			mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
			mxlc.cControls = 1;
			mxlc.cbmxctrl = sizeof(MIXERCONTROL);
			mxlc.pamxctrl = &mxc;
			res = ::mixerGetLineControls((HMIXEROBJ)m_hMixer, &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
			if (res == MMSYSERR_NOERROR)
			{
				m_dwWaveMuteControlID = mxc.dwControlID;
				return TRUE;
			}
			else
				return FALSE;
		}
	}

	return FALSE;
}

void CVideoAviDoc::CPlayAudioFileThread::CMixerOut::Close()
{
	if (m_hMixer)
	{
		::mixerClose(m_hMixer);
		m_hMixer = NULL;
	}
	m_hWndMixerCallback =		NULL;
	m_uiMixerID =				0xFFFFFFFF;
	m_dwMasterChannels =		0;
	m_dwWaveChannels =			0;
	m_dwMasterVolumeControlID =	0xFFFFFFFF;
	m_dwWaveVolumeControlID =	0xFFFFFFFF;
}

BOOL CVideoAviDoc::CPlayAudioFileThread::CMixerOut::GetMasterVolume(DWORD& dwVolumeLeft, DWORD& dwVolumeRight)
{
	MMRESULT res;
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(mxcd);
	
	if (m_hMixer == NULL)
		return FALSE;

	if (m_dwMasterVolumeControlID == 0xFFFFFFFF)
		return FALSE;
	
	mxcd.dwControlID = m_dwMasterVolumeControlID;
	mxcd.cChannels = m_dwMasterChannels;

	LPMIXERCONTROLDETAILS_UNSIGNED mxcd_u = new MIXERCONTROLDETAILS_UNSIGNED[mxcd.cChannels];
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mxcd.paDetails = mxcd_u;
	res = ::mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);
	if (MMSYSERR_NOERROR != res)
	{
		delete [] mxcd_u;
		return FALSE;
	}
	if (mxcd.cChannels == 1) // Mono
	{
		dwVolumeLeft = mxcd_u[0].dwValue;
		dwVolumeRight = mxcd_u[0].dwValue;
		
	}
	else if (mxcd.cChannels > 1) // Stereo or More Channels
	{
		dwVolumeLeft = mxcd_u[0].dwValue;
		dwVolumeRight = mxcd_u[1].dwValue;
	}
	delete [] mxcd_u;
	return TRUE;
}

BOOL CVideoAviDoc::CPlayAudioFileThread::CMixerOut::GetWaveVolume(DWORD& dwVolumeLeft, DWORD& dwVolumeRight)
{
	MMRESULT res;
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(mxcd);
	
	if (m_hMixer == NULL)
		return FALSE;

	if (m_dwWaveVolumeControlID == 0xFFFFFFFF)
		return FALSE;
	
	mxcd.dwControlID = m_dwWaveVolumeControlID;
	mxcd.cChannels = m_dwWaveChannels;

	LPMIXERCONTROLDETAILS_UNSIGNED mxcd_u = new MIXERCONTROLDETAILS_UNSIGNED[mxcd.cChannels];
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mxcd.paDetails = mxcd_u;
	res = ::mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);
	if (MMSYSERR_NOERROR != res)
	{
		delete [] mxcd_u;
		return FALSE;
	}
	else
	{
		if (mxcd.cChannels == 1) // Mono
		{
			dwVolumeLeft = mxcd_u[0].dwValue;
			dwVolumeRight = mxcd_u[0].dwValue;
			
		}
		else if (mxcd.cChannels > 1) // Stereo or More Channels
		{
			dwVolumeLeft = mxcd_u[0].dwValue;
			dwVolumeRight = mxcd_u[1].dwValue;
		}
		delete [] mxcd_u;
		return TRUE;
	}
}

BOOL CVideoAviDoc::CPlayAudioFileThread::CMixerOut::SetMasterVolume(DWORD dwVolumeLeft, DWORD dwVolumeRight)
{
	MMRESULT res;
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(mxcd);

	if (m_hMixer == NULL)
		return FALSE;

	if (m_dwMasterVolumeControlID == 0xFFFFFFFF)
		return FALSE;

	if ((dwVolumeLeft > m_dwMasterVolumeControlMax) ||
		(dwVolumeLeft < m_dwMasterVolumeControlMin) ||
		(dwVolumeRight > m_dwMasterVolumeControlMax) ||
		(dwVolumeRight < m_dwMasterVolumeControlMin))
		return FALSE;

	mxcd.dwControlID = m_dwMasterVolumeControlID;
	mxcd.cChannels = m_dwMasterChannels;

	LPMIXERCONTROLDETAILS_UNSIGNED mxcd_u = new MIXERCONTROLDETAILS_UNSIGNED[mxcd.cChannels];
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mxcd.paDetails = mxcd_u;
	res = ::mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);
	if (MMSYSERR_NOERROR != res) 
	{
		delete [] mxcd_u;
		return FALSE;
	}
	if (mxcd.cChannels == 1) // Mono
	{
		DWORD dwValueMono = (dwVolumeLeft + dwVolumeRight) / 2;
		mxcd_u[0].dwValue  = dwValueMono;
	}
	else if (mxcd.cChannels > 1) // Stereo or More Channels
	{
		mxcd_u[0].dwValue  = dwVolumeLeft;
		mxcd_u[1].dwValue  = dwVolumeRight;
	}
	res = ::mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);
	if (MMSYSERR_NOERROR != res)
	{
		delete [] mxcd_u;
		return FALSE;
	}
	else
	{
		delete [] mxcd_u;
		return TRUE;
	}
}

BOOL CVideoAviDoc::CPlayAudioFileThread::CMixerOut::SetWaveVolume(DWORD dwVolumeLeft, DWORD dwVolumeRight)
{
	MMRESULT res;
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(mxcd);

	if (m_hMixer == NULL)
		return FALSE;

	if (m_dwWaveVolumeControlID == 0xFFFFFFFF)
		return FALSE;

	if ((dwVolumeLeft > m_dwWaveVolumeControlMax) ||
		(dwVolumeLeft < m_dwWaveVolumeControlMin) ||
		(dwVolumeRight > m_dwWaveVolumeControlMax) ||
		(dwVolumeRight < m_dwWaveVolumeControlMin))
		return FALSE;

	mxcd.dwControlID = m_dwWaveVolumeControlID;
	mxcd.cChannels = m_dwWaveChannels;

	LPMIXERCONTROLDETAILS_UNSIGNED mxcd_u = new MIXERCONTROLDETAILS_UNSIGNED[mxcd.cChannels];
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mxcd.paDetails = mxcd_u;
	res = ::mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);
	if (MMSYSERR_NOERROR != res) 
	{
		delete [] mxcd_u;
		return FALSE;
	}
	if (mxcd.cChannels == 1) // Mono
	{
		DWORD dwValueMono = (dwVolumeLeft + dwVolumeRight) / 2;
		mxcd_u[0].dwValue  = dwValueMono;
	}
	else if (mxcd.cChannels > 1) // Stereo or More Channels
	{
		mxcd_u[0].dwValue  = dwVolumeLeft;
		mxcd_u[1].dwValue  = dwVolumeRight;
	}
	res = ::mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);
	if (MMSYSERR_NOERROR != res)
	{
		delete [] mxcd_u;
		return FALSE;
	}
	else
	{
		delete [] mxcd_u;
		return TRUE;
	}
}


BOOL CVideoAviDoc::CPlayAudioFileThread::CMixerOut::GetMasterMute(BOOL& bCheck)
{
	MMRESULT res;
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(mxcd);

	if (m_hMixer == NULL)
		return FALSE;

	if (m_dwMasterMuteControlID == 0xFFFFFFFF)
		return FALSE;
	
	mxcd.dwControlID = m_dwMasterMuteControlID;
	mxcd.cChannels = 1;
	
	MIXERCONTROLDETAILS_BOOLEAN mxcd_bool;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mxcd.paDetails = &mxcd_bool;
	res = ::mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);
	if (MMSYSERR_NOERROR != res)
		return FALSE;
	else
	{
		bCheck = mxcd_bool.fValue;
		return TRUE;
	}
}

BOOL CVideoAviDoc::CPlayAudioFileThread::CMixerOut::SetMasterMute(BOOL bCheck)
{
	MMRESULT res;
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(mxcd);

	if (m_hMixer == NULL)
		return FALSE;

	if (m_dwMasterMuteControlID == 0xFFFFFFFF)
		return FALSE;

	mxcd.dwControlID = m_dwMasterMuteControlID;
	mxcd.cChannels = 1;

	MIXERCONTROLDETAILS_BOOLEAN mxcd_bool = {bCheck};
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mxcd.paDetails = &mxcd_bool;
	res = ::mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);
	if (MMSYSERR_NOERROR != res)
		return FALSE;
	else
		return TRUE;
}

BOOL CVideoAviDoc::CPlayAudioFileThread::CMixerOut::GetWaveMute(BOOL& bCheck)
{
	MMRESULT res;
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(mxcd);

	if (m_hMixer == NULL)
		return FALSE;

	if (m_dwMasterMuteControlID == 0xFFFFFFFF)
		return FALSE;
	
	mxcd.dwControlID = m_dwWaveMuteControlID;
	mxcd.cChannels = 1;
	
	MIXERCONTROLDETAILS_BOOLEAN mxcd_bool;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mxcd.paDetails = &mxcd_bool;
	res = ::mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);
	if (MMSYSERR_NOERROR != res)
		return FALSE;
	else
	{
		bCheck = mxcd_bool.fValue;
		return TRUE;
	}
}

BOOL CVideoAviDoc::CPlayAudioFileThread::CMixerOut::SetWaveMute(BOOL bCheck)
{
	MMRESULT res;
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(mxcd);

	if (m_hMixer == NULL)
		return FALSE;

	if (m_dwWaveMuteControlID == 0xFFFFFFFF)
		return FALSE;

	mxcd.dwControlID = m_dwWaveMuteControlID;
	mxcd.cChannels = 1;

	MIXERCONTROLDETAILS_BOOLEAN mxcd_bool = {bCheck};
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mxcd.paDetails = &mxcd_bool;
	res = ::mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);
	if (MMSYSERR_NOERROR != res)
		return FALSE;
	else
		return TRUE;
}

CVideoAviDoc::CPlayVideoFileThread::CPlayVideoFileThread()
{
	m_pDoc = NULL;
	m_nPlaySpeedPercent = 100;
	m_nMilliSecondsCorrection = 0;
	m_nMilliSecondsCorrectionAvg = 0;
	m_uiTimerDelay = 0;
	m_dwDroppedFramesCount = 0;
	m_hPlaySyncEvent = NULL;
	m_bWaitingForStart = FALSE;
	m_hPlaySyncEventArray[0] = GetKillEvent();
	m_hPlaySyncEventArray[1] = NULL;
	m_hTimerEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEventArray[0] = GetKillEvent();
	m_hEventArray[1] = m_hTimerEvent;

	m_bDoFullScreenBlt = 0;
	m_bDoUpdateDoFullScreenBlt = FALSE;
	m_bDoSafePause = FALSE;
	m_dwDelayedRestartTimeout = 0;
	m_bDoSetSafePaused = FALSE;
	m_lSafePausedMsgId = 0;
	m_hSafePausedMsgWnd = NULL;
	m_wSafePausedMsgWParam = 0;
	m_lSafePausedMsgLParam = 0;
	m_dwSafePausedMsgSeq = 0;
	m_hDelayedRestartEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	::InitializeCriticalSection(&m_csSafePauseDelayedRestart);
}

CVideoAviDoc::CPlayVideoFileThread::~CPlayVideoFileThread() 
{
	Kill();
	::CloseHandle(m_hTimerEvent);
	m_hTimerEvent = NULL;
	::CloseHandle(m_hDelayedRestartEvent);
	m_hDelayedRestartEvent = NULL;
	::DeleteCriticalSection(&m_csSafePauseDelayedRestart);
}

void CVideoAviDoc::CPlayVideoFileThread::Rew()
{
	if (m_pDoc && m_pDoc->m_pAVIPlay)
	{
		CAVIPlay::CAVIVideoStream* pVideoStream = m_pDoc->m_pAVIPlay->GetVideoStream(m_pDoc->m_nActiveVideoStream);
		if (pVideoStream)
			pVideoStream->Rew();
	}
}

double CVideoAviDoc::CPlayVideoFileThread::GetFrameRate() const
{
	if (m_pDoc && m_pDoc->m_pAVIPlay)
	{
		CAVIPlay::CAVIVideoStream* pVideoStream = m_pDoc->m_pAVIPlay->GetVideoStream(m_pDoc->m_nActiveVideoStream);
		if (pVideoStream)
			return pVideoStream->GetFrameRate();
		else
			return 0.0;
	}
	else
		return 0.0;
}

BOOL CVideoAviDoc::CPlayVideoFileThread::GetFrameRate(DWORD* pdwRate, DWORD* pdwScale) const
{
	if (m_pDoc && m_pDoc->m_pAVIPlay)
	{
		CAVIPlay::CAVIVideoStream* pVideoStream = m_pDoc->m_pAVIPlay->GetVideoStream(m_pDoc->m_nActiveVideoStream);
		if (pVideoStream)
			return pVideoStream->GetFrameRate(pdwRate, pdwScale) ? TRUE : FALSE;
		else
			return FALSE;
	}
	else
		return FALSE;
}

void CVideoAviDoc::CPlayVideoFileThread::DxDrawGDIDisplay(	HWND hSafePausedMsgWnd,
															LONG lSafePausedMsgId,
															WPARAM wparam,
															LPARAM lparam)
{
	if (!DoFullScreenBlt() && m_pDoc->m_DxDraw.IsFullScreen())
	{
		SafePauseDelayedRestart(hSafePausedMsgWnd,
								lSafePausedMsgId,
								wparam,
								lparam,
								DXDRAW_GDIDISPLAY_SAFEPAUSED_TIMEOUT,
								DXDRAW_GDIDISPLAY_DELAYEDRESTART_TIMEOUT,
								TRUE);
	}
	else
	{
		::PostMessage(	hSafePausedMsgWnd,
						lSafePausedMsgId,
						wparam,
						lparam);
	}
}

void CVideoAviDoc::CPlayVideoFileThread::SafePauseDelayedRestart(HWND hSafePausedMsgWnd,
																 LONG lSafePausedMsgId,
																 WPARAM wparam,
																 LPARAM lparam,
																 DWORD dwSafePausedMsgTimeout,
																 DWORD dwDelayedRestartTimeout,
																 BOOL bDoUpdateDoFullScreenBlt)
{
	if (!IsAlive()) // Is Not Playing
	{
		// Set Full Screen Blt
		if (bDoUpdateDoFullScreenBlt)
			SetFullScreenBlt();

		// Post Message
		if ((hSafePausedMsgWnd != NULL) &&
			(lSafePausedMsgId != 0))
		{
			::PostMessage(	hSafePausedMsgWnd,
							lSafePausedMsgId,
							wparam,
							lparam);
		}
	}
	else
	{
		// Enter CS
		::EnterCriticalSection(&m_csSafePauseDelayedRestart);

		::ResetEvent(m_hDelayedRestartEvent);
		m_hDelayedRestartEventArray[0] = GetKillEvent();
		m_hDelayedRestartEventArray[1] = m_hDelayedRestartEvent;
		m_hSafePausedMsgWnd = hSafePausedMsgWnd;
		m_lSafePausedMsgId = lSafePausedMsgId;
		m_wSafePausedMsgWParam = wparam;
		m_lSafePausedMsgLParam = lparam;
		m_dwDelayedRestartTimeout = dwDelayedRestartTimeout;
		m_bDoUpdateDoFullScreenBlt = bDoUpdateDoFullScreenBlt;
		m_bDoSafePause = TRUE;
		m_bDoSetSafePaused = FALSE;
		m_dwSafePausedMsgSeq++;

		// Timeout Message
		if ((hSafePausedMsgWnd != NULL)			&&
			(lSafePausedMsgId != 0)				&&
			(dwSafePausedMsgTimeout != 0)		&&
			(dwSafePausedMsgTimeout != INFINITE))
		{
			CPostDelayedMessageThread::PostDelayedMessage(	m_pDoc->GetView()->GetSafeHwnd(),
															WM_SAFE_PAUSE_TIMEOUT,
															dwSafePausedMsgTimeout,
															(WPARAM)m_dwSafePausedMsgSeq,
															(LPARAM)bDoUpdateDoFullScreenBlt);
		}

		// Leave CS
		::LeaveCriticalSection(&m_csSafePauseDelayedRestart);
	}
}

void CVideoAviDoc::CPlayVideoFileThread::OnSafePauseTimeout(WPARAM wparam, LPARAM lparam)
{
	// Enter CS
	::EnterCriticalSection(&m_csSafePauseDelayedRestart);

	// If no new SafePauseDelayedRestart() called
	if (m_dwSafePausedMsgSeq == (DWORD)wparam)
	{	
		if ((m_hSafePausedMsgWnd != NULL) &&
			(m_lSafePausedMsgId != 0))
		{
			// Set Full Screen Blt
			if ((BOOL)lparam)
				SetFullScreenBlt();

			// Post Message
			::PostMessage(	m_hSafePausedMsgWnd,
							m_lSafePausedMsgId,
							m_wSafePausedMsgWParam,
							m_lSafePausedMsgLParam);
		}

		// Reset Flags
		m_bDoSafePause = FALSE;
		m_bDoSetSafePaused = FALSE;
		m_hSafePausedMsgWnd = NULL;
		m_lSafePausedMsgId = 0;
	}

	// Leave CS
	::LeaveCriticalSection(&m_csSafePauseDelayedRestart);
}

__forceinline BOOL CVideoAviDoc::CPlayVideoFileThread::OnSafePause()
{
	BOOL res = TRUE;

	// Enter CS
	::EnterCriticalSection(&m_csSafePauseDelayedRestart);

	if (m_bDoSetSafePaused)
	{
		// Post Message
		if ((m_hSafePausedMsgWnd != NULL) &&
			(m_lSafePausedMsgId != 0))
		{
			::PostMessage(	m_hSafePausedMsgWnd,
							m_lSafePausedMsgId,
							m_wSafePausedMsgWParam,
							m_lSafePausedMsgLParam);
		}

		// Reset Flags
		m_bDoSetSafePaused = FALSE;
		m_hSafePausedMsgWnd = NULL;
		m_lSafePausedMsgId = 0;

		// Restart
		DWORD Event = ::WaitForMultipleObjects(	2,
												m_hDelayedRestartEventArray,
												FALSE,
												m_dwDelayedRestartTimeout);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :		res = FALSE;
										break;

			// Restart Event
			case WAIT_OBJECT_0 + 1 :	break;

			// Timeout
			case WAIT_TIMEOUT :			break;

			default :					res = FALSE;
										break;
		}
	}
	else if (m_bDoSafePause)
	{
		m_bDoSafePause = FALSE;
		m_bDoSetSafePaused = TRUE;
		if (m_bDoUpdateDoFullScreenBlt)
			SetFullScreenBlt();
	}

	// Leave CS
	::LeaveCriticalSection(&m_csSafePauseDelayedRestart);

	return res;
}

void CVideoAviDoc::CPlayVideoFileThread::OnExit()
{
	// Reset Waiting For Start Flag
	::EnterCriticalSection(&m_pDoc->m_csPlayWaitingForStart);
	m_bWaitingForStart = FALSE;
	::LeaveCriticalSection(&m_pDoc->m_csPlayWaitingForStart);

	// Get Current Frame Pos
	int nCurrentFramePos = 0;
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pDoc->m_pAVIPlay->GetVideoStream(m_pDoc->m_nActiveVideoStream);
	if (pVideoStream)
		nCurrentFramePos = pVideoStream->GetCurrentFramePos();
	if (nCurrentFramePos < 0)
		nCurrentFramePos = 0;

	// Display Frame / Update View
	if (pVideoStream)
	{
		m_pDoc->DisplayFrame(	nCurrentFramePos,
								THREAD_SAFE_UPDATEWINDOWSIZES_DELAY);
	}
	else
	{
		CPostDelayedMessageThread::PostDelayedMessage(
							m_pDoc->GetView()->GetSafeHwnd(),
							WM_THREADSAFE_UPDATEWINDOWSIZES,
							THREAD_SAFE_UPDATEWINDOWSIZES_DELAY,
							(WPARAM)(UPDATEWINDOWSIZES_INVALIDATE),
							(LPARAM)0);
	}

	::CoUninitialize();
}

__forceinline BOOL CVideoAviDoc::CPlayVideoFileThread::OnSync()
{
	if (m_hPlaySyncEvent)
	{
		do
		{
			// Sync
			DWORD Event = ::WaitForMultipleObjects(2, m_hPlaySyncEventArray, FALSE, 50);
			switch (Event)
			{
				// Shutdown Event
				case WAIT_OBJECT_0 :		return FALSE;

				// Sync Event
				case WAIT_OBJECT_0 + 1 :	::EnterCriticalSection(&m_pDoc->m_csPlayWaitingForStart);
											m_bWaitingForStart = FALSE;
											::LeaveCriticalSection(&m_pDoc->m_csPlayWaitingForStart);
											break;

				// Timeout
				case WAIT_TIMEOUT :			break;

				default:					return FALSE;
			}

			// Call OnSafePause() to eventually
			// post a message
			if (!OnSafePause())
				return FALSE;
		}
		while (m_bWaitingForStart);
	}

	return TRUE;
}

int CVideoAviDoc::CPlayVideoFileThread::Work() 
{
#ifdef _DEBUG
	CString sTrace;
#endif
	DWORD Event;
	int nFrameTime;
	UINT uiTimerId;
	BOOL bFirstPlayedFrame;
	int nTimeCorrection;
	int nOldMilliSecondsCorrectionAvg;
	int nWaitForCorrectionCountDown;
	BOOL bDoRewind;
	BOOL bDxDraw = FALSE;
	BOOL bChangingSpeed = FALSE;

	if (!m_pDoc)
		return 0;

	if (!m_pDoc->m_pAVIPlay)
		return 0;

	// Get Video Stream
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pDoc->m_pAVIPlay->GetVideoStream(m_pDoc->m_nActiveVideoStream);
	if (!pVideoStream)
		return 0;

	// Increase Timer Resolution to 1 ms if possible
	TIMECAPS tc;
	UINT wTimerRes;
	if (::timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR) 
		return 0;
	wTimerRes = MIN(MAX(tc.wPeriodMin, 1), tc.wPeriodMax); // 1 millisecond target resolution
	::timeBeginPeriod(wTimerRes);

	// Init COM
	::CoInitialize(NULL);

	// Init Audio Sample Offset
	LONGLONG llAudioSampleOffset = 0;
	if (m_pDoc->m_PlayAudioFileThread.IsOpen())
		llAudioSampleOffset = m_pDoc->m_PlayAudioFileThread.GetNextSamplePos();
	LONGLONG llLastAudioSamplePos = 0;

	// Init Video Frame Offset
	int nVideoFrameOffset = pVideoStream->GetNextFramePos();

	// Wait For Sync.
	if (m_hPlaySyncEvent)
	{
		::EnterCriticalSection(&m_pDoc->m_csPlayWaitingForStart);
		if (!m_pDoc->m_PlayAudioFileThread.IsWaitingForStart())
		{
			m_bWaitingForStart = TRUE;
			::ResetEvent(m_hPlaySyncEvent);
		}
		::LeaveCriticalSection(&m_pDoc->m_csPlayWaitingForStart);
	}

	// No Rewind
	bDoRewind = FALSE;

	// Loop
	do
	{
		// Init Vars
		nFrameTime = 0;
		uiTimerId = 0;
		bFirstPlayedFrame = TRUE;
		nWaitForCorrectionCountDown = 2; // Loop at least once to be able to make corrections!

		// Sync with Audio
		if (m_hPlaySyncEvent && !m_bWaitingForStart)
			::SetEvent(m_hPlaySyncEvent);
		if (!OnSync())
		{
			::timeEndPeriod(wTimerRes);
			OnExit();
			return 0;
		}

		// Rewind?
		if (bDoRewind)
			Rew();

		// Wait if Audio Leads Video at Start-Up
		int nVideoPos = Round(pVideoStream->GetNextFramePos() * 1000.0 / m_pDoc->GetPlayFrameRate());
		if (nVideoPos < m_pDoc->m_nAudioLeadsVideoShift)
			::Sleep(m_pDoc->m_nAudioLeadsVideoShift - nVideoPos);

		// Init Start System Up-Time
		DWORD dwInitSystemUpTime = ::timeGetTime();
	
		// Get Frames
		while (TRUE)
		{
			// Do Pause a Moment?
			if (!OnSafePause())
			{
				::timeKillEvent(uiTimerId);
				::timeEndPeriod(wTimerRes);
				OnExit();
				return 0;
			}

			// Enter DirectDraw CS
			if (m_pDoc->m_DxDraw.HasDxDraw())
				m_pDoc->m_DxDraw.EnterCS();

			// Get Frame
			bDxDraw = m_pDoc->m_bUseDxDraw && m_pDoc->m_DxDraw.IsInit();
			if (!bDxDraw)
			{
				::EnterCriticalSection(&m_pDoc->m_csDib);
				if (!pVideoStream->GetFrame(m_pDoc->m_pDib))
				{
					::LeaveCriticalSection(&m_pDoc->m_csDib);
					if (m_pDoc->m_DxDraw.HasDxDraw())
						m_pDoc->m_DxDraw.LeaveCS();
					break;
				}
				else
					::LeaveCriticalSection(&m_pDoc->m_csDib);
			}
			else
			{
				if (!pVideoStream->GetFrame(&m_pDoc->m_DxDraw,
											m_pDoc->GetView()->m_UserZoomRect))
				{
					if (m_pDoc->m_DxDraw.HasDxDraw())
						m_pDoc->m_DxDraw.LeaveCS();
					break;
				}
			}

			// Count-Up
			m_pDoc->m_dwFrameCountUp++;

			// Changing speed mode?
			if (m_nPlaySpeedPercent != 100)
				bChangingSpeed = TRUE;
	
			// The Frame Time, Reference Threshold Time in ms
			// for timer adjust and Count-Down Amount.
			int nNewFrameTime = Round(100000.0 / (m_nPlaySpeedPercent * m_pDoc->GetPlayFrameRate()));
			if (nNewFrameTime <= 0)
				nNewFrameTime = 1;
			if (!bFirstPlayedFrame && nFrameTime != nNewFrameTime)
			{
				bFirstPlayedFrame = TRUE;
				if (bChangingSpeed)
				{
					nVideoFrameOffset = pVideoStream->GetNextFramePos();
					dwInitSystemUpTime = ::timeGetTime();
				}
			}
			nFrameTime = nNewFrameTime;
			int nRefThresholdTimeBig = nFrameTime + (nFrameTime>>2);
			int nRefThresholdTimeSmall = MIN(VIDEO_PLAYER_THRESHOLD_TIME, nFrameTime>>2); 

			// Set the Timer
			if (bFirstPlayedFrame)
			{
				if (uiTimerId) 
					::timeKillEvent(uiTimerId);
				uiTimerId = ::timeSetEvent(	m_uiTimerDelay = nFrameTime,
											0, (LPTIMECALLBACK)m_hTimerEvent, 0,
											TIME_PERIODIC | TIME_CALLBACK_EVENT_SET | TIME_KILL_SYNCHRONOUS);
				m_nMilliSecondsCorrection = 0;
				m_nMilliSecondsCorrectionAvg = 0;
				nOldMilliSecondsCorrectionAvg = 0;
				nTimeCorrection = 0;
				bFirstPlayedFrame = FALSE;
			}	
			else
			{
				// Dec. Correction Countdown
				if (nWaitForCorrectionCountDown > 0)
					nWaitForCorrectionCountDown--;

				// Average Millisenconds Correction
				m_nMilliSecondsCorrectionAvg = (m_nMilliSecondsCorrection + m_nMilliSecondsCorrectionAvg)>>2;

				// Video Really To Fast -> Wait
				if ((nWaitForCorrectionCountDown == 0) &&
					(m_nMilliSecondsCorrectionAvg >= nRefThresholdTimeBig))
				{
					if (uiTimerId) 
						::timeKillEvent(uiTimerId);
					uiTimerId = ::timeSetEvent(	m_uiTimerDelay = (nFrameTime + m_nMilliSecondsCorrectionAvg),
												0, (LPTIMECALLBACK)m_hTimerEvent, 0,
												TIME_PERIODIC | TIME_CALLBACK_EVENT_SET | TIME_KILL_SYNCHRONOUS);
					nWaitForCorrectionCountDown = 4;
					bFirstPlayedFrame = TRUE;
				}
				// Video Really Slow -> Drop Frame(s)
				else if ((nWaitForCorrectionCountDown == 0) &&
						(m_nMilliSecondsCorrectionAvg <= - nRefThresholdTimeBig))
				{
					// Init vars
					int nSkipCount = (-m_nMilliSecondsCorrectionAvg) / nFrameTime;
					int nRemainingTime = (-m_nMilliSecondsCorrectionAvg) - nSkipCount * nFrameTime;

					// Skip Frames
					if (!pVideoStream->SkipFrame(nSkipCount))
					{
						if (m_pDoc->m_DxDraw.HasDxDraw())
							m_pDoc->m_DxDraw.LeaveCS();
						break;
					}
					m_dwDroppedFramesCount += (DWORD)nSkipCount;

					// Wait For The Remaining Time
					if (nRemainingTime <= 0)
						nRemainingTime = 1;
					if (uiTimerId) 
						::timeKillEvent(uiTimerId);
					uiTimerId = ::timeSetEvent(	m_uiTimerDelay = nRemainingTime,
												0, (LPTIMECALLBACK)m_hTimerEvent, 0,
												TIME_PERIODIC | TIME_CALLBACK_EVENT_SET | TIME_KILL_SYNCHRONOUS);

					// Come back faster if we skipped many frames!
					nWaitForCorrectionCountDown = 4 / nSkipCount;
					bFirstPlayedFrame = TRUE;
				}
				// Video To Fast
				else if ((nWaitForCorrectionCountDown == 0) &&
						(m_nMilliSecondsCorrectionAvg > nRefThresholdTimeSmall) &&
						(m_nMilliSecondsCorrectionAvg >= nOldMilliSecondsCorrectionAvg))
				{
					nTimeCorrection++;
					if (uiTimerId) 
						::timeKillEvent(uiTimerId);
					uiTimerId = ::timeSetEvent(	m_uiTimerDelay = (nFrameTime + nTimeCorrection),
												0, (LPTIMECALLBACK)m_hTimerEvent, 0,
												TIME_PERIODIC | TIME_CALLBACK_EVENT_SET | TIME_KILL_SYNCHRONOUS);
					nWaitForCorrectionCountDown = 8;
					nOldMilliSecondsCorrectionAvg = m_nMilliSecondsCorrectionAvg;
				}
				// Video To Slow
				else if ((nWaitForCorrectionCountDown == 0) &&
						(m_nMilliSecondsCorrectionAvg < -nRefThresholdTimeSmall) &&
						(m_nMilliSecondsCorrectionAvg <= nOldMilliSecondsCorrectionAvg))
				{
					nTimeCorrection--;
					if (uiTimerId) 
						::timeKillEvent(uiTimerId);
					if ((nFrameTime + nTimeCorrection) <= 0)
					{
						nTimeCorrection++;
						uiTimerId = ::timeSetEvent(	m_uiTimerDelay = 1,
													0, (LPTIMECALLBACK)m_hTimerEvent, 0,
													TIME_PERIODIC | TIME_CALLBACK_EVENT_SET | TIME_KILL_SYNCHRONOUS);
					}
					else
					{
						uiTimerId = ::timeSetEvent(	m_uiTimerDelay = (nFrameTime + nTimeCorrection),
													0, (LPTIMECALLBACK)m_hTimerEvent, 0,
													TIME_PERIODIC | TIME_CALLBACK_EVENT_SET | TIME_KILL_SYNCHRONOUS);
					}
					nWaitForCorrectionCountDown = 8;
					nOldMilliSecondsCorrectionAvg = m_nMilliSecondsCorrectionAvg;
				}
			}

			// Display Frame
			if (bDxDraw)
				m_pDoc->GetView()->Draw();
			else
				::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
								WM_THREADSAFE_UPDATEWINDOWSIZES,
								(WPARAM)(UPDATEWINDOWSIZES_INVALIDATE),
								(LPARAM)0);

			// Leave DirectDraw CS
			if (m_pDoc->m_DxDraw.HasDxDraw())
				m_pDoc->m_DxDraw.LeaveCS();
			
			// Sync with Audio
			if (!bChangingSpeed && m_pDoc->m_PlayAudioFileThread.IsOpen())
			{
				MMTIME mmTime;
				mmTime.wType = TIME_SAMPLES;
				mmTime.u.ms = 0;
				if (m_pDoc->m_PlayAudioFileThread.IsResetting() ||
					m_pDoc->m_PlayAudioFileThread.IsSleepingVideoLeadsAudio())
				{
					if (m_nMilliSecondsCorrection != 0)
						bFirstPlayedFrame = TRUE;
					m_nMilliSecondsCorrection = 0;
				}
				else
				{	
					// waveOutGetPosition wraps-over at 2^27 samples and not at 2^32!
					MMRESULT res = ::waveOutGetPosition(m_pDoc->m_PlayAudioFileThread.GetWaveHandle(), &mmTime, sizeof(MMTIME));
					if (res == MMSYSERR_NOERROR)
					{
						int nShiftValue = m_pDoc->m_nAudioLeadsVideoShift;

						// 2^27 Wrap-Around Happened?
						if (llLastAudioSamplePos > (LONGLONG)mmTime.u.sample)
							llAudioSampleOffset += (1 << 27);
						llLastAudioSamplePos = (LONGLONG)mmTime.u.sample;
						m_nMilliSecondsCorrection = (int)
														(
															( 
																(
																	((double)pVideoStream->GetCurrentFramePos())
																	* 1000.0 / m_pDoc->GetPlayFrameRate()
																)
																+
																(double)nShiftValue
															)
															-
															(
																(
																	(double)mmTime.u.sample +
																	(double)llAudioSampleOffset
																)
																* 1000.0 / (double)m_pDoc->m_PlayAudioFileThread.GetSampleRate()
															)
														);
					}
					else
						m_nMilliSecondsCorrection = 0;
				}
			}

			// Wait For One Frame Time +/- Corrections
			int nWait = nFrameTime + m_nMilliSecondsCorrection;
			if (nWait < 0) // If Video is Slow, go to next Frame!
				nWait = 0;
			Event = ::WaitForMultipleObjects(2, m_hEventArray, FALSE, (uiTimerId == 0) ? nWait : INFINITE);
			switch (Event)
			{
				// Shutdown Event
				case WAIT_OBJECT_0 :	::timeKillEvent(uiTimerId);
										::timeEndPeriod(wTimerRes);
										OnExit();
										return 0;

				// Timer Event
				case WAIT_OBJECT_0 + 1:	::ResetEvent(m_hTimerEvent);
										break;

				// Timeout
				case WAIT_TIMEOUT :		break;

				default :				::timeKillEvent(uiTimerId);
										::timeEndPeriod(wTimerRes);
										OnExit();
										return 0;
			}

			// Sync With System Up-Time
			if (bChangingSpeed || !m_pDoc->m_PlayAudioFileThread.IsOpen())
			{
				DWORD dwCurrentSystemUpTime = ::timeGetTime();
				int nElapsedSystemTime = dwCurrentSystemUpTime - dwInitSystemUpTime; // No Wrap Around Problem, the difference is always positive!
				int nElapsedFrameTime = Round(((pVideoStream->GetNextFramePos()
												- nVideoFrameOffset) * 100000.0) /
												(m_nPlaySpeedPercent * m_pDoc->GetPlayFrameRate()));
				m_nMilliSecondsCorrection = nElapsedFrameTime - nElapsedSystemTime;
			}
		}
		
		::EnterCriticalSection(&m_pDoc->m_csPlayWaitingForStart);
		if (m_pDoc->IsLoop())
		{
			llAudioSampleOffset = 0;
			llLastAudioSamplePos = 0;
			nVideoFrameOffset = 0;
			bDoRewind = TRUE;
			if (m_hPlaySyncEvent)
			{
				if (!m_pDoc->m_PlayAudioFileThread.IsWaitingForStart())
				{
					// Read frame to Dib while waiting that audio finishes,
					// this because RestoreFrame() may be called
					// in the mean time!
					if (bDxDraw)
					{
						::EnterCriticalSection(&m_pDoc->m_csDib);
						pVideoStream->GetFrameAt(m_pDoc->m_pDib,
												pVideoStream->GetCurrentFramePos());
						::LeaveCriticalSection(&m_pDoc->m_csDib);
					}
					m_bWaitingForStart = TRUE;
					::ResetEvent(m_hPlaySyncEvent);
				}
			}
		}
		::LeaveCriticalSection(&m_pDoc->m_csPlayWaitingForStart);

		// Kill Event Timer
		::timeKillEvent(uiTimerId);
	}
	while (m_pDoc->IsLoop());

	// Exit
	::timeEndPeriod(wTimerRes);
	OnExit();

	return 0;
}

int CVideoAviDoc::CProcessingThread::Work() 
{
	if (!m_pDoc)
		return 0;

	::CoInitialize(NULL);

	// No Error
	m_pDoc->m_sProcessingError = _T("");

	// Store Old Rect
	CRect OldDocRect = m_pDoc->m_DocRect;

	// Call Processing Function
	BOOL res = m_pFunct->Do();

	::CoUninitialize();

	// On error or on exit: Restore Frame / Update Window
	if (!res)
	{
		if (m_pDoc->m_pAVIPlay &&
			m_pDoc->m_pAVIPlay->HasVideo() &&
			(m_pDoc->m_nActiveVideoStream >= 0))
		{
			m_pDoc->RestoreFrame(THREAD_SAFE_UPDATEWINDOWSIZES_DELAY);
		}
		else
		{
			CPostDelayedMessageThread::PostDelayedMessage(
									m_pDoc->GetView()->GetSafeHwnd(),
									WM_THREADSAFE_UPDATEWINDOWSIZES,
									THREAD_SAFE_UPDATEWINDOWSIZES_DELAY,
									(WPARAM)(UPDATEWINDOWSIZES_INVALIDATE),
									(LPARAM)0);
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CVideoAviDoc Functions

CVideoAviDoc::CVideoAviDoc()
{	
	// General Vars Init
	m_nAudioLeadsVideoShift = 0;
	m_nAudioLeadsVideoShiftInitValue = 0;
	m_pView = NULL;
	m_pFrame = NULL;
	m_bAboutToStopAviPlay = FALSE;
	m_pAVIPlay = NULL;
	m_bLoop = 0;
	m_dwPlayRate = 1;
	m_dwPlayScale = 1;
	m_nActiveVideoStream = -1;
	m_nActiveAudioStream = -1;
	m_pAviInfoDlg = NULL;
	m_pOutVolDlg = NULL;
	m_pAudioVideoShiftDlg = NULL;
	m_pPlayerToolBarDlg = NULL;
	m_bUseDxDraw = TRUE;
	m_bForceRgb = TRUE;
	m_bNoDrawing = FALSE;
	m_bAboutToRestoreFrame = 0;
	m_nEndThumbTrackRetryCountDown = 0;
	m_dwFrameCountUp = 0U;
	m_pAudioCompressorWaveFormat = NULL;
	m_PrevUserZoomRect = CRect(0,0,0,0);
	m_bAVCodecPriority = true;

	// Threads Init
	m_PlayAudioFileThread.SetDoc(this);
	m_PlayVideoFileThread.SetDoc(this);
	m_ProcessingThread.SetDoc(this);
	m_SaveAsProcessing.SetDoc(this);
	m_ExtractframesProcessing.SetDoc(this);
	m_FileMergeAsProcessing.SetDoc(this);
	m_ShrinkDocToProcessing.SetDoc(this);
	m_sProcessingError = _T("");

	// Init Player Loop Sync. Audio / Video
	::InitializeCriticalSection(&m_csPlayWaitingForStart);

	// Audio / Video Play Sync
	m_hPlaySyncEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	// Reset Percent Done
	ResetPercentDone();

	// Load the Settings
	LoadSettings();

	// If no DirectX make sure m_bUseDxDraw is not set
	if (!m_DxDraw.HasDxDraw())
		m_bUseDxDraw = FALSE;
}

CVideoAviDoc::~CVideoAviDoc()
{
	if (m_pAudioCompressorWaveFormat)
	{
		delete [] m_pAudioCompressorWaveFormat;
		m_pAudioCompressorWaveFormat = NULL;
	}

	if (m_pAVIPlay)
	{
		delete m_pAVIPlay;
		m_pAVIPlay = NULL;
	}

	::DeleteCriticalSection(&m_csPlayWaitingForStart);
	::CloseHandle(m_hPlaySyncEvent);
	m_hPlaySyncEvent = NULL;
}

BOOL CVideoAviDoc::IsAVI(LPCTSTR szFileName)
{
	return ((::GetFileExt(szFileName) == _T(".avi"))	||
			(::GetFileExt(szFileName) == _T(".divx")));
}

void CVideoAviDoc::CloseDocument()
{
	GetFrame()->PostMessage(WM_CLOSE, 0, 0);
}

void CVideoAviDoc::CloseDocumentForce()
{
	SetModifiedFlag(FALSE);
	GetFrame()->PostMessage(WM_CLOSE, 0, 0);
}

void CVideoAviDoc::SetDocumentTitle()
{
	CString strInfo(_T(""));
	CString strAudioInfo(_T(""));
	CString strVideoInfo(_T(""));
	CString sShortFileName;

	int index;
	if ((index = m_sFileName.ReverseFind(_T('\\'))) >= 0)
		sShortFileName = m_sFileName.Right(m_sFileName.GetLength() - index - 1);
	else
		sShortFileName = m_sFileName;

	if (m_pAVIPlay)
	{
		if (m_pAVIPlay->HasVideo())
		{
			strInfo.Format(
				_T(" , %dx%d , %.3ffps"),
				m_DocRect.Width(), 
				m_DocRect.Height(),
				GetPlayFrameRate());
		}

		CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
		if (pVideoStream)
		{
			strVideoInfo = _T(" , ") + CDib::GetCompressionName(pVideoStream->GetFormat(true));
			strInfo += strVideoInfo;
		}

		CAVIPlay::CAVIAudioStream* pAudioStream = m_pAVIPlay->GetAudioStream(m_nActiveAudioStream);
		if (pAudioStream)
		{
			if (CAVIPlay::GetWaveFormatTagString(pAudioStream->GetFormatTag(true)) != _T(""))
			{
				if (pAudioStream->GetFormatTag(true) == WAVE_FORMAT_MPEGLAYER3)
				{
					if (pAudioStream->IsVBR())
					{
						strAudioInfo.Format(_T(" , Mp3 VBR (%i samples/chunk)"),
														pAudioStream->GetVBRSamplesPerChunk());
					}
					else
						strAudioInfo.Format(_T(" , Mp3 CBR"));
				}
				else if (pAudioStream->GetFormatTag(true) == WAVE_FORMAT_MPEG)
				{
					if (pAudioStream->IsVBR())
					{
						if (pAudioStream->GetMpegAudioLayer() == 2)
						{
							strAudioInfo.Format(_T(" , Mp2 VBR (%i samples/chunk)"),
														pAudioStream->GetVBRSamplesPerChunk());
						}
						else if (pAudioStream->GetMpegAudioLayer() == 1)
						{
							strAudioInfo.Format(_T(" , Mp1 VBR (%i samples/chunk)"),
														pAudioStream->GetVBRSamplesPerChunk());
						}
						else
						{
							strAudioInfo.Format(_T(" , %s"),
								CAVIPlay::GetWaveFormatTagString(pAudioStream->GetFormatTag(true)));
						}
					}
					else
					{
						if (pAudioStream->GetMpegAudioLayer() == 2)
							strAudioInfo.Format(_T(" , Mp2 CBR"));
						else if (pAudioStream->GetMpegAudioLayer() == 1)
							strAudioInfo.Format(_T(" , Mp1 CBR"));
						else
						{
							strAudioInfo.Format(_T(" , %s"),
								CAVIPlay::GetWaveFormatTagString(pAudioStream->GetFormatTag(true)));
						}
					}
				}
				else if (pAudioStream->GetFormatTag(true) == WAVE_FORMAT_PCM)
					strAudioInfo.Format(_T(" , Uncompressed Audio"));
				else
					strAudioInfo.Format(_T(" , %s"),
						CAVIPlay::GetWaveFormatTagString(pAudioStream->GetFormatTag(true)));	
			}
			else
				strAudioInfo.Format(_T(" , Audio 0x%04x"), pAudioStream->GetFormatTag(true));

			strInfo += strAudioInfo;
		}
	}
		
	if (IsModified())
		strInfo += _T(" *");

	CDocument::SetTitle(sShortFileName + strInfo);
}

void CVideoAviDoc::LoadSettings()
{
	CWinApp* pApp = ::AfxGetApp();
	CString sSection;

	// Doc settings
	sSection = _T("VideoAviDoc");
	m_bTimePositionShow = (BOOL) pApp->GetProfileInt(sSection, _T("TimePositionShow"), FALSE);
	m_bUseDxDraw = (BOOL) pApp->GetProfileInt(sSection, _T("UseDxDraw"), TRUE);
	m_bForceRgb = (BOOL) pApp->GetProfileInt(sSection, _T("ForceRgb"), TRUE);
	m_dwPlayAudioDeviceID = (DWORD) pApp->GetProfileInt(sSection, _T("AudioPlayDeviceID"), 0);
	
	m_dwVideoCompressorFourCC = (DWORD) pApp->GetProfileInt(sSection, _T("VideoCompressorFourCC"), DEFAULT_VIDEO_FOURCC);
	m_fVideoCompressorQuality = (float) pApp->GetProfileInt(sSection, _T("VideoCompressorQuality"), (int)DEFAULT_VIDEO_QUALITY);
	m_nVideoCompressorKeyframesRate = (int) pApp->GetProfileInt(sSection, _T("VideoCompressorKeyframesRate"), DEFAULT_KEYFRAMESRATE);

	if (m_pAudioCompressorWaveFormat)
		delete [] m_pAudioCompressorWaveFormat;
	UINT uiSize = 0;
	pApp->GetProfileBinary(sSection, _T("AudioCompressorWaveFormat"), (LPBYTE*)&m_pAudioCompressorWaveFormat, &uiSize);
	if (m_pAudioCompressorWaveFormat == NULL || uiSize != sizeof(WAVEFORMATEX)) // Default Audio: Mono , 11025 Hz , 8 bits
	{
		// Make Sure Nothing Has Been Allocated!
		if (m_pAudioCompressorWaveFormat)
			delete [] m_pAudioCompressorWaveFormat;
		m_pAudioCompressorWaveFormat = (WAVEFORMATEX*) new BYTE[sizeof(WAVEFORMATEX)];
		m_pAudioCompressorWaveFormat->wFormatTag = WAVE_FORMAT_PCM;
		m_pAudioCompressorWaveFormat->nChannels = 1;
		m_pAudioCompressorWaveFormat->nSamplesPerSec = 11025;
		m_pAudioCompressorWaveFormat->nAvgBytesPerSec = 11025;
		m_pAudioCompressorWaveFormat->nBlockAlign = 1;
		m_pAudioCompressorWaveFormat->wBitsPerSample = 8;
		m_pAudioCompressorWaveFormat->cbSize = 0;
	}

	m_bAVCodecPriority = pApp->GetProfileInt(sSection, _T("AVCodecPriority"), true) ? true : false;

	// View settings
	sSection = _T("VideoAviView");
	m_PrevUserZoomRect.left   = (LONG) pApp->GetProfileInt(sSection, _T("UserZoomRectLeft"), 0);
	m_PrevUserZoomRect.top    = (LONG) pApp->GetProfileInt(sSection, _T("UserZoomRectTop"), 0);
	m_PrevUserZoomRect.right  = (LONG) pApp->GetProfileInt(sSection, _T("UserZoomRectRight"), 0);
	m_PrevUserZoomRect.bottom = (LONG) pApp->GetProfileInt(sSection, _T("UserZoomRectBottom"), 0);
}

void CVideoAviDoc::SaveSettings()
{
	CWinApp* pApp = ::AfxGetApp();
	CString sSection;

	// Doc settings
	sSection = _T("VideoAviDoc");
	pApp->WriteProfileInt(sSection, _T("TimePositionShow"), m_bTimePositionShow);
	pApp->WriteProfileInt(sSection, _T("UseDxDraw"), m_bUseDxDraw);
	pApp->WriteProfileInt(sSection, _T("ForceRgb"), m_bForceRgb);
	pApp->WriteProfileInt(sSection, _T("AudioPlayDeviceID"), m_dwPlayAudioDeviceID);
		
	pApp->WriteProfileInt(sSection, _T("VideoCompressorFourCC"), m_dwVideoCompressorFourCC);
	pApp->WriteProfileInt(sSection, _T("VideoCompressorQuality"), (int)m_fVideoCompressorQuality);
	pApp->WriteProfileInt(sSection, _T("VideoCompressorKeyframesRate"), m_nVideoCompressorKeyframesRate);
		
	if (m_pAudioCompressorWaveFormat)
		pApp->WriteProfileBinary(sSection, _T("AudioCompressorWaveFormat"), (LPBYTE)m_pAudioCompressorWaveFormat, sizeof(WAVEFORMATEX));
	pApp->WriteProfileInt(sSection, _T("AVCodecPriority"), (int)m_bAVCodecPriority);

	// View settings
	sSection = _T("VideoAviView");
	pApp->WriteProfileInt(sSection, _T("UserZoomRectLeft"), m_PrevUserZoomRect.left);
	pApp->WriteProfileInt(sSection, _T("UserZoomRectTop"), m_PrevUserZoomRect.top);
	pApp->WriteProfileInt(sSection, _T("UserZoomRectRight"), m_PrevUserZoomRect.right);
	pApp->WriteProfileInt(sSection, _T("UserZoomRectBottom"), m_PrevUserZoomRect.bottom);
}

__forceinline BOOL CVideoAviDoc::GetPlayFrameRate(DWORD* pdwRate, DWORD* pdwScale) const
{
	if (pdwRate)
		*pdwRate = m_dwPlayRate;
	if (pdwScale)
		*pdwScale = m_dwPlayScale;
	return (pdwRate || pdwScale);
}

BOOL CVideoAviDoc::SaveAs(CString sDlgTitle/*=_T("")*/) 
{
	BOOL res = FALSE;
	BOOL bSaveCopyAs;
	TCHAR FileName[MAX_PATH] = _T("");
	_tcscpy(FileName, m_sFileName);
	CSaveFileDlg dlgFile(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, GetView());
	TCHAR defext[10] = _T("");
	LPTSTR lpPos = _tcsrchr(FileName, _T('.'));
	if (lpPos != NULL)
		_tcscpy(defext, lpPos+1);
	dlgFile.m_ofn.lpstrFile = FileName;
	dlgFile.m_ofn.nMaxFile = MAX_PATH;
	dlgFile.m_ofn.lpstrDefExt = defext;
	dlgFile.m_ofn.lpstrFilter = _T("Avi File (*.avi)\0*.avi\0")
								_T("Swf File (*.swf)\0*.swf\0")
								_T("Animated GIF (*.gif)\0*.gif\0");
	dlgFile.m_ofn.nFilterIndex = 1;
	if (sDlgTitle != _T(""))
		dlgFile.m_ofn.lpstrTitle = sDlgTitle;

	if (dlgFile.DoModal() == IDOK)
	{
		// Save to itself?
		if (m_sFileName.CompareNoCase(FileName) == 0)
		{
			if (IsModified())
				return Save();
			else
				bSaveCopyAs = FALSE;
		}
		else
			bSaveCopyAs = TRUE;

		// Store Current Frame Position
		int nCurrentFramePos = 0;
		CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
		if (pVideoStream)
			nCurrentFramePos = pVideoStream->GetCurrentFramePos();

		// Check whether the file is read only
		if (::IsReadonly(FileName))
		{
			CString str;
			str.Format(ML_STRING(1250, "The file %s\nis read only"), FileName);
			::AfxMessageBox(str, MB_ICONSTOP);
			return FALSE;
		}

		// Check whether we have write access to the file
		if (!::HasWriteAccess(FileName))
		{
			CString str;
			str.Format(ML_STRING(1251, "Access denied to %s"), FileName);
			::AfxMessageBox(str, MB_ICONSTOP);
			return FALSE;
		}

		TCHAR ext[10] = _T("");
		lpPos = _tcsrchr(FileName, _T('.'));
		if (lpPos != NULL)
			_tcscpy(ext, lpPos+1);
		CString extension = ext;
		extension.MakeLower();

		BOOL bSaveAsVideoFile = FALSE;

		if (extension == _T("gif"))
		{
			CAnimGifSaveDlg AnimGifSaveDlg(GetView());
			if (AnimGifSaveDlg.DoModal() == IDOK)
			{	
				UINT uiMaxColors;
				switch (AnimGifSaveDlg.m_nNumColors)
				{
					case 0 : uiMaxColors = 255;
							break;
					case 1 : uiMaxColors = 127;
							break;
					case 2 : uiMaxColors = 63;
							break;
					case 3 : uiMaxColors = 31;
							break;
					case 4 : uiMaxColors = 15;
							break;
					default: uiMaxColors = 7;
							break;
				}
				res = SaveAsAnimGIF(FileName,
									AnimGifSaveDlg.m_nColorTables == 0,
									AnimGifSaveDlg.m_bDitherColorConversion,
									uiMaxColors,
									(AnimGifSaveDlg.m_nLoopInfinite == 1) ?
									0 : AnimGifSaveDlg.m_uiPlayTimes);
			}
			else
			{
				// Set user interruption so that the error message box is not shown
				m_ProcessingThread.Kill_NoBlocking();
			}
		}
		else
		{
			// Vars
			bSaveAsVideoFile = TRUE;

			// Save
			if (extension != _T("avi")	&&
				extension != _T("divx"))
			{
				res = SaveAsAVCODEC(FileName,
									TRUE,
									nCurrentFramePos);
			}
			else
			{
				res = SaveAsAVCODEC(FileName,
									bSaveCopyAs,
									nCurrentFramePos);
			}
		}

		// Load Picture
		if (!bSaveAsVideoFile)
		{
			// Restore Current Frame Position
			pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
			if (pVideoStream)
			{
				pVideoStream->SetCurrentFramePos(nCurrentFramePos);
				m_PlayAudioFileThread.PlaySyncAudioFromVideo();
			}

			// Open Document
			if (res)
			{
				::PostMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
								WM_THREADSAFE_OPEN_DOC,
								(WPARAM)(new CString(FileName)),
								(LPARAM)NULL);
			}
			else
			{
				// Show Error Message if not interrupted by user
				if (!m_ProcessingThread.DoExit())
					::AfxMessageBox(ML_STRING(1431, "Error while saving file"), MB_ICONSTOP);
			}

			// Restore Frame
			RestoreFrame(THREAD_SAFE_UPDATEWINDOWSIZES_DELAY);
		}
	}
	
	return res;
}
									
BOOL CVideoAviDoc::SaveAsAVCODEC(	const CString& sFileName,
									BOOL bSaveCopyAs,
									int nCurrentFramePos)
{	
	// Save to itself?
	CString sDstFileName = sFileName;
	if (m_sFileName.CompareNoCase(sFileName) == 0)
	{
		// Temporary File
		sDstFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);
	}

	// Save
	int res = SaveAsAVCODECDlgs(sDstFileName);
	if (res == 0)
		return FALSE; // Dlg Canceled
	else if (res == 1)
	{
		// Open Document
		if (bSaveCopyAs)
		{
			// Restore Current Frame Position
			CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
			if (pVideoStream)
			{
				pVideoStream->SetCurrentFramePos(nCurrentFramePos);
				m_PlayAudioFileThread.PlaySyncAudioFromVideo();
			}

			// Open New Document
			if (CUImagerApp::IsAVIFile(sFileName))
			{
				::PostMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
								WM_THREADSAFE_OPEN_DOC,
								(WPARAM)(new CString(sFileName)),
								(LPARAM)NULL);
			}
			else if (!((CUImagerApp*)::AfxGetApp())->m_bDisableExtProg)
				::ShellExecute(NULL, _T("open"), sFileName, NULL, NULL, SW_SHOWNORMAL);

			// Restore Frame
			if (m_pAVIPlay &&
				m_pAVIPlay->HasVideo() &&
				(m_nActiveVideoStream >= 0))
			{
				RestoreFrame(THREAD_SAFE_UPDATEWINDOWSIZES_DELAY);
			}
			else
			{
				CPostDelayedMessageThread::PostDelayedMessage(
										GetView()->GetSafeHwnd(),
										WM_THREADSAFE_UPDATEWINDOWSIZES,
										THREAD_SAFE_UPDATEWINDOWSIZES_DELAY,
										(WPARAM)(UPDATEWINDOWSIZES_INVALIDATE),
										(LPARAM)0);
			}	
		}
		// Load
		else
		{
			// Clear Modified Flag
			SetModifiedFlag(FALSE);

			// Save to itself?
			if (m_sFileName.CompareNoCase(sFileName) == 0)
			{
				// Free
				delete m_pAVIPlay;
				m_pAVIPlay = NULL;

				// Remove & Rename
				try
				{
					CFile::Remove(m_sFileName);
					CFile::Rename(sDstFileName, m_sFileName);
				}
				catch (CFileException* e)
				{
					e->ReportError();
					e->Delete();
					CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
					if (pVideoStream)
					{
						pVideoStream->SetCurrentFramePos(nCurrentFramePos);
						m_PlayAudioFileThread.PlaySyncAudioFromVideo();
					}
					::DeleteFile(sDstFileName); // Delete Tmp File
					return FALSE;
				}
			}

			// Load AVI
			CPostDelayedMessageThread::PostDelayedMessage(
										GetView()->GetSafeHwnd(),
										WM_THREADSAFE_LOAD_AVI,
										THREAD_SAFE_LOAD_AVI_DELAY,
										(WPARAM)(new CString(sFileName)),
										(LPARAM)nCurrentFramePos);
		}

		return TRUE;
	}
	else
	{
		// Show Error Message if not interrupted by user
		if (!m_ProcessingThread.DoExit())
			::AfxMessageBox(ML_STRING(1431, "Error while saving file"), MB_ICONSTOP);

		// Restore Current Frame Position
		CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
		if (pVideoStream)
		{
			pVideoStream->SetCurrentFramePos(nCurrentFramePos);
			m_PlayAudioFileThread.PlaySyncAudioFromVideo();
		}

		// Delete Tmp File
		if (!bSaveCopyAs)
			::DeleteFile(sDstFileName);

		return FALSE;
	}
}

// Return Values:
// -1: Error
// 0 : Dlg Canceled
// 1 : Ok
int CVideoAviDoc::SaveAsAVCODECDlgs(const CString& sDstFileName)
{
	// Open AVI File
	CAVIPlay* pAVIPlay = new CAVIPlay;
	if (!pAVIPlay)
		return -1;
	pAVIPlay->SetShowMessageBoxOnError(false);
	if (!pAVIPlay->Open(m_sFileName))
	{
		delete pAVIPlay;
		return -1;
	}

	CAVRec* pAVRec = NULL;
	int ret = SaveAsAVCODECDlgs(sDstFileName,
								&pAVRec,
								pAVIPlay,
								m_dwVideoCompressorFourCC,
								m_nVideoCompressorKeyframesRate,
								m_fVideoCompressorQuality,
								m_pAudioCompressorWaveFormat,
								GetView(),
								FALSE,
								NULL,
								&m_ProcessingThread);

	// Free
	if (pAVRec)
	{
		if (!pAVRec->Close())
			ret = -1;
		delete pAVRec;
	}

	// Restore the original dwRate, dwScale and dwStart values for the video stream(s) 
	// and update the dwStart value for the audio stream(s)
	if (ret == 1 && CUImagerApp::IsAVIFile(sDstFileName))
	{
		for (DWORD dwVideoStreamNum = 0 ; dwVideoStreamNum < pAVIPlay->GetVideoStreamsCount() ; dwVideoStreamNum++)
		{
			CAVIPlay::CAVIVideoStream* pSrcVideoStream = pAVIPlay->GetVideoStream(dwVideoStreamNum);
			if (pSrcVideoStream)
			{
				int dst_rate, dst_scale;
				av_reduce(&dst_scale, &dst_rate, (int64_t)pSrcVideoStream->GetScale(), (int64_t)pSrcVideoStream->GetRate(), INT_MAX);
				CAVIPlay::AviChangeVideoFrameRate(	sDstFileName,
													dwVideoStreamNum,
													(DWORD)dst_rate,
													(DWORD)dst_scale,
													false);
				CAVIPlay::AviChangeVideoStartOffset(sDstFileName,
													dwVideoStreamNum,
													pSrcVideoStream->GetStart(),
													false);
			}
		}
		for (DWORD dwAudioStreamNum = 0 ; dwAudioStreamNum < pAVIPlay->GetAudioStreamsCount() ; dwAudioStreamNum++)
		{
			int nShiftMs = 0;
			CAVIPlay::CAVIAudioStream* pSrcAudioStream = pAVIPlay->GetAudioStream(dwAudioStreamNum);
			if (pSrcAudioStream && pSrcAudioStream->GetRate() > 0)
				nShiftMs = ::MulDiv(pSrcAudioStream->GetStart() * 1000,
									pSrcAudioStream->GetScale(),
									pSrcAudioStream->GetRate());
			if (nShiftMs > 0)
			{
				CAVIPlay* pDstAVIPlay = new CAVIPlay(sDstFileName);
				if (pDstAVIPlay)
				{
					CAVIPlay::CAVIAudioStream* pDstAudioStream = pDstAVIPlay->GetAudioStream(dwAudioStreamNum);
					if (pDstAudioStream && pDstAudioStream->GetScale() > 0)
					{
						DWORD dwStart = (DWORD)::MulDiv(nShiftMs,
														pDstAudioStream->GetRate(),
														pDstAudioStream->GetScale()) / 1000;
						delete pDstAVIPlay;
						CAVIPlay::AviChangeAudioStartOffset(sDstFileName,
															dwAudioStreamNum,
															dwStart,
															false);
					}
					else
						delete pDstAVIPlay;
				}
			}
		}
	}

	// Free
	delete pAVIPlay;

	return ret;
}

// Return Values:
// -1: Error
// 0 : Dlg Canceled
// 1 : Ok
int CVideoAviDoc::SaveAsAVCODECDlgs(const CString& sDstFileName,
									CAVRec** ppAVRec,		// If first file *ppAVRec is NULL and will be allocated
									CAVIPlay* pAVIPlay,		// Already Opened Input AVI File
									DWORD& dwVideoCompressorFourCC,
									int& nVideoCompressorKeyframesRate,
									float& fVideoCompressorQuality,
									LPWAVEFORMATEX pAudioCompressorWaveFormat,
									CWnd* pProgressWnd/*=NULL*/,
									BOOL bProgressSend/*=TRUE*/,
									CPercentProgress* pPercentProgress/*=NULL*/,
									CWorkerThread* pThread/*=NULL*/)
{
	CVideoFormatDlg VideoFormatDlg(pProgressWnd);
	CAudioFormatDlg AudioFormatDlg(pProgressWnd);

	// Check
	if (!pAVIPlay || !ppAVRec)
		return -1;

	// Audio Format
	double dAudioLength = 0.0;
	if (pAudioCompressorWaveFormat && pAVIPlay->GetAudioStreamsCount() > 0)
	{
		memcpy(&AudioFormatDlg.m_WaveFormat, pAudioCompressorWaveFormat, sizeof(WAVEFORMATEX));
		CAVIPlay::CAVIAudioStream* pSrcAudioStream = pAVIPlay->GetAudioStream(0);
		if (pSrcAudioStream)
		{
			// Set format for dlg
			memcpy(&AudioFormatDlg.m_WaveFormat, pSrcAudioStream->GetFormat(true), sizeof(WAVEFORMATEX));
						
			// GetTotalTime() needs m_nVBRSamplesPerChunk which is
			// calculated when opening the decompressor
			if (pSrcAudioStream->IsVBR())
			{
				pSrcAudioStream->OpenDecompression();
				pSrcAudioStream->SetStart();
			}
			dAudioLength = pSrcAudioStream->GetTotalTime();
		}
		if (CUImagerApp::IsAVIFile(sDstFileName))
		{
			if (AudioFormatDlg.DoModal() != IDOK)
				return 0;
			memcpy(pAudioCompressorWaveFormat, &AudioFormatDlg.m_WaveFormat, sizeof(WAVEFORMATEX));
		}
		else if (CUImagerApp::IsSWFFile(sDstFileName))
		{
			// Swf only supports mp3 with sample rates of 44100 Hz, 22050 Hz
			// and 11025 Hz. Emulate the audio dialog and make the best choice
			// depending from the original sample rate.
			if (AudioFormatDlg.m_WaveFormat.nSamplesPerSec >= 44100)
			{
				pAudioCompressorWaveFormat->nSamplesPerSec = 44100;
				if (AudioFormatDlg.m_WaveFormat.nChannels >= 2)
				{
					pAudioCompressorWaveFormat->nChannels = 2;
					pAudioCompressorWaveFormat->nAvgBytesPerSec = (96000 / 8);
				}
				else
				{
					pAudioCompressorWaveFormat->nChannels = 1;
					pAudioCompressorWaveFormat->nAvgBytesPerSec = (56000 / 8);
				}
			}
			else if (AudioFormatDlg.m_WaveFormat.nSamplesPerSec >= 22050)
			{
				pAudioCompressorWaveFormat->nSamplesPerSec = 22050;
				if (AudioFormatDlg.m_WaveFormat.nChannels >= 2)
				{
					pAudioCompressorWaveFormat->nChannels = 2;
					pAudioCompressorWaveFormat->nAvgBytesPerSec = (56000 / 8);
				}
				else
				{
					pAudioCompressorWaveFormat->nChannels = 1;
					pAudioCompressorWaveFormat->nAvgBytesPerSec = (32000 / 8);
				}
			}
			else
			{
				pAudioCompressorWaveFormat->nSamplesPerSec = 11025;
				if (AudioFormatDlg.m_WaveFormat.nChannels >= 2)
				{
					pAudioCompressorWaveFormat->nChannels = 2;
					pAudioCompressorWaveFormat->nAvgBytesPerSec = (32000 / 8);
				}
				else
				{
					pAudioCompressorWaveFormat->nChannels = 1;
					pAudioCompressorWaveFormat->nAvgBytesPerSec = (24000 / 8);
				}
			}
			pAudioCompressorWaveFormat->wFormatTag = WAVE_FORMAT_MPEGLAYER3;
			pAudioCompressorWaveFormat->nBlockAlign = 0;
			pAudioCompressorWaveFormat->wBitsPerSample = 0;
		}
	}

	// Video Format
	if (pAVIPlay->GetVideoStreamsCount() > 0)
	{
		if (CUImagerApp::IsAVIFile(sDstFileName))
		{
			DWORD dwFourCC = dwVideoCompressorFourCC;
			int nKeyframesRate = nVideoCompressorKeyframesRate;
			CAVIPlay::CAVIVideoStream* pSrcVideoStream = pAVIPlay->GetVideoStream(0);
			if (pSrcVideoStream)
			{
				// Get from file
				dwFourCC = pSrcVideoStream->GetFourCC(true);
				nKeyframesRate = Round((double)pSrcVideoStream->GetTotalFrames() / (double)pSrcVideoStream->GetTotalKeyFrames());
								
				// Get from previous settings if to small
				if (nKeyframesRate <= 1)
					nKeyframesRate = nVideoCompressorKeyframesRate;
			}
			VideoFormatDlg.m_dwVideoCompressorFourCC = dwFourCC;
			VideoFormatDlg.m_nVideoCompressorKeyframesRate = nKeyframesRate;
			VideoFormatDlg.m_fVideoCompressorQuality = fVideoCompressorQuality;
			if (VideoFormatDlg.DoModal() != IDOK)
				return 0;
			fVideoCompressorQuality = VideoFormatDlg.m_fVideoCompressorQuality;
			nVideoCompressorKeyframesRate = VideoFormatDlg.m_nVideoCompressorKeyframesRate;
			dwVideoCompressorFourCC = VideoFormatDlg.m_dwVideoCompressorFourCC;
		}
		else if (CUImagerApp::IsSWFFile(sDstFileName))
		{
			DWORD dwFourCC = dwVideoCompressorFourCC;
			int nKeyframesRate = nVideoCompressorKeyframesRate;
			CAVIPlay::CAVIVideoStream* pSrcVideoStream = pAVIPlay->GetVideoStream(0);
			if (pSrcVideoStream)
			{
				// Get from file
				dwFourCC = pSrcVideoStream->GetFourCC(true);
				nKeyframesRate = Round((double)pSrcVideoStream->GetTotalFrames() / (double)pSrcVideoStream->GetTotalKeyFrames());
						
				// Get from previous settings if to small
				if (nKeyframesRate <= 1)
					nKeyframesRate = nVideoCompressorKeyframesRate;

				// Warn
				if (pSrcVideoStream->GetTotalFrames() > 16000)
				{
					if (::AfxMessageBox(ML_STRING(1432, "The Flash Player has a limit of 16000 frames.\n") +
										ML_STRING(1433, "The file you are saving is bigger than that,\n") +
										ML_STRING(1434, "a solution is to split it in more pieces.\n") +
										ML_STRING(1435, "Do you want to continue anyway?\n"),
										MB_YESNO | MB_ICONWARNING) == IDNO)
						return 0;
				}
			}
			VideoFormatDlg.m_dwVideoCompressorFourCC = dwFourCC;
			VideoFormatDlg.m_nVideoCompressorKeyframesRate = nKeyframesRate;
			VideoFormatDlg.m_fVideoCompressorQuality = fVideoCompressorQuality;
			VideoFormatDlg.m_nFileType = CVideoFormatDlg::FILETYPE_SWF;
			if (VideoFormatDlg.DoModal() != IDOK)
				return 0;
			fVideoCompressorQuality = VideoFormatDlg.m_fVideoCompressorQuality;
			nVideoCompressorKeyframesRate = VideoFormatDlg.m_nVideoCompressorKeyframesRate;
			dwVideoCompressorFourCC = VideoFormatDlg.m_dwVideoCompressorFourCC;
		}
	}

	BOOL res = SaveAsAVCODECMultiFile(	sDstFileName,
										ppAVRec,
										pAVIPlay,
										dwVideoCompressorFourCC,
										nVideoCompressorKeyframesRate,
										fVideoCompressorQuality,
										pAudioCompressorWaveFormat,
										pProgressWnd,
										bProgressSend,
										pPercentProgress,
										pThread);
	
	return (res ? 1 : -1);
}

BOOL CVideoAviDoc::SaveAsAVCODEC(	const CString& sDstFileName,
									const CString& sSrcFileName,
									DWORD dwVideoCompressorFourCC,
									int nVideoCompressorKeyframesRate,
									float fVideoCompressorQuality,
									LPWAVEFORMATEX pAudioCompressorWaveFormat,
									CWnd* pProgressWnd/*=NULL*/,
									BOOL bProgressSend/*=TRUE*/,
									CWorkerThread* pThread/*=NULL*/)
{	
	// Open AVI File
	CAVIPlay* pAVIPlay = new CAVIPlay;
	if (!pAVIPlay)
		return FALSE;
	pAVIPlay->SetShowMessageBoxOnError(false);
	if (!pAVIPlay->Open(sSrcFileName))
	{
		delete pAVIPlay;
		return FALSE;
	}

	BOOL res = SaveAsAVCODECSingleFile(	sDstFileName,
										pAVIPlay,
										dwVideoCompressorFourCC,
										nVideoCompressorKeyframesRate,
										fVideoCompressorQuality,
										pAudioCompressorWaveFormat,
										pProgressWnd,
										bProgressSend,
										pThread);

	// Restore the original dwRate, dwScale and dwStart values for the video stream(s) 
	// and update the dwStart value for the audio stream(s)
	if (res && CUImagerApp::IsAVIFile(sDstFileName))
	{
		for (DWORD dwVideoStreamNum = 0 ; dwVideoStreamNum < pAVIPlay->GetVideoStreamsCount() ; dwVideoStreamNum++)
		{
			CAVIPlay::CAVIVideoStream* pSrcVideoStream = pAVIPlay->GetVideoStream(dwVideoStreamNum);
			if (pSrcVideoStream)
			{
				int dst_rate, dst_scale;
				av_reduce(&dst_scale, &dst_rate, (int64_t)pSrcVideoStream->GetScale(), (int64_t)pSrcVideoStream->GetRate(), INT_MAX);
				CAVIPlay::AviChangeVideoFrameRate(	sDstFileName,
													dwVideoStreamNum,
													(DWORD)dst_rate,
													(DWORD)dst_scale,
													false);
				CAVIPlay::AviChangeVideoStartOffset(sDstFileName,
													dwVideoStreamNum,
													pSrcVideoStream->GetStart(),
													false);
			}
		}
		for (DWORD dwAudioStreamNum = 0 ; dwAudioStreamNum < pAVIPlay->GetAudioStreamsCount() ; dwAudioStreamNum++)
		{
			int nShiftMs = 0;
			CAVIPlay::CAVIAudioStream* pSrcAudioStream = pAVIPlay->GetAudioStream(dwAudioStreamNum);
			if (pSrcAudioStream && pSrcAudioStream->GetRate() > 0)
				nShiftMs = ::MulDiv(pSrcAudioStream->GetStart() * 1000,
									pSrcAudioStream->GetScale(),
									pSrcAudioStream->GetRate());
			if (nShiftMs > 0)
			{
				CAVIPlay* pDstAVIPlay = new CAVIPlay(sDstFileName);
				if (pDstAVIPlay)
				{
					CAVIPlay::CAVIAudioStream* pDstAudioStream = pDstAVIPlay->GetAudioStream(dwAudioStreamNum);
					if (pDstAudioStream && pDstAudioStream->GetScale() > 0)
					{
						DWORD dwStart = (DWORD)::MulDiv(nShiftMs,
														pDstAudioStream->GetRate(),
														pDstAudioStream->GetScale()) / 1000;
						delete pDstAVIPlay;
						CAVIPlay::AviChangeAudioStartOffset(sDstFileName,
															dwAudioStreamNum,
															dwStart,
															false);
					}
					else
						delete pDstAVIPlay;
				}
			}
		}
	}

	// Free
	delete pAVIPlay;
	
	return res;
}

BOOL CVideoAviDoc::SaveAsAVCODECSingleFile(	const CString& sDstFileName,
											CAVIPlay* pAVIPlay,		// Already Opened Input AVI File
											DWORD dwVideoCompressorFourCC,
											int nVideoCompressorKeyframesRate,
											float fVideoCompressorQuality,
											LPWAVEFORMATEX pAudioCompressorWaveFormat,
											CWnd* pProgressWnd/*=NULL*/,
											BOOL bProgressSend/*=TRUE*/,
											CWorkerThread* pThread/*=NULL*/)
{	
	CAVRec* pAVRec = NULL;
	BOOL res = SaveAsAVCODECMultiFile(	sDstFileName,
										&pAVRec,	// This function allocates pAVRec!
										pAVIPlay,
										dwVideoCompressorFourCC,
										nVideoCompressorKeyframesRate,
										fVideoCompressorQuality,
										pAudioCompressorWaveFormat,
										pProgressWnd,
										bProgressSend,
										NULL,
										pThread);

	// Free
	if (pAVRec)
	{
		if (!pAVRec->Close())
			res = FALSE;
		delete pAVRec;
	}

	return res;
}

BOOL CVideoAviDoc::SaveAsAVCODECMultiFile(	const CString& sDstFileName,
											CAVRec** ppAVRec,		// If first file *ppAVRec is NULL and will be allocated
											CAVIPlay* pAVIPlay,		// Already Opened Input AVI File
											DWORD dwVideoCompressorFourCC,
											int nVideoCompressorKeyframesRate,
											float fVideoCompressorQuality,
											LPWAVEFORMATEX pAudioCompressorWaveFormat,
											CWnd* pProgressWnd/*=NULL*/,
											BOOL bProgressSend/*=TRUE*/,
											CPercentProgress* pPercentProgress/*=NULL*/,
											CWorkerThread* pThread/*=NULL*/)
{	
	BOOL res = TRUE;
	BOOL bFirstFile = FALSE;
	CAVIPlay::CAVIVideoStream* pSrcVideoStream;
	CAVIPlay::CAVIAudioStream* pSrcAudioStream;
	DWORD dwVideoStreamNum;
	DWORD dwAudioStreamNum;
	BOOL bVideoAvailable = TRUE;
	BOOL bAudioAvailable = TRUE;
	DWORD dwVideoFrame = 0;
	double dFrameRate = 0.0;
	double dElapsedTime = 0.0;
	DWORD dwTotalFrames = 0;
	int nVideoPercent = 0;
	int nPrevVideoPercent = -1;
	DWORD dwTotalAudioChunks = 0;
	int nCurrentAudioChunk = -1;
	int nAudioPercent = 0;
	int nPrevAudioPercent = -1;
	bool bInterleave; 
	if (CUImagerApp::IsAVIFile(sDstFileName))
		bInterleave = false;	// Already better done in this function for AVI, do not enable!
	else
		bInterleave = true;		// Necessary for Swf to play well inside a browser

	// Check
	if (!pAVIPlay || !ppAVRec)
		goto error;

	// Decompressed source video frame and wanted destination video format
	CDib* pDib = new CDib;
	if (!pDib)
		goto error;
	pDib->SetShowMessageBoxOnError(FALSE);
	LPBITMAPINFO pBmi = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
	memset(pBmi, 0, sizeof(BITMAPINFOHEADER));
	pBmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pBmi->bmiHeader.biPlanes = 1;
	pBmi->bmiHeader.biCompression = dwVideoCompressorFourCC;

	// AVRec
	if (*ppAVRec == NULL)
	{
		*ppAVRec = new CAVRec;
		if (!(*ppAVRec))
			goto error;
		if (!(*ppAVRec)->Init(sDstFileName))
			goto error;
		bFirstFile = TRUE;
	}

	// Add Video Stream(s)
	for (dwVideoStreamNum = 0 ; dwVideoStreamNum < pAVIPlay->GetVideoStreamsCount() ; dwVideoStreamNum++)
	{
		pSrcVideoStream = pAVIPlay->GetVideoStream(dwVideoStreamNum);
		if (pSrcVideoStream)
		{
			// Set Width & Height
			pBmi->bmiHeader.biWidth = pSrcVideoStream->GetWidth();
			pBmi->bmiHeader.biHeight = pSrcVideoStream->GetHeight();
			
			// Open input decompressing to RGB
			pSrcVideoStream->OpenDecompression(true);

			// Add Stream
			if (bFirstFile)
			{
				if ((*ppAVRec)->AddVideoStream(	pSrcVideoStream->GetFormat(false),							// Decompressed Src Video Format
												pBmi,														// Dst Video Format
												pSrcVideoStream->GetRate(),									// Dst Rate
												pSrcVideoStream->GetScale(),								// Dst Scale
												nVideoCompressorKeyframesRate,								// Keyframes Rate
												fVideoCompressorQuality,									// 2.0f best quality, 31.0f worst quality
												((CUImagerApp*)::AfxGetApp())->m_nAVCodecThreadsCount) < 0)
					goto error;
			}
		}
	}
	
	// Add Audio Stream(s)
	for (dwAudioStreamNum = 0 ; dwAudioStreamNum < pAVIPlay->GetAudioStreamsCount() ; dwAudioStreamNum++)
	{
		pSrcAudioStream = pAVIPlay->GetAudioStream(dwAudioStreamNum);
		if (pSrcAudioStream)
		{
			// Open Input
			pSrcAudioStream->OpenDecompression();
			pSrcAudioStream->SetStart();

			// Add Stream
			if (bFirstFile)
			{
				if ((*ppAVRec)->AddAudioStream(	pSrcAudioStream->GetFormat(false),			// Decompressed Src Wave Format
												pAudioCompressorWaveFormat) < 0)			// Dst Wave Format	
					goto error;
			}
		}
	}

	// Open AVRec
	if (bFirstFile)
	{
		if (!(*ppAVRec)->Open())
			goto error;
	}

	// Compress
	while (bVideoAvailable || bAudioAvailable)
	{
		// Exit?
		if (pThread && pThread->DoExit())
			goto error;

		// Reset vars
		bVideoAvailable = FALSE;
		bAudioAvailable = FALSE;
		DWORD dwRecStreamNum = 0;

		// Add Video
		for (dwVideoStreamNum = 0 ; dwVideoStreamNum < pAVIPlay->GetVideoStreamsCount() ; dwVideoStreamNum++)
		{
			pSrcVideoStream = pAVIPlay->GetVideoStream(dwVideoStreamNum);
			if (pSrcVideoStream)
			{
				if (pSrcVideoStream->GetFrame(pDib))
				{
					bVideoAvailable = TRUE;

					// 4 bpp and 8 bpp not supported by CAVRec
					if ((pDib->GetCompression() == BI_RGB ||
						pDib->GetCompression() == BI_BITFIELDS)	&&
						pDib->GetBitCount() < 16)
						pDib->ConvertTo32bits();

					(*ppAVRec)->AddFrame(	dwRecStreamNum,
											pDib,
											bInterleave);
				}

				// Init Vars
				if (dwTotalFrames == 0)
					dwTotalFrames = pSrcVideoStream->GetTotalFrames();
				if (dFrameRate == 0.0)
					dFrameRate = pSrcVideoStream->GetFrameRate();

				// Inc. Rec Stream Num
				dwRecStreamNum++;
			}
		}

		// Set Elapsed Time
		if (dFrameRate > 0.0)
			dElapsedTime += 1.0 / dFrameRate;

		// Add audio
		for (dwAudioStreamNum = 0 ; dwAudioStreamNum < pAVIPlay->GetAudioStreamsCount() ; dwAudioStreamNum++)
		{
			pSrcAudioStream = pAVIPlay->GetAudioStream(dwAudioStreamNum);
			if (pSrcAudioStream)
			{
				LONGLONG llSampleNum = (LONGLONG)(dElapsedTime * pSrcAudioStream->GetSampleRate(false));
				if (llSampleNum < 0)
					llSampleNum = 0;
				while (!bVideoAvailable ||
					((int)pSrcAudioStream->SampleToChunk(llSampleNum) >= pSrcAudioStream->GetCurrentChunkPos()))
				{
					if (pSrcAudioStream->GetNextChunksSamples())
					{
						bAudioAvailable = TRUE;
						(*ppAVRec)->AddAudioSamples(dwRecStreamNum,
													pSrcAudioStream->GetBufSamplesCount(),
													pSrcAudioStream->GetBufData(),
													bInterleave);
						if (nCurrentAudioChunk < 0)
							nCurrentAudioChunk = pSrcAudioStream->GetCurrentChunkPos();
					}
					else
						break;

					if (!bVideoAvailable) // Interleave possible multiple audio streams
						break;
				}

				// Init Var
				if (dwTotalAudioChunks == 0)
					dwTotalAudioChunks = pSrcAudioStream->GetTotalChunks();

				// Inc. Rec Stream Num
				dwRecStreamNum++;
			}
		}

		// Video Progress
		dwVideoFrame++;
		if (dwTotalFrames)
		{
			if (pPercentProgress)
			{
				nVideoPercent = (int)(pPercentProgress->dVideoPercentOffset +
									(pPercentProgress->dVideoPercentSize * dwVideoFrame / dwTotalFrames));
			}
			else
				nVideoPercent = 100 * dwVideoFrame / dwTotalFrames;
			if (nVideoPercent >= 99)
				nVideoPercent = 100;
			if (nVideoPercent > nPrevVideoPercent)
			{
				if (pProgressWnd)
				{
					if (bProgressSend)
						::SendMessage(pProgressWnd->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeVIDEO, (LPARAM)nVideoPercent);	
					else
						::PostMessage(pProgressWnd->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeVIDEO, (LPARAM)nVideoPercent);	
				}
				nPrevVideoPercent = nVideoPercent;
			}
		}

		// Audio Progress
		if (dwTotalAudioChunks && nCurrentAudioChunk >= 0)
		{
			if (pPercentProgress)
			{
				nAudioPercent = (int)(pPercentProgress->dAudioPercentOffset +
									(pPercentProgress->dAudioPercentSize * nCurrentAudioChunk / dwTotalAudioChunks));
			}
			else
				nAudioPercent = 100 * nCurrentAudioChunk / dwTotalAudioChunks;
			if (nAudioPercent >= 99)
				nAudioPercent = 100;
			if (nAudioPercent > nPrevAudioPercent)
			{
				if (pProgressWnd)
				{
					if (bProgressSend)
						::SendMessage(pProgressWnd->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeAUDIO, (LPARAM)nAudioPercent);
					else
						::PostMessage(pProgressWnd->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeAUDIO, (LPARAM)nAudioPercent);	
				}
				nPrevAudioPercent = nAudioPercent;
			}
			nCurrentAudioChunk = -1;
		}
	}

	goto free;

error:
	res = FALSE;

free:
	if (pDib)
		delete pDib;
	if (pBmi)
		delete [] pBmi;

	return res;
}

void CVideoAviDoc::OnFileExtractframes() 
{
	if (((CUImagerApp*)::AfxGetApp())->IsDocAvailable(this, TRUE))
	{
		ResetPercentDone();
		m_ProcessingThread.SetProcessingFunct(&m_ExtractframesProcessing);
		m_ProcessingThread.Start();
	}
}

void CVideoAviDoc::OnUpdateFileExtractframes(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	!IsProcessing()	&&
					!GetView()->m_bFullScreenMode);
}

BOOL CVideoAviDoc::FileExtractframes()
{
	BOOL res = FALSE;
	TCHAR FileName[MAX_PATH];
	_tcscpy(FileName, ::GetFileNameNoExt(m_sFileName) + _T(".bmp"));
	CSaveFileDlg dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY, NULL, GetView());
	dlgFile.m_ofn.lpstrFile = FileName;
	dlgFile.m_ofn.nMaxFile = MAX_PATH;
	dlgFile.m_ofn.lpstrDefExt = _T("bmp");
	dlgFile.m_ofn.lpstrFilter = _T("BMP Sequence (*.bmp)\0*.bmp\0")
								_T("JPEG Sequence (*.jpg)\0*.jpg\0");
	dlgFile.m_ofn.nFilterIndex = 1;
	if (dlgFile.DoModal() == IDOK)
	{
		// Store Current Frame Position
		int nCurrentFramePos = 0;
		CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
		if (pVideoStream)
			nCurrentFramePos = pVideoStream->GetCurrentFramePos();

		// Extract
		TCHAR ext[10] = _T("");
		LPTSTR lpPos = _tcsrchr(FileName, _T('.'));
		if (lpPos != NULL)
			_tcscpy(ext, lpPos+1);
		CString extension = ext;
		extension.MakeLower();
		CString sFirstFileName;
		if ((extension == _T("bmp")) || (extension == _T("dib")))
		{
			sFirstFileName = ExtractAsBMP(FileName);
			res = (sFirstFileName != _T(""));
		}
		else if (::IsJPEGExt(extension))
		{
			sFirstFileName = ExtractAsJPEG(FileName, dlgFile.GetJpegCompressionQuality());
			res = (sFirstFileName != _T(""));
		}

		// Restore Current Frame Position
		pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
		if (pVideoStream)
		{
			pVideoStream->SetCurrentFramePos(nCurrentFramePos);
			m_PlayAudioFileThread.PlaySyncAudioFromVideo();
		}

		// Open Document
		if (res)
		{
			::PostMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
							WM_THREADSAFE_OPEN_DOC,
							(WPARAM)(new CString(sFirstFileName)),
							(LPARAM)NULL);
		}
		else
		{
			// Show Error Message if not interrupted by user
			if (!m_ProcessingThread.DoExit())
				::AfxMessageBox(ML_STRING(1252, "Could Not Save The Picture."), MB_ICONSTOP);
		}

		// Restore Frame
		RestoreFrame(THREAD_SAFE_UPDATEWINDOWSIZES_DELAY);
	}

	return res;
}

CString CVideoAviDoc::ExtractAsBMP(const CString& sFileName)
{
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (pVideoStream)
	{
		// Number of Digits for File Names
		int nDigits = (int)log10((double)pVideoStream->GetTotalFrames()) + 1;

		// Save BMP Files
		pVideoStream->Rew();
		int nPercentDone;
		int nPrevPercentDone = -1;
		CDib Dib;
		CString sFirstFileName;
		BOOL bFirst = TRUE;
		for (unsigned int i = 0 ; i < pVideoStream->GetTotalFrames() ; i++)
		{
			if (pVideoStream->GetFrame(&Dib))
			{
				// Progress
				nPercentDone = Round(	(double)pVideoStream->GetNextFramePos() * 100.0 /
										(double)pVideoStream->GetTotalFrames());
				if (nPercentDone > nPrevPercentDone)
					::PostMessage(GetView()->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeVIDEO, (LPARAM)nPercentDone);
				nPrevPercentDone = nPercentDone;

				// Decompress to 24 bpp
				Dib.Decompress(24);

				// Save BMP
				CString sFormat;
				CString sCurrentFileName;
				sFormat.Format(_T("%%0%du"), nDigits);
				sCurrentFileName.Format(_T("%s") + sFormat + _T("%s"),
										::GetFileNameNoExt(sFileName),
										i + 1,
										::GetFileExt(sFileName));
				int iCopy = 0;
				while (::IsExistingFile(sCurrentFileName))
				{
					sCurrentFileName.Format(_T("%s") + sFormat + _T("(%d)%s"),
										::GetFileNameNoExt(sFileName),
										i + 1,
										++iCopy,
										::GetFileExt(sFileName));
				}
				if (!Dib.SaveBMP(	sCurrentFileName,
									NULL,
									TRUE,
									&m_ProcessingThread))
					return _T("");

				// Set First File Name
				if (bFirst)
				{
					sFirstFileName = sCurrentFileName;
					bFirst = FALSE;
				}

				// Do Exit?
				if (m_ProcessingThread.DoExit())
					return _T("");					
			}
			else
				return _T("");
		}

		// OK
		return sFirstFileName;
	}
	else
		return _T("");
}

CString CVideoAviDoc::ExtractAsJPEG(const CString& sFileName, int nCompressionQuality)
{
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (pVideoStream)
	{
		// Number of Digits for File Names
		int nDigits = (int)log10((double)pVideoStream->GetTotalFrames()) + 1;

		// Save JPEG Files
		pVideoStream->Rew();
		int nPercentDone;
		int nPrevPercentDone = -1;
		CDib Dib;
		CString sFirstFileName;
		BOOL bFirst = TRUE;
		for (unsigned int i = 0 ; i < pVideoStream->GetTotalFrames() ; i++)
		{
			if (pVideoStream->GetFrame(&Dib))
			{
				// Progress
				nPercentDone = Round(	(double)pVideoStream->GetNextFramePos() * 100.0 /
										(double)pVideoStream->GetTotalFrames());
				if (nPercentDone > nPrevPercentDone)
					::PostMessage(GetView()->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeVIDEO, (LPARAM)nPercentDone);
				nPrevPercentDone = nPercentDone;

				// Decompress to 24 bpp
				Dib.Decompress(24);

				// Save JPEG
				CString sFormat;
				CString sCurrentFileName;
				sFormat.Format(_T("%%0%du"), nDigits);
				sCurrentFileName.Format(_T("%s") + sFormat + _T("%s"),
										::GetFileNameNoExt(sFileName),
										i + 1,
										::GetFileExt(sFileName));
				int iCopy = 0;
				while (::IsExistingFile(sCurrentFileName))
				{
					sCurrentFileName.Format(_T("%s") + sFormat + _T("(%d)%s"),
										::GetFileNameNoExt(sFileName),
										i + 1,
										++iCopy,
										::GetFileExt(sFileName));
				}
				if (!Dib.SaveJPEG(	sCurrentFileName,
									nCompressionQuality,
									FALSE,
									_T(""),
									FALSE,
									FALSE,
									NULL,
									TRUE,
									&m_ProcessingThread))
					return _T("");

				// Set First File Name
				if (bFirst)
				{
					sFirstFileName = sCurrentFileName;
					bFirst = FALSE;
				}

				// Do Exit?
				if (m_ProcessingThread.DoExit())
					return _T("");					
			}
			else
				return _T("");
		}

		// OK
		return sFirstFileName;
	}
	else
		return _T("");
}

// uiPlayTimes = 0: Infinite
// uiPlayTimes = 1: Show All Frames One Time
// uiPlayTimes = 2: Show All Frames Two Times
// ...
BOOL CVideoAviDoc::SaveAsAnimGIF(	const CString& sFileName,
									BOOL bUniqueColorTable,
									BOOL bDitherColorConversion,
									UINT uiMaxColors,
									UINT uiPlayTimes)
{
	// Check
	if (uiMaxColors > 255)
		return FALSE;
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (!pVideoStream)
		return FALSE;

	// Init Vars
	CAnimGifSave AnimGifSave(sFileName,
							((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),
							bDitherColorConversion,
							uiMaxColors,
							uiPlayTimes,
							m_crBackgroundColor);
	int nMsDelay = Round(1000.0 / GetPlayFrameRate());
	unsigned int i;
	int nPercentDone;
	int nPrevPercentDone = -1;
	CDib Dib;

	// Progress 0%
	::PostMessage(GetView()->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeVIDEO, (LPARAM)0);

	// Create a big dib with all frames
	if (bUniqueColorTable)
	{
		// Allocate m_pAllFramesDib
		CDib* pAllFramesDib = new CDib;
		if (!pAllFramesDib)
			return FALSE;
		if (!pAllFramesDib->AllocateBitsFast(24,
											BI_RGB,
											m_DocRect.Width(),
											m_DocRect.Height() * pVideoStream->GetTotalFrames()))
		{
			::AfxMessageBox(ML_STRING(1436, "AVI file is to big to create one unique gif color table,\n") +
							ML_STRING(1437, "please choose the multiple color tables option."));
			delete pAllFramesDib;

			// Set user interruption so that the error message box is not shown
			m_ProcessingThread.Kill_NoBlocking();

			return FALSE;
		}

		// Init m_pAllFramesDib
		pVideoStream->Rew();
		for (i = 0 ; i < pVideoStream->GetTotalFrames() ; i++)
		{
			if (pVideoStream->GetFrame(&Dib))
			{
				Dib.Decompress(24);
				memcpy(pAllFramesDib->GetBits() + Dib.GetImageSize() * i, Dib.GetBits(), Dib.GetImageSize());
			}
			else
			{
				delete pAllFramesDib;
				return FALSE;
			}

			// Do Exit?
			if (m_ProcessingThread.DoExit())
			{
				delete pAllFramesDib;
				return FALSE;
			}
		}

		// Calc. the uiMaxColors colors palette from the big dib
		RGBQUAD* pGIFColors = (RGBQUAD*)new RGBQUAD[uiMaxColors];
		CQuantizer Quantizer(uiMaxColors, 8); // max is 255 colors = 256 (8 bits colors) - 1 (transparency index)
		Quantizer.ProcessImage(pAllFramesDib);
		Quantizer.SetColorTable(pGIFColors);
		if (!AnimGifSave.CreatePaletteFromColors(uiMaxColors, pGIFColors))
		{
			delete [] pGIFColors;
			delete pAllFramesDib;
			return FALSE;
		}
		delete [] pGIFColors;
		delete pAllFramesDib;
	}

	// Save Animated GIF
	pVideoStream->Rew();
	for (i = 0 ; i < pVideoStream->GetTotalFrames() ; i++)
	{
		if (pVideoStream->GetFrame(&Dib))
		{
			// Progress
			nPercentDone = Round(	(double)pVideoStream->GetNextFramePos() * 100.0 /
									(double)pVideoStream->GetTotalFrames());
			if (nPercentDone > nPrevPercentDone)
				::PostMessage(GetView()->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeVIDEO, (LPARAM)nPercentDone);
			nPrevPercentDone = nPercentDone;

			// Decompress to 24 bpp
			Dib.Decompress(24);

			// Save Frame
			if (!AnimGifSave.SaveAsAnimGIF(&Dib, nMsDelay))
				return FALSE;

			// Do Exit?
			if (m_ProcessingThread.DoExit())
				return FALSE;
		}
	}

	return TRUE;
}

BOOL CVideoAviDoc::Save(BOOL bNoReload/*=FALSE*/) 
{
	int nCurrentFramePos = 0;

	if (m_sFileName != _T(""))
	{
		// Check whether the file is read only
		if (::IsReadonly(m_sFileName))
		{
			CString str;
			str.Format(ML_STRING(1256, "The file %s\nis read only.\nUse Save As with a different file name!"), m_sFileName);
			::AfxMessageBox(str, MB_ICONSTOP);
			return FALSE;
		}

		// Check whether we have write access to the file
		if (!::HasWriteAccess(m_sFileName))
		{
			CString str;
			str.Format(ML_STRING(1257, "Access denied to %s.\nUse Save As with a different file name!"), m_sFileName);
			::AfxMessageBox(str, MB_ICONSTOP);
			return FALSE;
		}

		// Do Saving...

		// Clear Modified Flag
		SetModifiedFlag(FALSE);

		// Re-Load? (No Reload when saving and exiting)
		if (!bNoReload)
		{
			CPostDelayedMessageThread::PostDelayedMessage(
										GetView()->GetSafeHwnd(),
										WM_THREADSAFE_LOAD_AVI,
										THREAD_SAFE_LOAD_AVI_DELAY,
										(WPARAM)(new CString(m_sFileName)),
										(LPARAM)nCurrentFramePos);
		}

		return TRUE;
	}
	else
		return FALSE;
}

BOOL CVideoAviDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	// Note:
	// OnSaveDocument() should never be called because I changed
	// the default implementation of OnFileSaveAs(),
	// OnFileSendMail() and SaveModified()
	//
	// - OnFileSaveAs() and OnFileSendMail() both call DoSave()
	// - SaveModified() both call DoFileSave()
	// - DoFileSave() calls DoSave() 
	// - DoSave() calls OnSaveDocument()

	ASSERT(FALSE);

	return FALSE;
}

BOOL CVideoAviDoc::SaveModified() 
{
	if (!IsModified())
		return TRUE;        // ok to continue

	// Get active view and force cursor
	CUImagerView* pActiveView = NULL;
	if (::AfxGetMainFrame()->m_bFullScreenMode)
	{
		CMDIChildWnd* pChild = ::AfxGetMainFrame()->MDIGetActive();
		if (pChild)
		{
			pActiveView = (CUImagerView*)pChild->GetActiveView();
			if (pActiveView && pActiveView->IsKindOf(RUNTIME_CLASS(CUImagerView)))
				pActiveView->ForceCursor();
			else
				pActiveView = NULL;
		}
	}

	CString Str;
	if (m_sFileName == _T(""))
		Str = ML_STRING(1261, "New Document");
	else
		Str = m_sFileName;

	CString prompt;
	AfxFormatString1(prompt, AFX_IDP_ASK_TO_SAVE, Str);
	BOOL res;
	switch (AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE))
	{
		case IDCANCEL:
		{
			if (pActiveView)
				pActiveView->ForceCursor(FALSE);
			return FALSE;		// don't continue
		}

		case IDYES:
			// If so, either Save or Update, as appropriate
			
			// Begin Wait Cursor
			BeginWaitCursor();
			
			// Save
			res = Save(TRUE);	// Save & No Reload

			// End Wait Cursor
			EndWaitCursor();

			if (!res)
			{
				if (pActiveView)
					pActiveView->ForceCursor(FALSE);
				return FALSE;	// don't continue
			}

			break;

		case IDNO:
			// If not saving changes
			break;

		default:
			ASSERT(FALSE);
			break;
	}

	if (pActiveView)
		pActiveView->ForceCursor(FALSE);

	return TRUE;    // keep going
}

void CVideoAviDoc::OnFileSaveAs()
{
	if (((CUImagerApp*)::AfxGetApp())->IsDocAvailable(this, TRUE))
	{
		ResetPercentDone();
		m_ProcessingThread.SetProcessingFunct(&m_SaveAsProcessing);
		m_ProcessingThread.Start();
	}
}

void CVideoAviDoc::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	!IsProcessing()	&&
					!GetView()->m_bFullScreenMode);
}

void CVideoAviDoc::OnEditDelete() 
{
	EditDelete(TRUE);
}

void CVideoAviDoc::OnUpdateEditDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	!IsModified() &&
					!IsProcessing() &&
					!GetView()->m_bFullScreenMode &&
					!m_PlayVideoFileThread.IsAlive() &&
					!m_PlayAudioFileThread.IsAlive());
}

void CVideoAviDoc::EditDelete(BOOL bPrompt)
{
	if (bPrompt)
	{
		CString sMsg;
		sMsg.Format(ML_STRING(1266, "Are you sure you want to delete %s ?"), ::GetShortFileName(m_sFileName));
		if (::AfxMessageBox(sMsg, MB_YESNO) == IDYES)
			DeleteDocFile();
	}
	else
		DeleteDocFile();
}

void CVideoAviDoc::DeleteDocFile()
{
	DWORD dwAttrib = ::GetFileAttributes(m_sFileName);
	if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
	{
		CString str;
		str.Format(ML_STRING(1250, "The file %s\nis read only"), m_sFileName);
		::AfxMessageBox(str, MB_ICONSTOP);
		return;
	}
	delete m_pAVIPlay;
	m_pAVIPlay = NULL;
	if (!::DeleteToRecycleBin(m_sFileName))
	{
		CString str;
		str.Format(ML_STRING(1267, "Cannot delete the %s file"), m_sFileName);
		::AfxMessageBox(str, MB_ICONSTOP);
	}
	CloseDocument();
}

void CVideoAviDoc::OnEditRename()
{
	/* If an accelerator has the same identifier as a menu item and the menu item is grayed or disabled,
	the accelerator is disabled and does not generate a WM_COMMAND or WM_SYSCOMMAND message.
	Also, an accelerator does not generate a command message if the corresponding window is minimized. */
	CRenameDlg dlg;
	dlg.m_sFileName = ::GetShortFileNameNoExt(m_sFileName);
	if (dlg.DoModal() == IDOK && ::IsValidFileName(dlg.m_sFileName, TRUE))
	{	
		// New file name
		CString sNewFileName =	::GetDriveName(m_sFileName) +
								::GetDirName(m_sFileName) +
								dlg.m_sFileName +
								::GetFileExt(m_sFileName);

		// Delete
		delete m_pAVIPlay;
		m_pAVIPlay = NULL;

		// Rename
		if (!::MoveFile(m_sFileName, sNewFileName))
		{
			::ShowLastError(TRUE);
			sNewFileName = m_sFileName;
		}
		
		// Reload active streams
		LoadAVI(sNewFileName, 0, TRUE);
	}
}

void CVideoAviDoc::OnUpdateEditRename(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	!IsModified() &&
					!IsProcessing() &&
					!GetView()->m_bFullScreenMode &&
					!m_PlayVideoFileThread.IsAlive() &&
					!m_PlayAudioFileThread.IsAlive());
}

BOOL CVideoAviDoc::LoadAVI(CString sFileName,
						   DWORD dwFramePos/*=0*/,
						   BOOL bActiveStreams/*=FALSE*/)
{
	// Check File
	try
	{
		// Empty Path?
		if (sFileName.IsEmpty())
			throw (int)AVI_E_ZEROPATH;

		// Check for .avi Extension
		if (!CVideoAviDoc::IsAVI(sFileName))
			throw (int)AVI_E_WRONGEXTENTION;
			
		// Check File Size
		CFile file(sFileName, CFile::modeRead | CFile::shareDenyNone);
		if (file.GetLength() == 0)
			throw (int)AVI_E_FILEEMPTY;
		file.Close();
	}
	catch (CFileException* e)
	{
		e->ReportError();
		e->Delete();
		if (((CUImagerApp*)::AfxGetApp())->m_bStartPlay)
			((CUImagerApp*)::AfxGetApp())->m_bStartPlay = FALSE;
		return FALSE;
	}
	catch (int error_code)
	{
		CString str;
#ifdef _DEBUG
		str.Format(_T("LoadAVI(%s):\n"), sFileName);
#endif
		switch (error_code)
		{
			case AVI_E_ZEROPATH :		str += ML_STRING(1439, "The file name is zero");
			break;
			case AVI_E_WRONGEXTENTION :	str += ML_STRING(1440, "The file extention is not .avi or .divx");
			break;
			case AVI_E_FILEEMPTY :		str += ML_STRING(1441, "File is empty");
			break;
			default:					str += ML_STRING(1442, "Unspecified error");
			break;
		}
		::AfxMessageBox(str);
	
		if (((CUImagerApp*)::AfxGetApp())->m_bStartPlay)
			((CUImagerApp*)::AfxGetApp())->m_bStartPlay = FALSE;

		return FALSE;
	}

	// Begin Wait Cursor
	BeginWaitCursor();

	// Allocate and Open AVI File
	if (m_pAVIPlay)
		delete m_pAVIPlay;
	m_pAVIPlay = new CAVIPlay;
	if (!m_pAVIPlay)
	{
		EndWaitCursor();
		if (((CUImagerApp*)::AfxGetApp())->m_bStartPlay)
			((CUImagerApp*)::AfxGetApp())->m_bStartPlay = FALSE;
		return FALSE;
	}
	if (!m_pAVIPlay->Open(sFileName))
	{
		EndWaitCursor();
		if (((CUImagerApp*)::AfxGetApp())->m_bStartPlay)
			((CUImagerApp*)::AfxGetApp())->m_bStartPlay = FALSE;
		return FALSE;
	}

	// Set Codec Priority
	m_pAVIPlay->m_bAVCodecPriority = m_bAVCodecPriority;

	// End Wait Cursor
	EndWaitCursor();

	// Load Streams
	BOOL res;
	if (bActiveStreams)
		res = LoadActiveStreams(dwFramePos);
	else
		res = LoadStreams(dwFramePos);
	if (!res)
	{
		delete m_pAVIPlay;
		m_pAVIPlay = NULL;
		if (((CUImagerApp*)::AfxGetApp())->m_bStartPlay)
			((CUImagerApp*)::AfxGetApp())->m_bStartPlay = FALSE;
		return FALSE;
	}
	else
	{
		// Set File Name
		if (!IsModified())
		{
			m_sFileName = sFileName;
			SetPathName(sFileName, TRUE);
		}

		// Set Title
		SetDocumentTitle();

		// Update by Fitting Window To Document
		if (!GetFrame()->IsZoomed())
		{
			GetView()->GetParentFrame()->SetWindowPos(	NULL,
														0, 0, 0, 0,
														SWP_NOSIZE |
														SWP_NOZORDER);
			GetView()->UpdateWindowSizes(TRUE, TRUE, TRUE);
		}
		else
			GetView()->UpdateWindowSizes(TRUE, TRUE, FALSE);
		
		// Start Play
		if (((CUImagerApp*)::AfxGetApp())->m_bStartPlay)
		{
			((CUImagerApp*)::AfxGetApp())->m_bStartPlay = FALSE;
			PlayAVI();
		}

		return TRUE;
	}
}

BOOL CVideoAviDoc::CloseActiveStreams()
{
	// Check
	if (!m_pAVIPlay)
		return FALSE;

	// Close Video Decompressor
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (pVideoStream)
		pVideoStream->Free();

	// Close Audio Decompressor
	CAVIPlay::CAVIAudioStream* pAudioStream = m_pAVIPlay->GetAudioStream(m_nActiveAudioStream);
	if (pAudioStream)
		pAudioStream->Free();

	return TRUE;
}

BOOL CVideoAviDoc::LoadStreams(DWORD dwFramePos)
{
	// Check
	if (!m_pAVIPlay)
		return FALSE;

	// Streams Select Dialog
	if ((m_pAVIPlay->GetVideoStreamsCount() > 1) ||
		(m_pAVIPlay->GetAudioStreamsCount() > 1))
	{
		CAviOpenStreamsDlg StreamsSelectDlg(this, GetView());
		if (StreamsSelectDlg.DoModal() != IDOK)
			return FALSE;
	}
	else
	{
		if (m_pAVIPlay->HasVideo())
			m_nActiveVideoStream = 0;
		if (m_pAVIPlay->HasAudio())
			m_nActiveAudioStream = 0;
	}

	return LoadActiveStreams(dwFramePos);	
}

// After LoadActiveStreams() these two expressions are equivalent:
//
// - if (HasVideo() && (m_nActiveVideoStream >= 0))
//       ...
//
// - CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
//   if (pVideoStream)
//       ...
//
// Same for Audio.
//
BOOL CVideoAviDoc::LoadActiveStreams(DWORD dwFramePos)
{
	// Check
	if (!m_pAVIPlay)
		return FALSE;

	// Open Video Decompressor
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (pVideoStream)
	{
		if (!pVideoStream->OpenDecompression(m_bForceRgb ? true : false)) 
			m_nActiveVideoStream = -1;
	}
	else
		m_nActiveVideoStream = -1;

	// Open Audio Decompressor
	CAVIPlay::CAVIAudioStream* pAudioStream = m_pAVIPlay->GetAudioStream(m_nActiveAudioStream);
	if (pAudioStream)
	{
		if (!pAudioStream->OpenDecompression())
			m_nActiveAudioStream = -1;
	}
	else
		m_nActiveAudioStream = -1;

	// Get Frame
	::EnterCriticalSection(&m_csDib);
	if (pVideoStream					&&
		pVideoStream->HasDecompressor()	&&
		pVideoStream->GetFrameAt(m_pDib, MIN(dwFramePos, pVideoStream->GetTotalFrames() - 1)))
	{
		::LeaveCriticalSection(&m_csDib);

		// Init Vars
		m_dwFrameCountUp = 0U;
		m_DocRect.top = 0;
		m_DocRect.left = 0;
		m_DocRect.bottom = pVideoStream->GetHeight();
		m_DocRect.right = pVideoStream->GetWidth();
		pVideoStream->GetFrameRate((DWORD*)&m_dwPlayRate, (DWORD*)&m_dwPlayScale);

		// Dx Draw
		if (m_DxDraw.HasDxDraw())
		{
			// Enter CS
			m_DxDraw.EnterCS();

			// Init DxDraw
			if (m_bUseDxDraw)
			{
				::EnterCriticalSection(&m_csDib);
				m_DxDraw.Init(	GetView()->GetSafeHwnd(),
								m_DocRect.right,
								m_DocRect.bottom,
								m_pDib->GetBMIH()->biCompression,
								IDB_BITSTREAM_VERA_11);

				// Copy Current Frame To DirectDraw Surface
				if (m_DxDraw.IsInit())
					m_DxDraw.RenderDib(m_pDib, GetView()->m_UserZoomRect);
				::LeaveCriticalSection(&m_csDib);
			}

			// Leave CS
			m_DxDraw.LeaveCS();
		}
	}
	else
	{
		::LeaveCriticalSection(&m_csDib);
		m_dwFrameCountUp = 0U;
		m_dwPlayRate = 1;
		m_dwPlayScale = 1;
		m_DocRect.top = 0;
		m_DocRect.left = 0;
		m_DocRect.bottom = AUDIOONLY_DEFAULT_HEIGHT;
		m_DocRect.right = AUDIOONLY_DEFAULT_WIDTH;
	}
	
	// Open Audio
	if (pAudioStream && pAudioStream->HasDecompressor())
		m_PlayAudioFileThread.OpenOutAudio(pAudioStream->GetFormat(false));

	// If Audio And Video Streams
	if ((pAudioStream && pAudioStream->HasDecompressor()) &&
		(pVideoStream && pVideoStream->HasDecompressor()))
	{
		// Init the Shift Var:

		// If Positive Values the Audio leads (is played before)
		// the video by the given amount of ms.
		// If Negative Values the opposite applies.

		if (pVideoStream->GetRate() > 0)
			m_nAudioLeadsVideoShift = ::MulDiv(	pVideoStream->GetStart() * 1000,
												pVideoStream->GetScale(),
												pVideoStream->GetRate());
		else
			m_nAudioLeadsVideoShift = 0;

		// Note: for VBR audio the scale should be set to
		// pAudioStream->GetVBRSamplesPerChunk() (usually 1152)

		if (pAudioStream->GetRate() > 0)
			m_nAudioLeadsVideoShift -= ::MulDiv(pAudioStream->GetStart() * 1000,
												pAudioStream->GetScale(),
												pAudioStream->GetRate());

		// Set Init Value
		m_nAudioLeadsVideoShiftInitValue = m_nAudioLeadsVideoShift;

		// Sync Audio From Video
		m_PlayAudioFileThread.PlaySyncAudioFromVideo();
	}

	// Reset Dropped Frames Count
	m_PlayVideoFileThread.ResetDroppedFramesCount();

	return ((pVideoStream && pVideoStream->HasDecompressor()) ||
			(pAudioStream && pAudioStream->HasDecompressor()));
}

BOOL CVideoAviDoc::LoadActiveStreams()
{
	// Check
	if (!m_pAVIPlay)
		return FALSE;

	int nCurrentFramePos = 0;
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (pVideoStream)
		nCurrentFramePos = pVideoStream->GetCurrentFramePos();
	if (nCurrentFramePos < 0)
		nCurrentFramePos = 0;
	return LoadActiveStreams((DWORD)nCurrentFramePos);
}

BOOL CVideoAviDoc::DisplayFrame(int nFrame, int nDelay/*=0*/)
{
	// Check
	if (m_nActiveVideoStream < 0 || !m_pAVIPlay)
		return FALSE;

	// Get Video Stream
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (!pVideoStream)
		return FALSE;

	if ((nFrame >= 0) && (nFrame < (int)pVideoStream->GetTotalFrames()))
	{
		// Get Frame
		::EnterCriticalSection(&m_csDib);
		pVideoStream->GetFrameAt(m_pDib, nFrame);
		::LeaveCriticalSection(&m_csDib);

		// Restore Frame
		RestoreFrame(nDelay);

		return TRUE;
	}
	else
		return FALSE;
}

void CVideoAviDoc::RestoreFrame(int nDelay/*=0*/)
{
	// A restore has already been posted and not yet processed
	if (m_bAboutToRestoreFrame)
		return;

	// Direct Post
	if (nDelay == 0)
	{
		::PostMessage(	GetView()->GetSafeHwnd(),
						WM_RESTORE_FRAME,
						(WPARAM)0,
						(LPARAM)0);
	}
	// Delayed Post
	else
	{
		CPostDelayedMessageThread::PostDelayedMessage(
									GetView()->GetSafeHwnd(),
									WM_RESTORE_FRAME,
									nDelay,
									(WPARAM)0,
									(LPARAM)0);
	}
}

BOOL CVideoAviDoc::PlayAVI(BOOL bDoRewind, int nSpeedPercent)
{
	// Init Video Thread's Variables
	if (m_pAVIPlay->HasVideo() && (m_nActiveVideoStream >= 0))
		m_PlayVideoFileThread.SetPlaySpeedPercent(nSpeedPercent);

	// Init Audio Thread's Variables
	if (m_pAVIPlay->HasAudio() && (m_nActiveAudioStream >= 0))
		m_PlayAudioFileThread.SetPlaySpeedPercent(nSpeedPercent);

	// Rewind or Sync
	if (bDoRewind)
	{
		if (m_pAVIPlay->HasVideo() && (m_nActiveVideoStream >= 0))
			m_PlayVideoFileThread.Rew();
		if (m_pAVIPlay->HasAudio() && (m_nActiveAudioStream >= 0))
			m_PlayAudioFileThread.Rew();
	}

	// Set Sync Var
	if (m_pAVIPlay->HasVideo() && (m_nActiveVideoStream >= 0) && 
		m_pAVIPlay->HasAudio() && (m_nActiveAudioStream >= 0) &&
		m_PlayAudioFileThread.IsOpen())
	{
		::ResetEvent(m_hPlaySyncEvent);
		m_PlayVideoFileThread.SetPlaySyncEvent(m_hPlaySyncEvent);
		m_PlayAudioFileThread.SetPlaySyncEvent(m_hPlaySyncEvent);
		if (!bDoRewind)
			m_PlayAudioFileThread.PlaySyncAudioFromVideo();
	}

	// We may set: 
	// THREAD_PRIORITY_HIGHEST,
	// THREAD_PRIORITY_ABOVE_NORMAL,
	// THREAD_PRIORITY_TIME_CRITICAL
	bool bResAudio, bResVideo;
	if (m_pAVIPlay->HasVideo() && (m_nActiveVideoStream >= 0))
		bResVideo = m_PlayVideoFileThread.Start();
	else
		bResVideo = true;

	if (m_pAVIPlay->HasAudio() && (m_nActiveAudioStream >= 0) &&
		m_PlayAudioFileThread.IsOpen())
		bResAudio = m_PlayAudioFileThread.Start();
	else
		bResAudio = true;

	return (bResAudio && bResVideo);
}

void CVideoAviDoc::OnPlayAvi() 
{
	PlayAVI();
}

void CVideoAviDoc::PlayAVI()
{
	int nSpeedPercent = 100;
	if (!m_PlayVideoFileThread.IsAlive() && !m_PlayAudioFileThread.IsAlive())
	{
		m_bAboutToStopAviPlay = FALSE;
		CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
		if (pVideoStream)
		{
			if (pVideoStream->GetNextFramePos() == pVideoStream->GetTotalFrames() ||	// Last Frame
				pVideoStream->GetNextFramePos() == 1)									// First Frame
				PlayAVI(TRUE, nSpeedPercent);
			else
				PlayAVI(FALSE, nSpeedPercent);
		}
		else if (m_pAVIPlay->HasAudio() && (m_nActiveAudioStream >= 0))
		{
			if (m_PlayAudioFileThread.GetNextSamplePos() == m_PlayAudioFileThread.GetTotalSamples() ||	// Last Sample
				m_PlayAudioFileThread.GetNextSamplePos() == 1)											// First Sample
				PlayAVI(TRUE, nSpeedPercent);
			else
				PlayAVI(FALSE, nSpeedPercent);
		}
	}
}

void CVideoAviDoc::OnUpdatePlayAvi(CCmdUI* pCmdUI) 
{		
	if (!m_PlayVideoFileThread.IsAlive() && !m_PlayAudioFileThread.IsAlive())
		m_bAboutToStopAviPlay = FALSE;

	pCmdUI->Enable(!m_bAboutToStopAviPlay && !IsProcessing());
	if (m_PlayVideoFileThread.IsAlive() || m_PlayAudioFileThread.IsAlive())
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CVideoAviDoc::EndThumbTrack()
{
	if (m_PlayVideoFileThread.IsAlive() ||
		m_PlayAudioFileThread.IsAlive())
	{
		m_nEndThumbTrackRetryCountDown = END_TRUMBTRACK_RETRY_COUNTDOWN;
		CPostDelayedMessageThread::PostDelayedMessage(	GetView()->GetSafeHwnd(),
														WM_END_THUMBTRACK,	
														END_TRUMBTRACK_RETRY_DELAY,
														(WPARAM)GetView()->m_dwThumbTrackSeq,
														0);
		TRACE(_T("Post Delayed: EndThumbTrack()\n"));
	}
	else
	{
		// Set Pos
		CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
		if (pVideoStream)
			pVideoStream->SetCurrentFramePos(GetView()->m_nThumbTrackPos);
		else if (m_pAVIPlay->HasAudio() && (m_nActiveAudioStream >= 0))
			m_PlayAudioFileThread.SetCurrentSamplePos((LONGLONG)GetView()->m_nThumbTrackPos);

		// Restart Playing
		if (GetView()->m_bWasPlayingBeforeThumbTrack)
		{
			// Reset flag
			GetView()->m_bWasPlayingBeforeThumbTrack = FALSE;

			// At The End?
			BOOL bLastPos = TRUE;
			if (pVideoStream)
				bLastPos = GetView()->m_nThumbTrackPos >= ((int)pVideoStream->GetTotalFrames() - 1);
			else if (m_pAVIPlay->HasAudio() && (m_nActiveAudioStream >= 0))
				bLastPos = GetView()->m_nThumbTrackPos >= ((int)m_PlayAudioFileThread.GetTotalSamples() - 1);
			
			// Play AVI
			if (m_bLoop || !bLastPos)
				PlayAVI();
		}
		else if (pVideoStream)
		{
			DisplayFrame(GetView()->m_nThumbTrackPos);
			m_PlayAudioFileThread.PlaySyncAudioFromVideo();
		}

		// Set Done Flag
		GetView()->m_bThumbTrackDone = TRUE;
	}
}

void CVideoAviDoc::OnStopAvi() 
{
	StopAVI();
}

void CVideoAviDoc::OnUpdateStopAvi(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!IsProcessing());
	if (!m_PlayVideoFileThread.IsAlive() && !m_PlayAudioFileThread.IsAlive())
	{
		m_bAboutToStopAviPlay = FALSE;
		pCmdUI->SetCheck(1);
	}
	else
	{
		if (!m_bAboutToStopAviPlay)
			pCmdUI->SetCheck(0);
	}
}

void CVideoAviDoc::StopAVI()
{
	m_bAboutToStopAviPlay = TRUE;
	m_PlayAudioFileThread.Kill_NoBlocking();
	m_PlayVideoFileThread.Kill_NoBlocking();
}

void CVideoAviDoc::PlayLoop() 
{
	if (m_pAVIPlay->HasVideo() && (m_nActiveVideoStream >= 0) && 
		m_pAVIPlay->HasAudio() && (m_nActiveAudioStream >= 0) &&
		m_PlayAudioFileThread.IsOpen())
	{
		::EnterCriticalSection(&m_csPlayWaitingForStart);
		
		// If Loop is Set -> Reset Loop and Kill Thread if necessary!
		if (IsLoop())
		{
			ChangeLoop();

			if (m_PlayAudioFileThread.IsWaitingForStart())
				m_PlayAudioFileThread.Kill_NoBlocking();
			if (m_PlayVideoFileThread.IsWaitingForStart())
				m_PlayVideoFileThread.Kill_NoBlocking();
		}
		// If Loop Not Set -> Kill Thread if necessary and Set Loop!
		else
		{
			// If one of the two Threads already exited because we are at the end
			// (this happens if one of the two Streams is longer), kill the other Thread
			// and let the user push the start button if he wants to loop to the start!
			// Note: we cannot restart the Thread that has already stopped,
			// many problems would arise with the Sync. and the Frames, Samples Offsets!
			if (!m_PlayAudioFileThread.IsAlive() && m_PlayVideoFileThread.IsAlive())
				m_PlayVideoFileThread.Kill_NoBlocking();
			if (!m_PlayVideoFileThread.IsAlive() && m_PlayAudioFileThread.IsAlive())
				m_PlayAudioFileThread.Kill_NoBlocking();

			ChangeLoop();
		}
		
		::LeaveCriticalSection(&m_csPlayWaitingForStart);
	}
	else
		ChangeLoop();
}

void CVideoAviDoc::OnPlayLoop() 
{
	PlayLoop();
}

void CVideoAviDoc::OnUpdatePlayLoop(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!IsProcessing());
	pCmdUI->SetCheck(m_bLoop ? 1 : 0);
}

void CVideoAviDoc::OnPlayFrameBack() 
{
	FrameBack();
}

void CVideoAviDoc::FrameBack() 
{
	// If Playing -> Stop
	if (m_PlayVideoFileThread.IsAlive() ||
		m_PlayAudioFileThread.IsAlive())
		StopAVI();

	// Get Video Stream
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (pVideoStream)
	{
		if (pVideoStream->GetCurrentFramePos() > 0)
		{
			DWORD nNewPos = pVideoStream->GetCurrentFramePos() - 1;
			if (nNewPos < 0)
				nNewPos = 0;
			DisplayFrame(nNewPos);
		}
	}
	else
	{
		LONGLONG llDec = m_PlayAudioFileThread.GetTotalSamples() / 20;
		LONGLONG llNewPos = m_PlayAudioFileThread.GetCurrentSamplePos() - llDec;
		if (llNewPos >= 0)
			m_PlayAudioFileThread.SetCurrentSamplePos(llNewPos);
		else
			m_PlayAudioFileThread.Rew();
		GetView()->UpdatePlaySlider();
		GetView()->UpdateWindowSizes(TRUE, TRUE, FALSE);
	}
}

void CVideoAviDoc::OnUpdatePlayFrameBack(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!IsProcessing());
}

void CVideoAviDoc::OnPlayFrameFront() 
{
	FrameFront();
}

void CVideoAviDoc::FrameFront() 
{
	// If Playing -> Stop
	if (m_PlayVideoFileThread.IsAlive() ||
		m_PlayAudioFileThread.IsAlive())
		StopAVI();

	// Get Video Stream
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (pVideoStream)
	{
		if (pVideoStream->GetCurrentFramePos() < ((int)pVideoStream->GetTotalFrames() - 1))
			DisplayFrame(pVideoStream->GetCurrentFramePos() + 1);
	}
	else
	{
		LONGLONG llInc = m_PlayAudioFileThread.GetTotalSamples() / 20;
		LONGLONG llNewPos = m_PlayAudioFileThread.GetCurrentSamplePos() + llInc;
		if (llNewPos < (m_PlayAudioFileThread.GetTotalSamples() - 1))
			m_PlayAudioFileThread.SetCurrentSamplePos(llNewPos);
		else
			m_PlayAudioFileThread.SetCurrentSamplePos((m_PlayAudioFileThread.GetTotalSamples() - 1));
		GetView()->UpdatePlaySlider();
		GetView()->UpdateWindowSizes(TRUE, TRUE, FALSE);
	}
}

void CVideoAviDoc::OnUpdatePlayFrameFront(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!IsProcessing());
}

void CVideoAviDoc::OnPlayFrameBackFast() 
{
	FrameBackFast();
}

void CVideoAviDoc::FrameBackFast(BOOL bVeryFast/*=FALSE*/) 
{
	// If Playing -> Stop
	if (m_PlayVideoFileThread.IsAlive() ||
		m_PlayAudioFileThread.IsAlive())
		StopAVI();

	// Get Video Stream
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (pVideoStream)
	{
		int nNewPos;
		if (pVideoStream->GetTotalKeyFrames() < pVideoStream->GetTotalFrames())
		{
			int nPrevKeyFrame = pVideoStream->GetPrevKeyFrame(pVideoStream->GetCurrentFramePos() - 1);
			if (bVeryFast)
			{
				if (nPrevKeyFrame > 0)
					nPrevKeyFrame = pVideoStream->GetPrevKeyFrame(nPrevKeyFrame - 1);
				if (nPrevKeyFrame > 0)
					nPrevKeyFrame = pVideoStream->GetPrevKeyFrame(nPrevKeyFrame - 1);
				if (nPrevKeyFrame > 0)
					nPrevKeyFrame = pVideoStream->GetPrevKeyFrame(nPrevKeyFrame - 1);
			}
			if (nPrevKeyFrame >= 0)
				nNewPos = nPrevKeyFrame;
			else
				nNewPos = 0;
		}
		else
		{	
			if (bVeryFast)
				nNewPos = pVideoStream->GetCurrentFramePos() - Round(4 * GetPlayFrameRate());
			else
				nNewPos = pVideoStream->GetCurrentFramePos() - Round(GetPlayFrameRate());
			if (nNewPos == pVideoStream->GetCurrentFramePos())
				nNewPos--;
			if (nNewPos < 0)
				nNewPos = 0;
		}
		DisplayFrame(nNewPos);
	}
	else
	{
		LONGLONG llDec;
		if (bVeryFast)
			llDec = m_PlayAudioFileThread.GetTotalSamples() / 5;
		else
			llDec = m_PlayAudioFileThread.GetTotalSamples() / 10;
		LONGLONG llNewPos = m_PlayAudioFileThread.GetCurrentSamplePos() - llDec;
		if (llNewPos >= 0)
			m_PlayAudioFileThread.SetCurrentSamplePos(llNewPos);
		else
			m_PlayAudioFileThread.Rew();
		GetView()->UpdatePlaySlider();
		GetView()->UpdateWindowSizes(TRUE, TRUE, FALSE);
	}
}

void CVideoAviDoc::OnUpdatePlayFrameBackFast(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!IsProcessing());
}

void CVideoAviDoc::OnPlayFrameFrontFast() 
{
	FrameFrontFast();
}

void CVideoAviDoc::FrameFrontFast(BOOL bVeryFast/*=FALSE*/) 
{
	// If Playing -> Stop
	if (m_PlayVideoFileThread.IsAlive() ||
		m_PlayAudioFileThread.IsAlive())
		StopAVI();

	// Get Video Stream
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (pVideoStream)
	{
		int nNewPos;
		if (pVideoStream->GetTotalKeyFrames() < pVideoStream->GetTotalFrames())
		{
			int nNextKeyFrame = pVideoStream->GetNextKeyFrame(pVideoStream->GetCurrentFramePos() + 1);
			if (bVeryFast)
			{
				if (nNextKeyFrame >= 0)
					nNextKeyFrame = pVideoStream->GetNextKeyFrame(nNextKeyFrame + 1);
				if (nNextKeyFrame >= 0)
					nNextKeyFrame = pVideoStream->GetNextKeyFrame(nNextKeyFrame + 1);
				if (nNextKeyFrame >= 0)
					nNextKeyFrame = pVideoStream->GetNextKeyFrame(nNextKeyFrame + 1);
			}
			if (nNextKeyFrame < 0)
				nNewPos = (int)pVideoStream->GetTotalFrames() - 1;
			else
				nNewPos = nNextKeyFrame;
		}
		else
		{
			if (bVeryFast)
				nNewPos = pVideoStream->GetCurrentFramePos() + Round(4 * GetPlayFrameRate());
			else
				nNewPos = pVideoStream->GetCurrentFramePos() + Round(GetPlayFrameRate());
			if (nNewPos == pVideoStream->GetCurrentFramePos())
				nNewPos++;
			if (nNewPos > ((int)pVideoStream->GetTotalFrames() - 1))
				nNewPos = (int)pVideoStream->GetTotalFrames() - 1;
		}
		DisplayFrame(nNewPos);
	}
	else
	{
		LONGLONG llInc;
		if (bVeryFast)
			llInc = m_PlayAudioFileThread.GetTotalSamples() / 5;
		else
			llInc = m_PlayAudioFileThread.GetTotalSamples() / 10;
		LONGLONG llNewPos = m_PlayAudioFileThread.GetCurrentSamplePos() + llInc;
		if (llNewPos < (m_PlayAudioFileThread.GetTotalSamples() - 1))
			m_PlayAudioFileThread.SetCurrentSamplePos(llNewPos);
		else
			m_PlayAudioFileThread.SetCurrentSamplePos(m_PlayAudioFileThread.GetTotalSamples() - 1);
		GetView()->UpdatePlaySlider();
		GetView()->UpdateWindowSizes(TRUE, TRUE, FALSE);
	}
}

void CVideoAviDoc::OnUpdatePlayFrameFrontFast(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!IsProcessing());
}

void CVideoAviDoc::OnPlayFrameFirst() 
{
	JumpToFirstFrame();
}

void CVideoAviDoc::JumpToFirstFrame() 
{
	// If Playing -> Stop
	if (m_PlayVideoFileThread.IsAlive() ||
		m_PlayAudioFileThread.IsAlive())
		StopAVI();

	// Get Video Stream
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (pVideoStream)
		DisplayFrame(0);
	else
	{
		m_PlayAudioFileThread.Rew();
		GetView()->UpdatePlaySlider();
		GetView()->UpdateWindowSizes(TRUE, TRUE, FALSE);
	}
}

void CVideoAviDoc::OnUpdatePlayFrameFirst(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!IsProcessing());
}

void CVideoAviDoc::OnPlayFrameLast() 
{
	JumpToLastFrame();
}

void CVideoAviDoc::JumpToLastFrame() 
{
	// If Playing -> Stop
	if (m_PlayVideoFileThread.IsAlive() ||
		m_PlayAudioFileThread.IsAlive())
		StopAVI();

	// Get Video Stream
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (pVideoStream)
		DisplayFrame(pVideoStream->GetTotalFrames() - 1);
	else
	{
		m_PlayAudioFileThread.SetCurrentSamplePos(m_PlayAudioFileThread.GetTotalSamples() - 1);
		GetView()->UpdatePlaySlider();
		GetView()->UpdateWindowSizes(TRUE, TRUE, FALSE);
	}
}

void CVideoAviDoc::OnUpdatePlayFrameLast(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!IsProcessing());
}

void CVideoAviDoc::ResetPercentDone()
{
	m_nVideoPercentDone = -1;
	m_nAudioPercentDone = -1;
}

void CVideoAviDoc::OnEditStop() 
{
	m_ProcessingThread.Kill_NoBlocking();
}

void CVideoAviDoc::OnUpdateEditStop(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_ProcessingThread.IsRunning());
}

BOOL CVideoAviDoc::ShrinkDocTo(CVideoAviDoc::CShrinkDocTo* pShrinkDocTo) 
{
	// Check
	if (!m_pAVIPlay)
		return FALSE;

	// Shrink
	WAVEFORMATEX WaveFormat;
	WaveFormat.wFormatTag = WAVE_FORMAT_MPEGLAYER3;
	WaveFormat.nChannels = 1;
	WaveFormat.nSamplesPerSec = 22050;
	WaveFormat.nAvgBytesPerSec = 32000 / 8;
	WaveFormat.nBlockAlign = 0;
	WaveFormat.wBitsPerSample = 0;
	WaveFormat.cbSize = 0;
	BOOL res = SaveAsAVCODEC(pShrinkDocTo->m_sOutFileName,
							m_pAVIPlay->GetFileName(),
							DEFAULT_VIDEO_FOURCC,
							DEFAULT_KEYFRAMESRATE,
							DEFAULT_SHRINK_VIDEO_QUALITY,
							&WaveFormat,
							GetView(),
							FALSE,
							&m_ProcessingThread);

	// Notify the Termination
	::PostMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
					WM_SHRINKDOC_TERMINATED,
					(WPARAM)res,
					(LPARAM)this);

	return res;
}

void CVideoAviDoc::StartShrinkDocTo(CString sOutFileName) 
{
	ResetPercentDone();
	m_ShrinkDocToProcessing.m_sOutFileName = sOutFileName;
	m_ProcessingThread.SetProcessingFunct(&m_ShrinkDocToProcessing);
	m_ProcessingThread.Start();
}

void CVideoAviDoc::OnFileInfo() 
{
	// Center if in Full-Screen Mode
	if (!IsProcessing())
		AviInfoDlg(GetView()->m_bFullScreenMode);
}

void CVideoAviDoc::OnUpdateFileInfo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!IsProcessing());
	pCmdUI->SetCheck(m_pAviInfoDlg != NULL ? 1 : 0);
}

void CVideoAviDoc::OnViewTimeposition() 
{
	m_bTimePositionShow = !m_bTimePositionShow;
	GetView()->UpdateWindowSizes(TRUE, TRUE, FALSE);
}

void CVideoAviDoc::OnUpdateViewTimeposition(CCmdUI* pCmdUI) 
{
	BOOL bHasVideo =	m_pAVIPlay				&&
						m_pAVIPlay->HasVideo()	&&
						m_nActiveVideoStream >= 0;
	pCmdUI->Enable(bHasVideo && !IsProcessing());
	pCmdUI->SetCheck(m_bTimePositionShow || !bHasVideo ? 1 : 0);
}

void CVideoAviDoc::OnFileClose() 
{
	CloseDocument();
}

void CVideoAviDoc::OnFileSave() 
{
	
}

void CVideoAviDoc::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(FALSE);
}

void CVideoAviDoc::AviInfoDlg(BOOL bCenterCursor/*=FALSE*/)
{
	if (m_pAviInfoDlg)
	{
		// m_pAviInfoDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		m_pAviInfoDlg->Close();
	}
	else
	{
		m_pAviInfoDlg = new CAviInfoDlg(GetView());
		m_PlayVideoFileThread.DxDrawGDIDisplay(	GetView()->GetSafeHwnd(),
												WM_AVIINFODLG_POPUP,
												(WPARAM)bCenterCursor,
												0);
	}
}

void CVideoAviDoc::PlayVolDlg(BOOL bCenterCursor/*=FALSE*/)
{
	if (m_pOutVolDlg)
	{
		// m_pOutVolDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		m_pOutVolDlg->Close();
	}
	else
	{
		m_pOutVolDlg = new COutVolDlg(GetView());
		m_PlayVideoFileThread.DxDrawGDIDisplay(	GetView()->GetSafeHwnd(),
												WM_PLAYVOLDLG_POPUP,
												(WPARAM)bCenterCursor,
												0);
	}
}

void CVideoAviDoc::OnPlayVol() 
{
	// Center if in Full-Screen Mode
	PlayVolDlg(GetView()->m_bFullScreenMode);
}

void CVideoAviDoc::OnUpdatePlayVol(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	m_pAVIPlay					&&
					m_pAVIPlay->HasAudio()		&&
					(m_nActiveAudioStream >= 0)	&&
					!IsProcessing());
	pCmdUI->SetCheck(m_pOutVolDlg != NULL ? 1 : 0);	
}

void CVideoAviDoc::AudioVideoShiftDlg(BOOL bCenterCursor/*=FALSE*/)
{
	if (m_pAudioVideoShiftDlg)
	{
		// m_pAudioVideoShiftDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		m_pAudioVideoShiftDlg->Close();
	}
	else
	{
		m_pAudioVideoShiftDlg = new CAudioVideoShiftDlg(GetView());
		m_PlayVideoFileThread.DxDrawGDIDisplay(	GetView()->GetSafeHwnd(),
												WM_AVSHIFTDLG_POPUP,
												(WPARAM)bCenterCursor,
												0);
	}
}

void CVideoAviDoc::OnPlayAudiovideoshift() 
{
	// Center if in Full-Screen Mode
	AudioVideoShiftDlg(GetView()->m_bFullScreenMode);
}

void CVideoAviDoc::OnUpdatePlayAudiovideoshift(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	m_pAVIPlay &&
					m_pAVIPlay->HasVideo() &&
					(m_nActiveVideoStream >= 0) &&
					!IsProcessing());
	pCmdUI->SetCheck(m_pAudioVideoShiftDlg != NULL ? 1 : 0);	
}

void CVideoAviDoc::PlayerToolBarDlg(CPoint ptPos)
{
	if (m_pPlayerToolBarDlg)
	{
		// m_pPlayerToolBarDlg pointer is set to NULL
		// from the dialog class (selfdeletion)
		m_pPlayerToolBarDlg->Close();
	}
	else
	{
		m_pPlayerToolBarDlg = new CPlayerToolBarDlg(GetView());
		m_PlayVideoFileThread.DxDrawGDIDisplay(	GetView()->GetSafeHwnd(),
												WM_PLAYERTOOLBARDLG_POPUP,
												(WPARAM)ptPos.x,
												(LPARAM)ptPos.y);
	}
}

void CVideoAviDoc::OnPlayVfwcodecpriority() 
{
	m_bAVCodecPriority = !m_bAVCodecPriority;
	int nCurrentFramePos = 0;
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (pVideoStream)
		nCurrentFramePos = pVideoStream->GetCurrentFramePos();
	if (nCurrentFramePos < 0)
		nCurrentFramePos = 0;
	m_pAVIPlay->m_bAVCodecPriority = m_bAVCodecPriority;
	LoadActiveStreams(nCurrentFramePos);
	UpdateAviInfoDlg();
	GetView()->UpdateWindowSizes(TRUE, TRUE, FALSE);
}

void CVideoAviDoc::OnUpdatePlayVfwcodecpriority(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	m_pAVIPlay							&&
					!IsProcessing()						&&
					!m_PlayVideoFileThread.IsAlive()	&&
					!m_PlayAudioFileThread.IsAlive());
	pCmdUI->SetRadio(!m_bAVCodecPriority);
}

void CVideoAviDoc::OnPlayInternalcodecpriority() 
{
	m_bAVCodecPriority = !m_bAVCodecPriority;
	int nCurrentFramePos = 0;
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (pVideoStream)
		nCurrentFramePos = pVideoStream->GetCurrentFramePos();
	if (nCurrentFramePos < 0)
		nCurrentFramePos = 0;
	m_pAVIPlay->m_bAVCodecPriority = m_bAVCodecPriority;
	LoadActiveStreams(nCurrentFramePos);
	UpdateAviInfoDlg();
	GetView()->UpdateWindowSizes(TRUE, TRUE, FALSE);
}

void CVideoAviDoc::OnUpdatePlayInternalcodecpriority(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	m_pAVIPlay							&&
					!IsProcessing()						&&
					!m_PlayVideoFileThread.IsAlive()	&&
					!m_PlayAudioFileThread.IsAlive());
	pCmdUI->SetRadio(m_bAVCodecPriority);
}

void CVideoAviDoc::OnEditCopy() 
{
	::EnterCriticalSection(&m_csDib);
	if (m_pDib && m_pDib->IsValid())
	{
		CDib Dib(*m_pDib);
		::LeaveCriticalSection(&m_csDib);
		Dib.Decompress(32);
		Dib.EditCopy();
	}
	else
		::LeaveCriticalSection(&m_csDib);
}

void CVideoAviDoc::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	m_pAVIPlay &&
					m_pAVIPlay->HasVideo() &&
					(m_nActiveVideoStream >= 0) &&
					!IsProcessing() &&
					!m_PlayVideoFileThread.IsAlive() &&
					!m_PlayAudioFileThread.IsAlive());
}

void CVideoAviDoc::EditSnapshot() 
{
	OnEditCopy();
	::PostMessage(::AfxGetMainFrame()->GetSafeHwnd(), WM_COMMAND, ID_EDIT_PASTE, 0);
}

void CVideoAviDoc::OnEditSnapshot() 
{
	EditSnapshot();
}

void CVideoAviDoc::OnUpdateEditSnapshot(CCmdUI* pCmdUI) 
{
	OnUpdateEditCopy(pCmdUI);
}

void CVideoAviDoc::UpdateAviInfoDlg()
{
	if (m_pAviInfoDlg)
		m_pAviInfoDlg->UpdateDisplay();
}

void CVideoAviDoc::StartFileMergeAs() 
{
	ResetPercentDone();
	m_ProcessingThread.SetProcessingFunct(&m_FileMergeAsProcessing);
	m_ProcessingThread.Start();
}

BOOL CVideoAviDoc::FileMergeAs() 
{
	// Display the Open Dialog
	TCHAR* OpenFileNames = new TCHAR[MAX_FILEDLG_PATH];
	OpenFileNames[0] = _T('\0');
	CNoVistaFileDlg OpenFilesDlg(TRUE);
	OpenFilesDlg.m_ofn.lpstrFile = OpenFileNames;
	OpenFilesDlg.m_ofn.nMaxFile = MAX_FILEDLG_PATH;
	OpenFilesDlg.m_ofn.lpstrCustomFilter = NULL;
	OpenFilesDlg.m_ofn.Flags |= OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT;
	OpenFilesDlg.m_ofn.lpstrFilter = _T("Avi File (*.avi;*.divx)\0*.avi;*.divx\0");
	if (OpenFilesDlg.DoModal() != IDOK)
	{
		CloseDocumentForce();
		delete [] OpenFileNames;
		return FALSE;
	}

	CSortableStringArray AviFileNames;
	TCHAR* sSource = OpenFileNames;
	TCHAR Path[MAX_PATH] = _T("");
	_tcscpy(Path, (LPCTSTR)sSource);
	while (*sSource != 0)
		sSource++;
	sSource++; // Skip the 0.
	if (*sSource == 0) // If two zeros -> single file selected -> File in Path
	{
		::AfxMessageBox(ML_STRING(1446, "Select Two Or More Files!\n(Tip: Hold Down the Ctrl-Key To Multiple File Select)"));
		CloseDocumentForce();
		delete [] OpenFileNames;
		return FALSE;
	}
	else // multiple file selected, File in FileName
	{
		TCHAR FileName[MAX_PATH] = _T("");

		// Copy File Names
		while (*sSource != 0) // If 0 -> end of file list.
		{
			_tcscpy(FileName, (LPCTSTR)Path);
			_tcscat(FileName, (LPCTSTR)_T("\\"));
			_tcscat(FileName, (LPCTSTR)sSource);
			AviFileNames.Add(CString(FileName));
			while (*sSource != 0)
					sSource++;
			sSource++; // Skip the 0.
		}
	}

	// Sort
	AviFileNames.Sort();

	// Display the Save As Dialog
	TCHAR SaveFileName[MAX_PATH];
	CNoVistaFileDlg SaveFileDlg(FALSE);
	_tcscpy(SaveFileName, _T("Merged.avi"));
	SaveFileDlg.m_ofn.lpstrFile = SaveFileName;
	SaveFileDlg.m_ofn.nMaxFile = MAX_PATH;
	SaveFileDlg.m_ofn.lpstrCustomFilter = NULL;
	SaveFileDlg.m_ofn.lpstrFilter = _T("Avi File (*.avi)\0*.avi\0");
	SaveFileDlg.m_ofn.lpstrDefExt = _T("avi");
	if (SaveFileDlg.DoModal() != IDOK)
	{
		CloseDocumentForce();
		delete [] OpenFileNames;
		return FALSE;
	}

	// Merge
	int ret = AVIFileMergeAVCODEC(	SaveFileName,
									&AviFileNames,
									m_dwVideoCompressorFourCC,
									m_nVideoCompressorKeyframesRate,
									m_fVideoCompressorQuality,
									m_pAudioCompressorWaveFormat,
									GetView(),
									&m_ProcessingThread,
									false);

	// Load Merged Avi
	if (ret == 1)		// Ok
	{
		// Load AVI
		CPostDelayedMessageThread::PostDelayedMessage(
										GetView()->GetSafeHwnd(),
										WM_THREADSAFE_LOAD_AVI,
										THREAD_SAFE_LOAD_AVI_DELAY,
										(WPARAM)(new CString(SaveFileName)),
										(LPARAM)0);

		// Free
		delete [] OpenFileNames;

		return TRUE;
	}
	else if (ret == -1)	// Error
	{
		::DeleteFile(SaveFileName);
		::AfxMessageBox(ML_STRING(1449, "Failed to Merge the Avi Files."), MB_ICONSTOP);
		CloseDocumentForce();
		delete [] OpenFileNames;
		return FALSE;
	}
	else				// Dlg Canceled
	{
		CloseDocumentForce();
		delete [] OpenFileNames;
		return FALSE;
	}
}

// Return Values:
// -1: Error
// 0 : Dlg Canceled
// 1 : Ok
int CVideoAviDoc::AVIFileMergeAVCODEC(	CString sSaveFileName,
										CSortableStringArray* pAviFileNames,
										DWORD& dwVideoCompressorFourCC,
										int& nVideoCompressorKeyframesRate,
										float& fVideoCompressorQuality,
										LPWAVEFORMATEX pAudioCompressorWaveFormat,
										CWnd* pWnd,
										CWorkerThread* pThread,
										bool bShowMessageBoxOnError)
{
	// Check
	if (!pAviFileNames || pAviFileNames->GetSize() <= 0)
		return FALSE;
	
	CAVRec* pAVRec = NULL;
	CAVIPlay* pAVIPlay = NULL;
	CAVIPlay* pFirstAVIPlay = NULL;
	int ret = 0;			// Dlg Canceled
	int i;
	CAVIPlay::CAVIVideoStream* pSrcVideoStream;
	CAVIPlay::CAVIAudioStream* pSrcAudioStream;
	DWORD dwVideoStreamNum;
	DWORD dwAudioStreamNum;
	double dPercentInc = 100.0 / pAviFileNames->GetSize();
	CPercentProgress PercentProgress;
	PercentProgress.dAudioPercentOffset = 0.0;
	PercentProgress.dVideoPercentOffset = 0.0;
	PercentProgress.dAudioPercentSize = dPercentInc;
	PercentProgress.dVideoPercentSize = dPercentInc;
	DWORD dwFirstVideoStreamsCount;
	DWORD dwFirstAudioStreamsCount;
	DWORD dwFirstWidth[MAX_VIDEO_STREAMS];
	DWORD dwFirstHeight[MAX_VIDEO_STREAMS];
	DWORD dwFirstFourCC[MAX_VIDEO_STREAMS];
	WORD wFirstTwoCC[MAX_AUDIO_STREAMS];

	// Loop through all input files
	for (i = 0 ; i < pAviFileNames->GetSize() ; i++)
	{
		// Open AVI
		pAVIPlay = new CAVIPlay;
		if (!pAVIPlay)
		{
			ret = -1;
			goto free;
		}
		pAVIPlay->SetShowMessageBoxOnError(bShowMessageBoxOnError);
		if (!pAVIPlay->Open(pAviFileNames->ElementAt(i)))
		{
			ret = -1;
			goto free;
		}

		// First file?
		if (i == 0)
		{
			ret = SaveAsAVCODECDlgs(sSaveFileName,
									&pAVRec, // This function allocates pAVRec!
									pAVIPlay,
									dwVideoCompressorFourCC,
									nVideoCompressorKeyframesRate,
									fVideoCompressorQuality,
									pAudioCompressorWaveFormat,
									pWnd,
									FALSE,
									&PercentProgress,
									pThread);
			if (ret != 1)
				goto free;

			// Store AVI Type
			dwFirstVideoStreamsCount = pAVIPlay->GetVideoStreamsCount();
			dwFirstAudioStreamsCount = pAVIPlay->GetAudioStreamsCount();
			for (dwVideoStreamNum = 0 ; dwVideoStreamNum < dwFirstVideoStreamsCount ; dwVideoStreamNum++)
			{
				pSrcVideoStream = pAVIPlay->GetVideoStream(dwVideoStreamNum);
				if (pSrcVideoStream)
				{
					dwFirstWidth[dwVideoStreamNum] = pSrcVideoStream->GetWidth();
					dwFirstHeight[dwVideoStreamNum] = pSrcVideoStream->GetHeight();
					dwFirstFourCC[dwVideoStreamNum] = pSrcVideoStream->GetFourCC(true);
				}
			}
			for (dwAudioStreamNum = 0 ; dwAudioStreamNum < dwFirstAudioStreamsCount ; dwAudioStreamNum++)
			{
				pSrcAudioStream = pAVIPlay->GetAudioStream(dwAudioStreamNum);
				if (pSrcAudioStream)
					wFirstTwoCC[dwAudioStreamNum] = pSrcAudioStream->GetFormatTag(true);
			}

			// Init first AVI Play pointer (pAVIPlay set to NULL in next instruction)
			pFirstAVIPlay = pAVIPlay;
		}
		else
		{
			// Check AVI Type, it must be the same!
			if (dwFirstVideoStreamsCount != pAVIPlay->GetVideoStreamsCount() ||
				dwFirstAudioStreamsCount != pAVIPlay->GetAudioStreamsCount())
			{
				ret = -1;
				goto free;
			}
			for (dwVideoStreamNum = 0 ; dwVideoStreamNum < dwFirstVideoStreamsCount ; dwVideoStreamNum++)
			{
				pSrcVideoStream = pAVIPlay->GetVideoStream(dwVideoStreamNum);
				if (pSrcVideoStream)
				{
					if (dwFirstWidth[dwVideoStreamNum] != pSrcVideoStream->GetWidth()	||
						dwFirstHeight[dwVideoStreamNum] != pSrcVideoStream->GetHeight()	||
						dwFirstFourCC[dwVideoStreamNum] != pSrcVideoStream->GetFourCC(true))
					{
						ret = -1;
						goto free;
					}
				}
			}
			for (dwAudioStreamNum = 0 ; dwAudioStreamNum < dwFirstAudioStreamsCount ; dwAudioStreamNum++)
			{
				pSrcAudioStream = pAVIPlay->GetAudioStream(dwAudioStreamNum);
				if (pSrcAudioStream)
				{
					if (wFirstTwoCC[dwAudioStreamNum] != pSrcAudioStream->GetFormatTag(true))
					{
						ret = -1;
						goto free;
					}
				}
			}

			// Save
			if (!SaveAsAVCODECMultiFile(sSaveFileName,
										&pAVRec,
										pAVIPlay,
										dwVideoCompressorFourCC,
										nVideoCompressorKeyframesRate,
										fVideoCompressorQuality,
										pAudioCompressorWaveFormat,
										pWnd,
										FALSE,
										&PercentProgress,
										pThread))
			{
				ret = -1;
				goto free;
			}

			// Free (pAVIPlay set to NULL in next instruction)
			delete pAVIPlay;
		}

		// Clear pointer
		pAVIPlay = NULL;

		// Increment progress offset
		PercentProgress.dAudioPercentOffset += dPercentInc;
		PercentProgress.dVideoPercentOffset += dPercentInc;
	}

free:

	// Close output file and free pointer
	if (pAVRec)
	{
		if (!pAVRec->Close())
			ret = -1;
		delete pAVRec;
	}

	// Restore the original dwRate and dwScale
	if (ret == 1 && pFirstAVIPlay)
	{
		for (dwVideoStreamNum = 0 ; dwVideoStreamNum < pFirstAVIPlay->GetVideoStreamsCount() ; dwVideoStreamNum++)
		{
			pSrcVideoStream = pFirstAVIPlay->GetVideoStream(dwVideoStreamNum);
			if (pSrcVideoStream)
			{
				int dst_rate, dst_scale;
				av_reduce(&dst_scale, &dst_rate, (int64_t)pSrcVideoStream->GetScale(), (int64_t)pSrcVideoStream->GetRate(), INT_MAX);
				CAVIPlay::AviChangeVideoFrameRate(	sSaveFileName,
													dwVideoStreamNum,
													(DWORD)dst_rate,
													(DWORD)dst_scale,
													false);
			}
		}
	}

	// Free
	if (pFirstAVIPlay)
		delete pFirstAVIPlay;
	if (pAVIPlay)
		delete pAVIPlay;
	
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
// CVideoAviDoc diagnostics

#ifdef _DEBUG
void CVideoAviDoc::AssertValid() const
{
	CUImagerDoc::AssertValid();
}

void CVideoAviDoc::Dump(CDumpContext& dc) const
{
	CUImagerDoc::Dump(dc);
}
#endif //_DEBUG