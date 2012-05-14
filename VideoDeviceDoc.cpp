#include "stdafx.h"
#include "uImager.h"
#include "VideoDeviceView.h"
#include "MainFrm.h"
#include "VideoDeviceDoc.h"
#include "VideoAviDoc.h"
#include "VideoAviView.h"
#include "AudioInSourceDlg.h"
#include "AssistantDlg.h"
#include "GeneralPage.h"
#include "SnapshotPage.h"
#include "VideoDevicePropertySheet.h"
#include "Quantizer.h"
#include "DxCapture.h"
#include "DxVideoFormatDlg.h"
#include "AudioFormatDlg.h"
#include "HttpVideoFormatDlg.h"
#include "AuthenticationDlg.h"
#include "SendMailConfigurationDlg.h"
#include "FTPUploadConfigurationDlg.h"
#include "FTPTransfer.h"
#include "Fourier.h"
#include "NetFrameHdr.h"
#include "NetworkPage.h"
#include "getdxver.h"
#include "BrowseDlg.h"
#include "PostDelayedMessage.h"
#include "MotionDetHelpers.h"
#include "Base64.h"
#if (_MSC_VER <= 1200)
#include "PJNMD5_vc6.h"
#else
#include "PJNMD5.h"
#endif
#include "Psapi.h"
#include "IniFile.h"
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
	ON_COMMAND(ID_CAPTURE_RECORD_PAUSE, OnCaptureRecordPause)
	ON_UPDATE_COMMAND_UI(ID_CAPTURE_RECORD_PAUSE, OnUpdateCaptureRecordPause)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_COMMAND(ID_VIEW_FIT, OnViewFit)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FIT, OnUpdateViewFit)
	ON_COMMAND(ID_VIEW_WEB, OnViewWeb)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_COMMAND(ID_CAPTURE_RESET, OnCaptureReset)
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

int CVideoDeviceDoc::CSaveFrameListThread::Work() 
{
	ASSERT(m_pDoc);
	m_bWorking = FALSE;
	CTime FirstTime(0);
	CTime LastTime(0);
	while (TRUE)
	{
		// Poll for Work
		m_pFrameList = NULL;
		m_nNumFramesToSave = 0;
		BOOL bPolling = TRUE;
		do
		{	
			while (TRUE)
			{
				// Is count >= 2?
				::EnterCriticalSection(&m_pDoc->m_csMovementDetectionsList);
				if (m_pDoc->m_MovementDetectionsList.GetCount() >= 2)
					break;
				::LeaveCriticalSection(&m_pDoc->m_csMovementDetectionsList);

				// Set that we are not working
				m_bWorking = FALSE;

				// Shutdown?
				if (::WaitForSingleObject(GetKillEvent(), MOVDET_SAVEFRAMES_POLL) == WAIT_OBJECT_0)
					return 0;
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
			FirstTime = CAVRec::CalcTime(dwFirstUpTime, RefTime, dwRefUpTime);
			if (FirstTime < LastTime)
			{
				if (::WaitForSingleObject(GetKillEvent(), 10U) == WAIT_OBJECT_0)
				{
					m_bWorking = FALSE;
					return 0;
				}
			}
			else
				break;
		}
		while ((dwRefUpTime - dwStartUpTime) <= 1100U); // be safe in case computer time has been changed
		ASSERT(FirstTime >= LastTime);
		LastTime = CAVRec::CalcTime(dwLastUpTime, RefTime, dwRefUpTime);
		CString sFirstTime(FirstTime.Format(_T("%Y_%m_%d_%H_%M_%S")));

		// Directory to Store Detection
		CString sThreadUniqueName;
		sThreadUniqueName.Format(_T("Detection%X"), ::GetCurrentThreadId());
		CString sDetectionAutoSaveDir;
		if (m_pDoc->m_bSaveSWFMovementDetection	||
			m_pDoc->m_bSaveAVIMovementDetection ||
			m_pDoc->m_bSaveAnimGIFMovementDetection)
		{
			// Check Whether Detection Dir Exists
			sDetectionAutoSaveDir = m_pDoc->m_sDetectionAutoSaveDir;
			DWORD dwAttrib = ::GetFileAttributes(sDetectionAutoSaveDir);
			if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) // Not Existing or Not A Directory
			{
				// Temp Dir To Store Files
				sDetectionAutoSaveDir = ((CUImagerApp*)::AfxGetApp())->GetAppTempDir() + sThreadUniqueName;
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
			sDetectionAutoSaveDir = ((CUImagerApp*)::AfxGetApp())->GetAppTempDir() + sThreadUniqueName;
			DWORD dwAttrib = ::GetFileAttributes(sDetectionAutoSaveDir);
			if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) // Not Existing or Not A Directory
			{
				if (!::CreateDir(sDetectionAutoSaveDir))
					::ShowLastError(FALSE);
			}
		}

		// Detection File Names
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
		
		// Note: CAVRec needs a file with .avi or .swf extention!

		// Create the Avi File
		CAVRec AVRecAvi(sAVIFileName, 0, _T(""), true); // fast encoding!
		AVRecAvi.SetInfo(	_T("Title"),
							_T("Author"),
							_T("Copyright"),
							_T("Comment"),
							_T("Album"),
							_T("Genre"));

		// Create the Swf File
		CAVRec AVRecSwf(sSWFFileName, 0, _T(""), true); // fast encoding!
		AVRecSwf.SetInfo(	_T("Title"),
							_T("Author"),
							_T("Copyright"),
							_T("Comment"),
							_T("Album"),
							_T("Genre"));
		
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
		BOOL bShowFrameTime = m_pDoc->m_bShowFrameTime;
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
#ifdef SUPPORT_GIFLIB
				GIFSaveDib.GetGif()->Close();
#endif
				::DeleteFile(sGIFTempFileName);
				AVRecSwf.Close();
				::DeleteFile(sSWFFileName);
				for (int i = 0 ; i < sJPGFileNames.GetSize() ; i++)
					::DeleteFile(sJPGFileNames[i]);
				m_bWorking = FALSE;
				return 0;
			}

			// Get Frame
			currentpos = nextpos;
			pDib = m_pFrameList->GetNext(nextpos);

			// Swf
			if (DoSaveSwf())
			{
				SWFSaveDib = *pDib;

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
					AVRecSwf.Open();
				}

				// Add Frame
				if (AVRecSwf.IsOpen())
				{
					AVRecSwf.AddFrame(	AVRecSwf.VideoStreamNumToStreamNum(ACTIVE_VIDEO_STREAM),
										&SWFSaveDib,
										false,	// No interleave for Video only
										m_pDoc->m_bVideoDetSwfDeinterlace ? true : false,
										bShowFrameTime ? true : false,
										RefTime,
										dwRefUpTime);
				}
			}

			// Avi
			if (DoSaveAvi())
			{
				AVISaveDib = *pDib;

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
					AVRecAvi.Open();
				}

				// Add Frame
				if (AVRecAvi.IsOpen())
				{
					AVRecAvi.AddFrame(	AVRecAvi.VideoStreamNumToStreamNum(ACTIVE_VIDEO_STREAM),
										&AVISaveDib,
										false,	// No interleave for Video only
										m_pDoc->m_bVideoDetDeinterlace ? true : false,
										bShowFrameTime ? true : false,
										RefTime,
										dwRefUpTime);
				}
			}

			// Jpeg
			if (DoSaveJpeg())
			{
				// FIRST movement frame
				if (sJPGFileNames.GetSize() == 0)
				{
					if (pDib->IsUserFlag())
					{
						JPGSaveDib = *pDib;
						CString sJPGFileName = SaveJpeg(&JPGSaveDib, sJPGDir, bShowFrameTime, RefTime, dwRefUpTime);
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
						CString sJPGFileName = SaveJpeg(&JPGSaveDib, sJPGDir, bShowFrameTime, RefTime, dwRefUpTime);
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
					CString sJPGFileName = SaveJpeg(&JPGSaveDib, sJPGDir, bShowFrameTime, RefTime, dwRefUpTime);
					if (sJPGFileName != _T(""))
						sJPGFileNames.Add(sJPGFileName);
				}	
			}

			// Animated Gif, because of differencing the saving is shifted by one frame
			BOOL bSaveGif = DoSaveGif();
			if (bSaveGif)
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
									bShowFrameTime,
									RefTime,
									dwRefUpTime);
				}
				// Next Frame?
				else
				{
					SaveAnimatedGif(&GIFSaveDib,
									&pDib,
									&pDibPrev,
									sGIFTempFileName,
									&bFirstGIFSave,	// First Frame To Save?
									nFrames == 1,	// Last Frame To Save?
									dDelayMul,
									dSpeedMul,
									pGIFColors,
									MOVDET_ANIMGIF_DIFF_MINLEVEL,
									bShowFrameTime,
									RefTime,
									dwRefUpTime);
				}
			}

			// Dec. Frame Count
			nFrames--;

			// Free some memory if possible
			if (!bSaveGif)
			{
				delete pDib;
				m_pFrameList->SetAt(currentpos, NULL);
			}
		}

		// Save single gif image if nothing done above:
		// this happens if m_nNumFramesToSave is 1
		if (DoSaveGif() && !::IsExistingFile(sGIFTempFileName))
		{
			SaveSingleGif(	m_pFrameList->GetHead(),
							sGIFTempFileName,
							pGIFColors,
							bShowFrameTime,
							RefTime,
							dwRefUpTime);
		}

		// Clean-Up
		if (pGIFColors)
			delete [] pGIFColors;
#ifdef SUPPORT_GIFLIB
		GIFSaveDib.GetGif()->Close();
#endif
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
			return 0;
		}
		dwMailFTPTimeMs = ::timeGetTime() - dwMailFTPTimeMs;

		// Execute Command
		if (m_pDoc->m_bExecCommandMovementDetection && m_pDoc->m_nExecModeMovementDetection == 1)
			m_pDoc->ExecCommandMovementDetection();

		// Delete Files if not wanted
		if (!m_pDoc->m_bSaveAVIMovementDetection)
			::DeleteFile(sAVIFileName);
		if (!m_pDoc->m_bSaveAnimGIFMovementDetection)
			::DeleteFile(sGIFFileName);
		if (!m_pDoc->m_bSaveSWFMovementDetection)
			::DeleteFile(sSWFFileName);
		for (int i = 0 ; i < sJPGFileNames.GetSize() ; i++)
			::DeleteFile(sJPGFileNames[i]);

		// Save time calculation
		DWORD dwSaveTimeMs = ::timeGetTime() - dwStartUpTime;
		DWORD dwFramesTimeMs = dwLastUpTime - dwFirstUpTime;
		if (dwFramesTimeMs >= 2000U) // Check only if at least 2 sec of frames
		{
			CString sMsg;
			if (dwFramesTimeMs < dwSaveTimeMs)
			{
				sMsg.Format(_T("%s, attention cannot realtime save the detections: SaveTime=%0.1fs > FramesTime=%0.1fs (MailFTP=%0.1fs)\n"),
							m_pDoc->GetDeviceName(), (double)dwSaveTimeMs / 1000.0, (double)dwFramesTimeMs / 1000.0,
							(double)dwMailFTPTimeMs / 1000.0);
				TRACE(sMsg);
				::LogLine(sMsg);
			}
			else if (m_pDoc->m_nDetectionLevel == 100)
			{
				sMsg.Format(_T("%s, realtime saving the detections is ok: SaveTime=%0.1fs < FramesTime=%0.1fs (MailFTP=%0.1fs)\n"),
							m_pDoc->GetDeviceName(), (double)dwSaveTimeMs / 1000.0, (double)dwFramesTimeMs / 1000.0,
							(double)dwMailFTPTimeMs / 1000.0);
				TRACE(sMsg);
				::LogLine(sMsg);
			}
		}
	}
	ASSERT(FALSE); // should never end up here...
	m_bWorking = FALSE;
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
	CString sSubject(Time.Format(_T("Movement Detection on %A, %d %B %Y at %H:%M:%S")));
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
		return FALSE;
	else
		return TRUE;
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
				result = CVideoDeviceDoc::FTPUpload(&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
													sAVIFileName, sUploadDir + _T("/") + ::GetShortFileName(sAVIFileName));
				break;

		case CVideoDeviceDoc::FILES_TO_UPLOAD_GIF :
				result = CVideoDeviceDoc::FTPUpload(&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
													sGIFFileName, sUploadDir + _T("/") + ::GetShortFileName(sGIFFileName));
				break;

		case CVideoDeviceDoc::FILES_TO_UPLOAD_SWF :
				result = CVideoDeviceDoc::FTPUpload(&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
													sSWFFileName, sUploadDir + _T("/") + ::GetShortFileName(sSWFFileName));
				break;

		case CVideoDeviceDoc::FILES_TO_UPLOAD_AVI_GIF :
				result = CVideoDeviceDoc::FTPUpload(&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
													sAVIFileName, sUploadDir + _T("/") + ::GetShortFileName(sAVIFileName));
				if (result == 1)
					result = CVideoDeviceDoc::FTPUpload(&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
														sGIFFileName, sUploadDir + _T("/") + ::GetShortFileName(sGIFFileName));
				break;

		case CVideoDeviceDoc::FILES_TO_UPLOAD_SWF_GIF :
				result = CVideoDeviceDoc::FTPUpload(&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
													sSWFFileName, sUploadDir + _T("/") + ::GetShortFileName(sSWFFileName));
				if (result == 1)
					result = CVideoDeviceDoc::FTPUpload(&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
														sGIFFileName, sUploadDir + _T("/") + ::GetShortFileName(sGIFFileName));
				break;

		case CVideoDeviceDoc::FILES_TO_UPLOAD_AVI_SWF_GIF :
				result = CVideoDeviceDoc::FTPUpload(&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
													sAVIFileName, sUploadDir + _T("/") + ::GetShortFileName(sAVIFileName));
				if (result == 1)
					result = CVideoDeviceDoc::FTPUpload(&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
														sSWFFileName, sUploadDir + _T("/") + ::GetShortFileName(sSWFFileName));
				if (result == 1)
					result = CVideoDeviceDoc::FTPUpload(&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
														sGIFFileName, sUploadDir + _T("/") + ::GetShortFileName(sGIFFileName));
				break;

		default :
				break;
	}

	// Do Exit?
	if (result == -1)
		return FALSE;
	else
		return TRUE;
}

CString CVideoDeviceDoc::CSaveFrameListThread::SaveJpeg(CDib* pDib,
														CString sJPGDir,
														BOOL bShowFrameTime,
														const CTime& RefTime,
														DWORD dwRefUpTime)
{
	// Calc. time and create file name
	CTime Time = CAVRec::CalcTime(pDib->GetUpTime(), RefTime, dwRefUpTime);
	CString sTime(Time.Format(_T("%Y_%m_%d_%H_%M_%S")));
	sJPGDir += _T("det_") + sTime + _T(".jpg");

	// Do not overwrite previous jpeg save
	if (::IsExistingFile(sJPGDir))
		return _T("");

	// Decompress to 32bpp and add frame time stamp
	if (pDib->IsCompressed() || pDib->GetBitCount() <= 8)
		pDib->Decompress(32);
	if (bShowFrameTime)
		CAVRec::AddFrameTime(pDib, RefTime, dwRefUpTime);

	// Save
	if (pDib->SaveJPEG(sJPGDir))
		return sJPGDir;
	else
		return _T("");
}

void CVideoDeviceDoc::CSaveFrameListThread::AnimatedGifInit(	RGBQUAD* pGIFColors,
																double& dDelayMul,
																double& dSpeedMul,
																double dCalcFrameRate,
																BOOL bShowFrameTime,
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
				if (!pDibForPalette1 && p->IsUserFlag())
					pDibForPalette1 = p;
				if (pDibForPalette1 && (nFirstCountDown > 0))
				{
					nFirstCountDown--;
					pDibForPalette1 = p;
				}
				if (nMiddleElementCount-- == 0)
					pDibForPalette2 = p;
				if (p->IsUserFlag())
					pDibForPalette3 = p;
			}
		}
		nFrameCountDown--;
	}
	
	// Make sure the 3 image pointers are ok
	if (!pDibForPalette1)
		pDibForPalette1 = m_pFrameList->GetHead();
	DibForPalette1 = *pDibForPalette1;
	if (DibForPalette1.IsCompressed() || DibForPalette1.GetBitCount() <= 8)
		DibForPalette1.Decompress(32);
	DibForPalette1.StretchBits(m_pDoc->m_dwAnimatedGifWidth, m_pDoc->m_dwAnimatedGifHeight);
	if (!pDibForPalette2)
		pDibForPalette2 = m_pFrameList->GetHead();
	DibForPalette2 = *pDibForPalette2;
	if (DibForPalette2.IsCompressed() || DibForPalette2.GetBitCount() <= 8)
		DibForPalette2.Decompress(32);
	DibForPalette2.StretchBits(m_pDoc->m_dwAnimatedGifWidth, m_pDoc->m_dwAnimatedGifHeight);
	if (!pDibForPalette3)
		pDibForPalette3 = m_pFrameList->GetHead();
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

	// Add Frame Time to include the colors of the time and date
	if (bShowFrameTime)
		CAVRec::AddFrameTime(&DibForPalette, RefTime, dwRefUpTime);
	
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
															BOOL bShowFrameTime,
															const CTime& RefTime,
															DWORD dwRefUpTime)
{
#ifdef SUPPORT_GIFLIB
	if (pDib && pGIFColors)
	{
		// Make sure we have a true RGB format
		if (pDib->IsCompressed() || pDib->GetBitCount() <= 8)
			pDib->Decompress(32);

		// Resize
		pDib->StretchBits(m_pDoc->m_dwAnimatedGifWidth, m_pDoc->m_dwAnimatedGifHeight);

		// Add Frame Time
		if (bShowFrameTime)
			CAVRec::AddFrameTime(pDib, RefTime, dwRefUpTime);

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
#endif
		return FALSE;
}

__forceinline void CVideoDeviceDoc::CSaveFrameListThread::To255Colors(	CDib* pDib,
																		RGBQUAD* pGIFColors,
																		BOOL bShowFrameTime,
																		const CTime& RefTime,
																		DWORD dwRefUpTime)
{
	// Make sure we have a true RGB format
	if (pDib->IsCompressed() || pDib->GetBitCount() <= 8)
		pDib->Decompress(32);

	// Resize
	pDib->StretchBits(m_pDoc->m_dwAnimatedGifWidth, m_pDoc->m_dwAnimatedGifHeight);

	// Add Frame Time
	if (bShowFrameTime)
		CAVRec::AddFrameTime(pDib, RefTime, dwRefUpTime);

	// Convert to 8 bpp
	if (pDib->GetBitCount() > 8)
	{
		pDib->CreatePaletteFromColors(255, pGIFColors); // One index for transparency!
		pDib->ConvertTo8bitsErrDiff(pDib->GetPalette());
	}
}

BOOL CVideoDeviceDoc::CSaveFrameListThread::SaveAnimatedGif(CDib* pGIFSaveDib,
															CDib** ppGIFDib,
															CDib** ppGIFDibPrev,
															const CString& sGIFFileName,
															BOOL* pbFirstGIFSave,
															BOOL bLastGIFSave,
															double dDelayMul,
															double dSpeedMul,
															RGBQUAD* pGIFColors,
															int nDiffMinLevel,
															BOOL bShowFrameTime,
															const CTime& RefTime,
															DWORD dwRefUpTime)
{
	BOOL res = FALSE;

	// Check
	if (!pGIFColors)
		return FALSE;

#ifdef SUPPORT_GIFLIB

	// Is First Frame To Save?
	if (*pbFirstGIFSave)
	{
		// Convert to 255 colors
		To255Colors(*ppGIFDibPrev, pGIFColors, bShowFrameTime, RefTime, dwRefUpTime);
		
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
		*ppGIFDibPrev = *ppGIFDib;

		// If it is also the last frame,
		// this happens if we have only 2 frames to save
		if (bLastGIFSave)
		{
			// Convert to 255 colors and save
			To255Colors(*ppGIFDib, pGIFColors, bShowFrameTime, RefTime, dwRefUpTime);
			pGIFSaveDib->GetGif()->SetDispose(GIF_DISPOSE_RESTORE);
			pGIFSaveDib->GetGif()->SetDelay(MOVDET_ANIMGIF_LAST_FRAME_DELAY);
			(*ppGIFDib)->DiffTransp8(pGIFSaveDib, nDiffMinLevel, 255);
			res = pGIFSaveDib->SaveNextGIF(	*ppGIFDib,
											NULL,
											TRUE,
											this);
		}
	}
	// Is Last Frame?
	else if (bLastGIFSave)
	{
		// Convert to 255 colors and save
		To255Colors(*ppGIFDibPrev, pGIFColors, bShowFrameTime, RefTime, dwRefUpTime);
		pGIFSaveDib->GetGif()->SetDispose(GIF_DISPOSE_RESTORE);
		pGIFSaveDib->GetGif()->SetDelay(MAX(100, Round((double)((*ppGIFDib)->GetUpTime() - (*ppGIFDibPrev)->GetUpTime()) / dSpeedMul)));
		(*ppGIFDibPrev)->DiffTransp8(pGIFSaveDib, nDiffMinLevel, 255);
		res = pGIFSaveDib->SaveNextGIF(	*ppGIFDibPrev,
										NULL,
										TRUE,
										this);
		
		// Convert to 255 colors and save
		To255Colors(*ppGIFDib, pGIFColors, bShowFrameTime, RefTime, dwRefUpTime);
		pGIFSaveDib->GetGif()->SetDispose(GIF_DISPOSE_RESTORE);
		pGIFSaveDib->GetGif()->SetDelay(MOVDET_ANIMGIF_LAST_FRAME_DELAY);
		(*ppGIFDib)->DiffTransp8(pGIFSaveDib, nDiffMinLevel, 255);
		res = pGIFSaveDib->SaveNextGIF(	*ppGIFDib,
										NULL,
										TRUE,
										this);
	}
	// Middle Frame?
	else if ((int)((*ppGIFDib)->GetUpTime() - (*ppGIFDibPrev)->GetUpTime()) >= Round(dDelayMul * MOVDET_ANIMGIF_DELAY))
	{
		// Convert to 255 colors and save
		To255Colors(*ppGIFDibPrev, pGIFColors, bShowFrameTime, RefTime, dwRefUpTime);
		pGIFSaveDib->GetGif()->SetDispose(GIF_DISPOSE_RESTORE);
		pGIFSaveDib->GetGif()->SetDelay(MAX(100, Round((double)((*ppGIFDib)->GetUpTime() - (*ppGIFDibPrev)->GetUpTime()) / dSpeedMul)));
		(*ppGIFDibPrev)->DiffTransp8(pGIFSaveDib, nDiffMinLevel, 255);
		res = pGIFSaveDib->SaveNextGIF(	*ppGIFDibPrev,
										NULL,
										TRUE,
										this);

		// Update Pointer
		*ppGIFDibPrev = *ppGIFDib;
	}
	else
		res = TRUE; // Skip Frame

#endif

	return res;
}

int CVideoDeviceDoc::CSaveSnapshotFTPThread::Work() 
{
	CFTPTransfer FTP(this);
	if (m_sLocalThumbFileName != _T("") && m_sRemoteThumbFileName != _T(""))
	{
		CVideoDeviceDoc::FTPUpload(	&FTP, &m_Config,
									m_sLocalThumbFileName,
									m_sRemoteThumbFileName);
		::DeleteFile(m_sLocalThumbFileName);
	}
	if (m_sLocalFileName != _T("") && m_sRemoteFileName != _T(""))
	{
		CVideoDeviceDoc::FTPUpload(	&FTP, &m_Config,
									m_sLocalFileName,
									m_sRemoteFileName);
		::DeleteFile(m_sLocalFileName);
	}
	return 0;
}

__forceinline void CVideoDeviceDoc::CSaveSnapshotThread::SWFFreeCopyFtp(BOOL bFtp/*=FALSE*/)
{
	// Close files and free
	if (m_pAVRecSwf)
		delete m_pAVRecSwf;
	if (m_pAVRecThumbSwf)
		delete m_pAVRecThumbSwf;

	// Copy
	if (m_sSnapshotAutoSaveDir != _T(""))
	{
		if (m_pAVRecSwf && m_sSWFTempFileName != _T("") && m_sSWFFileName != _T(""))
			::CopyFile(m_sSWFTempFileName, m_sSWFFileName, FALSE);
		if (m_pAVRecThumbSwf && m_sSWFTempThumbFileName != _T("") && m_sSWFThumbFileName != _T(""))
			::CopyFile(m_sSWFTempThumbFileName, m_sSWFThumbFileName, FALSE);
	}

	// Ftp
	if (bFtp && m_pSaveSnapshotFTPThread)
	{
		if (m_pSaveSnapshotFTPThread->IsAlive())
			m_pSaveSnapshotFTPThread->Kill();
		CString sUploadDir;
		int nYear, nMonth, nDay;
		if (m_pAVRecSwf && m_sSWFTempFileName != _T("") && m_sSWFFileName != _T(""))
		{
			CString sSWFShortFileName = ::GetShortFileName(m_sSWFFileName);
			_stscanf(sSWFShortFileName, _T("shot_%d_%d_%d.swf"), &nYear, &nMonth, &nDay);
			sUploadDir.Format(_T("%d/%02d/%02d"), nYear, nMonth, nDay);
			m_pSaveSnapshotFTPThread->m_sLocalFileName = m_sSWFTempFileName;
			m_pSaveSnapshotFTPThread->m_sRemoteFileName = sUploadDir + _T("/") + sSWFShortFileName;
		}
		if (m_pAVRecThumbSwf && m_sSWFTempThumbFileName != _T("") && m_sSWFThumbFileName != _T(""))
		{
			CString sSWFThumbShortFileName = ::GetShortFileName(m_sSWFThumbFileName);
			_stscanf(sSWFThumbShortFileName, _T("shot_%d_%d_%d.swf"), &nYear, &nMonth, &nDay);
			sUploadDir.Format(_T("%d/%02d/%02d"), nYear, nMonth, nDay);
			m_pSaveSnapshotFTPThread->m_sLocalThumbFileName = m_sSWFTempThumbFileName;
			m_pSaveSnapshotFTPThread->m_sRemoteThumbFileName = sUploadDir + _T("/") + sSWFThumbShortFileName;
		}
		m_pSaveSnapshotFTPThread->m_Config = m_Config;
		m_pSaveSnapshotFTPThread->Start();
	}
	else
	{
		if (m_pAVRecSwf && m_sSWFTempFileName != _T(""))
			::DeleteFile(m_sSWFTempFileName);
		if (m_pAVRecThumbSwf && m_sSWFTempThumbFileName != _T(""))
			::DeleteFile(m_sSWFTempThumbFileName);
	}

	// Reset
	m_pAVRecSwf = NULL;
	m_pAVRecThumbSwf = NULL;
}

int CVideoDeviceDoc::CSaveSnapshotThread::Work() 
{
	// Free and copy and ev. ftp the swf files
	if (m_Time >= m_NextRecTime &&
		(m_pAVRecSwf || m_pAVRecThumbSwf))
		SWFFreeCopyFtp(m_bSnapshotHistorySwfFtp);

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
		DibThumb.StretchBits(m_nSnapshotThumbWidth, m_nSnapshotThumbHeight, &m_Dib);
	}

	// Save history as Swf to Temp
	if (m_bSnapshotHistorySwf)
	{
		if (!m_pAVRecSwf)
		{
			CTime TempTime = m_Time + CTimeSpan(1, 0, 0, 0);	// + 1 day
			m_NextRecTime = CTime(	TempTime.GetYear(),
									TempTime.GetMonth(),
									TempTime.GetDay(),
									0, 0, 0);					// Back to midnight
			m_sSWFFileName = MakeSwfHistoryFileName();
			m_sSWFTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sSWFFileName);
			m_pAVRecSwf = new CAVRec(m_sSWFTempFileName);
		}
		if (m_pAVRecSwf)
		{
			// Setup
			m_pAVRecSwf->SetInfo(	_T("Title"),
									_T("Author"),
									_T("Copyright"),
									_T("Comment"),
									_T("Album"),
									_T("Genre"));
			CDib SWFSaveDib = m_Dib;

			// Open
			if (!m_pAVRecSwf->IsOpen())
			{
				AVRational FrameRate = av_d2q(m_dSnapshotHistoryFrameRate, MAX_SIZE_FOR_RATIONAL);
				BITMAPINFOHEADER DstBmi;
				memset(&DstBmi, 0, sizeof(BITMAPINFOHEADER));
				DstBmi.biSize = sizeof(BITMAPINFOHEADER);
				DstBmi.biWidth = SWFSaveDib.GetWidth();
				DstBmi.biHeight = SWFSaveDib.GetHeight();
				DstBmi.biPlanes = 1;
				DstBmi.biCompression = FCC('FLV1');								// FLV1, need Flash 6 to play it
				m_pAVRecSwf->AddVideoStream(SWFSaveDib.GetBMI(),				// Source Video Format
											(LPBITMAPINFO)(&DstBmi),			// Destination Video Format
											FrameRate.num,						// Rate
											FrameRate.den,						// Scale
											0,									// Not using bitrate
											DEFAULT_KEYFRAMESRATE,				// Keyframes Rate				
											m_fSnapshotVideoCompressorQuality);	// 0.0f use bitrate, 2.0f best quality, 31.0f worst quality
				m_pAVRecSwf->Open();
			}

			// Add Frame
			if (m_pAVRecSwf->IsOpen())
			{
				m_pAVRecSwf->AddFrame(	m_pAVRecSwf->VideoStreamNumToStreamNum(ACTIVE_VIDEO_STREAM),
										&SWFSaveDib,
										false,	// No interleave for Video only
										m_bSnapshotHistoryDeinterlace ? true : false,
										m_bShowFrameTime ? true : false,
										m_Time,
										dwUpTime);
			}
		}
		if (m_bSnapshotThumb)
		{
			if (!m_pAVRecThumbSwf)
			{
				m_sSWFThumbFileName = ::GetFileNameNoExt(m_sSWFFileName) + _T("_thumb.swf");
				m_sSWFTempThumbFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), m_sSWFThumbFileName);
				m_pAVRecThumbSwf = new CAVRec(m_sSWFTempThumbFileName);
			}
			if (m_pAVRecThumbSwf)
			{
				// Setup
				m_pAVRecThumbSwf->SetInfo(	_T("Title"),
											_T("Author"),
											_T("Copyright"),
											_T("Comment"),
											_T("Album"),
											_T("Genre"));
				CDib SWFSaveDib = DibThumb;

				// Open
				if (!m_pAVRecThumbSwf->IsOpen())
				{
					AVRational FrameRate = av_d2q(m_dSnapshotHistoryFrameRate, MAX_SIZE_FOR_RATIONAL);
					BITMAPINFOHEADER DstBmi;
					memset(&DstBmi, 0, sizeof(BITMAPINFOHEADER));
					DstBmi.biSize = sizeof(BITMAPINFOHEADER);
					DstBmi.biWidth = SWFSaveDib.GetWidth();
					DstBmi.biHeight = SWFSaveDib.GetHeight();
					DstBmi.biPlanes = 1;
					DstBmi.biCompression = FCC('FLV1');									// FLV1, need Flash 6 to play it
					m_pAVRecThumbSwf->AddVideoStream(SWFSaveDib.GetBMI(),				// Source Video Format
													(LPBITMAPINFO)(&DstBmi),			// Destination Video Format
													FrameRate.num,						// Rate
													FrameRate.den,						// Scale
													0,									// Not using bitrate
													DEFAULT_KEYFRAMESRATE,				// Keyframes Rate				
													m_fSnapshotVideoCompressorQuality);	// 0.0f use bitrate, 2.0f best quality, 31.0f worst quality
					m_pAVRecThumbSwf->Open();
				}

				// Add Frame
				if (m_pAVRecThumbSwf->IsOpen())
				{
					m_pAVRecThumbSwf->AddFrame(	m_pAVRecThumbSwf->VideoStreamNumToStreamNum(ACTIVE_VIDEO_STREAM),
												&SWFSaveDib,
												false,	// No interleave for Video only
												m_bSnapshotHistoryDeinterlace ? true : false,
												m_bShowFrameTime ? true : false,
												m_Time,
												dwUpTime);
				}
			}
		}
	}

	// Add frame time
	if (m_bShowFrameTime)
		CAVRec::AddFrameTime(&m_Dib, m_Time, dwUpTime);

	// Save Thumb to Temp
	if (m_bSnapshotThumb)
	{
		// Add frame time
		if (m_bShowFrameTime)
			CAVRec::AddFrameTime(&DibThumb, m_Time, dwUpTime);

		// Save
		sTempThumbFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(),
										::GetFileNameNoExt(sHistoryFileName) + _T("_thumb.jpg"));
		DibThumb.SaveJPEG(sTempThumbFileName, m_nSnapshotCompressionQuality);
	}

	// Save Full-size to Temp
	sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sHistoryFileName);
	m_Dib.SaveJPEG(sTempFileName, m_nSnapshotCompressionQuality);
	
	// Copy from Temp and Ftp Upload
	if (m_bSnapshotLiveJpeg)
	{
		CString sLiveFileName;
		if (m_sSnapshotAutoSaveDir != _T(""))
		{
			sLiveFileName = m_sSnapshotAutoSaveDir;
			sLiveFileName.TrimRight(_T('\\'));
			sLiveFileName = sLiveFileName + _T("\\") + DEFAULT_SNAPSHOT_LIVE_FILE;
			::CopyFile(sTempFileName, sLiveFileName, FALSE);
		}
		else
			sLiveFileName = DEFAULT_SNAPSHOT_LIVE_FILE;
		if (m_bSnapshotLiveJpegFtp) // FTP Upload
		{
			int result;
			CFTPTransfer FTP(this);
			result = CVideoDeviceDoc::FTPUpload(&FTP, &m_Config,
												sTempFileName, DEFAULT_SNAPSHOT_LIVE_FILE);
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
			CString sLiveThumbFileName = ::GetFileNameNoExt(sLiveFileName) + _T("_thumb.jpg");
			if (m_sSnapshotAutoSaveDir != _T(""))
				::CopyFile(sTempThumbFileName, sLiveThumbFileName, FALSE);
			if (m_bSnapshotLiveJpegFtp) // FTP Upload
			{
				int result;
				CFTPTransfer FTP(this);
				result = CVideoDeviceDoc::FTPUpload(&FTP, &m_Config,
													sTempThumbFileName, ::GetShortFileName(sLiveThumbFileName));
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
			result = CVideoDeviceDoc::FTPUpload(&FTP, &m_Config,
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
				result = CVideoDeviceDoc::FTPUpload(&FTP, &m_Config,
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

__forceinline CString CVideoDeviceDoc::CSaveSnapshotThread::MakeSwfHistoryFileName()
{
	CString sYearMonthDayDir(_T(""));

	// Snapshots time
	CString sTime = m_Time.Format(_T("%Y_%m_%d"));

	// Adjust Directory Name
	CString sSnapshotDir = m_sSnapshotAutoSaveDir;
	sSnapshotDir.TrimRight(_T('\\'));

	// Create directory if necessary
	if (sSnapshotDir != _T("") && m_bSnapshotHistorySwf)
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
#if (_MSC_VER > 1200)
	pMessage->ParseMultipleRecipients(pSendMailConfiguration->m_sTo, pMessage->m_To);
#else
	pMessage->AddMultipleRecipients(pSendMailConfiguration->m_sTo, CPJNSMTPMessage::TO);
#endif
	if (!pSendMailConfiguration->m_sCC.IsEmpty())
	{
#if (_MSC_VER > 1200)
		pMessage->ParseMultipleRecipients(pSendMailConfiguration->m_sCC, pMessage->m_CC);
#else
		pMessage->AddMultipleRecipients(pSendMailConfiguration->m_sCC, CPJNSMTPMessage::CC);
#endif
	}
	if (!pSendMailConfiguration->m_sBCC.IsEmpty()) 
	{
#if (_MSC_VER > 1200)
		pMessage->ParseMultipleRecipients(pSendMailConfiguration->m_sBCC, pMessage->m_BCC);
#else
		pMessage->AddMultipleRecipients(pSendMailConfiguration->m_sBCC, CPJNSMTPMessage::BCC);
#endif
	}
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
			m_pThread->SetSendMailProgress(0);
		return (m_bDoExit == false);
	}
	else
		return TRUE;
}

// Return Values
// -1 : Do Exit Thread
// 0  : Error Sending Email
// 1  : Ok
int CVideoDeviceDoc::CSaveFrameListThread::SendMailMessage(CPJNSMTPMessage* pMessage)
{
	// Check
	if (!pMessage)
		return 0;

	// Init Connection class
	CVideoDeviceDoc::CSaveFrameListSMTPConnection connection(this);

	// Auto connect to the internet?
	if (m_pDoc->m_MovDetSendMailConfiguration.m_bAutoDial)
		connection.ConnectToInternet();

	CString sHost;
	BOOL bSend = TRUE;
	if (m_pDoc->m_MovDetSendMailConfiguration.m_bDNSLookup)
	{
#if (_MSC_VER > 1200)
		if (pMessage->m_To.GetSize() == 0)
#else
		if (pMessage->GetNumberOfRecipients() == 0)
#endif
		{
			CString sMsg;
			sMsg.Format(_T("%s, at least one recipient must be specified to use the DNS lookup option\n"), m_pDoc->GetDeviceName());
			TRACE(sMsg);
			::LogLine(sMsg);
			bSend = FALSE;
		}
		else
		{
#if (_MSC_VER > 1200)
			CString sAddress(pMessage->m_To.ElementAt(0).m_sEmailAddress);
#else
			CString sAddress(pMessage->GetRecipient(0)->m_sEmailAddress);
#endif
			int nAmpersand = sAddress.Find(_T("@"));
			if (nAmpersand == -1)
			{
				CString sMsg;
				sMsg.Format(_T("%s, unable to determine the domain for the email address %s\n"), m_pDoc->GetDeviceName(), sAddress);
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
																m_pDoc->GetDeviceName(), sDomain, GetLastError());
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
#if (_MSC_VER > 1200)
		connection.SetBindAddress(m_pDoc->m_MovDetSendMailConfiguration.m_sBoundIP);
#else
		connection.SetBoundAddress(m_pDoc->m_MovDetSendMailConfiguration.m_sBoundIP);
#endif
		if (m_pDoc->m_MovDetSendMailConfiguration.m_sUsername == _T("") &&
			m_pDoc->m_MovDetSendMailConfiguration.m_sPassword == _T(""))
		{
			connection.Connect(	sHost,
								CPJNSMTPConnection::AUTH_NONE,
								m_pDoc->m_MovDetSendMailConfiguration.m_sUsername,
								m_pDoc->m_MovDetSendMailConfiguration.m_sPassword,
								m_pDoc->m_MovDetSendMailConfiguration.m_nPort
#if !defined (CPJNSMTP_NOSSL) && (_MSC_VER > 1200)
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
#if !defined (CPJNSMTP_NOSSL) && (_MSC_VER > 1200)
								, m_pDoc->m_MovDetSendMailConfiguration.m_ConnectionType
#endif
								);
		}
		connection.SendMessage(*pMessage);
	}

	// Auto disconnect from the internet
	if (m_pDoc->m_MovDetSendMailConfiguration.m_bAutoDial)
		connection.CloseInternetConnection();

	// Sending Interrupted?
	if (connection.m_bDoExit)
	{
		connection.Disconnect(FALSE);	// Disconnect no Gracefully,
										// otherwise the thread blocks
										// long time to get a answer!
		return -1;
	}
	else
		return 1;
}

// Return Values
// -1 : Do Exit Thread
// 0  : Error Sending Email
// 1  : Ok
int CVideoDeviceDoc::CSaveFrameListThread::SendMail(const CStringArray& sFiles) 
{
	int res = 0;
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
		try
		{
			if (m_pDoc->m_MovDetSendMailConfiguration.m_bHTML == FALSE)
			{
				m_pDoc->m_MovDetSendMailConfiguration.m_bMime = FALSE;
				m_pDoc->m_MovDetSendMailConfiguration.m_sFiles = _T("");
				m_pDoc->m_MovDetSendMailConfiguration.m_sBody = _T("Movement Detection!");

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
				if (sFiles.GetSize() > 0)
					sHtml += _T("<p><b>Movement Detection:</b></p>");
				else
					sHtml += _T("<p><b>Movement Detection!</b></p>");
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
			res = SendMailMessage(pMessage);

			// Clean-up
			if (pMessage)
				delete pMessage;
		}
		catch (CPJNSMTPException* pEx)
		{
			// Clean-up
			if (pMessage)
				delete pMessage;

			// Display the error
			CString sMsg;
			sMsg.Format(_T("%s, an error occured sending the message, Error:%x\nDescription:%s\n"),
						m_pDoc->GetDeviceName(),
						pEx->m_hr,
						pEx->GetErrorMessage());
			TRACE(sMsg);
			::LogLine(sMsg);
			pEx->Delete();
			return 0;
		}

		return res;
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
		pFTP->m_bShowMessageBoxOnError = FALSE; // Be Quiet!
		if (pConfig->m_sRemoteDir.IsEmpty())
			pFTP->m_sRemoteFile = sRemoteFileName;
		else
		{
			// Adjust Remote File Name
			if (sRemoteFileName.GetAt(0) == _T('/') || sRemoteFileName.GetAt(0) == _T('\\'))
				sRemoteFileName.Delete(0);

			// Adjust Remote Dir Path
			CString sRemoteDir = pConfig->m_sRemoteDir;
			sRemoteDir.TrimRight(_T('\\'));
			
			pFTP->m_sRemoteFile = sRemoteDir + _T("/") + sRemoteFileName;
		}
		pFTP->m_sLocalFile = sLocalFileName;
		pFTP->m_sServer = pConfig->m_sHost;
		pFTP->m_nPort = pConfig->m_nPort;
		pFTP->m_bDownload = FALSE;
		pFTP->m_bBinary = pConfig->m_bBinary;
		pFTP->m_bPromptOverwrite = FALSE;
		pFTP->m_dbLimit = 0.0;	// For BANDWIDTH throttling, the value in Bytes / Second to limit the connection to
		pFTP->m_bPasv = pConfig->m_bPasv;
		pFTP->m_bUsePreconfig = TRUE;	// Should preconfigured settings be used i.e. take proxy settings etc from the control panel
		pFTP->m_bUseProxy = pConfig->m_bProxy;
		pFTP->m_sProxy = pConfig->m_sProxy;
		pFTP->m_dwConnectionTimeout = pConfig->m_dwConnectionTimeout;
		if (!pConfig->m_sUsername.IsEmpty())
		{
			pFTP->m_sUserName = pConfig->m_sUsername;
			pFTP->m_sPassword = pConfig->m_sPassword;
		}

		return pFTP->Transfer();
	}
}

CVideoDeviceDoc::CCaptureAudioThread::CCaptureAudioThread() 
{
	m_pDoc = NULL;
	memset(&m_WaveHeader[0], 0, sizeof(m_WaveHeader[0]));
	memset(&m_WaveHeader[1], 0, sizeof(m_WaveHeader[1]));

	// Create Input Event
	m_hWaveInEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hRestartEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEventArray[0] = GetKillEvent();
	m_hEventArray[1] = m_hRestartEvent;
	m_hEventArray[2] = m_hWaveInEvent;
	m_hWaveIn = NULL;
	m_MeanLevelTime = (0,0,0,0);
	m_bSmallBuffers = TRUE;

	// Audio Format set Default to: Mono , 11025 Hz , 8 bits
	m_pSrcWaveFormat = (WAVEFORMATEX*)new BYTE[sizeof(WAVEFORMATEX)];
	if (m_pSrcWaveFormat)
	{
		m_pSrcWaveFormat->wFormatTag = WAVE_FORMAT_PCM;
		m_pSrcWaveFormat->nChannels = 1;
		m_pSrcWaveFormat->nSamplesPerSec = 11025;
		m_pSrcWaveFormat->nAvgBytesPerSec = 11025;
		m_pSrcWaveFormat->nBlockAlign = 1;
		m_pSrcWaveFormat->wBitsPerSample = 8;
		m_pSrcWaveFormat->cbSize = 0;
	}
	m_pDstWaveFormat = (WAVEFORMATEX*)new BYTE[sizeof(WAVEFORMATEX)];
	if (m_pDstWaveFormat)
	{
		m_pDstWaveFormat->wFormatTag = WAVE_FORMAT_PCM;
		m_pDstWaveFormat->nChannels = 1;
		m_pDstWaveFormat->nSamplesPerSec = 11025;
		m_pDstWaveFormat->nAvgBytesPerSec = 11025;
		m_pDstWaveFormat->nBlockAlign = 1;
		m_pDstWaveFormat->wBitsPerSample = 8;
		m_pDstWaveFormat->cbSize = 0;
	}

	// ACM
	m_pUncompressedBuf[0] = NULL;
	m_pUncompressedBuf[0] = NULL;
	m_dwUncompressedBufSize = 0;
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
	if (m_pUncompressedBuf[0])
	{
		delete [] m_pUncompressedBuf[0];
		m_pUncompressedBuf[0] = NULL;
		m_pUncompressedBuf[1] = NULL;
	}
	::CloseHandle(m_hRestartEvent);
	::CloseHandle(m_hWaveInEvent);
	m_hWaveInEvent = NULL;
}

void CVideoDeviceDoc::CCaptureAudioThread::AudioInSourceDialog()
{
	CAudioInSourceDlg dlg(m_pDoc->m_dwCaptureAudioDeviceID);
	if (dlg.DoModal() == IDOK)
	{
		if (dlg.m_uiDeviceID != m_pDoc->m_dwCaptureAudioDeviceID)
		{
			SetDeviceID(dlg.m_uiDeviceID);
			if (m_pDoc->m_bCaptureAudio)
			{
				Kill();
				m_Mixer.Close();
				Start();
			}
		}
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

int CVideoDeviceDoc::CCaptureAudioThread::Work() 
{
	DWORD Event;
	bool bExit, bDoRestart;

	if (!m_pDoc)
		return 0;
	
	do
	{
		// Init Vars
		bExit = false;
		bDoRestart = false;

		// Open Audio
		if (!OpenInAudio())
			return 0;

		// Uncompressed Buffers
		if (m_pUncompressedBuf[0])
			delete [] m_pUncompressedBuf[0];
		LPBYTE p = new BYTE[2 * (m_dwUncompressedBufSize + FF_INPUT_BUFFER_PADDING_SIZE)];
		m_pUncompressedBuf[0] = p;
		m_pUncompressedBuf[1] = p + m_dwUncompressedBufSize + FF_INPUT_BUFFER_PADDING_SIZE;

		// Reset The Open Event
		::ResetEvent(m_hWaveInEvent);

		// Start Buffering
		m_nWaveInToggle = 0;	
		if (!DataInAudio(&m_pUncompressedBuf[m_nWaveInToggle][0], m_dwUncompressedBufSize))
			bExit = true;
		else if (!DataInAudio(&m_pUncompressedBuf[m_nWaveInToggle][0], m_dwUncompressedBufSize))
			bExit = true;

		while (!bExit)
		{
			Event = ::WaitForMultipleObjects(3, m_hEventArray, FALSE, INFINITE);
			switch (Event)
			{
				// Shutdown Event
				case WAIT_OBJECT_0 :		bExit = true;
											break;

				// Restart Event
				case WAIT_OBJECT_0 + 1 :	::ResetEvent(m_hRestartEvent);
											bExit = true;
											bDoRestart = true;
											break;

				// Wave In Event
				case WAIT_OBJECT_0 + 2 :	::ResetEvent(m_hWaveInEvent);

											// Record
											if (!Record(m_WaveHeader[m_nWaveInToggle].dwBytesRecorded,
														&m_pUncompressedBuf[m_nWaveInToggle][0]))
											{
												// Stop Recording
												m_pDoc->m_bAboutToStopRec = TRUE;
												m_pDoc->m_bStopRec = TRUE;
											}

											// If Video Audio Settings Page Opened and Visible
											// -> Calculate Mean Level For Peak-Meter and Display it.
											if (m_pDoc->m_pGeneralPage &&
												m_pDoc->m_pGeneralPage->IsWindowVisible())
											{
												CalcMeanLevel(	m_WaveHeader[m_nWaveInToggle].dwBytesRecorded,
																&m_pUncompressedBuf[m_nWaveInToggle][0]);
											}
											
											// New Buffer
											if (!DataInAudio(&m_pUncompressedBuf[m_nWaveInToggle][0], m_dwUncompressedBufSize))
												bExit = true;
											break;

				// Default
				default :					bExit = true;
											break;
			}
		}
		
		// Clean-Up
		::waveInReset(m_hWaveIn);
		::waveInUnprepareHeader(m_hWaveIn, &m_WaveHeader[0], sizeof(WAVEHDR));
		::waveInUnprepareHeader(m_hWaveIn, &m_WaveHeader[1], sizeof(WAVEHDR));
		memset(&m_WaveHeader[0], 0, sizeof(m_WaveHeader[0]));
		memset(&m_WaveHeader[1], 0, sizeof(m_WaveHeader[1]));
		CloseInAudio();
	}
	while (bDoRestart);

	return 0;
}

void CVideoDeviceDoc::CCaptureAudioThread::SetSmallBuffers(BOOL bSmallBuffers)
{
	if (m_bSmallBuffers != bSmallBuffers)
	{
		m_bSmallBuffers = bSmallBuffers;
		if (IsAlive())
			SetEvent(m_hRestartEvent);
	}
}

//
// Data format     Maximum value      Minimum value      Midpoint value 
// 8-bit PCM     : 255 (0xFF)         0                  128 (0x80) 
// 16-bit PCM    : 32,767 (0x7FFF)    -32,768 (0x8000)   0 
//
// 8-bit mono    : Each sample is 1 byte that corresponds to a single audio channel.
//                 Sample 1 is followed by samples 2, 3, 4, and so on.  
// 8-bit stereo  : Each sample is 2 bytes. Sample 1 is followed by samples 2, 3, 4, and so on.
//                 For each sample, the first byte is channel 0 (the left channel)
//                 and the second byte is channel 1 (the right channel). 
// 16-bit mono   : Each sample is 2 bytes. Sample 1 is followed by samples 2, 3, 4, and so on.
//                 For each sample, the first byte is the low-order byte of channel 0
//                 and the second byte is the high-order byte of channel 0.
// 16-bit stereo : Each sample is 4 bytes. Sample 1 is followed by samples 2, 3, 4, and so on.
//                 For each sample, the first byte is the low-order byte of channel 0 (left channel);
//                 the second byte is the high-order byte of channel 0;
//                 the third byte is the low-order byte of channel 1 (right channel);
//                 and the fourth byte is the high-order byte of channel 1. 
//
void CVideoDeviceDoc::CCaptureAudioThread::CalcMeanLevel(DWORD dwSize, LPBYTE pBuf)
{
	// Check
	if (!m_pSrcWaveFormat || m_pSrcWaveFormat->nBlockAlign == 0)
		return;

	// Vars
	double dPeakLeft = 0.0;
	double dPeakRight = 0.0;
	int nLevels[2];
	int nBlockAlign = m_pSrcWaveFormat->nBlockAlign;
	int nNumOfSamples = dwSize / nBlockAlign;	
	int nNumOfChannels = m_pSrcWaveFormat->nChannels;
	int nAudioBits = m_pSrcWaveFormat->wBitsPerSample;
	
	// Calc. max Peak(s)
	int nRep = nNumOfSamples / AUDIO_IN_MIN_SMALL_BUF_SIZE;
	for (int i = 0 ; i < nRep ; i++)
	{
		double dPL;
		double dPR;
		CalcPeak(AUDIO_IN_MIN_SMALL_BUF_SIZE,
				nNumOfChannels,
				nAudioBits, 
				pBuf,
				dPL,
				dPR);
		dPeakLeft = max(dPeakLeft, dPL);
		dPeakRight = max(dPeakRight, dPR);
		pBuf += AUDIO_IN_MIN_SMALL_BUF_SIZE * nBlockAlign;
	}

	// Clip Peak Levels
	if (nNumOfChannels == 1)
	{
		nLevels[1] = nLevels[0] = MIN(100, Round(dPeakLeft));
		if (nLevels[0] <= 10)
			nLevels[1] = nLevels[0] = 0;
	}
	else
	{
		nLevels[0] = MIN(100, Round(dPeakLeft));
		nLevels[1] = MIN(100, Round(dPeakRight));
		if (nLevels[0] <= 10)
			nLevels[0] = 0;
		if (nLevels[1] <= 10)
			nLevels[1] = 0;
	}
	
	// Update Peak Meter
	::PostMessage(	m_pDoc->m_pGeneralPage->GetSafeHwnd(),
					WM_PEAKMETER_UPDATE,
					(WPARAM)nLevels[0],
					(LPARAM)nLevels[1]);

	// Store Current Time to know the Age of a Mean Level
	m_MeanLevelTime = CTime::GetCurrentTime();
}

__forceinline void CVideoDeviceDoc::CCaptureAudioThread::CalcPeak(	int nNumOfSamples,
																	int nNumOfChannels,
																	int nAudioBits, 
																	LPBYTE pBuf,
																	double& dPeakLeft,
																	double& dPeakRight)
{
	// Vars
	int i;
	double dLeft;
	double dRight;
	int nValueLeft;
	int nValueRight;

	// Check
	ASSERT(nNumOfSamples <= AUDIO_IN_MIN_SMALL_BUF_SIZE);

	// Reset
	dPeakLeft = 0.0;
	dPeakRight = 0.0;

	// Copy audio signal to FFT Real Component
	for (i = 0 ; i < nNumOfSamples ; i++)
	{
		if (nAudioBits == 8) // 8 bits
		{
			if (nNumOfChannels == 1) // Mono
			{
				nValueLeft = ((LPBYTE)(pBuf))[i];
				nValueLeft -= 128;
				m_dInLeft[i] = (double)nValueLeft * (32767.0 / 127.0); // To Have the same Levels as with 16 bits
			}
			else // Stereo
			{
				WORD wSample = ((LPWORD)(pBuf))[i];
				nValueLeft = (int)(wSample & 0xFF);
				nValueRight = (int)((wSample >> 8) & 0xFF);
				nValueLeft -= 128;
				nValueRight -= 128;
				m_dInLeft[i] = (double)nValueLeft * (32767.0 / 127.0); // To Have the same Levels as with 16 bits
				m_dInRight[i] = (double)nValueRight * (32767.0 / 127.0); // To Have the same Levels as with 16 bits
			}
		}
		else // 16 bits
		{
			if (nNumOfChannels == 1) // Mono
			{
				nValueLeft = ((SHORT*)(pBuf))[i];
				m_dInLeft[i] = (double)nValueLeft;
			}
			else // Stereo
			{
				DWORD dwSample = ((LPDWORD)(pBuf))[i];
				nValueLeft = (SHORT)(dwSample & 0xFFFF);
				nValueRight = (SHORT)((dwSample >> 16) & 0xFFFF);
				m_dInLeft[i] = (double)nValueLeft;
				m_dInRight[i] = (double)nValueRight;
			}
		}
	}
	
	// FFT & Calc. Peak
	if (nNumOfChannels == 1)
	{
		fft_double(nNumOfSamples, false, m_dInLeft, NULL, m_dOutRe, m_dOutIm);
		for (i = 0 ; i < nNumOfSamples / 2 ; i++)
		{
			dLeft = fabs(INTENSITY(m_dOutRe[i], m_dOutIm[i]));
			dPeakLeft = dLeft > dPeakLeft? dLeft : dPeakLeft;
		}
		dPeakLeft /= (nNumOfSamples * 100.0);
	}
	else
	{
		fft_double(nNumOfSamples, false, m_dInLeft, NULL, m_dOutRe, m_dOutIm);
		for (i = 0 ; i < nNumOfSamples / 2 ; i++)
		{
			dLeft = fabs(INTENSITY(m_dOutRe[i], m_dOutIm[i]));
			dPeakLeft = dLeft > dPeakLeft? dLeft : dPeakLeft;
		}
		dPeakLeft /= (nNumOfSamples * 100.0);
		fft_double(nNumOfSamples, false, m_dInRight, NULL, m_dOutRe, m_dOutIm);
		for (i = 0 ; i < nNumOfSamples / 2 ; i++)
		{
			dRight = fabs(INTENSITY(m_dOutRe[i], m_dOutIm[i]));
			dPeakRight = dRight > dPeakRight ? dRight : dPeakRight;
		}
		dPeakRight /= (nNumOfSamples * 100.0);
	}
}

BOOL CVideoDeviceDoc::CCaptureAudioThread::Record(DWORD dwSize, LPBYTE pBuf)
{
	BOOL res = TRUE;

	// Record Audio
	if (m_pDoc->m_pAVRec)
	{
		// Do Stop Recording?
		if (m_pDoc->m_bStopRec)
		{
			if ((m_pDoc->m_bAudioRecWait == FALSE) && (m_pDoc->m_bVideoRecWait == FALSE))
				m_pDoc->m_bVideoRecWait = TRUE; // Stop Video Recording, record last buffer later in this function
			else if ((m_pDoc->m_bAudioRecWait == FALSE) && (m_pDoc->m_bVideoRecWait == TRUE))
			{
				m_pDoc->m_bAudioRecWait = TRUE; // Stop Video Recording
				m_pDoc->m_bStopRec = FALSE;		// Recording Stopped

				// Close Avi if not pausing!
				if (!m_pDoc->m_bCaptureRecordPause)
				{
					::EnterCriticalSection(&m_pDoc->m_csAVRec);
					m_pDoc->CloseAndShowAviRec();
					::LeaveCriticalSection(&m_pDoc->m_csAVRec);
				}

				// Stopped
				m_pDoc->m_bAboutToStopRec = FALSE;
			}
		}
		// Start or Running Recording
		else
		{
			// If paused, do nothing
			if (m_pDoc->m_bCaptureRecordPause)
				return TRUE;

			// Do Start?
			if (m_pDoc->m_bAudioRecWait && m_pDoc->m_bVideoRecWait)
				m_pDoc->m_bVideoRecWait = FALSE; // Video Record Can Start, Audio Record starts with the Next Buffer!
			else if (m_pDoc->m_bAudioRecWait && (m_pDoc->m_bVideoRecWait == FALSE))
			{
				// Also Audio Record can start now
				m_pDoc->m_bAudioRecWait = FALSE;

				// Started
				m_pDoc->m_bAboutToStartRec = FALSE;
			}
		}

		// Record Audio
		if (m_pDoc->m_bAudioRecWait == FALSE)
		{
			// Check for segmentation
			BOOL bNextAviFileCalled = FALSE;
			if (!m_pDoc->m_bStopRec)
			{
				if (m_pDoc->m_bRecTimeSegmentation)
				{
					CTime t = CTime::GetCurrentTime();
					if (t >= m_pDoc->m_NextRecTime)
					{
						// Update m_pDoc->m_NextRecTime
						m_pDoc->NextRecTime(t);

						// Note: NextAviFile() always adds the samples to
						// the old file, even if returning FALSE!
						res = NextAviFile(dwSize, pBuf);
						bNextAviFileCalled = TRUE;
					}
				}
			}

			// If samples not added by NextAviFile() add them here
			if (!bNextAviFileCalled)
			{
				// Compress Stream & Store to Avi File
				::EnterCriticalSection(&m_pDoc->m_csAVRec);
				if (m_pDstWaveFormat->wFormatTag == WAVE_FORMAT_PCM)
				{
					res = m_pDoc->m_pAVRec->AddRawAudioPacket(	m_pDoc->m_pAVRec->AudioStreamNumToStreamNum(ACTIVE_AUDIO_STREAM),
																dwSize,
																pBuf,
																m_pDoc->m_bInterleave ? true : false);
				}
				else
				{
					int nNumOfSrcSamples = (m_pSrcWaveFormat && (m_pSrcWaveFormat->nBlockAlign > 0)) ? dwSize / m_pSrcWaveFormat->nBlockAlign : 0;
					res = m_pDoc->m_pAVRec->AddAudioSamples(	m_pDoc->m_pAVRec->AudioStreamNumToStreamNum(ACTIVE_AUDIO_STREAM),
																nNumOfSrcSamples,
																pBuf,
																m_pDoc->m_bInterleave ? true : false);
				}
				::LeaveCriticalSection(&m_pDoc->m_csAVRec);
			}
		}
	}

	return res;
}

BOOL CVideoDeviceDoc::CCaptureAudioThread::NextAviFile(DWORD dwSize, LPBYTE pBuf)
{
	BOOL res = FALSE;
	int nRecordedFrames = 0;
	CAVRec* pNextAVRec = NULL;
	CAVRec* pOldAVRec = NULL;
	BOOL bFreeOldAVRec = FALSE;

	// Set Old AVRec Pointer
	pOldAVRec = m_pDoc->m_pAVRec;
	
	// Allocate & Init pNextAVRec
	if (m_pDoc->MakeAVRec(m_pDoc->MakeRecFileName(), &pNextAVRec))
	{
		// Change Pointer and
		// restart with frame counting and time measuring
		::EnterCriticalSection(&m_pDoc->m_csAVRec);
		m_pDoc->m_pAVRec = pNextAVRec;
		nRecordedFrames = m_pDoc->m_nRecordedFrames;
		m_pDoc->m_bRecFirstFrame = TRUE; // Video thread will reset m_pDoc->m_nRecordedFrames!
		::LeaveCriticalSection(&m_pDoc->m_csAVRec);				

		// Set Free Flag
		bFreeOldAVRec = TRUE;

		// Set Ok
		res = TRUE;
	}
	else
	{
		if (pNextAVRec)
			delete pNextAVRec;
		
	}

	// Store samples to Old File
	if (pOldAVRec)
	{
		if (m_pDstWaveFormat->wFormatTag == WAVE_FORMAT_PCM)
		{
			pOldAVRec->AddRawAudioPacket(	pOldAVRec->AudioStreamNumToStreamNum(ACTIVE_AUDIO_STREAM),
											dwSize,
											pBuf,
											m_pDoc->m_bInterleave ? true : false);
		}
		else
		{
			int nNumOfSrcSamples = (m_pSrcWaveFormat && (m_pSrcWaveFormat->nBlockAlign > 0)) ? dwSize / m_pSrcWaveFormat->nBlockAlign : 0;
			pOldAVRec->AddAudioSamples(	pOldAVRec->AudioStreamNumToStreamNum(ACTIVE_AUDIO_STREAM),
										nNumOfSrcSamples,
										pBuf,
										m_pDoc->m_bInterleave ? true : false);
		}
		if (bFreeOldAVRec)
		{
			// Get Samples Count
			LONGLONG llSamplesCount = pOldAVRec->GetSampleCount(pOldAVRec->AudioStreamNumToStreamNum(ACTIVE_AUDIO_STREAM));

			// Store old rec file name
			CString sOldRecFileName = pOldAVRec->GetFileName();

			// Free
			delete pOldAVRec;

			// Change Frame Rate
			double dFrameRate = 0.0;
			if (nRecordedFrames > 0	&&
				llSamplesCount > 0	&&
				m_pDstWaveFormat->nSamplesPerSec > 0)
			{
				dFrameRate =	(double)nRecordedFrames /
								((double)llSamplesCount / (double)m_pDstWaveFormat->nSamplesPerSec);
			}
			m_pDoc->ChangeRecFileFrameRate(sOldRecFileName, dFrameRate);

			// Open the video file
			m_pDoc->OpenAVIFile(sOldRecFileName);
		}
	}

	return res;
}

BOOL CVideoDeviceDoc::CCaptureAudioThread::OpenInAudio()
{
	MMRESULT res;

	// Check Wave Format Pointers
	if (!m_pSrcWaveFormat || !m_pDstWaveFormat)
		return FALSE;

	// First Close
	CloseInAudio();

	// Get Number of Audio Devices
	UINT num = ::waveInGetNumDevs();
	if (num == 0)
	{
		::AfxMessageBox(ML_STRING(1354, "No Sound Input Device."));
		return FALSE;
	}

	// If a WebCam with Audio has been removed device ID my not exist
	if (m_pDoc->m_dwCaptureAudioDeviceID > (num - 1))
		m_pDoc->m_dwCaptureAudioDeviceID = 0;

	// Test for Audio In availability
	res = ::waveInGetDevCaps(m_pDoc->m_dwCaptureAudioDeviceID, &m_WaveInDevCaps, sizeof(WAVEINCAPS));
	if (res != MMSYSERR_NOERROR)
	{
	   ::AfxMessageBox(ML_STRING(1355, "Sound Input Cannot Determine Card Capabilities!"));
	   return FALSE;
	}

	// Calculate The Source (=Uncompressed) Buffer Size
	if (m_bSmallBuffers)
	{
		int nSamplesPerSec = m_pSrcWaveFormat->nSamplesPerSec;
		int nBlockAlign = m_pSrcWaveFormat->nBlockAlign;
		if (nSamplesPerSec <= 11025) 
			m_dwUncompressedBufSize = 1 * AUDIO_IN_MIN_SMALL_BUF_SIZE * nBlockAlign;
		else if (nSamplesPerSec <= 22050)
			m_dwUncompressedBufSize = 2 * AUDIO_IN_MIN_SMALL_BUF_SIZE * nBlockAlign;
		else
			m_dwUncompressedBufSize = 4 * AUDIO_IN_MIN_SMALL_BUF_SIZE * nBlockAlign;
	}
	else
	{
		// Buffer Size
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
			if (nFrameSize > 0 && nRemainder > 0)
				m_dwUncompressedBufSize += nFrameSize - nRemainder;
		}
	}

	// Open Input
	res = ::waveInOpen(	&m_hWaveIn,
						m_pDoc->m_dwCaptureAudioDeviceID,
						m_pSrcWaveFormat,
						(DWORD)m_hWaveInEvent,
						NULL,
						CALLBACK_EVENT); 
	if (res != MMSYSERR_NOERROR)
	{
        ::AfxMessageBox(ML_STRING(1459, "Sound Input Cannot Open Device!"));
	    return FALSE;
	}

	return TRUE;
}

void CVideoDeviceDoc::CCaptureAudioThread::CloseInAudio()
{
	if (m_hWaveIn)
	{
		::waveInClose(m_hWaveIn);
		m_hWaveIn = NULL;
		// Reset The Close Event
		::ResetEvent(m_hWaveInEvent);
	}
}

BOOL CVideoDeviceDoc::CCaptureAudioThread::DataInAudio(LPBYTE lpData, DWORD dwSize)
{
	MMRESULT res;

	if (lpData == NULL)
		return FALSE;

	if (dwSize == 0)
		return FALSE;

	// Obs.: waveInUnprepareHeader in Win2000 fails if the WAVEHDR is empty (all zeros)
	// Make sure the buffer has been used (not first 2 buffers)
	if (m_WaveHeader[m_nWaveInToggle].dwFlags & WHDR_DONE)
	{
		res = ::waveInUnprepareHeader(m_hWaveIn, &m_WaveHeader[m_nWaveInToggle], sizeof(WAVEHDR)); 
		if (res != MMSYSERR_NOERROR) 
		{
			TRACE(_T("Sound Input Cannot UnPrepareHeader!\n"));
			return FALSE;
		}
	}

    m_WaveHeader[m_nWaveInToggle].lpData = (CHAR*)lpData;
    m_WaveHeader[m_nWaveInToggle].dwBufferLength = dwSize;
	m_WaveHeader[m_nWaveInToggle].dwLoops = 0;
	m_WaveHeader[m_nWaveInToggle].dwFlags = 0;

    res = ::waveInPrepareHeader(m_hWaveIn, &m_WaveHeader[m_nWaveInToggle], sizeof(WAVEHDR)); 
	if ((res != MMSYSERR_NOERROR) || (m_WaveHeader[m_nWaveInToggle].dwFlags != WHDR_PREPARED))
	{
		TRACE(_T("Sound Input Cannot PrepareHeader!\n"));
		return FALSE;
	}

	res = ::waveInAddBuffer(m_hWaveIn, &m_WaveHeader[m_nWaveInToggle], sizeof(WAVEHDR));
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

	m_nWaveInToggle = !m_nWaveInToggle;

	return TRUE;
}

CVideoDeviceDoc::CCaptureAudioThread::CMixerIn::CMixerIn()
{
	m_hMixer = NULL;
	m_hWndMixerCallback =	NULL;
	m_uiMixerID =			0xFFFFFFFF;
	m_dwChannels =			0;
	m_pChannels =			NULL;
	m_pLineID =				NULL;
	m_dwVolumeControlID =	0xFFFFFFFF;
	m_dwMuteControlID =		0xFFFFFFFF;
	m_pVolumeControlID =	NULL;
	m_pMuteControlID =		NULL;
	m_dwMuxControlID =		0xFFFFFFFF;
	m_dwMuxMultipleItems =	0;
	m_dwVolumeControlMin =	0;
	m_dwVolumeControlMax =	0;
	m_pVolumeControlMin =	NULL;
	m_pVolumeControlMax =	NULL;
	m_dwMuxControlMin =		0;
	m_dwMuxControlMax =		0;
	m_dwSourcesCount =		0;
}

CVideoDeviceDoc::CCaptureAudioThread::CMixerIn::~CMixerIn()
{
	Close();
}

BOOL CVideoDeviceDoc::CCaptureAudioThread::CMixerIn::Open(HWAVEIN hWaveIn, HWND hWndCallBack/*=NULL*/)
{
	MMRESULT res;
	int dest;
	int source;

	if (hWaveIn == NULL)
		return FALSE;

	res = ::mixerGetID((HMIXEROBJ)hWaveIn, &m_uiMixerID, MIXER_OBJECTF_HWAVEIN);
	if (res != MMSYSERR_NOERROR)
	{
		TRACE(_T("Sound Input Mixer Is Not Available\n"));
		return FALSE;
	}

	MIXERCAPS MixerCaps;
	res = ::mixerGetDevCaps(m_uiMixerID, &MixerCaps, sizeof(MIXERCAPS));
	if (res != MMSYSERR_NOERROR)
	{
		TRACE(_T("Sound Input Mixer Cannot Get Mixer Capabilities\n"));
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
		TRACE(_T("Sound Input Mixer Cannot Be Opened\n"));
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

	// Find The Wave In Destination
	for (dest = 0 ; dest < (int)(MixerCaps.cDestinations) ; dest++)
	{
		mxl.dwSource = 0xFFFFFFF; // Not Used
		mxl.dwDestination = dest;
		res = ::mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mxl, MIXER_GETLINEINFOF_DESTINATION);
		if (res != MMSYSERR_NOERROR)
		{
			TRACE(_T("Sound Input Cannot Get Line Information\n"));
			return FALSE;
		}

		if (mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN)
			break;
	}

	// Number Of Channels
	m_dwChannels = mxl.cChannels;

	// Volume Destination Control
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(MIXERCONTROL);
	mxlc.pamxctrl = &mxc;
	res = ::mixerGetLineControls((HMIXEROBJ)m_hMixer, &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
	if (res == MMSYSERR_NOERROR)
	{
		m_dwVolumeControlID = mxc.dwControlID;
		m_dwVolumeControlMin = mxc.Bounds.dwMinimum;
		m_dwVolumeControlMax = mxc.Bounds.dwMaximum;
	}

	// Mute Destination Control
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(MIXERCONTROL);
	mxlc.pamxctrl = &mxc;
	res = ::mixerGetLineControls((HMIXEROBJ)m_hMixer, &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
	if (res == MMSYSERR_NOERROR)
	{
		m_dwMuteControlID = mxc.dwControlID;
	}

	// Mux Destination Control
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUX;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(MIXERCONTROL);
	mxlc.pamxctrl = &mxc;
	res = ::mixerGetLineControls((HMIXEROBJ)m_hMixer, &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
	if (res == MMSYSERR_NOERROR)
	{
		m_dwMuxControlID = mxc.dwControlID;
		m_dwMuxControlMin = mxc.Bounds.dwMinimum;
		m_dwMuxControlMax = mxc.Bounds.dwMaximum;
		m_dwMuxMultipleItems = mxc.cMultipleItems;
	}

	// Source Connections
	m_dwSourcesCount = mxl.cConnections;
	if (m_dwSourcesCount > 0)
	{
		if (m_pChannels) delete [] m_pChannels;
		m_pChannels = new DWORD[m_dwSourcesCount];

		if (m_pLineID) delete [] m_pLineID;
		m_pLineID = new DWORD[m_dwSourcesCount];

		if (m_pVolumeControlID) 
		{
			delete [] m_pVolumeControlID;
			m_pVolumeControlID = NULL;
		}
	
		if (m_pMuteControlID) 
		{
			delete [] m_pMuteControlID;
			m_pMuteControlID = NULL;
		}

		if (m_pVolumeControlMin) 
		{
			delete [] m_pVolumeControlMin;
			m_pVolumeControlMin = NULL;
		}

		if (m_pVolumeControlMax) 
		{
			delete [] m_pVolumeControlMax;
			m_pVolumeControlMax = NULL;
		}
	}
	for (source = 0 ; source < (int)m_dwSourcesCount ; source++)
	{
		mxl.dwSource = source;
		mxl.dwDestination = dest;
		res = ::mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mxl, MIXER_GETLINEINFOF_SOURCE);
		if (res != MMSYSERR_NOERROR)
		{
			TRACE(_T("Sound Input Cannot Get Line Information\n"));
			return FALSE;
		}

		// Number Of Channels
		m_pChannels[source] = mxl.cChannels;

		// Line Id
		m_pLineID[source] = mxl.dwLineID;

		// Volume Source Controls
		mxlc.dwLineID = mxl.dwLineID;
		mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
		mxlc.cControls = 1;
		mxlc.cbmxctrl = sizeof(MIXERCONTROL);
		mxlc.pamxctrl = &mxc;
		res = ::mixerGetLineControls((HMIXEROBJ)m_hMixer, &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
		if (res == MMSYSERR_NOERROR)
		{
			if (m_pVolumeControlID == NULL)
				m_pVolumeControlID = new DWORD[m_dwSourcesCount];
			m_pVolumeControlID[source] = mxc.dwControlID;

			if (m_pVolumeControlMin == NULL)
				m_pVolumeControlMin = new DWORD[m_dwSourcesCount];
			m_pVolumeControlMin[source] = mxc.Bounds.dwMinimum;

			if (m_pVolumeControlMax == NULL)
				m_pVolumeControlMax = new DWORD[m_dwSourcesCount];
			m_pVolumeControlMax[source] = mxc.Bounds.dwMaximum;
		}

		// Mute Source Controls
		mxlc.dwLineID = mxl.dwLineID;
		mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
		mxlc.cControls = 1;
		mxlc.cbmxctrl = sizeof(MIXERCONTROL);
		mxlc.pamxctrl = &mxc;
		res = ::mixerGetLineControls((HMIXEROBJ)m_hMixer, &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE);
		if (res == MMSYSERR_NOERROR)
		{
			if (m_pMuteControlID == NULL)
				m_pMuteControlID = new DWORD[m_dwSourcesCount];
			m_pMuteControlID[source] = mxc.dwControlID;
		}
	}

	return TRUE;
}

void CVideoDeviceDoc::CCaptureAudioThread::CMixerIn::Close()
{
	if (m_hMixer)
	{
		::mixerClose(m_hMixer);
		m_hMixer = NULL;
	}
	m_hWndMixerCallback =	NULL;
	m_uiMixerID =			0xFFFFFFFF;
	m_dwChannels =			0;
	m_dwVolumeControlID =	0xFFFFFFFF;
	m_dwMuxControlID =		0xFFFFFFFF;
	m_dwMuteControlID =		0xFFFFFFFF;
	m_dwVolumeControlMin =	0;
	m_dwVolumeControlMax =	0;
	m_dwMuxControlMin =		0;
	m_dwMuxControlMax =		0;
	if (m_pChannels)
	{
		delete [] m_pChannels;
		m_pChannels = NULL;
	}
	if (m_pLineID)
	{
		delete [] m_pLineID;
		m_pLineID = NULL;
	}
	if (m_pVolumeControlID)
	{
		delete [] m_pVolumeControlID;
		m_pVolumeControlID = NULL;
	}
	if (m_pMuteControlID)
	{
		delete [] m_pMuteControlID;
		m_pMuteControlID = NULL;
	}
	if (m_pVolumeControlMin)
	{
		delete [] m_pVolumeControlMin;
		m_pVolumeControlMin = NULL;
	}
	if (m_pVolumeControlMax)
	{
		delete [] m_pVolumeControlMax;
		m_pVolumeControlMax = NULL;
	}
}

DWORD CVideoDeviceDoc::CCaptureAudioThread::CMixerIn::GetMux() const
{
	MMRESULT res;
	MIXERCONTROLDETAILS mxcd;

	if (m_hMixer == NULL)
		return 0;

	if (m_dwMuxControlID == 0xFFFFFFFF)
		return 0;
	
	// Mux List Text, m_dwChannels in the allocation size is not needed,
	// but I found some cards which wrote some bytes after the last list text label
	// -> Better to follow the specifications and use also the channel size!
	LPMIXERCONTROLDETAILS_LISTTEXT mxcd_listtext = new MIXERCONTROLDETAILS_LISTTEXT[m_dwChannels * m_dwMuxMultipleItems];
	mxcd.cbStruct = sizeof(mxcd);
	mxcd.dwControlID = m_dwMuxControlID;
	mxcd.cChannels = m_dwChannels;
	mxcd.cMultipleItems = m_dwMuxMultipleItems;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
	mxcd.paDetails = mxcd_listtext;
	res = ::mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_LISTTEXT);
	if (MMSYSERR_NOERROR != res)
	{
		delete [] mxcd_listtext;
		return 0;
	}

	// Mux Value, same like above for m_dwChannels!
	LPMIXERCONTROLDETAILS_BOOLEAN mxcd_bool = new MIXERCONTROLDETAILS_BOOLEAN[m_dwChannels * m_dwMuxMultipleItems]; 
	mxcd.cbStruct = sizeof(mxcd);
	mxcd.dwControlID = m_dwMuxControlID;
	mxcd.cChannels = m_dwChannels;
	mxcd.cMultipleItems = m_dwMuxMultipleItems;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mxcd.paDetails = mxcd_bool;
	res = ::mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);
	if (MMSYSERR_NOERROR != res)
	{
		delete [] mxcd_listtext;
		delete [] mxcd_bool;
		return 0;
	}
	
	for (int i = 0 ; i < (int)m_dwMuxMultipleItems ; i++)
	{
		if (mxcd_bool[i].fValue)
		{
			for (int j = 0 ; j < (int)m_dwMuxMultipleItems ; j++)
			{
				// dwParam1 is the LineID and dwParam2 is the ComponentType
				if (mxcd_listtext[i].dwParam1 == m_pLineID[j])
				{
					delete [] mxcd_listtext;
					delete [] mxcd_bool;
					return j;
				}
			}
		}
	}

	// Should Never Reach Here, But You Never Know...
	delete [] mxcd_listtext;
	delete [] mxcd_bool;
	return 0;
}

DWORD CVideoDeviceDoc::CCaptureAudioThread::CMixerIn::GetSrcVolumeControlID() const
{
	if (m_pVolumeControlID)
		return m_pVolumeControlID[GetMux()];
	else
		return 0xFFFFFFFF;
}

DWORD CVideoDeviceDoc::CCaptureAudioThread::CMixerIn::GetSrcVolumeControlMin() const
{
	if (m_pVolumeControlMin)
		return m_pVolumeControlMin[GetMux()];
	else
		return 0;
}

DWORD CVideoDeviceDoc::CCaptureAudioThread::CMixerIn::GetSrcVolumeControlMax() const 
{
	if (m_pVolumeControlMax)
		return m_pVolumeControlMax[GetMux()];
	else
		return 0;
}

DWORD CVideoDeviceDoc::CCaptureAudioThread::CMixerIn::GetSrcMuteControlID() const
{
	if (m_pMuteControlID)
		return m_pMuteControlID[GetMux()];
	else
		return 0xFFFFFFFF;
}

DWORD CVideoDeviceDoc::CCaptureAudioThread::CMixerIn::GetDstNumOfChannels() const
{
	if (m_dwVolumeControlID != 0xFFFFFFFF)
		return m_dwChannels;
	else
		return 0;
}

DWORD CVideoDeviceDoc::CCaptureAudioThread::CMixerIn::GetSrcNumOfChannels() const
{
	if (m_pChannels)
		return m_pChannels[GetMux()];
	else
		return 0;
}

// Some controls are stereo (2 channels), but work only if queried with mono (1 channel).
// This is the purpose of the bForceMono parameter
BOOL CVideoDeviceDoc::CCaptureAudioThread::CMixerIn::GetDstMute(BOOL bForceMono/*=FALSE*/) const
{
	MMRESULT res;
	BOOL bMute;
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(mxcd);

	if (m_hMixer == NULL)
		return FALSE;

	if (m_dwMuteControlID != 0xFFFFFFFF)
	{
		mxcd.dwControlID = m_dwMuteControlID;
		if (bForceMono)
			mxcd.cChannels = 1;
		else
			mxcd.cChannels = m_dwChannels;
	}
	else
		return FALSE;

	LPMIXERCONTROLDETAILS_BOOLEAN mxcd_bool = new MIXERCONTROLDETAILS_BOOLEAN[mxcd.cChannels];
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mxcd.paDetails = mxcd_bool;
	res = ::mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);
	if (MMSYSERR_NOERROR != res)
	{
		delete [] mxcd_bool;
		if (!bForceMono)
			return GetDstMute(TRUE);
		else
			return FALSE;
	}
	if (mxcd.cChannels >= 1)
	{
		bMute = (BOOL)(mxcd_bool[0].fValue);
		delete [] mxcd_bool;
		return bMute;
	}
	
	delete [] mxcd_bool;
	return FALSE;
}

// Some controls are stereo (2 channels), but work only if queried with mono (1 channel).
// This is the purpose of the bForceMono parameter
BOOL CVideoDeviceDoc::CCaptureAudioThread::CMixerIn::GetSrcMute(BOOL bForceMono/*=FALSE*/) const
{
	MMRESULT res;
	BOOL bMute;
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(mxcd);
	
	if (m_hMixer == NULL)
		return FALSE;

	if (m_pMuteControlID)
	{
		mxcd.dwControlID = m_pMuteControlID[GetMux()];
		if (bForceMono)
			mxcd.cChannels = 1;
		else
			mxcd.cChannels = m_pChannels[GetMux()];
	}
	else
		return FALSE;

	LPMIXERCONTROLDETAILS_BOOLEAN mxcd_bool = new MIXERCONTROLDETAILS_BOOLEAN[mxcd.cChannels];
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mxcd.paDetails = mxcd_bool;
	res = ::mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);
	if (MMSYSERR_NOERROR != res)
	{
		delete [] mxcd_bool;
		if (!bForceMono)
			return GetSrcMute(TRUE);
		else
			return FALSE;
	}
	if (mxcd.cChannels >= 1)
	{
		bMute = (BOOL)(mxcd_bool[0].fValue);
		delete [] mxcd_bool;
		return bMute;
	}
	
	delete [] mxcd_bool;
	return FALSE;
}

BOOL CVideoDeviceDoc::CCaptureAudioThread::CMixerIn::GetDstVolume(DWORD& dwVolumeLeft, DWORD& dwVolumeRight) const
{
	MMRESULT res;
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(mxcd);
	
	if (m_hMixer == NULL)
		return FALSE;

	if (m_dwVolumeControlID != 0xFFFFFFFF)
	{
		mxcd.dwControlID = m_dwVolumeControlID;
		mxcd.cChannels = m_dwChannels;
	}
	else
		return FALSE;

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

BOOL CVideoDeviceDoc::CCaptureAudioThread::CMixerIn::GetSrcVolume(DWORD& dwVolumeLeft, DWORD& dwVolumeRight) const
{
	MMRESULT res;
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(mxcd);
	
	if (m_hMixer == NULL)
		return FALSE;

	if (m_pVolumeControlID)
	{
		mxcd.dwControlID = m_pVolumeControlID[GetMux()];
		mxcd.cChannels = m_pChannels[GetMux()];
	}
	else
		return FALSE;

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

BOOL CVideoDeviceDoc::CCaptureAudioThread::CMixerIn::SetDstVolume(DWORD dwVolumeLeft, DWORD dwVolumeRight)
{
	MMRESULT res;
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(mxcd);

	if (m_hMixer == NULL)
		return FALSE;
		
	if (m_dwVolumeControlID != 0xFFFFFFFF)
	{
		mxcd.dwControlID = m_dwVolumeControlID;
		mxcd.cChannels = m_dwChannels;
	}
	else
		return FALSE;

	if ((dwVolumeLeft > m_dwVolumeControlMax) ||
		(dwVolumeLeft < m_dwVolumeControlMin) ||
		(dwVolumeRight > m_dwVolumeControlMax) ||
		(dwVolumeRight < m_dwVolumeControlMin))
		return FALSE;

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

BOOL CVideoDeviceDoc::CCaptureAudioThread::CMixerIn::SetSrcVolume(DWORD dwVolumeLeft, DWORD dwVolumeRight)
{
	MMRESULT res;
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(mxcd);

	if (m_hMixer == NULL)
		return FALSE;
		
	if (m_pVolumeControlID)
	{
		mxcd.dwControlID = m_pVolumeControlID[GetMux()];
		mxcd.cChannels = m_pChannels[GetMux()];
	}
	else
		return FALSE;

	if ((dwVolumeLeft > m_pVolumeControlMax[GetMux()]) ||
		(dwVolumeLeft < m_pVolumeControlMin[GetMux()]) ||
		(dwVolumeRight > m_pVolumeControlMax[GetMux()]) ||
		(dwVolumeRight < m_pVolumeControlMin[GetMux()]))
		return FALSE;

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

void CVideoDeviceDoc::MovementDetectionProcessing(CDib* pDib, DWORD dwVideoProcessorMode, BOOL b1SecTick)
{
	BOOL bMovement = FALSE;
	BOOL bLumChange = FALSE;
	BOOL bExternalFileTriggerMovement = FALSE;

	// Init from UI thread because of a UI control update and
	// initialization of variables used by the UI drawing
	if (m_lMovDetTotalZones == 0 || m_nCurrentDetectionZoneSize != m_nDetectionZoneSize)
	{
		if (::SendMessage(	GetView()->GetSafeHwnd(),
							WM_THREADSAFE_INIT_MOVDET,
							0, 0) == 0)
			return; // Cannot init, unsupported resolution
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
					return;
				m_pMovementDetectorBackgndDib->SetShowMessageBoxOnError(FALSE);
			}
			if (!m_pDifferencingDib)
			{
				m_pDifferencingDib = new CDib;
				if (!m_pDifferencingDib)
					return;
				m_pDifferencingDib->SetShowMessageBoxOnError(FALSE);
				if (!m_pDifferencingDib->AllocateBitsFast(	pDib->GetBitCount(),
															pDib->GetCompression(),
															pDib->GetWidth(),
															pDib->GetHeight()))
					return;
			}

			// Luminosity change detector
			if (m_bDoLumChangeDetection)
				bLumChange = LumChangeDetector(pDib);

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
	{
		for (int i = 0 ; i < m_lMovDetTotalZones ; i++)
			m_MovementDetections[i] = FALSE;
	}

	// Trigger file detection enabled?
	if ((dwVideoProcessorMode & TRIGGER_FILE_DETECTOR) &&
		b1SecTick && !m_sDetectionTriggerFileName.IsEmpty())
	{
		CString sDetectionTriggerFileName(m_sDetectionTriggerFileName);
		sDetectionTriggerFileName.TrimLeft();
		sDetectionTriggerFileName.TrimRight();
		sDetectionTriggerFileName.TrimLeft(_T('\"'));
		sDetectionTriggerFileName.TrimRight(_T('\"'));
		if (sDetectionTriggerFileName.Find(_T('\\')) < 0)
		{
			CString sDetectionAutoSaveDir = m_sDetectionAutoSaveDir;
			sDetectionAutoSaveDir.TrimRight(_T('\\'));
			sDetectionTriggerFileName = sDetectionAutoSaveDir + _T("\\") + sDetectionTriggerFileName;
		}
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

	// Store frames?
	BOOL bStoreFrames =	dwVideoProcessorMode			&&
						(m_bSaveSWFMovementDetection	||
						m_bSaveAVIMovementDetection		||
						m_bSaveAnimGIFMovementDetection	||
						m_bSendMailMovementDetection	||
						m_bFTPUploadMovementDetection);

	// If Movement
	if ((bMovement && !bLumChange) || bExternalFileTriggerMovement)
	{
		// Mark the Frame as a Cause of Movement
		pDib->SetUserFlag(TRUE);

		// Reset var
		m_dwWithoutMovementDetection = pDib->GetUpTime();

		// First detected frame?
		if (!m_bDetectingMovement)
		{
			m_bDetectingMovement = TRUE;
			if (m_bExecCommandMovementDetection && m_nExecModeMovementDetection == 0)
				ExecCommandMovementDetection();
		}
	}
	// If No Movement
	else
	{
		// Mark the Frame as no Movement Cause
		pDib->SetUserFlag(FALSE);

		// Reset var
		if (!m_bDetectingMovement)
			m_dwWithoutMovementDetection = pDib->GetUpTime();
	}

	// If in detection state
	if (m_bDetectingMovement)
	{
		// Add new frame
		if (bStoreFrames)
			AddNewFrameToNewestList(pDib);

		// Check if end of detection period
		if ((pDib->GetUpTime() - m_dwWithoutMovementDetection) > (DWORD)m_nMilliSecondsRecAfterMovementEnd)
		{
			// Reset var
			m_bDetectingMovement = FALSE;

			// Save frames
			SaveFrameList();
		}
		else 
		{
			// Check memory load if having MOVDET_MIN_FRAMES_IN_LIST
			// frames and MOVDET_MIN_FRAMES_IN_LIST passed since last check
			int nFramesCount = GetNewestMovementDetectionsListCount();
			if (nFramesCount >= MOVDET_MIN_FRAMES_IN_LIST	&&
				((m_dwFrameCountUp % MOVDET_MIN_FRAMES_IN_LIST) == 0))
			{
				// This document load
				int nTotalPhysInMB = ::GetTotPhysMemMB(FALSE);
				if (nTotalPhysInMB > MOVDET_MEM_MAX_MB)
					nTotalPhysInMB = MOVDET_MEM_MAX_MB;
				else if (nTotalPhysInMB <= 0)
					nTotalPhysInMB = 1;		// At least 1MB...
				double dDocLoad = ((double)(GetTotalMovementDetectionFrames() * (pDib->GetImageSize() >> 10)) / 10.24) / (double)nTotalPhysInMB; // Load in %
				double dTotalDocsMovementDetecting = (double)((CUImagerApp*)::AfxGetApp())->GetTotalVideoDeviceDocsMovementDetecting();

				// This application load
				double dAppLoad = GetAppMemoryLoad(); // Load in %, on Win9x this always returns 100.0

				// High thresholds, should not end up here,
				// this means that frames saving is to slow!
				// -> Throw the oldest MOVDET_MIN_FRAMES_IN_LIST frames 
				if (dAppLoad >= MOVDET_MEM_LOAD_CRITICAL &&
					dTotalDocsMovementDetecting * dDocLoad >= MOVDET_MEM_LOAD_CRITICAL)
				{
					DWORD dwFirstUpTime, dwLastUpTime;
					ShrinkNewestFrameListBy(MOVDET_MIN_FRAMES_IN_LIST, dwFirstUpTime, dwLastUpTime);
					ThumbMessage(	ML_STRING(1817, "Dropping det frames:"),
									ML_STRING(1818, "set lower framerate"),
									ML_STRING(1819, "or resolution!"),
									dwFirstUpTime, dwLastUpTime);
					CString sMsg;
					sMsg.Format(_T("%s, doc mem load %0.1f%%%%, app mem load %0.1f%%%% -> dropping det frames!\n"),
																			GetDeviceName(), dDocLoad, dAppLoad);
					TRACE(sMsg);
					::LogLine(sMsg);
				}
				
				// Low load threshold or maximum number of frames reached
				if (m_SaveFrameListThread.IsAlive() && !m_SaveFrameListThread.IsWorking()	&&
					(dTotalDocsMovementDetecting * dDocLoad >= MOVDET_MEM_LOAD_THRESHOLD	||
					nFramesCount >= MOVDET_MAX_FRAMES_IN_LIST))
					SaveFrameList();
			}
		}
	}
	else
	{
		// Add new frame
		if (bStoreFrames)
			AddNewFrameToNewestListAndShrink(pDib);
		else
			ClearNewestFrameList();
	}
}

void CVideoDeviceDoc::ExecCommandMovementDetection()
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
		SHELLEXECUTEINFO sei;
		memset(&sei, 0, sizeof(sei));
		sei.cbSize = sizeof(sei);
		sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
		sei.nShow = m_bHideExecCommandMovementDetection ? SW_HIDE : SW_SHOWNORMAL;
		sei.lpFile = m_sExecCommandMovementDetection;
		sei.lpParameters = m_sExecParamsMovementDetection; 
		if (::ShellExecuteEx(&sei))
			m_hExecCommandMovementDetection = sei.hProcess;
	}
	::LeaveCriticalSection(&m_csExecCommandMovementDetection);
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
		CTime FirstTime = CAVRec::CalcTime(dwFirstUpTime, RefTime, dwRefUpTime);
		
		// Last Frame Time
		CTime LastTime = CAVRec::CalcTime(dwLastUpTime, RefTime, dwRefUpTime);

		// Check Whether Detection Dir Exists
		CString sDetectionAutoSaveDir = m_sDetectionAutoSaveDir;
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

		// Date
		sTime = ::MakeDateLocalFormat(FirstTime);
		if (!ThumbDib.AddSingleLineText(sTime,
										rcRect,
										NULL,
										(DT_LEFT | DT_TOP),
										FRAMEDATE_COLOR,
										TRANSPARENT,
										RGB(0,0,0)))
			return FALSE;

		// Message1
		rcRect.top = rcRect.bottom / 4;
		if (!ThumbDib.AddSingleLineText(sMessage1,
										rcRect,
										NULL,
										(DT_CENTER | DT_TOP),
										RGB(0xff,0,0),
										TRANSPARENT,
										RGB(0,0,0)))
			return FALSE;

		// Message2
		rcRect.top = 0;
		if (!ThumbDib.AddSingleLineText(sMessage2,
										rcRect,
										NULL,
										(DT_CENTER | DT_VCENTER),
										RGB(0xff,0,0),
										TRANSPARENT,
										RGB(0,0,0)))
			return FALSE;

		// Message3
		rcRect.bottom = 3 * rcRect.bottom / 4;
		if (!ThumbDib.AddSingleLineText(sMessage3,
										rcRect,
										NULL,
										(DT_CENTER | DT_BOTTOM),
										RGB(0xff,0,0),
										TRANSPARENT,
										RGB(0,0,0)))
			return FALSE;

		// Time
		rcRect.bottom = ThumbDib.GetHeight();
		sTime = ::MakeTimeLocalFormat(FirstTime, TRUE);
		if (!ThumbDib.AddSingleLineText(sTime,
										rcRect,
										NULL,
										(DT_LEFT | DT_BOTTOM),
										FRAMETIME_COLOR,
										TRANSPARENT,
										RGB(0,0,0)))
			return FALSE;
		if (!ThumbDib.AddSingleLineText(_T("->"),
										rcRect,
										NULL,
										(DT_CENTER | DT_BOTTOM),
										FRAMETIME_COLOR,
										TRANSPARENT,
										RGB(0,0,0)))
			return FALSE;
		rcRect.right -= 1; // Looks nicer!
		sTime = ::MakeTimeLocalFormat(LastTime, TRUE);
		if (!ThumbDib.AddSingleLineText(sTime,
										rcRect,
										NULL,
										(DT_RIGHT | DT_BOTTOM),
										FRAMETIME_COLOR,
										TRANSPARENT,
										RGB(0,0,0)))
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

__forceinline int CVideoDeviceDoc::GetAppMemoryUsageMB()
{
	PROCESS_MEMORY_COUNTERS pmc;
	typedef BOOL (WINAPI * FPGETPROCESSMEMORYINFO) (HANDLE, PPROCESS_MEMORY_COUNTERS, DWORD);
	HINSTANCE h = LoadLibrary(_T("psapi.dll"));
	if (!h)
		return -1;
	FPGETPROCESSMEMORYINFO lpGetProcessMemoryInfo = (FPGETPROCESSMEMORYINFO)GetProcAddress(h, "GetProcessMemoryInfo");
    if (lpGetProcessMemoryInfo && lpGetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {   
		FreeLibrary(h);
		return MAX(pmc.WorkingSetSize >> 20, pmc.PagefileUsage >> 20);
    }
	else
	{
		FreeLibrary(h);
		return -1;
	}
}

__forceinline double CVideoDeviceDoc::GetAppMemoryLoad()
{
	int nUsageInMB = GetAppMemoryUsageMB();
	if (nUsageInMB < 0)
		return 100;
	int nTotalPhysInMB = ::GetTotPhysMemMB(FALSE);
	if (nTotalPhysInMB > MOVDET_MEM_MAX_MB)
		nTotalPhysInMB = MOVDET_MEM_MAX_MB;
	if (nTotalPhysInMB > 0)
		return (double)nUsageInMB * 100.0 / (double)nTotalPhysInMB;
	else
		return 100.0;
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
			// Remove closed or old connections
			CTimeSpan ConnectionAge = CTime::GetCurrentTime() - pNetCom->m_InitTime;
			if (pNetCom->IsShutdown()												||
				ConnectionAge.GetTotalSeconds() >= HTTPGETFRAME_CONNECTION_TIMEOUT	||
				ConnectionAge.GetTotalSeconds() < 0)
			{
				delete pNetCom;
				m_HttpGetFrameNetComList.RemoveHead();
				pHttpGetFrameParseProcess = m_HttpGetFrameParseProcessList.GetHead();
				if (pHttpGetFrameParseProcess)
					delete pHttpGetFrameParseProcess;
				m_HttpGetFrameParseProcessList.RemoveHead();
			}
			else
			{
				// Buffer is full, it's time to force a connection shutdown
				// (connection may have been abruptly closed by the peer
				// so that our socket has not been notified about that)
				if (!bDoNewPoll)
				{
					delete pNetCom; // This calls Close() which blocks till all threads are done
					m_HttpGetFrameNetComList.RemoveHead();
					pHttpGetFrameParseProcess = m_HttpGetFrameParseProcessList.GetHead();
					if (pHttpGetFrameParseProcess)
						delete pHttpGetFrameParseProcess;
					m_HttpGetFrameParseProcessList.RemoveHead();
				}
				// Nothing to clean-up ... exit from here
				else
					break;
			}
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
	::AfxMessageBox(ML_STRING(1465, "Cannot connect to the specified network device or server"), MB_ICONSTOP);
	m_pDoc->CloseDocRemoveAutorunDev();
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
					if (m_pDoc->m_pHttpGetFrameParseProcess->m_bConnectionKeepAlive)
						m_pDoc->m_pHttpGetFrameParseProcess->SendRequest();
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
			// Store start time
			m_pDoc->m_CaptureStartTime = CTime::GetCurrentTime();

			// Log the starting (no log for Video Avi mode)
			if (!m_pDoc->m_pVideoAviDoc)
			{
				CString sMsg;
				sMsg.Format(_T("%s starting\n"), m_pDoc->GetDeviceName());
				TRACE(sMsg);
				::LogLine(sMsg);
			}

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
					m_pDoc->GetTotalMovementDetectionFrames() > 0		&&
					m_pDoc->m_SaveFrameListThread.IsAlive()				&&
					!m_pDoc->m_SaveFrameListThread.IsWorking()			&&
					(m_pDoc->m_bSaveSWFMovementDetection				||
					m_pDoc->m_bSaveAVIMovementDetection					||
					m_pDoc->m_bSaveAnimGIFMovementDetection				||
					m_pDoc->m_bSendMailMovementDetection				||
					m_pDoc->m_bFTPUploadMovementDetection))
					m_pDoc->SaveFrameList();

				// Http Networking Reconnect
				if (dwCurrentUpTime - dwLastHttpReconnectUpTime > HTTPWATCHDOG_RETRY_TIMEOUT &&
					m_pDoc->m_pGetFrameNetCom && m_pDoc->m_pGetFrameNetCom->IsClient())
				{
					dwLastHttpReconnectUpTime = dwCurrentUpTime;
					m_pDoc->m_HttpGetFrameThread.SetEventConnect();
					CString sMsg;
					sMsg.Format(_T("%s try reconnecting\n"), m_pDoc->GetDeviceName());
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
#if _MFC_VER >= 0x0700
				nNextYear > 3000	||	// MFC CTime Limitation
#else
				nNextYear > 2037	||	// MFC CTime Limitation
#endif
				nYear < 1971		||
#if _MFC_VER >= 0x0700
				nYear > 3000		||	// MFC CTime Limitation
#else
				nYear > 2037		||	// MFC CTime Limitation
#endif		
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
#if _MFC_VER >= 0x0700
			nYear > 3000	||		// MFC CTime Limitation
#else
			nYear > 2037	||		// MFC CTime Limitation
#endif
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
#if _MFC_VER >= 0x0700
			nYear > 3000	||		// MFC CTime Limitation
#else
			nYear > 2037	||		// MFC CTime Limitation
#endif
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
	ULONGLONG llDiskTotalSize;
	ULONGLONG llDiskFreeSpace;
	int nDiskFreeSpacePercent;

	// Check and adjust Auto-Save directory
	dwAttrib = ::GetFileAttributes(sAutoSaveDir);
	if (dwAttrib != 0xFFFFFFFF && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
	{
		sAutoSaveDir.TrimRight(_T('\\'));
		int nAutoSaveDirSize = sAutoSaveDir.GetLength() + 1;

		// Do recursive file find
		FileFind.InitRecursive(sAutoSaveDir + _T("\\*"));
		if (FileFind.WaitRecursiveDone(GetKillEvent()) < 0)
			return FALSE; // Exit Thread

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
		llDiskTotalSize = ::GetDiskSize(sAutoSaveDir);
		if (llDiskTotalSize > 0)
		{
			// Get the time of the oldest existing directory
			if (!CalcOldestDir(	FileFind,
								nAutoSaveDirSize,
								OldestDirTime,
								CurrentTime))
				return FALSE; // Exit Thread

			TimeDiff = CurrentTime - OldestDirTime;
			llDaysAgo = (LONGLONG)TimeDiff.GetDays();
			llDiskFreeSpace = ::GetDiskSpace(sAutoSaveDir);
			nDiskFreeSpacePercent = (int)(100 * llDiskFreeSpace / llDiskTotalSize);
			bDeletingOld = FALSE;
			while (llDaysAgo > 0 && nDiskFreeSpacePercent < MIN_DISKFREE_PERCENT)
			{
				// Delete old
				if (!DeleteOld(	FileFind,
								nAutoSaveDirSize,
								llDaysAgo,
								CurrentTime))
					return FALSE; // Exit Thread
				bDeletingOld = TRUE;
				llDaysAgo--;
				llDiskFreeSpace = ::GetDiskSpace(sAutoSaveDir);
				nDiskFreeSpacePercent = (int)(100 * llDiskFreeSpace / llDiskTotalSize);
			}

			// Log
			if (bDeletingOld)
			{
				CString sMsg;
				sMsg.Format(_T("%s, deleting old files in \"%s\" because the available disk space is less than %d%%%%\n"),
							m_pDoc->GetDeviceName(), sAutoSaveDir, MIN_DISKFREE_PERCENT);
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
		srand(::makeseed(::timeGetTime(), ::GetCurrentProcessId(), ::GetCurrentThreadId()));   // Seed
		DWORD dwDeleteInMs = FILES_DELETE_INTERVAL_MIN + irand(FILES_DELETE_INTERVAL_RANGE); // [10min,15min[

		// dwDeleteInMs should never be to small (at least 60 seconds)
		// otherwise when entering a new days value for deletion like 30 after
		// typing the number 3 all files older than 3 days are deleted even if we
		// intended 30!
		Event = ::WaitForSingleObject(GetKillEvent(), dwDeleteInMs);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :
				return 0;

			// Delete
			case WAIT_TIMEOUT :		
			{
				// Alternatively call them
				if (m_dwCounter & 0x1U)
				{
					if (!DeleteIt(m_pDoc->m_sDetectionAutoSaveDir,	m_pDoc->m_nDeleteDetectionsOlderThanDays))
						return 0; // Exit Thread
					if (!DeleteIt(m_pDoc->m_sSnapshotAutoSaveDir,	m_pDoc->m_nDeleteSnapshotsOlderThanDays))
						return 0; // Exit Thread
					if (!DeleteIt(m_pDoc->m_sRecordAutoSaveDir,		m_pDoc->m_nDeleteRecordingsOlderThanDays))
						return 0; // Exit Thread
				}
				else
				{
					if (!DeleteIt(m_pDoc->m_sRecordAutoSaveDir,		m_pDoc->m_nDeleteRecordingsOlderThanDays))
						return 0; // Exit Thread
					if (!DeleteIt(m_pDoc->m_sSnapshotAutoSaveDir,	m_pDoc->m_nDeleteSnapshotsOlderThanDays))
						return 0; // Exit Thread
					if (!DeleteIt(m_pDoc->m_sDetectionAutoSaveDir,	m_pDoc->m_nDeleteDetectionsOlderThanDays))
						return 0; // Exit Thread
				}

				// Inc. Counter
				m_dwCounter++;

				break;
			}

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
	CTime t = CTime::GetCurrentTime();
	t = CTime(2000, 1, 1, t.GetHour(), t.GetMinute(), t.GetSecond());

	// Disable Message Box Show
	m_pDib->SetShowMessageBoxOnError(FALSE);
	
	// Allocate Helper Dibs
	m_pProcessFrameDib = new CDib;
	m_pProcessFrameDib->SetShowMessageBoxOnError(FALSE);
	m_pProcessFrameExtraDib = new CDib;
	m_pProcessFrameExtraDib->SetShowMessageBoxOnError(FALSE);

	// General Vars
	m_bResetSettings = FALSE;
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
	m_bRecDeinterlace = FALSE;
	memset(&m_CaptureBMI, 0, sizeof(BITMAPINFOFULL));
	memset(&m_ProcessFrameBMI, 0, sizeof(BITMAPINFOFULL));
	m_dFrameRate = DEFAULT_FRAMERATE;
	m_dEffectiveFrameRate = 0.0;
	m_dEffectiveFrameTimeSum = 0.0;
	m_dwEffectiveFrameTimeCountUp = 0U;
	m_pVideoAviDoc = NULL;
	m_bDoEditCopy = FALSE;
	m_bDoEditSnapshot = FALSE;
	m_lProcessFrameTime = 0;
	m_lCompressedDataRate = 0;
	m_lCompressedDataRateSum = 0;
	m_bCaptureStarted = FALSE;
	m_bShowFrameTime = TRUE;
	m_bVideoView = TRUE;
	m_dwVideoProcessorMode = NO_DETECTOR;
	m_bDecodeFramesForPreview = FALSE;
	m_dwFrameCountUp = 0U;
	m_bSizeToDoc = TRUE;
	m_bDeviceFirstRun = FALSE;
	m_1SecTime = t;
	m_4SecTime = t;

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
	m_pSendFrameNetCom = NULL;
	m_pGetFrameNetCom = NULL;
	m_pHttpGetFrameParseProcess = NULL;
	m_pSendFrameParseProcess = (CSendFrameParseProcess*)new CSendFrameParseProcess(this);
	m_pGetFrameParseProcess = NULL;
	m_pGetFrameGenerator = NULL;
	m_nSendFrameConnectionsCount = 0;
	m_nSendFrameVideoPort = DEFAULT_UDP_PORT;
	m_nSendFrameMaxConnectionsConfig = m_nSendFrameMaxConnections = DEFAULT_SENDFRAME_CONNECTIONS;
	m_nSendFrameMTU = DEFAULT_SENDFRAME_FRAGMENT_SIZE;
	m_nSendFrameDataRate = DEFAULT_SENDFRAME_DATARATE;
	m_nSendFrameSizeDiv = 0;
	m_nSendFrameFreqDiv = 1;
	m_bSendVideoFrame = FALSE;
	m_dwLastSendUDPKeyFrameUpTime = 0U;
	m_wLastSendUDPFrameSeq = 0U;
	m_sGetFrameVideoHost = _T("");
	m_nGetFrameVideoPort = DEFAULT_UDP_PORT;
	m_dwGetFrameMaxFrames = NETFRAME_DEFAULT_FRAMES;
	m_bGetFrameDisableResend = FALSE;
	m_nNetworkDeviceTypeMode = INTERNAL_UDP;
	m_dwMaxSendFrameFragmentsPerFrame = 0U;
	m_dwSendFrameTotalSentBytes = 0U;
	m_dwSendFrameOverallDatarate = 0U;
	m_dSendFrameDatarateCorrection = 1.0;
	m_dwSendFrameTotalLastSentBytes = 0U;
	m_nHttpVideoQuality = DEFAULT_HTTP_VIDEO_QUALITY;
	m_nHttpVideoSizeX = DEFAULT_HTTP_VIDEO_SIZE_CX;
	m_nHttpVideoSizeY = DEFAULT_HTTP_VIDEO_SIZE_CY;
	m_nHttpGetFrameLocationPos = 0;
	m_HttpGetFrameLocations.Add(_T("/"));								// Start trying home to see whether cam is reachable
	// First try JPEG because we can change the framerate
	m_HttpGetFrameLocations.Add(_T("/image.jpg"));						// Many cams
	m_HttpGetFrameLocations.Add(_T("/IMAGE.JPG"));						// Many cams
	m_HttpGetFrameLocations.Add(_T("/goform/video2"));					// REPOTEC, TRENDNET, PLANET
	m_HttpGetFrameLocations.Add(_T("/goform/video"));					// REPOTEC, TRENDNET, PLANET
	m_HttpGetFrameLocations.Add(_T("/goform/capture"));					// REPOTEC, TRENDNET, PLANET
	m_HttpGetFrameLocations.Add(_T("/cgi/jpg/image.cgi"));				// TRENDNET
	m_HttpGetFrameLocations.Add(_T("/Jpeg/CamImg.jpg"));				// PLANET, SOLWISE, GADSPOT, VEO Observer
	m_HttpGetFrameLocations.Add(_T("/netcam.jpg"));						// STARDOT
	m_HttpGetFrameLocations.Add(_T("/jpg/image.jpg"));					// EDIMAX, INTELLINET
	m_HttpGetFrameLocations.Add(_T("/record/current.jpg"));				// MOBOTIX
	m_HttpGetFrameLocations.Add(_T("/cgi-bin/getimage.cgi?motion=0"));	// GADSPOT GS1200G
	m_HttpGetFrameLocations.Add(_T("/cgi-bin/video.jpg"));				// D-LINK
	m_HttpGetFrameLocations.Add(_T("/image/jpeg.cgi"));					// D-LINK
	m_HttpGetFrameLocations.Add(_T("/cgi-bin/image.cgi?control=0&id=admin&passwd=admin")); // Blue Net Video Server
	m_HttpGetFrameLocations.Add(_T("/img/snapshot.cgi"));				// LINKSYS
	m_HttpGetFrameLocations.Add(_T("/snapshot.cgi"));					// BSTI, Heden VisionCam
	m_HttpGetFrameLocations.Add(_T("/oneshotimage.jpg"));				// SONY
	m_HttpGetFrameLocations.Add(_T("/-wvhttp-01-/GetLiveImage"));		// CANON
	// then try mixed JPEG/MJPEG
	m_HttpGetFrameLocations.Add(_T("/image"));							// JPEG for ARECONT VISION and MJPEG for SONY
	// finally try MJPEG
	m_HttpGetFrameLocations.Add(_T("/video.cgi"));						// Many cams
	m_HttpGetFrameLocations.Add(_T("/VIDEO.CGI"));						// Many cams
	m_HttpGetFrameLocations.Add(_T("/GetData.cgi"));					// GADSPOT, ORITE, PLANET
	m_HttpGetFrameLocations.Add(_T("/cgi-bin/auto.cgi"));				// ANEXTEK

	// Snapshot
	m_sSnapshotAutoSaveDir = _T("");
	m_bSnapshotLiveJpeg = TRUE;
	m_bSnapshotHistoryJpeg = FALSE;
	m_bSnapshotHistorySwf = FALSE;
	m_bSnapshotLiveJpegFtp = FALSE;
	m_bSnapshotHistoryJpegFtp = FALSE;
	m_bSnapshotHistorySwfFtp = FALSE;
	m_bSnapshotHistoryDeinterlace = FALSE;
	m_bManualSnapshotAutoOpen = TRUE;
	m_nSnapshotRate = DEFAULT_SNAPSHOT_RATE;
	m_nSnapshotHistoryFrameRate = DEFAULT_SNAPSHOT_HISTORY_FRAMERATE;
	m_bSnapshotHistoryCloseSwfFile = 0;
	m_nSnapshotCompressionQuality = DEFAULT_SNAPSHOT_COMPR_QUALITY;
	m_fSnapshotVideoCompressorQuality = DEFAULT_VIDEO_QUALITY;
	m_bSnapshotThumb = TRUE;
	m_nSnapshotThumbWidth = DEFAULT_SNAPSHOT_THUMB_WIDTH;
	m_nSnapshotThumbHeight = DEFAULT_SNAPSHOT_THUMB_HEIGHT;
	m_dwNextSnapshotUpTime = 0U;
	m_bSnapshotStartStop = FALSE;
	m_SnapshotStartTime = t;
	m_SnapshotStopTime = t;
	m_nDeleteSnapshotsOlderThanDays = 0;

	// Threads Init
	m_CaptureAudioThread.SetDoc(this);
	m_HttpGetFrameThread.SetDoc(this);
	m_WatchdogAndDrawThread.SetDoc(this);
	m_DeleteThread.SetDoc(this);
	m_SaveFrameListThread.SetDoc(this);

	// Recording
	m_sRecordAutoSaveDir = _T("");
	m_bRecAutoOpen = TRUE;
	m_bRecAutoOpenAllowed = TRUE;
	m_bRecTimeSegmentation = FALSE;
	m_nTimeSegmentationIndex = 0;
	m_bAudioRecWait = TRUE;
	m_bVideoRecWait = TRUE;
	m_bStopRec = FALSE;
	m_bCaptureRecordPause = FALSE;
	m_bRecResume = FALSE;
	m_bAboutToStopRec = FALSE;
	m_bAboutToStartRec = FALSE;
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
	m_sDetectionAutoSaveDir = _T("");
	m_sDetectionTriggerFileName = _T("");
	m_DetectionTriggerLastWriteTime.dwLowDateTime = 0;
	m_DetectionTriggerLastWriteTime.dwHighDateTime = 0;
	m_nMilliSecondsRecBeforeMovementBegin = DEFAULT_PRE_BUFFER_MSEC;
	m_nMilliSecondsRecAfterMovementEnd = DEFAULT_POST_BUFFER_MSEC;
	m_bDoAdjacentZonesDetection = TRUE;
	m_bDoLumChangeDetection = TRUE;
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
	m_nDetectionLevel = DEFAULT_MOVDET_LEVEL;
	m_nDetectionZoneSize = 0;
	m_nMovementDetectorIntensityLimit = DEFAULT_MOVDET_INTENSITY_LIMIT;
	m_dwAnimatedGifWidth = MOVDET_ANIMGIF_DEFAULT_WIDTH;
	m_dwAnimatedGifHeight = MOVDET_ANIMGIF_DEFAULT_HEIGHT;
	m_LumChangeDetectorBkgY = new int[MOVDET_MAX_ZONES];
	m_LumChangeDetectorDiffY = new int[MOVDET_MAX_ZONES];
	m_MovementDetectorCurrentIntensity = new int[MOVDET_MAX_ZONES];
	m_MovementDetectionsUpTime = new DWORD[MOVDET_MAX_ZONES];
	m_MovementDetections = new BOOL[MOVDET_MAX_ZONES];
	m_DoMovementDetection = new BOOL[MOVDET_MAX_ZONES];
	m_lMovDetXZonesCount = MOVDET_MIN_ZONES_XORY;
	m_lMovDetYZonesCount = MOVDET_MIN_ZONES_XORY;
	m_lMovDetTotalZones = 0;
	m_nVideoDetDataRate = DEFAULT_VIDEO_DATARATE;
	m_nVideoDetKeyframesRate = DEFAULT_KEYFRAMESRATE;
	m_fVideoDetQuality = ((CUImagerApp*)::AfxGetApp())->m_fFFPreferredVideoEncQuality;
	m_nVideoDetQualityBitrate = 0;
	m_dwVideoDetFourCC = ((CUImagerApp*)::AfxGetApp())->m_dwFFPreferredVideoEncFourCC;
	m_bVideoDetDeinterlace = FALSE;
	m_nVideoDetSwfDataRate = DEFAULT_VIDEO_DATARATE;
	m_nVideoDetSwfKeyframesRate = DEFAULT_KEYFRAMESRATE;
	m_fVideoDetSwfQuality = DEFAULT_VIDEO_QUALITY;
	m_nVideoDetSwfQualityBitrate = 0;
	m_dwVideoDetSwfFourCC = FCC('FLV1');
	m_bVideoDetSwfDeinterlace = FALSE;
	m_bDetectionStartStop = FALSE;
	m_DetectionStartTime = t;
	m_DetectionStopTime = t;
	m_nDeleteDetectionsOlderThanDays = 0;
	m_bUnsupportedVideoSizeForMovDet = FALSE;
	m_nMovDetFreqDiv = 1;
	m_dMovDetFrameRateFreqDivCalc = 0.0;

	// Property Sheet
	m_pMovementDetectionPage = NULL;
	m_pGeneralPage = NULL;
	m_pSnapshotPage = NULL;
	m_pNetworkPage = NULL;
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
	m_MovDetSendMailConfiguration.m_bAutoDial = FALSE;
	m_MovDetSendMailConfiguration.m_sBoundIP = _T("");
	m_MovDetSendMailConfiguration.m_sEncodingFriendly = _T("Western European (ISO)");
	m_MovDetSendMailConfiguration.m_sEncodingCharset = _T("iso-8859-1");
	m_MovDetSendMailConfiguration.m_bMime = TRUE;
	m_MovDetSendMailConfiguration.m_bHTML = TRUE;
#if (_MSC_VER > 1200)
	m_MovDetSendMailConfiguration.m_ConnectionType = CPJNSMTPConnection::PlainText;
	m_MovDetSendMailConfiguration.m_Priority = CPJNSMTPMessage::NoPriority;
#else
	m_MovDetSendMailConfiguration.m_Priority = CPJNSMTPMessage::NO_PRIORITY;
#endif

	// FTP Settings
	m_MovDetFTPUploadConfiguration.m_sHost = _T("");
	m_MovDetFTPUploadConfiguration.m_sRemoteDir = _T("");
	m_MovDetFTPUploadConfiguration.m_nPort = 21;
	m_MovDetFTPUploadConfiguration.m_bPasv = TRUE;
	m_MovDetFTPUploadConfiguration.m_bBinary = TRUE;
	m_MovDetFTPUploadConfiguration.m_dwConnectionTimeout = FTP_CONNECTION_TIMEOUT_MS;
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
	m_SnapshotFTPUploadConfiguration.m_dwConnectionTimeout = FTP_CONNECTION_TIMEOUT_MS;
	m_SnapshotFTPUploadConfiguration.m_bProxy = FALSE;
	m_SnapshotFTPUploadConfiguration.m_sProxy = _T("");
	m_SnapshotFTPUploadConfiguration.m_sUsername = _T("");
	m_SnapshotFTPUploadConfiguration.m_sPassword = _T("");
	m_SnapshotFTPUploadConfiguration.m_FilesToUpload = FILES_TO_UPLOAD_AVI; // Not used

	// Init Command Execution on Detection Critical Section
	::InitializeCriticalSection(&m_csExecCommandMovementDetection);

	// Init Re-Send UDP Frame List Critical Section
	::InitializeCriticalSection(&m_csReSendUDPFrameList);

	// Init Avi File Critical Section
	::InitializeCriticalSection(&m_csAVRec);

	// Init Movement Detections List Critical Section
	::InitializeCriticalSection(&m_csMovementDetectionsList);

	// Init Send Frame Critical Section
	::InitializeCriticalSection(&m_csSendFrameNetCom);

	// Init Http Video Size and Compression Critical Section
	::InitializeCriticalSection(&m_csHttpParams);

	// Init Http Video Processing Image Data Critical Section
	::InitializeCriticalSection(&m_csHttpProcess);

	// Init Snapshot FTP Upload Configuration Critical Section
	::InitializeCriticalSection(&m_csSnapshotFTPUploadConfiguration);

	// Init OSD Message Critical Section
	::InitializeCriticalSection(&m_csOSDMessage);

	// Init Process Frame Stop Engine Critical Section
	::InitializeCriticalSection(&m_csProcessFrameStop);

	// Init Movement Detector
	OneEmptyFrameList();
	ResetMovementDetector();

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
	if (m_pSendFrameParseProcess)
	{
		delete m_pSendFrameParseProcess;
		m_pSendFrameParseProcess = NULL;
	}
	if (m_pGetFrameParseProcess)
	{
		delete m_pGetFrameParseProcess;
		m_pGetFrameParseProcess = NULL;
	}
	if (m_pGetFrameGenerator)
	{
		delete m_pGetFrameGenerator;
		m_pGetFrameGenerator = NULL;
	}
	FreeMovementDetector();
	if (m_LumChangeDetectorBkgY)
	{
		delete [] m_LumChangeDetectorBkgY;
		m_LumChangeDetectorBkgY = NULL;
	}
	if (m_LumChangeDetectorDiffY)
	{
		delete [] m_LumChangeDetectorDiffY;
		m_LumChangeDetectorDiffY = NULL;
	}
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
	ClearReSendUDPFrameList();
	::DeleteCriticalSection(&m_csProcessFrameStop);
	::DeleteCriticalSection(&m_csOSDMessage);
	::DeleteCriticalSection(&m_csSnapshotFTPUploadConfiguration);
	::DeleteCriticalSection(&m_csHttpProcess);
	::DeleteCriticalSection(&m_csHttpParams);
	::DeleteCriticalSection(&m_csSendFrameNetCom);
	::DeleteCriticalSection(&m_csMovementDetectionsList);
	::DeleteCriticalSection(&m_csAVRec);
	::DeleteCriticalSection(&m_csReSendUDPFrameList);
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
}

void CVideoDeviceDoc::CloseDocument()
{
	GetFrame()->PostMessage(WM_CLOSE, 0, 0);
}

void CVideoDeviceDoc::CloseDocRemoveAutorunDev()
{
	::PostMessage(	GetView()->GetSafeHwnd(),
					WM_THREADSAFE_AUTORUNREMOVEDEVICE_CLOSEDOC,
					0, 0);
}

CString CVideoDeviceDoc::GetDevicePathName()
{
	CString sDevice(_T(""));

	if (m_pDxCapture)
		sDevice = m_pDxCapture->GetDevicePath();
	else if (((CUImagerApp*)::AfxGetApp())->IsDoc((CUImagerDoc*)m_pVideoAviDoc))
	{
		CString sShortFileName;
		int index;
		if ((index = m_pVideoAviDoc->GetPathName().ReverseFind(_T('\\'))) >= 0)
			sDevice = m_pVideoAviDoc->GetPathName().Right(m_pVideoAviDoc->GetPathName().GetLength() - index - 1);
		else
			sDevice = m_pVideoAviDoc->GetPathName();
	}
	else if (m_pGetFrameNetCom)
		sDevice.Format(_T("%s:%d:%s:%d"), m_sGetFrameVideoHost, m_nGetFrameVideoPort, m_HttpGetFrameLocations[0], m_nNetworkDeviceTypeMode);

	// Registry keys cannot begin with a backslash and should
	// not contain backslashes otherwise subkeys are created!
	sDevice.Replace(_T('\\'), _T('/'));

	return sDevice;
}

CString CVideoDeviceDoc::GetAssignedDeviceName()
{
	CString sName = GetAutoSaveDir();
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
	CString sDevice(_T(""));

	if (m_pDxCapture)
		sDevice = m_pDxCapture->GetDeviceName();
	else if (((CUImagerApp*)::AfxGetApp())->IsDoc((CUImagerDoc*)m_pVideoAviDoc))
	{
		CString sShortFileName;
		int index;
		if ((index = m_pVideoAviDoc->GetPathName().ReverseFind(_T('\\'))) >= 0)
			sDevice = m_pVideoAviDoc->GetPathName().Right(m_pVideoAviDoc->GetPathName().GetLength() - index - 1);
		else
			sDevice = m_pVideoAviDoc->GetPathName();
	}
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
	CString strInfo(_T(""));

	// Name
	CString sName = GetAssignedDeviceName();

	// General info
	if (m_DocRect.Width() > 0 && 
		m_DocRect.Height() > 0)
	{
		// Set format string
		CString sFormat = _T("");
		if (m_CaptureBMI.bmiHeader.biCompression != m_ProcessFrameBMI.bmiHeader.biCompression)
			sFormat = CDib::GetCompressionName((LPBITMAPINFO)&m_CaptureBMI) + _T(" -> ");
		sFormat += CDib::GetCompressionName((LPBITMAPINFO)&m_ProcessFrameBMI);
		if (m_bDecodeFramesForPreview)
			sFormat += _T(" -> RGB32");

		// Name , Size , Frame rate , Pixel format
		strInfo.Format(
			_T("%s , %dx%d , %0.1ff/s , %s"),
			sName,
			m_DocRect.Width(), 
			m_DocRect.Height(),
			m_dEffectiveFrameRate >= MIN_FRAMERATE ? m_dEffectiveFrameRate : m_dFrameRate,
			sFormat);
	}
	else
		strInfo = sName;

	// Set title string
	CDocument::SetTitle(strInfo);
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

			// Clip to MDI rect (ClipToMDIRect() not working for all OSs if auto starting minimized to tray)
			if (!pApp->m_bTrayIcon || !::AfxGetMainFrame()->m_TrayIcon.IsMinimizedToTray())
				::AfxGetMainFrame()->ClipToMDIRect(&pwp->rcNormalPosition);

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
	}

	// Update m_ZoomRect and show the Please wait... message
	GetView()->UpdateWindowSizes(TRUE, FALSE, FALSE);

	// Device First Run
	m_bDeviceFirstRun = pApp->GetProfileString(sSection, _T("DeviceName"), _T("")) == _T("") ? TRUE : FALSE;

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
#if (_MSC_VER > 1200)
	m_MovDetSendMailConfiguration.m_ConnectionType = (CPJNSMTPConnection::ConnectionType) pApp->GetProfileInt(sSection, _T("SendMailConnectionType"), CPJNSMTPConnection::PlainText);
#endif

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
	m_nSendFrameVideoPort = (int) pApp->GetProfileInt(sSection, _T("SendFrameVideoPort"), DEFAULT_UDP_PORT);
	m_nSendFrameMaxConnectionsConfig = m_nSendFrameMaxConnections = (int) pApp->GetProfileInt(sSection, _T("SendFrameMaxConnections"), DEFAULT_SENDFRAME_CONNECTIONS);
	m_nSendFrameMTU = (int) pApp->GetProfileInt(sSection, _T("SendFrameMTU"), DEFAULT_SENDFRAME_FRAGMENT_SIZE);
	m_nSendFrameDataRate = (int) pApp->GetProfileInt(sSection, _T("SendFrameDataRate"), DEFAULT_SENDFRAME_DATARATE);
	m_nSendFrameSizeDiv = (int) pApp->GetProfileInt(sSection, _T("SendFrameSizeDiv"), 0);
	m_nSendFrameFreqDiv = (int) pApp->GetProfileInt(sSection, _T("SendFrameFreqDiv"), 1);
	m_bSendVideoFrame = (int) pApp->GetProfileInt(sSection, _T("DoSendVideoFrame"), FALSE);
	m_dwGetFrameMaxFrames = (DWORD) pApp->GetProfileInt(sSection, _T("GetFrameMaxFrames"), NETFRAME_DEFAULT_FRAMES);
	m_bGetFrameDisableResend = (BOOL) pApp->GetProfileInt(sSection, _T("GetFrameDisableResend"), FALSE); 
	m_nHttpVideoQuality = (int) pApp->GetProfileInt(sSection, _T("HTTPVideoQuality"), DEFAULT_HTTP_VIDEO_QUALITY);
	m_nHttpVideoSizeX = (int) pApp->GetProfileInt(sSection, _T("HTTPVideoSizeX"), DEFAULT_HTTP_VIDEO_SIZE_CX);
	m_nHttpVideoSizeY = (int) pApp->GetProfileInt(sSection, _T("HTTPVideoSizeY"), DEFAULT_HTTP_VIDEO_SIZE_CY);
	m_sHttpGetFrameUsername = pApp->GetSecureProfileString(sSection, _T("HTTPGetFrameUsername"), _T(""));
	m_sHttpGetFramePassword = pApp->GetSecureProfileString(sSection, _T("HTTPGetFramePassword"), _T(""));
	m_sGetFrameUsername = pApp->GetSecureProfileString(sSection, _T("GetFrameUsername"), _T(""));
	m_sGetFramePassword = pApp->GetSecureProfileString(sSection, _T("GetFramePassword"), _T(""));
	m_sSendFrameUsername = pApp->GetSecureProfileString(sSection, _T("SendFrameUsername"), _T(""));
	m_sSendFramePassword = pApp->GetSecureProfileString(sSection, _T("SendFramePassword"), _T(""));

	// All other
	m_bVideoView = (BOOL) pApp->GetProfileInt(sSection, _T("VideoView"), TRUE);
	m_bDeinterlace = (BOOL) pApp->GetProfileInt(sSection, _T("Deinterlace"), FALSE);
	m_bRotate180 = (BOOL) pApp->GetProfileInt(sSection, _T("Rotate180"), FALSE);
	m_bRecDeinterlace = (BOOL) pApp->GetProfileInt(sSection, _T("RecDeinterlace"), FALSE);
	m_bRecAutoOpen = (BOOL) pApp->GetProfileInt(sSection, _T("RecAutoOpen"), TRUE);
	m_bRecTimeSegmentation = (BOOL) pApp->GetProfileInt(sSection, _T("RecTimeSegmentation"), FALSE);
	m_nTimeSegmentationIndex = pApp->GetProfileInt(sSection, _T("TimeSegmentationIndex"), 0);
	m_sRecordAutoSaveDir = pApp->GetProfileString(sSection, _T("RecordAutoSaveDir"), sDefaultAutoSaveDir);
	m_sDetectionAutoSaveDir = pApp->GetProfileString(sSection, _T("DetectionAutoSaveDir"), sDefaultAutoSaveDir);
	m_sDetectionTriggerFileName = pApp->GetProfileString(sSection, _T("DetectionTriggerFileName"), _T("movtrigger.txt"));
	CString sDetectionTriggerFileName(m_sDetectionTriggerFileName);
	sDetectionTriggerFileName.TrimLeft();
	sDetectionTriggerFileName.TrimRight();
	sDetectionTriggerFileName.TrimLeft(_T('\"'));
	sDetectionTriggerFileName.TrimRight(_T('\"'));
	if (sDetectionTriggerFileName.Find(_T('\\')) < 0)
	{
		CString sDetectionAutoSaveDir = m_sDetectionAutoSaveDir;
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
	m_bSnapshotHistoryDeinterlace = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotHistoryDeinterlace"), FALSE);
	m_bManualSnapshotAutoOpen = (BOOL) pApp->GetProfileInt(sSection, _T("ManualSnapshotAutoOpen"), TRUE);
	m_nSnapshotRate = (int) pApp->GetProfileInt(sSection, _T("SnapshotRate"), DEFAULT_SNAPSHOT_RATE);
	m_nSnapshotHistoryFrameRate = (int) pApp->GetProfileInt(sSection, _T("SnapshotHistoryFrameRate"), DEFAULT_SNAPSHOT_HISTORY_FRAMERATE);
	m_nSnapshotCompressionQuality = (int) pApp->GetProfileInt(sSection, _T("SnapshotCompressionQuality"), DEFAULT_SNAPSHOT_COMPR_QUALITY);
	m_fSnapshotVideoCompressorQuality = (float) pApp->GetProfileInt(sSection, _T("SnapshotVideoCompressorQuality"), (int)DEFAULT_VIDEO_QUALITY);
	m_sSnapshotAutoSaveDir = pApp->GetProfileString(sSection, _T("SnapshotAutoSaveDir"), sDefaultAutoSaveDir);
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
	m_dwCaptureAudioDeviceID = (int) pApp->GetProfileInt(sSection, _T("AudioCaptureDeviceID"), 0);
	m_nDeviceInputId = (int) pApp->GetProfileInt(sSection, _T("VideoCaptureDeviceInputID"), -1);
	m_nDeviceFormatId = (int) pApp->GetProfileInt(sSection, _T("VideoCaptureDeviceFormatID"), -1);
	m_nDeviceFormatWidth = (int) pApp->GetProfileInt(sSection, _T("VideoCaptureDeviceFormatWidth"), 0);
	m_nDeviceFormatHeight = (int) pApp->GetProfileInt(sSection, _T("VideoCaptureDeviceFormatHeight"), 0);
	m_nMilliSecondsRecBeforeMovementBegin = (int) pApp->GetProfileInt(sSection, _T("MilliSecondsRecBeforeMovementBegin"), DEFAULT_PRE_BUFFER_MSEC);
	m_nMilliSecondsRecAfterMovementEnd = (int) pApp->GetProfileInt(sSection, _T("MilliSecondsRecAfterMovementEnd"), DEFAULT_POST_BUFFER_MSEC);
	m_nDetectionLevel = (int) pApp->GetProfileInt(sSection, _T("DetectionLevel"), DEFAULT_MOVDET_LEVEL);
	m_nDetectionZoneSize = (int) pApp->GetProfileInt(sSection, _T("DetectionZoneSize"), 0);
	m_bDoAdjacentZonesDetection = (BOOL) pApp->GetProfileInt(sSection, _T("DoAdjacentZonesDetection"), TRUE);
	m_bDoLumChangeDetection = (BOOL) pApp->GetProfileInt(sSection, _T("DoLumChangeDetection"), TRUE);
	m_bSaveSWFMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("SaveSWFMovementDetection"), TRUE);
	m_bSaveAVIMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("SaveAVIMovementDetection"), FALSE);
	m_bSaveAnimGIFMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("SaveAnimGIFMovementDetection"), TRUE);
	m_bSendMailMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("SendMailMovementDetection"), FALSE);
	m_bFTPUploadMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("FTPUploadMovementDetection"), FALSE);
	m_bExecCommandMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("DoExecCommandMovementDetection"), FALSE);
	m_nExecModeMovementDetection = pApp->GetProfileInt(sSection, _T("ExecModeMovementDetection"), 0);
	m_sExecCommandMovementDetection = pApp->GetProfileString(sSection, _T("ExecCommandMovementDetection"), _T(""));
	m_sExecParamsMovementDetection = pApp->GetProfileString(sSection, _T("ExecParamsMovementDetection"), _T(""));
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
	m_bVideoDetDeinterlace = (BOOL) pApp->GetProfileInt(sSection, _T("VideoDetDeinterlace"), FALSE);
	m_fVideoDetQuality = (float) pApp->GetProfileInt(sSection, _T("VideoDetQuality"), (int)(((CUImagerApp*)::AfxGetApp())->m_fFFPreferredVideoEncQuality));
	m_nVideoDetQualityBitrate = (int) pApp->GetProfileInt(sSection, _T("VideoDetQualityBitrate"), 0);
	m_nVideoDetKeyframesRate = (int) pApp->GetProfileInt(sSection, _T("VideoDetKeyframesRate"), DEFAULT_KEYFRAMESRATE);
	m_nVideoDetDataRate = (int) pApp->GetProfileInt(sSection, _T("VideoDetDataRate"), DEFAULT_VIDEO_DATARATE);
	m_dwVideoDetSwfFourCC = (DWORD) pApp->GetProfileInt(sSection, _T("VideoDetSwfFourCC"), FCC('FLV1'));
	m_bVideoDetSwfDeinterlace = (BOOL) pApp->GetProfileInt(sSection, _T("VideoDetSwfDeinterlace"), FALSE);
	m_fVideoDetSwfQuality = (float) pApp->GetProfileInt(sSection, _T("VideoDetSwfQuality"), (int)DEFAULT_VIDEO_QUALITY);
	m_nVideoDetSwfQualityBitrate = (int) pApp->GetProfileInt(sSection, _T("VideoDetSwfQualityBitrate"), 0);
	m_nVideoDetSwfKeyframesRate = (int) pApp->GetProfileInt(sSection, _T("VideoDetSwfKeyframesRate"), DEFAULT_KEYFRAMESRATE);
	m_nVideoDetSwfDataRate = (int) pApp->GetProfileInt(sSection, _T("VideoDetSwfDataRate"), DEFAULT_VIDEO_DATARATE);
	m_bDetectionStartStop = (BOOL) pApp->GetProfileInt(sSection, _T("DetectionStartStop"), FALSE);
	m_DetectionStartTime = CTime(2000, 1, 1,	pApp->GetProfileInt(sSection, _T("DetectionStartHour"), t.GetHour()),
												pApp->GetProfileInt(sSection, _T("DetectionStartMin"), t.GetMinute()),
												pApp->GetProfileInt(sSection, _T("DetectionStartSec"), t.GetSecond()));
	m_DetectionStopTime = CTime(2000, 1, 1,		pApp->GetProfileInt(sSection, _T("DetectionStopHour"), t.GetHour()),
												pApp->GetProfileInt(sSection, _T("DetectionStopMin"), t.GetMinute()),
												pApp->GetProfileInt(sSection, _T("DetectionStopSec"), t.GetSecond()));
	m_bShowFrameTime = (BOOL) pApp->GetProfileInt(sSection, _T("ShowFrameTime"), TRUE);
	m_bShowMovementDetections = (BOOL) pApp->GetProfileInt(sSection, _T("ShowMovementDetections"), FALSE);
	m_nMovementDetectorIntensityLimit = (int) pApp->GetProfileInt(sSection, _T("IntensityLimit"), DEFAULT_MOVDET_INTENSITY_LIMIT);
	m_dwAnimatedGifWidth = (DWORD) pApp->GetProfileInt(sSection, _T("AnimatedGifWidth"), MOVDET_ANIMGIF_DEFAULT_WIDTH);
	m_dwAnimatedGifHeight = (DWORD) pApp->GetProfileInt(sSection, _T("AnimatedGifHeight"), MOVDET_ANIMGIF_DEFAULT_HEIGHT);
	m_nDeleteDetectionsOlderThanDays = (int) pApp->GetProfileInt(sSection, _T("DeleteDetectionsOlderThanDays"), 0);
	m_nDeleteRecordingsOlderThanDays = (int) pApp->GetProfileInt(sSection, _T("DeleteRecordingsOlderThanDays"), 0);
	m_nDeleteSnapshotsOlderThanDays = (int) pApp->GetProfileInt(sSection, _T("DeleteSnapshotsOlderThanDays"), 0);

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
	if (m_CaptureAudioThread.m_pSrcWaveFormat == NULL || uiSize != sizeof(WAVEFORMATEX)) // Default Audio: Mono , 11025 Hz , 8 bits
	{
		// Make Sure Nothing Has Been Allocated!
		if (m_CaptureAudioThread.m_pSrcWaveFormat)
			delete [] m_CaptureAudioThread.m_pSrcWaveFormat;
		m_CaptureAudioThread.m_pSrcWaveFormat = (WAVEFORMATEX*) new BYTE[sizeof(WAVEFORMATEX)];
		m_CaptureAudioThread.m_pSrcWaveFormat->wFormatTag = WAVE_FORMAT_PCM;
		m_CaptureAudioThread.m_pSrcWaveFormat->nChannels = 1;
		m_CaptureAudioThread.m_pSrcWaveFormat->nSamplesPerSec = 11025;
		m_CaptureAudioThread.m_pSrcWaveFormat->nAvgBytesPerSec = 11025;
		m_CaptureAudioThread.m_pSrcWaveFormat->nBlockAlign = 1;
		m_CaptureAudioThread.m_pSrcWaveFormat->wBitsPerSample = 8;
		m_CaptureAudioThread.m_pSrcWaveFormat->cbSize = 0;
	}
	if (m_CaptureAudioThread.m_pDstWaveFormat)
		delete [] m_CaptureAudioThread.m_pDstWaveFormat;
	uiSize = 0;
	pApp->GetProfileBinary(sSection, _T("DstWaveFormat"), (LPBYTE*)&m_CaptureAudioThread.m_pDstWaveFormat, &uiSize);
	if (m_CaptureAudioThread.m_pDstWaveFormat == NULL || uiSize != sizeof(WAVEFORMATEX)) // Default Audio: Mono , 11025 Hz , 8 bits
	{
		// Make Sure Nothing Has Been Allocated!
		if (m_CaptureAudioThread.m_pDstWaveFormat)
			delete [] m_CaptureAudioThread.m_pDstWaveFormat;
		m_CaptureAudioThread.m_pDstWaveFormat = (WAVEFORMATEX*) new BYTE[sizeof(WAVEFORMATEX)];
		m_CaptureAudioThread.m_pDstWaveFormat->wFormatTag = WAVE_FORMAT_PCM;
		m_CaptureAudioThread.m_pDstWaveFormat->nChannels = 1;
		m_CaptureAudioThread.m_pDstWaveFormat->nSamplesPerSec = 11025;
		m_CaptureAudioThread.m_pDstWaveFormat->nAvgBytesPerSec = 11025;
		m_CaptureAudioThread.m_pDstWaveFormat->nBlockAlign = 1;
		m_CaptureAudioThread.m_pDstWaveFormat->wBitsPerSample = 8;
		m_CaptureAudioThread.m_pDstWaveFormat->cbSize = 0;
	}

	// Create dirs
	::CreateDir(m_sRecordAutoSaveDir);
	::CreateDir(m_sDetectionAutoSaveDir);
	::CreateDir(m_sSnapshotAutoSaveDir);

	// Check whether the web files exist in the given directory.
	// With first device run m_sRecordAutoSaveDir, m_sDetectionAutoSaveDir
	// and m_sSnapshotAutoSaveDir are the same
	if (m_bDeviceFirstRun)
		MicroApacheCheckWebFiles(GetAutoSaveDir());

	// Start Send Frame
	if (m_bSendVideoFrame)
	{
		CNetCom* pSendFrameNetCom = (CNetCom*)new CNetCom;
		if (ConnectSendFrameUDP(pSendFrameNetCom, m_nSendFrameVideoPort))
		{
			::EnterCriticalSection(&m_csSendFrameNetCom);
			m_pSendFrameNetCom = pSendFrameNetCom;
			::LeaveCriticalSection(&m_csSendFrameNetCom);
		}
		else
		{
			m_bSendVideoFrame = FALSE;
			delete pSendFrameNetCom;
		}
	}
}

void CVideoDeviceDoc::SaveSettings()
{
	WINDOWPLACEMENT wndpl;
	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();
	CString sSection(GetDevicePathName());

	if (((CUImagerApp*)::AfxGetApp())->m_bUseRegistry)
	{
		if (m_bResetSettings)
		{
			::DeleteRegistryKey(HKEY_CURRENT_USER,	_T("Software\\") +
													CString(MYCOMPANY) + CString(_T("\\")) +
													CString(APPNAME_NOEXT) + _T("\\") +
													sSection);
		}
		else
		{
			// Store the device name to identify the entry when manually looking the registry
			// and to know that it is not the first run of the device
			pApp->WriteProfileString(sSection, _T("DeviceName"), GetDeviceName());

			// Store Placement
			if (!pApp->m_bForceSeparateInstance && !pApp->m_bServiceProcess)
			{
				memset(&wndpl, 0, sizeof(wndpl));
				wndpl.length = sizeof(wndpl);
				if (GetFrame()->GetWindowPlacement(&wndpl))
					pApp->WriteProfileBinary(sSection, _T("WindowPlacement"), (BYTE*)&wndpl, sizeof(wndpl));
			}

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
#if (_MSC_VER > 1200)
			pApp->WriteProfileInt(sSection, _T("SendMailConnectionType"), (int)m_MovDetSendMailConfiguration.m_ConnectionType);
#endif

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
			pApp->WriteProfileInt(sSection, _T("SendFrameVideoPort"), m_nSendFrameVideoPort);
			pApp->WriteProfileInt(sSection, _T("SendFrameMaxConnections"), m_nSendFrameMaxConnectionsConfig);
			pApp->WriteProfileInt(sSection, _T("SendFrameMTU"), m_nSendFrameMTU);
			pApp->WriteProfileInt(sSection, _T("SendFrameDataRate"), m_nSendFrameDataRate);
			pApp->WriteProfileInt(sSection, _T("SendFrameSizeDiv"), m_nSendFrameSizeDiv);
			pApp->WriteProfileInt(sSection, _T("SendFrameFreqDiv"), m_nSendFrameFreqDiv);
			pApp->WriteProfileInt(sSection, _T("DoSendVideoFrame"), m_bSendVideoFrame);
			pApp->WriteProfileInt(sSection, _T("GetFrameMaxFrames"), (int)m_dwGetFrameMaxFrames);
			pApp->WriteProfileInt(sSection, _T("GetFrameDisableResend"), (int)m_bGetFrameDisableResend);
			pApp->WriteProfileInt(sSection, _T("HTTPVideoQuality"), m_nHttpVideoQuality);
			pApp->WriteProfileInt(sSection, _T("HTTPVideoSizeX"), m_nHttpVideoSizeX);
			pApp->WriteProfileInt(sSection, _T("HTTPVideoSizeY"), m_nHttpVideoSizeY);
			pApp->WriteSecureProfileString(sSection, _T("SendFrameUsername"), m_sSendFrameUsername);
			pApp->WriteSecureProfileString(sSection, _T("SendFramePassword"), m_sSendFramePassword);

			// All other
			pApp->WriteProfileInt(sSection, _T("VideoView"), m_bVideoView);
			pApp->WriteProfileInt(sSection, _T("Deinterlace"), (int)m_bDeinterlace);
			pApp->WriteProfileInt(sSection, _T("Rotate180"), (int)m_bRotate180);
			pApp->WriteProfileInt(sSection, _T("RecDeinterlace"), m_bRecDeinterlace);
			pApp->WriteProfileInt(sSection, _T("RecAutoOpen"), m_bRecAutoOpen);
			pApp->WriteProfileInt(sSection, _T("RecTimeSegmentation"), m_bRecTimeSegmentation);
			pApp->WriteProfileInt(sSection, _T("TimeSegmentationIndex"), m_nTimeSegmentationIndex);
			pApp->WriteProfileString(sSection, _T("RecordAutoSaveDir"), m_sRecordAutoSaveDir);
			pApp->WriteProfileString(sSection, _T("DetectionAutoSaveDir"), m_sDetectionAutoSaveDir);
			pApp->WriteProfileString(sSection, _T("DetectionTriggerFileName"), m_sDetectionTriggerFileName);
			pApp->WriteProfileInt(sSection, _T("SnapshotLiveJpeg"), (int)m_bSnapshotLiveJpeg);
			pApp->WriteProfileInt(sSection, _T("SnapshotHistoryJpeg"), (int)m_bSnapshotHistoryJpeg);
			pApp->WriteProfileInt(sSection, _T("SnapshotHistorySwf"), (int)m_bSnapshotHistorySwf);
			pApp->WriteProfileInt(sSection, _T("SnapshotLiveJpegFtp"), (int)m_bSnapshotLiveJpegFtp);
			pApp->WriteProfileInt(sSection, _T("SnapshotHistoryJpegFtp"), (int)m_bSnapshotHistoryJpegFtp);
			pApp->WriteProfileInt(sSection, _T("SnapshotHistorySwfFtp"), (int)m_bSnapshotHistorySwfFtp);
			pApp->WriteProfileInt(sSection, _T("SnapshotHistoryDeinterlace"), (int)m_bSnapshotHistoryDeinterlace);
			pApp->WriteProfileInt(sSection, _T("ManualSnapshotAutoOpen"), (int)m_bManualSnapshotAutoOpen);
			pApp->WriteProfileInt(sSection, _T("SnapshotRate"), m_nSnapshotRate);
			pApp->WriteProfileInt(sSection, _T("SnapshotHistoryFrameRate"), m_nSnapshotHistoryFrameRate);
			pApp->WriteProfileInt(sSection, _T("SnapshotCompressionQuality"), m_nSnapshotCompressionQuality);
			pApp->WriteProfileInt(sSection, _T("SnapshotVideoCompressorQuality"), (int)m_fSnapshotVideoCompressorQuality);
			pApp->WriteProfileString(sSection, _T("SnapshotAutoSaveDir"), m_sSnapshotAutoSaveDir);
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
			pApp->WriteProfileInt(sSection, _T("DetectionLevel"), m_nDetectionLevel);
			pApp->WriteProfileInt(sSection, _T("DetectionZoneSize"), m_nDetectionZoneSize);
			pApp->WriteProfileInt(sSection, _T("DoAdjacentZonesDetection"), m_bDoAdjacentZonesDetection);
			pApp->WriteProfileInt(sSection, _T("DoLumChangeDetection"), m_bDoLumChangeDetection);
			pApp->WriteProfileInt(sSection, _T("SaveSWFMovementDetection"), m_bSaveSWFMovementDetection);
			pApp->WriteProfileInt(sSection, _T("SaveAVIMovementDetection"), m_bSaveAVIMovementDetection);
			pApp->WriteProfileInt(sSection, _T("SaveAnimGIFMovementDetection"), m_bSaveAnimGIFMovementDetection);
			pApp->WriteProfileInt(sSection, _T("SendMailMovementDetection"), m_bSendMailMovementDetection);
			pApp->WriteProfileInt(sSection, _T("FTPUploadMovementDetection"), m_bFTPUploadMovementDetection);
			pApp->WriteProfileInt(sSection, _T("DoExecCommandMovementDetection"), m_bExecCommandMovementDetection);
			pApp->WriteProfileInt(sSection, _T("ExecModeMovementDetection"), m_nExecModeMovementDetection);
			pApp->WriteProfileString(sSection, _T("ExecCommandMovementDetection"), m_sExecCommandMovementDetection);
			pApp->WriteProfileString(sSection, _T("ExecParamsMovementDetection"), m_sExecParamsMovementDetection);
			pApp->WriteProfileInt(sSection, _T("HideExecCommandMovementDetection"), m_bHideExecCommandMovementDetection);
			pApp->WriteProfileInt(sSection, _T("WaitExecCommandMovementDetection"), m_bWaitExecCommandMovementDetection);
			pApp->WriteProfileInt(sSection, _T("VideoRecFourCC"), m_dwVideoRecFourCC);
			pApp->WriteProfileInt(sSection, _T("VideoRecQuality"), (int)m_fVideoRecQuality);
			pApp->WriteProfileInt(sSection, _T("VideoRecKeyframesRate"), m_nVideoRecKeyframesRate);
			pApp->WriteProfileInt(sSection, _T("VideoRecDataRate"), m_nVideoRecDataRate);
			pApp->WriteProfileInt(sSection, _T("VideoRecQualityBitrate"), m_nVideoRecQualityBitrate);
			pApp->WriteProfileInt(sSection, _T("VideoDetFourCC"), m_dwVideoDetFourCC);
			pApp->WriteProfileInt(sSection, _T("VideoDetDeinterlace"), m_bVideoDetDeinterlace);
			pApp->WriteProfileInt(sSection, _T("VideoDetQuality"), (int)m_fVideoDetQuality);
			pApp->WriteProfileInt(sSection, _T("VideoDetQualityBitrate"), m_nVideoDetQualityBitrate);
			pApp->WriteProfileInt(sSection, _T("VideoDetKeyframesRate"), m_nVideoDetKeyframesRate);
			pApp->WriteProfileInt(sSection, _T("VideoDetDataRate"), m_nVideoDetDataRate);
			pApp->WriteProfileInt(sSection, _T("VideoDetSwfFourCC"), m_dwVideoDetSwfFourCC);
			pApp->WriteProfileInt(sSection, _T("VideoDetSwfDeinterlace"), m_bVideoDetSwfDeinterlace);
			pApp->WriteProfileInt(sSection, _T("VideoDetSwfQuality"), (int)m_fVideoDetSwfQuality);
			pApp->WriteProfileInt(sSection, _T("VideoDetSwfQualityBitrate"), m_nVideoDetSwfQualityBitrate);
			pApp->WriteProfileInt(sSection, _T("VideoDetSwfKeyframesRate"), m_nVideoDetSwfKeyframesRate);
			pApp->WriteProfileInt(sSection, _T("VideoDetSwfDataRate"), m_nVideoDetSwfDataRate);
			pApp->WriteProfileInt(sSection, _T("DetectionStartStop"), (int)m_bDetectionStartStop);
			pApp->WriteProfileInt(sSection, _T("DetectionStartHour"), m_DetectionStartTime.GetHour());
			pApp->WriteProfileInt(sSection, _T("DetectionStartMin"), m_DetectionStartTime.GetMinute());
			pApp->WriteProfileInt(sSection, _T("DetectionStartSec"), m_DetectionStartTime.GetSecond());
			pApp->WriteProfileInt(sSection, _T("DetectionStopHour"), m_DetectionStopTime.GetHour());
			pApp->WriteProfileInt(sSection, _T("DetectionStopMin"), m_DetectionStopTime.GetMinute());
			pApp->WriteProfileInt(sSection, _T("DetectionStopSec"), m_DetectionStopTime.GetSecond());
			pApp->WriteProfileInt(sSection, _T("ShowFrameTime"), m_bShowFrameTime);
			pApp->WriteProfileInt(sSection, _T("ShowMovementDetections"), m_bShowMovementDetections);
			pApp->WriteProfileInt(sSection, _T("IntensityLimit"), m_nMovementDetectorIntensityLimit);
			pApp->WriteProfileInt(sSection, _T("AnimatedGifWidth"), m_dwAnimatedGifWidth);
			pApp->WriteProfileInt(sSection, _T("AnimatedGifHeight"), m_dwAnimatedGifHeight);
			pApp->WriteProfileInt(sSection, _T("DeleteDetectionsOlderThanDays"), m_nDeleteDetectionsOlderThanDays);
			pApp->WriteProfileInt(sSection, _T("DeleteRecordingsOlderThanDays"), m_nDeleteRecordingsOlderThanDays);
			pApp->WriteProfileInt(sSection, _T("DeleteSnapshotsOlderThanDays"), m_nDeleteSnapshotsOlderThanDays);

			pApp->WriteProfileInt(sSection, _T("MovDetTotalZones"), m_lMovDetTotalZones);
			for (int i = 0 ; i < m_lMovDetTotalZones ; i++)
			{
				CString sZone;
				sZone.Format(MOVDET_ZONE_FORMAT, i);
				pApp->WriteProfileInt(sSection, sZone, m_DoMovementDetection[i]);
			}

			if (m_CaptureAudioThread.m_pSrcWaveFormat)
				pApp->WriteProfileBinary(sSection, _T("SrcWaveFormat"), (LPBYTE)m_CaptureAudioThread.m_pSrcWaveFormat, sizeof(WAVEFORMATEX));
			if (m_CaptureAudioThread.m_pDstWaveFormat)
				pApp->WriteProfileBinary(sSection, _T("DstWaveFormat"), (LPBYTE)m_CaptureAudioThread.m_pDstWaveFormat, sizeof(WAVEFORMATEX));
			pApp->WriteProfileInt(sSection, _T("VideoProcessorMode"), m_dwVideoProcessorMode);
			unsigned int nSize = sizeof(m_dFrameRate);
			pApp->WriteProfileBinary(sSection, _T("FrameRate"), (LPBYTE)&m_dFrameRate, nSize);
		}
	}
	else
	{
		if (m_bResetSettings)
		{
			TCHAR s[2] = {_T('\0'), _T('\0')};
			::WritePrivateProfileSection(sSection, s, pApp->m_pszProfileName);
		}
		else
		{
			// Ini file writing is slow, especially on memory sticks
			BeginWaitCursor();

			// Make a temporary copy because writing to memory sticks is so slow! 
			CString sTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), pApp->m_pszProfileName);
			::WritePrivateProfileString(NULL, NULL, NULL, pApp->m_pszProfileName); // recache
			::CopyFile(pApp->m_pszProfileName, sTempFileName, FALSE);
		
			// Store the device name to identify the entry when manually looking the registry
			// and to know that it is not the first run of the device
			::WriteProfileIniString(sSection, _T("DeviceName"), GetDeviceName(), sTempFileName);

			// Store Placement
			if (!pApp->m_bForceSeparateInstance && !pApp->m_bServiceProcess)
			{
				memset(&wndpl, 0, sizeof(wndpl));
				wndpl.length = sizeof(wndpl);
				if (GetFrame()->GetWindowPlacement(&wndpl))
					::WriteProfileIniBinary(sSection, _T("WindowPlacement"), (BYTE*)&wndpl, sizeof(wndpl), sTempFileName);
			}

			// Email Settings
			::WriteProfileIniString(sSection, _T("SendMailFiles"), m_MovDetSendMailConfiguration.m_sFiles, sTempFileName);
			::WriteProfileIniInt(sSection, _T("AttachmentType"), (int)m_MovDetSendMailConfiguration.m_AttachmentType, sTempFileName);
			::WriteProfileIniString(sSection, _T("SendMailSubject"), m_MovDetSendMailConfiguration.m_sSubject, sTempFileName);
			::WriteProfileIniString(sSection, _T("SendMailTo"), m_MovDetSendMailConfiguration.m_sTo, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SendMailPort"), m_MovDetSendMailConfiguration.m_nPort, sTempFileName);
			::WriteProfileIniString(sSection, _T("SendMailFrom"), m_MovDetSendMailConfiguration.m_sFrom, sTempFileName);
			::WriteProfileIniString(sSection, _T("SendMailHost"), m_MovDetSendMailConfiguration.m_sHost, sTempFileName);
			::WriteProfileIniString(sSection, _T("SendMailFromName"), m_MovDetSendMailConfiguration.m_sFromName, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SendMailAuth"), (int)m_MovDetSendMailConfiguration.m_Auth, sTempFileName);
			::WriteSecureProfileIniString(sSection, _T("SendMailUsername"), m_MovDetSendMailConfiguration.m_sUsername, sTempFileName);
			::WriteSecureProfileIniString(sSection, _T("SendMailPassword"), m_MovDetSendMailConfiguration.m_sPassword, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SendMailHTML"), m_MovDetSendMailConfiguration.m_bHTML, sTempFileName);
#if (_MSC_VER > 1200)
			::WriteProfileIniInt(sSection, _T("SendMailConnectionType"), (int)m_MovDetSendMailConfiguration.m_ConnectionType, sTempFileName);
#endif

			// FTP Settings
			::WriteProfileIniString(sSection, _T("MovDetFTPHost"), m_MovDetFTPUploadConfiguration.m_sHost, sTempFileName);
			::WriteProfileIniString(sSection, _T("MovDetFTPRemoteDir"), m_MovDetFTPUploadConfiguration.m_sRemoteDir, sTempFileName);
			::WriteProfileIniInt(sSection, _T("MovDetFTPPort"), m_MovDetFTPUploadConfiguration.m_nPort, sTempFileName);
			::WriteProfileIniInt(sSection, _T("MovDetFTPPasv"), m_MovDetFTPUploadConfiguration.m_bPasv, sTempFileName);
			::WriteProfileIniInt(sSection, _T("MovDetFTPProxy"), m_MovDetFTPUploadConfiguration.m_bProxy, sTempFileName);
			::WriteProfileIniString(sSection, _T("MovDetFTPProxyHost"), m_MovDetFTPUploadConfiguration.m_sProxy, sTempFileName);
			::WriteSecureProfileIniString(sSection, _T("MovDetFTPUsername"), m_MovDetFTPUploadConfiguration.m_sUsername, sTempFileName);
			::WriteSecureProfileIniString(sSection, _T("MovDetFTPPassword"), m_MovDetFTPUploadConfiguration.m_sPassword, sTempFileName);
			::WriteProfileIniInt(sSection, _T("MovDetFilesToUpload"), (int)m_MovDetFTPUploadConfiguration.m_FilesToUpload, sTempFileName);
			::WriteProfileIniString(sSection, _T("SnapshotFTPHost"), m_SnapshotFTPUploadConfiguration.m_sHost, sTempFileName);
			::WriteProfileIniString(sSection, _T("SnapshotFTPRemoteDir"), m_SnapshotFTPUploadConfiguration.m_sRemoteDir, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotFTPPort"), m_SnapshotFTPUploadConfiguration.m_nPort, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotFTPPasv"), m_SnapshotFTPUploadConfiguration.m_bPasv, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotFTPProxy"), m_SnapshotFTPUploadConfiguration.m_bProxy, sTempFileName);
			::WriteProfileIniString(sSection, _T("SnapshotFTPProxyHost"), m_SnapshotFTPUploadConfiguration.m_sProxy, sTempFileName);
			::WriteSecureProfileIniString(sSection, _T("SnapshotFTPUsername"), m_SnapshotFTPUploadConfiguration.m_sUsername, sTempFileName);
			::WriteSecureProfileIniString(sSection, _T("SnapshotFTPPassword"), m_SnapshotFTPUploadConfiguration.m_sPassword, sTempFileName);

			// Networking
			::WriteProfileIniInt(sSection, _T("SendFrameVideoPort"), m_nSendFrameVideoPort, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SendFrameMaxConnections"), m_nSendFrameMaxConnectionsConfig, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SendFrameMTU"), m_nSendFrameMTU, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SendFrameDataRate"), m_nSendFrameDataRate, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SendFrameSizeDiv"), m_nSendFrameSizeDiv, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SendFrameFreqDiv"), m_nSendFrameFreqDiv, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DoSendVideoFrame"), m_bSendVideoFrame, sTempFileName);
			::WriteProfileIniInt(sSection, _T("GetFrameMaxFrames"), (int)m_dwGetFrameMaxFrames, sTempFileName);
			::WriteProfileIniInt(sSection, _T("GetFrameDisableResend"), (int)m_bGetFrameDisableResend, sTempFileName);
			::WriteProfileIniInt(sSection, _T("HTTPVideoQuality"), m_nHttpVideoQuality, sTempFileName);
			::WriteProfileIniInt(sSection, _T("HTTPVideoSizeX"), m_nHttpVideoSizeX, sTempFileName);
			::WriteProfileIniInt(sSection, _T("HTTPVideoSizeY"), m_nHttpVideoSizeY, sTempFileName);
			::WriteSecureProfileIniString(sSection, _T("SendFrameUsername"), m_sSendFrameUsername, sTempFileName);
			::WriteSecureProfileIniString(sSection, _T("SendFramePassword"), m_sSendFramePassword, sTempFileName);

			// All other
			::WriteProfileIniInt(sSection, _T("VideoView"), m_bVideoView, sTempFileName);
			::WriteProfileIniInt(sSection, _T("Deinterlace"), (int)m_bDeinterlace, sTempFileName);
			::WriteProfileIniInt(sSection, _T("Rotate180"), (int)m_bRotate180, sTempFileName);
			::WriteProfileIniInt(sSection, _T("RecDeinterlace"), m_bRecDeinterlace, sTempFileName);
			::WriteProfileIniInt(sSection, _T("RecAutoOpen"), m_bRecAutoOpen, sTempFileName);
			::WriteProfileIniInt(sSection, _T("RecTimeSegmentation"), m_bRecTimeSegmentation, sTempFileName);
			::WriteProfileIniInt(sSection, _T("TimeSegmentationIndex"), m_nTimeSegmentationIndex, sTempFileName);
			::WriteProfileIniString(sSection, _T("RecordAutoSaveDir"), m_sRecordAutoSaveDir, sTempFileName);
			::WriteProfileIniString(sSection, _T("DetectionAutoSaveDir"), m_sDetectionAutoSaveDir, sTempFileName);
			::WriteProfileIniString(sSection, _T("DetectionTriggerFileName"), m_sDetectionTriggerFileName, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotLiveJpeg"), (int)m_bSnapshotLiveJpeg, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotHistoryJpeg"), (int)m_bSnapshotHistoryJpeg, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotHistorySwf"), (int)m_bSnapshotHistorySwf, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotLiveJpegFtp"), (int)m_bSnapshotLiveJpegFtp, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotHistoryJpegFtp"), (int)m_bSnapshotHistoryJpegFtp, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotHistorySwfFtp"), (int)m_bSnapshotHistorySwfFtp, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotHistoryDeinterlace"), (int)m_bSnapshotHistoryDeinterlace, sTempFileName);
			::WriteProfileIniInt(sSection, _T("ManualSnapshotAutoOpen"), (int)m_bManualSnapshotAutoOpen, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotRate"), m_nSnapshotRate, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotHistoryFrameRate"), m_nSnapshotHistoryFrameRate, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotCompressionQuality"), m_nSnapshotCompressionQuality, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotVideoCompressorQuality"), (int)m_fSnapshotVideoCompressorQuality, sTempFileName);
			::WriteProfileIniString(sSection, _T("SnapshotAutoSaveDir"), m_sSnapshotAutoSaveDir, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotThumb"), (int)m_bSnapshotThumb, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotThumbWidth"), m_nSnapshotThumbWidth, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotThumbHeight"), m_nSnapshotThumbHeight, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotStartStop"), (int)m_bSnapshotStartStop, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotStartHour"), m_SnapshotStartTime.GetHour(), sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotStartMin"), m_SnapshotStartTime.GetMinute(), sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotStartSec"), m_SnapshotStartTime.GetSecond(), sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotStopHour"), m_SnapshotStopTime.GetHour(), sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotStopMin"), m_SnapshotStopTime.GetMinute(), sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotStopSec"), m_SnapshotStopTime.GetSecond(), sTempFileName);
			::WriteProfileIniInt(sSection, _T("CaptureAudio"), m_bCaptureAudio, sTempFileName);
			::WriteProfileIniInt(sSection, _T("AudioCaptureDeviceID"), m_dwCaptureAudioDeviceID, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoCaptureDeviceInputID"), m_nDeviceInputId, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoCaptureDeviceFormatID"), m_nDeviceFormatId, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoCaptureDeviceFormatWidth"), m_nDeviceFormatWidth, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoCaptureDeviceFormatHeight"), m_nDeviceFormatHeight, sTempFileName);
			::WriteProfileIniInt(sSection, _T("MilliSecondsRecBeforeMovementBegin"), m_nMilliSecondsRecBeforeMovementBegin, sTempFileName);
			::WriteProfileIniInt(sSection, _T("MilliSecondsRecAfterMovementEnd"), m_nMilliSecondsRecAfterMovementEnd, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DetectionLevel"), m_nDetectionLevel, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DetectionZoneSize"), m_nDetectionZoneSize, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DoAdjacentZonesDetection"), m_bDoAdjacentZonesDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DoLumChangeDetection"), m_bDoLumChangeDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SaveSWFMovementDetection"), m_bSaveSWFMovementDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SaveAVIMovementDetection"), m_bSaveAVIMovementDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SaveAnimGIFMovementDetection"), m_bSaveAnimGIFMovementDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SendMailMovementDetection"), m_bSendMailMovementDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("FTPUploadMovementDetection"), m_bFTPUploadMovementDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DoExecCommandMovementDetection"), m_bExecCommandMovementDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("ExecModeMovementDetection"), m_nExecModeMovementDetection, sTempFileName);
			::WriteProfileIniString(sSection, _T("ExecCommandMovementDetection"), m_sExecCommandMovementDetection, sTempFileName);
			::WriteProfileIniString(sSection, _T("ExecParamsMovementDetection"), m_sExecParamsMovementDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("HideExecCommandMovementDetection"), m_bHideExecCommandMovementDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("WaitExecCommandMovementDetection"), m_bWaitExecCommandMovementDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoRecFourCC"), m_dwVideoRecFourCC, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoRecQuality"), (int)m_fVideoRecQuality, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoRecKeyframesRate"), m_nVideoRecKeyframesRate, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoRecDataRate"), m_nVideoRecDataRate, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoRecQualityBitrate"), m_nVideoRecQualityBitrate, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoDetFourCC"), m_dwVideoDetFourCC, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoDetDeinterlace"), m_bVideoDetDeinterlace, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoDetQuality"), (int)m_fVideoDetQuality, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoDetQualityBitrate"), m_nVideoDetQualityBitrate, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoDetKeyframesRate"), m_nVideoDetKeyframesRate, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoDetDataRate"), m_nVideoDetDataRate, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoDetSwfFourCC"), m_dwVideoDetSwfFourCC, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoDetSwfDeinterlace"), m_bVideoDetSwfDeinterlace, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoDetSwfQuality"), (int)m_fVideoDetSwfQuality, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoDetSwfQualityBitrate"), m_nVideoDetSwfQualityBitrate, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoDetSwfKeyframesRate"), m_nVideoDetSwfKeyframesRate, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoDetSwfDataRate"), m_nVideoDetSwfDataRate, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DetectionStartStop"), (int)m_bDetectionStartStop, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DetectionStartHour"), m_DetectionStartTime.GetHour(), sTempFileName);
			::WriteProfileIniInt(sSection, _T("DetectionStartMin"), m_DetectionStartTime.GetMinute(), sTempFileName);
			::WriteProfileIniInt(sSection, _T("DetectionStartSec"), m_DetectionStartTime.GetSecond(), sTempFileName);
			::WriteProfileIniInt(sSection, _T("DetectionStopHour"), m_DetectionStopTime.GetHour(), sTempFileName);
			::WriteProfileIniInt(sSection, _T("DetectionStopMin"), m_DetectionStopTime.GetMinute(), sTempFileName);
			::WriteProfileIniInt(sSection, _T("DetectionStopSec"), m_DetectionStopTime.GetSecond(), sTempFileName);
			::WriteProfileIniInt(sSection, _T("ShowFrameTime"), m_bShowFrameTime, sTempFileName);
			::WriteProfileIniInt(sSection, _T("ShowMovementDetections"), m_bShowMovementDetections, sTempFileName);
			::WriteProfileIniInt(sSection, _T("IntensityLimit"), m_nMovementDetectorIntensityLimit, sTempFileName);
			::WriteProfileIniInt(sSection, _T("AnimatedGifWidth"), m_dwAnimatedGifWidth, sTempFileName);
			::WriteProfileIniInt(sSection, _T("AnimatedGifHeight"), m_dwAnimatedGifHeight, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DeleteDetectionsOlderThanDays"), m_nDeleteDetectionsOlderThanDays, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DeleteRecordingsOlderThanDays"), m_nDeleteRecordingsOlderThanDays, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DeleteSnapshotsOlderThanDays"), m_nDeleteSnapshotsOlderThanDays, sTempFileName);

			::WriteProfileIniInt(sSection, _T("MovDetTotalZones"), m_lMovDetTotalZones, sTempFileName);
			for (int i = 0 ; i < m_lMovDetTotalZones ; i++)
			{
				CString sZone;
				sZone.Format(MOVDET_ZONE_FORMAT, i);
				::WriteProfileIniInt(sSection, sZone, m_DoMovementDetection[i], sTempFileName);
			}

			if (m_CaptureAudioThread.m_pSrcWaveFormat)
				::WriteProfileIniBinary(sSection, _T("SrcWaveFormat"), (LPBYTE)m_CaptureAudioThread.m_pSrcWaveFormat, sizeof(WAVEFORMATEX), sTempFileName);
			if (m_CaptureAudioThread.m_pDstWaveFormat)
				::WriteProfileIniBinary(sSection, _T("DstWaveFormat"), (LPBYTE)m_CaptureAudioThread.m_pDstWaveFormat, sizeof(WAVEFORMATEX), sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoProcessorMode"), m_dwVideoProcessorMode, sTempFileName);
			unsigned int nSize = sizeof(m_dFrameRate);
			::WriteProfileIniBinary(sSection, _T("FrameRate"), (LPBYTE)&m_dFrameRate, nSize, sTempFileName);

			// Move it
			::DeleteFile(pApp->m_pszProfileName);
			::WritePrivateProfileString(NULL, NULL, NULL, sTempFileName); // recache
			::MoveFile(sTempFileName, pApp->m_pszProfileName);

			// Ini file writing is slow, especially on memory sticks
			EndWaitCursor();
		}
	}
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
	// Ini file writing is slow, especially on memory sticks
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
	
	// Ini file writing is slow, especially on memory sticks
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
	int i;

	// Ini file writing is slow, especially on memory sticks
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
	for (i = 0 ; i < lImportMovDetTotalZones ; i++)
	{
		CString sZone;
		sZone.Format(MOVDET_ZONE_FORMAT, i);
		m_DoMovementDetection[i] = ::GetProfileIniInt(_T("MovementDetectionZones"), sZone, TRUE, sFileName);
	}

	// Store settings
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
	{
		CString sSection(GetDevicePathName());
		((CUImagerApp*)::AfxGetApp())->WriteProfileInt(sSection, _T("MovDetTotalZones"), lImportMovDetTotalZones);
		for (i = 0 ; i < lImportMovDetTotalZones ; i++)
		{
			CString sZone;
			sZone.Format(MOVDET_ZONE_FORMAT, i);
			((CUImagerApp*)::AfxGetApp())->WriteProfileInt(sSection, sZone, m_DoMovementDetection[i]);
		}
	}

	// Ini file writing is slow, especially on memory sticks
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
	if (nId >= 0)
	{
		// Already open?
		if (m_pDxCapture)
			return TRUE;

		// Load Settings
		if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		{
			CString sDevicePathName = CDxCapture::GetDevicePath(nId);
			CString sDeviceName = CDxCapture::GetDeviceName(nId);
			sDevicePathName.Replace(_T('\\'), _T('/'));
			LoadSettings(DEFAULT_FRAMERATE, sDevicePathName, sDeviceName);
		}

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
		::AfxMessageBox(ML_STRING(1466, "The capture device is already in use or not compatible"), MB_ICONSTOP);
		return FALSE;
	}
	else
		return FALSE;
}

double CVideoDeviceDoc::GetDefaultNetworkFrameRate(NetworkDeviceTypeMode nNetworkDeviceTypeMode) 
{
	switch(nNetworkDeviceTypeMode)
	{
		case INTERNAL_UDP :	return DEFAULT_FRAMERATE;
		case OTHERONE :		return HTTPCLIENTPOLL_DEFAULT_FRAMERATE;
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
		default :			return HTTPCLIENTPOLL_DEFAULT_FRAMERATE;
	}
}

// sAddress: Must have the IP:Port:FrameLocation:NetworkDeviceTypeMode or
//           HostName:Port:FrameLocation:NetworkDeviceTypeMode Format
// For http FrameLocation is m_HttpGetFrameLocations[0],
// for udp it is not used and set to _T('/') 
BOOL CVideoDeviceDoc::OpenGetVideo(CString sAddress) 
{
	// Init Host, Port, FrameLocation and NetworkDeviceTypeMode
	int i = sAddress.ReverseFind(_T(':'));
	if (i >= 0)
	{
		CString sNetworkDeviceTypeMode = sAddress.Right(sAddress.GetLength() - i - 1);
		NetworkDeviceTypeMode nNetworkDeviceTypeMode = (NetworkDeviceTypeMode)_tcstol(sNetworkDeviceTypeMode.GetBuffer(0), NULL, 10);
		sNetworkDeviceTypeMode.ReleaseBuffer();
		if (nNetworkDeviceTypeMode >= INTERNAL_UDP && nNetworkDeviceTypeMode < LAST_DEVICE)
			m_nNetworkDeviceTypeMode = nNetworkDeviceTypeMode;
		else
			m_nNetworkDeviceTypeMode = INTERNAL_UDP;
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
					m_nGetFrameVideoPort = DEFAULT_UDP_PORT;
				m_sGetFrameVideoHost = sAddress.Left(i);
			}
			else
			{
				m_sGetFrameVideoHost = sAddress;
				if (m_nNetworkDeviceTypeMode == INTERNAL_UDP)
					m_nGetFrameVideoPort = DEFAULT_UDP_PORT;
				else
					m_nGetFrameVideoPort = DEFAULT_TCP_PORT;
			}
		}
		else
		{
			m_sGetFrameVideoHost = sAddress;
			if (m_nNetworkDeviceTypeMode == INTERNAL_UDP)
				m_nGetFrameVideoPort = DEFAULT_UDP_PORT;
			else
				m_nGetFrameVideoPort = DEFAULT_TCP_PORT;
			m_HttpGetFrameLocations[0] = _T("/");
		}
	}
	else
	{
		m_sGetFrameVideoHost = sAddress;
		m_nGetFrameVideoPort = DEFAULT_UDP_PORT;
		m_HttpGetFrameLocations[0] = _T("/");
		m_nNetworkDeviceTypeMode = INTERNAL_UDP;
	}

	// Free if Necessary
	if (m_pGetFrameNetCom)
	{
		delete m_pGetFrameNetCom;
		m_pGetFrameNetCom = NULL;
	}

	// Allocate
	m_pGetFrameNetCom = (CNetCom*)new CNetCom;

	// Load Settings
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
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
		return FALSE;

	// Start Audio Capture
	if (m_bCaptureAudio)
		m_CaptureAudioThread.Start();

	return TRUE;
}

BOOL CVideoDeviceDoc::OpenGetVideo(CHostPortDlg* pDlg) 
{
	// Check
	if (!pDlg)
		return FALSE;

	// Init Vars
	int nPos, nPosEnd;
	BOOL bUrl = FALSE;
	int nUrlPort = 80; // Default url port is always 80
	CString sGetFrameVideoHost(pDlg->m_sHost);
	CString sGetFrameVideoHostLowerCase(sGetFrameVideoHost);
	sGetFrameVideoHostLowerCase.MakeLower();

	// Numeric IP6 with format http://[ip6%interfacenum]:port/framelocation
	if ((nPos = sGetFrameVideoHostLowerCase.Find(_T("http://["))) >= 0)
	{
		// Set flag
		bUrl = TRUE;

		// Remove leading http://[ from url
		sGetFrameVideoHost = sGetFrameVideoHost.Right(sGetFrameVideoHost.GetLength() - 8 - nPos);

		// Has Port?
		if ((nPos = sGetFrameVideoHost.Find(_T("]:"))) >= 0)
		{
			CString sPort;
			if ((nPosEnd = sGetFrameVideoHost.Find(_T('/'), nPos)) >= 0)
			{
				sPort = sGetFrameVideoHost.Mid(nPos + 2, nPosEnd - nPos - 2);
				sGetFrameVideoHost.Delete(nPos, nPosEnd - nPos);
			}
			else
			{
				sPort = sGetFrameVideoHost.Mid(nPos + 2, sGetFrameVideoHost.GetLength() - nPos - 2);
				sGetFrameVideoHost.Delete(nPos, sGetFrameVideoHost.GetLength() - nPos);
			}
			sPort.TrimLeft();
			sPort.TrimRight();
			int nPort = _tcstol(sPort.GetBuffer(0), NULL, 10);
			sPort.ReleaseBuffer();
			if (nPort > 0 && nPort <= 65535) // Port 0 is Reserved
				nUrlPort = nPort;
		}
		else if ((nPos = sGetFrameVideoHost.Find(_T("]"))) >= 0)
			sGetFrameVideoHost.Delete(nPos);
		else
			nPos = sGetFrameVideoHost.GetLength(); // Just in case ] is missing

		// Split
		CString sLocation = sGetFrameVideoHost.Right(sGetFrameVideoHost.GetLength() - nPos);
		sGetFrameVideoHost = sGetFrameVideoHost.Left(nPos);

		// Get Location which is set as first automatic camera type detection query string
		nPos = sLocation.Find(_T('/'));
		if (nPos >= 0)
		{	
			m_HttpGetFrameLocations[0] = sLocation.Right(sLocation.GetLength() - nPos);
			m_HttpGetFrameLocations[0].TrimLeft();
			m_HttpGetFrameLocations[0].TrimRight();
		}
		else
			m_HttpGetFrameLocations[0] = _T("/");
	}
	// Numeric IP4 or hostname with format http://host:port/framelocation
	else if ((nPos = sGetFrameVideoHostLowerCase.Find(_T("http://"))) >= 0)
	{
		// Set flag
		bUrl = TRUE;

		// Remove leading http:// from url
		sGetFrameVideoHost = sGetFrameVideoHost.Right(sGetFrameVideoHost.GetLength() - 7 - nPos);

		// Has Port?
		if ((nPos = sGetFrameVideoHost.Find(_T(":"))) >= 0)
		{
			CString sPort;
			if ((nPosEnd = sGetFrameVideoHost.Find(_T('/'), nPos)) >= 0)
			{
				sPort = sGetFrameVideoHost.Mid(nPos + 1, nPosEnd - nPos - 1);
				sGetFrameVideoHost.Delete(nPos, nPosEnd - nPos);
			}
			else
			{
				sPort = sGetFrameVideoHost.Mid(nPos + 1, sGetFrameVideoHost.GetLength() - nPos - 1);
				sGetFrameVideoHost.Delete(nPos, sGetFrameVideoHost.GetLength() - nPos);
			}
			sPort.TrimLeft();
			sPort.TrimRight();
			int nPort = _tcstol(sPort.GetBuffer(0), NULL, 10);
			sPort.ReleaseBuffer();
			if (nPort > 0 && nPort <= 65535) // Port 0 is Reserved
				nUrlPort = nPort;
		}

		// Get Location which is set as first automatic camera type detection query string
		nPos = sGetFrameVideoHost.Find(_T('/'));
		if (nPos >= 0)
		{	
			m_HttpGetFrameLocations[0] = sGetFrameVideoHost.Right(sGetFrameVideoHost.GetLength() - nPos);
			sGetFrameVideoHost = sGetFrameVideoHost.Left(nPos);
			m_HttpGetFrameLocations[0].TrimLeft();
			m_HttpGetFrameLocations[0].TrimRight();
		}
		else
			m_HttpGetFrameLocations[0] = _T("/");
	}
	else
		m_HttpGetFrameLocations[0] = _T("/");

	// Set vars
	sGetFrameVideoHost.TrimLeft();
	sGetFrameVideoHost.TrimRight();
	m_sGetFrameVideoHost = sGetFrameVideoHost;
	m_nGetFrameVideoPort = bUrl ? nUrlPort : pDlg->m_nPort;
	m_nNetworkDeviceTypeMode = bUrl ? OTHERONE : (NetworkDeviceTypeMode)pDlg->m_nDeviceTypeMode;

	// Free if Necessary
	if (m_pGetFrameNetCom)
	{
		delete m_pGetFrameNetCom;
		m_pGetFrameNetCom = NULL;
	}

	// Allocate
	m_pGetFrameNetCom = (CNetCom*)new CNetCom;

	// Load Settings
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		LoadSettings(GetDefaultNetworkFrameRate(m_nNetworkDeviceTypeMode), GetDevicePathName(), GetDeviceName());

	// Start Delete Detections Thread
	if (!m_DeleteThread.IsAlive())
		m_DeleteThread.Start(THREAD_PRIORITY_LOWEST);

	// Overwrite the loaded values
	m_dwGetFrameMaxFrames = pDlg->m_dwMaxFrames;			
	m_bGetFrameDisableResend = pDlg->m_bDisableResend;

	// Reset vars
	m_dwFrameCountUp = 0U;
	m_dwNextSnapshotUpTime = ::timeGetTime();
	::InterlockedExchange(&m_lCurrentInitUpTime, (LONG)m_dwNextSnapshotUpTime);

	// Connect
	if (!ConnectGetFrame())
	{
		::AfxMessageBox(ML_STRING(1465, "Cannot connect to the specified network device or server"), MB_ICONSTOP);
		return FALSE;
	}

	// Start Audio Capture
	if (m_bCaptureAudio)
		m_CaptureAudioThread.Start();

	return TRUE;
}

BOOL CVideoDeviceDoc::OpenVideoAvi(CVideoAviDoc* pDoc, CDib* pDib) 
{
	// Check
	if (!((CUImagerApp*)::AfxGetApp())->IsDoc((CUImagerDoc*)pDoc)	||
		!pDib														||
		!pDib->IsValid())
		return FALSE;

	// Set pointer
	m_pVideoAviDoc = pDoc;
	
	// Load Settings
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		LoadSettings(DEFAULT_FRAMERATE, GetDevicePathName(), GetDeviceName());

	// Start Delete Detections Thread
	if (!m_DeleteThread.IsAlive())
		m_DeleteThread.Start(THREAD_PRIORITY_LOWEST);

	// Reset vars
	m_dwFrameCountUp = 0U;
	m_dwNextSnapshotUpTime = ::timeGetTime();
	::InterlockedExchange(&m_lCurrentInitUpTime, (LONG)m_dwNextSnapshotUpTime);
	
	// Frame Rate
	m_dFrameRate = m_pVideoAviDoc->m_PlayVideoFileThread.GetFrameRate();
	m_dEffectiveFrameRate = m_dFrameRate;

	// Format and Size
	memcpy(&m_CaptureBMI, pDib->GetBMI(), MIN(sizeof(BITMAPINFOFULL), pDib->GetBMISize()));
	memcpy(&m_ProcessFrameBMI, pDib->GetBMI(), MIN(sizeof(BITMAPINFOFULL), pDib->GetBMISize()));
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

	// Free Movement Detector because we changed size and/or format!
	FreeMovementDetector();
	ResetMovementDetector();

	// Start Audio Capture
	if (m_bCaptureAudio)
		m_CaptureAudioThread.Start();

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

	return TRUE;
}

CString CVideoDeviceDoc::MakeJpegManualSnapshotFileName(const CTime& Time)
{
	CString sYearMonthDayDir(_T(""));

	// Snapshot time
	CString sTime = Time.Format(_T("%Y_%m_%d_%H_%M_%S"));

	// Adjust Directory Name
	CString sSnapshotAutoSaveDir = m_sSnapshotAutoSaveDir;
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

CString CVideoDeviceDoc::MakeRecFileName()
{
	CString sYearMonthDayDir(_T(""));

	// Recording time
	CTime Time = CTime::GetCurrentTime();
	CString sTime = Time.Format(_T("%Y_%m_%d_%H_%M_%S"));

	// Adjust Directory Name
	CString sRecordAutoSaveDir = m_sRecordAutoSaveDir;
	sRecordAutoSaveDir.TrimRight(_T('\\'));

	// Create directory if necessary
	if (sRecordAutoSaveDir != _T(""))
	{
		DWORD dwAttrib = ::GetFileAttributes(sRecordAutoSaveDir);
		if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
			::CreateDir(sRecordAutoSaveDir);
		if (!CVideoDeviceDoc::CreateCheckYearMonthDayDir(Time, sRecordAutoSaveDir, sYearMonthDayDir))
			return _T("");
	}
	
	// Return file name
	if (sYearMonthDayDir == _T(""))
		return _T("rec_") + sTime + _T(".avi");
	else
		return sYearMonthDayDir + _T("\\") + _T("rec_") + sTime + _T(".avi");
}

BOOL CVideoDeviceDoc::MakeAVRec(const CString& sFileName, CAVRec** ppAVRec)
{
	// Check
	if (!ppAVRec)
		return FALSE;

	// Allocate
	*ppAVRec = new CAVRec;
	if (!(*ppAVRec))
		return FALSE;

	// Set File Name
	if (!(*ppAVRec)->Init(sFileName, 0, _T(""), true)) // fast encoding!
		return FALSE;

	// Set File Info
	if (!(*ppAVRec)->SetInfo(	_T("Title"),
								_T("Author"),
								_T("Copyright"),
								_T("Comment"),
								_T("Album"),
								_T("Genre")))
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

BOOL CVideoDeviceDoc::RecError(BOOL bShowMessageBoxOnError, CAVRec* pAVRec)
{
	// Free
	if (pAVRec)
	{
		delete pAVRec;
		pAVRec = NULL;
	}

	// Leave CS
	::LeaveCriticalSection(&m_csAVRec);

	if (m_bCaptureAudio)
	{
		m_bAboutToStartRec = FALSE;
		m_CaptureAudioThread.SetSmallBuffers(TRUE);
	}
	
	CString sMsg = ML_STRING(1493, "Cannot Create the AVI File!\n");
	TRACE(sMsg);
	if (bShowMessageBoxOnError)
		::AfxMessageBox(sMsg, MB_ICONSTOP);
	
	return FALSE;
}

void CVideoDeviceDoc::OnCaptureRecord() 
{
	if (!m_bAboutToStopRec && !m_bAboutToStartRec)
		CaptureRecord();
}

void CVideoDeviceDoc::OnUpdateCaptureRecord(CCmdUI* pCmdUI) 
{	
	pCmdUI->SetCheck(m_pAVRec != NULL ? 1 : 0);
	pCmdUI->Enable(!m_bAboutToStopRec && !m_bAboutToStartRec);
}

BOOL CVideoDeviceDoc::CaptureRecord(BOOL bShowMessageBoxOnError/*=TRUE*/) 
{
	// Enter CS
	::EnterCriticalSection(&m_csAVRec);

	// Stop Recording
	if (m_pAVRec)
	{
		// If paused, just close avi
		if (m_bCaptureRecordPause)
		{
			CloseAndShowAviRec();
		
			// Recording Stopped, no pausing available
			m_bCaptureRecordPause = FALSE;
		}
		else
		{
			// Samples/Frames Synchronized Stopping of Video And Audio Threads
			if (m_bCaptureAudio)
			{ 
				m_bAboutToStopRec = TRUE;
				m_bStopRec = TRUE;
			}
			else
				CloseAndShowAviRec();
		}

		// Leave CS
		::LeaveCriticalSection(&m_csAVRec);

		return TRUE;
	}
	// Start Recording
	else
	{
		// Capture Audio?
		if (m_bCaptureAudio)
		{
			// Set Big Buffers for a stable recording
			m_CaptureAudioThread.SetSmallBuffers(FALSE);
			m_bAboutToStartRec = TRUE;
		}
		else
			m_bAboutToStartRec = FALSE;

		// Set next rec time for time segmentation
		if (m_bRecTimeSegmentation)
		{
			CTime t = CTime::GetCurrentTime();
			NextRecTime(t);
		}

		// Free
		FreeAVIFile();
		
		// Allocate & Init pAVRec
		CAVRec* pAVRec = NULL;
		if (!MakeAVRec(MakeRecFileName(), &pAVRec))
			return RecError(bShowMessageBoxOnError, pAVRec);

		// Set AV Rec Pointer
		m_pAVRec = pAVRec;

		// Start Recording
		m_bRecFirstFrame = TRUE;

		// Do Not Wait Audio, if No Audio
		if (!m_bCaptureAudio)
			m_bVideoRecWait = FALSE;

		// Leave CS
		::LeaveCriticalSection(&m_csAVRec);

		return TRUE;
	}
}

void CVideoDeviceDoc::OnCaptureRecordPause() 
{
	if (m_pAVRec					&&
		!m_bAboutToStopRec			&&
		!m_bAboutToStartRec)
		CaptureRecordPause();
}

void CVideoDeviceDoc::OnUpdateCaptureRecordPause(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bCaptureRecordPause ? 1 : 0);
	pCmdUI->Enable(	m_pAVRec					&&
					!m_bAboutToStopRec			&&
					!m_bAboutToStartRec);
}

void CVideoDeviceDoc::CaptureRecordPause()
{
	// Enter Pause
	if (!m_bCaptureRecordPause)
	{
		// Stop Recording
		// (Avi file is left open)
		if (m_bCaptureAudio)
		{ 
			m_bAboutToStopRec = TRUE;
			m_bStopRec = TRUE;
		}
		else
		{
			m_bVideoRecWait = TRUE;
			m_bAudioRecWait = TRUE;
			m_bStopRec = FALSE;
		}
	}
	// Leave Pause
	else
	{
		if (m_bCaptureAudio)
			m_bAboutToStartRec = TRUE;

		// Resume Recording
		m_bRecResume = TRUE;

		// Do Not Wait Audio, if No Audio
		if (!m_bCaptureAudio)
			m_bVideoRecWait = FALSE;
	}

	m_bCaptureRecordPause = !m_bCaptureRecordPause;
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
				sMsg.Format(_T("%s, error getting current video format!\n"), GetDeviceName());
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
				sMsg.Format(_T("%s, unsupported video format!\n"), GetDeviceName());
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
	ResetMovementDetector();

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
			ResetMovementDetector();
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
			ResetMovementDetector();
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

void CVideoDeviceDoc::ResetMovementDetector()
{
	m_bFirstMovementDetection = TRUE;			
	m_bDetectingMovement = FALSE;
	for (int i = 0 ; i < m_lMovDetTotalZones ; i++)
		m_MovementDetections[i] = FALSE;
}

void CVideoDeviceDoc::AudioFormatDialog() 
{
	CAudioFormatDlg dlg(GetView());
	if (m_CaptureAudioThread.m_pDstWaveFormat)
		memcpy(&dlg.m_WaveFormat, m_CaptureAudioThread.m_pDstWaveFormat, sizeof(WAVEFORMATEX));
	if (dlg.DoModal() == IDOK)
	{
		// Stop Audio Thread
		if (m_bCaptureAudio)
		{
			m_CaptureAudioThread.Kill();
			m_CaptureAudioThread.m_Mixer.Close();
		}

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
	}
}

void CVideoDeviceDoc::OnCaptureReset() 
{
	if (::AfxMessageBox(ML_STRING(1740, "Settings are reset to the default and\nthe capture window will be closed.\nDo you want to proceed?"),
						MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		m_bResetSettings = TRUE;
		CloseDocRemoveAutorunDev();
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
	CAssistantDlg dlg(this, GetView());
	dlg.DoModal();
}

void CVideoDeviceDoc::OnCaptureSettings() 
{
	CaptureSettings();
}

void CVideoDeviceDoc::OnUpdateCaptureSettings(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	m_pDxCapture														||
					((CUImagerApp*)::AfxGetApp())->IsDoc((CUImagerDoc*)m_pVideoAviDoc)	||
					m_pGetFrameNetCom);
	if (m_pVideoDevicePropertySheet)
		pCmdUI->SetCheck(m_pVideoDevicePropertySheet->IsVisible() ? 1 : 0);
	else
		pCmdUI->SetCheck(0);
}

void CVideoDeviceDoc::CaptureSettings() 
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
		m_pVideoDevicePropertySheet->Toggle();

	SetDocumentTitle();
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
		if (m_nNetworkDeviceTypeMode == OTHERONE	||
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

void CVideoDeviceDoc::ViewVideo() 
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

void CVideoDeviceDoc::OnViewVideo() 
{
	ViewVideo();
}

void CVideoDeviceDoc::OnUpdateViewVideo(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bVideoView ? 1 : 0);
}

BOOL CVideoDeviceDoc::MicroApacheCheckWebFiles(CString sAutoSaveDir, BOOL bOverwrite/*=FALSE*/)
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
	FileFind.InitRecursive(sMicroapacheHtDocs + _T("*"), FALSE);
	FileFind.WaitRecursiveDone();
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
				::CopyFile(sName, sAutoSaveDir + sRelName, !bOverwrite);
		}
	}
	return TRUE;
}

void CVideoDeviceDoc::MicroApacheViewOnWeb(CString sAutoSaveDir, const CString& sWebPageFileName)
{
	// Trim ending backslash
	sAutoSaveDir.TrimRight(_T('\\'));
	CString sMicroApacheDocRoot = ((CUImagerApp*)::AfxGetApp())->m_sMicroApacheDocRoot;
	sMicroApacheDocRoot.TrimRight(_T('\\'));

	// Check whether sAutoSaveDir is inside the document root directory
	if (sAutoSaveDir.Find(sMicroApacheDocRoot) < 0)
	{
		::AfxMessageBox(ML_STRING(1473, "Movement detection, recording and snapshot directories\nmust all reside inside the document root directory!"), MB_OK | MB_ICONSTOP);
		return;
	}

	// Check whether the web files exist in the given auto-save directory
	MicroApacheCheckWebFiles(sAutoSaveDir);
	
	// Execute Browser
	CString sRelPath(sAutoSaveDir.Right(sAutoSaveDir.GetLength() - sMicroApacheDocRoot.GetLength()));
	sRelPath.TrimLeft(_T('\\'));
	sRelPath.Replace(_T('\\'), _T('/'));// Change path from \ to /
	CString sUrl, sPort;
	sPort.Format(_T("%d"), ((CUImagerApp*)::AfxGetApp())->m_nMicroApachePort);
	if (sPort != _T("80"))
	{
		if (sRelPath != _T(""))
			sUrl = _T("http://localhost:") + sPort + _T("/") + sRelPath + _T("/") + sWebPageFileName;
		else
			sUrl = _T("http://localhost:") + sPort + _T("/") + sWebPageFileName;
	}
	else
	{
		if (sRelPath != _T(""))
			sUrl = _T("http://localhost/") + sRelPath + _T("/") + sWebPageFileName;
		else
			sUrl = _T("http://localhost/") + sWebPageFileName;
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

CString CVideoDeviceDoc::GetAutoSaveDir()
{
	if (m_sRecordAutoSaveDir != _T(""))
		return m_sRecordAutoSaveDir;
	else if (m_sDetectionAutoSaveDir != _T(""))
		return m_sDetectionAutoSaveDir;
	else if (m_sSnapshotAutoSaveDir != _T(""))
		return m_sSnapshotAutoSaveDir;
	else
		return _T("");
}

void CVideoDeviceDoc::OnViewWeb() 
{
	CString sAutoSaveDir = GetAutoSaveDir();
	if (sAutoSaveDir != _T(""))
		MicroApacheViewOnWeb(sAutoSaveDir, _T("index.php"));
	else
		::AfxMessageBox(ML_STRING(1476, "Please configure a directory in the Device Settings dialog"));
}

CString CVideoDeviceDoc::MicroApacheGetConfigFileName()
{
	// Get App Data Folder
	CString sMicroapacheConfigFile = ::GetSpecialFolderPath(CSIDL_APPDATA);

	// It's important to have a place to write the config file,
	// under win95 and NT4 CSIDL_APPDATA is not available
	// return the program's directory
	if (sMicroapacheConfigFile == _T(""))
	{
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szProgramName[MAX_PATH];
		if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
			return _T("");
		_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
		sMicroapacheConfigFile = CString(szDrive) + CString(szDir);
		sMicroapacheConfigFile += MICROAPACHE_CONFIGNAME_EXT;
	}
	else
		sMicroapacheConfigFile += _T("\\") + MICROAPACHE_CONFIG_FILE;

	return sMicroapacheConfigFile;
}

CString CVideoDeviceDoc::MicroApacheGetLogFileName()
{
	// Get App Data Folder
	CString sMicroapacheLogFile = ::GetSpecialFolderPath(CSIDL_APPDATA);

	// It's important to have a place to write the log file,
	// under win95 and NT4 CSIDL_APPDATA is not available
	// return the program's directory
	if (sMicroapacheLogFile == _T(""))
	{
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szProgramName[MAX_PATH];
		if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
			return _T("");
		_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
		sMicroapacheLogFile = CString(szDrive) + CString(szDir);
		sMicroapacheLogFile += MICROAPACHE_LOGNAME_EXT;
	}
	else
		sMicroapacheLogFile += _T("\\") + MICROAPACHE_LOG_FILE;

	return sMicroapacheLogFile;
}

CString CVideoDeviceDoc::MicroApacheGetPidFileName()
{
	// Get App Data Folder
	CString sMicroapachePidFile = ::GetSpecialFolderPath(CSIDL_APPDATA);

	// It's important to have a place to write the pid file,
	// under win95 and NT4 CSIDL_APPDATA is not available
	// return the program's directory
	if (sMicroapachePidFile == _T(""))
	{
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szProgramName[MAX_PATH];
		if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
			return _T("");
		_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
		sMicroapachePidFile = CString(szDrive) + CString(szDir);
		sMicroapachePidFile += MICROAPACHE_PIDNAME_EXT;
	}
	else
		sMicroapachePidFile += _T("\\") + MICROAPACHE_PID_FILE;

	return sMicroapachePidFile;
}

CString CVideoDeviceDoc::MicroApacheGetPwFileName()
{
	// Get App Data Folder
	CString sMicroapachePwFile = ::GetSpecialFolderPath(CSIDL_APPDATA);

	// It's important to have a place to write the pid file,
	// under win95 and NT4 CSIDL_APPDATA is not available
	// return the program's directory
	if (sMicroapachePwFile == _T(""))
	{
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szProgramName[MAX_PATH];
		if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
			return _T("");
		_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
		sMicroapachePwFile = CString(szDrive) + CString(szDir);
		sMicroapachePwFile += MICROAPACHE_PWNAME_EXT;
	}
	else
		sMicroapachePwFile += _T("\\") + MICROAPACHE_PW_FILE;

	return sMicroapachePwFile;
}

BOOL CVideoDeviceDoc::MicroApacheCheckConfigFile()
{
	CString sMicroapacheConfigFile = MicroApacheGetConfigFileName();
	if (!::IsExistingFile(sMicroapacheConfigFile))
	{
		CString sConfig;
		sConfig = _T("# Change the Listen directive if you want to use a different port\r\n\
# Decrease ThreadsPerChild to a lower value if you want a faster start-up and less memory usage\r\n\
# Increase ThreadsPerChild for a high load server (250 is a good value)\r\n\r\n\
Listen 8800\r\n\
ServerName localhost\r\n\
ServerRoot .\r\n\
ServerAdmin webmaster@nowhere.com\r\n\
DocumentRoot \"./htdocs/\"\r\n\
ThreadsPerChild 128\r\n\
Win32DisableAcceptEx On\r\n\
LoadModule access_module modules/mod_access.dll\r\n\
LoadModule dir_module modules/mod_dir.dll\r\n\
LoadModule mime_module modules/mod_mime.dll\r\n\
LoadModule rewrite_module modules/mod_rewrite.dll\r\n\
LoadModule auth_module modules/mod_auth.dll\r\n\
LoadModule auth_digest_module modules/mod_auth_digest.dll\r\n\
LoadModule php5_module \"php5apache2.dll\"\r\n\
AddType application/x-httpd-php .php .php3\r\n\
AcceptPathInfo off\r\n\
KeepAlive on\r\n\
KeepAliveTimeout 15\r\n\
TimeOut 30\r\n\
DirectoryIndex index.html index.htm index.php\r\n\
LogLevel crit\r\n");
		
		CString sDir = ::GetASCIICompatiblePath(::GetDriveAndDirName(sMicroapacheConfigFile)); // directory must exist!
		sDir.Replace(_T('\\'), _T('/')); // Change path from \ to / (otherwise apache is not happy)
		sConfig += _T("ErrorLog \"") + sDir + MICROAPACHE_LOGNAME_EXT + _T("\"\r\n");
		sConfig += _T("PidFile \"") + sDir + MICROAPACHE_PIDNAME_EXT + _T("\"\r\n");
		
		sConfig += _T("<Directory />\r\n");
		sConfig += _T("RewriteEngine on\r\n");
		sConfig += _T("RewriteBase /\r\n");
		sConfig += _T("RewriteCond %{REQUEST_FILENAME} -d\r\n");
		sConfig += _T("RewriteRule [^/]$ http://%{HTTP_HOST}%{REQUEST_URI}/ [L,R=301]\r\n");
		sConfig += _T("</Directory>\r\n");

		sConfig += _T("<Location ") + CString(MICROAPACHE_FAKE_LOCATION) + _T("\r\n");
		sConfig += _T("AuthDigestFile \"") + sDir + MICROAPACHE_PWNAME_EXT + _T("\"\r\n");
		sConfig += _T("AuthUserFile \"") + sDir + MICROAPACHE_PWNAME_EXT + _T("\"\r\n");
		sConfig += _T("AuthName \"") + CString(MICROAPACHE_REALM) + _T("\"\r\n");
		sConfig += _T("AuthType Digest\r\n");
		sConfig += _T("AuthDigestDomain /\r\n");
		sConfig += _T("Require valid-user\r\n");
		sConfig += _T("</Location>");

		return SaveMicroApacheConfigFile(sConfig);
	}
	else
		return TRUE;
}

BOOL CVideoDeviceDoc::MicroApacheMakePasswordFile(BOOL bDigest, const CString& sUsername, const CString& sPassword)
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
		CString sToHash = sUsername + _T(":")					+
						CString(MICROAPACHE_REALM) + _T(":")	+
						sPassword;
		char* pszA1 = T2A(const_cast<LPTSTR>(sToHash.operator LPCTSTR()));
		if (hmac.Hash((const BYTE*)pszA1, (DWORD)strlen(pszA1), hash))
		{
			CString sHA1 = hash.Format(FALSE);
			CString sPasswordFileData = sUsername + _T(":") + CString(MICROAPACHE_REALM) + _T(":") + sHA1 + _T("\n");
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
		// Update / create config file and doc root index.php for microapache
		pApp->MicroApacheUpdateFiles(); // do not overwrite doc root index.php

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

BOOL CVideoDeviceDoc::MicroApacheConfigFileSetParam(const CString& sParam, const CString& sValue)
{
	// Load Config File
	CString sConfig = LoadMicroApacheConfigFile();
	if (sConfig == _T(""))
		return FALSE;

	// Find Param
	CString s;
	int nIndexEnd;
	int nIndexStart = sConfig.Find(sParam);
	int i = nIndexStart;
	while (--i >= 0)
	{
		// Start of line, ok
		if (sConfig[i] == _T('\r') || sConfig[i] == _T('\n'))
			break;
		// Ops it was a comment, find next
		else if (sConfig[i] == _T('#'))
		{
			nIndexStart = sConfig.Find(sParam, nIndexStart + 1);
			i = nIndexStart;
		}
	}

	// Append new Param
	if (nIndexStart < 0)
		sConfig += _T("\r\n") + sParam + _T(" ") + sValue;
	// Update existing Param
	else
	{
		nIndexStart += sParam.GetLength(); // Size of Param
		s = sConfig.Right(sConfig.GetLength() - nIndexStart);
		nIndexEnd = s.FindOneOf(_T("\r\n"));
		if (nIndexEnd > 0)
			sConfig.Delete(nIndexStart, nIndexEnd);
		else
			sConfig.Delete(nIndexStart, s.GetLength());
		sConfig.Insert(nIndexStart, _T(" ") + sValue);
	}

	return SaveMicroApacheConfigFile(sConfig);
}

CString CVideoDeviceDoc::MicroApacheConfigFileGetParam(const CString& sParam)
{
	// Load Config File
	CString sConfig = LoadMicroApacheConfigFile();
	if (sConfig == _T(""))
		return _T("");

	// Find Param
	CString s;
	int nIndexEnd;
	int nIndexStart = sConfig.Find(sParam);
	int i = nIndexStart;
	while (--i >= 0)
	{
		// Start of line, ok
		if (sConfig[i] == _T('\r') || sConfig[i] == _T('\n'))
			break;
		// Ops it was a comment, find next
		else if (sConfig[i] == _T('#'))
		{
			nIndexStart = sConfig.Find(sParam, nIndexStart + 1);
			i = nIndexStart;
		}
	}

	// Not found
	if (nIndexStart < 0)
		return _T("");
	else
	{
		CString sValue;
		nIndexStart += sParam.GetLength(); // Size of Param
		s = sConfig.Right(sConfig.GetLength() - nIndexStart);
		nIndexEnd = s.FindOneOf(_T("\r\n"));
		if (nIndexEnd > 0)
			sValue = sConfig.Mid(nIndexStart, nIndexEnd);
		else
			sValue = sConfig.Mid(nIndexStart, s.GetLength());
		sValue.TrimLeft();
		sValue.TrimRight();
		return sValue;
	}
}

CString CVideoDeviceDoc::LoadMicroApacheConfigFile() 
{
	CString sMicroapacheConfigFile = MicroApacheGetConfigFileName();
	LPBYTE pData = NULL;
	try
	{
		// Open Config File
		CFile f(sMicroapacheConfigFile,
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

BOOL CVideoDeviceDoc::SaveMicroApacheConfigFile(const CString& sConfig)
{
	CString sMicroapacheConfigFile = MicroApacheGetConfigFileName();
	CString sPath = ::GetDriveAndDirName(sMicroapacheConfigFile);
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
		CFile f(sMicroapacheConfigFile,
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

CString CVideoDeviceDoc::PhpGetConfigFileName()
{
	CString sAutoSaveDir = GetAutoSaveDir();
	sAutoSaveDir.TrimRight(_T('\\'));
	if (sAutoSaveDir != _T(""))
		return sAutoSaveDir + _T("\\") + PHP_CONFIGNAME_EXT;
	else
		return _T("");
}

CString CVideoDeviceDoc::LoadPhpConfigFile() 
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

BOOL CVideoDeviceDoc::SavePhpConfigFile(const CString& sConfig)
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
	CString sConfig = LoadPhpConfigFile();
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
						return SavePhpConfigFile(sConfig);
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
		return SavePhpConfigFile(sConfig);
	}
	// If also not found -> insert after <?php
	else
	{
		nIndexInsert = sConfigLowerCase.Find(_T("<?php"));
		if (nIndexInsert >= 0)
		{
			nIndexInsert += 5; // Skip <?php
			sConfig.Insert(nIndexInsert, _T("\r\n") + sDefine + _T(" (\"") + sParam + _T("\",\"") + sValue + _T("\");"));
			return SavePhpConfigFile(sConfig);
		}
	}

	return FALSE;
}

CString CVideoDeviceDoc::PhpConfigFileGetParam(const CString& sParam)
{
	// Load Config File
	CString sConfig = LoadPhpConfigFile();
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
		m_lCompressedDataRateSum += dwSize;
		ProcessI420Frame(m_pProcessFrameExtraDib->GetBits(), m_pProcessFrameExtraDib->GetImageSize());
	}
	// Other formats
	else if (m_CaptureBMI.bmiHeader.biCompression != FCC('MJPG'))
	{
		m_pProcessFrameExtraDib->SetBMI((LPBITMAPINFO)&m_CaptureBMI);
		m_pProcessFrameExtraDib->SetBits(pData, dwSize);
		if (m_pProcessFrameExtraDib->IsCompressed())
			m_pProcessFrameExtraDib->Decompress(32);
		if (m_pProcessFrameExtraDib->Compress(FCC('I420')))
			ProcessI420Frame(m_pProcessFrameExtraDib->GetBits(), m_pProcessFrameExtraDib->GetImageSize());
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
		sMsg.Format(_T("%s, error setting I420 format for M420 decoding!\n"), GetDeviceName());
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
			sMsg.Format(_T("%s, error allocating I420 buffer for M420 decoding!\n"), GetDeviceName());
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
#if (_MSC_VER > 1200)
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
#else
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
#endif

	// Call Process Frame
	ProcessI420Frame(m_pProcessFrameExtraDib->GetBits(), m_pProcessFrameExtraDib->GetImageSize());
}

void CVideoDeviceDoc::ProcessI420Frame(LPBYTE pData, DWORD dwSize)
{
	// Timing
	DWORD dwCurrentFrameTime;
	DWORD dwPrevInitUpTime = (DWORD)m_lCurrentInitUpTime;
	CTime CurrentTime = CTime::GetCurrentTime();
	DWORD dwCurrentInitUpTime = ::timeGetTime();
	CTimeSpan TimeDiff1(CurrentTime - m_1SecTime);
	CTimeSpan TimeDiff4(CurrentTime - m_4SecTime);
	BOOL b1SecTick = FALSE;
	BOOL b4SecTick = FALSE;
	if (TimeDiff1.GetTotalSeconds() >= 1 || TimeDiff1.GetTotalSeconds() < 0)
	{
		b1SecTick = TRUE;
		m_1SecTime = CurrentTime;
	}
	if (TimeDiff4.GetTotalSeconds() >= 4 || TimeDiff4.GetTotalSeconds() < 0)
	{
		b4SecTick = TRUE;
		m_4SecTime = CurrentTime;
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
	
	// Detect, Copy, Snapshot, Record, Send over UDP Network and finally Draw
	if (bDoProcessFrame && pData && dwSize > 0)
	{
		// Init Vars
		BOOL bOk;
		BOOL bShowFrameTime = m_bShowFrameTime;

		// Set Dib Pointer
		CDib* pDib = m_pProcessFrameDib;
		if (!pDib)
			goto exit;

		// Copy Bits
		if (!pDib->SetBMI((LPBITMAPINFO)&m_ProcessFrameBMI))
			goto exit;
		if (!pDib->SetBits(pData, dwSize))
			goto exit;
		
		// De-Interlace if divisible by 4
		if (m_bDeinterlace && (pDib->GetWidth() & 3) == 0 && (pDib->GetHeight() & 3) == 0)
			Deinterlace(pDib);

		// Rotate by 180° if divisible by 4
		if (m_bRotate180 && (pDib->GetHeight() & 3) == 0)
			Rotate180(pDib);

		// Set the UpTime Var
		pDib->SetUpTime(dwCurrentInitUpTime);

		// Movement Detection
		DWORD dwVideoProcessorMode = m_dwVideoProcessorMode;
		if (dwVideoProcessorMode && m_bDetectionStartStop) // Detection Scheduler
		{
			CTime timeonly(	2000,
							1,
							1,
							CurrentTime.GetHour(),
							CurrentTime.GetMinute(),
							CurrentTime.GetSecond());
			if (m_DetectionStartTime <= m_DetectionStopTime)
			{
				if (timeonly < m_DetectionStartTime || timeonly > m_DetectionStopTime)
					dwVideoProcessorMode = NO_DETECTOR;
			}
			else
			{
				if (timeonly < m_DetectionStartTime && timeonly > m_DetectionStopTime)
					dwVideoProcessorMode = NO_DETECTOR;
			}
		}
		if (dwVideoProcessorMode && m_bFirstMovementDetection && m_pMovementDetectorBackgndDib)
		{
			delete m_pMovementDetectorBackgndDib;
			m_pMovementDetectorBackgndDib = NULL;
		}
		MovementDetectionProcessing(pDib, dwVideoProcessorMode, b1SecTick);
		if (!dwVideoProcessorMode && !m_bFirstMovementDetection)
			m_bFirstMovementDetection = TRUE;

		// Copy to Clipboard
		if (m_bDoEditCopy)
			EditCopy(pDib, CurrentTime);

		// Manual Snapshot to JPEG File(s) (this copies also to clipboard)
		if (m_bDoEditSnapshot)
			EditSnapshot(pDib, CurrentTime);

		// Timed Snapshot
		Snapshot(pDib, CurrentTime);

		// Record Video
		if (m_bVideoRecWait == FALSE)
		{
			::EnterCriticalSection(&m_csAVRec);
			if (m_pAVRec)
			{
				// Add Frame
				bool bAVRecShowFrameTime = false;
				bool bRecDeinterlace = m_bRecDeinterlace ? true : false;
				if (bShowFrameTime)
				{
					if (!bRecDeinterlace)
					{
						CAVRec::AddFrameTime(pDib, CurrentTime, dwCurrentInitUpTime);
						bShowFrameTime = FALSE;
					}
					else
						bAVRecShowFrameTime = true;
				}
				bOk = m_pAVRec->AddFrame(	m_pAVRec->VideoStreamNumToStreamNum(ACTIVE_VIDEO_STREAM),
											pDib,
											m_bInterleave ? true : false,
											bRecDeinterlace,
											bAVRecShowFrameTime,
											CurrentTime,
											dwCurrentInitUpTime);

				// Recording Up-Time Init
				if (m_bRecFirstFrame)
				{
					m_dwRecLastUpTime  = m_dwRecFirstUpTime = dwCurrentInitUpTime;
					m_nRecordedFrames = 1;
					m_bRecFirstFrame = FALSE;
				}
				// Resume Recording After Pause
				else if (m_bRecResume)
				{
					if (m_nRecordedFrames > 1)
					{
						// Calc. the Frame Time
						double dFrameTime = dFrameTime = (double)(	m_dwRecLastUpTime - m_dwRecFirstUpTime) /
																	(m_nRecordedFrames - 1);

						// Add the Pause Time minus one Frame
						// Time to the First Up-Time,
						// this to get a correct frame rate calculation!
						m_dwRecFirstUpTime += (DWORD)Round(dwCurrentInitUpTime - m_dwRecLastUpTime - dFrameTime);
					}

					m_dwRecLastUpTime = dwCurrentInitUpTime;
					m_nRecordedFrames++;
					m_bRecResume = FALSE;
				}
				// Recording Up-Time Update
				else
				{
					m_dwRecLastUpTime = dwCurrentInitUpTime;
					m_nRecordedFrames++;
				}
					
				// Every second check for segmentation
				//
				// Note: if capturing audio the following is handled
				//       inside the audio thread
				if (bOk					&&
					!m_bCaptureAudio	&&
					b1SecTick)
				{
					if (m_bRecTimeSegmentation)
					{
						CTime t = CurrentTime;
						if (t >= m_NextRecTime)
						{
							NextRecTime(t);
							bOk = NextAviFile();
						}
					}
				}

				// If not OK -> Stop Recording
				if (!bOk)
				{
					if (m_bCaptureAudio)
					{ 
						m_bAboutToStopRec = TRUE;
						m_bStopRec = TRUE;
					}
					else
						CloseAndShowAviRec();
				}
			}
			::LeaveCriticalSection(&m_csAVRec);
		}

		// Add Frame Time if User Wants it
		if (bShowFrameTime)
			CAVRec::AddFrameTime(pDib, CurrentTime, dwCurrentInitUpTime);

		// Send Video Frame
		if (m_bSendVideoFrame)
		{
			// Encode and Send
			if ((m_dwFrameCountUp % m_nSendFrameFreqDiv) == 0)
			{
				int nEncodedSize;
				::EnterCriticalSection(&m_csSendFrameNetCom);
				if (m_nSendFrameConnectionsCount > 0	&&
					(nEncodedSize = m_pSendFrameParseProcess->Encode(	pDib,
																		CurrentTime,
																		dwCurrentInitUpTime)) > 0)
				{
					SendUDPFrame(	m_pSendFrameNetCom,
									NULL, // To All
									m_pSendFrameParseProcess->GetEncodedDataBuf(),
									nEncodedSize,
									dwCurrentInitUpTime,
									++m_wLastSendUDPFrameSeq,
									m_pSendFrameParseProcess->IsKeyFrame(),
									m_nSendFrameMTU,
									FALSE,	// Normal Priority
									FALSE);	// Not Re-Sending
					StoreUDPFrame(	m_pSendFrameParseProcess->GetEncodedDataBuf(),
									nEncodedSize,
									dwCurrentInitUpTime,
									m_wLastSendUDPFrameSeq,
									m_pSendFrameParseProcess->IsKeyFrame());
				}
				::LeaveCriticalSection(&m_csSendFrameNetCom);
			}

			// Every 1 sec Update
			if (b1SecTick)
			{
				// Get Tx Fifo Size
				::EnterCriticalSection(&m_csSendFrameNetCom);
				int nTxFifoSize = 0;
				if (m_pSendFrameNetCom)
					nTxFifoSize = (int)m_pSendFrameNetCom->GetTxFifoSize();
				::LeaveCriticalSection(&m_csSendFrameNetCom);

				// Update Frame Send To Table and Flow Control
				UpdateFrameSendToTableAndFlowControl(b4SecTick);

				// Prepare Statistics
				CString sMsg;
				if (m_nSendFrameConnectionsCount == 0)
				{
					if (m_nSendFrameMaxConnections == 0)
						sMsg = ML_STRING(1820, "Overload: lower maximum number of connections!\r\n\r\n");
					else
						sMsg = ML_STRING(1479, "Listening Ok\r\n\r\n");
				}
				else
					sMsg.Format(ML_STRING(1480, "Listening Ok:\r\n%d connection%s, Tx size / connection: %d fragments, Max fragments / frame: %u\r\n") +
								ML_STRING(1481, "Datarate / connection: %dkbps , Overall Datarate: %ukbps (+%ukbps)\r\n\r\n"),
								m_nSendFrameConnectionsCount,
								m_nSendFrameConnectionsCount > 1 ? _T("s") : _T(""),
								nTxFifoSize / m_nSendFrameConnectionsCount,
								m_dwMaxSendFrameFragmentsPerFrame,
								m_nSendFrameDataRate / 1000,					// bps -> kbps
								m_dwSendFrameOverallDatarate * 8U / 1000U,		// bytes / sec -> kbps
								(DWORD)Round((m_dSendFrameDatarateCorrection - 1.0) * (m_dwSendFrameOverallDatarate * 8U / 1000U)));	// bytes / sec -> kbps

				::EnterCriticalSection(&m_pSendFrameParseProcess->m_csSendToTable);

				for (int i = 0 ; i < m_nSendFrameMaxConnections ; i++)
				{
					CString sTableEntry;
					if (m_pSendFrameParseProcess->m_SendToTable[i].IsAddrSet())
					{
						CString sPeerAddress;
						if (m_pSendFrameParseProcess->m_SendToTable[i].GetAddrPtr()->sa_family == AF_INET6)
						{
							sPeerAddress.Format(_T("%x:%x:%x:%x:%x:%x:%x:%x"),
									ntohs(((sockaddr_in6*)m_pSendFrameParseProcess->m_SendToTable[i].GetAddrPtr())->sin6_addr.u.Word[0]),
									ntohs(((sockaddr_in6*)m_pSendFrameParseProcess->m_SendToTable[i].GetAddrPtr())->sin6_addr.u.Word[1]),
									ntohs(((sockaddr_in6*)m_pSendFrameParseProcess->m_SendToTable[i].GetAddrPtr())->sin6_addr.u.Word[2]),
									ntohs(((sockaddr_in6*)m_pSendFrameParseProcess->m_SendToTable[i].GetAddrPtr())->sin6_addr.u.Word[3]),
									ntohs(((sockaddr_in6*)m_pSendFrameParseProcess->m_SendToTable[i].GetAddrPtr())->sin6_addr.u.Word[4]),
									ntohs(((sockaddr_in6*)m_pSendFrameParseProcess->m_SendToTable[i].GetAddrPtr())->sin6_addr.u.Word[5]),
									ntohs(((sockaddr_in6*)m_pSendFrameParseProcess->m_SendToTable[i].GetAddrPtr())->sin6_addr.u.Word[6]),
									ntohs(((sockaddr_in6*)m_pSendFrameParseProcess->m_SendToTable[i].GetAddrPtr())->sin6_addr.u.Word[7]));
						}
						else
						{
							sPeerAddress.Format(_T("%d.%d.%d.%d"),
									((sockaddr_in*)m_pSendFrameParseProcess->m_SendToTable[i].GetAddrPtr())->sin_addr.S_un.S_un_b.s_b1,
									((sockaddr_in*)m_pSendFrameParseProcess->m_SendToTable[i].GetAddrPtr())->sin_addr.S_un.S_un_b.s_b2,
									((sockaddr_in*)m_pSendFrameParseProcess->m_SendToTable[i].GetAddrPtr())->sin_addr.S_un.S_un_b.s_b3,
									((sockaddr_in*)m_pSendFrameParseProcess->m_SendToTable[i].GetAddrPtr())->sin_addr.S_un.S_un_b.s_b4);
						}
						sTableEntry.Format(	ML_STRING(1482, "%02i: %s, Port: %i, Sent: %u, Re-Sent: %u, Confirmed: %u, Lost: %u\r\n"),
											i,
											sPeerAddress,
											ntohs(SOCKADDRPORT(m_pSendFrameParseProcess->m_SendToTable[i].GetAddrPtr())),
											m_pSendFrameParseProcess->m_SendToTable[i].m_dwSentFrameCount,
											m_pSendFrameParseProcess->m_SendToTable[i].m_dwReSentFrameCount,
											m_pSendFrameParseProcess->m_SendToTable[i].m_dwConfirmedFrameCount,
											m_pSendFrameParseProcess->m_SendToTable[i].m_dwLostFrameCount);
					}
					else
						sTableEntry.Format(_T("%02i: Not Set\r\n"), i);

					sMsg += sTableEntry;
				}

				::LeaveCriticalSection(&m_pSendFrameParseProcess->m_csSendToTable);

				// Show Statistics
				::PostMessage(	GetView()->GetSafeHwnd(),
								WM_THREADSAFE_SENDFRAME_MSG,
								(WPARAM)(new CString(sMsg)),
								0);
			}
		}

		// Swap Dib pointers
		::EnterCriticalSection(&m_csDib);
		m_pProcessFrameDib = m_pDib;
		m_pDib = pDib;
		::LeaveCriticalSection(&m_csDib);

		// Trigger a Draw
		if (m_bVideoView)
			m_WatchdogAndDrawThread.TriggerDraw();

		// Set started flag and open the Assistant dialog
		if (!m_bCaptureStarted)
		{
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

exit:
	DWORD dwCurrentEndUpTime = ::timeGetTime();
	DWORD dwProcessFrameTime = dwCurrentEndUpTime - dwCurrentInitUpTime;
	::InterlockedExchange(&m_lProcessFrameTime, (LONG)dwProcessFrameTime);
}

BOOL CVideoDeviceDoc::Snapshot(CDib* pDib, const CTime& Time)
{
	// Check
	if (m_SaveSnapshotThread.IsAlive())
		return FALSE;

	// Get uptime
	DWORD dwUpTime = pDib->GetUpTime();

	// Init bDoSnapshot flag
	BOOL bDoSnapshot = FALSE;
	int nFrameTime = Round(1000.0 / m_dFrameRate);
	if (m_dEffectiveFrameRate > 0.0)
		nFrameTime = Round(1000.0 / m_dEffectiveFrameRate);
	if (nFrameTime >= m_nSnapshotRate * 1000)
		bDoSnapshot = TRUE;
	else
	{
		DWORD dwMaxUpTimeDiff = (DWORD)m_nSnapshotRate * 2500U;
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
				m_dwNextSnapshotUpTime += (DWORD)m_nSnapshotRate * 1000U;
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
	}

	// If nothing to do, return
	if (!bDoSnapshot)
		return FALSE;

	// Decode if compressed
	m_SaveSnapshotThread.m_Dib.SetShowMessageBoxOnError(FALSE);
	m_SaveSnapshotThread.m_Dib = *pDib;
	if (m_SaveSnapshotThread.m_Dib.IsCompressed())
		m_SaveSnapshotThread.m_Dib.Decompress(32);

	// Start Thread 
	m_SaveSnapshotThread.m_bShowFrameTime = m_bShowFrameTime;
	m_SaveSnapshotThread.m_bSnapshotThumb = m_bSnapshotThumb;
	m_SaveSnapshotThread.m_bSnapshotLiveJpeg = m_bSnapshotLiveJpeg;
	m_SaveSnapshotThread.m_bSnapshotHistoryJpeg = m_bSnapshotHistoryJpeg;
	m_SaveSnapshotThread.m_bSnapshotHistorySwf = m_bSnapshotHistorySwf;
	m_SaveSnapshotThread.m_bSnapshotLiveJpegFtp = m_bSnapshotLiveJpegFtp;
	m_SaveSnapshotThread.m_bSnapshotHistoryJpegFtp = m_bSnapshotHistoryJpegFtp;
	m_SaveSnapshotThread.m_bSnapshotHistorySwfFtp = m_bSnapshotHistorySwfFtp;
	m_SaveSnapshotThread.m_bSnapshotHistoryDeinterlace = m_bSnapshotHistoryDeinterlace;
	m_SaveSnapshotThread.m_nSnapshotThumbWidth = m_nSnapshotThumbWidth;
	m_SaveSnapshotThread.m_nSnapshotThumbHeight = m_nSnapshotThumbHeight;
	m_SaveSnapshotThread.m_nSnapshotCompressionQuality = m_nSnapshotCompressionQuality;
	m_SaveSnapshotThread.m_fSnapshotVideoCompressorQuality = m_fSnapshotVideoCompressorQuality;
	m_SaveSnapshotThread.m_dSnapshotHistoryFrameRate = (double)m_nSnapshotHistoryFrameRate;
	m_SaveSnapshotThread.m_Time = Time;
	::EnterCriticalSection(&m_csSnapshotFTPUploadConfiguration);
	m_SaveSnapshotThread.m_Config = m_SnapshotFTPUploadConfiguration;
	::LeaveCriticalSection(&m_csSnapshotFTPUploadConfiguration);
	m_SaveSnapshotThread.m_pSaveSnapshotFTPThread = &m_SaveSnapshotFTPThread;
	if (m_bSnapshotHistoryCloseSwfFile)
	{
		m_SaveSnapshotThread.m_NextRecTime = Time;
		::InterlockedExchange(&m_bSnapshotHistoryCloseSwfFile, 0);
	}
	m_SaveSnapshotThread.m_sSnapshotAutoSaveDir = m_sSnapshotAutoSaveDir;
	return m_SaveSnapshotThread.Start();
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
		CAVRec::AddFrameTime(&Dib, Time, dwUpTime);
	
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

	// Decode if compressed
	CDib Dib;
	Dib.SetShowMessageBoxOnError(FALSE);
	Dib = *pDib;
	if (Dib.IsCompressed())
		Dib.Decompress(32);

	// Resize Thumb
	CDib DibThumb;
	if (m_bSnapshotThumb)
	{
		// No Message Box on Error
		DibThumb.SetShowMessageBoxOnError(FALSE);

		// Resize
		DibThumb.StretchBits(m_nSnapshotThumbWidth, m_nSnapshotThumbHeight, &Dib);
	}

	// Add frame time
	if (m_bShowFrameTime)
	{
		CAVRec::AddFrameTime(&Dib, Time, dwUpTime);
		if (DibThumb.IsValid())
			CAVRec::AddFrameTime(&DibThumb, Time, dwUpTime);
	}
	
	// Copy to clipboard (not necessary but can be useful)
	Dib.EditCopy();

	// Save to JPEG File(s)
	BOOL res = Dib.SaveJPEG(sFileName, m_nSnapshotCompressionQuality);
	if (DibThumb.IsValid())
	{
		DibThumb.SaveJPEG(	::GetFileNameNoExt(sFileName) + _T("_thumb.jpg"),
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

void CVideoDeviceDoc::ShowSendFrameMsg()
{
	if (m_pNetworkPage)
	{
		CString sCurrentMsg;
		CEdit* pEdit = (CEdit*)m_pNetworkPage->GetDlgItem(IDC_VIDEO_SENDFRAME_MSG);
		pEdit->GetWindowText(sCurrentMsg);
		if (sCurrentMsg != m_sSendFrameMsg)
		{
			int nPos = pEdit->GetScrollPos(SB_VERT);
			pEdit->SetWindowText(m_sSendFrameMsg);
			pEdit->LineScroll(nPos);
		}
	}
}

void CVideoDeviceDoc::ChangeRecFileFrameRate(const CString& sFileName, double dFrameRate/*=0.0*/)
{
	if (dFrameRate == 0.0)
	{
		if (m_nRecordedFrames > 1)
		{
			dFrameRate =	(1000.0 * (m_nRecordedFrames - 1)) /
							(double)(m_dwRecLastUpTime - m_dwRecFirstUpTime);
		}
		else
			dFrameRate =	1.0;
	}
	CAVIPlay::AviChangeVideoFrameRate(	(LPCTSTR)sFileName,
										0,
										dFrameRate,
										false);
}

void CVideoDeviceDoc::OpenAVIFile(const CString& sFileName)
{
	if (m_bRecAutoOpen && m_bRecAutoOpenAllowed)
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

	// Free & Reset Vars
	FreeAVIFile();
	m_bVideoRecWait = TRUE;
	m_bAudioRecWait = TRUE;
	m_bStopRec = FALSE;

	// Change Frame Rate(s)
	if (m_bCaptureAudio)
	{
		// Change Frame Rate
		double dFrameRate = 0.0;
		if (m_nRecordedFrames > 0	&&
			llSamplesCount > 0		&&
			m_CaptureAudioThread.m_pDstWaveFormat->nSamplesPerSec > 0)
		{
			dFrameRate =	(double)m_nRecordedFrames /
							((double)llSamplesCount / (double)m_CaptureAudioThread.m_pDstWaveFormat->nSamplesPerSec);
		}
		ChangeRecFileFrameRate(sOldRecFileName, dFrameRate);
	}
	else
		ChangeRecFileFrameRate(sOldRecFileName);

	// If ending the windows session do not perform the following
	if (::AfxGetApp() && !((CUImagerApp*)::AfxGetApp())->m_bEndSession)
	{
		// Set Small Buffers for a faster Peak Meter Reaction
		if (m_bCaptureAudio)
			m_CaptureAudioThread.SetSmallBuffers(TRUE);

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
	if (!MakeAVRec(MakeRecFileName(), &pNextAVRec))
	{
		if (pNextAVRec)
			delete pNextAVRec;
		return FALSE;
	}

	// Close old file, change frame rate and open it
	if (m_pAVRec)
	{
		// Store old rec file name
		CString sOldRecFileName = m_pAVRec->GetFileName();

		// Free
		delete m_pAVRec;

		// Change Frame Rate
		ChangeRecFileFrameRate(sOldRecFileName);

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

#if (_MSC_VER > 1200)
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
#else
	for (int y = 0 ; y < ry ; y++ , data += width)
	{
		for (int x = 0 ; x < rx ; x++)
		{
			summ += data[x]; 
		}
	}
#endif
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
	
	// First Frame Already Passed?
	if (!m_bFirstMovementDetection)
	{
		// Single Zone Detection and Current Time Set
		BOOL bSingleZoneDetection = FALSE;
		int nIntensityThreshold = Round(dDetectionLevel * nMaxIntensityPerZone);
		for (i = 0 ; i < m_lMovDetTotalZones ; i++)
		{
			if (m_DoMovementDetection[i] &&
				m_MovementDetectorCurrentIntensity[i] > nIntensityThreshold)
			{
				bSingleZoneDetection = TRUE;
				m_MovementDetections[i] = TRUE;
				m_MovementDetectionsUpTime[i] = pDib->GetUpTime();
			}
		}

		// Clear Old Detection Zones
		for (i = 0 ; i < m_lMovDetTotalZones ; i++)
		{
			if (m_MovementDetections[i]	&&
				(pDib->GetUpTime() - m_MovementDetectionsUpTime[i]) >= MOVDET_TIMEOUT)
				m_MovementDetections[i] = FALSE;
		}

		// Adjacent Zones Detection
		BOOL bAdjacentZonesDetection = FALSE;
		if (bSingleZoneDetection && m_bDoAdjacentZonesDetection)
		{
			for (y = 0 ; y < m_lMovDetYZonesCount ; y++)
			{
				for (x = 0 ; x < m_lMovDetXZonesCount ; x++)
				{
					if (m_DoMovementDetection[y*m_lMovDetXZonesCount+x] && m_MovementDetections[y*m_lMovDetXZonesCount+x])
					{
						if (x > 0													&&
							y > 0													&&
							m_DoMovementDetection[(y-1)*m_lMovDetXZonesCount+(x-1)]	&&
							m_MovementDetections[(y-1)*m_lMovDetXZonesCount+(x-1)])
						{
							bAdjacentZonesDetection = TRUE;
							break;
						}
						if (y > 0													&&
							m_DoMovementDetection[(y-1)*m_lMovDetXZonesCount+x]		&&
							m_MovementDetections[(y-1)*m_lMovDetXZonesCount+x])
						{
							bAdjacentZonesDetection = TRUE;
							break;
						}
						if (y > 0													&&
							x < (m_lMovDetXZonesCount - 1)							&&
							m_DoMovementDetection[(y-1)*m_lMovDetXZonesCount+(x+1)]	&&
							m_MovementDetections[(y-1)*m_lMovDetXZonesCount+(x+1)])
						{
							bAdjacentZonesDetection = TRUE;
							break;
						}
						if (x > 0													&&
							m_DoMovementDetection[y*m_lMovDetXZonesCount+(x-1)]		&&
							m_MovementDetections[y*m_lMovDetXZonesCount+(x-1)])
						{
							bAdjacentZonesDetection = TRUE;
							break;
						}
						if (x < (m_lMovDetXZonesCount - 1)							&&
							m_DoMovementDetection[y*m_lMovDetXZonesCount+(x+1)]		&&
							m_MovementDetections[y*m_lMovDetXZonesCount+(x+1)])
						{
							bAdjacentZonesDetection = TRUE;
							break;
						}
						if (x > 0													&&
							y < (m_lMovDetYZonesCount - 1)							&&
							m_DoMovementDetection[(y+1)*m_lMovDetXZonesCount+(x-1)]	&&
							m_MovementDetections[(y+1)*m_lMovDetXZonesCount+(x-1)])
						{
							bAdjacentZonesDetection = TRUE;
							break;
						}
						if (y < (m_lMovDetYZonesCount - 1)							&&
							m_DoMovementDetection[(y+1)*m_lMovDetXZonesCount+x]		&&
							m_MovementDetections[(y+1)*m_lMovDetXZonesCount+x])
						{
							bAdjacentZonesDetection = TRUE;
							break;
						}
						if (x < (m_lMovDetXZonesCount - 1)							&&
							y < (m_lMovDetYZonesCount - 1)							&&
							m_DoMovementDetection[(y+1)*m_lMovDetXZonesCount+(x+1)]	&&
							m_MovementDetections[(y+1)*m_lMovDetXZonesCount+(x+1)])
						{
							bAdjacentZonesDetection = TRUE;
							break;
						}
					}
				}
			}
		}

		return m_bDoAdjacentZonesDetection ? bAdjacentZonesDetection : bSingleZoneDetection;
	}
	else
	{		
		m_bFirstMovementDetection = FALSE;
		return FALSE;
	}
}

BOOL CVideoDeviceDoc::LumChangeDetector(CDib* pDib)
{
	int x, y;
	int nCount = 0;
	LPBYTE pDataBkg = m_pMovementDetectorBackgndDib->GetBits();
	LPBYTE pDataCur = pDib->GetBits();
	int width = pDib->GetWidth();
	int nZoneWidth = pDib->GetWidth() / m_lMovDetXZonesCount;
	int nZoneHeight = pDib->GetHeight() / m_lMovDetYZonesCount;

	// Calc. difference between current Y and background Y
	// at the grid intersection points
	for (y = 1 ; y < m_lMovDetYZonesCount ; y++)
	{
		int posY = y*nZoneHeight;
		for (x = 1 ; x < m_lMovDetXZonesCount ; x++)
		{
			// Offset
			int posX = x*nZoneWidth;
			int nOffset = width*posY + posX;

			// Consider pixels which can shift with a luminosity change
			// and remember that the Y range is: [16,235] (220 steps)
			if (pDataBkg[nOffset] > 71 && pDataBkg[nOffset] < 180)
			{
				// Inc.
				nCount++;

				// Store
				m_LumChangeDetectorBkgY[y*m_lMovDetXZonesCount+x] = pDataBkg[nOffset];

				// Diff
				m_LumChangeDetectorDiffY[y*m_lMovDetXZonesCount+x] = pDataCur[nOffset] - pDataBkg[nOffset];
			}
			else
			{
				m_LumChangeDetectorBkgY[y*m_lMovDetXZonesCount+x] = 0;
				m_LumChangeDetectorDiffY[y*m_lMovDetXZonesCount+x] = 0;
			}
		}
	}

	// Statistics (only if we have "statistically" enough samples)
	int nTotalGridIntersections = (m_lMovDetXZonesCount - 1) * (m_lMovDetYZonesCount - 1);
	int nTotalGridIntersections20 = 2 * nTotalGridIntersections / 10; // 20%
	if (nCount > 0 && nCount >= nTotalGridIntersections20)
	{
		// Avg
		int nAvg = 0;
		int nCountPlus = 0;
		int nCountMinus = 0;
		for (y = 1 ; y < m_lMovDetYZonesCount ; y++)
		{
			for (x = 1 ; x < m_lMovDetXZonesCount ; x++)
			{
				if (m_LumChangeDetectorBkgY[y*m_lMovDetXZonesCount+x] > 0)
				{
					if (m_LumChangeDetectorDiffY[y*m_lMovDetXZonesCount+x] > 0)
						nCountPlus++;
					else if (m_LumChangeDetectorDiffY[y*m_lMovDetXZonesCount+x] < 0)
						nCountMinus++;
					nAvg += m_LumChangeDetectorDiffY[y*m_lMovDetXZonesCount+x];
				}
			}
		}
		nAvg = nAvg / nCount;
		int nAvgAbs;
		if (nAvg >= 0)
			nAvgAbs = nAvg;
		else
			nAvgAbs = -nAvg;

		// Std Dev
		int nStdDev = 0;
		for (y = 1 ; y < m_lMovDetYZonesCount ; y++)
		{
			for (x = 1 ; x < m_lMovDetXZonesCount ; x++)
			{
				if (m_LumChangeDetectorBkgY[y*m_lMovDetXZonesCount+x] > 0)
				{
					int nDistance = m_LumChangeDetectorDiffY[y*m_lMovDetXZonesCount+x] - nAvg;
					if (nDistance < 0)
						nDistance = -nDistance;
					nStdDev += nDistance;
				}
			}
		}
		nStdDev = nStdDev / nCount;

		// Check conditions for luminosity change
		// Note: do not set the compare nAvgAbs > 8 less than 8 because the Mix15To1
		// mixer can settle to a difference of 8 between current and background
		// bitmaps even if both are identical, that's correct and it's a consequence of the rounding!
		// Note for release 3.9.0 and above:
		// Mix15To1 has been removed, Mix7To1 remains -> we could set the compare as low as 4.
		if (nAvgAbs > 8)
		{
			double dThreshold = 256.0; // Just a high value in case nStdDev is 0...
			if (nStdDev > 0)
				dThreshold = (double)nAvgAbs / (double)nStdDev;
			if (dThreshold >= 1.2)
			{
				int nCount80 = 8 * nCount / 10; // 80%
				if (nCountPlus >= nCount80)
				{
					TRACE(_T("+++: nAvgAbs=%d , nStdDev=%d , nAvgAbs / nStdDev=%0.2f , nCountPlus=%d, nCountMinus=%d, nCount=%d\n"), nAvgAbs, nStdDev, dThreshold, nCountPlus, nCountMinus, nCount);
					return TRUE;
				}
				else if (nCountMinus >= nCount80)
				{
					TRACE(_T("---: nAvgAbs=%d , nStdDev=%d , nAvgAbs / nStdDev=%0.2f , nCountPlus=%d, nCountMinus=%d, nCount=%d\n"), nAvgAbs, nStdDev, dThreshold, nCountPlus, nCountMinus, nCount);
					return TRUE;
				}
			}
			//TRACE(_T("FALSE: nAvgAbs=%d , nStdDev=%d , nAvgAbs / nStdDev=%0.2f , nCountPlus=%d, nCountMinus=%d, nCount=%d\n"), nAvgAbs, nStdDev, dThreshold, nCountPlus, nCountMinus, nCount);
		}
		//else
		//	TRACE(_T("FALSE: nAvgAbs <= 8\n"));
	}
	//else
	//	TRACE(_T("FALSE: nCount < %d\n"), nTotalGridIntersections20);

	return FALSE;
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

__forceinline int CVideoDeviceDoc::GetTotalMovementDetectionFrames()
{
	::EnterCriticalSection(&m_csMovementDetectionsList);
	if (!m_MovementDetectionsList.IsEmpty())
	{
		int nCount = 0;
		POSITION pos = m_MovementDetectionsList.GetHeadPosition();
		while (pos)
		{
			CDib::LIST* pList = m_MovementDetectionsList.GetNext(pos);
			if (pList)
				nCount += pList->GetCount();
		}
		::LeaveCriticalSection(&m_csMovementDetectionsList);
		return nCount;
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

__forceinline void CVideoDeviceDoc::AddNewFrameToNewestList(CDib* pDib)
{
	if (pDib)
	{
		::EnterCriticalSection(&m_csMovementDetectionsList);
		if (!m_MovementDetectionsList.IsEmpty())
		{
			CDib::LIST* pTail = m_MovementDetectionsList.GetTail();
			if (pTail)
			{
				// Check whether format or size changed
				if (!pTail->IsEmpty())
				{
					CDib* pHeadDib = pTail->GetHead();
					if (pHeadDib && !pDib->IsSameBMI(pHeadDib->GetBMI()))
						CDib::FreeList(*pTail);
				}

				// Add
				CDib* pNewDib = new CDib(*pDib);
				if (pNewDib)
					pTail->AddTail(pNewDib);
			}
		}
		::LeaveCriticalSection(&m_csMovementDetectionsList);
	}
}

__forceinline void CVideoDeviceDoc::AddNewFrameToNewestListAndShrink(CDib* pDib)
{
	if (pDib)
	{
		::EnterCriticalSection(&m_csMovementDetectionsList);
		if (!m_MovementDetectionsList.IsEmpty())
		{
			CDib::LIST* pTail = m_MovementDetectionsList.GetTail();
			if (pTail)
			{
				// Check whether format or size changed
				CDib* pHeadDib;
				if (!pTail->IsEmpty())
				{
					pHeadDib = pTail->GetHead();
					if (pHeadDib && !pDib->IsSameBMI(pHeadDib->GetBMI()))
						CDib::FreeList(*pTail);
				}

				// New dib
				DWORD dwNewUpTime = pDib->GetUpTime();
				CDib* pNewDib;
				
				// Get current queue length in milliseconds
				DWORD dwQueueLengthMs = 0U;
				if (!pTail->IsEmpty() && pTail->GetTail() && pTail->GetHead())
					dwQueueLengthMs = pTail->GetTail()->GetUpTime() - pTail->GetHead()->GetUpTime();

				// Only add the new frame
				if (dwQueueLengthMs < (DWORD)m_nMilliSecondsRecBeforeMovementBegin)
				{
					pNewDib = new CDib(*pDib);
					if (pNewDib)
						pTail->AddTail(pNewDib);
				}
				// Remove old frame(s) and add the new one recycling the oldest one
				else
				{
					// Get head dib and remove from head position
					pHeadDib = pTail->GetHead();
					pTail->RemoveHead();

					// Add new frame recycling the oldest one
					if (pHeadDib)
					{
						pHeadDib->SetBits(pDib->GetBits());
						pHeadDib->SetUpTime(pDib->GetUpTime());
						pHeadDib->SetUserFlag(pDib->IsUserFlag());
						pTail->AddTail(pHeadDib);
					}
					else
					{
						pNewDib = new CDib(*pDib);
						if (pNewDib)
							pTail->AddTail(pNewDib);
					}

					// Other old ones to remove?
					if ((int)dwQueueLengthMs > m_nMilliSecondsRecBeforeMovementBegin + 500) // +500ms to avoid continuous adds and removes
					{																		// (working well for high framerates, for low framerates it doesn't matter)
						// Shrink to a size of m_nMilliSecondsRecBeforeMovementBegin
						while (pTail->GetCount() > 1)
						{
							pHeadDib = pTail->GetHead();
							if (pHeadDib)
							{
								if ((dwNewUpTime - pHeadDib->GetUpTime()) <= (DWORD)m_nMilliSecondsRecBeforeMovementBegin)
									break;
								delete pHeadDib;
							}
							pTail->RemoveHead();
						}
					}
				}
			}
		}
		::LeaveCriticalSection(&m_csMovementDetectionsList);
	}
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

__forceinline void CVideoDeviceDoc::SaveFrameList()
{
	::EnterCriticalSection(&m_csMovementDetectionsList);
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

void CVideoDeviceDoc::OnViewDetectionZones() 
{
	m_bShowEditDetectionZones = !m_bShowEditDetectionZones;
	GetView()->ForceCursor(m_bShowEditDetectionZones);
	if (!m_bShowEditDetectionZones)
	{
		m_bShowEditDetectionZonesMinus = FALSE;
		::AfxGetMainFrame()->StatusText();
	}
	else
		::AfxGetMainFrame()->StatusText(ML_STRING(1483, "*** Click Inside The Capture Window to Add Zones. Press Ctrl (or Shift) to Remove Them ***"));
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

BOOL CVideoDeviceDoc::ConnectSendFrameUDP(	CNetCom* pNetCom,
											int nPort)
{
	// Check
	if (!pNetCom)
		return FALSE;

	// Clear Table
	m_pSendFrameParseProcess->ClearTable();

	// Set Thread Priority
	pNetCom->SetThreadsPriority(THREAD_PRIORITY_HIGHEST);

	// Init
	if (!pNetCom->Init(
					FALSE,					// No Meaning For Datagram
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
					m_pSendFrameParseProcess,// Parser
					NULL,					// No Generator
					SOCK_DGRAM,				// SOCK_DGRAM (UDP)
					_T(""),					// Local Address (IP or Host Name).
					nPort,					// Local Port.
					_T(""),					// Peer Address (IP or Host Name).
					0,						// Peer Port.
					NULL,					// Handle to an Event Object that will get Accept Events.
					NULL,					// Handle to an Event Object that will get Connect Events.
					NULL,					// Handle to an Event Object that will get Connect Failed Events.
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
					((CUImagerApp*)::AfxGetApp())->m_bIPv6 ? AF_INET6 : AF_INET)) // Socket family
	{
		m_sSendFrameMsg = ML_STRING(1486, "Error: cannot bind to the specified port, it may already be in use");
		ShowSendFrameMsg();
		return FALSE;
	}
	else
	{
		m_sSendFrameMsg = ML_STRING(1487, "Listening Ok");
		ShowSendFrameMsg();
		return TRUE;
	}
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


PIXORD or NETCOMM
-----------------

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


TRENDNET or  Raytalk RTAV 600W or Airlink101 or TRUST or GENIUS or
D-LINK DCS-700 and DCS-900 or PLANET ICA-100C and ICA-100PE
------------------------------------------------------------------

Note: Some people are writing image.jpg and video.cgi
lowercase some other highercase -> Try both!

JPEG
_T("GET /image.jpg HTTP/1.1\r\n")

MJPEG
_T("GET /video.cgi HTTP/1.1\r\n")


REPOTEC or TRENDNET TV-IP201W, TV-IP301 and TV-IP301W or PLANET ICA-110, ICA-210
--------------------------------------------------------------------------------

JPEG
_T("GET /goform/video HTTP/1.1\r\n")       -> get one frame, don't response until frame ready
_T("GET /goform/video2 HTTP/1.1\r\n")      -> get one frame, response no matter frame ready or not
_T("GET /goform/capture HTTP/1.1\r\n")     -> get one frame, server will disconnect the connection after


TRENDNET TV-IP410W, TV-IP110WN, TV-IP442W, TV-IP312W
----------------------------------------------------

JPEG
_T("GET /cgi/jpg/image.cgi HTTP/1.1\r\n")

MJPEG
_T("GET /cgi/mjpg/mjpg.cgi HTTP/1.1\r\n")
_T("GET /cgi/mjpg/mjpeg.cgi HTTP/1.1\r\n")


D-LINK
------

JPEG for DCS-2000, DCS-2100, DCS-3230, DCS-5300, DCS-6620G, DVS-104, DVS-301
_T("GET /cgi-bin/video.jpg HTTP/1.1\r\n")

JPEG for DCS-920, DCS-2121, DCS-2102, ...
_T("GET /image/jpeg.cgi HTTP/1.1\r\n")

MJPEG for DCS-920, DCS-2121, DCS-2102, ...
_T("GET /video/mjpg.cgi HTTP/1.1\r\n")


PLANET or SOLWISE or GADSPOT or VEO Observer
--------------------------------------------

JPEG
_T("GET /Jpeg/CamImg.jpg HTTP/1.1\r\n")


GADSPOT or ORITE or PLANET IVS-100, ICA-300, ICA-302, ICA-500
-------------------------------------------------------------

MJPEG
_T("GET /GetData.cgi HTTP/1.1\r\n")


GADSPOT GS1200G
---------------

JPEG
_T("GET /cgi-bin/getimage.cgi?motion=0 HTTP/1.1\r\n")

MJPEG
_T("GET /cgi-bin/getimage.cgi?motion=1 HTTP/1.1\r\n")


STARDOT (http://www.stardot-tech.com/developer/api.html)
--------------------------------------------------------

JPEG
_T("GET /netcam.jpg HTTP/1.1\r\n")

MJPEG
_T("GET /nph-mjpeg.cgi?x HTTP/1.1\r\n")  ,  (where x = input number, 0 origin) 


INTELLINET
----------

JPEG
_T("GET /jpg/image.jpg HTTP/1.1\r\n")


ANEXTEK
-------

MJPEG
_T("GET /cgi-bin/auto.cgi HTTP/1.1\r\n")


SONY
----

JPEG
_T("GET /oneshotimage.jpg HTTP/1.1\r\n")

MJPEG
_T("GET /image HTTP/1.1\r\n")


ARECONT VISION
--------------

JPEG
_T("GET /image HTTP/1.1\r\n")

MJPEG
_T("GET /mjpeg HTTP/1.1\r\n")


MOBOTIX (http://developer.mobotix.com/mobotix_sdk_1.0.2/paks/help_cgi-image.html)
---------------------------------------------------------------------------------

JPEG (this HTTP API supports so-called HTTP Keep Alive connections!)
_T("GET /record/current.jpg HTTP/1.1\r\n")

MJPEG
_T("GET /cgi-bin/faststream.jpg HTTP/1.1\r\n")


Blue Net Video Server
---------------------

JPEG
_T("GET /cgi-bin/image.cgi?control=0&id=admin&passwd=admin HTTP/1.1\r\n")


LINKSYS
-------

JPEG
_T("GET /img/snapshot.cgi HTTP/1.1\r\n")

MJPEG
_T("GET /img/video.mjpeg HTTP/1.1\r\n")
_T("GET /img/mjpeg.jpg HTTP/1.1\r\n")


BSTI or Heden VisionCam
-----------------------

JPEG
_T("GET /snapshot.cgi HTTP/1.1\r\n")

MJPEG
_T("GET /videostream.cgi HTTP/1.1\r\n")


CANON
-----

JPEG
_T("GET /-wvhttp-01-/GetLiveImage HTTP/1.1\r\n")
_T("GET /-wvhttp-01-/image.cgi HTTP/1.1\r\n")

MJPEG
_T("GET /-wvhttp-01-/GetOneShot HTTP/1.1\r\n")
_T("GET /-wvhttp-01-/video.cgi HTTP/1.1\r\n")
*/

BOOL CVideoDeviceDoc::ConnectGetFrame()
{
	BOOL res;

	// Check
	if (!m_pGetFrameNetCom || m_sGetFrameVideoHost == _T(""))
		return FALSE;
	
	// Close and Kill
	m_pGetFrameNetCom->Close();
	m_HttpGetFrameThread.Kill();

	// Connect
	switch (m_nNetworkDeviceTypeMode)
	{
		case INTERNAL_UDP :	// Internal UDP Server
		{
			if (m_pGetFrameParseProcess)
				m_pGetFrameParseProcess->Close();
			else
				m_pGetFrameParseProcess = (CGetFrameParseProcess*)new CGetFrameParseProcess(this);
			if (!m_pGetFrameGenerator)
				m_pGetFrameGenerator = (CGetFrameGenerator*)new CGetFrameGenerator;
			res = ConnectGetFrameUDP(m_sGetFrameVideoHost, m_nGetFrameVideoPort);
			break;
		}
		case OTHERONE :		// Other HTTP device
		{
			if (m_pHttpGetFrameParseProcess)
				m_pHttpGetFrameParseProcess->Close();
			else
				m_pHttpGetFrameParseProcess = (CHttpGetFrameParseProcess*)new CHttpGetFrameParseProcess(this);
			if (m_pHttpGetFrameParseProcess)
			{
				m_pHttpGetFrameParseProcess->m_bTryConnecting = TRUE;
				m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATUNKNOWN;
				m_nHttpGetFrameLocationPos = 0;
			}
			m_HttpGetFrameThread.Start();
			res = m_HttpGetFrameThread.SetEventConnect();
			break;
		}
		case AXIS_SP :		// Axis Server Push (mjpeg)
		{
			if (m_pHttpGetFrameParseProcess)
				m_pHttpGetFrameParseProcess->Close();
			else
				m_pHttpGetFrameParseProcess = (CHttpGetFrameParseProcess*)new CHttpGetFrameParseProcess(this);
			if (m_pHttpGetFrameParseProcess)
			{
				m_pHttpGetFrameParseProcess->m_bQueryProperties = TRUE;
				m_pHttpGetFrameParseProcess->m_bTryConnecting = TRUE;
				m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATMJPEG;
			}
			m_HttpGetFrameThread.Start();
			res = m_HttpGetFrameThread.SetEventConnect();
			break;
		}
		case AXIS_CP :		// Axis Client Poll (jpegs)
		{
			if (m_pHttpGetFrameParseProcess)
				m_pHttpGetFrameParseProcess->Close();
			else
				m_pHttpGetFrameParseProcess = (CHttpGetFrameParseProcess*)new CHttpGetFrameParseProcess(this);
			if (m_pHttpGetFrameParseProcess)
			{
				m_pHttpGetFrameParseProcess->m_bQueryProperties = TRUE;
				m_pHttpGetFrameParseProcess->m_bTryConnecting = TRUE;
				m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATJPEG;
			}
			m_HttpGetFrameThread.Start();
			res = m_HttpGetFrameThread.SetEventConnect();
			break;
		}
		case PANASONIC_SP :	// Panasonic Server Push (mjpeg)
		{
			if (m_pHttpGetFrameParseProcess)
				m_pHttpGetFrameParseProcess->Close();
			else
				m_pHttpGetFrameParseProcess = (CHttpGetFrameParseProcess*)new CHttpGetFrameParseProcess(this);
			if (m_pHttpGetFrameParseProcess)
			{
				m_pHttpGetFrameParseProcess->m_bTryConnecting = TRUE;
				m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATMJPEG;
				m_pHttpGetFrameParseProcess->m_Sizes.RemoveAll();			
				m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(160, 120));
				m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(192, 144));
				m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(320, 240));
				m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(640, 480));
				m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(1280, 1024)); // Support models such as BB-HCM515
			}
			m_HttpGetFrameThread.Start();
			res = m_HttpGetFrameThread.SetEventConnect();
			break;
		}
		case PANASONIC_CP :	// Panasonic Client Poll (jpegs)
		{
			if (m_pHttpGetFrameParseProcess)
				m_pHttpGetFrameParseProcess->Close();
			else
				m_pHttpGetFrameParseProcess = (CHttpGetFrameParseProcess*)new CHttpGetFrameParseProcess(this);
			if (m_pHttpGetFrameParseProcess)
			{
				m_pHttpGetFrameParseProcess->m_bTryConnecting = TRUE;
				m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATJPEG;
				m_pHttpGetFrameParseProcess->m_Sizes.RemoveAll();			
				m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(160, 120));
				m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(192, 144));
				m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(320, 240));
				m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(640, 480));
				m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(1280, 1024)); // Support models such as BB-HCM515
			}
			m_HttpGetFrameThread.Start();
			res = m_HttpGetFrameThread.SetEventConnect();
			break;
		}
		case PIXORD_SP :	// Pixord Server Push (mjpeg)
		{
			if (m_pHttpGetFrameParseProcess)
				m_pHttpGetFrameParseProcess->Close();
			else
				m_pHttpGetFrameParseProcess = (CHttpGetFrameParseProcess*)new CHttpGetFrameParseProcess(this);
			if (m_pHttpGetFrameParseProcess)
			{
				m_pHttpGetFrameParseProcess->m_bTryConnecting = TRUE;
				m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATMJPEG;
				m_pHttpGetFrameParseProcess->m_Sizes.RemoveAll();			
				m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(176, 112));
				m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(352, 240));
				m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(704, 480));
			}
			m_HttpGetFrameThread.Start();
			res = m_HttpGetFrameThread.SetEventConnect();
			break;
		}
		case PIXORD_CP :	// Pixord Client Poll (jpegs)
		{
			if (m_pHttpGetFrameParseProcess)
				m_pHttpGetFrameParseProcess->Close();
			else
				m_pHttpGetFrameParseProcess = (CHttpGetFrameParseProcess*)new CHttpGetFrameParseProcess(this);
			if (m_pHttpGetFrameParseProcess)
			{
				m_pHttpGetFrameParseProcess->m_bTryConnecting = TRUE;
				m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATJPEG;
				m_pHttpGetFrameParseProcess->m_Sizes.RemoveAll();			
				m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(176, 112));
				m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(352, 240));
				m_pHttpGetFrameParseProcess->m_Sizes.Add(CSize(704, 480));
			}
			m_HttpGetFrameThread.Start();
			res = m_HttpGetFrameThread.SetEventConnect();
			break;
		}
		case EDIMAX_SP :	// Edimax Server Push (mjpeg)
		{
			if (m_pHttpGetFrameParseProcess)
				m_pHttpGetFrameParseProcess->Close();
			else
				m_pHttpGetFrameParseProcess = (CHttpGetFrameParseProcess*)new CHttpGetFrameParseProcess(this);
			if (m_pHttpGetFrameParseProcess)
			{
				m_pHttpGetFrameParseProcess->m_bQueryProperties = TRUE;
				m_pHttpGetFrameParseProcess->m_bSetResolution = TRUE;
				m_pHttpGetFrameParseProcess->m_bSetCompression = TRUE;
				m_pHttpGetFrameParseProcess->m_bSetFramerate = TRUE;
				m_pHttpGetFrameParseProcess->m_bTryConnecting = TRUE;
				m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATMJPEG;
			}
			m_HttpGetFrameThread.Start();
			res = m_HttpGetFrameThread.SetEventConnect();
			break;
		}
		case EDIMAX_CP :	// Edimax Client Poll (jpegs)
		{
			if (m_pHttpGetFrameParseProcess)
				m_pHttpGetFrameParseProcess->Close();
			else
				m_pHttpGetFrameParseProcess = (CHttpGetFrameParseProcess*)new CHttpGetFrameParseProcess(this);
			if (m_pHttpGetFrameParseProcess)
			{
				m_pHttpGetFrameParseProcess->m_bQueryProperties = TRUE;
				m_pHttpGetFrameParseProcess->m_bSetResolution = TRUE;
				m_pHttpGetFrameParseProcess->m_bSetCompression = TRUE;
				m_pHttpGetFrameParseProcess->m_bTryConnecting = TRUE;
				m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATJPEG;
			}
			m_HttpGetFrameThread.Start();
			res = m_HttpGetFrameThread.SetEventConnect();
			break;
		}
		case TPLINK_SP :	// TP-Link Server Push (mjpeg)
		{
			if (m_pHttpGetFrameParseProcess)
				m_pHttpGetFrameParseProcess->Close();
			else
				m_pHttpGetFrameParseProcess = (CHttpGetFrameParseProcess*)new CHttpGetFrameParseProcess(this);
			if (m_pHttpGetFrameParseProcess)
			{
				m_pHttpGetFrameParseProcess->m_bTryConnecting = TRUE;
				m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATMJPEG;
			}
			m_HttpGetFrameThread.Start();
			res = m_HttpGetFrameThread.SetEventConnect();
			break;
		}
		case TPLINK_CP :	// TP-Link Client Poll (jpegs)
		{
			if (m_pHttpGetFrameParseProcess)
				m_pHttpGetFrameParseProcess->Close();
			else
				m_pHttpGetFrameParseProcess = (CHttpGetFrameParseProcess*)new CHttpGetFrameParseProcess(this);
			if (m_pHttpGetFrameParseProcess)
			{
				m_pHttpGetFrameParseProcess->m_bTryConnecting = TRUE;
				m_pHttpGetFrameParseProcess->m_FormatType = CHttpGetFrameParseProcess::FORMATJPEG;
			}
			m_HttpGetFrameThread.Start();
			res = m_HttpGetFrameThread.SetEventConnect();
			break;
		}
		default :
		{
			ASSERT(FALSE);
			res = FALSE;
			break;
		}
	}

	return res;
}

BOOL CVideoDeviceDoc::ConnectGetFrameUDP(LPCTSTR pszHostName, int nPort)
{
	// Set Thread Priority
	m_pGetFrameNetCom->SetThreadsPriority(THREAD_PRIORITY_HIGHEST);

	// Init UDP
	if (!m_pGetFrameNetCom->Init(
					FALSE,					// No Meaning For Datagram
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
					m_pGetFrameParseProcess,// Parser
					m_pGetFrameGenerator,	// Generator
					SOCK_DGRAM,				// SOCK_DGRAM (UDP)
					_T(""),					// Local Address (IP or Host Name).
					0,						// Local Port, let the OS choose one
					pszHostName,			// Peer Address (IP or Host Name).
					nPort,					// Peer Port.
					NULL,					// Handle to an Event Object that will get Accept Events.
					NULL,					// Handle to an Event Object that will get Connect Events.
					NULL,					// Handle to an Event Object that will get Connect Failed Events.
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
					GETFRAME_GENERATOR_RATE,/*uiTxPacketTimeout*/// After this timeout a Packet is sent
											// even if no Write Event Happened (A zero meens INFINITE Timeout).
											// This is also the Generator rate,
											// if set to zero the Generator is never called!
					NULL,					// Message Class for Notice, Warning and Error Visualization.
					((CUImagerApp*)::AfxGetApp())->m_bIPv6 ? AF_INET6 : AF_INET)) // Socket family
		return FALSE;
	else
	{
		m_pGetFrameNetCom->SetMaxTxDatagramBandwidth(NETFRAME_10MBPS_BANDWIDTH);
		m_pGetFrameNetCom->EnableIdleGenerator(TRUE);
		return TRUE;
	}
}

BOOL CVideoDeviceDoc::StoreUDPFrame(BYTE* Data,
									int Size,
									DWORD dwFrameUpTime,
									WORD wFrameSeq,
									BOOL bKeyFrame)
{
	CReSendFrame* pReSendFrame = new CReSendFrame(	Data,
													Size,
													dwFrameUpTime,
													wFrameSeq,
													bKeyFrame);
	if (pReSendFrame)
	{
		::EnterCriticalSection(&m_csReSendUDPFrameList);
		m_ReSendUDPFrameList.AddTail(pReSendFrame);
		if (m_ReSendUDPFrameList.GetCount() > NETFRAME_MAX_FRAMES)
		{
			delete m_ReSendUDPFrameList.GetHead();
			m_ReSendUDPFrameList.RemoveHead();
		}
		::LeaveCriticalSection(&m_csReSendUDPFrameList);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CVideoDeviceDoc::ReSendUDPFrame(sockaddr* pTo, WORD wFrameSeq)
{
	BOOL res = FALSE;
	::EnterCriticalSection(&m_csReSendUDPFrameList);
	POSITION nextpos = m_ReSendUDPFrameList.GetHeadPosition();
	while (nextpos)
	{
		POSITION currentpos = nextpos;
		CReSendFrame* pReSendFrame = m_ReSendUDPFrameList.GetNext(nextpos);
		if (pReSendFrame && pReSendFrame->m_wFrameSeq == wFrameSeq)
		{
			res = SendUDPFrame(	m_pSendFrameNetCom,
								pTo,
								pReSendFrame->m_Data,
								pReSendFrame->m_Size,
								pReSendFrame->m_dwFrameUpTime,
								pReSendFrame->m_wFrameSeq,
								pReSendFrame->m_bKeyFrame,
								m_nSendFrameMTU,
								TRUE,	// High Priority
								TRUE);	// Re-Sending
			break;
		}
	}
	::LeaveCriticalSection(&m_csReSendUDPFrameList);
	return res;
}

void CVideoDeviceDoc::ClearReSendUDPFrameList()
{
	::EnterCriticalSection(&m_csReSendUDPFrameList);
	while (!m_ReSendUDPFrameList.IsEmpty())
	{
		delete m_ReSendUDPFrameList.GetHead();
		m_ReSendUDPFrameList.RemoveHead();
	}
	::LeaveCriticalSection(&m_csReSendUDPFrameList);
}

BOOL CVideoDeviceDoc::SendUDPFrame(	CNetCom* pNetCom,
									sockaddr* pTo, // if NULL send to all!
									BYTE* Data,
									int Size,
									DWORD dwFrameUpTime,
									WORD wFrameSeq,
									BOOL bKeyFrame,
									int nMaxFragmentSize,
									BOOL bHighPriority,
									BOOL bReSending)
{
	// Check
	if (Data == NULL || Size <= 0 || pNetCom == NULL)
		return FALSE;

	// Fragment Size Init Value
	if (nMaxFragmentSize <= 0)
		nMaxFragmentSize = NETCOM_MAX_TX_BUFFER_SIZE;
	else
		nMaxFragmentSize = MIN(nMaxFragmentSize, NETCOM_MAX_TX_BUFFER_SIZE);
	if (nMaxFragmentSize <= sizeof(NetFrameHdrStruct))
		nMaxFragmentSize = sizeof(NetFrameHdrStruct) + 1;
	int nDataFragmentSize = nMaxFragmentSize - sizeof(NetFrameHdrStruct);
	
	// Fragments Count Calculation and Check
	int nTotalFragments = Size / nDataFragmentSize + 1; 
	if (nTotalFragments > NETFRAME_MAX_FRAGMENTS)
		return FALSE;

	// Header
	NetFrameHdrStruct Hdr;
	Hdr.dwUpTime = dwFrameUpTime;
	Hdr.wSeq = wFrameSeq;
	Hdr.TotalFragments = nTotalFragments;
	Hdr.Type = NETFRAME_TYPE_FRAME_ANS;
	Hdr.Flags = NETFRAME_FLAG_VIDEO;
	if (bKeyFrame)
	{
		Hdr.Flags |= NETFRAME_FLAG_KEYFRAME;
		if (!bReSending)
		{
			DWORD dwTimeDiff = dwFrameUpTime - m_dwLastSendUDPKeyFrameUpTime;
			DWORD dwBytesDiff = m_dwSendFrameTotalSentBytes - m_dwSendFrameTotalLastSentBytes;
			if (dwTimeDiff >= SENDFRAME_MIN_KEYFRAME_TIMEDIFF	&&
				dwTimeDiff <= SENDFRAME_MAX_KEYFRAME_TIMEDIFF	&&
				dwBytesDiff >= SENDFRAME_MIN_KEYFRAME_BYTESDIFF)
				m_dwSendFrameOverallDatarate = (DWORD)Round((double)dwBytesDiff / (double)dwTimeDiff * 1000.0);
			m_dwSendFrameTotalLastSentBytes = m_dwSendFrameTotalSentBytes;
			m_dwLastSendUDPKeyFrameUpTime = dwFrameUpTime;
		}
	}

	// Update the Maximum Total Fragments Per Stream And Per Frame
	m_dwMaxSendFrameFragmentsPerFrame = MAX(m_dwMaxSendFrameFragmentsPerFrame, (DWORD)nTotalFragments);
	
	// Send
	int nDataSendCount = 0;
	for (Hdr.FragmentNum = 0 ; Hdr.FragmentNum < nTotalFragments ; (Hdr.FragmentNum)++)
	{
		nDataSendCount += nDataFragmentSize;
		
		// Is Last Fragment?
		if (nDataSendCount > Size)
		{
			if (!SendUDPFragment(	pNetCom,
									pTo,
									(LPBYTE)&Hdr,
									sizeof(NetFrameHdrStruct),
									Data + (Hdr.FragmentNum * nDataFragmentSize),
									nDataFragmentSize - (nDataSendCount - Size),
									bHighPriority,
									bReSending))
				return FALSE;
		}
		else
		{
			if (!SendUDPFragment(	pNetCom,
									pTo,
									(LPBYTE)&Hdr,
									sizeof(NetFrameHdrStruct),
									Data + (Hdr.FragmentNum * nDataFragmentSize),
									nDataFragmentSize,
									bHighPriority,
									bReSending))
				return FALSE;
		}
	}

	return TRUE; 
}

__forceinline BOOL CVideoDeviceDoc::SendUDPFragment(CNetCom* pNetCom,
													sockaddr* pTo, // if NULL send to all!
													BYTE* Hdr,
													int HdrSize,
													BYTE* Data,
													int DataSize,
													BOOL bHighPriority,
													BOOL bReSending)
{
	// Check
	if (pNetCom == NULL)
		return FALSE;

	// Send
	::EnterCriticalSection(&m_pSendFrameParseProcess->m_csSendToTable);
	for (int i = 0 ; i < m_nSendFrameMaxConnections ; i++)
	{
		if (m_pSendFrameParseProcess->m_SendToTable[i].IsAddrSet())
		{
			// Send to All
			if (pTo == NULL)
			{
				SendUDPFragmentInternal(pNetCom,
										i,
										Hdr,
										HdrSize,
										Data,
										DataSize,
										bHighPriority,
										bReSending);
			}
			// Send to given one only
			else if (m_pSendFrameParseProcess->m_SendToTable[i].IsAddrEqualTo(pTo))
			{
				SendUDPFragmentInternal(pNetCom,
										i,
										Hdr,
										HdrSize,
										Data,
										DataSize,
										bHighPriority,
										bReSending);
				break;
			}
		}
	}
	::LeaveCriticalSection(&m_pSendFrameParseProcess->m_csSendToTable);

	return TRUE;
}

__forceinline void CVideoDeviceDoc::SendUDPFragmentInternal(	CNetCom* pNetCom,
																int nTo,
																BYTE* Hdr,
																int HdrSize,
																BYTE* Data,
																int DataSize,
																BOOL bHighPriority,
																BOOL bReSending)
{
	NetFrameHdrStruct* pHdr = (NetFrameHdrStruct*)Hdr;

	if (m_pSendFrameParseProcess->m_SendToTable[nTo].m_bDoSendFirstFrame)
	{
		// Send Keyframe?
		if ((pHdr->FragmentNum == 0) && (pHdr->Flags & NETFRAME_FLAG_KEYFRAME))
			m_pSendFrameParseProcess->m_SendToTable[nTo].m_bSendingKeyFrame = TRUE;

		// Do Send?
		if (m_pSendFrameParseProcess->m_SendToTable[nTo].m_bSendingKeyFrame)
		{
			// Inc. Count
			if (pHdr->FragmentNum == 0)
			{
				if (bReSending)
					++m_pSendFrameParseProcess->m_SendToTable[nTo].m_dwReSentFrameCount;
				else
					++m_pSendFrameParseProcess->m_SendToTable[nTo].m_dwSentFrameCount;
			}

			// Send Fragment
			m_dwSendFrameTotalSentBytes += (DWORD)pNetCom->WriteDatagramTo(	m_pSendFrameParseProcess->m_SendToTable[nTo].GetAddrPtr(),
																			Hdr, HdrSize, Data, DataSize, bHighPriority);
		}

		// Is Last Fragment of Keyframe?
		if (pHdr->FragmentNum == pHdr->TotalFragments - 1 &&
			m_pSendFrameParseProcess->m_SendToTable[nTo].m_bSendingKeyFrame)
		{
			m_pSendFrameParseProcess->m_SendToTable[nTo].m_bSendingKeyFrame = FALSE;
			m_pSendFrameParseProcess->m_SendToTable[nTo].m_bDoSendFirstFrame = FALSE;
		}
	}
	else
	{
		// Inc. Count
		if (pHdr->FragmentNum == 0)
		{
			if (bReSending)
				++m_pSendFrameParseProcess->m_SendToTable[nTo].m_dwReSentFrameCount;
			else
				++m_pSendFrameParseProcess->m_SendToTable[nTo].m_dwSentFrameCount;
		}
			
		// Send Fragment
		m_dwSendFrameTotalSentBytes += (DWORD)pNetCom->WriteDatagramTo(	m_pSendFrameParseProcess->m_SendToTable[nTo].GetAddrPtr(),
																		Hdr, HdrSize, Data, DataSize, bHighPriority);
	}
}

void CVideoDeviceDoc::UpdateFrameSendToTableAndFlowControl(BOOL b4SecTick)
{
	// Clear dead streams and count the number of alive connections
	int nCount = 0;
	::EnterCriticalSection(&m_pSendFrameParseProcess->m_csSendToTable);
	for (int i = 0 ; i < m_nSendFrameMaxConnections ; i++)
	{
		if (m_pSendFrameParseProcess->m_SendToTable[i].IsAddrSet())
		{
			if (m_pSendFrameParseProcess->m_SendToTable[i].IsKeepAliveOlderThan(SENDFRAME_MAX_CONNECTION_TIMEOUT))
				m_pSendFrameParseProcess->m_SendToTable[i].Clear();
			else
				nCount++;
		}
	}
	::LeaveCriticalSection(&m_pSendFrameParseProcess->m_csSendToTable);
	m_nSendFrameConnectionsCount = nCount;

	// Flow control every 4 sec
	if (b4SecTick)
	{
		// Get Tx Fifo Size
		::EnterCriticalSection(&m_csSendFrameNetCom);
		int nTxFifoSize = 0;
		if (m_pSendFrameNetCom)
			nTxFifoSize = (int)m_pSendFrameNetCom->GetTxFifoSize();
		::LeaveCriticalSection(&m_csSendFrameNetCom);

		// Update SendFrame Datarate Correction and SendFrame Max Connections
		double dNewSendFrameDatarateCorrection = m_dSendFrameDatarateCorrection;
		int nTxFifoSizePerConnection = 0;
		if (m_nSendFrameConnectionsCount > 0)
			nTxFifoSizePerConnection = nTxFifoSize / m_nSendFrameConnectionsCount;
		if (nTxFifoSizePerConnection > 32 * (int)m_dwMaxSendFrameFragmentsPerFrame)
		{
			m_nSendFrameMaxConnections = 0;
			dNewSendFrameDatarateCorrection = 1.0;
		}
		else if (nTxFifoSizePerConnection > 16 * (int)m_dwMaxSendFrameFragmentsPerFrame)
			dNewSendFrameDatarateCorrection += 2.0;
		else if (nTxFifoSizePerConnection > 8 * (int)m_dwMaxSendFrameFragmentsPerFrame)
			dNewSendFrameDatarateCorrection += 1.2;
		else if (nTxFifoSizePerConnection > 4 * (int)m_dwMaxSendFrameFragmentsPerFrame)
			dNewSendFrameDatarateCorrection += 0.7;
		else if (nTxFifoSizePerConnection > 2 * (int)m_dwMaxSendFrameFragmentsPerFrame)
			dNewSendFrameDatarateCorrection += 0.3;
		else if (nTxFifoSizePerConnection > (3 * (int)m_dwMaxSendFrameFragmentsPerFrame / 2))
			dNewSendFrameDatarateCorrection += 0.1;
		else if (nTxFifoSizePerConnection <= (int)m_dwMaxSendFrameFragmentsPerFrame)
		{
			// Restore max connections
			if (m_nSendFrameMaxConnections == 0 && nTxFifoSize == 0)
			{
				m_nSendFrameMaxConnections = m_nSendFrameMaxConnectionsConfig;
				dNewSendFrameDatarateCorrection = 1.25;
			}
			// Decrease datarate correction
			else
				dNewSendFrameDatarateCorrection -= 0.2;
		}

		// Clip
		if (dNewSendFrameDatarateCorrection < 1.0)
			dNewSendFrameDatarateCorrection = 1.0;
		else if (dNewSendFrameDatarateCorrection > 10.0)
			dNewSendFrameDatarateCorrection = 10.0;
		m_dSendFrameDatarateCorrection = dNewSendFrameDatarateCorrection;
	}

	// Update Total Send Bandwidth
	DWORD dwOverallDatarate;
	if (m_nSendFrameMaxConnections == 0)
	{
		dwOverallDatarate = MAX(m_dwSendFrameOverallDatarate,
							(DWORD)m_nSendFrameMaxConnectionsConfig * (DWORD)(m_nSendFrameDataRate / 8)); // bps -> bytes / sec
	}
	else
	{
		dwOverallDatarate = m_dwSendFrameOverallDatarate;
		if (dwOverallDatarate == 0)
			dwOverallDatarate = (DWORD)m_nSendFrameConnectionsCount * (DWORD)(m_nSendFrameDataRate / 8); // bps -> bytes / sec
		if (dwOverallDatarate == 0)
			dwOverallDatarate = (DWORD)m_nSendFrameMaxConnectionsConfig * (DWORD)(m_nSendFrameDataRate / 8); // bps -> bytes / sec
	}
	DWORD dwMaxBandwidth = MAX((DWORD)(SENDFRAME_MIN_DATARATE / 8),	// bps -> bytes / sec
							(DWORD)Round(m_dSendFrameDatarateCorrection * dwOverallDatarate));
	::EnterCriticalSection(&m_csSendFrameNetCom);
	if (m_pSendFrameNetCom)
		m_pSendFrameNetCom->SetMaxTxDatagramBandwidth(dwMaxBandwidth);
	::LeaveCriticalSection(&m_csSendFrameNetCom);
}

int CVideoDeviceDoc::CSendFrameParseProcess::Encode(CDib* pDib, CTime RefTime, DWORD dwRefUpTime)
{
	int res = 0;

	// Check
	if (!pDib)
		return -1;

	// (Re)Open AV Codec
	double dSendFrameRate = GetSendFrameRate();
	if (!pDib->IsSameBMI((LPBITMAPINFO)&m_CurrentBMI)			||
		m_nCurrentDataRate != m_pDoc->m_nSendFrameDataRate		||
		m_nCurrentSizeDiv != m_pDoc->m_nSendFrameSizeDiv		||
		m_nCurrentFreqDiv != m_pDoc->m_nSendFrameFreqDiv		||
		dSendFrameRate > m_dCurrentSendFrameRate * 1.3			||
		dSendFrameRate < m_dCurrentSendFrameRate * 0.7)
		OpenAVCodec(pDib->GetBMI());

	// Check Codec Context
	if (!m_pCodecCtx)
		return -1;

	// Fill Src Frame
	avpicture_fill(	(AVPicture*)m_pFrame,
					(uint8_t*)pDib->GetBits(),
					CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pDib->GetBMI()),
					pDib->GetWidth(),
					pDib->GetHeight());

	// Direct Encode?
	if (m_nCurrentSizeDiv == 0)
	{
		// Encode
		int nEncodedSize = avcodec_encode_video(m_pCodecCtx,
												m_pOutbuf + NETFRAME_HEADER_SIZE,
												m_nOutbufSize - NETFRAME_HEADER_SIZE,
												m_pFrame);
		if (nEncodedSize < 0)
			return -1;
		ASSERT(m_nOutbufSize + FF_INPUT_BUFFER_PADDING_SIZE >= nEncodedSize + NETFRAME_HEADER_SIZE + m_pCodecCtx->extradata_size);
		*((DWORD*)&m_pOutbuf[0]) = (DWORD)m_CodecID;
		*((DWORD*)&m_pOutbuf[4]) = m_dwEncryptionType;
		*((DWORD*)&m_pOutbuf[8]) = (DWORD)nEncodedSize;
		*((DWORD*)&m_pOutbuf[16]) = (DWORD)m_pCodecCtx->width;
		*((DWORD*)&m_pOutbuf[20]) = (DWORD)m_pCodecCtx->height;
		*((DWORD*)&m_pOutbuf[24]) = 0U; // Reserved1
		*((DWORD*)&m_pOutbuf[28]) = 0U; // Reserved2
		if (m_pCodecCtx->extradata_size > 0 &&
			IsKeyFrame()					&&
			dwRefUpTime - m_dwLastExtradataSendUpTime > SENDFRAME_EXTRADATA_SENDRATE)
		{
			m_dwLastExtradataSendUpTime = dwRefUpTime;
			memcpy(&m_pOutbuf[NETFRAME_HEADER_SIZE + nEncodedSize], m_pCodecCtx->extradata, m_pCodecCtx->extradata_size);
			*((DWORD*)&m_pOutbuf[12]) = (DWORD)m_pCodecCtx->extradata_size;
			return NETFRAME_HEADER_SIZE + nEncodedSize + m_pCodecCtx->extradata_size;
		}
		else
		{
			*((DWORD*)&m_pOutbuf[12]) = 0U;
			return NETFRAME_HEADER_SIZE + nEncodedSize;
		}
	}
	else
	{
		// Shrink
		if (m_pImgConvertCtx)
		{
			int sws_scale_res = sws_scale(	m_pImgConvertCtx,		// Image Convert Context
											m_pFrame->data,			// Source Data
											m_pFrame->linesize,		// Source Stride
											0,						// Source Slice Y
											pDib->GetHeight(),		// Source Height
											m_pFrameI420->data,		// Destination Data
											m_pFrameI420->linesize);// Destination Stride
#ifdef SUPPORT_LIBSWSCALE
			res = sws_scale_res > 0 ? 1 : -1;
#else
			res = sws_scale_res >= 0 ? 1 : -1;
#endif
		}		
		if (res == 1)
		{
			// Re-add frame time after shrinking
			if (m_pDoc->m_bShowFrameTime)
			{
				BITMAPINFOHEADER Bmi;
				memset(&Bmi, 0, sizeof(BITMAPINFOHEADER));
				Bmi.biSize = sizeof(BITMAPINFOHEADER);
				Bmi.biWidth = m_pCodecCtx->width;
				Bmi.biHeight = m_pCodecCtx->height;
				Bmi.biPlanes = 1;
				Bmi.biBitCount = 12;
				Bmi.biCompression = FCC('I420');
				Bmi.biSizeImage = m_dwI420ImageSize;
				CAVRec::AddFrameTime((LPBITMAPINFO)&Bmi,
									(LPBYTE)m_pFrameI420->data[0],
									pDib->GetUpTime(),
									RefTime,
									dwRefUpTime);
			}

			// Encode
			int nEncodedSize = avcodec_encode_video(m_pCodecCtx,
													m_pOutbuf + NETFRAME_HEADER_SIZE,
													m_nOutbufSize - NETFRAME_HEADER_SIZE,
													m_pFrameI420);
			if (nEncodedSize < 0)
				return -1;
			ASSERT(m_nOutbufSize + FF_INPUT_BUFFER_PADDING_SIZE >= nEncodedSize + NETFRAME_HEADER_SIZE + m_pCodecCtx->extradata_size);
			*((DWORD*)&m_pOutbuf[0]) = (DWORD)m_CodecID;
			*((DWORD*)&m_pOutbuf[4]) = m_dwEncryptionType;
			*((DWORD*)&m_pOutbuf[8]) = (DWORD)nEncodedSize;
			*((DWORD*)&m_pOutbuf[16]) = (DWORD)m_pCodecCtx->width;
			*((DWORD*)&m_pOutbuf[20]) = (DWORD)m_pCodecCtx->height;
			*((DWORD*)&m_pOutbuf[24]) = 0U; // Reserved1
			*((DWORD*)&m_pOutbuf[28]) = 0U; // Reserved2
			if (m_pCodecCtx->extradata_size > 0 &&
				IsKeyFrame()					&&
				dwRefUpTime - m_dwLastExtradataSendUpTime > SENDFRAME_EXTRADATA_SENDRATE)
			{
				m_dwLastExtradataSendUpTime = dwRefUpTime;
				memcpy(&m_pOutbuf[NETFRAME_HEADER_SIZE + nEncodedSize], m_pCodecCtx->extradata, m_pCodecCtx->extradata_size);
				*((DWORD*)&m_pOutbuf[12]) = (DWORD)m_pCodecCtx->extradata_size;
				return NETFRAME_HEADER_SIZE + nEncodedSize + m_pCodecCtx->extradata_size;
			}
			else
			{
				*((DWORD*)&m_pOutbuf[12]) = 0U;
				return NETFRAME_HEADER_SIZE + nEncodedSize;
			}
		}
	}

	return -1;
}

BOOL CVideoDeviceDoc::CSendFrameParseProcess::OpenAVCodec(LPBITMAPINFO pBMI)
{
	// Check
	if (!pBMI)
		return FALSE;

	// Free
	FreeAVCodec();

	// Frequency
	m_nCurrentFreqDiv = m_pDoc->m_nSendFrameFreqDiv;
	m_dCurrentSendFrameRate = GetSendFrameRate();
	AVRational CalcFrameRate = av_d2q(m_dCurrentSendFrameRate, MAX_SIZE_FOR_RATIONAL);

    // Find the decoder for the video stream
	m_pCodec = avcodec_find_encoder(m_CodecID);
    if (!m_pCodec)
        goto error_noclose;

	// Allocate Context
	m_pCodecCtx = avcodec_alloc_context();
	if (!m_pCodecCtx)
		goto error_noclose;

	// Set Output Width and Height
	m_nCurrentSizeDiv = m_pDoc->m_nSendFrameSizeDiv;
	m_pCodecCtx->width = pBMI->bmiHeader.biWidth >> m_nCurrentSizeDiv;
	m_pCodecCtx->height = pBMI->bmiHeader.biHeight >> m_nCurrentSizeDiv;

	// Key frame each second
	m_pCodecCtx->gop_size = m_dCurrentSendFrameRate <= 1.0 ? 1 : Round(m_dCurrentSendFrameRate);

	// No B-Frames
	m_pCodecCtx->max_b_frames = 0;

	// Format
	m_pCodecCtx->pix_fmt = PIX_FMT_YUV420P;
	m_pCodecCtx->codec_type = CODEC_TYPE_VIDEO;

	// Codec specific settings
	if (m_CodecID == CODEC_ID_SNOW)
	{
		m_pCodecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
		m_pCodecCtx->prediction_method = 0;
		m_pCodecCtx->me_cmp = 1;
		m_pCodecCtx->me_sub_cmp = 1;
		m_pCodecCtx->mb_cmp = 1;
		m_pCodecCtx->flags |= CODEC_FLAG_QPEL;
	}
	else if (m_CodecID == CODEC_ID_MPEG4)
	{
		m_pCodecCtx->mb_decision = 2;							// mbd: macroblock decision mode
		m_pCodecCtx->flags |= (CODEC_FLAG_AC_PRED			|	// aic: MPEG-4 AC prediction
								CODEC_FLAG_4MV);				// mv4: 4 MV per MB allowed
	}
	else if (	m_CodecID == CODEC_ID_H263P ||
				m_CodecID == CODEC_ID_H263)
	{
		m_pCodecCtx->mb_decision = 2;							// mbd: macroblock decision mode
		m_pCodecCtx->flags |=	(CODEC_FLAG_AC_PRED			|	// aic: H.263 advanced intra coding
								CODEC_FLAG_4MV				|	// mv4: advanced prediction for H.263
								CODEC_FLAG_LOOP_FILTER		|	// lf:  use loop filter (h263+)
								/*CODEC_FLAG_H263P_SLICE_STRUCT	|*/	// ssm: necessary if multi-threading (h263+)
								CODEC_FLAG_H263P_AIV		|	// aiv: H.263+ alternative inter VLC
								CODEC_FLAG_H263P_UMV);			// umv: Enable Unlimited Motion Vector (h263+)
	}
	else if (m_CodecID == CODEC_ID_THEORA)
		m_pCodecCtx->flags |= CODEC_FLAG2_FAST;
	else if (m_CodecID == CODEC_ID_MJPEG)
		m_pCodecCtx->strict_std_compliance = FF_COMPLIANCE_INOFFICIAL; // to allow the PIX_FMT_YUV420P format

	// Set Datarate
	m_nCurrentDataRate = m_pCodecCtx->bit_rate = m_pDoc->m_nSendFrameDataRate;

	/* time base: this is the fundamental unit of time (in seconds) in terms
       of which frame timestamps are represented. for fixed-fps content,
       timebase should be 1/framerate and timestamp increments should be
       identically 1. */
	m_pCodecCtx->time_base.den = CalcFrameRate.num;
	m_pCodecCtx->time_base.num = CalcFrameRate.den;

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

	// Allocate Outbuf
	m_nOutbufSize = NETFRAME_HEADER_SIZE + 4 * m_pCodecCtx->width * m_pCodecCtx->height + m_pCodecCtx->extradata_size;
	if (m_nOutbufSize < FF_MIN_BUFFER_SIZE)
		m_nOutbufSize = FF_MIN_BUFFER_SIZE;
	m_pOutbuf = new uint8_t[m_nOutbufSize + FF_INPUT_BUFFER_PADDING_SIZE];
	if (!m_pOutbuf)
	{
		m_nOutbufSize = 0;
		return FALSE;
	}

	// Init Image Convert
	if (m_nCurrentSizeDiv != 0)
	{
		// Determine required buffer size and allocate buffer if necessary
		m_dwI420ImageSize = avpicture_get_size(	PIX_FMT_YUV420P,
												m_pCodecCtx->width,
												m_pCodecCtx->height);
		if ((int)(m_dwI420BufSize) < m_dwI420ImageSize || m_pI420Buf == NULL)
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
		m_pImgConvertCtx = sws_getContext(	pBMI->bmiHeader.biWidth,	// Source Width
											pBMI->bmiHeader.biHeight,	// Source Height
											CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pBMI), // Source Format
											m_pCodecCtx->width,			// Destination Width
											m_pCodecCtx->height,		// Destination Height
											m_pCodecCtx->pix_fmt,		// Destination Format
											SWS_BICUBIC,				// SWS_CPU_CAPS_MMX2, SWS_CPU_CAPS_MMX, SWS_CPU_CAPS_3DNOW
											NULL,						// No Src Filter
											NULL,						// No Dst Filter
											NULL);						// Param
	}

	// Set Current BMI
	memcpy(&m_CurrentBMI, pBMI, MIN(sizeof(BITMAPINFOFULL), CDib::GetBMISize(pBMI)));

	return (m_pImgConvertCtx != NULL);

error:
	FreeAVCodec();
	return FALSE;
error_noclose:
	FreeAVCodec(TRUE);
	return FALSE;
}

void CVideoDeviceDoc::CSendFrameParseProcess::FreeAVCodec(BOOL bNoClose/*=FALSE*/)
{
	if (m_pCodecCtx)
	{
		// Close
		if (!bNoClose)
			avcodec_close_thread_safe(m_pCodecCtx);

		// Free
		if (m_pCodecCtx->stats_in)
			av_freep(&m_pCodecCtx->stats_in);
		if (m_pCodecCtx->extradata)
			av_freep(&m_pCodecCtx->extradata);
		m_pCodecCtx->extradata_size = 0;
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
	memset(&m_CurrentBMI, 0, sizeof(BITMAPINFOFULL));
	if (m_pOutbuf)
	{
		delete [] m_pOutbuf;
		m_pOutbuf = NULL;
	}
	m_nOutbufSize = 0;
}

BOOL CVideoDeviceDoc::CSendFrameParseProcess::Parse(CNetCom* pNetCom, BOOL bLastCall)
{
	int i;

	// The Received Datagram
	CNetCom::CBuf* pBuf = pNetCom->GetReadHeadBuf();
	
	// Check Packet Family
	if (pBuf->GetAddrPtr()->sa_family != AF_INET && pBuf->GetAddrPtr()->sa_family != AF_INET6)
	{
		pNetCom->RemoveReadHeadBuf();
		delete pBuf;
		return FALSE;
	}

	// Get Header
	NetFrameHdrStruct Hdr;
	if (pBuf->GetMsgSize() < sizeof(NetFrameHdrStruct))
	{
		pNetCom->RemoveReadHeadBuf();
		delete pBuf;
		return FALSE;
	}
	memcpy(&Hdr, pBuf->GetBuf(), sizeof(NetFrameHdrStruct));

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
		pNetCom->WriteDatagramTo(pBuf->GetAddrPtr(),
								(LPBYTE)&AnsPingHdr,
								sizeof(NetFrameHdrPingAuth),
								NULL,
								0,
								TRUE);

	}
	
	// Video?
	if (Hdr.Flags & NETFRAME_FLAG_VIDEO)
	{
		// Frame Confirmation?
		if (Hdr.Type & NETFRAME_TYPE_FRAME_CONF)
		{
			// Update Confirmed Frames Count
			::EnterCriticalSection(&m_csSendToTable);
			for (i = 0 ; i < m_pDoc->m_nSendFrameMaxConnections ; i++)
			{
				if (m_SendToTable[i].IsAddrEqualTo(pBuf->GetAddrPtr()))
				{
					m_SendToTable[i].m_dwConfirmedFrameCount++;
					break;
				}
			}
			::LeaveCriticalSection(&m_csSendToTable);
		}
		else if (Hdr.Type & NETFRAME_TYPE_FRAME_RESEND)
		{
			NetFrameHdrStructConf* pHdrConf = (NetFrameHdrStructConf*)&Hdr;
			m_pDoc->ReSendUDPFrame(pBuf->GetAddrPtr(), pHdrConf->wSeq);
		}
		else if (Hdr.Type & NETFRAME_TYPE_FRAMES_LOST)
		{
			NetFrameHdrStructFramesLost* pHdrLost = (NetFrameHdrStructFramesLost*)&Hdr;

			// Update Lost Frames Count
			::EnterCriticalSection(&m_csSendToTable);
			for (i = 0 ; i < m_pDoc->m_nSendFrameMaxConnections ; i++)
			{
				if (m_SendToTable[i].IsAddrEqualTo(pBuf->GetAddrPtr()))
				{
					m_SendToTable[i].m_dwLostFrameCount += pHdrLost->wLostCount;
					break;
				}
			}
			::LeaveCriticalSection(&m_csSendToTable);
		}
		
		// Frame Request & Keep-Alive?
		if ((Hdr.Type & NETFRAME_TYPE_FRAME_REQ)	||
			(Hdr.Type & NETFRAME_TYPE_FRAME_REQ_AUTH))
		{
			BOOL bKeepAlive = FALSE;

			::EnterCriticalSection(&m_csSendToTable);

			for (i = 0 ; i < m_pDoc->m_nSendFrameMaxConnections ; i++)
			{
				// Is Keep Alive?
				if (m_SendToTable[i].IsAddrEqualTo(pBuf->GetAddrPtr()))
				{
					if (Authenticate(pNetCom, pBuf, (NetFrameHdrPingAuth*)&Hdr, &m_SendToTable[i]))
						m_SendToTable[i].SetCurrentKeepAliveUpTime();
					bKeepAlive = TRUE;
					break;
				}
			}

			// New Addr?
			if (!bKeepAlive)
			{
				// Find an empty place, if any left
				for (i = 0 ; i < m_pDoc->m_nSendFrameMaxConnections ; i++)
				{
					if (!m_SendToTable[i].IsAddrSet())
					{
						if (Authenticate(pNetCom, pBuf, (NetFrameHdrPingAuth*)&Hdr, &m_SendToTable[i]))
						{
							m_SendToTable[i].SetCurrentKeepAliveUpTime();
							m_SendToTable[i].m_bDoSendFirstFrame = TRUE;
							m_SendToTable[i].SetAddr(pBuf->GetAddrPtr());
						}
						break;
					}
				}
			}

			::LeaveCriticalSection(&m_csSendToTable);
		}
	}

	// Clean-up
	pNetCom->RemoveReadHeadBuf();
	delete pBuf;

	return FALSE; // Never Call Processor!
}

BOOL CVideoDeviceDoc::CSendFrameParseProcess::Authenticate(CNetCom* pNetCom,
														   CNetCom::CBuf* pBuf,
														   NetFrameHdrPingAuth* pHdr,
														   CSendFrameToEntry* pTableEntry)
{
	// Get current up-time
	DWORD dwCurrentUpTime = ::timeGetTime();

	// Send an authentication request?
	if ((pHdr->Type & NETFRAME_TYPE_FRAME_REQ)	&&
		(m_pDoc->m_sSendFrameUsername != _T("")	||
		m_pDoc->m_sSendFramePassword != _T("")))
	{
		if ((dwCurrentUpTime - pTableEntry->m_dwAuthUpTime > GETFRAME_GENERATOR_RATE)	&&
			(!pTableEntry->m_bAuthSent || (dwCurrentUpTime - pTableEntry->m_dwAuthUpTime > SENDFRAME_AUTH_TIMEOUT)))
		{
			// Header
			NetFrameHdrPingAuth AuthHrd;
			pTableEntry->m_dwAuthUpTime = dwCurrentUpTime;
			AuthHrd.dwUpTime = pTableEntry->m_dwAuthUpTime;
			AuthHrd.wSeq = ++(pTableEntry->m_wAuthSeq);
			AuthHrd.dwExtraSize = 0;
			AuthHrd.Type = NETFRAME_TYPE_FRAME_REQ_AUTH;
			AuthHrd.Flags = NETFRAME_FLAG_VIDEO;
			if (pTableEntry->m_bAuthFailed)
				AuthHrd.Flags |= NETFRAME_FLAG_AUTH_FAILED;

			// Send
			if (pNetCom->WriteDatagramTo(pBuf->GetAddrPtr(),
										(LPBYTE)&AuthHrd,
										sizeof(NetFrameHdrPingAuth),
										NULL,
										0,
										TRUE) > 0)
				pTableEntry->m_bAuthSent = TRUE;
			else
				pTableEntry->m_bAuthSent = FALSE;
		}

		return FALSE; // Not authenticated yet
	}
	else
	{
		// Authentication response received, verify it
		if (pHdr->Type & NETFRAME_TYPE_FRAME_REQ_AUTH)
		{
			if (pHdr->dwExtraSize == 16									&&
				pBuf->GetMsgSize() >= sizeof(NetFrameHdrPingAuth) + 16	&&
				pTableEntry->m_bAuthSent								&&
				pHdr->wSeq == pTableEntry->m_wAuthSeq)
			{
				pTableEntry->m_bAuthSent = FALSE;
				USES_CONVERSION;
				CPJNMD5 hmac;
				CPJNMD5Hash calc_hash;
				CPJNMD5Hash rx_hash;
				memcpy(rx_hash.m_byHash, pBuf->GetBuf() + sizeof(NetFrameHdrPingAuth), 16);
				CString sNonce, sCNonce, sToHash;
				sNonce.Format(_T("%08x"), pTableEntry->m_dwAuthUpTime);
				sCNonce.Format(_T("%08x"), pHdr->dwUpTime);
				sToHash = m_pDoc->m_sSendFrameUsername + sNonce + m_pDoc->m_sSendFramePassword + sCNonce;
				char* psz = T2A(const_cast<LPTSTR>(sToHash.operator LPCTSTR()));
				if (hmac.Hash((const BYTE*)psz, (DWORD)strlen(psz), calc_hash))
				{
					if (memcmp(calc_hash.m_byHash, rx_hash.m_byHash, 16) == 0)
					{
						pTableEntry->m_bAuthFailed = FALSE;
						return TRUE; // Authentication ok
					}
					else
						pTableEntry->m_bAuthFailed = TRUE;
				}
			}

			return FALSE; // Not authenticated
		}
		// NETFRAME_TYPE_FRAME_REQ request with username and password not set
		else
			return TRUE; // Authentication ok
	}
}

void CVideoDeviceDoc::CSendFrameParseProcess::ClearTable()
{
	::EnterCriticalSection(&m_csSendToTable);

	for (int i = 0 ; i < m_pDoc->m_nSendFrameMaxConnections ; i++)
		m_SendToTable[i].Clear();

	::LeaveCriticalSection(&m_csSendToTable);
}

BOOL CVideoDeviceDoc::CGetFrameGenerator::Generate(CNetCom* pNetCom)
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

__forceinline BYTE CVideoDeviceDoc::CGetFrameParseProcess::ReSendCountDown(	int nReSendCount,
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
	switch (m_pDoc->m_dwGetFrameMaxFrames)
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

BOOL CVideoDeviceDoc::CGetFrameParseProcess::Parse(CNetCom* pNetCom, BOOL bLastCall)
{
	ASSERT(m_pDoc);
	DWORD dwFrame;
	DWORD dwTimeDiff;
	DWORD dwMaxFragmentAge = m_pDoc->m_dwGetFrameMaxFrames * 1000U; // ms

	// The Received Datagram
	CNetCom::CBuf* pBuf = pNetCom->GetReadHeadBuf();
	
	// Check Packet Family
	if (pBuf->GetAddrPtr()->sa_family != AF_INET && pBuf->GetAddrPtr()->sa_family != AF_INET6)
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

		// Prompt for username and password?
		if ((pReqAuthHdr->Flags & NETFRAME_FLAG_AUTH_FAILED)	||	// Wrong username and/or password
			(m_pDoc->m_sGetFrameUsername == _T("")	&&	// No username
			m_pDoc->m_sGetFramePassword == _T("")))		// No password
		{
			CAuthenticationDlg dlg;
			dlg.m_sUsername = m_pDoc->m_sGetFrameUsername;
			if (dlg.DoModal() == IDCANCEL)
			{
				pNetCom->RemoveReadHeadBuf();
				delete pBuf;
				return FALSE;
			}
			else
			{
				m_pDoc->m_sGetFrameUsername = dlg.m_sUsername;
				m_pDoc->m_sGetFramePassword = dlg.m_sPassword;
				if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings &&
					dlg.m_bSaveAuthenticationData)
				{
					CString sSection(m_pDoc->GetDevicePathName());
					((CUImagerApp*)::AfxGetApp())->WriteSecureProfileString(sSection, _T("GetFrameUsername"), m_pDoc->m_sGetFrameUsername);
					((CUImagerApp*)::AfxGetApp())->WriteSecureProfileString(sSection, _T("GetFramePassword"), m_pDoc->m_sGetFramePassword);
				}
			}
		}

		// Calc. hash and send it back
		USES_CONVERSION;
		CPJNMD5 hmac;
		CPJNMD5Hash calc_hash;
		CString sNonce, sCNonce, sToHash;
		sNonce.Format(_T("%08x"), pReqAuthHdr->dwUpTime);
		sCNonce.Format(_T("%08x"), AuthHrd.dwUpTime);
		sToHash = m_pDoc->m_sGetFrameUsername + sNonce + m_pDoc->m_sGetFramePassword + sCNonce;
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
		for (dwFrame = 0U ; dwFrame < m_pDoc->m_dwGetFrameMaxFrames ; dwFrame++)
		{
			m_dwUpTime[dwFrame] = Hdr.dwUpTime;
			m_dwFrameSize[dwFrame] = 0U;
			m_bKeyFrame[dwFrame] = FALSE;
		}
		for (dwFrame = 1U ; dwFrame < m_pDoc->m_dwGetFrameMaxFrames ; dwFrame++)
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
	for (dwFrame = 0U ; dwFrame < m_pDoc->m_dwGetFrameMaxFrames ; dwFrame++)
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
	for (dwFrame = 0U ; dwFrame < m_pDoc->m_dwGetFrameMaxFrames ; dwFrame++)
	{
		if (m_wFrameSeq[dwFrame] == Hdr.wSeq)
		{
			// Set Vars
			bFragmentAdded = TRUE;
			if (m_Fragment[dwFrame][Hdr.FragmentNum])
				delete m_Fragment[dwFrame][Hdr.FragmentNum];
			m_Fragment[dwFrame][Hdr.FragmentNum] = pBuf;
			pNetCom->RemoveReadHeadBuf();
			m_bKeyFrame[dwFrame] = ((Hdr.Flags & NETFRAME_FLAG_KEYFRAME) == NETFRAME_FLAG_KEYFRAME);
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
		for (dwFrame = 0U ; dwFrame < m_pDoc->m_dwGetFrameMaxFrames ; dwFrame++)
		{
			if (m_nTotalFragments[dwFrame] == 0)
			{
				// Set Vars
				bFragmentAdded = TRUE;
				if (m_Fragment[dwFrame][Hdr.FragmentNum])
					delete m_Fragment[dwFrame][Hdr.FragmentNum];
				m_Fragment[dwFrame][Hdr.FragmentNum] = pBuf;
				pNetCom->RemoveReadHeadBuf();
				m_bKeyFrame[dwFrame] = ((Hdr.Flags & NETFRAME_FLAG_KEYFRAME) == NETFRAME_FLAG_KEYFRAME);
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
			for (dwFrame = 1U ; dwFrame < m_pDoc->m_dwGetFrameMaxFrames ; dwFrame++)
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
				m_bKeyFrame[dwOldestIndex] = ((Hdr.Flags & NETFRAME_FLAG_KEYFRAME) == NETFRAME_FLAG_KEYFRAME);
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
	for (dwFrame = 0U ; dwFrame < m_pDoc->m_dwGetFrameMaxFrames ; dwFrame++)
	{
		if (m_dwFrameSize[dwFrame] > 0U)
			++dwReadyFramesCount;
	}

	// Frame Presentation
	DWORD dwFrameIndex;
	BOOL bFrameReady = FALSE;
	DWORD dwUpTimeDiff = dwCurrentUpTime - m_dwLastPresentationUpTime;
	DWORD dwUnit = m_pDoc->m_dwGetFrameMaxFrames / NETFRAME_MIN_FRAMES;
	DWORD dwMaxFrames2 = m_pDoc->m_dwGetFrameMaxFrames / 2U;
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
		for (dwFrame = 0U ; dwFrame < m_pDoc->m_dwGetFrameMaxFrames ; dwFrame++)
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
			for (dwFrame = 0U ; dwFrame < m_pDoc->m_dwGetFrameMaxFrames ; dwFrame++)
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
	if (!m_pDoc->m_bGetFrameDisableResend)
	{
		if (dwCurrentUpTime - m_dwLastReSendUpTime > 3U * m_dwAvgFrameTime / 2U)
		{
			WORD wHighestReadySeqDiff = 0U;
			for (dwFrame = 0U ; dwFrame < m_pDoc->m_dwGetFrameMaxFrames ; dwFrame++)
			{
				if (m_dwFrameSize[dwFrame] > 0U)
				{
					WORD wSeqDiff = m_wFrameSeq[dwFrame] - m_wPrevSeq;
					if (wSeqDiff > wHighestReadySeqDiff && wSeqDiff < 0x8000U)
						wHighestReadySeqDiff = wSeqDiff;
				}
			}
			WORD wSeqDiff = wHighestReadySeqDiff - (WORD)m_pDoc->m_dwGetFrameMaxFrames;
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
				// Reset
				memset(pFrame, 0, m_dwFrameSize[dwFrameIndex] + FF_INPUT_BUFFER_PADDING_SIZE);

				// Compose Fragments
				LPBYTE p = pFrame;
				for (int i = 0 ; i < m_nTotalFragments[dwFrameIndex] ; i++)
				{
					int nDataFragmentSize = m_Fragment[dwFrameIndex][i]->GetMsgSize() - sizeof(NetFrameHdrStruct);
					memcpy(p, m_Fragment[dwFrameIndex][i]->GetBuf() + sizeof(NetFrameHdrStruct), nDataFragmentSize);
					p += nDataFragmentSize;
				}

				// Decode and Process
				DecodeAndProcess(pFrame, m_dwFrameSize[dwFrameIndex]);

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

BOOL CVideoDeviceDoc::CGetFrameParseProcess::SendConfirmation(	CNetCom* pNetCom,
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

BOOL CVideoDeviceDoc::CGetFrameParseProcess::SendLostCount(	CNetCom* pNetCom,
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
	
	// Counter
	m_dwLostCount += wLostCount;

	// Send
	return (pNetCom->WriteDatagram(	(LPBYTE)&Hdr,
									sizeof(NetFrameHdrStructFramesLost),
									NULL,
									0,
									FALSE) > 0);
}

__forceinline BOOL CVideoDeviceDoc::CGetFrameParseProcess::ReSendFrame(	CNetCom* pNetCom,
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

__forceinline DWORD CVideoDeviceDoc::CGetFrameParseProcess::CalcFrameSize(DWORD dwFrame)
{
	DWORD dwSize = 0U;
	for (int i = 0 ; i < m_nTotalFragments[dwFrame] ; i++)
	{
		if (m_Fragment[dwFrame][i])
			dwSize += (m_Fragment[dwFrame][i]->GetMsgSize() - sizeof(NetFrameHdrStruct));
	}
	return dwSize;
}

__forceinline int CVideoDeviceDoc::CGetFrameParseProcess::GetReceivedFragmentsCount(DWORD dwFrame)
{
	int nCount = 0;
	for (int i = 0 ; i < m_nTotalFragments[dwFrame] ; i++)
	{
		if (m_Fragment[dwFrame][i])
			++nCount;
	}
	return nCount;
}

__forceinline BOOL CVideoDeviceDoc::CGetFrameParseProcess::IsFrameReady(WORD wSeq)
{
	for (DWORD dwFrame = 0U ; dwFrame < m_pDoc->m_dwGetFrameMaxFrames ; dwFrame++)
		if (wSeq == m_wFrameSeq[dwFrame] && m_dwFrameSize[dwFrame])
			return TRUE;
	return FALSE;
}

#ifdef _DEBUG
void CVideoDeviceDoc::CGetFrameParseProcess::TraceIncompleteFrame(DWORD dwFrame)
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

void CVideoDeviceDoc::CGetFrameParseProcess::TraceReSendCount()
{
	TRACE(_T("Prev Seq=%u\n"), m_wPrevSeq);
	for (int i = 0 ; i < NETFRAME_RESEND_ARRAY_SIZE ; i++)
	{
		if (m_ReSendCount[NETFRAME_RESEND_ARRAY_MASK & i] != 0)
		{
			TRACE(_T("m_ReSendCount[NETFRAME_RESEND_ARRAY_MASK & %d] = %u\n"),
								i, m_ReSendCount[NETFRAME_RESEND_ARRAY_MASK & i]);
		}
	}
	TRACE(_T("\n"));
}
#endif

BOOL CVideoDeviceDoc::CGetFrameParseProcess::OpenAVCodec(enum CodecID CodecId, int width, int height)
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
    if (avcodec_open_thread_safe(m_pCodecCtx, m_pCodec) < 0)
        goto error_noclose;

	// Allocate video frame
    m_pFrame = avcodec_alloc_frame();
	if (!m_pFrame)
        goto error;

	return TRUE;

error:
	FreeAVCodec();
	return FALSE;
error_noclose:
	FreeAVCodec(TRUE);
	return FALSE;
}

void CVideoDeviceDoc::CGetFrameParseProcess::FreeAVCodec(BOOL bNoClose/*=FALSE*/)
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
	if (m_pOutbuf)
	{
		delete [] m_pOutbuf;
		m_pOutbuf = NULL;
	}
	m_nOutbufSize = 0;
}

BOOL CVideoDeviceDoc::CGetFrameParseProcess::DecodeAndProcess(LPBYTE pFrame, DWORD dwFrameSize)
{
	// Minimum size check
	if (dwFrameSize < NETFRAME_HEADER_SIZE)
		return FALSE;

	// Get frame header
	NetFrameStruct FrameHdr;
	memcpy(&FrameHdr, pFrame, NETFRAME_HEADER_SIZE);

	// Check header consistency
	if (NETFRAME_HEADER_SIZE + FrameHdr.dwFrameDataSize + FrameHdr.dwExtraDataSize != dwFrameSize)
		return FALSE;

	// Init vars
	enum CodecID CodecId = (enum CodecID)FrameHdr.dwCodecID;
	switch (CodecId)
	{
		case  CODEC_ID_MJPEG	:	m_pDoc->m_CaptureBMI.bmiHeader.biCompression = FCC('MJPG'); break;
		case  CODEC_ID_H263		:
		case  CODEC_ID_H263P	:	m_pDoc->m_CaptureBMI.bmiHeader.biCompression = FCC('H263'); break;
		case  CODEC_ID_MPEG4	:	m_pDoc->m_CaptureBMI.bmiHeader.biCompression = FCC('DIVX'); break;
		case  CODEC_ID_THEORA	:	m_pDoc->m_CaptureBMI.bmiHeader.biCompression = FCC('theo'); break;
		case  CODEC_ID_SNOW		:	m_pDoc->m_CaptureBMI.bmiHeader.biCompression = FCC('SNOW'); break;
		default					:	m_pDoc->m_CaptureBMI.bmiHeader.biCompression = BI_RGB;      break;
	}
	if (CodecId == CODEC_ID_H263P)
		CodecId = CODEC_ID_H263;
	m_dwEncryptionType = FrameHdr.dwEncryptionType;
	if (m_dwEncryptionType != 0U) // Encryption not yet supported...
		return FALSE;
	DWORD dwFrameDataSize = FrameHdr.dwFrameDataSize;
	m_nExtradataSize = (int)FrameHdr.dwExtraDataSize;
	int width = (int)FrameHdr.dwWidth;
	int height = (int)FrameHdr.dwHeight;
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
		memcpy(	m_pExtradata,
				&pFrame[NETFRAME_HEADER_SIZE + dwFrameDataSize],
				m_nExtradataSize);
		memset(&pFrame[NETFRAME_HEADER_SIZE + dwFrameDataSize], 0, m_nExtradataSize);
	}

	// Re-init
	if (width != m_pDoc->m_DocRect.Width() || height != m_pDoc->m_DocRect.Height())
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
									(unsigned __int8 *)(pFrame + NETFRAME_HEADER_SIZE),
									(int)dwFrameDataSize);
	if (len > 0 && got_picture)
	{
		// Init Doc?
		if (m_bFirstFrame									||
			m_CodecId != CodecId							||
			m_pDoc->m_DocRect.Width() != m_pCodecCtx->width	||
			m_pDoc->m_DocRect.Height() != m_pCodecCtx->height)
		{
			m_pDoc->m_ProcessFrameBMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			m_pDoc->m_ProcessFrameBMI.bmiHeader.biWidth = (DWORD)m_pCodecCtx->width;
			m_pDoc->m_ProcessFrameBMI.bmiHeader.biHeight = (DWORD)m_pCodecCtx->height;
			m_pDoc->m_ProcessFrameBMI.bmiHeader.biPlanes = 1; // must be 1
			m_pDoc->m_ProcessFrameBMI.bmiHeader.biBitCount = 12;
			m_pDoc->m_ProcessFrameBMI.bmiHeader.biCompression = FCC('I420');    
			m_pDoc->m_ProcessFrameBMI.bmiHeader.biSizeImage = avpicture_get_size(PIX_FMT_YUV420P,
																				m_pCodecCtx->width,
																				m_pCodecCtx->height);
			m_pDoc->m_DocRect.right = m_pDoc->m_ProcessFrameBMI.bmiHeader.biWidth;
			m_pDoc->m_DocRect.bottom = m_pDoc->m_ProcessFrameBMI.bmiHeader.biHeight;
			m_CodecId = CodecId;
			m_bFirstFrame = FALSE;

			// Free Movement Detector because we changed size and/or format!
			m_pDoc->FreeMovementDetector();
			m_pDoc->ResetMovementDetector();

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

		// Convert to minimal stride
		int width2 = m_pCodecCtx->width >> 1;
		int height2 = m_pCodecCtx->height >> 1;
		if (m_pFrame->linesize[0] > m_pCodecCtx->width	||
			m_pFrame->linesize[1] > width2				||
			m_pFrame->linesize[2] > width2)
		{
			if (m_nOutbufSize < (12 * m_pCodecCtx->width * m_pCodecCtx->height / 8) || m_pOutbuf == NULL)
			{
				if (m_pOutbuf)
					delete [] m_pOutbuf;
				m_nOutbufSize = 12 * m_pCodecCtx->width * m_pCodecCtx->height / 8;
				m_pOutbuf = new uint8_t[m_nOutbufSize + FF_INPUT_BUFFER_PADDING_SIZE];
				if (!m_pOutbuf)
					return FALSE;
			}
			int h;
			LPBYTE p = (LPBYTE)m_pOutbuf;
			for (h = 0 ; h < m_pCodecCtx->height ; h++)
			{
				memcpy(p, m_pFrame->data[0] + h * m_pFrame->linesize[0], m_pCodecCtx->width);
				p += m_pCodecCtx->width;
			}
			for (h = 0 ; h < height2 ; h++)
			{
				memcpy(p, m_pFrame->data[1] + h * m_pFrame->linesize[1], width2);
				p += width2;
			}
			for (h = 0 ; h < height2 ; h++)
			{
				memcpy(p, m_pFrame->data[2] + h * m_pFrame->linesize[2], width2);
				p += width2;
			}
			m_pDoc->m_lCompressedDataRateSum += dwFrameSize;
			m_pDoc->ProcessI420Frame(	(LPBYTE)(m_pOutbuf),
										m_pDoc->m_ProcessFrameBMI.bmiHeader.biSizeImage);
		}
		else
		{
			m_pDoc->m_lCompressedDataRateSum += dwFrameSize;
			m_pDoc->ProcessI420Frame(	(LPBYTE)(m_pFrame->data[0]),
										m_pDoc->m_ProcessFrameBMI.bmiHeader.biSizeImage);
		}
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CVideoDeviceDoc::CHttpGetFrameParseProcess::SendRawRequest(const CString& sRequest)
{
	CString sMsg;
	m_sLastRequest = sRequest;

	TRACE(sRequest);

	if (m_AnswerAuthorizationType == AUTHBASIC)
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
		sMsg.Format(_T("%s")
					_T("Host: %s\r\n")
					/*_T("User-Agent: Mozilla/4.0 (Windows) %s/%s\r\n")*/
					/*_T("Accept: text/html,text/plain,image/jpeg,multipart/x-mixed-replace\r\n")*/
					/*_T("Accept-Encoding:\r\n")	// No encoding accepted */
					/*_T("Accept-Charset: ISO-8859-1\r\n")*/
					_T("Connection: keep-alive\r\n")
					_T("Authorization: Basic %s\r\n\r\n"),
					sRequest,
					m_pDoc->m_sGetFrameVideoHost,
					/*APPNAME_NOEXT,
					APPVERSION,*/
					lpszAuthorization);
	}
	else if (m_AnswerAuthorizationType == AUTHDIGEST)
	{
		m_AnswerAuthorizationType = AUTHNONE; // reset it!
		m_LastRequestAuthorizationType = AUTHDIGEST;
		BOOL bQop = FALSE;
		CString sQopLowerCase = m_sQop;
		sQopLowerCase.MakeLower();
		if (sQopLowerCase.Find(_T("auth")) >= 0)
			bQop = TRUE;
		USES_CONVERSION;
		int nPos, nPosEnd;
		CString sMethod;
		CString sUri;
		CString sHA1;
		CString sHA2;
		CString sHResponse;
		CString sToHash;
		CPJNMD5 hmac;
		CPJNMD5Hash hash;
		if ((nPos = sRequest.Find(_T(' '), 0)) >= 0)
		{
			sMethod = sRequest.Left(nPos);
			nPos++;
			if ((nPosEnd = sRequest.Find(_T(' '), nPos)) >= 0)
				sUri = sRequest.Mid(nPos, nPosEnd - nPos);
		}

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
			sMsg.Format(_T("%s")
						_T("Host: %s\r\n")
						/*_T("User-Agent: Mozilla/4.0 (Windows) %s/%s\r\n")*/
						/*_T("Accept: text/html,text/plain,image/jpeg,multipart/x-mixed-replace\r\n")*/
						/*_T("Accept-Encoding:\r\n")	// No encoding accepted */
						/*_T("Accept-Charset: ISO-8859-1\r\n")*/
						_T("Connection: keep-alive\r\n")
						_T("Authorization: Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",qop=auth,nc=%s,cnonce=\"%s\",response=\"%s\",opaque=\"%s\"\r\n\r\n"),
						sRequest,
						m_pDoc->m_sGetFrameVideoHost,
						/*APPNAME_NOEXT,
						APPVERSION,*/
						m_pDoc->m_sHttpGetFrameUsername,
						m_sRealm,
						m_sNonce,
						sUri,
						sCNonceCount,
						sCNonce,
						sHResponse,
						m_sOpaque);
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
			sMsg.Format(_T("%s")
						_T("Host: %s\r\n")
						/*_T("User-Agent: Mozilla/4.0 (Windows) %s/%s\r\n")*/
						/*_T("Accept: text/html,text/plain,image/jpeg,multipart/x-mixed-replace\r\n")*/
						/*_T("Accept-Encoding:\r\n")	// No encoding accepted */
						/*_T("Accept-Charset: ISO-8859-1\r\n")*/
						_T("Connection: keep-alive\r\n")
						_T("Authorization: Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s\",response=\"%s\",opaque=\"%s\"\r\n\r\n"),
						sRequest,
						m_pDoc->m_sGetFrameVideoHost,
						/*APPNAME_NOEXT,
						APPVERSION,*/
						m_pDoc->m_sHttpGetFrameUsername,
						m_sRealm,
						m_sNonce,
						sUri,
						sHResponse,
						m_sOpaque);
		}
	}
	else
	{
		m_LastRequestAuthorizationType = AUTHNONE;
		// Keep it short because some stupid ip cams (like Planet)
		// run out of buffer or do not parse well if we send to much!
		sMsg.Format(_T("%s")
					_T("Host: %s\r\n")
					/*_T("User-Agent: Mozilla/4.0 (Windows) %s/%s\r\n")*/
					/*_T("Accept: text/html,text/plain,image/jpeg,multipart/x-mixed-replace\r\n")*/
					/*_T("Accept-Encoding:\r\n")	// No encoding accepted */
					/*_T("Accept-Charset: ISO-8859-1\r\n")*/
					_T("Connection: keep-alive\r\n\r\n"),
					sRequest,
					m_pDoc->m_sGetFrameVideoHost/*,
					APPNAME_NOEXT,
					APPVERSION*/);
	}

	// Send
	return (m_pNetCom->WriteStr(sMsg) > 0);
}

BOOL CVideoDeviceDoc::CHttpGetFrameParseProcess::SendRequest()
{
	CString sLocation;
	CString sRequest;

	::EnterCriticalSection(&m_pDoc->m_csHttpParams);

	switch (m_pDoc->m_nNetworkDeviceTypeMode)
	{
		case OTHERONE :		// Other HTTP device
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
			if ((nPosEnd = sMsg.Find(_T("\r\n"), nPosType)) < 0)
			if ((nPosEnd = sMsg.Find(_T('\n'), nPosType)) < 0)
			if ((nPosEnd = sMsg.Find(_T('\r'), nPosType)) < 0)
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
		if ((nPosEnd = sMsg.Find(_T("\r\n"), nPos)) < 0)
		if ((nPosEnd = sMsg.Find(_T('\n'), nPos)) < 0)
		if ((nPosEnd = sMsg.Find(_T('\r'), nPos)) < 0)
			return FALSE;

		// Parse Content Length
		CString sMultipartLength = sMsgLowerCase.Mid(nPos, nPosEnd - nPos);
		if (sMultipartLength == _T(""))
			return FALSE;
		sMultipartLength.TrimLeft();
		if ((nPos = sMultipartLength.Find(_T(' '), 0)) >= 0)
			sMultipartLength = sMultipartLength.Left(nPos);
		nMultipartLength = _ttoi(sMultipartLength);
		if (nMultipartLength <= 0)
			return FALSE;

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
		if ((nPosEnd = sMsg.Find(_T("\r\n"), nPos)) < 0)
		if ((nPosEnd = sMsg.Find(_T('\n'), nPos)) < 0)
		if ((nPosEnd = sMsg.Find(_T('\r'), nPos)) < 0)
			return FALSE;
		CString sSinglepartLength = sMsgLowerCase.Mid(nPos, nPosEnd - nPos);
		if (sSinglepartLength == _T(""))
			return FALSE;
		sSinglepartLength.TrimLeft();
		if ((nPos = sSinglepartLength.Find(_T(' '), 0)) >= 0)
			sSinglepartLength = sSinglepartLength.Left(nPos);
		int nSinglepartLength = _ttoi(sSinglepartLength);
		if (nSinglepartLength <= 0)
			return FALSE;
		else
			m_nProcessSize = nSinglepartLength;
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
				if ((nPosEnd = sMsg.Find(_T("\r\n"), nPos)) < 0)
				{
					if ((nPosEnd = sMsg.Find(_T("\n\n"), nPos)) < 0)
					{
						delete [] pMsg;
						return FALSE; // Do not call Processor
					}
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
				if (m_pDoc->m_nNetworkDeviceTypeMode == OTHERONE && m_bTryConnecting &&
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
					m_pDoc->m_HttpGetFrameThread.SetEventConnect(_T(""));
				}
				else if (m_bSetCompression)
				{
					// Reset flags
					m_bSetCompression = FALSE;
					m_bTryConnecting = FALSE;

					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Start Connection
					m_pDoc->m_HttpGetFrameThread.SetEventConnect(_T(""));
				}
				else if (m_bSetFramerate)
				{
					// Reset flags
					m_bSetFramerate = FALSE;
					m_bTryConnecting = FALSE;

					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Start Connection
					m_pDoc->m_HttpGetFrameThread.SetEventConnect(_T(""));
				}
				else if (m_bTryConnecting)
				{
					// Reset flag
					m_bTryConnecting = FALSE;

					// Msg
					::AfxMessageBox(ML_STRING(1488, "Camera is telling you something,\nfirst open it in a browser, then come back here."), MB_ICONSTOP);
					
					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Close
					m_pDoc->CloseDocRemoveAutorunDev();
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
				if (m_pDoc->m_nNetworkDeviceTypeMode == OTHERONE && m_bTryConnecting &&
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
					::AfxMessageBox(ML_STRING(1489, "Camera is asking you something (probably to set a password),\nfirst open it in a browser, then come back here."), MB_ICONSTOP);
					
					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();
					
					// Close
					m_pDoc->CloseDocRemoveAutorunDev();
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
			if ((nPosEnd = sMsgLowerCase.Find(_T("\r\n"), nPos)) < 0)
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
			if ((nPosEnd = sMsgLowerCase.Find(_T("\r\n"), nPos)) < 0)
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
				if ((nPosEnd = sMsgLowerCase.Find(_T("\r\n"), nPos)) < 0)
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

			// Pop-up authentication dialog?
			if (m_LastRequestAuthorizationType != AUTHNONE	||	// Wrong username and/or password
				(m_pDoc->m_sHttpGetFrameUsername == _T("")	&&	// No username
				m_pDoc->m_sHttpGetFramePassword == _T("")))		// No password
			{
				CAuthenticationDlg dlg;
				dlg.m_sUsername = m_pDoc->m_sHttpGetFrameUsername;
				if (dlg.DoModal() == IDCANCEL)
				{
					if (m_bTryConnecting)
					{
						m_bTryConnecting = FALSE;
						m_pDoc->CloseDocRemoveAutorunDev();
					}
					else
						m_pDoc->CloseDocument();
					delete [] pMsg;
					return FALSE; // Do not call Processor
				}
				else
				{
					m_pDoc->m_sHttpGetFrameUsername = dlg.m_sUsername;
					m_pDoc->m_sHttpGetFramePassword = dlg.m_sPassword;
					if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings &&
						dlg.m_bSaveAuthenticationData)
					{
						CString sSection(m_pDoc->GetDevicePathName());
						((CUImagerApp*)::AfxGetApp())->WriteSecureProfileString(sSection, _T("HTTPGetFrameUsername"), m_pDoc->m_sHttpGetFrameUsername);
						((CUImagerApp*)::AfxGetApp())->WriteSecureProfileString(sSection, _T("HTTPGetFramePassword"), m_pDoc->m_sHttpGetFramePassword);
					}
				}
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
			if (m_pDoc->m_nNetworkDeviceTypeMode == OTHERONE && m_bTryConnecting &&
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
					::AfxMessageBox(ML_STRING(1491, "Server is to busy, try later"), MB_ICONSTOP);
				else
					::AfxMessageBox(ML_STRING(1490, "Unsupported network device type or mode"), MB_ICONSTOP);

				// Empty the buffers, so that parser stops calling us!
				pNetCom->Read();

				// Close
				m_pDoc->CloseDocRemoveAutorunDev();
			}
			// Maybe we polled to fast or we changed a param and camera is not yet ready
			else
			{
				// Empty the buffers, so that parser stops calling us!
				pNetCom->Read();

				// Retry start connection with delay
				m_pDoc->m_HttpGetFrameThread.SetEventConnect(_T(""), HTTPGETFRAME_CONNECTION_STARTDELAY);
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
			m_pDoc->ResetMovementDetector();

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
			m_pDoc->ProcessI420Frame(m_pI420Buf, m_dwI420ImageSize);
		}
	}
	// In case that avcodec_decode_video fails
	// use LoadJPEG which is more fault tolerant, but slower...
	else
	{
		TRACE(_T("*** Error: ffmpeg failed to decode mjpeg, trying CDib::LoadJPEG() ***\n"));
		CDib Dib;
		Dib.SetShowMessageBoxOnError(FALSE);
		if (Dib.LoadJPEG(pLinBuf, nSize) && Dib.Compress(FCC('I420')))
		{
			m_pDoc->m_lCompressedDataRateSum += nSize;
			m_pDoc->ProcessI420Frame(Dib.GetBits(), Dib.GetImageSize());
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
	// Determine required buffer size and allocate buffer if necessary
	m_dwI420ImageSize = avpicture_get_size(	PIX_FMT_YUV420P,
											m_pCodecCtx->width,
											m_pCodecCtx->height);
	if ((int)(m_dwI420BufSize) < m_dwI420ImageSize || m_pI420Buf == NULL)
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
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
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
	}
	return _T("");
}

void CVideoDeviceDoc::AutorunAddDevice(const CString& sDevicePathName)
{
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings &&
		!((CUImagerApp*)::AfxGetApp())->m_bServiceProcess) // No autorun changes in service mode!
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
}

void CVideoDeviceDoc::AutorunRemoveDevice(const CString& sDevicePathName)
{
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings &&
		!((CUImagerApp*)::AfxGetApp())->m_bServiceProcess) // No autorun changes in service mode!
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
