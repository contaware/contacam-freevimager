#include "stdafx.h"
#include "uImager.h"
#include "MainFrm.h"
#include "VideoAviDoc.h"
#include "VideoAviView.h"
#include "VideoDeviceDoc.h"
#include "SaveFileDlg.h"
#include "ResizingDlg.h"
#include "AudioOutDestinationDlg.h"
#include "AviSaveAsStreamsDlg.h"
#include "AnimGifSaveDlg.h"
#include "AviOpenStreamsDlg.h"
#include "AviInfoDlg.h"
#include "OutVolDlg.h"
#include "AudioVideoShiftDlg.h"
#include "PlayerToolBarDlg.h"
#include "AviFileMerge.h"
#include "PostDelayedMessage.h"
#include "Quantizer.h"
#include "JpegCompressionQualityDlg.h"
#include "AnimGifSave.h"
#include "NoVistaFileDlg.h"
#include "VideoFormatDlg.h"
#include "AudioFormatDlg.h"
#if (_MSC_VER <= 1200)
#include "BigFile.h"
#endif
#include "IniFile.h"

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
	//}}AFX_MSG_MAP
#ifdef VIDEODEVICEDOC
	ON_COMMAND(ID_CAPTURE_AVIPLAY, OnCaptureAviplay)
	ON_UPDATE_COMMAND_UI(ID_CAPTURE_AVIPLAY, OnUpdateCaptureAviplay)
#endif
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
	if (dlg.DoModal() == IDOK)
	{
		if (dlg.m_uiDeviceID != m_pDoc->m_dwPlayAudioDeviceID)
		{
			SetDeviceID(dlg.m_uiDeviceID);
			m_pDoc->StopAVI();
			if (m_pDoc->m_pOutVolDlg)
			{
				// m_pOutVolDlg pointer is set to NULL
				// from the dialog class (selfdeletion)
				m_pDoc->m_pOutVolDlg->Close();
			}
			m_pDoc->m_PlayAudioFileThread.WaitDone_Blocking();
			CloseOutAudio();
			if (m_pDoc && m_pDoc->m_pAVIPlay)
			{
				CAVIPlay::CAVIAudioStream* pAudioStream = m_pDoc->m_pAVIPlay->GetAudioStream(m_pDoc->m_nActiveAudioStream);
				if (pAudioStream)
					OpenOutAudio(pAudioStream->GetFormat(false));
			}
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

	// Test for Audio Out availability
	res = ::waveOutGetDevCaps(0, &m_WaveOutDevCaps, sizeof(WAVEOUTCAPS));
	if (res != MMSYSERR_NOERROR)
	{
	   ::AfxMessageBox(ML_STRING(1356, "Sound Output Cannot Determine Card Capabilities!"));
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
	res = ::waveOutOpen(&m_hWaveOut, GetDeviceID(), (pUncompressedWaveFormat != NULL) ? pUncompressedWaveFormat : m_pWaveFormat, (DWORD)m_hWaveOutEvent, NULL, CALLBACK_EVENT); 
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

#ifdef VIDEODEVICEDOC
	// Reset Capture Var
	if (((CUImagerApp*)::AfxGetApp())->IsDoc(m_pDoc->m_pVideoDeviceDoc))
		m_pDoc->m_pVideoDeviceDoc->m_bCapture = FALSE;
#endif

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
				{
#ifdef VIDEODEVICEDOC
					if (((CUImagerApp*)::AfxGetApp())->IsDoc(m_pDoc->m_pVideoDeviceDoc)	&&
						m_pDoc->m_pDib && m_pDoc->m_pDib->IsValid() && m_pDoc->m_pVideoDeviceDocDib)
						*m_pDoc->m_pVideoDeviceDocDib = *m_pDoc->m_pDib;
#endif
					::LeaveCriticalSection(&m_pDoc->m_csDib);
				}
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
				uiTimerId = ::timeSetEventCompatible(	m_uiTimerDelay = nFrameTime,
														0, (LPTIMECALLBACK)m_hTimerEvent, 0,
														TIME_PERIODIC | TIME_CALLBACK_EVENT_SET);
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
					uiTimerId = ::timeSetEventCompatible(	m_uiTimerDelay = (nFrameTime + m_nMilliSecondsCorrectionAvg),
															0, (LPTIMECALLBACK)m_hTimerEvent, 0,
															TIME_PERIODIC | TIME_CALLBACK_EVENT_SET);
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
					uiTimerId = ::timeSetEventCompatible(	m_uiTimerDelay = nRemainingTime,
															0, (LPTIMECALLBACK)m_hTimerEvent, 0,
															TIME_PERIODIC | TIME_CALLBACK_EVENT_SET);

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
					uiTimerId = ::timeSetEventCompatible(	m_uiTimerDelay = (nFrameTime + nTimeCorrection),
															0, (LPTIMECALLBACK)m_hTimerEvent, 0,
															TIME_PERIODIC | TIME_CALLBACK_EVENT_SET);
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
						uiTimerId = ::timeSetEventCompatible(	m_uiTimerDelay = 1,
																0, (LPTIMECALLBACK)m_hTimerEvent, 0,
																TIME_PERIODIC | TIME_CALLBACK_EVENT_SET);
					}
					else
					{
						uiTimerId = ::timeSetEventCompatible(	m_uiTimerDelay = (nFrameTime + nTimeCorrection),
																0, (LPTIMECALLBACK)m_hTimerEvent, 0,
																TIME_PERIODIC | TIME_CALLBACK_EVENT_SET);
					}
					nWaitForCorrectionCountDown = 8;
					nOldMilliSecondsCorrectionAvg = m_nMilliSecondsCorrectionAvg;
				}
			}

			// Send To Capture Doc
#ifdef VIDEODEVICEDOC
			if (((CUImagerApp*)::AfxGetApp())->IsDoc(m_pDoc->m_pVideoDeviceDoc)	&&
				m_pDoc->m_pVideoDeviceDocDib && m_pDoc->m_pVideoDeviceDocDib->IsValid())
			{
				m_pDoc->m_pVideoDeviceDoc->m_bCapture = TRUE;
				m_pDoc->m_pVideoDeviceDoc->ProcessFrame(m_pDoc->m_pVideoDeviceDocDib->GetBits(),
														m_pDoc->m_pVideoDeviceDocDib->GetImageSize());
			}
#endif

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
	m_bAVIFileEditable = FALSE;
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
#ifdef SUPPORT_LIBAVCODEC
	m_bAVCodecPriority = true;
#endif
#ifdef VIDEODEVICEDOC
	m_pVideoDeviceDoc = NULL;
	m_pVideoDeviceDocDib = NULL;
#endif

	// Threads Init
	m_PlayAudioFileThread.SetDoc(this);
	m_PlayVideoFileThread.SetDoc(this);
	m_ProcessingThread.SetDoc(this);
	m_SaveAsProcessing.SetDoc(this);
#if defined (SUPPORT_LIBAVCODEC) && defined (VIDEODEVICEDOC)
	m_PostRecProcessing.SetDoc(this);
#endif
	m_FileMergeSerialAsProcessing.SetDoc(this);
	m_FileMergeParallelAsProcessing.SetDoc(this);
	m_ShrinkDocToProcessing.SetDoc(this);
	m_sProcessingError = _T("");

	// Init Player Loop Sync. Audio / Video
	::InitializeCriticalSection(&m_csPlayWaitingForStart);

	// Audio / Video Play Sync
	m_hPlaySyncEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	// Reset Percent Done
	ResetPercentDone();

	// Load the Settings
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		LoadSettings();
	else
	{
		m_bTimePositionShow = FALSE;
		m_dwPlayAudioDeviceID = 0;
		m_dwVideoCompressorFourCC = FCC('MJPG');
		m_fVideoCompressorQuality = DEFAULT_VIDEO_QUALITY;
		m_nVideoCompressorDataRate = DEFAULT_VIDEO_DATARATE;
		m_nVideoCompressorKeyframesRate = DEFAULT_KEYFRAMESRATE;
		m_nVideoCompressorQualityBitrate = 0;
		m_bDeinterlace = FALSE;

		// Audio Format set Default to: Mono , 11025 Hz , 8 bits
		m_pAudioCompressorWaveFormat = (WAVEFORMATEX*) new BYTE[sizeof(WAVEFORMATEX)];
		m_pAudioCompressorWaveFormat->wFormatTag = WAVE_FORMAT_PCM;
		m_pAudioCompressorWaveFormat->nChannels = 1;
		m_pAudioCompressorWaveFormat->nSamplesPerSec = 11025;
		m_pAudioCompressorWaveFormat->nAvgBytesPerSec = 11025;
		m_pAudioCompressorWaveFormat->nBlockAlign = 1;
		m_pAudioCompressorWaveFormat->wBitsPerSample = 8;
		m_pAudioCompressorWaveFormat->cbSize = 0;
	}

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

#ifdef VIDEODEVICEDOC
	if (m_pVideoDeviceDocDib)
	{
		delete m_pVideoDeviceDocDib;
		m_pVideoDeviceDocDib = NULL;
	}
#endif

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
	m_bTimePositionShow = (int) pApp->GetProfileInt(sSection, _T("TimePositionShow"), FALSE);
	m_bUseDxDraw = (int) pApp->GetProfileInt(sSection, _T("UseDxDraw"), TRUE);
	m_bForceRgb = (int) pApp->GetProfileInt(sSection, _T("ForceRgb"), TRUE);
	m_dwPlayAudioDeviceID = (int) pApp->GetProfileInt(sSection, _T("AudioPlayDeviceID"), 0);
	
	m_dwVideoCompressorFourCC = (DWORD)pApp->GetProfileInt(sSection, _T("VideoCompressorFourCC"), FCC('MJPG'));
	m_fVideoCompressorQuality = (float)pApp->GetProfileInt(sSection, _T("VideoCompressorQuality"), (int)DEFAULT_VIDEO_QUALITY);
	m_nVideoCompressorKeyframesRate = (int)pApp->GetProfileInt(sSection, _T("VideoCompressorKeyframesRate"), DEFAULT_KEYFRAMESRATE);
	m_nVideoCompressorDataRate = (int)pApp->GetProfileInt(sSection, _T("VideoCompressorDataRate"), DEFAULT_VIDEO_DATARATE);
	m_nVideoCompressorQualityBitrate = (int)pApp->GetProfileInt(sSection, _T("VideoCompressorQualityBitrate"), 0);
	m_bDeinterlace = (BOOL)pApp->GetProfileInt(sSection, _T("Deinterlace"), FALSE);

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

#ifdef SUPPORT_LIBAVCODEC
	m_bAVCodecPriority = pApp->GetProfileInt(sSection, _T("AVCodecPriority"), true) ? true : false;
#endif

	// View settings
	sSection = _T("VideoAviView");
	m_PrevUserZoomRect.left   = (int)pApp->GetProfileInt(sSection, _T("UserZoomRectLeft"), 0);
	m_PrevUserZoomRect.top    = (int)pApp->GetProfileInt(sSection, _T("UserZoomRectTop"), 0);
	m_PrevUserZoomRect.right  = (int)pApp->GetProfileInt(sSection, _T("UserZoomRectRight"), 0);
	m_PrevUserZoomRect.bottom = (int)pApp->GetProfileInt(sSection, _T("UserZoomRectBottom"), 0);
}

void CVideoAviDoc::SaveSettings()
{
	CWinApp* pApp = ::AfxGetApp();
	CString sSection;

	if (((CUImagerApp*)::AfxGetApp())->m_bUseRegistry)
	{
		// Doc settings
		sSection = _T("VideoAviDoc");
		pApp->WriteProfileInt(sSection, _T("TimePositionShow"), m_bTimePositionShow);
		pApp->WriteProfileInt(sSection, _T("UseDxDraw"), m_bUseDxDraw);
		pApp->WriteProfileInt(sSection, _T("ForceRgb"), m_bForceRgb);
		pApp->WriteProfileInt(sSection, _T("AudioPlayDeviceID"), m_dwPlayAudioDeviceID);
		
		pApp->WriteProfileInt(sSection, _T("VideoCompressorFourCC"), m_dwVideoCompressorFourCC);
		pApp->WriteProfileInt(sSection, _T("VideoCompressorQuality"), (int)m_fVideoCompressorQuality);
		pApp->WriteProfileInt(sSection, _T("VideoCompressorKeyframesRate"), m_nVideoCompressorKeyframesRate);
		pApp->WriteProfileInt(sSection, _T("VideoCompressorDataRate"), m_nVideoCompressorDataRate);
		pApp->WriteProfileInt(sSection, _T("VideoCompressorQualityBitrate"), m_nVideoCompressorQualityBitrate);
		pApp->WriteProfileInt(sSection, _T("Deinterlace"), m_bDeinterlace);
		
		if (m_pAudioCompressorWaveFormat)
			pApp->WriteProfileBinary(sSection, _T("AudioCompressorWaveFormat"), (LPBYTE)m_pAudioCompressorWaveFormat, sizeof(WAVEFORMATEX));
#ifdef SUPPORT_LIBAVCODEC
		pApp->WriteProfileInt(sSection, _T("AVCodecPriority"), (int)m_bAVCodecPriority);
#endif

		// View settings
		sSection = _T("VideoAviView");
		pApp->WriteProfileInt(sSection, _T("UserZoomRectLeft"), m_PrevUserZoomRect.left);
		pApp->WriteProfileInt(sSection, _T("UserZoomRectTop"), m_PrevUserZoomRect.top);
		pApp->WriteProfileInt(sSection, _T("UserZoomRectRight"), m_PrevUserZoomRect.right);
		pApp->WriteProfileInt(sSection, _T("UserZoomRectBottom"), m_PrevUserZoomRect.bottom);
	}
	else
	{
		// Make a temporary copy because writing to memory sticks is so slow! 
		CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), pApp->m_pszProfileName);
		::WritePrivateProfileString(NULL, NULL, NULL, pApp->m_pszProfileName); // recache
		::CopyFile(pApp->m_pszProfileName, sTempFileName, FALSE);

		// Doc settings
		sSection = _T("VideoAviDoc");
		::WriteProfileIniInt(sSection, _T("TimePositionShow"), m_bTimePositionShow, sTempFileName);
		::WriteProfileIniInt(sSection, _T("UseDxDraw"), m_bUseDxDraw, sTempFileName);
		::WriteProfileIniInt(sSection, _T("ForceRgb"), m_bForceRgb, sTempFileName);
		::WriteProfileIniInt(sSection, _T("AudioPlayDeviceID"), m_dwPlayAudioDeviceID, sTempFileName);
		
		::WriteProfileIniInt(sSection, _T("VideoCompressorFourCC"), m_dwVideoCompressorFourCC, sTempFileName);
		::WriteProfileIniInt(sSection, _T("VideoCompressorQuality"), (int)m_fVideoCompressorQuality, sTempFileName);
		::WriteProfileIniInt(sSection, _T("VideoCompressorKeyframesRate"), m_nVideoCompressorKeyframesRate, sTempFileName);
		::WriteProfileIniInt(sSection, _T("VideoCompressorDataRate"), m_nVideoCompressorDataRate, sTempFileName);
		::WriteProfileIniInt(sSection, _T("VideoCompressorQualityBitrate"), m_nVideoCompressorQualityBitrate, sTempFileName);
		::WriteProfileIniInt(sSection, _T("Deinterlace"), m_bDeinterlace, sTempFileName);
		
		if (m_pAudioCompressorWaveFormat)
			::WriteProfileIniBinary(sSection, _T("AudioCompressorWaveFormat"), (LPBYTE)m_pAudioCompressorWaveFormat, sizeof(WAVEFORMATEX), sTempFileName);
#ifdef SUPPORT_LIBAVCODEC
		::WriteProfileIniInt(sSection, _T("AVCodecPriority"), (int)m_bAVCodecPriority, sTempFileName);
#endif

		// View settings
		sSection = _T("VideoAviView");
		::WriteProfileIniInt(sSection, _T("UserZoomRectLeft"), m_PrevUserZoomRect.left, sTempFileName);
		::WriteProfileIniInt(sSection, _T("UserZoomRectTop"), m_PrevUserZoomRect.top, sTempFileName);
		::WriteProfileIniInt(sSection, _T("UserZoomRectRight"), m_PrevUserZoomRect.right, sTempFileName);
		::WriteProfileIniInt(sSection, _T("UserZoomRectBottom"), m_PrevUserZoomRect.bottom, sTempFileName);

		// Move it
		::DeleteFile(pApp->m_pszProfileName);
		::WritePrivateProfileString(NULL, NULL, NULL, sTempFileName); // recache
		::MoveFile(sTempFileName, pApp->m_pszProfileName);
	}
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
	CDib Dib24;
	TCHAR FileName[MAX_PATH] = _T("");
	_tcscpy(FileName, m_sFileName);
	CAviSaveFileDlg dlgFile(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, GetView());
	TCHAR defext[10] = _T("");
	LPTSTR lpPos = _tcsrchr(FileName, _T('.'));
	if (lpPos != NULL)
		_tcscpy(defext, lpPos+1);
	dlgFile.m_ofn.lpstrFile = FileName;
	dlgFile.m_ofn.nMaxFile = MAX_PATH;
	dlgFile.m_ofn.lpstrDefExt = defext;
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

		if ((extension == _T("bmp")) || (extension == _T("dib")))
		{
#ifdef SUPPORT_BMP
			CString sFirstFileName = SaveAsBMP(FileName);
			if (sFirstFileName != _T(""))
			{
				res = TRUE;
				_tcscpy(FileName, sFirstFileName);
			}
#endif
		}
		else if (extension == _T("png"))
		{
#ifdef SUPPORT_LIBPNG
			CString sFirstFileName = SaveAsPNG(FileName);
			if (sFirstFileName != _T(""))
			{
				res = TRUE;
				_tcscpy(FileName, sFirstFileName);
			}
#endif
		}
		else if ((extension == _T("jpg"))	||
				(extension == _T("jpe"))	||
				(extension == _T("jpeg"))	||
				(extension == _T("thm")))
		{
#ifdef SUPPORT_LIBJPEG
			CJpegCompressionQualityDlg JpegCompressionQualityDlg(GetView());
			JpegCompressionQualityDlg.m_nCompressionQuality = DEFAULT_AVISAVEAS_JPEGCOMPRESSION;
			if (JpegCompressionQualityDlg.DoModal() == IDOK)
			{
				CString sFirstFileName = SaveAsJPEG(FileName,
													JpegCompressionQualityDlg.m_nCompressionQuality,
													JpegCompressionQualityDlg.m_bSaveAsGrayscale);
				if (sFirstFileName != _T(""))
				{
					res = TRUE;
					_tcscpy(FileName, sFirstFileName);
				}
			}
#endif
		}
		else if (extension == _T("gif"))
		{
#ifdef SUPPORT_GIFLIB
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
#endif
		}
		else
		{
			// Vars
			bSaveAsVideoFile = TRUE;
			BOOL bCodecsSelect = FALSE;

			// Avi
#ifdef SUPPORT_LIBAVCODEC
			if (extension == _T("avi")	||
				extension == _T("divx"))
			{	
				if (m_pAVIPlay->HasVBRAudio())
					bCodecsSelect = FALSE;
				else
					bCodecsSelect = TRUE;
			}
#else
			if (extension != _T("avi")	&&
				extension != _T("divx"))
				return FALSE;
#endif

			bool bVideoStreamsSave[MAX_VIDEO_STREAMS];
			bool bVideoStreamsChange[MAX_VIDEO_STREAMS];
			bool bAudioStreamsSave[MAX_AUDIO_STREAMS];
			bool bAudioStreamsChange[MAX_AUDIO_STREAMS];
#ifdef SUPPORT_LIBAVCODEC
			if (extension != _T("avi")	&&
				extension != _T("divx"))
			{	
				for (int i = 0 ; i < MAX_VIDEO_STREAMS ; i++)
				{
					bVideoStreamsSave[i] = true;
					bVideoStreamsChange[i] = true;
					bAudioStreamsSave[i] = true;
					bAudioStreamsChange[i] = true;
				}
				res = SaveAsAVCODEC(FileName,
									TRUE,
									nCurrentFramePos,
									true,	// Try two pass encoding
									bVideoStreamsSave,
									bVideoStreamsChange,
									bAudioStreamsSave,
									bAudioStreamsChange);
			}
			else
#endif
			{
				// Streams selection dialog
				CDWordArray VideoStreamsSave;
				CDWordArray VideoStreamsChange;
				CDWordArray AudioStreamsSave;
				CDWordArray AudioStreamsChange;
				CAviSaveAsStreamsDlg dlgStreams(m_pAVIPlay,
												bCodecsSelect ?
												IDD_SAVEAS_STREAMS_CODEC_SELECT :	// With radio button to select between Internal or VfW codecs
												IDD_SAVEAS_STREAMS_SELECT,
												&VideoStreamsSave,
												&VideoStreamsChange,
												&AudioStreamsSave,
												&AudioStreamsChange,
												GetView());
				if (bCodecsSelect)
					dlgStreams.m_bUseVfwCodecs = FALSE;
				else
				{
	#ifdef SUPPORT_LIBAVCODEC
					dlgStreams.m_bUseVfwCodecs = FALSE;
	#else
					dlgStreams.m_bUseVfwCodecs = TRUE;
	#endif
				}
				if (dlgStreams.DoModal() == IDOK)
				{
					int nStreamNum;
					
					// Init
					for (nStreamNum = 0 ; nStreamNum < MAX_VIDEO_STREAMS ; nStreamNum++)
					{
						bVideoStreamsSave[nStreamNum] = false;
						bVideoStreamsChange[nStreamNum] = false;
					}
					for (nStreamNum = 0 ; nStreamNum < MAX_AUDIO_STREAMS ; nStreamNum++)
					{
						bAudioStreamsSave[nStreamNum] = false;
						bAudioStreamsChange[nStreamNum] = false;
					}
					for (nStreamNum = 0 ; nStreamNum <= VideoStreamsSave.GetUpperBound() ; nStreamNum++)
					{
						if (VideoStreamsSave[nStreamNum])
							bVideoStreamsSave[nStreamNum] = true;
					}
					for (nStreamNum = 0 ; nStreamNum <= VideoStreamsChange.GetUpperBound() ; nStreamNum++)
					{
						if (VideoStreamsChange[nStreamNum])
							bVideoStreamsChange[nStreamNum] = true;
					}
					for (nStreamNum = 0 ; nStreamNum <= AudioStreamsSave.GetUpperBound() ; nStreamNum++)
					{
						if (AudioStreamsSave[nStreamNum])
							bAudioStreamsSave[nStreamNum] = true;
					}
					for (nStreamNum = 0 ; nStreamNum <= AudioStreamsChange.GetUpperBound() ; nStreamNum++)
					{
						if (AudioStreamsChange[nStreamNum])
							bAudioStreamsChange[nStreamNum] = true;
					}

					// Save
					if (dlgStreams.m_bUseVfwCodecs)
					{
						res = SaveAsAviVfW(	FileName,
											bSaveCopyAs,			
											nCurrentFramePos,
											bVideoStreamsSave,
											bVideoStreamsChange,
											bAudioStreamsSave,
											bAudioStreamsChange);
					}
	#ifdef SUPPORT_LIBAVCODEC
					else
					{
						res = SaveAsAVCODEC(FileName,
											bSaveCopyAs,
											nCurrentFramePos,
											true,	// Try two pass encoding
											bVideoStreamsSave,
											bVideoStreamsChange,
											bAudioStreamsSave,
											bAudioStreamsChange);
					}
	#endif
				}
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
				if (bSaveCopyAs)
					RestoreFrame(THREAD_SAFE_UPDATEWINDOWSIZES_DELAY);
				else
					CloseDocumentForce();
			}
			else
			{
				// Show Error Message if not interrupted by user
				if (!m_ProcessingThread.DoExit())
					::AfxMessageBox(ML_STRING(1431, "Error while saving file"), MB_ICONSTOP);

				// Restore Frame
				RestoreFrame(THREAD_SAFE_UPDATEWINDOWSIZES_DELAY);
			}
		}
	}
	
	return res;
}

BOOL CVideoAviDoc::SaveAsAviVfW(	const CString& sFileName,
									BOOL bSaveCopyAs,
									int nCurrentFramePos,
									bool* pbVideoStreamsSave,
									bool* pbVideoStreamsChange,
									bool* pbAudioStreamsSave,
									bool* pbAudioStreamsChange)
{
	// Open CAVIFile
	CAVIFile AVIFile(GetView(), m_pAVIPlay->GetFileName(), false);

	// For Processing Progress Stop
	AVIFile.SetKillEvent(m_ProcessingThread.GetKillEvent());

	// Save to itself?
	CString sDstFileName = sFileName;
	if (m_sFileName.CompareNoCase(sFileName) == 0)
	{
		// Temporary File
		sDstFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);
	}

	// Show Dialog(s) And ReCompress
	if (AVIFile.ReCompress(	sDstFileName,
							pbVideoStreamsSave,
							pbVideoStreamsChange,
							pbAudioStreamsSave,
							pbAudioStreamsChange,
							NULL, NULL, NULL, NULL, NULL, NULL, 0))
	{
		// Close
		AVIFile.Close();

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

			// Open New Document
			::PostMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
							WM_THREADSAFE_OPEN_DOC,
							(WPARAM)(new CString(sFileName)),
							(LPARAM)NULL);
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
		// Close
		AVIFile.Close();

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

#ifdef SUPPORT_LIBAVCODEC

#ifdef VIDEODEVICEDOC
BOOL CVideoAviDoc::PostRecAVCODEC(	const CString& sFileName,
									int nCurrentFramePos,
									DWORD dwVideoCompressorFourCC,
									int nVideoCompressorDataRate,
									int nVideoCompressorKeyframesRate,
									float fVideoCompressorQuality,
									int nVideoCompressorQualityBitrate,
									bool bDeinterlace,
									bool bCloseWhenDone,
									bool b2Pass,
									LPWAVEFORMATEX pAudioCompressorWaveFormat,
									bool* pbVideoStreamsSave,
									bool* pbVideoStreamsChange,
									bool* pbAudioStreamsSave,
									bool* pbAudioStreamsChange)
{
	// Save to itself?
	CString sDstFileName = sFileName;
	if (m_sFileName.CompareNoCase(sFileName) == 0)
	{
		// Temporary File
		sDstFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);
	}

	BOOL res;
	int nPassNumber;
	if (b2Pass)
	{
		nPassNumber = 1;
		if (!SaveAsAVCODEC(	nPassNumber,		// 1: First Pass
							sDstFileName,
							m_sFileName,
							dwVideoCompressorFourCC,
							nVideoCompressorDataRate,
							nVideoCompressorKeyframesRate,
							fVideoCompressorQuality,
							nVideoCompressorQualityBitrate,
							bDeinterlace,
							pAudioCompressorWaveFormat,
							pbVideoStreamsSave,
							pbVideoStreamsChange,
							pbAudioStreamsSave,
							pbAudioStreamsChange,
							GetView(),
							FALSE,
							&m_ProcessingThread))
			return FALSE;
										
		if (nPassNumber != 0)
		{
			nPassNumber = 2;
			res = SaveAsAVCODEC(nPassNumber,	// 2: Second Pass
								sDstFileName,
								m_sFileName,
								dwVideoCompressorFourCC,
								nVideoCompressorDataRate,
								nVideoCompressorKeyframesRate,
								fVideoCompressorQuality,
								nVideoCompressorQualityBitrate,
								bDeinterlace,
								pAudioCompressorWaveFormat,
								pbVideoStreamsSave,
								pbVideoStreamsChange,
								pbAudioStreamsSave,
								pbAudioStreamsChange,
								GetView(),
								FALSE,
								&m_ProcessingThread);
		}
		else
			res = TRUE;	// Only Single Pass Necessary

	}
	else
	{
		nPassNumber = 0;
		res = SaveAsAVCODEC(nPassNumber,		// 0: Single Pass
							sDstFileName,
							m_sFileName,
							dwVideoCompressorFourCC,
							nVideoCompressorDataRate,
							nVideoCompressorKeyframesRate,
							fVideoCompressorQuality,
							nVideoCompressorQualityBitrate,
							bDeinterlace,
							pAudioCompressorWaveFormat,
							pbVideoStreamsSave,
							pbVideoStreamsChange,
							pbAudioStreamsSave,
							pbAudioStreamsChange,
							GetView(),
							FALSE,
							&m_ProcessingThread);
	}

	if (res)
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
				::DeleteFile(sDstFileName); // Delete Tmp File
				e->ReportError();
				e->Delete();
				return FALSE;
			}
		}

		// Close?
		if (bCloseWhenDone)
		{
			CPostDelayedMessageThread::PostDelayedMessage(
											GetFrame()->GetSafeHwnd(),
											WM_CLOSE,
											THREAD_SAFE_CLOSE_AVI_DELAY,
											0, 0);
		}
		else
		{
			// Re-Load AVI
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

		return FALSE;
	}
}
#endif
									
BOOL CVideoAviDoc::SaveAsAVCODEC(	const CString& sFileName,
									BOOL bSaveCopyAs,
									int nCurrentFramePos,
									bool b2Pass,
									bool* pbVideoStreamsSave,
									bool* pbVideoStreamsChange,
									bool* pbAudioStreamsSave,
									bool* pbAudioStreamsChange)
{	
	// Save to itself?
	CString sDstFileName = sFileName;
	if (m_sFileName.CompareNoCase(sFileName) == 0)
	{
		// Temporary File
		sDstFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sFileName);
	}

	int res;
	int nPassNumber;
	if (b2Pass)
	{
		nPassNumber = 1;
		res = SaveAsAVCODECDlgs(nPassNumber,			// 1: First Pass
								sDstFileName,
								pbVideoStreamsSave,
								pbVideoStreamsChange,
								pbAudioStreamsSave,
								pbAudioStreamsChange);
		if (res == 0)
			return FALSE;								// Dlg Canceled
		else if (res == 1 && nPassNumber != 0)			// The above function sets nPassNumber to 0
		{												// if only a single pass is necessary for the specified codec
			nPassNumber = 2;
			res = SaveAsAVCODECDlgs(nPassNumber,		// 2: Second Pass
									sDstFileName,
									pbVideoStreamsSave,
									pbVideoStreamsChange,
									pbAudioStreamsSave,
									pbAudioStreamsChange);
		}
	}
	else
	{
		nPassNumber = 0;
		res = SaveAsAVCODECDlgs(	nPassNumber,		// 0: Single Pass
									sDstFileName,
									pbVideoStreamsSave,
									pbVideoStreamsChange,
									pbAudioStreamsSave,
									pbAudioStreamsChange);
		if (res == 0)
			return FALSE;								// Dlg Canceled
	}

	if (res == 1)
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
int CVideoAviDoc::SaveAsAVCODECDlgs(int& nPassNumber,		// 0: Single Pass, 1: First Pass, 2: Second Pass
									const CString& sDstFileName,
									bool* pbVideoStreamsSave,
									bool* pbVideoStreamsChange,
									bool* pbAudioStreamsSave,
									bool* pbAudioStreamsChange)
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
	int ret = SaveAsAVCODECDlgs(nPassNumber,		// 0: Single Pass, 1: First Pass, 2: Second Pass
								sDstFileName,
								&pAVRec,
								pAVIPlay,
								m_dwVideoCompressorFourCC,
								m_nVideoCompressorDataRate,
								m_nVideoCompressorKeyframesRate,
								m_fVideoCompressorQuality,
								m_nVideoCompressorQualityBitrate,
								m_bDeinterlace,
								m_pAudioCompressorWaveFormat,
								pbVideoStreamsSave,
								pbVideoStreamsChange,
								pbAudioStreamsSave,
								pbAudioStreamsChange,
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
int CVideoAviDoc::SaveAsAVCODECDlgs(int& nPassNumber,		// 0: Single Pass, 1: First Pass, 2: Second Pass
									const CString& sDstFileName,
									CAVRec** ppAVRec,		// If first file *ppAVRec is NULL and will be allocated
									CAVIPlay* pAVIPlay,		// Already Opened Input AVI File
									DWORD& dwVideoCompressorFourCC,
									int& nVideoCompressorDataRate,
									int& nVideoCompressorKeyframesRate,
									float& fVideoCompressorQuality,
									int& nVideoCompressorQualityBitrate,
									BOOL& bDeinterlace,
									LPWAVEFORMATEX pAudioCompressorWaveFormat,
									bool* pbVideoStreamsSave,
									bool* pbVideoStreamsChange,
									bool* pbAudioStreamsSave,
									bool* pbAudioStreamsChange,
									CWnd* pProgressWnd/*=NULL*/,
									BOOL bProgressSend/*=TRUE*/,
									CPercentProgress* pPercentProgress/*=NULL*/,
									CWorkerThread* pThread/*=NULL*/)
{	
	int nTotalVideoStreamsToChange = 0;
	int nTotalAudioStreamsToChange = 0;
	DWORD dwVideoStreamNum, dwAudioStreamNum;
	CAVIPlay::CAVIVideoStream* pSrcVideoStream = NULL;
	CAVIPlay::CAVIAudioStream* pSrcAudioStream = NULL;
	CVideoFormatDlg VideoFormatDlg(pProgressWnd);
	CAudioFormatDlg AudioFormatDlg(pProgressWnd);
	LONGLONG llTotalAudioBytes = 0;

	// Check
	if (!pAVIPlay || !ppAVRec)
		return -1;

	// Remove audio if saving to swf file and no mp3 support,
	// this because the swf ffmpeg muxer only supports mp3!
	if (CUImagerApp::IsSWFFile(sDstFileName) && !((CUImagerApp*)::AfxGetApp())->m_bFFMpegAudioEnc)
	{
		pAudioCompressorWaveFormat = NULL;
		pbAudioStreamsSave = NULL;
		pbAudioStreamsChange = NULL;
	}

	// Count the Video Streams to Compress
	for (dwVideoStreamNum = 0 ; dwVideoStreamNum < pAVIPlay->GetVideoStreamsCount() ; dwVideoStreamNum++)
	{
		if (pbVideoStreamsChange && pbVideoStreamsChange[dwVideoStreamNum])
			nTotalVideoStreamsToChange++;
	}

	// Count the Audio Streams to Compress
	for (dwAudioStreamNum = 0 ; dwAudioStreamNum < pAVIPlay->GetAudioStreamsCount() ; dwAudioStreamNum++)
	{
		if (pbAudioStreamsChange && pbAudioStreamsChange[dwAudioStreamNum])
			nTotalAudioStreamsToChange++;
		else if (pbAudioStreamsSave && pbAudioStreamsSave[dwAudioStreamNum])
		{
			pSrcAudioStream = pAVIPlay->GetAudioStream(dwAudioStreamNum);
			if (pSrcAudioStream)
				llTotalAudioBytes += pSrcAudioStream->GetTotalBytes();
		}
	}

	// Format Dialogs
	if (nPassNumber < 2)
	{
		// Audio Format
		double dAudioLength = 0.0;
		if (pAudioCompressorWaveFormat && nTotalAudioStreamsToChange > 0)
		{
			memcpy(&AudioFormatDlg.m_WaveFormat, pAudioCompressorWaveFormat, sizeof(WAVEFORMATEX));
			for (dwAudioStreamNum = 0 ; dwAudioStreamNum < pAVIPlay->GetAudioStreamsCount() ; dwAudioStreamNum++)
			{
				if (pbAudioStreamsChange && pbAudioStreamsChange[dwAudioStreamNum])
				{
					pSrcAudioStream = pAVIPlay->GetAudioStream(dwAudioStreamNum);
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
						break;
					}
				}
			}
			if (CUImagerApp::IsAVIFile(sDstFileName))
			{
				if (AudioFormatDlg.DoModal() != IDOK)
					return 0;
				memcpy(pAudioCompressorWaveFormat, &AudioFormatDlg.m_WaveFormat, sizeof(WAVEFORMATEX));
			}
			else if (CUImagerApp::IsSWFFile(sDstFileName) && ((CUImagerApp*)::AfxGetApp())->m_bFFMpegAudioEnc)
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

		// Add Total Audio Bytes to Change
		if (pAudioCompressorWaveFormat)
			llTotalAudioBytes += (LONGLONG)(dAudioLength * (double)nTotalAudioStreamsToChange * (double)pAudioCompressorWaveFormat->nAvgBytesPerSec);

		// Video Format
		if (nTotalVideoStreamsToChange > 0)
		{
			if (CUImagerApp::IsAVIFile(sDstFileName))
			{
				DWORD dwFourCC = dwVideoCompressorFourCC;
				double dVideoLength = 0.0;
				int nDataRate = nVideoCompressorDataRate;	// bps
				int nKeyframesRate = nVideoCompressorKeyframesRate;
				for (dwVideoStreamNum = 0 ; dwVideoStreamNum < pAVIPlay->GetVideoStreamsCount() ; dwVideoStreamNum++)
				{
					if (pbVideoStreamsChange && pbVideoStreamsChange[dwVideoStreamNum])
					{
						pSrcVideoStream = pAVIPlay->GetVideoStream(dwVideoStreamNum);
						if (pSrcVideoStream)
						{
							dwFourCC = pSrcVideoStream->GetFourCC(true); 
							dVideoLength = pSrcVideoStream->GetTotalTime();
							if (dwFourCC != BI_RGB			&&
								dwFourCC != FCC('HFYU')		&&
								dwFourCC != FCC('FFVH')		&&
								dwFourCC != FCC('FFV1')		&&
								dwFourCC != FCC('MJPG'))
							{
								// Get from file
								nDataRate = Round((double)pSrcVideoStream->GetTotalBytes() / dVideoLength * 8.0); // bps
								nKeyframesRate = Round((double)pSrcVideoStream->GetTotalFrames() / (double)pSrcVideoStream->GetTotalKeyFrames());

								// Get from previous settings if to big
								if (nDataRate > 2000000)
									nDataRate = nVideoCompressorDataRate;
								
								// Get from previous settings if to small
								if (nKeyframesRate <= 1)
									nKeyframesRate = nVideoCompressorKeyframesRate;
							}
							break;
						}
					}
				}
				VideoFormatDlg.m_dwVideoCompressorFourCC = dwFourCC;
				VideoFormatDlg.m_dVideoLength = dVideoLength;
				VideoFormatDlg.m_llTotalAudioBytes = llTotalAudioBytes;
				VideoFormatDlg.m_nVideoCompressorDataRate = nDataRate / 1000;	// kbps
				VideoFormatDlg.m_nVideoCompressorKeyframesRate = nKeyframesRate;
				VideoFormatDlg.m_fVideoCompressorQuality = fVideoCompressorQuality;
				VideoFormatDlg.m_nQualityBitrate = nVideoCompressorQualityBitrate;
				VideoFormatDlg.m_bDeinterlace = bDeinterlace;
				if (VideoFormatDlg.DoModal() != IDOK)
					return 0;
				bDeinterlace = VideoFormatDlg.m_bDeinterlace;
				nVideoCompressorQualityBitrate = VideoFormatDlg.m_nQualityBitrate;
				fVideoCompressorQuality = VideoFormatDlg.m_fVideoCompressorQuality;
				nVideoCompressorDataRate = VideoFormatDlg.m_nVideoCompressorDataRate * 1000;
				nVideoCompressorKeyframesRate = VideoFormatDlg.m_nVideoCompressorKeyframesRate;
				dwVideoCompressorFourCC = VideoFormatDlg.m_dwVideoCompressorFourCC;
			}
			else if (CUImagerApp::IsSWFFile(sDstFileName))
			{
				DWORD dwFourCC = dwVideoCompressorFourCC;
				double dVideoLength = 0.0;
				int nDataRate = nVideoCompressorDataRate;	// bps
				int nKeyframesRate = nVideoCompressorKeyframesRate;
				pSrcVideoStream = pAVIPlay->GetVideoStream(0);
				if (pSrcVideoStream)
				{
					dwFourCC = pSrcVideoStream->GetFourCC(true); 
					dVideoLength = pSrcVideoStream->GetTotalTime();
					if (dwFourCC != BI_RGB			&&
						dwFourCC != FCC('HFYU')		&&
						dwFourCC != FCC('FFVH')		&&
						dwFourCC != FCC('FFV1')		&&
						dwFourCC != FCC('MJPG'))
					{
						// Get from file
						nDataRate = Round((double)pSrcVideoStream->GetTotalBytes() / dVideoLength * 8.0); // bps
						nKeyframesRate = Round((double)pSrcVideoStream->GetTotalFrames() / (double)pSrcVideoStream->GetTotalKeyFrames());

						// Get from previous settings if to big
						if (nDataRate > 2000000)
							nDataRate = nVideoCompressorDataRate;
						
						// Get from previous settings if to small
						if (nKeyframesRate <= 1)
							nKeyframesRate = nVideoCompressorKeyframesRate;
					}

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
				VideoFormatDlg.m_dVideoLength = dVideoLength;
				VideoFormatDlg.m_llTotalAudioBytes = llTotalAudioBytes;
				VideoFormatDlg.m_nVideoCompressorDataRate = nDataRate / 1000;	// kbps
				VideoFormatDlg.m_nVideoCompressorKeyframesRate = nKeyframesRate;
				VideoFormatDlg.m_fVideoCompressorQuality = fVideoCompressorQuality;
				VideoFormatDlg.m_nQualityBitrate = nVideoCompressorQualityBitrate;
				VideoFormatDlg.m_bDeinterlace = bDeinterlace;
				VideoFormatDlg.m_bShowRawChoose = FALSE;
				VideoFormatDlg.m_nFileType = CVideoFormatDlg::FILETYPE_SWF;
				if (VideoFormatDlg.DoModal() != IDOK)
					return 0;
				bDeinterlace = VideoFormatDlg.m_bDeinterlace;
				nVideoCompressorQualityBitrate = VideoFormatDlg.m_nQualityBitrate;
				fVideoCompressorQuality = VideoFormatDlg.m_fVideoCompressorQuality;
				nVideoCompressorDataRate = VideoFormatDlg.m_nVideoCompressorDataRate * 1000;
				nVideoCompressorKeyframesRate = VideoFormatDlg.m_nVideoCompressorKeyframesRate;
				dwVideoCompressorFourCC = VideoFormatDlg.m_dwVideoCompressorFourCC;
			}
		}
	}

	// Update nPassNumber
	if (nTotalVideoStreamsToChange == 0)
		nPassNumber = 0;

	BOOL res = SaveAsAVCODECMultiFile(	nPassNumber,
										sDstFileName,
										ppAVRec,
										pAVIPlay,
										dwVideoCompressorFourCC,
										nVideoCompressorDataRate,
										nVideoCompressorKeyframesRate,
										fVideoCompressorQuality,
										nVideoCompressorQualityBitrate,
										bDeinterlace ? true : false,
										pAudioCompressorWaveFormat,
										pbVideoStreamsSave,
										pbVideoStreamsChange,
										pbAudioStreamsSave,
										pbAudioStreamsChange,
										pProgressWnd,
										bProgressSend,
										pPercentProgress,
										pThread);
	
	return (res ? 1 : -1);
}

BOOL CVideoAviDoc::SaveAsAVCODEC(	int& nPassNumber,		// 0: Single Pass, 1: First Pass, 2: Second Pass
									const CString& sDstFileName,
									const CString& sSrcFileName,
									DWORD dwVideoCompressorFourCC,
									int nVideoCompressorDataRate,
									int nVideoCompressorKeyframesRate,
									float fVideoCompressorQuality,
									int nVideoCompressorQualityBitrate,
									bool bDeinterlace,
									LPWAVEFORMATEX pAudioCompressorWaveFormat,
									bool* pbVideoStreamsSave,
									bool* pbVideoStreamsChange,
									bool* pbAudioStreamsSave,
									bool* pbAudioStreamsChange,
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

	BOOL res = SaveAsAVCODECSingleFile(	nPassNumber,
										sDstFileName,
										pAVIPlay,
										dwVideoCompressorFourCC,
										nVideoCompressorDataRate,
										nVideoCompressorKeyframesRate,
										fVideoCompressorQuality,
										nVideoCompressorQualityBitrate,
										bDeinterlace,
										pAudioCompressorWaveFormat,
										pbVideoStreamsSave,
										pbVideoStreamsChange,
										pbAudioStreamsSave,
										pbAudioStreamsChange,
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

BOOL CVideoAviDoc::SaveAsAVCODECSingleFile(	int& nPassNumber,		// 0: Single Pass, 1: First Pass, 2: Second Pass
											const CString& sDstFileName,
											CAVIPlay* pAVIPlay,		// Already Opened Input AVI File
											DWORD dwVideoCompressorFourCC,
											int nVideoCompressorDataRate,
											int nVideoCompressorKeyframesRate,
											float fVideoCompressorQuality,
											int nVideoCompressorQualityBitrate,
											bool bDeinterlace,
											LPWAVEFORMATEX pAudioCompressorWaveFormat,
											bool* pbVideoStreamsSave,
											bool* pbVideoStreamsChange,
											bool* pbAudioStreamsSave,
											bool* pbAudioStreamsChange,
											CWnd* pProgressWnd/*=NULL*/,
											BOOL bProgressSend/*=TRUE*/,
											CWorkerThread* pThread/*=NULL*/)
{	
	CAVRec* pAVRec = NULL;
	BOOL res = SaveAsAVCODECMultiFile(	nPassNumber,
										sDstFileName,
										&pAVRec,	// This function allocates pAVRec!
										pAVIPlay,
										dwVideoCompressorFourCC,
										nVideoCompressorDataRate,
										nVideoCompressorKeyframesRate,
										fVideoCompressorQuality,
										nVideoCompressorQualityBitrate,
										bDeinterlace,
										pAudioCompressorWaveFormat,
										pbVideoStreamsSave,
										pbVideoStreamsChange,
										pbAudioStreamsSave,
										pbAudioStreamsChange,
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

// Tested and working with the following YUV FourCCs:
// I420, YV12, YV16, Y800, Y41P, YVU9, YUV9, YUY2, UYVY, YVYU
BOOL CVideoAviDoc::SaveAsAVCODECMultiFile(	int& nPassNumber,		// 0: Single Pass, 1: First Pass, 2: Second Pass
											const CString& sDstFileName,
											CAVRec** ppAVRec,		// If first file *ppAVRec is NULL and will be allocated
											CAVIPlay* pAVIPlay,		// Already Opened Input AVI File
											DWORD dwVideoCompressorFourCC,
											int nVideoCompressorDataRate,
											int nVideoCompressorKeyframesRate,
											float fVideoCompressorQuality,
											int nVideoCompressorQualityBitrate,
											bool bDeinterlace,
											LPWAVEFORMATEX pAudioCompressorWaveFormat,
											bool* pbVideoStreamsSave,
											bool* pbVideoStreamsChange,
											bool* pbAudioStreamsSave,
											bool* pbAudioStreamsChange,
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
	DWORD dwVideoRawChunkPos[MAX_VIDEO_STREAMS];
	DWORD dwAudioRawChunkPos[MAX_AUDIO_STREAMS];
	memset(dwVideoRawChunkPos, 0, MAX_VIDEO_STREAMS * sizeof(DWORD));
	memset(dwAudioRawChunkPos, 0, MAX_AUDIO_STREAMS * sizeof(DWORD));
	DWORD dwSrcBufSizeUsed = 0;
	DWORD dwSrcBufSize;
	LPBYTE pSrcBuf = NULL;
	CDib* pDib = NULL;
	LPBITMAPINFO pBmi = NULL;
	LPWAVEFORMATEX pWaveFormat = NULL;
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

	// Allocate Src Buffer
	dwSrcBufSize = pAVIPlay->GetMaxChunkSize();
	pSrcBuf = new BYTE[dwSrcBufSize + FF_INPUT_BUFFER_PADDING_SIZE];
	if (!pSrcBuf)
		goto error;

	// Update nPassNumber
	if (nVideoCompressorQualityBitrate == 0		||
		(dwVideoCompressorFourCC != FCC('DIVX')	&&
		dwVideoCompressorFourCC != FCC('XVID')	&&
		dwVideoCompressorFourCC != FCC('FLV1')	&&
		dwVideoCompressorFourCC != FCC('H263')	&&
		dwVideoCompressorFourCC != FCC('H264')	&&
		dwVideoCompressorFourCC != FCC('SNOW')))
		nPassNumber = 0; // No two pass mode

	// Video Format
	pDib = new CDib;
	if (!pDib)
		goto error;
	pBmi = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFOHEADER)];
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
		if (!(*ppAVRec)->Init(sDstFileName, nPassNumber, ((CUImagerApp*)::AfxGetApp())->GetAppTempDir()))
			goto error;
		bFirstFile = TRUE;
	}

	// TODO: Get Info from pAVIPlay and set it here
	if (bFirstFile)
	{
		if (!(*ppAVRec)->SetInfo(_T("Title"),
							_T("Author"),
							_T("Copyright"),
							_T("Comment"),
							_T("Album"),
							_T("Genre")))
			goto error;
	}

	// Add Video Stream(s)
	bool bVideoStreamsRawOutput[MAX_VIDEO_STREAMS];
	bool bVideoStreamsRawInput[MAX_VIDEO_STREAMS];
	DWORD dwVideoStreamsCompressFourCC[MAX_VIDEO_STREAMS];
	for (dwVideoStreamNum = 0 ; dwVideoStreamNum < pAVIPlay->GetVideoStreamsCount() ; dwVideoStreamNum++)
	{
		if (pbVideoStreamsSave && pbVideoStreamsSave[dwVideoStreamNum])
		{
			pSrcVideoStream = pAVIPlay->GetVideoStream(dwVideoStreamNum);
			if (pSrcVideoStream)
			{
				bVideoStreamsRawInput[dwVideoStreamNum] = true;
				bVideoStreamsRawOutput[dwVideoStreamNum] = !pbVideoStreamsChange || !pbVideoStreamsChange[dwVideoStreamNum];
				dwVideoStreamsCompressFourCC[dwVideoStreamNum] = 0;
				bool bDecompressorForceRgb = false;
				if (!bVideoStreamsRawOutput[dwVideoStreamNum])
				{
					// Clear raw input flag if not part of the following FourCCs
					// which are supported by CAVRec as input formats
					LPBITMAPINFOHEADER pSrcVideoFormat = (LPBITMAPINFOHEADER)pSrcVideoStream->GetFormat(true);
					bVideoStreamsRawInput[dwVideoStreamNum] =	((pSrcVideoFormat->biCompression == BI_RGB		||
																pSrcVideoFormat->biCompression == BI_BITFIELDS)	&&
																pSrcVideoFormat->biBitCount >= 16)				||	// 4 bpp & 8 bpp have to be converted to 32 bpp!
																													
																pSrcVideoFormat->biCompression == BI_RGB16		||
																pSrcVideoFormat->biCompression == BI_RGB15		||
																pSrcVideoFormat->biCompression == BI_BGR16		||
																pSrcVideoFormat->biCompression == BI_BGR15		||
														
																pSrcVideoFormat->biCompression == FCC('I420')	||
																pSrcVideoFormat->biCompression == FCC('IYUV')	||
																pSrcVideoFormat->biCompression == FCC('YV12')	||

																pSrcVideoFormat->biCompression == FCC('YUNV')	||
																pSrcVideoFormat->biCompression == FCC('VYUY')	||
																pSrcVideoFormat->biCompression == FCC('V422')	||
																pSrcVideoFormat->biCompression == FCC('YUYV')	||
																pSrcVideoFormat->biCompression == FCC('YUY2')	||

																pSrcVideoFormat->biCompression == FCC('UYVY')	||
																pSrcVideoFormat->biCompression == FCC('Y422')	||
																pSrcVideoFormat->biCompression == FCC('UYNV')	||

																pSrcVideoFormat->biCompression == FCC('YVU9')	||
																pSrcVideoFormat->biCompression == FCC('YUV9')	||

																pSrcVideoFormat->biCompression == FCC('Y41B')	||

																pSrcVideoFormat->biCompression == FCC('YV16')	||
																pSrcVideoFormat->biCompression == FCC('Y42B')	||

																pSrcVideoFormat->biCompression == FCC('  Y8')	||
																pSrcVideoFormat->biCompression == FCC('Y800')	||
																pSrcVideoFormat->biCompression == FCC('GREY');

					// Let CAVIPlay decode the YUV formats that are not supported by CAVRec
					if (::IsSupportedYuvToRgbFormat(pSrcVideoFormat->biCompression) &&
						!bVideoStreamsRawInput[dwVideoStreamNum])
						bDecompressorForceRgb = true;

					// If raw or lossless compression selected, if input and output are the same
					// and if no deinterlacing is wanted then we can copy input to output
					bool bCopy = false;
					if (!bDeinterlace)
					{
						if (dwVideoCompressorFourCC == BI_RGB)
						{
							if (pSrcVideoFormat->biCompression == BI_RGB		||
								pSrcVideoFormat->biCompression == BI_BITFIELDS	||
								pSrcVideoFormat->biCompression == BI_RGB16		||
								pSrcVideoFormat->biCompression == BI_RGB15		||
								pSrcVideoFormat->biCompression == BI_BGR16		||
								pSrcVideoFormat->biCompression == BI_BGR15)
							{
								bCopy = true;
								bVideoStreamsRawInput[dwVideoStreamNum] = true;
								bVideoStreamsRawOutput[dwVideoStreamNum] = true;
							}
						}
						else if (pSrcVideoFormat->biCompression == dwVideoCompressorFourCC	&&
								(dwVideoCompressorFourCC == FCC('I420')	||
								dwVideoCompressorFourCC == FCC('YV12')	||
								dwVideoCompressorFourCC == FCC('YV16')	||
								dwVideoCompressorFourCC == FCC('YVU9')	||
								dwVideoCompressorFourCC == FCC('YUV9')	||
								dwVideoCompressorFourCC == FCC('YUY2')	||
								dwVideoCompressorFourCC == FCC('UYVY')	||
								dwVideoCompressorFourCC == FCC('Y800')	||
								dwVideoCompressorFourCC == FCC('YVYU')	||
								dwVideoCompressorFourCC == FCC('Y41P')	||
								dwVideoCompressorFourCC == FCC('HFYU')	||
								dwVideoCompressorFourCC == FCC('FFVH')	||
								dwVideoCompressorFourCC == FCC('FFV1')))
						{
							bCopy = true;
							bVideoStreamsRawInput[dwVideoStreamNum] = true;
							bVideoStreamsRawOutput[dwVideoStreamNum] = true;
						}
					}

					// If not copying
					if (!bCopy)
					{
						// Set Width & Height
						pBmi->bmiHeader.biWidth = pSrcVideoStream->GetWidth();
						pBmi->bmiHeader.biHeight = pSrcVideoStream->GetHeight();

						// RGB32
						if (dwVideoCompressorFourCC == BI_RGB)
						{
							pBmi->bmiHeader.biBitCount = 32;
							pBmi->bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(	pBmi->bmiHeader.biBitCount	*
																				pBmi->bmiHeader.biWidth)	*
																				pBmi->bmiHeader.biHeight;
						}
						// YUV Formats supported by CAVRec
						else if (	dwVideoCompressorFourCC == FCC('I420')	||
									dwVideoCompressorFourCC == FCC('YV12')	||
									dwVideoCompressorFourCC == FCC('YV16')	||
									dwVideoCompressorFourCC == FCC('YVU9')	||
									dwVideoCompressorFourCC == FCC('YUV9')	||
									dwVideoCompressorFourCC == FCC('YUY2')	||
									dwVideoCompressorFourCC == FCC('UYVY'))
						{
							pBmi->bmiHeader.biBitCount = ::FourCCToBpp(dwVideoCompressorFourCC);
							pBmi->bmiHeader.biCompression = dwVideoCompressorFourCC;
							int stride = ::CalcYUVStride(pBmi->bmiHeader.biCompression, pBmi->bmiHeader.biWidth);
							if (stride > 0)
								pBmi->bmiHeader.biSizeImage = ::CalcYUVSize(pBmi->bmiHeader.biCompression,
																			stride,
																			pBmi->bmiHeader.biHeight);
						}
						// YUV Formats not supported by CAVRec, but supported by Compress() of CDib
						// Note: Y800 is supported by CAVRec, but with errors!
						// Note: No De-interlacing because we are using raw output!
						else if (	dwVideoCompressorFourCC == FCC('Y800')	||
									dwVideoCompressorFourCC == FCC('YVYU')	||
									dwVideoCompressorFourCC == FCC('Y41P'))
						{
							pBmi->bmiHeader.biBitCount = ::FourCCToBpp(dwVideoCompressorFourCC);
							pBmi->bmiHeader.biCompression = dwVideoCompressorFourCC;
							int stride = ::CalcYUVStride(pBmi->bmiHeader.biCompression, pBmi->bmiHeader.biWidth);
							if (stride > 0)
								pBmi->bmiHeader.biSizeImage = ::CalcYUVSize(pBmi->bmiHeader.biCompression,
																			stride,
																			pBmi->bmiHeader.biHeight);
							bDecompressorForceRgb = true;
							bVideoStreamsRawInput[dwVideoStreamNum] = false;
							bVideoStreamsRawOutput[dwVideoStreamNum] = true;
							dwVideoStreamsCompressFourCC[dwVideoStreamNum] = pBmi->bmiHeader.biCompression;
						}
					}									
				}

				// Open Input
				pSrcVideoStream->OpenDecompression(bDecompressorForceRgb);	// This changes BI_RGB16 & BI_BGR16 to BI_BITFIELDS
																			// BI_RGB15 & BI_BGR15 to BI_RGB!

				// Add Stream
				if (bFirstFile)
				{
					if (bVideoStreamsRawOutput[dwVideoStreamNum])
					{
						if (dwVideoStreamsCompressFourCC[dwVideoStreamNum] > 0)
						{
							if ((*ppAVRec)->AddRawVideoStream(	pBmi,																		// Video Format
																sizeof(BITMAPINFOHEADER),													// Video Format Size
																pSrcVideoStream->GetRate(),													// Set in avi strh
																pSrcVideoStream->GetScale()) < 0)											// Set in avi strh
								goto error;
						}
						else
						{
							if ((*ppAVRec)->AddRawVideoStream(	pSrcVideoStream->GetFormat(bVideoStreamsRawInput[dwVideoStreamNum]),		// Video Format
																pSrcVideoStream->GetFormatSize(bVideoStreamsRawInput[dwVideoStreamNum]),	// Video Format Size
																pSrcVideoStream->GetRate(),													// Set in avi strh
																pSrcVideoStream->GetScale()) < 0)											// Set in avi strh
								goto error;
						}
					}
					else
					{
						int nQualityBitrate = nVideoCompressorQualityBitrate;
						if (pBmi->bmiHeader.biCompression == FCC('MJPG'))
							nQualityBitrate = 0;
						if ((*ppAVRec)->AddVideoStream(	pSrcVideoStream->GetFormat(bVideoStreamsRawInput[dwVideoStreamNum]),				// Src Video Format
														pBmi,																				// Dst Video Format
														pSrcVideoStream->GetRate(),															// Dst Rate
														pSrcVideoStream->GetScale(),														// Dst Scale
														nQualityBitrate == 1 ? nVideoCompressorDataRate : 0,								// Bitrate in bits/s
														nVideoCompressorKeyframesRate,														// Keyframes Rate
														nQualityBitrate == 0 ? fVideoCompressorQuality : 0.0f) < 0)							// 2.0f best quality, 31.0f worst quality
							goto error;
					}
				}
			}
		}
	}
	
	// Add Audio Stream(s)
	for (dwAudioStreamNum = 0 ; dwAudioStreamNum < pAVIPlay->GetAudioStreamsCount() ; dwAudioStreamNum++)
	{
		if (pbAudioStreamsSave && pbAudioStreamsSave[dwAudioStreamNum])
		{
			pSrcAudioStream = pAVIPlay->GetAudioStream(dwAudioStreamNum);
			if (pSrcAudioStream)
			{
				// Open Input, also necessary for raw copy because
				// m_nVBRSamplesPerChunk is calculated when opening
				// the decompressor. m_nVBRSamplesPerChunk is used by
				// SampleToChunk() for VBR audio.
				pSrcAudioStream->OpenDecompression();
				pSrcAudioStream->SetStart();

				if (bFirstFile)
				{
					bool bRawCopy = !pbAudioStreamsChange || !pbAudioStreamsChange[dwAudioStreamNum] || !pAudioCompressorWaveFormat;
					if (bRawCopy)
					{
						if ((*ppAVRec)->AddRawAudioStream(	pSrcAudioStream->GetFormat(true),		// Wave Format
															pSrcAudioStream->GetFormatSize(true),	// Wave Format Size
															pSrcAudioStream->GetHdr()->dwSampleSize,// Set in avi strh
															pSrcAudioStream->GetRate(),				// Set in avi strh
															pSrcAudioStream->GetScale()) < 0)		// Set in avi strh
							goto error;
					}
					else
					{
						
					
						if ((*ppAVRec)->AddAudioStream(	pSrcAudioStream->GetFormat(false),			// Src Wave Format
														pAudioCompressorWaveFormat) < 0)			// Dst Wave Format	
							goto error;
					}
				}
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
			if (pbVideoStreamsSave && pbVideoStreamsSave[dwVideoStreamNum])
			{
				pSrcVideoStream = pAVIPlay->GetVideoStream(dwVideoStreamNum);
				if (pSrcVideoStream)
				{
					if (bVideoStreamsRawOutput[dwVideoStreamNum])
					{
						if (bVideoStreamsRawInput[dwVideoStreamNum])
						{
							dwSrcBufSizeUsed = dwSrcBufSize;
							if (pSrcVideoStream->GetChunkData(	dwVideoRawChunkPos[dwVideoStreamNum],
																pSrcBuf,
																&dwSrcBufSizeUsed))
							{
								bVideoAvailable = TRUE;
								(*ppAVRec)->AddRawVideoPacket(	dwRecStreamNum,
																dwSrcBufSizeUsed,
																pSrcBuf,
																pSrcVideoStream->IsKeyFrame(dwVideoRawChunkPos[dwVideoStreamNum]),
																bInterleave);
								dwVideoRawChunkPos[dwVideoStreamNum]++;
							}
						}
						else
						{
							if (pSrcVideoStream->GetFrame(pDib))
							{
								bVideoAvailable = TRUE;

								// Compress with CDib
								if (dwVideoStreamsCompressFourCC[dwVideoStreamNum] > 0)
									pDib->Compress(dwVideoStreamsCompressFourCC[dwVideoStreamNum]);

								(*ppAVRec)->AddRawVideoPacket(	dwRecStreamNum,
																pDib->GetImageSize(),
																pDib->GetBits(),
																true,	// Is Keyframe
																bInterleave);
								dwVideoRawChunkPos[dwVideoStreamNum]++;
							}
						}
					}
					else
					{
						if (bVideoStreamsRawInput[dwVideoStreamNum])
						{
							dwSrcBufSizeUsed = dwSrcBufSize;
							if (pSrcVideoStream->GetChunkData(	dwVideoRawChunkPos[dwVideoStreamNum],
																pSrcBuf,
																&dwSrcBufSizeUsed))
							{
								bVideoAvailable = TRUE;
								(*ppAVRec)->AddFrame(	dwRecStreamNum,
														pSrcVideoStream->GetFormat(true),
														pSrcBuf,
														bInterleave,
														bDeinterlace);
								dwVideoRawChunkPos[dwVideoStreamNum]++;
							}
						}
						else
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
														bInterleave,
														bDeinterlace);
							}
						}
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
		}

		// Set Elapsed Time
		if (dFrameRate > 0.0)
			dElapsedTime += 1.0 / dFrameRate;

		// Add audio for Single Pass (nPassNumber == 0) or Second Pass (nPassNumber == 2)
		if (nPassNumber != 1)
		{
			for (dwAudioStreamNum = 0 ; dwAudioStreamNum < pAVIPlay->GetAudioStreamsCount() ; dwAudioStreamNum++)
			{
				if (pbAudioStreamsSave && pbAudioStreamsSave[dwAudioStreamNum])
				{
					pSrcAudioStream = pAVIPlay->GetAudioStream(dwAudioStreamNum);
					if (pSrcAudioStream)
					{
						LONGLONG llSampleNum = (LONGLONG)(dElapsedTime * pSrcAudioStream->GetSampleRate(false));
						if (llSampleNum < 0)
							llSampleNum = 0;
						bool bRawCopy = !pbAudioStreamsChange || !pbAudioStreamsChange[dwAudioStreamNum];
						while (!bVideoAvailable ||
							((int)pSrcAudioStream->SampleToChunk(llSampleNum) >=
							(bRawCopy ? dwAudioRawChunkPos[dwAudioStreamNum] : pSrcAudioStream->GetCurrentChunkPos())))
						{
							if (bRawCopy)
							{
								dwSrcBufSizeUsed = dwSrcBufSize;
								if (pSrcAudioStream->GetChunkData(	dwAudioRawChunkPos[dwAudioStreamNum],
																	pSrcBuf,
																	&dwSrcBufSizeUsed))
								{
									bAudioAvailable = TRUE;
									(*ppAVRec)->AddRawAudioPacket(	dwRecStreamNum,
																	dwSrcBufSizeUsed,
																	pSrcBuf,
																	bInterleave);
									dwAudioRawChunkPos[dwAudioStreamNum]++;
									if (nCurrentAudioChunk < 0)
										nCurrentAudioChunk = dwAudioRawChunkPos[dwAudioStreamNum];
								}
								else
									break;
							}
							else
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
							}
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
			}
		}

		// Video Progress
		dwVideoFrame++;
		if (dwTotalFrames)
		{
			if (pPercentProgress)
			{
				nVideoPercent = pPercentProgress->nVideoPercentOffset +
								(pPercentProgress->nVideoPercentSize * dwVideoFrame / dwTotalFrames);
			}
			else
			{
				// Single pass
				if (nPassNumber == 0)	
					nVideoPercent = 100 * dwVideoFrame / dwTotalFrames;
				// First pass of a 2 pass saving
				else if (nPassNumber == 1)
					nVideoPercent = 50 * dwVideoFrame / dwTotalFrames;
				// Second pass of a 2 pass saving
				else
					nVideoPercent = 50 + (50 * dwVideoFrame / dwTotalFrames);
			}

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
				nAudioPercent = pPercentProgress->nAudioPercentOffset +
								(pPercentProgress->nAudioPercentSize * nCurrentAudioChunk / dwTotalAudioChunks);
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
	if (pSrcBuf)
		delete [] pSrcBuf;
	if (pWaveFormat)
		delete [] pWaveFormat;
	if (pDib)
		delete pDib;
	if (pBmi)
		delete [] pBmi;

	return res;
}

#endif

CString CVideoAviDoc::SaveAsBMP(const CString& sFileName)
{
	CDib Dib;
	CString sFormat;
	CString sFirstFileName;
	CString sCurrentFileName;
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (pVideoStream)
	{
		// Number of Digits for File Names
		double dDigits = log10((double)pVideoStream->GetTotalFrames());
		int nDigits = (int)ceil(dDigits);

		// Save BMP Files
		pVideoStream->Rew();
		int nPercentDone;
		int nPrevPercentDone = -1;
		BOOL bFirst = TRUE;
		for (unsigned int i = 0 ; i < pVideoStream->GetTotalFrames() ; i++)
		{
			if (pVideoStream->GetFrame(&Dib))
			{
				nPercentDone = Round(	(double)pVideoStream->GetNextFramePos() * 100.0 /
										(double)pVideoStream->GetTotalFrames());
				if (nPercentDone > nPrevPercentDone)
					::PostMessage(GetView()->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeVIDEO, (LPARAM)nPercentDone);
				nPrevPercentDone = nPercentDone;

				// Decompress to 24 bpp
				Dib.Decompress(24);

				// Save BMP
				sFormat.Format(_T("_%%0%du.bmp"), nDigits);
				sCurrentFileName.Format(_T("%s") + sFormat, ::GetFileNameNoExt(sFileName), i + 1);
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

CString CVideoAviDoc::SaveAsPNG(const CString& sFileName)
{
	CDib Dib;
	CString sFormat;
	CString sFirstFileName;
	CString sCurrentFileName;
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (pVideoStream)
	{
		// Number of Digits for File Names
		double dDigits = log10((double)pVideoStream->GetTotalFrames());
		int nDigits = (int)ceil(dDigits);

		// Save PNG Files
		pVideoStream->Rew();
		int nPercentDone;
		int nPrevPercentDone = -1;
		BOOL bFirst = TRUE;
		for (unsigned int i = 0 ; i < pVideoStream->GetTotalFrames() ; i++)
		{
			if (pVideoStream->GetFrame(&Dib))
			{
				nPercentDone = Round(	(double)pVideoStream->GetNextFramePos() * 100.0 /
										(double)pVideoStream->GetTotalFrames());
				if (nPercentDone > nPrevPercentDone)
					::PostMessage(GetView()->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeVIDEO, (LPARAM)nPercentDone);
				nPrevPercentDone = nPercentDone;

				// Decompress to 24 bpp
				Dib.Decompress(24);

				// Save PNG
				sFormat.Format(_T("_%%0%du.png"), nDigits);
				sCurrentFileName.Format(_T("%s") + sFormat, ::GetFileNameNoExt(sFileName), i + 1);
				if (!Dib.SavePNG(	sCurrentFileName,
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

CString CVideoAviDoc::SaveAsJPEG(	const CString& sFileName,
									int nCompressionQuality,
									BOOL bGrayscale)
{
	CDib Dib;
	CString sFormat;
	CString sFirstFileName;
	CString sCurrentFileName;
	CAVIPlay::CAVIVideoStream* pVideoStream = m_pAVIPlay->GetVideoStream(m_nActiveVideoStream);
	if (pVideoStream)
	{
		// Number of Digits for File Names
		double dDigits = log10((double)pVideoStream->GetTotalFrames());
		int nDigits = (int)ceil(dDigits);

		// Save JPEG Files
		pVideoStream->Rew();
		int nPercentDone;
		int nPrevPercentDone = -1;
		BOOL bFirst = TRUE;
		for (unsigned int i = 0 ; i < pVideoStream->GetTotalFrames() ; i++)
		{
			if (pVideoStream->GetFrame(&Dib))
			{
				nPercentDone = Round(	(double)pVideoStream->GetNextFramePos() * 100.0 /
										(double)pVideoStream->GetTotalFrames());
				if (nPercentDone > nPrevPercentDone)
					::PostMessage(GetView()->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeVIDEO, (LPARAM)nPercentDone);
				nPrevPercentDone = nPercentDone;

				// Decompress to 24 bpp
				Dib.Decompress(24);

				// Save JPEG
				sFormat.Format(_T("_%%0%du.jpg"), nDigits);
				sCurrentFileName.Format(_T("%s") + sFormat, ::GetFileNameNoExt(sFileName), i + 1);
				if (!Dib.SaveJPEG(	sCurrentFileName,
									nCompressionQuality,
									bGrayscale,
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
		if (!pAllFramesDib->AllocateBits(	24,
											BI_RGB,
											m_DocRect.Width(),
											m_DocRect.Height() * pVideoStream->GetTotalFrames()))
		{
			::AfxMessageBox(ML_STRING(1436, "AVI file is to big to create one unique gif color table,\n") +
							ML_STRING(1437, "please choose the multiple color tables option."));
			delete pAllFramesDib;
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
			ASSERT_VALID(pActiveView);
			pActiveView->ForceCursor();
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
	if (!m_bAVIFileEditable)
		::AfxMessageBox(ML_STRING(1438, "AVI Files with VBR Mp3 Audio cannot be Saved."), MB_ICONINFORMATION);
	else if (((CUImagerApp*)::AfxGetApp())->IsCurrentDocAvailable(TRUE))
	{
		ResetPercentDone();
		m_ProcessingThread.SetProcessingFunct(&m_SaveAsProcessing);
		m_ProcessingThread.Start();
	}
}

void CVideoAviDoc::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	!IsProcessing()	&&
					!AfxGetMainFrame()->m_bFullScreenMode);
}

#if defined (SUPPORT_LIBAVCODEC) && defined (VIDEODEVICEDOC)
void CVideoAviDoc::PostRecProcessing(	const CString& sFileName,
										DWORD dwVideoCompressorFourCC,
										int nVideoCompressorDataRate,
										int nVideoCompressorKeyframesRate,
										float fVideoCompressorQuality,
										int nVideoCompressorQualityBitrate,
										BOOL bDeinterlace,
										BOOL bCloseWhenDone)
{
	m_PostRecProcessing.m_sFileName = sFileName;
	m_PostRecProcessing.m_dwVideoCompressorFourCC = dwVideoCompressorFourCC;
	m_PostRecProcessing.m_nVideoCompressorDataRate = nVideoCompressorDataRate;
	m_PostRecProcessing.m_nVideoCompressorKeyframesRate = nVideoCompressorKeyframesRate;
	m_PostRecProcessing.m_fVideoCompressorQuality = fVideoCompressorQuality;
	m_PostRecProcessing.m_nVideoCompressorQualityBitrate = nVideoCompressorQualityBitrate;
	m_PostRecProcessing.m_bDeinterlace = (bDeinterlace ? true : false);
	m_PostRecProcessing.m_bCloseWhenDone = (bCloseWhenDone ? true : false);
	ResetPercentDone();
	m_ProcessingThread.SetProcessingFunct(&m_PostRecProcessing);
	m_ProcessingThread.Start();
}
#endif

void CVideoAviDoc::OnEditDelete() 
{
	EditDelete(TRUE);
}

void CVideoAviDoc::OnUpdateEditDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	!IsModified() &&
					!IsProcessing() &&
					!AfxGetMainFrame()->m_bFullScreenMode &&
					!m_PlayVideoFileThread.IsAlive() &&
					!m_PlayAudioFileThread.IsAlive());
}

void CVideoAviDoc::EditDelete(BOOL bPrompt)
{
	GetView()->ForceCursor();
	if (bPrompt)
	{
		CString sMsg;
		sMsg.Format(ML_STRING(1266, "Are you sure you want to delete %s ?"), ::GetShortFileName(m_sFileName));
		if (::AfxMessageBox(sMsg, MB_YESNO) == IDYES)
			DeleteDocFile();
	}
	else
		DeleteDocFile();
	GetView()->ForceCursor(FALSE);
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
#if (_MSC_VER <= 1200)
		CBigFile file(sFileName, CFile::modeRead | CFile::shareDenyNone);
		if (file.GetLength64() == 0)
			throw (int)AVI_E_FILEEMPTY;
#else
		CFile file(sFileName, CFile::modeRead | CFile::shareDenyNone);
		if (file.GetLength() == 0)
			throw (int)AVI_E_FILEEMPTY;
#endif
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

	// Init the AVI file editable flag
#ifdef SUPPORT_LIBAVCODEC
	m_bAVIFileEditable = TRUE;
#else
	// With the old VfW Api if VBR Audio the avi file is not editable
	// because the stream copy CAVIFile::SaveCopyAs, which is based
	// on the ::AVISaveV() Api, changes the chunk size!!!
	m_bAVIFileEditable = !m_pAVIPlay->HasVBRAudio();
#endif

	// Set Codec Priority
#ifdef SUPPORT_LIBAVCODEC
	m_pAVIPlay->m_bAVCodecPriority = m_bAVCodecPriority;
#endif

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

		// Init DxDraw

		// Enter CS
		if (m_DxDraw.HasDxDraw())
		{
			// Enter CS
			m_DxDraw.EnterCS();

			// Initialize the DirectDraw Interface
			::EnterCriticalSection(&m_csDib);
			m_DxDraw.Init(	GetView()->GetSafeHwnd(),
							m_DocRect.right,
							m_DocRect.bottom,
							m_pDib->GetBMIH()->biCompression,
							IDB_BITSTREAM_VERA_11);

			// Copy Current Frame To DirectDraw Surface
			if (m_bUseDxDraw && m_DxDraw.IsInit())
				m_DxDraw.RenderDib(m_pDib, GetView()->m_UserZoomRect);
			::LeaveCriticalSection(&m_csDib);

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

void CVideoAviDoc::RenderingSwitch(int nRenderingMode)
{
	m_PlayVideoFileThread.SafePauseDelayedRestart(	GetView()->GetSafeHwnd(),
													WM_RENDERING_SWITCH,
													(WPARAM)nRenderingMode,
													(LPARAM)0,
													(GetPlayFrameRate() > 0.0) ?
													Round(RENDERING_SWITCH_SAFEPAUSED_FRAMES_TIMEOUT * 1000.0 /
													GetPlayFrameRate()) :
													500,
													RENDERING_SWITCH_DELAYEDRESTART_TIMEOUT,
													TRUE);
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

#ifdef SUPPORT_LIBAVCODEC
	// All Streams have to be recompressed
	bool bVideoStreamsSave[MAX_VIDEO_STREAMS];
	bool bVideoStreamsChange[MAX_VIDEO_STREAMS];
	bool bAudioStreamsSave[MAX_AUDIO_STREAMS];
	bool bAudioStreamsChange[MAX_AUDIO_STREAMS];
	DWORD dwStreamNum;
	for (dwStreamNum = 0 ; dwStreamNum < m_pAVIPlay->GetVideoStreamsCount() ; dwStreamNum++)
	{
		bVideoStreamsSave[dwStreamNum] = true;
		bVideoStreamsChange[dwStreamNum] = true;
	}
	for (dwStreamNum = 0 ; dwStreamNum < m_pAVIPlay->GetAudioStreamsCount() ; dwStreamNum++)
	{
		bAudioStreamsSave[dwStreamNum] = true;
		bAudioStreamsChange[dwStreamNum] = true;
	}

	// Single Pass
	int nPassNumber = 0;
	WAVEFORMATEX WaveFormat;
	if (((CUImagerApp*)::AfxGetApp())->m_bFFMpegAudioEnc)
	{
		WaveFormat.wFormatTag = WAVE_FORMAT_MPEGLAYER3;
		WaveFormat.nChannels = 1;
		WaveFormat.nSamplesPerSec = 22050;
		WaveFormat.nAvgBytesPerSec = 32000 / 8;
		WaveFormat.nBlockAlign = 0;
		WaveFormat.wBitsPerSample = 0;
		WaveFormat.cbSize = 0;
	}
	else
	{
		WaveFormat.wFormatTag = WAVE_FORMAT_VORBIS;
		WaveFormat.nChannels = 2;				// Only stereo supported
		WaveFormat.nSamplesPerSec = 22050;
		WaveFormat.nAvgBytesPerSec = 90000 / 8;	// q = 15
		WaveFormat.nBlockAlign = 0;
		WaveFormat.wBitsPerSample = 0;
		WaveFormat.cbSize = 0;
	}
	DWORD dwFourCC =		((CUImagerApp*)::AfxGetApp())->m_bFFMpeg4VideoEnc ? FCC('DIVX') :
							(((CUImagerApp*)::AfxGetApp())->m_bFFSnowVideoEnc ? FCC('SNOW') :
							FCC('theo'));
	float fVideoQuality =	(dwFourCC == FCC('DIVX')) ? 6.0f :
							(dwFourCC == FCC('SNOW')) ? 5.0f :
							23.0f;
	BOOL res = SaveAsAVCODEC(nPassNumber,
							pShrinkDocTo->m_sOutFileName,
							m_pAVIPlay->GetFileName(),
							dwFourCC,
							0,		// Set Bitrate to 0 because we use quality
							DEFAULT_KEYFRAMESRATE,
							fVideoQuality,
							0,		// Use Quality
							false,	// No De-interlace
							&WaveFormat,
							bVideoStreamsSave,
							bVideoStreamsChange,
							bAudioStreamsSave,
							bAudioStreamsChange,
							GetView(),
							FALSE,
							&m_ProcessingThread);

	// Notify the Termination
	::PostMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
					WM_SHRINKDOC_TERMINATED,
					(WPARAM)res,
					(LPARAM)this);

	return res;
#else
	// AVI File
	CAVIFile AVIFile(GetView(), m_pAVIPlay->GetFileName(), false);

	// Audio
	LPWAVEFORMATEX pWaveFormat = NULL; // The following function allocates mem. for the structure
	AVIFile.AutoChooseAudioCompressor(	m_nActiveAudioStream,
										&pWaveFormat);

	// Choose Codec And Open Video Compressor Configuration Dialog
	DWORD dwFourCC = 0;
	if (m_pAVIPlay->HasVideo() && (m_nActiveVideoStream >= 0))
	{
		CAVIFile TempAvi(GetView(), false);
		dwFourCC = TempAvi.AutoChooseVideoCompressor();
		TempAvi.SetVideoCompressor(m_nActiveVideoStream, dwFourCC);
		CString sFourCC = TempAvi.GetFourCCString(m_nActiveVideoStream);
		sFourCC.MakeUpper();
		HIC hIC = TempAvi.OpenVideoCompressor(m_nActiveVideoStream);
	
		if (sFourCC == _T("XVID")	||
			sFourCC == _T("DIVX")	||
			sFourCC == _T("3IV2"))
		{
			::AfxMessageBox(ML_STRING(1443, "In The Next Dialog Box Select A Low Quality\n") +
							ML_STRING(1444, "Or A Low Bitrate (for example: 192 kbit/s) To\n") +
							ML_STRING(1445, "Get A Small Video File Size!"));
			TempAvi.VideoCompressorConfigDialog(hIC);
		}

		TempAvi.CloseVideoCompressor(hIC);
	}

	// All Streams have to be recompressed
	bool bVideoStreamsSave[MAX_VIDEO_STREAMS];
	bool bVideoStreamsChange[MAX_VIDEO_STREAMS];
	bool bAudioStreamsSave[MAX_AUDIO_STREAMS];
	bool bAudioStreamsChange[MAX_AUDIO_STREAMS];
	DWORD dwStreamNum;
	for (dwStreamNum = 0 ; dwStreamNum < m_pAVIPlay->GetVideoStreamsCount() ; dwStreamNum++)
	{
		bVideoStreamsSave[dwStreamNum] = true;
		bVideoStreamsChange[dwStreamNum] = true;
	}
	for (dwStreamNum = 0 ; dwStreamNum < m_pAVIPlay->GetAudioStreamsCount() ; dwStreamNum++)
	{
		bAudioStreamsSave[dwStreamNum] = true;
		bAudioStreamsChange[dwStreamNum] = true;
	}

	// For Processing Progress Stop
	AVIFile.SetKillEvent(m_ProcessingThread.GetKillEvent());

	// ReCompress
	DWORD dwQuality = DEFAULT_VCM_QUALITY;
	DWORD dwKeyframesRate = DEFAULT_KEYFRAMESRATE;
	DWORD dwDataRate = DEFAULT_VCM_DATARATE;
	bool res = AVIFile.ReCompress(	pShrinkDocTo->m_sOutFileName,
									bVideoStreamsSave,
									bVideoStreamsChange,
									bAudioStreamsSave,
									bAudioStreamsChange,
									&dwFourCC,
									pWaveFormat,
									&dwQuality,
									&dwKeyframesRate,
									&dwDataRate,
									NULL,
									0);

	// Clean-up
	delete [] pWaveFormat;

	// Notify the Termination
	::PostMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
					WM_SHRINKDOC_TERMINATED,
					(WPARAM)((res == true) ? TRUE : FALSE),
					(LPARAM)this);

	return ((res == true) ? TRUE : FALSE);
#endif
}

BOOL CVideoAviDoc::StartShrinkDocTo(CString sOutFileName) 
{
	if (!m_bAVIFileEditable)
		return FALSE;
	ResetPercentDone();
	m_ShrinkDocToProcessing.m_sOutFileName = sOutFileName;
	m_ProcessingThread.SetProcessingFunct(&m_ShrinkDocToProcessing);
	return (m_ProcessingThread.Start() == true);
}

void CVideoAviDoc::OnFileInfo() 
{
	// Center if in Full-Screen Mode
	AviInfoDlg(::AfxGetMainFrame()->m_bFullScreenMode);
}

void CVideoAviDoc::OnUpdateFileInfo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!IsProcessing());
}

void CVideoAviDoc::ViewTimeposition()
{
	m_bTimePositionShow = !m_bTimePositionShow;
	GetView()->UpdateWindowSizes(TRUE, TRUE, FALSE);
}

void CVideoAviDoc::OnViewTimeposition() 
{
	ViewTimeposition();
}

void CVideoAviDoc::OnUpdateViewTimeposition(CCmdUI* pCmdUI) 
{
	BOOL bHasVideo =	m_pAVIPlay				&&
						m_pAVIPlay->HasVideo()	&&
						m_nActiveVideoStream >= 0;
	pCmdUI->Enable(bHasVideo && !IsProcessing());
	pCmdUI->SetCheck(m_bTimePositionShow || !bHasVideo ? 1 : 0);
}

#ifdef VIDEODEVICEDOC

void CVideoAviDoc::OnCaptureAviplay() 
{
	if (((CUImagerApp*)::AfxGetApp())->IsDoc(m_pVideoDeviceDoc))
	{
		// Close Video Device Doc
		m_pVideoDeviceDoc->GetFrame()->PostMessage(WM_CLOSE, 0, 0);
	}
	else
	{
		// Switch to GDI!
		if (m_bUseDxDraw)
			::AfxMessageBox(ML_STRING(1763, "Switch to GDI under the View menu"));
		else
		{
			// Open New
			CVideoDeviceDoc* pDoc = (CVideoDeviceDoc*)((CUImagerApp*)::AfxGetApp())->GetVideoDeviceDocTemplate()->OpenDocumentFile(NULL);
			if (pDoc)
			{
				if (!m_pVideoDeviceDocDib)
					m_pVideoDeviceDocDib = new CDib;
				::EnterCriticalSection(&m_csDib);
				if (m_pDib && m_pDib->IsValid() && m_pVideoDeviceDocDib)
					*m_pVideoDeviceDocDib = *m_pDib;
				::LeaveCriticalSection(&m_csDib);
				if (pDoc->OpenVideoAvi(this, m_pVideoDeviceDocDib))
					m_pVideoDeviceDoc = pDoc;
				else
				{
					// Close Video Device Doc
					pDoc->GetFrame()->PostMessage(WM_CLOSE, 0, 0);
				}
			}
		}
	}
}

void CVideoAviDoc::OnUpdateCaptureAviplay(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	m_pAVIPlay &&
					m_pAVIPlay->HasVideo() &&
					(m_nActiveVideoStream >= 0) &&
					!IsProcessing());
	pCmdUI->SetCheck(((CUImagerApp*)::AfxGetApp())->IsDoc(m_pVideoDeviceDoc) ? 1 : 0);
}

#endif

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
	PlayVolDlg(::AfxGetMainFrame()->m_bFullScreenMode);
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
	AudioVideoShiftDlg(::AfxGetMainFrame()->m_bFullScreenMode);
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
#ifdef SUPPORT_LIBAVCODEC
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
#endif
}

void CVideoAviDoc::OnUpdatePlayVfwcodecpriority(CCmdUI* pCmdUI) 
{
#ifdef SUPPORT_LIBAVCODEC
	pCmdUI->Enable(	m_pAVIPlay							&&
					!IsProcessing()						&&
					!m_PlayVideoFileThread.IsAlive()	&&
					!m_PlayAudioFileThread.IsAlive());
	pCmdUI->SetRadio(!m_bAVCodecPriority);
#endif
}

void CVideoAviDoc::OnPlayInternalcodecpriority() 
{
#ifdef SUPPORT_LIBAVCODEC
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
#endif
}

void CVideoAviDoc::OnUpdatePlayInternalcodecpriority(CCmdUI* pCmdUI) 
{
#ifdef SUPPORT_LIBAVCODEC
	pCmdUI->Enable(	m_pAVIPlay							&&
					!IsProcessing()						&&
					!m_PlayVideoFileThread.IsAlive()	&&
					!m_PlayAudioFileThread.IsAlive());
	pCmdUI->SetRadio(m_bAVCodecPriority);
#endif
}

void CVideoAviDoc::OnEditCopy() 
{
	::EnterCriticalSection(&m_csDib);
	if (m_pDib && m_pDib->IsValid())
	{
		CDib Dib24(*m_pDib);
		::LeaveCriticalSection(&m_csDib);
		Dib24.Decompress(24);
		Dib24.EditCopy();
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

void CVideoAviDoc::StartFileMergeSerialAs() 
{
	ResetPercentDone();
	m_ProcessingThread.SetProcessingFunct(&m_FileMergeSerialAsProcessing);
	m_ProcessingThread.Start();
}

void CVideoAviDoc::StartFileMergeParallelAs() 
{	
	ResetPercentDone();
	m_ProcessingThread.SetProcessingFunct(&m_FileMergeParallelAsProcessing);
	m_ProcessingThread.Start();
}

BOOL CVideoAviDoc::FileMergeAs(BOOL bSerial) 
{
	// Display the Open Dialog
	TCHAR* OpenFileNames = new TCHAR[MAX_FILEDLG_PATH];
	OpenFileNames[0] = _T('\0');
	CNoVistaFileDlg OpenFilesDlg(TRUE);
	OpenFilesDlg.m_ofn.lpstrFile = OpenFileNames;
	OpenFilesDlg.m_ofn.nMaxFile = MAX_FILEDLG_PATH;
	OpenFilesDlg.m_ofn.lpstrCustomFilter = NULL;
	OpenFilesDlg.m_ofn.Flags |= OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
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
	SaveFileDlg.m_ofn.Flags |= OFN_EXPLORER;
	SaveFileDlg.m_ofn.lpstrFilter = _T("Avi File (*.avi)\0*.avi\0");
	SaveFileDlg.m_ofn.lpstrDefExt = _T("avi");
	if (SaveFileDlg.DoModal() != IDOK)
	{
		CloseDocumentForce();
		delete [] OpenFileNames;
		return FALSE;
	}

	// Merge
	int ret;
	if (bSerial)
	{
#ifdef SUPPORT_LIBAVCODEC
		ret = AVIFileMergeSerialAVCODEC(SaveFileName,
										&AviFileNames,
										m_dwVideoCompressorFourCC,
										m_nVideoCompressorDataRate,
										m_nVideoCompressorKeyframesRate,
										m_fVideoCompressorQuality,
										m_nVideoCompressorQualityBitrate,
										m_bDeinterlace,
										m_pAudioCompressorWaveFormat,
										GetView(),
										&m_ProcessingThread,
										false);
#else
		bool bReCompressVideo, bReCompressAudio;
		if (::AfxMessageBox(ML_STRING(1447, "Do you want to Compress / Decompress the Video Stream(s)?"), MB_YESNO) == IDYES)
			bReCompressVideo = true;
		else
			bReCompressVideo = false;
		if (::AfxMessageBox(ML_STRING(1448, "Do you want to Compress / Decompress the Audio Stream(s)?"), MB_YESNO) == IDYES)
			bReCompressAudio = true;
		else
			bReCompressAudio = false;
		if (::AVIFileMergeSerialVfW(	SaveFileName,
										&AviFileNames,
										bReCompressVideo,
										bReCompressAudio,
										GetView(),
										m_ProcessingThread.GetKillEvent(),
										false))
			ret = 1;
		else
			ret = -1;
#endif
	}
	else
	{
#ifdef SUPPORT_LIBAVCODEC
		if (AVIFileMergeParallelAVCODEC(	SaveFileName,
											&AviFileNames,
											GetView(),
											&m_ProcessingThread,
											false))
			ret = 1;
		else
			ret = -1;
#else
		if (::AVIFileMergeParallelVfW(SaveFileName,
										&AviFileNames,
										GetView(),
										m_ProcessingThread.GetKillEvent(),
										false))
			ret = 1;
		else
			ret = -1;

#endif
	}

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

#ifdef SUPPORT_LIBAVCODEC

BOOL CVideoAviDoc::AVIFileMergeParallelAVCODEC(	CString sSaveFileName,
												CSortableStringArray* pAviFileNames,
												CWnd* pWnd,
												CWorkerThread* pThread,
												bool bShowMessageBoxOnError) 
{
	// Check
	if (!pAviFileNames)
		return FALSE;

	BOOL res = TRUE;
	BOOL bFirst = TRUE;
	CAVIPlay::CAVIVideoStream* pSrcVideoStream = NULL;
	CAVIPlay::CAVIAudioStream* pSrcAudioStream = NULL;
	DWORD dwVideoStreamNum;
	DWORD dwAudioStreamNum;
	BOOL bVideoAvailable = TRUE;
	BOOL bAudioAvailable = TRUE;
	DWORD dwVideoFrame = 0;
	DWORD dwVideoRawChunkPos[MAX_VIDEO_STREAMS];
	DWORD dwAudioRawChunkPos[MAX_AUDIO_STREAMS];
	memset(dwVideoRawChunkPos, 0, MAX_VIDEO_STREAMS * sizeof(DWORD));
	memset(dwAudioRawChunkPos, 0, MAX_AUDIO_STREAMS * sizeof(DWORD));
	DWORD dwSrcBufSizeUsed = 0;
	DWORD dwSrcBufSize = 0;
	LPBYTE pSrcBuf = NULL;
	CAVRec* pAVRec = NULL;
	double dFrameRate = 0.0;
	double dElapsedTime = 0.0;
	DWORD dwTotalFrames = 0;
	int nVideoPercent = 0;
	int nPrevVideoPercent = -1;
	DWORD dwTotalAudioChunks = 0;
	int nCurrentAudioChunk = -1;
	int nAudioPercent = 0;
	int nPrevAudioPercent = -1;
	const bool bInterleave = false; // Already better done in this function for AVI, do not enable!
	int i;
	CVideoAviDoc::AVIPLAYARRAY AVIPlayArray;
	CAVIPlay::VIDEOSTREAMARRAY VideoStreams;
	CAVIPlay::AUDIOSTREAMARRAY AudioStreams;
	
	// AVRec
	pAVRec = new CAVRec;
	if (!pAVRec)
		goto error;
	if (!pAVRec->Init(sSaveFileName))
		goto error;

	// Set Info
	if (!pAVRec->SetInfo(_T("Title"),
						_T("Author"),
						_T("Copyright"),
						_T("Comment"),
						_T("Album"),
						_T("Genre")))
		goto error;

	// Loop through all input files
	for (i = 0 ; i < pAviFileNames->GetSize() ; i++)
	{
		// Open AVI
		CAVIPlay* pAVIPlay = new CAVIPlay;
		if (!pAVIPlay)
			goto error;
		pAVIPlay->SetShowMessageBoxOnError(bShowMessageBoxOnError);
		if (!pAVIPlay->Open(pAviFileNames->ElementAt(i)))
		{
			delete pAVIPlay;
			goto error;
		}
		AVIPlayArray.Add(pAVIPlay);

		// Src Buffer Size
		if (pAVIPlay->GetMaxChunkSize() > dwSrcBufSize)
			dwSrcBufSize = pAVIPlay->GetMaxChunkSize();
	
		// Add Video Stream(s)
		for (dwVideoStreamNum = 0 ; dwVideoStreamNum < pAVIPlay->GetVideoStreamsCount() ; dwVideoStreamNum++)
		{
			pSrcVideoStream = pAVIPlay->GetVideoStream(dwVideoStreamNum);
			if (pSrcVideoStream)
			{
				// Open Input
				pSrcVideoStream->OpenDecompression(false);	// This changes BI_RGB16 & BI_BGR16 to BI_BITFIELDS
															// BI_RGB15 & BI_BGR15 to BI_RGB!
				if (pAVRec->AddRawVideoStream(	pSrcVideoStream->GetFormat(true),		// Video Format
												pSrcVideoStream->GetFormatSize(true),	// Video Format Size
												pSrcVideoStream->GetRate(),				// Set in avi strh
												pSrcVideoStream->GetScale()) < 0)		// Set in avi strh
					goto error;
				VideoStreams.Add(pSrcVideoStream);
			}
		}
		
		// Add Audio Stream(s)
		for (dwAudioStreamNum = 0 ; dwAudioStreamNum < pAVIPlay->GetAudioStreamsCount() ; dwAudioStreamNum++)
		{
			pSrcAudioStream = pAVIPlay->GetAudioStream(dwAudioStreamNum);
			if (pSrcAudioStream)
			{
				// Open Input, necessary for raw copy because
				// m_nVBRSamplesPerChunk is calculated when opening
				// the decompressor. m_nVBRSamplesPerChunk is used by
				// SampleToChunk() for VBR audio.
				pSrcAudioStream->OpenDecompression();
				if (pAVRec->AddRawAudioStream(	pSrcAudioStream->GetFormat(true),		// Wave Format
												pSrcAudioStream->GetFormatSize(true),	// Wave Format Size
												pSrcAudioStream->GetHdr()->dwSampleSize,// Set in avi strh
												pSrcAudioStream->GetRate(),				// Set in avi strh
												pSrcAudioStream->GetScale()) < 0)		// Set in avi strh
					goto error;
				AudioStreams.Add(pSrcAudioStream);
			}
		}
	}

	// Allocate Src Buffer
	pSrcBuf = new BYTE[dwSrcBufSize + FF_INPUT_BUFFER_PADDING_SIZE];
	if (!pSrcBuf)
		goto error;

	// Open AVRec
	if (!pAVRec->Open())
		goto error;
	
	// Add data to output file
	while (bVideoAvailable || bAudioAvailable)
	{
		// Exit?
		if (pThread->DoExit())
			goto error;

		// Reset available flags
		bVideoAvailable = FALSE;
		bAudioAvailable = FALSE;

		// Add Video
		for (dwVideoStreamNum = 0 ; dwVideoStreamNum < (DWORD)VideoStreams.GetSize() ; dwVideoStreamNum++)
		{
			pSrcVideoStream = VideoStreams.GetAt(dwVideoStreamNum);
			if (pSrcVideoStream)
			{
				dwSrcBufSizeUsed = dwSrcBufSize;
				if (pSrcVideoStream->GetChunkData(	dwVideoRawChunkPos[dwVideoStreamNum],
													pSrcBuf,
													&dwSrcBufSizeUsed))
				{
					bVideoAvailable = TRUE;
					pAVRec->AddRawVideoPacket(	pAVRec->VideoStreamNumToStreamNum(dwVideoStreamNum),
												dwSrcBufSizeUsed,
												pSrcBuf,
												pSrcVideoStream->IsKeyFrame(dwVideoRawChunkPos[dwVideoStreamNum]),
												bInterleave);
					dwVideoRawChunkPos[dwVideoStreamNum]++;
				}

				// Init Vars
				if (dwTotalFrames == 0)
					dwTotalFrames = pSrcVideoStream->GetTotalFrames();
				if (dFrameRate == 0.0)
					dFrameRate = pSrcVideoStream->GetFrameRate();
			}
		}

		// Set Elapsed Time
		if (dFrameRate > 0.0)
			dElapsedTime += 1.0 / dFrameRate;

		// Add Audio
		for (dwAudioStreamNum = 0 ; dwAudioStreamNum < (DWORD)AudioStreams.GetSize() ; dwAudioStreamNum++)
		{
			pSrcAudioStream = AudioStreams.GetAt(dwAudioStreamNum);
			if (pSrcAudioStream)
			{
				LONGLONG llSampleNum = (LONGLONG)(dElapsedTime * pSrcAudioStream->GetSampleRate(false));
				if (llSampleNum < 0)
					llSampleNum = 0;
				while (!bVideoAvailable ||
					((int)pSrcAudioStream->SampleToChunk(llSampleNum) >= dwAudioRawChunkPos[dwAudioStreamNum]))
				{
					dwSrcBufSizeUsed = dwSrcBufSize;
					if (pSrcAudioStream->GetChunkData(	dwAudioRawChunkPos[dwAudioStreamNum],
														pSrcBuf,
														&dwSrcBufSizeUsed))
					{
						bAudioAvailable = TRUE;
						pAVRec->AddRawAudioPacket(	pAVRec->AudioStreamNumToStreamNum(dwAudioStreamNum),
													dwSrcBufSizeUsed,
													pSrcBuf,
													bInterleave);
						dwAudioRawChunkPos[dwAudioStreamNum]++;
						if (nCurrentAudioChunk < 0)
							nCurrentAudioChunk = dwAudioRawChunkPos[dwAudioStreamNum];
					}
					else
						break;
					if (!bVideoAvailable) // Interleave possible multiple audio streams
						break;
				}

				// Init Var
				if (dwTotalAudioChunks == 0)
					dwTotalAudioChunks = pSrcAudioStream->GetTotalChunks();
			}
		}
		bFirst = FALSE;

		// Video Progress
		dwVideoFrame++;
		if (dwTotalFrames)
		{	
			nVideoPercent = 100 * dwVideoFrame / dwTotalFrames;	
			if (nVideoPercent >= 99)
				nVideoPercent = 100;
			if (nVideoPercent > nPrevVideoPercent)
			{
				if (pWnd)
					::PostMessage(pWnd->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeVIDEO, (LPARAM)nVideoPercent);	
				nPrevVideoPercent = nVideoPercent;
			}
		}

		// Audio Progress
		if (dwTotalAudioChunks && nCurrentAudioChunk >= 0)
		{
			nAudioPercent = 100 * nCurrentAudioChunk / dwTotalAudioChunks;
			if (nAudioPercent >= 99)
				nAudioPercent = 100;
			if (nAudioPercent > nPrevAudioPercent)
			{
				if (pWnd)
					::PostMessage(pWnd->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeAUDIO, (LPARAM)nAudioPercent);	
				nPrevAudioPercent = nAudioPercent;
			}
			nCurrentAudioChunk = -1;
		}
	}

	goto free;

error:
	res = FALSE;

free:
	for (i = 0 ; i < AVIPlayArray.GetSize() ; i++)
	{
		if (AVIPlayArray[i])
			delete AVIPlayArray[i];
	}
	if (pSrcBuf)
		delete [] pSrcBuf;
	if (pAVRec)
	{
		if (!pAVRec->Close())
			res = FALSE;
		delete pAVRec;
	}

	return res;
}

// Return Values:
// -1: Error
// 0 : Dlg Canceled
// 1 : Ok
int CVideoAviDoc::AVIFileMergeSerialAVCODEC(	CString sSaveFileName,
												CSortableStringArray* pAviFileNames,
												DWORD& dwVideoCompressorFourCC,
												int& nVideoCompressorDataRate,
												int& nVideoCompressorKeyframesRate,
												float& fVideoCompressorQuality,
												int& nVideoCompressorQualityBitrate,
												BOOL& bDeinterlace,
												LPWAVEFORMATEX pAudioCompressorWaveFormat,
												CWnd* pWnd,
												CWorkerThread* pThread,
												bool bShowMessageBoxOnError) 
{
	// Check
	if (!pAviFileNames)
		return FALSE;
	
	CAVRec* pAVRec = NULL;
	int nPassNumber = 0;	// Single Pass
	int ret = 0;			// Dlg Canceled
	int i;
	CAVIPlay::CAVIVideoStream* pSrcVideoStream;
	CAVIPlay::CAVIAudioStream* pSrcAudioStream;
	DWORD dwVideoStreamNum;
	DWORD dwAudioStreamNum;
	CVideoAviDoc::AVIPLAYARRAY AVIPlayArray;
	CDWordArray VideoStreamsSave;
	CDWordArray VideoStreamsChange;
	CDWordArray AudioStreamsSave;
	CDWordArray AudioStreamsChange;
	int nStreamNum;
	bool bVideoStreamsSave[MAX_VIDEO_STREAMS];
	bool bVideoStreamsChange[MAX_VIDEO_STREAMS];
	bool bAudioStreamsSave[MAX_AUDIO_STREAMS];
	bool bAudioStreamsChange[MAX_AUDIO_STREAMS];
	int nPercentInc = 100 / pAviFileNames->GetSize();
	CPercentProgress PercentProgress;
	PercentProgress.nAudioPercentOffset = 0;
	PercentProgress.nVideoPercentOffset = 0;
	PercentProgress.nAudioPercentSize = nPercentInc;
	PercentProgress.nVideoPercentSize = nPercentInc;
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
		CAVIPlay* pAVIPlay = new CAVIPlay;
		if (!pAVIPlay)
		{
			ret = -1;
			goto free;
		}
		pAVIPlay->SetShowMessageBoxOnError(bShowMessageBoxOnError);
		if (!pAVIPlay->Open(pAviFileNames->ElementAt(i)))
		{
			delete pAVIPlay;
			ret = -1;
			goto free;
		}
		AVIPlayArray.Add(pAVIPlay);

		if (i == 0)
		{
			CAviSaveAsStreamsDlg dlgStreams(pAVIPlay,
											IDD_SAVEAS_STREAMS_SELECT,
											&VideoStreamsSave,
											&VideoStreamsChange,
											&AudioStreamsSave,
											&AudioStreamsChange,
											pWnd);
			if (dlgStreams.DoModal() == IDOK)
			{
				// Init
				for (nStreamNum = 0 ; nStreamNum < MAX_VIDEO_STREAMS ; nStreamNum++)
				{
					bVideoStreamsSave[nStreamNum] = false;
					bVideoStreamsChange[nStreamNum] = false;
				}
				for (nStreamNum = 0 ; nStreamNum < MAX_AUDIO_STREAMS ; nStreamNum++)
				{
					bAudioStreamsSave[nStreamNum] = false;
					bAudioStreamsChange[nStreamNum] = false;
				}
				for (nStreamNum = 0 ; nStreamNum <= VideoStreamsSave.GetUpperBound() ; nStreamNum++)
				{
					if (VideoStreamsSave[nStreamNum])
						bVideoStreamsSave[nStreamNum] = true;
				}
				for (nStreamNum = 0 ; nStreamNum <= VideoStreamsChange.GetUpperBound() ; nStreamNum++)
				{
					if (VideoStreamsChange[nStreamNum])
						bVideoStreamsChange[nStreamNum] = true;
				}
				for (nStreamNum = 0 ; nStreamNum <= AudioStreamsSave.GetUpperBound() ; nStreamNum++)
				{
					if (AudioStreamsSave[nStreamNum])
						bAudioStreamsSave[nStreamNum] = true;
				}
				for (nStreamNum = 0 ; nStreamNum <= AudioStreamsChange.GetUpperBound() ; nStreamNum++)
				{
					if (AudioStreamsChange[nStreamNum])
						bAudioStreamsChange[nStreamNum] = true;
				}
				ret = SaveAsAVCODECDlgs(nPassNumber,
										sSaveFileName,
										&pAVRec, // This function allocates pAVRec!
										pAVIPlay,
										dwVideoCompressorFourCC,
										nVideoCompressorDataRate,
										nVideoCompressorKeyframesRate,
										fVideoCompressorQuality,
										nVideoCompressorQualityBitrate,
										bDeinterlace,
										pAudioCompressorWaveFormat,
										bVideoStreamsSave,
										bVideoStreamsChange,
										bAudioStreamsSave,
										bAudioStreamsChange,
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
			}
			else
			{
				ret = 0;
				goto free;
			}
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
			if (!SaveAsAVCODECMultiFile(nPassNumber,
										sSaveFileName,
										&pAVRec,
										pAVIPlay,
										dwVideoCompressorFourCC,
										nVideoCompressorDataRate,
										nVideoCompressorKeyframesRate,
										fVideoCompressorQuality,
										nVideoCompressorQualityBitrate,
										bDeinterlace ? true : false,
										pAudioCompressorWaveFormat,
										bVideoStreamsSave,
										bVideoStreamsChange,
										bAudioStreamsSave,
										bAudioStreamsChange,
										pWnd,
										FALSE,
										&PercentProgress,
										pThread))
			{
				ret = -1;
				goto free;
			}
		}
		PercentProgress.nAudioPercentOffset += nPercentInc;
		PercentProgress.nVideoPercentOffset += nPercentInc;
	}

free:
	if (pAVRec)
	{
		if (!pAVRec->Close())
			ret = -1;
		delete pAVRec;
	}

	// Restore the original dwRate and dwScale
	if (ret == 1 && AVIPlayArray[0])
	{
		for (dwVideoStreamNum = 0 ; dwVideoStreamNum < AVIPlayArray[0]->GetVideoStreamsCount() ; dwVideoStreamNum++)
		{
			pSrcVideoStream = AVIPlayArray[0]->GetVideoStream(dwVideoStreamNum);
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

	for (i = 0 ; i < AVIPlayArray.GetSize() ; i++)
	{
		if (AVIPlayArray[i])
			delete AVIPlayArray[i];
	}
	
	return ret;
}

#endif

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