#if !defined(AFX_HELPERS_AUDIO_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)
#define AFX_HELPERS_AUDIO_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_

#pragma once

#include "MMSystem.h"
#include "Audioclient.h"
#include "mmdeviceapi.h"

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

// Play an Audio File
extern int MCIPlayFile(HWND hWndNotify, BOOL bStartPlaying, LPCTSTR lpszFileName);
extern MCIERROR MCIPlayDevice(HWND hWndNotify, int nDeviceID);
extern MCIERROR MCICloseDevice(HWND hWndNotify, int nDeviceID);

// Helper function which returns a WORD audio tag from the given wave format
extern WORD GetWaveFormatTag(const LPWAVEFORMATEX pWaveFormat);

// Play a Sinus Wave
extern double GenerateSinus(LPBYTE pData, UINT32 uiNumFrames, const LPWAVEFORMATEX pWaveFormat,
							double dFrequency = 440.0, double dPhase = 0.0, double dAmplitude = 0.3);
extern BOOL PlayFrequency(int nDurationSec, double dFrequency = 440.0, double dAmplitude = 0.3); // blocking function that must be called from the STA thread

// Audio Resampler (only for floating-point format)
class CAudioResampler
{
	public:
		CAudioResampler(){m_pInbuf = NULL; m_nInbufSizeBytes = 0; m_pOutbuf = NULL; m_nOutbufSizeBytes = 0;}
		virtual ~CAudioResampler(){Free();}

		// This function returns the number of audio frames written to m_pOutbuf
		int Resample(	float* pSrc,		// pSrc contains nSrcSize audio frames in floating-point format
						int nSrcSize,		// the number of audio frames (size of such a frame is sizeof(float) * nChannels bytes)
						int nSrcSampleRate,
						int nDstSampleRate,
						int nChannels);
		float* GetOutputBuffer() {return m_pOutbuf;}; // returns the output buffer pointer valid till next Resample() call

	protected:
		void Free();
		void LowPass(float* pSrc, float* pDst, int nSize, int nSrcSampleRate, int nDstSampleRate, int nChannels);
		float* m_pInbuf;
		int m_nInbufSizeBytes;
		float* m_pOutbuf;
		int m_nOutbufSizeBytes;
};

// Audio Buffer Player
// The first use of IAudioClient to access the audio device should be on
// the STA thread. Calls from an MTA thread may result in undefined behavior!
class CAudioPlay
{
	public:
		CAudioPlay(){	m_pEnumerator = NULL;
						m_pDevice = NULL;
						m_pAudioClient = NULL;
						m_pRenderClient = NULL;
						m_pWaveFormat = NULL;
						m_bStarted = FALSE;
						m_uiBufferFrameCount = 0;
						m_hnsBufferDuration = 0;}
		virtual ~CAudioPlay(){Close();}
		BOOL Init(REFERENCE_TIME hnsRequestedBufferDuration = 10000000);		// by default 1 sec buffer (100 ns unit)
		void Close();															// stop playing and free memory
		BOOL Write(LPBYTE pData, UINT32 uiNumFrames);							// write the given amount of frames to the buffer,
																				// Attention: not all frames are written if 
																				// uiNumFrames > GetAvailableSpaceForFrames()
																				// Note: it starts playing when buffer is at least half-full
		LPWAVEFORMATEX GetWaveFormat() const {return m_pWaveFormat;}			// returns the default wave format which has to be used
																				// when feeding the samples through the Write() function
		UINT32 GetBufferFrameCount() const {return m_uiBufferFrameCount;}		// returns the buffer size in frames
		REFERENCE_TIME GetBufferDuration() const {return m_hnsBufferDuration;}	// returns the buffer size in 100 ns units
		UINT32 GetAvailableSpaceForFrames() {									// get the available free space in buffer (unit is frames)
									UINT32 uiNumFramesPadding;
									if (m_pAudioClient && SUCCEEDED(m_pAudioClient->GetCurrentPadding(&uiNumFramesPadding)))
										return m_uiBufferFrameCount - uiNumFramesPadding;
									else
										return 0;
								}

	protected:
		IMMDeviceEnumerator* m_pEnumerator;
		IMMDevice* m_pDevice;
		IAudioClient* m_pAudioClient;
		IAudioRenderClient* m_pRenderClient;
		LPWAVEFORMATEX m_pWaveFormat;
		BOOL m_bStarted;
		UINT32 m_uiBufferFrameCount;
		REFERENCE_TIME m_hnsBufferDuration;
};

#endif // !defined(AFX_HELPERS_AUDIO_H__8FD88286_7192_47B9_B311_4C2F27BF8B85__INCLUDED_)