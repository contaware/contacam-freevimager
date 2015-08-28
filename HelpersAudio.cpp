#include "stdafx.h"
#include "HelpersAudio.h"
#include "Round.h"
#include "Rpc.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib, "winmm.lib")

// Plays a specified file using MCI_OPEN and MCI_PLAY. 
// Returns when playback begins.
// Returns the device id on success,
// on error it returns -1.
int MCIPlayFile(HWND hWndNotify, BOOL bStartPlaying, LPCTSTR lpszFileName)
{
    MCI_OPEN_PARMS mciOpenParms;
	ZeroMemory(&mciOpenParms, sizeof(MCI_OPEN_PARMS));

    // Open the device by specifying the filename.
    // MCI will choose a device capable of playing the specified file.
    mciOpenParms.lpstrElementName = lpszFileName;
    if (mciSendCommand(0,
						MCI_OPEN,
						MCI_OPEN_ELEMENT,
						(DWORD)(LPVOID)&mciOpenParms))
    {
        // Failed to open device.
        return -1;
    }

    // Play?
	if (bStartPlaying)
	{
		if (MCIPlayDevice(hWndNotify, (int)mciOpenParms.wDeviceID))
		{
			MCICloseDevice(hWndNotify, (int)mciOpenParms.wDeviceID);
			return -1;
		}
	}

    return (int)mciOpenParms.wDeviceID;
}

// Begin playback. The window procedure function for the parent 
// window will be notified with an MM_MCINOTIFY message when 
// playback is complete. At this time, the window procedure closes 
// the device.
MCIERROR MCIPlayDevice(HWND hWndNotify, int nDeviceID)
{
	if (nDeviceID >= 0)
	{
		MCI_PLAY_PARMS mciPlayParms;
		ZeroMemory(&mciPlayParms, sizeof(MCI_PLAY_PARMS));
		mciPlayParms.dwCallback = (DWORD)hWndNotify;
		return mciSendCommand((MCIDEVICEID)nDeviceID,
								MCI_PLAY, 
								hWndNotify ? MCI_NOTIFY : 0,
								(DWORD)(LPVOID)&mciPlayParms);
	}
	else
		return MCIERR_INVALID_DEVICE_ID;
}

MCIERROR MCICloseDevice(HWND hWndNotify, int nDeviceID)
{
	if (nDeviceID >= 0)
	{
		MCI_GENERIC_PARMS mciGenericParms;
		mciGenericParms.dwCallback = (DWORD)hWndNotify;
		return mciSendCommand((MCIDEVICEID)nDeviceID,
								MCI_CLOSE, 
								hWndNotify ? MCI_NOTIFY : 0,
								(DWORD)(LPVOID)&mciGenericParms);
	}
	else
		return MCIERR_INVALID_DEVICE_ID;
}

WORD GetWaveFormatTag(const LPWAVEFORMATEX pWaveFormat)
{
	if (pWaveFormat->wFormatTag == WAVE_FORMAT_PCM ||
		(pWaveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE && ((WAVEFORMATEXTENSIBLE*)pWaveFormat)->SubFormat == KSDATAFORMAT_SUBTYPE_PCM))
		return WAVE_FORMAT_PCM;
	else if (pWaveFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ||
			(pWaveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE && ((WAVEFORMATEXTENSIBLE*)pWaveFormat)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
		return WAVE_FORMAT_IEEE_FLOAT;
	else
		return pWaveFormat->wFormatTag;
}

double GenerateSinus(LPBYTE pData, UINT32 uiNumFrames, const LPWAVEFORMATEX pWaveFormat,
					double dFrequency/*=440.0*/, double dPhase/*=0.0*/, double dAmplitude/*=0.3*/)
{
	// Check
	if (!pData || uiNumFrames == 0 || !pWaveFormat)
		return dPhase;

	// Phase increment
    double dPhaseInc = 2.0 * PI * dFrequency / pWaveFormat->nSamplesPerSec;

	// Positive peaks can go up to +1.0, negative peaks to -1.0 and silence are 0.0
	WORD wFormatTag = GetWaveFormatTag(pWaveFormat);
	if (wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
	{
		float* fData = (float*)pData;
		for (UINT32 iFrame = 0 ; iFrame < uiNumFrames ; iFrame++)
		{
			for (int iChannel = 0; iChannel < pWaveFormat->nChannels ; iChannel++)
				*fData++ = (float)(dAmplitude * sin(dPhase));
			dPhase += dPhaseInc;
		}
	}
	else if (wFormatTag == WAVE_FORMAT_PCM)
	{
		// Positive peaks can go up to +32767, negative peaks to -32768 and silence are 0
		if (pWaveFormat->wBitsPerSample == 16)
		{
			short* pData16 = (short*)pData;
			for (UINT32 iFrame = 0 ; iFrame < uiNumFrames ; iFrame++)
			{
				double v = dAmplitude * sin(dPhase);
				for (int iChannel = 0; iChannel < pWaveFormat->nChannels ; iChannel++)
					*pData16++ = (short)(v * SHORT_MAX + 0.5);
				dPhase += dPhaseInc;
			}
		}
		// 8-bit data is in offset binary where positive peaks are 255, negative peaks are 0 and 128 is silence
		// https://en.wikipedia.org/wiki/Offset_binary
		else if (pWaveFormat->wBitsPerSample == 8)
		{
			unsigned char* pData8 = (unsigned char*)pData;
			for (UINT32 iFrame = 0 ; iFrame < uiNumFrames ; iFrame++)
			{
				double v = dAmplitude * sin(dPhase);
				for (int iChannel = 0; iChannel < pWaveFormat->nChannels ; iChannel++)
				{
					int sample = (int)(v * INT8_MAX + 0.5);
					*pData8++ = (unsigned char)(sample + 128);
				}
				dPhase += dPhaseInc;
			}
		}
	}

	// Return phase back into range [0, 2.0*PI]
	// to avoid an overflow when looping this function
	return fmod(dPhase, 2.0*PI);
}

BOOL PlayFrequency(int nDurationSec, double dFrequency/*=440.0*/, double dAmplitude/*=0.3*/)
{
	// Init with 1 sec buffer
	CAudioPlay AudioPlay;
	if (!AudioPlay.Init())
		return FALSE;

	// Allocate 1 sec buffer
	int nBufSizeBytes = AudioPlay.GetBufferFrameCount() * AudioPlay.GetWaveFormat()->nBlockAlign;
	LPBYTE pData = (LPBYTE)BIGALLOC(nBufSizeBytes);
	if (!pData)
		return FALSE;

	// Init one second of sinus data
	nDurationSec--;
	nDurationSec *= 2; // duration in units of 500 ms
	double dPhase = 0.0;
	UINT32 uiAvailableSpaceForFrames = AudioPlay.GetAvailableSpaceForFrames();
	dPhase = GenerateSinus(pData, uiAvailableSpaceForFrames, AudioPlay.GetWaveFormat(), dFrequency, dPhase, dAmplitude);
	AudioPlay.Write(pData, uiAvailableSpaceForFrames);
    
	// Loop
	while (nDurationSec > 0)
    {
        // Sleep for half second
        Sleep(500);
		nDurationSec--;

        // Get next portion of sinus data
		uiAvailableSpaceForFrames = AudioPlay.GetAvailableSpaceForFrames();
		dPhase = GenerateSinus(pData, uiAvailableSpaceForFrames, AudioPlay.GetWaveFormat(), dFrequency, dPhase, dAmplitude);
		AudioPlay.Write(pData, uiAvailableSpaceForFrames);
    }

    // Sleep for a second
    Sleep(1000);

	// Stop playing
    AudioPlay.Close();

	// Free
	BIGFREE(pData);

	return TRUE;
}

int CAudioResampler::Resample(float* pSrc, int nSrcSize, int nSrcSampleRate, int nDstSampleRate, int nChannels)
{
	// Check
	float dRatio = (float)nDstSampleRate / (float)nSrcSampleRate;
	if (!pSrc || nChannels < 1)
		return 0;

	// Downsample?
	if (dRatio < 1.0f)
	{
		// Setup low-pass filter output buffer
		int nSrcSizeBytes = nSrcSize * sizeof(float) * nChannels;
		if (nSrcSizeBytes > m_nInbufSizeBytes)
		{
			m_nInbufSizeBytes = nSrcSizeBytes;
			if (m_pInbuf)
				BIGFREE(m_pInbuf);
			m_pInbuf = (float*)BIGALLOC(m_nInbufSizeBytes);
			if (!m_pInbuf)
			{
				m_nInbufSizeBytes = 0;
				return 0;
			}
		}

		// Low-pass
		LowPass(pSrc, m_pInbuf, nSrcSize, nSrcSampleRate, nDstSampleRate, nChannels);

		// New source
		pSrc = m_pInbuf;
	}

	// Setup output buffer
	int nDstSize = (int)(dRatio * nSrcSize); // could also use RoundF()
	int nDstSizeBytes = nDstSize * sizeof(float) * nChannels;
	if (nDstSizeBytes > m_nOutbufSizeBytes)
	{
		m_nOutbufSizeBytes = nDstSizeBytes;
		if (m_pOutbuf)
			BIGFREE(m_pOutbuf);
		m_pOutbuf = (float*)BIGALLOC(m_nOutbufSizeBytes);
		if (!m_pOutbuf)
		{
			m_nOutbufSizeBytes = 0;
			return 0;
		}
	}

	// Linear interpolation
	for (int j = 0 ; j < nDstSize ; j++)
	{
		// From output buffer index to input buffer index
		float di = (float)j / dRatio; 
		int i0 = (int)di;
		int i1 = i0 + 1;

		// Loop through all channels
		for (int iChannel = 0 ; iChannel < nChannels ; iChannel++)
		{
			// Check
			if (i1 >= nSrcSize)
				m_pOutbuf[nChannels*j + iChannel] = pSrc[nChannels*(nSrcSize - 1) + iChannel];
			else
			{
				// Slope
				// Note: not necessary to divide by (i1 - i0) because that value is 1
				float dA = pSrc[nChannels*i1 + iChannel] - pSrc[nChannels*i0 + iChannel];

				// Y Intercept
				// Note: calculated from B = y - AX
				float dB = pSrc[nChannels*i0 + iChannel] - (dA * i0);

				// Calculate output value from the linear function y = Ax + B
				m_pOutbuf[nChannels*j + iChannel] = dA * di + dB;
			}
		}
	}

	return nDstSize;
}

// https://en.wikipedia.org/wiki/Low-pass_filter#Simple_infinite_impulse_response_filter
void CAudioResampler::LowPass(float* pSrc, float* pDst, int nSize, int nSrcSampleRate, int nDstSampleRate, int nChannels)
{
	float T = 1.0f / nSrcSampleRate;											// sampling period
	float fcutoff = nDstSampleRate / 4.0f;										// cutoff frequency
	float a = (2.0f * PI_F * T * fcutoff) / (2.0f * PI_F * T * fcutoff + 1.0f); // 0 < a < 1

	for (int iChannel = 0 ; iChannel < nChannels ; iChannel++)
		pDst[iChannel] = pSrc[iChannel];
	for (int i = 1 ; i < nSize ; i++)
	{
		// pDst[i] = a*pSrc[i] + (1-a)*pDst[i-1] = pDst[i-1] + a*(pSrc[i] - pDst[i-1])
		for (int iChannel = 0 ; iChannel < nChannels ; iChannel++)
			 pDst[nChannels*i + iChannel] = pDst[nChannels*(i-1) + iChannel] + a*(pSrc[nChannels*i + iChannel] - pDst[nChannels*(i-1) + iChannel]);
	}
}

void CAudioResampler::Free()
{
	if (m_pInbuf)
	{
		BIGFREE(m_pInbuf);
		m_pInbuf = NULL;
	}
	m_nInbufSizeBytes = 0;
	if (m_pOutbuf)
	{
		BIGFREE(m_pOutbuf);
		m_pOutbuf = NULL;
	}
	m_nOutbufSizeBytes = 0;
}

BOOL CAudioPlay::Init(REFERENCE_TIME hnsRequestedBufferDuration/*=10000000*/)
{
	// First close
	Close();

	// Get interfaces
    HRESULT hr = ::CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&m_pEnumerator);
	if (FAILED(hr))
		return FALSE;
    hr = m_pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_pDevice);
	if (FAILED(hr))
		return FALSE;
    hr = m_pDevice->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, (void**)&m_pAudioClient);
	if (FAILED(hr))
		return FALSE;

	// Share mode format is always floating-point
    hr = m_pAudioClient->GetMixFormat(&m_pWaveFormat);
	if (FAILED(hr))
		return FALSE;

	// In shared mode, the audio engine always supports the mix format,
	// which the client can obtain by calling the IAudioClient::GetMixFormat
	// method. In addition, the audio engine might support similar formats
	// that have the same sample rate and number of channels as the mix format
	// but differ in the representation of audio sample values. The audio engine
	// represents sample values internally as floating-point numbers, but if the
	// caller-specified format represents sample values as integers, the audio
	// engine typically can convert between the integer sample values and its
	// internal floating-point representation
	//
	// To test PCM formats uncomment the following lines (changing sample rate is not working)
	//m_pWaveFormat->wFormatTag = WAVE_FORMAT_PCM;
	//m_pWaveFormat->wBitsPerSample = 16;	// 16 or 8
	//m_pWaveFormat->nChannels = 2;			// 2 or 1 
	//m_pWaveFormat->nBlockAlign = m_pWaveFormat->nChannels * m_pWaveFormat->wBitsPerSample / 8;
	//m_pWaveFormat->nAvgBytesPerSec = m_pWaveFormat->nSamplesPerSec * m_pWaveFormat->nBlockAlign;
	//m_pWaveFormat->cbSize = 0;
    hr = m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,		// share the audio with other clients (no exclusive access to device)
									AUDCLNT_STREAMFLAGS_NOPERSIST,	// this flag disables persistence of the volume and mute settings						
									hnsRequestedBufferDuration,		// the buffer capacity as a 100-nanosecond units time value
									0,								// in shared mode always set hnsPeriodicity to 0
									m_pWaveFormat,					// wanted audio format
									NULL);							// no audio session GUID
	if (FAILED(hr))
		return FALSE;

    // Get the actual size of the allocated buffer
    hr = m_pAudioClient->GetBufferSize(&m_uiBufferFrameCount);
	if (FAILED(hr))
		return FALSE;

	// Calculate the actual duration of the allocated buffer
    m_hnsBufferDuration = hnsRequestedBufferDuration * m_uiBufferFrameCount / m_pWaveFormat->nSamplesPerSec;

	// Get renderer
    hr = m_pAudioClient->GetService(__uuidof(IAudioRenderClient), (void**)&m_pRenderClient);
	return SUCCEEDED(hr);
}

void CAudioPlay::Close()
{
	if (m_pAudioClient)
		m_pAudioClient->Stop();
	if (m_pWaveFormat)
	{
		::CoTaskMemFree(m_pWaveFormat);
		m_pWaveFormat = NULL;
	}
	if (m_pEnumerator)
	{
		m_pEnumerator->Release();
		m_pEnumerator = NULL;
	}
    if (m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = NULL;
	}
    if (m_pAudioClient)
	{
		m_pAudioClient->Release();
		m_pAudioClient = NULL;
	}
    if (m_pRenderClient)
	{
		m_pRenderClient->Release();
		m_pRenderClient = NULL;
	}
	m_bStarted = FALSE;
	m_uiBufferFrameCount = 0;
	m_hnsBufferDuration = 0;
}

BOOL CAudioPlay::Write(LPBYTE pData, UINT32 uiNumFrames)
{
	// Check
	if (!m_pAudioClient || !m_pRenderClient || !m_pWaveFormat)
		return FALSE;
	if (!pData || uiNumFrames == 0)
		return TRUE;

	// Calc. the number of frames that can be written
    UINT32 uiNumFramesPadding;
	HRESULT hr = m_pAudioClient->GetCurrentPadding(&uiNumFramesPadding); // amount of valid, unread data that the endpoint buffer contains
	if (FAILED(hr))
		return FALSE;
	UINT32 uiAvailableSpaceForFrames = m_uiBufferFrameCount - uiNumFramesPadding;
	UINT32 uiNumFramesToWrite = MIN(uiAvailableSpaceForFrames, uiNumFrames);

	// Write
	LPBYTE pBuf;
	hr = m_pRenderClient->GetBuffer(uiNumFramesToWrite, &pBuf);
	if (FAILED(hr))
		return FALSE;
	memcpy(pBuf, pData, m_pWaveFormat->nBlockAlign * uiNumFramesToWrite);
	hr = m_pRenderClient->ReleaseBuffer(uiNumFramesToWrite, 0);
	if (FAILED(hr))
		return FALSE;

	// Start?
	if (!m_bStarted && ((uiNumFramesPadding + uiNumFramesToWrite) >= m_uiBufferFrameCount / 2))
	{
		m_pAudioClient->Start();
		m_bStarted = TRUE;
	}

	return TRUE;
}