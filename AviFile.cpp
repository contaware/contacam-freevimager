#include "stdafx.h"
#include <windowsx.h>
#include <memory.h>
#include "uImager.h"
#include <vfw.h>
#include "AviFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib, "vfw32.lib")
#pragma comment(lib, "msacm32.lib")

// Constructor To Get Compressor Informations
CAVIFile::CAVIFile()
{
	InitWaveFormatTagTable();

	m_bShowMessageBoxOnError = true;
	m_pParentWnd = NULL;
	m_bCreateFile = false;
	m_bTruncateFile = false;
	m_sFileName = _T("");
	m_bOk = true;
	m_bInitRead = false;
	m_pFile = NULL;
	m_uiFileMode = 0;
	m_hKillEvent = NULL;
	m_dwTotalVideoStreams = 0;
	m_dwTotalAudioStreams = 0;
	DWORD dwStreamNum;

	for (dwStreamNum = 0 ; dwStreamNum < MAX_VIDEO_STREAMS ; dwStreamNum++)
	{
		m_dwFourCC[dwStreamNum] = 0;
		m_dwRate[dwStreamNum]  = 1;
		m_dwScale[dwStreamNum] = 1;
		m_dwStartVideoOffset[dwStreamNum] = 0;
		m_dwReadNextFrame[dwStreamNum] = 0;
		m_dwWriteNextFrame[dwStreamNum] = 0;
		m_dwTotalWrittenVideoBytes[dwStreamNum] = 0;
		m_dwTotalFrames[dwStreamNum] = 0;
		m_pVideoStream[dwStreamNum] = NULL;
		m_pVideoStreamCompressed[dwStreamNum] = NULL;
		m_pVideoGetFrame[dwStreamNum] = NULL;
		m_dwHeight[dwStreamNum] = 0;
		m_dwWidth[dwStreamNum] = 0;
		memset(&m_VideoCompressorOptions[dwStreamNum], 0, sizeof(AVICOMPRESSOPTIONSVOLATILE));
		m_hCompressionIC[dwStreamNum] = NULL;
		m_pCompressedFormat[dwStreamNum] = NULL;
		m_dwCompressedFormatSize[dwStreamNum] = 0;
		m_pCompressedBuf[dwStreamNum] = NULL;
		m_pPrevUncompressedBuf[dwStreamNum] = NULL;
		m_nKeyRateCounter[dwStreamNum] = 1;
	}

	for (dwStreamNum = 0 ; dwStreamNum < MAX_AUDIO_STREAMS ; dwStreamNum++)
	{
		m_pAudioStream[dwStreamNum] = NULL;
		m_pWaveFormat[dwStreamNum] = (WAVEFORMATEX*)new BYTE[sizeof(WAVEFORMATEX)];
		memset(m_pWaveFormat[dwStreamNum], 0, sizeof(WAVEFORMATEX));
		m_dwReadNextSample[dwStreamNum] = 0;
		m_dwWriteNextSample[dwStreamNum] = 0;
		m_dwTotalWrittenAudioBytes[dwStreamNum] = 0;
		m_dwTotalSamples[dwStreamNum] = 0;
		m_dwAudioSkew[dwStreamNum] = 0;
		m_dwSuggestedAudioBufferSize[dwStreamNum] = 0;
		m_dwStartAudioOffset[dwStreamNum] = 0;
	}
	
	::InitializeCriticalSection(&m_csAVI);

	WORD wVer = HIWORD(VideoForWindowsVersion());
	if (wVer < 0x010A)
	{
		// oops, we are too old, blow out of here
		m_bOk = false;
	}
	else
	{
		::AVIFileInit();
	}
}

// Constructor To Get Compressor Informations
CAVIFile::CAVIFile(CWnd* pParentWnd)
 :  m_pParentWnd(pParentWnd)
{
	InitWaveFormatTagTable();

	m_bShowMessageBoxOnError = true;
	m_bCreateFile = false;
	m_bTruncateFile = false;
	m_sFileName = _T("");
	m_bOk = true;
	m_bInitRead = false;
	m_pFile = NULL;
	m_uiFileMode = 0;
	m_hKillEvent = NULL;
	m_dwTotalVideoStreams = 0;
	m_dwTotalAudioStreams = 0;
	DWORD dwStreamNum;

	for (dwStreamNum = 0 ; dwStreamNum < MAX_VIDEO_STREAMS ; dwStreamNum++)
	{
		m_dwFourCC[dwStreamNum] = 0;
		m_dwRate[dwStreamNum]  = 1;
		m_dwScale[dwStreamNum] = 1;
		m_dwStartVideoOffset[dwStreamNum] = 0;
		m_dwReadNextFrame[dwStreamNum] = 0;
		m_dwWriteNextFrame[dwStreamNum] = 0;
		m_dwTotalWrittenVideoBytes[dwStreamNum] = 0;
		m_dwTotalFrames[dwStreamNum] = 0;
		m_pVideoStream[dwStreamNum] = NULL;
		m_pVideoStreamCompressed[dwStreamNum] = NULL;
		m_pVideoGetFrame[dwStreamNum] = NULL;
		m_dwHeight[dwStreamNum] = 0;
		m_dwWidth[dwStreamNum] = 0;
		memset(&m_VideoCompressorOptions[dwStreamNum], 0, sizeof(AVICOMPRESSOPTIONSVOLATILE));
		m_hCompressionIC[dwStreamNum] = NULL;
		m_pCompressedFormat[dwStreamNum] = NULL;
		m_dwCompressedFormatSize[dwStreamNum] = 0;
		m_pCompressedBuf[dwStreamNum] = NULL;
		m_pPrevUncompressedBuf[dwStreamNum] = NULL;
		m_nKeyRateCounter[dwStreamNum] = 1;
	}

	for (dwStreamNum = 0 ; dwStreamNum < MAX_AUDIO_STREAMS ; dwStreamNum++)
	{
		m_pAudioStream[dwStreamNum] = NULL;
		m_pWaveFormat[dwStreamNum] = (WAVEFORMATEX*)new BYTE[sizeof(WAVEFORMATEX)];
		memset(m_pWaveFormat[dwStreamNum], 0, sizeof(WAVEFORMATEX));
		m_dwReadNextSample[dwStreamNum] = 0;
		m_dwWriteNextSample[dwStreamNum] = 0;
		m_dwTotalWrittenAudioBytes[dwStreamNum] = 0;
		m_dwTotalSamples[dwStreamNum] = 0;
		m_dwAudioSkew[dwStreamNum] = 0;
		m_dwSuggestedAudioBufferSize[dwStreamNum] = 0;
		m_dwStartAudioOffset[dwStreamNum] = 0;
	}
	
	::InitializeCriticalSection(&m_csAVI);

	WORD wVer = HIWORD(VideoForWindowsVersion());
	if (wVer < 0x010A)
	{
		// oops, we are too old, blow out of here
		m_bOk = false;
	}
	else
	{
		::AVIFileInit();
	}
}

// Constructor To Get Compressor Informations
CAVIFile::CAVIFile(	CWnd* pParentWnd,
					bool bShowMessageBoxOnError)
 :  m_pParentWnd(pParentWnd),
	m_bShowMessageBoxOnError(bShowMessageBoxOnError)
{
	InitWaveFormatTagTable();

	m_bCreateFile = false;
	m_bTruncateFile = false;
	m_sFileName = _T("");
	m_bOk = true;
	m_bInitRead = false;
	m_pFile = NULL;
	m_uiFileMode = 0;
	m_hKillEvent = NULL;
	m_dwTotalVideoStreams = 0;
	m_dwTotalAudioStreams = 0;
	DWORD dwStreamNum;

	for (dwStreamNum = 0 ; dwStreamNum < MAX_VIDEO_STREAMS ; dwStreamNum++)
	{
		m_dwFourCC[dwStreamNum] = 0;
		m_dwRate[dwStreamNum]  = 1;
		m_dwScale[dwStreamNum] = 1;
		m_dwStartVideoOffset[dwStreamNum] = 0;
		m_dwReadNextFrame[dwStreamNum] = 0;
		m_dwWriteNextFrame[dwStreamNum] = 0;
		m_dwTotalWrittenVideoBytes[dwStreamNum] = 0;
		m_dwTotalFrames[dwStreamNum] = 0;
		m_pVideoStream[dwStreamNum] = NULL;
		m_pVideoStreamCompressed[dwStreamNum] = NULL;
		m_pVideoGetFrame[dwStreamNum] = NULL;
		m_dwHeight[dwStreamNum] = 0;
		m_dwWidth[dwStreamNum] = 0;
		memset(&m_VideoCompressorOptions[dwStreamNum], 0, sizeof(AVICOMPRESSOPTIONSVOLATILE));
		m_hCompressionIC[dwStreamNum] = NULL;
		m_pCompressedFormat[dwStreamNum] = NULL;
		m_dwCompressedFormatSize[dwStreamNum] = 0;
		m_pCompressedBuf[dwStreamNum] = NULL;
		m_pPrevUncompressedBuf[dwStreamNum] = NULL;
		m_nKeyRateCounter[dwStreamNum] = 1;
	}

	for (dwStreamNum = 0 ; dwStreamNum < MAX_AUDIO_STREAMS ; dwStreamNum++)
	{
		m_pAudioStream[dwStreamNum] = NULL;
		m_pWaveFormat[dwStreamNum] = (WAVEFORMATEX*)new BYTE[sizeof(WAVEFORMATEX)];
		memset(m_pWaveFormat[dwStreamNum], 0, sizeof(WAVEFORMATEX));
		m_dwReadNextSample[dwStreamNum] = 0;
		m_dwWriteNextSample[dwStreamNum] = 0;
		m_dwTotalWrittenAudioBytes[dwStreamNum] = 0;
		m_dwTotalSamples[dwStreamNum] = 0;
		m_dwAudioSkew[dwStreamNum] = 0;
		m_dwSuggestedAudioBufferSize[dwStreamNum] = 0;
		m_dwStartAudioOffset[dwStreamNum] = 0;
	}
	
	::InitializeCriticalSection(&m_csAVI);

	WORD wVer = HIWORD(VideoForWindowsVersion());
	if (wVer < 0x010A)
	{
		// oops, we are too old, blow out of here
		m_bOk = false;
	}
	else
	{
		::AVIFileInit();
	}
}

// Constructor To Write An Avi File
CAVIFile::CAVIFile(CWnd* pParentWnd,
				   LPCTSTR lpszFileName,
				   DWORD dwRate,
				   DWORD dwScale,
				   const LPWAVEFORMATEX pWaveFormat,
				   bool bCreateFile,
				   bool bTruncateFile,
				   bool bShowMessageBoxOnError)
 :	m_sFileName(lpszFileName),
	m_pParentWnd(pParentWnd),
	m_bCreateFile(bCreateFile),
	m_bTruncateFile(bTruncateFile),
	m_bShowMessageBoxOnError(bShowMessageBoxOnError)
{
	InitWaveFormatTagTable();

	m_bOk = true;
	m_bInitRead = false;
	m_pFile = NULL;
	m_uiFileMode = 0;
	m_hKillEvent = NULL;
	m_dwTotalVideoStreams = 0;
	m_dwTotalAudioStreams = 0;
	DWORD dwStreamNum;

	for (dwStreamNum = 0 ; dwStreamNum < MAX_VIDEO_STREAMS ; dwStreamNum++)
	{
		m_dwRate[dwStreamNum] = dwRate;
		m_dwScale[dwStreamNum] = dwScale;
		m_dwStartVideoOffset[dwStreamNum] = 0;
		m_dwFourCC[dwStreamNum] = 0;
		m_dwReadNextFrame[dwStreamNum] = 0;
		m_dwWriteNextFrame[dwStreamNum] = 0;
		m_dwTotalWrittenVideoBytes[dwStreamNum] = 0;
		m_dwTotalFrames[dwStreamNum] = 0;
		m_pVideoStream[dwStreamNum] = NULL;
		m_pVideoStreamCompressed[dwStreamNum] = NULL;
		m_pVideoGetFrame[dwStreamNum] = NULL;
		m_dwHeight[dwStreamNum] = 0;
		m_dwWidth[dwStreamNum] = 0;
		memset(&m_VideoCompressorOptions[dwStreamNum], 0, sizeof(AVICOMPRESSOPTIONSVOLATILE));
		m_hCompressionIC[dwStreamNum] = NULL;
		m_pCompressedFormat[dwStreamNum] = NULL;
		m_dwCompressedFormatSize[dwStreamNum] = 0;
		m_pCompressedBuf[dwStreamNum] = NULL;
		m_pPrevUncompressedBuf[dwStreamNum] = NULL;
		m_nKeyRateCounter[dwStreamNum] = 1;
	}

	for (dwStreamNum = 0 ; dwStreamNum < MAX_AUDIO_STREAMS ; dwStreamNum++)
	{
		m_pAudioStream[dwStreamNum] = NULL;
		if (pWaveFormat)
		{
			m_pWaveFormat[dwStreamNum] = NULL;
			SetWaveFormat(dwStreamNum, pWaveFormat); // Allocates memory!
		}
		else
		{
			m_pWaveFormat[dwStreamNum] = (WAVEFORMATEX*)new BYTE[sizeof(WAVEFORMATEX)];
			memset(m_pWaveFormat[dwStreamNum], 0, sizeof(WAVEFORMATEX));
		}
		m_dwReadNextSample[dwStreamNum] = 0;
		m_dwWriteNextSample[dwStreamNum] = 0;
		m_dwTotalWrittenAudioBytes[dwStreamNum] = 0;
		m_dwTotalSamples[dwStreamNum] = 0;
		m_dwAudioSkew[dwStreamNum] = 0;
		m_dwSuggestedAudioBufferSize[dwStreamNum] = 0;
		m_dwStartAudioOffset[dwStreamNum] = 0;
	}

	::InitializeCriticalSection(&m_csAVI);

	WORD wVer = HIWORD(VideoForWindowsVersion());
	if (wVer < 0x010A)
	{
		// oops, we are too old, blow out of here
		m_bOk = false;
	}
	else
	{
		::AVIFileInit();
		if (!bCreateFile)
			if (!InitRead(OF_READWRITE | OF_SHARE_DENY_NONE))
				m_bOk = false;
	}
}

// Constructor To Read An Avi File
CAVIFile::CAVIFile(CWnd* pParentWnd,
				   LPCTSTR lpszFileName,
				   bool bShowMessageBoxOnError/*=true*/)
 :	m_sFileName(lpszFileName),
	m_pParentWnd(pParentWnd),
	m_bShowMessageBoxOnError(bShowMessageBoxOnError)

{
	InitWaveFormatTagTable();

	m_bCreateFile = false;
	m_bTruncateFile = false;
	m_bOk = true;
	m_bInitRead = false;
	m_pFile = NULL;
	m_uiFileMode = 0;
	m_hKillEvent = NULL;
	m_dwTotalVideoStreams = 0;
	m_dwTotalAudioStreams = 0;
	DWORD dwStreamNum;

	for (dwStreamNum = 0 ; dwStreamNum < MAX_VIDEO_STREAMS ; dwStreamNum++)
	{
		m_dwFourCC[dwStreamNum] = 0;
		m_dwRate[dwStreamNum]  = 1;
		m_dwScale[dwStreamNum] = 1;
		m_dwStartVideoOffset[dwStreamNum] = 0;
		m_dwReadNextFrame[dwStreamNum] = 0;
		m_dwWriteNextFrame[dwStreamNum] = 0;
		m_dwTotalWrittenVideoBytes[dwStreamNum] = 0;
		m_dwTotalFrames[dwStreamNum] = 0;
		m_pVideoStream[dwStreamNum] = NULL;
		m_pVideoStreamCompressed[dwStreamNum] = NULL;
		m_pVideoGetFrame[dwStreamNum] = NULL;
		m_dwHeight[dwStreamNum] = 0;
		m_dwWidth[dwStreamNum] = 0;
		memset(&m_VideoCompressorOptions[dwStreamNum], 0, sizeof(AVICOMPRESSOPTIONSVOLATILE));
		m_hCompressionIC[dwStreamNum] = NULL;
		m_pCompressedFormat[dwStreamNum] = NULL;
		m_dwCompressedFormatSize[dwStreamNum] = 0;
		m_pCompressedBuf[dwStreamNum] = NULL;
		m_pPrevUncompressedBuf[dwStreamNum] = NULL;
		m_nKeyRateCounter[dwStreamNum] = 1;
	}

	for (dwStreamNum = 0 ; dwStreamNum < MAX_AUDIO_STREAMS ; dwStreamNum++)
	{
		m_pAudioStream[dwStreamNum] = NULL;
		m_pWaveFormat[dwStreamNum] = (WAVEFORMATEX*)new BYTE[sizeof(WAVEFORMATEX)];
		memset(m_pWaveFormat[dwStreamNum], 0, sizeof(WAVEFORMATEX));
		m_dwReadNextSample[dwStreamNum] = 0;
		m_dwWriteNextSample[dwStreamNum] = 0;
		m_dwTotalWrittenAudioBytes[dwStreamNum] = 0;
		m_dwTotalSamples[dwStreamNum] = 0;
		m_dwAudioSkew[dwStreamNum] = 0;
		m_dwSuggestedAudioBufferSize[dwStreamNum] = 0;
		m_dwStartAudioOffset[dwStreamNum] = 0;
	}

	::InitializeCriticalSection(&m_csAVI);

	WORD wVer = HIWORD(VideoForWindowsVersion());
	if (wVer < 0x010A)
	{
		// oops, we are too old, blow out of here
		m_bOk = false;
	}
	else
	{
		::AVIFileInit();
		if (!InitRead())
			m_bOk = false;
	}
}

CAVIFile::~CAVIFile()
{
	Close();
	::DeleteCriticalSection(&m_csAVI);
	::AVIFileExit();
}

void CAVIFile::Rew()
{
	RewVideo();
	RewAudio();
}

void CAVIFile::RewVideo(DWORD dwStreamNum)									
{
	m_dwReadNextFrame[dwStreamNum] = 0;
}

void CAVIFile::RewAudio(DWORD dwStreamNum)									
{
	m_dwReadNextSample[dwStreamNum] = 0;
}

void CAVIFile::RewVideo()									
{
	for (DWORD dwStreamNum = 0 ; dwStreamNum < MAX_VIDEO_STREAMS ; dwStreamNum++)
		m_dwReadNextFrame[dwStreamNum] = 0;
}

void CAVIFile::RewAudio()									
{
	for (DWORD dwStreamNum = 0 ; dwStreamNum < MAX_AUDIO_STREAMS ; dwStreamNum++)
		m_dwReadNextSample[dwStreamNum] = 0;
}

bool CAVIFile::GetFrameRate(DWORD dwStreamNum, DWORD* pdwRate, DWORD* pdwScale) const
{
	if (pdwRate)
		*pdwRate = m_dwRate[dwStreamNum];
	if (pdwScale)
		*pdwScale = m_dwScale[dwStreamNum];
	return (pdwRate || pdwScale);
}

void CAVIFile::Close()
{
	DWORD dwStreamNum;

	::EnterCriticalSection(&m_csAVI);

	for (dwStreamNum = 0 ; dwStreamNum < MAX_VIDEO_STREAMS ; dwStreamNum++)
	{
		// Decompressor End
		if (m_pPrevUncompressedBuf[dwStreamNum])
			::ICDecompressEnd(m_hCompressionIC[dwStreamNum]);

		// If Has Decompressor
		if (m_hCompressionIC[dwStreamNum])
		{
			// Compressor End
			::ICCompressEnd(m_hCompressionIC[dwStreamNum]);

			// Restore State
			if (m_VideoCompressorOptions[dwStreamNum].lpParms &&
				m_VideoCompressorOptions[dwStreamNum].cbParms > 0)
			{
				::ICSetState(m_hCompressionIC[dwStreamNum],
							m_VideoCompressorOptions[dwStreamNum].lpParms,
							m_VideoCompressorOptions[dwStreamNum].cbParms);
			}

			// Close Compressor
			::ICClose(m_hCompressionIC[dwStreamNum]);
			m_hCompressionIC[dwStreamNum] = NULL;
		}

		if (m_pVideoGetFrame[dwStreamNum])
		{
			::AVIStreamGetFrameClose(m_pVideoGetFrame[dwStreamNum]);
			m_pVideoGetFrame[dwStreamNum] = NULL;
		}
		if (m_pVideoStreamCompressed[dwStreamNum])
		{
			::AVIStreamEndStreaming(m_pVideoStreamCompressed[dwStreamNum]);
			::AVIStreamRelease(m_pVideoStreamCompressed[dwStreamNum]);
			m_pVideoStreamCompressed[dwStreamNum] = NULL;
		}
		if (m_pVideoStream[dwStreamNum])
		{
			::AVIStreamEndStreaming(m_pVideoStream[dwStreamNum]);
			::AVIStreamRelease(m_pVideoStream[dwStreamNum]);
			m_pVideoStream[dwStreamNum] = NULL;
		}
		if (m_VideoCompressorOptions[dwStreamNum].lpParms)
		{
			delete [] (m_VideoCompressorOptions[dwStreamNum].lpParms);
			m_VideoCompressorOptions[dwStreamNum].lpParms = NULL;
			m_VideoCompressorOptions[dwStreamNum].cbParms = 0;
		}
		if (m_pCompressedFormat[dwStreamNum])
		{
			delete [] m_pCompressedFormat[dwStreamNum];
			m_pCompressedFormat[dwStreamNum] = NULL;
		}
		if (m_pCompressedBuf[dwStreamNum])
		{
			delete [] m_pCompressedBuf[dwStreamNum];
			m_pCompressedBuf[dwStreamNum] = NULL;
		}
		if (m_pPrevUncompressedBuf[dwStreamNum])
		{
			delete [] m_pPrevUncompressedBuf[dwStreamNum];
			m_pPrevUncompressedBuf[dwStreamNum] = NULL;
		}
		m_nKeyRateCounter[dwStreamNum] = 1;
		m_dwCompressedFormatSize[dwStreamNum] = 0;
		m_dwFourCC[dwStreamNum] = 0;
		m_dwRate[dwStreamNum]  = 1;
		m_dwScale[dwStreamNum] = 1;
		m_dwStartVideoOffset[dwStreamNum] = 0;
		m_dwReadNextFrame[dwStreamNum] = 0;
		m_dwWriteNextFrame[dwStreamNum] = 0;
		m_dwTotalWrittenVideoBytes[dwStreamNum] = 0;
		m_dwTotalFrames[dwStreamNum] = 0;
		m_dwHeight[dwStreamNum] = 0;
		m_dwWidth[dwStreamNum] = 0;
	}

	for (dwStreamNum = 0 ; dwStreamNum < MAX_AUDIO_STREAMS ; dwStreamNum++)
	{
		if (m_pAudioStream[dwStreamNum])
		{
			::AVIStreamRelease(m_pAudioStream[dwStreamNum]);
			m_pAudioStream[dwStreamNum] = NULL;
		}
		if (m_pWaveFormat[dwStreamNum])
		{
			delete [] m_pWaveFormat[dwStreamNum];
			m_pWaveFormat[dwStreamNum] = NULL;
		}
		m_dwReadNextSample[dwStreamNum] = 0;
		m_dwWriteNextSample[dwStreamNum] = 0;
		m_dwTotalWrittenAudioBytes[dwStreamNum] = 0;
		m_dwTotalSamples[dwStreamNum] = 0;
		m_dwAudioSkew[dwStreamNum] = 0;
		m_dwSuggestedAudioBufferSize[dwStreamNum] = 0;
		m_dwStartAudioOffset[dwStreamNum] = 0;
	}

	// Close File
	CloseFile();

	m_pParentWnd = NULL;
	m_bCreateFile = false;
	m_bTruncateFile = false;
	m_sFileName = _T("");
	m_bInitRead = false;
	m_uiFileMode = 0;
	m_hKillEvent = NULL;
	m_dwTotalVideoStreams = 0;
	m_dwTotalAudioStreams = 0;

	::LeaveCriticalSection(&m_csAVI);
}

void CAVIFile::CloseFile()
{
	if (m_pFile)
	{
		::AVIFileRelease(m_pFile);
		m_pFile = NULL;
	}
}

bool CAVIFile::InitRead(UINT uiMode/*=OF_READ | OF_SHARE_DENY_NONE*/)
{
	HRESULT hr;
	m_uiFileMode = uiMode;

	::EnterCriticalSection(&m_csAVI);

	if (!m_bInitRead)
	{
		// If Not Open In Write Mode
		if (m_pFile == NULL)
		{
			hr = ::AVIFileOpen(	&(PAVIFILE)m_pFile,	// returned file pointer
								m_sFileName,		// file name
								m_uiFileMode,
								NULL);				// use handler determined
													// from file extension....
			if (hr != AVIERR_OK)
			{
				CString str;
#ifdef _DEBUG
				str.Format(_T("InitRead():\n"));
#endif
				str += _T("Cannot Open Avi File!\n");
				TRACE(str);
				if (m_bShowMessageBoxOnError)
					::AfxMessageBox(str, MB_ICONSTOP);
				m_bOk = false;
				::LeaveCriticalSection(&m_csAVI);
				return false;
			}

			// Get Avi File Information
			AVIFILEINFO FileInfo;
			hr = ::AVIFileInfo(m_pFile, &FileInfo, sizeof(FileInfo));
			if (hr != AVIERR_OK)
			{
				m_bOk = false;
				::LeaveCriticalSection(&m_csAVI);
				return false;
			}
		}

		// Open the Video Stream(s)
		m_VideoStreamName.RemoveAll();
		DWORD dwStreamNum;
		for (dwStreamNum = 0 ; dwStreamNum < MAX_VIDEO_STREAMS ; dwStreamNum++)
		{
			if (m_pVideoStream[dwStreamNum] == NULL)
			{
				hr = ::AVIFileGetStream(m_pFile, &(PAVISTREAM)m_pVideoStream[dwStreamNum], streamtypeVIDEO, dwStreamNum);
				if (hr == AVIERR_OK)
				{
					// Streams Count
					++m_dwTotalVideoStreams;

					// Get Video Info
					AVISTREAMINFO VideoStreamInfo;  
					hr = ::AVIStreamInfo(m_pVideoStream[dwStreamNum], &VideoStreamInfo, sizeof(AVISTREAMINFO));
					if (hr != AVIERR_OK)
					{
						m_bOk = false;
						::LeaveCriticalSection(&m_csAVI);
						return false;
					}
			
					// Get Video Frame Rate, Scale, Frame Count, Quality, Stream Name
					m_dwRate[dwStreamNum] = VideoStreamInfo.dwRate;
					m_dwScale[dwStreamNum] = VideoStreamInfo.dwScale;
					m_dwStartVideoOffset[dwStreamNum] = VideoStreamInfo.dwStart;
					m_dwTotalFrames[dwStreamNum] = VideoStreamInfo.dwLength;
					m_VideoCompressorOptions[dwStreamNum].dwQuality = VideoStreamInfo.dwQuality;
					m_VideoStreamName.Add(VideoStreamInfo.szName);
					
					// Get Video Decompressor Info & Bits/Pix
					// Obs.: Stream Information from AVIStreamInfo doesn't return compressor info in read mode...
					LPBITMAPINFOHEADER pBmih = NULL;
					LONG lSize = 0;
					LPBYTE p = NULL;
					hr = ::AVIStreamReadFormat(m_pVideoStream[dwStreamNum], 0, NULL, &lSize);
					if (hr != AVIERR_OK)
					{
						m_bOk = false;
						::LeaveCriticalSection(&m_csAVI);
						return false;
					}
					if (lSize > 0)
					{
						p = new BYTE[lSize];
						pBmih = (LPBITMAPINFOHEADER)p;
						hr = ::AVIStreamReadFormat(m_pVideoStream[dwStreamNum], 0, pBmih, &lSize);
						if (hr != AVIERR_OK)
						{
							m_bOk = false;
							delete [] p;
							::LeaveCriticalSection(&m_csAVI);
							return false;
						}	
						m_dwFourCC[dwStreamNum] = FourCCMakeLowerCase(pBmih->biCompression);
					}
					else
						m_dwFourCC[dwStreamNum] = 0;

					// Prepare the Dib Format
					BITMAPINFOHEADER BmiH;
					memset(&BmiH, 0 , sizeof(BmiH));
					BmiH.biSize = sizeof(BmiH);
					if ((pBmih->biBitCount != 4) &&
						(pBmih->biBitCount != 8) &&
						(pBmih->biBitCount != 16) &&
						(pBmih->biBitCount != 24) &&
						(pBmih->biBitCount != 32))
						BmiH.biBitCount = 24;
					else
						BmiH.biBitCount =  pBmih->biBitCount;
					BmiH.biPlanes = 1;
					BmiH.biHeight = pBmih->biHeight;
					BmiH.biWidth = pBmih->biWidth;
					BmiH.biCompression = BI_RGB;
					BmiH.biSizeImage = 0;

					// Set Height & Width
					m_dwHeight[dwStreamNum] = pBmih->biHeight;
					m_dwWidth[dwStreamNum] = pBmih->biWidth;

					// Clean-up
					if (p)
					{
						delete [] p;
						p = NULL;
						pBmih = NULL;
					}

					// Open for Single Frame Extraction
					m_pVideoGetFrame[dwStreamNum] = ::AVIStreamGetFrameOpen(m_pVideoStream[dwStreamNum],
																			&BmiH);
					if (!m_pVideoGetFrame[dwStreamNum] && BmiH.biBitCount != 24)
					{
						BmiH.biBitCount = 24;
						m_pVideoGetFrame[dwStreamNum] = ::AVIStreamGetFrameOpen(m_pVideoStream[dwStreamNum],
																				&BmiH);
					}

					// If Error
					if (!m_pVideoGetFrame[dwStreamNum])
					{
						::AVIStreamRelease(m_pVideoStream[dwStreamNum]);
						m_pVideoStream[dwStreamNum] = NULL;
						CString str;
						str.Format(_T("No VCM Video Codec Installed For The FourCC: %s\n"),
									FourCCToString(m_dwFourCC[dwStreamNum]));
#ifdef _DEBUG
						str = _T("InitRead():\n") + str;
#endif
						TRACE(str);
						if (m_bShowMessageBoxOnError)
							::AfxMessageBox(str, MB_ICONSTOP);
					}
					else
						m_bInitRead = true;
				}
				else
					break;
			}
			else
			{	
				// Get Video Decompressor Info & Bits/Pix
				// Obs.: Stream Information from AVIStreamInfo doesn't return compressor info in read mode...
				LPBITMAPINFOHEADER pBmih = NULL;
				LONG lSize = 0;
				LPBYTE p = NULL;
				hr = ::AVIStreamReadFormat(m_pVideoStream[dwStreamNum], 0, NULL, &lSize);
				if (hr != AVIERR_OK || lSize <= 0)
				{
					m_bOk = false;
					::LeaveCriticalSection(&m_csAVI);
					return false;
				}
				p = new BYTE[lSize];
				pBmih = (LPBITMAPINFOHEADER)p;
				hr = ::AVIStreamReadFormat(m_pVideoStream[dwStreamNum], 0, pBmih, &lSize);
				if (hr != AVIERR_OK)
				{
					m_bOk = false;
					delete [] p;
					::LeaveCriticalSection(&m_csAVI);
					return false;
				}	

				// Prepare the Dib Format
				BITMAPINFOHEADER BmiH;
				memset(&BmiH, 0 , sizeof(BmiH));
				BmiH.biSize = sizeof(BmiH);
				if ((pBmih->biBitCount != 4) &&
					(pBmih->biBitCount != 8) &&
					(pBmih->biBitCount != 16) &&
					(pBmih->biBitCount != 24) &&
					(pBmih->biBitCount != 32))
					BmiH.biBitCount = 24;
				else
					BmiH.biBitCount =  pBmih->biBitCount;
				BmiH.biPlanes = 1;
				BmiH.biHeight = m_dwHeight[dwStreamNum];
				BmiH.biWidth = m_dwWidth[dwStreamNum];
				BmiH.biCompression = BI_RGB;
				BmiH.biSizeImage = 0;

				// Clean-up
				if (p)
				{
					delete [] p;
					p = NULL;
					pBmih = NULL;
				}

				// Open for Single Frame Extraction
				m_pVideoGetFrame[dwStreamNum] = ::AVIStreamGetFrameOpen(m_pVideoStream[dwStreamNum],
																		&BmiH);
				if (!m_pVideoGetFrame[dwStreamNum] && BmiH.biBitCount != 24)
				{
					BmiH.biBitCount = 24;
					m_pVideoGetFrame[dwStreamNum] = ::AVIStreamGetFrameOpen(m_pVideoStream[dwStreamNum],
																			&BmiH);
				}

				// If Error
				if (!m_pVideoGetFrame[dwStreamNum])
				{
					::AVIStreamRelease(m_pVideoStream[dwStreamNum]);
					m_pVideoStream[dwStreamNum] = NULL;
					CString str;
					str.Format(_T("No VCM Video Codec Installed For The FourCC: %s\n"),
								FourCCToString(m_dwFourCC[dwStreamNum]));
#ifdef _DEBUG
					str = _T("InitRead():\n") + str;
#endif
					TRACE(str);
					if (m_bShowMessageBoxOnError)
						::AfxMessageBox(str, MB_ICONSTOP);
				}
				else
					m_bInitRead = true;
			}
		}

		// Open the Audio Stream(s)
		m_AudioStreamName.RemoveAll();
		for (dwStreamNum = 0 ; dwStreamNum < MAX_AUDIO_STREAMS ; dwStreamNum++)
		{
			if (m_pAudioStream[dwStreamNum] == NULL)
			{
				hr = ::AVIFileGetStream(m_pFile, &(PAVISTREAM)m_pAudioStream[dwStreamNum], streamtypeAUDIO, dwStreamNum);
				if (hr == AVIERR_OK)
				{
					// Streams Count
					++m_dwTotalAudioStreams;

					// Get Audio Info
					AVISTREAMINFO AudioStreamInfo;
					hr = ::AVIStreamInfo(m_pAudioStream[dwStreamNum], &AudioStreamInfo, sizeof(AVISTREAMINFO));
					if (hr != AVIERR_OK)
					{
						m_bOk = false;
						::LeaveCriticalSection(&m_csAVI);
						return false;
					}

					// Get the Length, the Audio Skew, the Suggested Buffer Size, Stream Name
					m_dwTotalSamples[dwStreamNum] = AudioStreamInfo.dwLength;
					m_dwAudioSkew[dwStreamNum] = AudioStreamInfo.dwInitialFrames;	// This member specifies how much to skew the audio data
																					// ahead of the video frames in interleaved files
					m_dwSuggestedAudioBufferSize[dwStreamNum] = AudioStreamInfo.dwSuggestedBufferSize;
					m_dwStartAudioOffset[dwStreamNum] = AudioStreamInfo.dwStart;
					m_AudioStreamName.Add(AudioStreamInfo.szName);
					
					// Get Audio Format
					LONG lSize = 0;
					hr = ::AVIStreamReadFormat(m_pAudioStream[dwStreamNum], 0, NULL, &lSize);
					if (hr != AVIERR_OK)
					{
						m_bOk = false;
						::LeaveCriticalSection(&m_csAVI);
						return false;
					}
					if (lSize > 0)
					{
						LPBYTE p = new BYTE[lSize];
						hr = ::AVIStreamReadFormat(m_pAudioStream[dwStreamNum], 0, p, &lSize);
						if (hr != AVIERR_OK)
						{
							m_bOk = false;
							delete [] p;
							::LeaveCriticalSection(&m_csAVI);
							return false;
						}
						if (lSize == sizeof(WAVEFORMATEX))
						{
							if (!m_pWaveFormat[dwStreamNum])
								m_pWaveFormat[dwStreamNum] = (WAVEFORMATEX*)new BYTE[sizeof(WAVEFORMATEX)];
							memcpy(m_pWaveFormat[dwStreamNum], p, sizeof(WAVEFORMATEX));
							m_pWaveFormat[dwStreamNum]->cbSize = 0;
							delete [] p;

						}
						else if (lSize > sizeof(WAVEFORMATEX))
						{
							if (m_pWaveFormat[dwStreamNum])
								delete [] m_pWaveFormat[dwStreamNum];
							m_pWaveFormat[dwStreamNum] = (LPWAVEFORMATEX)p;
						}
						else
						{
							if (!m_pWaveFormat[dwStreamNum])
							{
								m_pWaveFormat[dwStreamNum] = (WAVEFORMATEX*)new BYTE[sizeof(WAVEFORMATEX)];
								memset(m_pWaveFormat[dwStreamNum], 0, sizeof(WAVEFORMATEX));
							}
							memcpy(m_pWaveFormat[dwStreamNum], p, lSize);
							delete [] p;
						}

						// Determines whether the specified waveform-audio output device 
						// supports a specified waveform-audio format. Returns 
						// MMSYSERR_NOERROR if the format is supported, WAVERR_BADFORMAT if 
						// the format is not supported, and one of the other MMSYSERR_ error 
						// codes if there are other errors encountered in opening the 
						// specified waveform-audio device. 
						MMRESULT res = ::waveOutOpen( 
							NULL,                 // ptr can be NULL for query 
							WAVE_MAPPER,          // the device identifier 
							m_pWaveFormat[dwStreamNum],// defines requested format 
							NULL,                 // no callback 
							NULL,                 // no instance data 
							WAVE_FORMAT_QUERY);   // query only, do not open device 
						
						if (res == WAVERR_BADFORMAT)
						{
							::AVIStreamRelease(m_pAudioStream[dwStreamNum]);
							m_pAudioStream[dwStreamNum] = NULL;
							CString str;
							str.Format(_T("No ACM Audio Codec Installed For %s, Tag ID 0x%X (%i)\n"),
										GetWaveFormatTagString(m_pWaveFormat[dwStreamNum]->wFormatTag),
										m_pWaveFormat[dwStreamNum]->wFormatTag,
										m_pWaveFormat[dwStreamNum]->wFormatTag);
#ifdef _DEBUG
							str = _T("InitRead():\n") + str;
#endif
							TRACE(str);
							if (m_bShowMessageBoxOnError)
								::AfxMessageBox(str, MB_ICONSTOP);
						}
						else
							m_bInitRead = true;
					}
					else
					{
						::AVIStreamRelease(m_pAudioStream[dwStreamNum]);
						m_pAudioStream[dwStreamNum] = NULL;
					}
				}
				else
					break;
			}
			else
			{
				// Get Audio Info
				AVISTREAMINFO AudioStreamInfo;
				hr = ::AVIStreamInfo(m_pAudioStream[dwStreamNum], &AudioStreamInfo, sizeof(AVISTREAMINFO));
				if (hr != AVIERR_OK)
				{
					m_bOk = false;
					::LeaveCriticalSection(&m_csAVI);
					return false;
				}

				// Get the Length, the Audio Skew The Suggested Buffer Size, Stream Name
				m_dwTotalSamples[dwStreamNum] = AudioStreamInfo.dwLength;
				m_dwAudioSkew[dwStreamNum] = AudioStreamInfo.dwInitialFrames;	// This member specifies how much to skew the audio data
																				// ahead of the video frames in interleaved files
				m_dwSuggestedAudioBufferSize[dwStreamNum] = AudioStreamInfo.dwSuggestedBufferSize;
				m_dwStartAudioOffset[dwStreamNum] = AudioStreamInfo.dwStart;
				m_AudioStreamName.Add(AudioStreamInfo.szName);
				m_bInitRead = true;
			}
		}

		::LeaveCriticalSection(&m_csAVI);

		return m_bInitRead;
	}
	else
	{
		::LeaveCriticalSection(&m_csAVI);
		return true;
	}
}

bool CAVIFile::InitVideoWrite(	DWORD dwStreamNum,
								LPBITMAPINFO pBMI,
								LPCTSTR szStreamName/*=NULL*/)
{
	HRESULT hr;

	::EnterCriticalSection(&m_csAVI);

	if (m_pFile == NULL)
	{
		// Note:
		// The AVIFileOpen function doesn't truncate the file to zero
		// when it is opened with the OF_CREATE flag, this to allow
		// captures to preallocated files (usually files with all 0's)
		if (m_bCreateFile && m_bTruncateFile && ::IsExistingFile(m_sFileName))
		{
			try
			{
				CFile::Remove(m_sFileName);
			}
			catch (CFileException* e)
			{
				TCHAR szCause[255];
				e->GetErrorMessage(szCause, 255);
				CString str(szCause);
				str += _T("\n");
				TRACE(str);
				if (m_bShowMessageBoxOnError)
					::AfxMessageBox(str, MB_ICONSTOP);
				e->Delete();
				m_bOk = false;
				::LeaveCriticalSection(&m_csAVI);
				return false;		
			}
		}
	
		m_uiFileMode =	OF_READWRITE |
						(m_bCreateFile ? OF_CREATE : 0) |
						OF_SHARE_DENY_NONE;
		hr = ::AVIFileOpen(&(PAVIFILE)m_pFile,	// returned file pointer
						   m_sFileName,			// file name
						   m_uiFileMode,		// mode to open file with
						   NULL);				// use handler determined
												// from file extension....
		if (hr != AVIERR_OK)
		{
			m_bOk = false;
			::LeaveCriticalSection(&m_csAVI);
			return false;
		}
	}

	if (m_pVideoStream[dwStreamNum] == NULL)
	{
		// Video Stream
		if (pBMI)
		{
			// Decompressor End
			if (m_pPrevUncompressedBuf[dwStreamNum])
				::ICDecompressEnd(m_hCompressionIC[dwStreamNum]);				

			// If Has Decompressor
			if (m_hCompressionIC[dwStreamNum])
			{
				// Compressor End
				::ICCompressEnd(m_hCompressionIC[dwStreamNum]);

				// Restore State
				if (m_VideoCompressorOptions[dwStreamNum].lpParms &&
					m_VideoCompressorOptions[dwStreamNum].cbParms > 0)
				{
					::ICSetState(m_hCompressionIC[dwStreamNum],
								m_VideoCompressorOptions[dwStreamNum].lpParms,
								m_VideoCompressorOptions[dwStreamNum].cbParms);
				}

				// Close Compressor
				::ICClose(m_hCompressionIC[dwStreamNum]);
				m_hCompressionIC[dwStreamNum] = NULL;
			}

			// Free
			if (m_pCompressedFormat[dwStreamNum])
			{
				delete [] m_pCompressedFormat[dwStreamNum];
				m_pCompressedFormat[dwStreamNum] = NULL;
			}
			if (m_pCompressedBuf[dwStreamNum])
			{
				delete [] m_pCompressedBuf[dwStreamNum];
				m_pCompressedBuf[dwStreamNum] = NULL;
			}
			if (m_pPrevUncompressedBuf[dwStreamNum])
			{
				delete [] m_pPrevUncompressedBuf[dwStreamNum];
				m_pPrevUncompressedBuf[dwStreamNum] = NULL;
			}
			m_nKeyRateCounter[dwStreamNum] = 1;
			m_dwCompressedFormatSize[dwStreamNum] = 0;

			// Compressing?
			if (m_VideoCompressorOptions[dwStreamNum].fccHandler)
			{
				// Open Compressor
				m_hCompressionIC[dwStreamNum] = ::ICOpen(ICTYPE_VIDEO,
												m_VideoCompressorOptions[dwStreamNum].fccHandler,
												ICMODE_COMPRESS);
				if (!m_hCompressionIC[dwStreamNum])
				{
					m_bOk = false;
					::LeaveCriticalSection(&m_csAVI);
					return false;
				}

				// Get Info
				ICINFO icinfo;
				memset(&icinfo, 0, sizeof(ICINFO));
				icinfo.dwSize = sizeof(ICINFO);
				::ICGetInfo(m_hCompressionIC[dwStreamNum], &icinfo, sizeof(icinfo));

				// Quality
				if ((icinfo.dwFlags & VIDCF_QUALITY) == 0)
					m_VideoCompressorOptions[dwStreamNum].dwQuality = 0;

				// Data Rate
				if ((icinfo.dwFlags & VIDCF_CRUNCH) == 0)
					m_VideoCompressorOptions[dwStreamNum].dwBytesPerSecond = 0;

				// Key Frames Rate
				if ((icinfo.dwFlags & VIDCF_TEMPORAL) == 0)
					m_VideoCompressorOptions[dwStreamNum].dwKeyFrameEvery = 0;

				// Set Compression Frame Info because:
				// - Indeo 5 needs this message for data rate clamping.
				// - The Morgan Codec requires the message otherwise it assumes 100% quality.
				// - The original version (2700) MPEG-4 V1 requires this message.
				//   V3 (DivX) gives crap if we don't send it.
				ICCOMPRESSFRAMES icf;
				memset(&icf, 0, sizeof icf);
				icf.dwFlags		= ICCOMPRESSFRAMES_PADDING; // Just set it...
															// because something might be looking
															// for a non-zero value here.
				icf.lStartFrame = 0;
				icf.lFrameCount = 0x0FFFFFFF;
				icf.lQuality	= m_VideoCompressorOptions[dwStreamNum].dwQuality;
				icf.lDataRate	= m_VideoCompressorOptions[dwStreamNum].dwBytesPerSecond;
				icf.lKeyRate	= m_VideoCompressorOptions[dwStreamNum].dwKeyFrameEvery;
				icf.dwRate		= m_dwRate[dwStreamNum];
				icf.dwScale		= m_dwScale[dwStreamNum];
				ICSendMessage(	m_hCompressionIC[dwStreamNum],
								ICM_COMPRESS_FRAMES_INFO,
								(WPARAM)&icf,
								sizeof(ICCOMPRESSFRAMES));

				// Get the Destination Format
				m_dwCompressedFormatSize[dwStreamNum] = ICCompressGetFormatSize(m_hCompressionIC[dwStreamNum], pBMI);
				if ((int)(m_dwCompressedFormatSize[dwStreamNum]) < 0)
				{
					m_bOk = false;
					::LeaveCriticalSection(&m_csAVI);
					return false;
				}
				m_pCompressedFormat[dwStreamNum] = (LPBITMAPINFOHEADER)new BYTE[m_dwCompressedFormatSize[dwStreamNum]];
				if (::ICCompressGetFormat(	m_hCompressionIC[dwStreamNum],
											pBMI,
											m_pCompressedFormat[dwStreamNum]) != ICERR_OK)
				{
					m_bOk = false;
					::LeaveCriticalSection(&m_csAVI);
					return false;
				}

				// Set Configuration State
				int nCompressionConfigDataSize = (int)ICGetStateSize(m_hCompressionIC[dwStreamNum]);
				if ((nCompressionConfigDataSize > 0)				&& // Matrox driver returns -1!
					m_VideoCompressorOptions[dwStreamNum].lpParms	&&
					(m_VideoCompressorOptions[dwStreamNum].cbParms ==
											nCompressionConfigDataSize))
				{
					::ICSetState(m_hCompressionIC[dwStreamNum],
								m_VideoCompressorOptions[dwStreamNum].lpParms,
								m_VideoCompressorOptions[dwStreamNum].cbParms);
				}

				// Allocate Enough Destination Buffer
				int nCompressedBufSize = ICCompressGetSize(	m_hCompressionIC[dwStreamNum],      
															(LPBITMAPINFO)pBMI,  
															(LPBITMAPINFO)m_pCompressedFormat[dwStreamNum]);
				if (nCompressedBufSize <= 0)
				{
					nCompressedBufSize = pBMI->bmiHeader.biWidth * pBMI->bmiHeader.biHeight * 4;
					m_pCompressedBuf[dwStreamNum] = new BYTE[nCompressedBufSize];
				}
				else
					m_pCompressedBuf[dwStreamNum] = new BYTE[2 * nCompressedBufSize]; // Be Safe!

				// Begin Compression
				if (::ICCompressBegin(	m_hCompressionIC[dwStreamNum],
										(LPBITMAPINFO)pBMI,
										(LPBITMAPINFO)m_pCompressedFormat[dwStreamNum]) != ICERR_OK)
				{
					m_bOk = false;
					::LeaveCriticalSection(&m_csAVI);
					return false;
				}

				// Needs Previous Buffer?
				if ((icinfo.dwFlags & VIDCF_TEMPORAL) &&
					!(icinfo.dwFlags & VIDCF_FASTTEMPORALC))
					m_pPrevUncompressedBuf[dwStreamNum] = new BYTE[pBMI->bmiHeader.biWidth * pBMI->bmiHeader.biHeight * 4];

				// Start decompression process if necessary
				if (m_pPrevUncompressedBuf[dwStreamNum])
				{
					// Decompress Begin
					if (ICDecompressBegin(	m_hCompressionIC[dwStreamNum],
											(LPBITMAPINFO)m_pCompressedFormat[dwStreamNum],
											(LPBITMAPINFO)pBMI) != ICERR_OK)
					{
						m_bOk = false;
						::LeaveCriticalSection(&m_csAVI);
						return false;
					}
				}
			}

			// Set Video Stream Header
			AVISTREAMINFO VideoStrHdr;
			memset(&VideoStrHdr, 0, sizeof(VideoStrHdr));
			VideoStrHdr.fccType					= streamtypeVIDEO;
			VideoStrHdr.fccHandler				= m_VideoCompressorOptions[dwStreamNum].fccHandler ?
													m_VideoCompressorOptions[dwStreamNum].fccHandler :
													pBMI->bmiHeader.biCompression;
			VideoStrHdr.dwScale					= m_dwScale[dwStreamNum];
			VideoStrHdr.dwRate					= m_dwRate[dwStreamNum];
			VideoStrHdr.dwStart					= m_dwStartVideoOffset[dwStreamNum];
			VideoStrHdr.dwSuggestedBufferSize	= pBMI->bmiHeader.biSizeImage;
			if (szStreamName)
			{
				_tcsncpy(VideoStrHdr.szName, szStreamName, 60);
				VideoStrHdr.szName[59] = _T('\0');
				m_VideoStreamName.Add(VideoStrHdr.szName);
			}
			else
				m_VideoStreamName.Add(_T(""));
			SetRect(&VideoStrHdr.rcFrame, 0, 0,	// rectangle for stream
				(int)pBMI->bmiHeader.biWidth,
				(int)pBMI->bmiHeader.biHeight);

			// Set the Video Size
			m_dwWidth[dwStreamNum] = pBMI->bmiHeader.biWidth;
			m_dwHeight[dwStreamNum] = pBMI->bmiHeader.biHeight;

			// Create Video Stream
			hr = ::AVIFileCreateStream(	m_pFile,									// file pointer
										&(PAVISTREAM)m_pVideoStream[dwStreamNum],	// returned stream pointer
										&VideoStrHdr);								// stream header
			if (hr != AVIERR_OK)
			{
				m_bOk = false;
				::LeaveCriticalSection(&m_csAVI);
				return false;
			}

			// Set the Destination Format
			if (m_VideoCompressorOptions[dwStreamNum].fccHandler)
			{
				hr = ::AVIStreamSetFormat(	m_pVideoStream[dwStreamNum],
											0,
											m_pCompressedFormat[dwStreamNum],
											m_dwCompressedFormatSize[dwStreamNum]);
			}
			else
			{
				hr = ::AVIStreamSetFormat(	m_pVideoStream[dwStreamNum],
											0,
											(LPVOID)pBMI,
											(LONG)CDib::GetBMISize(pBMI));

			}
			if (hr != AVIERR_OK)
			{
				m_bOk = false;
				::LeaveCriticalSection(&m_csAVI);
				return false;
			}

			// Inc. Count
			++m_dwTotalVideoStreams;
		}
	}

	::LeaveCriticalSection(&m_csAVI);

	return true;
}

bool CAVIFile::InitAudioWrite(DWORD dwStreamNum, LPCTSTR szStreamName/*=NULL*/)
{
	HRESULT hr;

	::EnterCriticalSection(&m_csAVI);

	if (m_pFile == NULL)
	{
		// Note:
		// The AVIFileOpen function doesn't truncate the file to zero
		// when it is opened with the OF_CREATE flag, this to allow
		// captures to preallocated files (usually files with all 0's)
		if (m_bCreateFile && m_bTruncateFile && ::IsExistingFile(m_sFileName))
		{
			try
			{
				CFile::Remove(m_sFileName);
			}
			catch (CFileException* e)
			{
				TCHAR szCause[255];
				e->GetErrorMessage(szCause, 255);
				CString str(szCause);
				str += _T("\n");
				TRACE(str);
				if (m_bShowMessageBoxOnError)
					::AfxMessageBox(str, MB_ICONSTOP);
				e->Delete();
				m_bOk = false;
				::LeaveCriticalSection(&m_csAVI);
				return false;
			}
		}

		m_uiFileMode =	OF_READWRITE |
						(m_bCreateFile ? OF_CREATE : 0) |
						OF_SHARE_DENY_NONE;
		hr = ::AVIFileOpen(&(PAVIFILE)m_pFile,	// returned file pointer
						   m_sFileName,			// file name
						   m_uiFileMode,		// mode to open file with
						   NULL);				// use handler determined
												// from file extension....
		if (hr != AVIERR_OK)
		{
			m_bOk = false;
			::LeaveCriticalSection(&m_csAVI);
			return false;
		}
	}

	if (m_pAudioStream[dwStreamNum] == NULL)
	{
		// If Not Allocated
		if (!m_pWaveFormat[dwStreamNum])
		{
			m_pWaveFormat[dwStreamNum] = (WAVEFORMATEX*)new BYTE[sizeof(WAVEFORMATEX)];
			memset(m_pWaveFormat[dwStreamNum], 0, sizeof(WAVEFORMATEX));
		}

		// Audio Stream
		if (m_pWaveFormat[dwStreamNum]->wFormatTag > 0)
		{
			// Set Audio Stream Header
			AVISTREAMINFO AudioStrHdr;
			memset(&AudioStrHdr, 0, sizeof(AVISTREAMINFO));
			AudioStrHdr.fccType					= streamtypeAUDIO;// stream type
			AudioStrHdr.fccHandler				= 0;
			AudioStrHdr.dwScale					= m_pWaveFormat[dwStreamNum]->nBlockAlign;
			AudioStrHdr.dwRate					= m_pWaveFormat[dwStreamNum]->nSamplesPerSec;
			AudioStrHdr.dwSuggestedBufferSize	= 0;
			AudioStrHdr.dwInitialFrames			= 0;
			AudioStrHdr.dwSampleSize			= m_pWaveFormat[dwStreamNum]->nBlockAlign;
			if (szStreamName)
			{
				_tcsncpy(AudioStrHdr.szName, szStreamName, 60);
				AudioStrHdr.szName[59] = _T('\0');
				m_AudioStreamName.Add(AudioStrHdr.szName);
			}
			else
				m_AudioStreamName.Add(_T(""));

			// Create Audio Stream
			hr = ::AVIFileCreateStream(	m_pFile,									// file pointer
										&(PAVISTREAM)m_pAudioStream[dwStreamNum],	// returned stream pointer
										&AudioStrHdr);								// stream header
			if (hr != AVIERR_OK)
			{
				m_bOk = false;
				::LeaveCriticalSection(&m_csAVI);
				return false;
			}

			// Set the Stream Format
			if (m_pWaveFormat[dwStreamNum]->wFormatTag == WAVE_FORMAT_PCM)
			{
				hr = ::AVIStreamSetFormat(	m_pAudioStream[dwStreamNum],
											0,
											m_pWaveFormat[dwStreamNum],
											sizeof(WAVEFORMATEX));
			}
			else
			{
				// Check the Size, because some Codecs return a big value
				// (Ogg Vorbis 2 returns cbSize = 2452)
				// that seems not to work, may be a buffer limitation 
				// of the Microsoft AVI functions...
				if (m_pWaveFormat[dwStreamNum]->cbSize > 1024)
				{
					m_bOk = false;
					::LeaveCriticalSection(&m_csAVI);
					return false;
				}
				hr = ::AVIStreamSetFormat(	m_pAudioStream[dwStreamNum],
											0,
											m_pWaveFormat[dwStreamNum],
											sizeof(WAVEFORMATEX) + m_pWaveFormat[dwStreamNum]->cbSize);
			}
			if (hr != AVIERR_OK)
			{
				m_bOk = false;
				::LeaveCriticalSection(&m_csAVI);
				return false;
			}

			// Inc. Count
			++m_dwTotalAudioStreams;
		}
	}

	::LeaveCriticalSection(&m_csAVI);

	return true;
}

bool CAVIFile::SkipFrame(DWORD dwStreamNum)
{
	if (!m_bOk)
		return false;

	// Check if it is the first read frame
	if (!m_bInitRead)
		if (!InitRead())
			return false;

	if (!m_pVideoGetFrame[dwStreamNum])
		return false;

	::EnterCriticalSection(&m_csAVI);

	// End of File -> Return
	if ((m_dwTotalFrames[dwStreamNum] > 0) && (m_dwReadNextFrame[dwStreamNum] >= m_dwTotalFrames[dwStreamNum]))
	{
		::LeaveCriticalSection(&m_csAVI);
		return false;
	}
	
	// Skip It
	m_dwReadNextFrame[dwStreamNum]++;

	::LeaveCriticalSection(&m_csAVI);

	return true;
}

bool CAVIFile::GetFrame(DWORD dwStreamNum, CDib* pDib)
{
	if (!m_bOk)
		return false;

	if (!pDib)
		return false;

	// Check if we are initialized
	if (!m_bInitRead)
		if (!InitRead())
			return false;

	// Check the Video Get Frame Pointer
	if (!m_pVideoGetFrame[dwStreamNum])
		return false;

	// Enter CS
	::EnterCriticalSection(&m_csAVI);

	// End of File -> Return
	if ((m_dwTotalFrames[dwStreamNum] > 0) &&
		(m_dwReadNextFrame[dwStreamNum] >= m_dwTotalFrames[dwStreamNum]))
	{
		::LeaveCriticalSection(&m_csAVI);
		return false;
	}

	// Get the Frame
	LPBYTE pData = (LPBYTE)::AVIStreamGetFrame(	m_pVideoGetFrame[dwStreamNum],
												m_dwReadNextFrame[dwStreamNum] +
												m_dwStartVideoOffset[dwStreamNum]);
	if (pData)
	{
		pDib->SetBMI((LPBITMAPINFO)pData);
		pDib->SetBits(pData + pDib->GetBMISize());
	}
	else
		pDib->Free();
	
	// Inc. Pos
	m_dwReadNextFrame[dwStreamNum]++;

	// Leave CS
	::LeaveCriticalSection(&m_csAVI);

	return true;
}

bool CAVIFile::GetFrameAt(DWORD dwStreamNum, CDib* pDib, DWORD dwFrame)
{
	if (!m_bOk)
		return false;

	if (!pDib)
		return false;

	// Check if we are initialized
	if (!m_bInitRead)
		if (!InitRead())
			return false;

	// Check the Video Get Frame Pointer
	if (!m_pVideoGetFrame[dwStreamNum])
		return false;
	
	// Enter CS
	::EnterCriticalSection(&m_csAVI);

	// Check Range
	if (dwFrame >= m_dwTotalFrames[dwStreamNum])
	{
		::LeaveCriticalSection(&m_csAVI);
		return false;
	}

	// Get the Frame
	LPBYTE pData = (LPBYTE)::AVIStreamGetFrame(	m_pVideoGetFrame[dwStreamNum],
												dwFrame +
												m_dwStartVideoOffset[dwStreamNum]);
	if (pData)
	{
		pDib->SetBMI((LPBITMAPINFO)pData);
		pDib->SetBits(pData + pDib->GetBMISize());
	}
	else
		pDib->Free();

	// Inc. Pos
	m_dwReadNextFrame[dwStreamNum] = dwFrame + 1;

	// Leave CS
	::LeaveCriticalSection(&m_csAVI);

	return true;
}

bool CAVIFile::SetReadCurrentFramePos(DWORD dwStreamNum, int nCurrentFramePos)
{
	if ((nCurrentFramePos < -1) ||
		(nCurrentFramePos >= (int)m_dwTotalFrames[dwStreamNum]))
		return false;
	else
	{
		m_dwReadNextFrame[dwStreamNum] = nCurrentFramePos + 1;
		return true;
	}
}

bool CAVIFile::SetReadNextFramePos(DWORD dwStreamNum, DWORD dwNextFramePos)
{
	if (dwNextFramePos > m_dwTotalFrames[dwStreamNum])
		return false;
	else
	{
		m_dwReadNextFrame[dwStreamNum] = dwNextFramePos;
		return true;
	}
}

bool CAVIFile::SetWriteCurrentFramePos(DWORD dwStreamNum, int nCurrentFramePos)
{
	if (nCurrentFramePos < -1)
		return false;
	else
	{
		m_dwWriteNextFrame[dwStreamNum] = nCurrentFramePos + 1;
		return true;
	}
}

bool CAVIFile::SetWriteNextFramePos(DWORD dwStreamNum, DWORD dwNextFramePos)
{
	m_dwWriteNextFrame[dwStreamNum] = dwNextFramePos;
	return true;
}

bool CAVIFile::AddFrame(DWORD dwStreamNum, CDib* pDib)
{
	HRESULT hr;
	LONG lBytesWritten;

	if (!m_bOk)
		return false;

	if (!pDib)
		return false;

	// Check if it is the first written frame
	if (m_pVideoStream[dwStreamNum] == NULL)
	{
		if (!InitVideoWrite(dwStreamNum, pDib->GetBMI()))
			return false;
	}
	if (!m_pVideoStream[dwStreamNum])
		return false;

	::EnterCriticalSection(&m_csAVI);

	// Compressing?
	if (m_VideoCompressorOptions[dwStreamNum].fccHandler)
	{
		DWORD dwFlagsIn = ICCOMPRESS_KEYFRAME;
		if (m_VideoCompressorOptions[dwStreamNum].dwKeyFrameEvery == 0)
		{
			if (m_dwWriteNextFrame[dwStreamNum] > 0)
				dwFlagsIn = 0;
		}
		else
		{
			if (--m_nKeyRateCounter[dwStreamNum])
				dwFlagsIn = 0;
			else
				m_nKeyRateCounter[dwStreamNum] = m_VideoCompressorOptions[dwStreamNum].dwKeyFrameEvery;
		}

		// Data Rate
		DWORD dwFrameSize = 0;
		if ((m_VideoCompressorOptions[dwStreamNum].dwBytesPerSecond > 0) &&
			(m_dwRate[dwStreamNum] > 0))
			dwFrameSize = (DWORD)MulDiv(m_VideoCompressorOptions[dwStreamNum].dwBytesPerSecond,
										m_dwScale[dwStreamNum],
										m_dwRate[dwStreamNum]);

		// Original Compressed Image Size
		DWORD dwOrigCompressedImageSize = m_pCompressedFormat[dwStreamNum]->biSizeImage;

		// For first frame always set to 0x7FFFFFFF (Win9x)
		// or 0x00FFFFFF (WinNT)
		DWORD dwFirstFrameSize;
		if (g_bNT)
			dwFirstFrameSize = 0x00FFFFFF;
		else
			dwFirstFrameSize = 0x7FFFFFFF;
		DWORD dwFlags = 0;
		DWORD dwCkID = 0;
		if (dwFlagsIn == ICCOMPRESS_KEYFRAME)
			dwFlags = AVIIF_KEYFRAME;
		if (::ICCompress(m_hCompressionIC[dwStreamNum], 
						dwFlagsIn, 
						m_pCompressedFormat[dwStreamNum], 
						m_pCompressedBuf[dwStreamNum], 
						pDib->GetBMIH(), 
						pDib->GetBits(),
						&dwCkID, 
						&dwFlags, 
						m_dwWriteNextFrame[dwStreamNum],
						(m_dwWriteNextFrame[dwStreamNum] > 0) ? dwFrameSize : dwFirstFrameSize,
						m_VideoCompressorOptions[dwStreamNum].dwQuality,
						(dwFlagsIn & ICCOMPRESS_KEYFRAME) ? NULL : pDib->GetBMIH(), 
						(dwFlagsIn & ICCOMPRESS_KEYFRAME) ? NULL : m_pPrevUncompressedBuf[dwStreamNum])
						!= ICERR_OK)
		{
			m_bOk = false;
			::LeaveCriticalSection(&m_csAVI);
			return false;
		}

		// Size to Write to AVI File
		DWORD dwCompressedImageSize = m_pCompressedFormat[dwStreamNum]->biSizeImage;

		// If we're using a compressor with a stupid algorithm (Microsoft Video 1),
		// we have to decompress the frame again to compress the next one....
		if (m_pPrevUncompressedBuf[dwStreamNum] &&
			((m_VideoCompressorOptions[dwStreamNum].dwKeyFrameEvery == 0) ||
			(m_nKeyRateCounter[dwStreamNum] > 1)))
		{
			if ((int)::ICDecompress(m_hCompressionIC[dwStreamNum],
									(dwFlags & AVIIF_KEYFRAME) ? 0 : ICDECOMPRESS_NOTKEYFRAME,
									(LPBITMAPINFOHEADER)m_pCompressedFormat[dwStreamNum],
									(LPVOID)m_pCompressedBuf[dwStreamNum],
									(LPBITMAPINFOHEADER)pDib->GetBMIH(),
									(LPVOID)m_pPrevUncompressedBuf[dwStreamNum]) < 0)
			{
				m_bOk = false;
				::LeaveCriticalSection(&m_csAVI);
				return false;
			}
		}

		// Reset if it was a keyframe
		if (dwFlags & AVIIF_KEYFRAME)
			m_nKeyRateCounter[dwStreamNum] = m_VideoCompressorOptions[dwStreamNum].dwKeyFrameEvery;

		// Restore Original Compressed Image Size
		m_pCompressedFormat[dwStreamNum]->biSizeImage = dwOrigCompressedImageSize;

		// Save this compressed frame in the AVI stream
		hr = ::AVIStreamWrite(	m_pVideoStream[dwStreamNum], 
								m_dwWriteNextFrame[dwStreamNum],
								1,
								m_pCompressedBuf[dwStreamNum], 
								dwCompressedImageSize, 
								(dwFlags & AVIIF_KEYFRAME),
								NULL, 
								&lBytesWritten);
	}
	else
	{
		// Save the uncompressed frame to the AVI stream
		hr = ::AVIStreamWrite(	m_pVideoStream[dwStreamNum], 
								m_dwWriteNextFrame[dwStreamNum],
								1,
								pDib->GetBits(), 
								pDib->GetImageSize(), 
								AVIIF_KEYFRAME,
								NULL, 
								&lBytesWritten);
	}
	if (hr != AVIERR_OK)
	{
		m_bOk = false;
		::LeaveCriticalSection(&m_csAVI);
		return false;
	}
	else
	{
		m_dwWriteNextFrame[dwStreamNum]++;
		m_dwTotalFrames[dwStreamNum] = m_dwWriteNextFrame[dwStreamNum];
		m_dwTotalWrittenVideoBytes[dwStreamNum] += lBytesWritten;
		::LeaveCriticalSection(&m_csAVI);
		return true;
	}
}

HIC CAVIFile::OpenVideoCompressor(DWORD dwStreamNum)
{
	ICINFO icinfo;
	HIC hIC;
	for (int i = 0 ; ::ICInfo(ICTYPE_VIDEO, i, &icinfo) ; i++)
	{
		hIC = ::ICOpen(icinfo.fccType, icinfo.fccHandler, ICMODE_QUERY);
		if (hIC)
		{
			if (FourCCMakeLowerCase(m_dwFourCC[dwStreamNum]) == FourCCMakeLowerCase(icinfo.fccHandler))
				return hIC;
			::ICClose(hIC);
		}
	}
	return NULL;
}

void CAVIFile::CloseVideoCompressor(HIC hIC)
{
	::ICClose(hIC);
}

bool CAVIFile::GetVideoCompressorConfig(DWORD dwStreamNum,
										HIC hIC,
										BYTE** ppConfigData/*=NULL*/,
										DWORD* pSize/*=NULL*/)
{
	// From Driver To Buffer And To m_VideoCompressorOptions
	if (hIC)
	{
		int nStateSize = (int)ICGetStateSize(hIC); // Matrox driver returns -1!
		if (nStateSize > 0)
		{
			if (ppConfigData)
			{
				if (*ppConfigData)
					delete [] (*ppConfigData);
				(*ppConfigData) = new BYTE[2*nStateSize];	// Stupid ffdshow gives a size and reads 1 byte more!!
															// Be safe and allocate the double, you never know...
				::ICGetState(hIC, (LPVOID)(*ppConfigData), (DWORD)nStateSize);
				if (pSize)
					(*pSize) = (DWORD)nStateSize;
			}

			if (m_VideoCompressorOptions[dwStreamNum].lpParms)
				delete [] (m_VideoCompressorOptions[dwStreamNum].lpParms);
			m_VideoCompressorOptions[dwStreamNum].cbParms = (DWORD)nStateSize;
			m_VideoCompressorOptions[dwStreamNum].lpParms = new BYTE[2*nStateSize];	// Stupid ffdshow gives a size and reads 1 byte more!!
																					// Be safe and allocate the double, you never know...
			::ICGetState(hIC, (LPVOID)(m_VideoCompressorOptions[dwStreamNum].lpParms), (DWORD)nStateSize);

			return true;
		}
		else
			return false;
	}
	// From m_VideoCompressorOptions To Buffer
	else if (m_VideoCompressorOptions[dwStreamNum].lpParms && (m_VideoCompressorOptions[dwStreamNum].cbParms > 0))
	{
		if (ppConfigData)
		{
			if (*ppConfigData)
				delete [] (*ppConfigData);
			(*ppConfigData) = new BYTE[m_VideoCompressorOptions[dwStreamNum].cbParms];
			memcpy((*ppConfigData), m_VideoCompressorOptions[dwStreamNum].lpParms, m_VideoCompressorOptions[dwStreamNum].cbParms);
		}
		if (pSize)
			(*pSize) = m_VideoCompressorOptions[dwStreamNum].cbParms;

		return true;
	}
	else
	{
		if (pSize)
			(*pSize) = 0;
		if (ppConfigData)
			(*ppConfigData) = NULL;
		return false;
	}
}

bool CAVIFile::SetVideoCompressorConfig(DWORD dwStreamNum,
										HIC hIC,
										const BYTE* pConfigData/*=NULL*/,
										DWORD dwSize/*=0*/)
{
	if (pConfigData && (dwSize > 0))
	{
		// From Buffer To m_VideoCompressorOptions
		if (m_VideoCompressorOptions[dwStreamNum].lpParms)
			delete [] (m_VideoCompressorOptions[dwStreamNum].lpParms);
		m_VideoCompressorOptions[dwStreamNum].cbParms = dwSize;
		m_VideoCompressorOptions[dwStreamNum].lpParms = new BYTE[dwSize];
		memcpy(m_VideoCompressorOptions[dwStreamNum].lpParms, pConfigData, dwSize);

		// From Buffer To Driver
		if (hIC)
		{
			int nStateSize = (int)ICGetStateSize(hIC); // Matrox driver returns -1!
			if ((nStateSize > 0) && (dwSize > 0) && ((DWORD)nStateSize == dwSize))
			{
				if ((int)::ICSetState(hIC, (LPVOID)pConfigData, (DWORD)nStateSize) != nStateSize)
					return false;
			}
		}

		return true;
	}
	else if (hIC)
	{
		// From m_VideoCompressorOptions To Driver
		int nStateSize = (int)ICGetStateSize(hIC); // Matrox driver returns -1!
		if ((nStateSize > 0) &&
			(nStateSize == (int)m_VideoCompressorOptions[dwStreamNum].cbParms) &&
			(m_VideoCompressorOptions[dwStreamNum].lpParms))
		{
			if ((int)::ICSetState(hIC,
								(LPVOID)(m_VideoCompressorOptions[dwStreamNum].lpParms),
								m_VideoCompressorOptions[dwStreamNum].cbParms) == nStateSize)
				return true;
		}
	}

	return false;
}

bool CAVIFile::HasVideoCompressorConfigDialog(HIC hIC)
{
	if (hIC)
		return (ICQueryConfigure(hIC));
	else
		return false;
}

bool CAVIFile::VideoCompressorConfigDialog(HIC hIC)
{
	if (hIC)
	{
		if (ICQueryConfigure(hIC))
		{
			// Has No Ok, Cancel Return...
			ICConfigure(hIC, m_pParentWnd->GetSafeHwnd());
			return true;
		}
	}
	return false;
}

bool CAVIFile::HasVideoCompressorAboutDialog(HIC hIC)
{
	if (hIC)
		return (ICQueryAbout(hIC));
	else
		return false;
}

bool CAVIFile::VideoCompressorAboutDialog(HIC hIC)
{
	if (hIC)
	{
		if (ICQueryAbout(hIC))
		{
			ICAbout(hIC, m_pParentWnd->GetSafeHwnd());
			return true;
		}
	}
	return false;
}

bool CAVIFile::VideoCompressorDialog(DWORD dwStreamNum)
{
	DWORD dwUnc, dwRaw, dwDib, dwRgb;
	dwUnc = BI_RGB; // = 0x00000000
	dwRaw = StringToFourCC(_T("raw "));
	dwDib = StringToFourCC(_T("dib "));
	dwRgb = StringToFourCC(_T("rgb "));
	if ((m_dwFourCC[dwStreamNum] == dwUnc) ||
		(m_dwFourCC[dwStreamNum] == dwRaw) ||
		(m_dwFourCC[dwStreamNum] == dwDib) ||
		(m_dwFourCC[dwStreamNum] == dwRgb))
		m_dwFourCC[dwStreamNum] = dwDib; // ICCompressorChoose Only Likes this FourCC!
	COMPVARS CompressorVars;
	::ZeroMemory(&CompressorVars, sizeof(COMPVARS));
	CompressorVars.cbSize = sizeof(COMPVARS);
	CompressorVars.dwFlags = ICMF_COMPVARS_VALID;
	CompressorVars.fccType = ICTYPE_VIDEO;
	CompressorVars.fccHandler = m_dwFourCC[dwStreamNum];
	CompressorVars.lQ = m_VideoCompressorOptions[dwStreamNum].dwQuality;
	CompressorVars.lKey = m_VideoCompressorOptions[dwStreamNum].dwKeyFrameEvery;
	CompressorVars.lDataRate = m_VideoCompressorOptions[dwStreamNum].dwBytesPerSecond >> 10; // lDataRate is in kilobytes per second!
	CompressorVars.hic = ::ICOpen(ICTYPE_VIDEO, m_dwFourCC[dwStreamNum], ICMODE_QUERY);
	SetVideoCompressorConfig(dwStreamNum, CompressorVars.hic);
	char szTitle[128];
	_snprintf(szTitle, 128, "Video Compression (Stream #%u)", dwStreamNum+1);
	if (::ICCompressorChoose(	m_pParentWnd->GetSafeHwnd(),
								ICMF_CHOOSE_ALLCOMPRESSORS | ICMF_CHOOSE_DATARATE | ICMF_CHOOSE_KEYFRAME,
								NULL,
								NULL,
								&CompressorVars,
								szTitle))
	{
		DWORD dwSelectedFourCC = FourCCMakeLowerCase(CompressorVars.fccHandler);

		if ((dwSelectedFourCC == dwUnc) ||
			(dwSelectedFourCC == dwRaw) ||
			(dwSelectedFourCC == dwDib) ||
			(dwSelectedFourCC == dwRgb))
			dwSelectedFourCC = dwUnc; // I Like This FourCC!
	
		if (m_VideoCompressorOptions[dwStreamNum].lpParms)
			delete [] (m_VideoCompressorOptions[dwStreamNum].lpParms);
		memset(&m_VideoCompressorOptions[dwStreamNum], 0, sizeof(m_VideoCompressorOptions[dwStreamNum]));
		m_VideoCompressorOptions[dwStreamNum].fccType = streamtypeVIDEO;
		m_VideoCompressorOptions[dwStreamNum].fccHandler = m_dwFourCC[dwStreamNum] = dwSelectedFourCC;
		m_VideoCompressorOptions[dwStreamNum].dwQuality = CompressorVars.lQ;
		m_VideoCompressorOptions[dwStreamNum].dwKeyFrameEvery = CompressorVars.lKey;				// 0 if not checked
		m_VideoCompressorOptions[dwStreamNum].dwBytesPerSecond = CompressorVars.lDataRate << 10;	// 0 if not checked, lDataRate is in kilobytes per second!
		m_VideoCompressorOptions[dwStreamNum].dwFlags |= AVICOMPRESSF_DATARATE | AVICOMPRESSF_KEYFRAMES;
		GetVideoCompressorConfig(dwStreamNum, CompressorVars.hic); // Write The Config To m_VideoCompressorOptions
		::ICClose(CompressorVars.hic);
		CompressorVars.hic = NULL;
		::ICCompressorFree(&CompressorVars);
		return true;
	}
	else
	{
		::ICClose(CompressorVars.hic);
		CompressorVars.hic = NULL;
		::ICCompressorFree(&CompressorVars);
		return false;
	}
}

bool CAVIFile::EnumVideoCodecs(	CStringArray &Names,
								CDWordArray &fcc,
								CDWordArray &QualitySupport,
								CDWordArray &KeyframeRateSupport,
								CDWordArray &DataRateSupport,
								CDWordArray &AboutDlgSupport,
								CDWordArray &ConfigDlgSupport)
{
	HIC hIC;
	ICINFO icinfo;
	
	// Clean-Up
	Names.RemoveAll( );
	fcc.RemoveAll( );
	QualitySupport.RemoveAll();
	KeyframeRateSupport.RemoveAll();
	DataRateSupport.RemoveAll();
	AboutDlgSupport.RemoveAll();
	ConfigDlgSupport.RemoveAll();

	// Uncompressed
	Names.Add(_T("Uncompressed"));
	fcc.Add(0);
	QualitySupport.Add(0);
	KeyframeRateSupport.Add(0);
	DataRateSupport.Add(0);
	AboutDlgSupport.Add(0);
	ConfigDlgSupport.Add(0);

	for (int i = 0 ; ::ICInfo(ICTYPE_VIDEO, i, &icinfo) ; i++)
	{
		hIC = ::ICOpen(icinfo.fccType, icinfo.fccHandler, ICMODE_QUERY);
		if (hIC)
		{
			// Get Info
			::ICGetInfo(hIC, &icinfo, sizeof(icinfo));
			DWORD dwQualitySupport = ((icinfo.dwFlags & VIDCF_QUALITY) > 0) ? 1 : 0;
			DWORD dwKeyframesRateSupport = ((icinfo.dwFlags & VIDCF_TEMPORAL) > 0) ? 1 : 0;
			DWORD dwDataRateSupport = ((icinfo.dwFlags & VIDCF_CRUNCH) > 0) ? 1 : 0;
			CString sDescription = CString(icinfo.szDescription);

			// Remove Duplicates
			for (int j = 0 ; j < Names.GetSize() ; j++)
			{
				if (Names[j] == sDescription)
				{
					Names.RemoveAt(j);
					fcc.RemoveAt(j);
					QualitySupport.RemoveAt(j);
					KeyframeRateSupport.RemoveAt(j);
					DataRateSupport.RemoveAt(j);
					AboutDlgSupport.RemoveAt(j);
					ConfigDlgSupport.RemoveAt(j);
					break;
				}
			}

			// Add It
			Names.Add(sDescription);
			fcc.Add(FourCCMakeLowerCase(icinfo.fccHandler));
			QualitySupport.Add(dwQualitySupport);
			KeyframeRateSupport.Add(dwKeyframesRateSupport);
			DataRateSupport.Add(dwDataRateSupport);
			AboutDlgSupport.Add(HasVideoCompressorAboutDialog(hIC) ? 1 : 0);
			ConfigDlgSupport.Add(HasVideoCompressorConfigDialog(hIC) ? 1 : 0);

			::ICClose(hIC);
		}
	}

	return (fcc.GetSize() > 0);
}

static const TCHAR* WaveFormatTagTable[0x10000];
static const TCHAR WaveFormatTag_0x0000[] = _T("Microsoft Unknown Wave Format");
static const TCHAR WaveFormatTag_0x0001[] = _T("Uncompressed PCM");
static const TCHAR WaveFormatTag_0x0002[] = _T("Microsoft ADPCM");
static const TCHAR WaveFormatTag_0x0003[] = _T("IEEE Float");
static const TCHAR WaveFormatTag_0x0004[] = _T("Compaq Computer VSELP");
static const TCHAR WaveFormatTag_0x0005[] = _T("IBM CVSD");
static const TCHAR WaveFormatTag_0x0006[] = _T("Microsoft A-Law");
static const TCHAR WaveFormatTag_0x0007[] = _T("Microsoft mu-Law");
static const TCHAR WaveFormatTag_0x0008[] = _T("Microsoft DTS");
static const TCHAR WaveFormatTag_0x0010[] = _T("OKI ADPCM");
static const TCHAR WaveFormatTag_0x0011[] = _T("Intel DVI/IMA ADPCM");
static const TCHAR WaveFormatTag_0x0012[] = _T("Videologic MediaSpace ADPCM");
static const TCHAR WaveFormatTag_0x0013[] = _T("Sierra Semiconductor ADPCM");
static const TCHAR WaveFormatTag_0x0014[] = _T("Antex Electronics G.723 ADPCM");
static const TCHAR WaveFormatTag_0x0015[] = _T("DSP Solutions DigiSTD");
static const TCHAR WaveFormatTag_0x0016[] = _T("DSP Solutions DigiFIX");
static const TCHAR WaveFormatTag_0x0017[] = _T("Dialogic OKI ADPCM");
static const TCHAR WaveFormatTag_0x0018[] = _T("MediaVision ADPCM");
static const TCHAR WaveFormatTag_0x0019[] = _T("Hewlett-Packard CU");
static const TCHAR WaveFormatTag_0x0020[] = _T("Yamaha ADPCM");
static const TCHAR WaveFormatTag_0x0021[] = _T("Speech Compression Sonarc");
static const TCHAR WaveFormatTag_0x0022[] = _T("DSP Group TrueSpeech");
static const TCHAR WaveFormatTag_0x0023[] = _T("Echo Speech EchoSC1");
static const TCHAR WaveFormatTag_0x0024[] = _T("Audiofile AF36");
static const TCHAR WaveFormatTag_0x0025[] = _T("Audio Processing Technology APTX");
static const TCHAR WaveFormatTag_0x0026[] = _T("AudioFile AF10");
static const TCHAR WaveFormatTag_0x0027[] = _T("Prosody 1612");
static const TCHAR WaveFormatTag_0x0028[] = _T("LRC");
static const TCHAR WaveFormatTag_0x0030[] = _T("Dolby AC2");
static const TCHAR WaveFormatTag_0x0031[] = _T("Microsoft GSM 6.10");
static const TCHAR WaveFormatTag_0x0032[] = _T("MSNAudio");
static const TCHAR WaveFormatTag_0x0033[] = _T("Antex Electronics ADPCME");
static const TCHAR WaveFormatTag_0x0034[] = _T("Control Resources VQLPC");
static const TCHAR WaveFormatTag_0x0035[] = _T("DSP Solutions DigiREAL");
static const TCHAR WaveFormatTag_0x0036[] = _T("DSP Solutions DigiADPCM");
static const TCHAR WaveFormatTag_0x0037[] = _T("Control Resources CR10");
static const TCHAR WaveFormatTag_0x0038[] = _T("Natural MicroSystems VBXADPCM");
static const TCHAR WaveFormatTag_0x0039[] = _T("Crystal Semiconductor IMA ADPCM");
static const TCHAR WaveFormatTag_0x003A[] = _T("EchoSC3");
static const TCHAR WaveFormatTag_0x003B[] = _T("Rockwell ADPCM");
static const TCHAR WaveFormatTag_0x003C[] = _T("Rockwell Digit LK");
static const TCHAR WaveFormatTag_0x003D[] = _T("Xebec");
static const TCHAR WaveFormatTag_0x0040[] = _T("Antex Electronics G.721 ADPCM");
static const TCHAR WaveFormatTag_0x0041[] = _T("G.728 CELP");
static const TCHAR WaveFormatTag_0x0042[] = _T("MSG723");
static const TCHAR WaveFormatTag_0x0050[] = _T("MPEG Layer-2 or Layer-1");
static const TCHAR WaveFormatTag_0x0052[] = _T("RT24");
static const TCHAR WaveFormatTag_0x0053[] = _T("PAC");
static const TCHAR WaveFormatTag_0x0055[] = _T("MPEG Layer-3");
static const TCHAR WaveFormatTag_0x0059[] = _T("Lucent G.723");
static const TCHAR WaveFormatTag_0x0060[] = _T("Cirrus");
static const TCHAR WaveFormatTag_0x0061[] = _T("ESPCM");
static const TCHAR WaveFormatTag_0x0062[] = _T("Voxware");
static const TCHAR WaveFormatTag_0x0063[] = _T("Canopus Atrac");
static const TCHAR WaveFormatTag_0x0064[] = _T("G.726 ADPCM");
static const TCHAR WaveFormatTag_0x0065[] = _T("G.722 ADPCM");
static const TCHAR WaveFormatTag_0x0066[] = _T("DSAT");
static const TCHAR WaveFormatTag_0x0067[] = _T("DSAT Display");
static const TCHAR WaveFormatTag_0x0069[] = _T("Voxware Byte Aligned");
static const TCHAR WaveFormatTag_0x0070[] = _T("Voxware AC8");
static const TCHAR WaveFormatTag_0x0071[] = _T("Voxware AC10");
static const TCHAR WaveFormatTag_0x0072[] = _T("Voxware AC16");
static const TCHAR WaveFormatTag_0x0073[] = _T("Voxware AC20");
static const TCHAR WaveFormatTag_0x0074[] = _T("Voxware MetaVoice");
static const TCHAR WaveFormatTag_0x0075[] = _T("Voxware MetaSound");
static const TCHAR WaveFormatTag_0x0076[] = _T("Voxware RT29HW");
static const TCHAR WaveFormatTag_0x0077[] = _T("Voxware VR12");
static const TCHAR WaveFormatTag_0x0078[] = _T("Voxware VR18");
static const TCHAR WaveFormatTag_0x0079[] = _T("Voxware TQ40");
static const TCHAR WaveFormatTag_0x0080[] = _T("Softsound");
static const TCHAR WaveFormatTag_0x0081[] = _T("Voxware TQ60");
static const TCHAR WaveFormatTag_0x0082[] = _T("MSRT24");
static const TCHAR WaveFormatTag_0x0083[] = _T("G.729A");
static const TCHAR WaveFormatTag_0x0084[] = _T("MVI MV12");
static const TCHAR WaveFormatTag_0x0085[] = _T("DF G.726");
static const TCHAR WaveFormatTag_0x0086[] = _T("DF GSM610");
static const TCHAR WaveFormatTag_0x0088[] = _T("ISIAudio");
static const TCHAR WaveFormatTag_0x0089[] = _T("Onlive");
static const TCHAR WaveFormatTag_0x0091[] = _T("SBC24");
static const TCHAR WaveFormatTag_0x0092[] = _T("Dolby AC3 SPDIF");
static const TCHAR WaveFormatTag_0x0093[] = _T("MediaSonic G.723");
static const TCHAR WaveFormatTag_0x0094[] = _T("Aculab PLC Prosody 8kbps");
static const TCHAR WaveFormatTag_0x0097[] = _T("ZyXEL ADPCM");
static const TCHAR WaveFormatTag_0x0098[] = _T("Philips LPCBB");
static const TCHAR WaveFormatTag_0x0099[] = _T("Packed");
static const TCHAR WaveFormatTag_0x00FF[] = _T("AAC");
static const TCHAR WaveFormatTag_0x0100[] = _T("Rhetorex ADPCM");
static const TCHAR WaveFormatTag_0x0101[] = _T("IBM mu-law");
static const TCHAR WaveFormatTag_0x0102[] = _T("IBM A-law");
static const TCHAR WaveFormatTag_0x0103[] = _T("IBM AVC Adaptive Differential Pulse Code Modulation (ADPCM)");
static const TCHAR WaveFormatTag_0x0111[] = _T("Vivo G.723");
static const TCHAR WaveFormatTag_0x0112[] = _T("Vivo Siren");
static const TCHAR WaveFormatTag_0x0123[] = _T("Digital G.723");
static const TCHAR WaveFormatTag_0x0125[] = _T("Sanyo LD ADPCM");
static const TCHAR WaveFormatTag_0x0130[] = _T("Sipro Lab Telecom ACELP NET");
static const TCHAR WaveFormatTag_0x0131[] = _T("Sipro Lab Telecom ACELP 4800");
static const TCHAR WaveFormatTag_0x0132[] = _T("Sipro Lab Telecom ACELP 8V3");
static const TCHAR WaveFormatTag_0x0133[] = _T("Sipro Lab Telecom G.729");
static const TCHAR WaveFormatTag_0x0134[] = _T("Sipro Lab Telecom G.729A");
static const TCHAR WaveFormatTag_0x0135[] = _T("Sipro Lab Telecom Kelvin");
static const TCHAR WaveFormatTag_0x0140[] = _T("Windows Media Video V8");
static const TCHAR WaveFormatTag_0x0150[] = _T("Qualcomm PureVoice");
static const TCHAR WaveFormatTag_0x0151[] = _T("Qualcomm HalfRate");
static const TCHAR WaveFormatTag_0x0155[] = _T("Ring Zero Systems TUB GSM");
static const TCHAR WaveFormatTag_0x0160[] = _T("Microsoft Audio 1");
static const TCHAR WaveFormatTag_0x0161[] = _T("Windows Media Audio V7 / V8 / V9");
static const TCHAR WaveFormatTag_0x0162[] = _T("Windows Media Audio Professional V9");
static const TCHAR WaveFormatTag_0x0163[] = _T("Windows Media Audio Lossless V9");
static const TCHAR WaveFormatTag_0x0200[] = _T("Creative Labs ADPCM");
static const TCHAR WaveFormatTag_0x0202[] = _T("Creative Labs Fastspeech8");
static const TCHAR WaveFormatTag_0x0203[] = _T("Creative Labs Fastspeech10");
static const TCHAR WaveFormatTag_0x0210[] = _T("UHER Informatic GmbH ADPCM");
static const TCHAR WaveFormatTag_0x0220[] = _T("Quarterdeck");
static const TCHAR WaveFormatTag_0x0230[] = _T("I-link Worldwide VC");
static const TCHAR WaveFormatTag_0x0240[] = _T("Aureal RAW Sport");
static const TCHAR WaveFormatTag_0x0250[] = _T("Interactive Products HSX");
static const TCHAR WaveFormatTag_0x0251[] = _T("Interactive Products RPELP");
static const TCHAR WaveFormatTag_0x0260[] = _T("Consistent Software CS2");
static const TCHAR WaveFormatTag_0x0270[] = _T("Sony SCX");
static const TCHAR WaveFormatTag_0x0300[] = _T("Fujitsu FM Towns Snd");
static const TCHAR WaveFormatTag_0x0400[] = _T("BTV Digital");
static const TCHAR WaveFormatTag_0x0401[] = _T("Intel Music Coder");
static const TCHAR WaveFormatTag_0x0450[] = _T("QDesign Music");
static const TCHAR WaveFormatTag_0x0680[] = _T("VME VMPCM");
static const TCHAR WaveFormatTag_0x0681[] = _T("AT&T Labs TPC");
static const TCHAR WaveFormatTag_0x08AE[] = _T("ClearJump LiteWave");
static const TCHAR WaveFormatTag_0x1000[] = _T("Olivetti GSM");
static const TCHAR WaveFormatTag_0x1001[] = _T("Olivetti ADPCM");
static const TCHAR WaveFormatTag_0x1002[] = _T("Olivetti CELP");
static const TCHAR WaveFormatTag_0x1003[] = _T("Olivetti SBC");
static const TCHAR WaveFormatTag_0x1004[] = _T("Olivetti OPR");
static const TCHAR WaveFormatTag_0x1100[] = _T("Lernout & Hauspie Codec");
static const TCHAR WaveFormatTag_0x1101[] = _T("Lernout & Hauspie CELP Codec");
static const TCHAR WaveFormatTag_0x1102[] = _T("Lernout & Hauspie SBC Codec");
static const TCHAR WaveFormatTag_0x1103[] = _T("Lernout & Hauspie SBC Codec");
static const TCHAR WaveFormatTag_0x1104[] = _T("Lernout & Hauspie SBC Codec");
static const TCHAR WaveFormatTag_0x1400[] = _T("Norris");
static const TCHAR WaveFormatTag_0x1401[] = _T("AT&T ISIAudio");
static const TCHAR WaveFormatTag_0x1500[] = _T("Soundspace Music Compression");
static const TCHAR WaveFormatTag_0x181C[] = _T("VoxWare RT24 Speech");
static const TCHAR WaveFormatTag_0x1FC4[] = _T("NCT Soft ALF2CD (www.nctsoft.com)");
static const TCHAR WaveFormatTag_0x2000[] = _T("Dolby AC3");
static const TCHAR WaveFormatTag_0x2001[] = _T("Dolby DTS");
static const TCHAR WaveFormatTag_0x2002[] = _T("WAVE_FORMAT_14_4");
static const TCHAR WaveFormatTag_0x2003[] = _T("WAVE_FORMAT_28_8");
static const TCHAR WaveFormatTag_0x2004[] = _T("WAVE_FORMAT_COOK");
static const TCHAR WaveFormatTag_0x2005[] = _T("WAVE_FORMAT_DNET");
static const TCHAR WaveFormatTag_0x674F[] = _T("Ogg Vorbis 1");
static const TCHAR WaveFormatTag_0x6750[] = _T("Ogg Vorbis 2");
static const TCHAR WaveFormatTag_0x6751[] = _T("Ogg Vorbis 3");
static const TCHAR WaveFormatTag_0x676F[] = _T("Ogg Vorbis 1+");
static const TCHAR WaveFormatTag_0x6770[] = _T("Ogg Vorbis 2+");
static const TCHAR WaveFormatTag_0x6771[] = _T("Ogg Vorbis 3+");
static const TCHAR WaveFormatTag_0x7A21[] = _T("GSM-AMR (CBR, no SID)");
static const TCHAR WaveFormatTag_0x7A22[] = _T("GSM-AMR (VBR, including SID)");
static const TCHAR WaveFormatTag_0xFFFE[] = _T("WAVE_FORMAT_EXTENSIBLE");
static const TCHAR WaveFormatTag_0xFFFF[] = _T("WAVE_FORMAT_DEVELOPMENT");
void CAVIFile::InitWaveFormatTagTable()
{
	for (int i = 0 ; i < 0x10000 ; i++)
		WaveFormatTagTable[i] = _T("");
	WaveFormatTagTable[0x0000] = WaveFormatTag_0x0000;
	WaveFormatTagTable[0x0001] = WaveFormatTag_0x0001;
	WaveFormatTagTable[0x0002] = WaveFormatTag_0x0002;
	WaveFormatTagTable[0x0003] = WaveFormatTag_0x0003;
	WaveFormatTagTable[0x0004] = WaveFormatTag_0x0004;
	WaveFormatTagTable[0x0005] = WaveFormatTag_0x0005;
	WaveFormatTagTable[0x0006] = WaveFormatTag_0x0006;
	WaveFormatTagTable[0x0007] = WaveFormatTag_0x0007;
	WaveFormatTagTable[0x0008] = WaveFormatTag_0x0008;
	WaveFormatTagTable[0x0010] = WaveFormatTag_0x0010;
	WaveFormatTagTable[0x0011] = WaveFormatTag_0x0011;
	WaveFormatTagTable[0x0012] = WaveFormatTag_0x0012;
	WaveFormatTagTable[0x0013] = WaveFormatTag_0x0013;
	WaveFormatTagTable[0x0014] = WaveFormatTag_0x0014;
	WaveFormatTagTable[0x0015] = WaveFormatTag_0x0015;
	WaveFormatTagTable[0x0016] = WaveFormatTag_0x0016;
	WaveFormatTagTable[0x0017] = WaveFormatTag_0x0017;
	WaveFormatTagTable[0x0018] = WaveFormatTag_0x0018;
	WaveFormatTagTable[0x0019] = WaveFormatTag_0x0019;
	WaveFormatTagTable[0x0020] = WaveFormatTag_0x0020;
	WaveFormatTagTable[0x0021] = WaveFormatTag_0x0021;
	WaveFormatTagTable[0x0022] = WaveFormatTag_0x0022;
	WaveFormatTagTable[0x0023] = WaveFormatTag_0x0023;
	WaveFormatTagTable[0x0024] = WaveFormatTag_0x0024;
	WaveFormatTagTable[0x0025] = WaveFormatTag_0x0025;
	WaveFormatTagTable[0x0026] = WaveFormatTag_0x0026;
	WaveFormatTagTable[0x0027] = WaveFormatTag_0x0027;
	WaveFormatTagTable[0x0028] = WaveFormatTag_0x0028;
	WaveFormatTagTable[0x0030] = WaveFormatTag_0x0030;
	WaveFormatTagTable[0x0031] = WaveFormatTag_0x0031;
	WaveFormatTagTable[0x0032] = WaveFormatTag_0x0032;
	WaveFormatTagTable[0x0033] = WaveFormatTag_0x0033;
	WaveFormatTagTable[0x0034] = WaveFormatTag_0x0034;
	WaveFormatTagTable[0x0035] = WaveFormatTag_0x0035;
	WaveFormatTagTable[0x0036] = WaveFormatTag_0x0036;
	WaveFormatTagTable[0x0037] = WaveFormatTag_0x0037;
	WaveFormatTagTable[0x0038] = WaveFormatTag_0x0038;
	WaveFormatTagTable[0x0039] = WaveFormatTag_0x0039;
	WaveFormatTagTable[0x003A] = WaveFormatTag_0x003A;
	WaveFormatTagTable[0x003B] = WaveFormatTag_0x003B;
	WaveFormatTagTable[0x003C] = WaveFormatTag_0x003C;
	WaveFormatTagTable[0x003D] = WaveFormatTag_0x003D;
	WaveFormatTagTable[0x0040] = WaveFormatTag_0x0040;
	WaveFormatTagTable[0x0041] = WaveFormatTag_0x0041;
	WaveFormatTagTable[0x0042] = WaveFormatTag_0x0042;
	WaveFormatTagTable[0x0050] = WaveFormatTag_0x0050;
	WaveFormatTagTable[0x0052] = WaveFormatTag_0x0052;
	WaveFormatTagTable[0x0053] = WaveFormatTag_0x0053;
	WaveFormatTagTable[0x0055] = WaveFormatTag_0x0055;
	WaveFormatTagTable[0x0059] = WaveFormatTag_0x0059;
	WaveFormatTagTable[0x0060] = WaveFormatTag_0x0060;
	WaveFormatTagTable[0x0061] = WaveFormatTag_0x0061;
	WaveFormatTagTable[0x0062] = WaveFormatTag_0x0062;
	WaveFormatTagTable[0x0063] = WaveFormatTag_0x0063;
	WaveFormatTagTable[0x0064] = WaveFormatTag_0x0064;
	WaveFormatTagTable[0x0065] = WaveFormatTag_0x0065;
	WaveFormatTagTable[0x0066] = WaveFormatTag_0x0066;
	WaveFormatTagTable[0x0067] = WaveFormatTag_0x0067;
	WaveFormatTagTable[0x0069] = WaveFormatTag_0x0069;
	WaveFormatTagTable[0x0070] = WaveFormatTag_0x0070;
	WaveFormatTagTable[0x0071] = WaveFormatTag_0x0071;
	WaveFormatTagTable[0x0072] = WaveFormatTag_0x0072;
	WaveFormatTagTable[0x0073] = WaveFormatTag_0x0073;
	WaveFormatTagTable[0x0074] = WaveFormatTag_0x0074;
	WaveFormatTagTable[0x0075] = WaveFormatTag_0x0075;
	WaveFormatTagTable[0x0076] = WaveFormatTag_0x0076;
	WaveFormatTagTable[0x0077] = WaveFormatTag_0x0077;
	WaveFormatTagTable[0x0078] = WaveFormatTag_0x0078;
	WaveFormatTagTable[0x0079] = WaveFormatTag_0x0079;
	WaveFormatTagTable[0x0080] = WaveFormatTag_0x0080;
	WaveFormatTagTable[0x0081] = WaveFormatTag_0x0081;
	WaveFormatTagTable[0x0082] = WaveFormatTag_0x0082;
	WaveFormatTagTable[0x0083] = WaveFormatTag_0x0083;
	WaveFormatTagTable[0x0084] = WaveFormatTag_0x0084;
	WaveFormatTagTable[0x0085] = WaveFormatTag_0x0085;
	WaveFormatTagTable[0x0086] = WaveFormatTag_0x0086;
	WaveFormatTagTable[0x0088] = WaveFormatTag_0x0088;
	WaveFormatTagTable[0x0089] = WaveFormatTag_0x0089;
	WaveFormatTagTable[0x0091] = WaveFormatTag_0x0091;
	WaveFormatTagTable[0x0092] = WaveFormatTag_0x0092;
	WaveFormatTagTable[0x0093] = WaveFormatTag_0x0093;
	WaveFormatTagTable[0x0094] = WaveFormatTag_0x0094;
	WaveFormatTagTable[0x0097] = WaveFormatTag_0x0097;
	WaveFormatTagTable[0x0098] = WaveFormatTag_0x0098;
	WaveFormatTagTable[0x0099] = WaveFormatTag_0x0099;
	WaveFormatTagTable[0x00FF] = WaveFormatTag_0x00FF;
	WaveFormatTagTable[0x0100] = WaveFormatTag_0x0100;
	WaveFormatTagTable[0x0101] = WaveFormatTag_0x0101;
	WaveFormatTagTable[0x0102] = WaveFormatTag_0x0102;
	WaveFormatTagTable[0x0103] = WaveFormatTag_0x0103;
	WaveFormatTagTable[0x0111] = WaveFormatTag_0x0111;
	WaveFormatTagTable[0x0112] = WaveFormatTag_0x0112;
	WaveFormatTagTable[0x0123] = WaveFormatTag_0x0123;
	WaveFormatTagTable[0x0125] = WaveFormatTag_0x0125;
	WaveFormatTagTable[0x0130] = WaveFormatTag_0x0130;
	WaveFormatTagTable[0x0131] = WaveFormatTag_0x0131;
	WaveFormatTagTable[0x0132] = WaveFormatTag_0x0132;
	WaveFormatTagTable[0x0133] = WaveFormatTag_0x0133;
	WaveFormatTagTable[0x0134] = WaveFormatTag_0x0134;
	WaveFormatTagTable[0x0135] = WaveFormatTag_0x0135;
	WaveFormatTagTable[0x0140] = WaveFormatTag_0x0140;
	WaveFormatTagTable[0x0150] = WaveFormatTag_0x0150;
	WaveFormatTagTable[0x0151] = WaveFormatTag_0x0151;
	WaveFormatTagTable[0x0155] = WaveFormatTag_0x0155;
	WaveFormatTagTable[0x0160] = WaveFormatTag_0x0160;
	WaveFormatTagTable[0x0161] = WaveFormatTag_0x0161;
	WaveFormatTagTable[0x0162] = WaveFormatTag_0x0162;
	WaveFormatTagTable[0x0163] = WaveFormatTag_0x0163;
	WaveFormatTagTable[0x0200] = WaveFormatTag_0x0200;
	WaveFormatTagTable[0x0202] = WaveFormatTag_0x0202;
	WaveFormatTagTable[0x0203] = WaveFormatTag_0x0203;
	WaveFormatTagTable[0x0210] = WaveFormatTag_0x0210;
	WaveFormatTagTable[0x0220] = WaveFormatTag_0x0220;
	WaveFormatTagTable[0x0230] = WaveFormatTag_0x0230;
	WaveFormatTagTable[0x0240] = WaveFormatTag_0x0240;
	WaveFormatTagTable[0x0250] = WaveFormatTag_0x0250;
	WaveFormatTagTable[0x0251] = WaveFormatTag_0x0251;
	WaveFormatTagTable[0x0260] = WaveFormatTag_0x0260;
	WaveFormatTagTable[0x0270] = WaveFormatTag_0x0270;
	WaveFormatTagTable[0x0300] = WaveFormatTag_0x0300;
	WaveFormatTagTable[0x0400] = WaveFormatTag_0x0400;
	WaveFormatTagTable[0x0401] = WaveFormatTag_0x0401;
	WaveFormatTagTable[0x0450] = WaveFormatTag_0x0450;
	WaveFormatTagTable[0x0680] = WaveFormatTag_0x0680;
	WaveFormatTagTable[0x0681] = WaveFormatTag_0x0681;
	WaveFormatTagTable[0x08AE] = WaveFormatTag_0x08AE;
	WaveFormatTagTable[0x1000] = WaveFormatTag_0x1000;
	WaveFormatTagTable[0x1001] = WaveFormatTag_0x1001;
	WaveFormatTagTable[0x1002] = WaveFormatTag_0x1002;
	WaveFormatTagTable[0x1003] = WaveFormatTag_0x1003;
	WaveFormatTagTable[0x1004] = WaveFormatTag_0x1004;
	WaveFormatTagTable[0x1100] = WaveFormatTag_0x1100;
	WaveFormatTagTable[0x1101] = WaveFormatTag_0x1101;
	WaveFormatTagTable[0x1102] = WaveFormatTag_0x1102;
	WaveFormatTagTable[0x1103] = WaveFormatTag_0x1103;
	WaveFormatTagTable[0x1104] = WaveFormatTag_0x1104;
	WaveFormatTagTable[0x1400] = WaveFormatTag_0x1400;
	WaveFormatTagTable[0x1401] = WaveFormatTag_0x1401;
	WaveFormatTagTable[0x1500] = WaveFormatTag_0x1500;
	WaveFormatTagTable[0x181C] = WaveFormatTag_0x181C;
	WaveFormatTagTable[0x1FC4] = WaveFormatTag_0x1FC4;
	WaveFormatTagTable[0x2000] = WaveFormatTag_0x2000;
	WaveFormatTagTable[0x2001] = WaveFormatTag_0x2001;
	WaveFormatTagTable[0x2002] = WaveFormatTag_0x2002;
	WaveFormatTagTable[0x2003] = WaveFormatTag_0x2003;
	WaveFormatTagTable[0x2004] = WaveFormatTag_0x2004;
	WaveFormatTagTable[0x2005] = WaveFormatTag_0x2005;
	WaveFormatTagTable[0x674F] = WaveFormatTag_0x674F;
	WaveFormatTagTable[0x6750] = WaveFormatTag_0x6750;
	WaveFormatTagTable[0x6751] = WaveFormatTag_0x6751;
	WaveFormatTagTable[0x676F] = WaveFormatTag_0x676F;
	WaveFormatTagTable[0x6770] = WaveFormatTag_0x6770;
	WaveFormatTagTable[0x6771] = WaveFormatTag_0x6771;
	WaveFormatTagTable[0x7A21] = WaveFormatTag_0x7A21;
	WaveFormatTagTable[0x7A22] = WaveFormatTag_0x7A22;
	WaveFormatTagTable[0xFFFE] = WaveFormatTag_0xFFFE;
	WaveFormatTagTable[0xFFFF] = WaveFormatTag_0xFFFF;
}

CString CAVIFile::GetWaveFormatTagString(WORD wFormatTag)
{
	return WaveFormatTagTable[wFormatTag];
}

bool CAVIFile::AudioCompressorDialog(DWORD dwStreamNum) 
{
	if (m_pWaveFormat)
	{
		int nMaxWaveFormatSize;
		if (::acmMetrics(NULL, ACM_METRIC_MAX_SIZE_FORMAT, (LPVOID)&nMaxWaveFormatSize) != MMSYSERR_NOERROR)
			return false;

		LPWAVEFORMATEX pWaveFormat = (LPWAVEFORMATEX)new BYTE[nMaxWaveFormatSize];
		memcpy(pWaveFormat, m_pWaveFormat[dwStreamNum], sizeof(WAVEFORMATEX));
		delete [] m_pWaveFormat[dwStreamNum];
		m_pWaveFormat[dwStreamNum] = pWaveFormat;

		// Ask the user which format they want to use
		ACMFORMATCHOOSE cfmt;
		::ZeroMemory(&cfmt, sizeof(ACMFORMATCHOOSE));
		cfmt.cbStruct = sizeof(ACMFORMATCHOOSE);
		CString sTitle;
		sTitle.Format(_T("Audio Compression (Stream #%u)"), dwStreamNum+1);
		cfmt.pszTitle = sTitle;
		cfmt.fdwStyle = ACMFORMATCHOOSE_STYLEF_INITTOWFXSTRUCT;
		cfmt.fdwEnum = ACM_FORMATENUMF_INPUT;
		cfmt.hwndOwner = m_pParentWnd->GetSafeHwnd();
		cfmt.pwfx = m_pWaveFormat[dwStreamNum];
		cfmt.pwfxEnum = NULL;
		cfmt.cbwfx = nMaxWaveFormatSize;
		if (::acmFormatChoose(&cfmt) == MMSYSERR_NOERROR)
		{
			if (m_pWaveFormat[dwStreamNum]->wFormatTag == WAVE_FORMAT_PCM)
				m_pWaveFormat[dwStreamNum]->cbSize = 0;
			return true;	
		}
		else
			return false;
	}
	else
		return false;
}

BOOL WINAPI CAVIFile::AcmFormatEnumCallback(HACMDRIVERID hadid, LPACMFORMATDETAILS pafd, DWORD dwInstance, DWORD fdwSupport)
{
	DWORD nMaxBitRate, nMinBitRate;
	if (pafd->pwfx->nChannels == 1)
	{
		nMaxBitRate = 32000; // 32 kbps
		nMinBitRate = 16000; // 16 kbps
	}
	else
	{
		nMaxBitRate = 64000; // 64 kbps
		nMinBitRate = 32000; // 32 kbps
	}

	if ((pafd->pwfx->wFormatTag == WAVE_FORMAT_MPEGLAYER3) &&
		(pafd->pwfx->cbSize == MPEGLAYER3_WFX_EXTRA_BYTES) &&
		(pafd->pwfx->nAvgBytesPerSec <= (nMaxBitRate / 8)) &&
		(pafd->pwfx->nAvgBytesPerSec >= (nMinBitRate / 8)))
		return FALSE;

	return TRUE;
}

bool CAVIFile::AutoChooseAudioCompressor(DWORD dwStreamNum, LPWAVEFORMATEX* ppWaveFormat)
{
	MMRESULT res;
	int nMaxWaveFormatSize;

	if (!HasAudio())
		return false;

	if (::acmMetrics(NULL, ACM_METRIC_MAX_SIZE_FORMAT, (LPVOID)&nMaxWaveFormatSize) != MMSYSERR_NOERROR)
		return false;
	*ppWaveFormat = (LPWAVEFORMATEX)new BYTE[nMaxWaveFormatSize];

	// Check For Mp3
	::ZeroMemory(*ppWaveFormat, sizeof(WAVEFORMATEX));
	(*ppWaveFormat)->wFormatTag = WAVE_FORMAT_MPEGLAYER3;
	if (m_pWaveFormat[dwStreamNum]->nChannels > 2)
		(*ppWaveFormat)->nChannels = 2;
	else
		(*ppWaveFormat)->nChannels = m_pWaveFormat[dwStreamNum]->nChannels;
	if (m_pWaveFormat[dwStreamNum]->nSamplesPerSec > 22050)
		(*ppWaveFormat)->nSamplesPerSec = 22050;
	else
		(*ppWaveFormat)->nSamplesPerSec = m_pWaveFormat[dwStreamNum]->nSamplesPerSec;
	ACMFORMATDETAILS AcmFormatDetails;
	::ZeroMemory(&AcmFormatDetails, sizeof(ACMFORMATDETAILS));
	AcmFormatDetails.cbStruct = sizeof(ACMFORMATDETAILS);
	AcmFormatDetails.dwFormatIndex = 0;
	AcmFormatDetails.dwFormatTag = WAVE_FORMAT_MPEGLAYER3;
	AcmFormatDetails.fdwSupport = 0;
	AcmFormatDetails.pwfx = *ppWaveFormat;
	AcmFormatDetails.cbwfx = nMaxWaveFormatSize;
	res = ::acmFormatEnum(	NULL, &AcmFormatDetails, AcmFormatEnumCallback, 0,
							ACM_FORMATENUMF_INPUT | ACM_FORMATENUMF_NSAMPLESPERSEC |
							ACM_FORMATENUMF_NCHANNELS | ACM_FORMATENUMF_WFORMATTAG);
	if (res == MMSYSERR_NOERROR)
	   return true;

	// Check For ADPCM
	::ZeroMemory(*ppWaveFormat, sizeof(WAVEFORMATEX));
	(*ppWaveFormat)->wFormatTag = WAVE_FORMAT_ADPCM;
	res = ::acmFormatSuggest(NULL, m_pWaveFormat[dwStreamNum], *ppWaveFormat, nMaxWaveFormatSize, ACM_FORMATSUGGESTF_WFORMATTAG);
	if (res == MMSYSERR_NOERROR)
	   return true;

	// PCM is Ok...
	::ZeroMemory(*ppWaveFormat, sizeof(WAVEFORMATEX));
	(*ppWaveFormat)->wFormatTag = WAVE_FORMAT_PCM;
	res = ::acmFormatSuggest(NULL, m_pWaveFormat[dwStreamNum], *ppWaveFormat, nMaxWaveFormatSize, ACM_FORMATSUGGESTF_WFORMATTAG);
	if (res == MMSYSERR_NOERROR)
	   return true;

	return false;
}

bool CAVIFile::Interleave(	CString sInputFileName,	
							CString sOutputFileName,
							CWnd* pWnd/*=NULL*/,
							HANDLE hKillEvent/*=NULL*/,
							bool bShowMessageBoxOnError/*=true*/)
{
	// Open Input File
	CAVIFile InAvi(	pWnd,
					sInputFileName,
					bShowMessageBoxOnError);
	InAvi.SetKillEvent(hKillEvent);	// For Processing Progress Stop

	// Check Total Streams
	DWORD dwTotalStreams = InAvi.GetVideoStreamsCount() + InAvi.GetAudioStreamsCount();
	if (dwTotalStreams <= 1)
		return false;

	// Create Streams Array
	PAVISTREAM* AviStreams = (PAVISTREAM*)new PAVISTREAM[InAvi.GetVideoStreamsCount() + InAvi.GetAudioStreamsCount()];
	DWORD dwStreamNum;
	int nOutStreamNum = 0;
	for (dwStreamNum = 0 ; dwStreamNum < InAvi.GetVideoStreamsCount() ; dwStreamNum++)
		AviStreams[nOutStreamNum++] = InAvi.GetVideoStream(dwStreamNum);
	for (dwStreamNum = 0 ; dwStreamNum < InAvi.GetAudioStreamsCount() ; dwStreamNum++)
		AviStreams[nOutStreamNum++] = InAvi.GetAudioStream(dwStreamNum);
	
	// Interleave
	bool res = InAvi.SaveCopyAs(sOutputFileName, dwTotalStreams, AviStreams);
	
	// Clean-Up
	delete [] AviStreams;

	return res;
}

bool CAVIFile::ReCompress(	CString sOutputFileName,
							bool* pbVideoStreamsSave,
							bool* pbVideoStreamsChange,
							bool* pbAudioStreamsSave,
							bool* pbAudioStreamsChange,
							const DWORD* pdwFourCC,
							const LPWAVEFORMATEX pWaveFormat,
							const DWORD* pdwQuality,
							const DWORD* pdwKeyframesRate,
							const DWORD* pdwDataRate,
							const BYTE* pVideoConfigData,
							DWORD dwVideoConfigDataSize)
{
	if (!m_bOk)
		return false;

	if ((sOutputFileName == m_sFileName) || (sOutputFileName == _T("")))
	{
		return ReCompress(	pbVideoStreamsSave,
							pbVideoStreamsChange,
							pbAudioStreamsSave,
							pbAudioStreamsChange,
							pdwFourCC,
							pWaveFormat,
							pdwQuality,
							pdwKeyframesRate,
							pdwDataRate,
							pVideoConfigData,
							dwVideoConfigDataSize);	
	}
	else
	{
		CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sOutputFileName);
		DWORD dwStreamNum;
		DWORD dwTotalVideoSaveStreams = 0;
		DWORD dwTotalVideoChangeStreams = 0;
		DWORD dwTotalAudioSaveStreams = 0;
		DWORD dwTotalAudioChangeStreams = 0;
		int nVideoOutStreamNum = 0;
		int nAudioOutStreamNum = 0;
		int nOutStreamNum;
		bool bDoCreate = true;

		// Make Sure We Are Initialized
		if (!m_bInitRead)
			return false;

		DWORD dwAttrib = ::GetFileAttributes(sOutputFileName);
		if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
		{
			CString str;
			str.Format(_T("The file %s\nis read only\n"), sOutputFileName);
#ifdef _DEBUG
			str = _T("ReCompress():\n") + str;
#endif
			TRACE(str);
			if (m_bShowMessageBoxOnError)
				::AfxMessageBox(str, MB_ICONSTOP);

			return false;
		}

		// Count Streams
		for (dwStreamNum = 0 ; dwStreamNum < GetVideoStreamsCount() ; dwStreamNum++)
		{
			if (pbVideoStreamsSave[dwStreamNum])
				++dwTotalVideoSaveStreams;
		}
		for (dwStreamNum = 0 ; dwStreamNum < GetVideoStreamsCount() ; dwStreamNum++)
		{
			if (pbVideoStreamsChange[dwStreamNum])
				++dwTotalVideoChangeStreams;
		}
		for (dwStreamNum = 0 ; dwStreamNum < GetAudioStreamsCount() ; dwStreamNum++)
		{
			if (pbAudioStreamsSave[dwStreamNum])
				++dwTotalAudioSaveStreams;
		}
		for (dwStreamNum = 0 ; dwStreamNum < GetAudioStreamsCount() ; dwStreamNum++)
		{
			if (pbAudioStreamsChange[dwStreamNum])
				++dwTotalAudioChangeStreams;
		}

		// Rewind All
		Rew();

		// Stream Copy
		if ((dwTotalVideoChangeStreams < dwTotalVideoSaveStreams) || (dwTotalAudioChangeStreams < dwTotalAudioSaveStreams))
		{
			nOutStreamNum = 0;
			PAVISTREAM* AviStreams = (PAVISTREAM*)new PAVISTREAM[dwTotalVideoSaveStreams + dwTotalAudioSaveStreams];
			for (dwStreamNum = 0 ; dwStreamNum < GetVideoStreamsCount() ; dwStreamNum++)
			{
				if (pbVideoStreamsSave[dwStreamNum] && !pbVideoStreamsChange[dwStreamNum])
				{
					AviStreams[nOutStreamNum++] = GetVideoStream(dwStreamNum);
					++nVideoOutStreamNum;
				}
			}
			for (dwStreamNum = 0 ; dwStreamNum < GetAudioStreamsCount() ; dwStreamNum++)
			{
				if (pbAudioStreamsSave[dwStreamNum] && !pbAudioStreamsChange[dwStreamNum])
				{
					AviStreams[nOutStreamNum++] = GetAudioStream(dwStreamNum);
					++nAudioOutStreamNum;
				}
			}
			bool res = SaveCopyAs(sTempFileName, nVideoOutStreamNum + nAudioOutStreamNum, AviStreams);
			delete [] AviStreams;
			if (!res)
				return false;

			bDoCreate = false;
		}

		// Compress / Decompress
		if ((dwTotalVideoChangeStreams > 0) || (dwTotalAudioChangeStreams > 0))
		{
			// Create or Open Output Avi File
			CAVIFile OutAvi(m_pParentWnd,
							sTempFileName,
							m_dwRate[0],
							m_dwScale[0],
							pWaveFormat ? pWaveFormat : GetWaveFormat(0),
							bDoCreate,
							true,
							m_bShowMessageBoxOnError);

			// Audio Dialog(s)
			nOutStreamNum = nAudioOutStreamNum;
			for (dwStreamNum = 0 ; dwStreamNum < GetAudioStreamsCount() ; dwStreamNum++)
			{
				if (pbAudioStreamsChange[dwStreamNum])
				{
					if (!pWaveFormat)
					{
						if (!OutAvi.AudioCompressorDialog(nOutStreamNum))
							return false;
					}
					++nOutStreamNum;
				}
			}

			// Video Dialog(s)
			nOutStreamNum = nVideoOutStreamNum;
			for (dwStreamNum = 0 ; dwStreamNum < GetVideoStreamsCount() ; dwStreamNum++)
			{
				if (pbVideoStreamsChange[dwStreamNum])
				{
					if (pdwFourCC)
					{
						OutAvi.SetVideoCompressor(nOutStreamNum, *pdwFourCC);
						if (pdwQuality)
							OutAvi.SetVideoCompressorQuality(nOutStreamNum, *pdwQuality);
						if (pdwKeyframesRate)
							OutAvi.SetVideoCompressorKeyframesRate(nOutStreamNum, *pdwKeyframesRate);
						if (pdwDataRate)
							OutAvi.SetVideoCompressorDataRate(nOutStreamNum, *pdwDataRate);
						if (pVideoConfigData)
							OutAvi.SetVideoCompressorConfig(nOutStreamNum, NULL, pVideoConfigData, dwVideoConfigDataSize);
					}
					else
					{
						OutAvi.SetVideoCompressor(nOutStreamNum, OutAvi.ChooseVideoCompressorForSave(GetFourCC(dwStreamNum)));
						OutAvi.SetVideoCompressorQuality(nOutStreamNum, GetVideoCompressorQuality(dwStreamNum));
						OutAvi.SetVideoCompressorKeyframesRate(nOutStreamNum, GetVideoCompressorKeyframesRate(dwStreamNum));
						OutAvi.SetVideoCompressorDataRate(nOutStreamNum, GetVideoCompressorDataRate(dwStreamNum));
						OutAvi.SetVideoCompressorConfig(nOutStreamNum,
														NULL,
														(const BYTE*)m_VideoCompressorOptions[dwStreamNum].lpParms,
														m_VideoCompressorOptions[dwStreamNum].cbParms);
						if (!OutAvi.VideoCompressorDialog(nOutStreamNum))
							return false;
					}
					++nOutStreamNum;
				}
			}

			// Video Processing
			for (dwStreamNum = 0 ; dwStreamNum < GetVideoStreamsCount() ; dwStreamNum++)
			{
				if (pbVideoStreamsChange[dwStreamNum])
				{	
					CDib Dib;
					int nPercentDone;
					int nPrevPercentDone = -1;
					bool bFirst = true;
					bool bToRgb24 = false;
					while (GetFrame(dwStreamNum, &Dib))
					{
						// Convert to 24bpp?
						if (bToRgb24)
							Dib.ConvertTo24bits();

						// Progress
						nPercentDone = Round((double)m_dwReadNextFrame[dwStreamNum] * 100.0 / (double)m_dwTotalFrames[dwStreamNum]);
						if (nPercentDone > nPrevPercentDone)
							::PostMessage(m_pParentWnd->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeVIDEO, (LPARAM)nPercentDone);
						nPrevPercentDone = nPercentDone;

						// First Frame?
						if (bFirst)
						{
							bFirst = false;
							if (!OutAvi.InitVideoWrite(nVideoOutStreamNum, Dib.GetBMI(), GetVideoStreamName(dwStreamNum)))
							{
								// Retry Init Video Write with 24 bpp
								if (Dib.GetBitCount() != 24)
								{
									Dib.ConvertTo24bits();
									bToRgb24 = true;
									OutAvi.SetOk(true); // Reset Last Error!
									if (!OutAvi.InitVideoWrite(nVideoOutStreamNum, Dib.GetBMI(), GetVideoStreamName(dwStreamNum)))
										return false;
								}
								else
									return false;
							}
						}

						// Add Frame
						// Note:
						// XVID 1.1 with 640x480 images is reading outside
						// the source Dib buffer, fuck!
						if (!OutAvi.AddFrame(nVideoOutStreamNum, &Dib))
							return false;

						// Do Exit?
						if (DoExit())
							return false;
					}
					++nVideoOutStreamNum;
				}
			}

			// Audio Processing
			for (dwStreamNum = 0 ; dwStreamNum < GetAudioStreamsCount() ; dwStreamNum++)
			{
				if (pbAudioStreamsChange[dwStreamNum])
					if (!AudioCodec(dwStreamNum, nAudioOutStreamNum++, &OutAvi))
						return false;
			}

			// Close
			OutAvi.Close();

			// Interleave: returns false on error or if only one stream
			if (!Interleave(sTempFileName,	
							sOutputFileName,
							m_pParentWnd,
							m_hKillEvent,
							m_bShowMessageBoxOnError))
			{
				try
				{
					if (::IsExistingFile(sOutputFileName))
						CFile::Remove(sOutputFileName);
					CFile::Rename(sTempFileName, sOutputFileName);
					return true;
				}
				catch (CFileException* e)
				{
					TCHAR szCause[255];
					e->GetErrorMessage(szCause, 255);
					CString str(szCause);
					str += _T("\n");
					TRACE(str);
					if (m_bShowMessageBoxOnError)
						::AfxMessageBox(str, MB_ICONSTOP);
					e->Delete();
					return false;
				}	
			}
			else
			{
				try
				{
					CFile::Remove(sTempFileName);
					return true;
				}
				catch (CFileException* e)
				{
					TCHAR szCause[255];
					e->GetErrorMessage(szCause, 255);
					CString str(szCause);
					str += _T("\n");
					TRACE(str);
					if (m_bShowMessageBoxOnError)
						::AfxMessageBox(str, MB_ICONSTOP);
					e->Delete();
					return false;
				}
			}
		}
		else
		{
			try
			{
				if (::IsExistingFile(sOutputFileName))
					CFile::Remove(sOutputFileName);
				CFile::Rename(sTempFileName, sOutputFileName);
				return true;
			}
			catch (CFileException* e)
			{
				TCHAR szCause[255];
				e->GetErrorMessage(szCause, 255);
				CString str(szCause);
				str += _T("\n");
				TRACE(str);
				if (m_bShowMessageBoxOnError)
					::AfxMessageBox(str, MB_ICONSTOP);
				e->Delete();
				return false;
			}
		}
	}
}

bool CAVIFile::ReCompress(	bool* pbVideoStreamsSave,
							bool* pbVideoStreamsChange,
							bool* pbAudioStreamsSave,
							bool* pbAudioStreamsChange,
							const DWORD* pdwFourCC,
							const LPWAVEFORMATEX pWaveFormat,
							const DWORD* pdwQuality,
							const DWORD* pdwKeyframesRate,
							const DWORD* pdwDataRate,
							const BYTE* pVideoConfigData,
							DWORD dwVideoConfigDataSize)
{
	if (!m_bOk)
		return false;

	// Store Src File Name
	CString sSrcFileName = GetFileName();

	DWORD dwAttrib = ::GetFileAttributes(sSrcFileName);
	if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
	{
		CString str;
		str.Format(_T("The file %s\nis read only\n"), sSrcFileName);
#ifdef _DEBUG
		str = _T("ReCompress():\n") + str;
#endif
		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		return false;
	}

	// Create Avi Temp Dst File Name
	CString sDstFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sSrcFileName);

	if (ReCompress(	sDstFileName,
					pbVideoStreamsSave,
					pbVideoStreamsChange,
					pbAudioStreamsSave,
					pbAudioStreamsChange,
					pdwFourCC,
					pWaveFormat,
					pdwQuality,
					pdwKeyframesRate,
					pdwDataRate,
					pVideoConfigData,
					dwVideoConfigDataSize))
	{
		try
		{
			Close();
			CFile::Remove(sSrcFileName);
			CFile::Rename(sDstFileName, sSrcFileName);
			return true;
		}
		catch (CFileException* e)
		{
			TCHAR szCause[255];
			e->GetErrorMessage(szCause, 255);
			CString str(szCause);
			str += _T("\n");
			TRACE(str);
			if (m_bShowMessageBoxOnError)
				::AfxMessageBox(str, MB_ICONSTOP);
			e->Delete();
			return false;
		}
	}
	else
		return false;
}

bool CAVIFile::AudioCodec(	DWORD dwInStreamNum,
							DWORD dwOutStreamNum,
							CAVIFile* pOutAvi,
							bool bForceOnePass/*=false*/)
{
	MMRESULT res, res1, res2, res3;
	int nPercentDone;
	int nPrevPercentDone = -1;

	if (!m_bOk || !pOutAvi)
		return false;

	if ((GetWaveFormatTag(dwInStreamNum) == pOutAvi->GetWaveFormatTag(dwOutStreamNum)) && 
		(GetWaveBits(dwInStreamNum) == pOutAvi->GetWaveBits(dwOutStreamNum)) &&
		(GetWaveNumOfChannels(dwInStreamNum) == pOutAvi->GetWaveNumOfChannels(dwOutStreamNum)) &&
		(GetWaveSampleRate(dwInStreamNum) == pOutAvi->GetWaveSampleRate(dwOutStreamNum)) &&
		(GetWaveBytesPerSeconds(dwInStreamNum) == pOutAvi->GetWaveBytesPerSeconds(dwOutStreamNum)))
	{
		LPBYTE pBuf = new BYTE[GetSuggestedAudioBufferSize(dwInStreamNum)];
		DWORD dwWrittenBytes; 
		DWORD dwWrittenSamples;
		bool bFirst = true;
		while (GetAudioSamples(	dwInStreamNum, GetSuggestedAudioBufferSize(dwInStreamNum) / GetWaveSampleSize(dwInStreamNum), pBuf,
								GetSuggestedAudioBufferSize(dwInStreamNum), &dwWrittenBytes, &dwWrittenSamples))
		{
			nPercentDone = Round((double)m_dwReadNextSample[dwInStreamNum] * 100.0 / (double)m_dwTotalSamples[dwInStreamNum]);
			if (nPercentDone > nPrevPercentDone)
				::PostMessage(m_pParentWnd->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeAUDIO, (LPARAM)nPercentDone);
			nPrevPercentDone = nPercentDone;
			if (bFirst)
			{
				bFirst = false;
				if (!pOutAvi->InitAudioWrite(dwOutStreamNum, GetAudioStreamName(dwInStreamNum)))
				{
					delete [] pBuf;
					return false;
				}
			}
			if (!pOutAvi->AddAudioSamples(dwOutStreamNum, dwWrittenSamples, pBuf))
			{
				delete [] pBuf;
				return false;
			}
			if (DoExit())
			{
				delete [] pBuf;
				return false;
			}
		}
		delete [] pBuf;
		return true;
	}
	else
	{
		// Check Whether Coding Splitting Is Necessary
		if (((GetWaveFormatTag(dwInStreamNum) != WAVE_FORMAT_PCM) && (pOutAvi->GetWaveFormatTag(dwOutStreamNum) != WAVE_FORMAT_PCM)) || // If Both Not PCM -> Split
			
			(((GetWaveFormatTag(dwInStreamNum) != WAVE_FORMAT_PCM) || (pOutAvi->GetWaveFormatTag(dwOutStreamNum) != WAVE_FORMAT_PCM)) && // If One of Both is PCM, but Sample Rate is Different -> Split
			(GetWaveSampleRate(dwInStreamNum) != pOutAvi->GetWaveSampleRate(dwOutStreamNum)) ||
			(GetWaveBits(dwInStreamNum) != pOutAvi->GetWaveBits(dwOutStreamNum)) ||
			(GetWaveNumOfChannels(dwInStreamNum) != pOutAvi->GetWaveNumOfChannels(dwOutStreamNum)))
			
			&& !bForceOnePass) // If 2 Or More Passes Allowed
		{
			// Create Avi Temp File Name
			CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), pOutAvi->GetFileName());

			 // If Both Not PCM
			if ((GetWaveFormatTag(dwInStreamNum) != WAVE_FORMAT_PCM) && (pOutAvi->GetWaveFormatTag(dwOutStreamNum) != WAVE_FORMAT_PCM))
			{
				WAVEFORMATEX UncompressedWaveFormat;
				::ZeroMemory(&UncompressedWaveFormat, sizeof(WAVEFORMATEX));
				UncompressedWaveFormat.wFormatTag = WAVE_FORMAT_PCM;
				res = ::acmFormatSuggest(NULL, pOutAvi->m_pWaveFormat[dwOutStreamNum], &UncompressedWaveFormat, sizeof(WAVEFORMATEX), ACM_FORMATSUGGESTF_WFORMATTAG);
				if (res != MMSYSERR_NOERROR)
				{
				   TRACE(_T("CAVIFile Cannot Choose The Right PCM Format!\n"));
				   return false;
				}

				// Output Avi File
				CAVIFile* pTempAvi = new CAVIFile(	m_pParentWnd,
													sTempFileName,
													m_dwRate[dwInStreamNum],
													m_dwScale[dwInStreamNum],
													&UncompressedWaveFormat,
													true,
													true,
													m_bShowMessageBoxOnError);
				if (AudioCodec(dwInStreamNum, 0, pTempAvi)) // May Have again 2-passes
				{
					delete pTempAvi;
					pTempAvi = new CAVIFile(m_pParentWnd, sTempFileName, m_bShowMessageBoxOnError);
					if (pTempAvi->AudioCodec(0, dwOutStreamNum, pOutAvi, true)) // 1-pass
					{
						delete pTempAvi;
						::DeleteFile(sTempFileName);
						return true;
					}
					else
					{
						delete pTempAvi;
						::DeleteFile(sTempFileName);
						return false;
					}
				}
				else
				{
					delete pTempAvi;
					::DeleteFile(sTempFileName);
					return false;
				}
			}
			else // If One of Both is PCM, but Sample Rate or Bits or Channels Are Different
			{
				WAVEFORMATEX UncompressedWaveFormat;
				::ZeroMemory(&UncompressedWaveFormat, sizeof(WAVEFORMATEX));
				UncompressedWaveFormat.wFormatTag = WAVE_FORMAT_PCM;
				
				if (GetWaveFormatTag(dwInStreamNum) != WAVE_FORMAT_PCM) // If Source Is Not PCM
				{
					res = ::acmFormatSuggest(NULL, m_pWaveFormat[dwInStreamNum], &UncompressedWaveFormat, sizeof(WAVEFORMATEX), ACM_FORMATSUGGESTF_WFORMATTAG);
					if (res != MMSYSERR_NOERROR)
					{
					   TRACE(_T("CAVIFile Cannot Choose The Right PCM Format!\n"));
					   return false;
					}
			
					// Output Avi File
					CAVIFile* pTempAvi = new CAVIFile(	m_pParentWnd,
														sTempFileName,
														m_dwRate[dwInStreamNum],
														m_dwScale[dwInStreamNum],
														&UncompressedWaveFormat,
														true,
														true,
														m_bShowMessageBoxOnError);
					if (AudioCodec(dwInStreamNum, 0, pTempAvi, true)) // 1-pass
					{
						delete pTempAvi;
						pTempAvi = new CAVIFile(m_pParentWnd, sTempFileName, m_bShowMessageBoxOnError);
						if (pTempAvi->AudioCodec(0, dwOutStreamNum, pOutAvi, true)) // 1-pass
						{
							delete pTempAvi;
							::DeleteFile(sTempFileName);
							return true;
						}
						else
						{
							delete pTempAvi;
							::DeleteFile(sTempFileName);
							return false;
						}
					}
					else
					{
						delete pTempAvi;
						::DeleteFile(sTempFileName);
						return false;
					}
				}
				else // If Destination Is Not PCM
				{
					res = ::acmFormatSuggest(NULL, pOutAvi->m_pWaveFormat[dwOutStreamNum], &UncompressedWaveFormat, sizeof(WAVEFORMATEX), ACM_FORMATSUGGESTF_WFORMATTAG);
					if (res != MMSYSERR_NOERROR)
					{
					   TRACE(_T("CAVIFile Cannot Choose The Right PCM Format!\n"));
					   return false;
					}
			
					// Output Avi File
					CAVIFile* pTempAvi = new CAVIFile(	m_pParentWnd,
														sTempFileName,
														m_dwRate[dwInStreamNum],
														m_dwScale[dwInStreamNum],
														&UncompressedWaveFormat,
														true,
														true,
														m_bShowMessageBoxOnError);
					if (AudioCodec(dwInStreamNum, 0, pTempAvi, true)) // 1-pass
					{
						delete pTempAvi;
						pTempAvi = new CAVIFile(m_pParentWnd, sTempFileName, m_bShowMessageBoxOnError);
						if (pTempAvi->AudioCodec(0, dwOutStreamNum, pOutAvi, true)) // 1-pass
						{
							delete pTempAvi;
							::DeleteFile(sTempFileName);
							return true;
						}
						else
						{
							delete pTempAvi;
							::DeleteFile(sTempFileName);
							return false;
						}
					}
					else
					{
						delete pTempAvi;
						::DeleteFile(sTempFileName);
						return false;
					}
				}
			}
		}

		HACMSTREAM hAcmStream;
		res = ::acmStreamOpen(	&hAcmStream,							// Stream Handle
								NULL,									// Driver
								GetWaveFormat(dwInStreamNum),			// Source (Input)
								pOutAvi->GetWaveFormat(dwOutStreamNum),	// Destination (Output)
								NULL,									// Filter
								NULL,									// CallBack
								0,										// User Data
								ACM_STREAMOPENF_NONREALTIME);			// Flags
		if (res != MMSYSERR_NOERROR)
		{
			TRACE(_T("CAVIFile Cannot Open Compression Stream!\n"));
			::acmStreamClose(hAcmStream, 0); 
			return false;
		}

		// Output Buffer
		DWORD dwOutBufSize;
		res = ::acmStreamSize(hAcmStream, GetSuggestedAudioBufferSize(dwInStreamNum), &dwOutBufSize, ACM_STREAMSIZEF_SOURCE);
		if (res != MMSYSERR_NOERROR)
		{
			TRACE(_T("CAVIFile Cannot Get Stream Buffer Size!\n"));
			::acmStreamClose(hAcmStream, 0);
			return false;
		}
		// Do Not Trust acmStreamSize, make it the double
		dwOutBufSize *= 2;
		LPBYTE pOutBuf = new BYTE [dwOutBufSize];

		// Input Buffer
		LPBYTE pInBuf = new BYTE [GetSuggestedAudioBufferSize(dwInStreamNum)];

		// Init Stream Header
		ACMSTREAMHEADER AcmStreamHeader;
		::ZeroMemory(&AcmStreamHeader, sizeof(ACMSTREAMHEADER));
		AcmStreamHeader.cbStruct = sizeof(ACMSTREAMHEADER);
		AcmStreamHeader.pbSrc = pInBuf;
		AcmStreamHeader.pbDst = pOutBuf;
		AcmStreamHeader.cbSrcLength = GetSuggestedAudioBufferSize(dwInStreamNum);
		AcmStreamHeader.cbDstLength = dwOutBufSize;
		res1 = ::acmStreamPrepareHeader(hAcmStream, &AcmStreamHeader, 0);
		if (res1 != MMSYSERR_NOERROR)
		{
			TRACE(_T("CAVIFile Cannot Prepare Compression Stream Headers!\n"));
			delete [] pInBuf;
			delete [] pOutBuf;
			::acmStreamClose(hAcmStream, 0);
			return false;
		}

		// Do Compression / Decompression
		DWORD dwReadSize, dwReadSamples;
		bool bOk;
		bool bFirst = true;
		DWORD dwUnconvertedBytesCount = 0;
		RewAudio();
		do
		{
			bOk = GetAudioSamples(	dwInStreamNum, (GetSuggestedAudioBufferSize(dwInStreamNum) - dwUnconvertedBytesCount) / GetWaveSampleSize(dwInStreamNum),
									pInBuf + dwUnconvertedBytesCount, GetSuggestedAudioBufferSize(dwInStreamNum) - dwUnconvertedBytesCount,
									&dwReadSize, &dwReadSamples);

			nPercentDone = Round((double)m_dwReadNextSample[dwInStreamNum] * 100.0 / (double)m_dwTotalSamples[dwInStreamNum]);
			if (nPercentDone > nPrevPercentDone)
				::PostMessage(m_pParentWnd->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeAUDIO, (LPARAM)nPercentDone);
			nPrevPercentDone = nPercentDone;

			if (dwReadSize)
			{
				AcmStreamHeader.cbSrcLength = dwUnconvertedBytesCount + dwReadSize;
				res2 = ::acmStreamConvert(hAcmStream, &AcmStreamHeader, (bFirst ? ACM_STREAMCONVERTF_START : 0) | ACM_STREAMCONVERTF_BLOCKALIGN);
				ASSERT(AcmStreamHeader.fdwStatus & ACMSTREAMHEADER_STATUSF_DONE);
				if (res2 == MMSYSERR_NOERROR)
				{
					ASSERT(AcmStreamHeader.cbSrcLengthUsed <= (dwUnconvertedBytesCount + dwReadSize));
					ASSERT(AcmStreamHeader.cbDstLengthUsed <= AcmStreamHeader.cbDstLength);
					if (AcmStreamHeader.cbSrcLengthUsed < (dwUnconvertedBytesCount + dwReadSize))
					{
						dwUnconvertedBytesCount = dwUnconvertedBytesCount + dwReadSize - AcmStreamHeader.cbSrcLengthUsed;
						memmove(pInBuf, pInBuf + AcmStreamHeader.cbSrcLengthUsed, dwUnconvertedBytesCount);
					}
					else
						dwUnconvertedBytesCount = 0;
					if (bFirst)
					{
						if (!pOutAvi->InitAudioWrite(dwOutStreamNum, GetAudioStreamName(dwInStreamNum)))
						{
							delete [] pInBuf;
							delete [] pOutBuf;
							::acmStreamClose(hAcmStream, 0);
							return false;
						}
					}
					if (!pOutAvi->AddAudioSamples(	dwOutStreamNum,
													AcmStreamHeader.cbDstLengthUsed /
													pOutAvi->GetWaveSampleSize(dwOutStreamNum),
													AcmStreamHeader.pbDst))
					{
						delete [] pInBuf;
						delete [] pOutBuf;
						::acmStreamClose(hAcmStream, 0);
						return false;
					}
					bFirst = false;
				}
			}
			else
			{
				if (dwUnconvertedBytesCount > 0)
				{
					AcmStreamHeader.cbSrcLength = dwUnconvertedBytesCount;
					res2 = ::acmStreamConvert(hAcmStream, &AcmStreamHeader, ACM_STREAMCONVERTF_END);
					if (res2 == MMSYSERR_NOERROR)
					{
						ASSERT(AcmStreamHeader.cbSrcLengthUsed <= dwUnconvertedBytesCount);
						ASSERT(AcmStreamHeader.cbDstLengthUsed <= AcmStreamHeader.cbDstLength);
						if (!pOutAvi->AddAudioSamples(	dwOutStreamNum,
														AcmStreamHeader.cbDstLengthUsed /
														pOutAvi->GetWaveSampleSize(dwOutStreamNum),
														AcmStreamHeader.pbDst))
						{
							delete [] pInBuf;
							delete [] pOutBuf;
							::acmStreamClose(hAcmStream, 0);
							return false;
						}
					}
				}
				else
					res2 = MMSYSERR_NOERROR;
			}
			if (DoExit())
			{
				delete [] pInBuf;
				delete [] pOutBuf;
				::acmStreamClose(hAcmStream, 0);
				return false;
			}
		}
		while (bOk && (res2 == MMSYSERR_NOERROR));

		// Release Stream Header
		AcmStreamHeader.pbSrc = pInBuf;
		AcmStreamHeader.pbDst = pOutBuf;
		AcmStreamHeader.cbDstLength = dwOutBufSize;
		AcmStreamHeader.cbSrcLength = GetSuggestedAudioBufferSize(dwInStreamNum);
		res3 = ::acmStreamUnprepareHeader(hAcmStream, &AcmStreamHeader, 0);

		// Clean-Up
		delete [] pInBuf;
		delete [] pOutBuf;

		// Close Stream
		::acmStreamClose(hAcmStream, 0);

		if ((res2 != MMSYSERR_NOERROR) || (res3 != MMSYSERR_NOERROR))
		{
			if (res2 != MMSYSERR_NOERROR)
				TRACE(_T("CAVIFile Cannot Uncompress Stream!\n"));
			else if (res3 != MMSYSERR_NOERROR)
				TRACE(_T("CAVIFile Cannot Unprepare Compression Stream Headers!\n"));
			return false;
		}
		else
			return true;
	}
}

DWORD CAVIFile::AutoChooseVideoCompressor()
{
	CAVIFile AviFile; // Empty Avi File for Compression Testing

	// Test whether we can set a compressor
	if (AviFile.SetVideoCompressor(0, _T("XVID"))) return CAVIFile::StringToFourCC(_T("XVID"));
	else if (AviFile.SetVideoCompressor(0, _T("DIVX"))) return StringToFourCC(_T("DIVX"));
	else if (AviFile.SetVideoCompressor(0, _T("3IV2"))) return StringToFourCC(_T("3IV2"));
	else if (AviFile.SetVideoCompressor(0, _T("CVID"))) return StringToFourCC(_T("CVID"));
	else if (AviFile.SetVideoCompressor(0, _T("MSVC"))) return StringToFourCC(_T("MSVC"));
	else return 0;
}

DWORD CAVIFile::ChooseVideoCompressorForSave(DWORD dwFourCC)
{
	CAVIFile AviFile(NULL); // Empty Avi File for Compression Testing
	CString sFourCC = FourCCToString(dwFourCC);
	sFourCC.MakeUpper();

	// Choose a divx compatible compressor
	if ((sFourCC == _T("DIVX")) ||	// This FOURCC code is used for versions 4.0 and later of the DivX codec.
									// DivX, "the MP3 of video,"  is the popular and market-leading MPEG-4
									// video codec that is emerging as the standard for full screen, full motion,
									// DVD-quality video over IP-based networks.
		(sFourCC == _T("XVID")) ||	// The Open Source MPEG-4
		(sFourCC == _T("DX50")) ||	// Apparently this is used interchangeably with the DIVX FOURCC
									// when using version 5 of the codec.
		(sFourCC == _T("DIV3")) ||	// Low motion codec (optimised for low motion source material?).
									// Several sources tell me that this is an old and illegal codec that should not be used to encode new material.
		(sFourCC == _T("DIV4")) ||	// Fast motion codec.Several sources tell me that this is an old and illegal codec
									// that should not be used to encode new material.
		(sFourCC == _T("DIV5")) ||	// Apparently almost as old as DIV3 and DIV4. Changing DIV5 AVI's FOURCC
									// to DIV3 or DIV4 seems to allow them to play just fine.
		(sFourCC == _T("DM4V")) ||	// MPEG4 codec compatible with DivX4 and 5. (Dicas)
		(sFourCC == _T("MP4V")) ||	// MPEG-4 Video (Media Excel)
		(sFourCC == _T("MP43")) ||	// Yet another MPEG-4 variation from Microsoft. This FOURCC is not, however,
									// listed on Microsoft's codecs site. This codec is distributed as part of
									// Microsoft Windows Media Tools 4. Includes further quality improvements over the earlier MPG4.
		(sFourCC == _T("MP42")) ||	// Apparently one of several different and incompatible MPEG-4 codecs.
									// Rumour has it that this codec is downloadable from the Microsoft site somewhere.
									// This codec is distributed as part of Microsoft Windows Media Tools 4.
									// Includes quality improvements over the earlier MPG4. 
									// Download as part of "Windows Media Codecs 8.0 for IT Professionals."
		(sFourCC == _T("MP45")))	// The first ISO standard codec for use with the Sharp digital camera
									// implementing a restricted feature set of MPEG4.

	{
		// Test whether we can set a compressor
		if (AviFile.SetVideoCompressor(0, _T("XVID")))
			return StringToFourCC(_T("XVID"));
		else if (AviFile.SetVideoCompressor(0, _T("DIVX")))
			return StringToFourCC(_T("DIVX"));
		else
			return dwFourCC;
	}
	else if (sFourCC == _T("CRAM"))
	{
		if (AviFile.SetVideoCompressor(0, _T("MSVC")))
			return StringToFourCC(_T("MSVC"));
	}

	return dwFourCC;
}

CString CAVIFile::GetVideoCompressorName(DWORD dwStreamNum) const
{
	HIC hic;
	ICINFO icinfo;

	for (int i = 0 ; ::ICInfo(ICTYPE_VIDEO, i, &icinfo) ; i++)
	{
		hic = ::ICOpen(icinfo.fccType, icinfo.fccHandler, ICMODE_QUERY);

		if (hic)
		{
			if (FourCCMakeLowerCase(m_VideoCompressorOptions[dwStreamNum].fccHandler) == FourCCMakeLowerCase(icinfo.fccHandler))
			{
				// Find out the compressor name
				::ICGetInfo(hic, &icinfo, sizeof(icinfo));
				CString sName = CString(icinfo.szName);	
				::ICClose(hic);
				return sName;
			}

			::ICClose(hic);
		}
	}

	return _T(""); // uncompressed
}

CString CAVIFile::GetVideoCompressorDriverName(DWORD dwStreamNum) const
{
	HIC hic;
	ICINFO icinfo;

	for (int i = 0 ; ::ICInfo(ICTYPE_VIDEO, i, &icinfo) ; i++)
	{
		hic = ::ICOpen(icinfo.fccType, icinfo.fccHandler, ICMODE_QUERY);

		if (hic)
		{
			if (FourCCMakeLowerCase(m_VideoCompressorOptions[dwStreamNum].fccHandler) == FourCCMakeLowerCase(icinfo.fccHandler))
			{
				// Find out the compressor name
				::ICGetInfo(hic, &icinfo, sizeof(icinfo));
				CString sDriver = CString(icinfo.szDriver);	
				::ICClose(hic);
				return sDriver;
			}

			::ICClose(hic);
		}
	}

	return _T(""); // uncompressed
}

CString CAVIFile::GetVideoCompressorDescription(DWORD dwStreamNum) const
{
	HIC hic;
	ICINFO icinfo;

	for (int i = 0 ; ::ICInfo(ICTYPE_VIDEO, i, &icinfo) ; i++)
	{
		hic = ::ICOpen(icinfo.fccType, icinfo.fccHandler, ICMODE_QUERY);

		if (hic)
		{
			if (FourCCMakeLowerCase(m_VideoCompressorOptions[dwStreamNum].fccHandler) == FourCCMakeLowerCase(icinfo.fccHandler))
			{
				// Find out the compressor name
				::ICGetInfo(hic, &icinfo, sizeof(icinfo));
				CString sDescription = CString(icinfo.szDescription);	
				::ICClose(hic);
				return sDescription;
			}

			::ICClose(hic);
		}
	}

	return _T(""); // uncompressed
}

bool CAVIFile::SetVideoCompressor(DWORD dwStreamNum, CString sFourCC)
{
	sFourCC.MakeLower();
	DWORD dwFourCC = StringToFourCC(sFourCC);
	dwFourCC = FourCCMakeLowerCase(dwFourCC);
	ICINFO icinfo;
	HIC hIC;
	for (int i = 0 ; ICInfo(ICTYPE_VIDEO, i, &icinfo) ; i++)
	{
		if (FourCCMakeLowerCase(icinfo.fccHandler) == dwFourCC)
		{
			// Check It!
			if ((hIC = ::ICOpen(icinfo.fccType, icinfo.fccHandler, ICMODE_QUERY)) == NULL)
				return false;
			::ICClose(hIC);

			if (m_VideoCompressorOptions[dwStreamNum].lpParms)
				delete [] (m_VideoCompressorOptions[dwStreamNum].lpParms);
			memset(&m_VideoCompressorOptions[dwStreamNum], 0, sizeof(AVICOMPRESSOPTIONSVOLATILE));
			m_VideoCompressorOptions[dwStreamNum].fccType = streamtypeVIDEO;
			m_dwFourCC[dwStreamNum] = m_VideoCompressorOptions[dwStreamNum].fccHandler = dwFourCC;
			return true;
		}
	}

	return false;
}

bool CAVIFile::SetVideoCompressor(DWORD dwStreamNum, DWORD dwFourCC)
{
	ICINFO icinfo;
	HIC hIC;
	dwFourCC = FourCCMakeLowerCase(dwFourCC);
	for (int i = 0 ; ICInfo(ICTYPE_VIDEO, i, &icinfo) ; i++)
	{
		if (FourCCMakeLowerCase(icinfo.fccHandler) == dwFourCC)
		{
			// Check It!
			if ((hIC = ::ICOpen(icinfo.fccType, icinfo.fccHandler, ICMODE_QUERY)) == NULL)
				return false;
			::ICClose(hIC);

			if (m_VideoCompressorOptions[dwStreamNum].lpParms)
				delete [] (m_VideoCompressorOptions[dwStreamNum].lpParms);
			memset(&m_VideoCompressorOptions[dwStreamNum], 0, sizeof(AVICOMPRESSOPTIONSVOLATILE));
			m_VideoCompressorOptions[dwStreamNum].fccType = streamtypeVIDEO;
			m_dwFourCC[dwStreamNum] = m_VideoCompressorOptions[dwStreamNum].fccHandler = dwFourCC;
			return true;
		}
	}

	return false;
}

DWORD CAVIFile::StringToFourCC(CString sFourCC)
{
#ifdef _UNICODE
	char FourCC[5];
	wcstombs(FourCC, sFourCC, 5);
	return ((DWORD)(BYTE)(FourCC[0]) |
			((DWORD)(BYTE)(FourCC[1]) << 8) |
			((DWORD)(BYTE)(FourCC[2]) << 16) |
			((DWORD)(BYTE)(FourCC[3]) << 24 ));
#else	
    return ((DWORD)(BYTE)(sFourCC[0]) |
			((DWORD)(BYTE)(sFourCC[1]) << 8) |
			((DWORD)(BYTE)(sFourCC[2]) << 16) |
			((DWORD)(BYTE)(sFourCC[3]) << 24 ));
#endif
}

CString CAVIFile::FourCCToString(DWORD dwFourCC)
{
	char ch0 = (char)(dwFourCC & 0xFF);
	char ch1 = (char)((dwFourCC >> 8) & 0xFF);
	char ch2 = (char)((dwFourCC >> 16) & 0xFF);
	char ch3 = (char)((dwFourCC >> 24) & 0xFF);
#ifdef _UNICODE
	WCHAR wch0, wch1, wch2, wch3;
	mbtowc(&wch0, &ch0, sizeof(WCHAR));
	mbtowc(&wch1, &ch1, sizeof(WCHAR));
	mbtowc(&wch2, &ch2, sizeof(WCHAR));
	mbtowc(&wch3, &ch3, sizeof(WCHAR));
	return (CString(wch0) + CString(wch1) + CString(wch2) + CString(wch3));
#else
	return (CString(ch0) + CString(ch1) + CString(ch2) + CString(ch3));
#endif
}

CString CAVIFile::FourCCToStringLowerCase(DWORD dwFourCC)
{
	CString sFourCC = FourCCToString(dwFourCC);
	sFourCC.MakeLower();
	return sFourCC;
}

CString CAVIFile::FourCCToStringUpperCase(DWORD dwFourCC)
{
	CString sFourCC = FourCCToString(dwFourCC);
	sFourCC.MakeUpper();
	return sFourCC;
}

DWORD CAVIFile::FourCCMakeLowerCase(DWORD dwFourCC)
{
	return StringToFourCC(FourCCToStringLowerCase(dwFourCC));
}

DWORD CAVIFile::FourCCMakeUpperCase(DWORD dwFourCC)
{
	return StringToFourCC(FourCCToStringUpperCase(dwFourCC));
}

bool CAVIFile::IsAudioReadable(DWORD dwStreamNum)
{
	bool res1, res2, res3;

	if (!m_bOk)
		return false;

	// Check if it is the first read sample
	if (!m_bInitRead)
		if (!InitRead())
			return false;

	if (!m_pAudioStream[dwStreamNum])
		return false;

	LPBYTE pBuf = new BYTE[GetWaveSampleSize(dwStreamNum)];

	::EnterCriticalSection(&m_csAVI);

	DWORD dwNextSample = m_dwReadNextSample[dwStreamNum];
	DWORD dwWrittenBytes = 0;
	DWORD dwWrittenSamples = 0;
	// Start
	res1 = GetAudioSamplesAt(dwStreamNum, 0, 1, pBuf, GetWaveSampleSize(dwStreamNum), &dwWrittenBytes, &dwWrittenSamples);
	if ((dwWrittenBytes != GetWaveSampleSize(dwStreamNum)) || (dwWrittenSamples != 1))
		res1 = false;
	// Read End
	res2 = GetAudioSamplesAt(dwStreamNum, m_dwTotalSamples[dwStreamNum] - 1, 1, pBuf, GetWaveSampleSize(dwStreamNum), &dwWrittenBytes, &dwWrittenSamples);
	if ((dwWrittenBytes != GetWaveSampleSize(dwStreamNum)) || (dwWrittenSamples != 1))
		res2 = false;
	// Read Middle
	res3 = GetAudioSamplesAt(dwStreamNum, m_dwTotalSamples[dwStreamNum] / 2 - 1, 1, pBuf, GetWaveSampleSize(dwStreamNum), &dwWrittenBytes, &dwWrittenSamples);
	if ((dwWrittenBytes != GetWaveSampleSize(dwStreamNum)) || (dwWrittenSamples != 1))
		res3 = false;
	m_dwReadNextSample[dwStreamNum] = dwNextSample;

	::LeaveCriticalSection(&m_csAVI);
	
	delete [] pBuf;

	return (res1 && res2 && res3);
}

bool CAVIFile::GetAudioSamples(	DWORD dwStreamNum,
								DWORD dwNumSamples,
								LPBYTE pBuf,
								DWORD dwBufSize,
								DWORD* pWrittenBytes,
								DWORD* pWrittenSamples)
{
	HRESULT hr;

	if (!m_bOk)
		return false;

	// Check if it is the first read sample
	if (!m_bInitRead)
		if (!InitRead())
			return false;

	if (!m_pAudioStream[dwStreamNum])
		return false;

	::EnterCriticalSection(&m_csAVI);

	// Read Audio Stream
	DWORD dwSamples = 0;
	DWORD dwSize = 0;
	if (pWrittenBytes == NULL)
		pWrittenBytes = &dwSize;
	if (pWrittenSamples == NULL)
		pWrittenSamples = &dwSamples;

	// Strange Bug:
	// Sometimes Reading in a Location Returns No Error, But No Bytes Are Read
	// Read in Another Location and If Successfull Read Again at Start Location
	unsigned int i = 0;
	do
	{
		hr = ::AVIStreamRead(m_pAudioStream[dwStreamNum], m_dwReadNextSample[dwStreamNum] + i, dwNumSamples,
							(LPVOID)pBuf, dwBufSize, (LONG*)pWrittenBytes, (LONG*)pWrittenSamples);
		i++;
	}
	while ((hr == AVIERR_OK) && ((*pWrittenBytes) == 0) && (i < 1024));

	if ((hr == AVIERR_OK) && ((*pWrittenBytes) != 0) && (i > 1))
		hr = ::AVIStreamRead(m_pAudioStream[dwStreamNum], m_dwReadNextSample[dwStreamNum], dwNumSamples,
							(LPVOID)pBuf, dwBufSize, (LONG*)pWrittenBytes, (LONG*)pWrittenSamples);

	// Other Strange Behaviour:
	// Sometimes We Need To Read A Chunk Size Or Less
	if ((hr != AVIERR_OK) && (m_dwReadNextSample < m_dwTotalSamples))
	{
		hr = ::AVIStreamRead(m_pAudioStream[dwStreamNum], m_dwReadNextSample[dwStreamNum], AVISTREAMREAD_CONVENIENT,
							(LPVOID)pBuf, dwBufSize, (LONG*)pWrittenBytes, (LONG*)pWrittenSamples);

		if (hr != AVIERR_OK)
		{
			for (i = 4096 ; i >= 1 ; i /= 2)
			{
				if (dwNumSamples >= i)
				{
					hr = ::AVIStreamRead(m_pAudioStream[dwStreamNum], m_dwReadNextSample[dwStreamNum], i,
							(LPVOID)pBuf, dwBufSize, (LONG*)pWrittenBytes, (LONG*)pWrittenSamples);
					if (hr == AVIERR_OK)
						break;
				}
			}
		}
	}

	if (hr != AVIERR_OK)
	{
		::LeaveCriticalSection(&m_csAVI);
		return false;
	}
	else
	{
		m_dwReadNextSample[dwStreamNum] += *pWrittenSamples;
		::LeaveCriticalSection(&m_csAVI);
		return true;
	}
}

bool CAVIFile::GetAudioSamplesAt(DWORD dwStreamNum,
								 DWORD dwStartSample,
								 DWORD dwNumSamples,
								 LPBYTE pBuf,
								 DWORD dwBufSize,
								 DWORD* pWrittenBytes,
								 DWORD* pWrittenSamples)
{
	HRESULT hr;

	if (!m_bOk)
		return false;

	// Check if it is the first read sample
	if (!m_bInitRead)
		if (!InitRead())
			return false;

	if (!m_pAudioStream[dwStreamNum]) 
		return false;

	::EnterCriticalSection(&m_csAVI);

	// Read Audio Stream
	DWORD dwSamples = 0;
	DWORD dwSize = 0;
	if (pWrittenBytes == NULL)
		pWrittenBytes = &dwSize;
	if (pWrittenSamples == NULL)
		pWrittenSamples = &dwSamples;

	// Strange Bug:
	// Sometimes Reading in a Location Returns No Error, But No Bytes Are Read
	// Read in Another Location and If Successfull Read Again at Start Location
	unsigned int i = 0;
	do
	{
		hr = ::AVIStreamRead(m_pAudioStream[dwStreamNum], dwStartSample + i, dwNumSamples,
							(LPVOID)pBuf, dwBufSize, (LONG*)pWrittenBytes, (LONG*)pWrittenSamples);
		i++;
	}
	while ((hr == AVIERR_OK) && ((*pWrittenBytes) == 0) && (i < 1024));

	if ((hr == AVIERR_OK) && ((*pWrittenBytes) != 0) && (i > 1))
		hr = ::AVIStreamRead(m_pAudioStream[dwStreamNum], dwStartSample, dwNumSamples,
							(LPVOID)pBuf, dwBufSize, (LONG*)pWrittenBytes, (LONG*)pWrittenSamples);

	// Other Strange Behaviour:
	// Sometimes We Need To Read A Chunk Size Or Less
	if ((hr != AVIERR_OK) && (m_dwReadNextSample < m_dwTotalSamples))
	{
		hr = ::AVIStreamRead(m_pAudioStream[dwStreamNum], dwStartSample, AVISTREAMREAD_CONVENIENT,
							(LPVOID)pBuf, dwBufSize, (LONG*)pWrittenBytes, (LONG*)pWrittenSamples);

		if (hr != AVIERR_OK)
		{
			for (i = 4096 ; i >= 1 ; i /= 2)
			{
				if (dwNumSamples >= i)
				{
					hr = ::AVIStreamRead(m_pAudioStream[dwStreamNum], dwStartSample, i,
							(LPVOID)pBuf, dwBufSize, (LONG*)pWrittenBytes, (LONG*)pWrittenSamples);
					if (hr == AVIERR_OK)
						break;
				}
			}
		}
	}

	if (hr != AVIERR_OK)
	{
		::LeaveCriticalSection(&m_csAVI);
		return false;
	}
	else
	{
		m_dwReadNextSample[dwStreamNum] = dwStartSample + *pWrittenSamples; 
		::LeaveCriticalSection(&m_csAVI);
		return true;
	}
}

bool CAVIFile::SetReadCurrentSamplePos(DWORD dwStreamNum, int nCurrentSamplePos)
{
	if ((nCurrentSamplePos < -1) || (nCurrentSamplePos >= (int)m_dwTotalSamples[dwStreamNum]))
		return false;
	else
	{
		m_dwReadNextSample[dwStreamNum] = nCurrentSamplePos + 1;
		return true;
	}
}

bool CAVIFile::SetReadNextSamplePos(DWORD dwStreamNum, DWORD dwNextSamplePos)
{
	if (dwNextSamplePos > m_dwTotalSamples[dwStreamNum])
		return false;
	else
	{
		m_dwReadNextSample[dwStreamNum] = dwNextSamplePos;
		return true;
	}
}

bool CAVIFile::SetWriteCurrentSamplePos(DWORD dwStreamNum, int nCurrentSamplePos)
{
	if ((nCurrentSamplePos < -1) || (nCurrentSamplePos >= (int)m_dwTotalSamples[dwStreamNum]))
		return false;
	else
	{
		m_dwWriteNextSample[dwStreamNum] = nCurrentSamplePos + 1;
		return true;
	}
}

bool CAVIFile::SetWriteNextSamplePos(DWORD dwStreamNum, DWORD dwNextSamplePos)
{
	if (dwNextSamplePos > m_dwTotalSamples[dwStreamNum])
		return false;
	else
	{
		m_dwWriteNextSample[dwStreamNum] = dwNextSamplePos;
		return true;
	}
}

bool CAVIFile::AddAudioSamples(DWORD dwStreamNum, DWORD dwNumSamples, LPBYTE pBuf)
{
	HRESULT hr;
	LONG lBytesWritten;

	if (!m_bOk)
		return false;

	// Check if it is the first read sample
	if (m_pAudioStream[dwStreamNum] == NULL)
		if (!InitAudioWrite(dwStreamNum))
			return false;

	if (!m_pAudioStream)
		return false;

	::EnterCriticalSection(&m_csAVI);

	// Write Audio Stream
	hr = ::AVIStreamWrite(m_pAudioStream[dwStreamNum],						// Stream pointer
							m_dwWriteNextSample[dwStreamNum],				// Next Sample Position
							dwNumSamples,									// Number of samples to write
							pBuf,											// Pointer to data
							dwNumSamples * m_pWaveFormat[dwStreamNum]->nBlockAlign,// Number of bytes to write
							0,												// No flags....
							NULL,
							&lBytesWritten);
	if (hr != AVIERR_OK)
	{
		m_bOk = false;
		::LeaveCriticalSection(&m_csAVI);
		return false;
	}
	else
	{
		m_dwWriteNextSample[dwStreamNum] += dwNumSamples;
		m_dwTotalSamples[dwStreamNum] = m_dwWriteNextSample[dwStreamNum];
		m_dwTotalWrittenAudioBytes[dwStreamNum] += lBytesWritten;
		::LeaveCriticalSection(&m_csAVI);
		return true;
	}
}

void CAVIFile::SetWaveFormat(DWORD dwStreamNum, const LPWAVEFORMATEX pWaveFormat)					
{
	if (pWaveFormat == NULL)
		return;

	if (m_pWaveFormat[dwStreamNum])
		delete [] m_pWaveFormat[dwStreamNum];

	int nWaveFormatSize;
	if (pWaveFormat->wFormatTag == WAVE_FORMAT_PCM)
		nWaveFormatSize = sizeof(WAVEFORMATEX);
	else
		nWaveFormatSize = sizeof(WAVEFORMATEX) + pWaveFormat->cbSize;
 
	m_pWaveFormat[dwStreamNum] = (LPWAVEFORMATEX)new BYTE[nWaveFormatSize];
	memcpy(m_pWaveFormat[dwStreamNum], pWaveFormat, nWaveFormatSize);
}

void CAVIFile::PlaySyncAudioFromVideo(DWORD dwAudioStreamNum, DWORD dwVideoStreamNum, CAVIFile* pAVIVideo/*=NULL*/)
{
	// Do Not Use AVIStreamTimeToSample, it's Buggy!
	if (pAVIVideo)
	{
		if (!pAVIVideo->HasVideo() || !HasAudio()) return;
		double dCurrentTimePos = 1000.0 / pAVIVideo->GetFrameRate(dwVideoStreamNum) * pAVIVideo->GetReadNextFramePos(dwVideoStreamNum);
		m_dwReadNextSample[dwAudioStreamNum] = (DWORD)Round(dCurrentTimePos * GetWaveSampleRate(dwAudioStreamNum) / 1000.0);
		if (m_dwReadNextSample[dwAudioStreamNum] > m_dwTotalSamples[dwAudioStreamNum])
			m_dwReadNextSample[dwAudioStreamNum] = m_dwTotalSamples[dwAudioStreamNum];
	}
	else
	{
		if (!HasVideo() || !HasAudio()) return;
		double dCurrentTimePos = 1000.0 / GetFrameRate(dwVideoStreamNum) * m_dwReadNextFrame[dwVideoStreamNum];
		m_dwReadNextSample[dwAudioStreamNum] = (DWORD)Round(dCurrentTimePos * GetWaveSampleRate(dwAudioStreamNum) / 1000.0);
		if (m_dwReadNextSample[dwAudioStreamNum] > m_dwTotalSamples[dwAudioStreamNum])
			m_dwReadNextSample[dwAudioStreamNum] = m_dwTotalSamples[dwAudioStreamNum];

	}
}

void CAVIFile::PlaySyncVideoFromAudio(DWORD dwVideoStreamNum, DWORD dwAudioStreamNum, CAVIFile* pAVIAudio/*=NULL*/)
{
	// Do Not Use AVIStreamTimeToSample, it's Buggy!
	if (pAVIAudio)
	{
		if (!HasVideo() || !pAVIAudio->HasAudio()) return;
		double dCurrentTimePos = 1000.0 / pAVIAudio->GetWaveSampleRate(dwAudioStreamNum) * pAVIAudio->GetReadNextSamplePos(dwAudioStreamNum);
		m_dwReadNextFrame[dwVideoStreamNum] = (DWORD)Round(dCurrentTimePos * GetFrameRate(dwVideoStreamNum) / 1000.0);
		if (m_dwReadNextFrame[dwVideoStreamNum] > m_dwTotalFrames[dwVideoStreamNum])
			m_dwReadNextFrame[dwVideoStreamNum] = m_dwTotalFrames[dwVideoStreamNum];
	}
	else
	{
		if (!HasVideo() || !HasAudio()) return;
		double dCurrentTimePos = 1000.0 / GetWaveSampleRate(dwAudioStreamNum) * m_dwReadNextSample[dwAudioStreamNum];
		m_dwReadNextFrame[dwVideoStreamNum] = (DWORD)Round(dCurrentTimePos * GetFrameRate(dwVideoStreamNum) / 1000.0);
		if (m_dwReadNextFrame[dwVideoStreamNum] > m_dwTotalFrames[dwVideoStreamNum])
			m_dwReadNextFrame[dwVideoStreamNum] = m_dwTotalFrames[dwVideoStreamNum];

	}

}

bool CAVIFile::AudioCopyFrom(DWORD dwSrcStreamNum, DWORD dwDstStreamNum, CAVIFile* pSourceAVIFile)
{
	DWORD dwSize, dwSamples;
	int nPercentDone;
	int nPrevPercentDone = -1;

	if (!m_bOk)
		return false;
	
	if (!pSourceAVIFile)
		return false;

	DWORD dwNextSample = pSourceAVIFile->m_dwReadNextSample[dwSrcStreamNum];

	if (!pSourceAVIFile->HasAudio())
		return false;

	BYTE* pBuf = new BYTE[pSourceAVIFile->GetSuggestedAudioBufferSize(dwSrcStreamNum)];
		
	pSourceAVIFile->RewAudio();
	while (	pSourceAVIFile->GetAudioSamples(dwSrcStreamNum, pSourceAVIFile->GetSuggestedAudioBufferSize(dwSrcStreamNum) / pSourceAVIFile->GetWaveSampleSize(dwSrcStreamNum),
			pBuf, pSourceAVIFile->GetSuggestedAudioBufferSize(dwSrcStreamNum), &dwSize, &dwSamples))
	{
		nPercentDone = Round(	(double)(pSourceAVIFile->GetReadNextSamplePos(dwSrcStreamNum)) * 100.0 /
								(double)(pSourceAVIFile->GetSampleCount(dwSrcStreamNum)));
		if (nPercentDone > nPrevPercentDone)
			::PostMessage(m_pParentWnd->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeAUDIO, (LPARAM)nPercentDone);
		nPrevPercentDone = nPercentDone;
		if (!AddAudioSamples(dwDstStreamNum, dwSize / pSourceAVIFile->GetWaveSampleSize(dwSrcStreamNum), pBuf))
		{
			delete [] pBuf;
			return false;
		}
		if (DoExit())
		{
			delete [] pBuf;
			return false;
		}
	}

	m_dwReadNextSample[dwDstStreamNum] = dwNextSample;
	pSourceAVIFile->m_dwReadNextSample[dwSrcStreamNum] = dwNextSample;

	delete [] pBuf;

	SetWaveFormat(dwDstStreamNum, pSourceAVIFile->GetWaveFormat(dwSrcStreamNum));

	return true;
}

volatile int CAVIFile::m_nPrevPercentDone = -1;
volatile int CAVIFile::m_nSaveCopyAsIsProcessing = 0;
CAVIFile* volatile CAVIFile::pThis = NULL;
LONG WINAPI CAVIFile::SaveCopyAsCallback(int nPercent)
{
	// Sometimes The 100 % is not sent:
	if (nPercent >= 99)
		nPercent = 100;

	if (nPercent > m_nPrevPercentDone)
	{
		switch (m_nSaveCopyAsIsProcessing)
		{
			case CAVIFile::SAVECOPYAS_VIDEO :
				::PostMessage((CAVIFile::pThis)->m_pParentWnd->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeVIDEO, (LPARAM)nPercent);
				break;

			case CAVIFile::SAVECOPYAS_AUDIO :
				::PostMessage((CAVIFile::pThis)->m_pParentWnd->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeAUDIO, (LPARAM)nPercent);
				break;

			case CAVIFile::SAVECOPYAS_BOTH :
				::PostMessage((CAVIFile::pThis)->m_pParentWnd->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeVIDEO, (LPARAM)nPercent);
				::PostMessage((CAVIFile::pThis)->m_pParentWnd->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeAUDIO, (LPARAM)nPercent);
				break;

			default :
				break;
		}
	}
	m_nPrevPercentDone = nPercent;

	if ((CAVIFile::pThis)->DoExit())
		return AVIERR_USERABORT;
	else
		return AVIERR_OK;
}

bool CAVIFile::MergeAviFilesParallel(	CString sOutputFileName,
										CSortableStringArray* pAviFileNames)
{
	DWORD dwStreamNum, dwFileNum;
	DWORD dwTotalMergeStreams = 0;
	DWORD dwFilesCount = pAviFileNames->GetSize();
	CAVIFile** ppAviFiles = (CAVIFile**) new CAVIFile*[dwFilesCount];
	
	// Open Files
	for (dwFileNum = 0 ; dwFileNum < dwFilesCount ; dwFileNum++)
	{
		ppAviFiles[dwFileNum] = (CAVIFile*) new CAVIFile(NULL, (*pAviFileNames)[dwFileNum], m_bShowMessageBoxOnError);
		dwTotalMergeStreams +=	ppAviFiles[dwFileNum]->GetVideoStreamsCount() +
								ppAviFiles[dwFileNum]->GetAudioStreamsCount();
	}

	// Allocate Array of Streams
	PAVISTREAM* AviStreams = (PAVISTREAM*)new PAVISTREAM[dwTotalMergeStreams];

	// Fill Array of Streams
	dwTotalMergeStreams = 0;
	for (dwFileNum = 0 ; dwFileNum < dwFilesCount ; dwFileNum++)
	{
		for (dwStreamNum = 0 ; dwStreamNum < ppAviFiles[dwFileNum]->GetVideoStreamsCount() ; dwStreamNum++)
			AviStreams[dwTotalMergeStreams++] = ppAviFiles[dwFileNum]->GetVideoStream(dwStreamNum);
		for (dwStreamNum = 0 ; dwStreamNum < ppAviFiles[dwFileNum]->GetAudioStreamsCount() ; dwStreamNum++)
			AviStreams[dwTotalMergeStreams++] = ppAviFiles[dwFileNum]->GetAudioStream(dwStreamNum);
	}

	// Save Array of Streams to a new Avi File
	bool res = SaveCopyAs(sOutputFileName, dwTotalMergeStreams, AviStreams);

	// Clean-up
	delete [] AviStreams;
	for (dwFileNum = 0 ; dwFileNum < dwFilesCount ; dwFileNum++)
		delete ppAviFiles[dwFileNum];
	delete [] ppAviFiles;

	return res;
}

bool CAVIFile::SaveCopyAs(CString sFileName, int nStreams, PAVISTREAM* ppAviStreams)
{
	int nStreamNum;
	int nVideoStreams = 0;
	int nAudioStreams = 0;
	HRESULT hr;
	AVISTREAMINFO StreamInfo;

	// Params Check
	if (nStreams <= 0)
		return false;
	if (!ppAviStreams)
		return false;

	DWORD dwAttrib = ::GetFileAttributes(sFileName);
	if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
	{
		CString str;
		str.Format(_T("The file %s\nis read only\n"), sFileName);
#ifdef _DEBUG
		str = _T("SaveCopyAs():\n") + str;
#endif
		TRACE(str);
		if (m_bShowMessageBoxOnError)
			::AfxMessageBox(str, MB_ICONSTOP);
		return false;
	}

	// Array of Compression Options
	LPAVICOMPRESSOPTIONSVOLATILE* ppCompressionOptions = (LPAVICOMPRESSOPTIONSVOLATILE*)new LPAVICOMPRESSOPTIONSVOLATILE[nStreams];
	for (nStreamNum = 0 ; nStreamNum < nStreams ; nStreamNum++)
	{
		ppCompressionOptions[nStreamNum] = (LPAVICOMPRESSOPTIONSVOLATILE)new AVICOMPRESSOPTIONSVOLATILE;
		::ZeroMemory(ppCompressionOptions[nStreamNum], sizeof(AVICOMPRESSOPTIONSVOLATILE));
		if (nStreamNum > 0)
		{
			ppCompressionOptions[nStreamNum]->dwFlags = AVICOMPRESSF_INTERLEAVE;
			ppCompressionOptions[nStreamNum]->dwInterleaveEvery = 1;
		}
	}

	// Audio / Video Streams Check & Count
	for (nStreamNum = 0 ; nStreamNum < nStreams ; nStreamNum++)
	{
		if (ppAviStreams[nStreamNum])
			hr = ::AVIStreamInfo(ppAviStreams[nStreamNum], &StreamInfo, sizeof(AVISTREAMINFO));
		else
			hr = AVIERR_BADPARAM;
		if (hr != AVIERR_OK)
		{
			for (nStreamNum = 0 ; nStreamNum < nStreams ; nStreamNum++)
				delete ppCompressionOptions[nStreamNum];
			delete [] ppCompressionOptions;
			return false;
		}
		if (StreamInfo.fccType == streamtypeVIDEO)
			++nVideoStreams;
		else if (StreamInfo.fccType == streamtypeAUDIO)
			++nAudioStreams;
	}

	CAVIFile::pThis = this;
	m_nPrevPercentDone = -1;
	// Only Video Streams
	if ((nVideoStreams > 0) && (nAudioStreams == 0))
		CAVIFile::m_nSaveCopyAsIsProcessing = CAVIFile::SAVECOPYAS_VIDEO;
	// Only Audio Streams
	else if ((nVideoStreams == 0) && (nAudioStreams > 0))
		CAVIFile::m_nSaveCopyAsIsProcessing = CAVIFile::SAVECOPYAS_AUDIO;
	// Video And Audio Streams
	else if ((nVideoStreams > 0) && (nAudioStreams > 0))
		CAVIFile::m_nSaveCopyAsIsProcessing = CAVIFile::SAVECOPYAS_BOTH;
	// Other Streams
	else
		CAVIFile::m_nSaveCopyAsIsProcessing = CAVIFile::SAVECOPYAS_UNKNOWN;
	hr = ::AVISaveV(sFileName,
					NULL,
					AVISAVECALLBACK(CAVIFile::SaveCopyAsCallback),  
					nStreams,                  
					ppAviStreams,          
					(LPAVICOMPRESSOPTIONS*)ppCompressionOptions);

	// Clean-up
	for (nStreamNum = 0 ; nStreamNum < nStreams ; nStreamNum++)
		delete ppCompressionOptions[nStreamNum];
	delete [] ppCompressionOptions;

	if (hr == AVIERR_OK)
		return true;
	else
		return false;
}

ULARGE_INTEGER CAVIFile::GetFileSize()
{
	ULARGE_INTEGER Size;
	Size.QuadPart = 0;

	if (m_sFileName == _T(""))
		return Size;

	HANDLE hFile = ::CreateFile(m_sFileName,
								0, // Only Query Access
								FILE_SHARE_READ |
								FILE_SHARE_WRITE,
								NULL, OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return Size;
	
	Size.LowPart = ::GetFileSize(hFile, &(Size.HighPart));

	::CloseHandle(hFile);

	return Size;
}