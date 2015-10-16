#if !defined(AFX_HELPERS_AUDIO_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
#define AFX_HELPERS_AUDIO_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_

#pragma once

#include "streams.h"
#include <MMSystem.h>
#include <Audioclient.h> // to avoid duplicated typedefs by the included ksmedia.h first include streams.h which includes ddraw.h
#include <mmdeviceapi.h>

// The size of an audio frame is specified by the nBlockAlign
// member of the WAVEFORMATEX (or WAVEFORMATEXTENSIBLE).
// The size in bytes of an audio frame equals the number of
// channels in the stream multiplied by the sample size per channel.
// For example, the frame size is four bytes for a stereo stream with
// 16-bit samples.

// The PI Constants
#ifndef PI
#define PI  3.14159265358979323846
#endif
#ifndef PI_F
#define PI_F 3.1415927f
#endif

// Play a Sinus Wave
// Note: blocking function that must be called from the STA thread
extern BOOL PlaySinus(int nDurationSec, double dFrequency = 440.0, double dAmplitude = 0.3);

// Audio Tools
class CAudioTools
{
	public:
		CAudioTools(){	m_pLowpassBuf = NULL; m_nLowpassBufSizeBytes = 0;
						m_pResampleBuf = NULL; m_nResampleBufSizeBytes = 0;
						m_pToFloatBuf = NULL; m_nToFloatBufSizeBytes = 0;
						m_pConvertChannelsBuf = NULL; m_nConvertChannelsBufSizeBytes = 0;}
		virtual ~CAudioTools(){Free();}

		// Helper function which returns a WORD audio tag from the given wave format
		static WORD GetWaveFormatTag(const LPWAVEFORMATEX pWaveFormat);

		// Fill the given buffer with a sinus wave
		static double GenerateSinus(LPBYTE pData, UINT32 uiNumFrames, const LPWAVEFORMATEX pWaveFormat,
									double dFrequency = 440.0, double dPhase = 0.0, double dAmplitude = 0.3);

		// To float format changer
		// returns the format changed buffer pointer valid till next ToFloat() call
		// (returns pSrc if the source format is already float)
		float* ToFloat(	LPBYTE pSrc,				// pSrc contains nNumFrames audio frames in integer format
						int nNumFrames,				// the number of audio frames (size of such a frame is nChannels*wSrcBitsPerSample/2 bytes)
						const LPWAVEFORMATEX pSrcWaveFormat);

		// Channels converter
		// returns the channels converted buffer pointer valid till next ConvertChannels() call
		// (returns pSrc if the channels count is the same for source and destination)
		float* ConvertChannels(	float* pSrc,		// pSrc contains nNumFrames audio frames in floating-point format
								int nNumFrames,		// the number of audio frames (size of such a frame is nChannels*sizeof(float) bytes)
								int nSrcChannels,
								int nDstChannels);

		// Resampler
		// returns the resample buffer pointer valid till next Resample() call
		// (returns pSrc and sets nNumDstFrames equal to nNumSrcFrames if the sample rate is the same for source and destination)
		float* Resample(float* pSrc,				// pSrc contains nNumSrcFrames audio frames in floating-point format
						int nNumSrcFrames,			// the number of input audio frames (size of such a frame is nChannels*sizeof(float) bytes)
						int* pNumDstFrames,			// returns the number of resampled audio frames written to m_pResampleBuf
						int nSrcSampleRate,
						int nDstSampleRate,
						int nChannels);

		// Lowpass filter
		// returns the lowpass filtered buffer pointer valid till next LowPass() call
		float* LowPass(	float* pSrc,				// pSrc contains nNumFrames audio frames in floating-point format
						int nNumFrames,				// the number of audio frames (size of such a frame is nChannels*sizeof(float) bytes)
						int nSrcSampleRate,
						int nDstSampleRate,
						int nChannels);

	protected:
		void Free();
		float* m_pLowpassBuf;
		int m_nLowpassBufSizeBytes;
		float* m_pResampleBuf;
		int m_nResampleBufSizeBytes;
		float* m_pToFloatBuf;
		int m_nToFloatBufSizeBytes;
		float* m_pConvertChannelsBuf;
		int m_nConvertChannelsBufSizeBytes;
};

// Audio Buffer Player
// Note: CoInitialize your thread before using any of the class functions,
//       the first use of IAudioClient to access the audio device should be on the
//       STA thread. Calls from an MTA thread may result in undefined behavior.
#define	FRAMES_CORRECTION_CHECK_INTERVAL_MS		3000U	// ms
#define	FRAMES_CORRECTION_MAX_REL_OFFSET		0.01
class CAudioPlay
{
	public:
		CAudioPlay(){	m_bInit = FALSE;
						m_bStarted = FALSE;
						m_dFramesCorrection = 0.0;
						m_uiBufferFrameCount = 0;
						m_pWaveFormat = NULL;
						m_pEnumerator = NULL;
						m_pDevice = NULL;
						m_pAudioClient = NULL;
						m_pRenderClient = NULL;}
		virtual ~CAudioPlay(){Close();}

		// Init
		BOOL Init(REFERENCE_TIME hnsRequestedBufferDuration = 10000000);		// by default 1 sec buffer (100 ns unit)
		BOOL IsInit() const {return m_bInit;}									// Has Init() been called successfully?

		// Stop playing and free memory
		void Close();
		
		// Write the given amount of frames to the buffer
		// Attention: not all frames are written if uiNumFrames > GetAvailableSpaceForFrames()
		// Note: it starts playing when buffer is at least half-full
		BOOL Write(	LPBYTE pData, UINT32 uiNumFrames,																				
					BOOL bDoFramesOffsetCorrection);							// TRUE: correct timing by repeating or dropping frames
																				// FALSE: no correction applied, poll with GetAvailableSpaceForFrames()
		
		// Returns the default wave format which has to be used
		// when feeding the samples through the Write() function
		LPWAVEFORMATEX GetWaveFormat() const {return m_pWaveFormat;}
		
		// Returns the total buffer size in frames
		UINT32 GetBufferFrameCount() const {return m_uiBufferFrameCount;}

		// Get the available free space in buffer (unit is frames)
		UINT32 GetAvailableSpaceForFrames() {
									UINT32 uiNumFramesPadding;
									if (m_pAudioClient && SUCCEEDED(m_pAudioClient->GetCurrentPadding(&uiNumFramesPadding)))
										return m_uiBufferFrameCount - uiNumFramesPadding;
									else
										return 0;
								}

	protected:
		BOOL m_bInit;
		BOOL m_bStarted;
		DWORD m_dwLastCheckUpTime;
		double m_dFramesCorrection; // if positive we need to add frames, if negative remove frames
		UINT32 m_uiBufferFrameCount;
		LPWAVEFORMATEX m_pWaveFormat;
		IMMDeviceEnumerator* m_pEnumerator;
		IMMDevice* m_pDevice;
		IAudioClient* m_pAudioClient;
		IAudioRenderClient* m_pRenderClient;
};

#endif // !defined(AFX_HELPERS_AUDIO_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)