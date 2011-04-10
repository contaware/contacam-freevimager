#include "stdafx.h"
#include "uImager.h"
#include "VideoDeviceView.h"
#include "MainFrm.h"
#include "VideoDeviceDoc.h"
#include "VideoAviDoc.h"
#include "VideoAviView.h"
#include "AudioInSourceDlg.h"
#include "CaptureDeviceDlg.h"
#include "AssistantPage.h"
#include "GeneralPage.h"
#include "SnapshotPage.h"
#include "VideoDevicePropertySheet.h"
#include "ColorDetectionPage.h"
#include "Quantizer.h"
#include "DxCapture.h"
#include "DxVideoFormatDlg.h"
#include "DxVideoInputDlg.h"
#include "AudioFormatDlg.h"
#include "HttpVideoFormatDlg.h"
#include "AuthenticationDlg.h"
#include "SendMailConfigurationDlg.h"
#include "FTPUploadConfigurationDlg.h"
#include "HostPortDlg.h"
#include "FTPTransfer.h"
#include "Fourier.h"
#include "NetFrameHdr.h"
#include "NetworkPage.h"
#include "getdxver.h"
#include "BrowseDlg.h"
#include "PostDelayedMessage.h"
#include "MotionDetHelpers.h"
#include "Base64.h"
#include "PJNMD5.h"
#include "Psapi.h"
#include "IniFile.h"
#include "NoVistaFileDlg.h"
#ifdef CRACKCHECK
#include "Crc32Dynamic.h"
#endif

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
	ON_COMMAND(ID_CAPTURE_DEINTERLACE, OnCaptureDeinterlace)
	ON_UPDATE_COMMAND_UI(ID_CAPTURE_DEINTERLACE, OnUpdateCaptureDeinterlace)
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

__forceinline BOOL CVideoDeviceDoc::CreateCheckYearMonthDayDir(CTime Time, CString sBaseDir, CString& sYearMonthDayDir)
{
	// Remove Trailing '\'
	sBaseDir.TrimRight(_T('\\'));

	// The Dir Strings
	CString sYear = Time.Format(_T("%Y"));
	CString sMonth = Time.Format(_T("%m"));
	CString sDay = Time.Format(_T("%d"));

	// Check Passed Dir
	DWORD dwAttrib =::GetFileAttributes(sBaseDir);
	if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) // Not Existing or Not A Directory
		return FALSE;

	// Create Year Dir if not existing
	dwAttrib =::GetFileAttributes(sBaseDir + _T("\\") + sYear);
	if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) // Not Existing or Not A Directory
	{
		if (!::CreateDir(sBaseDir + _T("\\") + sYear))
		{
			::ShowLastError(FALSE);
			return FALSE;
		}
	}

	// Create Month Dir if not existing
	dwAttrib =::GetFileAttributes(sBaseDir + _T("\\") + sYear + _T("\\") + sMonth);
	if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) // Not Existing or Not A Directory
	{
		if (!::CreateDir(sBaseDir + _T("\\") + sYear + _T("\\") + sMonth))
		{
			::ShowLastError(FALSE);
			return FALSE;
		}
	}

	// Create Day Dir if not existing
	dwAttrib =::GetFileAttributes(sBaseDir + _T("\\") + sYear + _T("\\") + sMonth + _T("\\") + sDay);
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

		// Current Reference Time, Current Reference Up-Time and
		// First Frame Time
		CTime RefTime = CTime::GetCurrentTime();
		DWORD dwRefUpTime = ::timeGetTime();
		DWORD dwTimeDifference = dwRefUpTime - dwFirstUpTime;
		CTimeSpan TimeSpan((time_t)(dwTimeDifference > 0U ? Round((double)dwTimeDifference / 1000.0) : 0));
		CTime FirstTime = RefTime - TimeSpan;
		CString sTime(FirstTime.Format(_T("%Y_%m_%d_%H_%M_%S")));
		
		// Directory to Store Detection
		CString sDetectionAutoSaveDir;
		if (m_pDoc->m_bSaveSWFMovementDetection	||
			m_pDoc->m_bSaveAVIMovementDetection ||
			m_pDoc->m_bSaveAnimGIFMovementDetection)
		{
			// Check Whether Detection Dir Exists
			sDetectionAutoSaveDir = m_pDoc->m_sDetectionAutoSaveDir;
			DWORD dwAttrib =::GetFileAttributes(sDetectionAutoSaveDir);
			if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) // Not Existing or Not A Directory
			{
				// Temp Dir To Store Files
				sDetectionAutoSaveDir = ((CUImagerApp*)::AfxGetApp())->GetAppTempDir() + _T("Detection");
				dwAttrib =::GetFileAttributes(sDetectionAutoSaveDir);
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
			sDetectionAutoSaveDir = ((CUImagerApp*)::AfxGetApp())->GetAppTempDir() + _T("Detection");
			DWORD dwAttrib =::GetFileAttributes(sDetectionAutoSaveDir);
			if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) // Not Existing or Not A Directory
			{
				if (!::CreateDir(sDetectionAutoSaveDir))
					::ShowLastError(FALSE);
			}
		}

		// Detection File Names (Full-Path)
		CString sAVIFileName;
		CString sSWFFileName;
		CString sGIFFileName;
		CString sGIFTempFileName;
		CVideoDeviceDoc::CreateCheckYearMonthDayDir(FirstTime, sDetectionAutoSaveDir, sAVIFileName);
		sGIFFileName = sSWFFileName = sAVIFileName;
		if (sAVIFileName == _T(""))
			sAVIFileName = _T("det_") + sTime + _T(".avi");
		else
			sAVIFileName = sAVIFileName + _T("\\") + _T("det_") + sTime + _T(".avi");
		if (sSWFFileName == _T(""))
			sSWFFileName = _T("det_") + sTime + _T(".swf");
		else
			sSWFFileName = sSWFFileName + _T("\\") + _T("det_") + sTime + _T(".swf");
		if (sGIFFileName == _T(""))
			sGIFFileName = _T("det_") + sTime + _T(".gif");
		else
			sGIFFileName = sGIFFileName + _T("\\") + _T("det_") + sTime + _T(".gif");
		sGIFTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sGIFFileName);
		
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
		int nAnimGifLastFrameToSave = 1;
		double dDelayMul = 1.0;
		double dSpeedMul = 1.0;
		CDib AVISaveDib;
		CDib SWFSaveDib;
		CDib GIFSaveDib;
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
					AVRecSwf.AddVideoStream((LPBITMAPINFO)SWFSaveDib.GetBMI(),	// Source Video Format
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
					AVRecAvi.AddVideoStream((LPBITMAPINFO)AVISaveDib.GetBMI(),		// Source Video Format
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

			// Animated Gif, because of differencing the saving is shifted by one frame
			BOOL bSaveGif = DoSaveGif();
			if (bSaveGif)
			{
				// Normal saving
				if (nFrames >= nAnimGifLastFrameToSave)
				{
					// First Frame?
					if (nFrames == m_nNumFramesToSave)
					{
						bFirstGIFSave = TRUE;
						pDibPrev = pDib;
						ASSERT(pGIFColors == NULL);
						pGIFColors = (RGBQUAD*)new RGBQUAD[256];
						if (pGIFColors)
						{
							AnimatedGIFInit(&pGIFColors,
											nAnimGifLastFrameToSave,		// Sets this
											dDelayMul,						// Sets this
											dSpeedMul,						// Sets this
											dCalcFrameRate,
											bShowFrameTime,
											RefTime,
											dwRefUpTime);
						}
					}
					// Next Frame?
					else
					{
						SaveAnimatedGif(&GIFSaveDib,
										&pDib,
										&pDibPrev,
										sGIFTempFileName,
										&bFirstGIFSave,						// First Frame To Save?
										nFrames == nAnimGifLastFrameToSave,	// Last Frame To Save?
										dDelayMul,
										dSpeedMul,
										pGIFColors,
										MOVDET_ANIMGIF_DIFF_MINLEVEL,
										bShowFrameTime,
										RefTime,
										dwRefUpTime);
					}
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
		// this happens if m_nNumFramesToSave is 1 or
		// if nAnimGifLastFrameToSave has been set to
		// m_nNumFramesToSave by AnimatedGIFInit()
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
		if (!SendMailFTPUpload(FirstTime, sAVIFileName, sGIFFileName, sSWFFileName))
		{
			// Delete Files if not wanted
			if (!m_pDoc->m_bSaveAVIMovementDetection)
				::DeleteFile(sAVIFileName);
			if (!m_pDoc->m_bSaveAnimGIFMovementDetection)
				::DeleteFile(sGIFFileName);
			if (!m_pDoc->m_bSaveSWFMovementDetection)
				::DeleteFile(sSWFFileName);
			m_bWorking = FALSE;
			return 0;
		}
		dwMailFTPTimeMs = ::timeGetTime() - dwMailFTPTimeMs;

		// Execute Command
		if (m_pDoc->m_bExecCommandMovementDetection)
		{
			::EnterCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
			if (m_pDoc->m_bWaitExecCommandMovementDetection)
			{
				if (m_pDoc->m_hExecCommandMovementDetection)
				{
					if (::WaitForSingleObject(m_pDoc->m_hExecCommandMovementDetection, MOVDET_EXEC_COMMAND_WAIT_TIMEOUT)
																									== WAIT_OBJECT_0)
					{
						::CloseHandle(m_pDoc->m_hExecCommandMovementDetection);
						m_pDoc->m_hExecCommandMovementDetection = NULL;
					}
				}
			}
			else if (m_pDoc->m_hExecCommandMovementDetection)
			{
				::CloseHandle(m_pDoc->m_hExecCommandMovementDetection);
				m_pDoc->m_hExecCommandMovementDetection = NULL;
			}
			if (m_pDoc->m_sExecCommandMovementDetection != _T("") &&
				m_pDoc->m_hExecCommandMovementDetection == NULL)
			{
				SHELLEXECUTEINFO sei;
				memset(&sei, 0, sizeof(sei));
				sei.cbSize = sizeof(sei);
				sei.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
				sei.nShow = m_pDoc->m_bHideExecCommandMovementDetection ? SW_HIDE : SW_SHOWNORMAL;
				sei.lpFile = m_pDoc->m_sExecCommandMovementDetection;
				sei.lpParameters = m_pDoc->m_sExecParamsMovementDetection; 
				if (::ShellExecuteEx(&sei))
					m_pDoc->m_hExecCommandMovementDetection = sei.hProcess;
			}
			::LeaveCriticalSection(&m_pDoc->m_csExecCommandMovementDetection);
		}

		// Delete Files if not wanted
		if (!m_pDoc->m_bSaveAVIMovementDetection)
			::DeleteFile(sAVIFileName);
		if (!m_pDoc->m_bSaveAnimGIFMovementDetection)
			::DeleteFile(sGIFFileName);
		if (!m_pDoc->m_bSaveSWFMovementDetection)
			::DeleteFile(sSWFFileName);

		// Save time calculation
		DWORD dwSaveTimeMs = ::timeGetTime() - dwRefUpTime;
		DWORD dwFramesTimeMs = dwLastUpTime - dwFirstUpTime;
		if (dwFramesTimeMs >= 1000U) // Do the check only if at least 1 sec of frames
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
																const CString& sSWFFileName)
{
	// Send By E-Mail
	if (m_pDoc->m_bSendMailMovementDetection &&
		!SendMailMovementDetection(Time, sAVIFileName, sGIFFileName))
		return FALSE;

	// FTP Upload
	if (m_pDoc->m_bFTPUploadMovementDetection &&
		!FTPUploadMovementDetection(Time, sAVIFileName, sGIFFileName, sSWFFileName))
		return FALSE;

	return TRUE;
}

__forceinline BOOL CVideoDeviceDoc::CSaveFrameListThread::SendMailMovementDetection(	const CTime& Time,
																						const CString& sAVIFileName,
																						const CString& sGIFFileName)
{
	CString sSubject(Time.Format(_T("Movement Detection on %A, %d %B %Y at %H:%M:%S")));
	m_pDoc->m_MovDetSendMailConfiguration.m_sSubject = sSubject;
	int result;
	switch (m_pDoc->m_MovDetSendMailConfiguration.m_AttachmentType)
	{
		case CVideoDeviceDoc::ATTACHMENT_AVI :
				result = SendEmail(sAVIFileName, _T(""));
				break;

		case CVideoDeviceDoc::ATTACHMENT_ANIMGIF :
				result = SendEmail(_T(""), sGIFFileName);
				break;

		case CVideoDeviceDoc::ATTACHMENT_AVI_ANIMGIF :
				result = SendEmail(sAVIFileName, sGIFFileName);
				break;

		default :
				result = SendEmail(_T(""), _T(""));
				break;
	}

	// Do Exit?
	if (result == -1)
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

		case CVideoDeviceDoc::FILES_TO_UPLOAD_ANIMGIF :
				result = CVideoDeviceDoc::FTPUpload(&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
													sGIFFileName, sUploadDir + _T("/") + ::GetShortFileName(sGIFFileName));
				break;

		case CVideoDeviceDoc::FILES_TO_UPLOAD_SWF :
				result = CVideoDeviceDoc::FTPUpload(&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
													sSWFFileName, sUploadDir + _T("/") + ::GetShortFileName(sSWFFileName));
				break;

		case CVideoDeviceDoc::FILES_TO_UPLOAD_AVI_ANIMGIF :
				result = CVideoDeviceDoc::FTPUpload(&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
													sAVIFileName, sUploadDir + _T("/") + ::GetShortFileName(sAVIFileName));
				if (result == 1)
					result = CVideoDeviceDoc::FTPUpload(&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
														sGIFFileName, sUploadDir + _T("/") + ::GetShortFileName(sGIFFileName));
				break;

		case CVideoDeviceDoc::FILES_TO_UPLOAD_SWF_ANIMGIF :
				result = CVideoDeviceDoc::FTPUpload(&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
													sSWFFileName, sUploadDir + _T("/") + ::GetShortFileName(sSWFFileName));
				if (result == 1)
					result = CVideoDeviceDoc::FTPUpload(&FTP, &m_pDoc->m_MovDetFTPUploadConfiguration,
														sGIFFileName, sUploadDir + _T("/") + ::GetShortFileName(sGIFFileName));
				break;

		case CVideoDeviceDoc::FILES_TO_UPLOAD_AVI_SWF_ANIMGIF :
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

void CVideoDeviceDoc::CSaveFrameListThread::AnimatedGIFInit(	RGBQUAD** ppGIFColors,
																int& nAnimGifLastFrameToSave,
																double& dDelayMul,
																double& dSpeedMul,
																double dCalcFrameRate,
																BOOL bShowFrameTime,
																const CTime& RefTime,
																DWORD dwRefUpTime)
{
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

	// Get the frame 1 sec after first movement, get middle frame
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
				{
					nAnimGifLastFrameToSave = nFrameCountDown;
					pDibForPalette3 = p;
				}
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
	DibForPalette.AllocateBits(	DibForPalette1.GetBitCount(),
								DibForPalette1.GetCompression(),
								DibForPalette1.GetWidth(),
								3 * DibForPalette1.GetHeight());
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
	Quantizer.SetColorTable(*ppGIFColors);
	
	// VGA Palette
	// Note: palette Entry 255 is the Transparency Index!
	int i;
	for (i = 0; i < 8; i++)
	{
		(*ppGIFColors)[i+239].rgbRed		= CDib::ms_StdColors[i].rgbRed;
		(*ppGIFColors)[i+239].rgbGreen		= CDib::ms_StdColors[i].rgbGreen;
		(*ppGIFColors)[i+239].rgbBlue		= CDib::ms_StdColors[i].rgbBlue;
		(*ppGIFColors)[i+239].rgbReserved	= 0;
	}
	for (i = 8; i < 16; i++)
	{
		(*ppGIFColors)[i+239].rgbRed		= CDib::ms_StdColors[248+i].rgbRed;
		(*ppGIFColors)[i+239].rgbGreen		= CDib::ms_StdColors[248+i].rgbGreen;
		(*ppGIFColors)[i+239].rgbBlue		= CDib::ms_StdColors[248+i].rgbBlue;
		(*ppGIFColors)[i+239].rgbReserved	= 0;
	}
	
	// Limit the saved frames to around MOVDET_ANIMGIF_MAX_FRAMES and
	// the play length to around MOVDET_ANIMGIF_MAX_LENGTH ms.
	// Note: ie has problems with to many anim. gifs frames and also
	// with to many anim. gifs images per displayed page!
	int nTotFramesCount = m_nNumFramesToSave - nAnimGifLastFrameToSave + 1;
	double dLengthMs = (double)nTotFramesCount / dCalcFrameRate * 1000.0;
	dSpeedMul = max(1.0, dLengthMs / MOVDET_ANIMGIF_MAX_LENGTH);
	if (nTotFramesCount >= MOVDET_ANIMGIF_MAX_FRAMES)
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
				m_pAVRecSwf->AddVideoStream((LPBITMAPINFO)SWFSaveDib.GetBMI(),	// Source Video Format
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
					m_pAVRecThumbSwf->AddVideoStream((LPBITMAPINFO)SWFSaveDib.GetBMI(),	// Source Video Format
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
#if 0
			// To debug web client poll
			HANDLE hLockFile = ::CreateFile(sLiveFileName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			HANDLE hLockThumbFile = ::CreateFile(sLiveThumbFileName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			::Sleep(900);
			::CloseHandle(hLockFile);
			::CloseHandle(hLockThumbFile);
#endif
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
	CString sFirstFileName(_T(""));

	// Snapshot time
	CString sTime = m_Time.Format(_T("%Y_%m_%d_%H_%M_%S"));

	// Adjust Directory Name
	CString sSnapshotDir = m_sSnapshotAutoSaveDir;
	sSnapshotDir.TrimRight(_T('\\'));

	// Create directory if necessary
	if (sSnapshotDir != _T("") && m_bSnapshotHistoryJpeg)
	{
		DWORD dwAttrib =::GetFileAttributes(sSnapshotDir);
		if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
			::CreateDir(sSnapshotDir);
		if (!CVideoDeviceDoc::CreateCheckYearMonthDayDir(m_Time, sSnapshotDir, sFirstFileName))
			return _T("");
	}

	// Return file name
	if (sFirstFileName == _T(""))
		return _T("shot_") + sTime + _T(".jpg");
	else
		return sFirstFileName + _T("\\") + _T("shot_") + sTime + _T(".jpg");
}

__forceinline CString CVideoDeviceDoc::CSaveSnapshotThread::MakeSwfHistoryFileName()
{
	CString sFirstFileName(_T(""));

	// Snapshot time
	CString sTime = m_Time.Format(_T("%Y_%m_%d"));

	// Adjust Directory Name
	CString sSnapshotDir = m_sSnapshotAutoSaveDir;
	sSnapshotDir.TrimRight(_T('\\'));

	// Create directory if necessary
	if (sSnapshotDir != _T("") && m_bSnapshotHistorySwf)
	{
		DWORD dwAttrib =::GetFileAttributes(sSnapshotDir);
		if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
			::CreateDir(sSnapshotDir);
		if (!CVideoDeviceDoc::CreateCheckYearMonthDayDir(m_Time, sSnapshotDir, sFirstFileName))
			return _T("");
	}

	// Return file name
	if (sFirstFileName == _T(""))
		return _T("shot_") + sTime + _T(".swf");
	else
		return sFirstFileName + _T("\\") + _T("shot_") + sTime + _T(".swf");
}

CPJNSMTPMessage* CVideoDeviceDoc::CreateEmailMessage()
{
	// Create the message
	CPJNSMTPMessage* pMessage = new CPJNSMTPMessage;
	CPJNSMTPBodyPart attachment;

	// Set the mime flag
	pMessage->SetMime(m_MovDetSendMailConfiguration.m_bMime);

	// Set the charset of the message and all attachments
	pMessage->SetCharset(m_MovDetSendMailConfiguration.m_sEncodingCharset);
	attachment.SetCharset(m_MovDetSendMailConfiguration.m_sEncodingCharset);

	// Set the message priority
	pMessage->m_Priority = m_MovDetSendMailConfiguration.m_Priority;

	// Setup the all the recipient types for this message,
	// valid separators between addresses are ',' or ';'
	pMessage->AddMultipleRecipients(m_MovDetSendMailConfiguration.m_sTo, CPJNSMTPMessage::TO);
	if (!m_MovDetSendMailConfiguration.m_sCC.IsEmpty()) 
		pMessage->AddMultipleRecipients(m_MovDetSendMailConfiguration.m_sCC, CPJNSMTPMessage::CC);
	if (!m_MovDetSendMailConfiguration.m_sBCC.IsEmpty()) 
		pMessage->AddMultipleRecipients(m_MovDetSendMailConfiguration.m_sBCC, CPJNSMTPMessage::BCC);
	if (!m_MovDetSendMailConfiguration.m_sSubject.IsEmpty()) 
		pMessage->m_sSubject = m_MovDetSendMailConfiguration.m_sSubject;
	if (!m_MovDetSendMailConfiguration.m_sBody.IsEmpty())
	{
		if (m_MovDetSendMailConfiguration.m_bHTML)
			pMessage->AddHTMLBody(m_MovDetSendMailConfiguration.m_sBody, _T(""));
		else
			pMessage->AddTextBody(m_MovDetSendMailConfiguration.m_sBody);
	}

	// Add the attachment(s) if necessary,
	// valid separators between attachments are ',' or ';'
	if (!m_MovDetSendMailConfiguration.m_sFiles.IsEmpty()) 
		pMessage->AddMultipleAttachments(m_MovDetSendMailConfiguration.m_sFiles);		

	// Setup the from address
	if (m_MovDetSendMailConfiguration.m_sFromName.IsEmpty()) 
	{
		pMessage->m_From = m_MovDetSendMailConfiguration.m_sFrom;
		//pMessage->m_ReplyTo = m_MovDetSendMailConfiguration.m_sFrom; uncomment this if you want to send a Reply-To header
	}
	else 
	{
		CPJNSMTPAddress address(m_MovDetSendMailConfiguration.m_sFromName, m_MovDetSendMailConfiguration.m_sFrom);
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
int CVideoDeviceDoc::CSaveFrameListThread::SendEmail(CString sAVIFile, CString sGIFFile) 
{
	int res = 0;

	// Check size -> Return Error
	if (!sAVIFile.IsEmpty() && ::GetFileSize64(sAVIFile).QuadPart == 0)
		return 0;
	if (!sGIFFile.IsEmpty() && ::GetFileSize64(sGIFFile).QuadPart == 0)
		return 0;

	// No Configuration -> Return Error
	if (m_pDoc->m_MovDetSendMailConfiguration.m_sHost.IsEmpty()||
		m_pDoc->m_MovDetSendMailConfiguration.m_sFrom.IsEmpty() ||
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
				m_pDoc->m_MovDetSendMailConfiguration.m_sBody = _T("Movement Detection!");

				// Attachment(s)
				if (!sAVIFile.IsEmpty())
				{
					m_pDoc->m_MovDetSendMailConfiguration.m_sFiles = sAVIFile;
					if (!sGIFFile.IsEmpty())
						m_pDoc->m_MovDetSendMailConfiguration.m_sFiles = m_pDoc->m_MovDetSendMailConfiguration.m_sFiles + _T(";") + sGIFFile;
				}
				else if (!sGIFFile.IsEmpty())
					m_pDoc->m_MovDetSendMailConfiguration.m_sFiles = sGIFFile;
				else
					m_pDoc->m_MovDetSendMailConfiguration.m_sFiles = _T("");

				// Create the message
				pMessage = m_pDoc->CreateEmailMessage();
			}
			else
			{
				m_pDoc->m_MovDetSendMailConfiguration.m_bMime = TRUE;
				m_pDoc->m_MovDetSendMailConfiguration.m_sFiles = _T("");
				m_pDoc->m_MovDetSendMailConfiguration.m_sBody = _T("");

				// Create the message
				pMessage = m_pDoc->CreateEmailMessage();

				for (int i = 0 ; i < pMessage->GetNumberOfBodyParts() ; i++)
					pMessage->RemoveBodyPart(i);

				// Setup all the body parts we want
				CPJNSMTPBodyPart related;
				related.SetContentType(_T("multipart/related"));

				CPJNSMTPBodyPart html;
				if (!sAVIFile.IsEmpty() && !sGIFFile.IsEmpty())
					html.SetText(_T("<p><b>Movement Detection:</b></p><a HREF=\"cid:AVI\"><IMG SRC=\"cid:ANIMGIF\" ALT=\"Animated Gif Detection Image\"></a>"));
				else if (!sAVIFile.IsEmpty())
					html.SetText(_T("<p><b>Movement Detection:</b></p><a HREF=\"cid:AVI\">AVI Video File</a>"));
				else if (!sGIFFile.IsEmpty())
					html.SetText(_T("<p><b>Movement Detection:</b></p><IMG SRC=\"cid:ANIMGIF\" ALT=\"Animated Gif Detection Image\">"));
				else
					html.SetText(_T("<p><b>Movement Detection!</b></p>"));
				html.SetContentType(_T("text/html"));

				CPJNSMTPBodyPart giffile;
				if (!sGIFFile.IsEmpty())
				{
					giffile.SetFilename(sGIFFile);
					giffile.SetContentID(_T("ANIMGIF"));
					giffile.SetContentType(_T("image/gif"));
				}

				CPJNSMTPBodyPart avifile;
				if (!sAVIFile.IsEmpty())
				{
					avifile.SetFilename(sAVIFile);
					avifile.SetContentID(_T("AVI"));
					avifile.SetContentType(_T("video/avi"));
				}

				related.AddChildBodyPart(html);
				if (!sGIFFile.IsEmpty())
					related.AddChildBodyPart(giffile);
				if (!sAVIFile.IsEmpty())
					related.AddChildBodyPart(avifile);
				pMessage->AddBodyPart(related);
				pMessage->GetBodyPart(0)->SetContentLocation(_T("http://localhost"));
			}

			// Init Connection class
			CVideoDeviceDoc::CSaveFrameListSMTPConnection connection(this);

			// Auto connect to the internet?
			if (m_pDoc->m_MovDetSendMailConfiguration.m_bAutoDial)
				connection.ConnectToInternet();

			CString sHost;
			BOOL bSend = TRUE;
			if (m_pDoc->m_MovDetSendMailConfiguration.m_bDNSLookup)
			{
				if (pMessage->GetNumberOfRecipients() == 0)
				{
					CString sMsg;
					sMsg.Format(_T("%s, at least one recipient must be specified to use the DNS lookup option\n"), m_pDoc->GetDeviceName());
					TRACE(sMsg);
					::LogLine(sMsg);
					bSend = FALSE;
				}
				else
				{
					CString sAddress = pMessage->GetRecipient(0)->m_sEmailAddress;
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
				connection.SetBoundAddress(m_pDoc->m_MovDetSendMailConfiguration.m_sBoundIP);
				connection.Connect(	sHost,
									m_pDoc->m_MovDetSendMailConfiguration.m_Auth,
									m_pDoc->m_MovDetSendMailConfiguration.m_sUsername,
									m_pDoc->m_MovDetSendMailConfiguration.m_sPassword,
									m_pDoc->m_MovDetSendMailConfiguration.m_nPort);
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
				res = -1;
			}
			else
				res = 1;

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
				if (m_pDoc->m_bRecSizeSegmentation)
				{
					LONGLONG llTotalWrittenBytes =	
					m_pDoc->m_pAVRec->GetTotalWrittenBytes(m_pDoc->m_pAVRec->VideoStreamNumToStreamNum(ACTIVE_VIDEO_STREAM)) +
					m_pDoc->m_pAVRec->GetTotalWrittenBytes(m_pDoc->m_pAVRec->AudioStreamNumToStreamNum(ACTIVE_AUDIO_STREAM));
					llTotalWrittenBytes += (llTotalWrittenBytes / 4); // Guess a 25% of AVI Overhead (Index, AVI Hdrs and Data Chunk Hdrs)
					if (llTotalWrittenBytes > m_pDoc->m_llRecFileSize)
					{
						// Note: NextAviFile() always adds the samples to
						// the old file, even if returning FALSE!
						res = NextAviFile(dwSize, pBuf);
						bNextAviFileCalled = TRUE;
					}
				}
				else if (m_pDoc->m_bRecTimeSegmentation)
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
	CString sFirstRecFileName;
	int nRecordedFrames = 0;
	CAVRec* pNextAVRec = NULL;
	CAVRec* pOldAVRec = NULL;
	BOOL bFreeOldAVRec = FALSE;
	if (m_pDoc->m_bRecSizeSegmentation)
	{
		// Set Old AVRec Pointer
		pOldAVRec = m_pDoc->m_pAVRec;

		// Get Next AVI File
		if ((m_pDoc->m_nRecFilePos + 1) < m_pDoc->m_AVRecs.GetSize())
		{
			// Get Next AVRec Pointer
			pNextAVRec = m_pDoc->m_AVRecs[++(m_pDoc->m_nRecFilePos)];

			// Change Pointer
			::EnterCriticalSection(&m_pDoc->m_csAVRec);
			m_pDoc->m_pAVRec = pNextAVRec;
			::LeaveCriticalSection(&m_pDoc->m_csAVRec);

			// Set Ok
			res = TRUE;
		}
	}
	// m_bRecTimeSegmentation
	else
	{
		// Set Old AVRec Pointer
		pOldAVRec = m_pDoc->m_pAVRec;

		// Make First Rec. File Name
		sFirstRecFileName = m_pDoc->MakeRecFileName();
		
		// Allocate & Init pNextAVRec
		if (sFirstRecFileName != _T(""))
		{
			if (m_pDoc->MakeAVRec(sFirstRecFileName, &pNextAVRec))
			{
				// Change Pointer and
				// restart with frame counting and time measuring
				::EnterCriticalSection(&m_pDoc->m_csAVRec);
				m_pDoc->m_pAVRec = pNextAVRec;
				if (m_pDoc->m_AVRecs.GetSize() > 0)
					m_pDoc->m_AVRecs[0] = pNextAVRec;
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
		}
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
			m_pDoc->ChangeRecFileFrameRate(dFrameRate);

			// Open the video file and ev. post process it
			m_pDoc->OpenAndPostProcess();

			// Set New File Name
			m_pDoc->m_sFirstRecFileName = sFirstRecFileName;
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

void CVideoDeviceDoc::ColorDetectionProcessing(CDib* pDib, BOOL bColorDetectionPreview)
{
	if (m_ColorDetection.Detector(pDib, m_dwColorDetectionAccuracy, bColorDetectionPreview))
	{
		if (m_pColorDetectionPage)
		{
			CString Str;
			CEdit* pEdit;
			switch (m_ColorDetection.GetColorsCount())
			{
				case 8 :
					m_pColorDetectionPage->m_ProgressColor7.SetPos(m_ColorDetection.GetDetectionLevel(7));
					
					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_COUNTUP7);
					Str.Format(_T("%u"), m_ColorDetection.GetDetectionCountup(7));
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_LASTTIME7);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetTimeBetweenCounts(7)/1000,
												m_ColorDetection.GetTimeBetweenCounts(7)%1000);
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_SHORTESTTIME7);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetShortestTimeBetweenCounts(7)/1000,
												m_ColorDetection.GetShortestTimeBetweenCounts(7)%1000);
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_LONGESTTIME7);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetLongestTimeBetweenCounts(7)/1000,
												m_ColorDetection.GetLongestTimeBetweenCounts(7)%1000);
					pEdit->SetWindowText(Str);
				case 7 :
					m_pColorDetectionPage->m_ProgressColor6.SetPos(m_ColorDetection.GetDetectionLevel(6));
					
					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_COUNTUP6);
					Str.Format(_T("%u"), m_ColorDetection.GetDetectionCountup(6));
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_LASTTIME6);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetTimeBetweenCounts(6)/1000,
												m_ColorDetection.GetTimeBetweenCounts(6)%1000);
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_SHORTESTTIME6);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetShortestTimeBetweenCounts(6)/1000,
												m_ColorDetection.GetShortestTimeBetweenCounts(6)%1000);
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_LONGESTTIME6);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetLongestTimeBetweenCounts(6)/1000,
												m_ColorDetection.GetLongestTimeBetweenCounts(6)%1000);
					pEdit->SetWindowText(Str);
				case 6 :
					m_pColorDetectionPage->m_ProgressColor5.SetPos(m_ColorDetection.GetDetectionLevel(5));
					
					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_COUNTUP5);
					Str.Format(_T("%u"), m_ColorDetection.GetDetectionCountup(5));
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_LASTTIME5);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetTimeBetweenCounts(5)/1000,
												m_ColorDetection.GetTimeBetweenCounts(5)%1000);
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_SHORTESTTIME5);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetShortestTimeBetweenCounts(5)/1000,
												m_ColorDetection.GetShortestTimeBetweenCounts(5)%1000);
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_LONGESTTIME5);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetLongestTimeBetweenCounts(5)/1000,
												m_ColorDetection.GetLongestTimeBetweenCounts(5)%1000);
					pEdit->SetWindowText(Str);
				case 5 :
					m_pColorDetectionPage->m_ProgressColor4.SetPos(m_ColorDetection.GetDetectionLevel(4));
					
					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_COUNTUP4);
					Str.Format(_T("%u"), m_ColorDetection.GetDetectionCountup(4));
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_LASTTIME4);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetTimeBetweenCounts(4)/1000,
												m_ColorDetection.GetTimeBetweenCounts(4)%1000);
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_SHORTESTTIME4);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetShortestTimeBetweenCounts(4)/1000,
												m_ColorDetection.GetShortestTimeBetweenCounts(4)%1000);
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_LONGESTTIME4);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetLongestTimeBetweenCounts(4)/1000,
												m_ColorDetection.GetLongestTimeBetweenCounts(4)%1000);
					pEdit->SetWindowText(Str);
				case 4 :
					m_pColorDetectionPage->m_ProgressColor3.SetPos(m_ColorDetection.GetDetectionLevel(3));
					
					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_COUNTUP3);
					Str.Format(_T("%u"), m_ColorDetection.GetDetectionCountup(3));
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_LASTTIME3);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetTimeBetweenCounts(3)/1000,
												m_ColorDetection.GetTimeBetweenCounts(3)%1000);
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_SHORTESTTIME3);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetShortestTimeBetweenCounts(3)/1000,
												m_ColorDetection.GetShortestTimeBetweenCounts(3)%1000);
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_LONGESTTIME3);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetLongestTimeBetweenCounts(3)/1000,
												m_ColorDetection.GetLongestTimeBetweenCounts(3)%1000);
					pEdit->SetWindowText(Str);
				case 3 :
					m_pColorDetectionPage->m_ProgressColor2.SetPos(m_ColorDetection.GetDetectionLevel(2));
					
					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_COUNTUP2);
					Str.Format(_T("%u"), m_ColorDetection.GetDetectionCountup(2));
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_LASTTIME2);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetTimeBetweenCounts(2)/1000,
												m_ColorDetection.GetTimeBetweenCounts(2)%1000);
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_SHORTESTTIME2);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetShortestTimeBetweenCounts(2)/1000,
												m_ColorDetection.GetShortestTimeBetweenCounts(2)%1000);
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_LONGESTTIME2);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetLongestTimeBetweenCounts(2)/1000,
												m_ColorDetection.GetLongestTimeBetweenCounts(2)%1000);
					pEdit->SetWindowText(Str);
				case 2 :
					m_pColorDetectionPage->m_ProgressColor1.SetPos(m_ColorDetection.GetDetectionLevel(1));
					
					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_COUNTUP1);
					Str.Format(_T("%u"), m_ColorDetection.GetDetectionCountup(1));
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_LASTTIME1);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetTimeBetweenCounts(1)/1000,
												m_ColorDetection.GetTimeBetweenCounts(1)%1000);
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_SHORTESTTIME1);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetShortestTimeBetweenCounts(1)/1000,
												m_ColorDetection.GetShortestTimeBetweenCounts(1)%1000);
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_LONGESTTIME1);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetLongestTimeBetweenCounts(1)/1000,
												m_ColorDetection.GetLongestTimeBetweenCounts(1)%1000);
					pEdit->SetWindowText(Str);
				case 1 :
					m_pColorDetectionPage->m_ProgressColor0.SetPos(m_ColorDetection.GetDetectionLevel(0));
					
					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_COUNTUP0);
					Str.Format(_T("%u"), m_ColorDetection.GetDetectionCountup(0));
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_LASTTIME0);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetTimeBetweenCounts(0)/1000,
												m_ColorDetection.GetTimeBetweenCounts(0)%1000);
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_SHORTESTTIME0);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetShortestTimeBetweenCounts(0)/1000,
												m_ColorDetection.GetShortestTimeBetweenCounts(0)%1000);
					pEdit->SetWindowText(Str);

					pEdit = (CEdit*)m_pColorDetectionPage->GetDlgItem(IDC_EDIT_LONGESTTIME0);
					Str.Format(_T("%u\" %ums"),	m_ColorDetection.GetLongestTimeBetweenCounts(0)/1000,
												m_ColorDetection.GetLongestTimeBetweenCounts(0)%1000);
					pEdit->SetWindowText(Str);

				case 0 :
				default :
					break;
			}
			switch (m_ColorDetection.GetColorsCount())
			{
				case 0 :
					m_pColorDetectionPage->m_ProgressColor0.SetPos(0);
				case 1 :
					m_pColorDetectionPage->m_ProgressColor1.SetPos(0);
				case 2 :
					m_pColorDetectionPage->m_ProgressColor2.SetPos(0);
				case 3 :
					m_pColorDetectionPage->m_ProgressColor3.SetPos(0);
				case 4 :
					m_pColorDetectionPage->m_ProgressColor4.SetPos(0);
				case 5 :
					m_pColorDetectionPage->m_ProgressColor5.SetPos(0);
				case 6 :
					m_pColorDetectionPage->m_ProgressColor6.SetPos(0);
				case 7 :
					m_pColorDetectionPage->m_ProgressColor7.SetPos(0);
				case 8 :
				default :
					break;
			}
		}
	}
}

void CVideoDeviceDoc::MovementDetectionProcessing(	CDib* pDib,
													BOOL bMovementDetectorPreview,
													BOOL bDoDetection)
{
	BOOL bMovement = FALSE;
	BOOL bLumChange = FALSE;

	// Init
	if (m_lMovDetTotalZones == 0)
	{
		if (::SendMessage(	GetView()->GetSafeHwnd(),
							WM_THREADSAFE_INIT_MOVDET,
							0, 0) == 0)
			return; // Cannot init, unsupported resolution
	}

	// Detection Enabled?
	if (bDoDetection)
	{
		if (pDib->GetCompression() == BI_RGB ||
			pDib->GetCompression() == BI_BITFIELDS)
		{
			// If first RGB Frame:
			// 1. Allocated m_pMovementDetectorY800Dib
			// 2. Free m_pDifferencingDib and m_pMovementDetectorBackgndDib
			if (!m_pMovementDetectorY800Dib)
			{
				m_pMovementDetectorY800Dib = new CDib;
				if (!m_pMovementDetectorY800Dib)
					return;
				m_pMovementDetectorY800Dib->SetShowMessageBoxOnError(FALSE);
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
			}
			
			// 24 bpp
			if (pDib->GetBitCount() == 24)
			{
				if (!m_pMovementDetectorY800Dib->GetBits())
				{
					if (!m_pMovementDetectorY800Dib->AllocateBitsFast(	8,
																		FCC('Y800'),
																		pDib->GetWidth(),
																		pDib->GetHeight()))
						return;
				}
				if (!::RGB24ToY800(	pDib->GetBits(),						// RGB24 Dib
									m_pMovementDetectorY800Dib->GetBits(),	// Y Plane
									pDib->GetWidth(),
									pDib->GetHeight()))
					return;
			}
			// 32 bpp
			else if (pDib->GetBitCount() == 32)
			{
				if (!m_pMovementDetectorY800Dib->GetBits())
				{
					if (!m_pMovementDetectorY800Dib->AllocateBitsFast(	8,
																		FCC('Y800'),
																		pDib->GetWidth(),
																		pDib->GetHeight()))
						return;
				}
				if (!::RGB32ToY800(	pDib->GetBits(),						// RGB32 Dib
									m_pMovementDetectorY800Dib->GetBits(),	// Y Plane
									pDib->GetWidth(),
									pDib->GetHeight()))
					return;
			}
			// 16 bpp
			else
			{
				*m_pMovementDetectorY800Dib = *pDib;
				if (!m_pMovementDetectorY800Dib->Compress(FCC('Y800')))
					return;
			}
		}
		// If first no RGB Frame:
		// Free m_pMovementDetectorY800Dib,
		//		m_pDifferencingDib and m_pMovementDetectorBackgndDib
		else if (m_pMovementDetectorY800Dib)
		{
			delete m_pMovementDetectorY800Dib;
			m_pMovementDetectorY800Dib = NULL;
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
		}
		CDib* pDibY = m_pMovementDetectorY800Dib ? m_pMovementDetectorY800Dib : pDib;
		if (!m_pMovementDetectorBackgndDib)
		{
			m_pMovementDetectorBackgndDib = new CDib(*pDibY);
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
			if (!m_pDifferencingDib->AllocateBitsFast(	pDibY->GetBitCount(),
														pDibY->GetCompression(),
														pDibY->GetWidth(),
														pDibY->GetHeight()))
				return;
		}

		// Color Space Type
		BOOL bPlanar;
		int nPackedYOffset = 0;
		if (pDibY->GetCompression() == FCC('I420')	||
			pDibY->GetCompression() == FCC('IYUV')	||
			pDibY->GetCompression() == FCC('YV12')	||
			pDibY->GetCompression() == FCC('YUV9')	||
			pDibY->GetCompression() == FCC('YVU9')	||
			pDibY->GetCompression() == FCC('Y41B')	||
			pDibY->GetCompression() == FCC('YV16')	||
			pDibY->GetCompression() == FCC('Y42B')	||
			pDibY->GetCompression() == FCC('  Y8')	||
			pDibY->GetCompression() == FCC('Y800')	||
			pDibY->GetCompression() == FCC('GREY'))
		{
			bPlanar = TRUE;
		}
		// Packed 422 with Y beginning the 16 bits pixel
		else if (	pDibY->GetCompression() == FCC('YUY2')	||	// Y0 U0 Y1 V0, Y2 U2 Y3 V2, ...
					pDibY->GetCompression() == FCC('YUNV')	||	// Equivalent to YUY2
					pDibY->GetCompression() == FCC('VYUY')	||	// Equivalent to YUY2
					pDibY->GetCompression() == FCC('V422')	||	// Equivalent to YUY2
					pDibY->GetCompression() == FCC('YUYV')	||	// Equivalent to YUY2
					pDibY->GetCompression() == FCC('YVYU'))		// Y0 V0 Y1 U0, Y2 V2 Y3 U2, ...
		{
			bPlanar = FALSE;
		}
		// Packed 422 with Y as the second byte of the 16 bits pixel
		else if (	pDibY->GetCompression() == FCC('UYVY')	||	// U0 Y0 V0 Y1, U2 Y2 V2 Y3, ...
					pDibY->GetCompression() == FCC('Y422')	||	// Equivalent to UYVY
					pDibY->GetCompression() == FCC('UYNV'))		// Equivalent to UYVY
		{
			bPlanar = FALSE;
			nPackedYOffset = 1;
		}
		// Not Supported Format!
		else
		{
			TRACE(_T("Video Format Not Supported by Motion Detector!\n"));
			return;
		}

		// Luminosity change detector
		bLumChange = LumChangeDetector(pDibY, bPlanar, nPackedYOffset);

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
		int nSize8;
		if (bPlanar)
			nSize8 = (pDibY->GetWidth() * pDibY->GetHeight()) >> 3;
		else
			nSize8 = (pDibY->GetWidth() * pDibY->GetHeight()) >> 2;

		// Do Differencing
		::DiffMMX(	m_pDifferencingDib->GetBits(),				// Dst
					pDibY->GetBits(),							// Src1
					m_pMovementDetectorBackgndDib->GetBits(),	// Src2
					nSize8,										// Size in 8 bytes units
					MinDiff);

		// Call Detector
		bMovement = MovementDetector(	m_pDifferencingDib,
										bPlanar,
										m_nDetectionLevel);

		// Update background
		int nFrameRate = Round(m_dEffectiveFrameRate);
		if (g_bSSE)
		{
			if (nFrameRate >= 10)
			{
				::Mix15To1MMX(	m_pMovementDetectorBackgndDib->GetBits(),	// Src1 & Dst
								pDibY->GetBits(),							// Src2
								nSize8);									// Size in 8 bytes units
			}
			else if (nFrameRate >= 5)
			{
				::Mix7To1MMX(	m_pMovementDetectorBackgndDib->GetBits(),	// Src1 & Dst
								pDibY->GetBits(),							// Src2
								nSize8);									// Size in 8 bytes units
			}
			else
			{
				::Mix3To1MMX(	m_pMovementDetectorBackgndDib->GetBits(),	// Src1 & Dst
								pDibY->GetBits(),							// Src2
								nSize8);									// Size in 8 bytes units
			}
		}
		else
		{
			int nSize;
			if (bPlanar)
				nSize = pDibY->GetWidth() * pDibY->GetHeight();
			else
				nSize = (pDibY->GetWidth() * pDibY->GetHeight()) << 1;
			LPBYTE p1 = m_pMovementDetectorBackgndDib->GetBits();
			LPBYTE p2 = pDibY->GetBits();
			if (nFrameRate >= 10)
			{
				for (int i = 0 ; i < nSize ; i++)
					p1[i] = (BYTE)((15 * (int)(p1[i]) + (int)(p2[i]) + 8)>>4);
			}
			else if (nFrameRate >= 5)
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
		
		// If Preview copy m_pDifferencingDib to pDib
		// and reset (set to 128) the U and V channels
		// to have a nice looking grayscale preview 
		if (bMovementDetectorPreview)
			MovementDetectorPreview(pDib);
	}
	else
	{
		for (int i = 0 ; i < m_lMovDetTotalZones ; i++)
			m_MovementDetections[i] = FALSE;
	}

	// Frame Time
	int nFrameTime = Round(1000.0 / m_dFrameRate);
	if (m_dEffectiveFrameRate > 0.0)
		nFrameTime = Round(1000.0 / m_dEffectiveFrameRate);

	// Store frames?
	BOOL bStoreFrames =	bDoDetection					&&
						(m_bSaveSWFMovementDetection	||
						m_bSaveAVIMovementDetection		||
						m_bSaveAnimGIFMovementDetection	||
						m_bSendMailMovementDetection	||
						m_bFTPUploadMovementDetection	||
						m_bExecCommandMovementDetection);

	// If Movement and no Luminosity change
	if (bMovement && !bLumChange)
	{
		// Mark the Frame as a Cause of Movement
		pDib->SetUserFlag(TRUE);

		// Reset var
		m_nMilliSecondsWithoutMovementDetection = 0;

		// First detected frame?
		if (!m_bDetectingMovement)
		{
			m_nMilliSecondsBeforeMovementDetection = 0;
			m_bDetectingMovement = TRUE;
		}
	}
	// If No Movement
	else
	{
		// Mark the Frame as no Movement Cause
		pDib->SetUserFlag(FALSE);

		// If detecting countup without movement var
		if (m_bDetectingMovement)
			m_nMilliSecondsWithoutMovementDetection += nFrameTime;
		else
		{
			m_nMilliSecondsBeforeMovementDetection += nFrameTime;
			if (bStoreFrames)
				AddNewFrameToNewestList(pDib);
			if (m_nMilliSecondsBeforeMovementDetection > m_nMilliSecondsRecBeforeMovementBegin)
				RemoveOldestFrameFromNewestList();
		}
	}

	// If in detection state
	if (m_bDetectingMovement)
	{
		// Countup the detection time and add frame
		m_nMilliSecondsSinceMovementDetection += nFrameTime;
		if (bStoreFrames)
			AddNewFrameToNewestList(pDib);

		// Check if end of detecting period
		if (m_nMilliSecondsWithoutMovementDetection > m_nMilliSecondsRecAfterMovementEnd)
		{
			// False Detection
			BOOL bFalseDetection = FALSE;
			if (m_bDoFalseDetectionCheck)
			{
				if (m_bDoFalseDetectionAnd)
				{
					if (m_nBlueMovementDetectionsCount > m_nFalseDetectionBlueThreshold	&&
						m_nNoneBlueMovementDetectionsCount > m_nFalseDetectionNoneBlueThreshold)
						bFalseDetection = TRUE;
				}
				else
				{
					if (m_nBlueMovementDetectionsCount > m_nFalseDetectionBlueThreshold	||
						m_nNoneBlueMovementDetectionsCount > m_nFalseDetectionNoneBlueThreshold)
						bFalseDetection = TRUE;
				}
			}

			// Reset vars
			m_bDetectingMovement = FALSE;
			m_nMilliSecondsSinceMovementDetection = 0;
			m_nMilliSecondsWithoutMovementDetection = 0;

			// Save frames
			if (bFalseDetection)
			{
				ClearNewestFrameList();
				if (bStoreFrames)
				{
					CString sMsg;
					sMsg.Format(_T("%s, false detection: Blue %d , None Blue %d\n"),
								GetDeviceName(), m_nBlueMovementDetectionsCount, m_nNoneBlueMovementDetectionsCount);
					TRACE(sMsg);
					::LogLine(sMsg);
				}
			}
			SaveFrameList();

			// Reset false detection counts
			m_nBlueMovementDetectionsCount = 0;
			m_nNoneBlueMovementDetectionsCount = 0;
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
		m_nBlueMovementDetectionsCount = 0;
		m_nNoneBlueMovementDetectionsCount = 0;
	}
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
		DWORD dwFirstTimeDifference = dwRefUpTime - dwFirstUpTime;
		CTimeSpan FirstTimeSpan((time_t)(dwFirstTimeDifference > 0U ? Round((double)dwFirstTimeDifference / 1000.0) : 0));
		CTime FirstTime = RefTime - FirstTimeSpan;
		
		// Last Frame Time
		DWORD dwLastTimeDifference = dwRefUpTime - dwLastUpTime;
		CTimeSpan LastTimeSpan((time_t)(dwLastTimeDifference > 0U ? Round((double)dwLastTimeDifference / 1000.0) : 0));
		CTime LastTime = RefTime - LastTimeSpan;
		
		// Check Whether Detection Dir Exists
		CString sDetectionAutoSaveDir = m_sDetectionAutoSaveDir;
		DWORD dwAttrib =::GetFileAttributes(sDetectionAutoSaveDir);
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

__forceinline void CVideoDeviceDoc::MovementDetectorPreview(CDib* pDib)
{
	// Not necessay, just that it's gray and has no colors:
	if (m_pDifferencingDib->GetCompression() == FCC('I420')	||			// YUV420P
		m_pDifferencingDib->GetCompression() == FCC('IYUV')	||
		m_pDifferencingDib->GetCompression() == FCC('YV12'))
	{
		memset(	m_pDifferencingDib->GetBits() + (m_pDifferencingDib->GetWidth() * m_pDifferencingDib->GetHeight()),
				128,
				(m_pDifferencingDib->GetWidth() * m_pDifferencingDib->GetHeight()) >> 1);
	}
	else if (	m_pDifferencingDib->GetCompression() == FCC('YUV9')	||	// YUV410P
				m_pDifferencingDib->GetCompression() == FCC('YVU9'))
	{
		memset(	m_pDifferencingDib->GetBits() + (m_pDifferencingDib->GetWidth() * m_pDifferencingDib->GetHeight()),
				128,
				(m_pDifferencingDib->GetWidth() * m_pDifferencingDib->GetHeight()) >> 3);
	}
	else if (	m_pDifferencingDib->GetCompression() == FCC('Y41B'))	// YUV411P
	{
		memset(	m_pDifferencingDib->GetBits() + (m_pDifferencingDib->GetWidth() * m_pDifferencingDib->GetHeight()),
				128,
				(m_pDifferencingDib->GetWidth() * m_pDifferencingDib->GetHeight()) >> 1);
	}
	else if (	m_pDifferencingDib->GetCompression() == FCC('YV16')	||	// YUV422P
				m_pDifferencingDib->GetCompression() == FCC('Y42B'))
	{
		memset(	m_pDifferencingDib->GetBits() + (m_pDifferencingDib->GetWidth() * m_pDifferencingDib->GetHeight()),
				128,
				m_pDifferencingDib->GetWidth() * m_pDifferencingDib->GetHeight());
	}
	else if (	m_pDifferencingDib->GetCompression() == FCC('YUY2')	||	// Packed 422: YUYV or YVYU
				m_pDifferencingDib->GetCompression() == FCC('YUNV')	||
				m_pDifferencingDib->GetCompression() == FCC('VYUY')	||
				m_pDifferencingDib->GetCompression() == FCC('V422')	||
				m_pDifferencingDib->GetCompression() == FCC('YUYV')	||
				m_pDifferencingDib->GetCompression() == FCC('YVYU'))
	{
		int size = (m_pDifferencingDib->GetWidth() * m_pDifferencingDib->GetHeight()) << 1;
		LPBYTE bits = m_pDifferencingDib->GetBits() + 1;
		for (int i = 0 ; i < size ; i += 2)
			bits[i] = 128;
	}
	else if (	m_pDifferencingDib->GetCompression() == FCC('UYVY')	||	// Packed 422: UYVY
				m_pDifferencingDib->GetCompression() == FCC('Y422')	||
				m_pDifferencingDib->GetCompression() == FCC('UYNV'))
	{
		int size = (m_pDifferencingDib->GetWidth() * m_pDifferencingDib->GetHeight()) << 1;
		LPBYTE bits = m_pDifferencingDib->GetBits();
		for (int i = 0 ; i < size ; i += 2)
			bits[i] = 128;
	}
	int nBitCount = pDib->GetBitCount();
	BOOL bRGB565 = pDib->IsRgb16_565();
	int nCompression = pDib->GetCompression();
	DWORD dwUpTime = pDib->GetUpTime();
	*pDib = *m_pDifferencingDib;
	if (nCompression == BI_RGB)	
		pDib->Decompress(nBitCount);
	else if (nCompression == BI_BITFIELDS)
	{
		if (bRGB565)
		{
			pDib->Decompress(32);
			pDib->ConvertTo16bitsMasks();
		}
		else
			pDib->Decompress(nBitCount);
	}
	pDib->SetUpTime(dwUpTime);
}

// Remember that this callback is called from a separate thread !!!
LRESULT CALLBACK CVideoDeviceDoc::CVfWCaptureVideoThread::OnCaptureVideo(HWND hWnd, LPVIDEOHDR lpVHdr)
{
	CVideoDeviceDoc::CVfWCaptureVideoThread* pCaptureThread = (CVideoDeviceDoc::CVfWCaptureVideoThread*)capGetUserData(hWnd);
	if (pCaptureThread && pCaptureThread->m_pDoc)
	{
		CVideoDeviceDoc* pDoc = pCaptureThread->m_pDoc;

		// Check whether data is valid
		if (lpVHdr->lpData && lpVHdr->dwBytesUsed != 0)
			pDoc->m_bVfWVideoFormatApplyPressed = FALSE;

		return (LRESULT)pDoc->ProcessFrame(lpVHdr->lpData, lpVHdr->dwBytesUsed);
	}
	else
		return FALSE;
}

// Remember that this callback is called from a separate thread !!!
LRESULT CALLBACK CVideoDeviceDoc::CVfWCaptureVideoThread::OnFrame(HWND hWnd, LPVIDEOHDR lpVHdr)
{
	CVideoDeviceDoc::CVfWCaptureVideoThread* pCaptureThread = (CVideoDeviceDoc::CVfWCaptureVideoThread*)capGetUserData(hWnd);
	if (pCaptureThread && pCaptureThread->m_pDoc)
	{
		CVideoDeviceDoc* pDoc = pCaptureThread->m_pDoc;

		// Some Changes Happened (Like Format Change)
		if (!lpVHdr->lpData || lpVHdr->dwBytesUsed == 0)
		{
			// Display Warning Message
			pDoc->m_bVfWVideoFormatApplyPressed = TRUE;

			// Empty Process Frame to make the watchdog happy
			pDoc->ProcessFrame(NULL, 0);

			// Draw
			HRESULT hr = ::CoInitialize(NULL);
			BOOL bCleanupCOM = ((hr == S_OK) || (hr == S_FALSE));
			pDoc->GetView()->Draw();
			if (bCleanupCOM)
				::CoUninitialize();
		}
		else
		{
			pDoc->m_bVfWVideoFormatApplyPressed = FALSE;
			pDoc->ProcessFrame(lpVHdr->lpData, lpVHdr->dwBytesUsed);
		}

		// Triggered Capture Terminates here ->
		// The Thread can Trigger Another Frame Capture
		pCaptureThread->m_bTriggeredCapture = FALSE;

		return (LRESULT)TRUE;
	}
	else	
		return (LRESULT)FALSE;
}

CVideoDeviceDoc::CVfWCaptureVideoThread::CVfWCaptureVideoThread()
{ 
	m_pDoc = NULL;
	m_hCapWnd = NULL;
	m_bTriggeredCapture = FALSE;
}

CVideoDeviceDoc::CVfWCaptureVideoThread::~CVfWCaptureVideoThread() 
{
	Kill();
	Disconnect();
	DestroyCaptureWnd();
}

int CVideoDeviceDoc::CVfWCaptureVideoThread::Work() 
{
	DWORD Event;

	if (!m_pDoc)
		return 0;

	for (;;)
	{		
		m_pDoc->m_bCapture = TRUE;
		int nFrameTime = VFW_MIN_TRIGGEREDCAP_FRAMETIME;
		double dFrameRate = m_pDoc->m_dEffectiveFrameRate;
		if (dFrameRate >= 0.5 && dFrameRate <= (1000.0 / (double)VFW_MIN_TRIGGEREDCAP_FRAMETIME))
			nFrameTime = Round(1000.0 / dFrameRate);
		Event = ::WaitForSingleObject(GetKillEvent(), nFrameTime);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :	m_pDoc->m_bCapture = FALSE;
									return 0;

			// Grab Frame
			case WAIT_TIMEOUT :		if (!m_bTriggeredCapture)
									{
										if (::IsWindow(m_hCapWnd))
										{
											m_bTriggeredCapture = TRUE;
											::PostMessage(m_hCapWnd, WM_CAP_GRAB_FRAME_NOSTOP, 0, 0);
										}
									}
									break;

			default: break;
		}
	}
}

BOOL CVideoDeviceDoc::CVfWCaptureVideoThread::CreateCaptureWnd()
{
	if (::IsWindow(m_hCapWnd))
		return FALSE;

	// Note: UNICODE version not available under win9x!
	m_hCapWnd = ::capCreateCaptureWindow(_T("Capture"),
										WS_POPUP,
										0,
										0,
										1,
										1, 
										m_pDoc->GetView()->GetSafeHwnd(),
										0);

	if (m_hCapWnd == NULL)
		return FALSE;
	else
		return TRUE;
}

BOOL CVideoDeviceDoc::CVfWCaptureVideoThread::ConnectForce(double dFrameRate)
{
	if (::IsWindow(m_hCapWnd))
	{
		if (!Disconnect())
			return FALSE;
		if (!DestroyCaptureWnd())
			return FALSE;
	}
	
	if (CreateCaptureWnd())
		return Connect(dFrameRate);
	else
		return FALSE;
}

BOOL CVideoDeviceDoc::CVfWCaptureVideoThread::Connect(double dFrameRate)
{
	// Set Exact NTSC Frame Rate
	if (Round(dFrameRate) == 30)
		dFrameRate = 30000.0 / 1001.0;

	// Set User Data For Callback Function
	capSetUserData(m_hCapWnd, this);
 
	// Disable Preview And Overlay
	capPreview(m_hCapWnd, FALSE);
	capOverlay(m_hCapWnd, FALSE);

	// Connect to Driver
	if (!capDriverConnect(m_hCapWnd, m_pDoc->m_dwVfWCaptureVideoDeviceID))
	{
		// Device may be open already or it may not have been
		// closed properly last time.
		::AfxMessageBox(ML_STRING(1464, "Unable to open Video Capture Device"));
		::DestroyWindow(m_hCapWnd);
		m_hCapWnd = NULL;
		return FALSE;
	}

	// Set the capture parameters
	CAPTUREPARMS CapParms;
	ZeroMemory(&CapParms, sizeof(CAPTUREPARMS));
	capCaptureGetSetup(m_hCapWnd, &CapParms, sizeof(CAPTUREPARMS));
	CapParms.dwRequestMicroSecPerFrame = (DWORD)Round(1000000.0 / dFrameRate); 
	CapParms.fMakeUserHitOKToCapture = FALSE;
	CapParms.wPercentDropForError = 60;
	CapParms.fYield = TRUE; // Separate Thread
	CapParms.dwIndexSize = 1;// (Max: 324000), here not used because we do not capture to a file directly
	CapParms.wChunkGranularity = 0;
	CapParms.fUsingDOSMemory = FALSE;
	CapParms.wNumVideoRequested = 5;
	CapParms.fCaptureAudio = FALSE;
	CapParms.wNumAudioRequested = 4;
	CapParms.vKeyAbort = 0; // 27 = ESC (Original Set)
	CapParms.fAbortLeftMouse = FALSE;
	CapParms.fAbortRightMouse = FALSE;
	CapParms.fLimitEnabled = FALSE;
	CapParms.wTimeLimit = 0;
	CapParms.fMCIControl = FALSE;
	CapParms.fStepMCIDevice = FALSE;
	CapParms.dwMCIStartTime = 0;
	CapParms.dwMCIStopTime = 0;
	CapParms.fStepCaptureAt2x = FALSE; 
	CapParms.wStepCaptureAverageFrames = 0;
	CapParms.dwAudioBufferSize = 0;
	CapParms.fDisableWriteCache = FALSE;
	CapParms.AVStreamMaster = AVSTREAMMASTER_NONE;

	if (capCaptureSetSetup(m_hCapWnd, &CapParms, sizeof(CapParms)) == FALSE)
	{
		capDriverDisconnect(m_hCapWnd);
		::DestroyWindow(m_hCapWnd);
		m_hCapWnd = NULL;
		return FALSE;
	}

	// Update the Video Dib
	::SendMessage(	m_pDoc->GetView()->GetSafeHwnd(),
					WM_THREADSAFE_CHANGEVIDEOFORMAT,
					0, 0);

	return TRUE;
}

BOOL CVideoDeviceDoc::CVfWCaptureVideoThread::StartCapture()
{
	if (::IsWindow(m_hCapWnd))
	{
		capSetCallbackOnVideoStream(m_hCapWnd, OnCaptureVideo);

		if (capCaptureSequenceNoFile(m_hCapWnd) == FALSE)
		{
			capSetCallbackOnVideoStream(m_hCapWnd, NULL);
			capDriverDisconnect(m_hCapWnd);
			::DestroyWindow(m_hCapWnd);
			m_hCapWnd = NULL;
			return FALSE;
		}
		else
		{
			m_pDoc->m_bCapture = TRUE;
			return TRUE;
		}
	}
	else
		return FALSE;
}

BOOL CVideoDeviceDoc::CVfWCaptureVideoThread::StartTriggeredFrameCapture()
{
	if (::IsWindow(m_hCapWnd))
	{
		BOOL res = capSetCallbackOnFrame(m_hCapWnd, OnFrame);
		if (res)
		{
			m_bTriggeredCapture = FALSE;
			Start();
		}
		return res;
	}
	else
		return FALSE;
}

int CVideoDeviceDoc::CVfWCaptureVideoThread::GetDroppedFrames()
{
	if (::IsWindow(m_hCapWnd))
	{
		CAPSTATUS st;
		memset(&st, 0, sizeof(CAPSTATUS));
		capGetStatus(m_hCapWnd, &st, sizeof(CAPSTATUS));
		return (int)st.dwCurrentVideoFramesDropped;
	}
	else
		return -1;
}

BOOL CVideoDeviceDoc::CVfWCaptureVideoThread::DestroyCaptureWnd()
{
	if (::IsWindow(m_hCapWnd))
	{
		::DestroyWindow(m_hCapWnd);
		m_hCapWnd = NULL;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CVideoDeviceDoc::CVfWCaptureVideoThread::Disconnect()
{
	if (::IsWindow(m_hCapWnd))
	{
		StopCapture();
		StopTriggeredFrameCapture();

		// Disconnect
		capDriverDisconnect(m_hCapWnd);

		return TRUE;
	}
	else
		return FALSE;
}

BOOL CVideoDeviceDoc::CVfWCaptureVideoThread::StopCapture()
{
	if (::IsWindow(m_hCapWnd))
	{
		capCaptureStop(m_hCapWnd);
		capCaptureAbort(m_hCapWnd);
		capSetCallbackOnVideoStream(m_hCapWnd, NULL);
		m_pDoc->m_bCapture = FALSE;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CVideoDeviceDoc::CVfWCaptureVideoThread::StopTriggeredFrameCapture()
{
	Kill();
	if (::IsWindow(m_hCapWnd))
		return capSetCallbackOnFrame(m_hCapWnd, NULL);
	else
		return FALSE;
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
					m_pDoc->m_sGetFrameVideoHost,
					m_pDoc->m_nGetFrameVideoPort,
					m_pDoc->m_pGetFrameNetCom->GetSocketFamily()))
		{
			m_HttpGetFrameNetComList.AddTail(pNetCom);
			m_HttpGetFrameParseProcessList.AddTail(pHttpGetFrameParseProcess);
			pHttpGetFrameParseProcess->SendFrameRequest();
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
	while (!m_HttpGetFrameNetComList.IsEmpty())
	{
		pNetCom = m_HttpGetFrameNetComList.GetHead();
		if (pNetCom)
		{
			// Peer closed the connection normally
			if (pNetCom->IsShutdown())
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
				// (connection may have been abrubtly closed by the peer
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
	m_pDoc->EndWaitCursor();
	::AfxMessageBox(ML_STRING(1465, "Cannot connect to the specified network device or server"), MB_ICONSTOP);
	m_pDoc->BeginWaitCursor();
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
												  LPCTSTR pszHostName,
												  int nPort,
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
					pszHostName,			// Peer Address (IP or Host Name).
					nPort,					// Peer Port.
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
	BOOL bAlarm1 = FALSE;
	BOOL bAlarm2 = FALSE;
	BOOL bAlarm3 = FALSE;
	BOOL bReadEvent = FALSE;
	DWORD dwLastSetupConnectionTime = ::timeGetTime();

	for (;;)
	{
		DWORD dwWaitDelay = HTTPGETFRAME_DELAY_DEFAULT;
		if (m_pDoc->m_dFrameRate > 0.0)
			dwWaitDelay = (DWORD)Round(1000.0 / m_pDoc->m_dFrameRate);
		if (bAlarm1)
			dwWaitDelay = 2U*dwWaitDelay;
		else if (bAlarm2)
			dwWaitDelay = MAX(4U*dwWaitDelay, HTTPGETFRAME_DELAY_ALARM2);
		else if (bAlarm3)
			dwWaitDelay = MAX(8U*dwWaitDelay, HTTPGETFRAME_DELAY_ALARM3);
		DWORD Event = ::WaitForMultipleObjects(	5,
												m_hEventArray,
												FALSE,
												dwWaitDelay);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :		
			{
				CleanUpAllConnections();
				return 0;
			}

			// Http Setup Connection Event
			case WAIT_OBJECT_0 + 1 :
			{
				::ResetEvent(m_hEventArray[1]);
				bReadEvent = FALSE;
				dwLastSetupConnectionTime = ::timeGetTime();
				if (!Connect(TRUE,
							m_pDoc->m_pGetFrameNetCom,
							m_pDoc->m_pHttpGetFrameParseProcess,
							m_sHostName,
							m_nPort,
							((CUImagerApp*)::AfxGetApp())->m_bIPv6 ? AF_INET6 : AF_INET))
				{
					if (m_pDoc->m_pHttpGetFrameParseProcess->m_bTryConnecting)
						return OnError();
				}
				break;
			}

			// Http Connected Event
			case WAIT_OBJECT_0 + 2 :
			{
				::ResetEvent(m_hEventArray[2]);
				if (m_sRequest == _T(""))
					m_pDoc->m_pHttpGetFrameParseProcess->SendFrameRequest();
				else
					m_pDoc->m_pHttpGetFrameParseProcess->SendRequest(m_sRequest);
				break;
			}

			// Http Read Event
			case WAIT_OBJECT_0 + 3 :
			{
				::ResetEvent(m_hEventArray[3]);
				bReadEvent = TRUE;
				break;
			}

			// Http Connection failed Event
			case WAIT_OBJECT_0 + 4 :
			{
				::ResetEvent(m_hEventArray[4]);
				if (m_pDoc->m_pHttpGetFrameParseProcess->m_bTryConnecting)
					return OnError();
				break;
			}

			// Timeout
			case WAIT_TIMEOUT :		
			{	
				// Setup connection timeout
				if (m_pDoc->m_pHttpGetFrameParseProcess->m_bTryConnecting	&&
					!bReadEvent												&&
					(::timeGetTime() - dwLastSetupConnectionTime) >= HTTPGETFRAME_CONNECTION_TIMEOUT)
					return OnError();

				// Poll
				if (m_pDoc->m_pHttpGetFrameParseProcess->m_bPollNextJpeg)
				{
					if (m_pDoc->m_pHttpGetFrameParseProcess->m_bConnectionKeepAlive)
						m_pDoc->m_pHttpGetFrameParseProcess->SendFrameRequest();
					else
					{
						if (m_HttpGetFrameNetComList.GetCount() >= HTTPGETFRAME_MAXCOUNT_ALARM3)
						{
							bAlarm1 = FALSE;
							bAlarm2 = FALSE;
							bAlarm3 = TRUE;
						}
						else if (m_HttpGetFrameNetComList.GetCount() >= HTTPGETFRAME_MAXCOUNT_ALARM2)
						{
							bAlarm1 = FALSE;
							bAlarm2 = TRUE;
							bAlarm3 = FALSE;
						}
						else if (m_HttpGetFrameNetComList.GetCount() >= HTTPGETFRAME_MAXCOUNT_ALARM1)
						{
							bAlarm1 = TRUE;
							bAlarm2 = FALSE;
							bAlarm3 = FALSE;
						}
						else
						{
							bAlarm1 = FALSE;
							bAlarm2 = FALSE;
							bAlarm3 = FALSE;
						}
						
						// If in Alarm3 state just free closed connection
						// and do not open new ones!
						PollAndClean(!bAlarm3);
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

int CVideoDeviceDoc::CWatchdogThread::Work()
{
	ASSERT(m_pDoc);
	DWORD Event;

	// Necessary because we are drawing with directx
	::CoInitialize(NULL);

	// Get the code section start address and size
#ifdef CRACKCHECK
	if (!g_pCodeStart)
	{
		if (!::CodeSectionInformation())
		{
			// Usually it fails if the exe is compressed (with upx for example)
			// I wrote a note in the license file which says:
			// "This software will not work in case the executable is packed/compressed."
			CPostDelayedMessageThread::PostDelayedMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
															WM_CLOSE, 4986, 0, 0);
		}
	}
#endif

	// Poll starting flag
	for (;;)
	{
		Event = ::WaitForSingleObject(GetKillEvent(), WATCHDOG_CHECK_TIME);
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
	}

	// Init
	DWORD dwLastHttpReconnectUpTime = ::timeGetTime();

	// Watch
	for (;;)
	{
		Event = ::WaitForSingleObject(GetKillEvent(), WATCHDOG_CHECK_TIME);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :		
			{
				::CoUninitialize();
				return 0;
			}

			// Check
			case WAIT_TIMEOUT :		
			{
				// Update m_bWatchDogAlarm
				DWORD dwFrameTime = (DWORD)Round(1000.0 / m_pDoc->m_dFrameRate);
				if (m_pDoc->m_dEffectiveFrameRate > 0.0)
					dwFrameTime = (DWORD)Round(1000.0 / m_pDoc->m_dEffectiveFrameRate);
				DWORD dwCurrentUpTime = ::timeGetTime();
				DWORD dwMsSinceLastProcessFrame = dwCurrentUpTime - (DWORD)m_pDoc->m_lCurrentInitUpTime;
				if (dwMsSinceLastProcessFrame > WATCHDOG_THRESHOLD	&&
					dwMsSinceLastProcessFrame > 7U * dwFrameTime)
					m_pDoc->m_bWatchDogAlarm = TRUE;
				else
				{
					m_pDoc->m_bWatchDogAlarm = FALSE;
					dwLastHttpReconnectUpTime = dwCurrentUpTime;
				}

				// Save Frame List may be called 1 to 4 times till
				// CSaveFrameListThread::Work() reacts and starts working:
				// it's not a problem because CSaveFrameListThread::Work()
				// removes empty lists.
				if (m_pDoc->m_bWatchDogAlarm							&&
					(m_pDoc->m_VideoProcessorMode & MOVEMENT_DETECTOR)	&&
					m_pDoc->GetTotalMovementDetectionFrames() > 0		&&
					m_pDoc->m_SaveFrameListThread.IsAlive()				&&
					!m_pDoc->m_SaveFrameListThread.IsWorking()			&&
					(m_pDoc->m_bSaveSWFMovementDetection				||
					m_pDoc->m_bSaveAVIMovementDetection					||
					m_pDoc->m_bSaveAnimGIFMovementDetection				||
					m_pDoc->m_bSendMailMovementDetection				||
					m_pDoc->m_bFTPUploadMovementDetection				||
					m_pDoc->m_bExecCommandMovementDetection))
					m_pDoc->SaveFrameList();

				// Http Server Push Networking Reconnect
				if (dwCurrentUpTime - dwLastHttpReconnectUpTime > HTTPWATCHDOG_RETRY_TIMEOUT					&&
					m_pDoc->m_pGetFrameNetCom																	&&
					m_pDoc->m_pGetFrameNetCom->IsClient()														&&
					m_pDoc->m_pHttpGetFrameParseProcess															&&
					m_pDoc->m_pHttpGetFrameParseProcess->m_FormatType == CHttpGetFrameParseProcess::FORMATMJPEG	&&
					!m_pDoc->m_pHttpGetFrameParseProcess->m_bFirstFrame)
				{
					dwLastHttpReconnectUpTime = dwCurrentUpTime;
					m_pDoc->ConnectGetFrameHTTP(m_pDoc->m_sGetFrameVideoHost,
												m_pDoc->m_nGetFrameVideoPort);
					CString sMsg;
					sMsg.Format(_T("%s try reconnecting\n"), m_pDoc->GetDeviceName());
					TRACE(sMsg);
					::LogLine(sMsg);
				}

				// Draw
				if (m_pDoc->GetView() &&
					dwMsSinceLastProcessFrame > WATCHDOG_DRAW_THRESHOLD)
					m_pDoc->GetView()->Draw();

				break;
			}

			default:
				break;
		}
	}

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
	dwAttrib =::GetFileAttributes(sAutoSaveDir);
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
		// FILES_DELETE_INTERVAL should never be to small (at least 60 seconds)
		// otherwise when entering a new days value for deletion like 30 after
		// typing the number 3 all files older than 3 days are deleted even if we
		// intended 30!
		Event = ::WaitForSingleObject(GetKillEvent(), FILES_DELETE_INTERVAL);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :
				return 0;

			// Delete
			case WAIT_TIMEOUT :		
			{
				// Code section's CRC32 check
#ifdef CRACKCHECK
				if (g_pCodeStart)
				{
					DWORD dwCrc32;
					CCrc32Dynamic::CalcAssembly(g_pCodeStart, g_dwCodeSize, dwCrc32);
					#define CRC32_KEY		0x23fa8c19U
					#define CRC32_OFFSET	23
					dwCrc32 ^= CRC32_KEY;
					if (*((LPDWORD)(&g_Crc32CheckArray[CRC32_OFFSET])) != dwCrc32)
					{
						CPostDelayedMessageThread::PostDelayedMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
																		WM_CLOSE, 9780, 0, 0);
					}
				}
				else
				{
					CPostDelayedMessageThread::PostDelayedMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
																	WM_CLOSE, 4459, 0, 0);
				}
#endif

				// Registration check, cannot call RSADecrypt here because the used rsaeuro lib
				// is not thread safe -> just check the m_bRegistered variable
				if (!((CUImagerApp*)::AfxGetApp())->m_bRegistered)
					m_pDoc->StopProcessFrame();
				
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

CVideoDeviceDoc::CColorDetection::CColorDetection()
{
	::InitializeCriticalSection(&m_cs);
	m_dwMaxWaitCount = 0;
	m_dwMaxCountsColorIndexes = 0;
	m_dwShortestTimeBetweenCountsColorIndexes = 0;
	m_dwLongestTimeBetweenCountsColorIndexes = 0;
	m_nColDetCount = 0;
	for (int i = 0 ; i < COLDET_MAX_COLORS ; i++)
	{
		m_DetectionLevels[i] = 0;
		m_DetectionLevelsThresholds[i] = 0;
		m_DetectionCountup[i] = 0;
		m_WaitCountup[i] = 0;
		m_DetectionTime[i] = 0;
		m_TimeBetweenCounts[i] = 0;
		m_ShortestTimeBetweenCounts[i] = 0;
		m_LongestTimeBetweenCounts[i] = 0;
		m_ColDetTable[i].Clear();
	}
}

CVideoDeviceDoc::CColorDetection::~CColorDetection()
{
	::DeleteCriticalSection(&m_cs);
}

// Simplest Color Detector:
// pDib   :             24 or 32 bit/pixels Bitmap To Test For Colors
// nDetectionAccuracy : 1 uses all Pixels, 2 uses 1/4 of the pixels, 3 uses 1/9 of the pixels ,
//                      4 uses 1/16 of the pixels , 5 uses 1/25 of the pixels , ...
BOOL CVideoDeviceDoc::CColorDetection::Detector(CDib* pDib, DWORD dwDetectionAccuracy, BOOL bColorImage/*=FALSE*/)
{
	int nColorIndex;

	if ((pDib == NULL) || !pDib->IsValid())
		return FALSE;

	LPBYTE pBits;
	if (!(pBits = pDib->GetBits()))
		return FALSE;

	LPBITMAPINFO pBMI;
	if (!(pBMI = pDib->GetBMI()))
		return FALSE;

	if ((pDib->GetBitCount() != 24) && (pDib->GetBitCount() != 32))
		return FALSE;

	::EnterCriticalSection(&m_cs);

	int nColDetCount = m_nColDetCount;
	CColDetEntry ColDetTable[COLDET_MAX_COLORS];
	for (int i = 0 ; i < nColDetCount ; i++)
		ColDetTable[i] = m_ColDetTable[i];

	// Detection Array 
	for (nColorIndex = 0 ; nColorIndex < nColDetCount ; nColorIndex++)
		m_DetectionLevels[nColorIndex] = 0;
	DWORD nPixelsCount = 0;

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize;
	uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(pDib->GetWidth() * pDib->GetBitCount());

	// Bytes Per Pixel
	int nBytesPerPixel;
	if (pDib->GetBitCount() == 24)
		nBytesPerPixel = 3;
	else // 32 bits/pix
		nBytesPerPixel = 4;

	// Detect
	for (int line = 0 ; line < (int)pDib->GetHeight(); line += dwDetectionAccuracy)
	{
		for (int i = 0 ; i < (int)pDib->GetWidth() ; i += dwDetectionAccuracy)
		{
			int nFirstDetCol = -1;
			int hue = pBits[nBytesPerPixel*i+2];
			int saturation = pBits[nBytesPerPixel*i+1];
			int value = pBits[nBytesPerPixel*i];
			BOOL bChromatic = ::RGB2HSV(&hue, &saturation, &value);
			if (bChromatic)
			{
				for (int n = 0 ; n < nColDetCount ; n++)
				{
					if (ColDetTable[n].huemax < ColDetTable[n].huemin)
					{
						if (!(hue <= ColDetTable[n].huemin				&&
							hue >= ColDetTable[n].huemax)				&&
							saturation < ColDetTable[n].saturationmax	&&
							saturation > ColDetTable[n].saturationmin	&&
							value < ColDetTable[n].valuemax				&&
							value > ColDetTable[n].valuemin)
						{
							if (nFirstDetCol == -1)
								nFirstDetCol = n;
							++(m_DetectionLevels[n]);
						}
		
					}
					else
					{
						if (hue < ColDetTable[n].huemax					&&
							hue > ColDetTable[n].huemin					&&
							saturation < ColDetTable[n].saturationmax	&&
							saturation > ColDetTable[n].saturationmin	&&
							value < ColDetTable[n].valuemax				&&
							value > ColDetTable[n].valuemin)
						{
							if (nFirstDetCol == -1)
								nFirstDetCol = n;
							++(m_DetectionLevels[n]);
						}
					}
				}
			}

			++nPixelsCount;
			
			if (bColorImage)
			{
				if (nFirstDetCol >= 0)
				{
					pBits[nBytesPerPixel*i+2] = ColDetTable[nFirstDetCol].red;
					pBits[nBytesPerPixel*i+1] = ColDetTable[nFirstDetCol].green;
					pBits[nBytesPerPixel*i] = ColDetTable[nFirstDetCol].blue;
				}
				else
				{	
					pBits[nBytesPerPixel*i+2] = 0;
					pBits[nBytesPerPixel*i+1] = 0;
					pBits[nBytesPerPixel*i] = 0;
				}
			}
		}
		pBits += (dwDetectionAccuracy * uiDIBSourceScanLineSize);
	}

	// If A Pixels Count is Over The Detection Level the color has been detected -> Count Up
	for (nColorIndex = 0 ; nColorIndex < nColDetCount ; nColorIndex++)
	{
		// Convert To Range 0 .. 10000
		m_DetectionLevels[nColorIndex] = m_DetectionLevels[nColorIndex] * 10000 / nPixelsCount;
		if (m_WaitCountup[nColorIndex] == 0)
		{
			if (m_DetectionLevels[nColorIndex] > m_DetectionLevelsThresholds[nColorIndex])
			{
				
				++m_DetectionCountup[nColorIndex];
				++m_WaitCountup[nColorIndex];

				if (m_DetectionCountup[nColorIndex] > 1)
				{
					// Set last time between counts
					m_TimeBetweenCounts[nColorIndex] = pDib->GetUpTime() - m_DetectionTime[nColorIndex];

					if (m_DetectionCountup[nColorIndex] == 2)
					{
						m_ShortestTimeBetweenCounts[nColorIndex] = m_TimeBetweenCounts[nColorIndex];
						m_LongestTimeBetweenCounts[nColorIndex] = m_TimeBetweenCounts[nColorIndex];
					}
					else // m_DetectionCountup[nColorIndex] > 2
					{
						// Update shortest time between counts if it is the case
						if (m_TimeBetweenCounts[nColorIndex] < m_ShortestTimeBetweenCounts[nColorIndex])
							m_ShortestTimeBetweenCounts[nColorIndex] = m_TimeBetweenCounts[nColorIndex];

						// Update longest time between counts if it is the case
						if (m_TimeBetweenCounts[nColorIndex] > m_LongestTimeBetweenCounts[nColorIndex])
							m_LongestTimeBetweenCounts[nColorIndex] = m_TimeBetweenCounts[nColorIndex];
					}
				}

				// Update with new time
				m_DetectionTime[nColorIndex] = pDib->GetUpTime();
			}
		}
		else 
			++m_WaitCountup[nColorIndex];

		if (m_WaitCountup[nColorIndex] >= m_dwMaxWaitCount)			
			m_WaitCountup[nColorIndex] = 0;
	}

	// Update overall maximum count color index if it is the case
	int nMaxCounts = 0;
	for (nColorIndex = 0 ; nColorIndex < nColDetCount ; nColorIndex++)
	{
		if ((int)m_DetectionCountup[nColorIndex] > nMaxCounts)
			nMaxCounts = m_DetectionCountup[nColorIndex];
	}
	m_dwMaxCountsColorIndexes = 0;
	for (nColorIndex = 0 ; nColorIndex < nColDetCount ; nColorIndex++)
	{
		if (m_DetectionCountup[nColorIndex] == nMaxCounts)
			m_dwMaxCountsColorIndexes |= (1<<nColorIndex);
	}

	// Update overall shortest time color index if it is the case
	DWORD dwShortestTime = 0xFFFFFFFF;
	for (nColorIndex = 0 ; nColorIndex < nColDetCount ; nColorIndex++)
	{
		if (m_DetectionCountup[nColorIndex] > 1)
		{
			if (m_ShortestTimeBetweenCounts[nColorIndex] < dwShortestTime)
				dwShortestTime = m_ShortestTimeBetweenCounts[nColorIndex];
		}
	}
	m_dwShortestTimeBetweenCountsColorIndexes = 0;
	for (nColorIndex = 0 ; nColorIndex < nColDetCount ; nColorIndex++)
	{
		if (m_DetectionCountup[nColorIndex] > 1)
		{
			if (m_ShortestTimeBetweenCounts[nColorIndex] == dwShortestTime)
				m_dwShortestTimeBetweenCountsColorIndexes |= (1<<nColorIndex);
		}
	}

	// Update overall longest time color index if it is the case
	DWORD dwLongestTime = 0;
	for (nColorIndex = 0 ; nColorIndex < nColDetCount ; nColorIndex++)
	{
		if (m_DetectionCountup[nColorIndex] > 1)
		{
			if (m_LongestTimeBetweenCounts[nColorIndex] > dwLongestTime)
				dwLongestTime = m_LongestTimeBetweenCounts[nColorIndex];
		}
	}
	m_dwLongestTimeBetweenCountsColorIndexes = 0;
	for (nColorIndex = 0 ; nColorIndex < nColDetCount ; nColorIndex++)
	{
		if (m_DetectionCountup[nColorIndex] > 1)
		{
			if (m_LongestTimeBetweenCounts[nColorIndex] == dwLongestTime)
				m_dwLongestTimeBetweenCountsColorIndexes |= (1<<nColorIndex);
		}
	}

	::LeaveCriticalSection(&m_cs);

	return TRUE;
}

COLORREF CVideoDeviceDoc::CColorDetection::CalcMeanValue(CDib* pDib, DWORD dwCalcAccuracy)
{
	if ((pDib == NULL) || !pDib->IsValid())
		return 0;

	LPBYTE pBits;
	if (!(pBits = pDib->GetBits()))
		return 0;

	LPBITMAPINFO pBMI;
	if (!(pBMI = pDib->GetBMI()))
		return FALSE;

	if (pDib->IsCompressed())
#ifndef _WIN32_WCE
		if (!pDib->Decompress(pDib->GetBitCount())) // Decompress
			return 0;
#else
		return 0;
#endif

	if ((pDib->GetBitCount() != 24) && (pDib->GetBitCount() != 32))
		return FALSE;

	// Scan Line Alignments
	DWORD uiDIBSourceScanLineSize;
	uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(pDib->GetWidth() * pDib->GetBitCount());

	// Bytes Per Pixel
	int nBytesPerPixel;
	if (pDib->GetBitCount() == 24)
		nBytesPerPixel = 3;
	else // 32 bits/pix
		nBytesPerPixel = 4;

	// Calc Mean Value
	DWORD nPixelsCount = 0;
	DWORD R = 0;
	DWORD G = 0;
	DWORD B = 0;
	for (int line = 0 ; line < (int)pDib->GetHeight(); line += dwCalcAccuracy)
	{
		for (int i = 0 ; i < (int)pDib->GetWidth() ; i += dwCalcAccuracy)
		{
			R += pBits[nBytesPerPixel*i+2];
			G += pBits[nBytesPerPixel*i+1];
			B += pBits[nBytesPerPixel*i];
			++nPixelsCount;
		}
		pBits += (dwCalcAccuracy * uiDIBSourceScanLineSize);
	}

	return (RGB(R / nPixelsCount, G / nPixelsCount, B / nPixelsCount));
}

DWORD CVideoDeviceDoc::CColorDetection::GetColorsCount()
{
	::EnterCriticalSection(&m_cs);
	int nColDetCount = m_nColDetCount;
	::LeaveCriticalSection(&m_cs);
	return nColDetCount;
}

COLORREF CVideoDeviceDoc::CColorDetection::GetColor(DWORD dwIndex)
{
	::EnterCriticalSection(&m_cs);

	if ((int)dwIndex >= m_nColDetCount)
	{
		::LeaveCriticalSection(&m_cs);
		return 0;
	}

	COLORREF col = RGB(	m_ColDetTable[dwIndex].red,
						m_ColDetTable[dwIndex].green,
						m_ColDetTable[dwIndex].blue);

	::LeaveCriticalSection(&m_cs);

	return col;
}

BOOL CVideoDeviceDoc::CColorDetection::SetDetectionThreshold(DWORD dwIndex, DWORD dwThreshold)
{
	if (dwIndex >= COLDET_MAX_COLORS)
		return FALSE;
	::EnterCriticalSection(&m_cs);
	m_DetectionLevelsThresholds[dwIndex] = dwThreshold;
	::LeaveCriticalSection(&m_cs);
	return TRUE;
}

BOOL CVideoDeviceDoc::CColorDetection::SetHueRadius(DWORD dwIndex, DWORD dwRadius)
{
	if (dwIndex >= COLDET_MAX_COLORS)
		return FALSE;
	::EnterCriticalSection(&m_cs);
	m_ColDetTable[dwIndex].huemin = m_ColDetTable[dwIndex].hue - dwRadius;
	m_ColDetTable[dwIndex].huemax = m_ColDetTable[dwIndex].hue + dwRadius;
	if (m_ColDetTable[dwIndex].huemin < 0)
		m_ColDetTable[dwIndex].huemin += 360;
	if (m_ColDetTable[dwIndex].huemax > 360)
		m_ColDetTable[dwIndex].huemax -= 360;					
	::LeaveCriticalSection(&m_cs);
	return TRUE;
}

BOOL CVideoDeviceDoc::CColorDetection::SetSaturationRadius(DWORD dwIndex, DWORD dwRadius)
{
	if (dwIndex >= COLDET_MAX_COLORS)
		return FALSE;
	::EnterCriticalSection(&m_cs);
	m_ColDetTable[dwIndex].saturationmin = m_ColDetTable[dwIndex].saturation - dwRadius;
	m_ColDetTable[dwIndex].saturationmax = m_ColDetTable[dwIndex].saturation + dwRadius;
	if (m_ColDetTable[dwIndex].saturationmin < 0)
		m_ColDetTable[dwIndex].saturationmin = 0;
	if (m_ColDetTable[dwIndex].saturationmax > 255)
		m_ColDetTable[dwIndex].saturationmax = 255;
	::LeaveCriticalSection(&m_cs);
	return TRUE;
}

BOOL CVideoDeviceDoc::CColorDetection::SetValueRadius(DWORD dwIndex, DWORD dwRadius)
{
	if (dwIndex >= COLDET_MAX_COLORS)
		return FALSE;
	::EnterCriticalSection(&m_cs);
	m_ColDetTable[dwIndex].valuemin = m_ColDetTable[dwIndex].value - dwRadius;
	m_ColDetTable[dwIndex].valuemax = m_ColDetTable[dwIndex].value + dwRadius;
	if (m_ColDetTable[dwIndex].valuemin < 0)
		m_ColDetTable[dwIndex].valuemin = 0;
	if (m_ColDetTable[dwIndex].valuemax > 255)
		m_ColDetTable[dwIndex].valuemax = 255;
	::LeaveCriticalSection(&m_cs);
	return TRUE;
}

int CVideoDeviceDoc::CColorDetection::GetHueRadius(DWORD dwIndex)
{
	if (dwIndex >= COLDET_MAX_COLORS)
		return -1;

	::EnterCriticalSection(&m_cs);
	int radius;
	if (m_ColDetTable[dwIndex].huemin > m_ColDetTable[dwIndex].huemax)
		radius = (360 - (m_ColDetTable[dwIndex].huemin - m_ColDetTable[dwIndex].huemax)) / 2;
	else
		radius = (m_ColDetTable[dwIndex].huemax - m_ColDetTable[dwIndex].huemin) / 2;
	::LeaveCriticalSection(&m_cs);
	return radius;
}

int CVideoDeviceDoc::CColorDetection::GetSaturationRadius(DWORD dwIndex)
{
	if (dwIndex >= COLDET_MAX_COLORS)
		return -1;

	::EnterCriticalSection(&m_cs);
	int radius = (m_ColDetTable[dwIndex].saturationmax - m_ColDetTable[dwIndex].saturationmin) / 2;
	::LeaveCriticalSection(&m_cs);
	return radius;
}

int CVideoDeviceDoc::CColorDetection::GetValueRadius(DWORD dwIndex)
{
	if (dwIndex >= COLDET_MAX_COLORS)
		return -1;

	::EnterCriticalSection(&m_cs);
	int radius = (m_ColDetTable[dwIndex].valuemax - m_ColDetTable[dwIndex].valuemin) / 2;
	::LeaveCriticalSection(&m_cs);
	return radius;
}

int CVideoDeviceDoc::CColorDetection::AppendColor(HSVARRAY& a)
{
	int nIndex = -1;

	::EnterCriticalSection(&m_cs);
	if (m_nColDetCount < COLDET_MAX_COLORS)
	{
		if (m_ColDetTable[m_nColDetCount].SetHSVArray(a))
		{
			nIndex = m_nColDetCount;
			m_nColDetCount++;
		}
	}
	::LeaveCriticalSection(&m_cs);

	return nIndex;
}

BOOL CVideoDeviceDoc::CColorDetection::ReplaceColor(DWORD dwIndex, HSVARRAY& a)
{
	if (dwIndex >= COLDET_MAX_COLORS)
		return FALSE;

	::EnterCriticalSection(&m_cs);
	BOOL res = m_ColDetTable[dwIndex].SetHSVArray(a);
	::LeaveCriticalSection(&m_cs);

	return res;
}

BOOL CVideoDeviceDoc::CColorDetection::RemoveColor(DWORD dwIndex)
{
	::EnterCriticalSection(&m_cs);

	if (m_nColDetCount > 0)
	{
		for (int i = (int)dwIndex + 1 ; i < m_nColDetCount ; i++)
			m_ColDetTable[i-1] = m_ColDetTable[i];
		m_nColDetCount--;
	}

	::LeaveCriticalSection(&m_cs);

	return TRUE;
}

int CVideoDeviceDoc::CColorDetection::GetDetectionLevel(DWORD dwIndex)// 0 .. 10000 %, -1 if error
{
	if (dwIndex >= COLDET_MAX_COLORS)
		return -1;
	else
	{
		::EnterCriticalSection(&m_cs);
		int detectionlevel = (int)(m_DetectionLevels[dwIndex]);
		::LeaveCriticalSection(&m_cs);
		return detectionlevel;
	}
}

DWORD CVideoDeviceDoc::CColorDetection::GetTimeBetweenCounts(DWORD dwIndex)
{
	if (dwIndex >= COLDET_MAX_COLORS)
		return -1;
	else
	{
		::EnterCriticalSection(&m_cs);
		DWORD time = m_TimeBetweenCounts[dwIndex];
		::LeaveCriticalSection(&m_cs);
		return time;
	}
}

DWORD CVideoDeviceDoc::CColorDetection::GetShortestTimeBetweenCounts(DWORD dwIndex)
{
	if (dwIndex >= COLDET_MAX_COLORS)
		return -1;
	else
	{
		::EnterCriticalSection(&m_cs);
		DWORD shortesttime = m_ShortestTimeBetweenCounts[dwIndex];
		::LeaveCriticalSection(&m_cs);
		return shortesttime;
	}
}

DWORD CVideoDeviceDoc::CColorDetection::GetLongestTimeBetweenCounts(DWORD dwIndex)
{
	if (dwIndex >= COLDET_MAX_COLORS)
		return -1;
	else
	{
		::EnterCriticalSection(&m_cs);
		DWORD longesttime = m_LongestTimeBetweenCounts[dwIndex];
		::LeaveCriticalSection(&m_cs);
		return longesttime;
	}
}

void CVideoDeviceDoc::CColorDetection::ResetCounter()
{
	::EnterCriticalSection(&m_cs);
	m_dwMaxCountsColorIndexes = 0;
	m_dwShortestTimeBetweenCountsColorIndexes = 0;
	m_dwLongestTimeBetweenCountsColorIndexes = 0;
	for (DWORD dwIndex = 0 ; dwIndex < COLDET_MAX_COLORS ; dwIndex++)
	{
		m_DetectionCountup[dwIndex] = 0;
		m_WaitCountup[dwIndex] = 0;
		m_DetectionTime[dwIndex] = 0;
		m_TimeBetweenCounts[dwIndex] = 0;
		m_ShortestTimeBetweenCounts[dwIndex] = 0;
		m_LongestTimeBetweenCounts[dwIndex] = 0;
	}
	::LeaveCriticalSection(&m_cs);
}

//
// Hue Quadrants
//
//           | 0°
//           |
//       4   |   1
// 270°      |        90°
// ---------------------
//           |
//       3   |   2
//           |
//           | 180°
//
BOOL CVideoDeviceDoc::CColorDetection::CColDetEntry::SetHSVArray(HSVARRAY& a)
{
	CString s(_T("\n"));
	CString t;

	// Calc. Saturation and Value and determine
	// in which Quadrants the given colors are
	int saturation_radius;
	int value_radius;
	int array_saturationmin = 255;
	int array_saturationmax = 0;
	int array_valuemin = 255;
	int array_valuemax = 0;
	BOOL bQuadrant1 = FALSE;
	BOOL bQuadrant2 = FALSE;
	BOOL bQuadrant3 = FALSE;
	BOOL bQuadrant4 = FALSE;
	saturation = 0;
	value = 0;
	for (int i = 0 ; i < a.GetSize() ; i++)
	{
		if (a[i].hue >= 0 && a[i].hue < 90)
			bQuadrant1 = TRUE;
		else if (a[i].hue >= 90 && a[i].hue < 180)
			bQuadrant2 = TRUE;
		else if (a[i].hue >= 180 && a[i].hue < 270)
			bQuadrant3 = TRUE;
		else if (a[i].hue >= 270 && a[i].hue < 360)
			bQuadrant4 = TRUE;

		array_saturationmin = MIN(array_saturationmin, a[i].saturation);
		array_saturationmax = MAX(array_saturationmax, a[i].saturation);
		array_valuemin = MIN(array_valuemin, a[i].value);
		array_valuemax = MAX(array_valuemax, a[i].value);
		saturation += a[i].saturation;
		value += a[i].value;

		t.Format(_T("%02d:     H=%03d S=%03d V=%03d\n"), i, a[i].hue, a[i].saturation, a[i].value); 
		s += t;
	}

	// Saturation and Value Means
	saturation /= a.GetSize();
	value /= a.GetSize();

	// Saturation and Value Radius
	saturation_radius = Round(COLDET_RADIUS_MARGINE * MAX(array_saturationmax - saturation, saturation - array_saturationmin));
	value_radius = Round(COLDET_RADIUS_MARGINE * MAX(array_valuemax - value, value - array_valuemin));

	int hue_radius;
	int array_huemin;
	int array_huemax;
	
	// Quandrants Check
	if (bQuadrant1 && bQuadrant2 && bQuadrant3 && bQuadrant4)
	{
		::AfxMessageBox(ML_STRING(1492, "You clicked on too different colors!"));
		return FALSE;
	}
	// Calc. Hue in quadrants 1 and 4 (eventually also in 2 or 3)
	else if (bQuadrant1 && bQuadrant4)
	{
		int hue_temp;
		hue = 0;
		array_huemin = 180;
		array_huemax = -180;
		for (int i = 0 ; i < a.GetSize() ; i++)
		{
			if (a[i].hue >= 180)
				hue_temp = a[i].hue - 360;
			else
				hue_temp = a[i].hue;
			hue += hue_temp;
			array_huemin = MIN(array_huemin, hue_temp);
			array_huemax = MAX(array_huemax, hue_temp);
		}
		hue /= a.GetSize();
		hue_radius = Round(COLDET_RADIUS_MARGINE * MAX(array_huemax - hue, hue - array_huemin));
		if (hue < 0)
			hue += 360;
		if (array_huemin < 0)
			array_huemin += 360;
	}
	// Calc. Hue in quadrants 2 and/or 3 (eventually also in 1 or 4)
	else
	{
		hue = 0;
		array_huemin = 360;
		array_huemax = 0;
		for (int i = 0 ; i < a.GetSize() ; i++)
		{
			array_huemin = MIN(array_huemin, a[i].hue);
			array_huemax = MAX(array_huemax, a[i].hue);
			hue += a[i].hue;
		}
		hue /= a.GetSize();
		hue_radius = Round(COLDET_RADIUS_MARGINE * MAX(array_huemax - hue, hue - array_huemin));
	}

	// Min Radius
	if (hue_radius == 0)
		hue_radius = COLDET_MIN_HUE_RADIUS;
	if (saturation_radius == 0)
		saturation_radius = COLDET_MIN_SATURATION_RADIUS;
	if (value_radius == 0)
		value_radius = COLDET_MIN_VALUE_RADIUS;

	// Max Radius
	if (hue_radius > COLDET_MAX_HUE_RADIUS)
		hue_radius = COLDET_MAX_HUE_RADIUS;
	if (saturation_radius > COLDET_MAX_SATURATION_RADIUS)
		saturation_radius = COLDET_MAX_SATURATION_RADIUS;
	if (value_radius > COLDET_MAX_VALUE_RADIUS)
		value_radius = COLDET_MAX_VALUE_RADIUS;

	// Set Min / Max
	huemin = hue - hue_radius;
	huemax = hue + hue_radius;
	if (huemin < 0)
		huemin += 360;
	if (huemax >= 360)
		huemax -= 360;
	saturationmin = saturation - saturation_radius;
	saturationmax = saturation + saturation_radius;
	if (saturationmin < 0)
		saturationmin = 0;
	if (saturationmax > 255)
		saturationmax = 255;
	valuemin = value - value_radius;
	valuemax = value + value_radius;
	if (valuemin < 0)
		valuemin = 0;
	if (valuemax > 255)
		valuemax = 255;

	// Mean Value To RGB
	red = hue;
	green = saturation;
	blue = value;
	::HSV2RGB((int*)&red, (int*)&green, (int*)&blue);

	// Trace
	t.Format(_T("Min:    H=%03d S=%03d V=%03d\n"), array_huemin, array_saturationmin, array_valuemin); 
	s += t;
	t.Format(_T("Max:    H=%03d S=%03d V=%03d\n"), array_huemax, array_saturationmax, array_valuemax); 
	s += t;
	t.Format(_T("Mean:   H=%03d S=%03d V=%03d\n"), hue, saturation, value); 
	s += t;
	t.Format(_T("Radius: H=%03d S=%03d V=%03d\n"), hue_radius, saturation_radius, value_radius); 
	s += t;
	t.Format(_T("Color:  R=%03d G=%03d B=%03d\n"), red, green, blue); 
	s += t;
	TRACE(s);

	return TRUE;
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

	// General Vars
	m_bResetSettings = FALSE;
	m_pView = NULL;
	m_pFrame = NULL;
	m_bCaptureAudio = FALSE;
	m_bStopProcessFrame = 0;
	m_bProcessFrameStopped = 0;
	m_pAVRec = NULL;
	m_bInterleave = FALSE; // Do not interleave because while recording the frame rate is not yet exactly known!
	m_bDeinterlace = FALSE;
	m_bRecDeinterlace = FALSE;
	memset(&m_OrigBMI, 0, sizeof(BITMAPINFOFULL));
	m_dFrameRate = DEFAULT_FRAMERATE;
	m_dEffectiveFrameRate = 0.0;
	m_dEffectiveFrameTimeSum = 0.0;
	m_dwEffectiveFrameTimeCountUp = 0U;
	m_pVideoAviDoc = NULL;
	m_bDoEditCopy = FALSE;
	m_bDoEditPaste = FALSE;
	m_bRgb32Frame = FALSE;
	m_lProcessFrameTime = 0;
	m_lCompressedDataRate = 0;
	m_lCompressedDataRateSum = 0;
	m_bCapture = FALSE;
	m_bCaptureStarted = 0;
	m_bShowFrameTime = TRUE;
	m_bVideoView = TRUE;
	m_VideoProcessorMode = NO_DETECTOR;
	m_bDecodeFramesForPreview = FALSE;
	m_dwFrameCountUp = 0U;
	m_bSizeToDoc = TRUE;
	m_bDeviceFirstRun = FALSE;

	// Capture Devices
	m_pDxCapture = NULL;
	m_dwCaptureAudioDeviceID = 0U;
	m_dwVfWCaptureVideoDeviceID = 0U;
	m_nDeviceInputId = -1;
	m_nDeviceFormatId = -1;
	m_bVfWVideoFormatApplyPressed = FALSE;
	m_bStopAndChangeFormat = FALSE;
	m_bVfWDialogDisplaying = FALSE;
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
	m_HttpGetFrameLocations.Add(_T("/"));
	m_HttpGetFrameLocations.Add(_T("/image.jpg"));
	m_HttpGetFrameLocations.Add(_T("/IMAGE.JPG"));
	m_HttpGetFrameLocations.Add(_T("/goform/video2"));
	m_HttpGetFrameLocations.Add(_T("/goform/video"));
	m_HttpGetFrameLocations.Add(_T("/goform/capture"));
	m_HttpGetFrameLocations.Add(_T("/Jpeg/CamImg.jpg"));
	m_HttpGetFrameLocations.Add(_T("/netcam.jpg"));
	m_HttpGetFrameLocations.Add(_T("/jpg/image.jpg"));
	m_HttpGetFrameLocations.Add(_T("/record/current.jpg"));
	m_HttpGetFrameLocations.Add(_T("/cgi-bin/getimage.cgi?motion=0"));
	m_HttpGetFrameLocations.Add(_T("/video.cgi"));
	m_HttpGetFrameLocations.Add(_T("/VIDEO.CGI"));
	m_HttpGetFrameLocations.Add(_T("/GetData.cgi"));
	m_HttpGetFrameLocations.Add(_T("/cgi-bin/video.jpg"));
	m_HttpGetFrameLocations.Add(_T("/cgi-bin/auto.cgi"));
	m_HttpGetFrameLocations.Add(_T("/image"));
	m_HttpGetFrameLocations.Add(_T("/cgi-bin/image.cgi?control=0&id=admin&passwd=admin"));
	m_HttpGetFrameLocations.Add(_T("/img/snapshot.cgi"));

	// Snapshot
	m_sSnapshotAutoSaveDir = _T("");
	m_bSnapshotLiveJpeg = FALSE;
	m_bSnapshotHistoryJpeg = FALSE;
	m_bSnapshotHistorySwf = FALSE;
	m_bSnapshotLiveJpegFtp = FALSE;
	m_bSnapshotHistoryJpegFtp = FALSE;
	m_bSnapshotHistorySwfFtp = FALSE;
	m_bSnapshotHistoryDeinterlace = FALSE;
	m_nSnapshotRate = DEFAULT_SNAPSHOT_RATE;
	m_nSnapshotHistoryFrameRate = DEFAULT_SNAPSHOT_HISTORY_FRAMERATE;
	m_bSnapshotHistoryCloseSwfFile = 0;
	m_nSnapshotCompressionQuality = DEFAULT_SNAPSHOT_COMPR_QUALITY;
	m_fSnapshotVideoCompressorQuality = DEFAULT_VIDEO_QUALITY;
	m_bSnapshotThumb = FALSE;
	m_nSnapshotThumbWidth = DEFAULT_SNAPSHOT_THUMB_WIDTH;
	m_nSnapshotThumbHeight = DEFAULT_SNAPSHOT_THUMB_HEIGHT;
	m_dwNextSnapshotUpTime = 0U;
	m_bSnapshotStartStop = FALSE;
	m_SnapshotStartTime = t;
	m_SnapshotStopTime = t;
	m_nDeleteSnapshotsOlderThanDays = 0;

	// Threads Init
	m_CaptureAudioThread.SetDoc(this);
	m_VfWCaptureVideoThread.SetDoc(this);
	m_HttpGetFrameThread.SetDoc(this);
	m_WatchdogThread.SetDoc(this);
	m_DeleteThread.SetDoc(this);
	m_SaveFrameListThread.SetDoc(this);

	// Recording
	m_sRecordAutoSaveDir = _T("");
	m_sFirstRecFileName  = _T("");
	m_nRecFilePos = 0;
	m_bRecAutoOpen = TRUE;
	m_bRecAutoOpenAllowed = TRUE;
	m_bRecSizeSegmentation = FALSE;
	m_bRecTimeSegmentation = FALSE;
	m_nTimeSegmentationIndex = 0;
	m_nRecFileCount = DEFAULT_REC_AVIFILE_COUNT;
	m_llRecFileSize = DEFAULT_REC_AVIFILE_SIZE;
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
	m_fVideoRecQuality = DEFAULT_VIDEOREC_QUALITY;
	m_dwVideoRecFourCC = DEFAULT_VIDEOREC_FOURCC;
	m_nVideoRecQualityBitrate = 0;
	m_nVideoPostRecDataRate = DEFAULT_VIDEO_DATARATE;
	m_nVideoPostRecKeyframesRate = DEFAULT_KEYFRAMESRATE;
	m_fVideoPostRecQuality = DEFAULT_VIDEOPOSTREC_QUALITY;
	m_dwVideoPostRecFourCC = DEFAULT_VIDEOPOSTREC_FOURCC;
	m_nVideoPostRecQualityBitrate = 0;
	m_bPostRec = FALSE;
	m_nDeleteRecordingsOlderThanDays = 0;

	// Movement Detection
	m_pDifferencingDib = NULL;
	m_pMovementDetectorBackgndDib = NULL;
	m_pMovementDetectorY800Dib = NULL;
	m_bShowMovementDetections = FALSE;
	m_bShowEditDetectionZones = FALSE;
	m_bShowEditDetectionZonesMinus = FALSE;
	m_bDetectingMovement = FALSE;
	m_sDetectionAutoSaveDir = _T("");
	m_nMilliSecondsRecBeforeMovementBegin = DEFAULT_PRE_BUFFER_MSEC;
	m_nMilliSecondsRecAfterMovementEnd = DEFAULT_POST_BUFFER_MSEC;
	m_bDoAdjacentZonesDetection = TRUE;
	m_bDoFalseDetectionCheck = FALSE;
	m_bDoFalseDetectionAnd = TRUE;
	m_nFalseDetectionBlueThreshold = 10;
	m_nFalseDetectionNoneBlueThreshold = 30;
	m_nBlueMovementDetectionsCount = 0;
	m_nNoneBlueMovementDetectionsCount = 0;
	m_bSaveSWFMovementDetection = TRUE;
	m_bSaveAVIMovementDetection = FALSE;
	m_bSaveAnimGIFMovementDetection = TRUE;
	m_bSendMailMovementDetection = FALSE;
	m_bFTPUploadMovementDetection = FALSE;
	m_bExecCommandMovementDetection = FALSE;
	m_sExecCommandMovementDetection = _T("");
	m_sExecParamsMovementDetection = _T("");
	m_bHideExecCommandMovementDetection = FALSE;
	m_bWaitExecCommandMovementDetection = FALSE;
	m_hExecCommandMovementDetection = NULL;
	m_nDetectionLevel = DEFAULT_MOVDET_LEVEL;
	m_nMovementDetectorIntensityLimit = DEFAULT_MOVDET_INTENSITY_LIMIT;
	m_bMovementDetectorPreview = FALSE;
	m_dwAnimatedGifWidth = MOVDET_ANIMGIF_DEFAULT_WIDTH;
	m_dwAnimatedGifHeight = MOVDET_ANIMGIF_DEFAULT_HEIGHT;
	m_LumChangeDetectorBkgY = new int[MOVDET_MAX_ZONES];
	m_LumChangeDetectorDiffY = new int[MOVDET_MAX_ZONES];
	m_MovementDetectorCurrentIntensity = new int[MOVDET_MAX_ZONES];
	m_MovementDetectionsUpTime = new DWORD[MOVDET_MAX_ZONES];
	m_MovementDetections = new BOOL[MOVDET_MAX_ZONES];
	m_DoMovementDetection = new BOOL[MOVDET_MAX_ZONES];
	m_lMovDetXZonesCount = MOVDET_MIN_ZONESX;
	m_lMovDetYZonesCount = MOVDET_MIN_ZONESY;
	m_lMovDetTotalZones = 0;
	m_nVideoDetDataRate = DEFAULT_VIDEO_DATARATE;
	m_nVideoDetKeyframesRate = DEFAULT_KEYFRAMESRATE;
	m_fVideoDetQuality = DEFAULT_VIDEODET_QUALITY;
	m_nVideoDetQualityBitrate = 0;
	m_dwVideoDetFourCC = DEFAULT_VIDEODET_FOURCC;
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

	// Color Detection
	m_nDoColorPickup = 0;
	m_bColorDetectionPreview = FALSE;
	m_dwColorDetectionAccuracy = 1U;
	m_dwColorDetectionWaitTime = DEFAULT_COLDET_WAITTIME;
	SetColorDetectionWaitTime(m_dwColorDetectionWaitTime); // (Needs Effective Framerate)

	// Property Sheet
	m_pAssistantPage = NULL;
	m_pMovementDetectionPage = NULL;
	m_pGeneralPage = NULL;
	m_pSnapshotPage = NULL;
	m_pNetworkPage = NULL;
	m_pColorDetectionPage = NULL;
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
	m_MovDetSendMailConfiguration.m_Priority = CPJNSMTPMessage::NO_PRIORITY;

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
	m_MovDetFTPUploadConfiguration.m_FilesToUpload = FILES_TO_UPLOAD_AVI_ANIMGIF;
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

	// Init Movement Detector
	OneEmptyFrameList();
	ResetMovementDetector();

	// Debugger present?
#ifdef CRACKCHECK
	if (::IsDebuggerPresentAsm())
		::AfxGetMainFrame()->PostMessage(WM_CLOSE, 0, 0);
#endif

	// Start Save Frame List Thread
	m_SaveFrameListThread.Start();

	// Start Video Watchdog Thread
	m_WatchdogThread.Start();
}

CVideoDeviceDoc::~CVideoDeviceDoc()
{
	// m_pAVRec is contained in this array and freed!
	FreeAVIFiles();
	m_pAVRec = NULL;
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
	if (m_pMovementDetectorY800Dib)
	{
		delete m_pMovementDetectorY800Dib;
		m_pMovementDetectorY800Dib = NULL;
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
	else if (::IsWindow(m_VfWCaptureVideoThread.m_hCapWnd)) 
		sDevice = _T("VfW");
	else if (((CUImagerApp*)::AfxGetApp())->IsDoc((CUImagerDoc*)m_pVideoAviDoc))
	{
		CString sShortFileName;
		if (int index = m_pVideoAviDoc->GetPathName().ReverseFind(_T('\\')))
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
	else if (::IsWindow(m_VfWCaptureVideoThread.m_hCapWnd)) 
		sDevice = _T("VfW");
	else if (((CUImagerApp*)::AfxGetApp())->IsDoc((CUImagerDoc*)m_pVideoAviDoc))
	{
		CString sShortFileName;
		if (int index = m_pVideoAviDoc->GetPathName().ReverseFind(_T('\\')))
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

	// Prepare title string
	if (m_DocRect.Width() > 0 && 
		m_DocRect.Height() > 0)
	{
		// Converting?
		BOOL bConverting = FALSE;
		if (m_bRgb32Frame	&&
			(m_OrigBMI.bmiHeader.biCompression != BI_RGB ||
			m_OrigBMI.bmiHeader.biBitCount != 32))
			bConverting = TRUE;

		// Set format string
		CString sFormat = _T("");
		if (bConverting)
			sFormat.Format(_T("%s -> RGB32"), CDib::GetCompressionName((LPBITMAPINFO)&m_OrigBMI));
		else
			sFormat.Format(_T("%s"), CDib::GetCompressionName((LPBITMAPINFO)&m_OrigBMI));

		// Name , Size , Frame rate , Pixel format
		strInfo.Format(
			_T("%s , %dx%d , %0.1ff/s , %s"),
			sName,
			m_DocRect.Width(), 
			m_DocRect.Height(),
			m_dEffectiveFrameRate >= MIN_FRAMERATE ? m_dEffectiveFrameRate : m_dFrameRate,
			sFormat);
		
		// Add IPv6 info if it's the case
		if (m_pGetFrameNetCom && m_pGetFrameNetCom->GetSocketFamily() == AF_INET6)
			strInfo += _T(" , IPv6");
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
	CString sAutoSaveDir;
	sAutoSaveDir = ::GetSpecialFolderPath(CSIDL_PERSONAL);
	if (sAutoSaveDir == _T(""))
		sAutoSaveDir = ::GetDriveName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir());
	sAutoSaveDir.TrimRight(_T('\\'));
	sAutoSaveDir += _T("\\") + CString(APPNAME_NOEXT) + _T("\\") + sDeviceName;

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
	m_MovDetSendMailConfiguration.m_sUsername = pApp->GetSecureProfileString(sSection, _T("SendMailUsername"), _T(""));
	m_MovDetSendMailConfiguration.m_sPassword = pApp->GetSecureProfileString(sSection, _T("SendMailPassword"), _T(""));
	m_MovDetSendMailConfiguration.m_bHTML = (BOOL) pApp->GetProfileInt(sSection, _T("SendMailHTML"), TRUE);

	// FTP Settings
	m_MovDetFTPUploadConfiguration.m_sHost = pApp->GetProfileString(sSection, _T("MovDetFTPHost"), _T(""));
	m_MovDetFTPUploadConfiguration.m_sRemoteDir = pApp->GetProfileString(sSection, _T("MovDetFTPRemoteDir"), _T(""));
	m_MovDetFTPUploadConfiguration.m_nPort = (int) pApp->GetProfileInt(sSection, _T("MovDetFTPPort"), 21);
	m_MovDetFTPUploadConfiguration.m_bPasv = (BOOL) pApp->GetProfileInt(sSection, _T("MovDetFTPPasv"), TRUE);
	m_MovDetFTPUploadConfiguration.m_bProxy = (BOOL) pApp->GetProfileInt(sSection, _T("MovDetFTPProxy"), FALSE);
	m_MovDetFTPUploadConfiguration.m_sProxy = pApp->GetProfileString(sSection, _T("MovDetFTPProxyHost"), _T(""));
	m_MovDetFTPUploadConfiguration.m_sUsername = pApp->GetSecureProfileString(sSection, _T("MovDetFTPUsername"), _T(""));
	m_MovDetFTPUploadConfiguration.m_sPassword = pApp->GetSecureProfileString(sSection, _T("MovDetFTPPassword"), _T(""));
	m_MovDetFTPUploadConfiguration.m_FilesToUpload = (FilesToUploadType) pApp->GetProfileInt(sSection, _T("MovDetFilesToUpload"), FILES_TO_UPLOAD_AVI_ANIMGIF);
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
	m_bRecDeinterlace = (BOOL) pApp->GetProfileInt(sSection, _T("RecDeinterlace"), FALSE);
	m_bRecAutoOpen = (BOOL) pApp->GetProfileInt(sSection, _T("RecAutoOpen"), TRUE);
	m_bRecSizeSegmentation = (BOOL) pApp->GetProfileInt(sSection, _T("RecSizeSegmentation"), FALSE);
	m_bRecTimeSegmentation = (BOOL) pApp->GetProfileInt(sSection, _T("RecTimeSegmentation"), FALSE);
	m_nTimeSegmentationIndex = pApp->GetProfileInt(sSection, _T("TimeSegmentationIndex"), 0);
	m_nRecFileCount = (int) pApp->GetProfileInt(sSection, _T("RecFileCount"), DEFAULT_REC_AVIFILE_COUNT);
	m_llRecFileSize = ((LONGLONG)pApp->GetProfileInt(sSection, _T("RecFileSizeMB"), DEFAULT_REC_AVIFILE_SIZE_MB)) << 20;
	m_bPostRec = (BOOL) pApp->GetProfileInt(sSection, _T("PostRec"), FALSE);
	m_sRecordAutoSaveDir = pApp->GetProfileString(sSection, _T("RecordAutoSaveDir"), sAutoSaveDir);
	m_sDetectionAutoSaveDir = pApp->GetProfileString(sSection, _T("DetectionAutoSaveDir"), sAutoSaveDir);
	m_bSnapshotLiveJpeg = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotLiveJpeg"), FALSE);
	m_bSnapshotHistoryJpeg = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotHistoryJpeg"), FALSE);
	m_bSnapshotHistorySwf = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotHistorySwf"), FALSE);
	m_bSnapshotLiveJpegFtp = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotLiveJpegFtp"), FALSE);
	m_bSnapshotHistoryJpegFtp = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotHistoryJpegFtp"), FALSE);
	m_bSnapshotHistorySwfFtp = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotHistorySwfFtp"), FALSE);
	m_bSnapshotHistoryDeinterlace = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotHistoryDeinterlace"), FALSE);
	m_nSnapshotRate = (int) pApp->GetProfileInt(sSection, _T("SnapshotRate"), DEFAULT_SNAPSHOT_RATE);
	m_nSnapshotHistoryFrameRate = (int) pApp->GetProfileInt(sSection, _T("SnapshotHistoryFrameRate"), DEFAULT_SNAPSHOT_HISTORY_FRAMERATE);
	m_nSnapshotCompressionQuality = (int) pApp->GetProfileInt(sSection, _T("SnapshotCompressionQuality"), DEFAULT_SNAPSHOT_COMPR_QUALITY);
	m_fSnapshotVideoCompressorQuality = (float) pApp->GetProfileInt(sSection, _T("SnapshotVideoCompressorQuality"), (int)DEFAULT_VIDEO_QUALITY);
	m_sSnapshotAutoSaveDir = pApp->GetProfileString(sSection, _T("SnapshotAutoSaveDir"), sAutoSaveDir);
	m_bSnapshotThumb = (BOOL) pApp->GetProfileInt(sSection, _T("SnapshotThumb"), FALSE);
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
	m_dwVfWCaptureVideoDeviceID = (int) pApp->GetProfileInt(sSection, _T("VfWVideoCaptureDeviceID"), 0);
	m_nMilliSecondsRecBeforeMovementBegin = (int) pApp->GetProfileInt(sSection, _T("MilliSecondsRecBeforeMovementBegin"), DEFAULT_PRE_BUFFER_MSEC);
	m_nMilliSecondsRecAfterMovementEnd = (int) pApp->GetProfileInt(sSection, _T("MilliSecondsRecAfterMovementEnd"), DEFAULT_POST_BUFFER_MSEC);
	m_nDetectionLevel = (int) pApp->GetProfileInt(sSection, _T("DetectionLevel"), DEFAULT_MOVDET_LEVEL);
	m_bDoAdjacentZonesDetection = (BOOL) pApp->GetProfileInt(sSection, _T("DoAdjacentZonesDetection"), TRUE);
	m_bDoFalseDetectionCheck = (BOOL) pApp->GetProfileInt(sSection, _T("DoFalseDetectionCheck"), FALSE);
	m_bDoFalseDetectionAnd = (BOOL) pApp->GetProfileInt(sSection, _T("DoFalseDetectionAnd"), TRUE);
	m_nFalseDetectionBlueThreshold = (int) pApp->GetProfileInt(sSection, _T("FalseDetectionBlueThreshold"), 10);
	m_nFalseDetectionNoneBlueThreshold = (int) pApp->GetProfileInt(sSection, _T("FalseDetectionNoneBlueThreshold"), 30);
	m_bSaveSWFMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("SaveSWFMovementDetection"), TRUE);
	m_bSaveAVIMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("SaveAVIMovementDetection"), FALSE);
	m_bSaveAnimGIFMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("SaveAnimGIFMovementDetection"), TRUE);
	m_bSendMailMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("SendMailMovementDetection"), FALSE);
	m_bFTPUploadMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("FTPUploadMovementDetection"), FALSE);
	m_bExecCommandMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("DoExecCommandMovementDetection"), FALSE);
	m_sExecCommandMovementDetection = pApp->GetProfileString(sSection, _T("ExecCommandMovementDetection"), _T(""));
	m_sExecParamsMovementDetection = pApp->GetProfileString(sSection, _T("ExecParamsMovementDetection"), _T(""));
	m_bHideExecCommandMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("HideExecCommandMovementDetection"), FALSE);
	m_bWaitExecCommandMovementDetection = (BOOL) pApp->GetProfileInt(sSection, _T("WaitExecCommandMovementDetection"), FALSE);
	m_VideoProcessorMode = (DWORD) pApp->GetProfileInt(sSection, _T("VideoProcessorMode"), NO_DETECTOR);
	m_dwVideoRecFourCC = (DWORD) pApp->GetProfileInt(sSection, _T("VideoRecFourCC"), DEFAULT_VIDEOREC_FOURCC);
	m_fVideoRecQuality = (float) pApp->GetProfileInt(sSection, _T("VideoRecQuality"), (int)DEFAULT_VIDEOREC_QUALITY);
	m_nVideoRecKeyframesRate = (int) pApp->GetProfileInt(sSection, _T("VideoRecKeyframesRate"), DEFAULT_KEYFRAMESRATE);
	m_nVideoRecDataRate = (int) pApp->GetProfileInt(sSection, _T("VideoRecDataRate"), DEFAULT_VIDEO_DATARATE);
	m_nVideoRecQualityBitrate = (int) pApp->GetProfileInt(sSection, _T("VideoRecQualityBitrate"), 0);
	m_dwVideoPostRecFourCC = (DWORD) pApp->GetProfileInt(sSection, _T("VideoPostRecFourCC"), DEFAULT_VIDEOPOSTREC_FOURCC);
	m_fVideoPostRecQuality = (float) pApp->GetProfileInt(sSection, _T("VideoPostRecQuality"), (int)DEFAULT_VIDEOPOSTREC_QUALITY);
	m_nVideoPostRecKeyframesRate = (int) pApp->GetProfileInt(sSection, _T("VideoPostRecKeyframesRate"), DEFAULT_KEYFRAMESRATE);
	m_nVideoPostRecDataRate = (int) pApp->GetProfileInt(sSection, _T("VideoPostRecDataRate"), DEFAULT_VIDEO_DATARATE);
	m_nVideoPostRecQualityBitrate = (int) pApp->GetProfileInt(sSection, _T("VideoPostRecQualityBitrate"), 0);
	m_dwVideoDetFourCC = (DWORD) pApp->GetProfileInt(sSection, _T("VideoDetFourCC"), DEFAULT_VIDEODET_FOURCC);
	m_bVideoDetDeinterlace = (BOOL) pApp->GetProfileInt(sSection, _T("VideoDetDeinterlace"), FALSE);
	m_fVideoDetQuality = (float) pApp->GetProfileInt(sSection, _T("VideoDetQuality"), (int)DEFAULT_VIDEODET_QUALITY);
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
	m_dwColorDetectionWaitTime = (DWORD) pApp->GetProfileInt(sSection, _T("ColorDetectionWaitTime"), DEFAULT_COLDET_WAITTIME);
	m_dwColorDetectionAccuracy = (DWORD) pApp->GetProfileInt(sSection, _T("ColorDetectionAccuracy"), 1);
	m_bShowFrameTime = (BOOL) pApp->GetProfileInt(sSection, _T("ShowFrameTime"), TRUE);
	m_bShowMovementDetections = (BOOL) pApp->GetProfileInt(sSection, _T("ShowMovementDetections"), FALSE);
	m_nMovementDetectorIntensityLimit = (int) pApp->GetProfileInt(sSection, _T("IntensityLimit"), DEFAULT_MOVDET_INTENSITY_LIMIT);
	m_bMovementDetectorPreview = (BOOL) pApp->GetProfileInt(sSection, _T("MovementDetectorPreview"), FALSE);
	m_bColorDetectionPreview = (BOOL) pApp->GetProfileInt(sSection, _T("ColorDetectionPreview"), FALSE);
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
	SetColorDetectionWaitTime(m_dwColorDetectionWaitTime); // (Needs Effective Framerate)

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
			pApp->WriteSecureProfileString(sSection, _T("SendMailUsername"), m_MovDetSendMailConfiguration.m_sUsername);
			pApp->WriteSecureProfileString(sSection, _T("SendMailPassword"), m_MovDetSendMailConfiguration.m_sPassword);
			pApp->WriteProfileInt(sSection, _T("SendMailHTML"), m_MovDetSendMailConfiguration.m_bHTML);

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
			pApp->WriteProfileInt(sSection, _T("RecDeinterlace"), m_bRecDeinterlace);
			pApp->WriteProfileInt(sSection, _T("RecAutoOpen"), m_bRecAutoOpen);
			pApp->WriteProfileInt(sSection, _T("RecSizeSegmentation"), m_bRecSizeSegmentation);
			pApp->WriteProfileInt(sSection, _T("RecTimeSegmentation"), m_bRecTimeSegmentation);
			pApp->WriteProfileInt(sSection, _T("TimeSegmentationIndex"), m_nTimeSegmentationIndex);
			pApp->WriteProfileInt(sSection, _T("RecFileCount"), m_nRecFileCount);
			pApp->WriteProfileInt(sSection, _T("RecFileSizeMB"), (int)(m_llRecFileSize >> 20));
			pApp->WriteProfileInt(sSection, _T("PostRec"), m_bPostRec);
			pApp->WriteProfileString(sSection, _T("RecordAutoSaveDir"), m_sRecordAutoSaveDir);
			pApp->WriteProfileString(sSection, _T("DetectionAutoSaveDir"), m_sDetectionAutoSaveDir);
			pApp->WriteProfileInt(sSection, _T("SnapshotLiveJpeg"), (int)m_bSnapshotLiveJpeg);
			pApp->WriteProfileInt(sSection, _T("SnapshotHistoryJpeg"), (int)m_bSnapshotHistoryJpeg);
			pApp->WriteProfileInt(sSection, _T("SnapshotHistorySwf"), (int)m_bSnapshotHistorySwf);
			pApp->WriteProfileInt(sSection, _T("SnapshotLiveJpegFtp"), (int)m_bSnapshotLiveJpegFtp);
			pApp->WriteProfileInt(sSection, _T("SnapshotHistoryJpegFtp"), (int)m_bSnapshotHistoryJpegFtp);
			pApp->WriteProfileInt(sSection, _T("SnapshotHistorySwfFtp"), (int)m_bSnapshotHistorySwfFtp);
			pApp->WriteProfileInt(sSection, _T("SnapshotHistoryDeinterlace"), (int)m_bSnapshotHistoryDeinterlace);
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
			pApp->WriteProfileInt(sSection, _T("VfWVideoCaptureDeviceID"), m_dwVfWCaptureVideoDeviceID);
			pApp->WriteProfileInt(sSection, _T("MilliSecondsRecBeforeMovementBegin"), m_nMilliSecondsRecBeforeMovementBegin);
			pApp->WriteProfileInt(sSection, _T("MilliSecondsRecAfterMovementEnd"), m_nMilliSecondsRecAfterMovementEnd);
			pApp->WriteProfileInt(sSection, _T("DetectionLevel"), m_nDetectionLevel);
			pApp->WriteProfileInt(sSection, _T("DoAdjacentZonesDetection"), m_bDoAdjacentZonesDetection);
			pApp->WriteProfileInt(sSection, _T("DoFalseDetectionCheck"), m_bDoFalseDetectionCheck);
			pApp->WriteProfileInt(sSection, _T("DoFalseDetectionAnd"), m_bDoFalseDetectionAnd);
			pApp->WriteProfileInt(sSection, _T("FalseDetectionBlueThreshold"), m_nFalseDetectionBlueThreshold);
			pApp->WriteProfileInt(sSection, _T("FalseDetectionNoneBlueThreshold"), m_nFalseDetectionNoneBlueThreshold);
			pApp->WriteProfileInt(sSection, _T("SaveSWFMovementDetection"), m_bSaveSWFMovementDetection);
			pApp->WriteProfileInt(sSection, _T("SaveAVIMovementDetection"), m_bSaveAVIMovementDetection);
			pApp->WriteProfileInt(sSection, _T("SaveAnimGIFMovementDetection"), m_bSaveAnimGIFMovementDetection);
			pApp->WriteProfileInt(sSection, _T("SendMailMovementDetection"), m_bSendMailMovementDetection);
			pApp->WriteProfileInt(sSection, _T("FTPUploadMovementDetection"), m_bFTPUploadMovementDetection);
			pApp->WriteProfileInt(sSection, _T("DoExecCommandMovementDetection"), m_bExecCommandMovementDetection);
			pApp->WriteProfileString(sSection, _T("ExecCommandMovementDetection"), m_sExecCommandMovementDetection);
			pApp->WriteProfileString(sSection, _T("ExecParamsMovementDetection"), m_sExecParamsMovementDetection);
			pApp->WriteProfileInt(sSection, _T("HideExecCommandMovementDetection"), m_bHideExecCommandMovementDetection);
			pApp->WriteProfileInt(sSection, _T("WaitExecCommandMovementDetection"), m_bWaitExecCommandMovementDetection);
			pApp->WriteProfileInt(sSection, _T("VideoRecFourCC"), m_dwVideoRecFourCC);
			pApp->WriteProfileInt(sSection, _T("VideoRecQuality"), (int)m_fVideoRecQuality);
			pApp->WriteProfileInt(sSection, _T("VideoRecKeyframesRate"), m_nVideoRecKeyframesRate);
			pApp->WriteProfileInt(sSection, _T("VideoRecDataRate"), m_nVideoRecDataRate);
			pApp->WriteProfileInt(sSection, _T("VideoRecQualityBitrate"), m_nVideoRecQualityBitrate);
			pApp->WriteProfileInt(sSection, _T("VideoPostRecFourCC"), m_dwVideoPostRecFourCC);
			pApp->WriteProfileInt(sSection, _T("VideoPostRecQuality"), (int)m_fVideoPostRecQuality);
			pApp->WriteProfileInt(sSection, _T("VideoPostRecKeyframesRate"), m_nVideoPostRecKeyframesRate);
			pApp->WriteProfileInt(sSection, _T("VideoPostRecDataRate"), m_nVideoPostRecDataRate);
			pApp->WriteProfileInt(sSection, _T("VideoPostRecQualityBitrate"), m_nVideoPostRecQualityBitrate);
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
			pApp->WriteProfileInt(sSection, _T("ColorDetectionWaitTime"), m_dwColorDetectionWaitTime);
			pApp->WriteProfileInt(sSection, _T("ColorDetectionAccuracy"), m_dwColorDetectionAccuracy);
			pApp->WriteProfileInt(sSection, _T("ShowFrameTime"), m_bShowFrameTime);
			pApp->WriteProfileInt(sSection, _T("ShowMovementDetections"), m_bShowMovementDetections);
			pApp->WriteProfileInt(sSection, _T("IntensityLimit"), m_nMovementDetectorIntensityLimit);
			pApp->WriteProfileInt(sSection, _T("MovementDetectorPreview"), m_bMovementDetectorPreview);
			pApp->WriteProfileInt(sSection, _T("ColorDetectionPreview"), m_bColorDetectionPreview);
			pApp->WriteProfileInt(sSection, _T("AnimatedGifWidth"), m_dwAnimatedGifWidth);
			pApp->WriteProfileInt(sSection, _T("AnimatedGifHeight"), m_dwAnimatedGifHeight);
			pApp->WriteProfileInt(sSection, _T("DeleteDetectionsOlderThanDays"), m_nDeleteDetectionsOlderThanDays);
			pApp->WriteProfileInt(sSection, _T("DeleteRecordingsOlderThanDays"), m_nDeleteRecordingsOlderThanDays);
			pApp->WriteProfileInt(sSection, _T("DeleteSnapshotsOlderThanDays"), m_nDeleteSnapshotsOlderThanDays);

			pApp->WriteProfileInt(sSection, _T("MovDetTotalZones"), m_lMovDetTotalZones);
			for (int i = 0 ; i < m_lMovDetTotalZones ; i++)
			{
				CString sZone;
				sZone.Format(_T("DoMovementDetection%03i"), i);
				pApp->WriteProfileInt(sSection, sZone, m_DoMovementDetection[i]);
			}

			if (m_CaptureAudioThread.m_pSrcWaveFormat)
				pApp->WriteProfileBinary(sSection, _T("SrcWaveFormat"), (LPBYTE)m_CaptureAudioThread.m_pSrcWaveFormat, sizeof(WAVEFORMATEX));
			if (m_CaptureAudioThread.m_pDstWaveFormat)
				pApp->WriteProfileBinary(sSection, _T("DstWaveFormat"), (LPBYTE)m_CaptureAudioThread.m_pDstWaveFormat, sizeof(WAVEFORMATEX));
			pApp->WriteProfileInt(sSection, _T("VideoProcessorMode"), m_VideoProcessorMode);
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
			::WriteSecureProfileIniString(sSection, _T("SendMailUsername"), m_MovDetSendMailConfiguration.m_sUsername, sTempFileName);
			::WriteSecureProfileIniString(sSection, _T("SendMailPassword"), m_MovDetSendMailConfiguration.m_sPassword, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SendMailHTML"), m_MovDetSendMailConfiguration.m_bHTML, sTempFileName);

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
			::WriteProfileIniInt(sSection, _T("RecDeinterlace"), m_bRecDeinterlace, sTempFileName);
			::WriteProfileIniInt(sSection, _T("RecAutoOpen"), m_bRecAutoOpen, sTempFileName);
			::WriteProfileIniInt(sSection, _T("RecSizeSegmentation"), m_bRecSizeSegmentation, sTempFileName);
			::WriteProfileIniInt(sSection, _T("RecTimeSegmentation"), m_bRecTimeSegmentation, sTempFileName);
			::WriteProfileIniInt(sSection, _T("TimeSegmentationIndex"), m_nTimeSegmentationIndex, sTempFileName);
			::WriteProfileIniInt(sSection, _T("RecFileCount"), m_nRecFileCount, sTempFileName);
			::WriteProfileIniInt(sSection, _T("RecFileSizeMB"), (int)(m_llRecFileSize >> 20), sTempFileName);
			::WriteProfileIniInt(sSection, _T("PostRec"), m_bPostRec, sTempFileName);
			::WriteProfileIniString(sSection, _T("RecordAutoSaveDir"), m_sRecordAutoSaveDir, sTempFileName);
			::WriteProfileIniString(sSection, _T("DetectionAutoSaveDir"), m_sDetectionAutoSaveDir, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotLiveJpeg"), (int)m_bSnapshotLiveJpeg, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotHistoryJpeg"), (int)m_bSnapshotHistoryJpeg, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotHistorySwf"), (int)m_bSnapshotHistorySwf, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotLiveJpegFtp"), (int)m_bSnapshotLiveJpegFtp, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotHistoryJpegFtp"), (int)m_bSnapshotHistoryJpegFtp, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotHistorySwfFtp"), (int)m_bSnapshotHistorySwfFtp, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SnapshotHistoryDeinterlace"), (int)m_bSnapshotHistoryDeinterlace, sTempFileName);
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
			::WriteProfileIniInt(sSection, _T("VfWVideoCaptureDeviceID"), m_dwVfWCaptureVideoDeviceID, sTempFileName);
			::WriteProfileIniInt(sSection, _T("MilliSecondsRecBeforeMovementBegin"), m_nMilliSecondsRecBeforeMovementBegin, sTempFileName);
			::WriteProfileIniInt(sSection, _T("MilliSecondsRecAfterMovementEnd"), m_nMilliSecondsRecAfterMovementEnd, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DetectionLevel"), m_nDetectionLevel, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DoAdjacentZonesDetection"), m_bDoAdjacentZonesDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DoFalseDetectionCheck"), m_bDoFalseDetectionCheck, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DoFalseDetectionAnd"), m_bDoFalseDetectionAnd, sTempFileName);
			::WriteProfileIniInt(sSection, _T("FalseDetectionBlueThreshold"), m_nFalseDetectionBlueThreshold, sTempFileName);
			::WriteProfileIniInt(sSection, _T("FalseDetectionNoneBlueThreshold"), m_nFalseDetectionNoneBlueThreshold, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SaveSWFMovementDetection"), m_bSaveSWFMovementDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SaveAVIMovementDetection"), m_bSaveAVIMovementDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SaveAnimGIFMovementDetection"), m_bSaveAnimGIFMovementDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("SendMailMovementDetection"), m_bSendMailMovementDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("FTPUploadMovementDetection"), m_bFTPUploadMovementDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DoExecCommandMovementDetection"), m_bExecCommandMovementDetection, sTempFileName);
			::WriteProfileIniString(sSection, _T("ExecCommandMovementDetection"), m_sExecCommandMovementDetection, sTempFileName);
			::WriteProfileIniString(sSection, _T("ExecParamsMovementDetection"), m_sExecParamsMovementDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("HideExecCommandMovementDetection"), m_bHideExecCommandMovementDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("WaitExecCommandMovementDetection"), m_bWaitExecCommandMovementDetection, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoRecFourCC"), m_dwVideoRecFourCC, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoRecQuality"), (int)m_fVideoRecQuality, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoRecKeyframesRate"), m_nVideoRecKeyframesRate, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoRecDataRate"), m_nVideoRecDataRate, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoRecQualityBitrate"), m_nVideoRecQualityBitrate, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoPostRecFourCC"), m_dwVideoPostRecFourCC, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoPostRecQuality"), (int)m_fVideoPostRecQuality, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoPostRecKeyframesRate"), m_nVideoPostRecKeyframesRate, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoPostRecDataRate"), m_nVideoPostRecDataRate, sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoPostRecQualityBitrate"), m_nVideoPostRecQualityBitrate, sTempFileName);
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
			::WriteProfileIniInt(sSection, _T("ColorDetectionWaitTime"), m_dwColorDetectionWaitTime, sTempFileName);
			::WriteProfileIniInt(sSection, _T("ColorDetectionAccuracy"), m_dwColorDetectionAccuracy, sTempFileName);
			::WriteProfileIniInt(sSection, _T("ShowFrameTime"), m_bShowFrameTime, sTempFileName);
			::WriteProfileIniInt(sSection, _T("ShowMovementDetections"), m_bShowMovementDetections, sTempFileName);
			::WriteProfileIniInt(sSection, _T("IntensityLimit"), m_nMovementDetectorIntensityLimit, sTempFileName);
			::WriteProfileIniInt(sSection, _T("MovementDetectorPreview"), m_bMovementDetectorPreview, sTempFileName);
			::WriteProfileIniInt(sSection, _T("ColorDetectionPreview"), m_bColorDetectionPreview, sTempFileName);
			::WriteProfileIniInt(sSection, _T("AnimatedGifWidth"), m_dwAnimatedGifWidth, sTempFileName);
			::WriteProfileIniInt(sSection, _T("AnimatedGifHeight"), m_dwAnimatedGifHeight, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DeleteDetectionsOlderThanDays"), m_nDeleteDetectionsOlderThanDays, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DeleteRecordingsOlderThanDays"), m_nDeleteRecordingsOlderThanDays, sTempFileName);
			::WriteProfileIniInt(sSection, _T("DeleteSnapshotsOlderThanDays"), m_nDeleteSnapshotsOlderThanDays, sTempFileName);

			::WriteProfileIniInt(sSection, _T("MovDetTotalZones"), m_lMovDetTotalZones, sTempFileName);
			for (int i = 0 ; i < m_lMovDetTotalZones ; i++)
			{
				CString sZone;
				sZone.Format(_T("DoMovementDetection%03i"), i);
				::WriteProfileIniInt(sSection, sZone, m_DoMovementDetection[i], sTempFileName);
			}

			if (m_CaptureAudioThread.m_pSrcWaveFormat)
				::WriteProfileIniBinary(sSection, _T("SrcWaveFormat"), (LPBYTE)m_CaptureAudioThread.m_pSrcWaveFormat, sizeof(WAVEFORMATEX), sTempFileName);
			if (m_CaptureAudioThread.m_pDstWaveFormat)
				::WriteProfileIniBinary(sSection, _T("DstWaveFormat"), (LPBYTE)m_CaptureAudioThread.m_pDstWaveFormat, sizeof(WAVEFORMATEX), sTempFileName);
			::WriteProfileIniInt(sSection, _T("VideoProcessorMode"), m_VideoProcessorMode, sTempFileName);
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

BOOL CVideoDeviceDoc::GetCaptureDriverDescription(WORD nIndex, CString& sName, CString& sVersion)
{
	// These two Functions are not available
	// in UNICODE version under win9x!!
	typedef BOOL (VFWAPI *PFNCAPGETDRIVERDESCRIPTIONA)(	UINT wDriverIndex,
														LPSTR lpszName, int cbName,
														LPSTR lpszVer, int cbVer);
	typedef BOOL (VFWAPI *PFNCAPGETDRIVERDESCRIPTIONW)(	UINT wDriverIndex,
														LPWSTR lpszName, int cbName,
														LPWSTR lpszVer, int cbVer);
	PFNCAPGETDRIVERDESCRIPTIONA fpcapGetDriverDescriptionA = NULL;
	PFNCAPGETDRIVERDESCRIPTIONW fpcapGetDriverDescriptionW = NULL;
	HMODULE hAviCap32 = ::LoadLibrary(_T("avicap32.dll"));
	if (hAviCap32)
	{
		fpcapGetDriverDescriptionA = (PFNCAPGETDRIVERDESCRIPTIONA)(
										::GetProcAddress(hAviCap32,
														"capGetDriverDescriptionA"));
			
		fpcapGetDriverDescriptionW = (PFNCAPGETDRIVERDESCRIPTIONW)(
										::GetProcAddress(hAviCap32,
														"capGetDriverDescriptionW"));
	}

	TCHAR* pszName = sName.GetBuffer(_MAX_PATH);
	TCHAR* pszVersion = sVersion.GetBuffer(_MAX_PATH);
	BOOL bSuccess = FALSE;
#ifdef UNICODE
		if (fpcapGetDriverDescriptionW)
		{
			bSuccess = fpcapGetDriverDescriptionW(
												nIndex,
												pszName,
												_MAX_PATH,
												pszVersion,
												_MAX_PATH);
		}
		else if (fpcapGetDriverDescriptionA)
		{
			char Name[_MAX_PATH];
			char Version[_MAX_PATH];
			wcstombs(Name, pszName, _MAX_PATH);
			wcstombs(Version, pszVersion, _MAX_PATH);
			bSuccess = fpcapGetDriverDescriptionA(
												nIndex,
												Name,
												_MAX_PATH,
												Version,
												_MAX_PATH);
			mbstowcs(pszName, Name, _MAX_PATH);
			mbstowcs(pszVersion, Version, _MAX_PATH);
		}
#else
		if (fpcapGetDriverDescriptionA)
		{
			bSuccess = fpcapGetDriverDescriptionA(
													nIndex,
													pszName,
													_MAX_PATH,
													pszVersion,
													_MAX_PATH);
		}
#endif

	sName.ReleaseBuffer();
	sVersion.ReleaseBuffer();
	if (hAviCap32)
		::FreeLibrary(hAviCap32);

	return bSuccess;
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
			::SendMessage(	GetView()->GetSafeHwnd(),
							WM_THREADSAFE_CHANGEVIDEOFORMAT,
							0, 0);

			// Start capturing video data
			SetProcessFrameStopped();
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

				// Set flag
				m_bCapture = TRUE;

				// Restart process frame
				ReStartProcessFrame();

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

// nId = -1 for Vfw
// nId >= 0 are for DirectShow Devices
BOOL CVideoDeviceDoc::OpenVideoDevice(int nId)
{
	// DirectShow
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
		m_bCapture = FALSE;
		::AfxMessageBox(ML_STRING(1466, "The capture device is already in use or not compatible"), MB_ICONSTOP);
		return FALSE;
	}
	// VfW
	else
	{
		// Open
		if (!::IsWindow(m_VfWCaptureVideoThread.m_hCapWnd))
		{
			// First see if there are any devices on the system. If
			// not display a message box and abort
			int nDevicesFound = 0;
			for (WORD i = 0 ; i < 10 ; i++)
			{
				CString sName, sVersion;
				if (CVideoDeviceDoc::GetCaptureDriverDescription(i, sName, sVersion))
					++nDevicesFound;
			}
			if (nDevicesFound == 0)
			{
				::AfxMessageBox(IDS_NO_DEVICES_FOUND, MB_ICONSTOP);
				return FALSE;
			}

			if (nDevicesFound > 1)
			{
				CCaptureDeviceDlg dlg(GetView());
				if (dlg.DoModal() == IDOK)
				{
					// Load Settings
					if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
						LoadSettings(DEFAULT_FRAMERATE, _T("VfW"), _T("VfW"));

					// Start Delete Detections Thread
					if (!m_DeleteThread.IsAlive())
						m_DeleteThread.Start(THREAD_PRIORITY_LOWEST);

					// Set Device ID
					m_dwVfWCaptureVideoDeviceID = dlg.m_wDeviceID;
				
					// Reset vars
					m_dwFrameCountUp = 0U;
					m_dwNextSnapshotUpTime = ::timeGetTime();
					::InterlockedExchange(&m_lCurrentInitUpTime, (LONG)m_dwNextSnapshotUpTime);

					// Video Connect
					if (!m_VfWCaptureVideoThread.ConnectForce(m_dFrameRate))
						return FALSE;
				
					// Video Capture Start
					if (!m_VfWCaptureVideoThread.StartCapture())
						return FALSE;

					// Start Audio Capture
					if (m_bCaptureAudio)
						m_CaptureAudioThread.Start();

					// Title
					SetDocumentTitle();

					return TRUE;
				}
				else
					return FALSE;
			}
			else
			{
				// Load Settings
				if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
					LoadSettings(DEFAULT_FRAMERATE, _T("VfW"), _T("VfW"));

				// Start Delete Detections Thread
				if (!m_DeleteThread.IsAlive())
					m_DeleteThread.Start(THREAD_PRIORITY_LOWEST);

				// Set Device ID 0
				m_dwVfWCaptureVideoDeviceID = 0;
				
				// Reset vars
				m_dwFrameCountUp = 0U;
				m_dwNextSnapshotUpTime = ::timeGetTime();
				::InterlockedExchange(&m_lCurrentInitUpTime, (LONG)m_dwNextSnapshotUpTime);

				// Video Connect
				if (!m_VfWCaptureVideoThread.ConnectForce(m_dFrameRate))
					return FALSE;

				// Video Capture Start
				if (!m_VfWCaptureVideoThread.StartCapture())
					return FALSE;
			
				// Start Audio Capture
				if (m_bCaptureAudio)
					m_CaptureAudioThread.Start();

				// Title
				SetDocumentTitle();

				return TRUE;
			}
		}
		else
			return TRUE;
	}
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
		default :			return HTTPCLIENTPOLL_DEFAULT_FRAMERATE;
	}
}

// sAddress: Must have the IP:Port:FrameLocation:NetworkDeviceTypeMode or
//           HostName:Port:FrameLocation:NetworkDeviceTypeMode Format
// For http FrameLocation is m_HttpGetFrameLocations[0],
// for udp it is not used and set to _T('/') 
BOOL CVideoDeviceDoc::OpenGetVideo(CString sAddress) 
{
	// Start wait cursor
	BeginWaitCursor();

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

BOOL CVideoDeviceDoc::OpenGetVideo() 
{
	CHostPortDlg dlg;
	if (dlg.DoModal() == IDOK) // BeginWaitCursor() called by dialog
	{
		// Init Vars
		int nPos, nPosEnd;
		BOOL bUrl = FALSE;
		int nUrlPort = 80; // Default url port is always 80
		CString sGetFrameVideoHost(dlg.m_sHost);
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
		m_nGetFrameVideoPort = bUrl ? nUrlPort : dlg.m_nPort;
		m_nNetworkDeviceTypeMode = bUrl ? OTHERONE : (NetworkDeviceTypeMode)dlg.m_nDeviceTypeMode;

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
		m_dwGetFrameMaxFrames = dlg.m_dwMaxFrames;			
		m_bGetFrameDisableResend = dlg.m_bDisableResend;

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
	else
		return FALSE;
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
	memcpy(&m_OrigBMI, pDib->GetBMI(), MIN(sizeof(BITMAPINFOFULL), pDib->GetBMISize()));
	m_DocRect.right = m_OrigBMI.bmiHeader.biWidth;
	m_DocRect.bottom = m_OrigBMI.bmiHeader.biHeight;

	// Free Movement Detector because we changed size and/or format!
	FreeMovementDetector();
	ResetMovementDetector();

	// Reset Flag
	m_bDecodeFramesForPreview = FALSE;

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

void CVideoDeviceDoc::OnCaptureDeinterlace() 
{
	if (IsDeinterlaceSupported((LPBITMAPINFO)&m_OrigBMI))
		m_bDeinterlace = !m_bDeinterlace;
	else
	{
		::AfxMessageBox(ML_STRING(1467, "Live De-Interlacing is only supported with the\n") +
						ML_STRING(1468, "I420, IYUV or YV12 video formats.\n") +
						ML_STRING(1469, "De-Interlacing can still be performed at save time.\n") +
						ML_STRING(1470, "Check the De-Interlace box of the saving settings."),
						MB_ICONINFORMATION);
	}
}

void CVideoDeviceDoc::OnUpdateCaptureDeinterlace(CCmdUI* pCmdUI) 
{
	if (IsDeinterlaceSupported((LPBITMAPINFO)&m_OrigBMI))
		pCmdUI->SetCheck(m_bDeinterlace ? 1 : 0);
	else
		pCmdUI->SetCheck(0);
}

void CVideoDeviceDoc::FreeAVIFiles()
{
	for (int i = 0 ; i < m_AVRecs.GetSize() ; i++)
	{
		if (m_AVRecs[i])
		{
			if (m_AVRecs[i]->GetFrameCount(m_AVRecs[i]->VideoStreamNumToStreamNum(ACTIVE_VIDEO_STREAM)) == 0)
			{
				CString sFileName = m_AVRecs[i]->GetFileName();
				delete m_AVRecs[i];
				::DeleteFile(sFileName);
			}
			else
				delete m_AVRecs[i];
		}
	}
	m_AVRecs.RemoveAll();
}

void CVideoDeviceDoc::CheckRecDir()
{
	// Not Existing or Not A Directory?
	DWORD dwAttrib =::GetFileAttributes(m_sRecordAutoSaveDir);
	if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
	{
		// Try create it
		if (m_sRecordAutoSaveDir != _T(""))
			::CreateDir(m_sRecordAutoSaveDir);

		// Try with some standard folders
		dwAttrib =::GetFileAttributes(m_sRecordAutoSaveDir);
		if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
		{
			// Try Video directory
			CString sRecordAutoSaveDir = ::GetSpecialFolderPath(CSIDL_MYVIDEO);

			// Try My Documents if video dir not available
			if (sRecordAutoSaveDir == _T(""))
				sRecordAutoSaveDir = ::GetSpecialFolderPath(CSIDL_PERSONAL);

			// Use main drive
			if (sRecordAutoSaveDir == _T(""))
				sRecordAutoSaveDir = ::GetDriveName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir()) + _T("\\") + DEFAULT_AVIREC_DIR;
			else
			{
				sRecordAutoSaveDir.TrimRight(_T('\\'));
				sRecordAutoSaveDir += CString(_T("\\")) + CString(DEFAULT_AVIREC_DIR);
			}

			// Try create it
			dwAttrib =::GetFileAttributes(sRecordAutoSaveDir);
			if (dwAttrib == 0xFFFFFFFF || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
			{
				if (!::CreateDir(sRecordAutoSaveDir))
					return;
			}

			// Set var
			m_sRecordAutoSaveDir = sRecordAutoSaveDir;

			// Update video mode page
			if (m_pGeneralPage)
			{
				m_pGeneralPage->m_DirLabel.SetLink(sRecordAutoSaveDir);
				CEdit* pEdit = (CEdit*)m_pGeneralPage->GetDlgItem(IDC_RECORD_SAVEAS_PATH);
				pEdit->SetWindowText(sRecordAutoSaveDir);
			}
		}
	}
}

__forceinline CString CVideoDeviceDoc::MakeRecFileName()
{
	CString sFirstRecFileName;
	CTime Time = CTime::GetCurrentTime();
	CString sTime = Time.Format(_T("%Y_%m_%d_%H_%M_%S"));

	// Adjust Directory Name
	CString sRecordAutoSaveDir = m_sRecordAutoSaveDir;
	sRecordAutoSaveDir.TrimRight(_T('\\'));

	// Recording File Name (Full-Path)
	if (!CVideoDeviceDoc::CreateCheckYearMonthDayDir(Time, sRecordAutoSaveDir, sFirstRecFileName))
		return _T("");
	
	if (sFirstRecFileName == _T(""))
		return _T("rec_") + sTime + _T(".avi");
	else
		return sFirstRecFileName + _T("\\") + _T("rec_") + sTime + _T(".avi");
}

__forceinline BOOL CVideoDeviceDoc::MakeAVRec(const CString& sFileName, CAVRec** ppAVRec)
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
	SrcBmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	SrcBmi.bmiHeader.biWidth = m_DocRect.right;
	SrcBmi.bmiHeader.biHeight = m_DocRect.bottom;
	SrcBmi.bmiHeader.biPlanes = 1;
	if (m_bRgb32Frame)
	{
		SrcBmi.bmiHeader.biBitCount = 32;
		SrcBmi.bmiHeader.biCompression = BI_RGB;
		SrcBmi.bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(	SrcBmi.bmiHeader.biBitCount	*
															SrcBmi.bmiHeader.biWidth)	*
															SrcBmi.bmiHeader.biHeight;
	}
	else
		memcpy(&SrcBmi, &m_OrigBMI, CDib::GetBMISize((LPBITMAPINFO)&m_OrigBMI));
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
	if (!(*ppAVRec)->Open(m_bRecSizeSegmentation)) // if m_bRecSizeSegmentation is TRUE -> do Not Truncate File
		return FALSE;
	else
		return TRUE;
}

void CVideoDeviceDoc::AllocateCaptureFiles()
{
	// Allocate Files
	int filenum;
	for (filenum = 0 ; filenum < m_nRecFileCount ; filenum++)
	{
		// Set File Name
		CString sFileName;
		if (filenum > 0)
		{
			sFileName.Format(PART_POSTFIX, filenum + 1); 
			sFileName = ::GetFileNameNoExt(m_sFirstRecFileName) + sFileName;
		}
		else
			sFileName = m_sFirstRecFileName;

		// Create File
		HANDLE hFile = ::CreateFile(sFileName,
									GENERIC_WRITE,
									FILE_SHARE_READ,
									NULL,
									OPEN_ALWAYS,
									FILE_ATTRIBUTE_NORMAL,
									NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			LARGE_INTEGER li;
			li.QuadPart = m_llRecFileSize;
			::SetFilePointer(hFile, li.LowPart, &li.HighPart, FILE_BEGIN);
			::SetEndOfFile(hFile);
			::CloseHandle(hFile);
		}
	}

	// Remove Remaining Files
	for ( ; ; filenum++)
	{
		CString sFileName;
		sFileName.Format(PART_POSTFIX, filenum + 1); 
		sFileName = ::GetFileNameNoExt(m_sFirstRecFileName) + sFileName;
		if (::IsExistingFile(sFileName))
			::DeleteFile(sFileName);
		else
			break;
	}
}

BOOL CVideoDeviceDoc::RecError(BOOL bShowMessageBoxOnError, CAVRec* pAVRec/*=NULL*/)
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
	if (m_bCapture					&&
		!m_bVfWDialogDisplaying		&&
		!m_bAboutToStopRec			&&
		!m_bAboutToStartRec)
		CaptureRecord();
}

void CVideoDeviceDoc::OnUpdateCaptureRecord(CCmdUI* pCmdUI) 
{	
	pCmdUI->SetCheck(m_pAVRec != NULL ? 1 : 0);
	pCmdUI->Enable(	m_bCapture					&&
					!m_bVfWDialogDisplaying		&&
					!m_bAboutToStopRec			&&
					!m_bAboutToStartRec);
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

		// Check whether a rec dir has been chosen,
		// if not use desktop or temp folder
		CheckRecDir();

		// Set First Rec. File Name
		m_sFirstRecFileName = MakeRecFileName();
		if (m_sFirstRecFileName == _T(""))
			return RecError(bShowMessageBoxOnError);

		// Set next rec time for time segmentation
		if (m_bRecTimeSegmentation)
		{
			CTime t = CTime::GetCurrentTime();
			NextRecTime(t);
		}

		// Allocate Capture Files
		if (m_bRecSizeSegmentation)
			AllocateCaptureFiles();

		// Make Sure Array is empty!
		FreeAVIFiles();
		m_pAVRec = NULL;

		// Reset Pos
		m_nRecFilePos = 0;
		
		// Set File Count
		int nRecFileCount = m_nRecFileCount;
		if (!m_bRecSizeSegmentation)
			nRecFileCount = 1;

		// Init File(s)
		for (int filenum = 0 ; filenum < nRecFileCount ; filenum++)
		{
			// Set File Name
			CString sFileName;
			if (filenum > 0)
			{
				sFileName.Format(PART_POSTFIX, filenum + 1); 
				sFileName = ::GetFileNameNoExt(m_sFirstRecFileName) + sFileName;
			}
			else
				sFileName = m_sFirstRecFileName;

			// Allocate & Init pAVRec
			CAVRec* pAVRec = NULL;
			if (!MakeAVRec(sFileName, &pAVRec))
				return RecError(bShowMessageBoxOnError, pAVRec);

			// Add To Array
			m_AVRecs.Add(pAVRec);
		}

		// Set Current AV Rec Pointer
		m_pAVRec = m_AVRecs[0];

		// Start Recording
		m_bRecFirstFrame = TRUE;

		// Do Not Wait Audio, if No Audio
		if (!m_bCaptureAudio)
			m_bVideoRecWait = FALSE;

		// Leave CS
		::LeaveCriticalSection(&m_csAVRec);

		// Disable Critical Controls
		::PostMessage(	GetView()->GetSafeHwnd(),
						WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
						(WPARAM)FALSE,	// Disable Them
						(LPARAM)0);

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

BOOL CVideoDeviceDoc::SaveModified() 
{
	if (m_bVfWDialogDisplaying)
	{
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
		::AfxMessageBox(ML_STRING(1472, "Please close the VfW dialog before exiting!"));
		if (pActiveView)
			pActiveView->ForceCursor(FALSE);
		return FALSE; // Do not exit
	}
	else
		return TRUE;
}

void CVideoDeviceDoc::VfWVideoFormatDialog() 
{
	if (::IsWindow(m_VfWCaptureVideoThread.m_hCapWnd))
	{
		// Hack To Force the Change of the FrameRate!
		if (m_dFrameRate < MAX_FRAMERATE)
		{
			m_VfWCaptureVideoThread.ConnectForce(MAX_FRAMERATE);
			capSetCallbackOnVideoStream(m_VfWCaptureVideoThread.m_hCapWnd, NULL);
			capCaptureSequenceNoFile(m_VfWCaptureVideoThread.m_hCapWnd);
			capCaptureStop(m_VfWCaptureVideoThread.m_hCapWnd);
			capCaptureAbort(m_VfWCaptureVideoThread.m_hCapWnd);
		}
		m_VfWCaptureVideoThread.ConnectForce(MAX_FRAMERATE); // Reconnect
		
		m_VfWCaptureVideoThread.StartTriggeredFrameCapture();
		m_bVfWDialogDisplaying = TRUE;
		capDlgVideoFormat(m_VfWCaptureVideoThread.m_hCapWnd);
		m_bVfWDialogDisplaying = FALSE;
		m_bSizeToDoc = TRUE;
		OnChangeVideoFormat();
		m_VfWCaptureVideoThread.StopTriggeredFrameCapture();
		m_VfWCaptureVideoThread.ConnectForce(m_dFrameRate); // Reconnect
		m_VfWCaptureVideoThread.StartCapture();
	}
}

void CVideoDeviceDoc::VfWVideoSourceDialog() 
{
	if (::IsWindow(m_VfWCaptureVideoThread.m_hCapWnd))
	{
		// Hack To Force the Change of the FrameRate!
		if (m_dFrameRate < MAX_FRAMERATE)
		{
			m_VfWCaptureVideoThread.ConnectForce(MAX_FRAMERATE);
			capSetCallbackOnVideoStream(m_VfWCaptureVideoThread.m_hCapWnd, NULL);
			capCaptureSequenceNoFile(m_VfWCaptureVideoThread.m_hCapWnd);
			capCaptureStop(m_VfWCaptureVideoThread.m_hCapWnd);
			capCaptureAbort(m_VfWCaptureVideoThread.m_hCapWnd);
		}
		m_VfWCaptureVideoThread.ConnectForce(MAX_FRAMERATE); // Reconnect
		
		m_VfWCaptureVideoThread.StartTriggeredFrameCapture();
		m_bVfWDialogDisplaying = TRUE;
		capDlgVideoSource(m_VfWCaptureVideoThread.m_hCapWnd);
		m_bVfWDialogDisplaying = FALSE;
		m_bSizeToDoc = TRUE;
		OnChangeVideoFormat();
		m_VfWCaptureVideoThread.StopTriggeredFrameCapture();
		m_VfWCaptureVideoThread.ConnectForce(m_dFrameRate); // Reconnect
		m_VfWCaptureVideoThread.StartCapture();
	}
}

// Function called from the UI thread and when ProcessFrame() is not called
void CVideoDeviceDoc::OnChangeVideoFormat()
{
	DWORD dwSize;

	if (::IsWindow(m_VfWCaptureVideoThread.m_hCapWnd))
	{
		capGetVideoFormat(	m_VfWCaptureVideoThread.m_hCapWnd, &m_OrigBMI,
							MIN(sizeof(BITMAPINFOFULL), capGetVideoFormatSize(m_VfWCaptureVideoThread.m_hCapWnd)));
		m_DocRect.right = m_OrigBMI.bmiHeader.biWidth;
		m_DocRect.bottom = m_OrigBMI.bmiHeader.biHeight;
		SetDocumentTitle();
	}
	else if (m_pDxCapture)
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
				m_OrigBMI.bmiHeader.biSize =			sizeof(BITMAPINFOHEADER);
				m_OrigBMI.bmiHeader.biWidth =			(LONG)nWidth;
				m_OrigBMI.bmiHeader.biHeight =			(LONG)nHeight;
				m_OrigBMI.bmiHeader.biPlanes =			1;
				m_OrigBMI.bmiHeader.biCompression =		FCC('YUY2');
				m_OrigBMI.bmiHeader.biBitCount =		16;
				int stride = ::CalcYUVStride(m_OrigBMI.bmiHeader.biCompression, (int)m_OrigBMI.bmiHeader.biWidth);
				m_OrigBMI.bmiHeader.biSizeImage = ::CalcYUVSize(m_OrigBMI.bmiHeader.biCompression, stride, (int)m_OrigBMI.bmiHeader.biHeight);
				m_DocRect.right = m_OrigBMI.bmiHeader.biWidth;
				m_DocRect.bottom = m_OrigBMI.bmiHeader.biHeight;
				SetDocumentTitle();
			}
		}
		else
		{
			AM_MEDIA_TYPE* pmtConfig = NULL;
			if (!m_pDxCapture->GetCurrentFormat(&pmtConfig))
				return;
			if (pmtConfig->formattype != FORMAT_VideoInfo	||
				!pmtConfig->pbFormat						||
				pmtConfig->cbFormat < sizeof(VIDEOINFOHEADER))
			{
				m_pDxCapture->DeleteMediaType(pmtConfig);
				return;
			}
			dwSize = MIN(sizeof(BITMAPINFOFULL), pmtConfig->cbFormat - SIZE_PREHEADER);
			VIDEOINFOHEADER* pVideoHeader = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
			memcpy(&m_OrigBMI, HEADER(pVideoHeader), dwSize);
			m_DocRect.right = m_OrigBMI.bmiHeader.biWidth;
			m_DocRect.bottom = m_OrigBMI.bmiHeader.biHeight;
			SetDocumentTitle();
			m_pDxCapture->DeleteMediaType(pmtConfig);
		}
	}
	else
		return;

	// Free Movement Detector because we changed size and/or format!
	FreeMovementDetector();
	ResetMovementDetector();

	// Reset Flag
	m_bDecodeFramesForPreview = FALSE;

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
		if (::IsWindow(m_VfWCaptureVideoThread.m_hCapWnd))
		{
			m_VfWCaptureVideoThread.Disconnect();
			m_VfWCaptureVideoThread.DestroyCaptureWnd();
			ResetMovementDetector();
			m_ColorDetection.ResetCounter();
			SetColorDetectionWaitTime(m_dwColorDetectionWaitTime); // Call it because frame rate changed!
			m_VfWCaptureVideoThread.ConnectForce(m_dFrameRate);
			ReStartProcessFrame();
			m_VfWCaptureVideoThread.StartCapture();
			SetDocumentTitle();
		}
		else if (m_pDxCapture)
		{
			if (m_pDxCapture->Stop())
				m_bCapture = FALSE;
			ResetMovementDetector();
			m_ColorDetection.ResetCounter();
			SetColorDetectionWaitTime(m_dwColorDetectionWaitTime); // Call it because frame rate changed!
			m_pDxCapture->SetFrameRate(m_dFrameRate);
			if (m_pDxCapture->Run())
			{
				// Some devices need that...
				// Process frame must still be stopped when calling Dx Stop()!
				m_pDxCapture->Stop();
				m_pDxCapture->Run();

				// Set flag
				m_bCapture = TRUE;

				// Restart process frame
				ReStartProcessFrame();
			}
			SetDocumentTitle();
		}
		else if (m_pGetFrameNetCom && m_pGetFrameNetCom->IsClient())
		{
			ResetMovementDetector();
			m_ColorDetection.ResetCounter();
			SetColorDetectionWaitTime(m_dwColorDetectionWaitTime); // Call it because frame rate changed!
			if (m_pHttpGetFrameParseProcess->m_FormatType == CHttpGetFrameParseProcess::FORMATMJPEG)
			{
				if (m_nNetworkDeviceTypeMode == CVideoDeviceDoc::EDIMAX_SP)
					m_pHttpGetFrameParseProcess->m_bSetFramerate = TRUE;
				ConnectGetFrameHTTP(m_sGetFrameVideoHost, m_nGetFrameVideoPort);
			}
			ReStartProcessFrame();
			SetDocumentTitle();
		}
	}
}

void CVideoDeviceDoc::ResetMovementDetector()
{
	m_bFirstMovementDetection = TRUE;			
	m_bDetectingMovement = FALSE;
	m_nMilliSecondsSinceMovementDetection = 0;
	m_nMilliSecondsWithoutMovementDetection = 0;
	m_nMilliSecondsBeforeMovementDetection = 0;
	m_nBlueMovementDetectionsCount = 0;
	m_nNoneBlueMovementDetectionsCount = 0;
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

void CVideoDeviceDoc::OnCaptureSettings() 
{
	CaptureSettings();
}

void CVideoDeviceDoc::OnUpdateCaptureSettings(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(	m_pDxCapture														||
					::IsWindow(m_VfWCaptureVideoThread.m_hCapWnd)						||
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
				// Disable Critical Controls
				::SendMessage(	GetView()->GetSafeHwnd(),
								WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
								(WPARAM)FALSE,	// Disable Them
								(LPARAM)0);
				m_bStopAndChangeFormat = TRUE;
				StopProcessFrame();
				double dFrameRate = m_dEffectiveFrameRate;
				int delay;
				if (dFrameRate >= 1.0)
					delay = Round(1000.0 / dFrameRate); // In ms
				else
					delay = 1000;
				CPostDelayedMessageThread::PostDelayedMessage(	GetView()->GetSafeHwnd(),
																WM_THREADSAFE_STOP_AND_CHANGEVIDEOFORMAT,
																delay, 0, delay);
			}
		}
		else
		{
			// Disable Critical Controls
			::SendMessage(	GetView()->GetSafeHwnd(),
							WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
							(WPARAM)FALSE,	// Disable Them
							(LPARAM)0);

			// Same stop processing and change format mechanism
			// is integrated inside CDxVideoFormatDlg
			CDxVideoFormatDlg dlg(this);
			dlg.DoModal();
			
			// Enable Critical Controls
			::SendMessage(	GetView()->GetSafeHwnd(),
							WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
							(WPARAM)TRUE,	// Enable Them
							(LPARAM)0);
		}
	}
	else if (::IsWindow(m_VfWCaptureVideoThread.m_hCapWnd))
	{
		// Disable Critical Controls
		::SendMessage(	GetView()->GetSafeHwnd(),
						WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
						(WPARAM)FALSE,	// Disable Them
						(LPARAM)0);

		// Stop processing and change format mechanism
		// not necessary because ProcessFrame() is called
		// from the main UI thread
		VfWVideoFormatDialog();
		
		// Enable Critical Controls
		::SendMessage(	GetView()->GetSafeHwnd(),
						WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
						(WPARAM)TRUE,	// Enable Them
						(LPARAM)0);
	}
	else if (m_pGetFrameNetCom && m_pGetFrameNetCom->IsClient())
	{
		// Disable Critical Controls
		::SendMessage(	GetView()->GetSafeHwnd(),
						WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
						(WPARAM)FALSE,	// Disable Them
						(LPARAM)0);

		// Stop processing and change format mechanism
		// not necessary because the size change
		// is detected when it happens!
		CHttpVideoFormatDlg dlg(this);
		dlg.DoModal();
		
		// Enable Critical Controls
		::SendMessage(	GetView()->GetSafeHwnd(),
						WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
						(WPARAM)TRUE,	// Enable Them
						(LPARAM)0);
	}
}

void CVideoDeviceDoc::VideoSourceDialog() 
{
	if (m_pDxCapture)
	{
		// Disable Critical Controls
		::SendMessage(	GetView()->GetSafeHwnd(),
						WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
						(WPARAM)FALSE,	// Disable Them
						(LPARAM)0);

		m_pDxCapture->ShowVideoCaptureFilterDlg();

		// Enable Critical Controls
		::SendMessage(	GetView()->GetSafeHwnd(),
						WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
						(WPARAM)TRUE,	// Enable Them
						(LPARAM)0);
	}
	else
	{
		// Disable Critical Controls
		::SendMessage(	GetView()->GetSafeHwnd(),
						WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
						(WPARAM)FALSE,	// Disable Them
						(LPARAM)0);

		VfWVideoSourceDialog();

		// Enable Critical Controls
		::SendMessage(	GetView()->GetSafeHwnd(),
						WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
						(WPARAM)TRUE,	// Enable Them
						(LPARAM)0);
	}
}

void CVideoDeviceDoc::VideoInputDialog() 
{
	if (m_pDxCapture)
	{
		CDxVideoInputDlg dlg(this);
		dlg.DoModal();
	}
}

void CVideoDeviceDoc::VideoTunerDialog() 
{
	if (m_pDxCapture)
		m_pDxCapture->ShowVideoTVTunerDlg();
}

void CVideoDeviceDoc::ViewVideo() 
{
	m_bVideoView = !m_bVideoView;
	if (!m_bVideoView)
		m_bDecodeFramesForPreview = FALSE;	// Reset Flag
}

void CVideoDeviceDoc::OnViewVideo() 
{
	ViewVideo();
}

void CVideoDeviceDoc::OnUpdateViewVideo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bCapture);
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
		::AfxMessageBox(ML_STRING(1476, "Please configure a directory in the Assistant and Settings dialog"));
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
		
		CString sDir = ::GetDriveAndDirName(sMicroapacheConfigFile);
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

void CVideoDeviceDoc::MicroApacheInitShutdown()
{
	::EnumKillProcByName(MICROAPACHE_FILENAME, TRUE);
}

BOOL CVideoDeviceDoc::MicroApacheFinishShutdown()
{
	BOOL res = TRUE;
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
	::DeleteFile(CVideoDeviceDoc::MicroApacheGetLogFileName()); // Avoid growing it to much!
	::DeleteFile(CVideoDeviceDoc::MicroApacheGetPidFileName());
	return res;
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

void CVideoDeviceDoc::SetColorDetectionWaitTime(DWORD dwWaitMilliseconds)
{
	m_dwColorDetectionWaitTime = dwWaitMilliseconds;
	if (m_dEffectiveFrameRate > 0.0)
		m_ColorDetection.SetWaitCount((DWORD)Round((double)dwWaitMilliseconds * m_dEffectiveFrameRate / 1000.0));
	else
		m_ColorDetection.SetWaitCount((DWORD)Round((double)dwWaitMilliseconds * m_dFrameRate / 1000.0));
}

BOOL CVideoDeviceDoc::DecodeFrameToRgb32(LPBYTE pSrcBits, DWORD dwSrcSize, CDib* pDstDib)
{
	if (!pSrcBits || (dwSrcSize == 0) || !pDstDib)
		return FALSE;

	// Is YUV?
	if (::IsSupportedYuvToRgbFormat(m_OrigBMI.bmiHeader.biCompression))
	{
		// De-Interlace Supported?
		BOOL bDoDeinterlace = (m_bDeinterlace && IsDeinterlaceSupported((LPBITMAPINFO)&m_OrigBMI));
		CDib* pTmpDib = NULL;
		if (bDoDeinterlace)
		{
			pTmpDib = new CDib;
			if (!pTmpDib)
				return FALSE;
			pTmpDib->SetShowMessageBoxOnError(FALSE);
			if (Deinterlace(pTmpDib, (LPBITMAPINFO)&m_OrigBMI, pSrcBits))
				pSrcBits = pTmpDib->GetBits();
		}

		// Allocate Bits
		if (!pDstDib->AllocateBitsFast(	32,
										BI_RGB,
										m_OrigBMI.bmiHeader.biWidth,
										m_OrigBMI.bmiHeader.biHeight))
		{
			if (pTmpDib)
				delete pTmpDib;
			return FALSE;
		}

		// Decode
		if (!::YUVToRGB32(	m_OrigBMI.bmiHeader.biCompression,
							pSrcBits,
							pDstDib->GetBits(),
							pDstDib->GetWidth(),
							pDstDib->GetHeight()))
		{
			if (pTmpDib)
				delete pTmpDib;
			return FALSE;
		}

		// Free
		if (pTmpDib)
			delete pTmpDib;
	}
	else
	{
		// Set BMI & Bits
		if (!pDstDib->SetBMI((LPBITMAPINFO)&m_OrigBMI))
			return FALSE;
		if (!pDstDib->SetBits(pSrcBits, dwSrcSize))
			return FALSE;

		// Decompress to 32 bpp
		if (pDstDib->IsCompressed())
		{
			if (!pDstDib->Decompress(32))
				return FALSE;
		}
		else if (pDstDib->GetBitCount() != 32)
		{		
			if (!pDstDib->ConvertTo32bits())
				return FALSE;
		}
	}

	return TRUE;
}

__forceinline BOOL CVideoDeviceDoc::IsDeinterlaceSupported(LPBITMAPINFO pBmi)
{
	if (pBmi)
	{
		PixelFormat src_pix_fmt = CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pBmi);
		if (((pBmi->bmiHeader.biWidth & 3) == 0)	&&
			((pBmi->bmiHeader.biHeight & 3) == 0))
		{
			if (src_pix_fmt == PIX_FMT_YUV420P ||
				src_pix_fmt == PIX_FMT_YUV422P ||
				src_pix_fmt == PIX_FMT_YUV444P ||
				src_pix_fmt == PIX_FMT_YUV411P)
				return TRUE;
		}
	}
	return FALSE;
}

__forceinline BOOL CVideoDeviceDoc::IsDeinterlaceSupported(CDib* pDib)
{
	if (pDib)
	{
		PixelFormat src_pix_fmt = CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pDib->GetBMI());
		if (((pDib->GetWidth() & 3) == 0)	&&
			((pDib->GetHeight() & 3) == 0))
		{
			if (src_pix_fmt == PIX_FMT_YUV420P ||
				src_pix_fmt == PIX_FMT_YUV422P ||
				src_pix_fmt == PIX_FMT_YUV444P ||
				src_pix_fmt == PIX_FMT_YUV411P)
				return TRUE;
		}
	}
	return FALSE;
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

BOOL CVideoDeviceDoc::Deinterlace(CDib* pDstDib, LPBITMAPINFO pSrcBMI, LPBYTE pSrcBits)
{
	// Check
	if (!pDstDib || !pSrcBMI || !pSrcBits)
		return FALSE;

	// Get Pix Format
	PixelFormat pix_fmt = CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pSrcBMI);

	// Allocate Dst Bits
	pDstDib->AllocateBitsFast(	pSrcBMI->bmiHeader.biBitCount,
								pSrcBMI->bmiHeader.biCompression,
								pSrcBMI->bmiHeader.biWidth,
								pSrcBMI->bmiHeader.biHeight);

	// Set Bits to Frames
	AVPicture DstFrame;
	AVPicture SrcFrame;
	avpicture_fill(	&DstFrame,
					(uint8_t*)pDstDib->GetBits(),
					pix_fmt,
					pSrcBMI->bmiHeader.biWidth,
					pSrcBMI->bmiHeader.biHeight);
	avpicture_fill(	&SrcFrame,
					(uint8_t*)pSrcBits,
					pix_fmt,
					pSrcBMI->bmiHeader.biWidth,
					pSrcBMI->bmiHeader.biHeight);

	// De-Interlace
	return (avpicture_deinterlace(	&DstFrame,
									&SrcFrame,
									pix_fmt,
									pSrcBMI->bmiHeader.biWidth,
									pSrcBMI->bmiHeader.biHeight) >= 0);
}

BOOL CVideoDeviceDoc::ProcessFrame(LPBYTE pData, DWORD dwSize)
{
	// A critical section is necessary because some devices may still have
	// an old thread running when re-opening. It follows that ProcessFrame()
	// may be called from two different threads at the same time!
	if (!m_csProcessFrame.TryEnterCriticalSection())
	{
		CString sMsg;
		sMsg.Format(_T("%s is already in process frame, not entering a second time\n"), GetDeviceName());
		TRACE(sMsg);
		::LogLine(sMsg);
		return FALSE;
	}

	// Do Stop ProcessFrame?
	if (m_bStopProcessFrame)
		SetProcessFrameStopped();

	// Timing
	DWORD dwCurrentFrameTime;
	DWORD dwPrevInitUpTime = (DWORD)m_lCurrentInitUpTime;
	CTime CurrentTime = CTime::GetCurrentTime();
	DWORD dwCurrentInitUpTime = ::timeGetTime();

	// Decode, Detect, Copy, Snapshot, Record, Send over UDP Network and finally Draw
	if (!m_bProcessFrameStopped && pData && dwSize > 0)
	{
		// Init Vars
		BOOL bRgb32Frame;
		DWORD VideoProcessorMode = m_VideoProcessorMode;
		BOOL bMovementDetectorPreview = m_bMovementDetectorPreview;
		BOOL bColorDetectionPreview = m_bColorDetectionPreview;
		BOOL bOk;
		BOOL bShowFrameTime = m_bShowFrameTime;
		BOOL bDecodeToRgb32 = FALSE;
		BOOL bAVCodecSrcFormatSupport = TRUE;
		if (CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat((LPBITMAPINFO)&m_OrigBMI) == PIX_FMT_NONE)
			bAVCodecSrcFormatSupport = FALSE;
		BOOL bIsAddSingleLineSupported = CDib::IsAddSingleLineTextSupported((LPBITMAPINFO)&m_OrigBMI);
		if ((VideoProcessorMode & COLOR_DETECTOR)			||
			(bShowFrameTime && !bIsAddSingleLineSupported)	||
			m_bDecodeFramesForPreview						||
			!bAVCodecSrcFormatSupport)
		{
			if ((m_OrigBMI.bmiHeader.biBitCount != 24 &&
				m_OrigBMI.bmiHeader.biBitCount != 32) ||
				m_OrigBMI.bmiHeader.biCompression != BI_RGB)
				bDecodeToRgb32 = TRUE;
		}

		// Allocate Dib
		CDib* pDib = (CDib*)new CDib;
		if (!pDib)
			goto exit;
		pDib->SetShowMessageBoxOnError(FALSE);

		// Decode Rgb (other than 24bpp and 32bpp) or Yuv to Rgb32
		if (bDecodeToRgb32)
		{
			// Decode Frame (De-Interlace inside this function)
			if (!DecodeFrameToRgb32(pData, dwSize, pDib))
			{
				delete pDib;
				goto exit;
			}
		}
		// No decode
		else
		{
			// Copy Bits
			if (!pDib->SetBMI((LPBITMAPINFO)&m_OrigBMI))
			{
				delete pDib;
				goto exit;
			}
			if (!pDib->SetBits(pData, dwSize))
			{
				delete pDib;
				goto exit;
			}

			// De-Interlace
			if (m_bDeinterlace && IsDeinterlaceSupported((LPBITMAPINFO)&m_OrigBMI))
				Deinterlace(pDib);
		}

		// Set Rgb32 Frame Flag
		if ((pDib->GetBitCount() == 32) && (pDib->GetCompression() == BI_RGB))
		{
			bRgb32Frame = TRUE;
			if (!m_bRgb32Frame)
			{
				m_bRgb32Frame = TRUE;
				::PostMessage(	GetView()->GetSafeHwnd(),
								WM_THREADSAFE_SETDOCUMENTTITLE,
								0, 0);
			}
		}
		else 
		{
			bRgb32Frame = FALSE;
			if (m_bRgb32Frame)
			{
				m_bRgb32Frame = FALSE;
				::PostMessage(	GetView()->GetSafeHwnd(),
								WM_THREADSAFE_SETDOCUMENTTITLE,
								0, 0);
			}
		}
		
		// Set the UpTime Var
		pDib->SetUpTime(dwCurrentInitUpTime);

		// Color Detection
		if ((VideoProcessorMode & COLOR_DETECTOR) &&
			(bRgb32Frame || (m_OrigBMI.bmiHeader.biCompression == BI_RGB && m_OrigBMI.bmiHeader.biBitCount == 24)))
			ColorDetectionProcessing(pDib, bColorDetectionPreview);

		// Movement Detection
		BOOL bDoDetection = VideoProcessorMode & MOVEMENT_DETECTOR;
		if (bDoDetection && m_bDetectionStartStop) // Detection Scheduler
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
					bDoDetection = FALSE;
			}
			else
			{
				if (timeonly < m_DetectionStartTime && timeonly > m_DetectionStopTime)
					bDoDetection = FALSE;
			}
		}
		if (bDoDetection && m_bFirstMovementDetection && m_pMovementDetectorBackgndDib)
		{
			delete m_pMovementDetectorBackgndDib;
			m_pMovementDetectorBackgndDib = NULL;
		}
		MovementDetectionProcessing(pDib,
									bMovementDetectorPreview,
									bDoDetection);
		if (!bDoDetection && !m_bFirstMovementDetection)
			m_bFirstMovementDetection = TRUE;

		// Copy to Clipboard
		if (m_bDoEditCopy)
			EditCopy(pDib, CurrentTime);

		// Snapshot
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
				DWORD dwFrameRate = (DWORD)Round(m_dEffectiveFrameRate);
				if (dwFrameRate == 0U)
					dwFrameRate = (DWORD)m_dFrameRate;
				if (dwFrameRate == 0U)
					dwFrameRate = 1U;
				if (bOk					&&
					!m_bCaptureAudio	&&
					(m_dwFrameCountUp % dwFrameRate) == 0)
				{
					if (m_bRecSizeSegmentation)
					{
						LONGLONG llTotalWrittenBytes =
						m_pAVRec->GetTotalWrittenBytes(m_pAVRec->VideoStreamNumToStreamNum(ACTIVE_VIDEO_STREAM));
						llTotalWrittenBytes += (llTotalWrittenBytes / 5); // Guess a 20% of AVI Overhead (Index, AVI Hdrs and Data Chunk Hdrs)
						if (llTotalWrittenBytes > m_llRecFileSize)
							bOk = NextAviFile();
					}
					else if (m_bRecTimeSegmentation)
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
			DWORD dwFrameRate = (DWORD)Round(m_dEffectiveFrameRate);
			if (dwFrameRate == 0U)
				dwFrameRate = (DWORD)m_dFrameRate;
			if (dwFrameRate == 0U)
				dwFrameRate = 1U;
			if ((m_dwFrameCountUp % dwFrameRate) == 0)
			{
				// Get Tx Fifo Size
				::EnterCriticalSection(&m_csSendFrameNetCom);
				int nTxFifoSize = 0;
				if (m_pSendFrameNetCom)
					nTxFifoSize = (int)m_pSendFrameNetCom->GetTxFifoSize();
				::LeaveCriticalSection(&m_csSendFrameNetCom);

				// Update Frame Send To Table and Flow Control
				UpdateFrameSendToTableAndFlowControl();

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

		// Update m_pDib pointer with new one
		::EnterCriticalSection(&m_csDib);
		delete m_pDib;
		m_pDib = pDib;
		::LeaveCriticalSection(&m_csDib);

		// Draw
		HRESULT hr = ::CoInitialize(NULL);
		BOOL bCleanupCOM = ((hr == S_OK) || (hr == S_FALSE));
		GetView()->Draw();
		if (bCleanupCOM)
			::CoUninitialize();

		// Set started flag and open the Settings dialog
		// if it's the first run of this device (leave this code
		// here because m_pDib must be initialized when setting
		// the m_bCaptureStarted flag otherwise the Draw()
		// function inside the watch dog doesn't work correctly!)
		if (!m_bCaptureStarted)
		{
			::InterlockedExchange(&m_bCaptureStarted, 1);
			if (m_bDeviceFirstRun)
			{
				::PostMessage(	GetView()->GetSafeHwnd(),
								WM_THREADSAFE_CAPTURESETTINGS,
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

	// Do Stop ProcessFrame?
	if (m_bStopProcessFrame)
		SetProcessFrameStopped();

	// Exiting from here
	m_csProcessFrame.LeaveCriticalSection();

	return m_bProcessFrameStopped;
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
	if (pDib->IsCompressed())
	{
		if (!DecodeFrameToRgb32(pDib->GetBits(),
								pDib->GetImageSize(),
								&m_SaveSnapshotThread.m_Dib))
			return FALSE;
		m_SaveSnapshotThread.m_Dib.SetUpTime(dwUpTime);
	}
	else
		m_SaveSnapshotThread.m_Dib = *pDib;

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
	m_SaveSnapshotThread.m_Config = m_SnapshotFTPUploadConfiguration;
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
	if (pDib->IsCompressed())
	{
		if (!DecodeFrameToRgb32(pDib->GetBits(),
								pDib->GetImageSize(),
								&Dib))
		{
			m_bDoEditCopy = FALSE;
			m_bDoEditPaste = FALSE;
			return FALSE;
		}
		Dib.SetUpTime(dwUpTime);
	}
	else
		Dib = *pDib;

	// Add frame time
	if (m_bShowFrameTime)
		CAVRec::AddFrameTime(&Dib, Time, dwUpTime);
	
	// Copy to clipboard
	Dib.EditCopy();

	// Paste from clipboard
	if (m_bDoEditPaste)
		::PostMessage(::AfxGetMainFrame()->GetSafeHwnd(), WM_COMMAND, ID_EDIT_PASTE, 0);

	// Clear flags
	m_bDoEditCopy = FALSE;
	m_bDoEditPaste = FALSE;

	return TRUE;
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

__forceinline void CVideoDeviceDoc::ChangeRecFileFrameRate(double dFrameRate/*=0.0*/)
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
	CAVIPlay::AviChangeVideoFrameRate(	(LPCTSTR)m_sFirstRecFileName,
										0,
										dFrameRate,
										false);
	for (int i = 1 ; i <= m_nRecFilePos ; i++)
	{
		CString sNextFileName;
		sNextFileName.Format(PART_POSTFIX, i + 1); 
		sNextFileName = ::GetFileNameNoExt(m_sFirstRecFileName) + sNextFileName;
		CAVIPlay::AviChangeVideoFrameRate(	(LPCTSTR)sNextFileName,
											0,
											dFrameRate,
											false);
	}
}

__forceinline void CVideoDeviceDoc::OpenAndPostProcess()
{
	if (m_bPostRec)
	{
		CPostRecParams* pPostRecParams = new CPostRecParams;
		if (pPostRecParams)
		{
			pPostRecParams->m_sSaveFileName = ::GetFileNameNoExt(m_sFirstRecFileName) + POSTREC_POSTFIX + _T(".avi");
			pPostRecParams->m_dwVideoCompressorFourCC = m_dwVideoPostRecFourCC;
			pPostRecParams->m_fVideoCompressorQuality = m_fVideoPostRecQuality;
			pPostRecParams->m_nVideoCompressorDataRate = m_nVideoPostRecDataRate;
			pPostRecParams->m_nVideoCompressorKeyframesRate = m_nVideoPostRecKeyframesRate;
			pPostRecParams->m_nVideoCompressorQualityBitrate = m_nVideoPostRecQualityBitrate;
			pPostRecParams->m_bDeinterlace = FALSE;
			pPostRecParams->m_bCloseWhenDone = !(m_bRecAutoOpen && m_bRecAutoOpenAllowed);
			::PostMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
							WM_THREADSAFE_OPEN_DOC,
							(WPARAM)(new CString(m_sFirstRecFileName)),
							(LPARAM)pPostRecParams);
		}
	}
	else if (m_bRecAutoOpen && m_bRecAutoOpenAllowed)
	{
		::PostMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
						WM_THREADSAFE_OPEN_DOC,
						(WPARAM)(new CString(m_sFirstRecFileName)),
						(LPARAM)NULL);
	}
	for (int i = 1 ; i <= m_nRecFilePos ; i++)
	{
		CString sPart;
		sPart.Format(PART_POSTFIX, i + 1); 
		CString sNextFileName = ::GetFileNameNoExt(m_sFirstRecFileName) + sPart;
		if (m_bPostRec)
		{
			CPostRecParams* pPostRecParams = new CPostRecParams;
			if (pPostRecParams)
			{
				pPostRecParams->m_sSaveFileName = ::GetFileNameNoExt(m_sFirstRecFileName) + POSTREC_POSTFIX + sPart;
				pPostRecParams->m_dwVideoCompressorFourCC = m_dwVideoPostRecFourCC;
				pPostRecParams->m_fVideoCompressorQuality = m_fVideoPostRecQuality;
				pPostRecParams->m_nVideoCompressorDataRate = m_nVideoPostRecDataRate;
				pPostRecParams->m_nVideoCompressorKeyframesRate = m_nVideoPostRecKeyframesRate;
				pPostRecParams->m_nVideoCompressorQualityBitrate = m_nVideoPostRecQualityBitrate;
				pPostRecParams->m_bDeinterlace = FALSE;
				pPostRecParams->m_bCloseWhenDone = !(m_bRecAutoOpen && m_bRecAutoOpenAllowed);
				::PostMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
								WM_THREADSAFE_OPEN_DOC,
								(WPARAM)(new CString(sNextFileName)),
								(LPARAM)pPostRecParams);
			}
		}
		else if (m_bRecAutoOpen && m_bRecAutoOpenAllowed)
		{
			::PostMessage(	::AfxGetMainFrame()->GetSafeHwnd(),
							WM_THREADSAFE_OPEN_DOC,
							(WPARAM)(new CString(sNextFileName)),
							(LPARAM)NULL);
		}
	}
}

void CVideoDeviceDoc::CloseAndShowAviRec()
{
	// Calc. Total Samples
	LONGLONG llSamplesCount = 0;
	if (m_bCaptureAudio)
	{
		for (int i = 0 ; i < m_AVRecs.GetSize() ; i++)
		{
			if (m_AVRecs[i])
				llSamplesCount += m_AVRecs[i]->GetSampleCount(m_AVRecs[i]->AudioStreamNumToStreamNum(ACTIVE_AUDIO_STREAM));
		}
	}

	// Free & Reset Vars
	FreeAVIFiles();
	m_pAVRec = NULL;
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
		ChangeRecFileFrameRate(dFrameRate);
	}
	else
		ChangeRecFileFrameRate();

	// If ending the windows session do not performe the following
	if (::AfxGetApp() && !((CUImagerApp*)::AfxGetApp())->m_bEndSession)
	{
		// Set Small Buffers for a faster Peak Meter Reaction
		if (m_bCaptureAudio)
			m_CaptureAudioThread.SetSmallBuffers(TRUE);

		// Re-Enable Critical Controls
		::PostMessage(	GetView()->GetSafeHwnd(),
						WM_ENABLE_DISABLE_CRITICAL_CONTROLS,
						(WPARAM)TRUE, // Enable Them
						(LPARAM)0);

		// Open the video file(s) and eventually post process them
		OpenAndPostProcess();
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
	BOOL res = FALSE;
	CAVRec* pNextAVRec = NULL;
	if (m_bRecSizeSegmentation)
	{
		// Get Next AVI File
		if ((m_nRecFilePos + 1) < m_AVRecs.GetSize()) 
			pNextAVRec = m_AVRecs[++m_nRecFilePos];
		else
			return FALSE;

		// Change Pointer
		m_pAVRec = pNextAVRec;

		// Set Ok
		res = TRUE;
	}
	// m_bRecTimeSegmentation
	else
	{
		// Make First Rec. File Name
		CString sFirstRecFileName = MakeRecFileName();
		if (sFirstRecFileName == _T(""))
			return FALSE;

		// Allocate & Init pNextAVRec
		if (!MakeAVRec(sFirstRecFileName, &pNextAVRec))
		{
			if (pNextAVRec)
				delete pNextAVRec;
			return FALSE;
		}

		// Close old file, change frame rate and ev. post process it
		if (m_pAVRec)
		{
			// Free
			delete m_pAVRec;

			// Change Frame Rate
			ChangeRecFileFrameRate();

			// Open the video file and ev. post process it
			OpenAndPostProcess();
		}

		// Change Pointer
		m_pAVRec = pNextAVRec;
		if (m_AVRecs.GetSize() > 0)
			m_AVRecs[0] = pNextAVRec;

		// Set new first file name
		m_sFirstRecFileName = sFirstRecFileName;

		// Restart with frame counting and time measuring
		m_bRecFirstFrame = TRUE;

		// Set Ok
		res = TRUE;
	}
	
	return res;
}

// pDib    : the frame pointer
// bPlanar : Planar Y format flag
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
												BOOL bPlanar,
												int width,
												int posX,
												int posY,
												int rx,
												int ry)
{
	int summ = 0;
	LPBYTE data = pDib->GetBits();

	// If not planar
	if (!bPlanar)
	{
		width <<= 1;
		posX <<= 1;
		rx <<= 1;
	}

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
// bPlanar			: Planar Y format flag
// nDetectionLevel	: 1 - 100 (1 - > low movement sensibility, 100 -> high movement sensibility)
BOOL CVideoDeviceDoc::MovementDetector(	CDib* pDib,
										BOOL bPlanar,
										int nDetectionLevel)
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
	int nMaxIntensityPerZone;
	if (bPlanar)
		nMaxIntensityPerZone = nZoneWidth * nZoneHeight * (235 - 16);
	else
		nMaxIntensityPerZone = nZoneWidth * nZoneHeight * 260;	// Guessed value, because there is the contribution of the chroma components!

	// Calculate the Intensities of all the zones
	for (y = 0 ; y < m_lMovDetYZonesCount ; y++)
	{
		for (x = 0 ; x < m_lMovDetXZonesCount ; x++)
		{
			m_MovementDetectorCurrentIntensity[y*m_lMovDetXZonesCount+x] = SummRectArea(pDib,
																						bPlanar,
																						pDib->GetWidth(),
																						x*nZoneWidth,	// start pixel in x direction
																						y*nZoneHeight,	// start pixel in y direction
																						nZoneWidth,
																						nZoneHeight);
		}
	}
	
	// First Frame Already Passed?
	if (!m_bFirstMovementDetection)
	{
		// Get Current Up-Time
		DWORD dwCurrentUpTime = ::timeGetTime();
		
		// Single Zone Detection and Current Time Set
		BOOL bSingleZoneDetection = FALSE;
		for (i = 0 ; i < m_lMovDetTotalZones ; i++)
		{
			if (m_MovementDetectorCurrentIntensity[i] > Round(dDetectionLevel * nMaxIntensityPerZone))
			{
				if (m_DoMovementDetection[i])
					bSingleZoneDetection = TRUE;
				m_MovementDetections[i] = TRUE;
				m_MovementDetectionsUpTime[i] = dwCurrentUpTime;
			}
		}

		// Clear Old Detection Zones and Count Detection Zones
		int nBlueMovementDetectionsCount = 0;
		int nNoneBlueMovementDetectionsCount = 0;
		for (i = 0 ; i < m_lMovDetTotalZones ; i++)
		{
			if (m_MovementDetections[i]	&&
				(dwCurrentUpTime - m_MovementDetectionsUpTime[i]) >= MOVDET_TIMEOUT)
				m_MovementDetections[i] = FALSE;
			if (m_MovementDetections[i])
			{
				if (m_DoMovementDetection[i])
					nBlueMovementDetectionsCount++;
				else
					nNoneBlueMovementDetectionsCount++;
			}
		}
		if (nBlueMovementDetectionsCount > m_nBlueMovementDetectionsCount)
			m_nBlueMovementDetectionsCount = nBlueMovementDetectionsCount;
		if (nNoneBlueMovementDetectionsCount > m_nNoneBlueMovementDetectionsCount)
			m_nNoneBlueMovementDetectionsCount = nNoneBlueMovementDetectionsCount;

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

BOOL CVideoDeviceDoc::LumChangeDetector(CDib* pDibY,
										BOOL bPlanar,
										int nPackedYOffset)
{
	int x, y;
	int nCount = 0;
	LPBYTE pDataBkg = m_pMovementDetectorBackgndDib->GetBits();
	LPBYTE pDataCur = pDibY->GetBits();
	int width = pDibY->GetWidth();
	int nZoneWidth = pDibY->GetWidth() / m_lMovDetXZonesCount;
	int nZoneHeight = pDibY->GetHeight() / m_lMovDetYZonesCount;
	if (!bPlanar)
	{
		nZoneWidth <<= 1;	// 16 bits pixels, skip chroma
		nZoneHeight <<= 1;	// 16 bits pixels, skip chroma
	}

	// Calc. difference between current Y and background Y
	// at the grid intersection points
	for (y = 1 ; y < m_lMovDetYZonesCount ; y++)
	{
		int posY = y*nZoneHeight;
		for (x = 1 ; x < m_lMovDetXZonesCount ; x++)
		{
			// Offset
			int posX = x*nZoneWidth;
			int nOffset = width*posY + posX + nPackedYOffset;

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
	if (nCount >= nTotalGridIntersections20)
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
				// Check
				if (!pTail->IsEmpty())
				{
					CDib* pHeadDib = pTail->GetHead();
					if (pHeadDib && memcmp(pHeadDib->GetBMIH(), pDib->GetBMIH(), sizeof(BITMAPINFOHEADER)) != 0)
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

__forceinline void CVideoDeviceDoc::RemoveOldestFrameFromNewestList()
{
	::EnterCriticalSection(&m_csMovementDetectionsList);
	if (!m_MovementDetectionsList.IsEmpty())
	{
		CDib::LIST* pTail = m_MovementDetectionsList.GetTail();
		if (pTail && !pTail->IsEmpty())
		{
			delete pTail->GetHead();
			pTail->RemoveHead();
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
	pCmdUI->Enable(m_bCapture);
	pCmdUI->SetCheck(m_bShowFrameTime ? 1 : 0);	
}

void CVideoDeviceDoc::OnViewDetections() 
{
	m_bShowMovementDetections = !m_bShowMovementDetections;
}

void CVideoDeviceDoc::OnUpdateViewDetections(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bCapture);
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
	pCmdUI->Enable(m_bCapture);
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
	pCmdUI->Enable(	!::AfxGetMainFrame()->m_bFullScreenMode	&&
					(rcClient != m_DocRect)					&&
					!GetFrame()->IsIconic());
}

void CVideoDeviceDoc::OnEditCopy() 
{
	// Copy Done in ProcessFrame()
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
	// Copy and Paste Done in ProcessFrame()
	m_bDoEditCopy = TRUE;
	m_bDoEditPaste = TRUE;
}

void CVideoDeviceDoc::OnUpdateEditSnapshot(CCmdUI* pCmdUI) 
{
	if (m_bDoEditCopy && m_bDoEditPaste)
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
_T("GET /axis-cgi/view/param.cgi?action=list&group=Properties.Image HTTP/1.1\r\n")            -> Get All Image Properties
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
_T("GET /camera-cgi/admin/param.cgi?action=list&group=Properties.Image.I0 HTTP/1.1\r\n")                          -> Get All Image Properties (for example: root.Properties.Image.I0.Resolution=1280x1024,640x480,320x240)
_T("GET /camera-cgi/admin/param.cgi?action=update&Image.I0.Appearance.Resolution=WidthxHeight HTTP/1.1\r\n")      -> set resolution
_T("GET /camera-cgi/admin/param.cgi?action=update&Image.I0.Appearance.Compression=CompressionLevel HTTP/1.1\r\n") -> set compression level, value range is 0-4
_T("GET /camera-cgi/admin/param.cgi?action=update&Image.I0.Stream.FPS=Fps HTTP/1.1\r\n")                          -> set framerate, valid values are: 1, 3, 5, 10, 15, 30


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

Live Cam: p120.demo.pixord.com


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
_T("GET /goform/video HTTP/1.1\r\n")
_T("GET /goform/video2 HTTP/1.1\r\n")
_T("GET /goform/capture HTTP/1.1\r\n")


D-LINK
------

JPEG for DCS-2000, DCS-2100, DCS-3230, DCS-5300, DCS-6620G, DVS-104, DVS-301
_T("GET /cgi-bin/video.jpg HTTP/1.1\r\n")


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


STARDOT
-------

JPEG
_T("GET /netcam.jpg HTTP/1.1\r\n")


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

MJPEG for SNC-RZ30N
_T("GET /image HTTP/1.1\r\n")


MOBOTIX
-------

UNKNOWN
_T("GET /record/current.jpg HTTP/1.1\r\n")


Blue Net Video Server
---------------------

JPEG
_T("GET /cgi-bin/image.cgi?control=0&id=admin&passwd=admin HTTP/1.1\r\n")


Linksys
-------

JPEG
_T("GET /img/snapshot.cgi HTTP/1.1\r\n")

*/

BOOL CVideoDeviceDoc::ConnectGetFrame()
{
	BOOL res;

	// Check
	if (!m_pGetFrameNetCom)
	{
		EndWaitCursor();
		return FALSE;
	}
	if (m_sGetFrameVideoHost == _T(""))
	{
		EndWaitCursor();
		return FALSE;
	}
	
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
			if (res)
				EndWaitCursor();
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
			res = ConnectGetFrameHTTP(m_sGetFrameVideoHost, m_nGetFrameVideoPort);
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
			res = ConnectGetFrameHTTP(m_sGetFrameVideoHost, m_nGetFrameVideoPort, _T("GET /axis-cgi/view/param.cgi?action=list&group=Properties.Image HTTP/1.1\r\n"));
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
			res = ConnectGetFrameHTTP(m_sGetFrameVideoHost, m_nGetFrameVideoPort, _T("GET /axis-cgi/view/param.cgi?action=list&group=Properties.Image HTTP/1.1\r\n"));
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
			res = ConnectGetFrameHTTP(m_sGetFrameVideoHost, m_nGetFrameVideoPort);
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
			res = ConnectGetFrameHTTP(m_sGetFrameVideoHost, m_nGetFrameVideoPort);
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
			res = ConnectGetFrameHTTP(m_sGetFrameVideoHost, m_nGetFrameVideoPort);
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
			res = ConnectGetFrameHTTP(m_sGetFrameVideoHost, m_nGetFrameVideoPort);
			break;
		}
		case EDIMAX_SP :		// Edimax Server Push (mjpeg)
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
			res = ConnectGetFrameHTTP(m_sGetFrameVideoHost, m_nGetFrameVideoPort, _T("GET /camera-cgi/admin/param.cgi?action=list&group=Properties.Image.I0 HTTP/1.1\r\n"));
			break;
		}
		default :
		{
			ASSERT(FALSE);
			res = FALSE;
			break;
		}
	}

	// End wait cursor on failure
	if (!res)
	{
		EndWaitCursor();
		return FALSE;
	}
	else
		return TRUE;
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

void CVideoDeviceDoc::UpdateFrameSendToTableAndFlowControl()
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
	DWORD dwRate = 4U * (DWORD)Round(m_dEffectiveFrameRate);
	if (dwRate == 0U)
		dwRate = 4U * (DWORD)m_dFrameRate;
	if (dwRate == 0U)
		dwRate = 1U;
	if ((m_dwFrameCountUp % dwRate) == 0)
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
	if (m_CurrentBMI.biWidth != pDib->GetWidth()				||
		m_CurrentBMI.biHeight != pDib->GetHeight()				||
		m_CurrentBMI.biCompression != pDib->GetCompression()	||
		m_CurrentBMI.biBitCount != pDib->GetBitCount()			||
		m_nCurrentDataRate != m_pDoc->m_nSendFrameDataRate		||
		m_nCurrentSizeDiv != m_pDoc->m_nSendFrameSizeDiv		||
		m_nCurrentFreqDiv != m_pDoc->m_nSendFrameFreqDiv		||
		dSendFrameRate > m_dCurrentSendFrameRate * 1.3			||
		dSendFrameRate < m_dCurrentSendFrameRate * 0.7)
		OpenAVCodec(pDib->GetBMIH());

	// Check Codec Context
	if (!m_pCodecCtx)
		return -1;

	// If RGB Flip Vertically
	LPBYTE pBits;
	if (pDib->GetCompression() == BI_RGB ||
		pDib->GetCompression() == BI_BITFIELDS)
	{
		DWORD dwDWAlignedLineSize = DWALIGNEDWIDTHBYTES(pDib->GetBitCount() * pDib->GetWidth());
		DWORD dwFlipBufSize = dwDWAlignedLineSize * pDib->GetHeight();
		if (!m_pFlipBuf || m_dwFlipBufSize < dwFlipBufSize)
		{
			if (m_pFlipBuf)
				delete [] m_pFlipBuf;
			m_pFlipBuf = new BYTE[dwFlipBufSize + FF_INPUT_BUFFER_PADDING_SIZE];
			if (!m_pFlipBuf)
			{
				m_dwFlipBufSize = 0;
				return -1;
			}
			m_dwFlipBufSize = dwFlipBufSize;
		}
		LPBYTE lpSrcBits = pDib->GetBits();
		LPBYTE lpDstBits = m_pFlipBuf + (pDib->GetHeight() - 1) * dwDWAlignedLineSize;
		for (DWORD dwCurLine = 0 ; dwCurLine < pDib->GetHeight() ; dwCurLine++)
		{
			memcpy((void*)lpDstBits, (void*)lpSrcBits, dwDWAlignedLineSize); 
			lpSrcBits += dwDWAlignedLineSize;
			lpDstBits -= dwDWAlignedLineSize;
		}
		pBits = m_pFlipBuf;
	}
	else
		pBits = pDib->GetBits();

	// Fill Src Frame
	avpicture_fill(	(AVPicture*)m_pFrame,
					(uint8_t*)pBits,
					CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat(pDib->GetBMI()),
					pDib->GetWidth(),
					pDib->GetHeight());

	// Flip Src Frame U <-> V pointers
	if (pDib->GetCompression() == FCC('YV12') ||
		pDib->GetCompression() == FCC('YV16') ||
		pDib->GetCompression() == FCC('YVU9'))
	{
		uint8_t* pTemp = m_pFrame->data[1];
		m_pFrame->data[1] = m_pFrame->data[2];
		m_pFrame->data[2] = pTemp;
		// Line Sizes for U and V are the same no need to swap
	}

	// Direct Encode?
	if ((pDib->GetCompression() == FCC('I420')	||
		pDib->GetCompression() == FCC('IYUV')	||
		pDib->GetCompression() == FCC('YV12'))	&&
		m_nCurrentSizeDiv == 0)
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
		// Color Space Conversion
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
				CAVRec::AddFrameTime((LPBYTE)m_pFrameI420->data[0],
									m_pCodecCtx->width,
									m_pCodecCtx->height,
									12,
									FCC('I420'),
									m_dwI420ImageSize,
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

BOOL CVideoDeviceDoc::CSendFrameParseProcess::OpenAVCodec(LPBITMAPINFOHEADER pBMI)
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
	m_pCodecCtx->width = pBMI->biWidth >> m_nCurrentSizeDiv;
	m_pCodecCtx->height = pBMI->biHeight >> m_nCurrentSizeDiv;

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
	if ((pBMI->biCompression != FCC('I420')	&&
		pBMI->biCompression != FCC('IYUV')	&&
		pBMI->biCompression != FCC('YV12'))	||
		m_nCurrentSizeDiv != 0)
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
		m_pImgConvertCtx = sws_getContext(	pBMI->biWidth,			// Source Width
											pBMI->biHeight,			// Source Height
											CAVIPlay::CAVIVideoStream::AVCodecBMIToPixFormat((LPBITMAPINFO)pBMI), // Source Format
											m_pCodecCtx->width,		// Destination Width
											m_pCodecCtx->height,	// Destination Height
											m_pCodecCtx->pix_fmt,	// Destination Format
											SWS_BICUBIC,			// SWS_CPU_CAPS_MMX2, SWS_CPU_CAPS_MMX, SWS_CPU_CAPS_3DNOW
											NULL,					// No Src Filter
											NULL,					// No Dst Filter
											NULL);					// Param
	}

	// Set Current BMI
	memcpy(&m_CurrentBMI, pBMI, sizeof(BITMAPINFOHEADER));

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
	if (m_pFlipBuf)
	{
		delete [] m_pFlipBuf;
		m_pFlipBuf = NULL;
	}
	m_dwFlipBufSize = 0;
	memset(&m_CurrentBMI, 0, sizeof(BITMAPINFOHEADER));
	if (m_pOutbuf)
	{
		delete [] m_pOutbuf;
		m_pOutbuf = NULL;
	}
	m_nOutbufSize = 0;
}

BOOL CVideoDeviceDoc::CSendFrameParseProcess::Parse(CNetCom* pNetCom)
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

BOOL CVideoDeviceDoc::CGetFrameParseProcess::Parse(CNetCom* pNetCom)
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
			m_pDoc->m_OrigBMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			m_pDoc->m_OrigBMI.bmiHeader.biWidth = (DWORD)m_pCodecCtx->width;
			m_pDoc->m_OrigBMI.bmiHeader.biHeight = (DWORD)m_pCodecCtx->height;
			m_pDoc->m_OrigBMI.bmiHeader.biPlanes = 1; // must be 1
			m_pDoc->m_OrigBMI.bmiHeader.biBitCount = 12;
			m_pDoc->m_OrigBMI.bmiHeader.biCompression = FCC('I420');    
			m_pDoc->m_OrigBMI.bmiHeader.biSizeImage = avpicture_get_size(	PIX_FMT_YUV420P,
																			m_pCodecCtx->width,
																			m_pCodecCtx->height);
			m_pDoc->m_DocRect.right = m_pDoc->m_OrigBMI.bmiHeader.biWidth;
			m_pDoc->m_DocRect.bottom = m_pDoc->m_OrigBMI.bmiHeader.biHeight;
			m_CodecId = CodecId;
			m_pDoc->m_bCapture = TRUE;
			m_bFirstFrame = FALSE;

			// Free Movement Detector because we changed size and/or format!
			m_pDoc->FreeMovementDetector();
			m_pDoc->ResetMovementDetector();

			// Reset Flag
			m_pDoc->m_bDecodeFramesForPreview = FALSE;

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
			m_pDoc->ProcessFrame(	(LPBYTE)(m_pOutbuf),
									m_pDoc->m_OrigBMI.bmiHeader.biSizeImage);
		}
		else
		{
			m_pDoc->m_lCompressedDataRateSum += dwFrameSize;
			m_pDoc->ProcessFrame(	(LPBYTE)(m_pFrame->data[0]),
									m_pDoc->m_OrigBMI.bmiHeader.biSizeImage);
		}
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CVideoDeviceDoc::CHttpGetFrameParseProcess::SendRequest(const CString& sRequest)
{
	CString sMsg;
	m_sLastRequest = sRequest;

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
			srand(::timeGetTime()); // Seed
			DWORD dwCNonce = (DWORD)rand();
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

BOOL CVideoDeviceDoc::CHttpGetFrameParseProcess::SendFrameRequest()
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
			break;
		}
		case AXIS_CP :		// Axis Client Poll (jpegs)
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
		case EDIMAX_SP :		// Edimax Server Push (mjpeg)
		{
			if (m_bSetResolution)
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
		default :
		{
			::LeaveCriticalSection(&m_pDoc->m_csHttpParams);
			ASSERT(FALSE);
			return FALSE;
		}
	}

	::LeaveCriticalSection(&m_pDoc->m_csHttpParams);
	
	return SendRequest(sRequest);
}

__forceinline int CVideoDeviceDoc::CHttpGetFrameParseProcess::FindMultipartBoundary(int nPos,
																					int nSize,
																					const char* pMsg)
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

BOOL CVideoDeviceDoc::CHttpGetFrameParseProcess::ParseMultipart(int nPos,
																int nSize,
																const char* pMsg,
																const CString& sMsg,
																const CString& sMsgLowerCase)
{
	int nPosEnd;
	m_FormatType = FORMATMJPEG;
	int nMultipartLength;

	if (m_bMultipartNoLength)
	{
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
		// Find Boundary
		if ((nPos = sMsg.Find(m_sMultipartBoundary, nPos)) < 0)
			return FALSE;

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

	m_nProcessOffset = nPos;
	m_nProcessSize = nMultipartLength;

	// Process data
	if (nSize >= m_nProcessOffset + m_nProcessSize)
		return TRUE;
	else
		return FALSE;
}

BOOL CVideoDeviceDoc::CHttpGetFrameParseProcess::ParseSingle(	int nSize,
																const CString& sMsg,
																const CString& sMsgLowerCase)
{
	int nPos;
	int nPosEnd;
	m_FormatType = FORMATJPEG;

	// Find Content Length
	if ((nPos = sMsgLowerCase.Find(_T("content-length:"), 0)) < 0)
		return FALSE;
	nPos += 15;
	if ((nPosEnd = sMsg.Find(_T("\r\n"), nPos)) < 0)
	if ((nPosEnd = sMsg.Find(_T('\n'), nPos)) < 0)
	if ((nPosEnd = sMsg.Find(_T('\r'), nPos)) < 0)
		return FALSE;

	// Parse Content Length
	CString sSinglepartLength = sMsgLowerCase.Mid(nPos, nPosEnd - nPos);
	if (sSinglepartLength == _T(""))
		return FALSE;
	sSinglepartLength.TrimLeft();
	if ((nPos = sSinglepartLength.Find(_T(' '), 0)) >= 0)
		sSinglepartLength = sSinglepartLength.Left(nPos);
	int nSinglepartLength = _ttoi(sSinglepartLength);
	if (nSinglepartLength <= 0)
		return FALSE;
	if ((nPos = sMsg.Find(_T("\r\n\r\n"), nPosEnd)) < 0)
	{
		if ((nPos = sMsg.Find(_T("\n\n"), nPosEnd)) < 0)
			return FALSE;
		else
			nPos += 2;
	}
	else
		nPos += 4;

	m_nProcessOffset = nPos;
	m_nProcessSize = nSinglepartLength;

	// Process data
	if (nSize >= m_nProcessOffset + m_nProcessSize)
		return TRUE;
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
BOOL CVideoDeviceDoc::CHttpGetFrameParseProcess::Parse(CNetCom* pNetCom)
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
		// Code
		CString sCode = sMsgLowerCase.Mid(9,3);
		
		// Version
		CString sVersion = sMsgLowerCase.Mid(5,3);
		if (sVersion == _T("0.9") || sVersion == _T("1.0"))
			m_bOldVersion = TRUE;
		else
			m_bOldVersion = FALSE;

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

				if (m_bTryConnecting)
				{
					m_bTryConnecting = FALSE;
					m_pDoc->EndWaitCursor();
				}
				m_bFirstFrame = TRUE;
				res = ParseMultipart(nPosEndLine, nSize, pMsg, sMsg, sMsgLowerCase);
				delete [] pMsg;
				return res;
			}
			// Single image
			else if ((nPos = sMsgLowerCase.Find(_T("content-type: image/jpeg"), 0)) >= 0)
			{
				if (m_bTryConnecting)
				{
					m_bTryConnecting = FALSE;
					m_pDoc->EndWaitCursor();
				}
				m_bFirstFrame = TRUE;
				delete [] pMsg;
				return ParseSingle(nSize, sMsg, sMsgLowerCase);
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
					m_pDoc->ConnectGetFrameHTTP(m_pDoc->m_sGetFrameVideoHost,
												m_pDoc->m_nGetFrameVideoPort);
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

					// Reset flag
					m_bQueryProperties = FALSE;

					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Start Connection
					m_pDoc->ConnectGetFrameHTTP(m_pDoc->m_sGetFrameVideoHost,
												m_pDoc->m_nGetFrameVideoPort);
				}
				else if (m_bSetResolution)
				{
					// Reset flag
					m_bSetResolution = FALSE;

					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Start Connection
					m_pDoc->ConnectGetFrameHTTP(m_pDoc->m_sGetFrameVideoHost,
												m_pDoc->m_nGetFrameVideoPort);
				}
				else if (m_bSetCompression)
				{
					// Reset flag
					m_bSetCompression = FALSE;

					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Start Connection
					m_pDoc->ConnectGetFrameHTTP(m_pDoc->m_sGetFrameVideoHost,
												m_pDoc->m_nGetFrameVideoPort);
				}
				else if (m_bSetFramerate)
				{
					// Reset flag
					m_bSetFramerate = FALSE;

					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Start Connection
					m_pDoc->ConnectGetFrameHTTP(m_pDoc->m_sGetFrameVideoHost,
												m_pDoc->m_nGetFrameVideoPort);
				}
				else if (m_bTryConnecting)
				{
					m_pDoc->EndWaitCursor();
					::AfxMessageBox(ML_STRING(1488, "Camera is telling you something,\nfirst open it in a browser, then come back here."), MB_ICONSTOP);
					m_pDoc->BeginWaitCursor();
					m_pDoc->CloseDocRemoveAutorunDev();
				}
				delete [] pMsg;
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
					m_pDoc->ConnectGetFrameHTTP(m_pDoc->m_sGetFrameVideoHost,
												m_pDoc->m_nGetFrameVideoPort);
				}
				else if (m_bTryConnecting)
				{
					m_pDoc->EndWaitCursor();
					::AfxMessageBox(ML_STRING(1489, "Camera is asking you something (probably to set a password),\nfirst open it in a browser, then come back here."), MB_ICONSTOP);
					m_pDoc->BeginWaitCursor();
					m_pDoc->CloseDocRemoveAutorunDev();
				}
				delete [] pMsg;
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
			// Parse new location
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

			// Start Connection
			m_pDoc->ConnectGetFrameHTTP(m_pDoc->m_sGetFrameVideoHost,
										m_pDoc->m_nGetFrameVideoPort,
										sNewRequest);
	
			delete [] pMsg;
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
				if (m_bTryConnecting)
					m_pDoc->EndWaitCursor();
				CAuthenticationDlg dlg;
				dlg.m_sUsername = m_pDoc->m_sHttpGetFrameUsername;
				if (dlg.DoModal() == IDCANCEL)
				{
					if (m_bTryConnecting)
					{
						m_pDoc->BeginWaitCursor();
						m_pDoc->CloseDocRemoveAutorunDev();
					}
					else
						m_pDoc->CloseDocument();
					delete [] pMsg;
					return FALSE; // Do not call Processor
				}
				else
				{
					if (m_bTryConnecting)
						m_pDoc->BeginWaitCursor();
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

			// Start Connection
			m_pDoc->ConnectGetFrameHTTP(m_pDoc->m_sGetFrameVideoHost,
										m_pDoc->m_nGetFrameVideoPort,
										m_sLastRequest);
	
			delete [] pMsg;
			return FALSE; // Do not call Processor
		}
		// Bad Request or Not Found
		else if (sCode == _T("400") || sCode == _T("404"))
		{
			if (m_pDoc->m_nNetworkDeviceTypeMode == OTHERONE && m_bTryConnecting &&
				++m_pDoc->m_nHttpGetFrameLocationPos < m_pDoc->m_HttpGetFrameLocations.GetSize())
			{
				// Empty the buffers, so that parser stops calling us!
				pNetCom->Read();

				// Try next possible device location string
				m_pDoc->ConnectGetFrameHTTP(m_pDoc->m_sGetFrameVideoHost,
											m_pDoc->m_nGetFrameVideoPort);
			}
			else if (m_bTryConnecting)
			{
				m_bTryConnecting = FALSE;
				m_pDoc->EndWaitCursor();
				::AfxMessageBox(ML_STRING(1490, "Unsupported network device type or mode"), MB_ICONSTOP);
				pNetCom->Read(); // Empty the buffers, so that parser stops calling us!
				::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
								WM_THREADSAFE_OPENGETVIDEO,
								0, 0);
			}
			// Maybe we polled to fast...
			else
				pNetCom->Read(); // Empty the buffers, so that parser stops calling us!
			delete [] pMsg;
			return FALSE; // Do not call Processor
		}
		// Service Unavailable
		else if (sCode == _T("503"))
		{
			if (m_bTryConnecting)
			{
				m_pDoc->EndWaitCursor();
				::AfxMessageBox(ML_STRING(1491, "Server is to busy, try later"), MB_ICONSTOP);
				m_pDoc->BeginWaitCursor();
				m_pDoc->CloseDocRemoveAutorunDev();
			}
			// Maybe we polled to fast...
			else
				pNetCom->Read(); // Empty the buffers, so that parser stops calling us!
			delete [] pMsg;
			return FALSE; // Do not call Processor
		}
		// Unknown
		else
		{
			if (m_bTryConnecting)
			{
				m_pDoc->EndWaitCursor();
				::AfxMessageBox(ML_STRING(1490, "Unsupported network device type or mode"), MB_ICONSTOP);
				m_pDoc->BeginWaitCursor();
				m_pDoc->CloseDocRemoveAutorunDev();
			}
			// Maybe we polled to fast...
			else
				pNetCom->Read(); // Empty the buffers, so that parser stops calling us!
			delete [] pMsg;
			return FALSE; // Do not call Processor
		}
	}
	// Multipart
	else
	{
		if (m_bTryConnecting)
		{
			m_pDoc->EndWaitCursor();
			::AfxMessageBox(ML_STRING(1490, "Unsupported network device type or mode"), MB_ICONSTOP);
			m_pDoc->BeginWaitCursor();
			m_pDoc->CloseDocRemoveAutorunDev();
			delete [] pMsg;
			return FALSE; // Do not call Processor
		}
		else
		{
			res = ParseMultipart(0, nSize, pMsg, sMsg, sMsgLowerCase);
			delete [] pMsg;
			return res;
		}
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
			m_pDoc->m_OrigBMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			m_pDoc->m_OrigBMI.bmiHeader.biWidth = (DWORD)m_pCodecCtx->width;
			m_pDoc->m_OrigBMI.bmiHeader.biHeight = (DWORD)m_pCodecCtx->height;
			m_pDoc->m_OrigBMI.bmiHeader.biPlanes = 1; // must be 1
			m_pDoc->m_OrigBMI.bmiHeader.biBitCount = 12;
			m_pDoc->m_OrigBMI.bmiHeader.biCompression = FCC('I420');    
			m_pDoc->m_OrigBMI.bmiHeader.biSizeImage = m_dwI420ImageSize;
			m_pDoc->m_DocRect.right = m_pDoc->m_OrigBMI.bmiHeader.biWidth;
			m_pDoc->m_DocRect.bottom = m_pDoc->m_OrigBMI.bmiHeader.biHeight;
			m_pDoc->m_bCapture = TRUE;

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

			// Reset Flag
			m_pDoc->m_bDecodeFramesForPreview = FALSE;

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
			m_pDoc->ProcessFrame(m_pI420Buf, m_dwI420ImageSize);
		}
	}
	// In case that avcodec_decode_video fails
	// use LoadJPEG which is more fault tolerant, but slower...
	else
	{
		TRACE(_T("*** Error: ffmpeg failed to decode mjpeg, trying Dib.LoadJPEG() ***\n"));
		CDib Dib;
		Dib.SetShowMessageBoxOnError(FALSE);
		if (Dib.LoadJPEG(pLinBuf, nSize) && Dib.Compress(FCC('I420')))
		{
			m_pDoc->m_lCompressedDataRateSum += nSize;
			m_pDoc->ProcessFrame(Dib.GetBits(), Dib.GetImageSize());
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