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

BOOL PlaySinus(int nDurationSec, double dFrequency/*=440.0*/, double dAmplitude/*=0.3*/)
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
	dPhase = CAudioTools::GenerateSinus(pData, uiAvailableSpaceForFrames, AudioPlay.GetWaveFormat(), dFrequency, dPhase, dAmplitude);
	AudioPlay.Write(pData, uiAvailableSpaceForFrames, FALSE);
    
	// Loop
	while (nDurationSec > 0)
    {
        // Sleep for half second
        Sleep(500);
		nDurationSec--;

        // Get next portion of sinus data
		uiAvailableSpaceForFrames = AudioPlay.GetAvailableSpaceForFrames();
		dPhase = CAudioTools::GenerateSinus(pData, uiAvailableSpaceForFrames, AudioPlay.GetWaveFormat(), dFrequency, dPhase, dAmplitude);
		AudioPlay.Write(pData, uiAvailableSpaceForFrames, FALSE);
    }

    // Sleep for a second
    Sleep(1000);

	// Stop playing
    AudioPlay.Close();

	// Free
	BIGFREE(pData);

	return TRUE;
}

WORD CAudioTools::GetWaveFormatTag(const LPWAVEFORMATEX pWaveFormat)
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

double CAudioTools::GenerateSinus(	LPBYTE pData, UINT32 uiNumFrames, const LPWAVEFORMATEX pWaveFormat,
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

float* CAudioTools::ToFloat(LPBYTE pSrc, int nNumFrames, const LPWAVEFORMATEX pSrcWaveFormat)
{
	// Check
	if (!pSrc || nNumFrames <= 0 || !pSrcWaveFormat)
		return NULL;

	// If already float return pSrc
	if (GetWaveFormatTag(pSrcWaveFormat) == WAVE_FORMAT_IEEE_FLOAT)
		return (float*)pSrc;

	// Setup format changed buffer
	int nDstSizeBytes = nNumFrames * sizeof(float) * pSrcWaveFormat->nChannels;
	if (nDstSizeBytes > m_nToFloatBufSizeBytes)
	{
		m_nToFloatBufSizeBytes = nDstSizeBytes;
		if (m_pToFloatBuf)
			BIGFREE(m_pToFloatBuf);
		m_pToFloatBuf = (float*)BIGALLOC(m_nToFloatBufSizeBytes);
		if (!m_pToFloatBuf)
		{
			m_nToFloatBufSizeBytes = 0;
			return NULL;
		}
	}
	
	// Convert
	if (pSrcWaveFormat->wBitsPerSample == 16)
	{
		// Positive peaks can go up to +32767, negative peaks to -32768 and silence are 0
		short* pData16 = (short*)pSrc;
		for (int i = 0 ; i < nNumFrames ; i++)
		{
			for (int iChannel = 0 ; iChannel < pSrcWaveFormat->nChannels ; iChannel++)
				 m_pToFloatBuf[pSrcWaveFormat->nChannels*i + iChannel] = pData16[pSrcWaveFormat->nChannels*i + iChannel] / 32768.0f;
		}
		return m_pToFloatBuf;
	}
	else if (pSrcWaveFormat->wBitsPerSample == 8)
	{
		// 8-bit data is in offset binary where positive peaks are 255, negative peaks are 0 and 128 is silence
		// https://en.wikipedia.org/wiki/Offset_binary
		unsigned char* pData8 = (unsigned char*)pSrc;
		for (int i = 0 ; i < nNumFrames ; i++)
		{
			for (int iChannel = 0; iChannel < pSrcWaveFormat->nChannels ; iChannel++)
			{
				int sample = (int)pData8[pSrcWaveFormat->nChannels*i + iChannel] - 128;
				m_pToFloatBuf[pSrcWaveFormat->nChannels*i + iChannel] = sample / 128.0f;
			}
		}
		return m_pToFloatBuf;
	}
	else
		return NULL;
}

float* CAudioTools::ConvertChannels(float* pSrc, int nNumFrames, int nSrcChannels, int nDstChannels)
{
	// Check
	if (!pSrc || nNumFrames <= 0 || nSrcChannels < 1 || nDstChannels < 1)
		return NULL;

	// If the same channels count return pSrc
	if (nSrcChannels == nDstChannels)
		return pSrc;

	// Setup channels converted buffer
	int nDstSizeBytes = nNumFrames * sizeof(float) * nDstChannels;
	if (nDstSizeBytes > m_nConvertChannelsBufSizeBytes)
	{
		m_nConvertChannelsBufSizeBytes = nDstSizeBytes;
		if (m_pConvertChannelsBuf)
			BIGFREE(m_pConvertChannelsBuf);
		m_pConvertChannelsBuf = (float*)BIGALLOC(m_nConvertChannelsBufSizeBytes);
		if (!m_pConvertChannelsBuf)
		{
			m_nConvertChannelsBufSizeBytes = 0;
			return NULL;
		}
	}
	
	// Mix source channels to mono destination channel
	if (nDstChannels == 1)
	{
		for (int i = 0 ; i < nNumFrames ; i++)
		{
			float fMixedValue = 0.0f;
			for (int iSrcChannel = 0 ; iSrcChannel < nSrcChannels ; iSrcChannel++)
				fMixedValue += pSrc[nSrcChannels*i + iSrcChannel];
			fMixedValue /= nSrcChannels;
			m_pConvertChannelsBuf[i] = fMixedValue;
		}
	}
	// Copy source channels to destination channels
	else
	{
		for (int i = 0 ; i < nNumFrames ; i++)
		{
			int iSrcChannel = 0;
			for (int iDstChannel = 0 ; iDstChannel < nDstChannels ; iDstChannel++)
			{
				m_pConvertChannelsBuf[nDstChannels*i + iDstChannel] = pSrc[nSrcChannels*i + iSrcChannel];
				if (iSrcChannel < (nSrcChannels - 1))
					iSrcChannel++;
			}
		}
	}

	return m_pConvertChannelsBuf;
}

float* CAudioTools::Resample(float* pSrc, int nNumSrcFrames, int* pNumDstFrames, int nSrcSampleRate, int nDstSampleRate, int nChannels)
{
	// Check
	if (!pSrc || nNumSrcFrames <= 0 || nChannels < 1)
	{
		if (pNumDstFrames)
			*pNumDstFrames = 0;
		return NULL;
	}

	// If the same sample rate return pSrc
	if (nSrcSampleRate == nDstSampleRate)
	{
		if (pNumDstFrames)
			*pNumDstFrames = nNumSrcFrames;
		return pSrc;
	}

	// If downsampling first lowpass the signal
	float dRatio = (float)nDstSampleRate / (float)nSrcSampleRate;
	if (dRatio < 1.0f)
		pSrc = LowPass(pSrc, nNumSrcFrames, nSrcSampleRate, nDstSampleRate, nChannels);

	// Setup output buffer
	int nCalcDstFrames = (int)(dRatio * nNumSrcFrames); // could also use RoundF()
	int nDstSizeBytes = nCalcDstFrames * sizeof(float) * nChannels;
	if (nDstSizeBytes > m_nResampleBufSizeBytes)
	{
		m_nResampleBufSizeBytes = nDstSizeBytes;
		if (m_pResampleBuf)
			BIGFREE(m_pResampleBuf);
		m_pResampleBuf = (float*)BIGALLOC(m_nResampleBufSizeBytes);
		if (!m_pResampleBuf)
		{
			m_nResampleBufSizeBytes = 0;
			if (pNumDstFrames)
				*pNumDstFrames = 0;
			return NULL;
		}
	}

	// Linear interpolation
	for (int j = 0 ; j < nCalcDstFrames ; j++)
	{
		// From output buffer index to input buffer index
		float di = (float)j / dRatio; 
		int i0 = (int)di;
		int i1 = i0 + 1;

		// Loop through all channels
		for (int iChannel = 0 ; iChannel < nChannels ; iChannel++)
		{
			// Check
			if (i1 >= nNumSrcFrames)
				m_pResampleBuf[nChannels*j + iChannel] = pSrc[nChannels*(nNumSrcFrames - 1) + iChannel];
			else
			{
				// Slope
				// Note: not necessary to divide by (i1 - i0) because that value is 1
				float dA = pSrc[nChannels*i1 + iChannel] - pSrc[nChannels*i0 + iChannel];

				// Y Intercept
				// Note: calculated from B = y - AX
				float dB = pSrc[nChannels*i0 + iChannel] - (dA * i0);

				// Calculate output value from the linear function y = Ax + B
				m_pResampleBuf[nChannels*j + iChannel] = dA * di + dB;
			}
		}
	}

	if (pNumDstFrames)
		*pNumDstFrames = nCalcDstFrames;
	return m_pResampleBuf;
}

// https://en.wikipedia.org/wiki/Low-pass_filter#Simple_infinite_impulse_response_filter
float* CAudioTools::LowPass(float* pSrc, int nNumFrames, int nSrcSampleRate, int nDstSampleRate, int nChannels)
{
	// Check
	if (!pSrc || nNumFrames <= 0 || nChannels < 1)
		return NULL;

	// Setup low-pass filter output buffer
	int nSrcSizeBytes = nNumFrames * sizeof(float) * nChannels;
	if (nSrcSizeBytes > m_nLowpassBufSizeBytes)
	{
		m_nLowpassBufSizeBytes = nSrcSizeBytes;
		if (m_pLowpassBuf)
			BIGFREE(m_pLowpassBuf);
		m_pLowpassBuf = (float*)BIGALLOC(m_nLowpassBufSizeBytes);
		if (!m_pLowpassBuf)
		{
			m_nLowpassBufSizeBytes = 0;
			return NULL;
		}
	}

	// Lowpass
	float T = 1.0f / nSrcSampleRate;											// sampling period
	float fcutoff = nDstSampleRate / 4.0f;										// cutoff frequency
	float a = (2.0f * PI_F * T * fcutoff) / (2.0f * PI_F * T * fcutoff + 1.0f); // 0 < a < 1
	for (int iChannel = 0 ; iChannel < nChannels ; iChannel++)
		m_pLowpassBuf[iChannel] = pSrc[iChannel];
	for (int i = 1 ; i < nNumFrames ; i++)
	{
		// pDst[i] = a*pSrc[i] + (1-a)*pDst[i-1] = pDst[i-1] + a*(pSrc[i] - pDst[i-1])
		for (int iChannel = 0 ; iChannel < nChannels ; iChannel++)
			 m_pLowpassBuf[nChannels*i + iChannel] = m_pLowpassBuf[nChannels*(i-1) + iChannel] + a*(pSrc[nChannels*i + iChannel] - m_pLowpassBuf[nChannels*(i-1) + iChannel]);
	}

	return m_pLowpassBuf;
}

void CAudioTools::Free()
{
	if (m_pLowpassBuf)
	{
		BIGFREE(m_pLowpassBuf);
		m_pLowpassBuf = NULL;
	}
	m_nLowpassBufSizeBytes = 0;
	if (m_pResampleBuf)
	{
		BIGFREE(m_pResampleBuf);
		m_pResampleBuf = NULL;
	}
	m_nResampleBufSizeBytes = 0;
	if (m_pToFloatBuf)
	{
		BIGFREE(m_pToFloatBuf);
		m_pToFloatBuf = NULL;
	}
	m_nToFloatBufSizeBytes = 0;
	if (m_pConvertChannelsBuf)
	{
		BIGFREE(m_pConvertChannelsBuf);
		m_pConvertChannelsBuf = NULL;
	}
	m_nConvertChannelsBufSizeBytes = 0;
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
									AUDCLNT_STREAMFLAGS_NOPERSIST,	// this flag disables persistence of the volume and mute settings for the session, see ISimpleAudioVolume						
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

	// Get renderer
    hr = m_pAudioClient->GetService(__uuidof(IAudioRenderClient), (void**)&m_pRenderClient);
	if (SUCCEEDED(hr))
	{
		m_bInit = TRUE;
		return TRUE;
	}
	else
		return FALSE;
}

void CAudioPlay::Close()
{
	// Stop playing
	if (m_pAudioClient)
		m_pAudioClient->Stop();

	// Reset vars
	m_bInit = FALSE;
	m_bStarted = FALSE;
	m_dFramesCorrection = 0.0;
	m_uiBufferFrameCount = 0;

	// Free
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
}

BOOL CAudioPlay::Write(LPBYTE pData, UINT32 uiNumFrames, BOOL bDoFramesOffsetCorrection)
{
	// Check
	if (!m_pAudioClient || !m_pRenderClient || !m_pWaveFormat)
		return FALSE;

	// Return if nothing to buffer
	if (!pData || uiNumFrames == 0)
		return TRUE;

	// Adjust the buffer size depending from the frames correction
	int nNumFramesGetBuf = uiNumFrames;
	if (bDoFramesOffsetCorrection)
	{
		int nFramesOffset = (int)(m_dFramesCorrection * uiNumFrames);
		double dFramesOffsetRel = (double)nFramesOffset / (double)uiNumFrames;
		if (abs(dFramesOffsetRel) > FRAMES_CORRECTION_MAX_REL_OFFSET)
			nNumFramesGetBuf += nFramesOffset;
		else
		{
			if (nFramesOffset > 0)
				nNumFramesGetBuf += 1;
			else if (nFramesOffset < 0)
				nNumFramesGetBuf -= 1;
		}
		//TRACE(_T("Frames Offset %d\n"), nNumFramesGetBuf - (int)uiNumFrames);
	}

	// Clip
	if (nNumFramesGetBuf <= 0)
		return TRUE;
    UINT32 uiNumFramesPadding;
	HRESULT hr = m_pAudioClient->GetCurrentPadding(&uiNumFramesPadding); // amount of valid, unread data that the endpoint buffer contains
	if (FAILED(hr))
		return FALSE;
	int nAvailableSpaceForFrames = (int)m_uiBufferFrameCount - (int)uiNumFramesPadding;
	if (nNumFramesGetBuf > nAvailableSpaceForFrames)
		nNumFramesGetBuf = nAvailableSpaceForFrames;

	// Write
	LPBYTE pBuf;
	hr = m_pRenderClient->GetBuffer((UINT32)nNumFramesGetBuf, &pBuf);
	if (FAILED(hr))
		return FALSE;
	if (nNumFramesGetBuf > (int)uiNumFrames)
	{
		memcpy(pBuf, pData, m_pWaveFormat->nBlockAlign * uiNumFrames);
		pData += m_pWaveFormat->nBlockAlign * (uiNumFrames - 1);	// point to last frame
		pBuf += m_pWaveFormat->nBlockAlign * uiNumFrames;			// point to after last frame
		int nRepeat = nNumFramesGetBuf - (int)uiNumFrames;
		while (nRepeat-- > 0)
		{
			memcpy(pBuf, pData, m_pWaveFormat->nBlockAlign);
			pBuf += m_pWaveFormat->nBlockAlign;
		}
	}
	else
		memcpy(pBuf, pData, m_pWaveFormat->nBlockAlign * nNumFramesGetBuf);
	hr = m_pRenderClient->ReleaseBuffer((UINT32)nNumFramesGetBuf, 0);
	if (FAILED(hr))
		return FALSE;

	// Update Frames Padding after buffer write
	// Note: it's not necessary to call GetCurrentPadding() again as we know what we wrote
	uiNumFramesPadding += nNumFramesGetBuf;

	// Started?
	if (m_bStarted)
	{
		DWORD dwCurrentUpTime = ::GetTickCount();
		DWORD dwDiffMs = dwCurrentUpTime - m_dwLastCheckUpTime;
		if (dwDiffMs >= FRAMES_CORRECTION_CHECK_INTERVAL_MS)
		{
			int nDelta = (int)(m_uiBufferFrameCount / 2) - (int)uiNumFramesPadding;
			int nTotalElapsedFrames = (int)(m_pWaveFormat->nSamplesPerSec * dwDiffMs / 1000U);
			m_dFramesCorrection = (double)nDelta / (double)nTotalElapsedFrames;
			//TRACE(_T("Audio Prelisten Buffer Usage %u/%u (%0.2f%%)\n"), uiNumFramesPadding, m_uiBufferFrameCount, 100.0 * uiNumFramesPadding / m_uiBufferFrameCount);
			m_dwLastCheckUpTime = dwCurrentUpTime;
		}
	}
	else if (uiNumFramesPadding >= m_uiBufferFrameCount / 2)
	{
		m_pAudioClient->Start();
		m_bStarted = TRUE;
		m_dwLastCheckUpTime = ::GetTickCount();
	}

	return TRUE;
}