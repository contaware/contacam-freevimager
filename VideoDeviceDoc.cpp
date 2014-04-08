#include "stdafx.h"
#include "uImager.h"
#include "VideoDeviceView.h"
#include "MainFrm.h"
#include "VideoDeviceDoc.h"
#include "AviPlay.h"
#include "AudioInSourceDlg.h"
#include "AssistantDlg.h"
#include "GeneralPage.h"
#include "SnapshotPage.h"
#include "VideoDevicePropertySheet.h"
#include "Quantizer.h"
#include "DxCapture.h"
#include "DxVideoFormatDlg.h"
#include "AudioFormatDlg.h"
#include "ConnectErrMsgBoxDlg.h"
#include "HttpVideoFormatDlg.h"
#include "SendMailConfigurationDlg.h"
#include "FTPUploadConfigurationDlg.h"
#include "FTPTransfer.h"
#include "getdxver.h"
#include "BrowseDlg.h"
#include "PostDelayedMessage.h"
#include "MotionDetHelpers.h"
#include "Base64.h"
#include "PJNMD5.h"
#include "Psapi.h"
#include "NoVistaFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

// Defined in uImager.cpp
int avcodec_open_thread_safe(AVCodecContext *avctx, AVCodec *codec);
int avcodec_close_thread_safe(AVCodecContext *avctx);

/////////////////////////////////////////////////////////////////////////////
// PictureDoc

IMPLEMENT_DYNCREATE(CVideoDeviceDoc, CUImagerDoc)

BEGIN_MESSAGE_MAP(CVideoDeviceDoc, CUImagerDoc)
	//{{AFX_MSG_MAP(CVideoDeviceDoc)
	ON_COMMAND(ID_CAPTURE_RECORD, OnCaptureRecord)
	ON_UPDATE_COMMAND_UI(ID_CAPTURE_RECORD, OnUpdateCaptureRecord)
	ON_COMMAND(ID_CAPTURE_SETTINGS, OnCaptureSettings)
	ON_COMMAND(ID_VIEW_VIDEO, OnViewVideo)
	ON_UPDATE_COMMAND_UI(ID_VIEW_VIDEO, OnUpdateViewVideo)
	ON_COMMAND(ID_VIEW_FRAMETIME, OnViewFrametime)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FRAMETIME, OnUpdateViewFrametime)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_COMMAND(ID_VIEW_DETECTIONS, OnViewDetections)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DETECTIONS, OnUpdateViewDetections)
	ON_UPDATE_COMMAND_UI(ID_CAPTURE_SETTINGS, OnUpdateCaptureSettings)
	ON_COMMAND(ID_VIEW_DETECTION_ZONES, OnViewDetectionZones)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DETECTION_ZONES, OnUpdateViewDetectionZones)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_COMMAND(ID_VIEW_FIT, OnViewFit)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FIT, OnUpdateViewFit)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_COMMAND(ID_EDIT_SNAPSHOT, OnEditSnapshot)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SNAPSHOT, OnUpdateEditSnapshot)
	ON_COMMAND(ID_EDIT_EXPORT_ZONES, OnEditExportZones)
	ON_COMMAND(ID_EDIT_IMPORT_ZONES, OnEditImportZones)
	ON_COMMAND(ID_CAPTURE_ASSISTANT, OnCaptureAssistant)
	ON_UPDATE_COMMAND_UI(ID_CAPTURE_ASSISTANT, OnUpdateCaptureAssistant)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CVideoDeviceDoc::CreateCheckYearMonthDayDir(CTime Time, CString sBaseDir, CString& sYearMonthDayDir)
{
	// Remove Trailing '\'
	sBaseDir.TrimRight(_T('\\'));

	// The Dir Strings
	CString sYear = Time.Format(_T("%Y"));
	CString sMonth = Time.Format(_T("%m"));
	CString sDay = Time.Format(_T("%d"));

	// Check Passed Dir
	DWORD dwAttrib = ::GetFileAttributes(sBaseDir);
	if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) // Not Existing or Not A Directory
		return FALSE;

	// Create Year Dir if not existing
	dwAttrib = ::GetFileAttributes(sBaseDir + _T("\\") + sYear);
	if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) // Not Existing or Not A Directory
	{
		if (!::CreateDir(sBaseDir + _T("\\") + sYear))
		{
			::ShowLastError(FALSE);
			return FALSE;
		}
	}

	// Create Month Dir if not existing
	dwAttrib = ::GetFileAttributes(sBaseDir + _T("\\") + sYear + _T("\\") + sMonth);
	if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) // Not Existing or Not A Directory
	{
		if (!::CreateDir(sBaseDir + _T("\\") + sYear + _T("\\") + sMonth))
		{
			::ShowLastError(FALSE);
			return FALSE;
		}
	}

	// Create Day Dir if not existing
	dwAttrib = ::GetFileAttributes(sBaseDir + _T("\\") + sYear + _T("\\") + sMonth + _T("\\") + sDay);
	if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) // Not Existing or Not A Directory
	{
		if (!::CreateDir(sBaseDir + _T("\\") + sYear + _T("\\") + sMonth + _T("\\") + sDay))
		{
			::ShowLastError(FALSE);
			return FALSE;
		}
	}

	sYearMonthDayDir = sBaseDir + _T("\\") + sYear + _T("\\") + sMonth + _T("\\") + sDay;
	return TRUE;
}

void CVideoDeviceDoc::CSaveFrameListThread::CalcMovementDetectionListsSize()
{
	m_pDoc->m_dwTotalMovementDetectionListSize = 0U;
	m_pDoc->m_dwNewestMovementDetectionListSize = 0U;
	if (!m_pDoc->m_MovementDetectionsList.IsEmpty())
	{
		POSITION pos = m_pDoc->m_MovementDetectionsList.GetHeadPosition();
		while (pos)
		{
			CDib::LIST* pList = m_pDoc->m_MovementDetectionsList.GetNext(pos);
			if (pList)
			{
				m_pDoc->m_dwNewestMovementDetectionListSize = 0U;
				POSITION posDibs = pList->GetHeadPosition();
				while (posDibs)
				{
					CDib* pDib = pList->GetNext(posDibs);
					if (pDib)
					{
						// For video frames BIGALLOC_USEDSIZE accounts for the 64 KB
						// allocation granularity and the address space waste
						DWORD dwVideoUsedSize = sizeof(CDib) + pDib->GetBMISize() + BIGALLOC_USEDSIZE(pDib->GetImageSize());

						// For audio the maximum allocated size for a single buffer
						// with AUDIO_IN_MIN_BUF_SIZE set to 256 is:
						// PCM and Vorbis = 5120  bytes + FF_INPUT_BUFFER_PADDING_SIZE
						// ADPCM          = 8136  bytes + FF_INPUT_BUFFER_PADDING_SIZE
						// Flac           = 18432 bytes + FF_INPUT_BUFFER_PADDING_SIZE
						// MP2 or MP3     = 9216  bytes + FF_INPUT_BUFFER_PADDING_SIZE
						DWORD dwAudioUsedSize = 0U;
						POSITION posAudioBuf = pDib->m_UserList.GetHeadPosition();
						while (posAudioBuf)
							dwAudioUsedSize += pDib->m_UserList.GetNext(posAudioBuf).m_dwSize + FF_INPUT_BUFFER_PADDING_SIZE;

						// Sum
						m_pDoc->m_dwNewestMovementDetectionListSize += dwVideoUsedSize + dwAudioUsedSize;
					}
				}
				m_pDoc->m_dwTotalMovementDetectionListSize += m_pDoc->m_dwNewestMovementDetectionListSize;
			}
		}
	}
}

BOOL CVideoDeviceDoc::CSaveFrameListThread::DecodeFrame(CDib* pDib)
{
	BOOL res = TRUE;

	// Check whether still compressed, could already have
	// been decompressed by thread loop or AnimatedGifInit()
	if (pDib->GetCompression() == FCC('MJPG') || pDib->GetCompression() == FCC('M601'))
	{
		// Init device flag
		BOOL bDeviceIsMJPG = FALSE;
		if (pDib->GetCompression() == FCC('MJPG'))
			bDeviceIsMJPG = TRUE;

		// Store old pointers
		LPBITMAPINFO pOldBmi = pDib->GetBMI();
		LPBYTE pOldBits = pDib->GetBits();
		pDib->SetDibPointers(NULL, NULL);

		// Set new header
		BITMAPINFO NewBmi;
		memset(&NewBmi, 0, sizeof(BITMAPINFO));
		NewBmi.bmiHeader.biSize =			sizeof(BITMAPINFOHEADER);
		NewBmi.bmiHeader.biWidth =			pOldBmi->bmiHeader.biWidth;
		NewBmi.bmiHeader.biHeight =			pOldBmi->bmiHeader.biHeight;
		NewBmi.bmiHeader.biPlanes =			1;
		NewBmi.bmiHeader.biCompression =	FCC('I420');
		NewBmi.bmiHeader.biBitCount =		12;
		int stride = ::CalcYUVStride(NewBmi.bmiHeader.biCompression, (int)NewBmi.bmiHeader.biWidth);
		NewBmi.bmiHeader.biSizeImage =		::CalcYUVSize(NewBmi.bmiHeader.biCompression, stride, (int)NewBmi.bmiHeader.biHeight);
		pDib->SetBMI(&NewBmi);

		// Decode
		res = m_AVDetDecoder.Decode(pOldBmi,
									pOldBits,
									pOldBmi->bmiHeader.biSizeImage,
									pDib);
		// In case that avcodec_decode_video fails use LoadJPEG which is more fault tolerant, but slower...
		if (!res)
			res = pDib->LoadJPEG(pOldBits, pOldBmi->bmiHeader.biSizeImage, 1, TRUE) && pDib->Compress(FCC('I420'));

		// Free
		delete [] pOldBmi;
		BIGFREE(pOldBits);

		// Frames coming from a mjpeg source have to be processed here
		// if set so. The other ones are already deinterlaced and/or
		// rotated by 180° in ProcessI420Frame() before being compressed
		if (res && bDeviceIsMJPG)
		{
			// De-Interlace?
			if (pDib->GetUserFlag() & FRAME_USER_FLAG_DEINTERLACE)
				CVideoDeviceDoc::Deinterlace(pDib);

			// Rotate by 180°?
			if (pDib->GetUserFlag() & FRAME_USER_FLAG_ROTATE180)
				CVideoDeviceDoc::Rotate180(pDib);
		}
	}

	return res;
}

int CVideoDeviceDoc::CSaveFrameListThread::Work() 
{
	ASSERT(m_pDoc);
	m_bWorking = FALSE;
	CTime FirstTime(0);
	CTime LastTime(0);
	CString sTempDetectionDir; 
	sTempDetectionDir.Format(_T("Detection%X"), ::GetCurrentThreadId());
	sTempDetectionDir = ((CUImagerApp*)::AfxGetApp())->GetAppTempDir() + sTempDetectionDir;
	while (TRUE)
	{
		// Poll for Work
		m_pFrameList = NULL;
		m_nNumFramesToSave = 0;
		m_nSaveProgress = 100;
		BOOL bPolling = TRUE;
		do
		{	
			while (TRUE)
			{
				// Is count >= 2?
				::EnterCriticalSection(&m_pDoc->m_csMovementDetectionsList);
				CalcMovementDetectionListsSize();
				if (m_pDoc->m_MovementDetectionsList.GetCount() >= 2)
					break;
				::LeaveCriticalSection(&m_pDoc->m_csMovementDetectionsList);

				// Set that we are not working
				m_bWorking = FALSE;

				// Shutdown?
				if (::WaitForSingleObject(GetKillEvent(), MOVDET_SAVEFRAMES_POLL) == WAIT_OBJECT_0)
				{
					if (::IsExistingDir(sTempDetectionDir))
						::DeleteDir(sTempDetectionDir);
					return 0;
				}
			}
			while (m_pDoc->m_MovementDetectionsList.GetCount() >= 2)
			{
				// Get oldest (head) list
				m_pFrameList = m_pDoc->m_MovementDetectionsList.GetHead();
				if (m_pFrameList)
					m_nNumFramesToSave = m_pFrameList->GetCount();
				else
					m_nNumFramesToSave = 0;

				// Ok there is something to do
				if (m_nNumFramesToSave > 0)
				{
					bPolling = FALSE;
					m_bWorking = TRUE;
					break;
				}
				// We have an empty list, remove it!
				else
				{
					if (m_pFrameList)
					{
						delete m_pFrameList;
						m_pFrameList = NULL;
					}
					m_pDoc->m_MovementDetectionsList.RemoveHead();
				}
			}
			::LeaveCriticalSection(&m_pDoc->m_csMovementDetectionsList);
		}
		while (bPolling);
		ASSERT(m_pFrameList);
		ASSERT(m_nNumFramesToSave > 0);
		ASSERT(m_bWorking);

		// Is this frame list the last list of the detection sequence?
		BOOL bDetectionSequenceDone = ((m_pFrameList->GetTail()->GetUserFlag() & FRAME_USER_FLAG_LAST) == FRAME_USER_FLAG_LAST);

		// First & Last Up-Times
		DWORD dwFirstUpTime = m_pFrameList->GetHead()->GetUpTime();
		DWORD dwLastUpTime = m_pFrameList->GetTail()->GetUpTime();

		// Find a good Reference Time and make the First Time string
		// (if new first frame is older than last frame from previous detection
		// then we have rounding errors -> search better ref. time + ref. up-time)
		DWORD dwStartUpTime = ::timeGetTime();
		CTime RefTime;
		DWORD dwRefUpTime;
		do
		{
			RefTime = CTime::GetCurrentTime();
			dwRefUpTime = ::timeGetTime();
			FirstTime = CalcTime(dwFirstUpTime, RefTime, dwRefUpTime);
			if (FirstTime < LastTime)
			{
				if (::WaitForSingleObject(GetKillEvent(), 10U) == WAIT_OBJECT_0)
				{
					m_bWorking = FALSE;
					if (::IsExistingDir(sTempDetectionDir))
						::DeleteDir(sTempDetectionDir);
					return 0;
				}
			}
			else
				break;
		}
		while ((dwRefUpTime - dwStartUpTime) <= 1100U); // be safe in case computer time has been changed
		ASSERT(FirstTime >= LastTime);
		LastTime = CalcTime(dwLastUpTime, RefTime, dwRefUpTime);
		CString sFirstTime(FirstTime.Format(_T("%Y_%m_%d_%H_%M_%S")));

		// Directory to Store Detection
		CString sDetectionAutoSaveDir;
		if (m_pDoc->m_bSaveSWFMovementDetection	||
			m_pDoc->m_bSaveAVIMovementDetection ||
			m_pDoc->m_bSaveAnimGIFMovementDetection)
		{
			// Check Whether Dir Exists
			sDetectionAutoSaveDir = m_pDoc->m_sRecordAutoSaveDir;
			DWORD dwAttrib = ::GetFileAttributes(sDetectionAutoSaveDir);
			if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) // Not Existing or Not A Directory
			{
				// Temp Dir To Store Files
				sDetectionAutoSaveDir = sTempDetectionDir;
				dwAttrib = ::GetFileAttributes(sDetectionAutoSaveDir);
				if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) // Not Existing or Not A Directory
				{
					if (!::CreateDir(sDetectionAutoSaveDir))
						::ShowLastError(FALSE);
				}	
			}
			else
				sDetectionAutoSaveDir.TrimRight(_T('\\'));
		}
		// Temp Dir To Store Files For Email Sending and/or Ftp Upload
		else
		{
			sDetectionAutoSaveDir = sTempDetectionDir;
			DWORD dwAttrib = ::GetFileAttributes(sDetectionAutoSaveDir);
			if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) // Not Existing or Not A Directory
			{
				if (!::CreateDir(sDetectionAutoSaveDir))
					::ShowLastError(FALSE);
			}
		}

		// Load the detection sequences counter and reset it if entering a new day.
		// The detection sequences are tagged 1, 2, 3, ... A detection sequence is
		// composed of 1 or more movies all tagged with the same sequence number
		CString sSection(m_pDoc->GetDevicePathName());
		m_pDoc->m_nMovDetSavesCount = ::AfxGetApp()->GetProfileInt(sSection, _T("MovDetSavesCount"), 1);
		m_pDoc->m_nMovDetSavesCountDay = ::AfxGetApp()->GetProfileInt(sSection, _T("MovDetSavesCountDay"), FirstTime.GetDay());
		m_pDoc->m_nMovDetSavesCountMonth = ::AfxGetApp()->GetProfileInt(sSection, _T("MovDetSavesCountMonth"), FirstTime.GetMonth());
		m_pDoc->m_nMovDetSavesCountYear = ::AfxGetApp()->GetProfileInt(sSection, _T("MovDetSavesCountYear"), FirstTime.GetYear());
		if (m_pDoc->m_nMovDetSavesCountDay != FirstTime.GetDay()		||
			m_pDoc->m_nMovDetSavesCountMonth != FirstTime.GetMonth()	||
			m_pDoc->m_nMovDetSavesCountYear != FirstTime.GetYear())
		{
			m_pDoc->m_nMovDetSavesCount = 1;
			m_pDoc->m_nMovDetSavesCountDay = FirstTime.GetDay();
			m_pDoc->m_nMovDetSavesCountMonth = FirstTime.GetMonth();
			m_pDoc->m_nMovDetSavesCountYear = FirstTime.GetYear();
		}

		// Detection File Names
		BOOL bMakeAvi = DoMakeAvi();
		BOOL bMakeSwf = DoMakeSwf();
		BOOL bMakeJpeg = DoMakeJpeg();
		BOOL bMakeGif = DoMakeGif();
		CString sAVIFileName;
		CString sSWFFileName;
		CString sGIFFileName;
		CString sGIFTempFileName; // Store to temp and then move so that web browser will not load half saved gifs
		CString sJPGDir;
		CStringArray sJPGFileNames;
		CVideoDeviceDoc::CreateCheckYearMonthDayDir(FirstTime, sDetectionAutoSaveDir, sAVIFileName);
		sJPGDir = sGIFFileName = sSWFFileName = sAVIFileName;
		if (sAVIFileName == _T(""))
			sAVIFileName = _T("det_") + sFirstTime + _T(".avi");
		else
			sAVIFileName = sAVIFileName + _T("\\") + _T("det_") + sFirstTime + _T(".avi");
		if (sSWFFileName == _T(""))
			sSWFFileName = _T("det_") + sFirstTime + _T(".swf");
		else
			sSWFFileName = sSWFFileName + _T("\\") + _T("det_") + sFirstTime + _T(".swf");
		if (sGIFFileName == _T(""))
			sGIFFileName = _T("det_") + sFirstTime + _T(".gif");
		else
			sGIFFileName = sGIFFileName + _T("\\") + _T("det_") + sFirstTime + _T(".gif");
		sGIFTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sGIFFileName);
		if (sJPGDir != _T(""))
			sJPGDir = sJPGDir + _T("\\");

		// Create the Avi File
		CAVRec AVRecAvi;
		if (bMakeAvi)
		{
			AVRecAvi.Init(sAVIFileName, 0, true); // fast encoding!
			AVRecAvi.SetInfo(	_T("Det: ") + ::MakeDateLocalFormat(FirstTime) +
								_T(", ") + ::MakeTimeLocalFormat(FirstTime, TRUE),
								APPNAME_NOEXT, MYCOMPANY_WEB);
		}

		// Create the Swf File
		CAVRec AVRecSwf;
		if (bMakeSwf)
			AVRecSwf.Init(sSWFFileName, 0, true); // fast encoding!

		// Store the Frames
		POSITION nextpos = m_pFrameList->GetHeadPosition();
		POSITION currentpos;
		int nFrames = m_nNumFramesToSave;
		double dDelayMul = 1.0;
		double dSpeedMul = 1.0;
		CDib AVISaveDib;
		CDib SWFSaveDib;
		CDib GIFSaveDib;
		CDib JPGSaveDib;
		CDib* pDibPrev;
		CDib* pDib = NULL;
		BOOL bFirstGIFSave;
		RGBQUAD* pGIFColors = NULL;
		double dCalcFrameRate = 1.0;
		if (nFrames > 1)
			dCalcFrameRate = (1000.0 * (nFrames - 1)) / (double)(dwLastUpTime - dwFirstUpTime);
		if (m_pDoc->m_dEffectiveFrameRate > 0.0)
		{
			if (dCalcFrameRate / m_pDoc->m_dEffectiveFrameRate < MOVDET_SAVE_MIN_FRAMERATE_RATIO)
				dCalcFrameRate = m_pDoc->m_dEffectiveFrameRate;
		}
		AVRational CalcFrameRate = av_d2q(dCalcFrameRate, MAX_SIZE_FOR_RATIONAL);
		while (!m_pFrameList->IsEmpty() && nextpos && nFrames)
		{
			// Shutdown?
			if (DoExit())
			{
				m_pDoc->RemoveOldestMovementDetectionList();
				if (pGIFColors)
					delete [] pGIFColors;
				AVRecAvi.Close();
				::DeleteFile(sAVIFileName);
				GIFSaveDib.GetGif()->Close();
				::DeleteFile(sGIFTempFileName);
				AVRecSwf.Close();
				::DeleteFile(sSWFFileName);
				for (int i = 0 ; i < sJPGFileNames.GetSize() ; i++)
					::DeleteFile(sJPGFileNames[i]);
				m_bWorking = FALSE;
				if (::IsExistingDir(sTempDetectionDir))
					::DeleteDir(sTempDetectionDir);
				return 0;
			}

			// Get Frame
			currentpos = nextpos;
			pDib = m_pFrameList->GetNext(nextpos);
			DecodeFrame(pDib);

			// Calc. detection lists size
			if ((nFrames % MOVDET_MIN_FRAMES_IN_LIST) == 0)
			{
				::EnterCriticalSection(&m_pDoc->m_csMovementDetectionsList);
				CalcMovementDetectionListsSize();
				::LeaveCriticalSection(&m_pDoc->m_csMovementDetectionsList);
			}

			// Swf
			if (bMakeSwf)
			{
				SWFSaveDib = *pDib;

				// Add Frame Tags
				if (m_pDoc->m_bShowFrameTime)
				{
					AddFrameTime(&SWFSaveDib, RefTime, dwRefUpTime, m_pDoc->m_nRefFontSize);
					AddFrameCount(&SWFSaveDib, m_pDoc->m_nMovDetSavesCount, m_pDoc->m_nRefFontSize);
				}

				// Open
				if (!AVRecSwf.IsOpen())
				{
					BITMAPINFOHEADER DstBmi;
					memset(&DstBmi, 0, sizeof(BITMAPINFOHEADER));
					DstBmi.biSize = sizeof(BITMAPINFOHEADER);
					DstBmi.biWidth = SWFSaveDib.GetWidth();
					DstBmi.biHeight = SWFSaveDib.GetHeight();
					DstBmi.biPlanes = 1;
					DstBmi.biCompression = m_pDoc->m_dwVideoDetSwfFourCC;		// FLV1, need Flash 6 to play it
					int nQualityBitrate = m_pDoc->m_nVideoDetSwfQualityBitrate;
					AVRecSwf.AddVideoStream(SWFSaveDib.GetBMI(),				// Source Video Format
											(LPBITMAPINFO)(&DstBmi),			// Destination Video Format
											CalcFrameRate.num,					// Rate
											CalcFrameRate.den,					// Scale
											nQualityBitrate == 1 ? m_pDoc->m_nVideoDetSwfDataRate : 0,		// Bitrate in bits/s
											m_pDoc->m_nVideoDetSwfKeyframesRate,// Keyframes Rate				
											nQualityBitrate == 0 ? m_pDoc->m_fVideoDetSwfQuality : 0.0f);	// 0.0f use bitrate, 2.0f best quality, 31.0f worst quality
					if (m_pDoc->m_bCaptureAudio)
					{	
						// Swf only supports mp3 with sample rates of 44100 Hz, 22050 Hz and 11025 Hz
						WAVEFORMATEX SwfWaveFormat;
						memset(&SwfWaveFormat, 0, sizeof(WAVEFORMATEX));
						SwfWaveFormat.wFormatTag = WAVE_FORMAT_MPEGLAYER3;
						if (m_pDoc->m_CaptureAudioThread.m_pSrcWaveFormat->nSamplesPerSec >= 44100)
						{
							SwfWaveFormat.nSamplesPerSec = 44100;
							if (m_pDoc->m_CaptureAudioThread.m_pSrcWaveFormat->nChannels >= 2)
							{
								SwfWaveFormat.nChannels = 2;
								SwfWaveFormat.nAvgBytesPerSec = (96000 / 8);
							}
							else
							{
								SwfWaveFormat.nChannels = 1;
								SwfWaveFormat.nAvgBytesPerSec = (56000 / 8);
							}
						}
						else if (m_pDoc->m_CaptureAudioThread.m_pSrcWaveFormat->nSamplesPerSec >= 22050)
						{
							SwfWaveFormat.nSamplesPerSec = 22050;
							if (m_pDoc->m_CaptureAudioThread.m_pSrcWaveFormat->nChannels >= 2)
							{
								SwfWaveFormat.nChannels = 2;
								SwfWaveFormat.nAvgBytesPerSec = (56000 / 8);
							}
							else
							{
								SwfWaveFormat.nChannels = 1;
								SwfWaveFormat.nAvgBytesPerSec = (32000 / 8);
							}
						}
						else
						{
							SwfWaveFormat.nSamplesPerSec = 11025;
							if (m_pDoc->m_CaptureAudioThread.m_pSrcWaveFormat->nChannels >= 2)
							{
								SwfWaveFormat.nChannels = 2;
								SwfWaveFormat.nAvgBytesPerSec = (32000 / 8);
							}
							else
							{
								SwfWaveFormat.nChannels = 1;
								SwfWaveFormat.nAvgBytesPerSec = (24000 / 8);
							}
						}
						AVRecSwf.AddAudioStream(m_pDoc->m_CaptureAudioThread.m_pSrcWaveFormat,	// Src Wave Format
												&SwfWaveFormat);	// Dst Wave Format
					}
					AVRecSwf.Open();
				}

				if (AVRecSwf.IsOpen())
				{
					// Add Frame
					AVRecSwf.AddFrame(	AVRecSwf.VideoStreamNumToStreamNum(ACTIVE_VIDEO_STREAM),
										&SWFSaveDib,
										false);	// No interleave

					// Add Audio Samples
					if (m_pDoc->m_bCaptureAudio)
					{
						POSITION posUserBuf = pDib->m_UserList.GetHeadPosition();
						while (posUserBuf)
						{
							CUserBuf UserBuf = pDib->m_UserList.GetNext(posUserBuf);
							int nNumOfSrcSamples = (m_pDoc->m_CaptureAudioThread.m_pSrcWaveFormat && (m_pDoc->m_CaptureAudioThread.m_pSrcWaveFormat->nBlockAlign > 0)) ? UserBuf.m_dwSize / m_pDoc->m_CaptureAudioThread.m_pSrcWaveFormat->nBlockAlign : 0;
							AVRecSwf.AddAudioSamples(	AVRecSwf.AudioStreamNumToStreamNum(ACTIVE_AUDIO_STREAM),
														nNumOfSrcSamples,
														UserBuf.m_pBuf,
														false);	// No interleave
						}
					}
				}
			}

			// Avi
			if (bMakeAvi)
			{
				AVISaveDib = *pDib;

				// Add Frame Tags
				if (m_pDoc->m_bShowFrameTime)
				{
					AddFrameTime(&AVISaveDib, RefTime, dwRefUpTime, m_pDoc->m_nRefFontSize);
					AddFrameCount(&AVISaveDib, m_pDoc->m_nMovDetSavesCount, m_pDoc->m_nRefFontSize);
				}

				// Open
				if (!AVRecAvi.IsOpen())
				{
					BITMAPINFOFULL DstBmi;
					memset(&DstBmi, 0, sizeof(BITMAPINFOFULL));
					DWORD dwVideoDetFourCC = m_pDoc->m_dwVideoDetFourCC;
					if (dwVideoDetFourCC == BI_RGB)
						memcpy(&DstBmi, AVISaveDib.GetBMI(), AVISaveDib.GetBMISize());
					else
					{
						DstBmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
						DstBmi.bmiHeader.biWidth = AVISaveDib.GetWidth();
						DstBmi.bmiHeader.biHeight = AVISaveDib.GetHeight();
						DstBmi.bmiHeader.biPlanes = 1;
						DstBmi.bmiHeader.biCompression = dwVideoDetFourCC;
					}
					int nQualityBitrate = m_pDoc->m_nVideoDetQualityBitrate;
					if (DstBmi.bmiHeader.biCompression == FCC('MJPG'))
						nQualityBitrate = 0;
					AVRecAvi.AddVideoStream(AVISaveDib.GetBMI(),					// Source Video Format
											(LPBITMAPINFO)(&DstBmi),				// Destination Video Format
											CalcFrameRate.num,						// Rate
											CalcFrameRate.den,						// Scale
											nQualityBitrate == 1 ? m_pDoc->m_nVideoDetDataRate : 0,		// Bitrate in bits/s
											m_pDoc->m_nVideoDetKeyframesRate,		// Keyframes Rate					
											nQualityBitrate == 0 ? m_pDoc->m_fVideoDetQuality : 0.0f);	// 0.0f use bitrate, 2.0f best quality, 31.0f worst quality
					if (m_pDoc->m_bCaptureAudio)
					{
						AVRecAvi.AddAudioStream(m_pDoc->m_CaptureAudioThread.m_pSrcWaveFormat,	// Src Wave Format
												m_pDoc->m_CaptureAudioThread.m_pDstWaveFormat);	// Dst Wave Format
					}
					AVRecAvi.Open();
				}

				if (AVRecAvi.IsOpen())
				{
					// Add Frame
					AVRecAvi.AddFrame(	AVRecAvi.VideoStreamNumToStreamNum(ACTIVE_VIDEO_STREAM),
										&AVISaveDib,
										false);	// No interleave

					// Add Audio Samples
					if (m_pDoc->m_bCaptureAudio)
					{
						POSITION posUserBuf = pDib->m_UserList.GetHeadPosition();
						while (posUserBuf)
						{
							CUserBuf UserBuf = pDib->m_UserList.GetNext(posUserBuf);
							if (m_pDoc->m_CaptureAudioThread.m_pDstWaveFormat->wFormatTag == WAVE_FORMAT_PCM)
							{
								AVRecAvi.AddRawAudioPacket(	AVRecAvi.AudioStreamNumToStreamNum(ACTIVE_AUDIO_STREAM),
															UserBuf.m_dwSize,
															UserBuf.m_pBuf,
															false);	// No interleave
							}
							else
							{
								int nNumOfSrcSamples = (m_pDoc->m_CaptureAudioThread.m_pSrcWaveFormat && (m_pDoc->m_CaptureAudioThread.m_pSrcWaveFormat->nBlockAlign > 0)) ? UserBuf.m_dwSize / m_pDoc->m_CaptureAudioThread.m_pSrcWaveFormat->nBlockAlign : 0;
								AVRecAvi.AddAudioSamples(	AVRecAvi.AudioStreamNumToStreamNum(ACTIVE_AUDIO_STREAM),
															nNumOfSrcSamples,
															UserBuf.m_pBuf,
															false);	// No interleave
							}
						}
					}
				}
			}

			// Jpeg
			if (bMakeJpeg)
			{
				// FIRST movement frame
				if (sJPGFileNames.GetSize() == 0)
				{
					if (pDib->GetUserFlag() & FRAME_USER_FLAG_MOTION)
					{
						JPGSaveDib = *pDib;
						CString sJPGFileName = SaveJpeg(&JPGSaveDib, sJPGDir, RefTime, dwRefUpTime);
						if (sJPGFileName != _T(""))
							sJPGFileNames.Add(sJPGFileName);
					}
				}
				// MIDDLE frame
				// Note: SaveJpeg() returns _T("") if a file already exists
				// -> maximum one frame per second
				else if (sJPGFileNames.GetSize() == 1)
				{
					if (nFrames <= m_nNumFramesToSave / 2)
					{
						JPGSaveDib = *pDib;
						CString sJPGFileName = SaveJpeg(&JPGSaveDib, sJPGDir, RefTime, dwRefUpTime);
						if (sJPGFileName != _T(""))
							sJPGFileNames.Add(sJPGFileName);
					}
				}
				// LAST frame
				// Note: SaveJpeg() returns _T("") if a file already exists
				// -> maximum one frame per second
				if (nFrames == 1)
				{
					JPGSaveDib = *pDib;
					CString sJPGFileName = SaveJpeg(&JPGSaveDib, sJPGDir, RefTime, dwRefUpTime);
					if (sJPGFileName != _T(""))
						sJPGFileNames.Add(sJPGFileName);
				}	
			}

			// Animated Gif, because of differencing the saving is shifted by one frame
			if (bMakeGif)
			{
				// First Frame?
				if (nFrames == m_nNumFramesToSave)
				{
					bFirstGIFSave = TRUE;
					pDibPrev = pDib;
					ASSERT(pGIFColors == NULL);
					pGIFColors = (RGBQUAD*)new RGBQUAD[256];
					AnimatedGifInit(pGIFColors,
									dDelayMul,		// Sets this
									dSpeedMul,		// Sets this
									dCalcFrameRate,
									RefTime,
									dwRefUpTime);
				}
				// Next Frame?
				else
				{
					SaveAnimatedGif(&GIFSaveDib,
									pDib,
									&pDibPrev,
									sGIFTempFileName,
									&bFirstGIFSave,	// First Frame To Save?
									nFrames == 1,	// Last Frame To Save?
									dDelayMul,
									dSpeedMul,
									pGIFColors,
									MOVDET_ANIMGIF_DIFF_MINLEVEL,
									RefTime,
									dwRefUpTime);
				}

				// Free unused memory
				if (pDib != pDibPrev)
				{
					delete pDib;
					m_pFrameList->SetAt(currentpos, NULL);
				}
			}
			// Free memory
			else
			{
				delete pDib;
				m_pFrameList->SetAt(currentpos, NULL);
			}

			// Dec. Frame Count
			nFrames--;

			// Update save progress
			m_nSaveProgress = (int)((m_nNumFramesToSave - nFrames) * 100.0 / m_nNumFramesToSave);
		}

		// Save single gif image if nothing done above:
		// this happens if m_nNumFramesToSave is 1
		if (bMakeGif && !::IsExistingFile(sGIFTempFileName))
		{
			SaveSingleGif(	m_pFrameList->GetHead(),
							sGIFTempFileName,
							pGIFColors,
							RefTime,
							dwRefUpTime);
		}

		// Clean-Up
		if (pGIFColors)
			delete [] pGIFColors;
		GIFSaveDib.GetGif()->Close();
		AVRecAvi.Close();
		AVRecSwf.Close();

		// Rename Saved Gif File
		::DeleteFile(sGIFFileName);
		::MoveFile(sGIFTempFileName, sGIFFileName);

		// Free
		m_pDoc->RemoveOldestMovementDetectionList();

		// SendMail and/or FTPUpload?
		// (this function returns FALSE if we have to exit the thread)
		DWORD dwMailFTPTimeMs = ::timeGetTime();
		if (!SendMailFTPUpload(FirstTime, sAVIFileName, sGIFFileName, sSWFFileName, sJPGFileNames))
		{
			// Delete Files if not wanted
			if (!m_pDoc->m_bSaveAVIMovementDetection)
				::DeleteFile(sAVIFileName);
			if (!m_pDoc->m_bSaveAnimGIFMovementDetection)
				::DeleteFile(sGIFFileName);
			if (!m_pDoc->m_bSaveSWFMovementDetection)
				::DeleteFile(sSWFFileName);
			for (int i = 0 ; i < sJPGFileNames.GetSize() ; i++)
				::DeleteFile(sJPGFileNames[i]);
			m_bWorking = FALSE;
			if (::IsExistingDir(sTempDetectionDir))
				::DeleteDir(sTempDetectionDir);
			return 0;
		}
		dwMailFTPTimeMs = ::timeGetTime() - dwMailFTPTimeMs;

		// Execute Command After Save
		if (m_pDoc->m_bExecCommandMovementDetection && m_pDoc->m_nExecModeMovementDetection == 1)
		{
			m_pDoc->ExecCommandMovementDetection(	TRUE, FirstTime,
													sAVIFileName, sGIFFileName, sSWFFileName,
													m_pDoc->m_nMovDetSavesCount);
		}

		// Delete Files if not wanted
		if (!m_pDoc->m_bSaveAVIMovementDetection)
			::DeleteFile(sAVIFileName);
		if (!m_pDoc->m_bSaveAnimGIFMovementDetection)
			::DeleteFile(sGIFFileName);
		if (!m_pDoc->m_bSaveSWFMovementDetection)
			::DeleteFile(sSWFFileName);
		for (int i = 0 ; i < sJPGFileNames.GetSize() ; i++)
			::DeleteFile(sJPGFileNames[i]);

		// Increment if detection sequence done and store settings
		if (bDetectionSequenceDone)
			m_pDoc->m_nMovDetSavesCount++;
		::AfxGetApp()->WriteProfileInt(sSection, _T("MovDetSavesCount"), m_pDoc->m_nMovDetSavesCount);
		::AfxGetApp()->WriteProfileInt(sSection, _T("MovDetSavesCountDay"), m_pDoc->m_nMovDetSavesCountDay);
		::AfxGetApp()->WriteProfileInt(sSection, _T("MovDetSavesCountMonth"), m_pDoc->m_nMovDetSavesCountMonth);
		::AfxGetApp()->WriteProfileInt(sSection, _T("MovDetSavesCountYear"), m_pDoc->m_nMovDetSavesCountYear);

		// Save time calculation
		DWORD dwSaveTimeMs = ::timeGetTime() - dwStartUpTime;
		DWORD dwFramesTimeMs = dwLastUpTime - dwFirstUpTime;
		if (dwFramesTimeMs >= 2000U) // Check only if at least 2 sec of frames
		{
			CString sMsg;
			if (dwFramesTimeMs < dwSaveTimeMs)
			{
				sMsg.Format(_T("%s, attention cannot realtime save the detections: SaveTime=%0.1fs > FramesTime=%0.1fs (MailFTP=%0.1fs)\n"),
							m_pDoc->GetAssignedDeviceName(), (double)dwSaveTimeMs / 1000.0, (double)dwFramesTimeMs / 1000.0,
							(double)dwMailFTPTimeMs / 1000.0);
				TRACE(sMsg);
				::LogLine(sMsg);
			}
			else if (m_pDoc->m_nDetectionLevel == 100)
			{
				sMsg.Format(_T("%s, realtime saving the detections is ok: SaveTime=%0.1fs < FramesTime=%0.1fs (MailFTP=%0.1fs)\n"),
							m_pDoc->GetAssignedDeviceName(), (double)dwSaveTimeMs / 1000.0, (double)dwFramesTimeMs / 1000.0,
							(double)dwMailFTPTimeMs / 1000.0);
				TRACE(sMsg);
				::LogLine(sMsg);
			}
		}
	}
	ASSERT(FALSE); // should never end up here...
	m_bWorking = FALSE;
	if (::IsExistingDir(sTempDetectionDir))
		::DeleteDir(sTempDetectionDir);
	return 0;
}

BOOL CVideoDeviceDoc::CSaveFrameListThread::SendMailFTPUpload(	const CTime& Time,
																const CString& sAVIFileName,
																const CString& sGIFFileName,
																const CString& sSWFFileName,
																const CStringArray& sJPGFileNames)
{
	// Send By E-Mail
	if (m_pDoc->m_bSendMailMovementDetection &&
		!SendMailMovementDetection(Time, sAVIFileName, sGIFFileName, sJPGFileNames))
		return FALSE;

	// FTP Upload
	if (m_pDoc->m_bFTPUploadMovementDetection &&
		!FTPUploadMovementDetection(Time, sAVIFileName, sGIFFileName, sSWFFileName))
		return FALSE;

	return TRUE;
}

__forceinline BOOL CVideoDeviceDoc::CSaveFrameListThread::SendMailMovementDetection(	const CTime& Time,
																						const CString& sAVIFileName,
																						const CString& sGIFFileName,
																						const CStringArray& sJPGFileNames)
{
	CString sSubject(_T("Movement Detection: ") + m_pDoc->GetAssignedDeviceName() + _T(" on ") +
				::MakeDateLocalFormat(Time) + _T(" at ") + ::MakeTimeLocalFormat(Time, TRUE));
	m_pDoc->m_MovDetSendMailConfiguration.m_sSubject = sSubject;
	CStringArray sFileNames;
	switch (m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType)
	{
		case CVideoDeviceDoc::ATTACHMENT_AVI :
				sFileNames.Add(sAVIFileName);
				break;

		case CVideoDeviceDoc::ATTACHMENT_GIF :
				sFileNames.Add(sGIFFileName);
				break;

		case CVideoDeviceDoc::ATTACHMENT_JPG :
				sFileNames.Append(sJPGFileNames);
				break;

		case CVideoDeviceDoc::ATTACHMENT_GIF_AVI :
				sFileNames.Add(sGIFFileName);
				sFileNames.Add(sAVIFileName);
				break;

		case CVideoDeviceDoc::ATTACHMENT_JPG_AVI :
				sFileNames.Append(sJPGFileNames);
				sFileNames.Add(sAVIFileName);
				break;
		
		case CVideoDeviceDoc::ATTACHMENT_GIF_JPG :
				sFileNames.Add(sGIFFileName);	
				sFileNames.Append(sJPGFileNames);
				break;

		case CVideoDeviceDoc::ATTACHMENT_GIF_JPG_AVI :
				sFileNames.Add(sGIFFileName);
				sFileNames.Append(sJPGFileNames);
				sFileNames.Add(sAVIFileName);
				break;

		default :
				break;
	}

	// Do Exit?
	if (SendMail(sFileNames) == -1)
	{
		m_nSendMailProgress = 100; // hide progress display
		return FALSE;
	}
	else
	{
		m_nSendMailProgress = 100; // hide progress display
		return TRUE;
	}
}

__forceinline BOOL CVideoDeviceDoc::CSaveFrameListThread::FTPUploadMovementDetection(	const CTime& Time,
																						const CString& sAVIFileName,
																						const CString& sGIFFileName,
																						const CString& sSWFFileName)
{
	// Upload Directory
	CString sUploadDir = Time.Format(_T("%Y")) + _T("/") + Time.Format(_T("%m")) + _T("/") + Time.Format(_T("%d"));

	int result = -1;
	CSaveFrameListFTPTransfer FTP(this);
	switch (m_pDoc->m_MovDetFTPUploadConfiguration.m_FilesToUpload)
	{
		case CVideoDeviceDoc::FILES_TO_UPLOAD_AVI :
				result = m_pDoc->FTPUpload(	&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
											sAVIFileName, sUploadDir + _T("/") + ::GetShortFileName(sAVIFileName));
				break;

		case CVideoDeviceDoc::FILES_TO_UPLOAD_GIF :
				result = m_pDoc->FTPUpload(	&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
											sGIFFileName, sUploadDir + _T("/") + ::GetShortFileName(sGIFFileName));
				break;

		case CVideoDeviceDoc::FILES_TO_UPLOAD_SWF :
				result = m_pDoc->FTPUpload(	&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
											sSWFFileName, sUploadDir + _T("/") + ::GetShortFileName(sSWFFileName));
				break;

		case CVideoDeviceDoc::FILES_TO_UPLOAD_AVI_GIF :
				result = m_pDoc->FTPUpload(	&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
											sAVIFileName, sUploadDir + _T("/") + ::GetShortFileName(sAVIFileName));
				if (result == 1)
					result = m_pDoc->FTPUpload(	&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
												sGIFFileName, sUploadDir + _T("/") + ::GetShortFileName(sGIFFileName));
				break;

		case CVideoDeviceDoc::FILES_TO_UPLOAD_SWF_GIF :
				result = m_pDoc->FTPUpload(	&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
											sSWFFileName, sUploadDir + _T("/") + ::GetShortFileName(sSWFFileName));
				if (result == 1)
					result = m_pDoc->FTPUpload(	&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
												sGIFFileName, sUploadDir + _T("/") + ::GetShortFileName(sGIFFileName));
				break;

		case CVideoDeviceDoc::FILES_TO_UPLOAD_AVI_SWF_GIF :
				result = m_pDoc->FTPUpload(	&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
											sAVIFileName, sUploadDir + _T("/") + ::GetShortFileName(sAVIFileName));
				if (result == 1)
					result = m_pDoc->FTPUpload(	&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
												sSWFFileName, sUploadDir + _T("/") + ::GetShortFileName(sSWFFileName));
				if (result == 1)
					result = m_pDoc->FTPUpload(	&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
												sGIFFileName, sUploadDir + _T("/") + ::GetShortFileName(sGIFFileName));
				break;

		default :
				break;
	}

	// Hide progress display
	m_nFTPUploadProgress = 100;

	// Do Exit?
	if (result == -1)
		return FALSE;
	else
		return TRUE;
}

CString CVideoDeviceDoc::CSaveFrameListThread::SaveJpeg(CDib* pDib,
														CString sJPGDir,
														const CTime& RefTime,
														DWORD dwRefUpTime)
{
	// Calc. time and create file name
	CTime Time = CalcTime(pDib->GetUpTime(), RefTime, dwRefUpTime);
	CString sTime(Time.Format(_T("%Y_%m_%d_%H_%M_%S")));
	sJPGDir += _T("det_") + sTime + _T(".jpg");

	// Do not overwrite previous jpeg save
	if (::IsExistingFile(sJPGDir))
		return _T("");

	// Add frame tags
	if (m_pDoc->m_bShowFrameTime)
	{
		AddFrameTime(pDib, RefTime, dwRefUpTime, m_pDoc->m_nRefFontSize);
		AddFrameCount(pDib, m_pDoc->m_nMovDetSavesCount, m_pDoc->m_nRefFontSize);
	}

	// Save
	CMJPEGEncoder MJPEGEncoder;
	if (CVideoDeviceDoc::SaveJpegFast(pDib, &MJPEGEncoder, sJPGDir, DEFAULT_JPEGCOMPRESSION))
		return sJPGDir;
	else
		return _T("");
}

void CVideoDeviceDoc::CSaveFrameListThread::AnimatedGifInit(	RGBQUAD* pGIFColors,
																double& dDelayMul,
																double& dSpeedMul,
																double dCalcFrameRate,
																const CTime& RefTime,
																DWORD dwRefUpTime)
{
	// Check
	if (!pGIFColors)
		return;

	unsigned int line;
	CDib* pDibForPalette1 = NULL;
	CDib* pDibForPalette2 = NULL;
	CDib* pDibForPalette3 = NULL;
	CDib DibForPalette1;
	CDib DibForPalette2;
	CDib DibForPalette3;
	CDib DibForPalette;
	DibForPalette1.SetShowMessageBoxOnError(FALSE);
	DibForPalette2.SetShowMessageBoxOnError(FALSE);
	DibForPalette3.SetShowMessageBoxOnError(FALSE);
	DibForPalette.SetShowMessageBoxOnError(FALSE);

	// Get frame 1 sec after first movement, get middle frame
	// and get last movement frame for a optimized palette creation
	int nMiddleElementCount = m_nNumFramesToSave / 2;
	POSITION posPalette = m_pFrameList->GetHeadPosition();
	int nFrameCountDown = m_nNumFramesToSave;
	int nFirstCountDown = Round(dCalcFrameRate);
	while (nFrameCountDown > 0)
	{
		CDib* p = m_pFrameList->GetNext(posPalette);
		if (p)
		{
			if (m_nNumFramesToSave <= 3)
			{
				if (!pDibForPalette1)
					pDibForPalette1 = p;
				else if (!pDibForPalette2)
					pDibForPalette2 = p;
				else if (!pDibForPalette3)
					pDibForPalette3 = p;
			}
			else
			{
				if (!pDibForPalette1 && (p->GetUserFlag() & FRAME_USER_FLAG_MOTION))
					pDibForPalette1 = p;
				if (pDibForPalette1 && (nFirstCountDown > 0))
				{
					nFirstCountDown--;
					pDibForPalette1 = p;
				}
				if (nMiddleElementCount-- == 0)
					pDibForPalette2 = p;
				if (p->GetUserFlag() & FRAME_USER_FLAG_MOTION)
					pDibForPalette3 = p;
			}
		}
		nFrameCountDown--;
	}
	
	// Make sure the 3 image pointers are ok
	if (!pDibForPalette1)
		pDibForPalette1 = m_pFrameList->GetHead();
	DecodeFrame(pDibForPalette1);
	DibForPalette1 = *pDibForPalette1;
	if (DibForPalette1.IsCompressed() || DibForPalette1.GetBitCount() <= 8)
		DibForPalette1.Decompress(32);
	DibForPalette1.StretchBits(m_pDoc->m_dwAnimatedGifWidth, m_pDoc->m_dwAnimatedGifHeight);
	if (!pDibForPalette2)
		pDibForPalette2 = m_pFrameList->GetHead();
	DecodeFrame(pDibForPalette2);
	DibForPalette2 = *pDibForPalette2;
	if (DibForPalette2.IsCompressed() || DibForPalette2.GetBitCount() <= 8)
		DibForPalette2.Decompress(32);
	DibForPalette2.StretchBits(m_pDoc->m_dwAnimatedGifWidth, m_pDoc->m_dwAnimatedGifHeight);
	if (!pDibForPalette3)
		pDibForPalette3 = m_pFrameList->GetHead();
	DecodeFrame(pDibForPalette3);
	DibForPalette3 = *pDibForPalette3;
	if (DibForPalette3.IsCompressed() || DibForPalette3.GetBitCount() <= 8)
		DibForPalette3.Decompress(32);
	DibForPalette3.StretchBits(m_pDoc->m_dwAnimatedGifWidth, m_pDoc->m_dwAnimatedGifHeight);
	
	// Dib for Palette Calculation
	DibForPalette.AllocateBitsFast(	DibForPalette1.GetBitCount(),
									DibForPalette1.GetCompression(),
									DibForPalette1.GetWidth(),
									3 * DibForPalette1.GetHeight(),
									DibForPalette1.GetCompression() == BI_BITFIELDS ?
									(RGBQUAD*)((LPBYTE)(DibForPalette1.GetBMI()) + DibForPalette1.GetBMIH()->biSize) :
									NULL);
	LPBYTE pSrcBits;
	LPBYTE pDstBits = DibForPalette.GetBits();
	int nScanLineSize = DWALIGNEDWIDTHBYTES(DibForPalette.GetWidth() * DibForPalette.GetBitCount());
	pSrcBits = DibForPalette1.GetBits();
	for (line = 0 ; line < DibForPalette1.GetHeight() ; line++)
	{
		memcpy(pDstBits, pSrcBits, nScanLineSize);
		pDstBits += nScanLineSize;
		pSrcBits += nScanLineSize;
	}
	pSrcBits = DibForPalette2.GetBits();
	for (line = 0 ; line < DibForPalette2.GetHeight() ; line++)
	{
		memcpy(pDstBits, pSrcBits, nScanLineSize);
		pDstBits += nScanLineSize;
		pSrcBits += nScanLineSize;
	}
	pSrcBits = DibForPalette3.GetBits();
	for (line = 0 ; line < DibForPalette3.GetHeight() ; line++)
	{
		memcpy(pDstBits, pSrcBits, nScanLineSize);
		pDstBits += nScanLineSize;
		pSrcBits += nScanLineSize;
	}

	// Add frame tags to include its colors
	if (m_pDoc->m_bShowFrameTime)
	{
		AddFrameTime(&DibForPalette, RefTime, dwRefUpTime, m_pDoc->m_nRefFontSize);
		AddFrameCount(&DibForPalette, m_pDoc->m_nMovDetSavesCount, m_pDoc->m_nRefFontSize);
	}
	
	// Calc. Palette
	CQuantizer Quantizer(239, 8); // 239 = 256 (8 bits colors) - 1 (transparency index) - 16 (vga palette)
	Quantizer.ProcessImage(&DibForPalette);
	Quantizer.SetColorTable(pGIFColors);
	
	// VGA Palette
	// Note: palette Entry 255 is the Transparency Index!
	int i;
	for (i = 0; i < 8; i++)
	{
		pGIFColors[i+239].rgbRed		= CDib::ms_StdColors[i].rgbRed;
		pGIFColors[i+239].rgbGreen		= CDib::ms_StdColors[i].rgbGreen;
		pGIFColors[i+239].rgbBlue		= CDib::ms_StdColors[i].rgbBlue;
		pGIFColors[i+239].rgbReserved	= 0;
	}
	for (i = 8; i < 16; i++)
	{
		pGIFColors[i+239].rgbRed		= CDib::ms_StdColors[248+i].rgbRed;
		pGIFColors[i+239].rgbGreen		= CDib::ms_StdColors[248+i].rgbGreen;
		pGIFColors[i+239].rgbBlue		= CDib::ms_StdColors[248+i].rgbBlue;
		pGIFColors[i+239].rgbReserved	= 0;
	}
	pGIFColors[255].rgbRed		= 255;
	pGIFColors[255].rgbGreen	= 255;
	pGIFColors[255].rgbBlue		= 255;
	pGIFColors[255].rgbReserved	= 0;
	
	// Limit the saved frames to around MOVDET_ANIMGIF_MAX_FRAMES and
	// the play length to around MOVDET_ANIMGIF_MAX_LENGTH ms.
	// Note: ie has problems with to many anim. gifs frames and also
	// with to many anim. gifs images per displayed page!
	double dLengthMs = (double)m_nNumFramesToSave / dCalcFrameRate * 1000.0;
	dSpeedMul = max(1.0, dLengthMs / MOVDET_ANIMGIF_MAX_LENGTH);
	if (m_nNumFramesToSave >= MOVDET_ANIMGIF_MAX_FRAMES)
	{
		double dSavedFrames = dLengthMs / MOVDET_ANIMGIF_DELAY;
		dDelayMul = max(1.0, dSavedFrames / (double)MOVDET_ANIMGIF_MAX_FRAMES);
	}
}

BOOL CVideoDeviceDoc::CSaveFrameListThread::SaveSingleGif(	CDib* pDib,
															const CString& sGIFFileName,
															RGBQUAD* pGIFColors,
															const CTime& RefTime,
															DWORD dwRefUpTime)
{
	if (pDib && pGIFColors)
	{
		// Make sure we have a true RGB format
		if (pDib->IsCompressed() || pDib->GetBitCount() <= 8)
			pDib->Decompress(32);

		// Resize
		pDib->StretchBits(m_pDoc->m_dwAnimatedGifWidth, m_pDoc->m_dwAnimatedGifHeight);

		// Add Frame Tags
		if (m_pDoc->m_bShowFrameTime)
		{
			AddFrameTime(pDib, RefTime, dwRefUpTime, m_pDoc->m_nRefFontSize);
			AddFrameCount(pDib, m_pDoc->m_nMovDetSavesCount, m_pDoc->m_nRefFontSize);
		}

		// Convert to 8 bpp
		if (pDib->GetBitCount() > 8)
		{
			pDib->CreatePaletteFromColors(256, pGIFColors); // Use all indexes for color!
			pDib->ConvertTo8bitsErrDiff(pDib->GetPalette());
		}

		// Save It
		pDib->GetGif()->SetBackgroundColorIndex(0);
		return pDib->SaveGIF(sGIFFileName,
							GIF_COLORINDEX_NOT_DEFINED,
							NULL,
							TRUE,
							this);
	}
	else
		return FALSE;
}

__forceinline void CVideoDeviceDoc::CSaveFrameListThread::To255Colors(	CDib* pDib,
																		RGBQUAD* pGIFColors,
																		const CTime& RefTime,
																		DWORD dwRefUpTime)
{
	// Make sure we have a true RGB format
	if (pDib->IsCompressed() || pDib->GetBitCount() <= 8)
		pDib->Decompress(32);

	// Resize
	pDib->StretchBits(m_pDoc->m_dwAnimatedGifWidth, m_pDoc->m_dwAnimatedGifHeight);

	// Add Frame Tags
	if (m_pDoc->m_bShowFrameTime)
	{
		AddFrameTime(pDib, RefTime, dwRefUpTime, m_pDoc->m_nRefFontSize);
		AddFrameCount(pDib, m_pDoc->m_nMovDetSavesCount, m_pDoc->m_nRefFontSize);
	}

	// Convert to 8 bpp
	if (pDib->GetBitCount() > 8)
	{
		pDib->CreatePaletteFromColors(255, pGIFColors); // One index for transparency!
		pDib->ConvertTo8bitsErrDiff(pDib->GetPalette());
	}
}

BOOL CVideoDeviceDoc::CSaveFrameListThread::SaveAnimatedGif(CDib* pGIFSaveDib,
															CDib* pGIFDib,
															CDib** ppGIFDibPrev,
															const CString& sGIFFileName,
															BOOL* pbFirstGIFSave,
															BOOL bLastGIFSave,
															double dDelayMul,
															double dSpeedMul,
															RGBQUAD* pGIFColors,
															int nDiffMinLevel,
															const CTime& RefTime,
															DWORD dwRefUpTime)
{
	BOOL res = FALSE;

	// Check
	if (!pGIFColors)
		return FALSE;

	// Is First Frame To Save?
	if (*pbFirstGIFSave)
	{
		// Convert to 255 colors
		To255Colors(*ppGIFDibPrev, pGIFColors, RefTime, dwRefUpTime);
		
		// Copy First Frame
		*pGIFSaveDib = **ppGIFDibPrev;

		// Set up GIF Vars
		pGIFSaveDib->GetGif()->SetConfig(TRUE,	// Screen Color Table
										FALSE,	// No Image Color Table
										TRUE,	// Graphic Extension (for Delay)
										TRUE);	// TRUE: See GIFSetLoopCount()
												// FALSE: No Loop Count -> Show all frames 1x
		pGIFSaveDib->GetGif()->SetRect(	0,
										0,
										pGIFSaveDib->GetWidth(),
										pGIFSaveDib->GetHeight());
		pGIFSaveDib->GetGif()->SetTransparency(TRUE);
		pGIFSaveDib->GetGif()->SetTransparencyColorIndex(255);
		pGIFSaveDib->GetGif()->SetDispose(GIF_DISPOSE_NONE);
		pGIFSaveDib->GetGif()->SetDelay(MOVDET_ANIMGIF_FIRST_FRAME_DELAY);
		// 0 means loop infinite
		// 1 means loop one time -> Show all frames 2x
		// 2 means loop two times -> Show all frames 3x
		pGIFSaveDib->GetGif()->SetLoopCount(0);
		res = pGIFSaveDib->SaveFirstGIF(sGIFFileName,
										NULL,
										TRUE,
										this);
		*pbFirstGIFSave = FALSE;
	
		// Update Pointer
		*ppGIFDibPrev = pGIFDib;

		// If it is also the last frame,
		// this happens if we have only 2 frames to save
		if (bLastGIFSave)
		{
			// Convert to 255 colors and save
			To255Colors(pGIFDib, pGIFColors, RefTime, dwRefUpTime);
			pGIFSaveDib->GetGif()->SetDispose(GIF_DISPOSE_RESTORE);
			pGIFSaveDib->GetGif()->SetDelay(MOVDET_ANIMGIF_LAST_FRAME_DELAY);
			pGIFDib->DiffTransp8(pGIFSaveDib, nDiffMinLevel, 255);
			res = pGIFSaveDib->SaveNextGIF(	pGIFDib,
											NULL,
											TRUE,
											this);
		}
	}
	// Is Last Frame?
	else if (bLastGIFSave)
	{
		// Convert to 255 colors and save
		To255Colors(*ppGIFDibPrev, pGIFColors, RefTime, dwRefUpTime);
		pGIFSaveDib->GetGif()->SetDispose(GIF_DISPOSE_RESTORE);
		pGIFSaveDib->GetGif()->SetDelay(MAX(100, Round((double)(pGIFDib->GetUpTime() - (*ppGIFDibPrev)->GetUpTime()) / dSpeedMul)));
		(*ppGIFDibPrev)->DiffTransp8(pGIFSaveDib, nDiffMinLevel, 255);
		res = pGIFSaveDib->SaveNextGIF(	*ppGIFDibPrev,
										NULL,
										TRUE,
										this);
		
		// Convert to 255 colors and save
		To255Colors(pGIFDib, pGIFColors, RefTime, dwRefUpTime);
		pGIFSaveDib->GetGif()->SetDispose(GIF_DISPOSE_RESTORE);
		pGIFSaveDib->GetGif()->SetDelay(MOVDET_ANIMGIF_LAST_FRAME_DELAY);
		pGIFDib->DiffTransp8(pGIFSaveDib, nDiffMinLevel, 255);
		res = pGIFSaveDib->SaveNextGIF(	pGIFDib,
										NULL,
										TRUE,
										this);
	}
	// Middle Frame?
	else if ((int)(pGIFDib->GetUpTime() - (*ppGIFDibPrev)->GetUpTime()) >= Round(dDelayMul * MOVDET_ANIMGIF_DELAY))
	{
		// Convert to 255 colors and save
		To255Colors(*ppGIFDibPrev, pGIFColors, RefTime, dwRefUpTime);
		pGIFSaveDib->GetGif()->SetDispose(GIF_DISPOSE_RESTORE);
		pGIFSaveDib->GetGif()->SetDelay(MAX(100, Round((double)(pGIFDib->GetUpTime() - (*ppGIFDibPrev)->GetUpTime()) / dSpeedMul)));
		(*ppGIFDibPrev)->DiffTransp8(pGIFSaveDib, nDiffMinLevel, 255);
		res = pGIFSaveDib->SaveNextGIF(	*ppGIFDibPrev,
										NULL,
										TRUE,
										this);

		// Update Pointer
		*ppGIFDibPrev = pGIFDib;
	}
	else
		res = TRUE; // Skip Frame

	return res;
}

int CVideoDeviceDoc::CSaveSnapshotSWFThread::Work()
{
	ASSERT(m_pDoc);

	// Init
	CDib Dib;
	Dib.SetShowMessageBoxOnError(FALSE);
	CAVRec* pAVRecSwf = NULL;
	CAVRec* pAVRecThumbSwf = NULL;
	CString sSWFFileName = MakeSwfHistoryFileName();
	CString sSWFThumbFileName = ::GetFileNameNoExt(sSWFFileName) + _T("_thumb.swf");
	CString sSWFTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sSWFFileName);
	CString sSWFTempThumbFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sSWFThumbFileName);

	// Find and process jpg snapshot history files
	CSortableFileFind FileFind;
	FileFind.AddAllowedExtension(_T("jpg"));
	if (FileFind.Init(::GetDriveAndDirName(sSWFFileName) + _T("\\") + _T("*")))
	{
		for (int pos = 0 ; pos < FileFind.GetFilesCount() ; pos++)
		{
			CString sShortFileNameNoExt(::GetShortFileNameNoExt(FileFind.GetFileName(pos)));
			sShortFileNameNoExt.MakeLower();
			if (sShortFileNameNoExt.Left(5) == _T("shot_"))
			{
				if (Dib.LoadJPEG(FileFind.GetFileName(pos)))
				{
					if (sShortFileNameNoExt.Right(6) != _T("_thumb"))
					{
						// Alloc
						if (!pAVRecSwf)
							pAVRecSwf = new CAVRec(sSWFTempFileName);
						if (pAVRecSwf)
						{
							// Open
							if (!pAVRecSwf->IsOpen())
							{
								AVRational FrameRate = av_d2q(m_dSnapshotHistoryFrameRate, MAX_SIZE_FOR_RATIONAL);
								BITMAPINFOHEADER DstBmi;
								memset(&DstBmi, 0, sizeof(BITMAPINFOHEADER));
								DstBmi.biSize = sizeof(BITMAPINFOHEADER);
								DstBmi.biWidth = Dib.GetWidth();
								DstBmi.biHeight = Dib.GetHeight();
								DstBmi.biPlanes = 1;
								DstBmi.biCompression = FCC('FLV1');								// FLV1, need Flash 6 to play it
								pAVRecSwf->AddVideoStream(	Dib.GetBMI(),						// Source Video Format
															(LPBITMAPINFO)(&DstBmi),			// Destination Video Format
															FrameRate.num,						// Rate
															FrameRate.den,						// Scale
															0,									// Not using bitrate
															DEFAULT_KEYFRAMESRATE,				// Keyframes Rate				
															m_fSnapshotVideoCompressorQuality);	// 0.0f use bitrate, 2.0f best quality, 31.0f worst quality
								pAVRecSwf->Open();
							}

							// Add Frame
							if (pAVRecSwf->IsOpen())
							{
								pAVRecSwf->AddFrame(pAVRecSwf->VideoStreamNumToStreamNum(ACTIVE_VIDEO_STREAM),
													&Dib,
													false);	// No interleave for Video only
							}
						}
					}
					else
					{
						// Alloc
						if (!pAVRecThumbSwf)
							pAVRecThumbSwf = new CAVRec(sSWFTempThumbFileName);
						if (pAVRecThumbSwf)
						{
							// Open
							if (!pAVRecThumbSwf->IsOpen())
							{
								AVRational FrameRate = av_d2q(m_dSnapshotHistoryFrameRate, MAX_SIZE_FOR_RATIONAL);
								BITMAPINFOHEADER DstBmi;
								memset(&DstBmi, 0, sizeof(BITMAPINFOHEADER));
								DstBmi.biSize = sizeof(BITMAPINFOHEADER);
								DstBmi.biWidth = Dib.GetWidth();
								DstBmi.biHeight = Dib.GetHeight();
								DstBmi.biPlanes = 1;
								DstBmi.biCompression = FCC('FLV1');									// FLV1, need Flash 6 to play it
								pAVRecThumbSwf->AddVideoStream(	Dib.GetBMI(),						// Source Video Format
																(LPBITMAPINFO)(&DstBmi),			// Destination Video Format
																FrameRate.num,						// Rate
																FrameRate.den,						// Scale
																0,									// Not using bitrate
																DEFAULT_KEYFRAMESRATE,				// Keyframes Rate				
																m_fSnapshotVideoCompressorQuality);	// 0.0f use bitrate, 2.0f best quality, 31.0f worst quality
								pAVRecThumbSwf->Open();
							}

							// Add Frame
							if (pAVRecThumbSwf->IsOpen())
							{
								pAVRecThumbSwf->AddFrame(pAVRecThumbSwf->VideoStreamNumToStreamNum(ACTIVE_VIDEO_STREAM),
														&Dib,
														false);	// No interleave for Video only
							}
						}
					}
				}

				// Delete unwanted history jpg files
				if (!m_bSnapshotHistoryJpeg)
					::DeleteFile(FileFind.GetFileName(pos));
			}

			// Do Exit?
			if (DoExit())
				goto exit;
		}
	}

	// Close temp file(s) by freeing CAVRec object(s)
	if (pAVRecSwf)
	{
		delete pAVRecSwf;
		pAVRecSwf = NULL;
	}
	if (pAVRecThumbSwf)
	{
		delete pAVRecThumbSwf;
		pAVRecThumbSwf = NULL;
	}

	// Copy from temp to snapshots folder
	if (::IsExistingFile(sSWFTempFileName))
		::CopyFile(sSWFTempFileName, sSWFFileName, FALSE);
	if (::IsExistingFile(sSWFTempThumbFileName))
		::CopyFile(sSWFTempThumbFileName, sSWFThumbFileName, FALSE);

	// Ftp upload
	if (m_bSnapshotHistorySwfFtp)
	{
		CString sYear = m_Time.Format(_T("%Y"));
		CString sMonth = m_Time.Format(_T("%m"));
		CString sDay = m_Time.Format(_T("%d"));
		CString sUploadDir(sYear + _T("/") + sMonth + _T("/") + sDay);
		CFTPTransfer FTP(this);
		if (::IsExistingFile(sSWFTempThumbFileName))
		{
			// Do Exit?
			if (m_pDoc->FTPUpload(	&FTP, &m_Config,
									sSWFTempThumbFileName,
									sUploadDir + _T("/") + ::GetShortFileName(sSWFThumbFileName)) == -1)
				goto exit;
		}
		if (::IsExistingFile(sSWFTempFileName))
		{
			// Do Exit?
			if (m_pDoc->FTPUpload(	&FTP, &m_Config,
									sSWFTempFileName,
									sUploadDir + _T("/") + ::GetShortFileName(sSWFFileName)) == -1)
				goto exit;
		}
	}

	// Set thread executed variable
	// (if thread is killed this var is not set, that's the correct behavior)
	m_ThreadExecutedForTime = m_Time;

	// Clean-up
exit:
	if (pAVRecSwf)
		delete pAVRecSwf;
	if (pAVRecThumbSwf)
		delete pAVRecThumbSwf;
	::DeleteFile(sSWFTempFileName);
	::DeleteFile(sSWFTempThumbFileName);

	return 0;
}

int CVideoDeviceDoc::CSaveSnapshotThread::Work() 
{
	ASSERT(m_pDoc);

	// Get uptime
	DWORD dwUpTime = m_Dib.GetUpTime();

	// Temp file names
	CString sTempFileName, sTempThumbFileName;

	// Init history file name, if m_bSnapshotHistoryJpeg is TRUE,
	// it creates also the year, month and day directories, otherwise
	// it just returns the file name without path
	CString sHistoryFileName = MakeJpegHistoryFileName();

	// Resize Thumb
	CDib DibThumb;
	if (m_bSnapshotThumb)
	{
		// No Message Box on Error
		DibThumb.SetShowMessageBoxOnError(FALSE);

		// Resize
		if (DibThumb.AllocateBitsFast(12, FCC('I420'), m_nSnapshotThumbWidth, m_nSnapshotThumbHeight))
		{
			CVideoDeviceDoc::ResizeFast(&m_Dib, &DibThumb);
			DibThumb.SetUpTime(m_Dib.GetUpTime());
		}
	}

	// Save Full-size to Temp
	if (m_bShowFrameTime)
		AddFrameTime(&m_Dib, m_Time, dwUpTime, m_nRefFontSize);
	sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sHistoryFileName);
	CVideoDeviceDoc::SaveJpegFast(&m_Dib, &m_MJPEGEncoder, sTempFileName, m_nSnapshotCompressionQuality);

	// Save Thumb to Temp
	if (m_bSnapshotThumb)
	{
		if (m_bShowFrameTime)
			AddFrameTime(&DibThumb, m_Time, dwUpTime, m_nRefFontSize);
		sTempThumbFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),
										::GetFileNameNoExt(sHistoryFileName) + _T("_thumb.jpg"));
		CVideoDeviceDoc::SaveJpegFast(&DibThumb, &m_MJPEGThumbEncoder, sTempThumbFileName, m_nSnapshotCompressionQuality);
	}
	
	// Copy from Temp and Ftp Upload
	// Note: always first copy/ftp upload full-size file then the
	// thumb version which links to the full-size in web interface!
	if (m_bSnapshotLiveJpeg)
	{
		CString sLiveFileName;
		if (m_sSnapshotAutoSaveDir != _T(""))
		{
			sLiveFileName = m_sSnapshotAutoSaveDir;
			sLiveFileName.TrimRight(_T('\\'));
			sLiveFileName = sLiveFileName + _T("\\") + m_sSnapshotLiveJpegName + _T(".jpg");
			::CopyFile(sTempFileName, sLiveFileName, FALSE);
		}
		else
			sLiveFileName = m_sSnapshotLiveJpegName + _T(".jpg");
		if (m_bSnapshotLiveJpegFtp) // FTP Upload
		{
			int result;
			CFTPTransfer FTP(this);
			result = m_pDoc->FTPUpload(	&FTP, &m_Config,
										sTempFileName, m_sSnapshotLiveJpegName + _T(".jpg"));
			if (result == -1) // Do Exit?
			{
				// Delete Temp
				if (sTempThumbFileName != _T(""))
					::DeleteFile(sTempThumbFileName);
				if (sTempFileName != _T(""))
					::DeleteFile(sTempFileName);
				return 0;
			}
		}
		if (m_bSnapshotThumb)
		{
			CString sLiveThumbFileName;
			if (m_sSnapshotAutoSaveDir != _T(""))
			{
				sLiveThumbFileName = m_sSnapshotAutoSaveDir;
				sLiveThumbFileName.TrimRight(_T('\\'));
				sLiveThumbFileName = sLiveThumbFileName + _T("\\") + m_sSnapshotLiveJpegThumbName + _T(".jpg");
				::CopyFile(sTempThumbFileName, sLiveThumbFileName, FALSE);
			}
			else
				sLiveThumbFileName = m_sSnapshotLiveJpegThumbName + _T(".jpg");
			if (m_bSnapshotLiveJpegFtp) // FTP Upload
			{
				int result;
				CFTPTransfer FTP(this);
				result = m_pDoc->FTPUpload(	&FTP, &m_Config,
											sTempThumbFileName, m_sSnapshotLiveJpegThumbName + _T(".jpg"));
				if (result == -1) // Do Exit?
				{
					// Delete Temp
					if (sTempThumbFileName != _T(""))
						::DeleteFile(sTempThumbFileName);
					if (sTempFileName != _T(""))
						::DeleteFile(sTempFileName);
					return 0;
				}
			}
		}
	}
	if (m_bSnapshotHistoryJpeg)
	{
		if (m_sSnapshotAutoSaveDir != _T(""))
			::CopyFile(sTempFileName, sHistoryFileName, FALSE);
		if (m_bSnapshotHistoryJpegFtp) // FTP Upload
		{
			CString sUploadDir = m_Time.Format(_T("%Y")) + _T("/") + m_Time.Format(_T("%m")) + _T("/") + m_Time.Format(_T("%d"));
			int result;
			CFTPTransfer FTP(this);
			result = m_pDoc->FTPUpload(	&FTP, &m_Config,
										sTempFileName, sUploadDir + _T("/") + ::GetShortFileName(sHistoryFileName));
			if (result == -1) // Do Exit?
			{
				// Delete Temp
				if (sTempThumbFileName != _T(""))
					::DeleteFile(sTempThumbFileName);
				if (sTempFileName != _T(""))
					::DeleteFile(sTempFileName);
				return 0;
			}
		}
		if (m_bSnapshotThumb)
		{
			CString sHistoryThumbFileName = ::GetFileNameNoExt(sHistoryFileName) + _T("_thumb.jpg");
			if (m_sSnapshotAutoSaveDir != _T(""))
				::CopyFile(sTempThumbFileName, sHistoryThumbFileName, FALSE);
			if (m_bSnapshotHistoryJpegFtp) // FTP Upload
			{
				CString sUploadDir = m_Time.Format(_T("%Y")) + _T("/") + m_Time.Format(_T("%m")) + _T("/") + m_Time.Format(_T("%d"));
				int result;
				CFTPTransfer FTP(this);
				result = m_pDoc->FTPUpload(	&FTP, &m_Config,
											sTempThumbFileName, sUploadDir + _T("/") + ::GetShortFileName(sHistoryThumbFileName));
				if (result == -1) // Do Exit?
				{
					// Delete Temp
					if (sTempThumbFileName != _T(""))
						::DeleteFile(sTempThumbFileName);
					if (sTempFileName != _T(""))
						::DeleteFile(sTempFileName);
					return 0;
				}
			}
		}
	}
	
	// Delete Temp
	if (sTempThumbFileName != _T(""))
		::DeleteFile(sTempThumbFileName);
	if (sTempFileName != _T(""))
		::DeleteFile(sTempFileName);

	return 0;
}

__forceinline CString CVideoDeviceDoc::CSaveSnapshotThread::MakeJpegHistoryFileName()
{
	CString sYearMonthDayDir(_T(""));

	// Snapshot time
	CString sTime = m_Time.Format(_T("%Y_%m_%d_%H_%M_%S"));

	// Adjust Directory Name
	CString sSnapshotDir = m_sSnapshotAutoSaveDir;
	sSnapshotDir.TrimRight(_T('\\'));

	// Create directory if necessary
	if (sSnapshotDir != _T("") && m_bSnapshotHistoryJpeg)
	{
		DWORD dwAttrib = ::GetFileAttributes(sSnapshotDir);
		if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
			::CreateDir(sSnapshotDir);
		if (!CVideoDeviceDoc::CreateCheckYearMonthDayDir(m_Time, sSnapshotDir, sYearMonthDayDir))
			return _T("");
	}

	// Return file name
	if (sYearMonthDayDir == _T(""))
		return _T("shot_") + sTime + _T(".jpg");
	else
		return sYearMonthDayDir + _T("\\") + _T("shot_") + sTime + _T(".jpg");
}

__forceinline CString CVideoDeviceDoc::CSaveSnapshotSWFThread::MakeSwfHistoryFileName()
{
	CString sYearMonthDayDir(_T(""));

	// Snapshots time
	CString sTime = m_Time.Format(_T("%Y_%m_%d"));

	// Adjust Directory Name
	CString sSnapshotDir = m_sSnapshotAutoSaveDir;
	sSnapshotDir.TrimRight(_T('\\'));

	// Create directory if necessary
	if (sSnapshotDir != _T(""))
	{
		DWORD dwAttrib = ::GetFileAttributes(sSnapshotDir);
		if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
			::CreateDir(sSnapshotDir);
		if (!CVideoDeviceDoc::CreateCheckYearMonthDayDir(m_Time, sSnapshotDir, sYearMonthDayDir))
			return _T("");
	}

	// Return file name
	if (sYearMonthDayDir == _T(""))
		return _T("shot_") + sTime + _T(".swf");
	else
		return sYearMonthDayDir + _T("\\") + _T("shot_") + sTime + _T(".swf");
}

CPJNSMTPMessage* CVideoDeviceDoc::CreateEmailMessage(SendMailConfigurationStruct* pSendMailConfiguration)
{
	// Check
	if (!pSendMailConfiguration)
		return NULL;

	// Create the message
	CPJNSMTPMessage* pMessage = new CPJNSMTPMessage;
	if (!pMessage)
		return NULL;
	CPJNSMTPBodyPart attachment;

	// Set the mime flag
	pMessage->SetMime(pSendMailConfiguration->m_bMime);

	// Set the charset of the message and all attachments
	pMessage->SetCharset(pSendMailConfiguration->m_sEncodingCharset);
	attachment.SetCharset(pSendMailConfiguration->m_sEncodingCharset);

	// Set the message priority
	pMessage->m_Priority = pSendMailConfiguration->m_Priority;

	// Setup the all the recipient types for this message,
	// valid separators between addresses are ',' or ';'
	pMessage->ParseMultipleRecipients(pSendMailConfiguration->m_sTo, pMessage->m_To);
	if (!pSendMailConfiguration->m_sCC.IsEmpty())
		pMessage->ParseMultipleRecipients(pSendMailConfiguration->m_sCC, pMessage->m_CC);
	if (!pSendMailConfiguration->m_sBCC.IsEmpty())
		pMessage->ParseMultipleRecipients(pSendMailConfiguration->m_sBCC, pMessage->m_BCC);
	if (!pSendMailConfiguration->m_sSubject.IsEmpty()) 
		pMessage->m_sSubject = pSendMailConfiguration->m_sSubject;
	if (!pSendMailConfiguration->m_sBody.IsEmpty())
	{
		if (pSendMailConfiguration->m_bHTML)
			pMessage->AddHTMLBody(pSendMailConfiguration->m_sBody, _T(""));
		else
			pMessage->AddTextBody(pSendMailConfiguration->m_sBody);
	}

	// Add the attachment(s) if necessary,
	// valid separators between attachments are ',' or ';'
	if (!pSendMailConfiguration->m_sFiles.IsEmpty()) 
		pMessage->AddMultipleAttachments(pSendMailConfiguration->m_sFiles);		

	// Setup the from address
	if (pSendMailConfiguration->m_sFromName.IsEmpty()) 
	{
		pMessage->m_From = pSendMailConfiguration->m_sFrom;
		//pMessage->m_ReplyTo = pSendMailConfiguration->m_sFrom; uncomment this if you want to send a Reply-To header
	}
	else 
	{
		CPJNSMTPAddress address(pSendMailConfiguration->m_sFromName, pSendMailConfiguration->m_sFrom);
		pMessage->m_From = address;
		//pMessage->m_ReplyTo = address; //uncomment this if you want to send a Reply-To header
	}

	pMessage->m_sXMailer = _T(""); //comment this line out if you want to send a X-Mailer header

	return pMessage;
}

BOOL CVideoDeviceDoc::CSaveFrameListSMTPConnection::OnSendProgress(DWORD dwCurrentBytes, DWORD dwTotalBytes)
{
	if (m_pThread)
	{
		m_bDoExit = m_pThread->DoExit();
		if (dwTotalBytes > 0)
			m_pThread->SetSendMailProgress(Round(dwCurrentBytes * 100.0 / dwTotalBytes));
		else
			m_pThread->SetSendMailProgress(100);
		if (m_bDoExit)
			Disconnect(FALSE);
		return (m_bDoExit == false);
	}
	else
		return TRUE;
}

void CVideoDeviceDoc::CSaveFrameListThread::SendMailMessage(const CString& sTempEmailFile, CVideoDeviceDoc::CSaveFrameListSMTPConnection& connection, CPJNSMTPMessage* pMessage)
{
	// Check
	ASSERT(pMessage);

	CString sHost;
	BOOL bSend = TRUE;
	if (m_pDoc->m_MovDetSendMailConfiguration.m_bDNSLookup)
	{
		if (pMessage->m_To.GetSize() == 0)
		{
			CString sMsg;
			sMsg.Format(_T("%s, at least one recipient must be specified to use the DNS lookup option\n"), m_pDoc->GetAssignedDeviceName());
			TRACE(sMsg);
			::LogLine(sMsg);
			bSend = FALSE;
		}
		else
		{
			CString sAddress(pMessage->m_To.ElementAt(0).m_sEmailAddress);
			int nAmpersand = sAddress.Find(_T("@"));
			if (nAmpersand == -1)
			{
				CString sMsg;
				sMsg.Format(_T("%s, unable to determine the domain for the email address %s\n"), m_pDoc->GetAssignedDeviceName(), sAddress);
				TRACE(sMsg);
				::LogLine(sMsg);
				bSend = FALSE;
			}
			else
			{
				// We just pick the first MX record found, other implementations could ask the user
				// or automatically pick the lowest priority record
				CString sDomain(sAddress.Right(sAddress.GetLength() - nAmpersand - 1));
				CStringArray servers;
				CWordArray priorities;
				if (!connection.MXLookup(sDomain, servers, priorities))
				{
					CString sMsg;
					sMsg.Format(_T("%s, unable to perform a DNS MX lookup for the domain %s, Error Code:%d\n"),
													m_pDoc->GetAssignedDeviceName(), sDomain, GetLastError());
					TRACE(sMsg);
					::LogLine(sMsg);
					bSend = FALSE;
				}
				else
					sHost = servers.GetAt(0);
			}
		}
	}
	else
		sHost = m_pDoc->m_MovDetSendMailConfiguration.m_sHost;

	// Connect and send the message
	if (bSend)
	{
		connection.SetBindAddress(m_pDoc->m_MovDetSendMailConfiguration.m_sBoundIP);
		if (m_pDoc->m_MovDetSendMailConfiguration.m_sUsername == _T("") &&
			m_pDoc->m_MovDetSendMailConfiguration.m_sPassword == _T(""))
		{
			connection.Connect(	sHost,
								CPJNSMTPConnection::AUTH_NONE,
								m_pDoc->m_MovDetSendMailConfiguration.m_sUsername,
								m_pDoc->m_MovDetSendMailConfiguration.m_sPassword,
								m_pDoc->m_MovDetSendMailConfiguration.m_nPort
#ifndef CPJNSMTP_NOSSL
								, m_pDoc->m_MovDetSendMailConfiguration.m_ConnectionType
#endif
								);
		}
		else
		{
			connection.Connect(	sHost,
								m_pDoc->m_MovDetSendMailConfiguration.m_Auth,
								m_pDoc->m_MovDetSendMailConfiguration.m_sUsername,
								m_pDoc->m_MovDetSendMailConfiguration.m_sPassword,
								m_pDoc->m_MovDetSendMailConfiguration.m_nPort
#ifndef CPJNSMTP_NOSSL
								, m_pDoc->m_MovDetSendMailConfiguration.m_ConnectionType
#endif
								);
		}

		// First save the message to disk then send it from disk
		// so that we have a unique progress display from 0%..100%
		// Note: connection.SendMessage(*pMessage) is not calling
		//       CSaveFrameListSMTPConnection::OnSendProgress()
		pMessage->SaveToDisk(sTempEmailFile);
		CString sENVID;
		connection.SendMessage(sTempEmailFile, pMessage->m_To, pMessage->m_From, sENVID);
	}
}

// Return Values
// -1 : Do Exit Thread
// 0  : Error Sending Email
// 1  : Ok
int CVideoDeviceDoc::CSaveFrameListThread::SendMail(const CStringArray& sFiles) 
{
	int i;

	// Check size -> Return Error
	for (i = 0 ; i < sFiles.GetSize() ; i++)
	{
		if (::GetFileSize64(sFiles[i]).QuadPart == 0)
			return 0;
	}

	// No Configuration -> Return Error
	if (m_pDoc->m_MovDetSendMailConfiguration.m_sHost.IsEmpty()	||
		m_pDoc->m_MovDetSendMailConfiguration.m_sFrom.IsEmpty()	||
		m_pDoc->m_MovDetSendMailConfiguration.m_sTo.IsEmpty())
		return 0;
	else 
	{
		CPJNSMTPMessage* pMessage = NULL;
		CVideoDeviceDoc::CSaveFrameListSMTPConnection connection(this);
		CString sTempEmailFile = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), _T("email.eml"));
		try
		{
			if (m_pDoc->m_MovDetSendMailConfiguration.m_bHTML == FALSE)
			{
				m_pDoc->m_MovDetSendMailConfiguration.m_bMime = FALSE;
				m_pDoc->m_MovDetSendMailConfiguration.m_sFiles = _T("");
				m_pDoc->m_MovDetSendMailConfiguration.m_sBody = _T("Movement Detection: ") + m_pDoc->GetAssignedDeviceName();

				// Attachment(s)
				for (i = 0 ; i < sFiles.GetSize() ; i++)
				{
					if (m_pDoc->m_MovDetSendMailConfiguration.m_sFiles.IsEmpty())
						m_pDoc->m_MovDetSendMailConfiguration.m_sFiles = sFiles[i];
					else
						m_pDoc->m_MovDetSendMailConfiguration.m_sFiles += _T(";") + sFiles[i];
				}

				// Create the message
				pMessage = CVideoDeviceDoc::CreateEmailMessage(&m_pDoc->m_MovDetSendMailConfiguration);
				if (!pMessage)
					return 0;
			}
			else
			{
				m_pDoc->m_MovDetSendMailConfiguration.m_bMime = TRUE;
				m_pDoc->m_MovDetSendMailConfiguration.m_sFiles = _T("");
				m_pDoc->m_MovDetSendMailConfiguration.m_sBody = _T("");

				// Create the message
				pMessage = CVideoDeviceDoc::CreateEmailMessage(&m_pDoc->m_MovDetSendMailConfiguration);
				if (!pMessage)
					return 0;
				for (i = 0 ; i < pMessage->GetNumberOfBodyParts() ; i++)
					pMessage->RemoveBodyPart(i);

				// Setup all the body parts we want
				srand(::makeseed(::timeGetTime(), ::GetCurrentProcessId(), ::GetCurrentThreadId())); // Seed
				CString sRanNum;
				sRanNum.Format(_T("%08X"), (DWORD)irand(4294967296.0)); // returns a hex random string in the range [0,0xFFFFFFFF]
				CPJNSMTPBodyPart related;
				related.SetContentType(_T("multipart/related"));
				CPJNSMTPBodyPart html;
				CString sHtml(_T("<html><body>"));
				sHtml += _T("<p>Movement Detection: ") + ::HtmlEncode(m_pDoc->GetAssignedDeviceName()) + _T("</p>");
				for (i = 0 ; i < sFiles.GetSize() ; i++)
				{
					CString s;
					if (::GetFileExt(sFiles[i]) == _T(".avi"))
					{
						s.Format(_T("<a href=\"cid:%s\">AVI Video File</a>"),
								::GetShortFileName(sFiles[i]) + _T("@") + sRanNum + _T(".com"));
					}
					else
					{
						s.Format(_T("<img src=\"cid:%s\" border=\"0\" alt=\"Detection Image\">"),
								::GetShortFileName(sFiles[i]) + _T("@") + sRanNum + _T(".com"));
					}
					sHtml += s;
				}
				sHtml += _T("</body></html>");
				html.SetText(sHtml);
				html.SetContentType(_T("text/html"));
				related.AddChildBodyPart(html);
				for (i = 0 ; i < sFiles.GetSize() ; i++)
				{
					CPJNSMTPBodyPart filebody;
					filebody.SetFilename(sFiles[i]);
					filebody.SetContentID(_T("<") + ::GetShortFileName(sFiles[i]) + _T("@") + sRanNum + _T(".com") + _T(">"));
					filebody.SetContentType(::FileNameToMime(sFiles[i]));
					related.AddChildBodyPart(filebody);
				}
				pMessage->AddBodyPart(related);
				pMessage->GetBodyPart(0)->SetContentLocation(_T("http://localhost"));
			}

			// Send It
			SendMailMessage(sTempEmailFile, connection, pMessage);

			// Clean-up
			if (pMessage)
				delete pMessage;
			::DeleteFile(sTempEmailFile);

			// Sending Interrupted (for old PJNSMTP version)
			if (connection.m_bDoExit)
				return -1;
			else
				return 1;
		}
		catch (CPJNSMTPException* pEx)
		{
			// Clean-up
			if (pMessage)
				delete pMessage;
			::DeleteFile(sTempEmailFile);

			// Sending Interrupted (new PJNSMTP version throws an exception for that)
			if (connection.m_bDoExit)
			{
				pEx->Delete();
				return -1;
			}
			// Display the error
			else
			{
				CString sMsg;
				sMsg.Format(_T("%s, an error occured sending the message, Error:%x\nDescription:%s\n"),
							m_pDoc->GetAssignedDeviceName(),
							pEx->m_hr,
							pEx->GetErrorMessage());
				TRACE(sMsg);
				::LogLine(sMsg);
				pEx->Delete();
				return 0;
			}
		}
	}
}

CVideoDeviceDoc::CSaveFrameListFTPTransfer::CSaveFrameListFTPTransfer(CSaveFrameListThread* pThread) : CFTPTransfer(pThread)
{
	m_pThread = pThread;
}

void CVideoDeviceDoc::CSaveFrameListFTPTransfer::OnTransferProgress(DWORD dwPercentage)
{
	if (m_pThread)
		m_pThread->SetFTPUploadProgress(dwPercentage);
}

// Return Values
// -1 : Do Exit Thread
// 0  : Error
// 1  : Ok
int CVideoDeviceDoc::FTPUpload(	CFTPTransfer* pFTP, FTPUploadConfigurationStruct* pConfig,
								CString sLocalFileName, CString sRemoteFileName) 
{
	// Nothing To Upload
	if (sLocalFileName.IsEmpty())
		return 1;

	// Check params
	if (!pFTP || !pConfig)
		return 0;

	// Default Remote File Name
	if (sRemoteFileName.IsEmpty())
		sRemoteFileName = ::GetShortFileName(sLocalFileName);

	// No Configuration -> Return Error
	if (pConfig->m_sHost.IsEmpty()) 
		return 0;
	else 
	{
		if (pConfig->m_sRemoteDir.IsEmpty())
			pFTP->m_sRemoteFile = sRemoteFileName;
		else
		{
			// Adjust Remote File Name
			sRemoteFileName.TrimLeft(_T("\\/"));

			// Adjust Remote Dir Path
			CString sRemoteDir = pConfig->m_sRemoteDir;
			sRemoteDir.TrimRight(_T("\\/"));
			
			pFTP->m_sRemoteFile = sRemoteDir + _T("/") + sRemoteFileName;
		}
		pFTP->m_sLocalFile = sLocalFileName;
		pFTP->m_sServer = pConfig->m_sHost;
		pFTP->m_nPort = pConfig->m_nPort;
		pFTP->m_bDownload = FALSE;
		pFTP->m_bBinary = pConfig->m_bBinary;
		pFTP->m_bPromptOverwrite = FALSE;
		pFTP->m_dBandwidthLimit = 0.0;	// For BANDWIDTH throttling, the value in KBytes / Second to limit the connection to
		pFTP->m_bPasv = pConfig->m_bPasv;
		pFTP->m_bUsePreconfig = TRUE;	// Should preconfigured settings be used i.e. take proxy settings etc from the control panel
		pFTP->m_bUseProxy = pConfig->m_bProxy;
		pFTP->m_sProxy = pConfig->m_sProxy;
		if (!pConfig->m_sUsername.IsEmpty())
		{
			pFTP->m_sUserName = pConfig->m_sUsername;
			pFTP->m_sPassword = pConfig->m_sPassword;
		}

		// Upload
		int nRet = pFTP->Transfer();
		if (nRet == 0 && pFTP->m_sError != _T(""))
		{
			CString sMsg(GetAssignedDeviceName() + _T(", ") + pFTP->m_sError + _T("\n"));
			TRACE(sMsg);
			::LogLine(sMsg);
		}
		return nRet;
	}
}

CVideoDeviceDoc::CCaptureAudioThread::CCaptureAudioThread() 
{
	m_pDoc = NULL;

	// Create Input Event
	m_hWaveInEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEventArray[0] = GetKillEvent();
	m_hEventArray[1] = m_hWaveInEvent;
	m_hWaveIn = NULL;

	// Audio Format set Default to: Mono , 11025 Hz , 16 bits
	m_pSrcWaveFormat = (WAVEFORMATEX*)new BYTE[sizeof(WAVEFORMATEX)];
	WaveInitFormat(1, 11025, 16, m_pSrcWaveFormat);
	m_pDstWaveFormat = (WAVEFORMATEX*)new BYTE[sizeof(WAVEFORMATEX)];
	WaveInitFormat(1, 11025, 16, m_pDstWaveFormat);

	// ACM
	for (int i = 0 ; i < AUDIO_UNCOMPRESSED_BUFS_COUNT ; i++)
	{
		memset(&m_WaveHeader[i], 0, sizeof(WAVEHDR));
		m_pUncompressedBuf[i] = NULL;
	}
	m_dwUncompressedBufSize = 0;

	// Init Samples List Critical Section
	::InitializeCriticalSection(&m_csAudioList);
}

CVideoDeviceDoc::CCaptureAudioThread::~CCaptureAudioThread() 
{
	Kill();
	if (m_pSrcWaveFormat)
	{
		delete [] m_pSrcWaveFormat;
		m_pSrcWaveFormat = NULL;
	}
	if (m_pDstWaveFormat)
	{
		delete [] m_pDstWaveFormat;
		m_pDstWaveFormat = NULL;
	}
	for (int i = 0 ; i < AUDIO_UNCOMPRESSED_BUFS_COUNT ; i++)
	{
		if (m_pUncompressedBuf[i])
		{
			delete [] m_pUncompressedBuf[i];
			m_pUncompressedBuf[i] = NULL;
		}
	}
	while (!m_AudioList.IsEmpty())
	{
		CUserBuf UserBuf = m_AudioList.RemoveHead();
		delete [] UserBuf.m_pBuf;
	}
	::CloseHandle(m_hWaveInEvent);
	m_hWaveInEvent = NULL;
	::DeleteCriticalSection(&m_csAudioList);
}

void CVideoDeviceDoc::CCaptureAudioThread::AudioInSourceDialog()
{
	CAudioInSourceDlg dlg(m_pDoc->m_dwCaptureAudioDeviceID);
	if (dlg.DoModal() == IDOK && dlg.m_uiDeviceID != m_pDoc->m_dwCaptureAudioDeviceID)
	{
		// Stop Save Frame List Thread
		m_pDoc->m_SaveFrameListThread.Kill();

		// Stop Rec
		if (m_pDoc->m_pAVRec)
			m_pDoc->CaptureRecord();

		// Set new ID
		if (m_pDoc->m_bCaptureAudio)
			Kill();
		m_pDoc->m_dwCaptureAudioDeviceID = dlg.m_uiDeviceID;
		if (m_pDoc->m_bCaptureAudio)
			Start();

		// Restart Save Frame List Thread
		m_pDoc->m_SaveFrameListThread.Start();
	}
}

void CVideoDeviceDoc::CCaptureAudioThread::WaveInitFormat(WORD wCh, DWORD dwSampleRate, WORD wBitsPerSample, LPWAVEFORMATEX pWaveFormat)
{
	if (!pWaveFormat) return;
	pWaveFormat->wFormatTag = WAVE_FORMAT_PCM;
	pWaveFormat->nChannels = wCh;
	pWaveFormat->nSamplesPerSec = dwSampleRate;
	pWaveFormat->nAvgBytesPerSec = dwSampleRate * wCh * wBitsPerSample/8;
	pWaveFormat->nBlockAlign = wCh * wBitsPerSample/8;
	pWaveFormat->wBitsPerSample = wBitsPerSample;
	pWaveFormat->cbSize = 0;
}

// return
// 0:	exit thread
// -1:	error
int CVideoDeviceDoc::CCaptureAudioThread::Loop()
{
	// nLoopState
	// 1:	ok
	// 0:	exit thread
	// -1:	error
	int nLoopState = 1;

	// Open Audio
	if (!OpenInAudio())
		return -1; // error

	// Start Buffering
	CUserBuf UserBuf;
	::EnterCriticalSection(&m_csAudioList);
	while (!m_AudioList.IsEmpty())
	{
		UserBuf = m_AudioList.RemoveHead();
		delete [] UserBuf.m_pBuf;
	}
	::LeaveCriticalSection(&m_csAudioList);
	m_uiWaveInBufPos = 0;
	int i;
	for (i = 0 ; i < AUDIO_UNCOMPRESSED_BUFS_COUNT ; i++)
	{
		if (m_pUncompressedBuf[i])
			delete [] m_pUncompressedBuf[i];
		m_pUncompressedBuf[i] = new BYTE[m_dwUncompressedBufSize + FF_INPUT_BUFFER_PADDING_SIZE];
		if (!DataInAudio())
		{
			nLoopState = -1; // error
			break;
		}
	}

	// Samples loop
	while (nLoopState == 1)
	{
		DWORD Event = ::WaitForMultipleObjects(2, m_hEventArray, FALSE, INFINITE);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :		nLoopState = 0; // exit thread
										break;

			// Wave In Event
			case WAIT_OBJECT_0 + 1 :	::ResetEvent(m_hWaveInEvent);
										for (i = 0 ; i < AUDIO_UNCOMPRESSED_BUFS_COUNT ; i++)
										{
											if (m_WaveHeader[m_uiWaveInBufPos].dwFlags & WHDR_DONE)
											{
												// Add samples to queue and limit its size
												::EnterCriticalSection(&m_csAudioList);
												UserBuf.m_dwSize = m_WaveHeader[m_uiWaveInBufPos].dwBytesRecorded;
												UserBuf.m_pBuf = m_pUncompressedBuf[m_uiWaveInBufPos];
												m_AudioList.AddTail(UserBuf);
												if (m_AudioList.GetCount() > AUDIO_MAX_LIST_SIZE)
												{
													UserBuf = m_AudioList.RemoveHead();
													delete [] UserBuf.m_pBuf;
												}
												::LeaveCriticalSection(&m_csAudioList);

												// New Buffer
												m_pUncompressedBuf[m_uiWaveInBufPos] = new BYTE[m_dwUncompressedBufSize + FF_INPUT_BUFFER_PADDING_SIZE];
												if (!DataInAudio())
												{
													nLoopState = -1; // error
													break;
												}
											}
											else
												break;
										}
										break;

			// Default
			default :					nLoopState = -1; // error
										break;
		}
	}
	
	// Clean-Up
	::waveInReset(m_hWaveIn);
	for (i = 0 ; i < AUDIO_UNCOMPRESSED_BUFS_COUNT ; i++)
	{
		::waveInUnprepareHeader(m_hWaveIn, &m_WaveHeader[i], sizeof(WAVEHDR));
		memset(&m_WaveHeader[i], 0, sizeof(WAVEHDR));
	}
	CloseInAudio();

	return nLoopState;
}

int CVideoDeviceDoc::CCaptureAudioThread::Work() 
{
	// Check
	if (!m_pDoc)
		return 0;

	// Loop
	while (Loop() != 0)
	{
		// On error try reconnecting after AUDIO_RECONNECTION_DELAY ms of wait time
		if (::WaitForSingleObject(m_hKillEvent, AUDIO_RECONNECTION_DELAY) == WAIT_OBJECT_0)
			break; // exit thread
	}

	return 0;
}

BOOL CVideoDeviceDoc::CCaptureAudioThread::OpenInAudio()
{
	// Check Wave Format Pointers
	if (!m_pSrcWaveFormat || !m_pDstWaveFormat)
		return FALSE;

	// First Close
	CloseInAudio();

	// Get Number of Audio Devices
	UINT num = ::waveInGetNumDevs();
	if (num == 0)
	{
		TRACE(_T("No Sound Input Device.\n"));
		return FALSE;
	}

	// Calculate The Source (=Uncompressed) Buffer Size
	int nSamplesPerSec = m_pSrcWaveFormat->nSamplesPerSec;
	int nBlockAlign = m_pSrcWaveFormat->nBlockAlign;
	if (nSamplesPerSec <= 11025)
		m_dwUncompressedBufSize = 1 * AUDIO_IN_MIN_BUF_SIZE * nBlockAlign;
	else if (nSamplesPerSec <= 22050)
		m_dwUncompressedBufSize = 2 * AUDIO_IN_MIN_BUF_SIZE * nBlockAlign;
	else if (nSamplesPerSec <= 32000)
		m_dwUncompressedBufSize = 3 * AUDIO_IN_MIN_BUF_SIZE * nBlockAlign;
	else if (nSamplesPerSec <= 44100)
		m_dwUncompressedBufSize = 4 * AUDIO_IN_MIN_BUF_SIZE * nBlockAlign;
	else if (nSamplesPerSec <= 48000)
		m_dwUncompressedBufSize = 5 * AUDIO_IN_MIN_BUF_SIZE * nBlockAlign;
	else
		m_dwUncompressedBufSize = 8 * AUDIO_IN_MIN_BUF_SIZE * nBlockAlign;

	// Get Frame Size in Bytes
	int nFrameSize = 0;
	if (m_pDstWaveFormat->wFormatTag == WAVE_FORMAT_MPEG ||
		m_pDstWaveFormat->wFormatTag == WAVE_FORMAT_MPEGLAYER3)
		nFrameSize = 2 * 1152 * m_pDstWaveFormat->nChannels;
	else if (m_pDstWaveFormat->wFormatTag == WAVE_FORMAT_DVI_ADPCM)
		nFrameSize = 2 * ((1024 - 4 * m_pDstWaveFormat->nChannels) * 8 / (4 * m_pDstWaveFormat->nChannels) + 1) * m_pDstWaveFormat->nChannels;
	else if (m_pDstWaveFormat->wFormatTag == WAVE_FORMAT_FLAC)
		nFrameSize = 2 * 4608 * m_pDstWaveFormat->nChannels;

	// Set the buffer size a multiple of the frame size
	if (nFrameSize > 0)
	{
		int nRemainder = m_dwUncompressedBufSize % nFrameSize;
		if (nRemainder > 0)
			m_dwUncompressedBufSize += nFrameSize - nRemainder;
	}

	// Open Input 
	if (::waveInOpen(	&m_hWaveIn,
						m_pDoc->m_dwCaptureAudioDeviceID,
						m_pSrcWaveFormat,
						(DWORD)m_hWaveInEvent,
						NULL,
						CALLBACK_EVENT) != MMSYSERR_NOERROR)
	{
		::ResetEvent(m_hWaveInEvent); // Reset The Open Event
        TRACE(_T("Sound Input Cannot Open Device!\n"));
	    return FALSE;
	}
	else
	{
		::ResetEvent(m_hWaveInEvent); // Reset The Open Event
		return TRUE;
	}
}

void CVideoDeviceDoc::CCaptureAudioThread::CloseInAudio()
{
	if (m_hWaveIn)
	{
		::waveInClose(m_hWaveIn);
		m_hWaveIn = NULL;
		::ResetEvent(m_hWaveInEvent); // Reset The Close Event
	}
}

BOOL CVideoDeviceDoc::CCaptureAudioThread::DataInAudio()
{
	MMRESULT res;

	// Obs.: waveInUnprepareHeader in Win2000 fails if the WAVEHDR is empty (all zeros)
	// Make sure the buffer has been used (not first 2 buffers)
	if (m_WaveHeader[m_uiWaveInBufPos].dwFlags & WHDR_DONE)
	{
		res = ::waveInUnprepareHeader(m_hWaveIn, &m_WaveHeader[m_uiWaveInBufPos], sizeof(WAVEHDR)); 
		if (res != MMSYSERR_NOERROR) 
		{
			TRACE(_T("Sound Input Cannot UnPrepareHeader!\n"));
			return FALSE;
		}
	}

    m_WaveHeader[m_uiWaveInBufPos].lpData = (CHAR*)m_pUncompressedBuf[m_uiWaveInBufPos];
    m_WaveHeader[m_uiWaveInBufPos].dwBufferLength = m_dwUncompressedBufSize;
	m_WaveHeader[m_uiWaveInBufPos].dwLoops = 0;
	m_WaveHeader[m_uiWaveInBufPos].dwFlags = 0;

    res = ::waveInPrepareHeader(m_hWaveIn, &m_WaveHeader[m_uiWaveInBufPos], sizeof(WAVEHDR)); 
	if ((res != MMSYSERR_NOERROR) || (m_WaveHeader[m_uiWaveInBufPos].dwFlags != WHDR_PREPARED))
	{
		TRACE(_T("Sound Input Cannot PrepareHeader!\n"));
		return FALSE;
	}

	res = ::waveInAddBuffer(m_hWaveIn, &m_WaveHeader[m_uiWaveInBufPos], sizeof(WAVEHDR));
	if (res != MMSYSERR_NOERROR) 
	{
		TRACE(_T("Sound Input Cannot Add Buffer!\n"));
		return FALSE;
	}

	res = ::waveInStart(m_hWaveIn);
	if (res != MMSYSERR_NOERROR) 
	{
		TRACE(_T("Sound Input Cannot Start Wave In!\n"));
		return FALSE;
	}

	m_uiWaveInBufPos = (m_uiWaveInBufPos + 1) % AUDIO_UNCOMPRESSED_BUFS_COUNT;

	return TRUE;
}

void CVideoDeviceDoc::MovementDetectionProcessing(CDib* pDib, LPBYTE pMJPGData, DWORD dwMJPGSize, DWORD dwVideoProcessorMode, BOOL b1SecTick)
{
	BOOL bMovement = FALSE;
	BOOL bExternalFileTriggerMovement = FALSE;

	// Init from UI thread because of a UI control update and
	// initialization of variables used by the UI drawing
	if (m_lMovDetTotalZones == 0 || m_nCurrentDetectionZoneSize != m_nDetectionZoneSize)
	{
		if (::SendMessage(	GetView()->GetSafeHwnd(),
							WM_THREADSAFE_INIT_MOVDET,
							0, 0) == 0)
			goto end_of_software_detection; // Cannot init, unsupported resolution
	}

	// Software Detection Enabled?
	if (dwVideoProcessorMode & SOFTWARE_MOVEMENT_DETECTOR)
	{
		// Every 1 sec check whether we have to update the Freq Div
		if (b1SecTick														&&
			(m_dEffectiveFrameRate > m_dMovDetFrameRateFreqDivCalc + 0.5	||
			m_dEffectiveFrameRate < m_dMovDetFrameRateFreqDivCalc - 0.5))
		{
			double dNewMovDetFreqDiv = m_dEffectiveFrameRate / MOVDET_WANTED_FREQ;
			if (dNewMovDetFreqDiv < 1.0)
				dNewMovDetFreqDiv = 1.0;
			m_nMovDetFreqDiv = Round(dNewMovDetFreqDiv);
			m_dMovDetFrameRateFreqDivCalc = m_dEffectiveFrameRate;
		}

		// Do detect?
		if ((m_dwFrameCountUp % m_nMovDetFreqDiv) == 0)
		{
			if (!m_pMovementDetectorBackgndDib)
			{
				m_pMovementDetectorBackgndDib = new CDib(*pDib);
				if (!m_pMovementDetectorBackgndDib)
					goto end_of_software_detection;
				m_pMovementDetectorBackgndDib->SetShowMessageBoxOnError(FALSE);
			}
			if (!m_pDifferencingDib)
			{
				m_pDifferencingDib = new CDib;
				if (!m_pDifferencingDib)
					goto end_of_software_detection;
				m_pDifferencingDib->SetShowMessageBoxOnError(FALSE);
				if (!m_pDifferencingDib->AllocateBitsFast(	pDib->GetBitCount(),
															pDib->GetCompression(),
															pDib->GetWidth(),
															pDib->GetHeight()))
					goto end_of_software_detection;
			}

			// Differencing
			BYTE p[16];
			LPBYTE MinDiff = (LPBYTE)((DWORD)(p+7) & 0xFFFFFFF8);
			MinDiff[0] = m_nMovementDetectorIntensityLimit;
			MinDiff[1] = MinDiff[0];
			MinDiff[2] = MinDiff[0];
			MinDiff[3] = MinDiff[0];
			MinDiff[4] = MinDiff[0];
			MinDiff[5] = MinDiff[0];
			MinDiff[6] = MinDiff[0];
			MinDiff[7] = MinDiff[0];

			// Size in 8 bytes units
			int nSize8 = (pDib->GetWidth() * pDib->GetHeight()) >> 3;

			// Do Differencing
			::DiffMMX(	m_pDifferencingDib->GetBits(),				// Dst
						pDib->GetBits(),							// Src1
						m_pMovementDetectorBackgndDib->GetBits(),	// Src2
						nSize8,										// Size in 8 bytes units
						MinDiff);

			// Call Detector
			m_pDifferencingDib->SetUpTime(pDib->GetUpTime());
			bMovement = MovementDetector(m_pDifferencingDib, m_nDetectionLevel);

			// Update background
			// Note: Mix7To1MMX and Mix3To1MMX use the pavgb instruction
			// which is available only on SSE processors
			if (g_bSSE)
			{
				if (m_dMovDetFrameRateFreqDivCalc / m_nMovDetFreqDiv >= MOVDET_MIX_THRESHOLD)
				{
					::Mix7To1MMX(	m_pMovementDetectorBackgndDib->GetBits(),	// Src1 & Dst
									pDib->GetBits(),							// Src2
									nSize8);									// Size in 8 bytes units
				}
				else
				{
					::Mix3To1MMX(	m_pMovementDetectorBackgndDib->GetBits(),	// Src1 & Dst
									pDib->GetBits(),							// Src2
									nSize8);									// Size in 8 bytes units
				}
			}
			else
			{
				int nSize = pDib->GetWidth() * pDib->GetHeight();
				LPBYTE p1 = m_pMovementDetectorBackgndDib->GetBits();
				LPBYTE p2 = pDib->GetBits();
				if (m_dMovDetFrameRateFreqDivCalc / m_nMovDetFreqDiv >= MOVDET_MIX_THRESHOLD)
				{
					for (int i = 0 ; i < nSize ; i++)
						p1[i] = (BYTE)((7 * (int)(p1[i]) + (int)(p2[i]) + 4)>>3);
				}
				else
				{
					for (int i = 0 ; i < nSize ; i++)
						p1[i] = (BYTE)((3 * (int)(p1[i]) + (int)(p2[i]) + 2)>>2);
				}
			}
		}
	}
	else
		memset(m_MovementDetections, 0, MOVDET_MAX_ZONES);

	// End of software detection
end_of_software_detection:

	// Trigger file detection
	if (b1SecTick && !m_sDetectionTriggerFileName.IsEmpty())
	{
		CString sDetectionTriggerFileName(m_sDetectionTriggerFileName);
		sDetectionTriggerFileName.TrimLeft();
		sDetectionTriggerFileName.TrimRight();
		sDetectionTriggerFileName.TrimLeft(_T('\"'));
		sDetectionTriggerFileName.TrimRight(_T('\"'));
		if (sDetectionTriggerFileName.Find(_T('\\')) < 0)
		{
			CString sDetectionAutoSaveDir = m_sRecordAutoSaveDir;
			sDetectionAutoSaveDir.TrimRight(_T('\\'));
			sDetectionTriggerFileName = sDetectionAutoSaveDir + _T("\\") + sDetectionTriggerFileName;
		}
		if (dwVideoProcessorMode & TRIGGER_FILE_DETECTOR)
		{
			FILETIME LastWriteTime;
			if (::GetFileTime(sDetectionTriggerFileName, NULL, NULL, &LastWriteTime)			&&
				(LastWriteTime.dwLowDateTime != m_DetectionTriggerLastWriteTime.dwLowDateTime	||
				LastWriteTime.dwHighDateTime != m_DetectionTriggerLastWriteTime.dwHighDateTime))
			{
				m_DetectionTriggerLastWriteTime.dwLowDateTime = LastWriteTime.dwLowDateTime;
				m_DetectionTriggerLastWriteTime.dwHighDateTime = LastWriteTime.dwHighDateTime;
				bExternalFileTriggerMovement = TRUE;
			}
		}
		else
			::GetFileTime(sDetectionTriggerFileName, NULL, NULL, &m_DetectionTriggerLastWriteTime);
	}

	// Store frames?
	BOOL bStoreFrames =	dwVideoProcessorMode			&&
						(m_bSaveSWFMovementDetection	||
						m_bSaveAVIMovementDetection		||
						m_bSaveAnimGIFMovementDetection	||
						m_bSendMailMovementDetection	||
						m_bFTPUploadMovementDetection);

	// If Movement
	if (bMovement || bExternalFileTriggerMovement)
	{
		// Mark the Frame as a Cause of Movement
		pDib->SetUserFlag(pDib->GetUserFlag() | FRAME_USER_FLAG_MOTION);

		// Update the Up-Time
		m_dwLastDetFrameUpTime = pDib->GetUpTime();

		// First detected frame?
		if (!m_bDetectingMovement)
		{
			m_bDetectingMovement = TRUE;
			m_dwFirstDetFrameUpTime = pDib->GetUpTime();
		}

		// Do we have a movement of at least m_nDetectionMinLengthMilliSeconds?
		// (if m_nDetectionMinLengthMilliSeconds is 0 then m_nDetectionMinLengthMilliSeconds
		// is the same as m_bDetectingMovement)
		if (!m_bDetectingMinLengthMovement &&
			(m_dwLastDetFrameUpTime - m_dwFirstDetFrameUpTime) >= (DWORD)m_nDetectionMinLengthMilliSeconds)
		{
			m_bDetectingMinLengthMovement = TRUE;
			if (m_bExecCommandMovementDetection && m_nExecModeMovementDetection == 0)
				ExecCommandMovementDetection();
		}
	}

	// Check memory load if having MOVDET_MIN_FRAMES_IN_LIST
	// frames and MOVDET_MIN_FRAMES_IN_LIST passed since last check
	double dDocLoad = 0.0;
	double dNewestListLoad = 0.0;
	double dTotalDocsMovementDetecting = 0.0;
	int nFramesCount = GetNewestMovementDetectionsListCount();
	if (nFramesCount >= MOVDET_MIN_FRAMES_IN_LIST &&
		((m_dwFrameCountUp % MOVDET_MIN_FRAMES_IN_LIST) == 0))
	{
		// Calculate the usable RAM
		int nTotalUsableMB = ::GetTotPhysMemMB(FALSE);
		if (nTotalUsableMB > MOVDET_MEM_MAX_MB)	// we are a 32 bits application
			nTotalUsableMB = MOVDET_MEM_MAX_MB;
		nTotalUsableMB -= ((CUImagerApp*)::AfxGetApp())->GetTotalVideoDeviceDocs() * MOVDET_BASE_MEM_USAGE_MB; // consider all open devices because also non-detecting ones use RAM
		if (nTotalUsableMB < MOVDET_MEM_MIN_MB)	// give a chance if many devices open or if we are on a PC with low amount of RAM 
			nTotalUsableMB = MOVDET_MEM_MIN_MB;

		// This document load in %
		dDocLoad = ((double)(GetTotalMovementDetectionListSize() >> 10) / 10.24) / (double)nTotalUsableMB;

		// Newest list load in %
		dNewestListLoad = ((double)(GetNewestMovementDetectionListSize() >> 10) / 10.24) / (double)nTotalUsableMB;

		// Get the total amount of devices which are movement detecting
		dTotalDocsMovementDetecting = (double)((CUImagerApp*)::AfxGetApp())->GetTotalVideoDeviceDocsMovementDetecting();

		// Debug
		TRACE(_T("%s, buf frames=%d, buf mem load=%0.1f, doc mem load=%0.1f\n"),
						GetAssignedDeviceName(), nFramesCount, dNewestListLoad, dDocLoad);

		// High threshold reached, frames saving is to slow:
		// -> drop oldest 3 * MOVDET_MIN_FRAMES_IN_LIST / 2 frames 
		// -> notify user with a gif thumb and in log file 
		if (dTotalDocsMovementDetecting * dDocLoad >= MOVDET_MEM_LOAD_CRITICAL)
		{
			DWORD dwFirstUpTime, dwLastUpTime;
			ShrinkNewestFrameListBy(3 * MOVDET_MIN_FRAMES_IN_LIST / 2, dwFirstUpTime, dwLastUpTime);
			ThumbMessage(	ML_STRING(1817, "Dropping det frames:"),
							ML_STRING(1818, "set lower framerate"),
							ML_STRING(1819, "or resolution!"),
							dwFirstUpTime, dwLastUpTime);
			CString sMsg;
			sMsg.Format(_T("%s, dropping det frames -> set lower framerate or resolution!\n"),
						GetAssignedDeviceName());
			TRACE(sMsg);
			::LogLine(sMsg);
		}
	}

	// If in detection state
	if (m_bDetectingMovement)
	{
		// Add new frame
		if (bStoreFrames)
			AddNewFrameToNewestList(pDib, pMJPGData, dwMJPGSize);

		// Check if end of detection period
		if ((pDib->GetUpTime() - m_dwLastDetFrameUpTime) > (DWORD)m_nMilliSecondsRecAfterMovementEnd)
		{
			// Reset vars
			m_bDetectingMovement = FALSE;
			m_bDetectingMinLengthMovement = FALSE;

			// Save frames if minimum length reached (if m_nDetectionMinLengthMilliSeconds is 0 always save frames)
			if ((m_dwLastDetFrameUpTime - m_dwFirstDetFrameUpTime) >= (DWORD)m_nDetectionMinLengthMilliSeconds)
				SaveFrameList(TRUE);
			else
				ShrinkNewestFrameList(); // shrink to a size of m_nMilliSecondsRecBeforeMovementBegin
		}
		// Low load threshold or maximum number of frames reached
		else if (m_SaveFrameListThread.IsAlive() && !m_SaveFrameListThread.IsWorking()	&&
				(dTotalDocsMovementDetecting * dDocLoad >= MOVDET_MEM_LOAD_THRESHOLD	||
				(nFramesCount + 1) >= m_nDetectionMaxFrames)) // + 1 because we added another frame after the counting
			SaveFrameList(FALSE);
	}
	else if (bStoreFrames)
	{
		// Add new frame and shrink to a size of m_nMilliSecondsRecBeforeMovementBegin
		AddNewFrameToNewestListAndShrink(pDib, pMJPGData, dwMJPGSize);
		
		// If pre-buffer is set to big:
		// -> drop oldest 3 * MOVDET_MIN_FRAMES_IN_LIST / 2 frames
		// -> notify user in log file
		if (dTotalDocsMovementDetecting * dNewestListLoad >= MOVDET_MEM_LOAD_THRESHOLD)
		{
			DWORD dwFirstUpTime, dwLastUpTime;
			ShrinkNewestFrameListBy(3 * MOVDET_MIN_FRAMES_IN_LIST / 2, dwFirstUpTime, dwLastUpTime);
			CString sMsg;
			sMsg.Format(_T("%s, cannot store %ds of det pre-buffer -> lower that!\n"),
						GetAssignedDeviceName(), m_nMilliSecondsRecBeforeMovementBegin / 1000);
			TRACE(sMsg);
			::LogLine(sMsg);
		}
	}
	else
		ClearNewestFrameList();
}

void CVideoDeviceDoc::ExecCommandMovementDetection(	BOOL bReplaceVars/*=FALSE*/,
													CTime StartTime/*=CTime(0)*/,
													const CString& sAVIFileName/*=_T("")*/,
													const CString& sGIFFileName/*=_T("")*/,
													const CString& sSWFFileName/*=_T("")*/,
													int nMovDetSavesCount/*=0*/)
{
	::EnterCriticalSection(&m_csExecCommandMovementDetection);
	if (m_bWaitExecCommandMovementDetection)
	{
		if (m_hExecCommandMovementDetection)
		{
			if (::WaitForSingleObject(m_hExecCommandMovementDetection, 0) == WAIT_OBJECT_0)
			{
				::CloseHandle(m_hExecCommandMovementDetection);
				m_hExecCommandMovementDetection = NULL;
			}
		}
	}
	else if (m_hExecCommandMovementDetection)
	{
		::CloseHandle(m_hExecCommandMovementDetection);
		m_hExecCommandMovementDetection = NULL;
	}
	if (m_sExecCommandMovementDetection != _T("") &&
		m_hExecCommandMovementDetection == NULL)
	{
		CString sExecParamsMovementDetection = m_sExecParamsMovementDetection;
		if (bReplaceVars)
		{
			CString sSecond, sMinute, sHour, sDay, sMonth, sYear, sMovDetSavesCount;
			sSecond.Format(_T("%02d"), StartTime.GetSecond());
			sMinute.Format(_T("%02d"), StartTime.GetMinute());
			sHour.Format(_T("%02d"), StartTime.GetHour());
			sDay.Format(_T("%02d"), StartTime.GetDay());
			sMonth.Format(_T("%02d"), StartTime.GetMonth());
			sYear.Format(_T("%04d"), StartTime.GetYear());
			sMovDetSavesCount.Format(_T("%d"), nMovDetSavesCount);
			sExecParamsMovementDetection.Replace(_T("%sec%"), sSecond);
			sExecParamsMovementDetection.Replace(_T("%min%"), sMinute);
			sExecParamsMovementDetection.Replace(_T("%hour%"), sHour);
			sExecParamsMovementDetection.Replace(_T("%day%"), sDay);
			sExecParamsMovementDetection.Replace(_T("%month%"), sMonth);
			sExecParamsMovementDetection.Replace(_T("%year%"), sYear);
			sExecParamsMovementDetection.Replace(_T("%avi%"), sAVIFileName);
			sExecParamsMovementDetection.Replace(_T("%gif%"), sGIFFileName);
			sExecParamsMovementDetection.Replace(_T("%swf%"), sSWFFileName);
			sExecParamsMovementDetection.Replace(_T("%counter%"), sMovDetSavesCount);
		}
		SHELLEXECUTEINFO sei;
		memset(&sei, 0, sizeof(sei));
		sei.cbSize = sizeof(sei);
		sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
		sei.nShow = m_bHideExecCommandMovementDetection ? SW_HIDE : SW_SHOWNORMAL;
		sei.lpFile = m_sExecCommandMovementDetection;
		sei.lpParameters = sExecParamsMovementDetection; 
		if (::ShellExecuteEx(&sei))
			m_hExecCommandMovementDetection = sei.hProcess;
	}
	::LeaveCriticalSection(&m_csExecCommandMovementDetection);
}

BOOL CVideoDeviceDoc::ResizeFast(CDib* pSrcDib, CDib* pDstDib)
{
	BOOL res = FALSE;
	AVFrame* pSrcFrame = NULL;
	AVFrame* pDstFrame = NULL;
	SwsContext* pImgConvertCtx = NULL;
	int sws_scale_res;
	PixelFormat src_pix_fmt, dst_pix_fmt;

	// Check
	if (!pSrcDib || !pSrcDib->GetBits() || !pDstDib || !pDstDib->GetBits())
		goto exit;

	// Source frame
	pSrcFrame = avcodec_alloc_frame();
	if (!pSrcFrame)
        goto exit;
	src_pix_fmt = CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pSrcDib->GetBMI());
	avpicture_fill(	(AVPicture*)pSrcFrame,
					(uint8_t*)pSrcDib->GetBits(),
					src_pix_fmt,
					pSrcDib->GetWidth(),
					pSrcDib->GetHeight());

	// Destination frame
	pDstFrame = avcodec_alloc_frame();
	if (!pDstFrame)
        goto exit;
	dst_pix_fmt = CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pDstDib->GetBMI());
	avpicture_fill(	(AVPicture*)pDstFrame,
					(uint8_t*)pDstDib->GetBits(),
					dst_pix_fmt,
					pDstDib->GetWidth(),
					pDstDib->GetHeight());

	// Prepare Image Conversion Context
	pImgConvertCtx = sws_getContext(pSrcDib->GetWidth(),	// Source Width
									pSrcDib->GetHeight(),	// Source Height
									src_pix_fmt,			// Source Format
									pDstDib->GetWidth(),	// Destination Width
									pDstDib->GetHeight(),	// Destination Height
									dst_pix_fmt,			// Destination Format
									SWS_BICUBIC,			// SWS_CPU_CAPS_MMX2, SWS_CPU_CAPS_MMX, SWS_CPU_CAPS_3DNOW
									NULL,					// No Src Filter
									NULL,					// No Dst Filter
									NULL);					// Param
	if (!pImgConvertCtx)
		goto exit;

	// Resize
	sws_scale_res = sws_scale(	pImgConvertCtx,			// Image Convert Context
								pSrcFrame->data,		// Source Data
								pSrcFrame->linesize,	// Source Stride
								0,						// Source Slice Y
								pSrcDib->GetHeight(),	// Source Height
								pDstFrame->data,		// Destination Data
								pDstFrame->linesize);	// Destination Stride
#ifdef SUPPORT_LIBSWSCALE
	if (sws_scale_res > 0)
		res = TRUE;
#else
	if (sws_scale_res >= 0)
		res = TRUE;
#endif

exit:
	if (pSrcFrame)
		av_freep(&pSrcFrame);
	if (pDstFrame)
		av_freep(&pDstFrame);
	if (pImgConvertCtx)
	{
		sws_freeContext(pImgConvertCtx);
		pImgConvertCtx = NULL;
	}
	return res;
}

BOOL CVideoDeviceDoc::SaveJpegFast(CDib* pDib, CMJPEGEncoder* pMJPEGEncoder, const CString& sFileName, int quality)
{
	BOOL res = FALSE;
	AVFrame* pSrcFrame = NULL;
	AVFrame* pDstFrame = NULL;
	LPBYTE pJ420Buf = NULL;
	SwsContext* pImgConvertCtx = NULL;
	BITMAPINFO DstBmi;
	DWORD dwEncodedLen;
	int nJ420ImageSize, qscale, sws_scale_res;
	PixelFormat src_pix_fmt, dst_pix_fmt;

	// Check
	if (!pDib || !pDib->GetBits() || !pMJPEGEncoder || sFileName.IsEmpty())
		goto exit;

	// Source frame
	pSrcFrame = avcodec_alloc_frame();
	if (!pSrcFrame)
        goto exit;
	src_pix_fmt = CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pDib->GetBMI());
	avpicture_fill(	(AVPicture*)pSrcFrame,
					(uint8_t*)pDib->GetBits(),
					src_pix_fmt,
					pDib->GetWidth(),
					pDib->GetHeight());

	// Destination frame
	pDstFrame = avcodec_alloc_frame();
	if (!pDstFrame)
        goto exit;
	dst_pix_fmt = PIX_FMT_YUVJ420P; // Full range YUV (0..255)
	nJ420ImageSize = avpicture_get_size(dst_pix_fmt,
										pDib->GetWidth(),
										pDib->GetHeight());
	if (nJ420ImageSize > 0)
		pJ420Buf = new BYTE[nJ420ImageSize + FF_INPUT_BUFFER_PADDING_SIZE];
	if (!pJ420Buf)
		goto exit;
	avpicture_fill(	(AVPicture*)pDstFrame,
					(uint8_t*)pJ420Buf,
					dst_pix_fmt,
					pDib->GetWidth(),
					pDib->GetHeight());

	// Prepare Image Conversion Context
	pImgConvertCtx = sws_getContext(pDib->GetWidth(),		// Source Width
									pDib->GetHeight(),		// Source Height
									src_pix_fmt,			// Source Format
									pDib->GetWidth(),		// Destination Width
									pDib->GetHeight(),		// Destination Height
									dst_pix_fmt,			// Destination Format
									SWS_BICUBIC,			// SWS_CPU_CAPS_MMX2, SWS_CPU_CAPS_MMX, SWS_CPU_CAPS_3DNOW
									NULL,					// No Src Filter
									NULL,					// No Dst Filter
									NULL);					// Param
	if (!pImgConvertCtx)
		goto exit;

	// Convert
	sws_scale_res = sws_scale(	pImgConvertCtx,			// Image Convert Context
								pSrcFrame->data,		// Source Data
								pSrcFrame->linesize,	// Source Stride
								0,						// Source Slice Y
								pDib->GetHeight(),		// Source Height
								pDstFrame->data,		// Destination Data
								pDstFrame->linesize);	// Destination Stride
#ifdef SUPPORT_LIBSWSCALE
	if (sws_scale_res <= 0)
		goto exit;
#else
	if (sws_scale_res < 0)
		goto exit;
#endif

	// Set Dst Header
	memset(&DstBmi, 0, sizeof(BITMAPINFO));
	DstBmi.bmiHeader.biSize =			sizeof(BITMAPINFOHEADER);
	DstBmi.bmiHeader.biWidth =			pDib->GetWidth();
	DstBmi.bmiHeader.biHeight =			pDib->GetHeight();
	DstBmi.bmiHeader.biPlanes =			1;
	DstBmi.bmiHeader.biCompression =	FCC('J420');
	DstBmi.bmiHeader.biBitCount =		12;
	DstBmi.bmiHeader.biSizeImage =		nJ420ImageSize;

	// Empirically found exponential quality scale conversion
	// function which gives libjpeg comparable image size and quality
	qscale = Round(pow(1.02083, (double)(170 - quality)) - 2.233); // from 0 .. 100 -> 31 .. 2

	// JPEG encode
	dwEncodedLen = pMJPEGEncoder->Encode(qscale, // 2: best quality, 31: worst quality
										&DstBmi, pJ420Buf);
	if (dwEncodedLen == 0U)
		goto exit;

	// Save to file
	try
	{
		CFile f(sFileName,
				CFile::modeCreate		|
				CFile::modeWrite		|
				CFile::shareDenyWrite);
		f.Write(pMJPEGEncoder->GetEncodedBuf(), dwEncodedLen);
		res = TRUE;
	}
	catch (CFileException* e)
	{
		e->Delete();
	}

exit:
	if (pSrcFrame)
		av_freep(&pSrcFrame);
	if (pDstFrame)
		av_freep(&pDstFrame);
	if (pImgConvertCtx)
	{
		sws_freeContext(pImgConvertCtx);
		pImgConvertCtx = NULL;
	}
	if (pJ420Buf)
	{
		delete [] pJ420Buf;
		pJ420Buf = NULL;
	}
	return res;
}

BOOL CVideoDeviceDoc::ThumbMessage(	const CString& sMessage1,
									const CString& sMessage2,
									const CString& sMessage3,
									DWORD dwFirstUpTime,
									DWORD dwLastUpTime)
{
	if (m_bSaveAnimGIFMovementDetection)
	{
		// Allocate Thumb Dib
		CDib ThumbDib;
		if (!ThumbDib.AllocateBits(32, BI_RGB, m_dwAnimatedGifWidth, m_dwAnimatedGifHeight, RGB(80,70,70)))
			return FALSE;

		// Current Reference Time and Current Reference Up-Time
		CTime RefTime = CTime::GetCurrentTime();
		DWORD dwRefUpTime = ::timeGetTime();

		// First Frame Time
		CTime FirstTime = CalcTime(dwFirstUpTime, RefTime, dwRefUpTime);
		
		// Last Frame Time
		CTime LastTime = CalcTime(dwLastUpTime, RefTime, dwRefUpTime);

		// Check Whether Dir Exists
		CString sDetectionAutoSaveDir = m_sRecordAutoSaveDir;
		DWORD dwAttrib = ::GetFileAttributes(sDetectionAutoSaveDir);
		if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) // Not Existing or Not A Directory
			return FALSE;
		else
			sDetectionAutoSaveDir.TrimRight(_T('\\'));

		// Thumb name
		CString sGIFFileName;
		CString sGIFTempFileName;
		CString sTime(FirstTime.Format(_T("%Y_%m_%d_%H_%M_%S")));
		if (!CVideoDeviceDoc::CreateCheckYearMonthDayDir(FirstTime, sDetectionAutoSaveDir, sGIFFileName))
			return FALSE;
		if (sGIFFileName == _T(""))
			sGIFFileName = _T("det_") + sTime + _T(".gif");
		else
			sGIFFileName = sGIFFileName + _T("\\") + _T("det_") + sTime + _T(".gif");
		sGIFTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sGIFFileName);

		// Draw rect
		CRect rcRect;
		rcRect.left = 0;
		rcRect.top = 0;
		rcRect.right = ThumbDib.GetWidth();
		rcRect.bottom = ThumbDib.GetHeight();

		// Font
		CFont Font;
		Font.CreatePointFont(THUMBMESSAGE_FONTSIZE * 10, DEFAULT_FONTFACE);

		// Date
		sTime = ::MakeDateLocalFormat(FirstTime);
		if (!ThumbDib.AddSingleLineText(sTime,
										rcRect,
										&Font,
										(DT_LEFT | DT_TOP),
										FRAMEDATE_COLOR,
										TRANSPARENT,
										DXDRAW_BKG_COLOR))
			return FALSE;

		// Message1
		rcRect.top = rcRect.bottom / 4;
		if (!ThumbDib.AddSingleLineText(sMessage1,
										rcRect,
										&Font,
										(DT_CENTER | DT_TOP),
										RGB(0xff,0,0),
										TRANSPARENT,
										DXDRAW_BKG_COLOR))
			return FALSE;

		// Message2
		rcRect.top = 0;
		if (!ThumbDib.AddSingleLineText(sMessage2,
										rcRect,
										&Font,
										(DT_CENTER | DT_VCENTER),
										RGB(0xff,0,0),
										TRANSPARENT,
										DXDRAW_BKG_COLOR))
			return FALSE;

		// Message3
		rcRect.bottom = 3 * rcRect.bottom / 4;
		if (!ThumbDib.AddSingleLineText(sMessage3,
										rcRect,
										&Font,
										(DT_CENTER | DT_BOTTOM),
										RGB(0xff,0,0),
										TRANSPARENT,
										DXDRAW_BKG_COLOR))
			return FALSE;

		// Time
		rcRect.bottom = ThumbDib.GetHeight();
		sTime = ::MakeTimeLocalFormat(FirstTime, TRUE);
		if (!ThumbDib.AddSingleLineText(sTime,
										rcRect,
										&Font,
										(DT_LEFT | DT_BOTTOM),
										FRAMETIME_COLOR,
										TRANSPARENT,
										DXDRAW_BKG_COLOR))
			return FALSE;
		if (!ThumbDib.AddSingleLineText(_T("->"),
										rcRect,
										&Font,
										(DT_CENTER | DT_BOTTOM),
										FRAMETIME_COLOR,
										TRANSPARENT,
										DXDRAW_BKG_COLOR))
			return FALSE;
		rcRect.right -= 1; // Looks nicer!
		sTime = ::MakeTimeLocalFormat(LastTime, TRUE);
		if (!ThumbDib.AddSingleLineText(sTime,
										rcRect,
										&Font,
										(DT_RIGHT | DT_BOTTOM),
										FRAMETIME_COLOR,
										TRANSPARENT,
										DXDRAW_BKG_COLOR))
			return FALSE;

		// Save
		if (!ThumbDib.SaveGIF(sGIFTempFileName))
			return FALSE;

		// Rename Saved Gif File
		::DeleteFile(sGIFFileName);
		::MoveFile(sGIFTempFileName, sGIFFileName);
	}
	
	return TRUE;
}

BOOL CVideoDeviceDoc::CHttpGetFrameThread::PollAndClean(BOOL bDoNewPoll)
{
	BOOL res = FALSE;
	CNetCom* pNetCom = NULL;
	CHttpGetFrameParseProcess* pHttpGetFrameParseProcess = NULL;

	// New Poll Connection
	if (bDoNewPoll)
	{
		pNetCom = new CNetCom;
		if (!pNetCom)
			return FALSE;
		pHttpGetFrameParseProcess = new CHttpGetFrameParseProcess(m_pDoc);
		if (!pHttpGetFrameParseProcess)
		{
			delete pNetCom;
			return FALSE;
		}
		if (m_pDoc->m_pHttpGetFrameParseProcess->m_AnswerAuthorizationType == CVideoDeviceDoc::CHttpGetFrameParseProcess::AUTHBASIC)
			pHttpGetFrameParseProcess->m_AnswerAuthorizationType = CVideoDeviceDoc::CHttpGetFrameParseProcess::AUTHBASIC;
		pHttpGetFrameParseProcess->m_bOldVersion = m_pDoc->m_pHttpGetFrameParseProcess->m_bOldVersion;
		pHttpGetFrameParseProcess->m_FormatType = m_pDoc->m_pHttpGetFrameParseProcess->m_FormatType;
		for (int i = 0 ; i < m_pDoc->m_pHttpGetFrameParseProcess->m_Sizes.GetSize() ; i++)
			pHttpGetFrameParseProcess->m_Sizes.Add(m_pDoc->m_pHttpGetFrameParseProcess->m_Sizes[i]);
		if (Connect(FALSE,
					pNetCom,
					pHttpGetFrameParseProcess,
					m_pDoc->m_pGetFrameNetCom->GetSocketFamily()))
		{
			m_HttpGetFrameNetComList.AddTail(pNetCom);
			m_HttpGetFrameParseProcessList.AddTail(pHttpGetFrameParseProcess);
			pHttpGetFrameParseProcess->SendRequest();
			res = TRUE;
		}
		else
		{
			delete pNetCom;
			delete pHttpGetFrameParseProcess;
			res = FALSE;
		}
	}

	// Clean-Up
	while (m_HttpGetFrameNetComList.GetCount() > (bDoNewPoll ? 1 : 0))
	{
		pNetCom = m_HttpGetFrameNetComList.GetHead();
		if (pNetCom)
		{
			// Remove oldest connection?
			CTimeSpan ConnectionAge = CTime::GetCurrentTime() - pNetCom->m_InitTime;
			if (pNetCom->IsShutdown()												||	// done?
				ConnectionAge.GetTotalSeconds() >= HTTPGETFRAME_CONNECTION_TIMEOUT	||	// too old?
				ConnectionAge.GetTotalSeconds() < 0									||	// "
				!bDoNewPoll)															// too many open connections?
			{
				delete pNetCom; // this calls Close() which blocks till all net threads are done
				m_HttpGetFrameNetComList.RemoveHead();
				pHttpGetFrameParseProcess = m_HttpGetFrameParseProcessList.GetHead();
				if (pHttpGetFrameParseProcess)
					delete pHttpGetFrameParseProcess;
				m_HttpGetFrameParseProcessList.RemoveHead();
			}
			// Nothing more to clean-up... exit loop
			else
				break;
		}
		// Should never happen...
		else
		{
			m_HttpGetFrameNetComList.RemoveHead();
			pHttpGetFrameParseProcess = m_HttpGetFrameParseProcessList.GetHead();
			if (pHttpGetFrameParseProcess)
				delete pHttpGetFrameParseProcess;
			m_HttpGetFrameParseProcessList.RemoveHead();
		}
	}

	return res;
}

int CVideoDeviceDoc::CHttpGetFrameThread::OnError()
{
	CleanUpAllConnections();
	m_pDoc->ConnectErr(ML_STRING(1465, "Cannot connect to the specified network device or server"), m_pDoc->GetDevicePathName(), m_pDoc->GetDeviceName());
	m_pDoc->CloseDocument();
	return 0;
}

void CVideoDeviceDoc::CHttpGetFrameThread::CleanUpAllConnections()
{
	// Start Shutdown All Connections
	POSITION pos = m_HttpGetFrameNetComList.GetHeadPosition();
	while (pos)
	{
		CNetCom* pNetCom = m_HttpGetFrameNetComList.GetNext(pos);
		if (pNetCom)
			pNetCom->ShutdownConnection_NoBlocking();
	}

	// Wait Till All Connections are down
	while (!m_HttpGetFrameNetComList.IsEmpty())
	{
		CNetCom* pNetCom = m_HttpGetFrameNetComList.GetHead();
		if (pNetCom)
			delete pNetCom; // This calls Close() which blocks till all threads are done
		m_HttpGetFrameNetComList.RemoveHead();
		CHttpGetFrameParseProcess* pHttpGetFrameParseProcess = m_HttpGetFrameParseProcessList.GetHead();
		if (pHttpGetFrameParseProcess)
			delete pHttpGetFrameParseProcess;
		m_HttpGetFrameParseProcessList.RemoveHead();
	}
}

BOOL CVideoDeviceDoc::CHttpGetFrameThread::Connect(BOOL bSignalEvents,
												  CNetCom* pNetCom,
												  CVideoDeviceDoc::CHttpGetFrameParseProcess* pParseProcess,
												  int nSocketFamily)
{
	// Check
	if (!pNetCom)
		return FALSE;

	// Init TCP
	return pNetCom->Init(
					FALSE,					// Be Client
					NULL,					// The Optional Owner Window to which send the Network Events.
					NULL,					// The lParam to send with the Messages
					NULL,					// The Optional Rx Buffer.
					NULL,					// The Optional Critical Section for the Rx Buffer.
					NULL,					// The Optional Rx Fifo.
					NULL,					// The Optional Critical Section fot the Rx Fifo.
					NULL,					// The Optional Tx Buffer.
					NULL,					// The Optional Critical Section for the Tx Buffer.
					NULL,					// The Optional Tx Fifo.
					NULL,					// The Optional Critical Section for the Tx Fifo.
					pParseProcess,			// Parser
					NULL,					// Generator
					SOCK_STREAM,			// TCP
					_T(""),					// Local Address (IP or Host Name).
					0,						// Local Port, let the OS choose one
					m_pDoc->m_sGetFrameVideoHost,// Peer Address (IP or Host Name).
					m_pDoc->m_nGetFrameVideoPort,// Peer Port.
					NULL,					// Handle to an Event Object that will get Accept Events.
					bSignalEvents ? GetHttpConnectedEvent() : NULL,// Handle to an Event Object that will get Connect Events.
					bSignalEvents ? GetHttpConnectFailedEvent() : NULL,// Handle to an Event Object that will get Connect Failed Events.
					NULL,					// Handle to an Event Object that will get Close Events.
					bSignalEvents ? GetHttpReadEvent() : NULL,// Handle to an Event Object that will get Read Events.
					NULL,					// Handle to an Event Object that will get Write Events.
					NULL,					// Handle to an Event Object that will get OOB Events.
					NULL,					// Handle to an Event Object that will get an event when 
											// all connection of a server have been closed.
					0,						// A combination of network events:
											// FD_ACCEPT | FD_CONNECT | FD_CONNECTFAILED | FD_CLOSE | FD_READ | FD_WRITE | FD_OOB | FD_ALLCLOSE.
											// A set value means that instead of setting an event it is reset.
					0,						// A combination of network events:
											// FD_ACCEPT | FD_CONNECT | FD_CONNECTFAILED | FD_CLOSE | FD_READ | FD_WRITE | FD_OOB | FD_ALLCLOSE.
											// The Following messages will be sent to the pOwnerWnd (if pOwnerWnd != NULL):
											// WM_NETCOM_ACCEPT_EVENT -> Notification of incoming connections.
											// WM_NETCOM_CONNECT_EVENT -> Notification of completed connection or multipoint "join" operation.
											// WM_NETCOM_CONNECTFAILED_EVENT -> Notification of connection failure.
											// WM_NETCOM_CLOSE_EVENT -> Notification of socket closure.
											// WM_NETCOM_READ_EVENT -> Notification of readiness for reading.
											// WM_NETCOM_WRITE_EVENT -> Notification of readiness for writing.
											// WM_NETCOM_OOB_EVENT -> Notification of the arrival of out-of-band data.
											// WM_NETCOM_ALLCLOSE_EVENT -> Notification that all connection have been closed.
					0,/*=uiRxMsgTrigger*/	// The number of bytes that triggers an hRxMsgTriggerEvent 
											// (if hRxMsgTriggerEvent != NULL).
											// And/Or the number of bytes that triggers a WM_NETCOM_RX Message
											// (if pOwnerWnd != NULL).
											// Upper bound for this value is NETCOM_MAX_RX_BUFFER_SIZE.
					NULL,/*hRxMsgTriggerEvent*/	// Handle to an Event Object that will get an Event
											// each time uiRxMsgTrigger bytes arrived.
					0,/*uiMaxTxPacketSize*/	// The maximum size for transmitted packets,
											// upper bound for this value is NETCOM_MAX_TX_BUFFER_SIZE.
					0,/*uiRxPacketTimeout*/	// After this timeout a Packet is returned
											// even if the uiRxMsgTrigger size is not reached (A zero meens INFINITE Timeout).
					0,/*uiTxPacketTimeout*/	// After this timeout a Packet is sent
											// even if no Write Event Happened (A zero meens INFINITE Timeout).
											// This is also the Generator rate,
											// if set to zero the Generator is never called!
					NULL,					// Message Class for Notice, Warning and Error Visualization.
					nSocketFamily);			// Socket family
}

int CVideoDeviceDoc::CHttpGetFrameThread::Work()
{
	ASSERT(m_pDoc);
	int nAlarmLevel = 0;
	BOOL bCheckConnectionTimeout = FALSE;
	int nConnectionKeepAliveSupported = HTTPGETFRAME_MIN_KEEPALIVE_REQUESTS; // 0: not supported, 1: supported, >1: to be verified

	for (;;)
	{
		// Set wait delay for client poll mode and set
		// check-timeout for both client poll and server
		// push modes when trying to setup a connection 
		DWORD dwWaitDelay = HTTPGETFRAME_DELAY_DEFAULT;
		if (m_pDoc->m_dFrameRate > 0.0)
			dwWaitDelay = (DWORD)Round(1000.0 / m_pDoc->m_dFrameRate);

		// Alarm dependent wait delay (only used in client poll mode)
		if (nAlarmLevel == 1)
		{
			dwWaitDelay = MAX(2U*dwWaitDelay, HTTPGETFRAME_MIN_DELAY_ALARM1);
			dwWaitDelay = MIN(dwWaitDelay, HTTPGETFRAME_MAX_DELAY_ALARM);
		}
		else if (nAlarmLevel == 2)
		{
			dwWaitDelay = MAX(4U*dwWaitDelay, HTTPGETFRAME_MIN_DELAY_ALARM2);
			dwWaitDelay = MIN(dwWaitDelay, HTTPGETFRAME_MAX_DELAY_ALARM);
		}
		else if (nAlarmLevel >= 3)
		{
			dwWaitDelay = MAX(8U*dwWaitDelay, HTTPGETFRAME_MIN_DELAY_ALARM3);
			dwWaitDelay = MIN(dwWaitDelay, HTTPGETFRAME_MAX_DELAY_ALARM);
		}	

		// Wait for events
		DWORD Event = ::WaitForMultipleObjects(	5,
												m_hEventArray,
												FALSE,
												dwWaitDelay);
		switch (Event)
		{
			// Shutdown Event (for both client poll and server push modes)
			case WAIT_OBJECT_0 :		
			{
				CleanUpAllConnections();
				return 0;
			}

			// Http Setup Connection Event (for client poll init and server push mode)
			case WAIT_OBJECT_0 + 1 :
			{
				::ResetEvent(m_hEventArray[1]);
				bCheckConnectionTimeout = TRUE;
				nConnectionKeepAliveSupported = HTTPGETFRAME_MIN_KEEPALIVE_REQUESTS; // 0: not supported, 1: supported, >1: to be verified
				::EnterCriticalSection(&m_csConnectRequestParams);
				DWORD dwConnectDelay = m_dwConnectDelay;
				::LeaveCriticalSection(&m_csConnectRequestParams);
				CleanUpAllConnections();
				if (::WaitForSingleObject(GetKillEvent(), dwConnectDelay / 2U) == WAIT_OBJECT_0)
					return 0;
				m_pDoc->m_pGetFrameNetCom->Close();
				if (::WaitForSingleObject(GetKillEvent(), dwConnectDelay / 2U) == WAIT_OBJECT_0)
					return 0;
				m_pDoc->m_pHttpGetFrameParseProcess->m_bPollNextJpeg = FALSE;
				if (!Connect(TRUE,
							m_pDoc->m_pGetFrameNetCom,
							m_pDoc->m_pHttpGetFrameParseProcess,
							((CUImagerApp*)::AfxGetApp())->m_bIPv6 ? AF_INET6 : AF_INET))
				{
					if (m_pDoc->m_pHttpGetFrameParseProcess->m_bTryConnecting)
					{
						m_pDoc->m_pHttpGetFrameParseProcess->m_bTryConnecting = FALSE;
						return OnError();
					}
				}
				break;
			}

			// Http Connected Event (for client poll init and server push mode)
			case WAIT_OBJECT_0 + 2 :
			{
				::ResetEvent(m_hEventArray[2]);
				::EnterCriticalSection(&m_csConnectRequestParams);
				CString sRequest = m_sRequest;
				::LeaveCriticalSection(&m_csConnectRequestParams);
				::PostMessage(m_pDoc->GetView()->GetSafeHwnd(), WM_THREADSAFE_SETDOCUMENTTITLE, 0, 0); // update device settings title
				if (sRequest == _T(""))
					m_pDoc->m_pHttpGetFrameParseProcess->SendRequest();
				else
					m_pDoc->m_pHttpGetFrameParseProcess->SendRawRequest(sRequest);
				break;
			}

			// Http Read Event (for client poll init and server push mode)
			case WAIT_OBJECT_0 + 3 :
			{
				::ResetEvent(m_hEventArray[3]);
				bCheckConnectionTimeout = FALSE;
				break;
			}

			// Http Connection failed Event (for client poll init and server push mode)
			case WAIT_OBJECT_0 + 4 :
			{
				::ResetEvent(m_hEventArray[4]);
				if (m_pDoc->m_pHttpGetFrameParseProcess->m_bTryConnecting)
				{
					m_pDoc->m_pHttpGetFrameParseProcess->m_bTryConnecting = FALSE;
					return OnError();
				}
				break;
			}

			// Timeout
			case WAIT_TIMEOUT :		
			{	
				// Setup connection timeout (for client poll init and server push mode)
				if (m_pDoc->m_pHttpGetFrameParseProcess->m_bTryConnecting && bCheckConnectionTimeout)
				{
					CTimeSpan ConnectionAge = CTime::GetCurrentTime() - m_pDoc->m_pGetFrameNetCom->m_InitTime;
					if (ConnectionAge.GetTotalSeconds() >= HTTPGETFRAME_CONNECTION_TIMEOUT ||
						ConnectionAge.GetTotalSeconds() < 0)
					{
						m_pDoc->m_pHttpGetFrameParseProcess->m_bTryConnecting = FALSE;
						return OnError();
					}
				}

				// Poll (only client poll mode)
				if (m_pDoc->m_pHttpGetFrameParseProcess->m_bPollNextJpeg)
				{
					// Keep-alive: just send the request to the already open connection
					if (m_pDoc->m_pHttpGetFrameParseProcess->m_bConnectionKeepAlive &&
						nConnectionKeepAliveSupported > 0)
					{
						// Keep-alive support already verified
						if (nConnectionKeepAliveSupported == 1)
							m_pDoc->m_pHttpGetFrameParseProcess->SendRequest();
						// Verify keep-alive support
						// (some crappy HTTP/1.1 servers do not return the "Connection: close"
						// response to our "Connection: keep-alive" request when they do not
						// support keep-alive)
						else
						{
							if (m_pDoc->m_pHttpGetFrameParseProcess->SendRequest())
								nConnectionKeepAliveSupported--;	// try several times to make sure the connection stays open
							else
								nConnectionKeepAliveSupported = 0;	// no keep alive support
						}
					}
					// Some servers by default limit the amount of requests per connection,
					// after a while the connection is closed by the server -> reconnect
					else if (nConnectionKeepAliveSupported == 1)
						SetEventConnect();
					else
					{
						if (m_HttpGetFrameNetComList.GetCount() >= HTTPGETFRAME_MAXCOUNT_ALARM3)
							nAlarmLevel = 3;
						else if (m_HttpGetFrameNetComList.GetCount() >= HTTPGETFRAME_MAXCOUNT_ALARM2)
							nAlarmLevel = 2;
						else if (m_HttpGetFrameNetComList.GetCount() >= HTTPGETFRAME_MAXCOUNT_ALARM1)
							nAlarmLevel = 1;
						else
							nAlarmLevel = 0;
						
						// If in nAlarmLevel 3 state close all connections
						// and do not open new ones!
						PollAndClean(nAlarmLevel < 3);
					}
				}
				break;
			}

			default:
				break;
		}
	}

	return 0;
}

int CVideoDeviceDoc::CWatchdogAndDrawThread::Work()
{
	ASSERT(m_pDoc);
	ASSERT(!m_pDoc->m_pDxDraw);
	DWORD Event;

	// Necessary because we are drawing with directx
	::CoInitialize(NULL);

	// Poll starting flag
	for (;;)
	{
		Event = ::WaitForSingleObject(GetKillEvent(), WATCHDOG_LONGCHECK_TIME);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :		
			{
				::CoUninitialize();
				return 0;
			}
			default:
				break;
		}
		if (m_pDoc->m_bCaptureStarted)
		{
			// Log the starting
			CString sMsg;
			sMsg.Format(_T("%s starting\n"), m_pDoc->GetAssignedDeviceName());
			TRACE(sMsg);
			::LogLine(sMsg);

			// Init settings reload timer
			::SetTimer(	m_pDoc->GetView()->GetSafeHwnd(),
						ID_TIMER_RELOAD_SETTINGS,
						ONESEC_POLL_TIMER_MS, NULL);
			
			break;
		}
		else if (m_pDoc->GetView())
		{
			// Trigger drawing of the Please wait... progress bar
			::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
							WM_THREADSAFE_UPDATEWINDOWSIZES,
							(WPARAM)UPDATEWINDOWSIZES_INVALIDATE,
							(LPARAM)0);
		}
	}

	// Init
	DWORD dwLastHttpReconnectUpTime = ::timeGetTime();
	if (!((CUImagerApp*)::AfxGetApp())->m_bServiceProcess)
		m_pDoc->m_pDxDraw = new CDxDraw;

	// Watch
	for (;;)
	{
		Event = ::WaitForMultipleObjects(2, m_hEventArray, FALSE, WATCHDOG_SHORTCHECK_TIME);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :		
			{
				if (m_pDoc->m_pDxDraw)
				{
					delete m_pDoc->m_pDxDraw;
					m_pDoc->m_pDxDraw = NULL;
				}
				::CoUninitialize();
				return 0;
			}

			// Check and Draw
			case WAIT_OBJECT_0 + 1 : ::ResetEvent(m_hEventArray[1]);
			case WAIT_TIMEOUT :		
			{
				// Get/Reset OSD Message
				CString sOSDMessage;
				COLORREF crOSDMessageColor = DXDRAW_MESSAGE_SUCCESS_COLOR;
				::EnterCriticalSection(&m_pDoc->m_csOSDMessage);
				DWORD dwCurrentUpTime = ::timeGetTime(); // uptime measurement must be inside the cs!
				if ((dwCurrentUpTime - m_pDoc->m_dwOSDMessageUpTime) <= DXDRAW_MESSAGE_SHOWTIME)
				{
					sOSDMessage = m_pDoc->m_sOSDMessage;
					crOSDMessageColor = m_pDoc->m_crOSDMessageColor;
				}
				else
					m_pDoc->m_sOSDMessage = _T("");
				::LeaveCriticalSection(&m_pDoc->m_csOSDMessage);

				// Update m_bWatchDogAlarm
				DWORD dwFrameTime = (DWORD)Round(1000.0 / m_pDoc->m_dFrameRate);
				if (m_pDoc->m_dEffectiveFrameRate > 0.0)
					dwFrameTime = (DWORD)Round(1000.0 / m_pDoc->m_dEffectiveFrameRate);
				DWORD dwMsSinceLastProcessFrame = dwCurrentUpTime - (DWORD)m_pDoc->m_lCurrentInitUpTime;
				if (dwMsSinceLastProcessFrame > WATCHDOG_THRESHOLD	&&
					dwMsSinceLastProcessFrame > 7U * dwFrameTime)
					m_pDoc->m_bWatchDogAlarm = TRUE;
				else
				{
					m_pDoc->m_bWatchDogAlarm = FALSE;
					dwLastHttpReconnectUpTime = dwCurrentUpTime;
				}

				// Save Frame List may be called many times till
				// CSaveFrameListThread::Work() reacts and starts working:
				// it's not a problem because CSaveFrameListThread::Work()
				// removes empty lists.
				if (m_pDoc->m_bWatchDogAlarm							&&
					m_pDoc->m_dwVideoProcessorMode						&&
					m_pDoc->m_bDetectingMovement						&&
					m_pDoc->GetTotalMovementDetectionListSize() > 0		&&
					m_pDoc->m_SaveFrameListThread.IsAlive()				&&
					!m_pDoc->m_SaveFrameListThread.IsWorking()			&&
					(m_pDoc->m_bSaveSWFMovementDetection				||
					m_pDoc->m_bSaveAVIMovementDetection					||
					m_pDoc->m_bSaveAnimGIFMovementDetection				||
					m_pDoc->m_bSendMailMovementDetection				||
					m_pDoc->m_bFTPUploadMovementDetection))
					m_pDoc->SaveFrameList(FALSE);

				// Http Networking Reconnect
				if (dwCurrentUpTime - dwLastHttpReconnectUpTime > (DWORD)(1000 * HTTPGETFRAME_CONNECTION_TIMEOUT) &&
					m_pDoc->m_pGetFrameNetCom && m_pDoc->m_pGetFrameNetCom->IsClient())
				{
					dwLastHttpReconnectUpTime = dwCurrentUpTime;
					m_pDoc->m_HttpGetFrameThread.SetEventConnect();
					CString sMsg;
					sMsg.Format(_T("%s try reconnecting\n"), m_pDoc->GetAssignedDeviceName());
					TRACE(sMsg);
					::LogLine(sMsg);
				}

				// Draw
				if (m_pDoc->GetView() && m_pDoc->m_pDxDraw &&
					::AfxGetMainFrame()->m_lSessionDisconnectedLockedCount <= 0)
				{
					if (!m_pDoc->GetView()->DxDraw(dwCurrentUpTime, sOSDMessage, crOSDMessageColor))
					{
						Event = ::WaitForSingleObject(GetKillEvent(), WATCHDOG_LONGCHECK_TIME);
						switch (Event)
						{
							// Shutdown Event
							case WAIT_OBJECT_0 :		
							{
								if (m_pDoc->m_pDxDraw)
								{
									delete m_pDoc->m_pDxDraw;
									m_pDoc->m_pDxDraw = NULL;
								}
								::CoUninitialize();
								return 0;
							}
							default:
								break;
						}
					}
				}

				break;
			}

			default:
				break;
		}
	}

	// We never end here!
	return 0;
}

BOOL CVideoDeviceDoc::CDeleteThread::DeleteOld(	CSortableFileFind& FileFind,
												int nAutoSaveDirSize,	
												LONGLONG llDeleteOlderThanDays,
												const CTime& CurrentTime)
{
	CString sDir;
	int nYear, nMonth, nDay;
	for (int pos = 0 ; pos < FileFind.GetDirsCount() ; pos++)
	{
		sDir = FileFind.GetDirName(pos);
		sDir.TrimRight(_T('\\'));
		sDir.Delete(0, nAutoSaveDirSize);
		if (sDir.GetLength() == 4)			// Year
		{
			nYear = _ttoi(sDir);
			nMonth = 12;
			nDay = 31;
		}
		else if (sDir.GetLength() == 7)		// Year + Month
		{
			nYear = _ttoi(sDir.Left(4));
			sDir.Delete(0, 5);
			nMonth = _ttoi(sDir);

			// Determine last Day of Month
			int nNextYear = nYear;
			int nNextMonth = nMonth + 1;
			if (nNextMonth > 12)
			{
				nNextYear++;
				nNextMonth = 1;
			}
			if (nNextYear < 1971	||
				nNextYear > 3000	||	// MFC CTime Limitation
				nYear < 1971		||
				nYear > 3000		||	// MFC CTime Limitation		
				nNextMonth < 1		||
				nNextMonth > 12		||
				nMonth < 1			||
				nMonth > 12)
				continue;
			CTime NextMonthTime(nNextYear, nNextMonth, 1, 12, 0, 0);
			CTime ThisMonthTime(nYear, nMonth, 28, 12, 0, 0);
			CTimeSpan DaysTime = NextMonthTime - ThisMonthTime;
			nDay = 27 + (int)DaysTime.GetDays();
		}
		else if (sDir.GetLength() == 10)	// Year + Month + Day
		{
			nYear = _ttoi(sDir.Left(4));
			sDir.Delete(0, 5);
			nMonth = _ttoi(sDir.Left(2));
			sDir.Delete(0, 3);
			nDay = _ttoi(sDir);
		}
		else
			continue;

		// Check
		if (nYear < 1971	||
			nYear > 3000	||		// MFC CTime Limitation
			nMonth < 1		||
			nMonth > 12		||
			nDay < 1		||
			nDay > 31)
			continue;

		// Delete Dir
		CTime DirTime(nYear, nMonth, nDay, 23, 59, 59);
		CTimeSpan TimeDiff = CurrentTime - DirTime;
		if ((LONGLONG)TimeDiff.GetDays() >= llDeleteOlderThanDays &&
			::IsExistingDir(FileFind.GetDirName(pos)))
			::DeleteDir(FileFind.GetDirName(pos));

		// Do Exit?
		if (DoExit())
			return FALSE;
	}

	return TRUE;
}

BOOL CVideoDeviceDoc::CDeleteThread::CalcOldestDir(	CSortableFileFind& FileFind,
													int nAutoSaveDirSize,
													CTime& OldestDirTime,
													const CTime& CurrentTime)
{
	OldestDirTime = CurrentTime;
	CString sDir;
	int nYear, nMonth, nDay;
	for (int pos = 0 ; pos < FileFind.GetDirsCount() ; pos++)
	{
		sDir = FileFind.GetDirName(pos),
		sDir.TrimRight(_T('\\'));
		sDir.Delete(0, nAutoSaveDirSize);
		if (sDir.GetLength() == 10)	// Year + Month + Day
		{
			nYear = _ttoi(sDir.Left(4));
			sDir.Delete(0, 5);
			nMonth = _ttoi(sDir.Left(2));
			sDir.Delete(0, 3);
			nDay = _ttoi(sDir);
		}
		else
			continue;

		// Check
		if (nYear < 1971	||
			nYear > 3000	||		// MFC CTime Limitation
			nMonth < 1		||
			nMonth > 12		||
			nDay < 1		||
			nDay > 31)
			continue;

		// Is found dir older?
		CTime DirTime(nYear, nMonth, nDay, 23, 59, 59);
		if (DirTime < OldestDirTime &&
			::IsExistingDir(FileFind.GetDirName(pos)))
			OldestDirTime = DirTime;

		// Do Exit?
		if (DoExit())
			return FALSE;
	}

	return TRUE;
}

BOOL CVideoDeviceDoc::CDeleteThread::DeleteIt(CString sAutoSaveDir, int nDeleteOlderThanDays)
{
	DWORD dwAttrib;
	CSortableFileFind FileFind;
	CTime CurrentTime, OldestDirTime;
	CTimeSpan TimeDiff;
	LONGLONG llDaysAgo;
	BOOL bDeletingOld;
	ULONGLONG ullDiskTotalSize;
	ULONGLONG ullMinDiskFreeSpace;

	// Check and adjust Auto-Save directory
	dwAttrib = ::GetFileAttributes(sAutoSaveDir);
	if (dwAttrib != 0xFFFFFFFF && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
	{
		sAutoSaveDir.TrimRight(_T('\\'));
		int nAutoSaveDirSize = sAutoSaveDir.GetLength() + 1;

		// Do recursive file find
		if (FileFind.InitRecursive(sAutoSaveDir + _T("\\*"), FALSE))
		{
			if (FileFind.WaitRecursiveDone(GetKillEvent()) == -1)
				return FALSE; // Exit Thread
		}

		// Get current time
		CurrentTime = CTime::GetCurrentTime();

		// Delete files which are older than the given days amount
		if (nDeleteOlderThanDays > 0)
		{
			if (!DeleteOld(	FileFind,
							nAutoSaveDirSize,
							nDeleteOlderThanDays,
							CurrentTime))
				return FALSE; // Exit Thread
		}

		// Delete oldest files if we are short of disk space
		ullDiskTotalSize = ::GetDiskSize(sAutoSaveDir);
		if (ullDiskTotalSize > 0)
		{
			// Minimum wanted disk space: MIN(10% of HD size, 10 GB)
			ullMinDiskFreeSpace = MIN(ullDiskTotalSize / 10, 10737418240UI64);

			// Get the time of the oldest existing directory
			if (!CalcOldestDir(	FileFind,
								nAutoSaveDirSize,
								OldestDirTime,
								CurrentTime))
				return FALSE; // Exit Thread

			TimeDiff = CurrentTime - OldestDirTime;
			llDaysAgo = (LONGLONG)TimeDiff.GetDays();
			bDeletingOld = FALSE;
			while (llDaysAgo > 0 && ::GetDiskSpace(sAutoSaveDir) < ullMinDiskFreeSpace)
			{
				// Delete old
				if (!DeleteOld(	FileFind,
								nAutoSaveDirSize,
								llDaysAgo,
								CurrentTime))
					return FALSE; // Exit Thread
				bDeletingOld = TRUE;
				llDaysAgo--;
			}

			// Log
			if (bDeletingOld)
			{
				CString sMsg;
				sMsg.Format(_T("%s, deleting old files in \"%s\" because the available disk space is low\n"),
							m_pDoc->GetAssignedDeviceName(), sAutoSaveDir);
				TRACE(sMsg);
				::LogLine(sMsg);
			}
		}
	}

	return TRUE;
}

int CVideoDeviceDoc::CDeleteThread::Work()
{
	if (!m_pDoc)
		return 0;

	DWORD Event;

	for (;;)
	{
		// If using a constant deletion time interval in case of multiple devices running
		// the first started one would be cleared more than the last one. To fix that
		// we use a random generator for the deletion interval
		srand(::makeseed(::timeGetTime(), ::GetCurrentProcessId(), ::GetCurrentThreadId())); // Seed
		DWORD dwDeleteInMs = FILES_DELETE_INTERVAL_MIN + irand(FILES_DELETE_INTERVAL_RANGE); // [10min,15min[
		Event = ::WaitForSingleObject(GetKillEvent(), dwDeleteInMs);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :
				return 0;

			// Delete
			case WAIT_TIMEOUT :
				if (!DeleteIt(m_pDoc->m_sRecordAutoSaveDir, m_pDoc->m_nDeleteRecordingsOlderThanDays))
					return 0; // Exit Thread
				break;

			default:
				break;
		}
	}

	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// CVideoDeviceDoc Functions

CVideoDeviceDoc::CVideoDeviceDoc()
{
	// Current Time
	CTime CurrentTime = CTime::GetCurrentTime();
	CTime CurrentTimeOnly = CTime(2000, 1, 1, CurrentTime.GetHour(), CurrentTime.GetMinute(), CurrentTime.GetSecond());

	// Disable Message Box Show
	m_pDib->SetShowMessageBoxOnError(FALSE);
	
	// Allocate Helper Dibs
	m_pProcessFrameDib = new CDib;
	m_pProcessFrameDib->SetShowMessageBoxOnError(FALSE);
	m_pProcessFrameExtraDib = new CDib;
	m_pProcessFrameExtraDib->SetShowMessageBoxOnError(FALSE);

	// General Vars
	m_pView = NULL;
	m_pFrame = NULL;
	m_pDxDraw = NULL;
	m_bCaptureAudio = FALSE;
	m_dwStopProcessFrame = 0U;
	m_dwProcessFrameStopped = 0U;
	m_pAVRec = NULL;
	m_bInterleave = FALSE; // Do not interleave because while recording the frame rate is not yet exactly known!
	m_bDeinterlace = FALSE;
	m_bRotate180 = FALSE;
	memset(&m_CaptureBMI, 0, sizeof(BITMAPINFOFULL));
	memset(&m_ProcessFrameBMI, 0, sizeof(BITMAPINFOFULL));
	m_dFrameRate = DEFAULT_FRAMERATE;
	m_dEffectiveFrameRate = 0.0;
	m_dEffectiveFrameTimeSum = 0.0;
	m_dwEffectiveFrameTimeCountUp = 0U;
	m_bDoEditCopy = FALSE;
	m_bDoEditSnapshot = FALSE;
	m_lProcessFrameTime = 0;
	m_lCompressedDataRate = 0;
	m_lCompressedDataRateSum = 0;
	m_bPlacementLoaded = FALSE;
	m_bCaptureStarted = FALSE;
	m_bShowFrameTime = TRUE;
	m_nRefFontSize = 9;
	m_nMovDetSavesCount = 1;
	m_nMovDetSavesCountDay = CurrentTime.GetDay();
	m_nMovDetSavesCountMonth = CurrentTime.GetMonth();
	m_nMovDetSavesCountYear = CurrentTime.GetYear();
	m_bVideoView = TRUE;
	m_dwVideoProcessorMode = NO_DETECTOR;
	m_bDecodeFramesForPreview = FALSE;
	m_dwFrameCountUp = 0U;
	m_bSizeToDoc = TRUE;
	m_bDeviceFirstRun = FALSE;
	m_1SecTime = CurrentTimeOnly;

	// Capture Devices
	m_pDxCapture = NULL;
	m_dwCaptureAudioDeviceID = 0U;
	m_nDeviceInputId = -1;
	m_nDeviceFormatId = -1;
	m_bStopAndChangeFormat = FALSE;
	m_bDxDeviceUnplugged = FALSE;
	m_nDeviceFormatWidth = 0;
	m_nDeviceFormatHeight = 0;
	m_lCurrentInitUpTime = 0;
	m_bWatchDogAlarm = FALSE;

	// Networking
	m_pGetFrameNetCom = NULL;
	m_pHttpGetFrameParseProcess = NULL;
	m_sGetFrameVideoHost = _T("");
	m_nGetFrameVideoPort = DEFAULT_TCP_PORT;
	m_nNetworkDeviceTypeMode = OTHERONE_SP;
	m_nHttpVideoQuality = DEFAULT_HTTP_VIDEO_QUALITY;
	m_nHttpVideoSizeX = DEFAULT_HTTP_VIDEO_SIZE_CX;
	m_nHttpVideoSizeY = DEFAULT_HTTP_VIDEO_SIZE_CY;
	m_nHttpGetFrameLocationPos = 0;
	m_HttpGetFrameLocations.Add(_T("/")); // start trying home to see whether cam is reachable

	// Snapshot
	m_bSnapshotLiveJpeg = TRUE;
	m_bSnapshotHistoryJpeg = FALSE;
	m_bSnapshotHistorySwf = FALSE;
	m_bSnapshotLiveJpegFtp = FALSE;
	m_bSnapshotHistoryJpegFtp = FALSE;
	m_bSnapshotHistorySwfFtp = FALSE;
	m_bManualSnapshotAutoOpen = TRUE;
	m_sSnapshotLiveJpegName = DEFAULT_SNAPSHOT_LIVE_JPEGNAME;
	m_sSnapshotLiveJpegThumbName = DEFAULT_SNAPSHOT_LIVE_JPEGTHUMBNAME;
	m_nSnapshotRate = DEFAULT_SNAPSHOT_RATE;
	m_nSnapshotRateMs = 0;
	m_nSnapshotHistoryFrameRate = DEFAULT_SNAPSHOT_HISTORY_FRAMERATE;
	m_nSnapshotCompressionQuality = DEFAULT_SNAPSHOT_COMPR_QUALITY;
	m_fSnapshotVideoCompressorQuality = DEFAULT_VIDEO_QUALITY;
	m_bSnapshotThumb = TRUE;
	m_nSnapshotThumbWidth = DEFAULT_SNAPSHOT_THUMB_WIDTH;
	m_nSnapshotThumbHeight = DEFAULT_SNAPSHOT_THUMB_HEIGHT;
	m_dwNextSnapshotUpTime = 0U;
	m_bSnapshotStartStop = FALSE;
	m_SnapshotStartTime = CurrentTimeOnly;
	m_SnapshotStopTime = CurrentTimeOnly;

	// Threads Init
	m_CaptureAudioThread.SetDoc(this);
	m_HttpGetFrameThread.SetDoc(this);
	m_WatchdogAndDrawThread.SetDoc(this);
	m_DeleteThread.SetDoc(this);
	m_SaveFrameListThread.SetDoc(this);
	m_SaveSnapshotThread.SetDoc(this);
	m_SaveSnapshotSWFThread.SetDoc(this);

	// Recording
	m_sRecordAutoSaveDir = _T("");
	m_bRecAutoOpen = TRUE;
	m_bRecTimeSegmentation = FALSE;
	m_nTimeSegmentationIndex = 0;
	m_dwRecFirstUpTime = 0;
	m_dwRecLastUpTime = 0;
	m_bRecFirstFrame = FALSE;
	m_nRecordedFrames = 0;
	m_nVideoRecDataRate = DEFAULT_VIDEO_DATARATE;
	m_nVideoRecKeyframesRate = DEFAULT_KEYFRAMESRATE;
	m_fVideoRecQuality = ((CUImagerApp*)::AfxGetApp())->m_fFFPreferredVideoEncQuality;
	m_dwVideoRecFourCC = ((CUImagerApp*)::AfxGetApp())->m_dwFFPreferredVideoEncFourCC;
	m_nVideoRecQualityBitrate = 0;
	m_nDeleteRecordingsOlderThanDays = 0;

	// Movement Detection
	m_pDifferencingDib = NULL;
	m_pMovementDetectorBackgndDib = NULL;
	m_bShowMovementDetections = FALSE;
	m_bShowEditDetectionZones = FALSE;
	m_bShowEditDetectionZonesMinus = FALSE;
	m_bDetectingMovement = FALSE;
	m_bDetectingMinLengthMovement = FALSE;
	m_sDetectionTriggerFileName = _T("");
	m_DetectionTriggerLastWriteTime.dwLowDateTime = 0;
	m_DetectionTriggerLastWriteTime.dwHighDateTime = 0;
	m_nMilliSecondsRecBeforeMovementBegin = DEFAULT_PRE_BUFFER_MSEC;
	m_nMilliSecondsRecAfterMovementEnd = DEFAULT_POST_BUFFER_MSEC;
	m_nDetectionMinLengthMilliSeconds = MOVDET_MIN_LENGTH_MSEC;
	m_nDetectionMaxFrames = MOVDET_MAX_FRAMES_IN_LIST;
	m_bSaveSWFMovementDetection = TRUE;
	m_bSaveAVIMovementDetection = FALSE;
	m_bSaveAnimGIFMovementDetection = TRUE;
	m_bSendMailMovementDetection = FALSE;
	m_bFTPUploadMovementDetection = FALSE;
	m_bExecCommandMovementDetection = FALSE;
	m_nExecModeMovementDetection = 0;
	m_sExecCommandMovementDetection = _T("");
	m_sExecParamsMovementDetection = _T("");
	m_bHideExecCommandMovementDetection = FALSE;
	m_bWaitExecCommandMovementDetection = FALSE;
	m_hExecCommandMovementDetection = NULL;
	m_dwTotalMovementDetectionListSize = 0U;
	m_dwNewestMovementDetectionListSize = 0U;
	m_nDetectionLevel = DEFAULT_MOVDET_LEVEL;
	m_nDetectionZoneSize = 0;
	m_nMovementDetectorIntensityLimit = DEFAULT_MOVDET_INTENSITY_LIMIT;
	m_dwAnimatedGifWidth = MOVDET_ANIMGIF_DEFAULT_WIDTH;
	m_dwAnimatedGifHeight = MOVDET_ANIMGIF_DEFAULT_HEIGHT;
	m_MovementDetectorCurrentIntensity = new int[MOVDET_MAX_ZONES];
	m_MovementDetectionsUpTime = new DWORD[MOVDET_MAX_ZONES];
	m_MovementDetections = new BYTE[MOVDET_MAX_ZONES];
	m_DoMovementDetection = new BYTE[MOVDET_MAX_ZONES];
	m_lMovDetXZonesCount = MOVDET_MIN_ZONES_XORY;
	m_lMovDetYZonesCount = MOVDET_MIN_ZONES_XORY;
	m_lMovDetTotalZones = 0;
	m_nVideoDetDataRate = DEFAULT_VIDEO_DATARATE;
	m_nVideoDetKeyframesRate = DEFAULT_KEYFRAMESRATE;
	m_fVideoDetQuality = ((CUImagerApp*)::AfxGetApp())->m_fFFPreferredVideoEncQuality;
	m_nVideoDetQualityBitrate = 0;
	m_dwVideoDetFourCC = ((CUImagerApp*)::AfxGetApp())->m_dwFFPreferredVideoEncFourCC;
	m_nVideoDetSwfDataRate = DEFAULT_VIDEO_DATARATE;
	m_nVideoDetSwfKeyframesRate = DEFAULT_KEYFRAMESRATE;
	m_fVideoDetSwfQuality = DEFAULT_VIDEO_QUALITY;
	m_nVideoDetSwfQualityBitrate = 0;
	m_dwVideoDetSwfFourCC = FCC('FLV1');
	m_nDetectionStartStop = 0;
	m_bDetectionSunday = TRUE;
	m_bDetectionMonday = TRUE;
	m_bDetectionTuesday = TRUE;
	m_bDetectionWednesday = TRUE;
	m_bDetectionThursday = TRUE;
	m_bDetectionFriday = TRUE;
	m_bDetectionSaturday = TRUE;
	m_DetectionStartTime = CurrentTimeOnly;
	m_DetectionStopTime = CurrentTimeOnly;
	m_bUnsupportedVideoSizeForMovDet = FALSE;
	m_nMovDetFreqDiv = 1;
	m_dMovDetFrameRateFreqDivCalc = 0.0;

	// Property Sheet
	m_pMovementDetectionPage = NULL;
	m_pGeneralPage = NULL;
	m_pSnapshotPage = NULL;
	m_pVideoDevicePropertySheet = NULL;

	// Email Settings
	m_MovDetSendMailConfiguration.m_sBCC = _T("");
	m_MovDetSendMailConfiguration.m_sBody = _T("");
	m_MovDetSendMailConfiguration.m_sCC = _T("");
	m_MovDetSendMailConfiguration.m_sFiles = _T("");
	m_MovDetSendMailConfiguration.m_AttachmentType = ATTACHMENT_NONE;
	m_MovDetSendMailConfiguration.m_sSubject = _T("");
	m_MovDetSendMailConfiguration.m_sTo = _T("");
	m_MovDetSendMailConfiguration.m_bDirectly = FALSE;
	m_MovDetSendMailConfiguration.m_bDNSLookup = FALSE;
	m_MovDetSendMailConfiguration.m_nPort = 25;
	m_MovDetSendMailConfiguration.m_sFrom = _T("");
	m_MovDetSendMailConfiguration.m_sHost = _T("");
	m_MovDetSendMailConfiguration.m_sFromName = _T("");
	m_MovDetSendMailConfiguration.m_Auth = CPJNSMTPConnection::AUTH_AUTO;
	m_MovDetSendMailConfiguration.m_sUsername = _T("");
	m_MovDetSendMailConfiguration.m_sPassword = _T("");
	m_MovDetSendMailConfiguration.m_sBoundIP = _T("");
	m_MovDetSendMailConfiguration.m_sEncodingFriendly = _T("Western European (ISO)");
	m_MovDetSendMailConfiguration.m_sEncodingCharset = _T("iso-8859-1");
	m_MovDetSendMailConfiguration.m_bMime = TRUE;
	m_MovDetSendMailConfiguration.m_bHTML = TRUE;
	m_MovDetSendMailConfiguration.m_ConnectionType = CPJNSMTPConnection::PlainText;
	m_MovDetSendMailConfiguration.m_Priority = CPJNSMTPMessage::NoPriority;

	// FTP Settings
	m_MovDetFTPUploadConfiguration.m_sHost = _T("");
	m_MovDetFTPUploadConfiguration.m_sRemoteDir = _T("");
	m_MovDetFTPUploadConfiguration.m_nPort = 21;
	m_MovDetFTPUploadConfiguration.m_bPasv = TRUE;
	m_MovDetFTPUploadConfiguration.m_bBinary = TRUE;
	m_MovDetFTPUploadConfiguration.m_bProxy = FALSE;
	m_MovDetFTPUploadConfiguration.m_sProxy = _T("");
	m_MovDetFTPUploadConfiguration.m_sUsername = _T("");
	m_MovDetFTPUploadConfiguration.m_sPassword = _T("");
	m_MovDetFTPUploadConfiguration.m_FilesToUpload = FILES_TO_UPLOAD_AVI_GIF;
	m_SnapshotFTPUploadConfiguration.m_sHost = _T("");
	m_SnapshotFTPUploadConfiguration.m_sRemoteDir = _T("");
	m_SnapshotFTPUploadConfiguration.m_nPort = 21;
	m_SnapshotFTPUploadConfiguration.m_bPasv = TRUE;
	m_SnapshotFTPUploadConfiguration.m_bBinary = TRUE;
	m_SnapshotFTPUploadConfiguration.m_bProxy = FALSE;
	m_SnapshotFTPUploadConfiguration.m_sProxy = _T("");
	m_SnapshotFTPUploadConfiguration.m_sUsername = _T("");
	m_SnapshotFTPUploadConfiguration.m_sPassword = _T("");
	m_SnapshotFTPUploadConfiguration.m_FilesToUpload = FILES_TO_UPLOAD_AVI; // Not used

	// Init Command Execution on Detection Critical Section
	::InitializeCriticalSection(&m_csExecCommandMovementDetection);

	// Init Avi File Critical Section
	::InitializeCriticalSection(&m_csAVRec);

	// Init Movement Detections List Critical Section
	::InitializeCriticalSection(&m_csMovementDetectionsList);

	// Init Http Video Size and Compression Critical Section
	::InitializeCriticalSection(&m_csHttpParams);

	// Init Http Video Processing Image Data Critical Section
	::InitializeCriticalSection(&m_csHttpProcess);

	// Init Snapshot Configuration Critical Section
	::InitializeCriticalSection(&m_csSnapshotConfiguration);

	// Init OSD Message Critical Section
	::InitializeCriticalSection(&m_csOSDMessage);

	// Init Process Frame Stop Engine Critical Section
	::InitializeCriticalSection(&m_csProcessFrameStop);

	// Init Movement Detector
	OneEmptyFrameList();
	FreeMovementDetector();

	// Start Save Frame List Thread
	m_SaveFrameListThread.Start();

	// Start Video Watchdog and Draw Thread
	m_WatchdogAndDrawThread.Start();
}

CVideoDeviceDoc::~CVideoDeviceDoc()
{
	FreeAVIFile();
	// Parsers and Generators always deleted after the related CNetCom objects!
	if (m_pHttpGetFrameParseProcess)
	{
		delete m_pHttpGetFrameParseProcess;
		m_pHttpGetFrameParseProcess = NULL;
	}
	FreeMovementDetector();
	if (m_MovementDetectorCurrentIntensity)
	{
		delete [] m_MovementDetectorCurrentIntensity;
		m_MovementDetectorCurrentIntensity = NULL;
	}
	if (m_MovementDetectionsUpTime)
	{
		delete [] m_MovementDetectionsUpTime;
		m_MovementDetectionsUpTime = NULL;
	}
	if (m_MovementDetections)
	{
		delete [] m_MovementDetections;
		m_MovementDetections = NULL;
	}
	if (m_DoMovementDetection)
	{
		delete [] m_DoMovementDetection;
		m_DoMovementDetection = NULL;
	}
	ClearMovementDetectionsList();
	::DeleteCriticalSection(&m_csProcessFrameStop);
	::DeleteCriticalSection(&m_csOSDMessage);
	::DeleteCriticalSection(&m_csSnapshotConfiguration);
	::DeleteCriticalSection(&m_csHttpProcess);
	::DeleteCriticalSection(&m_csHttpParams);
	::DeleteCriticalSection(&m_csMovementDetectionsList);
	::DeleteCriticalSection(&m_csAVRec);
	::DeleteCriticalSection(&m_csExecCommandMovementDetection);
	if (m_hExecCommandMovementDetection)
	{
		::CloseHandle(m_hExecCommandMovementDetection);
		m_hExecCommandMovementDetection = NULL;
	}
	if (m_pProcessFrameDib)
	{
		delete m_pProcessFrameDib;
		m_pProcessFrameDib = NULL;
	}
	if (m_pProcessFrameExtraDib)
	{
		delete m_pProcessFrameExtraDib;
		m_pProcessFrameExtraDib = NULL;
	}
}

void CVideoDeviceDoc::ConnectErr(LPCTSTR lpszText, const CString& sDevicePathName, const CString& sDeviceName)
{
	if (((CUImagerApp*)::AfxGetApp())->m_bServiceProcess)
		::LogLine(sDeviceName + _T(", ") + lpszText);
	else
	{
		if (AutorunGetDeviceKey(sDevicePathName) != _T(""))
		{
			CConnectErrMsgBoxDlg dlg(sDeviceName + _T(", ") + lpszText);
			dlg.DoModal();
			if (!dlg.m_bAutorun)
				AutorunRemoveDevice(sDevicePathName);
		}
		else
			::AfxMessageBox(sDeviceName + _T(", ") + lpszText, MB_OK | MB_ICONSTOP);
	}
}

void CVideoDeviceDoc::FreeMovementDetector()
{
	if (m_pDifferencingDib)
	{
		delete m_pDifferencingDib;
		m_pDifferencingDib = NULL;
	}
	if (m_pMovementDetectorBackgndDib)
	{
		delete m_pMovementDetectorBackgndDib;
		m_pMovementDetectorBackgndDib = NULL;
	}
	::InterlockedExchange(&m_lMovDetTotalZones, 0);
	m_bDetectingMovement = FALSE;
	m_bDetectingMinLengthMovement = FALSE;
	if (m_MovementDetections)
		memset(m_MovementDetections, 0, MOVDET_MAX_ZONES);
}

void CVideoDeviceDoc::CloseDocument()
{
	GetFrame()->PostMessage(WM_CLOSE, 0, 0);
}

CString CVideoDeviceDoc::GetHostFromDevicePathName(const CString& sDevicePathName)
{
	// Network devices have the format:
	// Host:Port:FrameLocation:NetworkDeviceTypeMode
	// (use reverse find because Host maybe a IP6 address with :)

	// NetworkDeviceTypeMode
	CString sAddress(sDevicePathName);
	int i = sAddress.ReverseFind(_T(':'));
	if (i < 0)
		return _T("");
	CString sNetworkDeviceTypeMode = sAddress.Right(sAddress.GetLength() - i - 1);
	NetworkDeviceTypeMode nNetworkDeviceTypeMode = (NetworkDeviceTypeMode)_tcstol(sNetworkDeviceTypeMode.GetBuffer(0), NULL, 10);
	sNetworkDeviceTypeMode.ReleaseBuffer();
	if (nNetworkDeviceTypeMode < OTHERONE_SP || nNetworkDeviceTypeMode >= LAST_DEVICE)
		return _T("");
	
	// FrameLocation
	sAddress = sAddress.Left(i);
	i = sAddress.ReverseFind(_T(':'));
	if (i < 0)
		return _T("");
	CString sFrameLocation = sAddress.Right(sAddress.GetLength() - i - 1);

	// Port
	sAddress = sAddress.Left(i);
	i = sAddress.ReverseFind(_T(':'));
	if (i < 0)
		return _T("");
	CString sPort = sAddress.Right(sAddress.GetLength() - i - 1);
	int nPort = _tcstol(sPort.GetBuffer(0), NULL, 10);
	sPort.ReleaseBuffer();
	if (nPort <= 0 || nPort > 65535) // Port 0 is Reserved
		return _T("");

	// Host
	return sAddress.Left(i);
}

CString CVideoDeviceDoc::GetNetworkDevicePathName(	const CString& sGetFrameVideoHost,
													int nGetFrameVideoPort,
													const CString& sHttpGetFrameLocation,
													int nNetworkDeviceTypeMode)
{
	CString sDevicePathName;
	sDevicePathName.Format(_T("%s:%d:%s:%d"), sGetFrameVideoHost, nGetFrameVideoPort, sHttpGetFrameLocation, nNetworkDeviceTypeMode);

	// Registry keys cannot begin with a backslash and should
	// not contain backslashes otherwise subkeys are created!
	sDevicePathName.Replace(_T('\\'), _T('/'));

	return sDevicePathName;
}

CString CVideoDeviceDoc::GetDevicePathName()
{
	CString sDevicePathName;

	if (m_pDxCapture)
	{
		sDevicePathName = m_pDxCapture->GetDevicePath();
		// Registry keys cannot begin with a backslash and should
		// not contain backslashes otherwise subkeys are created!
		sDevicePathName.Replace(_T('\\'), _T('/'));
	}
	else if (m_pGetFrameNetCom)
		sDevicePathName = GetNetworkDevicePathName(m_sGetFrameVideoHost, m_nGetFrameVideoPort, m_HttpGetFrameLocations[0], m_nNetworkDeviceTypeMode);

	return sDevicePathName;
}

CString CVideoDeviceDoc::GetAssignedDeviceName()
{
	CString sName = m_sRecordAutoSaveDir;
	sName.TrimRight(_T('\\'));
	int index = sName.ReverseFind(_T('\\'));
	if (index >= 0)
		sName = sName.Mid(index + 1);
	if (sName == _T(""))
		sName = GetDeviceName();
	return sName;
}

CString CVideoDeviceDoc::GetDeviceName()
{
	CString sDevice;

	if (m_pDxCapture)
		sDevice = m_pDxCapture->GetDeviceName();
	else if (m_pGetFrameNetCom)
	{
		sDevice.Format(_T("%s:%d"), m_sGetFrameVideoHost, m_nGetFrameVideoPort);
		sDevice.Replace(_T("%"), _T(":interface")); // for IP6 link-local addresses
	}

	// Registry key cannot begin with a backslash and should
	// not contain backslashes otherwise subkeys are created!
	sDevice.Replace(_T('\\'), _T('/'));

	return sDevice;
}

void CVideoDeviceDoc::SetDocumentTitle()
{
	// Get name
	CString sTitle = GetAssignedDeviceName();

	// General info
	CString sWidthHeight;
	CString sFramerate;
	CString sPixelFormat;
	if (m_DocRect.Width() > 0 && m_DocRect.Height() > 0)
	{
		// Width and Height
		sWidthHeight.Format(_T("%dx%d"), m_DocRect.Width(), m_DocRect.Height());

		// Framerate
		sFramerate.Format(_T("%0.1ff/s"), m_dEffectiveFrameRate > 0.0 ? m_dEffectiveFrameRate : m_dFrameRate);

		// Pixel Format
		if (m_CaptureBMI.bmiHeader.biCompression != m_ProcessFrameBMI.bmiHeader.biCompression)
			sPixelFormat = CDib::GetCompressionName((LPBITMAPINFO)&m_CaptureBMI) + _T(" -> ");
		sPixelFormat += CDib::GetCompressionName((LPBITMAPINFO)&m_ProcessFrameBMI);
		if (m_bDecodeFramesForPreview)
			sPixelFormat += _T(" -> RGB32");
	}

	// Network info
	CString sNetworkMode;
	if (m_pGetFrameNetCom && m_pHttpGetFrameParseProcess)
	{
		if (m_pHttpGetFrameParseProcess->m_FormatType == CHttpGetFrameParseProcess::FORMATMJPEG)
			sNetworkMode = ML_STRING(1865, "Server Push Mode");
		else if (m_pHttpGetFrameParseProcess->m_FormatType == CHttpGetFrameParseProcess::FORMATJPEG)
			sNetworkMode = ML_STRING(1866, "Client Poll Mode");
	}

	// Update Property Sheet title
	if (m_pVideoDevicePropertySheet)
		m_pVideoDevicePropertySheet->UpdateTitle();

	// Set main title
	if (!sWidthHeight.IsEmpty())
		sTitle += _T(" , ") + sWidthHeight;
	if (!sFramerate.IsEmpty())
		sTitle += _T(" , ") + sFramerate;
	if (!sPixelFormat.IsEmpty())
		sTitle += _T(" , ") + sPixelFormat;
	if (!sNetworkMode.IsEmpty())
		sTitle += _T(" , ") + sNetworkMode;
	CDocument::SetTitle(sTitle);
}

CString CVideoDeviceDoc::GetValidName(CString sName)
{
	// Empty String is already ok!
	if (sName.IsEmpty())
		return _T("");

	LPSTR c = NULL;
	if (::ToANSI(sName, &c) <= 0 || !c)
	{
		if (c)
			delete [] c;
		return _T("");
	}
    CString sValidName(_T(""));
	for (int i = 0 ; i < (int)strlen(c) ; i++)
	{
		if ( (48 <= c[i] && c[i] <= 57) ||	// 0-9
			 (65 <= c[i] && c[i] <= 90) ||	// ABC...XYZ
			 (97 <= c[i] && c[i] <= 122)||	// abc...xyz
			 (c[i]== ' ' || c[i] == '-' || c[i] == '_' ||
			 c[i] == '.' || c[i] == '!' ||
			 c[i] == '(' || c[i]== ')'))
			sValidName += CString(c[i]);
		// a family
		else if (c[i] == 'ä')
			sValidName += _T("ae");
		else if (c[i] == 'Ä')
			sValidName += _T("AE");
		else if (c[i] == 'à')
			sValidName += _T("a");
		else if (c[i] == 'À')
			sValidName += _T("A");
		else if (c[i] == 'á')
			sValidName += _T("a");
		else if (c[i] == 'Á')
			sValidName += _T("A");
		else if (c[i] == 'â')
			sValidName += _T("a");
		else if (c[i] == 'Â')
			sValidName += _T("A");
		else if (c[i] == 'å')
			sValidName += _T("a");
		else if (c[i] == 'Å')
			sValidName += _T("A");
		// e family
		else if (c[i] == 'è')
			sValidName += _T("e");
		else if (c[i] == 'È')
			sValidName += _T("E");
		else if (c[i] == 'é')
			sValidName += _T("e");
		else if (c[i] == 'É')
			sValidName += _T("E");
		else if (c[i] == 'ê')
			sValidName += _T("e");
		else if (c[i] == 'Ê')
			sValidName += _T("E");
		// i family
		else if (c[i] == 'ì')
			sValidName += _T("i");
		else if (c[i] == 'Ì')
			sValidName += _T("I");
		else if (c[i] == 'í')
			sValidName += _T("i");
		else if (c[i] == 'Í')
			sValidName += _T("I");
		else if (c[i] == 'î')
			sValidName += _T("i");
		else if (c[i] == 'Î')
			sValidName += _T("I");
		// o family
		else if (c[i] == 'ö')
			sValidName += _T("oe");
		else if (c[i] == 'Ö')
			sValidName += _T("OE");
		else if (c[i] == 'ò')
			sValidName += _T("o");
		else if (c[i] == 'Ò')
			sValidName += _T("O");
		else if (c[i] == 'ó')
			sValidName += _T("o");
		else if (c[i] == 'Ó')
			sValidName += _T("O");
		else if (c[i] == 'ô')
			sValidName += _T("o");
		else if (c[i] == 'Ô')
			sValidName += _T("O");
		// u family
		else if (c[i] == 'ü')
			sValidName += _T("ue");
		else if (c[i] == 'Ü')
			sValidName += _T("UE");
		else if (c[i] == 'ù')
			sValidName += _T("u");
		else if (c[i] == 'Ù')
			sValidName += _T("U");
		else if (c[i] == 'ú')
			sValidName += _T("u");
		else if (c[i] == 'Ú')
			sValidName += _T("U");
		else if (c[i] == 'û')
			sValidName += _T("u");
		else if (c[i] == 'Û')
			sValidName += _T("U");
		// others
		else if (c[i] == 'ç')
			sValidName += _T("c");
		else if (c[i] == 'Ç')
			sValidName += _T("C");
		else if (c[i] == 'ñ')
			sValidName += _T("n");
		else if (c[i] == 'Ñ')
			sValidName += _T("N");
		else if (c[i] == '\'' || c[i] == '\"' || c[i] == '^' || c[i] == '´' || c[i] == '`')
			sValidName += _T("");
		else
			sValidName += _T("_");
	}
	delete [] c;
	return sValidName;
}

void CVideoDeviceDoc::LoadAndDeleteOldZonesSettings()
{
	int i;
	CString sSection(GetDevicePathName());
	CString sZone;

	// Load old style Zones Settings
	for (i = 0 ; i < MOVDET_MAX_ZONES ; i++)
	{
		sZone.Format(MOVDET_ZONE_FORMAT, i);
		m_DoMovementDetection[i] = (BYTE)::AfxGetApp()->GetProfileInt(sSection, sZone, 1);
	}

	// Store new style Zones Settings
	SaveZonesSettings();

	// Clean-up all old style Zones Settings
	for (i = 0 ; i < MOVDET_MAX_ZONES ; i++)
	{
		sZone.Format(MOVDET_ZONE_FORMAT, i);
		if (::AfxGetApp()->m_pszRegistryKey)
		{
			::DeleteRegistryValue(	HKEY_CURRENT_USER,
									CString(_T("Software\\")) + MYCOMPANY + _T("\\") + APPNAME_NOEXT + _T("\\") + sSection,
									sZone);
		}
		else
		{
			::WritePrivateProfileString(sSection,
										sZone,
										NULL,
										::AfxGetApp()->m_pszProfileName);
		}
	}
}

BOOL CVideoDeviceDoc::LoadZonesSettings()
{
	BOOL bOK = FALSE;
	CString sSection(GetDevicePathName());
	for (int i = 0 ; i < MOVDET_MAX_ZONES / MOVDET_MAX_ZONES_BLOCK_SIZE ; i++)
	{
		UINT uiSize = 0U;
		LPBYTE pBlock = NULL;
		CString sBlock;
		sBlock.Format(MOVDET_ZONES_BLOCK_FORMAT, i);
		::AfxGetApp()->GetProfileBinary(sSection, sBlock, &pBlock, &uiSize);
		if (pBlock && uiSize > 0U)
		{
			bOK = TRUE;
			memcpy(	m_DoMovementDetection + i * MOVDET_MAX_ZONES_BLOCK_SIZE,
					pBlock,
					MIN(MOVDET_MAX_ZONES_BLOCK_SIZE, uiSize));
		}
		if (pBlock)
			delete [] pBlock;
	}
	return bOK;
}

void CVideoDeviceDoc::SaveZonesSettings()
{
	CString sSection(GetDevicePathName());
	for (int i = 0 ; i < MOVDET_MAX_ZONES / MOVDET_MAX_ZONES_BLOCK_SIZE ; i++)
	{
		CString sBlock;
		sBlock.Format(MOVDET_ZONES_BLOCK_FORMAT, i);
		::AfxGetApp()->WriteProfileBinary(	sSection,
											sBlock,
											m_DoMovementDetection + i * MOVDET_MAX_ZONES_BLOCK_SIZE,
											MOVDET_MAX_ZONES_BLOCK_SIZE);
	}
}

void CVideoDeviceDoc::LoadSettings(double dDefaultFrameRate, CString sSection, CString sDeviceName)
{
	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();

	// Current Time
	CTime t = CTime::GetCurrentTime();

	// Default auto-save directory
	sDeviceName = GetValidName(sDeviceName);
	CString sDefaultAutoSaveDir(pApp->m_sMicroApacheDocRoot);
	sDefaultAutoSaveDir.TrimRight(_T('\\'));
	sDefaultAutoSaveDir += _T("\\") + sDeviceName;

	// Set Placement
	if (!pApp->m_bForceSeparateInstance && !pApp->m_bServiceProcess)
	{
		LPBYTE pData = NULL;
		UINT nBytes = 0;
		pApp->GetProfileBinary(sSection, _T("WindowPlacement"), &pData, &nBytes);
		WINDOWPLACEMENT* pwp = (WINDOWPLACEMENT*)pData;
		if (pwp && (nBytes == sizeof(WINDOWPLACEMENT)))
		{
			// Reset size to doc flag
			m_bSizeToDoc = FALSE;

			// Open Maximized?
			if (GetFrame()->IsZoomed())
				pwp->showCmd = SW_SHOWMAXIMIZED;

			// Set Window Placement
			GetFrame()->SetWindowPlacement(pwp);
			if (pwp->showCmd == SW_SHOWMAXIMIZED)
				GetFrame()->MDIMaximize();
		}
		if (pData)
			delete [] pData;

		// Set flag
		m_bPlacementLoaded = TRUE;
	}

	// Update m_ZoomRect and show the Please wait... message
	GetView()->UpdateWindowSizes(TRUE, FALSE, FALSE);

	// Device First Run
	m_bDeviceFirstRun = (pApp->GetProfileString(sSection, _T("DeviceName"), _T("")) == _T(""));

	// Email Settings
	m_MovDetSendMailConfiguration.m_sFiles = pApp->GetProfileString(sSection, _T("SendMailFiles"), _T(""));
	m_MovDetSendMailConfiguration.m_AttachmentType = (AttachmentType) pApp->GetProfileInt(sSection, _T("AttachmentType"), ATTACHMENT_NONE);
	m_MovDetSendMailConfiguration.m_sSubject = pApp->GetProfileString(sSection, _T("SendMailSubject"), _T(""));
	m_MovDetSendMailConfiguration.m_sTo = pApp->GetProfileString(sSection, _T("SendMailTo"), _T(""));
	m_MovDetSendMailConfiguration.m_nPort = (int) pApp->GetProfileInt(sSection, _T("SendMailPort"), 25);
	m_MovDetSendMailConfiguration.m_sFrom = pApp->GetProfileString(sSection, _T("SendMailFrom"), _T(""));
	m_MovDetSendMailConfiguration.m_sHost = pApp->GetProfileString(sSection, _T("SendMailHost"), _T(""));
	m_MovDetSendMailConfiguration.m_sFromName = pApp->GetProfileString(sSection, _T("SendMailFromName"), _T(""));
	m_MovDetSendMailConfiguration.m_Auth = (CPJNSMTPConnection::AuthenticationMethod) pApp->GetProfileInt(sSection, _T("SendMailAuth"), CPJNSMTPConnection::AUTH_AUTO);
	m_MovDetSendMailConfiguration.m_sUsername = pApp->GetSecureProfileString(sSection, _T("SendMailUsername"), _T(""));
	m_MovDetSendMailConfiguration.m_sPassword = pApp->GetSecureProfileString(sSection, _T("SendMailPassword"), _T(""));
	m_MovDetSendMailConfiguration.m_bHTML = (BOOL) pApp->GetProfileInt(sSection, _T("SendMailHTML"), TRUE);
	m_MovDetSendMailConfiguration.m_ConnectionType = (CPJNSMTPConnection::ConnectionType) pApp->GetProfileInt(sSection, _T("SendMailConnectionType"), CPJNSMTPConnection::PlainText);

	// FTP Settings
	m_MovDetFTPUploadConfiguration.m_sHost = pApp->GetProfileString(sSection, _T("MovDetFTPHost"), _T(""));
	m_MovDetFTPUploadConfiguration.m_sRemoteDir = pApp->GetProfileString(sSection, _T("MovDetFTPRemoteDir"), _T(""));
	m_MovDetFTPUploadConfiguration.m_nPort = (int) pApp->GetProfileInt(sSection, _T("MovDetFTPPort"), 21);
	m_MovDetFTPUploadConfiguration.m_bPasv = (BOOL) pApp->GetProfileInt(sSection, _T("MovDetFTPPasv"), TRUE);
	m_MovDetFTPUploadConfiguration.m_bProxy = (BOOL) pApp->GetProfileInt(sSection, _T("MovDetFTPProxy"), FALSE);
	m_MovDetFTPUploadConfiguration.m_sProxy = pApp->GetProfileString(sSection, _T("MovDetFTPProxyHost"), _T(""));
	m_MovDetFTPUploadConfiguration.m_sUsername = pApp->GetSecureProfileString(sSection, _T("MovDetFTPUsername"), _T(""));
	m_MovDetFTPUploadConfiguration.m_sPassword = pApp->GetSecureProfileString(sSection, _T("MovDetFTPPassword"), _T(""));
	m_MovDetFTPUploadConfiguration.m_FilesToUpload = (FilesToUploadType) pApp->GetProfileInt(sSection, _T("MovDetFilesToUpload"), FILES_TO_UPLOAD_AVI_GIF);
	m_SnapshotFTPUploadConfiguration.m_sHost = pApp->GetProfileString(sSection, _T("SnapshotFTPHost"), _T(""));
	m_SnapshotFTPUploadConfiguration.m_sRemoteDir = pApp->GetProfileString(sSection, _T("SnapshotFTPRemoteDir"), _T(""));
	m_SnapshotFTPUploadConfiguration.m_nPort = (int) pApp->GetProfileInt(sSection, _T("SnapshotFTPPort"), 21);
	m_SnapshotFTPUploadConfiguration.m_bPasv = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotFTPPasv"), TRUE);
	m_SnapshotFTPUploadConfiguration.m_bProxy = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotFTPProxy"), FALSE);
	m_SnapshotFTPUploadConfiguration.m_sProxy = pApp->GetProfileString(sSection, _T("SnapshotFTPProxyHost"), _T(""));
	m_SnapshotFTPUploadConfiguration.m_sUsername = pApp->GetSecureProfileString(sSection, _T("SnapshotFTPUsername"), _T(""));
	m_SnapshotFTPUploadConfiguration.m_sPassword = pApp->GetSecureProfileString(sSection, _T("SnapshotFTPPassword"), _T(""));

	// Networking
	m_nHttpVideoQuality = (int) pApp->GetProfileInt(sSection, _T("HTTPVideoQuality"), DEFAULT_HTTP_VIDEO_QUALITY);
	m_nHttpVideoSizeX = (int) pApp->GetProfileInt(sSection, _T("HTTPVideoSizeX"), DEFAULT_HTTP_VIDEO_SIZE_CX);
	m_nHttpVideoSizeY = (int) pApp->GetProfileInt(sSection, _T("HTTPVideoSizeY"), DEFAULT_HTTP_VIDEO_SIZE_CY);
	m_sHttpGetFrameUsername = pApp->GetSecureProfileString(sSection, _T("HTTPGetFrameUsername"), _T(""));
	m_sHttpGetFramePassword = pApp->GetSecureProfileString(sSection, _T("HTTPGetFramePassword"), _T(""));

	// All other
	m_bVideoView = (BOOL) pApp->GetProfileInt(sSection, _T("VideoView"), TRUE);
	m_bDeinterlace = (BOOL) pApp->GetProfileInt(sSection, _T("Deinterlace"), FALSE);
	m_bRotate180 = (BOOL) pApp->GetProfileInt(sSection, _T("Rotate180"), FALSE);
	m_bRecAutoOpen = (BOOL) pApp->GetProfileInt(sSection, _T("RecAutoOpen"), TRUE);
	m_bRecTimeSegmentation = (BOOL) pApp->GetProfileInt(sSection, _T("RecTimeSegmentation"), FALSE);
	m_nTimeSegmentationIndex = pApp->GetProfileInt(sSection, _T("TimeSegmentationIndex"), 0);
	m_sRecordAutoSaveDir = pApp->GetProfileString(sSection, _T("RecordAutoSaveDir"), sDefaultAutoSaveDir);
	m_sDetectionTriggerFileName = pApp->GetProfileString(sSection, _T("DetectionTriggerFileName"), _T("movtrigger.txt"));
	CString sDetectionTriggerFileName(m_sDetectionTriggerFileName);
	sDetectionTriggerFileName.TrimLeft();
	sDetectionTriggerFileName.TrimRight();
	sDetectionTriggerFileName.TrimLeft(_T('\"'));
	sDetectionTriggerFileName.TrimRight(_T('\"'));
	if (sDetectionTriggerFileName.Find(_T('\\')) < 0)
	{
		CString sDetectionAutoSaveDir = m_sRecordAutoSaveDir;
		sDetectionAutoSaveDir.TrimRight(_T('\\'));
		sDetectionTriggerFileName = sDetectionAutoSaveDir + _T("\\") + sDetectionTriggerFileName;
	}
	::GetFileTime(sDetectionTriggerFileName, NULL, NULL, &m_DetectionTriggerLastWriteTime);
	m_bSnapshotLiveJpeg = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotLiveJpeg"), TRUE);
	m_bSnapshotHistoryJpeg = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotHistoryJpeg"), FALSE);
	m_bSnapshotHistorySwf = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotHistorySwf"), FALSE);
	m_bSnapshotLiveJpegFtp = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotLiveJpegFtp"), FALSE);
	m_bSnapshotHistoryJpegFtp = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotHistoryJpegFtp"), FALSE);
	m_bSnapshotHistorySwfFtp = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotHistorySwfFtp"), FALSE);
	m_bManualSnapshotAutoOpen = (BOOL) pApp->GetProfileInt(sSection, _T("ManualSnapshotAutoOpen"), TRUE);
	m_sSnapshotLiveJpegName = pApp->GetProfileString(sSection, _T("SnapshotLiveJpegName"), DEFAULT_SNAPSHOT_LIVE_JPEGNAME);
	m_sSnapshotLiveJpegThumbName = pApp->GetProfileString(sSection, _T("SnapshotLiveJpegThumbName"), DEFAULT_SNAPSHOT_LIVE_JPEGTHUMBNAME);
	m_nSnapshotRate = (int) pApp->GetProfileInt(sSection, _T("SnapshotRate"), DEFAULT_SNAPSHOT_RATE);
	m_nSnapshotRateMs = (int) pApp->GetProfileInt(sSection, _T("SnapshotRateMs"), 0);
	m_nSnapshotHistoryFrameRate = (int) pApp->GetProfileInt(sSection, _T("SnapshotHistoryFrameRate"), DEFAULT_SNAPSHOT_HISTORY_FRAMERATE);
	m_nSnapshotCompressionQuality = (int) pApp->GetProfileInt(sSection, _T("SnapshotCompressionQuality"), DEFAULT_SNAPSHOT_COMPR_QUALITY);
	m_fSnapshotVideoCompressorQuality = (float) pApp->GetProfileInt(sSection, _T("SnapshotVideoCompressorQuality"), (int)DEFAULT_VIDEO_QUALITY);
	m_bSnapshotThumb = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotThumb"), TRUE);
	m_nSnapshotThumbWidth = (int) pApp->GetProfileInt(sSection, _T("SnapshotThumbWidth"), DEFAULT_SNAPSHOT_THUMB_WIDTH);
	m_nSnapshotThumbHeight = (int) pApp->GetProfileInt(sSection, _T("SnapshotThumbHeight"), DEFAULT_SNAPSHOT_THUMB_HEIGHT);
	m_bSnapshotStartStop = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotStartStop"), FALSE);
	m_SnapshotStartTime = CTime(2000, 1, 1,	pApp->GetProfileInt(sSection, _T("SnapshotStartHour"), t.GetHour()),
											pApp->GetProfileInt(sSection, _T("SnapshotStartMin"), t.GetMinute()),
											pApp->GetProfileInt(sSection, _T("SnapshotStartSec"), t.GetSecond()));
	m_SnapshotStopTime = CTime(2000, 1, 1,	pApp->GetProfileInt(sSection, _T("SnapshotStopHour"), t.GetHour()),
											pApp->GetProfileInt(sSection, _T("SnapshotStopMin"), t.GetMinute()),
											pApp->GetProfileInt(sSection, _T("SnapshotStopSec"), t.GetSecond()));
	m_bCaptureAudio = (BOOL) pApp->GetProfileInt(sSection, _T("CaptureAudio"), FALSE);
	m_dwCaptureAudioDeviceID = (DWORD) pApp->GetProfileInt(sSection, _T("AudioCaptureDeviceID"), 0);
	m_nDeviceInputId = (int) pApp->GetProfileInt(sSection, _T("VideoCaptureDeviceInputID"), -1);
	m_nDeviceFormatId = (int) pApp->GetProfileInt(sSection, _T("VideoCaptureDeviceFormatID"), -1);
	m_nDeviceFormatWidth = (int) pApp->GetProfileInt(sSection, _T("VideoCaptureDeviceFormatWidth"), 0);
	m_nDeviceFormatHeight = (int) pApp->GetProfileInt(sSection, _T("VideoCaptureDeviceFormatHeight"), 0);
	m_nMilliSecondsRecBeforeMovementBegin = (int) pApp->GetProfileInt(sSection, _T("MilliSecondsRecBeforeMovementBegin"), DEFAULT_PRE_BUFFER_MSEC);
	m_nMilliSecondsRecAfterMovementEnd = (int) pApp->GetProfileInt(sSection, _T("MilliSecondsRecAfterMovementEnd"), DEFAULT_POST_BUFFER_MSEC);
	m_nDetectionMinLengthMilliSeconds = (int) pApp->GetProfileInt(sSection, _T("DetectionMinLengthMilliSeconds"), MOVDET_MIN_LENGTH_MSEC);
	m_nDetectionMaxFrames = (int) pApp->GetProfileInt(sSection, _T("DetectionMaxFrames"), MOVDET_MAX_FRAMES_IN_LIST);
	m_nDetectionLevel = (int) pApp->GetProfileInt(sSection, _T("DetectionLevel"), DEFAULT_MOVDET_LEVEL);
	m_nDetectionZoneSize = (int) pApp->GetProfileInt(sSection, _T("DetectionZoneSize"), 0);
	m_bSaveSWFMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("SaveSWFMovementDetection"), TRUE);
	m_bSaveAVIMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("SaveAVIMovementDetection"), FALSE);
	m_bSaveAnimGIFMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("SaveAnimGIFMovementDetection"), TRUE);
	m_bSendMailMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("SendMailMovementDetection"), FALSE);
	m_bFTPUploadMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("FTPUploadMovementDetection"), FALSE);
	m_bExecCommandMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("DoExecCommandMovementDetection"), FALSE);
	m_nExecModeMovementDetection = pApp->GetProfileInt(sSection, _T("ExecModeMovementDetection"), 0);
	
	// Attention: GetPrivateProfileString() used by GetProfileString() for INI files strips quotes!
	m_sExecCommandMovementDetection = pApp->GetProfileString(sSection, _T("ExecCommandMovementDetection"), _T(""));
	m_sExecCommandMovementDetection.Replace(_T("%singlequote%"), _T("\'"));
	m_sExecCommandMovementDetection.Replace(_T("%doublequote%"), _T("\""));
	m_sExecParamsMovementDetection = pApp->GetProfileString(sSection, _T("ExecParamsMovementDetection"), _T(""));
	m_sExecParamsMovementDetection.Replace(_T("%singlequote%"), _T("\'"));
	m_sExecParamsMovementDetection.Replace(_T("%doublequote%"), _T("\""));

	m_bHideExecCommandMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("HideExecCommandMovementDetection"), FALSE);
	m_bWaitExecCommandMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("WaitExecCommandMovementDetection"), FALSE);
	m_dwVideoProcessorMode = (DWORD) pApp->GetProfileInt(sSection, _T("VideoProcessorMode"), NO_DETECTOR);
	if (GetFrame() && GetFrame()->GetToolBar())
		((CVideoDeviceToolBar*)(GetFrame()->GetToolBar()))->m_DetComboBox.SetCurSel(m_dwVideoProcessorMode);
	m_dwVideoRecFourCC = (DWORD) pApp->GetProfileInt(sSection, _T("VideoRecFourCC"), ((CUImagerApp*)::AfxGetApp())->m_dwFFPreferredVideoEncFourCC);
	m_fVideoRecQuality = (float) pApp->GetProfileInt(sSection, _T("VideoRecQuality"), (int)(((CUImagerApp*)::AfxGetApp())->m_fFFPreferredVideoEncQuality));
	m_nVideoRecKeyframesRate = (int) pApp->GetProfileInt(sSection, _T("VideoRecKeyframesRate"), DEFAULT_KEYFRAMESRATE);
	m_nVideoRecDataRate = (int) pApp->GetProfileInt(sSection, _T("VideoRecDataRate"), DEFAULT_VIDEO_DATARATE);
	m_nVideoRecQualityBitrate = (int) pApp->GetProfileInt(sSection, _T("VideoRecQualityBitrate"), 0);
	m_dwVideoDetFourCC = (DWORD) pApp->GetProfileInt(sSection, _T("VideoDetFourCC"), ((CUImagerApp*)::AfxGetApp())->m_dwFFPreferredVideoEncFourCC);
	m_fVideoDetQuality = (float) pApp->GetProfileInt(sSection, _T("VideoDetQuality"), (int)(((CUImagerApp*)::AfxGetApp())->m_fFFPreferredVideoEncQuality));
	m_nVideoDetQualityBitrate = (int) pApp->GetProfileInt(sSection, _T("VideoDetQualityBitrate"), 0);
	m_nVideoDetKeyframesRate = (int) pApp->GetProfileInt(sSection, _T("VideoDetKeyframesRate"), DEFAULT_KEYFRAMESRATE);
	m_nVideoDetDataRate = (int) pApp->GetProfileInt(sSection, _T("VideoDetDataRate"), DEFAULT_VIDEO_DATARATE);
	m_dwVideoDetSwfFourCC = (DWORD) pApp->GetProfileInt(sSection, _T("VideoDetSwfFourCC"), FCC('FLV1'));
	m_fVideoDetSwfQuality = (float) pApp->GetProfileInt(sSection, _T("VideoDetSwfQuality"), (int)DEFAULT_VIDEO_QUALITY);
	m_nVideoDetSwfQualityBitrate = (int) pApp->GetProfileInt(sSection, _T("VideoDetSwfQualityBitrate"), 0);
	m_nVideoDetSwfKeyframesRate = (int) pApp->GetProfileInt(sSection, _T("VideoDetSwfKeyframesRate"), DEFAULT_KEYFRAMESRATE);
	m_nVideoDetSwfDataRate = (int) pApp->GetProfileInt(sSection, _T("VideoDetSwfDataRate"), DEFAULT_VIDEO_DATARATE);
	m_nDetectionStartStop = (int) pApp->GetProfileInt(sSection, _T("DetectionStartStop"), 0);
	m_bDetectionSunday = (BOOL) pApp->GetProfileInt(sSection, _T("DetectionSunday"), TRUE);
	m_bDetectionMonday = (BOOL) pApp->GetProfileInt(sSection, _T("DetectionMonday"), TRUE);
	m_bDetectionTuesday = (BOOL) pApp->GetProfileInt(sSection, _T("DetectionTuesday"), TRUE);
	m_bDetectionWednesday = (BOOL) pApp->GetProfileInt(sSection, _T("DetectionWednesday"), TRUE);
	m_bDetectionThursday = (BOOL) pApp->GetProfileInt(sSection, _T("DetectionThursday"), TRUE);
	m_bDetectionFriday = (BOOL) pApp->GetProfileInt(sSection, _T("DetectionFriday"), TRUE);
	m_bDetectionSaturday = (BOOL) pApp->GetProfileInt(sSection, _T("DetectionSaturday"), TRUE);
	m_DetectionStartTime = CTime(2000, 1, 1,	pApp->GetProfileInt(sSection, _T("DetectionStartHour"), t.GetHour()),
												pApp->GetProfileInt(sSection, _T("DetectionStartMin"), t.GetMinute()),
												pApp->GetProfileInt(sSection, _T("DetectionStartSec"), t.GetSecond()));
	m_DetectionStopTime = CTime(2000, 1, 1,		pApp->GetProfileInt(sSection, _T("DetectionStopHour"), t.GetHour()),
												pApp->GetProfileInt(sSection, _T("DetectionStopMin"), t.GetMinute()),
												pApp->GetProfileInt(sSection, _T("DetectionStopSec"), t.GetSecond()));
	m_bShowFrameTime = (BOOL) pApp->GetProfileInt(sSection, _T("ShowFrameTime"), TRUE);
	m_nRefFontSize = (int) pApp->GetProfileInt(sSection, _T("RefFontSize"), 9);
	m_bShowMovementDetections = (BOOL) pApp->GetProfileInt(sSection, _T("ShowMovementDetections"), FALSE);
	m_nMovementDetectorIntensityLimit = (int) pApp->GetProfileInt(sSection, _T("IntensityLimit"), DEFAULT_MOVDET_INTENSITY_LIMIT);
	m_dwAnimatedGifWidth = (DWORD) pApp->GetProfileInt(sSection, _T("AnimatedGifWidth"), MOVDET_ANIMGIF_DEFAULT_WIDTH);
	m_dwAnimatedGifHeight = (DWORD) pApp->GetProfileInt(sSection, _T("AnimatedGifHeight"), MOVDET_ANIMGIF_DEFAULT_HEIGHT);
	m_nDeleteRecordingsOlderThanDays = (int) pApp->GetProfileInt(sSection, _T("DeleteRecordingsOlderThanDays"), 0);

	unsigned int nSize = sizeof(m_dFrameRate);
	volatile double* pFrameRate = &m_dFrameRate;
	pApp->GetProfileBinary(sSection, _T("FrameRate"), (LPBYTE*)&pFrameRate, &nSize);
	if (pFrameRate && (nSize > 0))
	{
		m_dFrameRate = *pFrameRate;
		delete [] (LPBYTE)pFrameRate;
	}
	else
	{
		if (pFrameRate)
			delete [] (LPBYTE)pFrameRate;
		m_dFrameRate = dDefaultFrameRate;
	}

	if (m_CaptureAudioThread.m_pSrcWaveFormat)
		delete [] m_CaptureAudioThread.m_pSrcWaveFormat;
	UINT uiSize = 0;
	pApp->GetProfileBinary(sSection, _T("SrcWaveFormat"), (LPBYTE*)&m_CaptureAudioThread.m_pSrcWaveFormat, &uiSize);
	if (m_CaptureAudioThread.m_pSrcWaveFormat == NULL || uiSize != sizeof(WAVEFORMATEX)) // Default Audio: Mono , 11025 Hz , 16 bits
	{
		// Make Sure Nothing Has Been Allocated!
		if (m_CaptureAudioThread.m_pSrcWaveFormat)
			delete [] m_CaptureAudioThread.m_pSrcWaveFormat;
		m_CaptureAudioThread.m_pSrcWaveFormat = (WAVEFORMATEX*) new BYTE[sizeof(WAVEFORMATEX)];
		CCaptureAudioThread::WaveInitFormat(1, 11025, 16, m_CaptureAudioThread.m_pSrcWaveFormat);
	}
	if (m_CaptureAudioThread.m_pDstWaveFormat)
		delete [] m_CaptureAudioThread.m_pDstWaveFormat;
	uiSize = 0;
	pApp->GetProfileBinary(sSection, _T("DstWaveFormat"), (LPBYTE*)&m_CaptureAudioThread.m_pDstWaveFormat, &uiSize);
	if (m_CaptureAudioThread.m_pDstWaveFormat == NULL || uiSize != sizeof(WAVEFORMATEX)) // Default Audio: Mono , 11025 Hz , 16 bits
	{
		// Make Sure Nothing Has Been Allocated!
		if (m_CaptureAudioThread.m_pDstWaveFormat)
			delete [] m_CaptureAudioThread.m_pDstWaveFormat;
		m_CaptureAudioThread.m_pDstWaveFormat = (WAVEFORMATEX*) new BYTE[sizeof(WAVEFORMATEX)];
		CCaptureAudioThread::WaveInitFormat(1, 11025, 16, m_CaptureAudioThread.m_pDstWaveFormat);
	}

	// Create dir
	::CreateDir(m_sRecordAutoSaveDir);

	// Overwrite web files in given directory
	MicroApacheUpdateWebFiles(m_sRecordAutoSaveDir);
}

void CVideoDeviceDoc::SavePlacement()
{
	if (m_bPlacementLoaded && ((CUImagerApp*)::AfxGetApp())->m_bCanSavePlacements)
	{
		WINDOWPLACEMENT wndpl;
		memset(&wndpl, 0, sizeof(wndpl));
		wndpl.length = sizeof(wndpl);
		if (GetFrame()->GetWindowPlacement(&wndpl))
			::AfxGetApp()->WriteProfileBinary(GetDevicePathName(), _T("WindowPlacement"), (BYTE*)&wndpl, sizeof(wndpl));
	}
}

void CVideoDeviceDoc::SaveSettings()
{
	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();
	CString sSection(GetDevicePathName());

	// Ini file writing is slow, especially on memory sticks and network devices
	BeginWaitCursor();

	// Store the device name to identify the entry when manually looking the registry
	// and to know that it is not the first run of the device
	pApp->WriteProfileString(sSection, _T("DeviceName"), GetDeviceName());

	// Store Placement
	SavePlacement();

	// Email Settings
	pApp->WriteProfileString(sSection, _T("SendMailFiles"), m_MovDetSendMailConfiguration.m_sFiles);
	pApp->WriteProfileInt(sSection, _T("AttachmentType"), (int)m_MovDetSendMailConfiguration.m_AttachmentType);
	pApp->WriteProfileString(sSection, _T("SendMailSubject"), m_MovDetSendMailConfiguration.m_sSubject);
	pApp->WriteProfileString(sSection, _T("SendMailTo"), m_MovDetSendMailConfiguration.m_sTo);
	pApp->WriteProfileInt(sSection, _T("SendMailPort"), m_MovDetSendMailConfiguration.m_nPort);
	pApp->WriteProfileString(sSection, _T("SendMailFrom"), m_MovDetSendMailConfiguration.m_sFrom);
	pApp->WriteProfileString(sSection, _T("SendMailHost"), m_MovDetSendMailConfiguration.m_sHost);
	pApp->WriteProfileString(sSection, _T("SendMailFromName"), m_MovDetSendMailConfiguration.m_sFromName);
	pApp->WriteProfileInt(sSection, _T("SendMailAuth"), (int)m_MovDetSendMailConfiguration.m_Auth);
	pApp->WriteSecureProfileString(sSection, _T("SendMailUsername"), m_MovDetSendMailConfiguration.m_sUsername);
	pApp->WriteSecureProfileString(sSection, _T("SendMailPassword"), m_MovDetSendMailConfiguration.m_sPassword);
	pApp->WriteProfileInt(sSection, _T("SendMailHTML"), m_MovDetSendMailConfiguration.m_bHTML);
	pApp->WriteProfileInt(sSection, _T("SendMailConnectionType"), (int)m_MovDetSendMailConfiguration.m_ConnectionType);

	// FTP Settings
	pApp->WriteProfileString(sSection, _T("MovDetFTPHost"), m_MovDetFTPUploadConfiguration.m_sHost);
	pApp->WriteProfileString(sSection, _T("MovDetFTPRemoteDir"), m_MovDetFTPUploadConfiguration.m_sRemoteDir);
	pApp->WriteProfileInt(sSection, _T("MovDetFTPPort"), m_MovDetFTPUploadConfiguration.m_nPort);
	pApp->WriteProfileInt(sSection, _T("MovDetFTPPasv"), m_MovDetFTPUploadConfiguration.m_bPasv);
	pApp->WriteProfileInt(sSection, _T("MovDetFTPProxy"), m_MovDetFTPUploadConfiguration.m_bProxy);
	pApp->WriteProfileString(sSection, _T("MovDetFTPProxyHost"), m_MovDetFTPUploadConfiguration.m_sProxy);
	pApp->WriteSecureProfileString(sSection, _T("MovDetFTPUsername"), m_MovDetFTPUploadConfiguration.m_sUsername);
	pApp->WriteSecureProfileString(sSection, _T("MovDetFTPPassword"), m_MovDetFTPUploadConfiguration.m_sPassword);
	pApp->WriteProfileInt(sSection, _T("MovDetFilesToUpload"), (int)m_MovDetFTPUploadConfiguration.m_FilesToUpload);
	pApp->WriteProfileString(sSection, _T("SnapshotFTPHost"), m_SnapshotFTPUploadConfiguration.m_sHost);
	pApp->WriteProfileString(sSection, _T("SnapshotFTPRemoteDir"), m_SnapshotFTPUploadConfiguration.m_sRemoteDir);
	pApp->WriteProfileInt(sSection, _T("SnapshotFTPPort"), m_SnapshotFTPUploadConfiguration.m_nPort);
	pApp->WriteProfileInt(sSection, _T("SnapshotFTPPasv"), m_SnapshotFTPUploadConfiguration.m_bPasv);
	pApp->WriteProfileInt(sSection, _T("SnapshotFTPProxy"), m_SnapshotFTPUploadConfiguration.m_bProxy);
	pApp->WriteProfileString(sSection, _T("SnapshotFTPProxyHost"), m_SnapshotFTPUploadConfiguration.m_sProxy);
	pApp->WriteSecureProfileString(sSection, _T("SnapshotFTPUsername"), m_SnapshotFTPUploadConfiguration.m_sUsername);
	pApp->WriteSecureProfileString(sSection, _T("SnapshotFTPPassword"), m_SnapshotFTPUploadConfiguration.m_sPassword);

	// Networking
	pApp->WriteProfileInt(sSection, _T("HTTPVideoQuality"), m_nHttpVideoQuality);
	pApp->WriteProfileInt(sSection, _T("HTTPVideoSizeX"), m_nHttpVideoSizeX);
	pApp->WriteProfileInt(sSection, _T("HTTPVideoSizeY"), m_nHttpVideoSizeY);

	// All other
	pApp->WriteProfileInt(sSection, _T("VideoView"), m_bVideoView);
	pApp->WriteProfileInt(sSection, _T("Deinterlace"), (int)m_bDeinterlace);
	pApp->WriteProfileInt(sSection, _T("Rotate180"), (int)m_bRotate180);
	pApp->WriteProfileInt(sSection, _T("RecAutoOpen"), m_bRecAutoOpen);
	pApp->WriteProfileInt(sSection, _T("RecTimeSegmentation"), m_bRecTimeSegmentation);
	pApp->WriteProfileInt(sSection, _T("TimeSegmentationIndex"), m_nTimeSegmentationIndex);
	pApp->WriteProfileString(sSection, _T("RecordAutoSaveDir"), m_sRecordAutoSaveDir);
	pApp->WriteProfileString(sSection, _T("DetectionTriggerFileName"), m_sDetectionTriggerFileName);
	pApp->WriteProfileInt(sSection, _T("SnapshotLiveJpeg"), (int)m_bSnapshotLiveJpeg);
	pApp->WriteProfileInt(sSection, _T("SnapshotHistoryJpeg"), (int)m_bSnapshotHistoryJpeg);
	pApp->WriteProfileInt(sSection, _T("SnapshotHistorySwf"), (int)m_bSnapshotHistorySwf);
	pApp->WriteProfileInt(sSection, _T("SnapshotLiveJpegFtp"), (int)m_bSnapshotLiveJpegFtp);
	pApp->WriteProfileInt(sSection, _T("SnapshotHistoryJpegFtp"), (int)m_bSnapshotHistoryJpegFtp);
	pApp->WriteProfileInt(sSection, _T("SnapshotHistorySwfFtp"), (int)m_bSnapshotHistorySwfFtp);
	pApp->WriteProfileInt(sSection, _T("ManualSnapshotAutoOpen"), (int)m_bManualSnapshotAutoOpen);
	pApp->WriteProfileString(sSection, _T("SnapshotLiveJpegName"), m_sSnapshotLiveJpegName);
	pApp->WriteProfileString(sSection, _T("SnapshotLiveJpegThumbName"), m_sSnapshotLiveJpegThumbName);
	pApp->WriteProfileInt(sSection, _T("SnapshotRate"), m_nSnapshotRate);
	pApp->WriteProfileInt(sSection, _T("SnapshotRateMs"), m_nSnapshotRateMs);
	pApp->WriteProfileInt(sSection, _T("SnapshotHistoryFrameRate"), m_nSnapshotHistoryFrameRate);
	pApp->WriteProfileInt(sSection, _T("SnapshotCompressionQuality"), m_nSnapshotCompressionQuality);
	pApp->WriteProfileInt(sSection, _T("SnapshotVideoCompressorQuality"), (int)m_fSnapshotVideoCompressorQuality);
	pApp->WriteProfileInt(sSection, _T("SnapshotThumb"), (int)m_bSnapshotThumb);
	pApp->WriteProfileInt(sSection, _T("SnapshotThumbWidth"), m_nSnapshotThumbWidth);
	pApp->WriteProfileInt(sSection, _T("SnapshotThumbHeight"), m_nSnapshotThumbHeight);
	pApp->WriteProfileInt(sSection, _T("SnapshotStartStop"), (int)m_bSnapshotStartStop);
	pApp->WriteProfileInt(sSection, _T("SnapshotStartHour"), m_SnapshotStartTime.GetHour());
	pApp->WriteProfileInt(sSection, _T("SnapshotStartMin"), m_SnapshotStartTime.GetMinute());
	pApp->WriteProfileInt(sSection, _T("SnapshotStartSec"), m_SnapshotStartTime.GetSecond());
	pApp->WriteProfileInt(sSection, _T("SnapshotStopHour"), m_SnapshotStopTime.GetHour());
	pApp->WriteProfileInt(sSection, _T("SnapshotStopMin"), m_SnapshotStopTime.GetMinute());
	pApp->WriteProfileInt(sSection, _T("SnapshotStopSec"), m_SnapshotStopTime.GetSecond());
	pApp->WriteProfileInt(sSection, _T("CaptureAudio"), m_bCaptureAudio);
	pApp->WriteProfileInt(sSection, _T("AudioCaptureDeviceID"), m_dwCaptureAudioDeviceID);
	pApp->WriteProfileInt(sSection, _T("VideoCaptureDeviceInputID"), m_nDeviceInputId);
	pApp->WriteProfileInt(sSection, _T("VideoCaptureDeviceFormatID"), m_nDeviceFormatId);
	pApp->WriteProfileInt(sSection, _T("VideoCaptureDeviceFormatWidth"), m_nDeviceFormatWidth);
	pApp->WriteProfileInt(sSection, _T("VideoCaptureDeviceFormatHeight"), m_nDeviceFormatHeight);
	pApp->WriteProfileInt(sSection, _T("MilliSecondsRecBeforeMovementBegin"), m_nMilliSecondsRecBeforeMovementBegin);
	pApp->WriteProfileInt(sSection, _T("MilliSecondsRecAfterMovementEnd"), m_nMilliSecondsRecAfterMovementEnd);
	pApp->WriteProfileInt(sSection, _T("DetectionMinLengthMilliSeconds"), m_nDetectionMinLengthMilliSeconds);
	pApp->WriteProfileInt(sSection, _T("DetectionMaxFrames"), m_nDetectionMaxFrames);
	pApp->WriteProfileInt(sSection, _T("DetectionLevel"), m_nDetectionLevel);
	pApp->WriteProfileInt(sSection, _T("DetectionZoneSize"), m_nDetectionZoneSize);
	pApp->WriteProfileInt(sSection, _T("SaveSWFMovementDetection"), m_bSaveSWFMovementDetection);
	pApp->WriteProfileInt(sSection, _T("SaveAVIMovementDetection"), m_bSaveAVIMovementDetection);
	pApp->WriteProfileInt(sSection, _T("SaveAnimGIFMovementDetection"), m_bSaveAnimGIFMovementDetection);
	pApp->WriteProfileInt(sSection, _T("SendMailMovementDetection"), m_bSendMailMovementDetection);
	pApp->WriteProfileInt(sSection, _T("FTPUploadMovementDetection"), m_bFTPUploadMovementDetection);
	pApp->WriteProfileInt(sSection, _T("DoExecCommandMovementDetection"), m_bExecCommandMovementDetection);
	pApp->WriteProfileInt(sSection, _T("ExecModeMovementDetection"), m_nExecModeMovementDetection);
		
	// Attention: GetPrivateProfileString() used by GetProfileString() for INI files
	// strips quotes -> encode quotes here!
	CString sExecCommandMovementDetection(m_sExecCommandMovementDetection); 
	sExecCommandMovementDetection.Replace(_T("\'"), _T("%singlequote%"));
	sExecCommandMovementDetection.Replace(_T("\""), _T("%doublequote%"));
	pApp->WriteProfileString(sSection, _T("ExecCommandMovementDetection"), sExecCommandMovementDetection);
	CString sExecParamsMovementDetection(m_sExecParamsMovementDetection); 
	sExecParamsMovementDetection.Replace(_T("\'"), _T("%singlequote%"));
	sExecParamsMovementDetection.Replace(_T("\""), _T("%doublequote%"));
	pApp->WriteProfileString(sSection, _T("ExecParamsMovementDetection"), sExecParamsMovementDetection);
		
	pApp->WriteProfileInt(sSection, _T("HideExecCommandMovementDetection"), m_bHideExecCommandMovementDetection);
	pApp->WriteProfileInt(sSection, _T("WaitExecCommandMovementDetection"), m_bWaitExecCommandMovementDetection);
	pApp->WriteProfileInt(sSection, _T("VideoRecFourCC"), m_dwVideoRecFourCC);
	pApp->WriteProfileInt(sSection, _T("VideoRecQuality"), (int)m_fVideoRecQuality);
	pApp->WriteProfileInt(sSection, _T("VideoRecKeyframesRate"), m_nVideoRecKeyframesRate);
	pApp->WriteProfileInt(sSection, _T("VideoRecDataRate"), m_nVideoRecDataRate);
	pApp->WriteProfileInt(sSection, _T("VideoRecQualityBitrate"), m_nVideoRecQualityBitrate);
	pApp->WriteProfileInt(sSection, _T("VideoDetFourCC"), m_dwVideoDetFourCC);
	pApp->WriteProfileInt(sSection, _T("VideoDetQuality"), (int)m_fVideoDetQuality);
	pApp->WriteProfileInt(sSection, _T("VideoDetQualityBitrate"), m_nVideoDetQualityBitrate);
	pApp->WriteProfileInt(sSection, _T("VideoDetKeyframesRate"), m_nVideoDetKeyframesRate);
	pApp->WriteProfileInt(sSection, _T("VideoDetDataRate"), m_nVideoDetDataRate);
	pApp->WriteProfileInt(sSection, _T("VideoDetSwfFourCC"), m_dwVideoDetSwfFourCC);
	pApp->WriteProfileInt(sSection, _T("VideoDetSwfQuality"), (int)m_fVideoDetSwfQuality);
	pApp->WriteProfileInt(sSection, _T("VideoDetSwfQualityBitrate"), m_nVideoDetSwfQualityBitrate);
	pApp->WriteProfileInt(sSection, _T("VideoDetSwfKeyframesRate"), m_nVideoDetSwfKeyframesRate);
	pApp->WriteProfileInt(sSection, _T("VideoDetSwfDataRate"), m_nVideoDetSwfDataRate);
	pApp->WriteProfileInt(sSection, _T("DetectionStartStop"), m_nDetectionStartStop);
	pApp->WriteProfileInt(sSection, _T("DetectionSunday"), (int)m_bDetectionSunday);
	pApp->WriteProfileInt(sSection, _T("DetectionMonday"), (int)m_bDetectionMonday);
	pApp->WriteProfileInt(sSection, _T("DetectionTuesday"), (int)m_bDetectionTuesday);
	pApp->WriteProfileInt(sSection, _T("DetectionWednesday"), (int)m_bDetectionWednesday);
	pApp->WriteProfileInt(sSection, _T("DetectionThursday"), (int)m_bDetectionThursday);
	pApp->WriteProfileInt(sSection, _T("DetectionFriday"), (int)m_bDetectionFriday);
	pApp->WriteProfileInt(sSection, _T("DetectionSaturday"), (int)m_bDetectionSaturday);
	pApp->WriteProfileInt(sSection, _T("DetectionStartHour"), m_DetectionStartTime.GetHour());
	pApp->WriteProfileInt(sSection, _T("DetectionStartMin"), m_DetectionStartTime.GetMinute());
	pApp->WriteProfileInt(sSection, _T("DetectionStartSec"), m_DetectionStartTime.GetSecond());
	pApp->WriteProfileInt(sSection, _T("DetectionStopHour"), m_DetectionStopTime.GetHour());
	pApp->WriteProfileInt(sSection, _T("DetectionStopMin"), m_DetectionStopTime.GetMinute());
	pApp->WriteProfileInt(sSection, _T("DetectionStopSec"), m_DetectionStopTime.GetSecond());
	pApp->WriteProfileInt(sSection, _T("ShowFrameTime"), m_bShowFrameTime);
	pApp->WriteProfileInt(sSection, _T("RefFontSize"), m_nRefFontSize);
	pApp->WriteProfileInt(sSection, _T("ShowMovementDetections"), m_bShowMovementDetections);
	pApp->WriteProfileInt(sSection, _T("IntensityLimit"), m_nMovementDetectorIntensityLimit);
	pApp->WriteProfileInt(sSection, _T("AnimatedGifWidth"), m_dwAnimatedGifWidth);
	pApp->WriteProfileInt(sSection, _T("AnimatedGifHeight"), m_dwAnimatedGifHeight);
	pApp->WriteProfileInt(sSection, _T("DeleteRecordingsOlderThanDays"), m_nDeleteRecordingsOlderThanDays);

	// Store detection zones only if the total size has already been calculated by OnThreadSafeInitMovDet()
	if (m_lMovDetTotalZones > 0)
	{
		pApp->WriteProfileInt(sSection, _T("MovDetTotalZones"), m_lMovDetTotalZones);
		SaveZonesSettings();
	}

	if (m_CaptureAudioThread.m_pSrcWaveFormat)
		pApp->WriteProfileBinary(sSection, _T("SrcWaveFormat"), (LPBYTE)m_CaptureAudioThread.m_pSrcWaveFormat, sizeof(WAVEFORMATEX));
	if (m_CaptureAudioThread.m_pDstWaveFormat)
		pApp->WriteProfileBinary(sSection, _T("DstWaveFormat"), (LPBYTE)m_CaptureAudioThread.m_pDstWaveFormat, sizeof(WAVEFORMATEX));
	pApp->WriteProfileInt(sSection, _T("VideoProcessorMode"), m_dwVideoProcessorMode);
	unsigned int nSize = sizeof(m_dFrameRate);
	pApp->WriteProfileBinary(sSection, _T("FrameRate"), (LPBYTE)&m_dFrameRate, nSize);

	// Ini file writing is slow, especially on memory sticks and network devices
	EndWaitCursor();
}

void CVideoDeviceDoc::OnEditExportZones() 
{
	GetView()->ForceCursor();
	CNoVistaFileDlg fd(	FALSE,
						_T("ini"),
						_T("MovementDetectionZones.ini"),
						OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
						_T("INI Files (*.ini)|*.ini||"));
	if (fd.DoModal() == IDOK)
		ExportDetectionZones(fd.GetPathName());
	GetView()->ForceCursor(FALSE);
}

void CVideoDeviceDoc::ExportDetectionZones(const CString& sFileName)
{
	// Ini file writing is slow, especially on memory sticks and network devices
	BeginWaitCursor();

	// Remove previous file if any
	::DeleteFile(sFileName);

	// Store settings to the export ini file
	::WriteProfileIniInt(_T("MovementDetectionZones"), _T("MovDetTotalZones"), m_lMovDetTotalZones, sFileName);
	::WriteProfileIniInt(_T("MovementDetectionZones"), _T("DetectionZoneSize"), m_nDetectionZoneSize, sFileName);
	for (int i = 0 ; i < m_lMovDetTotalZones ; i++)
	{
		CString sZone;
		sZone.Format(MOVDET_ZONE_FORMAT, i);
		::WriteProfileIniInt(_T("MovementDetectionZones"), sZone, m_DoMovementDetection[i], sFileName);
	}
	
	// Ini file writing is slow, especially on memory sticks and network devices
	EndWaitCursor();
}

void CVideoDeviceDoc::OnEditImportZones() 
{
	GetView()->ForceCursor();
	CNoVistaFileDlg fd(	TRUE,
						_T("ini"),
						_T(""),
						OFN_HIDEREADONLY, // Hides the Read Only check box
						_T("INI Files (*.ini)|*.ini||"));
	if (fd.DoModal() == IDOK)
		ImportDetectionZones(fd.GetPathName());
	GetView()->ForceCursor(FALSE);
}

void CVideoDeviceDoc::ImportDetectionZones(const CString& sFileName)
{
	// Ini file writing is slow, especially on memory sticks and network devices
	BeginWaitCursor();

	// Load settings from the import ini file
	// Note: changing m_nDetectionZoneSize will call OnThreadSafeInitMovDet()
	// which re-loads the settings stored below
	long lImportMovDetTotalZones = MIN(MOVDET_MAX_ZONES,
							::GetProfileIniInt(_T("MovementDetectionZones"), _T("MovDetTotalZones"), 0, sFileName));
	m_nDetectionZoneSize = ::GetProfileIniInt(_T("MovementDetectionZones"), _T("DetectionZoneSize"), 0, sFileName);
	if (m_pMovementDetectionPage)
	{
		CComboBox* pComboBox = (CComboBox*)m_pMovementDetectionPage->GetDlgItem(IDC_DETECTION_ZONE_SIZE);
		pComboBox->SetCurSel(m_nDetectionZoneSize);
	}
	for (int i = 0 ; i < lImportMovDetTotalZones ; i++)
	{
		CString sZone;
		sZone.Format(MOVDET_ZONE_FORMAT, i);
		m_DoMovementDetection[i] = (BYTE)::GetProfileIniInt(_T("MovementDetectionZones"), sZone, 1, sFileName);
	}

	// Store Zones Settings
	::AfxGetApp()->WriteProfileInt(GetDevicePathName(), _T("MovDetTotalZones"), lImportMovDetTotalZones);
	SaveZonesSettings();

	// Ini file writing is slow, especially on memory sticks and network devices
	EndWaitCursor();
}

BOOL CVideoDeviceDoc::InitOpenDxCapture(int nId)
{
	m_pDxCapture = new CDxCapture;
	if (m_pDxCapture)
	{
		m_pDxCapture->SetDoc(this);
		BOOL bOpened = m_pDxCapture->Open(	GetView()->GetSafeHwnd(),
											nId,
											m_dFrameRate,
											m_nDeviceFormatId,
											m_nDeviceFormatWidth,
											m_nDeviceFormatHeight);
		if (!bOpened)
			bOpened = m_pDxCapture->Open(	GetView()->GetSafeHwnd(),
											nId,
											m_dFrameRate,
											m_nDeviceFormatId,
											m_nDeviceFormatWidth,
											m_nDeviceFormatHeight,
											&MEDIASUBTYPE_YUY2);
		if (bOpened)
		{
			// Update format
			OnChangeDxVideoFormat();

			// Start capturing video data
			StopProcessFrame(PROCESSFRAME_DXOPEN);
			if (m_pDxCapture->Run())
			{
				// Select Input Id for Capture Devices with multiple inputs (S-Video, TV-Tuner,...)
				if (m_nDeviceInputId >= 0 && m_nDeviceInputId < m_pDxCapture->GetInputsCount())
				{
					if (!m_pDxCapture->SetCurrentInput(m_nDeviceInputId))
						m_nDeviceInputId = -1;
				}
				else
					m_nDeviceInputId = m_pDxCapture->SetDefaultInput();

				// Some devices need that...
				// Process frame must still be stopped when calling Dx Stop()!
				m_pDxCapture->Stop();
				m_pDxCapture->Run();

				// Restart process frame
				StartProcessFrame(PROCESSFRAME_DXOPEN);

				// Start Audio Capture Thread
				if (m_bCaptureAudio)
					m_CaptureAudioThread.Start();

				// Title
				SetDocumentTitle();

				return TRUE;
			}
		}
		delete m_pDxCapture;
		m_pDxCapture = NULL;
	}
	return FALSE;
}

BOOL CVideoDeviceDoc::OpenVideoDevice(int nId)
{
	// Already open?
	if (m_pDxCapture)
		return TRUE;

	// Device path and name
	CString sDevicePathName = CDxCapture::GetDevicePath(nId);
	CString sDeviceName = CDxCapture::GetDeviceName(nId);
	sDevicePathName.Replace(_T('\\'), _T('/'));

	// Load Settings
	LoadSettings(DEFAULT_FRAMERATE, sDevicePathName, sDeviceName);

	// Start Delete Detections Thread
	if (!m_DeleteThread.IsAlive())
		m_DeleteThread.Start(THREAD_PRIORITY_LOWEST);

	// Reset vars
	m_dwFrameCountUp = 0U;
	m_dwNextSnapshotUpTime = ::timeGetTime();
	::InterlockedExchange(&m_lCurrentInitUpTime, (LONG)m_dwNextSnapshotUpTime);

	// Init and Open Dx Capture
	if (InitOpenDxCapture(nId))
		return TRUE;

	// Failure
	ConnectErr(ML_STRING(1466, "The capture device is already in use or not compatible"), sDevicePathName, sDeviceName);
	return FALSE;
}

void CVideoDeviceDoc::InitHttpGetFrameLocations()
{
	// Free (always leave first element!)
	while (m_HttpGetFrameLocations.GetSize() > 1)
		m_HttpGetFrameLocations.RemoveAt(m_HttpGetFrameLocations.GetUpperBound());

	// MJPEG
	if (m_nNetworkDeviceTypeMode == OTHERONE_SP)
	{
		m_HttpGetFrameLocations.Add(_T("/videostream.cgi"));				// 7Links, Acromedia, Agasio, AirLink, Apexis, Aztech, BiQu, BSTI, cowKey,
																			// CVLM, Dericam, EasyN, EasySE, Elro, Eminent, ENSIDIO, EyeSight, EZCam,
																			// Foscam, Heden, HooToo, ICam, iCam+, INSTAR, KARE, Loftek, Maygion,
																			// PROCCTV, Smarthome, Solwise, Storage Options, Tenvis, Vonnic, Vstarcam,
																			// Wanscam, Wansview, X10, Xenta, Zmodo
		
		m_HttpGetFrameLocations.Add(_T("/video.cgi"));						// 7Links, ABS, AirLink, AirLive, A-Link, Bowya, corega, D-Link, EasySE,
																			// Foscam, Genius, Grandtec, Heden, Imogen, Intellinet, Loftek, LongShine,
																			// Nilox, Ovislink, Planet, QNAP, Rosewill, Savitmicro, Topica, TrendNet, Trust
		
		m_HttpGetFrameLocations.Add(_T("/mjpeg.cgi"));						// AirLink, Cellvision, corega, Digicom, D-Link, Elro, EMSTONE, Hamlet, LinkPro,
																			// NeuFusion, QNAP, Sertek, Sparklan, Surecom, Topcom, TrendNet, Trust
		
		m_HttpGetFrameLocations.Add(_T("/cgi/mjpg/mjpeg.cgi"));				// 7Links, AirLink, A-Link, Allnet, Conceptronic, corega, Digicom, Fitivision,
																			// Marmitek, QNAP, Swann, TrendNet, Zonet
		
		m_HttpGetFrameLocations.Add(_T("/video.mjpg"));						// ABUS, AirLive, Allnet, Beward, GadSpot, Intellinet, LevelOne, Lorex,
																			// Planet, TP-Link, Vivotek, Yawcam, Zavio
		
		m_HttpGetFrameLocations.Add(_T("/mjpg/video.mjpg"));				// AirLink, AirLive, BlueStork, Edimax, Hawking, Planet, Rosewill, Savitmicro,
																			// Sitecom, Zonet
		
		m_HttpGetFrameLocations.Add(_T("/cgi/mjpg/mjpg.cgi"));				// 7Links, Activa, AirLink, Digitus, Encore, IPUX, QNAP, TrendNet
		
		m_HttpGetFrameLocations.Add(_T("/img/video.mjpeg"));				// Allnet, Cisco, Clas, LevelOne, Linksys, Sercomm, Sitecom
		
		m_HttpGetFrameLocations.Add(_T("/VIDEO.CGI"));						// Conceptronic, D-Link, Elro, Marmitek, Repotec, Sparklan
		
		m_HttpGetFrameLocations.Add(_T("/MJPEG.CGI"));						// Conceptronic, D-Link, Elro, Marmitek, Repotec
		
		m_HttpGetFrameLocations.Add(_T("/img/mjpeg.cgi"));					// Planet, Sony, Allnet, NorthQ
		
		m_HttpGetFrameLocations.Add(_T("/ipcam/stream.cgi"));				// Appro, D-Link, Eneo, HDL
		
		m_HttpGetFrameLocations.Add(_T("/ipcam/mjpeg.cgi"));				// AirLive, Appro, Aviosys, MESSOA
		
		m_HttpGetFrameLocations.Add(_T("/image.cgi?type=motion"));			// ALinking, Asante, TELCA, VISIONxIP
		
		m_HttpGetFrameLocations.Add(_T("/img/mjpeg.jpg"));					// Cisco, Linksys, Planet
		
		m_HttpGetFrameLocations.Add(_T("/mjpeg"));							// Arecont, Celius, Tenvis
		
		m_HttpGetFrameLocations.Add(_T("/video/mjpg.cgi"));					// D-Link, Sparklan, TrendNet
		
		m_HttpGetFrameLocations.Add(_T("/mjpegStreamer.cgi"));				// Compro, PROCCTV, Zyxel
		
		m_HttpGetFrameLocations.Add(_T("/-wvhttp-01-/GetOneShot?frame_count=0")); // Canon, NuSpectra
		
		m_HttpGetFrameLocations.Add(_T("/video2.mjpg"));					// Cisco, D-Link
		
		m_HttpGetFrameLocations.Add(_T("/cgi-bin/getimage.cgi?motion=1"));	// GadSpot
		
		m_HttpGetFrameLocations.Add(_T("/nph-mjpeg.cgi"));					// Stardot

		m_HttpGetFrameLocations.Add(_T("/cgi-bin/cmd/system?GET_STREAM"));	// ACTi

		m_HttpGetFrameLocations.Add(_T("/control/faststream.jpg?stream=full"));	// Mobotix
		
		m_HttpGetFrameLocations.Add(CString(_T("/image.cgi?mode=http")) +
									_T("&id=") + HTTPGETFRAME_USERNAME_PLACEHOLDER + 
									_T("&passwd=") + HTTPGETFRAME_PASSWORD_PLACEHOLDER);	// Intellinet

		m_HttpGetFrameLocations.Add(CString(_T("/cgi-bin/CGIProxy.fcgi?cmd=setSubStreamFormat&format=1")) +
									_T("&usr=") + HTTPGETFRAME_USERNAME_PLACEHOLDER + 
									_T("&pwd=") + HTTPGETFRAME_PASSWORD_PLACEHOLDER);		// Enable MJPG for Foscam HD
		m_HttpGetFrameLocations.Add(CString(_T("/cgi-bin/CGIStream.cgi?cmd=GetMJStream")) +
									_T("&usr=") + HTTPGETFRAME_USERNAME_PLACEHOLDER + 
									_T("&pwd=") + HTTPGETFRAME_PASSWORD_PLACEHOLDER);		// Foscam HD (mjpeg stream is VGA resolution @ 15fps)
	}
	// JPEG
	else if (m_nNetworkDeviceTypeMode == OTHERONE_CP)
	{
		m_HttpGetFrameLocations.Add(_T("/snapshot.jpg"));					// Aviosys, BluePix, Compro, Digitus, EasyN, Edimax, Eminent, GadSpot,
																			// Gembird, GKB, Goscam, Hama, Hungtek, Intellinet, LevelOne, LogiLink,
																			// LTS, Micronet, Planet, Planex, PROCCTV, Rimax, Sharx, SmartIndustry,
																			// Swann, ZyXEL

		m_HttpGetFrameLocations.Add(_T("/image.jpg"));						// AirLink, Asante, D-Link, Genius, Hawking, Sparklan, TrendNet, Trust 
		
		m_HttpGetFrameLocations.Add(_T("/IMAGE.JPG"));						// Conceptronic, D-Link, Elro, Genius, Hawking, Marmitek, Planet,
																			// Repotec, Sparklan
		
		m_HttpGetFrameLocations.Add(_T("/goform/video2"));					// AirLink, Planet, Repotec, Sparklan, TrendNet
		
		m_HttpGetFrameLocations.Add(_T("/goform/capture"));					// AirLink, Cellvision, corega, Digitus, Micronet, NeuFusion, Planet,
																			// QNAP, Quimz, Repotec, Sparklan, TrendNet
		
		m_HttpGetFrameLocations.Add(_T("/cgi/jpg/image.cgi"));				// 7Links, Activa, AirLink, A-Link, Allnet, Conceptronic, Digicom,
																			// Digitus, Encore, Fitivision, IPUX, Marmitek, QNAP, Swann, TrendNet
		
		m_HttpGetFrameLocations.Add(_T("/Jpeg/CamImg.jpg"));				// Aviosys, GadSpot, Grandtec, Orite, Pixord, Planet, Security,
																			// Skyway Security, Solwise, SVAT, Sweex, Toshiba, Veo/Vidi, WowWee, Yoko
		
		m_HttpGetFrameLocations.Add(_T("/jpg/image.jpg"));					// ABUS, AirLink, AirLive, Allnet, AVS, Axis, Beward, CNB, Edimax,
																			// GadSpot, Hawking, Intellinet, Lorex, Planet, Rosewill, Savitmicro,
																			// Sitecom, TP-Link, TSM, Vivotek, Zavio
		
		m_HttpGetFrameLocations.Add(_T("/cgi-bin/video.jpg"));				// 4XEM, ABUS, Allnet, Cellvision, ConnectionNC, Digi-Lan, Digitus, D-Link,
																			// Eyseo, Hawking, JVC, LevelOne, Moxa, NeuFusion, Ovislink, Planex, Quimz,
																			// Sertek, Telefonica, TrendNet, Trendware, Vivotek
		
		m_HttpGetFrameLocations.Add(_T("/snap.jpg"));						// Agasio, A-MTK, Apexis, Aztech, Bosch, Dericam, EasyN, EasySE, EZCam,
																			// Foscam, iCam+, Instar, IPC, IPCMontor, Jen-Fu, Loftek, Maygion,
																			// Micronet, PelcoNet, PROCCTV, SmartEye, Vonnic, Wanscam, Wansview, Xenta

		m_HttpGetFrameLocations.Add(_T("/tmpfs/auto.jpg"));					// Apexis, Astak, Aztech, Dannovo, Dericam, Ensidio, Foscam, iCam+,
																			// Instar, IPCMontor, Loftek, Maygion, Niceview, PROCCTV, SmartEye,
																			// Suneyes, Tenvis, Vonnic, Vstarcam, Wansview, Xenta

		m_HttpGetFrameLocations.Add(_T("/img/snapshot.cgi"));				// Allnet, Cisco, Clas, LevelOne, Linksys, NorthQ, Sercomm, Sitecom

		m_HttpGetFrameLocations.Add(_T("/image/jpeg.cgi"));					// D-Link, Sparklan, TrendNet
		
		m_HttpGetFrameLocations.Add(_T("/cgi-bin/viewer/video.jpg"));		// ABUS, D-Link, Toshiba, Vivotek
		
		m_HttpGetFrameLocations.Add(_T("/record/current.jpg"));				// Mobotix
		
		m_HttpGetFrameLocations.Add(_T("/cgi-bin/getimage.cgi?motion=0"));	// GadSpot

		m_HttpGetFrameLocations.Add(_T("/oneshotimage.jpg"));				// Sony
		
		m_HttpGetFrameLocations.Add(_T("/-wvhttp-01-/GetLiveImage"));		// Canon

		m_HttpGetFrameLocations.Add(_T("/netcam.jpg"));						// Stardot

		m_HttpGetFrameLocations.Add(_T("/cgi-bin/encoder?SNAPSHOT"));		// ACTi

		m_HttpGetFrameLocations.Add(_T("/capture1.jpg"));					// Active WebCam Video Surveillance Software

		m_HttpGetFrameLocations.Add(CString(_T("/cgi-bin/CGIProxy.fcgi?cmd=snapPicture2")) +
									_T("&usr=") + HTTPGETFRAME_USERNAME_PLACEHOLDER + 
									_T("&pwd=") + HTTPGETFRAME_PASSWORD_PLACEHOLDER); // Foscam HD
	}

	// Finally add the mixed commands (it depends from the maker whether those commands return JPEG or MJPEG)
	if (m_nNetworkDeviceTypeMode == OTHERONE_SP ||
		m_nNetworkDeviceTypeMode == OTHERONE_CP)
	{
		m_HttpGetFrameLocations.Add(_T("/GetData.cgi"));					// Active, ALinking, Asante, ASIP, Asoni, Aviosys, Aviptek, BVUSA,
																			// Defender, GadSpot, Grand, Hesavision, Hunt, INVID, Lorex, Lupus,
																			// Orite, Piczel, Pixord, Planet, Sertek, Skyway Security, SVAT,
																			// Sweex, Toshiba, TrendNet, Veo, Well, WowWee
		
		m_HttpGetFrameLocations.Add(_T("/getimage"));						// Lorex, Merit, Pixord, Seteye, Speco, TCLINK
		
		m_HttpGetFrameLocations.Add(_T("/image"));							// Arecont, Sony
		
		m_HttpGetFrameLocations.Add(_T("/snapshot.cgi"));					// 7Links, Acromedia, AirLink, Apexis, Aviosys, Aztech, BiQu, BSTI, CVLM,
																			// Dericam, EasyN, EasySE, Edimax, Elro, Eminent, EZCam, Foscam, GKB, Hama,
																			// Heden, HooToo, ICam, iCam+, INSTAR, Intellinet, Loftek, LogiLink, LongShine,
																			// Maygion, Micronet, Nilox, Planet, Planex, PROCCTV, Smarthome, Solwise,
																			// Storage Options, Tenvis, Ubiquiti, Vonnic, Wansview, X10, XTS, Zmodo
		
		m_HttpGetFrameLocations.Add(_T("/image.cgi"));						// AirLive, A-Link, ALinking, Asante, Intellinet, MonoPrice, NetMedia, 
																			// Planet, Rosewill, Savitmicro, TELCA, Topica, VISIONxIP
	}
}

double CVideoDeviceDoc::GetDefaultNetworkFrameRate(NetworkDeviceTypeMode nNetworkDeviceTypeMode) 
{
	switch(nNetworkDeviceTypeMode)
	{
		case OTHERONE_SP :	return HTTPSERVERPUSH_DEFAULT_FRAMERATE;
		case OTHERONE_CP :	return HTTPCLIENTPOLL_DEFAULT_FRAMERATE;
		case AXIS_SP :		return HTTPSERVERPUSH_DEFAULT_FRAMERATE;
		case AXIS_CP :		return HTTPCLIENTPOLL_DEFAULT_FRAMERATE;
		case PANASONIC_SP :	return HTTPSERVERPUSH_DEFAULT_FRAMERATE;
		case PANASONIC_CP :	return HTTPCLIENTPOLL_DEFAULT_FRAMERATE;
		case PIXORD_SP :	return HTTPSERVERPUSH_DEFAULT_FRAMERATE;
		case PIXORD_CP :	return HTTPCLIENTPOLL_DEFAULT_FRAMERATE;
		case EDIMAX_SP :	return HTTPSERVERPUSH_EDIMAX_DEFAULT_FRAMERATE;
		case EDIMAX_CP :	return HTTPCLIENTPOLL_DEFAULT_FRAMERATE;
		case TPLINK_SP :	return HTTPSERVERPUSH_DEFAULT_FRAMERATE;
		case TPLINK_CP :	return HTTPCLIENTPOLL_DEFAULT_FRAMERATE;
		case FOSCAM_SP :	return HTTPSERVERPUSH_DEFAULT_FRAMERATE;
		case FOSCAM_CP :	return HTTPCLIENTPOLL_DEFAULT_FRAMERATE;
		default :			return HTTPCLIENTPOLL_DEFAULT_FRAMERATE;
	}
}

// sAddress: Must have the IP:Port:FrameLocation:NetworkDeviceTypeMode or
//           HostName:Port:FrameLocation:NetworkDeviceTypeMode Format
// Note: FrameLocation is m_HttpGetFrameLocations[0]
BOOL CVideoDeviceDoc::OpenGetVideo(CString sAddress, DWORD dwConnectDelay/*=0U*/) 
{
	// Init Host, Port, FrameLocation and NetworkDeviceTypeMode
	int i = sAddress.ReverseFind(_T(':'));
	if (i >= 0)
	{
		CString sNetworkDeviceTypeMode = sAddress.Right(sAddress.GetLength() - i - 1);
		NetworkDeviceTypeMode nNetworkDeviceTypeMode = (NetworkDeviceTypeMode)_tcstol(sNetworkDeviceTypeMode.GetBuffer(0), NULL, 10);
		sNetworkDeviceTypeMode.ReleaseBuffer();
		if (nNetworkDeviceTypeMode >= OTHERONE_SP && nNetworkDeviceTypeMode < LAST_DEVICE)
			m_nNetworkDeviceTypeMode = nNetworkDeviceTypeMode;
		else
			m_nNetworkDeviceTypeMode = OTHERONE_SP;
		sAddress = sAddress.Left(i);
		i = sAddress.ReverseFind(_T(':'));
		if (i >= 0)
		{
			m_HttpGetFrameLocations[0] = sAddress.Right(sAddress.GetLength() - i - 1);
			sAddress = sAddress.Left(i);
			i = sAddress.ReverseFind(_T(':'));
			if (i >= 0)
			{
				CString sPort = sAddress.Right(sAddress.GetLength() - i - 1);
				int nPort = _tcstol(sPort.GetBuffer(0), NULL, 10);
				sPort.ReleaseBuffer();
				if (nPort > 0 && nPort <= 65535) // Port 0 is Reserved
					m_nGetFrameVideoPort = nPort;
				else
					m_nGetFrameVideoPort = DEFAULT_TCP_PORT;
				m_sGetFrameVideoHost = sAddress.Left(i);
			}
			else
			{
				m_sGetFrameVideoHost = sAddress;
				m_nGetFrameVideoPort = DEFAULT_TCP_PORT;
			}
		}
		else
		{
			m_sGetFrameVideoHost = sAddress;
			m_nGetFrameVideoPort = DEFAULT_TCP_PORT;
			m_HttpGetFrameLocations[0] = _T("/");
		}
	}
	else
	{
		m_sGetFrameVideoHost = sAddress;
		m_nGetFrameVideoPort = DEFAULT_TCP_PORT;
		m_HttpGetFrameLocations[0] = _T("/");
		m_nNetworkDeviceTypeMode = OTHERONE_SP;
	}

	// Free if Necessary
	if (m_pGetFrameNetCom)
	{
		delete m_pGetFrameNetCom;
		m_pGetFrameNetCom = NULL;
	}

	// Init http get frame locations array
	InitHttpGetFrameLocations();

	// Allocate
	m_pGetFrameNetCom = (CNetCom*)new CNetCom;

	// Load Settings
	LoadSettings(GetDefaultNetworkFrameRate(m_nNetworkDeviceTypeMode), GetDevicePathName(), GetDeviceName());

	// Start Delete Detections Thread
	if (!m_DeleteThread.IsAlive())
		m_DeleteThread.Start(THREAD_PRIORITY_LOWEST);

	// Reset vars
	m_dwFrameCountUp = 0U;
	m_dwNextSnapshotUpTime = ::timeGetTime();
	::InterlockedExchange(&m_lCurrentInitUpTime, (LONG)m_dwNextSnapshotUpTime);

	// Connect
	if (!ConnectGetFrame(dwConnectDelay))
	{
		ConnectErr(ML_STRING(1465, "Cannot connect to the specified network device or server"), GetDevicePathName(), GetDeviceName());
		return FALSE;
	}

	// Start Audio Capture
	if (m_bCaptureAudio)
		m_CaptureAudioThread.Start();

	return TRUE;
}

BOOL CVideoDeviceDoc::OpenGetVideo(CHostPortDlg* pDlg) 
{
	ASSERT(pDlg);
	CHostPortDlg::ParseUrl(	pDlg->m_sHost, pDlg->m_nPort, pDlg->m_nDeviceTypeMode,
							m_sGetFrameVideoHost, (int&)m_nGetFrameVideoPort,
							m_HttpGetFrameLocations[0], (int&)m_nNetworkDeviceTypeMode);

	// Free if Necessary
	if (m_pGetFrameNetCom)
	{
		delete m_pGetFrameNetCom;
		m_pGetFrameNetCom = NULL;
	}

	// Init http get frame locations array
	InitHttpGetFrameLocations();

	// Allocate
	m_pGetFrameNetCom = (CNetCom*)new CNetCom;

	// Load Settings
	LoadSettings(GetDefaultNetworkFrameRate(m_nNetworkDeviceTypeMode), GetDevicePathName(), GetDeviceName());

	// Start Delete Detections Thread
	if (!m_DeleteThread.IsAlive())
		m_DeleteThread.Start(THREAD_PRIORITY_LOWEST);

	// Reset vars
	m_dwFrameCountUp = 0U;
	m_dwNextSnapshotUpTime = ::timeGetTime();
	::InterlockedExchange(&m_lCurrentInitUpTime, (LONG)m_dwNextSnapshotUpTime);

	// Connect
	if (!ConnectGetFrame())
	{
		ConnectErr(ML_STRING(1465, "Cannot connect to the specified network device or server"), GetDevicePathName(), GetDeviceName());
		return FALSE;
	}

	// Start Audio Capture
	if (m_bCaptureAudio)
		m_CaptureAudioThread.Start();

	return TRUE;
}

CString CVideoDeviceDoc::MakeJpegManualSnapshotFileName(const CTime& Time)
{
	CString sYearMonthDayDir(_T(""));

	// Snapshot time
	CString sTime = Time.Format(_T("%Y_%m_%d_%H_%M_%S"));

	// Adjust Directory Name
	CString sSnapshotAutoSaveDir = m_sRecordAutoSaveDir;
	sSnapshotAutoSaveDir.TrimRight(_T('\\'));

	// Create directory if necessary
	if (sSnapshotAutoSaveDir != _T(""))
	{
		DWORD dwAttrib = ::GetFileAttributes(sSnapshotAutoSaveDir);
		if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
			::CreateDir(sSnapshotAutoSaveDir);
		if (!CVideoDeviceDoc::CreateCheckYearMonthDayDir(Time, sSnapshotAutoSaveDir, sYearMonthDayDir))
			return _T("");
	}

	// Return file name
	if (sYearMonthDayDir == _T(""))
		return _T("manualshot_") + sTime + _T(".jpg");
	else
		return sYearMonthDayDir + _T("\\") + _T("manualshot_") + sTime + _T(".jpg");
}

void CVideoDeviceDoc::FreeAVIFile()
{
	if (m_pAVRec)
	{
		if (m_pAVRec->GetFrameCount(m_pAVRec->VideoStreamNumToStreamNum(ACTIVE_VIDEO_STREAM)) == 0)
		{
			CString sFileName = m_pAVRec->GetFileName();
			delete m_pAVRec;
			::DeleteFile(sFileName);
		}
		else
			delete m_pAVRec;
		m_pAVRec = NULL;
	}
}

BOOL CVideoDeviceDoc::MakeAVRec(CAVRec** ppAVRec)
{
	// Check
	if (!ppAVRec)
		return FALSE;

	// Record time and file name
	CString sFileName;
	CString sYearMonthDayDir;
	CTime CurrentTime = CTime::GetCurrentTime();
	CString sCurrentTime = CurrentTime.Format(_T("%Y_%m_%d_%H_%M_%S"));
	CString sRecordAutoSaveDir = m_sRecordAutoSaveDir;
	sRecordAutoSaveDir.TrimRight(_T('\\'));
	if (sRecordAutoSaveDir != _T(""))
	{
		DWORD dwAttrib = ::GetFileAttributes(sRecordAutoSaveDir);
		if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
			::CreateDir(sRecordAutoSaveDir);
		if (!CVideoDeviceDoc::CreateCheckYearMonthDayDir(CurrentTime, sRecordAutoSaveDir, sYearMonthDayDir))
			return FALSE;
	}
	if (sYearMonthDayDir == _T(""))
		sFileName = _T("rec_") + sCurrentTime + _T(".avi");
	else
		sFileName = sYearMonthDayDir + _T("\\") + _T("rec_") + sCurrentTime + _T(".avi");

	// Allocate
	*ppAVRec = new CAVRec;
	if (!(*ppAVRec))
		return FALSE;

	// Set File Name
	if (!(*ppAVRec)->Init(sFileName, 0, true)) // fast encoding!
		return FALSE;

	// Set File Info
	if (!(*ppAVRec)->SetInfo(	_T("Rec: ") + ::MakeDateLocalFormat(CurrentTime) +
								_T(", ") + ::MakeTimeLocalFormat(CurrentTime, TRUE),
								APPNAME_NOEXT, MYCOMPANY_WEB))
		return FALSE;

	// Add Video Stream
	BITMAPINFOFULL SrcBmi;
	BITMAPINFOFULL DstBmi;
	memset(&SrcBmi, 0, sizeof(BITMAPINFOFULL));
	memset(&DstBmi, 0, sizeof(BITMAPINFOFULL));
	memcpy(&SrcBmi, &m_ProcessFrameBMI, CDib::GetBMISize((LPBITMAPINFO)&m_ProcessFrameBMI));
	if (m_dwVideoRecFourCC == BI_RGB)
		memcpy(&DstBmi, &SrcBmi, CDib::GetBMISize((LPBITMAPINFO)&SrcBmi));
	else
	{
		DstBmi.bmiHeader.biSize = SrcBmi.bmiHeader.biSize;
		DstBmi.bmiHeader.biWidth = SrcBmi.bmiHeader.biWidth;
		DstBmi.bmiHeader.biHeight = SrcBmi.bmiHeader.biHeight;
		DstBmi.bmiHeader.biPlanes = SrcBmi.bmiHeader.biPlanes;
		DstBmi.bmiHeader.biCompression = m_dwVideoRecFourCC;
	}
	AVRational FrameRate;
	if (m_dEffectiveFrameRate > 0.0)
		FrameRate = av_d2q(m_dEffectiveFrameRate, MAX_SIZE_FOR_RATIONAL);
	else
		FrameRate = av_d2q(m_dFrameRate, MAX_SIZE_FOR_RATIONAL);
	int nQualityBitrate = m_nVideoRecQualityBitrate;
	if (DstBmi.bmiHeader.biCompression == FCC('MJPG'))
		nQualityBitrate = 0;
	if ((*ppAVRec)->AddVideoStream(	(LPBITMAPINFO)(&SrcBmi),		// Source Video Format
									(LPBITMAPINFO)(&DstBmi),		// Destination Video Format
									FrameRate.num,					// Rate
									FrameRate.den,					// Scale
									nQualityBitrate == 1 ? m_nVideoRecDataRate : 0,			// Bitrate in bits/s
									m_nVideoRecKeyframesRate,		// Keyframes Rate	
									nQualityBitrate == 0 ? m_fVideoRecQuality : 0.0f) < 0)	// 0.0f use bitrate, 2.0f best quality, 31.0f worst quality		
		return FALSE;

	// Add Audio Stream
	if (m_bCaptureAudio)
	{
		if ((*ppAVRec)->AddAudioStream(	m_CaptureAudioThread.m_pSrcWaveFormat,		// Src Wave Format
										m_CaptureAudioThread.m_pDstWaveFormat) < 0)	// Dst Wave Format
			return FALSE;
	}

	// Open
	if (!(*ppAVRec)->Open())
		return FALSE;
	else
		return TRUE;
}

void CVideoDeviceDoc::OnCaptureRecord() 
{
	CaptureRecord();
}

void CVideoDeviceDoc::OnUpdateCaptureRecord(CCmdUI* pCmdUI) 
{	
	pCmdUI->SetCheck(m_pAVRec != NULL ? 1 : 0);
}

BOOL CVideoDeviceDoc::CaptureRecord(BOOL bShowMessageBoxOnError/*=TRUE*/) 
{
	// Enter CS
	::EnterCriticalSection(&m_csAVRec);

	// Stop Recording
	if (m_pAVRec)
	{
		// Close avi and show it (if set so by user)
		CloseAndShowAviRec();

		// Leave CS
		::LeaveCriticalSection(&m_csAVRec);

		return TRUE;
	}
	// Start Recording
	else
	{
		// Set next rec time for time segmentation
		if (m_bRecTimeSegmentation)
			NextRecTime(CTime::GetCurrentTime());

		// Free
		FreeAVIFile();
		
		// Allocate & Init pAVRec
		CAVRec* pAVRec = NULL;
		if (!MakeAVRec(&pAVRec))
		{
			// Free
			if (pAVRec)
			{
				delete pAVRec;
				pAVRec = NULL;
			}

			// Leave CS
			::LeaveCriticalSection(&m_csAVRec);
			
			CString sMsg = ML_STRING(1493, "Cannot Create the AVI File!\n");
			TRACE(sMsg);
			if (bShowMessageBoxOnError)
				::AfxMessageBox(sMsg, MB_ICONSTOP);
			
			return FALSE;
		}

		// Set AV Rec Pointer
		m_pAVRec = pAVRec;

		// Start Recording
		m_bRecFirstFrame = TRUE;

		// Leave CS
		::LeaveCriticalSection(&m_csAVRec);

		return TRUE;
	}
}

// Function called from the UI thread and when ProcessI420Frame() is not called
void CVideoDeviceDoc::OnChangeDxVideoFormat()
{
	DWORD dwSize;

	// Check
	if (m_bClosing)
		return;

	if (m_pDxCapture)
	{
		// DV
		if (m_pDxCapture->IsDV())
		{
			_DVENCODERVIDEOFORMAT VideoFormat;
			if (m_pDxCapture->GetDVFormat(&VideoFormat))
			{
				if (VideoFormat == DVENCODERVIDEOFORMAT_NTSC)
					m_dFrameRate = 30000.0 / 1001.0; // ~29,97
				else
					m_dFrameRate = 25.0;
				m_lCompressedDataRate = Round(m_dFrameRate * (double)m_pDxCapture->GetAvgFrameSize());
			}
			int nWidth, nHeight;
			if (m_pDxCapture->GetDVSize(&nWidth, &nHeight))
			{
				m_CaptureBMI.bmiHeader.biSize =					sizeof(BITMAPINFOHEADER);
				m_CaptureBMI.bmiHeader.biWidth =				(LONG)nWidth;
				m_CaptureBMI.bmiHeader.biHeight =				(LONG)nHeight;
				m_CaptureBMI.bmiHeader.biPlanes =				1;
				m_CaptureBMI.bmiHeader.biCompression =			FCC('YUY2');
				m_CaptureBMI.bmiHeader.biBitCount =				16;
				int stride = ::CalcYUVStride(m_CaptureBMI.bmiHeader.biCompression, (int)m_CaptureBMI.bmiHeader.biWidth);
				m_CaptureBMI.bmiHeader.biSizeImage =			::CalcYUVSize(m_CaptureBMI.bmiHeader.biCompression, stride, (int)m_CaptureBMI.bmiHeader.biHeight);
				m_ProcessFrameBMI.bmiHeader.biSize =			sizeof(BITMAPINFOHEADER);
				m_ProcessFrameBMI.bmiHeader.biWidth =			(LONG)nWidth;
				m_ProcessFrameBMI.bmiHeader.biHeight =			(LONG)nHeight;
				m_ProcessFrameBMI.bmiHeader.biPlanes =			1;
				m_ProcessFrameBMI.bmiHeader.biCompression =		FCC('I420');
				m_ProcessFrameBMI.bmiHeader.biBitCount =		12;
				stride = ::CalcYUVStride(m_ProcessFrameBMI.bmiHeader.biCompression, (int)m_ProcessFrameBMI.bmiHeader.biWidth);
				m_ProcessFrameBMI.bmiHeader.biSizeImage =		::CalcYUVSize(m_ProcessFrameBMI.bmiHeader.biCompression, stride, (int)m_ProcessFrameBMI.bmiHeader.biHeight);
				m_DocRect.right = m_ProcessFrameBMI.bmiHeader.biWidth;
				m_DocRect.bottom = m_ProcessFrameBMI.bmiHeader.biHeight;
			}
		}
		else
		{
			AM_MEDIA_TYPE* pmtConfig = NULL;
			if (!m_pDxCapture->GetCurrentFormat(&pmtConfig))
			{
				CString sMsg;
				sMsg.Format(_T("%s, error getting current video format!\n"), GetAssignedDeviceName());
				TRACE(sMsg);
				::LogLine(sMsg);
				return;
			}
			if (pmtConfig->formattype != FORMAT_VideoInfo	||
				!pmtConfig->pbFormat						||
				pmtConfig->cbFormat < sizeof(VIDEOINFOHEADER))
			{
				m_pDxCapture->DeleteMediaType(pmtConfig);
				CString sMsg;
				sMsg.Format(_T("%s, unsupported video format!\n"), GetAssignedDeviceName());
				TRACE(sMsg);
				::LogLine(sMsg);
				return;
			}
			dwSize = MIN(sizeof(BITMAPINFOFULL), pmtConfig->cbFormat - SIZE_PREHEADER);
			VIDEOINFOHEADER* pVideoHeader = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
			memcpy(&m_CaptureBMI, HEADER(pVideoHeader), dwSize);
			memcpy(&m_ProcessFrameBMI, HEADER(pVideoHeader), dwSize);
			if (m_ProcessFrameBMI.bmiHeader.biCompression != FCC('I420'))
			{
				m_ProcessFrameBMI.bmiHeader.biSize =			sizeof(BITMAPINFOHEADER);
				m_ProcessFrameBMI.bmiHeader.biPlanes =			1;
				m_ProcessFrameBMI.bmiHeader.biCompression =		FCC('I420');
				m_ProcessFrameBMI.bmiHeader.biBitCount =		12;
				int stride = ::CalcYUVStride(m_ProcessFrameBMI.bmiHeader.biCompression, (int)m_ProcessFrameBMI.bmiHeader.biWidth);
				m_ProcessFrameBMI.bmiHeader.biSizeImage = ::CalcYUVSize(m_ProcessFrameBMI.bmiHeader.biCompression, stride, (int)m_ProcessFrameBMI.bmiHeader.biHeight);
			}
			m_DocRect.right = m_ProcessFrameBMI.bmiHeader.biWidth;
			m_DocRect.bottom = m_ProcessFrameBMI.bmiHeader.biHeight;
			m_lCompressedDataRate = 0;
			m_lCompressedDataRateSum = 0;
			m_pDxCapture->DeleteMediaType(pmtConfig);
		}
	}
	else
		return;

	// Free Movement Detector because we changed size and/or format!
	FreeMovementDetector();

	// Update
	if (m_bSizeToDoc)
	{
		// This sizes the view to m_DocRect in normal screen mode,
		// in full-screen mode it updates m_ZoomRect from m_DocRect
		::PostMessage(	GetView()->GetSafeHwnd(),
						WM_THREADSAFE_UPDATEWINDOWSIZES,
						(WPARAM)UPDATEWINDOWSIZES_SIZETODOC,
						(LPARAM)0);
		m_bSizeToDoc = FALSE;
	}
	else
	{
		// In full-screen mode it updates m_ZoomRect from m_DocRect
		::PostMessage(	GetView()->GetSafeHwnd(),
						WM_THREADSAFE_UPDATEWINDOWSIZES,
						(WPARAM)0,
						(LPARAM)0);
	}
	::PostMessage(	GetView()->GetSafeHwnd(),
					WM_THREADSAFE_SETDOCUMENTTITLE,
					0, 0);
	::PostMessage(	GetView()->GetSafeHwnd(),
					WM_THREADSAFE_UPDATE_PHPPARAMS,
					0, 0);
}

void CVideoDeviceDoc::OnChangeFrameRate()
{
	if (!m_bClosing)
	{
		if (m_pDxCapture)
		{
			m_pDxCapture->Stop();
			m_pDxCapture->SetFrameRate(m_dFrameRate);
			if (m_pDxCapture->Run())
			{
				// Some devices need that...
				// Process frame must still be stopped when calling Dx Stop()!
				m_pDxCapture->Stop();
				m_pDxCapture->Run();

				// Restart process frame
				StartProcessFrame(PROCESSFRAME_CHANGEFRAMERATE);
			}
			SetDocumentTitle();
		}
		else if (m_pGetFrameNetCom && m_pGetFrameNetCom->IsClient())
		{
			if (m_pHttpGetFrameParseProcess->m_FormatType == CHttpGetFrameParseProcess::FORMATMJPEG)
			{
				if (m_nNetworkDeviceTypeMode == CVideoDeviceDoc::EDIMAX_SP)
					m_pHttpGetFrameParseProcess->m_bSetFramerate = TRUE;
				m_HttpGetFrameThread.SetEventConnect();
			}
			StartProcessFrame(PROCESSFRAME_CHANGEFRAMERATE);
			SetDocumentTitle();
		}
	}
}

void CVideoDeviceDoc::AudioFormatDialog() 
{
	CAudioFormatDlg dlg(GetView());
	if (m_CaptureAudioThread.m_pDstWaveFormat)
		memcpy(&dlg.m_WaveFormat, m_CaptureAudioThread.m_pDstWaveFormat, sizeof(WAVEFORMATEX));
	if (dlg.DoModal() == IDOK)
	{
		// Stop Save Frame List Thread
		m_SaveFrameListThread.Kill();

		// Stop Rec
		if (m_pAVRec)
			CaptureRecord();

		// Stop Audio Thread
		if (m_bCaptureAudio)
			m_CaptureAudioThread.Kill();

		// Copy from Dialog to Doc
		if (m_CaptureAudioThread.m_pSrcWaveFormat && m_CaptureAudioThread.m_pDstWaveFormat)
		{
			// Src Wave Format
			memcpy(m_CaptureAudioThread.m_pSrcWaveFormat, &dlg.m_WaveFormat, sizeof(WAVEFORMATEX));
			if (m_CaptureAudioThread.m_pSrcWaveFormat->wFormatTag != WAVE_FORMAT_PCM)
			{
				m_CaptureAudioThread.m_pSrcWaveFormat->wFormatTag			= WAVE_FORMAT_PCM;
				m_CaptureAudioThread.m_pSrcWaveFormat->wBitsPerSample		= 16;
				m_CaptureAudioThread.m_pSrcWaveFormat->nBlockAlign			= 2 * m_CaptureAudioThread.m_pSrcWaveFormat->nChannels;
				m_CaptureAudioThread.m_pSrcWaveFormat->nAvgBytesPerSec		= m_CaptureAudioThread.m_pSrcWaveFormat->nSamplesPerSec *
																				m_CaptureAudioThread.m_pSrcWaveFormat->nBlockAlign;
				m_CaptureAudioThread.m_pSrcWaveFormat->cbSize				= 0;
			}

			// Dst Wave Format
			memcpy(m_CaptureAudioThread.m_pDstWaveFormat, &dlg.m_WaveFormat, sizeof(WAVEFORMATEX));
		}

		// Start Audio Thread
		if (m_bCaptureAudio)
			m_CaptureAudioThread.Start();

		// Restart Save Frame List Thread
		m_SaveFrameListThread.Start();
	}
}

void CVideoDeviceDoc::OnCaptureAssistant() 
{
	CaptureAssistant();
}

void CVideoDeviceDoc::OnUpdateCaptureAssistant(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bCaptureStarted && !m_bClosing);
}

void CVideoDeviceDoc::CaptureAssistant()
{
	GetView()->ForceCursor();
	CAssistantDlg dlg(this, GetView());
	dlg.DoModal();
	GetView()->ForceCursor(FALSE);
}

void CVideoDeviceDoc::OnCaptureSettings() 
{
	// Create if First Time
	if (!m_pVideoDevicePropertySheet)
	{
		m_pVideoDevicePropertySheet = new CVideoDevicePropertySheet(this);
		CRect rect(0, 0, 0, 0);
		if (!m_pVideoDevicePropertySheet->Create(GetView(),
			WS_POPUP | WS_CAPTION | WS_SYSMENU, WS_EX_TOOLWINDOW))
		{
			m_pVideoDevicePropertySheet = NULL;
			return;
		}
		m_pVideoDevicePropertySheet->CenterWindow();
		m_pVideoDevicePropertySheet->Show();
	}
	// Toggle Visible / Invisible State
	else
	{
		if (m_pVideoDevicePropertySheet->IsWindowVisible())
			m_pVideoDevicePropertySheet->Hide(TRUE);
		else
			m_pVideoDevicePropertySheet->Show();
	}

	SetDocumentTitle();
}

void CVideoDeviceDoc::OnUpdateCaptureSettings(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_pDxCapture || m_pGetFrameNetCom);
	if (m_pVideoDevicePropertySheet)
		pCmdUI->SetCheck(m_pVideoDevicePropertySheet->IsWindowVisible() ? 1 : 0);
	else
		pCmdUI->SetCheck(0);
}

void CVideoDeviceDoc::VideoFormatDialog() 
{
	if (m_pDxCapture)
	{
		if (m_pDxCapture->IsDV())
		{
			// Do not call 2 or more times!
			if (!m_bStopAndChangeFormat)
			{
				m_bStopAndChangeFormat = TRUE;
				StopProcessFrame(PROCESSFRAME_DVFORMATDIALOG);
				double dFrameRate = m_dEffectiveFrameRate;
				int delay;
				if (dFrameRate >= 1.0)
					delay = Round(1000.0 / dFrameRate); // In ms
				else
					delay = 1000;
				CPostDelayedMessageThread::PostDelayedMessage(	GetView()->GetSafeHwnd(),
																WM_THREADSAFE_DVCHANGEVIDEOFORMAT,
																delay, 0, delay);
			}
		}
		else
		{
			// Same stop processing and change format mechanism
			// is integrated inside CDxVideoFormatDlg
			CDxVideoFormatDlg dlg(this);
			dlg.DoModal();
		}
	}
	else if (m_pGetFrameNetCom && m_pGetFrameNetCom->IsClient())
	{
		if (m_nNetworkDeviceTypeMode == OTHERONE_SP	||
			m_nNetworkDeviceTypeMode == OTHERONE_CP	||
			m_nNetworkDeviceTypeMode == TPLINK_SP	||
			m_nNetworkDeviceTypeMode == TPLINK_CP)
		{
			CString sUrl;
			if (m_nGetFrameVideoPort != 80)
				sUrl.Format(_T("http://%s:%d"), m_sGetFrameVideoHost, m_nGetFrameVideoPort);
			else
				sUrl.Format(_T("http://%s"), m_sGetFrameVideoHost);
			BeginWaitCursor();
			::ShellExecute(	NULL,
							_T("open"),
							::UrlEncode(sUrl, FALSE),
							NULL,
							NULL,
							SW_SHOWNORMAL);
			EndWaitCursor();
		}
		else
		{
			// Stop processing and change format mechanism
			// not necessary because the size change
			// is detected when it happens!
			CHttpVideoFormatDlg dlg(this);
			dlg.DoModal();
		}
	}
}

void CVideoDeviceDoc::OnViewVideo() 
{
	m_bVideoView = !m_bVideoView;
	if (!m_bVideoView)
	{
		m_bDecodeFramesForPreview = FALSE;
		::PostMessage(	GetView()->GetSafeHwnd(),
						WM_THREADSAFE_SETDOCUMENTTITLE,
						0, 0);
	}
}

void CVideoDeviceDoc::OnUpdateViewVideo(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bVideoView ? 1 : 0);
}

void CVideoDeviceDoc::MicroApacheUpdateMainFiles()
{
	// Copy index.php to Doc Root (overwrite if existing)
	CString sDocRoot = ((CUImagerApp*)::AfxGetApp())->m_sMicroApacheDocRoot;
	sDocRoot.TrimRight(_T('\\'));
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szProgramName[MAX_PATH];
	if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) != 0)
	{
		_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
		CString sMicroapacheHtDocs = CString(szDrive) + CString(szDir) + MICROAPACHE_HTDOCS + _T("\\");
		::CopyFile(sMicroapacheHtDocs + MICROAPACHE_INDEX_ROOTDIR_FILENAME, sDocRoot + _T("\\") + _T("index.php"), FALSE);
	}

	// Warning
	CString sConfig, sFormat;
	sConfig =  _T("# DO NOT MODIFY THIS FILE (ContaCam will overwrite your changes)\r\n");
	sConfig += _T("# Make your customizations in ") + CString(MICROAPACHE_EDITABLE_CONFIGNAME_EXT) + _T("\r\n\r\n");

	// Listen Port
	sFormat.Format(_T("Listen %d\r\n"), ((CUImagerApp*)::AfxGetApp())->m_nMicroApachePort);
	sConfig += sFormat;

	// Server name, root and admin email
	sConfig += _T("ServerName localhost\r\n");
	sConfig += _T("ServerRoot .\r\n");
	sConfig += _T("ServerAdmin webmaster@nowhere.com\r\n");

	// Document root
	sDocRoot = ::GetASCIICompatiblePath(sDocRoot); // directory must exist!
	sDocRoot.Replace(_T('\\'), _T('/')); // change path from \ to / (otherwise apache is not happy)
	sFormat.Format(_T("DocumentRoot \"%s/\"\r\n"), sDocRoot); // add a trailing /, otherwise it is not
	sConfig += sFormat;			// working when the root directory is the drive itself (c: for example)

	// Global settings
	sConfig += _T("ThreadsPerChild 128\r\n");
	sConfig += _T("Win32DisableAcceptEx On\r\n");
	sConfig += _T("LoadModule access_module modules/mod_access.dll\r\n");
	sConfig += _T("LoadModule dir_module modules/mod_dir.dll\r\n");
	sConfig += _T("LoadModule mime_module modules/mod_mime.dll\r\n");
	sConfig += _T("LoadModule rewrite_module modules/mod_rewrite.dll\r\n");
	sConfig += _T("LoadModule auth_module modules/mod_auth.dll\r\n");
	sConfig += _T("LoadModule auth_digest_module modules/mod_auth_digest.dll\r\n");
	sConfig += _T("LoadModule php5_module \"php5apache2.dll\"\r\n");
	sConfig += _T("AddType application/x-httpd-php .php .php3\r\n");
	sConfig += _T("AcceptPathInfo off\r\n");
	sConfig += _T("KeepAlive on\r\n");
	sConfig += _T("KeepAliveTimeout 15\r\n");
	sConfig += _T("MaxKeepAliveRequests 0\r\n");
	sConfig += _T("TimeOut 300\r\n");
	sConfig += _T("DirectoryIndex index.html index.htm index.php\r\n");
	sConfig += _T("LogLevel crit\r\n");
	
	// Error log and pid file locations
	CString sMicroapacheConfigFile = MicroApacheGetConfigFileName();
	CString sConfigDir = ::GetDriveAndDirName(sMicroapacheConfigFile);
	if (!::IsExistingDir(sConfigDir))
		::CreateDir(sConfigDir);
	sConfigDir = ::GetASCIICompatiblePath(sConfigDir); // directory must exist!
	sConfigDir.Replace(_T('\\'), _T('/')); // Change path from \ to / (otherwise apache is not happy)
	sConfig += _T("ErrorLog \"") + sConfigDir + MICROAPACHE_LOGNAME_EXT + _T("\"\r\n");
	sConfig += _T("PidFile \"") + sConfigDir + MICROAPACHE_PIDNAME_EXT + _T("\"\r\n");
	
	// Rewrite engine
	sConfig += _T("<Directory />\r\n");
	sConfig += _T("RewriteEngine on\r\n");
	sConfig += _T("RewriteBase /\r\n");
	sConfig += _T("RewriteCond %{REQUEST_FILENAME} -d\r\n");
	sConfig += _T("RewriteRule [^/]$ http://%{HTTP_HOST}%{REQUEST_URI}/ [L,R=301]\r\n");
	sConfig += _T("</Directory>\r\n");

	// Deny Access to Temp folder
	sFormat.Format(_T("<Directory \"%s/Temp/\">\r\n"), sDocRoot);
	sConfig += sFormat;
	sConfig += _T("Order allow,deny\r\n");
	sConfig += _T("Deny from all\r\n");
	sConfig += _T("</Directory>\r\n");

	// Make password file and set authentication type
	if (((CUImagerApp*)::AfxGetApp())->m_sMicroApacheUsername != _T("") ||
		((CUImagerApp*)::AfxGetApp())->m_sMicroApachePassword != _T(""))
	{
		MicroApacheMakePasswordFile(((CUImagerApp*)::AfxGetApp())->m_bMicroApacheDigestAuth,
									((CUImagerApp*)::AfxGetApp())->m_sMicroApacheAreaname,						
									((CUImagerApp*)::AfxGetApp())->m_sMicroApacheUsername,
									((CUImagerApp*)::AfxGetApp())->m_sMicroApachePassword);
		sConfig += _T("<Location />\r\n");
		if (((CUImagerApp*)::AfxGetApp())->m_bMicroApacheDigestAuth)
		{
			sConfig += _T("AuthType Digest\r\n");
			sConfig += _T("AuthDigestDomain /\r\n");
			sConfig += _T("AuthDigestFile \"") + sConfigDir + MICROAPACHE_PWNAME_EXT + _T("\"\r\n");
		}
		else
		{
			sConfig += _T("AuthType Basic\r\n");
			sConfig += _T("AuthUserFile \"") + sConfigDir + MICROAPACHE_PWNAME_EXT + _T("\"\r\n");
		}
		sConfig += _T("AuthName \"") + ((CUImagerApp*)::AfxGetApp())->m_sMicroApacheAreaname + _T("\"\r\n");
		sConfig += _T("Require valid-user\r\n");
		sConfig += _T("</Location>\r\n");
	}
	
	// Include custom configurations file (create an empty one if not existing)
	sConfig += _T("Include \"") + sConfigDir + MICROAPACHE_EDITABLE_CONFIGNAME_EXT + _T("\"");
	CString sMicroapacheEditableConfigFile = MicroApacheGetEditableConfigFileName();
	if (!::IsExistingFile(sMicroapacheEditableConfigFile))
	{
		LPSTR pData = NULL;
		int nLen = ::ToANSI(_T("# Add apache custom configurations here (this file is not modified by ContaCam)\r\n\r\n"), &pData);
		if (nLen > 0 && pData)
		{
			try
			{
				CFile f(sMicroapacheEditableConfigFile,
						CFile::modeCreate		|
						CFile::modeWrite		|
						CFile::shareDenyWrite);
				f.Write(pData, nLen);
			}
			catch (CFileException* e)
			{
				e->Delete();
			}
		}
		if (pData)
			delete [] pData;
	}

	// Finally save config file (overwrite if existing)
	LPSTR pData = NULL;
	int nLen = ::ToANSI(sConfig, &pData);
	if (nLen > 0 && pData)
	{
		try
		{
			CFile f(sMicroapacheConfigFile,
					CFile::modeCreate		|
					CFile::modeWrite		|
					CFile::shareDenyWrite);
			f.Write(pData, nLen);
		}
		catch (CFileException* e)
		{
			e->Delete();
		}
	}
	if (pData)
		delete [] pData;
}

BOOL CVideoDeviceDoc::MicroApacheUpdateWebFiles(CString sAutoSaveDir)
{
	sAutoSaveDir.TrimRight(_T('\\'));
	sAutoSaveDir += _T('\\');
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szProgramName[MAX_PATH];
	if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
		return FALSE;
	_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
	CString sMicroapacheHtDocs = CString(szDrive) + CString(szDir) + MICROAPACHE_HTDOCS + _T("\\");
	CSortableFileFind FileFind;
	if (!FileFind.InitRecursive(sMicroapacheHtDocs + _T("*"), FALSE))
		return FALSE;
	if (FileFind.WaitRecursiveDone() != 1)
		return FALSE;
	CString sRootDirName = FileFind.GetRootDirName();
	sRootDirName.TrimRight(_T('\\'));
	int nRootDirNameSize = sRootDirName.GetLength() + 1;
	int pos;
	for (pos = 0 ; pos < FileFind.GetDirsCount() ; pos++)
	{
		CString sDir = FileFind.GetDirName(pos);
		CString sRelDir = sDir.Mid(nRootDirNameSize);
		::CreateDir(sAutoSaveDir + sRelDir); // Does nothing if dir already exists
	}
	for (pos = 0 ; pos < FileFind.GetFilesCount() ; pos++)
	{
		CString sName = FileFind.GetFileName(pos);
		CString sShortName = ::GetShortFileName(sName);
		CString sRelName = sName.Mid(nRootDirNameSize);
		if (sShortName.CompareNoCase(MICROAPACHE_INDEX_ROOTDIR_FILENAME) != 0 &&
			sShortName.CompareNoCase(THUMBS_DB) != 0)
		{
			if (sShortName.CompareNoCase(PHP_CONFIGNAME_EXT) == 0)
				::CopyFile(sName, sAutoSaveDir + sRelName, TRUE);	// Never overwrite the configuration file!
			else
				::CopyFile(sName, sAutoSaveDir + sRelName, FALSE);	// Always overwrite to get new version!
		}
	}
	return TRUE;
}

void CVideoDeviceDoc::ViewWeb() 
{
	if (m_sRecordAutoSaveDir != _T(""))
	{
		// Init vars
		CString sAutoSaveDir = m_sRecordAutoSaveDir;
		sAutoSaveDir.TrimRight(_T('\\'));
		CString sMicroApacheDocRoot = ((CUImagerApp*)::AfxGetApp())->m_sMicroApacheDocRoot;
		sMicroApacheDocRoot.TrimRight(_T('\\'));

		// Fail if sAutoSaveDir is not inside document root
		if (!::IsSubDir(sMicroApacheDocRoot, sAutoSaveDir))
		{
			::AfxMessageBox(ML_STRING(1473, "Camera folder must reside inside the document root directory!"), MB_OK | MB_ICONSTOP);
			return;
		}

		// Overwrite web files in given directory
		MicroApacheUpdateWebFiles(sAutoSaveDir);
		
		// Execute Browser
		CString sRelPath(sAutoSaveDir.Right(sAutoSaveDir.GetLength() - sMicroApacheDocRoot.GetLength()));
		sRelPath.TrimLeft(_T('\\'));
		sRelPath.Replace(_T('\\'), _T('/'));// Change path from \ to /
		CString sUrl, sPort;
		sPort.Format(_T("%d"), ((CUImagerApp*)::AfxGetApp())->m_nMicroApachePort);
		if (sPort != _T("80"))
		{
			if (sRelPath != _T(""))
				sUrl = _T("http://localhost:") + sPort + _T("/") + sRelPath + _T("/");
			else
				sUrl = _T("http://localhost:") + sPort + _T("/");
		}
		else
		{
			if (sRelPath != _T(""))
				sUrl = _T("http://localhost/") + sRelPath + _T("/");
			else
				sUrl = _T("http://localhost/");
		}
		sUrl = ::UrlEncode(sUrl, FALSE);
		if (((CUImagerApp*)::AfxGetApp())->m_bFullscreenBrowser)
		{
			int nRet = IDYES;
			if (((CUImagerApp*)::AfxGetApp())->m_sFullscreenBrowserExitString != _T(""))
			{
				CString sMsg;
				sMsg.Format(ML_STRING(1762, "Entering fullscreen mode, exit pressing anywhere\nthe ESC key followed by %s\nDo you want to continue?"),
							((CUImagerApp*)::AfxGetApp())->m_sFullscreenBrowserExitString);
				nRet = ::AfxMessageBox(sMsg, MB_YESNO | MB_ICONINFORMATION);
			}
			if (nRet == IDYES)
			{
				BeginWaitCursor();
				TCHAR szDrive[_MAX_DRIVE];
				TCHAR szDir[_MAX_DIR];
				TCHAR szProgramName[MAX_PATH];
				if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
				{
					EndWaitCursor();
					return;
				}
				_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
				CString sFullscreenExe = CString(szDrive) + CString(szDir) + CString(FULLSCREENBROWSER_EXE_NAME_EXT);
				::ShellExecute(	NULL,
								_T("open"),
								sFullscreenExe,
								sUrl,
								NULL,
								SW_SHOWNORMAL);
				EndWaitCursor();
			}
		}
		else
		{
			BeginWaitCursor();
			::ShellExecute(	NULL,
							_T("open"),
							sUrl,
							NULL,
							NULL,
							SW_SHOWNORMAL);
			EndWaitCursor();
		}
	}
}

void CVideoDeviceDoc::ViewFiles() 
{
	::ShellExecute(	NULL,
					_T("open"),
					m_sRecordAutoSaveDir,
					NULL,
					NULL,
					SW_SHOWNORMAL);
}

CString CVideoDeviceDoc::MicroApacheGetConfigFileName()
{
	CString sMicroapacheConfigFile = ::GetSpecialFolderPath(CSIDL_APPDATA);
	sMicroapacheConfigFile += _T("\\") + MICROAPACHE_CONFIG_FILE;
	return sMicroapacheConfigFile;
}

CString CVideoDeviceDoc::MicroApacheGetEditableConfigFileName()
{
	CString sMicroapacheEditableConfigFile = ::GetSpecialFolderPath(CSIDL_APPDATA);
	sMicroapacheEditableConfigFile += _T("\\") + MICROAPACHE_EDITABLE_CONFIG_FILE;
	return sMicroapacheEditableConfigFile;
}

CString CVideoDeviceDoc::MicroApacheGetLogFileName()
{
	CString sMicroapacheLogFile = ::GetSpecialFolderPath(CSIDL_APPDATA);
	sMicroapacheLogFile += _T("\\") + MICROAPACHE_LOG_FILE;
	return sMicroapacheLogFile;
}

CString CVideoDeviceDoc::MicroApacheGetPidFileName()
{
	CString sMicroapachePidFile = ::GetSpecialFolderPath(CSIDL_APPDATA);
	sMicroapachePidFile += _T("\\") + MICROAPACHE_PID_FILE;
	return sMicroapachePidFile;
}

CString CVideoDeviceDoc::MicroApacheGetPwFileName()
{
	CString sMicroapachePwFile = ::GetSpecialFolderPath(CSIDL_APPDATA);
	sMicroapachePwFile += _T("\\") + MICROAPACHE_PW_FILE;
	return sMicroapachePwFile;
}

BOOL CVideoDeviceDoc::MicroApacheMakePasswordFile(BOOL bDigest, const CString& sAreaname, const CString& sUsername, const CString& sPassword)
{
	// Delete password file if existing
	CString sMicroapachePwFile = MicroApacheGetPwFileName();
	if (::IsExistingFile(sMicroapachePwFile))
		::DeleteFile(sMicroapachePwFile);

	// Make password file
	if (bDigest)
	{
		USES_CONVERSION;
		CPJNMD5 hmac;
		CPJNMD5Hash hash;
		CString sToHash = sUsername + _T(":") + sAreaname + _T(":") + sPassword;
		char* pszA1 = T2A(const_cast<LPTSTR>(sToHash.operator LPCTSTR()));
		if (hmac.Hash((const BYTE*)pszA1, (DWORD)strlen(pszA1), hash))
		{
			CString sHA1 = hash.Format(FALSE);
			CString sPasswordFileData = sUsername + _T(":") + sAreaname + _T(":") + sHA1 + _T("\n");
			LPSTR pData = NULL;
			int nLen = ::ToANSI(sPasswordFileData, &pData);
			if (nLen <= 0 || !pData)
			{
				if (pData)
					delete [] pData;
				return FALSE;
			}
			try
			{
				CFile f(sMicroapachePwFile,
						CFile::modeCreate		|
						CFile::modeWrite		|
						CFile::shareDenyWrite);
				f.Write(pData, nLen);
				delete [] pData;
				return TRUE;
			}
			catch (CFileException* e)
			{
				delete [] pData;
				e->Delete();
				return FALSE;
			}
		}
		else
			return FALSE;
	}
	else
	{
		// Apache's basic auth password file format is quite complicated
		// (see apr_md5_encode() in apr_md5.c)
		// -> we use the htpasswd.exe tool:
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szProgramName[MAX_PATH];
		if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
			return FALSE;
		_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
		CString sMicroapachePwToolFile = CString(szDrive) + CString(szDir);
		sMicroapachePwToolFile += MICROAPACHE_PWTOOL_RELPATH;
		if (!::IsExistingFile(sMicroapachePwToolFile))
			return FALSE;
		const char Init[] = "    ";
		DWORD NumberOfBytesWritten;
		HANDLE hFile = ::CreateFile(sMicroapachePwFile,
									GENERIC_WRITE, 0, NULL,
									CREATE_NEW,
									FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			::WriteFile(hFile, Init, strlen(Init), &NumberOfBytesWritten, NULL);
			::CloseHandle(hFile);
		}
		sMicroapachePwFile = ::GetASCIICompatiblePath(sMicroapachePwFile); // file must exist!
		sMicroapachePwFile.Replace(_T('\\'), _T('/')); // Change path from \ to / (otherwise pw tool is not happy)
		CString sParams = _T("-bc \"") + sMicroapachePwFile + _T("\" \"") + sUsername + _T("\" \"") + sPassword + _T("\"");
		return ::ExecHiddenApp(sMicroapachePwToolFile, sParams);
	}
}

BOOL CVideoDeviceDoc::MicroApacheIsPortUsed(int nPort)
{
	BOOL bUsed = FALSE;
	CNetCom NetCom;
	HANDLE hEventArray[2];
	hEventArray[0] = ::CreateEvent(NULL, TRUE, FALSE, NULL); // Http Connected Event						
	hEventArray[1] = ::CreateEvent(NULL, TRUE, FALSE, NULL); // Http Connect Failed Event
	if (NetCom.Init(
				FALSE,					// Be Client
				NULL,					// The Optional Owner Window to which send the Network Events.
				NULL,					// The lParam to send with the Messages
				NULL,					// The Optional Rx Buffer.
				NULL,					// The Optional Critical Section for the Rx Buffer.
				NULL,					// The Optional Rx Fifo.
				NULL,					// The Optional Critical Section fot the Rx Fifo.
				NULL,					// The Optional Tx Buffer.
				NULL,					// The Optional Critical Section for the Tx Buffer.
				NULL,					// The Optional Tx Fifo.
				NULL,					// The Optional Critical Section for the Tx Fifo.
				NULL,					// Parser
				NULL,					// Generator
				SOCK_STREAM,			// TCP
				_T(""),					// Local Address (IP or Host Name).
				0,						// Local Port, let the OS choose one
				_T("localhost"),		// Peer Address (IP or Host Name).
				nPort,					// Peer Port.
				NULL,					// Handle to an Event Object that will get Accept Events.
				hEventArray[0],			// Handle to an Event Object that will get Connect Events.
				hEventArray[1],			// Handle to an Event Object that will get Connect Failed Events.
				NULL,					// Handle to an Event Object that will get Close Events.
				NULL,					// Handle to an Event Object that will get Read Events.
				NULL,					// Handle to an Event Object that will get Write Events.
				NULL,					// Handle to an Event Object that will get OOB Events.
				NULL,					// Handle to an Event Object that will get an event when 
										// all connection of a server have been closed.
				0,						// A combination of network events:
										// FD_ACCEPT | FD_CONNECT | FD_CONNECTFAILED | FD_CLOSE | FD_READ | FD_WRITE | FD_OOB | FD_ALLCLOSE.
										// A set value means that instead of setting an event it is reset.
				0,						// A combination of network events:
										// FD_ACCEPT | FD_CONNECT | FD_CONNECTFAILED | FD_CLOSE | FD_READ | FD_WRITE | FD_OOB | FD_ALLCLOSE.
										// The Following messages will be sent to the pOwnerWnd (if pOwnerWnd != NULL):
										// WM_NETCOM_ACCEPT_EVENT -> Notification of incoming connections.
										// WM_NETCOM_CONNECT_EVENT -> Notification of completed connection or multipoint "join" operation.
										// WM_NETCOM_CONNECTFAILED_EVENT -> Notification of connection failure.
										// WM_NETCOM_CLOSE_EVENT -> Notification of socket closure.
										// WM_NETCOM_READ_EVENT -> Notification of readiness for reading.
										// WM_NETCOM_WRITE_EVENT -> Notification of readiness for writing.
										// WM_NETCOM_OOB_EVENT -> Notification of the arrival of out-of-band data.
										// WM_NETCOM_ALLCLOSE_EVENT -> Notification that all connection have been closed.
				0,/*=uiRxMsgTrigger*/	// The number of bytes that triggers an hRxMsgTriggerEvent 
										// (if hRxMsgTriggerEvent != NULL).
										// And/Or the number of bytes that triggers a WM_NETCOM_RX Message
										// (if pOwnerWnd != NULL).
										// Upper bound for this value is NETCOM_MAX_RX_BUFFER_SIZE.
				NULL,/*hRxMsgTriggerEvent*/	// Handle to an Event Object that will get an Event
										// each time uiRxMsgTrigger bytes arrived.
				0,/*uiMaxTxPacketSize*/	// The maximum size for transmitted packets,
										// upper bound for this value is NETCOM_MAX_TX_BUFFER_SIZE.
				0,/*uiRxPacketTimeout*/	// After this timeout a Packet is returned
										// even if the uiRxMsgTrigger size is not reached (A zero meens INFINITE Timeout).
				0,/*uiTxPacketTimeout*/	// After this timeout a Packet is sent
										// even if no Write Event Happened (A zero meens INFINITE Timeout).
										// This is also the Generator rate,
										// if set to zero the Generator is never called!
				NULL,					// Message Class for Notice, Warning and Error Visualization.
				AF_UNSPEC))				// Socket family
	{
		DWORD Event = ::WaitForMultipleObjects(	2,
												hEventArray,
												FALSE,
												MICROAPACHE_TIMEOUT_MS);
		switch (Event)
		{
			// Http Connected Event
			case WAIT_OBJECT_0 :
				bUsed = TRUE;
				break;

			// Http Connection failed Event
			case WAIT_OBJECT_0 + 1 :
				break;

			// Timeout
			default :
				break;
		}
	}
	NetCom.Close();
	::CloseHandle(hEventArray[0]);
	::CloseHandle(hEventArray[1]);
	return bUsed;
}

BOOL CVideoDeviceDoc::MicroApacheInitStart()
{
	CString sMicroapacheConfigFile = MicroApacheGetConfigFileName();
	if (!::IsExistingFile(sMicroapacheConfigFile))
		return FALSE;
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szProgramName[MAX_PATH];
	if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
		return FALSE;
	_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
	CString sMicroapacheStartFile = CString(szDrive) + CString(szDir);
	sMicroapacheStartFile += MICROAPACHE_RELPATH;
	if (!::IsExistingFile(sMicroapacheStartFile))
		return FALSE;
	::DeleteFile(MicroApacheGetLogFileName()); // Avoid growing it to much!
	sMicroapacheConfigFile = ::GetASCIICompatiblePath(sMicroapacheConfigFile); // file must exist!
	sMicroapacheConfigFile.Replace(_T('\\'), _T('/')); // Change path from \ to / (otherwise apache is not happy)
	CString sParams = _T("-f \"") + sMicroapacheConfigFile + _T("\"");
	return ::ExecHiddenApp(sMicroapacheStartFile, sParams);
}

BOOL CVideoDeviceDoc::MicroApacheWaitStartDone()
{
	DWORD dwSleep = 0U;
	while (::EnumKillProcByName(MICROAPACHE_FILENAME) < MICROAPACHE_NUM_PROCESS)
	{
		dwSleep += MICROAPACHE_WAITTIME_MS;
		::Sleep(MICROAPACHE_WAITTIME_MS);
		if (dwSleep >= MICROAPACHE_TIMEOUT_MS)
			return FALSE;
	}
	return TRUE;
}

BOOL CVideoDeviceDoc::MicroApacheWaitCanConnect()
{
	CNetCom NetCom;
	HANDLE hEventArray[2];
	hEventArray[0] = ::CreateEvent(NULL, TRUE, FALSE, NULL); // Http Connected Event						
	hEventArray[1] = ::CreateEvent(NULL, TRUE, FALSE, NULL); // Http Connect Failed Event
	int nConnectAttempt = 0;
	while (++nConnectAttempt <= MICROAPACHE_CANCONNECT_ATTEMPTS)
	{
		::ResetEvent(hEventArray[0]);
		::ResetEvent(hEventArray[1]);
		if (NetCom.Init(
					FALSE,					// Be Client
					NULL,					// The Optional Owner Window to which send the Network Events.
					NULL,					// The lParam to send with the Messages
					NULL,					// The Optional Rx Buffer.
					NULL,					// The Optional Critical Section for the Rx Buffer.
					NULL,					// The Optional Rx Fifo.
					NULL,					// The Optional Critical Section fot the Rx Fifo.
					NULL,					// The Optional Tx Buffer.
					NULL,					// The Optional Critical Section for the Tx Buffer.
					NULL,					// The Optional Tx Fifo.
					NULL,					// The Optional Critical Section for the Tx Fifo.
					NULL,					// Parser
					NULL,					// Generator
					SOCK_STREAM,			// TCP
					_T(""),					// Local Address (IP or Host Name).
					0,						// Local Port, let the OS choose one
					_T("localhost"),									// Peer Address (IP or Host Name).
					((CUImagerApp*)::AfxGetApp())->m_nMicroApachePort,	// Peer Port.
					NULL,					// Handle to an Event Object that will get Accept Events.
					hEventArray[0],			// Handle to an Event Object that will get Connect Events.
					hEventArray[1],			// Handle to an Event Object that will get Connect Failed Events.
					NULL,					// Handle to an Event Object that will get Close Events.
					NULL,					// Handle to an Event Object that will get Read Events.
					NULL,					// Handle to an Event Object that will get Write Events.
					NULL,					// Handle to an Event Object that will get OOB Events.
					NULL,					// Handle to an Event Object that will get an event when 
											// all connection of a server have been closed.
					0,						// A combination of network events:
											// FD_ACCEPT | FD_CONNECT | FD_CONNECTFAILED | FD_CLOSE | FD_READ | FD_WRITE | FD_OOB | FD_ALLCLOSE.
											// A set value means that instead of setting an event it is reset.
					0,						// A combination of network events:
											// FD_ACCEPT | FD_CONNECT | FD_CONNECTFAILED | FD_CLOSE | FD_READ | FD_WRITE | FD_OOB | FD_ALLCLOSE.
											// The Following messages will be sent to the pOwnerWnd (if pOwnerWnd != NULL):
											// WM_NETCOM_ACCEPT_EVENT -> Notification of incoming connections.
											// WM_NETCOM_CONNECT_EVENT -> Notification of completed connection or multipoint "join" operation.
											// WM_NETCOM_CONNECTFAILED_EVENT -> Notification of connection failure.
											// WM_NETCOM_CLOSE_EVENT -> Notification of socket closure.
											// WM_NETCOM_READ_EVENT -> Notification of readiness for reading.
											// WM_NETCOM_WRITE_EVENT -> Notification of readiness for writing.
											// WM_NETCOM_OOB_EVENT -> Notification of the arrival of out-of-band data.
											// WM_NETCOM_ALLCLOSE_EVENT -> Notification that all connection have been closed.
					0,/*=uiRxMsgTrigger*/	// The number of bytes that triggers an hRxMsgTriggerEvent 
											// (if hRxMsgTriggerEvent != NULL).
											// And/Or the number of bytes that triggers a WM_NETCOM_RX Message
											// (if pOwnerWnd != NULL).
											// Upper bound for this value is NETCOM_MAX_RX_BUFFER_SIZE.
					NULL,/*hRxMsgTriggerEvent*/	// Handle to an Event Object that will get an Event
											// each time uiRxMsgTrigger bytes arrived.
					0,/*uiMaxTxPacketSize*/	// The maximum size for transmitted packets,
											// upper bound for this value is NETCOM_MAX_TX_BUFFER_SIZE.
					0,/*uiRxPacketTimeout*/	// After this timeout a Packet is returned
											// even if the uiRxMsgTrigger size is not reached (A zero meens INFINITE Timeout).
					0,/*uiTxPacketTimeout*/	// After this timeout a Packet is sent
											// even if no Write Event Happened (A zero meens INFINITE Timeout).
											// This is also the Generator rate,
											// if set to zero the Generator is never called!
					NULL,					// Message Class for Notice, Warning and Error Visualization.
					AF_UNSPEC))				// Socket family
		{
			DWORD Event = ::WaitForMultipleObjects(	2,
													hEventArray,
													FALSE,
													MICROAPACHE_TIMEOUT_MS);
			switch (Event)
			{
				// Http Connected Event
				case WAIT_OBJECT_0 :
					NetCom.Close();
					::CloseHandle(hEventArray[0]);
					::CloseHandle(hEventArray[1]);
					return TRUE;

				// Http Connection failed Event
				case WAIT_OBJECT_0 + 1 :
					::Sleep(nConnectAttempt * MICROAPACHE_WAITTIME_MS);
					break;

				// Timeout
				default :
					NetCom.Close();
					::CloseHandle(hEventArray[0]);
					::CloseHandle(hEventArray[1]);
					return FALSE;
			}
		}
		else
			::Sleep(nConnectAttempt * MICROAPACHE_WAITTIME_MS);
	}
	NetCom.Close();
	::CloseHandle(hEventArray[0]);
	::CloseHandle(hEventArray[1]);
	return FALSE;
}

BOOL CVideoDeviceDoc::MicroApacheShutdown()
{
	BOOL res;
	LPBYTE pData = NULL;
	try
	{
		// Open Pid File
		CFile f(MicroApacheGetPidFileName(),
				CFile::modeRead | CFile::shareDenyNone);
		DWORD dwLength = (DWORD)f.GetLength();
		if (dwLength > 0)
		{
			// Allocate Buffer
			pData = new BYTE [dwLength+1];
			if (pData)
			{
				// Read Data
				dwLength = f.Read(pData, dwLength);
				pData[dwLength] = '\0';
				CString sPid;
				sPid = CString((LPCSTR)pData);
				sPid.TrimLeft();
				sPid.TrimRight();
				delete [] pData;

				// Get the existing event
				CString sEventName;
				sEventName = _T("ap") + sPid + _T("_shutdown");
				HANDLE hShutdownEvent = ::OpenEvent(EVENT_MODIFY_STATE, FALSE, sEventName);
				if (hShutdownEvent)
				{
					// Set the event
					res = ::SetEvent(hShutdownEvent);

					// Clean-up
					::CloseHandle(hShutdownEvent);

					// Check
					if (!res)
						::EnumKillProcByName(MICROAPACHE_FILENAME, TRUE);
				}
				else
					::EnumKillProcByName(MICROAPACHE_FILENAME, TRUE);
			}
			else
				::EnumKillProcByName(MICROAPACHE_FILENAME, TRUE);
		}
		else
			::EnumKillProcByName(MICROAPACHE_FILENAME, TRUE);
	}
	catch (CFileException* e)
	{
		if (pData)
			delete [] pData;
		e->Delete();
		::EnumKillProcByName(MICROAPACHE_FILENAME, TRUE);
	}

	// Wait a max of MICROAPACHE_TIMEOUT_MS
	res = TRUE;
	DWORD dwSleep = 0U;
	while (::EnumKillProcByName(MICROAPACHE_FILENAME) > 0)
	{
		dwSleep += MICROAPACHE_WAITTIME_MS;
		::Sleep(MICROAPACHE_WAITTIME_MS);
		if (dwSleep >= MICROAPACHE_TIMEOUT_MS)
		{
			res = FALSE;
			break;
		}
	}

	// Wait again a max of MICROAPACHE_TIMEOUT_MS
	if (!res)
	{
		::EnumKillProcByName(MICROAPACHE_FILENAME, TRUE);
		res = TRUE;
		dwSleep = 0U;
		while (::EnumKillProcByName(MICROAPACHE_FILENAME) > 0)
		{
			dwSleep += MICROAPACHE_WAITTIME_MS;
			::Sleep(MICROAPACHE_WAITTIME_MS);
			if (dwSleep >= MICROAPACHE_TIMEOUT_MS)
			{
				res = FALSE;
				break;
			}
		}
	}

	// Delete pid file
	::DeleteFile(MicroApacheGetPidFileName());

	return res;
}

// Return Values
// 1  : OK
// 0  : Failed to stop the web server
// -1 : Failed to start the web server
int CVideoDeviceDoc::MicroApacheReload()
{
	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();
	BOOL bOk = TRUE;
	if (pApp->m_bMicroApacheStarted)
	{
		pApp->m_MicroApacheWatchdogThread.Kill();
		if (bOk = MicroApacheShutdown())
			pApp->m_bMicroApacheStarted = FALSE;
	}
	if (!bOk)
		return 0;
	else
	{
		// Update / create doc root index.php and config file for microapache
		MicroApacheUpdateMainFiles();

		// Start server
		if (pApp->m_bStartMicroApache)
		{
			if (MicroApacheInitStart() && MicroApacheWaitStartDone())
			{
				pApp->m_bMicroApacheStarted = TRUE;
				pApp->m_MicroApacheWatchdogThread.Start(THREAD_PRIORITY_BELOW_NORMAL);
			}
			else
				return -1;
		}
	}
	return 1;
}

CString CVideoDeviceDoc::PhpGetConfigFileName()
{
	CString sAutoSaveDir = m_sRecordAutoSaveDir;
	sAutoSaveDir.TrimRight(_T('\\'));
	if (sAutoSaveDir != _T(""))
		return sAutoSaveDir + _T("\\") + PHP_CONFIGNAME_EXT;
	else
		return _T("");
}

CString CVideoDeviceDoc::PhpLoadConfigFile() 
{
	CString sPhpConfigFile = PhpGetConfigFileName();
	if (sPhpConfigFile == _T(""))
		return _T("");
	LPBYTE pData = NULL;
	try
	{
		// Open Config File
		CFile f(sPhpConfigFile,
				CFile::modeRead |
				CFile::shareDenyWrite);
		DWORD dwLength = (DWORD)f.GetLength();
		if (dwLength == 0)
			return _T("");

		// Allocate Buffer
		pData = new BYTE [dwLength+1];
		if (!pData)
			return _T("");
		
		// Read Data
		dwLength = f.Read(pData, dwLength);
		pData[dwLength] = '\0';
		CString s((LPCSTR)pData);
		delete [] pData;
		return s;
	}
	catch (CFileException* e)
	{
		if (pData)
			delete [] pData;
		e->Delete();
		return _T("");
	}
}

BOOL CVideoDeviceDoc::PhpSaveConfigFile(const CString& sConfig)
{
	CString sPhpConfigFile = PhpGetConfigFileName();
	if (sPhpConfigFile == _T(""))
		return FALSE;
	CString sPath = ::GetDriveAndDirName(sPhpConfigFile);
	if (!::IsExistingDir(sPath))
	{
		if (!::CreateDir(sPath))
			return FALSE;
	}
	LPSTR pData = NULL;
	int nLen = ::ToANSI(sConfig, &pData);
	if (nLen <= 0 || !pData)
	{
		if (pData)
			delete [] pData;
		return FALSE;
	}
	try
	{
		CFile f(sPhpConfigFile,
				CFile::modeCreate		|
				CFile::modeWrite		|
				CFile::shareDenyWrite);
		f.Write(pData, nLen);
		delete [] pData;
		return TRUE;
	}
	catch (CFileException* e)
	{
		delete [] pData;
		e->Delete();
		return FALSE;
	}
}

BOOL CVideoDeviceDoc::PhpConfigFileSetParam(const CString& sParam, const CString& sValue)
{
	// Load Config File
	CString sConfig = PhpLoadConfigFile();
	if (sConfig == _T(""))
		return FALSE;

	// Get length
	int nLength = sConfig.GetLength();

	// Find define
	const CString sDefine(_T("define"));
	CString sConfigLowerCase = sConfig;
	sConfigLowerCase.MakeLower();
	int nIndexDefine = sConfigLowerCase.Find(sDefine);
	while (nIndexDefine >= 0)
	{	
		// Skip define
		int i = nIndexDefine + sDefine.GetLength();

		// Skip spaces
		while (i < nLength && _istspace(sConfig[i])) i++;
		
		// Is (
		if (i < nLength && sConfig[i] == _T('('))
		{
			// Skip (
			i++;

			// Skip spaces
			while (i < nLength && _istspace(sConfig[i])) i++;

			// Skip " or '
			if (i < nLength && (sConfig[i] == _T('\"') || sConfig[i] == _T('\''))) i++;

			// Is sParam
			if (sConfig.Mid(i, sParam.GetLength()) == sParam)
			{
				// Skip sParam
				i += sParam.GetLength();

				// Skip " or '
				if (i < nLength && (sConfig[i] == _T('\"') || sConfig[i] == _T('\''))) i++;

				// Skip spaces
				while (i < nLength && _istspace(sConfig[i])) i++;

				// Skip ,
				if (i < nLength && sConfig[i] == _T(',')) i++;

				// Find end of define which is a )
				int nIndexValueStart = i;
				BOOL bParamStringStarted = FALSE;
				BOOL bParamStringEnded = FALSE;
				TCHAR cParamStringDelimiter = _T('\0');
				while (i < nLength)
				{
					if (cParamStringDelimiter == _T('\0'))	
					{ 
						if (sConfig[i] == _T('\"'))
						{
							bParamStringStarted = TRUE;
							cParamStringDelimiter = _T('\"');
						}
						else if (sConfig[i] == _T('\''))
						{
							bParamStringStarted = TRUE;
							cParamStringDelimiter = _T('\'');
						}
					}
					else if (sConfig[i-1] != _T('\\') && sConfig[i] == cParamStringDelimiter)
						bParamStringEnded = TRUE;
					if (((bParamStringStarted && bParamStringEnded) ||
						(!bParamStringStarted && !bParamStringEnded)) &&
						sConfig[i] == _T(')'))
					{
						sConfig.Delete(nIndexValueStart, i - nIndexValueStart);
						sConfig.Insert(nIndexValueStart, _T("\"") + sValue + _T("\""));
						return PhpSaveConfigFile(sConfig);
					}
					i++;
				}
				return FALSE;
			}
			else
				nIndexDefine = sConfigLowerCase.Find(sDefine, nIndexDefine + 1);
		}
		else
			nIndexDefine = sConfigLowerCase.Find(sDefine, nIndexDefine + 1);
	}

	// If not found -> insert before last comment
	int nIndexInsert = sConfigLowerCase.Find(_T("/**************************************\r\n* initialization, do not remove that! *\r\n**************************************/"));
	if (nIndexInsert >= 0)
	{
		sConfig.Insert(nIndexInsert, sDefine + _T(" (\"") + sParam + _T("\",\"") + sValue + _T("\");") + _T("\r\n\r\n"));
		return PhpSaveConfigFile(sConfig);
	}
	// If also not found -> insert after <?php
	else
	{
		nIndexInsert = sConfigLowerCase.Find(_T("<?php"));
		if (nIndexInsert >= 0)
		{
			nIndexInsert += 5; // Skip <?php
			sConfig.Insert(nIndexInsert, _T("\r\n") + sDefine + _T(" (\"") + sParam + _T("\",\"") + sValue + _T("\");"));
			return PhpSaveConfigFile(sConfig);
		}
	}

	return FALSE;
}

CString CVideoDeviceDoc::PhpConfigFileGetParam(const CString& sParam)
{
	// Load Config File
	CString sConfig = PhpLoadConfigFile();
	if (sConfig == _T(""))
		return _T("");

	// Get length
	int nLength = sConfig.GetLength();

	// Find define
	CString sDefine(_T("define"));
	CString sConfigLowerCase = sConfig;
	sConfigLowerCase.MakeLower();
	int nIndexDefine = sConfigLowerCase.Find(sDefine);
	while (nIndexDefine >= 0)
	{	
		// Skip define
		int i = nIndexDefine + sDefine.GetLength();

		// Skip spaces
		while (i < nLength && _istspace(sConfig[i])) i++;
		
		// Is (
		if (i < nLength && sConfig[i] == _T('('))
		{
			// Skip (
			i++;

			// Skip spaces
			while (i < nLength && _istspace(sConfig[i])) i++;

			// Skip " or '
			if (i < nLength && (sConfig[i] == _T('\"') || sConfig[i] == _T('\''))) i++;

			// Is sParam
			if (sConfig.Mid(i, sParam.GetLength()) == sParam)
			{
				// Skip sParam
				i += sParam.GetLength();

				// Skip " or '
				if (i < nLength && (sConfig[i] == _T('\"') || sConfig[i] == _T('\''))) i++;

				// Skip spaces
				while (i < nLength && _istspace(sConfig[i])) i++;

				// Skip ,
				if (i < nLength && sConfig[i] == _T(',')) i++;

				// Find end of define which is a )
				int nIndexValueStart = i;
				int nIndexParamStringStart = -1;
				int nIndexParamStringEnd = -1;
				TCHAR cParamStringDelimiter = _T('\0');
				while (i < nLength)
				{
					if (cParamStringDelimiter == _T('\0'))	
					{ 
						if (sConfig[i] == _T('\"'))
						{
							nIndexParamStringStart = i + 1;
							cParamStringDelimiter = _T('\"');
						}
						else if (sConfig[i] == _T('\''))
						{
							nIndexParamStringStart = i + 1;
							cParamStringDelimiter = _T('\'');
						}
					}
					else if (sConfig[i-1] != _T('\\') && sConfig[i] == cParamStringDelimiter)
					{
						nIndexParamStringEnd = i - 1;
					}
					if (sConfig[i] == _T(')'))
					{
						if (nIndexParamStringStart != -1 && nIndexParamStringEnd != -1)
							return sConfig.Mid(nIndexParamStringStart, nIndexParamStringEnd - nIndexParamStringStart + 1);
						else if (nIndexParamStringStart == -1 && nIndexParamStringEnd == -1)
							return sConfig.Mid(nIndexValueStart, i - nIndexValueStart);
					}
					i++;
				}
				return _T("");
			}
			else
				nIndexDefine = sConfigLowerCase.Find(sDefine, nIndexDefine + 1);
		}
		else
			nIndexDefine = sConfigLowerCase.Find(sDefine, nIndexDefine + 1);
	}
	return _T("");
}

CTime CVideoDeviceDoc::CalcTime(DWORD dwUpTime, const CTime& RefTime, DWORD dwRefUpTime)
{
	// Ref. time older
	DWORD dwTimeDifference = dwRefUpTime - dwUpTime;
	if (dwTimeDifference >= 0x80000000U)
	{
		CTimeSpan TimeSpan((time_t)Round((double)(dwUpTime - dwRefUpTime) / 1000.0));
		return RefTime + TimeSpan;
	}
	// Ref. time younger
	else
	{
		CTimeSpan TimeSpan((time_t)Round((double)dwTimeDifference / 1000.0));
		return RefTime - TimeSpan;
	}
}

void CVideoDeviceDoc::AddFrameTime(CDib* pDib, CTime RefTime, DWORD dwRefUpTime, int nRefFontSize)
{
	// Check
	if (!pDib)
		return;

	RefTime = CalcTime(pDib->GetUpTime(), RefTime, dwRefUpTime);
	CRect rcRect;
	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = pDib->GetWidth();
	rcRect.bottom = pDib->GetHeight();

	CFont Font;
	int nFontSize = ::ScaleFont(rcRect.right, rcRect.bottom, nRefFontSize, FRAMETAG_REFWIDTH, FRAMETAG_REFHEIGHT);
	Font.CreatePointFont(nFontSize * 10, DEFAULT_FONTFACE);

	CString sTime = ::MakeTimeLocalFormat(RefTime, TRUE);
	pDib->AddSingleLineText(sTime,
							rcRect,
							&Font,
							(DT_LEFT | DT_BOTTOM),
							FRAMETIME_COLOR,
							OPAQUE,
							DXDRAW_BKG_COLOR);

	CString sDate = ::MakeDateLocalFormat(RefTime);
	pDib->AddSingleLineText(sDate,
							rcRect,
							&Font,
							(DT_LEFT | DT_TOP),
							FRAMEDATE_COLOR,
							OPAQUE,
							DXDRAW_BKG_COLOR);
}

void CVideoDeviceDoc::AddFrameCount(CDib* pDib, int nCount, int nRefFontSize)
{
	// Check
	if (!pDib)
		return;

	CRect rcRect;
	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = pDib->GetWidth();
	rcRect.bottom = pDib->GetHeight();

	CFont Font;
	int nFontSize = ::ScaleFont(rcRect.right, rcRect.bottom, nRefFontSize, FRAMETAG_REFWIDTH, FRAMETAG_REFHEIGHT);
	Font.CreatePointFont(nFontSize * 10, DEFAULT_FONTFACE);

	CString sCount;
	sCount.Format(_T("%d"), nCount);
	pDib->AddSingleLineText(sCount,
							rcRect,
							&Font,
							(DT_RIGHT | DT_BOTTOM),
							FRAMECOUNT_COLOR,
							OPAQUE,
							DXDRAW_BKG_COLOR);
}

BOOL CVideoDeviceDoc::Rotate180(CDib* pDib)
{
	// Check
	if (!pDib || !pDib->GetBMI() || !pDib->GetBits())
		return FALSE;
	
	BYTE pix;
	int CurLine;
	int nHeight2 = pDib->GetHeight() / 2;
	int nHeight4 = nHeight2 / 2;
	int nWidth2 = pDib->GetWidth() / 2;
	LPBYTE lpSrcBitsY = pDib->GetBits();
	LPBYTE lpDstBitsY = lpSrcBitsY + (pDib->GetHeight() - 1) * pDib->GetWidth();
	LPBYTE lpSrcBitsU = pDib->GetBits() + pDib->GetHeight() * pDib->GetWidth();
	LPBYTE lpDstBitsU = lpSrcBitsU + (nHeight2 - 1) * nWidth2;
	LPBYTE lpSrcBitsV = pDib->GetBits() + pDib->GetHeight() * pDib->GetWidth() + nHeight2 * nWidth2;
	LPBYTE lpDstBitsV = lpSrcBitsV + (nHeight2 - 1) * nWidth2;

	// Rotate Y
	// pDib->GetHeight() needs to be divisible by 2,
	// otherwise the middle line is not rotated!
	for (CurLine = 0 ; CurLine < nHeight2 ; CurLine++)
	{
		for (int i = 0 ; i < (int)pDib->GetWidth() ; i++)
		{
			pix = lpSrcBitsY[i];
			lpSrcBitsY[i] = lpDstBitsY[pDib->GetWidth() - i - 1];
			lpDstBitsY[pDib->GetWidth() - i - 1] = pix;
		}
		lpDstBitsY -= pDib->GetWidth();
		lpSrcBitsY += pDib->GetWidth();
	}

	// Rotate U
	// pDib->GetHeight() needs to be divisible by 4,
	// otherwise the middle line is not rotated!
	for (CurLine = 0 ; CurLine < nHeight4 ; CurLine++)
	{
		for (int i = 0 ; i < nWidth2 ; i++)
		{
			pix = lpSrcBitsU[i];
			lpSrcBitsU[i] = lpDstBitsU[nWidth2 - i - 1];
			lpDstBitsU[nWidth2 - i - 1] = pix;
		}
		lpDstBitsU -= nWidth2;
		lpSrcBitsU += nWidth2;
	}

	// Rotate V
	// pDib->GetHeight() needs to be divisible by 4,
	// otherwise the middle line is not rotated!
	for (CurLine = 0 ; CurLine < nHeight4 ; CurLine++)
	{
		for (int i = 0 ; i < nWidth2 ; i++)
		{
			pix = lpSrcBitsV[i];
			lpSrcBitsV[i] = lpDstBitsV[nWidth2 - i - 1];
			lpDstBitsV[nWidth2 - i - 1] = pix;
		}
		lpDstBitsV -= nWidth2;
		lpSrcBitsV += nWidth2;
	}

	return TRUE;
}

BOOL CVideoDeviceDoc::Deinterlace(CDib* pDib)
{
	if (!pDib)
		return FALSE;
	AVPicture Frame;
	PixelFormat pix_fmt = CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pDib->GetBMI());
	avpicture_fill(	&Frame,
					(uint8_t*)pDib->GetBits(),
					pix_fmt,
					pDib->GetWidth(),
					pDib->GetHeight());
	return (avpicture_deinterlace(	&Frame,	// Dst
									&Frame,	// Src
									pix_fmt,
									pDib->GetWidth(),
									pDib->GetHeight()) >= 0);
}

void CVideoDeviceDoc::ProcessNoI420NoM420Frame(LPBYTE pData, DWORD dwSize)
{	
	// Decode ffmpeg supported formats
	m_pProcessFrameExtraDib->SetBMI((LPBITMAPINFO)&m_ProcessFrameBMI);
	if (m_AVDecoder.Decode(	(LPBITMAPINFO)&m_CaptureBMI,
							pData,
							dwSize,
							m_pProcessFrameExtraDib)) // this function will allocate the dst bits if necessary
	{
		if (m_AVDecoder.GetCodecId() == CODEC_ID_MJPEG)
		{
			m_lCompressedDataRateSum += dwSize;
			ProcessI420Frame(m_pProcessFrameExtraDib->GetBits(), m_pProcessFrameExtraDib->GetImageSize(), pData, dwSize);
		}
		else
			ProcessI420Frame(m_pProcessFrameExtraDib->GetBits(), m_pProcessFrameExtraDib->GetImageSize(), NULL, 0U);
	}
	// In case that avcodec_decode_video fails use LoadJPEG which is more fault tolerant, but slower...
	else if (m_CaptureBMI.bmiHeader.biCompression == FCC('MJPG'))
	{
		if (m_pProcessFrameExtraDib->LoadJPEG(pData, dwSize, 1, TRUE) && m_pProcessFrameExtraDib->Compress(FCC('I420')))
		{
			m_lCompressedDataRateSum += dwSize;
			ProcessI420Frame(m_pProcessFrameExtraDib->GetBits(), m_pProcessFrameExtraDib->GetImageSize(), pData, dwSize);
		}
	}
	// Other formats
	else
	{
		m_pProcessFrameExtraDib->SetBMI((LPBITMAPINFO)&m_CaptureBMI);
		m_pProcessFrameExtraDib->SetBits(pData, dwSize);
		if (m_pProcessFrameExtraDib->IsCompressed())
			m_pProcessFrameExtraDib->Decompress(32);
		if (m_pProcessFrameExtraDib->Compress(FCC('I420')))
			ProcessI420Frame(m_pProcessFrameExtraDib->GetBits(), m_pProcessFrameExtraDib->GetImageSize(), NULL, 0U);
	}
}

/* 
M420 is a YUV 4:2:0 format, with 2 lines Y and 1 line UV interleaved
There are multiple benefits using M420:
- M420 is easier/faster to convert to I420
- Requires 25% less USB bandwidth than YUY2
- Equivalent quality compared to YUY2 when used as a source for H.264/VC-1/DivX
- Packet based for compatibility with current CMOS webcams
- Requires less CPU than MJPEG, delivers higher quality
- Requires less CPU than YUY2
- Allows up to 720p @ 15fps when transported through a USB 2.0 HS Isochronous pipe
Note:
When using __asm to write assembly language in C/C++ functions, you don't need to
preserve the EAX, EBX, ECX, EDX, ESI, or EDI registers. However, using these registers
will affect code quality because the register allocator cannot use them to store values
across __asm blocks. The compiler avoids enregistering variables across an __asm block
if the register's contents would be changed by the __asm block. In addition, by using
EBX, ESI or EDI in inline assembly code, you force the compiler to save and restore
those registers in the function prologue and epilogue.
*/
void CVideoDeviceDoc::ProcessM420Frame(LPBYTE pData, DWORD dwSize)
{
	// Allocate Bits?
	BITMAPINFO DstBmi;
	memset(&DstBmi, 0, sizeof(BITMAPINFOHEADER));
	DstBmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	DstBmi.bmiHeader.biWidth = m_ProcessFrameBMI.bmiHeader.biWidth;
	DstBmi.bmiHeader.biHeight = m_ProcessFrameBMI.bmiHeader.biHeight;
	DstBmi.bmiHeader.biPlanes = 1;
	DstBmi.bmiHeader.biCompression = FCC('I420');
	DstBmi.bmiHeader.biBitCount = 12;
	int stride = ::CalcYUVStride(DstBmi.bmiHeader.biCompression, DstBmi.bmiHeader.biWidth);
	DstBmi.bmiHeader.biSizeImage = ::CalcYUVSize(DstBmi.bmiHeader.biCompression, stride, DstBmi.bmiHeader.biHeight);
	if (!m_pProcessFrameExtraDib->SetBMI(&DstBmi))
	{
		CString sMsg;
		sMsg.Format(_T("%s, error setting I420 format for M420 decoding!\n"), GetAssignedDeviceName());
		TRACE(sMsg);
		::LogLine(sMsg);
		return;
	}
	if (!m_pProcessFrameExtraDib->GetBits())
	{
		if (!m_pProcessFrameExtraDib->AllocateBitsFast(	m_pProcessFrameExtraDib->GetBitCount(),
														m_pProcessFrameExtraDib->GetCompression(),
														m_pProcessFrameExtraDib->GetWidth(),
														m_pProcessFrameExtraDib->GetHeight()))
		{
			CString sMsg;
			sMsg.Format(_T("%s, error allocating I420 buffer for M420 decoding!\n"), GetAssignedDeviceName());
			TRACE(sMsg);
			::LogLine(sMsg);
			return;
		}
	}

	// Init conversion vars
	int width = m_ProcessFrameBMI.bmiHeader.biWidth;
	int height = m_ProcessFrameBMI.bmiHeader.biHeight;
	int halfwidth = width >> 1;
	int halfheight = height >> 1;
	LPBYTE src_y = pData;
	LPBYTE src_uv = src_y + 2 * width; 
	LPBYTE dst_y = m_pProcessFrameExtraDib->GetBits();
	LPBYTE dst_u = dst_y + width * height;
	LPBYTE dst_v = dst_u + halfwidth * halfheight;
	int y;

	// Copy Y plane
	for (y = 0 ; y < height - 1 ; y += 2)
	{
		memcpy(dst_y, src_y, width);
		src_y += width;
		dst_y += width;
		memcpy(dst_y, src_y, width);
		src_y += 2 * width; // skip UV row
		dst_y += width;
	}

	// Unpack UV
	if (g_bSSE2						&&
		ISALIGNED(halfwidth, 16)	&&
		ISALIGNED(src_uv, 16)		&&
		ISALIGNED(dst_u, 16)		&&
		ISALIGNED(dst_v, 16))
	{
		for (y = 0 ; y < halfheight ; y++)
		{
			__asm
			{
				mov        eax, src_uv
				mov        edx, dst_u
				mov        edi, dst_v
				mov        ecx, halfwidth
				pcmpeqb    xmm5, xmm5	// generate mask 0x00ff00ff
				psrlw      xmm5, 8
				sub        edi, edx

				align      16
				convertloop:
				movdqa     xmm0, [eax]
				movdqa     xmm1, [eax + 16]
				lea        eax,  [eax + 32]
				movdqa     xmm2, xmm0
				movdqa     xmm3, xmm1
				pand       xmm0, xmm5	// even bytes
				pand       xmm1, xmm5
				packuswb   xmm0, xmm1
				psrlw      xmm2, 8		// odd bytes
				psrlw      xmm3, 8
				packuswb   xmm2, xmm3
				movdqa     [edx], xmm0
				movdqa     [edx + edi], xmm2
				lea        edx, [edx + 16]
				sub        ecx, 16
				ja         convertloop
			}
			dst_u += halfwidth;
			dst_v += halfwidth;
			src_uv += 3 * width;
		}
	}
	else
	{
		for (y = 0 ; y < halfheight ; y++)
		{
			for (int x = 0 ; x < halfwidth ; x++)
			{
				dst_u[0] = src_uv[0];
				dst_v[0] = src_uv[1];
				src_uv += 2;
				dst_u++;
				dst_v++;
			}
			src_uv += 2 * width;
		}
	}

	// Call Process Frame
	ProcessI420Frame(m_pProcessFrameExtraDib->GetBits(), m_pProcessFrameExtraDib->GetImageSize(), NULL, 0U);
}

void CVideoDeviceDoc::ProcessI420Frame(LPBYTE pData, DWORD dwSize, LPBYTE pMJPGData, DWORD dwMJPGSize)
{
	// Timing
	DWORD dwCurrentFrameTime;
	DWORD dwPrevInitUpTime = (DWORD)m_lCurrentInitUpTime;
	CTime CurrentTime = CTime::GetCurrentTime();
	DWORD dwCurrentInitUpTime = ::timeGetTime();
	CTimeSpan TimeDiff1(CurrentTime - m_1SecTime);
	BOOL b1SecTick = FALSE;
	if (TimeDiff1.GetTotalSeconds() >= 1 || TimeDiff1.GetTotalSeconds() < 0)
	{
		b1SecTick = TRUE;
		m_1SecTime = CurrentTime;
	}
	BOOL bStartupSettled = FALSE;
	if (m_bCaptureStarted)
	{
		CTimeSpan TimeSinceStart = CurrentTime - m_CaptureStartTime;
		if (TimeSinceStart.GetTotalSeconds() >= STARTUP_SETTLE_TIME_SEC ||
			TimeSinceStart.GetTotalSeconds() < 0)
			bStartupSettled = TRUE;
	}

	// Process Frame Stop Engine
	::EnterCriticalSection(&m_csProcessFrameStop);
	// If restarting reset frame rate calculation
	if (m_dwProcessFrameStopped != 0U && m_dwStopProcessFrame == 0U)
	{
		m_dwEffectiveFrameTimeCountUp = 0U;
		m_dEffectiveFrameTimeSum = 0.0;
	}
	m_dwProcessFrameStopped = m_dwStopProcessFrame;
	BOOL bDoProcessFrame = (m_dwProcessFrameStopped == 0U);
	::LeaveCriticalSection(&m_csProcessFrameStop);
	
	// Detect, Copy, Snapshot, Record and finally Draw
	CDib* pDib = m_pProcessFrameDib;
	if (bDoProcessFrame && pData && dwSize > 0 && pDib &&
		pDib->SetBMI((LPBITMAPINFO)&m_ProcessFrameBMI) &&
		pDib->SetBits(pData, dwSize))
	{
		// Clear the user flag from any previous content
		pDib->SetUserFlag(0);

		// De-Interlace if divisible by 4
		if (m_bDeinterlace && (pDib->GetWidth() & 3) == 0 && (pDib->GetHeight() & 3) == 0)
		{
			Deinterlace(pDib);
			pDib->SetUserFlag(pDib->GetUserFlag() | FRAME_USER_FLAG_DEINTERLACE);
		}

		// Rotate by 180° if divisible by 4
		if (m_bRotate180 && (pDib->GetHeight() & 3) == 0)
		{
			Rotate180(pDib);
			pDib->SetUserFlag(pDib->GetUserFlag() | FRAME_USER_FLAG_ROTATE180);
		}

		// Set the UpTime Var
		pDib->SetUpTime(dwCurrentInitUpTime);

		// Move samples from audio queue to Dib
		if (m_bCaptureAudio)
		{
			::EnterCriticalSection(&m_CaptureAudioThread.m_csAudioList);
			pDib->MoveUserList(m_CaptureAudioThread.m_AudioList);
			::LeaveCriticalSection(&m_CaptureAudioThread.m_csAudioList);
		}
		else
			pDib->FreeUserList();

		// Movement Detection only when start-up settled
		// (especially for audio/video synchronization)
		if (bStartupSettled)
		{
			// Detection Scheduler
			DWORD dwVideoProcessorMode = m_dwVideoProcessorMode;
			if (dwVideoProcessorMode > 0 && m_nDetectionStartStop > 0)
			{
				BOOL bInSchedule = TRUE;
				switch (CurrentTime.GetDayOfWeek())
				{
					case 1 : if (!m_bDetectionSunday)	bInSchedule = FALSE; break;
					case 2 : if (!m_bDetectionMonday)	bInSchedule = FALSE; break;
					case 3 : if (!m_bDetectionTuesday)	bInSchedule = FALSE; break;
					case 4 : if (!m_bDetectionWednesday)bInSchedule = FALSE; break;
					case 5 : if (!m_bDetectionThursday)	bInSchedule = FALSE; break;
					case 6 : if (!m_bDetectionFriday)	bInSchedule = FALSE; break;
					case 7 : if (!m_bDetectionSaturday)	bInSchedule = FALSE; break;
					default: break;
				}
				CTime timeonly(	2000,
								1,
								1,
								CurrentTime.GetHour(),
								CurrentTime.GetMinute(),
								CurrentTime.GetSecond());
				if (m_DetectionStartTime <= m_DetectionStopTime)
				{
					if (timeonly < m_DetectionStartTime || timeonly > m_DetectionStopTime)
						bInSchedule = FALSE;
				}
				else
				{
					if (timeonly < m_DetectionStartTime && timeonly > m_DetectionStopTime)
						bInSchedule = FALSE;
				}

				// 1 -> Enable detection on specified schedule
				if (m_nDetectionStartStop == 1) 
				{
					if (!bInSchedule)
						dwVideoProcessorMode = NO_DETECTOR;
				}
				// 2 -> Disable detection on specified schedule
				else
				{
					if (bInSchedule)
						dwVideoProcessorMode = NO_DETECTOR;
				}
			}

			// Reset moving background frame if software detection engine has been turned-off
			if (!(dwVideoProcessorMode & SOFTWARE_MOVEMENT_DETECTOR))
			{	
				if (m_pMovementDetectorBackgndDib)
				{
					delete m_pMovementDetectorBackgndDib;
					m_pMovementDetectorBackgndDib = NULL;
				}
			}

			// Do Motion Detection Processing
			MovementDetectionProcessing(pDib, pMJPGData, dwMJPGSize, dwVideoProcessorMode, b1SecTick);
		}

		// Copy to Clipboard
		if (m_bDoEditCopy)
			EditCopy(pDib, CurrentTime);

		// Manual Snapshot to JPEG File(s) (this copies also to clipboard)
		if (m_bDoEditSnapshot)
			EditSnapshot(pDib, CurrentTime);

		// Timed Snapshot
		Snapshot(pDib, CurrentTime);

		// Add Frame Time if User Wants it
		if (m_bShowFrameTime)
			AddFrameTime(pDib, CurrentTime, dwCurrentInitUpTime, m_nRefFontSize);

		// Record Video only when start-up settled
		// (especially for audio/video synchronization)
		if (bStartupSettled)
		{
			::EnterCriticalSection(&m_csAVRec);
			if (m_pAVRec)
			{
				// Add Frame
				BOOL bOk = m_pAVRec->AddFrame(	m_pAVRec->VideoStreamNumToStreamNum(ACTIVE_VIDEO_STREAM),
												pDib,
												m_bInterleave ? true : false);

				// Add Audio Samples
				if (m_bCaptureAudio)
				{
					POSITION posUserBuf = pDib->m_UserList.GetHeadPosition();
					while (posUserBuf)
					{
						CUserBuf UserBuf = pDib->m_UserList.GetNext(posUserBuf);
						if (m_CaptureAudioThread.m_pDstWaveFormat->wFormatTag == WAVE_FORMAT_PCM)
						{
							m_pAVRec->AddRawAudioPacket(m_pAVRec->AudioStreamNumToStreamNum(ACTIVE_AUDIO_STREAM),
														UserBuf.m_dwSize,
														UserBuf.m_pBuf,
														m_bInterleave ? true : false);
						}
						else
						{
							int nNumOfSrcSamples = (m_CaptureAudioThread.m_pSrcWaveFormat && (m_CaptureAudioThread.m_pSrcWaveFormat->nBlockAlign > 0)) ? UserBuf.m_dwSize / m_CaptureAudioThread.m_pSrcWaveFormat->nBlockAlign : 0;
							m_pAVRec->AddAudioSamples(	m_pAVRec->AudioStreamNumToStreamNum(ACTIVE_AUDIO_STREAM),
														nNumOfSrcSamples,
														UserBuf.m_pBuf,
														m_bInterleave ? true : false);
						}
					}
				}

				// Recording Up-Time Init
				if (m_bRecFirstFrame)
				{
					m_dwRecLastUpTime  = m_dwRecFirstUpTime = dwCurrentInitUpTime;
					m_nRecordedFrames = 1;
					m_bRecFirstFrame = FALSE;
				}
				// Recording Up-Time Update
				else
				{
					m_dwRecLastUpTime = dwCurrentInitUpTime;
					m_nRecordedFrames++;
				}
					
				// Every second check for segmentation
				if (bOk && b1SecTick && m_bRecTimeSegmentation &&
					CurrentTime >= m_NextRecTime)
				{
					NextRecTime(CurrentTime);
					bOk = NextAviFile();
				}

				// If not OK -> Stop Recording
				if (!bOk)
					CloseAndShowAviRec();
			}
			::LeaveCriticalSection(&m_csAVRec);
		}

		// Swap Dib pointers
		::EnterCriticalSection(&m_csDib);
		m_pProcessFrameDib = m_pDib;
		m_pDib = pDib;
		::LeaveCriticalSection(&m_csDib);

		// Trigger a Draw
		if (m_bVideoView)
			m_WatchdogAndDrawThread.TriggerDraw();

		// Set start time, flag and open the Assistant dialog
		if (!m_bCaptureStarted)
		{
			// Do not invert the order of the following two assignments!
			m_CaptureStartTime = CurrentTime;
			m_bCaptureStarted = TRUE;
			if (m_bDeviceFirstRun)
			{
				::PostMessage(	GetView()->GetSafeHwnd(),
								WM_THREADSAFE_CAPTUREASSISTANT,
								0, 0);
			}
		}
	}

	// Count-Up Frames
	m_dwFrameCountUp++;

	// Calc. Effective Frame Rate
	::InterlockedExchange(&m_lCurrentInitUpTime, (LONG)dwCurrentInitUpTime);
	dwCurrentFrameTime = dwCurrentInitUpTime - dwPrevInitUpTime;
	if (dwCurrentFrameTime <= PROCESS_MAX_FRAMETIME)
	{
		// It takes 2 sec (or more for frame rates < 1fps)
		// to calc. the effective framerate and datarate
		m_dEffectiveFrameTimeSum += (double)dwCurrentFrameTime;
		++m_dwEffectiveFrameTimeCountUp;
		if (dwCurrentFrameTime >= 1000U)
		{
			if (m_dwEffectiveFrameTimeCountUp >= 2U && m_dEffectiveFrameTimeSum > 0.0)
			{
				m_dEffectiveFrameRate = 1000.0 * (double)m_dwEffectiveFrameTimeCountUp / m_dEffectiveFrameTimeSum;
				m_dwEffectiveFrameTimeCountUp = 0U;
				if (m_lCompressedDataRateSum > 0)
				{
					m_lCompressedDataRate = Round(1000.0 * (double)m_lCompressedDataRateSum / m_dEffectiveFrameTimeSum);
					m_lCompressedDataRateSum = 0;
				}
				m_dEffectiveFrameTimeSum = 0.0;
			}
		}
		else if (dwCurrentFrameTime >= 500U)
		{
			if (m_dwEffectiveFrameTimeCountUp >= 4U && m_dEffectiveFrameTimeSum > 0.0)
			{
				m_dEffectiveFrameRate = 1000.0 * (double)m_dwEffectiveFrameTimeCountUp / m_dEffectiveFrameTimeSum;
				m_dwEffectiveFrameTimeCountUp = 0U;
				if (m_lCompressedDataRateSum > 0)
				{
					m_lCompressedDataRate = Round(1000.0 * (double)m_lCompressedDataRateSum / m_dEffectiveFrameTimeSum);
					m_lCompressedDataRateSum = 0;
				}
				m_dEffectiveFrameTimeSum = 0.0;
			}
		}
		else if (dwCurrentFrameTime >= 250U)
		{
			if (m_dwEffectiveFrameTimeCountUp >= 8U && m_dEffectiveFrameTimeSum > 0.0)
			{
				m_dEffectiveFrameRate = 1000.0 * (double)m_dwEffectiveFrameTimeCountUp / m_dEffectiveFrameTimeSum;
				m_dwEffectiveFrameTimeCountUp = 0U;
				if (m_lCompressedDataRateSum > 0)
				{
					m_lCompressedDataRate = Round(1000.0 * (double)m_lCompressedDataRateSum / m_dEffectiveFrameTimeSum);
					m_lCompressedDataRateSum = 0;
				}
				m_dEffectiveFrameTimeSum = 0.0;
			}
		}
		else if (dwCurrentFrameTime >= 125U)
		{
			if (m_dwEffectiveFrameTimeCountUp >= 16U && m_dEffectiveFrameTimeSum > 0.0)
			{
				m_dEffectiveFrameRate = 1000.0 * (double)m_dwEffectiveFrameTimeCountUp / m_dEffectiveFrameTimeSum;
				m_dwEffectiveFrameTimeCountUp = 0U;
				if (m_lCompressedDataRateSum > 0)
				{
					m_lCompressedDataRate = Round(1000.0 * (double)m_lCompressedDataRateSum / m_dEffectiveFrameTimeSum);
					m_lCompressedDataRateSum = 0;
				}
				m_dEffectiveFrameTimeSum = 0.0;
			}
		}
		else if (dwCurrentFrameTime >= 63U)
		{
			if (m_dwEffectiveFrameTimeCountUp >= 32U && m_dEffectiveFrameTimeSum > 0.0)
			{
				m_dEffectiveFrameRate = 1000.0 * (double)m_dwEffectiveFrameTimeCountUp / m_dEffectiveFrameTimeSum;
				m_dwEffectiveFrameTimeCountUp = 0U;
				if (m_lCompressedDataRateSum > 0)
				{
					m_lCompressedDataRate = Round(1000.0 * (double)m_lCompressedDataRateSum / m_dEffectiveFrameTimeSum);
					m_lCompressedDataRateSum = 0;
				}
				m_dEffectiveFrameTimeSum = 0.0;
			}
		}
		else
		{
			if (m_dwEffectiveFrameTimeCountUp >= 64U && m_dEffectiveFrameTimeSum > 0.0)
			{
				m_dEffectiveFrameRate = 1000.0 * (double)m_dwEffectiveFrameTimeCountUp / m_dEffectiveFrameTimeSum;
				m_dwEffectiveFrameTimeCountUp = 0U;
				if (m_lCompressedDataRateSum > 0)
				{
					m_lCompressedDataRate = Round(1000.0 * (double)m_lCompressedDataRateSum / m_dEffectiveFrameTimeSum);
					m_lCompressedDataRateSum = 0;
				}
				m_dEffectiveFrameTimeSum = 0.0;
			}
		}
		if (m_dwEffectiveFrameTimeCountUp == 0U)
		{
			::PostMessage(	GetView()->GetSafeHwnd(),
							WM_THREADSAFE_SETDOCUMENTTITLE,
							0, 0);
		}
	}

	DWORD dwCurrentEndUpTime = ::timeGetTime();
	DWORD dwProcessFrameTime = dwCurrentEndUpTime - dwCurrentInitUpTime;
	::InterlockedExchange(&m_lProcessFrameTime, (LONG)dwProcessFrameTime);
}

void CVideoDeviceDoc::SnapshotRate(double dRate)
{
	CString sText;
	double dRateFloor = floor(dRate);
	int nRate = (int)dRateFloor;
	int nRateMs = Round(1000.0 * (dRate - dRateFloor));

	// Set seconds rate
	if (nRate >= 0)
	{
		sText.Format(_T("%d"), nRate);
		m_nSnapshotRate = nRate;
	}
	else
	{
		sText.Format(_T("%d"), DEFAULT_SNAPSHOT_RATE);
		m_nSnapshotRate = DEFAULT_SNAPSHOT_RATE;
	}
	PhpConfigFileSetParam(PHPCONFIG_SNAPSHOTREFRESHSEC, sText);

	// Set milliseconds rate
	if (nRate == 0 && nRateMs != 0) // rate values in range ]0.0,1.0[
	{
		sText.Format(_T("%d"), nRateMs);
		m_nSnapshotRateMs = nRateMs;
	}
	else
	{
		sText.Format(_T("%d"), DEFAULT_SERVERPUSH_POLLRATE_MS);
		m_nSnapshotRateMs = 0;
	}
	PhpConfigFileSetParam(PHPCONFIG_SERVERPUSH_POLLRATE_MS, sText);
}

void CVideoDeviceDoc::Snapshot(CDib* pDib, const CTime& Time)
{
	// Check
	if (m_SaveSnapshotThread.IsAlive())
		return;

	// Get uptime
	DWORD dwUpTime = pDib->GetUpTime();

	// Init bDoSnapshot flag
	BOOL bDoSnapshot = FALSE;
	int nFrameTime = Round(1000.0 / m_dFrameRate);
	if (m_dEffectiveFrameRate > 0.0)
		nFrameTime = Round(1000.0 / m_dEffectiveFrameRate);
	int nSnapshotRateMs = 1000 * m_nSnapshotRate + m_nSnapshotRateMs;
	if (nFrameTime >= nSnapshotRateMs)
		bDoSnapshot = TRUE;
	else
	{
		DWORD dwMaxUpTimeDiff = (DWORD)(3 * nSnapshotRateMs);
		DWORD dwCurrentUpTimeDiff = dwUpTime - m_dwNextSnapshotUpTime;
		DWORD dwCurrentUpTimeDiffInv = m_dwNextSnapshotUpTime - dwUpTime;
		if (dwCurrentUpTimeDiff >= 0x80000000U)
		{
			if (dwCurrentUpTimeDiffInv >= dwMaxUpTimeDiff)	// m_dwNextSnapshotUpTime is to much in the future
				m_dwNextSnapshotUpTime = dwUpTime;			// reset it!
		}
		else
		{
			if (dwCurrentUpTimeDiff >= dwMaxUpTimeDiff)		// m_dwNextSnapshotUpTime is to old
				m_dwNextSnapshotUpTime = dwUpTime;			// reset it!
			else
			{
				m_dwNextSnapshotUpTime += (DWORD)nSnapshotRateMs;
				bDoSnapshot = TRUE;
			}
		}
	}
	if (bDoSnapshot && m_bSnapshotStartStop)
	{
		CTime timeonly(	2000,
						1,
						1,
						Time.GetHour(),
						Time.GetMinute(),
						Time.GetSecond());
		::EnterCriticalSection(&m_csSnapshotConfiguration);
		if (m_SnapshotStartTime <= m_SnapshotStopTime)
		{
			if (timeonly < m_SnapshotStartTime || timeonly > m_SnapshotStopTime)
				bDoSnapshot = FALSE;
		}
		else
		{
			if (timeonly < m_SnapshotStartTime && timeonly > m_SnapshotStopTime)
				bDoSnapshot = FALSE;
		}
		::LeaveCriticalSection(&m_csSnapshotConfiguration);
	}

	// If nothing to do, return
	if (!bDoSnapshot)
		return;

	// Start Snapshot Thread
	// (we need the history jpgs to make the swf video file inside the snapshot SWF thread,
	// user unwanted history jpgs are deleted in snapshot SWF thread)
	m_SaveSnapshotThread.m_Dib = *pDib;
	m_SaveSnapshotThread.m_bSnapshotHistoryJpeg = (m_bSnapshotHistoryJpeg || m_bSnapshotHistorySwf);
	m_SaveSnapshotThread.m_bSnapshotHistoryJpegFtp = m_bSnapshotHistoryJpegFtp;
	m_SaveSnapshotThread.m_bShowFrameTime = m_bShowFrameTime;
	m_SaveSnapshotThread.m_nRefFontSize = m_nRefFontSize;
	m_SaveSnapshotThread.m_bSnapshotThumb = m_bSnapshotThumb;
	m_SaveSnapshotThread.m_bSnapshotLiveJpeg = m_bSnapshotLiveJpeg;
	m_SaveSnapshotThread.m_bSnapshotLiveJpegFtp = m_bSnapshotLiveJpegFtp;
	m_SaveSnapshotThread.m_nSnapshotThumbWidth = m_nSnapshotThumbWidth;
	m_SaveSnapshotThread.m_nSnapshotThumbHeight = m_nSnapshotThumbHeight;
	m_SaveSnapshotThread.m_nSnapshotCompressionQuality = m_nSnapshotCompressionQuality;
	m_SaveSnapshotThread.m_Time = Time;
	m_SaveSnapshotThread.m_sSnapshotAutoSaveDir = m_sRecordAutoSaveDir;
	::EnterCriticalSection(&m_csSnapshotConfiguration);
	m_SaveSnapshotThread.m_sSnapshotLiveJpegName = m_sSnapshotLiveJpegName;
	m_SaveSnapshotThread.m_sSnapshotLiveJpegThumbName = m_sSnapshotLiveJpegThumbName;
	m_SaveSnapshotThread.m_Config = m_SnapshotFTPUploadConfiguration;
	::LeaveCriticalSection(&m_csSnapshotConfiguration);
	m_SaveSnapshotThread.Start();

	// Start Snapshot SWF Thread?
	if (!m_sRecordAutoSaveDir.IsEmpty() && m_bSnapshotHistorySwf && !m_SaveSnapshotSWFThread.IsAlive())
	{
		CTime Yesterday = Time - CTimeSpan(1, 0, 0, 0);	// - 1 day
		Yesterday = CTime(	Yesterday.GetYear(),
							Yesterday.GetMonth(),
							Yesterday.GetDay(),
							0, 0, 0);					// Back to midnight
		if (m_SaveSnapshotSWFThread.m_ThreadExecutedForTime < Yesterday)
		{
			m_SaveSnapshotSWFThread.m_bSnapshotHistoryJpeg = m_bSnapshotHistoryJpeg;
			m_SaveSnapshotSWFThread.m_bSnapshotHistorySwfFtp = m_bSnapshotHistorySwfFtp;
			m_SaveSnapshotSWFThread.m_fSnapshotVideoCompressorQuality = m_fSnapshotVideoCompressorQuality;
			m_SaveSnapshotSWFThread.m_dSnapshotHistoryFrameRate = (double)m_nSnapshotHistoryFrameRate;
			m_SaveSnapshotSWFThread.m_Time = Yesterday;
			m_SaveSnapshotSWFThread.m_sSnapshotAutoSaveDir = m_sRecordAutoSaveDir;
			::EnterCriticalSection(&m_csSnapshotConfiguration);
			m_SaveSnapshotSWFThread.m_Config = m_SnapshotFTPUploadConfiguration;
			::LeaveCriticalSection(&m_csSnapshotConfiguration);
			m_SaveSnapshotSWFThread.Start();
		}
	}
}

BOOL CVideoDeviceDoc::EditCopy(CDib* pDib, const CTime& Time)
{
	// Get uptime
	DWORD dwUpTime = pDib->GetUpTime();

	// Decode if compressed
	CDib Dib;
	Dib.SetShowMessageBoxOnError(FALSE);
	Dib = *pDib;
	if (Dib.IsCompressed())
		Dib.Decompress(32);

	// Add frame time
	if (m_bShowFrameTime)
		AddFrameTime(&Dib, Time, dwUpTime, m_nRefFontSize);
	
	// Copy to clipboard
	Dib.EditCopy();

	// Clear flag
	m_bDoEditCopy = FALSE;

	return TRUE;
}

BOOL CVideoDeviceDoc::EditSnapshot(CDib* pDib, const CTime& Time)
{
	// Make FileName
	CString sFileName = MakeJpegManualSnapshotFileName(Time);

	// Do not overwrite existing because of the
	// below posted open document message
	if (::IsExistingFile(sFileName))
	{
		m_bDoEditSnapshot = FALSE;
		return FALSE;
	}

	// Get uptime
	DWORD dwUpTime = pDib->GetUpTime();

	// Dib
	CDib Dib(*pDib);

	// Resize Thumb
	CDib DibThumb;
	if (m_bSnapshotThumb)
	{
		// No Message Box on Error
		DibThumb.SetShowMessageBoxOnError(FALSE);

		// Resize
		if (DibThumb.AllocateBitsFast(12, FCC('I420'), m_nSnapshotThumbWidth, m_nSnapshotThumbHeight))
		{
			CVideoDeviceDoc::ResizeFast(&Dib, &DibThumb);
			DibThumb.SetUpTime(Dib.GetUpTime());
		}
	}

	// Add frame time
	if (m_bShowFrameTime)
	{
		AddFrameTime(&Dib, Time, dwUpTime, m_nRefFontSize);
		if (DibThumb.IsValid())
			AddFrameTime(&DibThumb, Time, dwUpTime, m_nRefFontSize);
	}
	
	// Copy to clipboard (not necessary but can be useful)
	Dib.EditCopy();

	// Save to JPEG File(s)
	// Note: always first save full-size file then the thumb
	// version which links to the full-size in web interface!
	CMJPEGEncoder MJPEGEncoder;
	BOOL res = CVideoDeviceDoc::SaveJpegFast(&Dib, &MJPEGEncoder, sFileName, m_nSnapshotCompressionQuality);
	if (DibThumb.IsValid())
	{
		CVideoDeviceDoc::SaveJpegFast(	&DibThumb, &MJPEGEncoder,
										::GetFileNameNoExt(sFileName) + _T("_thumb.jpg"),
										m_nSnapshotCompressionQuality);
	}

	// Open Document File
	if (res && m_bManualSnapshotAutoOpen)
	{
		::PostMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
						WM_THREADSAFE_OPEN_DOC,
						(WPARAM)(new CString(sFileName)),
						(LPARAM)NULL);
	}

	// Clear flag
	m_bDoEditSnapshot = FALSE;

	// Show OSD Message
	if (res)
	{
		if (!m_bManualSnapshotAutoOpen)
			ShowOSDMessage(ML_STRING(1849, "Snapshot Saved"), DXDRAW_MESSAGE_SUCCESS_COLOR);
	}
	else
		ShowOSDMessage(ML_STRING(1850, "Snapshot Save Failed!"), DXDRAW_MESSAGE_ERROR_COLOR);

	return res;
}

void CVideoDeviceDoc::ShowOSDMessage(const CString& sOSDMessage, COLORREF crOSDMessageColor)
{
	::EnterCriticalSection(&m_csOSDMessage);
	m_dwOSDMessageUpTime = ::timeGetTime();
	m_sOSDMessage = sOSDMessage;
	m_crOSDMessageColor = crOSDMessageColor;
	::LeaveCriticalSection(&m_csOSDMessage);
}

void CVideoDeviceDoc::OpenAVIFile(const CString& sFileName)
{
	if (m_bRecAutoOpen)
	{
		::PostMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
						WM_THREADSAFE_OPEN_DOC,
						(WPARAM)(new CString(sFileName)),
						(LPARAM)NULL);
	}
}

void CVideoDeviceDoc::CloseAndShowAviRec()
{
	LONGLONG llSamplesCount = 0;
	CString sOldRecFileName;
	if (m_pAVRec)
	{
		// Get Total Samples
		if (m_bCaptureAudio)
			llSamplesCount = m_pAVRec->GetSampleCount(m_pAVRec->AudioStreamNumToStreamNum(ACTIVE_AUDIO_STREAM));

		// Store old rec file name
		sOldRecFileName = m_pAVRec->GetFileName();
	}

	// Free
	FreeAVIFile();

	// Change Frame Rate
	double dFrameRate = 1.0;
	if (m_bCaptureAudio)
	{
		if (m_nRecordedFrames > 0	&&
			llSamplesCount > 0		&&
			m_CaptureAudioThread.m_pDstWaveFormat->nSamplesPerSec > 0)
		{
			dFrameRate =	(double)m_nRecordedFrames /
							((double)llSamplesCount / (double)m_CaptureAudioThread.m_pDstWaveFormat->nSamplesPerSec);
		}
	}
	else
	{
		if (m_nRecordedFrames > 1	&&
			m_dwRecLastUpTime - m_dwRecFirstUpTime > 0U)
		{
			dFrameRate =	(1000.0 * (m_nRecordedFrames - 1)) /
							(double)(m_dwRecLastUpTime - m_dwRecFirstUpTime);
		}
	}
	CAVIPlay::AviChangeVideoFrameRate(	(LPCTSTR)sOldRecFileName,
										0,
										dFrameRate,
										false);

	// If ending the windows session do not perform the following
	if (::AfxGetApp() && !((CUImagerApp*)::AfxGetApp())->m_bEndSession)
	{
		// Open the video file
		OpenAVIFile(sOldRecFileName);
	}
}

void CVideoDeviceDoc::NextRecTime(CTime t)
{
	int hours;
	switch (m_nTimeSegmentationIndex)
	{
		case 0 :	// 1 hour
			t = t + CTimeSpan(0, 1, 0, 0);			// + 1 hour
			m_NextRecTime = CTime(	t.GetYear(),
									t.GetMonth(),
									t.GetDay(),
									t.GetHour(),
									0, 0);// Back to 0 min and 0 sec
			break;
		case 1 :	// 2 hours
			hours = 2;
			hours -= (t.GetHour()%2);
			t = t + CTimeSpan(0, hours, 0, 0);		// + hours
			m_NextRecTime = CTime(	t.GetYear(),
									t.GetMonth(),
									t.GetDay(),
									t.GetHour(),
									0, 0);// Back to 0 min and 0 sec
			break;
		case 2 :	// 3 hours
			hours = 3;
			hours -= (t.GetHour()%3);
			t = t + CTimeSpan(0, hours, 0, 0);		// + hours
			m_NextRecTime = CTime(	t.GetYear(),
									t.GetMonth(),
									t.GetDay(),
									t.GetHour(),
									0, 0);// Back to 0 min and 0 sec
			break;
		case 3 :	// 6 hours
			hours = 6;
			hours -= (t.GetHour()%6);
			t = t + CTimeSpan(0, hours, 0, 0);		// + hours
			m_NextRecTime = CTime(	t.GetYear(),
									t.GetMonth(),
									t.GetDay(),
									t.GetHour(),
									0, 0);// Back to 0 min and 0 sec
			break;
		case 4 :	// 12 hours
			hours = 12;
			hours -= (t.GetHour()%12);
			t = t + CTimeSpan(0, hours, 0, 0);		// + hours
			m_NextRecTime = CTime(	t.GetYear(),
									t.GetMonth(),
									t.GetDay(),
									t.GetHour(),
									0, 0);// Back to 0 min and 0 sec
			break;
		case 5 :	// 24 hours
		default:
			t = t + CTimeSpan(1, 0, 0, 0);			// + 1 day
			m_NextRecTime = CTime(	t.GetYear(),
									t.GetMonth(),
									t.GetDay(),
									0, 0, 0);// Back to midnight
			break;
	}
}

BOOL CVideoDeviceDoc::NextAviFile()
{
	// Allocate & Init pNextAVRec
	CAVRec* pNextAVRec = NULL;
	if (!MakeAVRec(&pNextAVRec))
	{
		if (pNextAVRec)
			delete pNextAVRec;
		return FALSE;
	}

	// Close old file, change frame rate and open it
	if (m_pAVRec)
	{
		// Get Samples Count
		LONGLONG llSamplesCount = 0;
		if (m_bCaptureAudio)
			llSamplesCount = m_pAVRec->GetSampleCount(m_pAVRec->AudioStreamNumToStreamNum(ACTIVE_AUDIO_STREAM));

		// Store old rec file name
		CString sOldRecFileName = m_pAVRec->GetFileName();

		// Free
		delete m_pAVRec;

		// Change Frame Rate
		double dFrameRate = 1.0;
		if (m_bCaptureAudio)
		{
			if (m_nRecordedFrames > 0	&&
				llSamplesCount > 0		&&
				m_CaptureAudioThread.m_pDstWaveFormat->nSamplesPerSec > 0)
			{
				dFrameRate =	(double)m_nRecordedFrames /
								((double)llSamplesCount / (double)m_CaptureAudioThread.m_pDstWaveFormat->nSamplesPerSec);
			}
		}
		else
		{
			if (m_nRecordedFrames > 1	&&
				m_dwRecLastUpTime - m_dwRecFirstUpTime > 0U)
			{
				dFrameRate =	(1000.0 * (m_nRecordedFrames - 1)) /
								(double)(m_dwRecLastUpTime - m_dwRecFirstUpTime);
			}
		}
		CAVIPlay::AviChangeVideoFrameRate(	(LPCTSTR)sOldRecFileName,
											0,
											dFrameRate,
											false);

		// Open the video file
		OpenAVIFile(sOldRecFileName);
	}

	// Change Pointer
	m_pAVRec = pNextAVRec;

	// Restart with frame counting and time measuring
	m_bRecFirstFrame = TRUE;

	return TRUE;
}

// pDib    : the frame pointer
// width   : image width in pixels
// posX    : start pixel position in x direction
// posY    : start pixel position in y direction
// rx      : pixels count to sum in x direction
// ry      : pixels count to sum in y direction
/* Note:
When using __asm to write assembly language in C/C++ functions, you don't need to
preserve the EAX, EBX, ECX, EDX, ESI, or EDI registers. However, using these registers
will affect code quality because the register allocator cannot use them to store values
across __asm blocks. The compiler avoids enregistering variables across an __asm block
if the register's contents would be changed by the __asm block. In addition, by using
EBX, ESI or EDI in inline assembly code, you force the compiler to save and restore
those registers in the function prologue and epilogue.
*/
__forceinline int CVideoDeviceDoc::SummRectArea(CDib* pDib,
												int width,
												int posX,
												int posY,
												int rx,
												int ry)
{
	int summ = 0;
	LPBYTE data = pDib->GetBits();

	// Offset
	data += (width*posY + posX);

	if (g_bSSE)
	{
		int rx8 = rx>>3;
		int yinc = width - rx;
		__asm
		{
			pxor	mm1,	mm1			; mm1 is 0
			mov		edx,	ry
			mov		eax,	data

		OuterLoop:			
			mov		ecx,	rx8

		InnerLoop:
			movq	mm0,	[eax]
			psadbw	mm0,	mm1			; integer SSE instruction, not recognized by VC6
			movd	ebx,	mm0
			add		summ,	ebx

			add		eax,	8
			dec		ecx
			jnz		InnerLoop
		
			add		eax,	yinc
			dec		edx
			jnz		OuterLoop

			emms						; Empty MMX State
		}
	}
	else
	{
		for (int y = 0 ; y < ry ; y++ , data += width)
		{
			for (int x = 0 ; x < rx ; x++)
			{
				summ += data[x]; 
			}
		}
	}
	return summ;
}

// Movement Detector
//
// pDib				: Difference Dib from which to detect movement
// nDetectionLevel	: 1 - 100 (1 - > low movement sensibility, 100 -> high movement sensibility)
BOOL CVideoDeviceDoc::MovementDetector(CDib* pDib, int nDetectionLevel)
{
	// Check Params
	if ((pDib == NULL) || !pDib->IsValid())
		return FALSE;
	if (nDetectionLevel < 1 || nDetectionLevel > 100)
		return FALSE;

	// Vars
	double dDetectionLevel = (101 - nDetectionLevel) / 1000.0;
	int i, x, y;
	int nZoneWidth = pDib->GetWidth() / m_lMovDetXZonesCount;
	int nZoneHeight = pDib->GetHeight() / m_lMovDetYZonesCount;

	// 235 is the MAX Y value, 16 is the MIN Y value
	// -> The maximum reachable difference is 235 - 16
	int nMaxIntensityPerZone = nZoneWidth * nZoneHeight * (235 - 16);

	// Calculate the Intensities of the enabled zones
	for (y = 0 ; y < m_lMovDetYZonesCount ; y++)
	{
		for (x = 0 ; x < m_lMovDetXZonesCount ; x++)
		{
			if (m_DoMovementDetection[y*m_lMovDetXZonesCount+x])
			{
				m_MovementDetectorCurrentIntensity[y*m_lMovDetXZonesCount+x] = SummRectArea(pDib,
																							pDib->GetWidth(),
																							x*nZoneWidth,	// start pixel in x direction
																							y*nZoneHeight,	// start pixel in y direction
																							nZoneWidth,
																							nZoneHeight);
			}
		}
	}
	
	// Set Detection flag and Current Time
	BOOL bDetection = FALSE;
	int nIntensityThreshold = Round(dDetectionLevel * nMaxIntensityPerZone);
	for (i = 0 ; i < m_lMovDetTotalZones ; i++)
	{
		if (m_DoMovementDetection[i] &&
			m_MovementDetectorCurrentIntensity[i] > nIntensityThreshold * (int)m_DoMovementDetection[i])
		{
			bDetection = TRUE;
			m_MovementDetections[i] = 1;
			m_MovementDetectionsUpTime[i] = pDib->GetUpTime();
		}
	}

	// Clear Old Detection Zones
	for (i = 0 ; i < m_lMovDetTotalZones ; i++)
	{
		if (m_MovementDetections[i]	&&
			(pDib->GetUpTime() - m_MovementDetectionsUpTime[i]) >= MOVDET_TIMEOUT)
			m_MovementDetections[i] = 0;
	}

	return bDetection;
}

__forceinline int CVideoDeviceDoc::GetNewestMovementDetectionsListCount()
{
	::EnterCriticalSection(&m_csMovementDetectionsList);
	if (!m_MovementDetectionsList.IsEmpty())
	{
		CDib::LIST* pTail = m_MovementDetectionsList.GetTail();
		if (pTail)
		{
			int nCount = pTail->GetCount();
			::LeaveCriticalSection(&m_csMovementDetectionsList);
			return nCount;
		}
		else
		{
			::LeaveCriticalSection(&m_csMovementDetectionsList);
			return 0;
		}
	}
	else
	{
		::LeaveCriticalSection(&m_csMovementDetectionsList);
		return 0;
	}
}

__forceinline void CVideoDeviceDoc::ClearMovementDetectionsList()
{
	::EnterCriticalSection(&m_csMovementDetectionsList);
	while (!m_MovementDetectionsList.IsEmpty())
	{
		CDib::LIST* pFrameList = m_MovementDetectionsList.GetTail();
		ClearFrameList(pFrameList);
		delete pFrameList;
		m_MovementDetectionsList.RemoveTail();
	}
	::LeaveCriticalSection(&m_csMovementDetectionsList);
}

__forceinline void CVideoDeviceDoc::RemoveOldestMovementDetectionList()
{
	::EnterCriticalSection(&m_csMovementDetectionsList);
	if (!m_MovementDetectionsList.IsEmpty())
	{
		CDib::LIST* pFrameList = m_MovementDetectionsList.GetHead();
		ClearFrameList(pFrameList);
		delete pFrameList;
		m_MovementDetectionsList.RemoveHead();
	}
	::LeaveCriticalSection(&m_csMovementDetectionsList);
}

__forceinline void CVideoDeviceDoc::ClearNewestFrameList()
{
	::EnterCriticalSection(&m_csMovementDetectionsList);
	if (!m_MovementDetectionsList.IsEmpty())
	{
		CDib::LIST* pTail = m_MovementDetectionsList.GetTail();
		if (pTail)
			CDib::FreeList(*pTail);
	}
	::LeaveCriticalSection(&m_csMovementDetectionsList);
}

__forceinline CDib* CVideoDeviceDoc::AllocMJPGFrame(CDib* pDib, LPBYTE pMJPGData, DWORD dwMJPGSize)
{
	CDib* pNewDib = NULL;
	if (pDib)
	{
		BITMAPINFO Bmi;
		memset(&Bmi, 0, sizeof(BITMAPINFO));
		Bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		Bmi.bmiHeader.biWidth = pDib->GetWidth();
		Bmi.bmiHeader.biHeight = pDib->GetHeight();
		Bmi.bmiHeader.biPlanes = 1;
		Bmi.bmiHeader.biBitCount = 12;
		if (pMJPGData && dwMJPGSize > 0U)
		{
			Bmi.bmiHeader.biCompression = FCC('MJPG');
			Bmi.bmiHeader.biSizeImage = dwMJPGSize;
			pNewDib = new CDib;
			if (pNewDib)
			{
				pNewDib->SetShowMessageBoxOnError(FALSE);
				pNewDib->SetBMI((LPBITMAPINFO)&Bmi);						// set BMI	
				pNewDib->SetBits(pMJPGData);								// copy bits
				pNewDib->SetUpTime(pDib->GetUpTime());						// copy frame uptime
				pNewDib->SetUserFlag(pDib->GetUserFlag());					// copy motion, deinterlace and rotate by 180° flags
				pNewDib->CopyUserList(pDib->m_UserList);					// copy audio bufs if any
			}
		}
		else
		{
			DWORD dwEncodedLen = m_MJPEGDetEncoder.Encode(	MOVDET_BUFFER_COMPRESSIONQUALITY,
															pDib->GetBMI(), pDib->GetBits());
			if (dwEncodedLen > 0U)
			{
				// Contaware introduced this fourcc to distinguish the inofficial jpeg ITU601
				// color space so that the mjpeg decompressor in CSaveFrameListThread
				// is re-opened when switching between normal FCC('MJPG') and inofficial FCC('M601')
				Bmi.bmiHeader.biCompression = FCC('M601');
				Bmi.bmiHeader.biSizeImage = dwEncodedLen;
				pNewDib = new CDib;
				if (pNewDib)
				{
					pNewDib->SetShowMessageBoxOnError(FALSE);
					pNewDib->SetBMI((LPBITMAPINFO)&Bmi);						// set BMI	
					pNewDib->SetBits((LPBYTE)m_MJPEGDetEncoder.GetEncodedBuf());// copy bits
					pNewDib->SetUpTime(pDib->GetUpTime());						// copy frame uptime
					pNewDib->SetUserFlag(pDib->GetUserFlag());					// copy motion, deinterlace and rotate by 180° flags
					pNewDib->CopyUserList(pDib->m_UserList);					// copy audio bufs if any
				}
			}
		}
	}
	return pNewDib;
}

__forceinline void CVideoDeviceDoc::AddNewFrameToNewestList(CDib* pDib, LPBYTE pMJPGData, DWORD dwMJPGSize)
{
	if (pDib)
	{
		::EnterCriticalSection(&m_csMovementDetectionsList);
		if (!m_MovementDetectionsList.IsEmpty())
		{
			CDib::LIST* pTail = m_MovementDetectionsList.GetTail();
			if (pTail)
			{
				// Check whether video size changed
				if (!pTail->IsEmpty())
				{
					CDib* pHeadDib = pTail->GetHead();
					if (pHeadDib &&
						(pDib->GetWidth() != pHeadDib->GetWidth() ||
						pDib->GetHeight() != pHeadDib->GetHeight()))
						CDib::FreeList(*pTail);
				}

				// Add the new frame
				CDib* pNewDib = AllocMJPGFrame(pDib, pMJPGData, dwMJPGSize);
				if (pNewDib)
					pTail->AddTail(pNewDib);
			}
		}
		::LeaveCriticalSection(&m_csMovementDetectionsList);
	}
}

__forceinline void CVideoDeviceDoc::AddNewFrameToNewestListAndShrink(CDib* pDib, LPBYTE pMJPGData, DWORD dwMJPGSize)
{
	if (pDib)
	{
		::EnterCriticalSection(&m_csMovementDetectionsList);
		if (!m_MovementDetectionsList.IsEmpty())
		{
			CDib::LIST* pTail = m_MovementDetectionsList.GetTail();
			if (pTail)
			{
				// Check whether video size changed
				if (!pTail->IsEmpty())
				{
					CDib* pHeadDib = pTail->GetHead();
					if (pHeadDib &&
						(pDib->GetWidth() != pHeadDib->GetWidth() ||
						pDib->GetHeight() != pHeadDib->GetHeight()))
						CDib::FreeList(*pTail);
				}

				// Alloc MJPG frame
				CDib* pNewDib = AllocMJPGFrame(pDib, pMJPGData, dwMJPGSize);
				if (pNewDib)
				{
					// Add the new frame
					pTail->AddTail(pNewDib);

					// Shrink to a size of m_nMilliSecondsRecBeforeMovementBegin
					while (pTail->GetCount() > 1)
					{
						CDib* pHeadDib = pTail->GetHead();
						if (pHeadDib)
						{
							if ((pNewDib->GetUpTime() - pHeadDib->GetUpTime()) <= (DWORD)m_nMilliSecondsRecBeforeMovementBegin)
								break;
							delete pHeadDib;
						}
						pTail->RemoveHead();
					}
				}
			}
		}
		::LeaveCriticalSection(&m_csMovementDetectionsList);
	}
}

__forceinline void CVideoDeviceDoc::ShrinkNewestFrameList()
{
	::EnterCriticalSection(&m_csMovementDetectionsList);
	if (!m_MovementDetectionsList.IsEmpty())
	{
		CDib::LIST* pTail = m_MovementDetectionsList.GetTail();
		if (pTail && !pTail->IsEmpty())
		{
			// Get Tail Dib
			CDib* pTailDib = pTail->GetTail();
			if (pTailDib)
			{
				// Shrink to a size of m_nMilliSecondsRecBeforeMovementBegin
				while (pTail->GetCount() > 1)
				{
					CDib* pHeadDib = pTail->GetHead();
					if (pHeadDib)
					{
						if ((pTailDib->GetUpTime() - pHeadDib->GetUpTime()) <= (DWORD)m_nMilliSecondsRecBeforeMovementBegin)
							break;
						delete pHeadDib;
					}
					pTail->RemoveHead();
				}
			}
		}
	}
	::LeaveCriticalSection(&m_csMovementDetectionsList);
}

__forceinline void CVideoDeviceDoc::ShrinkNewestFrameListBy(int nSize, DWORD& dwFirstUpTime, DWORD& dwLastUpTime)
{
	dwFirstUpTime = dwLastUpTime = 0U;
	::EnterCriticalSection(&m_csMovementDetectionsList);
	if (!m_MovementDetectionsList.IsEmpty())
	{
		CDib::LIST* pTail = m_MovementDetectionsList.GetTail();
		if (pTail)
		{
			CDib* pHeadDib;
			if (!pTail->IsEmpty())
			{
				pHeadDib = pTail->GetHead();
				if (pHeadDib)
					dwFirstUpTime = pHeadDib->GetUpTime();
			}
			while (!pTail->IsEmpty() && nSize > 0)
			{
				pHeadDib = pTail->GetHead();
				if (pHeadDib)
				{
					dwLastUpTime = pHeadDib->GetUpTime();
					delete pHeadDib;
				}
				pTail->RemoveHead();
				--nSize;
			}
		}
	}
	::LeaveCriticalSection(&m_csMovementDetectionsList);
}

__forceinline void CVideoDeviceDoc::SaveFrameList(BOOL bDetectionSequenceDone)
{
	::EnterCriticalSection(&m_csMovementDetectionsList);
	if (bDetectionSequenceDone && !m_MovementDetectionsList.IsEmpty())
	{
		CDib::LIST* pTail = m_MovementDetectionsList.GetTail();
		if (pTail && !pTail->IsEmpty())
		{
			CDib* pTailDib = pTail->GetTail();
			if (pTailDib)
			{
				// Mark the frame as being the last frame of the detection sequence
				pTailDib->SetUserFlag(pTailDib->GetUserFlag() | FRAME_USER_FLAG_LAST);
			}
		}
	}
	CDib::LIST* pNewList = new CDib::LIST;
	if (pNewList)
		m_MovementDetectionsList.AddTail(pNewList);
	::LeaveCriticalSection(&m_csMovementDetectionsList);
}

void CVideoDeviceDoc::OnViewFrametime() 
{
	m_bShowFrameTime = !m_bShowFrameTime;
}

void CVideoDeviceDoc::OnUpdateViewFrametime(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bShowFrameTime ? 1 : 0);	
}

void CVideoDeviceDoc::OnViewDetections() 
{
	m_bShowMovementDetections = !m_bShowMovementDetections;
}

void CVideoDeviceDoc::OnUpdateViewDetections(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bShowMovementDetections ? 1 : 0);	
}

void CVideoDeviceDoc::ShowDetectionZones()
{
	if (!m_bShowEditDetectionZones)
	{
		m_bShowEditDetectionZones = TRUE;
		GetView()->ForceCursor();
		::AfxGetMainFrame()->StatusText(ML_STRING(1483, "*** Click Inside The Capture Window to Add Zones. Press Ctrl (or Shift) to Remove Them ***"));
	}
}

void CVideoDeviceDoc::HideDetectionZones(BOOL bSaveSettingsOnHiding)
{
	if (m_bShowEditDetectionZones)
	{
		m_bShowEditDetectionZones = FALSE;
		GetView()->ForceCursor(FALSE);
		m_bShowEditDetectionZonesMinus = FALSE;
		::AfxGetMainFrame()->StatusText();
		if (bSaveSettingsOnHiding)
		{
			BeginWaitCursor();
			SaveZonesSettings();
			EndWaitCursor();
		}
	}
}

void CVideoDeviceDoc::OnViewDetectionZones()
{
	if (m_bShowEditDetectionZones)
		HideDetectionZones(TRUE);
	else
		ShowDetectionZones();
}

void CVideoDeviceDoc::OnUpdateViewDetectionZones(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bShowEditDetectionZones ? 1 : 0);
}

void CVideoDeviceDoc::OnViewFit() 
{
	if (GetFrame()->IsZoomed())
	{
		// Restore mainframe if maximized
		if (::AfxGetMainFrame()->IsZoomed())
			::AfxGetMainFrame()->ShowWindow(SW_RESTORE);

		// First resize in x direction,
		// because the menu may get taller,
		// then resize in y direction!
		CRect rcClient;
		GetView()->GetClientRect(&rcClient);
		int delta = m_DocRect.Width() - rcClient.Width();
		CRect rcMainWnd;
		::AfxGetMainFrame()->GetWindowRect(&rcMainWnd);
		::AfxGetMainFrame()->SetWindowPos(	NULL, 0, 0,
											rcMainWnd.Width() + delta,
											rcMainWnd.Height(),
											SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOMOVE);
		GetView()->GetClientRect(&rcClient);
		::AfxGetMainFrame()->GetWindowRect(&rcMainWnd);
		delta = m_DocRect.Height() - rcClient.Height();
		::AfxGetMainFrame()->SetWindowPos(	NULL, 0, 0,
											rcMainWnd.Width(),
											rcMainWnd.Height() + delta,
											SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOMOVE);

	}
	else
		GetView()->UpdateWindowSizes(FALSE, FALSE, TRUE);
}

void CVideoDeviceDoc::OnUpdateViewFit(CCmdUI* pCmdUI) 
{
	CRect rcClient;
	GetView()->GetClientRect(&rcClient);
	pCmdUI->Enable(	!GetView()->m_bFullScreenMode	&&
					(rcClient != m_DocRect)			&&
					!GetFrame()->IsIconic());
}

void CVideoDeviceDoc::OnEditDelete() 
{
	::AfxMessageBox(ML_STRING(1872, "Try again after closing all devices"), MB_OK | MB_ICONERROR);
}

void CVideoDeviceDoc::OnEditCopy() 
{
	// Copy Done in ProcessI420Frame()
	m_bDoEditCopy = TRUE;
}

void CVideoDeviceDoc::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	if (m_bDoEditCopy)
	{
		pCmdUI->Enable(FALSE);
		pCmdUI->SetText(ML_STRING(1484, "&Copying...\tCtrl+C"));
	}
	else
	{
		pCmdUI->Enable(TRUE);
		pCmdUI->SetText(ML_STRING(1485, "&Copy\tCtrl+C"));
	}
}

void CVideoDeviceDoc::OnEditSnapshot() 
{
	// Snapshot Done in ProcessI420Frame()
	m_bDoEditSnapshot = TRUE;
}

void CVideoDeviceDoc::OnUpdateEditSnapshot(CCmdUI* pCmdUI) 
{
	if (m_bDoEditSnapshot)
	{
		pCmdUI->Enable(FALSE);
		pCmdUI->SetText(ML_STRING(1758, "&Snapshooting...\tIns"));
	}
	else
	{
		pCmdUI->Enable(TRUE);
		pCmdUI->SetText(ML_STRING(1759, "&Snapshot\tIns"));
	}
}

void CVideoDeviceDoc::OnFileClose() 
{
	CloseDocument();
}

void CVideoDeviceDoc::OnFileSave() 
{
	
}

void CVideoDeviceDoc::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(FALSE);	
}

void CVideoDeviceDoc::OnFileSaveAs() 
{

}

void CVideoDeviceDoc::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(FALSE);
}

/*
AXIS
----

JPEG
_T("GET /axis-cgi/jpg/image.cgi?resolution=320x240&compression=60 HTTP/1.1\r\n")

MJPEG
_T("GET /axis-cgi/mjpg/video.cgi?resolution=320x240&compression=60&fps=1 HTTP/1.1\r\n")

CONFIG  
_T("GET /axis-cgi/view/param.cgi?action=list&group=Properties.Image HTTP/1.1\r\n")            -> get all image properties
_T("GET /axis-cgi/view/param.cgi?action=list&group=Properties.Image.Rotation HTTP/1.1\r\n")   -> root.Properties.Image.Rotation=0,180
_T("GET /axis-cgi/view/param.cgi?action=list&group=Properties.Image.Format HTTP/1.1\r\n")     -> root.Properties.Image.Format=jpeg,mjpeg
_T("GET /axis-cgi/view/param.cgi?action=list&group=Properties.Image.Resolution HTTP/1.1\r\n") -> root.Properties.Image.Resolution=640x480,640x360,320x240,160x120


PANASONIC
---------

JPEG
_T("GET /SnapshotJPEG[?Resolution=<resolution>][&Quality=<quality>] HTTP/1.1\r\n")
_T("GET /SnapshotJPEG?Resolution=320x240&Quality=Standard HTTP/1.1\r\n")

MJPEG
_T("GET /nphMotionJpeg[?Resolution=<resolution>][&Quality=<quality>] HTTP/1.1\r\n")
_T("GET /nphMotionJpeg?Resolution=320x240&Quality=Standard HTTP/1.1\r\n")

Resolutions:
192x144
160x120
320x240
640x480
1280x1024

Qualities:
Motion   -> Higher compression rate, more frames, lower image clarity.
Standard -> Standard quality.
Clarity  -> Lower compression rate, fewer frames, better image clarity.


PIXORD/NETCOMM
--------------

JPEG
_T("GET /images<channel><resolution> HTTP/1.1\r\n")
_T("GET /images1sif HTTP/1.1\r\n")

MJPEG
_T("GET /getimage?camera=<channel>[&fmt=<resolution>][&delay=<delay>] HTTP/1.1\r\n")
_T("GET /getimage?camera=1&fmt=sif&delay=10 HTTP/1.1\r\n")

qsif     -> 176x112 (NTSC)
sif      -> 352x240 (NTSC)
full     -> 704x480 (NTSC)
delay=10 -> 100 ms


EDIMAX
------

JPEG
_T("GET /jpg/image.jpg HTTP/1.1\r\n")

MJPEG
_T("GET /mjpg/video.mjpg HTTP/1.1\r\n")

CONFIG  
_T("GET /camera-cgi/admin/param.cgi?action=list&group=Properties.Image.I0 HTTP/1.1\r\n")                          -> get all image properties (returns for example: root.Properties.Image.I0.Resolution=1280x1024,640x480,320x240)
_T("GET /camera-cgi/admin/param.cgi?action=update&Image.I0.Appearance.Resolution=WidthxHeight HTTP/1.1\r\n")      -> set resolution
_T("GET /camera-cgi/admin/param.cgi?action=update&Image.I0.Appearance.Compression=CompressionLevel HTTP/1.1\r\n") -> set compression, valid range: 0-4
_T("GET /camera-cgi/admin/param.cgi?action=update&Image.I0.Stream.FPS=Fps HTTP/1.1\r\n")                          -> set framerate, valid values: 1, 3, 5, 10, 15, 30


TP-LINK
-------

JPEG
_T("GET /jpg/image.jpg HTTP/1.1\r\n")

MJPEG
_T("GET /video.mjpg HTTP/1.1\r\n")

CONFIG only for MJPEG and quite crappy: after a config change had to wait many seconds before re-connecting
to the stream and even so not always applied -> let user config it on the web interface
_T("GET /cgi-bin/view/param?action=list&group=Properties.Image.Resolution HTTP/1.1\r\n")                   -> get all resolutions (returns for example: root.Properties.Image.Resolution=176x120,352x240,720x480)
_T("GET /cgi-bin/admin/param?action=update&Image.I0.MJPEG.Resolution=WidthxHeight HTTP/1.1\r\n")           -> set resolution
_T("GET /cgi-bin/admin/param?action=update&Image.I0.Appearance.Compression=CompressionLevel HTTP/1.1\r\n") -> set compression, valid range: 0-100 (0: for auto-rate control)
_T("GET /cgi-bin/admin/param?action=update&Image.I0.MJPEG.FPS=Fps HTTP/1.1\r\n")                           -> set framerate, valid range: 1-30


FOSCAM/TENVIS/CLONES
--------------------

JPEG
_T("GET /snapshot.cgi?user=<user>&pwd=<password>&resolution=<resolution> HTTP/1.1\r\n")

MJPEG
_T("GET /videostream.cgi?user=<user>&pwd=<password>&resolution=<resolution>&rate=<rate> HTTP/1.1\r\n")

resolutions:
8  -> 320*240
32 -> 640*480

rates (value rang 0-23):
0  -> full speed
1  -> 20 fps
3  -> 15 fps
6  -> 10 fps
11 -> 5 fps
12 -> 4 fps
13 -> 3 fps
14 -> 2 fps
15 -> 1 fps
17 -> 0.5 fps
19 -> 0.333333 fps
21 -> 0.25 fps
23 -> 0.2 fps
*/
BOOL CVideoDeviceDoc::ConnectGetFrame(DWORD dwConnectDelay/*=0U*/)
{
	// Check
	if (!m_pGetFrameNetCom || m_sGetFrameVideoHost == _T(""))
		return FALSE;
	
	// Init
	m_pGetFrameNetCom->Close();
	m_HttpGetFrameThread.Kill();
	if (m_pHttpGetFrameParseProcess)
		m_pHttpGetFrameParseProcess->Close();
	else
		m_pHttpGetFrameParseProcess = (CHttpGetFrameParseProcess*)new CHttpGetFrameParseProcess(this);
	if (!m_pHttpGetFrameParseProcess)
		return FALSE;
	m_pHttpGetFrameParseProcess->m_bTryConnecting = TRUE;
	switch (m_nNetworkDeviceTypeMode)
	{
		case OTHERONE_SP :	// Other HTTP device (mjpeg)
		case OTHERONE_CP :	// Other HTTP device (jpegs)
			// Format not yet known because there are ambivalent connection strings in m_HttpGetFrameLocations
			m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATUNKNOWN;
			m_nHttpGetFrameLocationPos = 0;
			break;

		case AXIS_SP :		// Axis Server Push (mjpeg)
			m_pHttpGetFrameParseProcess->m_bQueryProperties = TRUE;
			m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATMJPEG;
			break;

		case AXIS_CP :		// Axis Client Poll (jpegs)
			m_pHttpGetFrameParseProcess->m_bQueryProperties = TRUE;
			m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATJPEG;
			break;

		case PANASONIC_SP :	// Panasonic Server Push (mjpeg)
			m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATMJPEG;
			m_pHttpGetFrameParseProcess->m_Sizes.RemoveAll();			
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(160, 120));
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(192, 144));
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(320, 240));
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(640, 480));
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(1280, 1024)); // Support models such as BB-HCM515
			break;

		case PANASONIC_CP :	// Panasonic Client Poll (jpegs)
			m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATJPEG;
			m_pHttpGetFrameParseProcess->m_Sizes.RemoveAll();			
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(160, 120));
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(192, 144));
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(320, 240));
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(640, 480));
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(1280, 1024)); // Support models such as BB-HCM515
			break;

		case PIXORD_SP :	// Pixord Server Push (mjpeg)
			m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATMJPEG;
			m_pHttpGetFrameParseProcess->m_Sizes.RemoveAll();			
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(176, 112));
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(352, 240));
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(704, 480));
			break;

		case PIXORD_CP :	// Pixord Client Poll (jpegs)
			m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATJPEG;
			m_pHttpGetFrameParseProcess->m_Sizes.RemoveAll();			
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(176, 112));
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(352, 240));
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(704, 480));
			break;

		case EDIMAX_SP :	// Edimax Server Push (mjpeg)
			m_pHttpGetFrameParseProcess->m_bQueryProperties = TRUE;
			m_pHttpGetFrameParseProcess->m_bSetResolution = TRUE;
			m_pHttpGetFrameParseProcess->m_bSetCompression = TRUE;
			m_pHttpGetFrameParseProcess->m_bSetFramerate = TRUE;
			m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATMJPEG;
			break;

		case EDIMAX_CP :	// Edimax Client Poll (jpegs)
			m_pHttpGetFrameParseProcess->m_bQueryProperties = TRUE;
			m_pHttpGetFrameParseProcess->m_bSetResolution = TRUE;
			m_pHttpGetFrameParseProcess->m_bSetCompression = TRUE;
			m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATJPEG;
			break;

		case TPLINK_SP :	// TP-Link Server Push (mjpeg)
			m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATMJPEG;
			break;

		case TPLINK_CP :	// TP-Link Client Poll (jpegs)
			m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATJPEG;
			break;

		case FOSCAM_SP :	// Foscam Server Push (mjpeg)
			m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATMJPEG;
			m_pHttpGetFrameParseProcess->m_Sizes.RemoveAll();			
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(320, 240));
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(640, 480));
			break;

		case FOSCAM_CP :	// Foscam Client Poll (jpegs)
			m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATJPEG;
			m_pHttpGetFrameParseProcess->m_Sizes.RemoveAll();			
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(320, 240));
			m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(640, 480));
			break;

		default :
			ASSERT(FALSE);
			return FALSE;
	}

	// Start thread and connect
	m_HttpGetFrameThread.Start();
	return m_HttpGetFrameThread.SetEventConnect(_T(""), dwConnectDelay);
}

BOOL CVideoDeviceDoc::CHttpGetFrameParseProcess::SendRawRequest(CString sRequest)
{
	// Store last request
	m_sLastRequest = sRequest;

	// Url encode uri inside request and insert eventual credentials
	CString sMethod;
	CString sUri;
	int nPos, nPosEnd;
	if ((nPos = sRequest.Find(_T(' '))) >= 0)
	{
		sMethod = sRequest.Left(nPos);
		nPos++; // skip space
		if ((nPosEnd = sRequest.ReverseFind(_T(' '))) >= 0)
		{
			// Get Uri
			sUri = sRequest.Mid(nPos, nPosEnd - nPos);

			// Make sure it is not already url encoded
			// (this happens when copying from IE address bar and pasting into ContaCam)
			sUri = ::UrlDecode(sUri);

			// Do not encode reserved chars like '?' or '&' or '=' used by uri parameters
			// or '[' and ']' found in HTTPGETFRAME_USERNAME_PLACEHOLDER or
			// HTTPGETFRAME_PASSWORD_PLACEHOLDER and replaced below
			sUri = ::UrlEncode(sUri, FALSE);

			// Replace uri parameters placeholders with fully url encoded username and password
			sUri.Replace(HTTPGETFRAME_USERNAME_PLACEHOLDER, ::UrlEncode(m_pDoc->m_sHttpGetFrameUsername, TRUE));
			sUri.Replace(HTTPGETFRAME_PASSWORD_PLACEHOLDER, ::UrlEncode(m_pDoc->m_sHttpGetFramePassword, TRUE));

			// Make request
			sRequest = sMethod + _T(" ") + sUri + sRequest.Mid(nPosEnd);
		}
	}

	// Debug
#ifdef _DEBUG
	CString sTrace(sRequest);
	sTrace.Replace(_T("%"), _T("%%"));
	TRACE(sTrace);
#endif

	// Host
	CString sHost;
	if (m_pDoc->m_nGetFrameVideoPort != 80)
		sHost.Format(_T("Host: %s:%d\r\n"), m_pDoc->m_sGetFrameVideoHost, m_pDoc->m_nGetFrameVideoPort);
	else
		sHost.Format(_T("Host: %s\r\n"), m_pDoc->m_sGetFrameVideoHost);

	CString sMsg;
	if (m_AnswerAuthorizationType == AUTHBASIC) // http://tools.ietf.org/html/rfc2617
	{
		m_LastRequestAuthorizationType = AUTHBASIC;
		USES_CONVERSION;
		CBase64 base64;
		CString sUserNamePassword;
		sUserNamePassword.Format(_T("%s:%s"), m_pDoc->m_sHttpGetFrameUsername, m_pDoc->m_sHttpGetFramePassword);
		char* pszUserNamePassword = T2A(const_cast<LPTSTR>(sUserNamePassword.operator LPCTSTR()));
		int nUserNamePasswordLength = (int) strlen(pszUserNamePassword);
		int nEncodedLength = base64.EncodeGetRequiredLength(nUserNamePasswordLength);
		LPSTR pszEncoded = (LPSTR)_alloca(nEncodedLength + 1); // Get space from stack, no need to free, limited to 500KB
		base64.Encode((const BYTE*)pszUserNamePassword, nUserNamePasswordLength, pszEncoded, &nEncodedLength);
		pszEncoded[nEncodedLength] = '\0';
		LPCTSTR lpszAuthorization = A2T(pszEncoded);

		// Keep it short because some stupid ip cams (like Planet)
		// run out of buffer or do not parse well if we send to much!
		sMsg.Format(_T("User-Agent: %s/%s\r\n")
					_T("Connection: keep-alive\r\n")
					_T("Authorization: Basic %s\r\n\r\n"),
					APPNAME_NOEXT,
					APPVERSION,
					lpszAuthorization);
	}
	else if (m_AnswerAuthorizationType == AUTHDIGEST) // http://tools.ietf.org/html/rfc2617
	{
		m_AnswerAuthorizationType = AUTHNONE; // reset it!
		m_LastRequestAuthorizationType = AUTHDIGEST;
		BOOL bQop = FALSE;
		CString sQopLowerCase = m_sQop;
		sQopLowerCase.MakeLower();
		if (sQopLowerCase.Find(_T("auth")) >= 0)
			bQop = TRUE;
		USES_CONVERSION;
		CString sHA1;
		CString sHA2;
		CString sHResponse;
		CString sToHash;
		CPJNMD5 hmac;
		CPJNMD5Hash hash;

		// HA1
		sToHash =	m_pDoc->m_sHttpGetFrameUsername + _T(":")	+
					m_sRealm + _T(":")							+
					m_pDoc->m_sHttpGetFramePassword;
		char* pszA1 = T2A(const_cast<LPTSTR>(sToHash.operator LPCTSTR()));
		if (hmac.Hash((const BYTE*)pszA1, (DWORD)strlen(pszA1), hash))
			sHA1 = hash.Format(FALSE);

		// HA2
		sToHash = sMethod + _T(":") + sUri;
		char* pszA2 = T2A(const_cast<LPTSTR>(sToHash.operator LPCTSTR()));
		if (hmac.Hash((const BYTE*)pszA2, (DWORD)strlen(pszA2), hash))
			sHA2 = hash.Format(FALSE);

		// With Qop, nc and cnonce
		if (bQop)
		{
			m_dwCNonceCount++;
			CString sCNonceCount;
			sCNonceCount.Format(_T("%08x"), m_dwCNonceCount);
			srand(::makeseed(::timeGetTime(), ::GetCurrentProcessId(), ::GetCurrentThreadId())); // Seed
			DWORD dwCNonce = (DWORD)irand(4294967296.0); // returns a random value in the range [0,0xFFFFFFFF]
			CString sCNonce;
			sCNonce.Format(_T("%08x"), dwCNonce);
			sToHash =	sHA1 + _T(":")			+
						m_sNonce + _T(":")		+
						sCNonceCount + _T(":")	+
						sCNonce + _T(":")		+
						_T("auth") + _T(":")	+
						sHA2;
			char* pszResponse = T2A(const_cast<LPTSTR>(sToHash.operator LPCTSTR()));
			if (hmac.Hash((const BYTE*)pszResponse, (DWORD)strlen(pszResponse), hash))
				sHResponse = hash.Format(FALSE);

			// Keep it short because some stupid ip cams (like Planet)
			// run out of buffer or do not parse well if we send to much!
			sMsg.Format(_T("User-Agent: %s/%s\r\n")
						_T("Connection: keep-alive\r\n")
						_T("Authorization: Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",qop=auth,nc=%s,cnonce=\"%s\",response=\"%s\""),
						APPNAME_NOEXT,
						APPVERSION,
						m_pDoc->m_sHttpGetFrameUsername,
						m_sRealm,
						m_sNonce,
						sUri,
						sCNonceCount,
						sCNonce,
						sHResponse);
		}
		else
		{
			sToHash =	sHA1 + _T(":")		+
						m_sNonce + _T(":")	+
						sHA2;
			char* pszResponse = T2A(const_cast<LPTSTR>(sToHash.operator LPCTSTR()));
			if (hmac.Hash((const BYTE*)pszResponse, (DWORD)strlen(pszResponse), hash))
				sHResponse = hash.Format(FALSE);

			// Keep it short because some stupid ip cams (like Planet)
			// run out of buffer or do not parse well if we send to much!
			sMsg.Format(_T("User-Agent: %s/%s\r\n")
						_T("Connection: keep-alive\r\n")
						_T("Authorization: Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\""),
						APPNAME_NOEXT,
						APPVERSION,
						m_pDoc->m_sHttpGetFrameUsername,
						m_sRealm,
						m_sNonce,
						sUri,
						sHResponse);
		}

		// Supply Algorithm?
		if (!m_sAlgorithm.IsEmpty())
			sMsg += _T(",algorithm=") + m_sAlgorithm;
		
		// Supply Opaque?
		if (!m_sOpaque.IsEmpty())
			sMsg += _T(",opaque=\"") + m_sOpaque + _T("\"");

		// Ending
		sMsg += _T("\r\n\r\n");
	}
	else
	{
		m_LastRequestAuthorizationType = AUTHNONE;
		// Keep it short because some stupid ip cams (like Planet)
		// run out of buffer or do not parse well if we send to much!
		sMsg.Format(_T("User-Agent: %s/%s\r\n")
					_T("Connection: keep-alive\r\n\r\n"),
					APPNAME_NOEXT,
					APPVERSION);
	}

	// Send
	return (m_pNetCom->WriteStr(sRequest + sHost + sMsg) > 0);
}

BOOL CVideoDeviceDoc::CHttpGetFrameParseProcess::SendRequest()
{
	CString sLocation;
	CString sRequest;

	::EnterCriticalSection(&m_pDoc->m_csHttpParams);

	switch (m_pDoc->m_nNetworkDeviceTypeMode)
	{
		case OTHERONE_SP :	// Other HTTP device (mjpeg)
		case OTHERONE_CP :	// Other HTTP device (jpegs)
		{
			if (m_pDoc->m_nHttpGetFrameLocationPos < m_pDoc->m_HttpGetFrameLocations.GetSize())
				sLocation = m_pDoc->m_HttpGetFrameLocations[m_pDoc->m_nHttpGetFrameLocationPos];
			else
				sLocation = m_pDoc->m_HttpGetFrameLocations[0];
			sRequest.Format(_T("GET %s HTTP/%s\r\n"),
							sLocation,
							m_bOldVersion ? _T("1.0") : _T("1.1"));
			break;
		}
		case AXIS_SP :		// Axis Server Push (mjpeg)
		{
			if (m_bQueryProperties)
			{
				sLocation = _T("/axis-cgi/view/param.cgi?action=list&group=Properties.Image");
				sRequest.Format(_T("GET %s HTTP/%s\r\n"),
								sLocation,
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			else
			{
				int nFrameRate = Round(m_pDoc->m_dFrameRate);
				if (nFrameRate <= 0)
					nFrameRate = 1;
				sLocation = _T("/axis-cgi/mjpg/video.cgi");
				if (HasResolution(CSize(m_pDoc->m_nHttpVideoSizeX, m_pDoc->m_nHttpVideoSizeY)))
				{
					sRequest.Format(_T("GET %s?clock=0&date=0&resolution=%dx%d&compression=%d&fps=%d HTTP/%s\r\n"),
							sLocation,
							m_pDoc->m_nHttpVideoSizeX,
							m_pDoc->m_nHttpVideoSizeY,
							m_pDoc->m_nHttpVideoQuality,
							nFrameRate,
							m_bOldVersion ? _T("1.0") : _T("1.1"));
				}
				else if (m_Sizes.GetSize() > 0)
				{
					sRequest.Format(_T("GET %s?clock=0&date=0&resolution=%dx%d&compression=%d&fps=%d HTTP/%s\r\n"),
							sLocation,
							m_Sizes[0].cx,
							m_Sizes[0].cy,
							m_pDoc->m_nHttpVideoQuality,
							nFrameRate,
							m_bOldVersion ? _T("1.0") : _T("1.1"));
					m_pDoc->m_nHttpVideoSizeX = m_Sizes[0].cx;
					m_pDoc->m_nHttpVideoSizeY = m_Sizes[0].cy;
				}
				else
				{
					sRequest.Format(_T("GET %s?clock=0&date=0&compression=%d&fps=%d HTTP/%s\r\n"),
							sLocation,
							m_pDoc->m_nHttpVideoQuality,
							nFrameRate,
							m_bOldVersion ? _T("1.0") : _T("1.1"));
				}
			}
			break;
		}
		case AXIS_CP :		// Axis Client Poll (jpegs)
		{
			if (m_bQueryProperties)
			{
				sLocation = _T("/axis-cgi/view/param.cgi?action=list&group=Properties.Image");
				sRequest.Format(_T("GET %s HTTP/%s\r\n"),
								sLocation,
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			else
			{
				sLocation = _T("/axis-cgi/jpg/image.cgi");
				if (HasResolution(CSize(m_pDoc->m_nHttpVideoSizeX, m_pDoc->m_nHttpVideoSizeY)))
				{
					sRequest.Format(_T("GET %s?clock=0&date=0&resolution=%dx%d&compression=%d HTTP/%s\r\n"),
									sLocation,
									m_pDoc->m_nHttpVideoSizeX,
									m_pDoc->m_nHttpVideoSizeY,
									m_pDoc->m_nHttpVideoQuality,
									m_bOldVersion ? _T("1.0") : _T("1.1"));		
				}
				else if (m_Sizes.GetSize() > 0)
				{
					sRequest.Format(_T("GET %s?clock=0&date=0&resolution=%dx%d&compression=%d HTTP/%s\r\n"),
									sLocation,
									m_Sizes[0].cx,
									m_Sizes[0].cy,
									m_pDoc->m_nHttpVideoQuality,
									m_bOldVersion ? _T("1.0") : _T("1.1"));
					m_pDoc->m_nHttpVideoSizeX = m_Sizes[0].cx;
					m_pDoc->m_nHttpVideoSizeY = m_Sizes[0].cy;
				}
				else
				{
					sRequest.Format(_T("GET %s?clock=0&date=0&compression=%d HTTP/%s\r\n"),
									sLocation,
									m_pDoc->m_nHttpVideoQuality,
									m_bOldVersion ? _T("1.0") : _T("1.1"));
				}
			}
			break;
		}
		case PANASONIC_SP :	// Panasonic Server Push (mjpeg)
		{
			sLocation = _T("/nphMotionJpeg");
			if (HasResolution(CSize(m_pDoc->m_nHttpVideoSizeX, m_pDoc->m_nHttpVideoSizeY)))
			{
				sRequest.Format(_T("GET %s?Resolution=%dx%d&Quality=%s HTTP/%s\r\n"),
								sLocation,
								m_pDoc->m_nHttpVideoSizeX,
								m_pDoc->m_nHttpVideoSizeY,
								m_pDoc->m_nHttpVideoQuality == 0 ?
								_T("Clarity") :
								(m_pDoc->m_nHttpVideoQuality == 100 ?
								_T("Motion") :
								_T("Standard")),
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			else if (m_Sizes.GetSize() > 0)
			{
				sRequest.Format(_T("GET %s?Resolution=%dx%d&Quality=%s HTTP/%s\r\n"),
								sLocation,
								m_Sizes[0].cx,
								m_Sizes[0].cy,
								m_pDoc->m_nHttpVideoQuality == 0 ?
								_T("Clarity") :
								(m_pDoc->m_nHttpVideoQuality == 100 ?
								_T("Motion") :
								_T("Standard")),
								m_bOldVersion ? _T("1.0") : _T("1.1"));
				m_pDoc->m_nHttpVideoSizeX = m_Sizes[0].cx;
				m_pDoc->m_nHttpVideoSizeY = m_Sizes[0].cy;
			}
			else
			{
				sRequest.Format(_T("GET %s?Quality=%s HTTP/%s\r\n"),
								sLocation,
								m_pDoc->m_nHttpVideoQuality == 0 ?
								_T("Clarity") :
								(m_pDoc->m_nHttpVideoQuality == 100 ?
								_T("Motion") :
								_T("Standard")),
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			break;
		}
		case PANASONIC_CP :	// Panasonic Client Poll (jpegs)
		{
			sLocation = _T("/SnapshotJPEG");
			if (HasResolution(CSize(m_pDoc->m_nHttpVideoSizeX, m_pDoc->m_nHttpVideoSizeY)))
			{
				sRequest.Format(_T("GET %s?Resolution=%dx%d&Quality=%s HTTP/%s\r\n"),
								sLocation,
								m_pDoc->m_nHttpVideoSizeX,
								m_pDoc->m_nHttpVideoSizeY,
								m_pDoc->m_nHttpVideoQuality == 0 ?
								_T("Clarity") :
								(m_pDoc->m_nHttpVideoQuality == 100 ?
								_T("Motion") :
								_T("Standard")),
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			else if (m_Sizes.GetSize() > 0)
			{
				sRequest.Format(_T("GET %s?Resolution=%dx%d&Quality=%s HTTP/%s\r\n"),
								sLocation,
								m_Sizes[0].cx,
								m_Sizes[0].cy,
								m_pDoc->m_nHttpVideoQuality == 0 ?
								_T("Clarity") :
								(m_pDoc->m_nHttpVideoQuality == 100 ?
								_T("Motion") :
								_T("Standard")),
								m_bOldVersion ? _T("1.0") : _T("1.1"));
				m_pDoc->m_nHttpVideoSizeX = m_Sizes[0].cx;
				m_pDoc->m_nHttpVideoSizeY = m_Sizes[0].cy;
			}
			else
			{
				sRequest.Format(_T("GET %s?Quality=%s HTTP/%s\r\n"),
								sLocation,
								m_pDoc->m_nHttpVideoQuality == 0 ?
								_T("Clarity") :
								(m_pDoc->m_nHttpVideoQuality == 100 ?
								_T("Motion") :
								_T("Standard")),
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			break;
		}
		case PIXORD_SP :	// Pixord Server Push (mjpeg)
		{
			sLocation = _T("/getimage");
			if (HasResolution(CSize(m_pDoc->m_nHttpVideoSizeX, m_pDoc->m_nHttpVideoSizeY)))
			{
				sRequest.Format(_T("GET %s?camera=1&fmt=%s&delay=%d HTTP/%s\r\n"),
								sLocation,
								m_pDoc->m_nHttpVideoSizeX == 176 ?
								_T("qsif") :
								(m_pDoc->m_nHttpVideoSizeX == 352 ?
								_T("sif") :
								_T("full")),
								Round(100.0 / m_pDoc->m_dFrameRate),
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			else if (m_Sizes.GetSize() > 0)
			{
				sRequest.Format(_T("GET %s?camera=1&fmt=%s&delay=%d HTTP/%s\r\n"),
								sLocation,
								m_Sizes[0].cx == 176 ?
								_T("qsif") :
								(m_Sizes[0].cx == 352 ?
								_T("sif") :
								_T("full")),
								Round(100.0 / m_pDoc->m_dFrameRate),
								m_bOldVersion ? _T("1.0") : _T("1.1"));
				m_pDoc->m_nHttpVideoSizeX = m_Sizes[0].cx;
				m_pDoc->m_nHttpVideoSizeY = m_Sizes[0].cy;
			}
			else
			{
				sRequest.Format(_T("GET %s?camera=1&delay=%d HTTP/%s\r\n"),
								sLocation,
								Round(100.0 / m_pDoc->m_dFrameRate),
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			break;
		}
		case PIXORD_CP :	// Pixord Client Poll (jpegs)
		{
			sLocation = _T("/images1");
			if (HasResolution(CSize(m_pDoc->m_nHttpVideoSizeX, m_pDoc->m_nHttpVideoSizeY)))
			{
				sRequest.Format(_T("GET %s%s HTTP/%s\r\n"),
								sLocation,
								m_pDoc->m_nHttpVideoSizeX == 176 ?
								_T("qsif") :
								(m_pDoc->m_nHttpVideoSizeX == 352 ?
								_T("sif") :
								_T("full")),
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			else if (m_Sizes.GetSize() > 0)
			{
				sRequest.Format(_T("GET %s%s HTTP/%s\r\n"),
								sLocation,
								m_Sizes[0].cx == 176 ?
								_T("qsif") :
								(m_Sizes[0].cx == 352 ?
								_T("sif") :
								_T("full")),
								m_bOldVersion ? _T("1.0") : _T("1.1"));
				m_pDoc->m_nHttpVideoSizeX = m_Sizes[0].cx;
				m_pDoc->m_nHttpVideoSizeY = m_Sizes[0].cy;
			}
			else
			{
				sRequest.Format(_T("GET %s%s HTTP/%s\r\n"),
								sLocation,
								_T("sif"),
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			break;
		}
		case EDIMAX_SP :	// Edimax Server Push (mjpeg)
		{
			if (m_bQueryProperties)
			{
				sLocation = _T("/camera-cgi/admin/param.cgi?action=list&group=Properties.Image.I0");
				sRequest.Format(_T("GET %s HTTP/%s\r\n"),
								sLocation,
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			else if (m_bSetResolution)
			{
				sLocation = _T("/camera-cgi/admin/param.cgi");
				if (HasResolution(CSize(m_pDoc->m_nHttpVideoSizeX, m_pDoc->m_nHttpVideoSizeY)))
				{
					sRequest.Format(_T("GET %s?action=update&Image.I0.Appearance.Resolution=%dx%d HTTP/%s\r\n"),
									sLocation,
									m_pDoc->m_nHttpVideoSizeX,
									m_pDoc->m_nHttpVideoSizeY,
									m_bOldVersion ? _T("1.0") : _T("1.1"));
				}
				else if (m_Sizes.GetSize() > 0)
				{
					sRequest.Format(_T("GET %s?action=update&Image.I0.Appearance.Resolution=%dx%d HTTP/%s\r\n"),
									sLocation,
									m_Sizes[0].cx,
									m_Sizes[0].cy,
									m_bOldVersion ? _T("1.0") : _T("1.1"));
					m_pDoc->m_nHttpVideoSizeX = m_Sizes[0].cx;
					m_pDoc->m_nHttpVideoSizeY = m_Sizes[0].cy;
				}
				else
				{
					sRequest.Format(_T("GET %s?action=update&Image.I0.Appearance.Resolution=%dx%d HTTP/%s\r\n"),
									sLocation,
									DEFAULT_HTTP_VIDEO_SIZE_CX,
									DEFAULT_HTTP_VIDEO_SIZE_CY,
									m_bOldVersion ? _T("1.0") : _T("1.1"));
					m_pDoc->m_nHttpVideoSizeX = DEFAULT_HTTP_VIDEO_SIZE_CX;
					m_pDoc->m_nHttpVideoSizeY = DEFAULT_HTTP_VIDEO_SIZE_CY;
				}
			}
			else if (m_bSetCompression)
			{
				sLocation = _T("/camera-cgi/admin/param.cgi");
				sRequest.Format(_T("GET %s?action=update&Image.I0.Appearance.Compression=%d HTTP/%s\r\n"),
								sLocation,
								(100 - m_pDoc->m_nHttpVideoQuality) / 25, // value range is 0-4
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			else if (m_bSetFramerate)
			{
				int nFrameRate = Round(m_pDoc->m_dFrameRate);
				if (nFrameRate <= 0)
					nFrameRate = 1;
				sLocation = _T("/camera-cgi/admin/param.cgi");
				sRequest.Format(_T("GET %s?action=update&Image.I0.Stream.FPS=%d HTTP/%s\r\n"),
								sLocation,
								nFrameRate, // (1, 3, 5, 10, 15, 30)
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			else
			{
				sLocation = _T("/mjpg/video.mjpg");
				sRequest.Format(_T("GET %s HTTP/%s\r\n"),
							sLocation,
							m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			break;
		}
		case EDIMAX_CP :	// Edimax Client Poll (jpegs)
		{
			if (m_bQueryProperties)
			{
				sLocation = _T("/camera-cgi/admin/param.cgi?action=list&group=Properties.Image.I0");
				sRequest.Format(_T("GET %s HTTP/%s\r\n"),
								sLocation,
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			else if (m_bSetResolution)
			{
				sLocation = _T("/camera-cgi/admin/param.cgi");
				if (HasResolution(CSize(m_pDoc->m_nHttpVideoSizeX, m_pDoc->m_nHttpVideoSizeY)))
				{
					sRequest.Format(_T("GET %s?action=update&Image.I0.Appearance.Resolution=%dx%d HTTP/%s\r\n"),
									sLocation,
									m_pDoc->m_nHttpVideoSizeX,
									m_pDoc->m_nHttpVideoSizeY,
									m_bOldVersion ? _T("1.0") : _T("1.1"));
				}
				else if (m_Sizes.GetSize() > 0)
				{
					sRequest.Format(_T("GET %s?action=update&Image.I0.Appearance.Resolution=%dx%d HTTP/%s\r\n"),
									sLocation,
									m_Sizes[0].cx,
									m_Sizes[0].cy,
									m_bOldVersion ? _T("1.0") : _T("1.1"));
					m_pDoc->m_nHttpVideoSizeX = m_Sizes[0].cx;
					m_pDoc->m_nHttpVideoSizeY = m_Sizes[0].cy;
				}
				else
				{
					sRequest.Format(_T("GET %s?action=update&Image.I0.Appearance.Resolution=%dx%d HTTP/%s\r\n"),
									sLocation,
									DEFAULT_HTTP_VIDEO_SIZE_CX,
									DEFAULT_HTTP_VIDEO_SIZE_CY,
									m_bOldVersion ? _T("1.0") : _T("1.1"));
					m_pDoc->m_nHttpVideoSizeX = DEFAULT_HTTP_VIDEO_SIZE_CX;
					m_pDoc->m_nHttpVideoSizeY = DEFAULT_HTTP_VIDEO_SIZE_CY;
				}
			}
			else if (m_bSetCompression)
			{
				sLocation = _T("/camera-cgi/admin/param.cgi");
				sRequest.Format(_T("GET %s?action=update&Image.I0.Appearance.Compression=%d HTTP/%s\r\n"),
								sLocation,
								(100 - m_pDoc->m_nHttpVideoQuality) / 25, // value range is 0-4
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			else
			{
				sLocation = _T("/jpg/image.jpg");
				sRequest.Format(_T("GET %s HTTP/%s\r\n"),
							sLocation,
							m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			break;
		}
		case TPLINK_SP :	// TP-Link Server Push (mjpeg)
		{
			sLocation = _T("/video.mjpg");
			sRequest.Format(_T("GET %s HTTP/%s\r\n"),
						sLocation,
						m_bOldVersion ? _T("1.0") : _T("1.1"));
			break;
		}
		case TPLINK_CP :	// TP-Link Client Poll (jpegs)
		{
			sLocation = _T("/jpg/image.jpg");
			sRequest.Format(_T("GET %s HTTP/%s\r\n"),
						sLocation,
						m_bOldVersion ? _T("1.0") : _T("1.1"));
			break;
		}
		case FOSCAM_SP :	// Foscam Server Push (mjpeg)
		{
			int nRate;
			if (m_pDoc->m_dFrameRate > 20.0)
				nRate = 0;	// full speed
			else if (m_pDoc->m_dFrameRate > 15.0)
				nRate = 1;	// 20 fps
			else if (m_pDoc->m_dFrameRate > 10.0)
				nRate = 3;	// 15 fps
			else if (m_pDoc->m_dFrameRate > 5.0)
				nRate = 6;	// 10 fps
			else if (m_pDoc->m_dFrameRate > 4.0)
				nRate = 11;	// 5 fps
			else if (m_pDoc->m_dFrameRate > 3.0)
				nRate = 12;	// 4 fps
			else if (m_pDoc->m_dFrameRate > 2.0)
				nRate = 13;	// 3 fps
			else if (m_pDoc->m_dFrameRate > 1.0)
				nRate = 14;	// 2 fps
			else if (m_pDoc->m_dFrameRate > 0.5)
				nRate = 15;	// 1 fps
			else if (m_pDoc->m_dFrameRate > 0.333333)
				nRate = 17;	// 0.5 fps
			else if (m_pDoc->m_dFrameRate > 0.25)
				nRate = 19;	// 0.333333 fps
			else if (m_pDoc->m_dFrameRate > 0.2)
				nRate = 21;	// 0.25 fps
			else
				nRate = 23;	// 0.2 fps
			sLocation =	CString(_T("/videostream.cgi")) +
						_T("?user=") + HTTPGETFRAME_USERNAME_PLACEHOLDER +
						_T("&pwd=") + HTTPGETFRAME_PASSWORD_PLACEHOLDER;
			if (HasResolution(CSize(m_pDoc->m_nHttpVideoSizeX, m_pDoc->m_nHttpVideoSizeY)))
			{
				sRequest.Format(_T("GET %s&resolution=%d&rate=%d HTTP/%s\r\n"),
								sLocation,
								m_pDoc->m_nHttpVideoSizeX == 640 ? 32 : 8,
								nRate,
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			else if (m_Sizes.GetSize() > 0)
			{
				sRequest.Format(_T("GET %s&resolution=%d&rate=%d HTTP/%s\r\n"),
								sLocation,
								m_Sizes[0].cx == 640 ? 32 : 8,
								nRate,
								m_bOldVersion ? _T("1.0") : _T("1.1"));
				m_pDoc->m_nHttpVideoSizeX = m_Sizes[0].cx;
				m_pDoc->m_nHttpVideoSizeY = m_Sizes[0].cy;
			}
			else
			{
				sRequest.Format(_T("GET %s&rate=%d HTTP/%s\r\n"),
								sLocation,
								nRate,
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			break;
		}
		case FOSCAM_CP :	// Foscam Client Poll (jpegs)
		{
			sLocation = CString(_T("/snapshot.cgi")) +
						_T("?user=") + HTTPGETFRAME_USERNAME_PLACEHOLDER +
						_T("&pwd=") + HTTPGETFRAME_PASSWORD_PLACEHOLDER;
			if (HasResolution(CSize(m_pDoc->m_nHttpVideoSizeX, m_pDoc->m_nHttpVideoSizeY)))
			{
				sRequest.Format(_T("GET %s&resolution=%d HTTP/%s\r\n"),
								sLocation,
								m_pDoc->m_nHttpVideoSizeX == 640 ? 32 : 8,
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			else if (m_Sizes.GetSize() > 0)
			{
				sRequest.Format(_T("GET %s&resolution=%d HTTP/%s\r\n"),
								sLocation,
								m_Sizes[0].cx == 640 ? 32 : 8,
								m_bOldVersion ? _T("1.0") : _T("1.1"));
				m_pDoc->m_nHttpVideoSizeX = m_Sizes[0].cx;
				m_pDoc->m_nHttpVideoSizeY = m_Sizes[0].cy;
			}
			else
			{
				sRequest.Format(_T("GET %s HTTP/%s\r\n"),
								sLocation,
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			break;
		}
		default :
		{
			::LeaveCriticalSection(&m_pDoc->m_csHttpParams);
			ASSERT(FALSE);
			return FALSE;
		}
	}

	::LeaveCriticalSection(&m_pDoc->m_csHttpParams);
	
	return SendRawRequest(sRequest);
}

__forceinline int CVideoDeviceDoc::CHttpGetFrameParseProcess::FindMultipartBoundary(int nPos,
																					int nSize,
																					const char* pMsg)
{
	if (m_nMultipartBoundaryLength >= 2)
	{
		int nPosEnd = nPos;
		char cFirst = m_szMultipartBoundary[0];
		char cSecond = m_szMultipartBoundary[1];
		while ((nPosEnd + m_nMultipartBoundaryLength) <= nSize)
		{
			if (*(pMsg + nPosEnd) == cFirst			&&
				*(pMsg + nPosEnd + 1) == cSecond	&&
				memcmp(pMsg + nPosEnd, m_szMultipartBoundary, m_nMultipartBoundaryLength) == 0)
				return nPosEnd;
			nPosEnd++;
		}
	}
	return -1;
}

__forceinline int CVideoDeviceDoc::CHttpGetFrameParseProcess::FindSOI(	int nPos,
																		int nSize,
																		const char* pMsg)
{
	int nPosEnd = nPos;
	while ((nPosEnd + 2) <= nSize)
	{
		// 0xFF = -1 0xD8 = -40 SOI marker
		if (*(pMsg + nPosEnd) == -1 &&
			*(pMsg + nPosEnd + 1) == -40)
			return nPosEnd;
		nPosEnd++;
	}
	return -1;
}

__forceinline int CVideoDeviceDoc::CHttpGetFrameParseProcess::FindEndOfLine(const CString& s, int nStart)
{
	int nPosCR = s.Find(_T('\r'), nStart);
	int nPosLF = s.Find(_T('\n'), nStart);
	if (nPosCR >= 0 && nPosLF >= 0)
		return MIN(nPosCR, nPosLF);
	else if (nPosCR >= 0)
		return nPosCR;
	else if (nPosLF >= 0)
		return nPosLF;
	else
		return -1;
}

BOOL CVideoDeviceDoc::CHttpGetFrameParseProcess::ParseMultipart(CNetCom* pNetCom,
																int nPos,
																int nSize,
																const char* pMsg,
																const CString& sMsg,
																const CString& sMsgLowerCase)
{
	int nPosType, nPosEnd;
	int nMultipartLength;
	CString sContentType;

	if (m_bMultipartNoLength)
	{
		// Security Size Check
		if (nSize > HTTP_MAX_MULTIPART_SIZE)
		{
			pNetCom->Read(); // Empty the buffers
			return FALSE;
		}

		// Find Start
		if ((nPos = FindMultipartBoundary(nPos, nSize, pMsg)) < 0)
			return FALSE;

		// Find Start of JPEG image
		if ((nPos = FindSOI(nPos, nSize, pMsg)) < 0)
			return FALSE;

		// Find End and calc. Length
		if ((nPosEnd = FindMultipartBoundary(nPos, nSize, pMsg)) < 0)
			return FALSE;
		nMultipartLength = nPosEnd - nPos;
	}
	else
	{
		// Security Size Check
		if (nSize > HTTP_MAX_MULTIPART_SIZE / 2)
		{
			m_bMultipartNoLength = TRUE;
			return FALSE;
		}

		// Find Boundary
		if (m_sMultipartBoundary.IsEmpty() || (nPos = sMsg.Find(m_sMultipartBoundary, nPos)) < 0)
			return FALSE;

		// Content Type
		if ((nPosType = sMsgLowerCase.Find(_T("content-type:"), nPos)) >= 0)
		{
			nPosType += 13;
			if ((nPosEnd = FindEndOfLine(sMsg, nPosType)) < 0)
				return FALSE;
			sContentType = sMsgLowerCase.Mid(nPosType, nPosEnd - nPosType);
			sContentType.TrimLeft();
		}

		// Find Content Length
		if ((nPos = sMsgLowerCase.Find(_T("content-length:"), nPos)) < 0)
		{
			if (nSize > HTTP_MIN_MULTIPART_SIZE)
				m_bMultipartNoLength = TRUE;
			return FALSE;
		}
		nPos += 15;
		if ((nPosEnd = FindEndOfLine(sMsg, nPos)) < 0)
			return FALSE;

		// Parse Content Length
		CString sMultipartLength = sMsgLowerCase.Mid(nPos, nPosEnd - nPos);
		if (sMultipartLength == _T(""))
			return FALSE;
		sMultipartLength.TrimLeft();
		if ((nPos = sMultipartLength.Find(_T(' '), 0)) >= 0)
			sMultipartLength = sMultipartLength.Left(nPos);
		nMultipartLength = MAX(0, _ttoi(sMultipartLength)); // zero length is valid

		// Find Start of Multipart Data
		if ((nPos = sMsg.Find(_T("\r\n\r\n"), nPosEnd)) < 0)
		{
			if ((nPos = sMsg.Find(_T("\n\n"), nPosEnd)) < 0)
				return FALSE;
			else
				nPos += 2;
		}
		else
			nPos += 4;
	}

	// Set Process Vars
	m_nProcessOffset = nPos;
	m_nProcessSize = nMultipartLength;

	// Process data
	if (nSize >= m_nProcessOffset + m_nProcessSize)
	{
		// Remove non-jpeg stream (usually a audio stream)
		if (sContentType != _T("") && sContentType.Find(_T("image/jpeg")) < 0)
		{
			pNetCom->Read(NULL, m_nProcessOffset + m_nProcessSize);
			return FALSE;
		}
		else
		{
			m_FormatType = FORMATMJPEG;
			return TRUE;
		}
	}
	else
		return FALSE;
}

BOOL CVideoDeviceDoc::CHttpGetFrameParseProcess::ParseSingle(	BOOL bLastCall,
																int nSize,
																const CString& sMsg,
																const CString& sMsgLowerCase)
{
	int nPos;
	int nPosEnd;

	// Find Init of JPEG image
	if ((nPos = sMsg.Find(_T("\r\n\r\n"))) < 0)
	{
		if ((nPos = sMsg.Find(_T("\n\n"))) < 0)
			return FALSE;
		else
			nPos += 2;
	}
	else
		nPos += 4;
	m_nProcessOffset = nPos;

	// Find Content Length
	if ((nPos = sMsgLowerCase.Find(_T("content-length:"))) >= 0)
	{
		nPos += 15;
		if ((nPosEnd = FindEndOfLine(sMsg, nPos)) < 0)
			return FALSE;
		CString sSinglepartLength = sMsgLowerCase.Mid(nPos, nPosEnd - nPos);
		if (sSinglepartLength == _T(""))
			return FALSE;
		sSinglepartLength.TrimLeft();
		if ((nPos = sSinglepartLength.Find(_T(' '), 0)) >= 0)
			sSinglepartLength = sSinglepartLength.Left(nPos);
		m_nProcessSize = MAX(0, _ttoi(sSinglepartLength)); // zero length is valid
	}
	// Wait last parser call if image with no Content Length
	else if (bLastCall)
		m_nProcessSize = nSize - m_nProcessOffset;

	// Process data
	if (nSize >= m_nProcessOffset + m_nProcessSize)
	{
		m_FormatType = FORMATJPEG;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CVideoDeviceDoc::CHttpGetFrameParseProcess::HasResolution(const CSize& Size)
{
	for (int nSize = 0 ; nSize < m_Sizes.GetSize() ; nSize++)
	{
		if (Size == m_Sizes[nSize])
			return TRUE;
	}
	return FALSE;
}

/* HTTP defines the sequence CR LF as the end-of-line marker
for all protocol elements except the Entity-Body.
The end-of-line marker within an Entity-Body is defined by its
associated media type.
http://tools.ietf.org/html/rfc2616
*/
BOOL CVideoDeviceDoc::CHttpGetFrameParseProcess::Parse(CNetCom* pNetCom, BOOL bLastCall)
{
	int nPos, nPosEnd;
	int nSize = 0;
	int nMsgSize = 0;
	CString sMsg;
	CString sMsgLowerCase;
	char* pMsg;
	BOOL res;
	
	::EnterCriticalSection(pNetCom->GetRxFifoSync());

	// Calc. Size
	POSITION pos = pNetCom->GetRxFifo()->GetHeadPosition();
	while (pos)
	{
		CNetCom::CBuf* pBuf = pNetCom->GetRxFifo()->GetNext(pos);
		if (pBuf)
			nSize += pBuf->GetMsgSize();
	}

	// Allocate Linear Buffer and copy message
	if (m_bMultipartNoLength)
	{
		pMsg = new char[MAX(HTTP_MAX_HEADER_SIZE, nSize)];
		if (!pMsg)
		{
			::LeaveCriticalSection(pNetCom->GetRxFifoSync());
			return FALSE; // Do not call Processor
		}
	}
	else
	{
		pMsg = new char[HTTP_MAX_HEADER_SIZE];
		if (!pMsg)
		{
			::LeaveCriticalSection(pNetCom->GetRxFifoSync());
			return FALSE; // Do not call Processor
		}
	}
	memset(pMsg, 0, HTTP_MAX_HEADER_SIZE);
	pos = pNetCom->GetRxFifo()->GetHeadPosition();
	while (pos)
	{
		CNetCom::CBuf* pBuf = pNetCom->GetRxFifo()->GetNext(pos);
		if (pBuf)
		{
			// Update Message Buffer and Size
			int nNextMsgSize = pBuf->GetMsgSize() + nMsgSize;
			if (!m_bMultipartNoLength)
			{
				if (nNextMsgSize >= HTTP_MAX_HEADER_SIZE)
					nNextMsgSize = HTTP_MAX_HEADER_SIZE - 1;
			}
			int nCopyMsgSize = nNextMsgSize - nMsgSize;
			if (nCopyMsgSize > 0)
			{
				memcpy(pMsg + nMsgSize, pBuf->GetBuf(), nCopyMsgSize);
				nMsgSize = nNextMsgSize;
			}
			else
				break;
		}
	}

	::LeaveCriticalSection(pNetCom->GetRxFifoSync());
	
	// Init sMsg and sMsgLowerCase Vars
	char cTemp = pMsg[HTTP_MAX_HEADER_SIZE - 1];
	pMsg[HTTP_MAX_HEADER_SIZE - 1] = '\0';
	sMsg = CString(pMsg);
	sMsgLowerCase = sMsg;
	sMsgLowerCase.MakeLower();
	pMsg[HTTP_MAX_HEADER_SIZE - 1] = cTemp;

	// Begin of Answer?
	if (sMsgLowerCase.Left(5) == _T("http/"))
	{
		// Length
		nPosEnd = sMsgLowerCase.GetLength();

		// Version
		CString sVersion;
		for (nPos = 5 ; nPos < nPosEnd ; nPos++)
		{
			if (_istspace(sMsgLowerCase[nPos]))
				break;
			sVersion += sMsgLowerCase[nPos];
		}
		if (sVersion == _T("0.9") || sVersion == _T("1.0"))
			m_bOldVersion = TRUE;
		else
			m_bOldVersion = FALSE;

		// Skip space(s)
		while (nPos < nPosEnd && _istspace(sMsgLowerCase[nPos]))
			nPos++;

		// 3 Digits Code
		CString sCode;
		for (; nPos < nPosEnd ; nPos++)
		{
			if (_istspace(sMsgLowerCase[nPos]))
				break;
			sCode += sMsgLowerCase[nPos];
		}
		if (sCode.GetLength() < 3)
		{
			delete [] pMsg;
			return FALSE; // Do not call Processor
		}

		// Connection Keep Alive?
		if (m_bOldVersion)
		{
			if (sMsgLowerCase.Find(_T("connection: keep-alive"), 0) >= 0)
				m_bConnectionKeepAlive = TRUE;
			else
				m_bConnectionKeepAlive = FALSE;
		}
		else
		{
			if (sMsgLowerCase.Find(_T("connection: close"), 0) >= 0)
				m_bConnectionKeepAlive = FALSE;
			else	
				m_bConnectionKeepAlive = TRUE;
		}

		// OK
		if (sCode == _T("200"))
		{
			// Multipart image
			if ((nPos = sMsgLowerCase.Find(_T("content-type: multipart/x-mixed-replace"), 0)) >= 0)
			{
				if ((nPos = sMsgLowerCase.Find(_T("boundary="), nPos)) < 0)
				{
					delete [] pMsg;
					return FALSE; // Do not call Processor
				}
				if ((nPosEnd = FindEndOfLine(sMsg, nPos)) < 0)
				{
					delete [] pMsg;
					return FALSE; // Do not call Processor
				}
				int nPosEndLine = nPosEnd;
				nPos += 9;
				nPosEnd--;
				while (	nPos < nPosEnd &&
						(sMsg[nPos] == _T(' ') ||
						sMsg[nPos] == _T('\"')))
					nPos++;
				while (	nPos < nPosEnd &&
						(sMsg[nPosEnd] == _T(' ') ||
						sMsg[nPosEnd] == _T('\"')))
					nPosEnd--;
				m_nMultipartBoundaryLength = MIN(nPosEnd - nPos + 1, HTTP_MAX_MULTIPART_BOUNDARY - 1);
				memcpy(m_szMultipartBoundary, pMsg + nPos, m_nMultipartBoundaryLength);
				m_szMultipartBoundary[m_nMultipartBoundaryLength] = '\0';
				if (m_nMultipartBoundaryLength <= 1)
				{
					delete [] pMsg;
					return FALSE; // Do not call Processor
				}
				m_sMultipartBoundary = CString(m_szMultipartBoundary);

				// Flags
				m_bTryConnecting = FALSE;
				m_bFirstFrame = TRUE;

				// Call mjpeg parser
				res = ParseMultipart(pNetCom, nPosEndLine, nSize, pMsg, sMsg, sMsgLowerCase);
				delete [] pMsg;
#if defined(_DEBUG) || defined(TRACELOGFILE)
				if (res && (nPosEnd = sMsg.Find(_T("\r\n\r\n"))) > 0)
					TRACE(sMsg.Left(nPosEnd) + _T('\n'));
#endif
				return res;
			}
			// Single image
			else if ((nPos = sMsgLowerCase.Find(_T("content-type: image/jpeg"), 0)) >= 0)
			{
				// Flags
				m_bTryConnecting = FALSE;
				m_bFirstFrame = TRUE;

				// Call jpeg parser
				res = ParseSingle(bLastCall, nSize, sMsg, sMsgLowerCase);
				delete [] pMsg;
#if defined(_DEBUG) || defined(TRACELOGFILE)
				if (res && (nPosEnd = sMsg.Find(_T("\r\n\r\n"))) > 0)
					TRACE(sMsg.Left(nPosEnd) + _T('\n'));
#endif
				return res;
			}
			// Text
			else if ((nPos = sMsgLowerCase.Find(_T("content-type: text/plain"), 0)) >= 0)
			{
				if ((m_pDoc->m_nNetworkDeviceTypeMode == OTHERONE_SP	||
					m_pDoc->m_nNetworkDeviceTypeMode == OTHERONE_CP)	&&
					m_bTryConnecting									&&
					++m_pDoc->m_nHttpGetFrameLocationPos < m_pDoc->m_HttpGetFrameLocations.GetSize())
				{
					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Try next possible device location string
					m_pDoc->m_HttpGetFrameThread.SetEventConnect();
				}
				else if (m_bQueryProperties)
				{
					if ((nPosEnd = sMsg.Find(_T("\r\n\r\n"), nPos)) < 0)
					{
						if ((nPosEnd = sMsg.Find(_T("\n\n"), nPos)) < 0)
						{
							delete [] pMsg;
							return FALSE; // Do not call Processor
						}
						else
							nPosEnd += 2;
					}
					else
						nPosEnd += 4;
					CString sText;
					sText = sMsg.Mid(nPosEnd);
					sText.MakeLower();
					if (sText == _T(""))
					{
						delete [] pMsg;
						return FALSE; // Do not call Processor
					}
					
					// Parse the Image Resolutions
					if ((nPos = sText.Find(_T("resolution"), 0)) < 0)
					{
						delete [] pMsg;
						return FALSE; // Do not call Processor
					}
					nPos += 11;	// Points to first resolution
					if ((nPosEnd = sText.Find(_T('\n'), nPos)) < 0)
					{
						delete [] pMsg;
						return FALSE; // Do not call Processor
					}
					m_Sizes.RemoveAll();
					int nNextPos = nPos - 1;
					while (nNextPos < nPosEnd)
					{
						nNextPos = sText.Find(_T(','), nPos);
						if (nNextPos < 0 || nNextPos > nPosEnd)
							nNextPos = nPosEnd;
						CString sSize = sText.Mid(nPos, nNextPos - nPos);
						int cx = 0;
						int cy = 0;
						if (_stscanf(sSize, _T("%dx%d"), &cx, &cy) == 2)
							m_Sizes.Add(CSize(cx, cy));
						nPos = nNextPos + 1;
					}

					// Reset flags
					m_bQueryProperties = FALSE;
					m_bTryConnecting = FALSE;

					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Start Connection
					m_pDoc->m_HttpGetFrameThread.SetEventConnect();
				}
				else if (m_bSetResolution)
				{
					// Reset flags
					m_bSetResolution = FALSE;
					m_bTryConnecting = FALSE;

					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Start Connection
					m_pDoc->m_HttpGetFrameThread.SetEventConnect();
				}
				else if (m_bSetCompression)
				{
					// Reset flags
					m_bSetCompression = FALSE;
					m_bTryConnecting = FALSE;

					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Start Connection
					m_pDoc->m_HttpGetFrameThread.SetEventConnect();
				}
				else if (m_bSetFramerate)
				{
					// Reset flags
					m_bSetFramerate = FALSE;
					m_bTryConnecting = FALSE;

					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Start Connection
					m_pDoc->m_HttpGetFrameThread.SetEventConnect();
				}
				else if (m_bTryConnecting)
				{
					// Reset flag
					m_bTryConnecting = FALSE;

					// Msg
					m_pDoc->ConnectErr(ML_STRING(1488, "Camera is telling you something,\nfirst open it in a browser, then come back here."), m_pDoc->GetDevicePathName(), m_pDoc->GetDeviceName());
					
					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Close
					m_pDoc->CloseDocument();
				}
				delete [] pMsg;
#if defined(_DEBUG) || defined(TRACELOGFILE)
				if ((nPosEnd = sMsg.Find(_T("\r\n\r\n"))) > 0)
					TRACE(sMsg.Left(nPosEnd) + _T('\n'));
#endif
				return FALSE; // Do not call Processor
			}
			// Html
			else if ((nPos = sMsgLowerCase.Find(_T("content-type: text/html"), 0)) >= 0)
			{
				if ((m_pDoc->m_nNetworkDeviceTypeMode == OTHERONE_SP	||
					m_pDoc->m_nNetworkDeviceTypeMode == OTHERONE_CP)	&&
					m_bTryConnecting									&&
					++m_pDoc->m_nHttpGetFrameLocationPos < m_pDoc->m_HttpGetFrameLocations.GetSize())
				{
					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Try next possible device location string
					m_pDoc->m_HttpGetFrameThread.SetEventConnect();
				}
				else if (m_bTryConnecting)
				{
					// Reset flag
					m_bTryConnecting = FALSE;

					// Msg
					m_pDoc->ConnectErr(ML_STRING(1489, "Camera is asking you something (probably to set a password),\nfirst open it in a browser, then come back here."), m_pDoc->GetDevicePathName(), m_pDoc->GetDeviceName());
					
					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();
					
					// Close
					m_pDoc->CloseDocument();
				}
				delete [] pMsg;
#if defined(_DEBUG) || defined(TRACELOGFILE)
				if ((nPosEnd = sMsg.Find(_T("\r\n\r\n"))) > 0)
					TRACE(sMsg.Left(nPosEnd) + _T('\n'));
#endif
				return FALSE; // Do not call Processor
			}
			// Unknown or incomplete
			else
			{
				delete [] pMsg;
				return FALSE; // Do not call Processor
			}
		}
		// Redirect
		else if (sCode == _T("301") || sCode == _T("302") || sCode == _T("303") || sCode == _T("307"))
		{
			// Parse new location (new location must have same host and port)
			if ((nPos = sMsgLowerCase.Find(_T("location:"), 0)) < 0)
			{
				delete [] pMsg;
				return FALSE; // Do not call Processor
			}
			if ((nPosEnd = FindEndOfLine(sMsgLowerCase, nPos)) < 0)
			{
				delete [] pMsg;
				return FALSE; // Do not call Processor
			}
			nPos += 9;
			CString sNewLocation = sMsg.Mid(nPos, nPosEnd - nPos);
			sNewLocation.TrimLeft();
			sNewLocation.TrimRight();
			CString sNewLocationLowerCase(sNewLocation);
			sNewLocationLowerCase.MakeLower();
			if (sNewLocationLowerCase.Find(_T("http://"), 0) >= 0)
			{
				sNewLocation = sNewLocation.Right(sNewLocation.GetLength() - 7);
				if ((nPos = sNewLocation.Find(_T('/'), 0)) >= 0)
					sNewLocation = sNewLocation.Right(sNewLocation.GetLength() - nPos);
			}

			// Add params if we requested some
			CString sNewRequest;
			if ((nPos = m_sLastRequest.Find(_T('?'), 0)) >= 0)
			{
				CString sTrailing;
				sTrailing = m_sLastRequest.Right(m_sLastRequest.GetLength() - nPos);
				sNewRequest.Format(_T("GET %s"), sNewLocation);
				sNewRequest += sTrailing;
			}
			else
			{
				sNewRequest.Format(	_T("GET %s HTTP/%s\r\n"),
									sNewLocation,
									m_bOldVersion ? _T("1.0") : _T("1.1"));
			}

			// Empty the buffers, so that parser stops calling us!
			pNetCom->Read();

			// Start Connection with New Request
			m_pDoc->m_HttpGetFrameThread.SetEventConnect(sNewRequest);
	
			delete [] pMsg;
#if defined(_DEBUG) || defined(TRACELOGFILE)
				if ((nPosEnd = sMsg.Find(_T("\r\n\r\n"))) > 0)
					TRACE(sMsg.Left(nPosEnd) + _T('\n'));
#endif
			return FALSE; // Do not call Processor
		}
		// Unauthorized
		else if (sCode == _T("401"))
		{
			// Parse first www-authenticate line
			if ((nPos = sMsgLowerCase.Find(_T("www-authenticate:"), 0)) < 0)
			{
				delete [] pMsg;
				return FALSE; // Do not call Processor
			}
			if ((nPosEnd = FindEndOfLine(sMsgLowerCase, nPos)) < 0)
			{
				delete [] pMsg;
				return FALSE; // Do not call Processor
			}
			CString sAuthLine1LowerCase = sMsgLowerCase.Mid(nPos, nPosEnd - nPos);
			CString sAuthLine1 = sMsg.Mid(nPos, nPosEnd - nPos);

			// There may be a second www-authenticate line if an additional authentication type is supported
			CString sAuthLine2LowerCase(_T(""));
			CString sAuthLine2(_T(""));
			if ((nPos = sMsgLowerCase.Find(_T("www-authenticate:"), nPosEnd)) >= 0)
			{
				if ((nPosEnd = FindEndOfLine(sMsgLowerCase, nPos)) < 0)
				{
					delete [] pMsg;
					return FALSE; // Do not call Processor
				}
				else
				{
					sAuthLine2LowerCase = sMsgLowerCase.Mid(nPos, nPosEnd - nPos);
					sAuthLine2 = sMsg.Mid(nPos, nPosEnd - nPos);
				}
			}
			pNetCom->Read(); // Empty the buffers, so that parser stops calling us!

			// Authentication failed?
			if (m_LastRequestAuthorizationType != AUTHNONE)
			{
				// Reset flag
				m_bTryConnecting = FALSE;

				// Msg
				m_pDoc->ConnectErr(ML_STRING(1780, "The request to connect could not be completed because the supplied user name and/or password are incorrect."), m_pDoc->GetDevicePathName(), m_pDoc->GetDeviceName());

				// Close
				m_pDoc->CloseDocument();

				delete [] pMsg;
				return FALSE; // Do not call Processor
			}

			// Choose best authentication type depending from the format type
			AUTHTYPE AnswerAuthorizationType1 = AUTHNONE;
			if (sAuthLine1LowerCase.Find(_T("basic"), 0) >= 0)
				AnswerAuthorizationType1 = AUTHBASIC;
			else if (sAuthLine1LowerCase.Find(_T("digest"), 0) >= 0)
				AnswerAuthorizationType1 = AUTHDIGEST;
			CString sChosenAuthLine;
			CString sChosenAuthLineLowerCase;
			if (sAuthLine2LowerCase == _T(""))
			{
				m_AnswerAuthorizationType = AnswerAuthorizationType1;
				sChosenAuthLine = sAuthLine1;
				sChosenAuthLineLowerCase = sAuthLine1LowerCase;
			}
			else
			{
				AUTHTYPE AnswerAuthorizationType2 = AUTHNONE;
				if (sAuthLine2LowerCase.Find(_T("basic"), 0) >= 0)
					AnswerAuthorizationType2 = AUTHBASIC;
				else if (sAuthLine2LowerCase.Find(_T("digest"), 0) >= 0)
					AnswerAuthorizationType2 = AUTHDIGEST;
				if (m_FormatType == FORMATJPEG)
				{
					// Choose fastest for jpeg polling
					if (AnswerAuthorizationType1 < AnswerAuthorizationType2)
					{
						m_AnswerAuthorizationType = AnswerAuthorizationType1;
						sChosenAuthLine = sAuthLine1;
						sChosenAuthLineLowerCase = sAuthLine1LowerCase;
					}
					else
					{
						m_AnswerAuthorizationType = AnswerAuthorizationType2;
						sChosenAuthLine = sAuthLine2;
						sChosenAuthLineLowerCase = sAuthLine2LowerCase;
					}
				}
				else
				{
					// Choose most secure for mjpeg
					if (AnswerAuthorizationType1 < AnswerAuthorizationType2)
					{
						m_AnswerAuthorizationType = AnswerAuthorizationType2;
						sChosenAuthLine = sAuthLine2;
						sChosenAuthLineLowerCase = sAuthLine2LowerCase;
					}
					else
					{
						m_AnswerAuthorizationType = AnswerAuthorizationType1;
						sChosenAuthLine = sAuthLine1;
						sChosenAuthLineLowerCase = sAuthLine1LowerCase;
					}
				}
			}

			// Realm (m_sRealm will not contain the double quotes)
			if ((nPos = sChosenAuthLineLowerCase.Find(_T("realm=\""), 0)) >= 0)
			{
				nPos += 7;
				if ((nPosEnd = sChosenAuthLineLowerCase.Find(_T('\"'), nPos)) >= 0)
					m_sRealm = sChosenAuthLine.Mid(nPos, nPosEnd - nPos);
			}

			// Qop (m_sQop will not contain the double quotes)
			if ((nPos = sChosenAuthLineLowerCase.Find(_T("qop=\""), 0)) >= 0)
			{
				nPos += 5;
				if ((nPosEnd = sChosenAuthLineLowerCase.Find(_T('\"'), nPos)) >= 0)
					m_sQop = sChosenAuthLine.Mid(nPos, nPosEnd - nPos); 
			}

			// Nonce (m_sNonce will not contain the double quotes)
			if ((nPos = sChosenAuthLineLowerCase.Find(_T("nonce=\""), 0)) >= 0)
			{
				nPos += 7;
				if ((nPosEnd = sChosenAuthLineLowerCase.Find(_T('\"'), nPos)) >= 0)
					m_sNonce = sChosenAuthLine.Mid(nPos, nPosEnd - nPos); 
			}

			// Algorithm
			if ((nPos = sChosenAuthLineLowerCase.Find(_T("algorithm="), 0)) >= 0)
			{
				nPos += 10;
				m_sAlgorithm = sChosenAuthLine.Mid(nPos);
				m_sAlgorithm.TrimLeft();
				if ((nPosEnd = m_sAlgorithm.Find(_T(','))) >= 0)
					m_sAlgorithm = m_sAlgorithm.Left(nPosEnd); 
				m_sAlgorithm.TrimRight();
			}

			// Opaque (m_sOpaque will not contain the double quotes)
			if ((nPos = sChosenAuthLineLowerCase.Find(_T("opaque=\""), 0)) >= 0)
			{
				nPos += 8;
				if ((nPosEnd = sChosenAuthLineLowerCase.Find(_T('\"'), nPos)) >= 0)
					m_sOpaque = sChosenAuthLine.Mid(nPos, nPosEnd - nPos); 
			}

			// Start Connection with Last Request
			m_pDoc->m_HttpGetFrameThread.SetEventConnect(m_sLastRequest);
	
			delete [] pMsg;
#if defined(_DEBUG) || defined(TRACELOGFILE)
				if ((nPosEnd = sMsg.Find(_T("\r\n\r\n"))) > 0)
					TRACE(sMsg.Left(nPosEnd) + _T('\n'));
#endif
			return FALSE; // Do not call Processor
		}
		else
		{
			if ((m_pDoc->m_nNetworkDeviceTypeMode == OTHERONE_SP	||
				m_pDoc->m_nNetworkDeviceTypeMode == OTHERONE_CP)	&&
				m_bTryConnecting									&&
				++m_pDoc->m_nHttpGetFrameLocationPos < m_pDoc->m_HttpGetFrameLocations.GetSize())
			{
				// Empty the buffers, so that parser stops calling us!
				pNetCom->Read();

				// Try next possible device location string
				m_pDoc->m_HttpGetFrameThread.SetEventConnect();
			}
			else if (m_bTryConnecting)
			{
				// Reset flag
				m_bTryConnecting = FALSE;

				// Msg
				if (sCode == _T("503")) // Service Unavailable
					m_pDoc->ConnectErr(ML_STRING(1491, "Server is to busy, try later"), m_pDoc->GetDevicePathName(), m_pDoc->GetDeviceName());
				else
					m_pDoc->ConnectErr(ML_STRING(1490, "Unsupported network device type or mode"), m_pDoc->GetDevicePathName(), m_pDoc->GetDeviceName());

				// Empty the buffers, so that parser stops calling us!
				pNetCom->Read();

				// Close
				m_pDoc->CloseDocument();
			}
			// Maybe we polled to fast or we changed a param and camera is not yet ready
			else
			{
				// Empty the buffers, so that parser stops calling us!
				pNetCom->Read();

				// Retry start connection with delay
				m_pDoc->m_HttpGetFrameThread.SetEventConnect(_T(""), HTTPGETFRAME_RECONNECTION_DELAY);
			}
			delete [] pMsg;
#if defined(_DEBUG) || defined(TRACELOGFILE)
				if ((nPosEnd = sMsg.Find(_T("\r\n\r\n"))) > 0)
					TRACE(sMsg.Left(nPosEnd) + _T('\n'));
#endif
			return FALSE; // Do not call Processor
		}
	}
	// Multipart or something more received before an above
	// called m_HttpGetFrameThread.SetEventConnect() gets executed. That's ok
	// because ParseMultipart() is robust!
	else
	{
		res = ParseMultipart(pNetCom, 0, nSize, pMsg, sMsg, sMsgLowerCase);
		delete [] pMsg;
#if defined(_DEBUG) || defined(TRACELOGFILE)
		if (res && (nPosEnd = sMsg.Find(_T("\r\n\r\n"))) > 0)
			TRACE(sMsg.Left(nPosEnd) + _T('\n'));
#endif
		return res;
	}
}

BOOL CVideoDeviceDoc::CHttpGetFrameParseProcess::Process(unsigned char* pLinBuf, int nSize)
{
	::EnterCriticalSection(&m_pDoc->m_csHttpProcess);

	// Poll Next Frame
	if (m_FormatType == FORMATJPEG)
		m_bPollNextJpeg = TRUE;

	// If new frame sequence do a reset
	if (m_bFirstFrame)
		OpenAVCodec();

	// Check
	if (!m_pCodecCtx)
	{
		::LeaveCriticalSection(&m_pDoc->m_csHttpProcess);
		return TRUE; // Auto-Delete pLinBuf
	}

	// Decode
	int got_picture = 0;
	int len = avcodec_decode_video(	m_pCodecCtx,
									m_pFrame,
									&got_picture,
									(unsigned __int8 *)pLinBuf,
									nSize);
    if (len < 0)
	{
		::LeaveCriticalSection(&m_pDoc->m_csHttpProcess);
		return TRUE; // Auto-Delete pLinBuf
	}

	// Re-init if the size changed externally
	if (m_pDoc->m_DocRect.right != m_pCodecCtx->width ||
		m_pDoc->m_DocRect.bottom != m_pCodecCtx->height)
		m_bFirstFrame = TRUE;

	// Init
	if (m_bFirstFrame)
	{
		// Init Image Convert Context
		if (!InitImgConvert())
		{
			::LeaveCriticalSection(&m_pDoc->m_csHttpProcess);
			return TRUE; // Auto-Delete pLinBuf
		}

		// Init if size changed
		if (m_pDoc->m_DocRect.right != m_pCodecCtx->width ||
			m_pDoc->m_DocRect.bottom != m_pCodecCtx->height)
		{
			m_pDoc->m_ProcessFrameBMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			m_pDoc->m_ProcessFrameBMI.bmiHeader.biWidth = (DWORD)m_pCodecCtx->width;
			m_pDoc->m_ProcessFrameBMI.bmiHeader.biHeight = (DWORD)m_pCodecCtx->height;
			m_pDoc->m_ProcessFrameBMI.bmiHeader.biPlanes = 1; // must be 1
			m_pDoc->m_ProcessFrameBMI.bmiHeader.biBitCount = 12;
			m_pDoc->m_ProcessFrameBMI.bmiHeader.biCompression = FCC('I420');    
			m_pDoc->m_ProcessFrameBMI.bmiHeader.biSizeImage = m_dwI420ImageSize;
			m_pDoc->m_DocRect.right = m_pDoc->m_ProcessFrameBMI.bmiHeader.biWidth;
			m_pDoc->m_DocRect.bottom = m_pDoc->m_ProcessFrameBMI.bmiHeader.biHeight;

			// Make sure the video size is set correctly
			::EnterCriticalSection(&m_pDoc->m_csHttpParams);
			if ((CSize(m_pDoc->m_nHttpVideoSizeX, m_pDoc->m_nHttpVideoSizeY) !=
				CSize(m_pDoc->m_DocRect.right, m_pDoc->m_DocRect.bottom))	&&
				m_Sizes.GetSize() <= 1)
			{	
				m_pDoc->m_nHttpVideoSizeX = m_pDoc->m_DocRect.right;
				m_pDoc->m_nHttpVideoSizeY = m_pDoc->m_DocRect.bottom;
				m_Sizes.RemoveAll();
				m_Sizes.Add(CSize(m_pDoc->m_nHttpVideoSizeX, m_pDoc->m_nHttpVideoSizeY));
			}
			::LeaveCriticalSection(&m_pDoc->m_csHttpParams);

			// Free Movement Detector because we changed size and/or format!
			m_pDoc->FreeMovementDetector();

			// Update
			if (m_pDoc->m_bSizeToDoc)
			{
				// This sizes the view to m_DocRect in normal screen mode,
				// in full-screen mode it updates m_ZoomRect from m_DocRect
				::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
								WM_THREADSAFE_UPDATEWINDOWSIZES,
								(WPARAM)UPDATEWINDOWSIZES_SIZETODOC,
								(LPARAM)0);
				m_pDoc->m_bSizeToDoc = FALSE;
			}
			else
			{
				// In full-screen mode it updates m_ZoomRect from m_DocRect
				::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
								WM_THREADSAFE_UPDATEWINDOWSIZES,
								(WPARAM)0,
								(LPARAM)0);
			}
			::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
							WM_THREADSAFE_SETDOCUMENTTITLE,
							0, 0);
			::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
							WM_THREADSAFE_UPDATE_PHPPARAMS,
							0, 0);
		}
	}

	// Color Space Conversion
	if (got_picture && m_pImgConvertCtx)
	{
		int sws_scale_res = sws_scale(	m_pImgConvertCtx,		// Image Convert Context
										m_pFrame->data,			// Source Data
										m_pFrame->linesize,		// Source Stride
										0,						// Source Slice Y
										m_pCodecCtx->height,	// Source Height
										m_pFrameI420->data,		// Destination Data
										m_pFrameI420->linesize);// Destination Stride
#ifdef SUPPORT_LIBSWSCALE
		int res = sws_scale_res > 0 ? 1 : -1;
#else
		int res = sws_scale_res >= 0 ? 1 : -1;
#endif
		// Process Frame
		if (res == 1)
		{
			m_pDoc->m_lCompressedDataRateSum += nSize;
			m_pDoc->ProcessI420Frame(m_pI420Buf, m_dwI420ImageSize, pLinBuf, nSize);
		}
	}
	// In case that avcodec_decode_video fails use LoadJPEG which is more fault tolerant, but slower...
	else
	{
		CDib Dib;
		Dib.SetShowMessageBoxOnError(FALSE);
		if (Dib.LoadJPEG(pLinBuf, nSize, 1, TRUE) && Dib.Compress(FCC('I420')))
		{
			m_pDoc->m_lCompressedDataRateSum += nSize;
			m_pDoc->ProcessI420Frame(Dib.GetBits(), Dib.GetImageSize(), pLinBuf, nSize);
		}
	}
	
	// Reset first frame flag after the process frame,
	// otherwise the watchdog is restarting the connection!
	m_bFirstFrame = FALSE;
	
	::LeaveCriticalSection(&m_pDoc->m_csHttpProcess);

	return TRUE; // Auto-Delete pLinBuf
}

BOOL CVideoDeviceDoc::CHttpGetFrameParseProcess::OpenAVCodec()
{
	// Free
	FreeAVCodec();

    // Find the decoder for the video stream
	m_pCodec = avcodec_find_decoder(CODEC_ID_MJPEG);
    if (!m_pCodec)
        goto error_noclose;
	m_pDoc->m_CaptureBMI.bmiHeader.biCompression = FCC('MJPG');

	// Allocate Context
	m_pCodecCtx = avcodec_alloc_context();
	if (!m_pCodecCtx)
		goto error_noclose;

	// Width and Height Unknown at this point
	m_pCodecCtx->coded_width = 0;
	m_pCodecCtx->coded_height = 0;

	// Set FourCC
	m_pCodecCtx->codec_tag = FCC('MJPG');

	// Set some other values
	m_pCodecCtx->error_concealment = 3;
	m_pCodecCtx->error_recognition = 1;

	// Open codec
    if (avcodec_open_thread_safe(m_pCodecCtx, m_pCodec) < 0)
        goto error_noclose;

	// Allocate video frames
    m_pFrame = avcodec_alloc_frame();
	if (!m_pFrame)
        goto error;
	m_pFrameI420 = avcodec_alloc_frame();
	if (!m_pFrameI420)
        goto error;

	return TRUE;

error:
	FreeAVCodec();
	return FALSE;
error_noclose:
	FreeAVCodec(TRUE);
	return FALSE;
}

void CVideoDeviceDoc::CHttpGetFrameParseProcess::FreeAVCodec(BOOL bNoClose/*=FALSE*/)
{
	if (m_pCodecCtx)
	{
		// Close
		if (!bNoClose)
			avcodec_close_thread_safe(m_pCodecCtx);

		// Free
		av_freep(&m_pCodecCtx);
		m_pCodec = NULL;
	}
	if (m_pFrame)
	{
		av_free(m_pFrame);
		m_pFrame = NULL;
	}
	if (m_pFrameI420)
    {
		av_free(m_pFrameI420);
		m_pFrameI420 = NULL;
	}
	if (m_pImgConvertCtx)
	{
		sws_freeContext(m_pImgConvertCtx);
		m_pImgConvertCtx = NULL;
	}
	if (m_pI420Buf)
	{
		delete [] m_pI420Buf;
		m_pI420Buf = NULL;
	}
	m_dwI420BufSize = 0;
	m_dwI420ImageSize = 0;
}

BOOL CVideoDeviceDoc::CHttpGetFrameParseProcess::InitImgConvert()
{
	// Determine required buffer size and allocate buffer if necessary.
	// Make sure width and height are not 0 because otherwise
	// avpicture_get_size() returns -1 which cast to DWORD gives 4GB - 1
	if (m_pCodecCtx->width > 0 && m_pCodecCtx->height > 0)
	{
		m_dwI420ImageSize = avpicture_get_size(	PIX_FMT_YUV420P,
												m_pCodecCtx->width,
												m_pCodecCtx->height);
	}
	else
		m_dwI420ImageSize = 0;
	if (m_dwI420BufSize < m_dwI420ImageSize || m_pI420Buf == NULL)
	{
		if (m_pI420Buf)
			delete [] m_pI420Buf;
		m_pI420Buf = new BYTE[m_dwI420ImageSize + FF_INPUT_BUFFER_PADDING_SIZE];
		if (!m_pI420Buf)
			return FALSE;
		m_dwI420BufSize = m_dwI420ImageSize;
	}

	// Assign appropriate parts of buffer to image planes
	avpicture_fill((AVPicture*)m_pFrameI420,
					(unsigned __int8 *)m_pI420Buf,
					PIX_FMT_YUV420P,
					m_pCodecCtx->width,
					m_pCodecCtx->height);

	// Prepare Image Conversion Context
	m_pImgConvertCtx = sws_getCachedContext(m_pImgConvertCtx,
											m_pCodecCtx->width,		// Source Width
											m_pCodecCtx->height,	// Source Height
											m_pCodecCtx->pix_fmt,	// Source Format
											m_pCodecCtx->width,		// Destination Width
											m_pCodecCtx->height,	// Destination Height
											PIX_FMT_YUV420P,		// Destination Format
											SWS_BICUBIC,			// SWS_CPU_CAPS_MMX2, SWS_CPU_CAPS_MMX, SWS_CPU_CAPS_3DNOW
											NULL,					// No Src Filter
											NULL,					// No Dst Filter
											NULL);					// Param

	return (m_pImgConvertCtx != NULL);
}

CString CVideoDeviceDoc::AutorunGetDeviceKey(const CString& sDevicePathName)
{
	CString sSection(_T("DeviceAutorun"));
	CWinApp* pApp = ::AfxGetApp();
	CString sKey;
	CString sDev;
	for (unsigned int i = 0 ; i < MAX_DEVICE_AUTORUN_KEYS ; i++)
	{
		sKey.Format(_T("%02u"), i);
		if ((sDev = pApp->GetProfileString(sSection, sKey, _T(""))) != _T("")	&&
			sDev == sDevicePathName)
			return sKey;
	}
	return _T("");
}

void CVideoDeviceDoc::AutorunAddDevice(const CString& sDevicePathName)
{
	AutorunRemoveDevice(sDevicePathName);
	CString sSection(_T("DeviceAutorun"));
	CWinApp* pApp = ::AfxGetApp();
	CString sKey;
	for (unsigned int i = 0 ; i < MAX_DEVICE_AUTORUN_KEYS ; i++)
	{
		sKey.Format(_T("%02u"), i);
		if (pApp->GetProfileString(sSection, sKey, _T("")) == _T(""))
		{
			pApp->WriteProfileString(sSection, sKey, sDevicePathName);
			return;
		}
	}
}

void CVideoDeviceDoc::AutorunRemoveDevice(const CString& sDevicePathName)
{
	CString sSection(_T("DeviceAutorun"));
	CWinApp* pApp = ::AfxGetApp();
	CString sKey;
	CString sDev;
	for (unsigned int i = 0 ; i < MAX_DEVICE_AUTORUN_KEYS ; i++)
	{
		sKey.Format(_T("%02u"), i);
		if ((sDev = pApp->GetProfileString(sSection, sKey, _T(""))) != _T("")	&&
			sDev == sDevicePathName)
		{
			pApp->WriteProfileString(sSection, sKey, _T(""));
			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CVideoDeviceDoc diagnostics

#ifdef _DEBUG
void CVideoDeviceDoc::AssertValid() const
{
	CUImagerDoc::AssertValid();
}

void CVideoDeviceDoc::Dump(CDumpContext& dc) const
{
	CUImagerDoc::Dump(dc);
}
#endif //_DEBUG

#endif
