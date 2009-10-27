#include "stdafx.h"
//#include "MainFrm.h"
#include "AviFileMerge.h"
#include "AviPlay.h"
#include "AviFile.h"
#include "AVRec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool DoExit(HANDLE hKillEvent)
{
	if (hKillEvent)
	{
		DWORD Event = ::WaitForSingleObject(hKillEvent, 0);
		switch (Event)
		{
			// Shutdown Event?
			case WAIT_OBJECT_0 :	return true;
			case WAIT_TIMEOUT :		return false;
			default:				return true;
		}
	}
	else
		return false;
}

bool VideoCodec(DWORD dwStreamNum,
				CAVIPlay* pInAviPlay,
				CAVIFile* pOutAviFile,
				bool bReCompressVideo,
				CWnd* pWnd,
				HANDLE hKillEvent)
{
	if (bReCompressVideo)
	{
		CAVIPlay::CAVIVideoStream* pVideoStream = pInAviPlay->GetVideoStream(dwStreamNum);
		if (pVideoStream)
		{
			if (!pVideoStream->OpenDecompression(true))
				return false;
		}
		else
			return false;

		CDib Dib;
		int nPercentDone;
		int nPrevPercentDone = -1;
		bool bFirst = true;
		bool bToRgb24 = false;
		int nFramePos = 0;
		while (pVideoStream->GetFrame(&Dib))
		{
			// Convert to 24bpp?
			if (bToRgb24)
				Dib.ConvertTo24bits();

			// Progress
			nPercentDone = Round((double)nFramePos * 100.0 / (double)(pVideoStream->GetTotalFrames()));
			if (nPercentDone > nPrevPercentDone)
				::PostMessage(pWnd->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeVIDEO, (LPARAM)nPercentDone);
			nPrevPercentDone = nPercentDone;
			
			// First Frame?
			if (bFirst)
			{
				bFirst = false;
				if (!pOutAviFile->InitVideoWrite(dwStreamNum, Dib.GetBMI()))
				{
					// Retry Init Video Write with 24 bpp
					if (Dib.GetBitCount() != 24)
					{
						Dib.ConvertTo24bits();
						bToRgb24 = true;
						pOutAviFile->SetOk(true); // Reset Last Error!
						if (!pOutAviFile->InitVideoWrite(dwStreamNum, Dib.GetBMI()))
							return false;
					}
					else
						return false;
				}
			}

			// Add Frame
			if (!pOutAviFile->AddFrame(dwStreamNum, &Dib))
				return false;

			// Do Exit?
			if (DoExit(hKillEvent))
				return false;

			// Inc. Frame Pos
			nFramePos++;
		}
	}
	else
	{
		CAVIPlay::CAVIVideoStream* pVideoStream = pInAviPlay->GetVideoStream(dwStreamNum);
		if (!pVideoStream)
			return false;

		int nPercentDone;
		int nPrevPercentDone = -1;
		bool bFirst = true;
		DWORD dwInFramePos = 0;
		LPBYTE pData = new BYTE[pVideoStream->GetWidth() * pVideoStream->GetHeight() * 4];
		DWORD dwSize = pVideoStream->GetWidth() * pVideoStream->GetHeight() * 4;
		while (pVideoStream->GetChunkData(dwInFramePos, pData, &dwSize))
		{
			// Progress
			nPercentDone = Round((double)dwInFramePos * 100.0 / (double)(pVideoStream->GetTotalFrames()));
			if (nPercentDone > nPrevPercentDone)
				::PostMessage(pWnd->GetSafeHwnd(), WM_AVIFILE_PROGRESS, (WPARAM)streamtypeVIDEO, (LPARAM)nPercentDone);
			nPrevPercentDone = nPercentDone;

			// Init?
			if (bFirst)
			{
				bFirst = false;
				if (!pOutAviFile->InitVideoWrite(dwStreamNum, pVideoStream->GetFormat(true)))
				{
					delete [] pData;
					return false;
				}
			}
			
			// Save the frame to the AVI stream
			DWORD dwOutFramePos = pOutAviFile->GetWriteNextFramePos(dwStreamNum);
			HRESULT hr = ::AVIStreamWrite(	pOutAviFile->GetVideoStream(dwStreamNum), 
											dwOutFramePos,
											1,
											pData, 
											dwSize, 
											pVideoStream->IsKeyFrame(dwInFramePos) ? AVIIF_KEYFRAME : 0,
											NULL, 
											NULL);
			if (hr != AVIERR_OK)
			{
				delete [] pData;
				return false;
			}
			
			// Do Exit?
			if (DoExit(hKillEvent))
			{
				delete [] pData;
				return false;
			}

			// Inc. Frame Pos
			dwInFramePos++;
			pOutAviFile->SetWriteNextFramePos(dwStreamNum, dwOutFramePos + 1);
		}

		// Clean-Up
		delete [] pData;
	}

	return true;
}

bool AppendInitSegment(	CString sInFileName,
						CString sOutVideoFileName,
						CString sOutAudioFileName,
						CAVIFile** ppOutAviVideo,
						CAVIFile** ppOutAviAudio,
						bool bReCompressVideo,
						CWnd* pWnd,
						HANDLE hKillEvent,
						bool bShowMessageBoxOnError)
{
	// Check
	if (!ppOutAviVideo && !ppOutAviAudio)
		return false;
	
	// Var
	DWORD dwStreamNum;

	// Open Input Avi File
	CAVIFile InAviFile(pWnd, sInFileName, bShowMessageBoxOnError);
	InAviFile.SetKillEvent(hKillEvent);	// For Processing Progress Stop

	// Create Output Video Avi File
	if (ppOutAviVideo)
	{
		if (*ppOutAviVideo)
			delete *ppOutAviVideo;
		*ppOutAviVideo = new CAVIFile (	pWnd,
										sOutVideoFileName,
										InAviFile.GetRate(0),
										InAviFile.GetScale(0),
										NULL,
										true,
										true,
										bShowMessageBoxOnError);
	}

	// Create Output Audio Avi File
	if (ppOutAviAudio)
	{
		if (*ppOutAviAudio)
			delete *ppOutAviAudio;
		*ppOutAviAudio = new CAVIFile (	pWnd,
										sOutAudioFileName,
										1,
										1,
										InAviFile.GetWaveFormat(0),
										true,
										true,
										bShowMessageBoxOnError);
	}

	// Video Config
	if (bReCompressVideo && ppOutAviVideo && *ppOutAviVideo)
	{
		// Video Dialog(s)
		for (dwStreamNum = 0 ; dwStreamNum < InAviFile.GetVideoStreamsCount() ; dwStreamNum++)
		{
			(*ppOutAviVideo)->SetVideoCompressor(dwStreamNum, (*ppOutAviVideo)->ChooseVideoCompressorForSave(InAviFile.GetFourCC(dwStreamNum)));
			(*ppOutAviVideo)->SetVideoCompressorQuality(dwStreamNum, InAviFile.GetVideoCompressorQuality(dwStreamNum));
			(*ppOutAviVideo)->SetVideoCompressorKeyframesRate(dwStreamNum, InAviFile.GetVideoCompressorKeyframesRate(dwStreamNum));
			(*ppOutAviVideo)->SetVideoCompressorDataRate(dwStreamNum, InAviFile.GetVideoCompressorDataRate(dwStreamNum));
			(*ppOutAviVideo)->SetVideoCompressorConfig(	dwStreamNum,
														NULL,
														(const BYTE*)InAviFile.GetVideoCompressorOptions(dwStreamNum)->lpParms,
														InAviFile.GetVideoCompressorOptions(dwStreamNum)->cbParms);
			if (!(*ppOutAviVideo)->VideoCompressorDialog(dwStreamNum))
				return false;
		}
	}

	// Video Processing
	if (ppOutAviVideo && *ppOutAviVideo)
	{
		CAVIPlay InAviPlay(sInFileName, bShowMessageBoxOnError);
		for (dwStreamNum = 0 ; dwStreamNum < InAviPlay.GetVideoStreamsCount() ; dwStreamNum++)
		{	
			if (!VideoCodec(dwStreamNum,
							&InAviPlay,
							*ppOutAviVideo,
							bReCompressVideo,
							pWnd,
							hKillEvent))
				return false;
		}
	}

	// Audio Processing, just copy stream(s)
	if (ppOutAviAudio && *ppOutAviAudio)
	{
		for (dwStreamNum = 0 ; dwStreamNum < InAviFile.GetAudioStreamsCount() ; dwStreamNum++)
		{
			if (!InAviFile.AudioCodec(	dwStreamNum,
										dwStreamNum,
										*ppOutAviAudio))
				return false;
		}
	}

	return true;
}

bool AppendNextSegment(	CString sInFileName,
						CAVIFile* pOutAviVideo,
						CAVIFile* pOutAviAudio,
						bool bReCompressVideo,
						CWnd* pWnd,
						HANDLE hKillEvent,
						bool bShowMessageBoxOnError)
{
	// Check
	if (!pOutAviVideo && !pOutAviAudio)
		return false;
	
	// Var
	DWORD dwStreamNum;

	// Open Input Avi File
	CAVIFile InAviFile(pWnd, sInFileName, bShowMessageBoxOnError);
	InAviFile.SetKillEvent(hKillEvent);	// For Processing Progress Stop

	// Video Processing
	if (pOutAviVideo)
	{
		CAVIPlay InAviPlay(sInFileName, bShowMessageBoxOnError);
		for (dwStreamNum = 0 ; dwStreamNum < InAviPlay.GetVideoStreamsCount() ; dwStreamNum++)
		{	
			if (!VideoCodec(dwStreamNum,
							&InAviPlay,
							pOutAviVideo,
							bReCompressVideo,
							pWnd,
							hKillEvent))
				return false;
		}
	}

	// Audio Processing, just copy stream(s)
	if (pOutAviAudio)
	{
		for (dwStreamNum = 0 ; dwStreamNum < InAviFile.GetAudioStreamsCount() ; dwStreamNum++)
		{
			if (!InAviFile.AudioCodec(dwStreamNum, dwStreamNum, pOutAviAudio))
				return false;
		}
	}

	return true;
}

bool AVIFileMergeSerialVfW(	CString sSaveFileName,
							CSortableStringArray* pAviFileNames,
							bool bReCompressVideo,
							bool bReCompressAudio,
							CWnd* pWnd,
							HANDLE hKillEvent,
							bool bShowMessageBoxOnError) 
{
	// Check
	if (!pAviFileNames)
		return false;

	// Merge Serial, Audio Stream remains as it is,
	// Video Stream is compressed with the chosen Codec.
	CAVIFile* pOutAviFileVideo = NULL;
	CAVIFile* pOutAviFileAudio = NULL;
	CAVIFile* pOutAviFileAudioCompressed = NULL;
	bool res = true;
	CString SaveFileNameVideo = ::GetFileNameNoExt(sSaveFileName) + _T("_video.avi");
	CString SaveFileNameAudio = ::GetFileNameNoExt(sSaveFileName) + _T("_audio.avi");
	CString SaveFileNameAudioCompressed = _T("");
	for (int i = 0 ; i < pAviFileNames->GetSize() ; i++)
	{
		// Be First?
		if (i == 0)
		{
			if (!AppendInitSegment(	(*pAviFileNames)[i],
									SaveFileNameVideo,
									SaveFileNameAudio,
									&pOutAviFileVideo,
									&pOutAviFileAudio,
									bReCompressVideo,
									pWnd,
									hKillEvent,
									bShowMessageBoxOnError))
			{
				res = false;
				break;
			}
		}
		else
		{
			if (!AppendNextSegment(	(*pAviFileNames)[i],
									pOutAviFileVideo,
									pOutAviFileAudio,
									bReCompressVideo,
									pWnd,
									hKillEvent,
									bShowMessageBoxOnError))
			{
				res = false;
				break;
			}
		}
	}

	// ReCompress Audio?
	if (res && pOutAviFileAudio && bReCompressAudio)
	{
		pOutAviFileAudio->InitRead();
		SaveFileNameAudioCompressed = ::GetFileNameNoExt(sSaveFileName) + _T("_audiocompressed.avi");
		pOutAviFileAudioCompressed = new CAVIFile (	pWnd,
													SaveFileNameAudioCompressed,
													1,
													1,
													NULL,
													true,
													true,
													bShowMessageBoxOnError);
		DWORD dwStreamNum;
		for (dwStreamNum = 0 ; dwStreamNum < pOutAviFileAudio->GetAudioStreamsCount() ; dwStreamNum++)
		{
			if (!pOutAviFileAudioCompressed->AudioCompressorDialog(dwStreamNum))
			{
				res = false;
				break;
			}
		}
		for (dwStreamNum = 0 ; dwStreamNum < pOutAviFileAudio->GetAudioStreamsCount() ; dwStreamNum++)
		{
			if (!pOutAviFileAudio->AudioCodec(dwStreamNum, dwStreamNum, pOutAviFileAudioCompressed))
			{
				res = false;
				break;
			}
		}
	}

	// Do Merge Parallel Flag
	bool bDoMergeParallel;
	if (pOutAviFileVideo && pOutAviFileAudio)
		bDoMergeParallel = true;
	else
		bDoMergeParallel = false;

	// Clean-up
	delete pOutAviFileVideo;
	delete pOutAviFileAudio;
	delete pOutAviFileAudioCompressed;

	// Merge Streams in Parallel
	if (res && bDoMergeParallel)
	{
		CSortableStringArray AviFileStreams;
		AviFileStreams.Add(SaveFileNameVideo);
		if (SaveFileNameAudioCompressed != _T(""))
			AviFileStreams.Add(SaveFileNameAudioCompressed);
		else
			AviFileStreams.Add(SaveFileNameAudio);
		res = AVIFileMergeParallelVfW(	sSaveFileName,
										&AviFileStreams,
										pWnd,
										hKillEvent,
										bShowMessageBoxOnError);
	}

	return res;
}

bool AVIFileMergeParallelVfW(	CString sSaveFileName,
								CSortableStringArray* pAviFileNames,
								CWnd* pWnd,
								HANDLE hKillEvent,
								bool bShowMessageBoxOnError) 
{
	if (!pAviFileNames)
		return false;
	CAVIFile AviFile(pWnd, bShowMessageBoxOnError);	// For Processing Progress Show
	AviFile.SetKillEvent(hKillEvent);				// For Processing Progress Stop
	return AviFile.MergeAviFilesParallel(sSaveFileName, pAviFileNames);
}
