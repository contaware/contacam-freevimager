#include "stdafx.h"
#include "uImager.h"
#include "VideoDeviceView.h"
#include "MainFrm.h"
#include "VideoDeviceDoc.h"
#include "LicenseHelper.h"
#include "AudioInSourceDlg.h"
#include "CameraBasicSettingsDlg.h"
#include "CameraAdvancedSettingsDlg.h"
#include "Quantizer.h"
#include "DxCapture.h"
#include "DxVideoFormatDlg.h"
#include "HttpVideoFormatDlg.h"
#include "BrowseDlg.h"
#include "PostDelayedMessage.h"
#include "MotionDetHelpers.h"
#include "Base64.h"
#include "CMD5.h"
#include "CSHA256.h"
#include "Psapi.h"
#include "NoVistaFileDlg.h"
#include "YuvToYuv.h"
#include "GetDirContentSize.h"
#include "TextEntryDlg.h"
#include <random>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

// TODO: remove the warnings suppression and use the new ffmpeg API interface
#pragma warning(push)
#pragma warning(disable : 4996)

// Defined in uImager.cpp
SwsContext *sws_getContextHelper(	int srcW, int srcH, enum AVPixelFormat srcFormat,
									int dstW, int dstH, enum AVPixelFormat dstFormat,
									int flags);
SwsContext *sws_getCachedContextHelper(	struct SwsContext *context,
										int srcW, int srcH, enum AVPixelFormat srcFormat,
                                        int dstW, int dstH, enum AVPixelFormat dstFormat,
										int flags);

/////////////////////////////////////////////////////////////////////////////
// PictureDoc

IMPLEMENT_DYNCREATE(CVideoDeviceDoc, CUImagerDoc)

BEGIN_MESSAGE_MAP(CVideoDeviceDoc, CUImagerDoc)
	//{{AFX_MSG_MAP(CVideoDeviceDoc)
	ON_UPDATE_COMMAND_UI(ID_CAPTURE_RECORD, OnUpdateCaptureRecord)
	ON_COMMAND(ID_SENSITIVITY_0, OnMovDetSensitivity0)
	ON_UPDATE_COMMAND_UI(ID_SENSITIVITY_0, OnUpdateMovDetSensitivity0)
	ON_COMMAND(ID_SENSITIVITY_10, OnMovDetSensitivity10)
	ON_UPDATE_COMMAND_UI(ID_SENSITIVITY_10, OnUpdateMovDetSensitivity10)
	ON_COMMAND(ID_SENSITIVITY_20, OnMovDetSensitivity20)
	ON_UPDATE_COMMAND_UI(ID_SENSITIVITY_20, OnUpdateMovDetSensitivity20)
	ON_COMMAND(ID_SENSITIVITY_30, OnMovDetSensitivity30)
	ON_UPDATE_COMMAND_UI(ID_SENSITIVITY_30, OnUpdateMovDetSensitivity30)
	ON_COMMAND(ID_SENSITIVITY_40, OnMovDetSensitivity40)
	ON_UPDATE_COMMAND_UI(ID_SENSITIVITY_40, OnUpdateMovDetSensitivity40)
	ON_COMMAND(ID_SENSITIVITY_50, OnMovDetSensitivity50)
	ON_UPDATE_COMMAND_UI(ID_SENSITIVITY_50, OnUpdateMovDetSensitivity50)
	ON_COMMAND(ID_SENSITIVITY_60, OnMovDetSensitivity60)
	ON_UPDATE_COMMAND_UI(ID_SENSITIVITY_60, OnUpdateMovDetSensitivity60)
	ON_COMMAND(ID_SENSITIVITY_70, OnMovDetSensitivity70)
	ON_UPDATE_COMMAND_UI(ID_SENSITIVITY_70, OnUpdateMovDetSensitivity70)
	ON_COMMAND(ID_SENSITIVITY_80, OnMovDetSensitivity80)
	ON_UPDATE_COMMAND_UI(ID_SENSITIVITY_80, OnUpdateMovDetSensitivity80)
	ON_COMMAND(ID_SENSITIVITY_90, OnMovDetSensitivity90)
	ON_UPDATE_COMMAND_UI(ID_SENSITIVITY_90, OnUpdateMovDetSensitivity90)
	ON_COMMAND(ID_SENSITIVITY_100, OnMovDetSensitivity100)
	ON_UPDATE_COMMAND_UI(ID_SENSITIVITY_100, OnUpdateMovDetSensitivity100)
	ON_COMMAND(ID_CAPTURE_OBSCURESOURCE, OnCaptureObscureSource)
	ON_UPDATE_COMMAND_UI(ID_CAPTURE_OBSCURESOURCE, OnUpdateCaptureObscureSource)
	ON_COMMAND(ID_CAPTURE_CAMERAADVANCEDSETTINGS, OnCaptureCameraAdvancedSettings)
	ON_COMMAND(ID_VIEW_FRAMEANNOTATION, OnViewFrameAnnotation)
	ON_COMMAND(ID_VIEW_FRAMETIME, OnViewFrametime)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FRAMETIME, OnUpdateViewFrametime)
	ON_COMMAND(ID_VIEW_FRAMEUPTIME, OnViewFrameUptime)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FRAMEUPTIME, OnUpdateViewFrameUptime)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_UPDATE_COMMAND_UI(ID_CAPTURE_CAMERAADVANCEDSETTINGS, OnUpdateCaptureCameraAdvancedSettings)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ZONE, OnUpdateEditZone)
	ON_COMMAND(ID_EDIT_ZONES_HIDE, OnEditZonesHide)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ZONES_HIDE, OnUpdateEditZonesHide)
	ON_COMMAND(ID_EDIT_ZONE_SENSITIVITY_100, OnEditZoneSensitivity100)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ZONE_SENSITIVITY_100, OnUpdateEditZoneSensitivity100)
	ON_COMMAND(ID_EDIT_ZONE_SENSITIVITY_50, OnEditZoneSensitivity50)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ZONE_SENSITIVITY_50, OnUpdateEditZoneSensitivity50)
	ON_COMMAND(ID_EDIT_ZONE_SENSITIVITY_25, OnEditZoneSensitivity25)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ZONE_SENSITIVITY_25, OnUpdateEditZoneSensitivity25)
	ON_COMMAND(ID_EDIT_ZONE_SENSITIVITY_10, OnEditZoneSensitivity10)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ZONE_SENSITIVITY_10, OnUpdateEditZoneSensitivity10)
	ON_COMMAND(ID_EDIT_ZONE_SENSITIVITY_5, OnEditZoneSensitivity5)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ZONE_SENSITIVITY_5, OnUpdateEditZoneSensitivity5)
	ON_COMMAND(ID_EDIT_ZONE_REMOVE, OnEditZoneRemove)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ZONE_REMOVE, OnUpdateEditZoneRemove)
	ON_COMMAND(ID_EDIT_ZONE_OBSCURE_REMOVED, OnEditZoneObscureRemoved)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ZONE_OBSCURE_REMOVED, OnUpdateEditZoneObscureRemoved)
	ON_COMMAND(ID_EDIT_ZONE_BIG, OnEditZoneBig)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ZONE_BIG, OnUpdateEditZoneBig)
	ON_COMMAND(ID_EDIT_ZONE_MEDIUM, OnEditZoneMedium)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ZONE_MEDIUM, OnUpdateEditZoneMedium)
	ON_COMMAND(ID_EDIT_ZONE_SMALL, OnEditZoneSmall)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ZONE_SMALL, OnUpdateEditZoneSmall)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_COMMAND(ID_VIEW_FIT, OnViewFit)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FIT, OnUpdateViewFit)
	ON_COMMAND(ID_VIEW_WEB, OnViewWeb)
	ON_COMMAND(ID_VIEW_FILES, OnViewFiles)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_COMMAND(ID_EDIT_SNAPSHOT, OnEditSnapshot)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SNAPSHOT, OnUpdateEditSnapshot)
	ON_COMMAND(ID_CAPTURE_CAMERABASICSETTINGS, OnCaptureCameraBasicSettings)
	ON_UPDATE_COMMAND_UI(ID_CAPTURE_CAMERABASICSETTINGS, OnUpdateCaptureCameraBasicSettings)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CVideoDeviceDoc::CreateBaseYearMonthDaySubDir(CString sBaseDir, CTime Time, CString sSubDir, CString& sBaseYearMonthDaySubDir)
{
	// Remove Trailing '\'
	sBaseDir.TrimRight(_T('\\'));
	
	// Remove Leading and Trailing '\'
	sSubDir.Trim(_T('\\'));

	// Create if not already existing
	if (!sBaseDir.IsEmpty())
	{
		CString sDir(sBaseDir + _T("\\") + Time.Format(_T("%Y")) + _T("\\") + Time.Format(_T("%m")) + _T("\\") + Time.Format(_T("%d")));
		if (!sSubDir.IsEmpty())
			sDir += _T("\\") + sSubDir;
		if (!::IsExistingDir(sDir))
		{
			if (!::CreateDir(sDir))
			{
				::ShowErrorMsg(::GetLastError(), FALSE);
				sBaseYearMonthDaySubDir = _T("");
			}
			else
				sBaseYearMonthDaySubDir = sDir;
		}
		else
			sBaseYearMonthDaySubDir = sDir;
	}
	else
		sBaseYearMonthDaySubDir = _T("");
}

void CVideoDeviceDoc::CSaveFrameListThread::LoadDetFrame(CDib* pDib, DWORD& dwUpdatedIfErrorNoSuccess)
{
	// Move back the bits from shared memory
	// Note: the CSaveFrameListThread::Work() code is robust and also if SharedMemoryToBits()
	//       fails the program doesn't crash, it just drops the frame and the audio samples     
	if (pDib && pDib->m_hBitsSharedMemory)
	{
		DWORD dwError;
		if ((dwError = pDib->SharedMemoryToBits()) != ERROR_SUCCESS)
			dwUpdatedIfErrorNoSuccess = dwError;
	}
}

int CVideoDeviceDoc::CSaveFrameListThread::Work() 
{
	// Init vars
	ASSERT(m_pDoc);
	CTime FirstTime(0);
	CTime LastTime(0);
	DWORD dwCurrentThreadId = ::GetCurrentThreadId();
	CString sTempRecordingDir;
	sTempRecordingDir.Format(_T("Recording%X"), dwCurrentThreadId);
	sTempRecordingDir = ((CUImagerApp*)::AfxGetApp())->GetAppTempDir() + sTempRecordingDir;

	// Save loop
	for (;;)
	{
		// Init vars for next saving
		m_pFrameList = NULL;
		m_nNumFramesToSave = 0;
		m_nSaveProgress = 100;
		
		// Poll for work
		BOOL bPolling = TRUE;
		do
		{
			// Remove our old reservation
			((CUImagerApp*)::AfxGetApp())->SaveReservationRemove(dwCurrentThreadId);

			// Poll lists count and reservation position
			while (TRUE)
			{
				// Continue to the empty lists remover?
				::EnterCriticalSection(&m_pDoc->m_csMovementDetectionsList);
				if (m_pDoc->m_MovementDetectionsList.GetCount() >= 2 &&
					((CUImagerApp*)::AfxGetApp())->SaveReservation(dwCurrentThreadId))
					break;
				::LeaveCriticalSection(&m_pDoc->m_csMovementDetectionsList);

				// Shutdown?
				if (::WaitForSingleObject(GetKillEvent(), CAN_SAVE_POLL_MS) == WAIT_OBJECT_0)
				{
					((CUImagerApp*)::AfxGetApp())->SaveReservationRemove(dwCurrentThreadId);
					::DeleteDir(sTempRecordingDir);
					return 0;
				}
			}

			// Remove empty lists
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

		// First & Last Up-Times
		DWORD dwFirstUpTime;
		DWORD dwLastUpTime;
		if (m_pFrameList->GetHead() && m_pFrameList->GetTail())
		{
			dwFirstUpTime = m_pFrameList->GetHead()->GetUpTime();
			dwLastUpTime = m_pFrameList->GetTail()->GetUpTime();
		}
		else if (m_pFrameList->GetHead() && !m_pFrameList->GetTail())
			dwLastUpTime = dwFirstUpTime = m_pFrameList->GetHead()->GetUpTime();
		else if (!m_pFrameList->GetHead() && m_pFrameList->GetTail())
			dwFirstUpTime = dwLastUpTime = m_pFrameList->GetTail()->GetUpTime();
		else
			dwFirstUpTime = dwLastUpTime = ::timeGetTime();

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
					((CUImagerApp*)::AfxGetApp())->SaveReservationRemove(dwCurrentThreadId);
					::DeleteDir(sTempRecordingDir);
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

		// Load the saves counter and reset it if entering a new day.
		// A sequence is composed of 1 or more movies, where the first
		// one starts with a '[' and the last one ends with a ']'.
		CString sSection(m_pDoc->GetDevicePathName());
		int nMovDetSavesCount = ::AfxGetApp()->GetProfileInt(sSection, _T("MovDetSavesCount"), 1);
		int nMovDetSavesCountDay = ::AfxGetApp()->GetProfileInt(sSection, _T("MovDetSavesCountDay"), FirstTime.GetDay());
		int nMovDetSavesCountMonth = ::AfxGetApp()->GetProfileInt(sSection, _T("MovDetSavesCountMonth"), FirstTime.GetMonth());
		int nMovDetSavesCountYear = ::AfxGetApp()->GetProfileInt(sSection, _T("MovDetSavesCountYear"), FirstTime.GetYear());
		if (nMovDetSavesCountDay != FirstTime.GetDay()		||
			nMovDetSavesCountMonth != FirstTime.GetMonth()	||
			nMovDetSavesCountYear != FirstTime.GetYear())
		{
			nMovDetSavesCount = 1;
			nMovDetSavesCountDay = FirstTime.GetDay();
			nMovDetSavesCountMonth = FirstTime.GetMonth();
			nMovDetSavesCountYear = FirstTime.GetYear();
		}
		CString sMovDetSavesCount;
		sMovDetSavesCount.Format(_T("%d"), nMovDetSavesCount);
		if (m_pFrameList->GetHead())
		{
			if ((m_pFrameList->GetHead()->GetUserFlag() & FRAME_USER_FLAG_START) == FRAME_USER_FLAG_START)
				sMovDetSavesCount = _T("[") + sMovDetSavesCount;
		}
		if (m_pFrameList->GetTail())
		{
			if ((m_pFrameList->GetTail()->GetUserFlag() & FRAME_USER_FLAG_END) == FRAME_USER_FLAG_END)
				sMovDetSavesCount = sMovDetSavesCount + _T("]");
		}

		// Detection creation flags and File Names
		BOOL bMakeVideo = m_pDoc->m_bSaveVideo;
		BOOL bMakeGif = m_pDoc->m_bSaveAnimGIF;
		CString sVideoFileName;
		CVideoDeviceDoc::CreateBaseYearMonthDaySubDir(m_pDoc->m_sRecordAutoSaveDir, FirstTime, _T(""), sVideoFileName);
		if (!sVideoFileName.IsEmpty())
			sVideoFileName += _T("\\");
		CString sGIFFileName(sVideoFileName);
		sVideoFileName += _T("rec_") + sFirstTime + DEFAULT_VIDEO_FILEEXT;
		sGIFFileName += _T("rec_") + sFirstTime + _T(".gif");
		CString sVideoTempFileName(sTempRecordingDir + _T("\\") + ::GetShortFileName(sVideoFileName));
		CString sGIFTempFileName(sTempRecordingDir + _T("\\") + ::GetShortFileName(sGIFFileName));

		// Make sure our temporary folder is existing
		// (some temporary folder managers may delete it if not used for some time)
		if (!::CreateDir(sTempRecordingDir)) // it does not fail if already existing
			::ShowErrorMsg(::GetLastError(), FALSE);

		// Init the Video File
		CAVRec AVRecVideo;
		if (bMakeVideo)
			AVRecVideo.Init(sVideoTempFileName, ((CUImagerApp*)::AfxGetApp())->m_bMovFragmented);

		// Store the Frames
		POSITION nextpos = m_pFrameList->GetHeadPosition();
		POSITION currentpos;
		int nFrames = m_nNumFramesToSave;
		DWORD dwLoadDetFrameErrorCode = ERROR_SUCCESS;
		double dDelayMul = 1.0;
		double dSpeedMul = 1.0;
		CDib GIFSaveDib;
		CDib* pDibPrev;
		CDib* pDib = NULL;
		BOOL bFirstGIFSave;
		RGBQUAD* pGIFColors = NULL;
		double dCalcFrameRate = 1.0;
		DWORD dwFramesTimeMs = dwLastUpTime - dwFirstUpTime;
		if (nFrames > 1 && dwFramesTimeMs > 0U)
			dCalcFrameRate = (1000.0 * (nFrames - 1)) / (double)dwFramesTimeMs;
		if (m_pDoc->m_dEffectiveFrameRate > 0.0)
		{
			// When there are multiple frame drops one after the other or in case of reconnection
			// the framerate calculated through time difference is a lot smaller than the
			// effective framerate. If that happens we choose the effective framerate!
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
				GIFSaveDib.GetGif()->Close();
				::DeleteFile(sGIFTempFileName);
				AVRecVideo.Close();
				::DeleteFile(sVideoTempFileName);
				m_nSaveProgress = 100;
				((CUImagerApp*)::AfxGetApp())->SaveReservationRemove(dwCurrentThreadId);
				::DeleteDir(sTempRecordingDir);
				return 0;
			}

			// Get Frame
			currentpos = nextpos;
			pDib = m_pFrameList->GetNext(nextpos);
			LoadDetFrame(pDib, dwLoadDetFrameErrorCode); // load from shared memory, if not already loaded by AnimatedGifInit()

			// Video
			if (bMakeVideo)
			{
				// Add Frame Tags
				if (m_pDoc->m_bShowFrameTime)
				{
					AddFrameTime(pDib, RefTime, dwRefUpTime, m_pDoc->m_szFrameAnnotation, m_pDoc->m_nRefFontSize, m_pDoc->m_bShowFrameUptime);
					AddFrameCount(pDib, sMovDetSavesCount, m_pDoc->m_nRefFontSize);
				}

				// Add "NO DONATION" tag
				if (g_DonorEmailValidateThread.m_bNoDonation)
					AddNoDonationTag(pDib, m_pDoc->m_nRefFontSize);

				// Open if first frame
				if (nFrames == m_nNumFramesToSave)
				{
					BITMAPINFOHEADER DstBmi;
					memset(&DstBmi, 0, sizeof(BITMAPINFOHEADER));
					DstBmi.biSize = sizeof(BITMAPINFOHEADER);
					DstBmi.biWidth = pDib ? pDib->GetWidth() : 0;
					DstBmi.biHeight = pDib ? pDib->GetHeight() : 0;
					DstBmi.biPlanes = 1;
					DstBmi.biCompression = DEFAULT_VIDEO_FOURCC;
					AVRecVideo.AddVideoStream(pDib ? pDib->GetBMI() : NULL,		// Source Video Format
											(LPBITMAPINFO)(&DstBmi),			// Destination Video Format
											CalcFrameRate.num,					// Rate
											CalcFrameRate.den,					// Scale			
											m_pDoc->m_fVideoRecQuality,			// Video quality
											((CUImagerApp*)::AfxGetApp())->m_nThreadCount);
					if (m_pDoc->m_bCaptureAudio)
					{	
						AVRecVideo.AddAudioStream(	m_pDoc->m_pSrcWaveFormat,	// Src Wave Format
													m_pDoc->m_pDstWaveFormat);	// Dst Wave Format
					}
					CString sTitle(m_pDoc->GetAssignedDeviceName() + _T(" ") + ::MakeDateLocalFormat(FirstTime) + _T(" ") + ::MakeTimeLocalFormat(FirstTime, TRUE));
					AVRecVideo.Open(sTitle);
				}

				// If open add data to file
				if (AVRecVideo.IsOpen() && pDib)
				{
					// Add Frame
					if (pDib->IsValid())
					{
						AVRecVideo.AddFrame(AVRecVideo.VideoStreamNumToStreamNum(ACTIVE_VIDEO_STREAM),
											pDib,
											false);	// No interleave
					}

					// Add Audio Samples
					if (m_pDoc->m_bCaptureAudio)
					{
						POSITION posUserBuf = pDib->m_UserList.GetHeadPosition();
						while (posUserBuf)
						{
							CUserBuf UserBuf = pDib->m_UserList.GetNext(posUserBuf);
							int nNumOfSrcSamples = (m_pDoc->m_pSrcWaveFormat && (m_pDoc->m_pSrcWaveFormat->nBlockAlign > 0)) ? UserBuf.m_dwSize / m_pDoc->m_pSrcWaveFormat->nBlockAlign : 0;
							AVRecVideo.AddAudioSamples(	AVRecVideo.AudioStreamNumToStreamNum(ACTIVE_AUDIO_STREAM),
														nNumOfSrcSamples,
														UserBuf.m_pBuf,
														false);	// No interleave
						}
					}
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
									dDelayMul,				// sets this
									dSpeedMul,				// sets this
									dwLoadDetFrameErrorCode,// sets this
									dCalcFrameRate,
									RefTime,
									dwRefUpTime,
									sMovDetSavesCount);
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
									dwRefUpTime,
									sMovDetSavesCount);
				}

				// Free unused memory
				if (pDib != pDibPrev)
				{
					if (pDib)
						delete pDib;
					m_pFrameList->SetAt(currentpos, NULL);
				}
			}
			// Free memory
			else
			{
				if (pDib)
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
							dwRefUpTime,
							sMovDetSavesCount);
		}

		// Clean-Up
		if (pGIFColors)
			delete [] pGIFColors;
		GIFSaveDib.GetGif()->Close();
		AVRecVideo.Close();

		// Rename Saved Temp Files
		::DeleteFile(sVideoFileName);
		::DeleteFile(sGIFFileName);
		::MoveFile(sVideoTempFileName, sVideoFileName);
		::MoveFile(sGIFTempFileName, sGIFFileName);

		// Free
		m_pDoc->RemoveOldestMovementDetectionList();

		// Send By E-Mail
		if (m_pDoc->m_bSendMailRecording)
		{
			if (::GetFileSize64(sVideoFileName).QuadPart > 0						&&
				(m_pDoc->m_AttachmentType == CVideoDeviceDoc::ATTACHMENT_VIDEO		||
				m_pDoc->m_AttachmentType == CVideoDeviceDoc::ATTACHMENT_JPG_VIDEO))
			{
				CTimeSpan TimeDiff = FirstTime - m_pDoc->m_MovDetLastVideoMailTime;
				if (TimeDiff.GetTotalSeconds() >= (LONGLONG)m_pDoc->m_nMovDetSendMailSecBetweenMsg &&
					CVideoDeviceDoc::SendMail(m_pDoc->m_SendMailConfiguration, m_pDoc->GetAssignedDeviceName(), FirstTime, _T("REC"), _T(""), sVideoFileName))
					m_pDoc->m_MovDetLastVideoMailTime = FirstTime;
			}
			else if (::GetFileSize64(sGIFFileName).QuadPart > 0						&&
					(m_pDoc->m_AttachmentType == CVideoDeviceDoc::ATTACHMENT_GIF	||
					m_pDoc->m_AttachmentType == CVideoDeviceDoc::ATTACHMENT_JPG_GIF))
			{
				CTimeSpan TimeDiff = FirstTime - m_pDoc->m_MovDetLastGIFMailTime;
				if (TimeDiff.GetTotalSeconds() >= (LONGLONG)m_pDoc->m_nMovDetSendMailSecBetweenMsg &&
					CVideoDeviceDoc::SendMail(m_pDoc->m_SendMailConfiguration, m_pDoc->GetAssignedDeviceName(), FirstTime, _T("REC"), _T(""), sGIFFileName))
					m_pDoc->m_MovDetLastGIFMailTime = FirstTime;
			}
		}

		// Execute Command
		if (m_pDoc->m_bExecCommand && m_pDoc->m_nExecCommandMode == 1)
			m_pDoc->ExecCommand(FirstTime, sVideoFileName, sGIFFileName);

		// Increment saves count and store settings
		nMovDetSavesCount++;
		::AfxGetApp()->WriteProfileInt(sSection, _T("MovDetSavesCount"), nMovDetSavesCount);
		::AfxGetApp()->WriteProfileInt(sSection, _T("MovDetSavesCountDay"), nMovDetSavesCountDay);
		::AfxGetApp()->WriteProfileInt(sSection, _T("MovDetSavesCountMonth"), nMovDetSavesCountMonth);
		::AfxGetApp()->WriteProfileInt(sSection, _T("MovDetSavesCountYear"), nMovDetSavesCountYear);

		// Saving speed
		DWORD dwSaveTimeMs = ::timeGetTime() - dwStartUpTime;
		if (dwSaveTimeMs > 0)
		{
			// Calc.
			double dSaveFrameListSpeed = (double)dwFramesTimeMs / (double)dwSaveTimeMs;
			m_pDoc->m_nSaveFrameListSpeedPercent = Round(dSaveFrameListSpeed * 100.0);

			// Alert
			if (dwFramesTimeMs > MOVDET_MIN_LENGTH_SAVESPEED_MSEC && dSaveFrameListSpeed < 1.0)
			{
				::LogLine(ML_STRING(1839, "%s, cannot realtime save (%fx), decrease framerate and/or video resolution!"),
						m_pDoc->GetAssignedDeviceName(), dSaveFrameListSpeed);
			}
		}

		// Log error
		if (dwLoadDetFrameErrorCode != ERROR_SUCCESS)
			::LogLine(ML_STRING(1815, "OUT OF MEMORY / OVERLOAD (0x%08X): dropping frames"), dwLoadDetFrameErrorCode);
	}
	ASSERT(FALSE); // should never end up here...
	return 0;
}

void CVideoDeviceDoc::CSaveFrameListThread::AnimatedGifInit(	RGBQUAD* pGIFColors,
																double& dDelayMul,
																double& dSpeedMul,
																DWORD& dwLoadDetFrameUpdatedIfErrorNoSuccess,
																double dCalcFrameRate,
																const CTime& RefTime,
																DWORD dwRefUpTime,
																const CString& sMovDetSavesCount)
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
	
	// Prepare Dib 1
	if (!pDibForPalette1)
		pDibForPalette1 = m_pFrameList->GetHead();
	LoadDetFrame(pDibForPalette1, dwLoadDetFrameUpdatedIfErrorNoSuccess); // load from shared memory, if not already loaded by main loop
	if (pDibForPalette1)
	{
		DibForPalette1 = *pDibForPalette1;
		StretchAnimatedGif(&DibForPalette1); // resize and convert from I420 to RGB32
	}

	// Prepare Dib 2
	if (!pDibForPalette2)
		pDibForPalette2 = m_pFrameList->GetHead();
	LoadDetFrame(pDibForPalette2, dwLoadDetFrameUpdatedIfErrorNoSuccess); // load from shared memory, if not already loaded by main loop
	if (pDibForPalette2)
	{
		DibForPalette2 = *pDibForPalette2;
		StretchAnimatedGif(&DibForPalette2); // resize and convert from I420 to RGB32
	}

	// Prepare Dib 3
	if (!pDibForPalette3)
		pDibForPalette3 = m_pFrameList->GetHead();
	LoadDetFrame(pDibForPalette3, dwLoadDetFrameUpdatedIfErrorNoSuccess); // load from shared memory, if not already loaded by main loop
	if (pDibForPalette3)
	{
		DibForPalette3 = *pDibForPalette3;
		StretchAnimatedGif(&DibForPalette3); // resize and convert from I420 to RGB32
	}
	
	// Generate the Palette
	if (DibForPalette1.GetWidth() == DibForPalette2.GetWidth() && DibForPalette2.GetWidth() == DibForPalette3.GetWidth()							&&
		DibForPalette1.GetHeight() == DibForPalette2.GetHeight() && DibForPalette2.GetHeight() == DibForPalette3.GetHeight()						&&
		DibForPalette1.GetCompression() == DibForPalette2.GetCompression() && DibForPalette2.GetCompression() == DibForPalette3.GetCompression()	&&
		DibForPalette1.GetBitCount() == DibForPalette2.GetBitCount() && DibForPalette2.GetBitCount() == DibForPalette3.GetBitCount()				&&
		DibForPalette1.GetCompression() == BI_RGB && DibForPalette1.GetBitCount() == 32)
	{
		// Dib for Palette Calculation
		DibForPalette.AllocateBitsFast(	32,
										BI_RGB,
										DibForPalette1.GetWidth(),
										3 * DibForPalette1.GetHeight());
		LPBYTE pDstBits = DibForPalette.GetBits();
		if (pDstBits)
		{
			int nScanLineSize = 4 * DibForPalette.GetWidth();
			LPBYTE pSrcBits = DibForPalette1.GetBits();
			if (pSrcBits)
			{
				for (line = 0; line < DibForPalette1.GetHeight(); line++)
				{
					memcpy(pDstBits, pSrcBits, nScanLineSize);
					pDstBits += nScanLineSize;
					pSrcBits += nScanLineSize;
				}
			}
			pSrcBits = DibForPalette2.GetBits();
			if (pSrcBits)
			{
				for (line = 0; line < DibForPalette2.GetHeight(); line++)
				{
					memcpy(pDstBits, pSrcBits, nScanLineSize);
					pDstBits += nScanLineSize;
					pSrcBits += nScanLineSize;
				}
			}
			pSrcBits = DibForPalette3.GetBits();
			if (pSrcBits)
			{
				for (line = 0; line < DibForPalette3.GetHeight(); line++)
				{
					memcpy(pDstBits, pSrcBits, nScanLineSize);
					pDstBits += nScanLineSize;
					pSrcBits += nScanLineSize;
				}
			}
		}

		// Add frame tags to include its colors
		if (m_pDoc->m_bShowFrameTime)
		{
			AddFrameTime(&DibForPalette, RefTime, dwRefUpTime, m_pDoc->m_szFrameAnnotation, m_pDoc->m_nRefFontSize, m_pDoc->m_bShowFrameUptime);
			AddFrameCount(&DibForPalette, sMovDetSavesCount, m_pDoc->m_nRefFontSize);
		}

		// Add "NO DONATION" tag to include its colors
		if (g_DonorEmailValidateThread.m_bNoDonation)
			AddNoDonationTag(&DibForPalette, m_pDoc->m_nRefFontSize);

		// Calc. Palette
		CQuantizer Quantizer(239, 8); // 239 = 256 (8 bits colors) - 1 (transparency index) - 16 (vga palette)
		Quantizer.ProcessImage(&DibForPalette);
		Quantizer.SetColorTable(pGIFColors);
	}

	// Append Microsoft Windows Standard 16-colors palette
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
		pGIFColors[i+239].rgbRed		= CDib::ms_StdColors[240+i].rgbRed;
		pGIFColors[i+239].rgbGreen		= CDib::ms_StdColors[240+i].rgbGreen;
		pGIFColors[i+239].rgbBlue		= CDib::ms_StdColors[240+i].rgbBlue;
		pGIFColors[i+239].rgbReserved	= 0;
	}
	pGIFColors[255].rgbRed		= 255;
	pGIFColors[255].rgbGreen	= 255;
	pGIFColors[255].rgbBlue		= 255;
	pGIFColors[255].rgbReserved	= 0;
	
	// Limit the saved frames to around MOVDET_ANIMGIF_MAX_FRAMES and
	// the play length to around MOVDET_ANIMGIF_MAX_LENGTH ms.
	// Note: IE has problems with too many anim. gifs frames and also
	// with too many anim. gifs images per displayed page!
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
															DWORD dwRefUpTime,
															const CString& sMovDetSavesCount)
{
	if (pDib && pGIFColors && pDib->GetCompression() == FCC('I420'))
	{
		// Resize and convert from I420 to RGB32
		StretchAnimatedGif(pDib);

		// Add Frame Tags
		if (m_pDoc->m_bShowFrameTime)
		{
			AddFrameTime(pDib, RefTime, dwRefUpTime, m_pDoc->m_szFrameAnnotation, m_pDoc->m_nRefFontSize, m_pDoc->m_bShowFrameUptime);
			AddFrameCount(pDib, sMovDetSavesCount, m_pDoc->m_nRefFontSize);
		}

		// Add "NO DONATION" tag
		if (g_DonorEmailValidateThread.m_bNoDonation)
			AddNoDonationTag(pDib, m_pDoc->m_nRefFontSize);

		// Convert to 8 bpp
		pDib->CreatePaletteFromColors(256, pGIFColors); // Use all indexes for color!
		pDib->ConvertTo8bitsErrDiff(pDib->GetPalette());

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

void CVideoDeviceDoc::CSaveFrameListThread::StretchAnimatedGif(CDib* pDib)
{
	if (pDib && pDib->GetCompression() == FCC('I420'))
	{
		CDib DibAnimatedGif;
		DibAnimatedGif.SetShowMessageBoxOnError(FALSE); // no Message Box on Error
		if (DibAnimatedGif.AllocateBitsFast(12, FCC('I420'), m_pDoc->m_dwAnimatedGifWidth, m_pDoc->m_dwAnimatedGifHeight))
		{
			CVideoDeviceDoc::ResizeFast(pDib, &DibAnimatedGif);
			DibAnimatedGif.SetUpTime(pDib->GetUpTime());		// copy frame uptime	
			DibAnimatedGif.SetUserFlag(pDib->GetUserFlag());	// copy motion, detection sequence start and stop flags
			*pDib = DibAnimatedGif;
			pDib->Decompress(32);
		}
	}
}

void CVideoDeviceDoc::CSaveFrameListThread::To255Colors(CDib* pDib,
														RGBQUAD* pGIFColors,
														const CTime& RefTime,
														DWORD dwRefUpTime,
														const CString& sMovDetSavesCount)
{
	if (pDib && pGIFColors && pDib->GetCompression() == FCC('I420'))
	{
		// Resize and convert from I420 to RGB32
		StretchAnimatedGif(pDib);

		// Add Frame Tags
		if (m_pDoc->m_bShowFrameTime)
		{
			AddFrameTime(pDib, RefTime, dwRefUpTime, m_pDoc->m_szFrameAnnotation, m_pDoc->m_nRefFontSize, m_pDoc->m_bShowFrameUptime);
			AddFrameCount(pDib, sMovDetSavesCount, m_pDoc->m_nRefFontSize);
		}

		// Add "NO DONATION" tag
		if (g_DonorEmailValidateThread.m_bNoDonation)
			AddNoDonationTag(pDib, m_pDoc->m_nRefFontSize);

		// Convert to 8 bpp
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
															DWORD dwRefUpTime,
															const CString& sMovDetSavesCount)
{
	BOOL res = FALSE;

	// Check
	if (!pGIFSaveDib || !pGIFDib || !ppGIFDibPrev || !(*ppGIFDibPrev) || !pbFirstGIFSave || !pGIFColors)
		return FALSE;

	// Is First Frame To Save?
	if (*pbFirstGIFSave)
	{
		// Convert to 255 colors
		To255Colors(*ppGIFDibPrev, pGIFColors, RefTime, dwRefUpTime, sMovDetSavesCount);
		
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
			To255Colors(pGIFDib, pGIFColors, RefTime, dwRefUpTime, sMovDetSavesCount);
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
		To255Colors(*ppGIFDibPrev, pGIFColors, RefTime, dwRefUpTime, sMovDetSavesCount);
		pGIFSaveDib->GetGif()->SetDispose(GIF_DISPOSE_RESTORE);
		pGIFSaveDib->GetGif()->SetDelay(MAX(100, Round((double)(pGIFDib->GetUpTime() - (*ppGIFDibPrev)->GetUpTime()) / dSpeedMul)));
		(*ppGIFDibPrev)->DiffTransp8(pGIFSaveDib, nDiffMinLevel, 255);
		res = pGIFSaveDib->SaveNextGIF(	*ppGIFDibPrev,
										NULL,
										TRUE,
										this);
		
		// Convert to 255 colors and save
		To255Colors(pGIFDib, pGIFColors, RefTime, dwRefUpTime, sMovDetSavesCount);
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
		To255Colors(*ppGIFDibPrev, pGIFColors, RefTime, dwRefUpTime, sMovDetSavesCount);
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

int CVideoDeviceDoc::CSaveSnapshotVideoThread::Work()
{
	ASSERT(m_pDoc);

	// Init
	CDib Dib;
	Dib.SetShowMessageBoxOnError(FALSE);
	DWORD dwCurrentThreadId = ::GetCurrentThreadId();
	CAVRec* pAVRecVideo = NULL;
	CString sVideoFileName = MakeVideoHistoryFileName();
	CString sVideoTempFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sVideoFileName);
	CSortableFileFind FileFind;
	FileFind.AddAllowedExtension(_T("jpg"));
	CString sDir(::GetDriveAndDirName(sVideoFileName));
	sDir.TrimRight(_T('\\'));

	// Wait enough time to make sure the snapshot thread finished with the last jpg snapshot history file of the day that ended
	if (::WaitForSingleObject(GetKillEvent(), SNAPSHOT_VIDEO_THREAD_STARTUP_DELAY_MS) == WAIT_OBJECT_0)
		goto exit;

	// We start saving only if there are no other recordings queued (low priority flag)
	while (((CUImagerApp*)::AfxGetApp())->SaveReservation(dwCurrentThreadId, TRUE) == FALSE)
	{
		if (::WaitForSingleObject(GetKillEvent(), CAN_SAVE_POLL_MS) == WAIT_OBJECT_0)
			goto exit;
	}

	// Find and process jpg snapshot history files
	if (FileFind.Init(sDir + _T("\\") + DEFAULT_SNAPSHOT_HISTORY_FOLDER + _T("\\*")))
	{
		for (int pos = 0 ; pos < FileFind.GetFilesCount() ; pos++)
		{
			CString sShortFileNameNoExt(::GetShortFileNameNoExt(FileFind.GetFileName(pos)));
			sShortFileNameNoExt.MakeLower();
			if (sShortFileNameNoExt.Left(5) == _T("shot_"))
			{
				if (Dib.LoadJPEG(FileFind.GetFileName(pos)))
				{
					// Alloc
					if (!pAVRecVideo)
						pAVRecVideo = new CAVRec(sVideoTempFileName, ((CUImagerApp*)::AfxGetApp())->m_bMovFragmented);
					if (pAVRecVideo)
					{
						// Open
						if (!pAVRecVideo->IsOpen())
						{
							AVRational FrameRate = av_d2q((double)m_pDoc->m_nSnapshotHistoryFrameRate, MAX_SIZE_FOR_RATIONAL);
							BITMAPINFOHEADER DstBmi;
							memset(&DstBmi, 0, sizeof(BITMAPINFOHEADER));
							DstBmi.biSize = sizeof(BITMAPINFOHEADER);
							DstBmi.biWidth = Dib.GetWidth();
							DstBmi.biHeight = Dib.GetHeight();
							DstBmi.biPlanes = 1;
							DstBmi.biCompression = DEFAULT_VIDEO_FOURCC;
							pAVRecVideo->AddVideoStream(Dib.GetBMI(),						// Source Video Format
														(LPBITMAPINFO)(&DstBmi),			// Destination Video Format
														FrameRate.num,						// Rate
														FrameRate.den,						// Scale				
														DEFAULT_VIDEO_QUALITY,				// Use default video quality
														((CUImagerApp*)::AfxGetApp())->m_nThreadCount);
							pAVRecVideo->Open(m_pDoc->GetAssignedDeviceName() + _T(" ") + ::MakeDateLocalFormat(m_Time));
						}

						// Add Frame
						if (pAVRecVideo->IsOpen())
						{
							pAVRecVideo->AddFrame(pAVRecVideo->VideoStreamNumToStreamNum(ACTIVE_VIDEO_STREAM),
												&Dib,
												false);	// No interleave for Video only
						}
					}
				}
			}

			// Do Exit?
			if (DoExit())
				goto exit;
		}
	}

	// Close temp file by freeing CAVRec object
	if (pAVRecVideo)
	{
		delete pAVRecVideo;
		pAVRecVideo = NULL;
	}

	// If video has been created
	if (::IsExistingFile(sVideoTempFileName))
	{
		// Remove folder
		::DeleteDir(sDir + _T("\\") + DEFAULT_SNAPSHOT_HISTORY_FOLDER);

		// Copy from temp to snapshots folder
		::CopyFile(sVideoTempFileName, sVideoFileName, FALSE);

		// Execute Command
		if (m_pDoc->m_bExecCommand && m_pDoc->m_nExecCommandMode == 3)
			m_pDoc->ExecCommand(m_Time, sVideoFileName);
	}

	// Set task completion time
	// (if thread is killed this var is not set, that's the correct behavior)
	m_TaskCompletedForTime = m_Time;

exit:
	// Clean-up
	if (pAVRecVideo)
		delete pAVRecVideo;
	::DeleteFile(sVideoTempFileName);
	((CUImagerApp*)::AfxGetApp())->SaveReservationRemove(dwCurrentThreadId);

	return 0;
}

int CVideoDeviceDoc::CSaveSnapshotThread::Work()
{
	ASSERT(m_pDoc);

	// Get uptime
	DWORD dwUpTime = m_Dib.GetUpTime();

	// Live file names
	CString sLiveFileName(m_pDoc->m_sRecordAutoSaveDir);
	sLiveFileName.TrimRight(_T('\\'));
	sLiveFileName += CString(_T("\\")) + DEFAULT_SNAPSHOT_LIVE_JPEGNAME;
	CString sLiveThumbFileName(m_pDoc->m_sRecordAutoSaveDir);
	sLiveThumbFileName.TrimRight(_T('\\'));
	sLiveThumbFileName += CString(_T("\\")) + DEFAULT_SNAPSHOT_LIVE_JPEGTHUMBNAME;

	// Init history file name
	// Note: if m_pDoc->m_bSnapshotHistoryVideo is TRUE
	//       it creates also year\month\day\DEFAULT_SNAPSHOT_HISTORY_FOLDER
	//       otherwise it just returns the file name without path
	CString sHistoryFileName(MakeJpegHistoryFileName());

	// Temp file names
	CString sTempFileName(::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), sHistoryFileName));
	CString sTempThumbFileName(::GetFileNameNoExt(sTempFileName) + _T("_thumb.jpg"));

	// Resize thumb
	CDib DibThumb;
	DibThumb.SetShowMessageBoxOnError(FALSE); // no Message Box on Error
	if (DibThumb.AllocateBitsFast(12, FCC('I420'), m_pDoc->m_nSnapshotThumbWidth, m_pDoc->m_nSnapshotThumbHeight))
	{
		CVideoDeviceDoc::ResizeFast(&m_Dib, &DibThumb);
		DibThumb.SetUpTime(m_Dib.GetUpTime());
	}

	// Add tags
	if (m_pDoc->m_bShowFrameTime)
	{
		AddFrameTime(&m_Dib, m_Time, dwUpTime, m_pDoc->m_szFrameAnnotation, m_pDoc->m_nRefFontSize, m_pDoc->m_bShowFrameUptime);
		AddFrameTime(&DibThumb, m_Time, dwUpTime, m_pDoc->m_szFrameAnnotation, m_pDoc->m_nRefFontSize, m_pDoc->m_bShowFrameUptime);
	}
	if (g_DonorEmailValidateThread.m_bNoDonation)
	{
		AddNoDonationTag(&m_Dib, m_pDoc->m_nRefFontSize);
		AddNoDonationTag(&DibThumb, m_pDoc->m_nRefFontSize);
	}
	if (m_pDoc->m_bDetectingMinLengthMovement)
	{
		AddRecSymbol(&m_Dib, m_pDoc->m_nRefFontSize);
		AddRecSymbol(&DibThumb, m_pDoc->m_nRefFontSize);
	}

	// Save to temp location
	CVideoDeviceDoc::SaveJpegFast(&m_Dib, &m_MJPEGEncoder, sTempFileName, DEFAULT_SNAPSHOT_COMPR_QUALITY);
	CVideoDeviceDoc::SaveJpegFast(&DibThumb, &m_MJPEGThumbEncoder, sTempThumbFileName, DEFAULT_SNAPSHOT_COMPR_QUALITY);
	
	// Go Live
	::CopyFile(sTempFileName, sLiveFileName, FALSE);
	::CopyFile(sTempThumbFileName, sLiveThumbFileName, FALSE);

	// Execute Command
	// Attention: the user is responsible to make sure that the executed
	//            program is fast enough to avoid that the passed live
	//            snapshots get overwritten with the next shot!
	if (m_pDoc->m_bExecCommand && m_pDoc->m_nExecCommandMode == 2)
		m_pDoc->ExecCommand(m_Time, sLiveFileName, sLiveThumbFileName);

	// We need the History jpgs to make the video file inside the snapshot video thread
	// (history jpgs are deleted in snapshot video thread)
	if (m_pDoc->m_bSnapshotHistoryVideo && (m_Time - m_LastSnapshotHistoryTime).GetTotalSeconds() >= (LONGLONG)m_pDoc->m_nSnapshotHistoryRate)
	{
		CVideoDeviceDoc::SaveJpegFast(&m_Dib, &m_MJPEGEncoder, sHistoryFileName, GOOD_SNAPSHOT_COMPR_QUALITY);
		m_LastSnapshotHistoryTime = m_Time;
	}

	// Clean-up
	::DeleteFile(sTempFileName);
	::DeleteFile(sTempThumbFileName);

	return 0;
}

__forceinline CString CVideoDeviceDoc::CSaveSnapshotThread::MakeJpegHistoryFileName()
{
	CString sBaseYearMonthDaySubDir;

	// Snapshot time
	CString sTime = m_Time.Format(_T("%Y_%m_%d_%H_%M_%S"));

	// Create directory if necessary
	if (m_pDoc->m_bSnapshotHistoryVideo)
		CVideoDeviceDoc::CreateBaseYearMonthDaySubDir(m_pDoc->m_sRecordAutoSaveDir, m_Time, DEFAULT_SNAPSHOT_HISTORY_FOLDER, sBaseYearMonthDaySubDir);

	// Return file name
	if (sBaseYearMonthDaySubDir.IsEmpty())
		return _T("shot_") + sTime + _T(".jpg");
	else
		return sBaseYearMonthDaySubDir + _T("\\") + _T("shot_") + sTime + _T(".jpg");
}

__forceinline CString CVideoDeviceDoc::CSaveSnapshotVideoThread::MakeVideoHistoryFileName()
{
	CString sBaseYearMonthDayDir;

	// Snapshots time
	CString sTime = m_Time.Format(_T("%Y_%m_%d"));

	// Create directory
	CVideoDeviceDoc::CreateBaseYearMonthDaySubDir(m_pDoc->m_sRecordAutoSaveDir, m_Time, _T(""), sBaseYearMonthDayDir);

	// Return file name
	if (sBaseYearMonthDayDir.IsEmpty())
		return _T("shot_") + sTime + DEFAULT_VIDEO_FILEEXT;
	else
		return sBaseYearMonthDayDir + _T("\\") + _T("shot_") + sTime + DEFAULT_VIDEO_FILEEXT;
}

BOOL CVideoDeviceDoc::SendMail(	const SendMailConfigurationStruct& Config,
								const CString& sName,
								const CTime& Time,
								const CString& sNote,
								CString sBody/*=_T("")*/,
								const CString& sFileName/*=_T("")*/,
								BOOL bShow/*=FALSE*/)
{
	if (!Config.m_sHost.IsEmpty() &&
		!Config.m_sFrom.IsEmpty() &&
		!Config.m_sTo.IsEmpty())
	{
		// Prepare params
		CString sOptions;
		CString sConnectionTypeOption;
		CString sSubject = Config.m_sSubject;
		sSubject.Replace(_T("%name%"), sName);
		sSubject.Replace(_T("%date%"), ::MakeDateLocalFormat(Time));
		sSubject.Replace(_T("%time%"), ::MakeTimeLocalFormat(Time, TRUE));
		sSubject.Replace(_T("%note%"), sNote);
		if (sBody.IsEmpty())
			sBody = sName + _T(": ") + ::MakeDateLocalFormat(Time) + _T(" ") + ::MakeTimeLocalFormat(Time, TRUE) + _T(" ") + sNote;
		switch (Config.m_ConnectionType)
		{
			case 0 : sConnectionTypeOption = _T(""); break;				// Plain Text
			case 1 : sConnectionTypeOption = _T("-ssl"); break;			// SSL and TLS
			default: sConnectionTypeOption = _T("-starttls"); break;	// STARTTLS
		}
		sOptions.Format(_T("-t \"%s\" -f %s %s %s -port %d %s -smtp %s -ct %d -read-timeout %d -sub \"%s\" +cc +bc -user \"%s\" -pass \"%s\" -cs \"utf-8\" -mime-type \"text/plain\" -enc-type \"base64\" -M \"%s\""),
						Config.m_sTo,
						Config.m_sFrom,
						Config.m_sFromName.IsEmpty() ? _T("") : _T("-name \"") + Config.m_sFromName + _T("\""),
						sConnectionTypeOption,
						Config.m_nPort,
						(Config.m_sUsername.IsEmpty() && Config.m_sPassword.IsEmpty()) ? _T("") : _T("-auth"),
						Config.m_sHost,
						MAILPROG_TIMEOUT_SEC, // connect timeout
						MAILPROG_TIMEOUT_SEC, // read timeout
						sSubject,
						Config.m_sUsername,
						Config.m_sPassword,
						sBody);
		if (::GetFileSize64(sFileName).QuadPart > 0)
			sOptions += _T(" -attach \"") + ::GetASCIICompatiblePath(sFileName) + _T("\"");

		// Send
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szProgramName[MAX_PATH];
		if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) != 0)
		{
			_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
			CString sMailerStartFile = CString(szDrive) + CString(szDir);
			sMailerStartFile += MAILPROG_RELPATH;
			if (::IsExistingFile(sMailerStartFile))
			{
				if (bShow)
					sOptions = _T("-w ") + sOptions; // wait for a CR after sending the mail

				// Start mailsend.exe
				//
				// Note: do not use ShellExecuteEx because it creates a separate thread
				//       and does not return until that thread completes. During this time
				//       ShellExecuteEx will pump window messages to prevent windows
				//       owned by the calling thread from appearing hung. 
				//       We also need the STARTF_FORCEOFFFEEDBACK flag which is not available
				//       with ShellExecuteEx, this flag avoids showing the busy cursor while
				//       starting the mailsend.exe process.
				//       The CreateProcessA() ANSI version is called so that the passed utf8
				//       command line remains untouched!
				STARTUPINFOA si;
				PROCESS_INFORMATION pi;
				memset(&si, 0, sizeof(si));
				memset(&pi, 0, sizeof(pi));
				si.cb = sizeof(si);
				si.dwFlags =	STARTF_FORCEOFFFEEDBACK |	// do not display the busy cursor
								STARTF_USESHOWWINDOW;		// use the following wShowWindow
				si.wShowWindow = bShow ? SW_SHOW : SW_HIDE;
				CStringA sAsciiMailerStartFile(::GetASCIICompatiblePath(sMailerStartFile));
				CStringA sAsciiDir(::GetASCIICompatiblePath(::GetDriveAndDirName(sMailerStartFile)));
				LPBYTE pUTF8Options = NULL;
				::ToUTF8(sOptions, &pUTF8Options); // utf8 allows any character in subject and body
				char lpCommandLine[32768];
				strcpy_s(lpCommandLine, "\"");
				strcat_s(lpCommandLine, sAsciiMailerStartFile);
				strcat_s(lpCommandLine, "\" ");
				strcat_s(lpCommandLine, (LPCSTR)pUTF8Options);
				BOOL bStarted = ::CreateProcessA(sAsciiMailerStartFile, lpCommandLine,
												NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL,
												sAsciiDir, &si, &pi);
				if (pUTF8Options)
					delete[] pUTF8Options;
				if (pi.hProcess)
					::CloseHandle(pi.hProcess);
				if (pi.hThread)
					::CloseHandle(pi.hThread);
				return bStarted;
			}
		}
	}
	return FALSE;
}

UINT CVideoDeviceDoc::EffectiveCaptureAudioDeviceID()
{
	// Prefer the device name over the id because the id can change when devices are added/removed.
	// Exception: in case of multiple devices with the same name give higher priority to the id.
	if (!m_sCaptureAudioDeviceName.IsEmpty() &&
		CaptureAudioDeviceIDToName(m_dwCaptureAudioDeviceID) != m_sCaptureAudioDeviceName)
		return CaptureAudioNameToDeviceID(m_sCaptureAudioDeviceName);
	else
		return m_dwCaptureAudioDeviceID;
}

CString CVideoDeviceDoc::CaptureAudioDeviceIDToName(UINT uiID)
{
	// Check
	if (uiID >= ::waveInGetNumDevs())
		return _T("Unknown Device");

	// Get device name
	WAVEINCAPS2 DevCaps;
	memset(&DevCaps, 0, sizeof(WAVEINCAPS2));
	MMRESULT res = ::waveInGetDevCaps(uiID, (LPWAVEINCAPS)(&DevCaps), sizeof(WAVEINCAPS2));
	if (res != MMSYSERR_NOERROR)
		return _T("Unknown Device");
	else
	{
		CString sDevName(DevCaps.szPname);
		CString sRegistryDevName = ::GetRegistryStringValue(HKEY_LOCAL_MACHINE,
									_T("System\\CurrentControlSet\\Control\\MediaCategories\\{") +
									::UuidToCString(&DevCaps.NameGuid) + _T("}"),
									_T("Name"));
		if (sDevName.GetLength() > sRegistryDevName.GetLength())
			return (sRegistryDevName.GetLength() > 5 ? sRegistryDevName : sDevName); // priority to registry device name if it has a reasonable length
		else
			return sRegistryDevName;
	}
}

UINT CVideoDeviceDoc::CaptureAudioNameToDeviceID(const CString& sName)
{
	UINT uiNumDev = ::waveInGetNumDevs();
	for (UINT i = 0 ; i < uiNumDev ; i++)
	{
		if (sName == CaptureAudioDeviceIDToName(i))
			return i;
	}
	return 0;
}

CVideoDeviceDoc::CCaptureAudioThread::CCaptureAudioThread() 
{
	// Set pointers to NULL
	m_pDoc = NULL;
	m_pAudioTools = NULL;
	m_pAudioPlay = NULL;

	// Create Input Event
	m_hWaveInEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEventArray[0] = GetKillEvent();
	m_hEventArray[1] = m_hWaveInEvent;
	m_hWaveIn = NULL;

	// ACM
	for (int i = 0 ; i < AUDIO_UNCOMPRESSED_BUFS_COUNT ; i++)
	{
		memset(&m_WaveHeader[i], 0, sizeof(WAVEHDR));
		m_pUncompressedBuf[i] = NULL;
	}
	m_dwUncompressedBufSize = 0;
}

CVideoDeviceDoc::CCaptureAudioThread::~CCaptureAudioThread() 
{
	Kill();
	for (int i = 0 ; i < AUDIO_UNCOMPRESSED_BUFS_COUNT ; i++)
	{
		if (m_pUncompressedBuf[i])
		{
			av_free(m_pUncompressedBuf[i]);
			m_pUncompressedBuf[i] = NULL;
		}
	}
	::CloseHandle(m_hWaveInEvent);
	m_hWaveInEvent = NULL;
}

void CVideoDeviceDoc::CCaptureAudioThread::AudioInSourceDialog()
{
	CAudioInSourceDlg dlg(m_pDoc->EffectiveCaptureAudioDeviceID());
	if (dlg.DoModal() == IDOK)
	{
		// Stop Save Frame List Thread
		m_pDoc->m_SaveFrameListThread.Kill();

		// Set new ID
		if (m_pDoc->m_bCaptureAudio && !m_pDoc->m_bCaptureAudioFromStream)
			Kill();
		m_pDoc->m_dwCaptureAudioDeviceID = dlg.m_uiDeviceID;
		m_pDoc->m_sCaptureAudioDeviceName = CaptureAudioDeviceIDToName(m_pDoc->m_dwCaptureAudioDeviceID);
		if (m_pDoc->m_bCaptureAudio && !m_pDoc->m_bCaptureAudioFromStream)
			Start();

		// Restart Save Frame List Thread
		m_pDoc->m_SaveFrameListThread.Start();
	}
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
		return -1; // error logged inside function

	// Start Buffering
	CUserBuf UserBuf;
	::EnterCriticalSection(&m_pDoc->m_csAudioList);
	while (!m_pDoc->m_AudioList.IsEmpty())
	{
		UserBuf = m_pDoc->m_AudioList.RemoveHead();
		if (UserBuf.m_pBuf)
		{
			av_free(UserBuf.m_pBuf);
			UserBuf.m_pBuf = NULL;
		}
	}
	::LeaveCriticalSection(&m_pDoc->m_csAudioList);
	m_uiWaveInBufPos = 0;
	int i;
	for (i = 0 ; i < AUDIO_UNCOMPRESSED_BUFS_COUNT ; i++)
	{
		if (m_pUncompressedBuf[i])
			av_free(m_pUncompressedBuf[i]);
		m_pUncompressedBuf[i] = (LPBYTE)av_malloc(m_dwUncompressedBufSize);
		if (!DataInAudio())
		{
			nLoopState = -1; // error logged inside function
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
												::EnterCriticalSection(&m_pDoc->m_csAudioList);
												UserBuf.m_dwSize = m_WaveHeader[m_uiWaveInBufPos].dwBytesRecorded;
												UserBuf.m_pBuf = m_pUncompressedBuf[m_uiWaveInBufPos];
												m_pDoc->m_AudioList.AddTail(UserBuf);
												m_pDoc->AudioListen(UserBuf.m_pBuf, UserBuf.m_dwSize, m_pAudioTools, m_pAudioPlay);
												if (m_pDoc->m_AudioList.GetCount() > AUDIO_MAX_LIST_SIZE)
												{
													UserBuf = m_pDoc->m_AudioList.RemoveHead();
													if (UserBuf.m_pBuf)
													{
														av_free(UserBuf.m_pBuf);
														UserBuf.m_pBuf = NULL;
													}
												}
												::LeaveCriticalSection(&m_pDoc->m_csAudioList);

												// New Buffer
												m_pUncompressedBuf[m_uiWaveInBufPos] = (LPBYTE)av_malloc(m_dwUncompressedBufSize);
												if (!DataInAudio())
												{
													nLoopState = -1; // error logged inside function
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

	// Init COM
	::CoInitialize(NULL);

	// Init audio listen
	if (!m_pAudioTools)
		m_pAudioTools = new CAudioTools;
	if (!m_pAudioPlay)
		m_pAudioPlay = new CAudioPlay;
	if (m_pAudioPlay)
		m_pAudioPlay->Init(5000000);

	// Loop
	while (Loop() != 0)
	{
		// On error try reconnecting after AUDIO_RECONNECTION_DELAY ms of wait time
		if (::WaitForSingleObject(m_hKillEvent, AUDIO_RECONNECTION_DELAY) == WAIT_OBJECT_0)
			break; // exit thread
	}

	// Close audio listen and free
	if (m_pAudioPlay)
	{
		delete m_pAudioPlay;
		m_pAudioPlay = NULL;
	}
	if (m_pAudioTools)
	{
		delete m_pAudioTools;
		m_pAudioTools = NULL;
	}

	// Uninit COM
	::CoUninitialize();

	return 0;
}

BOOL CVideoDeviceDoc::CCaptureAudioThread::OpenInAudio()
{
	// Check Wave Format Pointers
	if (!m_pDoc->m_pSrcWaveFormat || !m_pDoc->m_pDstWaveFormat)
	{
		::LogLine(_T("%s"), m_pDoc->GetAssignedDeviceName() + _T(", open sound input device error because of NULL wave format!"));
		return FALSE;
	}

	// First Close
	CloseInAudio();

	// Get Number of Audio Devices
	UINT num = ::waveInGetNumDevs();
	if (num == 0)
	{
		::LogLine(_T("%s"), m_pDoc->GetAssignedDeviceName() + _T(", ") + ML_STRING(1354, "No Sound Input Device."));
		return FALSE;
	}

	// Calculate The Source (=Uncompressed) Buffer Size
	int nSamplesPerSec = m_pDoc->m_pSrcWaveFormat->nSamplesPerSec;
	int nBlockAlign = m_pDoc->m_pSrcWaveFormat->nBlockAlign;
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
	if (m_pDoc->m_pDstWaveFormat->wFormatTag == WAVE_FORMAT_AAC2)
		nFrameSize = 2 * 1024 * m_pDoc->m_pDstWaveFormat->nChannels;
	else if (m_pDoc->m_pDstWaveFormat->wFormatTag == WAVE_FORMAT_MPEGLAYER3)
		nFrameSize = 2 * 1152 * m_pDoc->m_pDstWaveFormat->nChannels;
	else if (m_pDoc->m_pDstWaveFormat->wFormatTag == WAVE_FORMAT_DVI_ADPCM)
		nFrameSize = 2 * ((1024 - 4 * m_pDoc->m_pDstWaveFormat->nChannels) * 8 / (4 * m_pDoc->m_pDstWaveFormat->nChannels) + 1) * m_pDoc->m_pDstWaveFormat->nChannels;

	// Set the buffer size a multiple of the frame size
	if (nFrameSize > 0)
	{
		int nRemainder = m_dwUncompressedBufSize % nFrameSize;
		if (nRemainder > 0)
			m_dwUncompressedBufSize += nFrameSize - nRemainder;
	}

	// Open Input
	if (::waveInOpen(	&m_hWaveIn,
						m_pDoc->EffectiveCaptureAudioDeviceID(),
						m_pDoc->m_pSrcWaveFormat,
						(DWORD)m_hWaveInEvent,
						NULL,
						CALLBACK_EVENT) != MMSYSERR_NOERROR)
	{
		::ResetEvent(m_hWaveInEvent); // Reset The Open Event
		::LogLine(_T("%s, sound input cannot open device"), m_pDoc->GetAssignedDeviceName());
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
			::LogLine(_T("%s"), m_pDoc->GetAssignedDeviceName() + _T(", sound input cannot UnprepareHeader!"));
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
		::LogLine(_T("%s"), m_pDoc->GetAssignedDeviceName() + _T(", sound input cannot PrepareHeader!"));
		return FALSE;
	}

	res = ::waveInAddBuffer(m_hWaveIn, &m_WaveHeader[m_uiWaveInBufPos], sizeof(WAVEHDR));
	if (res != MMSYSERR_NOERROR) 
	{
		::LogLine(_T("%s"), m_pDoc->GetAssignedDeviceName() + _T(", sound input cannot AddBuffer!"));
		return FALSE;
	}

	res = ::waveInStart(m_hWaveIn);
	if (res != MMSYSERR_NOERROR) 
	{
		::LogLine(_T("%s"), m_pDoc->GetAssignedDeviceName() + _T(", sound input cannot Start Wave In!"));
		return FALSE;
	}

	m_uiWaveInBufPos = (m_uiWaveInBufPos + 1) % AUDIO_UNCOMPRESSED_BUFS_COUNT;

	return TRUE;
}

BOOL CVideoDeviceDoc::AudioListen(	LPBYTE pData, DWORD dwSizeInBytes,
									CAudioTools* pAudioTools, CAudioPlay* pAudioPlay)
{
	// Check
	if (!pData || dwSizeInBytes == 0 || !m_pSrcWaveFormat || !pAudioTools || !pAudioPlay)
		return FALSE;
	
	// Play audio
	if (pAudioPlay->IsInit())
	{
		// Calc. source frames
		int nNumSrcFrames = dwSizeInBytes / m_pSrcWaveFormat->nBlockAlign;

		// CAudioPlay always wants the floating-point format
		float* pToFloatBuf = pAudioTools->ToFloat(	pData,
													nNumSrcFrames,
													m_pSrcWaveFormat);

		// Adjust to the channels wanted by CAudioPlay
		float* pConvertChannelsBuf = pAudioTools->ConvertChannels(	pToFloatBuf,
																	nNumSrcFrames,
																	m_pSrcWaveFormat->nChannels,
																	pAudioPlay->GetWaveFormat()->nChannels);

		// Resample to the sampling-rate wanted by CAudioPlay
		int nNumDstFrames;
		float* pResampleBuf = pAudioTools->Resample(pConvertChannelsBuf,
													nNumSrcFrames,
													&nNumDstFrames,
													m_pSrcWaveFormat->nSamplesPerSec,
													pAudioPlay->GetWaveFormat()->nSamplesPerSec,
													pAudioPlay->GetWaveFormat()->nChannels);

		// Clear buffer if not listening or if obscuring the source
		if (!m_bAudioListen || m_bObscureSource)
			memset(pResampleBuf, 0, nNumDstFrames * pAudioPlay->GetWaveFormat()->nChannels * sizeof(float));

		// Finally play it
		return pAudioPlay->Write((LPBYTE)pResampleBuf, nNumDstFrames, TRUE);
	}
	else
		return FALSE;
}

void CVideoDeviceDoc::MovementDetectionProcessing(CDib* pDib, const CTime& Time, BOOL b1SecTick)
{
	BOOL bSoftwareDetectionMovement = FALSE;
	int nDetectionLevel = m_nDetectionLevel; // use local var
	int nMovementDetectorIntensityLimit = 50 - nDetectionLevel / 2;	// noise floor

	// Init from UI thread because of a UI control update and
	// initialization of variables used by the UI drawing
	if (m_lMovDetTotalZones == 0 || m_nCurrentDetectionZoneSize != m_nDetectionZoneSize)
	{
		if (::SendMessage(	GetView()->GetSafeHwnd(),
							WM_THREADSAFE_INIT_MOVDET,
							0, 0) == 0)
			goto end_of_software_detection; // Cannot init, unsupported resolution
	}

	// REC OFF
	if (nDetectionLevel == 0)
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
		if (m_MovementDetections)
			memset(m_MovementDetections, 0, m_lMovDetTotalZones);
	}
	// Continuous REC
	else if (nDetectionLevel == 100)
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
		for (int i = 0; i < m_lMovDetTotalZones; i++)
		{
			if (m_DoMovementDetection[i])
			{
				m_MovementDetections[i] = 1;
				m_MovementDetectionsUpTime[i] = pDib->GetUpTime();
			}
			else
				m_MovementDetections[i] = 0;
		}
		bSoftwareDetectionMovement = TRUE;
	}
	// Software Motion Detection
	else
	{
		// Every 1 sec check whether we have to update the Freq Div
		if (b1SecTick &&
			(m_dEffectiveFrameRate > m_dMovDetFrameRateFreqDivCalc + 0.5 ||
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
			// Init Dibs
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
			LPBYTE MinDiff = (LPBYTE)((DWORD)(p + 7) & 0xFFFFFFF8);
			MinDiff[0] = nMovementDetectorIntensityLimit; // noise floor
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
			bSoftwareDetectionMovement = MovementDetector(m_pDifferencingDib, nDetectionLevel);

			// Update background
			// Note: Mix7To1MMX and Mix3To1MMX use the pavgb instruction
			// which is available only on SSE processors
			if (g_bSSE)
			{
				if (m_dMovDetFrameRateFreqDivCalc / m_nMovDetFreqDiv >= MOVDET_MIX_THRESHOLD)
				{
					::Mix7To1MMX(m_pMovementDetectorBackgndDib->GetBits(),	// Src1 & Dst
								pDib->GetBits(),							// Src2
								nSize8);									// Size in 8 bytes units
				}
				else
				{
					::Mix3To1MMX(m_pMovementDetectorBackgndDib->GetBits(),	// Src1 & Dst
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
					for (int i = 0; i < nSize; i++)
						p1[i] = (BYTE)((7 * (int)(p1[i]) + (int)(p2[i]) + 4) >> 3);
				}
				else
				{
					for (int i = 0; i < nSize; i++)
						p1[i] = (BYTE)((3 * (int)(p1[i]) + (int)(p2[i]) + 2) >> 2);
				}
			}
		}
	}

	// End of software detection
end_of_software_detection:

	// If Movement
	BOOL bMarkStart = FALSE;
	if (bSoftwareDetectionMovement)
	{
		// Mark the Frame as a Cause of Movement
		pDib->SetUserFlag(pDib->GetUserFlag() | FRAME_USER_FLAG_MOTION);

		// Update the Up-Time
		m_dwLastDetFrameUpTime = pDib->GetUpTime();

		// First detected frame?
		if (!m_bDetectingMovement)
		{
			m_bDetectingMovement = TRUE;
			bMarkStart = TRUE;
			m_dwFirstDetFrameUpTime = pDib->GetUpTime();
		}

		// Do we have a movement of at least m_nDetectionMinLengthMilliSeconds?
		// (if m_nDetectionMinLengthMilliSeconds is 0 then m_bDetectingMinLengthMovement
		// is the same as m_bDetectingMovement)
		if (!m_bDetectingMinLengthMovement &&
			(m_dwLastDetFrameUpTime - m_dwFirstDetFrameUpTime) >= (DWORD)m_nDetectionMinLengthMilliSeconds)
		{
			// Set flag
			m_bDetectingMinLengthMovement = TRUE;

			// Common var
			CString sSavedJpegRec;

			// Send E-Mail
			if (m_bSendMailRecording)
			{
				if (m_AttachmentType == ATTACHMENT_NONE)
				{
					CTimeSpan TimeDiff = Time - m_MovDetLastMailTime;
					if (TimeDiff.GetTotalSeconds() >= (LONGLONG)m_nMovDetSendMailSecBetweenMsg)
					{
						if (CVideoDeviceDoc::SendMail(m_SendMailConfiguration, GetAssignedDeviceName(), Time, _T("REC")))
							m_MovDetLastMailTime = Time;
					}
				}
				else if (	m_AttachmentType == ATTACHMENT_JPG			||
							m_AttachmentType == ATTACHMENT_JPG_VIDEO	||
							m_AttachmentType == ATTACHMENT_JPG_GIF)
				{
					CTimeSpan TimeDiff = Time - m_MovDetLastJPGMailTime;
					if (TimeDiff.GetTotalSeconds() >= (LONGLONG)m_nMovDetSendMailSecBetweenMsg)
					{
						sSavedJpegRec = SaveJpegRec(pDib, Time);
						if (CVideoDeviceDoc::SendMail(m_SendMailConfiguration, GetAssignedDeviceName(), Time, _T("REC"), _T(""), sSavedJpegRec))
							m_MovDetLastJPGMailTime = Time;
					}
				}
			}

			// Execute Command
			if (m_bExecCommand && m_nExecCommandMode == 0)
			{
				if (sSavedJpegRec.IsEmpty())
					sSavedJpegRec = SaveJpegRec(pDib, Time);
				ExecCommand(Time, sSavedJpegRec);
			}
		}
	}

	// If in detection state
	BOOL bStoreFrame = (m_bSaveVideo || m_bSaveAnimGIF);
	BOOL bDropFrame = ((CUImagerApp*)::AfxGetApp())->m_bMovDetDropFrames;
	DWORD dwError = ERROR_SUCCESS;
	if (m_bDetectingMovement)
	{
		// Add new frame
		if (bStoreFrame && !bDropFrame)
			dwError = AddNewFrameToNewestList(bMarkStart, pDib);

		// Check if end of detection period
		// Attention: m_nMilliSecondsRecAfterMovementEnd must be at least 1 sec!
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
		// Maximum number of frames reached?
		else if (GetNewestMovementDetectionsListCount() >= m_nDetectionMaxFrames)
			SaveFrameList(FALSE);
	}
	else if (bStoreFrame)
	{
		// Add new frame and shrink to a size of m_nMilliSecondsRecBeforeMovementBegin
		if (!bDropFrame)
			dwError = AddNewFrameToNewestListAndShrink(pDib);
	}
	else
		ClearNewestFrameList();

	// Error: disable frames storing and save the frame list!
	//        Frames storing is re-enabled in CMainFrame::OnTimer()
	//        when the memory usage is normalized.
	if (dwError != ERROR_SUCCESS)
	{
		((CUImagerApp*)::AfxGetApp())->m_bMovDetDropFrames = bDropFrame = TRUE;
		::LogLine(ML_STRING(1815, "OUT OF MEMORY / OVERLOAD (0x%08X): dropping frames"), dwError);
	}

	// Drop frames
	if (bDropFrame)
		ClearNewestFrameList();
}

void CVideoDeviceDoc::ExecCommand(const CTime& Time,
								const CString& sFullFileName/*=_T("")*/,
								const CString& sSmallFileName/*=_T("")*/)
{
	// Critical section is necessary for two distinct reasons:
	// 1. to serialize the access to the m_sExecCommand and m_sExecParams strings
	// 2. to avoid getting called by two separate threads at the same time,
	//    this could happen while switching m_nExecCommandMode
	::EnterCriticalSection(&m_csExecCommand);
	if (m_bWaitExecCommand)
	{
		if (m_hExecCommand)
		{
			if (::WaitForSingleObject(m_hExecCommand, 0) == WAIT_OBJECT_0)
			{
				::CloseHandle(m_hExecCommand);
				m_hExecCommand = NULL;
			}
		}
	}
	else if (m_hExecCommand)
	{
		::CloseHandle(m_hExecCommand);
		m_hExecCommand = NULL;
	}
	if (m_sExecCommand != _T("") && m_hExecCommand == NULL)
	{
		// Replace variables
		CString sExecParams = m_sExecParams;
		CString sSecond, sMinute, sHour, sDay, sMonth, sYear, sMovDetSavesCount;
		sSecond.Format(_T("%02d"), Time.GetSecond());
		sMinute.Format(_T("%02d"), Time.GetMinute());
		sHour.Format(_T("%02d"), Time.GetHour());
		sDay.Format(_T("%02d"), Time.GetDay());
		sMonth.Format(_T("%02d"), Time.GetMonth());
		sYear.Format(_T("%04d"), Time.GetYear());
		sExecParams.Replace(_T("%sec%"), sSecond);
		sExecParams.Replace(_T("%min%"), sMinute);
		sExecParams.Replace(_T("%hour%"), sHour);
		sExecParams.Replace(_T("%day%"), sDay);
		sExecParams.Replace(_T("%month%"), sMonth);
		sExecParams.Replace(_T("%year%"), sYear);
		sExecParams.Replace(_T("%full%"), sFullFileName);
		sExecParams.Replace(_T("%small%"), sSmallFileName);
		
		// Execute command
		//
		// Note: do not use ShellExecuteEx because it creates a separate thread
		//       and does not return until that thread completes. During this time
		//       ShellExecuteEx will pump window messages to prevent windows
		//       owned by the calling thread from appearing hung. 
		//       We also need the STARTF_FORCEOFFFEEDBACK flag which is not available
		//       with ShellExecuteEx, this flag avoids showing the busy cursor while
		//       starting the given process.
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		memset(&si, 0, sizeof(si));
		memset(&pi, 0, sizeof(pi));
		si.cb = sizeof(si);
		si.dwFlags =	STARTF_FORCEOFFFEEDBACK |	// do not display the busy cursor
						STARTF_USESHOWWINDOW;		// use the following wShowWindow
		si.wShowWindow = m_bHideExecCommand ? SW_HIDE : SW_SHOWNORMAL;
		TCHAR lpCommandLine[32768];
		_tcscpy_s(lpCommandLine, _T("\"") + m_sExecCommand + _T("\"") + _T(" ") + sExecParams);
		::CreateProcess(m_sExecCommand, lpCommandLine,
						NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL,
						::GetDriveAndDirName(m_sExecCommand), &si, &pi);
		m_hExecCommand = pi.hProcess;
		if (pi.hThread)
			::CloseHandle(pi.hThread);
	}
	::LeaveCriticalSection(&m_csExecCommand);
}

BOOL CVideoDeviceDoc::ResizeFast(CDib* pSrcDib, CDib* pDstDib)
{
	BOOL res = FALSE;
	AVFrame* pSrcFrame = NULL;
	AVFrame* pDstFrame = NULL;
	SwsContext* pImgConvertCtx = NULL;
	AVPixelFormat src_pix_fmt, dst_pix_fmt;

	// Check
	if (!pSrcDib || !pSrcDib->GetBits() || !pDstDib || !pDstDib->GetBits())
		goto exit;
	src_pix_fmt = CAVRec::AVCodecBMIToPixFormat(pSrcDib->GetBMI());
	dst_pix_fmt = CAVRec::AVCodecBMIToPixFormat(pDstDib->GetBMI());
	if (pSrcDib->GetWidth() <= 0 || pSrcDib->GetHeight() <= 0	||
		src_pix_fmt < 0 || src_pix_fmt >= AV_PIX_FMT_NB			||
		pDstDib->GetWidth() <= 0 || pDstDib->GetHeight() <= 0	||
		dst_pix_fmt < 0 || dst_pix_fmt >= AV_PIX_FMT_NB)
		goto exit;

	// Source frame
	pSrcFrame = av_frame_alloc();
	if (!pSrcFrame)
        goto exit;
	avpicture_fill(	(AVPicture*)pSrcFrame,
					(uint8_t*)pSrcDib->GetBits(),
					src_pix_fmt,
					pSrcDib->GetWidth(),
					pSrcDib->GetHeight());

	// Destination frame
	pDstFrame = av_frame_alloc();
	if (!pDstFrame)
        goto exit;
	avpicture_fill(	(AVPicture*)pDstFrame,
					(uint8_t*)pDstDib->GetBits(),
					dst_pix_fmt,
					pDstDib->GetWidth(),
					pDstDib->GetHeight());

	// Prepare Image Conversion Context
	pImgConvertCtx = sws_getContextHelper(	pSrcDib->GetWidth(),	// Source Width
											pSrcDib->GetHeight(),	// Source Height
											src_pix_fmt,			// Source Format
											pDstDib->GetWidth(),	// Destination Width
											pDstDib->GetHeight(),	// Destination Height
											dst_pix_fmt,			// Destination Format
											SWS_BICUBIC);			// Interpolation (add SWS_PRINT_INFO to debug)
	if (!pImgConvertCtx)
		goto exit;

	// Resize
	if (sws_scale(	pImgConvertCtx,				// Image Convert Context
					pSrcFrame->data,			// Source Data
					pSrcFrame->linesize,		// Source Stride
					0,							// Source Slice Y
					pSrcDib->GetHeight(),		// Source Height
					pDstFrame->data,			// Destination Data
					pDstFrame->linesize) > 0)	// Destination Stride
		res = TRUE;

exit:
	if (pSrcFrame)
		av_frame_free(&pSrcFrame);
	if (pDstFrame)
		av_frame_free(&pDstFrame);
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
	int nJ420ImageSize, qscale;
	AVPixelFormat src_pix_fmt, dst_pix_fmt;

	// Check
	if (!pDib || !pDib->GetBits() || !pMJPEGEncoder || sFileName.IsEmpty())
		goto exit;
	src_pix_fmt = CAVRec::AVCodecBMIToPixFormat(pDib->GetBMI());
	if (pDib->GetWidth() <= 0 || pDib->GetHeight() <= 0	||
		src_pix_fmt < 0 || src_pix_fmt >= AV_PIX_FMT_NB)
		goto exit;

	// Source frame
	pSrcFrame = av_frame_alloc();
	if (!pSrcFrame)
        goto exit;
	avpicture_fill(	(AVPicture*)pSrcFrame,
					(uint8_t*)pDib->GetBits(),
					src_pix_fmt,
					pDib->GetWidth(),
					pDib->GetHeight());

	// Destination frame
	pDstFrame = av_frame_alloc();
	if (!pDstFrame)
        goto exit;
	dst_pix_fmt = AV_PIX_FMT_YUVJ420P; // Full range YUV (0..255)
	nJ420ImageSize = avpicture_get_size(dst_pix_fmt,
										pDib->GetWidth(),
										pDib->GetHeight());
	if (nJ420ImageSize > 0)
		pJ420Buf = (LPBYTE)av_malloc(nJ420ImageSize + FF_INPUT_BUFFER_PADDING_SIZE);
	if (!pJ420Buf)
		goto exit;
	avpicture_fill(	(AVPicture*)pDstFrame,
					(uint8_t*)pJ420Buf,
					dst_pix_fmt,
					pDib->GetWidth(),
					pDib->GetHeight());

	// Convert (first try fast conversion, if source format not supported fall back to sws_scale)
	if (!ITU601JPEGConvert(	src_pix_fmt,			// Source Format
							dst_pix_fmt,			// Destination Format
							pSrcFrame->data,		// Source Data
							pSrcFrame->linesize,	// Source Stride
							pDstFrame->data,		// Destination Data
							pDstFrame->linesize,	// Destination Stride
							pDib->GetWidth(),		// Width
							pDib->GetHeight()))		// Height
	{
		pImgConvertCtx = sws_getContextHelper(	pDib->GetWidth(),	// Source Width
												pDib->GetHeight(),	// Source Height
												src_pix_fmt,		// Source Format
												pDib->GetWidth(),	// Destination Width
												pDib->GetHeight(),	// Destination Height
												dst_pix_fmt,		// Destination Format
												SWS_BICUBIC);		// Interpolation (add SWS_PRINT_INFO to debug)
		if (!pImgConvertCtx)
			goto exit;
		if (sws_scale(	pImgConvertCtx,				// Image Convert Context
						pSrcFrame->data,			// Source Data
						pSrcFrame->linesize,		// Source Stride
						0,							// Source Slice Y
						pDib->GetHeight(),			// Source Height
						pDstFrame->data,			// Destination Data
						pDstFrame->linesize) <= 0)	// Destination Stride
			goto exit;
	}

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
	// Note: as we are a 32-bit application and as the video resolutions get bigger and bigger,
	// we can incur into a virtual memory address space fragmentation problem especially if many threads
	// are running interleaved and allocating blocks that do not fit into the heap (blocks bigger than
	// 512 KB are gotten from normal VM which is quite prone to fragmentation). So we better set the
	// threads count to 1, later on when we get a 64-bit application we can set the threads count to:
	// ((CUImagerApp*)::AfxGetApp())->m_nThreadCount
	dwEncodedLen = pMJPEGEncoder->Encode(qscale,	// 2: best quality, 31: worst quality
										&DstBmi, pJ420Buf,
										1);			// threads count
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
		av_frame_free(&pSrcFrame);
	if (pDstFrame)
		av_frame_free(&pDstFrame);
	if (pImgConvertCtx)
	{
		sws_freeContext(pImgConvertCtx);
		pImgConvertCtx = NULL;
	}
	if (pJ420Buf)
	{
		av_free(pJ420Buf);
		pJ420Buf = NULL;
	}
	return res;
}

BOOL CVideoDeviceDoc::CHttpThread::PollAndClean(BOOL bDoNewPoll)
{
	BOOL res = FALSE;
	CNetCom* pNetCom = NULL;
	CHttpParseProcess* pHttpVideoParseProcess = NULL;

	// New Poll Connection
	if (bDoNewPoll)
	{
		// Allocate
		pNetCom = new CNetCom;
		if (!pNetCom)
			return FALSE;
		pHttpVideoParseProcess = new CHttpParseProcess(m_pDoc);
		if (!pHttpVideoParseProcess)
		{
			delete pNetCom;
			return FALSE;
		}
		
		// Copy the authorization variables and increment the client nonce count
		pHttpVideoParseProcess->m_AnswerAuthorizationType = m_pDoc->m_pHttpVideoParseProcess->m_AnswerAuthorizationType;
		pHttpVideoParseProcess->m_bAuthorized = m_pDoc->m_pHttpVideoParseProcess->m_bAuthorized;
		pHttpVideoParseProcess->m_sRealm = m_pDoc->m_pHttpVideoParseProcess->m_sRealm;
		pHttpVideoParseProcess->m_sQop = m_pDoc->m_pHttpVideoParseProcess->m_sQop;
		pHttpVideoParseProcess->m_sNonce = m_pDoc->m_pHttpVideoParseProcess->m_sNonce;
		pHttpVideoParseProcess->m_sAlgorithm = m_pDoc->m_pHttpVideoParseProcess->m_sAlgorithm;
		pHttpVideoParseProcess->m_sOpaque = m_pDoc->m_pHttpVideoParseProcess->m_sOpaque;
		pHttpVideoParseProcess->m_dwCNonceCount = m_pDoc->m_pHttpVideoParseProcess->m_dwCNonceCount;
		m_pDoc->m_pHttpVideoParseProcess->m_dwCNonceCount++;

		// Init http version, format type and sizes
		pHttpVideoParseProcess->m_bOldVersion = m_pDoc->m_pHttpVideoParseProcess->m_bOldVersion;
		pHttpVideoParseProcess->m_FormatType = m_pDoc->m_pHttpVideoParseProcess->m_FormatType;
		for (int i = 0 ; i < m_pDoc->m_pHttpVideoParseProcess->m_Sizes.GetSize() ; i++)
			pHttpVideoParseProcess->m_Sizes.Add(m_pDoc->m_pHttpVideoParseProcess->m_Sizes[i]);

		// Connect
		if (Connect(pNetCom,
					pHttpVideoParseProcess,
					m_pDoc->m_pVideoNetCom->GetSocketFamily()))
		{
			m_HttpVideoNetComList.AddTail(pNetCom);
			m_HttpVideoParseProcessList.AddTail(pHttpVideoParseProcess);
			pHttpVideoParseProcess->SendRequest();
			res = TRUE;
		}
		else
		{
			delete pNetCom;
			delete pHttpVideoParseProcess;
			res = FALSE;
		}
	}

	// Clean-Up
	while (m_HttpVideoNetComList.GetCount() > (bDoNewPoll ? 1 : 0))
	{
		pNetCom = m_HttpVideoNetComList.GetHead();
		if (pNetCom)
		{
			// Remove oldest connection?
			CTimeSpan ConnectionAge = CTime::GetCurrentTime() - pNetCom->m_InitTime;
			if (pNetCom->IsShutdown()											||	// done?
				ConnectionAge.GetTotalSeconds() >= DEFAULT_CONNECTION_TIMEOUT	||	// too old?
				ConnectionAge.GetTotalSeconds() < 0								||	// "
				!bDoNewPoll)														// too many open connections?
			{
				delete pNetCom; // this calls Close() which blocks till all net threads are done
				m_HttpVideoNetComList.RemoveHead();
				pHttpVideoParseProcess = m_HttpVideoParseProcessList.GetHead();
				if (pHttpVideoParseProcess)
					delete pHttpVideoParseProcess;
				m_HttpVideoParseProcessList.RemoveHead();
			}
			// Nothing more to clean-up... exit loop
			else
				break;
		}
		// Should never happen...
		else
		{
			m_HttpVideoNetComList.RemoveHead();
			pHttpVideoParseProcess = m_HttpVideoParseProcessList.GetHead();
			if (pHttpVideoParseProcess)
				delete pHttpVideoParseProcess;
			m_HttpVideoParseProcessList.RemoveHead();
		}
	}

	return res;
}

void CVideoDeviceDoc::CHttpThread::CleanUpAllPollConnections()
{
	// Start Shutdown All Connections
	POSITION pos = m_HttpVideoNetComList.GetHeadPosition();
	while (pos)
	{
		CNetCom* pNetCom = m_HttpVideoNetComList.GetNext(pos);
		if (pNetCom)
			pNetCom->ShutdownConnection_NoBlocking();
	}

	// Wait Till All Connections are down
	while (!m_HttpVideoNetComList.IsEmpty())
	{
		CNetCom* pNetCom = m_HttpVideoNetComList.GetHead();
		if (pNetCom)
			delete pNetCom; // This calls Close() and blocks till all threads are done
		m_HttpVideoNetComList.RemoveHead();
		CHttpParseProcess* pHttpVideoParseProcess = m_HttpVideoParseProcessList.GetHead();
		if (pHttpVideoParseProcess)
			delete pHttpVideoParseProcess;
		m_HttpVideoParseProcessList.RemoveHead();
	}
}

BOOL CVideoDeviceDoc::CHttpThread::Connect(	CNetCom* pNetCom,
											CVideoDeviceDoc::CHttpParseProcess* pParseProcess,
											int nSocketFamily,
											HANDLE hConnectedEvent/*=NULL*/,
											HANDLE hConnectFailedEvent/*=NULL*/)
{
	// Check
	if (!pNetCom)
		return FALSE;

	// Init
	return pNetCom->Init(pParseProcess,					// Parser
						m_pDoc->m_sGetFrameVideoHost,	// Peer Address (IP or Host Name)
						m_pDoc->m_nGetFrameVideoPort,	// Peer Port
						hConnectedEvent,				// Handle to an Event Object that will get Connect Events
						hConnectFailedEvent,			// Handle to an Event Object that will get Connect Failed Events
						NULL,							// Handle to an Event Object that will get Read Events
						nSocketFamily);					// Socket family priority: AF_INET for IPv4, AF_INET6 for IPv6
}

int CVideoDeviceDoc::CHttpThread::Work()
{
	ASSERT(m_pDoc);
	int nAlarmLevel = 0;
	int nConnectionKeepAliveSupported = HTTP_MIN_KEEPALIVE_REQUESTS; // 0: not supported, 1: supported, >1: to be verified

	for (;;)
	{
		// Set wait delay for HTTP jpeg snapshots mode
		DWORD dwWaitDelay = HTTP_THREAD_DEFAULT_DELAY;
		if (m_pDoc->m_dFrameRate > 0.0)
			dwWaitDelay = (DWORD)Round(1000.0 / m_pDoc->m_dFrameRate);
		if (nAlarmLevel == 1)
		{
			dwWaitDelay = MAX(2U*dwWaitDelay, HTTP_THREAD_MIN_DELAY_ALARM1);
			dwWaitDelay = MIN(dwWaitDelay, HTTP_THREAD_MAX_DELAY_ALARM);
		}
		else if (nAlarmLevel == 2)
		{
			dwWaitDelay = MAX(4U*dwWaitDelay, HTTP_THREAD_MIN_DELAY_ALARM2);
			dwWaitDelay = MIN(dwWaitDelay, HTTP_THREAD_MAX_DELAY_ALARM);
		}
		else if (nAlarmLevel >= 3)
		{
			dwWaitDelay = MAX(8U*dwWaitDelay, HTTP_THREAD_MIN_DELAY_ALARM3);
			dwWaitDelay = MIN(dwWaitDelay, HTTP_THREAD_MAX_DELAY_ALARM);
		}	

		// Wait for events
		DWORD Event = ::WaitForMultipleObjects(4, m_hEventArray, FALSE, dwWaitDelay);
		switch (Event)
		{
			// Shutdown Event (for both HTTP jpeg snapshots and HTTP motion jpeg modes)
			case WAIT_OBJECT_0 :		
			{
				CleanUpAllPollConnections();
				return 0;
			}

			// Http Setup Video Connection Event (for HTTP jpeg snapshots init and HTTP motion jpeg mode)
			case WAIT_OBJECT_0 + 1 :
			{
				::ResetEvent(m_hEventArray[1]);
				nConnectionKeepAliveSupported = HTTP_MIN_KEEPALIVE_REQUESTS; // 0: not supported, 1: supported, >1: to be verified
				::EnterCriticalSection(&m_csVideoConnectRequestParams);
				BOOL bResetHttpGetFrameLocationPos = m_bResetHttpGetFrameLocationPos;
				::LeaveCriticalSection(&m_csVideoConnectRequestParams);
				CleanUpAllPollConnections();
				m_pDoc->m_pVideoNetCom->Close(); // this also empties the rx & tx fifos
				if (bResetHttpGetFrameLocationPos)
					m_pDoc->m_nHttpGetFrameLocationPos = 0;
				m_pDoc->m_pHttpVideoParseProcess->m_bPollNextJpeg = FALSE;
				if (m_pDoc->m_bCaptureAudio && !m_pDoc->m_bCaptureAudioFromStream)
					m_pDoc->m_CaptureAudioThread.Start();
				if (!Connect(m_pDoc->m_pVideoNetCom,
							m_pDoc->m_pHttpVideoParseProcess,
							AF_INET,			// Socket family priority: AF_INET for IPv4, AF_INET6 for IPv6
							m_hEventArray[2],	// Http Video Connected Event
							m_hEventArray[3]))	// Http Video Connect Failed Event
					m_pDoc->ConnectErr(ML_STRING(1465, "Cannot connect to camera"), m_pDoc->GetDeviceName());
				break;
			}

			// Http Video Connected Event (for HTTP jpeg snapshots init and HTTP motion jpeg mode)
			case WAIT_OBJECT_0 + 2 :
			{
				::ResetEvent(m_hEventArray[2]);
				::EnterCriticalSection(&m_csVideoConnectRequestParams);
				CString sRequest = m_sVideoRequest;
				::LeaveCriticalSection(&m_csVideoConnectRequestParams);
				::PostMessage(m_pDoc->GetView()->GetSafeHwnd(), WM_THREADSAFE_SETDOCUMENTTITLE, 0, 0); // update camera advanced settings title
				if (sRequest == _T(""))
					m_pDoc->m_pHttpVideoParseProcess->SendRequest();
				else
					m_pDoc->m_pHttpVideoParseProcess->SendRawRequest(sRequest);
				break;
			}

			// Http Video Connection failed Event (for HTTP jpeg snapshots init and HTTP motion jpeg mode)
			case WAIT_OBJECT_0 + 3 :
			{
				::ResetEvent(m_hEventArray[3]);
				m_pDoc->ConnectErr(ML_STRING(1465, "Cannot connect to camera"), m_pDoc->GetDeviceName());
				break;
			}

			// Timeout
			case WAIT_TIMEOUT :		
			{
				// Poll (only HTTP jpeg snapshots mode)
				if (m_pDoc->m_pHttpVideoParseProcess->m_bPollNextJpeg)
				{
					// Keep-alive: just send the request to the already open connection
					if (m_pDoc->m_pHttpVideoParseProcess->m_bConnectionKeepAlive &&
						nConnectionKeepAliveSupported > 0)
					{
						// Keep-alive support already verified
						if (nConnectionKeepAliveSupported == 1)
							m_pDoc->m_pHttpVideoParseProcess->SendRequest();
						// Verify keep-alive support
						// (some crappy HTTP/1.1 servers do not return the "Connection: close"
						// response to our "Connection: keep-alive" request when they do not
						// support keep-alive)
						else
						{
							if (m_pDoc->m_pHttpVideoParseProcess->SendRequest())
								nConnectionKeepAliveSupported--;	// try several times to make sure the connection stays open
							else
								nConnectionKeepAliveSupported = 0;	// no keep alive support
						}
					}
					// Some servers by default limit the amount of requests per connection,
					// after a while the connection is closed by the server -> reconnect
					else if (nConnectionKeepAliveSupported == 1)
						SetEventVideoConnect();
					else
					{
						if (m_HttpVideoNetComList.GetCount() >= HTTP_MAXPOLLS_ALARM3)
							nAlarmLevel = 3;
						else if (m_HttpVideoNetComList.GetCount() >= HTTP_MAXPOLLS_ALARM2)
							nAlarmLevel = 2;
						else if (m_HttpVideoNetComList.GetCount() >= HTTP_MAXPOLLS_ALARM1)
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

int CVideoDeviceDoc::CRtspThread::Work()
{
	ASSERT(m_pDoc);

	// Init COM for audio play
	::CoInitialize(NULL);

	for (;;)
	{
		int ret = 0;
		AVCodec* pVideoCodec = NULL;
		AVCodecContext* pVideoCodecCtx = NULL;
		AVCodec* pAudioCodec = NULL;
		AVCodecContext* pAudioCodecCtx = NULL;
		SwsContext* pImgConvertCtx = NULL;
		AVFrame* pVideoFrame = NULL;
		AVFrame* pAudioFrame = NULL;
		AVFrame* pVideoFrameI420 = NULL;
		LPBYTE pI420Buf = NULL;
		AVFormatContext* pFormatCtx = avformat_alloc_context();
		CAudioTools* pAudioTools = NULL;
		CAudioPlay* pAudioPlay = NULL;
		BOOL bAudioSupported = FALSE;

		// Set options
		AVDictionary* opts = NULL;
		if (m_pDoc->m_bPreferTcpforRtsp)
			av_dict_set(&opts, "rtsp_flags", "prefer_tcp", 0);		// if set, and if TCP is available as RTP transport, then TCP will be tried first instead of UDP
		av_dict_set_int(&opts, "stimeout", RTSP_SOCKET_TIMEOUT, 0);	// set timeout (in microseconds) of socket I/O operations
		// SO_RCVBUF is the size of the buffer the system allocates to hold the data arriving
		// into the given socket during the time between it arrives over the network and when it
		// is read by the program that owns this socket. With TCP, if data arrives and you aren't
		// reading it, the buffer will fill up, and the sender will be told to slow down (using TCP
		// window adjustment mechanism). For UDP, once the buffer is full, new packets will just be
		// discarded.
		// SO_RCVBUF is set by the OS to 8K for Win7 or older and to 64K for newer. ffmpeg inits it
		// to UDP_MAX_PKT_SIZE. The user can change it with the "buffer_size" option.
		// Note: the 'circular_buffer_size' option was set but it is not supported warning is emitted
		//       in udp.c. This warning cannot be removed, but is not interfering with the setting of
		//       the following 'buffer_size' which is something different.
		av_dict_set_int(&opts, "buffer_size", 10485760, 0); // 10 MB should be enough for 4K cams

		// Open rtsp
		CStringA sAnsiURL(m_sURL);
		if ((ret = avformat_open_input(&pFormatCtx, sAnsiURL, NULL, &opts)) < 0)
			goto free;

		// Search video and audio streams for which we have decoders
		pFormatCtx->max_analyze_duration = 10*AV_TIME_BASE; // the default of 5*AV_TIME_BASE = 5 sec is too low
		if ((ret = avformat_find_stream_info(pFormatCtx, NULL)) < 0)
			goto free;
		int nVideoStreamIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pVideoCodec, 0);
		if (nVideoStreamIndex < 0)
		{
			ret = nVideoStreamIndex;
			goto free;
		}
		int nAudioStreamIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &pAudioCodec, 0);

		// Set capture FourCC from codec id
		const struct AVCodecTag *table[] = { avformat_get_riff_video_tags(), 0 };
		m_pDoc->m_CaptureBMI.bmiHeader.biCompression = av_codec_get_tag(table, pVideoCodec->id);

		// Open codec context
		pVideoCodecCtx = pFormatCtx->streams[nVideoStreamIndex]->codec;
		if ((ret = avcodec_open2(pVideoCodecCtx, pVideoCodec, 0)) < 0)
			goto free;
		if (nAudioStreamIndex >= 0)
		{
			pAudioCodecCtx = pFormatCtx->streams[nAudioStreamIndex]->codec;
			if (avcodec_open2(pAudioCodecCtx, pAudioCodec, 0) >= 0 && pAudioCodecCtx->channels <= 2)
				bAudioSupported = TRUE;
			else
				::LogLine(_T("%s, %s"), m_pDoc->GetAssignedDeviceName(), ML_STRING(1492, "Audio format/encoding not supported"));
		}

		// Set codec IDs (-1 means not set)
		if (pVideoCodec)
			m_nVideoCodecID = pVideoCodec->id;
		else
			m_nVideoCodecID = AV_CODEC_ID_NONE;
		if (pAudioCodec)
			m_nAudioCodecID = pAudioCodec->id;
		else
			m_nAudioCodecID = AV_CODEC_ID_NONE;

		// Init Video
		pImgConvertCtx = sws_getContextHelper(	pVideoCodecCtx->width, pVideoCodecCtx->height,
												pVideoCodecCtx->pix_fmt,
												pVideoCodecCtx->width, pVideoCodecCtx->height,
												AV_PIX_FMT_YUV420P,
												SWS_BICUBIC);
		if (!pImgConvertCtx)
			goto free;
		pVideoFrame = av_frame_alloc();
		if (!pVideoFrame)
			goto free;
		pVideoFrameI420 = av_frame_alloc();
		if (!pVideoFrameI420)
			goto free;
		int nI420ImageSize = avpicture_get_size(AV_PIX_FMT_YUV420P, pVideoCodecCtx->width, pVideoCodecCtx->height);
		if (nI420ImageSize <= 0)
			goto free;
		pI420Buf = (LPBYTE)av_malloc(2 * (nI420ImageSize + FF_INPUT_BUFFER_PADDING_SIZE));
		if (!pI420Buf)
			goto free;
		avpicture_fill((AVPicture *)pVideoFrameI420, pI420Buf, AV_PIX_FMT_YUV420P, pVideoCodecCtx->width, pVideoCodecCtx->height);

		// Init Audio
		if (bAudioSupported)
		{
			pAudioFrame = av_frame_alloc();
			if (!pAudioFrame)
				goto free;
			int bits = av_get_bytes_per_sample(pAudioCodecCtx->sample_fmt) << 3;
			if (bits <= 16)
				WaveInitFormat(pAudioCodecCtx->channels, pAudioCodecCtx->sample_rate, bits, m_pDoc->m_pSrcWaveFormat);
			else
				WaveInitFormat(pAudioCodecCtx->channels, pAudioCodecCtx->sample_rate, 16, m_pDoc->m_pSrcWaveFormat);
		}

		// Start Audio Capture Thread?
		// Note: use the stream's m_pSrcWaveFormat to avoid problems
		//       when toggling the m_bCaptureAudioFromStream flag
		if (m_pDoc->m_bCaptureAudio && !m_pDoc->m_bCaptureAudioFromStream)
			m_pDoc->m_CaptureAudioThread.Start();

		// Exit?
		if (DoExit())
			goto free;

		// Get frames
		AVPacket avpkt;
		av_init_packet(&avpkt);
		avpkt.data = NULL; // set data to NULL, let the demuxer fill it
		avpkt.size = 0;
		while ((ret = av_read_frame(pFormatCtx, &avpkt)) >= 0)
		{
			// Get underlying transport, that may change while streaming,
			// so poll it regularly.
			// The offset of lower_transport from RTSPState in rtsp.h
			// (cannot include rtsp.h here as it doesn't compile)
			// has been calculated taking into account the padding/alignment
			// for the int64_t variables.
			//
			// ATTENTION: when updating the ffmpeg source make sure the offset
			//            is still correct!!
			//
			m_nUnderlyingTransport = *((int*)((LPBYTE)(pFormatCtx->priv_data) + 564));
			
			AVPacket orig_pkt = avpkt;

			do
			{
				int decoded = avpkt.size;

				// Video packet
				if (avpkt.stream_index == nVideoStreamIndex)
				{
					// Decode
					int got_picture = 0;
					ret = avcodec_decode_video2(pVideoCodecCtx, pVideoFrame, &got_picture, &avpkt);
					if (ret < 0)
					{
						av_free_packet(&orig_pkt);
						goto free;
					}
					decoded = MIN(ret, avpkt.size);

					// Init if size changed
					if (m_pDoc->m_DocRect.right != pVideoCodecCtx->width ||
						m_pDoc->m_DocRect.bottom != pVideoCodecCtx->height)
					{
						m_pDoc->m_ProcessFrameBMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
						m_pDoc->m_ProcessFrameBMI.bmiHeader.biWidth = (DWORD)pVideoCodecCtx->width;
						m_pDoc->m_ProcessFrameBMI.bmiHeader.biHeight = (DWORD)pVideoCodecCtx->height;
						m_pDoc->m_ProcessFrameBMI.bmiHeader.biPlanes = 1; // must be 1
						m_pDoc->m_ProcessFrameBMI.bmiHeader.biBitCount = 12;
						m_pDoc->m_ProcessFrameBMI.bmiHeader.biCompression = FCC('I420');
						m_pDoc->m_ProcessFrameBMI.bmiHeader.biSizeImage = nI420ImageSize;
						m_pDoc->m_DocRect.right = m_pDoc->m_ProcessFrameBMI.bmiHeader.biWidth;
						m_pDoc->m_DocRect.bottom = m_pDoc->m_ProcessFrameBMI.bmiHeader.biHeight;

						// Free Movement Detector because we changed size and/or format!
						m_pDoc->FreeMovementDetector();

						// Update
						if (m_pDoc->m_bSizeToDoc)
						{
							// This sizes the view to m_DocRect in normal screen mode,
							// in full-screen mode it updates m_ZoomRect from m_DocRect
							::PostMessage(m_pDoc->GetView()->GetSafeHwnd(),
								WM_THREADSAFE_UPDATEWINDOWSIZES,
								(WPARAM)UPDATEWINDOWSIZES_SIZETODOC,
								(LPARAM)0);
							m_pDoc->m_bSizeToDoc = FALSE;
						}
						else
						{
							// In full-screen mode it updates m_ZoomRect from m_DocRect
							::PostMessage(m_pDoc->GetView()->GetSafeHwnd(),
								WM_THREADSAFE_UPDATEWINDOWSIZES,
								(WPARAM)0,
								(LPARAM)0);
						}
						::PostMessage(m_pDoc->GetView()->GetSafeHwnd(),
							WM_THREADSAFE_SETDOCUMENTTITLE,
							0, 0);
						::PostMessage(m_pDoc->GetView()->GetSafeHwnd(),
							WM_THREADSAFE_UPDATE_PHPPARAMS,
							0, 0);
					}

					// Convert and Process
					if (got_picture)
					{
						if ((ret = sws_scale(pImgConvertCtx, pVideoFrame->data, pVideoFrame->linesize, 0, pVideoCodecCtx->height, pVideoFrameI420->data, pVideoFrameI420->linesize)) <= 0)
						{
							av_free_packet(&orig_pkt);
							goto free;
						}
						m_pDoc->m_lEffectiveDataRateSum += avpkt.size;
						m_pDoc->ProcessI420Frame(pI420Buf, nI420ImageSize);
					}
				}
				// Audio Packet
				else if (avpkt.stream_index == nAudioStreamIndex && bAudioSupported &&
						m_pDoc->m_bCaptureAudio && m_pDoc->m_bCaptureAudioFromStream)
				{
					// Init audio listen
					if (!pAudioTools)
						pAudioTools = new CAudioTools;
					if (!pAudioPlay)
						pAudioPlay = new CAudioPlay;
					if (pAudioPlay && !pAudioPlay->IsInit())
						pAudioPlay->Init(10000000);

					/* Decode
					* For planar audio, each channel has a separate data pointer, and
					* pAudioFrame->linesize[i] contains the size of each channel buffer.
					* For packed audio, there is just one data pointer, and
					* pAudioFrame->linesize[0] contains the total size of the buffer for
					* all channels.
					* - pAudioFrame->nb_samples is the number of audio samples (per channel)
					* - pAudioCodecCtx->channels
					* - pAudioCodecCtx->sample_rate
					* - pAudioCodecCtx->sample_fmt
					* - av_get_bytes_per_sample(pAudioCodecCtx->sample_fmt) returns the
					*   bytes count per sample and per channel
					* - Attention:
					*   pAudioFrame->linesize[0]  >=
					*   av_get_bytes_per_sample(pAudioCodecCtx->sample_fmt) * pAudioCodecCtx->channels * pAudioFrame->nb_samples
					*/
					int got_frame = 0;
					ret = avcodec_decode_audio4(pAudioCodecCtx, pAudioFrame, &got_frame, &avpkt);

					// Process
					if (ret >= 0)
					{
						decoded = MIN(ret, avpkt.size);

						// Add samples to queue and limit its size
						if (got_frame)
						{
							CUserBuf UserBuf;

							// Convert from planar float to packed 16 bit
							if (pAudioCodecCtx->sample_fmt == AV_SAMPLE_FMT_FLTP)
							{
								// Mono
								if (pAudioCodecCtx->channels == 1)
								{
									UserBuf.m_dwSize = 2 * pAudioFrame->nb_samples;
									UserBuf.m_pBuf = (LPBYTE)av_malloc(UserBuf.m_dwSize);
									if (!UserBuf.m_pBuf)
										goto free;

									// Convert
									// (positive peaks can go up to +32767, negative peaks to -32768 and silence is 0)
									short* pDst = (short*)UserBuf.m_pBuf;
									float* pSrc = (float*)pAudioFrame->data[0];
									for (int i = 0; i < pAudioFrame->nb_samples; i++)
									{	
										pDst[i] = (short)(pSrc[i] * 32767.0f);
									}
								}
								// Stereo
								else
								{
									UserBuf.m_dwSize = 4 * pAudioFrame->nb_samples;
									UserBuf.m_pBuf = (LPBYTE)av_malloc(UserBuf.m_dwSize);
									if (!UserBuf.m_pBuf)
										goto free;

									// Convert
									// (positive peaks can go up to +32767, negative peaks to -32768 and silence is 0)
									short* pDst = (short*)UserBuf.m_pBuf;
									float* pSrc0 = (float*)pAudioFrame->data[0];
									float* pSrc1 = (float*)pAudioFrame->data[1];
									for (int i = 0; i < pAudioFrame->nb_samples; i++)
									{
										// Channel 0
										pDst[2*i] = (short)(pSrc0[i] * 32767.0f);

										// Channel 1
										pDst[2*i + 1] = (short)(pSrc1[i] * 32767.0f);
									}
								}
							}
							// Convert from packed float to packed 16 bit
							else if (pAudioCodecCtx->sample_fmt == AV_SAMPLE_FMT_FLT)
							{
								// Mono
								if (pAudioCodecCtx->channels == 1)
								{
									UserBuf.m_dwSize = 2 * pAudioFrame->nb_samples;
									UserBuf.m_pBuf = (LPBYTE)av_malloc(UserBuf.m_dwSize);
									if (!UserBuf.m_pBuf)
										goto free;

									// Convert
									// (positive peaks can go up to +32767, negative peaks to -32768 and silence is 0)
									short* pDst = (short*)UserBuf.m_pBuf;
									float* pSrc = (float*)pAudioFrame->data[0];
									for (int i = 0; i < pAudioFrame->nb_samples; i++)
									{
										pDst[i] = (short)(pSrc[i] * 32767.0f);
									}
								}
								// Stereo
								else
								{
									UserBuf.m_dwSize = 4 * pAudioFrame->nb_samples;
									UserBuf.m_pBuf = (LPBYTE)av_malloc(UserBuf.m_dwSize);
									if (!UserBuf.m_pBuf)
										goto free;

									// Convert
									// (positive peaks can go up to +32767, negative peaks to -32768 and silence is 0)
									short* pDst = (short*)UserBuf.m_pBuf;
									float* pSrc = (float*)pAudioFrame->data[0];
									for (int i = 0; i < pAudioFrame->nb_samples; i++)
									{
										// Channel 0
										pDst[2*i] = (short)(pSrc[2*i] * 32767.0f);

										// Channel 1
										pDst[2*i + 1] = (short)(pSrc[2*i + 1] * 32767.0f);
									}
								}
							}
							else
							{
								UserBuf.m_dwSize = av_get_bytes_per_sample(pAudioCodecCtx->sample_fmt) * pAudioCodecCtx->channels * pAudioFrame->nb_samples;
								UserBuf.m_pBuf = (LPBYTE)av_malloc(UserBuf.m_dwSize);
								if (!UserBuf.m_pBuf)
									goto free;
								memcpy(UserBuf.m_pBuf, pAudioFrame->data[0], UserBuf.m_dwSize);
							}
							::EnterCriticalSection(&m_pDoc->m_csAudioList);
							m_pDoc->m_AudioList.AddTail(UserBuf);
							m_pDoc->AudioListen(UserBuf.m_pBuf, UserBuf.m_dwSize, pAudioTools, pAudioPlay);
							if (m_pDoc->m_AudioList.GetCount() > AUDIO_MAX_LIST_SIZE)
							{
								UserBuf = m_pDoc->m_AudioList.RemoveHead();
								if (UserBuf.m_pBuf)
								{
									av_free(UserBuf.m_pBuf);
									UserBuf.m_pBuf = NULL;
								}
							}
							::LeaveCriticalSection(&m_pDoc->m_csAudioList);
						}
					}
				}

				avpkt.data += decoded;
				avpkt.size -= decoded;
			}
			while (avpkt.size > 0);

			// Free
			av_free_packet(&orig_pkt);

			// Exit?
			if (DoExit())
				goto free;
		}

		// Free
	free:
		/*
		Close a given AVCodecContext and free all the data associated with it
		(but not the AVCodecContext itself).
		Calling this function on an AVCodecContext that hasn't been opened will free
		the codec-specific data allocated in avcodec_alloc_context3() /
		avcodec_get_context_defaults3() with a non-NULL codec. Subsequent calls will
		do nothing.
		*/
		if (pVideoCodecCtx)
			avcodec_close(pVideoCodecCtx);
		if (pAudioCodecCtx)
			avcodec_close(pAudioCodecCtx);
		if (pVideoFrame)
			av_frame_free(&pVideoFrame);
		if (pAudioFrame)
			av_frame_free(&pAudioFrame);
		if (pVideoFrameI420)
			av_frame_free(&pVideoFrameI420);
		if (pI420Buf)
			av_free(pI420Buf);
		if (pImgConvertCtx)
			sws_freeContext(pImgConvertCtx);
		if (pFormatCtx)
			avformat_close_input(&pFormatCtx);
		av_dict_free(&opts);
		if (pAudioPlay)
		{
			delete pAudioPlay;
			pAudioPlay = NULL;
		}
		if (pAudioTools)
		{
			delete pAudioTools;
			pAudioTools = NULL;
		}

		// Exit?
		if (DoExit())
			goto exit;
		else
		{
			// Error message
			CString sErrorMsg;
			switch (ret)
			{
				case AVERROR_HTTP_BAD_REQUEST:	sErrorMsg = _T("400 Bad Request"); break;
				case AVERROR_HTTP_UNAUTHORIZED:	sErrorMsg = ML_STRING(1780, "Authorization failed"); break;
				case AVERROR_HTTP_FORBIDDEN:	sErrorMsg = _T("403 Forbidden"); break;
				case AVERROR_HTTP_NOT_FOUND:	sErrorMsg = _T("404 Not Found"); break;
				case AVERROR_HTTP_OTHER_4XX:	sErrorMsg = _T("Error 4XX"); break;
				case AVERROR_HTTP_SERVER_ERROR:	sErrorMsg = _T("Error 5XX"); break;
				default:						sErrorMsg = ML_STRING(1465, "Cannot connect to camera"); break;
			}
			m_pDoc->ConnectErr(sErrorMsg, m_pDoc->GetDeviceName());

			// Wait some time before reconnecting
			if (::WaitForSingleObject(m_hKillEvent, DEFAULT_CONNECTION_TIMEOUT * 1000) == WAIT_OBJECT_0)
				goto exit;

			// Reconnect
			m_pDoc->ClearConnectErr();
		}
	}

exit:
	::CoUninitialize();
	return 0;
}

int CVideoDeviceDoc::CWatchdogThread::Work()
{
	ASSERT(m_pDoc);

	// Init vars
	CTime LastHttpReconnectTime = 0; // set time far in the past, 0 is the same as CTime(0)
	BOOL bDeviceAlert = FALSE;

	// Watch
	for (;;)
	{
		DWORD Event = ::WaitForSingleObject(GetKillEvent(), WATCHDOG_CHECK_TIME);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :
				return 0;

			// Check
			case WAIT_TIMEOUT :		
			{
				// Current times
				DWORD dwCurrentUpTime = ::timeGetTime();
				CTime CurrentTime = CTime::GetCurrentTime();

				// Video watchdog
				DWORD dwMsSinceLastProcessFrame = dwCurrentUpTime - (DWORD)m_pDoc->m_lCurrentInitUpTime;
				m_pDoc->m_bWatchDogVideoAlarm = (dwMsSinceLastProcessFrame > WATCHDOG_THRESHOLD);

				// SaveFrameList() may be called many times till
				// CSaveFrameListThread::Work() reacts:
				// it's not a problem because CSaveFrameListThread::Work()
				// removes empty lists
				if (m_pDoc->m_bWatchDogVideoAlarm						&&
					m_pDoc->m_bDetectingMovement						&&
					m_pDoc->GetNewestMovementDetectionsListCount() > 0	&&
					(m_pDoc->m_bSaveVideo || m_pDoc->m_bSaveAnimGIF))
					m_pDoc->SaveFrameList(FALSE);

				// Watchdog actions
				if (m_pDoc->m_bWatchDogVideoAlarm)
				{
					// Http reconnect
					if (m_pDoc->m_pVideoNetCom)
					{
						CTimeSpan TimeSpan = CurrentTime - LastHttpReconnectTime;
						if (TimeSpan.GetTotalSeconds() > DEFAULT_CONNECTION_TIMEOUT)
						{
							m_pDoc->ClearConnectErr();
							LastHttpReconnectTime = CurrentTime;
							m_pDoc->m_HttpThread.SetEventVideoConnect(_T(""), TRUE);
						}
					}

					// Device Alert
					if (dwMsSinceLastProcessFrame > WATCHDOG_MALFUNCTION_THRESHOLD && !bDeviceAlert)
					{
						if (m_pDoc->m_bSendMailMalfunction)
							CVideoDeviceDoc::SendMail(m_pDoc->m_SendMailConfiguration, m_pDoc->GetAssignedDeviceName(), CurrentTime, _T("OFF!"));
						bDeviceAlert = TRUE;
					}
				}
				// Clear
				else
				{
					if (bDeviceAlert)
					{
						if (m_pDoc->m_bSendMailMalfunction)
							CVideoDeviceDoc::SendMail(m_pDoc->m_SendMailConfiguration, m_pDoc->GetAssignedDeviceName(), CurrentTime, _T("ON"));
						bDeviceAlert = FALSE;
					}
				}

				// Trigger drawing in case no frames reaching
				if (m_pDoc->GetView())
				{
					::PostMessage(	m_pDoc->GetView()->GetSafeHwnd(),
									WM_THREADSAFE_UPDATEWINDOWSIZES,
									(WPARAM)UPDATEWINDOWSIZES_INVALIDATE,
									(LPARAM)0);
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
												LONGLONG llDeleteOlderThanDays,
												const CTime& CurrentTime)
{
	CString sDir;
	int nYear, nMonth, nDay;
	for (int pos = 0 ; pos < FileFind.GetDirsCount() ; pos++)
	{
		sDir = FileFind.GetDirName(pos);
		sDir.TrimRight(_T('\\'));
		CString sRootDirName = FileFind.GetRootDirName();
		sRootDirName.TrimRight(_T('\\'));
		int nRootDirNameSize = sRootDirName.GetLength() + 1; // + 1 for ending backslash
		sDir.Delete(0, nRootDirNameSize);
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
			nDay = ::GetLastDayOfMonth(nMonth, nYear);
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
		// Note: the directory time is set to 23:59:59 so that GetDays()
		// returns one or greater only after 24-hours passed from 23:59:59 
		CTime DirTime(nYear, nMonth, nDay, 23, 59, 59);
		CTimeSpan TimeDiff = CurrentTime - DirTime;
		if (TimeDiff.GetDays() >= llDeleteOlderThanDays &&
			::IsExistingDir(FileFind.GetDirName(pos)))
			::DeleteDir(FileFind.GetDirName(pos));

		// Do Exit?
		if (DoExit())
			return FALSE;
	}

	return TRUE;
}

BOOL CVideoDeviceDoc::CDeleteThread::CalcOldestDir(	CSortableFileFind& FileFind,
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
		CString sRootDirName = FileFind.GetRootDirName();
		sRootDirName.TrimRight(_T('\\'));
		int nRootDirNameSize = sRootDirName.GetLength() + 1; // + 1 for ending backslash
		sDir.Delete(0, nRootDirNameSize);
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

int CVideoDeviceDoc::CDeleteThread::Work()
{
	if (!m_pDoc)
		return 0;

	DWORD Event;
	CString sAutoSaveDir = m_pDoc->m_sRecordAutoSaveDir;
	sAutoSaveDir.TrimRight(_T('\\'));
	DWORD dwAttrib;
	CSortableFileFind FileFind;
	CTime CurrentTime, OldestDirTime;
	CTimeSpan TimeDiff;
	LONGLONG llDaysAgo, llStartDiskFreeSpaceDaysAgo, llStartCameraFolderSizeDaysAgo;
	ULONGLONG ullStartDiskFreeSpace, ullDiskFreeSpace, ullMinDiskFreeSpace;
	ULONGLONG ullStartCameraFolderSize, ullCameraFolderSize, ullMaxCameraFolderSize;
	std::random_device TrueRandom; // non-deterministic generator implemented as crypto-secure in Visual C++
	std::mt19937 PseudoRandom(TrueRandom());
	std::uniform_int_distribution<DWORD> Distribution(FILES_DELETE_INTERVAL_MIN, FILES_DELETE_INTERVAL_MAX); // distribute results: [FILES_DELETE_INTERVAL_MIN, FILES_DELETE_INTERVAL_MAX]

	for (;;)
	{
		// If using a constant deletion time interval in case of multiple devices running
		// the first started one would be cleared more than the last one. To fix that
		// we use a random generator for the deletion interval
		DWORD dwDeleteInMs = Distribution(PseudoRandom);
		Event = ::WaitForSingleObject(GetKillEvent(), dwDeleteInMs);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :
				return 0;

			// Delete
			case WAIT_TIMEOUT :
				dwAttrib = ::GetFileAttributes(sAutoSaveDir);
				if (dwAttrib != 0xFFFFFFFF && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
				{
					// Do recursive file find
					if (FileFind.InitRecursive(sAutoSaveDir + _T("\\*"), FALSE))
					{
						if (FileFind.WaitRecursiveDone(GetKillEvent()) == -1)
							return 0; // Exit Thread
					}

					// Get current time
					CurrentTime = CTime::GetCurrentTime();

					// Delete dirs which are older than the given days amount
					int nDeleteRecordingsOlderThanDays = m_pDoc->m_nDeleteRecordingsOlderThanDays;
					if (nDeleteRecordingsOlderThanDays > 0) // 0 means never delete any file
					{
						if (!DeleteOld(FileFind, nDeleteRecordingsOlderThanDays, CurrentTime))
							return 0; // Exit Thread
					}

					// Minimum wanted disk free space
					ullMinDiskFreeSpace = ::GetDiskTotalSize(sAutoSaveDir) / 1000000 * m_pDoc->m_nMinDiskFreePermillion;

					// Maximum allowed camera folder size
					ullMaxCameraFolderSize = m_pDoc->m_nMaxCameraFolderSizeMB;
					ullMaxCameraFolderSize <<= 20; // MB to Bytes
					if (ullMaxCameraFolderSize == 0) // 0 means no limit
						ullMaxCameraFolderSize = ULLONG_MAX;

					// Oldest existing directory
					if (!CalcOldestDir(FileFind, OldestDirTime, CurrentTime))
						return 0; // Exit Thread
					TimeDiff = CurrentTime - OldestDirTime;
					llDaysAgo = TimeDiff.GetDays();

					// Delete oldest dirs if space limit reached 
					llStartDiskFreeSpaceDaysAgo = 0;
					llStartCameraFolderSizeDaysAgo = 0;
					ullDiskFreeSpace = ::GetDiskAvailableFreeSpace(sAutoSaveDir);
					ullCameraFolderSize = ::GetDirContentSize(sAutoSaveDir, NULL, this).QuadPart;
					if (DoExit())
						return 0; // GetDirContentSize() may return before finishing calculating the size
					while (	llDaysAgo > 0 &&
							(ullDiskFreeSpace < ullMinDiskFreeSpace ||		// 'less than' is mandatory because both vars may be 0
							ullCameraFolderSize > ullMaxCameraFolderSize))	// 'greater than' is mandatory because ullMaxCameraFolderSize may be ULLONG_MAX 
					{
						// Store start vars
						if (llStartDiskFreeSpaceDaysAgo == 0 && ullDiskFreeSpace < ullMinDiskFreeSpace)
						{
							llStartDiskFreeSpaceDaysAgo = llDaysAgo;
							ullStartDiskFreeSpace = ullDiskFreeSpace;
						}
						if (llStartCameraFolderSizeDaysAgo == 0 && ullCameraFolderSize > ullMaxCameraFolderSize)
						{
							llStartCameraFolderSizeDaysAgo = llDaysAgo;
							ullStartCameraFolderSize = ullCameraFolderSize;
						}

						// Delete old
						if (!DeleteOld(FileFind, llDaysAgo, CurrentTime))
							return 0; // Exit Thread

						// Update vars
						llDaysAgo--;
						ullDiskFreeSpace = ::GetDiskAvailableFreeSpace(sAutoSaveDir);
						ullCameraFolderSize = ::GetDirContentSize(sAutoSaveDir, NULL, this).QuadPart;
						if (DoExit())
							return 0; // GetDirContentSize() may return before finishing calculating the size
					}

					// Log
					if (llStartDiskFreeSpaceDaysAgo != 0)
					{
						CString sStartDiskFreeSpaceMB, sDiskFreeSpaceMB, sMinDiskFreeSpaceMB, sDaysAgo;
						sStartDiskFreeSpaceMB.Format(_T("%I64u"), ullStartDiskFreeSpace >> 20);
						sDiskFreeSpaceMB.Format(_T("%I64u"), ullDiskFreeSpace >> 20);
						sMinDiskFreeSpaceMB.Format(_T("%I64u"), ullMinDiskFreeSpace >> 20);
						if (llStartDiskFreeSpaceDaysAgo == (llDaysAgo + 1))
							sDaysAgo.Format(_T("%I64d day%s ago"), llStartDiskFreeSpaceDaysAgo, llStartDiskFreeSpaceDaysAgo == 1 ? _T("") : _T("s"));
						else
							sDaysAgo.Format(_T("%I64d->%I64d days ago"), llStartDiskFreeSpaceDaysAgo, llDaysAgo + 1);
						::LogLine(	_T("%s, deleted %s: HD space %s->%s ") + ML_STRING(1825, "MB") +
									_T(" (set min %s ") + ML_STRING(1825, "MB") + _T(")"),
									m_pDoc->GetAssignedDeviceName(), sDaysAgo,
									::FormatIntegerNumber(sStartDiskFreeSpaceMB), ::FormatIntegerNumber(sDiskFreeSpaceMB),
									::FormatIntegerNumber(sMinDiskFreeSpaceMB));
					}
					if (llStartCameraFolderSizeDaysAgo != 0)
					{
						CString sStartCameraFolderSizeMB, sCameraFolderSizeMB, sMaxCameraFolderSizeMB, sDaysAgo;
						sStartCameraFolderSizeMB.Format(_T("%I64u"), ullStartCameraFolderSize >> 20);
						sCameraFolderSizeMB.Format(_T("%I64u"), ullCameraFolderSize >> 20);
						sMaxCameraFolderSizeMB.Format(_T("%I64u"), ullMaxCameraFolderSize >> 20);
						if (llStartCameraFolderSizeDaysAgo == (llDaysAgo + 1))
							sDaysAgo.Format(_T("%I64d day%s ago"), llStartCameraFolderSizeDaysAgo, llStartCameraFolderSizeDaysAgo == 1 ? _T("") : _T("s"));
						else
							sDaysAgo.Format(_T("%I64d->%I64d days ago"), llStartCameraFolderSizeDaysAgo, llDaysAgo + 1);
						::LogLine(	_T("%s, deleted %s: camera folder size %s->%s ") + ML_STRING(1825, "MB") +
									_T(" (set max %s ") + ML_STRING(1825, "MB") + _T(")"),
									m_pDoc->GetAssignedDeviceName(), sDaysAgo,
									::FormatIntegerNumber(sStartCameraFolderSizeMB), ::FormatIntegerNumber(sCameraFolderSizeMB),
									::FormatIntegerNumber(sMaxCameraFolderSizeMB));
					}
				}
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
	m_pDrawDibRGB32 = new CDib;
	m_pDrawDibRGB32->SetShowMessageBoxOnError(FALSE);
	m_pCamOffDib = new CDib;
	m_pCamOffDib->SetShowMessageBoxOnError(FALSE);

	// General Vars
	m_pView = NULL;
	m_pFrame = NULL;
	m_dwStopProcessFrame = 0U;
	m_dwProcessFrameStopped = 0U;
	m_bFlipH = FALSE;
	m_bFlipV = FALSE;
	memset(&m_CaptureBMI, 0, sizeof(BITMAPINFOFULL));
	memset(&m_ProcessFrameBMI, 0, sizeof(BITMAPINFOFULL));
	m_dwLastVideoWidth = 0;
	m_dwLastVideoHeight = 0;
	m_dFrameRate = DEFAULT_FRAMERATE;
	m_dEffectiveFrameRate = 0.0;
	m_dEffectiveFrameTimeSum = 0.0;
	m_dwEffectiveFrameTimeCountUp = 0U;
	m_bDoEditCopy = FALSE;
	m_bDoEditSnapshot = FALSE;
	m_lEffectiveDataRate = 0;
	m_lEffectiveDataRateSum = 0;
	m_bPlacementLoaded = FALSE;
	m_bCaptureStarted = FALSE;
	memset(m_szFrameAnnotation, 0, MAX_PATH * sizeof(TCHAR));
	m_bShowFrameTime = TRUE;
	m_bShowFrameUptime = FALSE;
	m_nRefFontSize = 9;
	m_bObscureSource = FALSE;
	m_dwFrameCountUp = 0U;
	m_bSizeToDoc = TRUE;
	m_bDeviceFirstRun = FALSE;
	m_1SecTime = CurrentTimeOnly;

	// Capture Devices
	m_pDxCapture = NULL;
	m_nDeviceInputId = -1;
	m_nDeviceFormatId = -1;
	m_bStopAndChangeDVFormat = FALSE;
	m_nDeviceFormatWidth = 0;
	m_nDeviceFormatHeight = 0;
	m_lCurrentInitUpTime = 0;
	m_bWatchDogVideoAlarm = FALSE;

	// Networking
	m_pVideoNetCom = NULL;
	m_pHttpVideoParseProcess = NULL;
	m_sGetFrameVideoHost = _T("");
	m_nGetFrameVideoPort = DEFAULT_HTTP_PORT;
	m_nNetworkDeviceTypeMode = GENERIC_SP;
	m_bPreferTcpforRtsp = FALSE;
	m_nHttpVideoQuality = HTTP_DEFAULT_VIDEO_QUALITY;
	m_nHttpVideoSizeX = HTTP_DEFAULT_VIDEO_SIZE_CX;
	m_nHttpVideoSizeY = HTTP_DEFAULT_VIDEO_SIZE_CY;
	m_nHttpGetFrameLocationPos = 0;
	m_HttpGetFrameLocations.Add(_T("/")); // first element must be valid ("/" => try home to see whether cam is reachable)

	// Snapshot
	m_nSnapshotRate = DEFAULT_SNAPSHOT_RATE;
	m_bSnapshotHistoryVideo = TRUE;
	m_nSnapshotHistoryRate = DEFAULT_SNAPSHOT_HISTORY_RATE;
	m_nSnapshotHistoryFrameRate = DEFAULT_SNAPSHOT_HISTORY_FRAMERATE;
	m_nSnapshotThumbWidth = DEFAULT_SNAPSHOT_THUMB_WIDTH;
	m_nSnapshotThumbHeight = DEFAULT_SNAPSHOT_THUMB_HEIGHT;
	m_dwNextSnapshotUpTime = 0U;

	// Threads Init
	m_CaptureAudioThread.SetDoc(this);
	m_HttpThread.SetDoc(this);
	m_RtspThread.SetDoc(this);
	m_WatchdogThread.SetDoc(this);
	m_DeleteThread.SetDoc(this);
	m_SaveFrameListThread.SetDoc(this);
	m_SaveSnapshotThread.SetDoc(this);
	m_SaveSnapshotVideoThread.SetDoc(this);

	// Recording
	m_sRecordAutoSaveDir = _T("");
	m_fVideoRecQuality = DEFAULT_VIDEO_QUALITY;
	m_nDeleteRecordingsOlderThanDays = DEFAULT_DEL_RECS_OLDER_THAN_DAYS;
	m_nMaxCameraFolderSizeMB = 0;
	m_nMinDiskFreePermillion = MIN_DISK_FREE_PERMILLION;
	m_nSaveFrameListSpeedPercent = 0;

	// Movement Detection
	m_pDifferencingDib = NULL;
	m_pMovementDetectorBackgndDib = NULL;
	m_nShowEditDetectionZones = 0;
	m_bDetectingMovement = FALSE;
	m_bDetectingMinLengthMovement = FALSE;
	m_nMilliSecondsRecBeforeMovementBegin = DEFAULT_PRE_BUFFER_MSEC;
	m_nMilliSecondsRecAfterMovementEnd = DEFAULT_POST_BUFFER_MSEC;
	m_nDetectionMinLengthMilliSeconds = MOVDET_MIN_LENGTH_MSEC;
	m_nDetectionMaxFrames = MOVDET_DEFAULT_MAX_FRAMES_IN_LIST;
	m_bSaveVideo = TRUE;
	m_bSaveAnimGIF = TRUE;
	m_bSendMailMalfunction = TRUE;
	m_bSendMailRecording = FALSE;
	m_bExecCommand = FALSE;
	m_nExecCommandMode = 0;
	m_sExecCommand = _T("");
	m_sExecParams = _T("");
	m_bHideExecCommand = FALSE;
	m_bWaitExecCommand = FALSE;
	m_hExecCommand = NULL;
	m_nDetectionLevel = DEFAULT_MOVDET_LEVEL;
	m_nCurrentDetectionZoneSize = m_nDetectionZoneSize = 0;
	m_dwAnimatedGifWidth = MOVDET_ANIMGIF_DEFAULT_WIDTH;
	m_dwAnimatedGifHeight = MOVDET_ANIMGIF_DEFAULT_HEIGHT;
	m_MovementDetectorCurrentIntensity = new int[MOVDET_MAX_ZONES];
	m_MovementDetectionsUpTime = new DWORD[MOVDET_MAX_ZONES];
	m_MovementDetections = new BYTE[MOVDET_MAX_ZONES];
	m_DoMovementDetection = new BYTE[MOVDET_MAX_ZONES];
	m_lMovDetXZonesCount = MOVDET_MIN_ZONES_XORY;
	m_lMovDetYZonesCount = MOVDET_MIN_ZONES_XORY;
	m_lMovDetTotalZones = 0;
	m_bObscureRemovedZones = FALSE;
	m_nMovDetFreqDiv = 1;
	m_dMovDetFrameRateFreqDivCalc = 0.0;

	// Audio
	m_dwCaptureAudioDeviceID = 0U;
	m_bCaptureAudio = FALSE;
	m_bCaptureAudioFromStream = FALSE;
	m_bAudioListen = FALSE;
	m_pSrcWaveFormat = (WAVEFORMATEX*)new BYTE[sizeof(WAVEFORMATEX)];
	WaveInitFormat(DEFAULT_AUDIO_CHANNELS, DEFAULT_AUDIO_SAMPLINGRATE, DEFAULT_AUDIO_BITS, m_pSrcWaveFormat);
	m_pDstWaveFormat = (WAVEFORMATEX*)new BYTE[sizeof(WAVEFORMATEX)];
	WaveInitFormat(DEFAULT_AUDIO_CHANNELS, DEFAULT_AUDIO_SAMPLINGRATE, DEFAULT_AUDIO_BITS, m_pDstWaveFormat);
	if (m_pDstWaveFormat)
	{
		m_pDstWaveFormat->wFormatTag = DEFAULT_AUDIO_FORMAT_TAG;
		m_pDstWaveFormat->nAvgBytesPerSec = DEFAULT_AUDIO_BITRATE / 8;
		m_pDstWaveFormat->nBlockAlign = 0;
		m_pDstWaveFormat->wBitsPerSample = 0;
	}

	// Camera Advanced Settings
	m_pCameraAdvancedSettingsDlg = NULL;

	// Email Settings
	m_MovDetLastMailTime = 0;
	m_MovDetLastJPGMailTime = 0;
	m_MovDetLastVideoMailTime = 0;
	m_MovDetLastGIFMailTime = 0;
	m_AttachmentType = ATTACHMENT_NONE;
	m_nMovDetSendMailSecBetweenMsg = 0;
	m_SendMailConfiguration.m_sSubject = DEFAULT_EMAIL_SUBJECT;
	m_SendMailConfiguration.m_sTo = _T("");
	m_SendMailConfiguration.m_nPort = 587;
	m_SendMailConfiguration.m_sFrom = _T("");
	m_SendMailConfiguration.m_sHost = _T("");
	m_SendMailConfiguration.m_sFromName = _T("");
	m_SendMailConfiguration.m_sUsername = _T("");
	m_SendMailConfiguration.m_sPassword = _T("");
	m_SendMailConfiguration.m_ConnectionType = STARTTLS;

	// Init Command Execution on Detection Critical Section
	::InitializeCriticalSection(&m_csExecCommand);

	// Init Movement Detections List Critical Section
	::InitializeCriticalSection(&m_csMovementDetectionsList);

	// Init Http Video Size and Compression Critical Section
	::InitializeCriticalSection(&m_csHttpParams);

	// Init Http Video Processing Image Data Critical Section
	::InitializeCriticalSection(&m_csHttpProcess);

	// Init Process Frame Stop Engine Critical Section
	::InitializeCriticalSection(&m_csProcessFrameStop);

	// Init Samples List Critical Section
	::InitializeCriticalSection(&m_csAudioList);

	// Init Connection Error Critical Section
	::InitializeCriticalSection(&m_csConnectionError);

	// Init Movement Detector
	OneEmptyFrameList();
	FreeMovementDetector();

	// Start Save Frame List Thread
	m_SaveFrameListThread.Start();

	// Start Watchdog Thread
	m_WatchdogThread.Start();
}

CVideoDeviceDoc::~CVideoDeviceDoc()
{
	// Parsers always deleted after the related CNetCom objects!
	if (m_pHttpVideoParseProcess)
	{
		delete m_pHttpVideoParseProcess;
		m_pHttpVideoParseProcess = NULL;
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
	::DeleteCriticalSection(&m_csConnectionError);
	::DeleteCriticalSection(&m_csAudioList);
	::DeleteCriticalSection(&m_csProcessFrameStop);
	::DeleteCriticalSection(&m_csHttpProcess);
	::DeleteCriticalSection(&m_csHttpParams);
	::DeleteCriticalSection(&m_csMovementDetectionsList);
	::DeleteCriticalSection(&m_csExecCommand);
	if (m_hExecCommand)
	{
		::CloseHandle(m_hExecCommand);
		m_hExecCommand = NULL;
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
	if (m_pDrawDibRGB32)
	{
		delete m_pDrawDibRGB32;
		m_pDrawDibRGB32 = NULL;
	}
	if (m_pCamOffDib)
	{
		delete m_pCamOffDib;
		m_pCamOffDib = NULL;
	}
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
	while (!m_AudioList.IsEmpty())
	{
		CUserBuf UserBuf = m_AudioList.RemoveHead();
		if (UserBuf.m_pBuf)
			av_free(UserBuf.m_pBuf);
	}
}

void CVideoDeviceDoc::ClearConnectErr()
{
	::EnterCriticalSection(&m_csConnectionError);
	m_sLastConnectionError.Empty();
	::LeaveCriticalSection(&m_csConnectionError);
}

void CVideoDeviceDoc::ConnectErr(LPCTSTR lpszText, const CString& sDeviceName)
{
	::EnterCriticalSection(&m_csConnectionError);
	m_sLastConnectionError = lpszText;
	::LeaveCriticalSection(&m_csConnectionError);
	::LogLine(_T("%s, %s"), sDeviceName, lpszText);
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

BOOL CVideoDeviceDoc::ParseNetworkDevicePathName(	const CString& sDevicePathName,
													CString& sOutGetFrameVideoHost,
													volatile int& nOutGetFrameVideoPort,
													CString& sOutGetFrameLocation,
													volatile NetworkDeviceTypeMode& nOutNetworkDeviceTypeMode)
{
	// Network devices have the format
	// Host:Port:FrameLocation:NetworkDeviceTypeMode
	// (use reverse find because Host maybe a IP6 address with :)
	CString sAddress(sDevicePathName);
	int i = sAddress.ReverseFind(_T(':'));
	if (i >= 0)
	{
		// NetworkDeviceTypeMode
		CString sNetworkDeviceTypeMode = sAddress.Right(sAddress.GetLength() - i - 1);
		NetworkDeviceTypeMode nNetworkDeviceTypeMode = (NetworkDeviceTypeMode)_tcstol(sNetworkDeviceTypeMode.GetBuffer(0), NULL, 10);
		sNetworkDeviceTypeMode.ReleaseBuffer();
		if (nNetworkDeviceTypeMode >= GENERIC_SP && nNetworkDeviceTypeMode < LAST_DEVICE)
			nOutNetworkDeviceTypeMode = nNetworkDeviceTypeMode;
		else
			return FALSE;

		// FrameLocation
		sAddress = sAddress.Left(i);
		i = sAddress.ReverseFind(_T(':'));
		if (i >= 0)
		{
			sOutGetFrameLocation = sAddress.Right(sAddress.GetLength() - i - 1);

			// Port
			sAddress = sAddress.Left(i);
			i = sAddress.ReverseFind(_T(':'));
			if (i >= 0)
			{
				CString sPort = sAddress.Right(sAddress.GetLength() - i - 1);
				int nPort = _tcstol(sPort.GetBuffer(0), NULL, 10);
				sPort.ReleaseBuffer();
				if (nPort > 0 && nPort <= 65535) // Port 0 is Reserved
				{
					sOutGetFrameVideoHost = sAddress.Left(i);
					nOutGetFrameVideoPort = nPort;
					return TRUE;
				}
				else
					return FALSE;
			}
			else
				return FALSE;
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}

CString CVideoDeviceDoc::MakeNetworkDevicePathName(	const CString& sGetFrameVideoHost,
													int nGetFrameVideoPort,
													const CString& sGetFrameLocation,
													NetworkDeviceTypeMode nNetworkDeviceTypeMode)
{
	CString sDevicePathName;
	sDevicePathName.Format(_T("%s:%d:%s:%d"), sGetFrameVideoHost, nGetFrameVideoPort, sGetFrameLocation, nNetworkDeviceTypeMode);

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
	else
		sDevicePathName = MakeNetworkDevicePathName(m_sGetFrameVideoHost, m_nGetFrameVideoPort, m_HttpGetFrameLocations[0], m_nNetworkDeviceTypeMode);

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
	else
	{
		sDevice.Format(_T("%s:%d"), m_sGetFrameVideoHost, m_nGetFrameVideoPort);
		sDevice.Replace(_T("%"), _T(":interface")); // for IP6 link-local addresses
	}

	// Registry key cannot begin with a backslash and should
	// not contain backslashes otherwise subkeys are created!
	sDevice.Replace(_T('\\'), _T('/'));

	return sDevice;
}

CString CVideoDeviceDoc::GetDeviceFormat()
{
	CString sFormat;
	if (m_pDxCapture)
		sFormat = CDib::GetCompressionName((LPBITMAPINFO)&m_CaptureBMI).MakeLower();
	else if (m_pVideoNetCom)
	{
		if (m_pHttpVideoParseProcess)
		{
			if (m_pHttpVideoParseProcess->m_FormatType == CHttpParseProcess::FORMATVIDEO_MJPEG)
				sFormat = ML_STRING(1865, "HTTP motion jpeg");
			else if (m_pHttpVideoParseProcess->m_FormatType == CHttpParseProcess::FORMATVIDEO_JPEG)
				sFormat = ML_STRING(1866, "HTTP jpeg snapshots");
		}
	}
	else
	{
		if (m_RtspThread.m_nVideoCodecID > AV_CODEC_ID_NONE)
			sFormat += CString(avcodec_get_name((enum AVCodecID)m_RtspThread.m_nVideoCodecID));
		if (m_RtspThread.m_nAudioCodecID > AV_CODEC_ID_NONE)
			sFormat += _T("/") + CString(avcodec_get_name((enum AVCodecID)m_RtspThread.m_nAudioCodecID));
		CString sProto(_T("RTSP"));
		if (m_RtspThread.m_nUnderlyingTransport >= 0)
		{
			switch (m_RtspThread.m_nUnderlyingTransport)
			{
			case 0: sProto += _T(" (UDP)"); break;
			case 1: sProto += _T(" (TCP)"); break;
			case 2: sProto += _T(" (UDP Multicast)"); break;
			default:sProto += _T(" (Unknown Transport)"); break;
			}
		}
		if (!sFormat.IsEmpty())
			sFormat = sProto + _T(" ") + sFormat;
		else
			sFormat = sProto;
	}
	return sFormat;
}

void CVideoDeviceDoc::SetDocumentTitle()
{
	CString sTitle;
	if (m_bClosing)
	{
		// Closing progress
		sTitle = ML_STRING(1566, "Closing") + _T(" ");
		CString sCurrentTitle(GetTitle());
		if (sCurrentTitle.Find(_T(".....")) >= 0)
			sTitle += _T(".");
		else if (sCurrentTitle.Find(_T("....")) >= 0)
			sTitle += _T(".....");
		else if (sCurrentTitle.Find(_T("...")) >= 0)
			sTitle += _T("....");
		else if (sCurrentTitle.Find(_T("..")) >= 0)
			sTitle += _T("...");
		else if (sCurrentTitle.Find(_T(".")) >= 0)
			sTitle += _T("..");
		else
			sTitle += _T(".");
	}
	else
	{
		// Get name
		sTitle = GetAssignedDeviceName();

		// General info
		CString sWidthHeight;
		CString sSaveFrameListSpeed;
		CString sFrameRate;
		CString sDataRate;
		CString sFormat;
		if (m_DocRect.Width() > 0 && m_DocRect.Height() > 0)
		{
			// Width and Height
			sWidthHeight.Format(_T("%dx%d"), m_DocRect.Width(), m_DocRect.Height());

			// Framerate
			if (m_dEffectiveFrameRate > 0.0)
				sFrameRate.Format(_T("%0.1ffps"), m_dEffectiveFrameRate);

			// Saving speed
			double dSaveFrameListSpeed = (double)m_nSaveFrameListSpeedPercent / 100.0;
			if (dSaveFrameListSpeed > 0.0)
				sSaveFrameListSpeed.Format(_T("%0.1fx"), dSaveFrameListSpeed);

			// Datarate
			if (m_lEffectiveDataRate > 0)
			{
				int kbps = m_lEffectiveDataRate / 125; // 1000 bits are 1 kbit, while 1024 bytes are 1 KB (or KiB)
				if (kbps >= 1000)
					sDataRate.Format(_T("%0.1fMbps"), (double)kbps / 1000.0);
				else
					sDataRate.Format(_T("%dkbps"), kbps);
			}

			// Format
			sFormat = GetDeviceFormat();
		}

		// Update Camera Advanced Settings Title and Recording Dir
		if (m_pCameraAdvancedSettingsDlg)
			m_pCameraAdvancedSettingsDlg->UpdateTitleAndDir();

		// Set main title
		if (!sWidthHeight.IsEmpty())
			sTitle += _T(" , ") + sWidthHeight;
		if (!sFrameRate.IsEmpty())
			sTitle += _T(" , ") + sFrameRate;
		if (!sSaveFrameListSpeed.IsEmpty())
			sTitle += _T(" , ") + sSaveFrameListSpeed;
		if (!sDataRate.IsEmpty())
			sTitle += _T(" , ") + sDataRate;
		if (!sFormat.IsEmpty())
			sTitle += _T(" , ") + sFormat;
	}
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

BOOL CVideoDeviceDoc::LoadZonesBlockSettings(int nBlock, CString sSection)
{
	BOOL bOK = FALSE;
	if (nBlock >= 0 && nBlock < (MOVDET_MAX_ZONES / MOVDET_MAX_ZONES_BLOCK_SIZE))
	{
		UINT uiSize = 0U;
		LPBYTE pBlock = NULL;
		CString sBlock;
		sBlock.Format(MOVDET_ZONES_BLOCK_FORMAT, nBlock);
		::AfxGetApp()->GetProfileBinary(sSection, sBlock, &pBlock, &uiSize);
		if (pBlock && uiSize > 0U)
		{
			bOK = TRUE;
			memcpy(	m_DoMovementDetection + nBlock * MOVDET_MAX_ZONES_BLOCK_SIZE,
					pBlock,
					MIN(MOVDET_MAX_ZONES_BLOCK_SIZE, uiSize));
		}
		if (pBlock)
			delete[] pBlock;
	}
	return bOK;
}

BOOL CVideoDeviceDoc::LoadZonesSettings(CString sSection)
{
	for (int i = 0 ; i < MOVDET_MAX_ZONES / MOVDET_MAX_ZONES_BLOCK_SIZE ; i++)
	{
		if (!LoadZonesBlockSettings(i, sSection))
			return FALSE;
	}
	return TRUE;
}

void CVideoDeviceDoc::SaveZonesBlockSettings(int nBlock, CString sSection)
{
	if (nBlock >= 0 && nBlock < (MOVDET_MAX_ZONES / MOVDET_MAX_ZONES_BLOCK_SIZE))
	{
		CString sBlock;
		sBlock.Format(MOVDET_ZONES_BLOCK_FORMAT, nBlock);
		::AfxGetApp()->WriteProfileBinary(	sSection,
											sBlock,
											m_DoMovementDetection + nBlock * MOVDET_MAX_ZONES_BLOCK_SIZE,
											MOVDET_MAX_ZONES_BLOCK_SIZE);
	}
}

void CVideoDeviceDoc::SaveZonesSettings(CString sSection)
{
	for (int i = 0 ; i < MOVDET_MAX_ZONES / MOVDET_MAX_ZONES_BLOCK_SIZE ; i++)
		SaveZonesBlockSettings(i, sSection);
}

// Valid values: 4,5,6,7,8,9,10,11,12,14,16
int CVideoDeviceDoc::ValidateRefFontSize(int nRefFontSize)
{
	if (nRefFontSize < 4)
		nRefFontSize = 4;
	else if (nRefFontSize > 16)
		nRefFontSize = 16;
	else if (nRefFontSize == 15)
		nRefFontSize = 14;
	else if (nRefFontSize == 13)
		nRefFontSize = 12;
	return nRefFontSize;
}

// Returns -1 on error, otherwise the read detection level
int CVideoDeviceDoc::ReadDetectionLevelFromFile(CString sRecordAutoSaveDir)
{
	int nDetectionLevel = -1;
	sRecordAutoSaveDir.TrimRight(_T('\\'));
	CString sCameraRecSensitivityFile(sRecordAutoSaveDir + _T("\\") + CAMERA_REC_SENSITIVITY_FILENAME);
	if (::IsExistingFile(sCameraRecSensitivityFile))
	{
		for (int nRetry = 0; nRetry < 10; nRetry++)
		{
			try
			{
				CStdioFile f(sCameraRecSensitivityFile, CFile::modeRead | CFile::typeText | CFile::shareDenyWrite);
				CString sDetectionLevel;
				f.ReadString(sDetectionLevel);
				nDetectionLevel = _tcstol(sDetectionLevel.GetBuffer(0), NULL, 10);
				sDetectionLevel.ReleaseBuffer();
				break;
			}
			catch (CFileException* e)
			{
				e->Delete();
				::Sleep(100);
			}
		}
	}
	return nDetectionLevel;
}

BOOL CVideoDeviceDoc::WriteDetectionLevelToFile(int nDetectionLevel, CString sRecordAutoSaveDir)
{
	sRecordAutoSaveDir.TrimRight(_T('\\'));
	CString sCameraRecSensitivityFile(sRecordAutoSaveDir + _T("\\") + CAMERA_REC_SENSITIVITY_FILENAME);
	for (int nRetry = 0; nRetry < 10; nRetry++)
	{
		try
		{
			CStdioFile f(sCameraRecSensitivityFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareExclusive);
			CString sDetectionLevel;
			sDetectionLevel.Format(_T("%d\n"), nDetectionLevel);
			f.WriteString(sDetectionLevel);
			return TRUE;
		}
		catch (CFileException* e)
		{
			e->Delete();
			::Sleep(100);
		}
	}
	return FALSE;
}

// Valid values: 0,10,20,30,40,50,60,70,80,90,100
int CVideoDeviceDoc::ValidateDetectionLevel(int nDetectionLevel)
{
	if (nDetectionLevel < 0)
		nDetectionLevel = 0;
	else if (nDetectionLevel > 100)
		nDetectionLevel = 100;
	return (nDetectionLevel / 10) * 10;
}

int CVideoDeviceDoc::ValidateSnapshotRate(int nSnapshotRate)
{
	if (nSnapshotRate >= 3600)
		return 3600;				// 1 hour
	else if (nSnapshotRate >= 1800)
		return 1800;				// 30 minutes
	else if (nSnapshotRate >= 900)
		return 900;					// 15 minutes
	else if (nSnapshotRate >= 600)
		return 600;					// 10 minutes
	else if (nSnapshotRate >= 300)
		return 300;					// 5 minutes
	else if (nSnapshotRate >= 240)
		return 240;					// 4 minutes
	else if (nSnapshotRate >= 180)
		return 180;					// 3 minutes
	else if (nSnapshotRate >= 120)
		return 120;					// 2 minutes
	else if (nSnapshotRate >= 60)
		return 60;					// 1 minute
	else if (nSnapshotRate >= 30)
		return 30;					// 30 seconds
	else if (nSnapshotRate >= 15)
		return 15;					// 15 seconds
	else if (nSnapshotRate >= 10)
		return 10;					// 10 seconds
	else if (nSnapshotRate >= 5)
		return 5;					// 5 seconds
	else if (nSnapshotRate >= 4)
		return 4;					// 4 seconds
	else if (nSnapshotRate >= 3)
		return 3;					// 3 seconds
	else if (nSnapshotRate >= 2)
		return 2;					// 2 seconds
	else if (nSnapshotRate >= 1)
		return 1;					// 1 second
	else
		return 0;					// Fast
}

int CVideoDeviceDoc::MakeSizeMultipleOf4(int nSize)
{
	if (nSize <= 4)
		return 4;
	else
		return nSize & ~0x3;
}

void CVideoDeviceDoc::LoadSettings(	double dDefaultFrameRate,
									BOOL bDefaultCaptureAudioFromStream,
									CString sSection,
									CString sDeviceName)
{
	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();

	// Current Time
	CTime CurrentTime = CTime::GetCurrentTime();

	// Default auto-save directory
	sDeviceName = GetValidName(sDeviceName);
	CString sDefaultAutoSaveDir(pApp->m_sMicroApacheDocRoot);
	sDefaultAutoSaveDir.TrimRight(_T('\\'));
	sDefaultAutoSaveDir += _T("\\") + sDeviceName;

	// Set Placement
	if (!pApp->m_bServiceProcess)
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

	// Email Settings
	m_AttachmentType = (AttachmentType) pApp->GetProfileInt(sSection, _T("AttachmentType"), ATTACHMENT_NONE);
	if (m_AttachmentType < ATTACHMENT_NONE)
		m_AttachmentType = ATTACHMENT_NONE;
	else if (m_AttachmentType > ATTACHMENT_JPG_GIF)
		m_AttachmentType = ATTACHMENT_JPG_GIF;
	m_nMovDetSendMailSecBetweenMsg = (int) pApp->GetProfileInt(sSection, _T("SendMailSecBetweenMsg"), 0);
	m_SendMailConfiguration.m_sSubject = pApp->GetProfileString(sSection, _T("SendMailSubject"), DEFAULT_EMAIL_SUBJECT);
	if (m_SendMailConfiguration.m_sSubject.IsEmpty())
		m_SendMailConfiguration.m_sSubject = DEFAULT_EMAIL_SUBJECT;
	m_SendMailConfiguration.m_sTo = pApp->GetProfileString(sSection, _T("SendMailTo"), _T(""));
	m_SendMailConfiguration.m_nPort = (int) pApp->GetProfileInt(sSection, _T("SendMailPort"), 587);
	m_SendMailConfiguration.m_sFrom = pApp->GetProfileString(sSection, _T("SendMailFrom"), _T(""));
	m_SendMailConfiguration.m_sHost = pApp->GetProfileString(sSection, _T("SendMailHost"), _T(""));
	m_SendMailConfiguration.m_sFromName = pApp->GetProfileString(sSection, _T("SendMailFromName"), _T(""));
	m_SendMailConfiguration.m_sUsername = pApp->GetSecureProfileString(sSection, _T("SendMailUsernameExportable"));
	m_SendMailConfiguration.m_sPassword = pApp->GetSecureProfileString(sSection, _T("SendMailPasswordExportable"));
	m_SendMailConfiguration.m_ConnectionType = (ConnectionType) pApp->GetProfileInt(sSection, _T("SendMailConnectionType"), STARTTLS);

	// Networking
	m_nHttpVideoQuality = (int) pApp->GetProfileInt(sSection, _T("HTTPVideoQuality"), HTTP_DEFAULT_VIDEO_QUALITY);
	m_nHttpVideoSizeX = (int) pApp->GetProfileInt(sSection, _T("HTTPVideoSizeX"), HTTP_DEFAULT_VIDEO_SIZE_CX);
	m_nHttpVideoSizeY = (int) pApp->GetProfileInt(sSection, _T("HTTPVideoSizeY"), HTTP_DEFAULT_VIDEO_SIZE_CY);
	m_sHttpGetFrameUsername = pApp->GetSecureProfileString(sSection, _T("HTTPGetFrameUsernameExportable"));
	m_sHttpGetFramePassword = pApp->GetSecureProfileString(sSection, _T("HTTPGetFramePasswordExportable"));
	m_bPreferTcpforRtsp = (BOOL) pApp->GetProfileInt(sSection, _T("PreferTcpforRtsp"), FALSE);

	// All other
	m_bFlipH = (BOOL)pApp->GetProfileInt(sSection, _T("FlipH"), FALSE);
	m_bFlipV = (BOOL)pApp->GetProfileInt(sSection, _T("FlipV"), FALSE);
	m_sRecordAutoSaveDir = pApp->GetProfileString(sSection, _T("RecordAutoSaveDir"), _T(""));
	if (m_sRecordAutoSaveDir.IsEmpty())
	{
		// First time we run this device
		m_bDeviceFirstRun = TRUE;

		// Set to default auto save folder
		m_sRecordAutoSaveDir = sDefaultAutoSaveDir;

		// It's important to write the auto save folder path into the registry right now
		// because we want the mapping between the folder (created below) and the registry
		pApp->WriteProfileString(sSection, _T("RecordAutoSaveDir"), m_sRecordAutoSaveDir);
	}
	CString sRecordAutoSaveDir = m_sRecordAutoSaveDir;
	sRecordAutoSaveDir.TrimRight(_T('\\'));
	m_bObscureSource = ::IsExistingFile(sRecordAutoSaveDir + _T("\\") + CAMERA_IS_OBSCURED_FILENAME);
	m_nSnapshotRate = ValidateSnapshotRate(pApp->GetProfileInt(sSection, _T("SnapshotRate"), DEFAULT_SNAPSHOT_RATE));
	m_bSnapshotHistoryVideo = (BOOL)pApp->GetProfileInt(sSection, _T("SnapshotHistoryVideo"), TRUE);
	m_nSnapshotHistoryRate = (int)pApp->GetProfileInt(sSection, _T("SnapshotHistoryRate"), DEFAULT_SNAPSHOT_HISTORY_RATE);
	m_nSnapshotHistoryFrameRate = (int)pApp->GetProfileInt(sSection, _T("SnapshotHistoryFrameRate"), DEFAULT_SNAPSHOT_HISTORY_FRAMERATE);
	m_nSnapshotThumbWidth = (int) MakeSizeMultipleOf4(pApp->GetProfileInt(sSection, _T("SnapshotThumbWidth"), DEFAULT_SNAPSHOT_THUMB_WIDTH));
	m_nSnapshotThumbHeight = (int) MakeSizeMultipleOf4(pApp->GetProfileInt(sSection, _T("SnapshotThumbHeight"), DEFAULT_SNAPSHOT_THUMB_HEIGHT));
	m_bCaptureAudio = (BOOL) pApp->GetProfileInt(sSection, _T("CaptureAudio"), FALSE);
	m_bCaptureAudioFromStream = (BOOL)pApp->GetProfileInt(sSection, _T("CaptureAudioFromStream"), bDefaultCaptureAudioFromStream);
	m_bAudioListen = (BOOL) pApp->GetProfileInt(sSection, _T("AudioListen"), FALSE);
	m_dwCaptureAudioDeviceID = (DWORD) pApp->GetProfileInt(sSection, _T("AudioCaptureDeviceID"), 0);
	m_sCaptureAudioDeviceName = pApp->GetProfileString(sSection, _T("AudioCaptureDeviceName"), CaptureAudioDeviceIDToName(m_dwCaptureAudioDeviceID));
	m_nDeviceInputId = (int) pApp->GetProfileInt(sSection, _T("VideoCaptureDeviceInputID"), -1);
	m_nDeviceFormatId = (int) pApp->GetProfileInt(sSection, _T("VideoCaptureDeviceFormatID"), -1);
	m_nDeviceFormatWidth = (int) pApp->GetProfileInt(sSection, _T("VideoCaptureDeviceFormatWidth"), 0);
	m_nDeviceFormatHeight = (int) pApp->GetProfileInt(sSection, _T("VideoCaptureDeviceFormatHeight"), 0);
	m_nMilliSecondsRecBeforeMovementBegin = (int) pApp->GetProfileInt(sSection, _T("MilliSecondsRecBeforeMovementBegin"), DEFAULT_PRE_BUFFER_MSEC);
	m_nMilliSecondsRecAfterMovementEnd = (int) pApp->GetProfileInt(sSection, _T("MilliSecondsRecAfterMovementEnd"), DEFAULT_POST_BUFFER_MSEC);
	m_nDetectionMinLengthMilliSeconds = (int) pApp->GetProfileInt(sSection, _T("DetectionMinLengthMilliSeconds"), MOVDET_MIN_LENGTH_MSEC);
	int nDetectionMaxFrames = (int) pApp->GetProfileInt(sSection, _T("DetectionMaxFrames"), MOVDET_DEFAULT_MAX_FRAMES_IN_LIST);
	if (nDetectionMaxFrames >= 1 && nDetectionMaxFrames <= MOVDET_MAX_MAX_FRAMES_IN_LIST)
		m_nDetectionMaxFrames = nDetectionMaxFrames;
	else
		m_nDetectionMaxFrames = MOVDET_DEFAULT_MAX_FRAMES_IN_LIST; // restore the default if a strange value is set
	int nDetectionLevel;
	if ((nDetectionLevel = ReadDetectionLevelFromFile(m_sRecordAutoSaveDir)) >= 0)
		m_nDetectionLevel = ValidateDetectionLevel(nDetectionLevel);
	else
	{
		// TODO: in future remove the following and just set nDetectionLevel = DEFAULT_MOVDET_LEVEL;
		
		// Import old registry value and write it to the file
		nDetectionLevel = ValidateDetectionLevel(pApp->GetProfileInt(sSection, _T("DetectionLevel"), DEFAULT_MOVDET_LEVEL));
		if (WriteDetectionLevelToFile(nDetectionLevel, m_sRecordAutoSaveDir))
			m_nDetectionLevel = nDetectionLevel; // always after the write as in OnTimer() it gets polled!
	}
	m_nCurrentDetectionZoneSize = m_nDetectionZoneSize = (int) pApp->GetProfileInt(sSection, _T("DetectionZoneSize"), 0);
	m_bSaveVideo = (BOOL) pApp->GetProfileInt(sSection, _T("SaveVideoMovementDetection"), TRUE);
	m_bSaveAnimGIF = (BOOL) pApp->GetProfileInt(sSection, _T("SaveAnimGIFMovementDetection"), TRUE);
	m_bSendMailMalfunction = (BOOL)pApp->GetProfileInt(sSection, _T("SendMailMalfunction"), TRUE);
	m_bSendMailRecording = (BOOL) pApp->GetProfileInt(sSection, _T("SendMailMovementDetection"), FALSE);
	m_bExecCommand = (BOOL) pApp->GetProfileInt(sSection, _T("DoExecCommandMovementDetection"), FALSE);
	m_nExecCommandMode = pApp->GetProfileInt(sSection, _T("ExecModeMovementDetection"), 0);
	
	// Attention: GetPrivateProfileString() used by GetProfileString() for INI files strips quotes!
	m_sExecCommand = pApp->GetProfileString(sSection, _T("ExecCommandMovementDetection"), _T(""));
	m_sExecCommand.Replace(_T("%singlequote%"), _T("\'"));
	m_sExecCommand.Replace(_T("%doublequote%"), _T("\""));
	m_sExecParams = pApp->GetProfileString(sSection, _T("ExecParamsMovementDetection"), _T(""));
	m_sExecParams.Replace(_T("%singlequote%"), _T("\'"));
	m_sExecParams.Replace(_T("%doublequote%"), _T("\""));

	m_bHideExecCommand = (BOOL) pApp->GetProfileInt(sSection, _T("HideExecCommandMovementDetection"), FALSE);
	m_bWaitExecCommand = (BOOL) pApp->GetProfileInt(sSection, _T("WaitExecCommandMovementDetection"), FALSE);
	m_fVideoRecQuality = (float) CAVRec::ClipVideoQuality((float)pApp->GetProfileInt(sSection, _T("VideoRecQuality"), (int)DEFAULT_VIDEO_QUALITY));
	m_bObscureRemovedZones = (BOOL) pApp->GetProfileInt(sSection, _T("ObscureRemovedZones"), FALSE);
	m_szFrameAnnotation[MAX_PATH - 1] = _T('\0');																	// first make sure it is NULL terminated
	_tcsncpy(m_szFrameAnnotation, pApp->GetProfileString(sSection, _T("FrameAnnotation"), _T("")), MAX_PATH - 1);	// and then copy a maximum of (MAX_PATH - 1) chars
	m_bShowFrameTime = (BOOL) pApp->GetProfileInt(sSection, _T("ShowFrameTime"), TRUE);
	m_bShowFrameUptime = (BOOL)pApp->GetProfileInt(sSection, _T("ShowFrameUptime"), FALSE);
	m_nRefFontSize = ValidateRefFontSize(pApp->GetProfileInt(sSection, _T("RefFontSize"), 9));
	m_dwAnimatedGifWidth = (DWORD) MakeSizeMultipleOf4(pApp->GetProfileInt(sSection, _T("AnimatedGifWidth"), MOVDET_ANIMGIF_DEFAULT_WIDTH));
	m_dwAnimatedGifHeight = (DWORD) MakeSizeMultipleOf4(pApp->GetProfileInt(sSection, _T("AnimatedGifHeight"), MOVDET_ANIMGIF_DEFAULT_HEIGHT));
	m_nDeleteRecordingsOlderThanDays = (int) pApp->GetProfileInt(sSection, _T("DeleteRecordingsOlderThanDays"), DEFAULT_DEL_RECS_OLDER_THAN_DAYS);
	m_nMaxCameraFolderSizeMB = (int) pApp->GetProfileInt(sSection, _T("MaxCameraFolderSizeMB"), 0);
	m_nMinDiskFreePermillion = (int) pApp->GetProfileInt(sSection, _T("MinDiskFreePermillion"), MIN_DISK_FREE_PERMILLION);

	// Frame-rate
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
		{
			// Make sure that the saved placement is visible
			if (::AfxGetMainFrame())
			{
				CRect rc(0, 0, 0, 0);
				::AfxGetMainFrame()->GetMDIClientRect(&rc);
				if (rc.Width() > 0 && rc.Height() > 0)
				{
					if (wndpl.rcNormalPosition.right < PLACEMENT_THRESHOLD_PIXELS				||
						wndpl.rcNormalPosition.bottom < PLACEMENT_THRESHOLD_PIXELS				||
						wndpl.rcNormalPosition.left > rc.Width() - PLACEMENT_THRESHOLD_PIXELS	||
						wndpl.rcNormalPosition.top > rc.Height() - PLACEMENT_THRESHOLD_PIXELS)
						::AfxGetMainFrame()->ClipToMDIRect(&wndpl.rcNormalPosition);
				}
			}

			// Store placement
			::AfxGetApp()->WriteProfileBinary(GetDevicePathName(), _T("WindowPlacement"), (BYTE*)&wndpl, sizeof(wndpl));
		}
	}
}

void CVideoDeviceDoc::SaveSettings()
{
	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();
	CString sSection(GetDevicePathName());

	// Store the device name
	pApp->WriteProfileString(sSection, _T("DeviceName"), GetDeviceName());

	// Store Placement
	SavePlacement();

	// Email Settings
	pApp->WriteProfileInt(sSection, _T("AttachmentType"), (int)m_AttachmentType);
	pApp->WriteProfileInt(sSection, _T("SendMailSecBetweenMsg"), m_nMovDetSendMailSecBetweenMsg);
	pApp->WriteProfileString(sSection, _T("SendMailSubject"), m_SendMailConfiguration.m_sSubject);
	pApp->WriteProfileString(sSection, _T("SendMailTo"), m_SendMailConfiguration.m_sTo);
	pApp->WriteProfileInt(sSection, _T("SendMailPort"), m_SendMailConfiguration.m_nPort);
	pApp->WriteProfileString(sSection, _T("SendMailFrom"), m_SendMailConfiguration.m_sFrom);
	pApp->WriteProfileString(sSection, _T("SendMailHost"), m_SendMailConfiguration.m_sHost);
	pApp->WriteProfileString(sSection, _T("SendMailFromName"), m_SendMailConfiguration.m_sFromName);
	pApp->WriteSecureProfileString(sSection, _T("SendMailUsernameExportable"), m_SendMailConfiguration.m_sUsername);
	pApp->WriteSecureProfileString(sSection, _T("SendMailPasswordExportable"), m_SendMailConfiguration.m_sPassword);
	pApp->WriteProfileInt(sSection, _T("SendMailConnectionType"), (int)m_SendMailConfiguration.m_ConnectionType);

	// Networking
	pApp->WriteProfileInt(sSection, _T("HTTPVideoQuality"), m_nHttpVideoQuality);
	pApp->WriteProfileInt(sSection, _T("HTTPVideoSizeX"), m_nHttpVideoSizeX);
	pApp->WriteProfileInt(sSection, _T("HTTPVideoSizeY"), m_nHttpVideoSizeY);
	pApp->WriteSecureProfileString(sSection, _T("HTTPGetFrameUsernameExportable"), m_sHttpGetFrameUsername);
	pApp->WriteSecureProfileString(sSection, _T("HTTPGetFramePasswordExportable"), m_sHttpGetFramePassword);
	pApp->WriteProfileInt(sSection, _T("PreferTcpforRtsp"), m_bPreferTcpforRtsp);

	// All other
	pApp->WriteProfileInt(sSection, _T("FlipH"), (int)m_bFlipH);
	pApp->WriteProfileInt(sSection, _T("FlipV"), (int)m_bFlipV);
	pApp->WriteProfileString(sSection, _T("RecordAutoSaveDir"), m_sRecordAutoSaveDir);
	pApp->WriteProfileInt(sSection, _T("SnapshotRate"), m_nSnapshotRate);
	pApp->WriteProfileInt(sSection, _T("SnapshotHistoryVideo"), (int)m_bSnapshotHistoryVideo);
	pApp->WriteProfileInt(sSection, _T("SnapshotHistoryRate"), m_nSnapshotHistoryRate);
	pApp->WriteProfileInt(sSection, _T("SnapshotHistoryFrameRate"), m_nSnapshotHistoryFrameRate);
	pApp->WriteProfileInt(sSection, _T("SnapshotThumbWidth"), m_nSnapshotThumbWidth);
	pApp->WriteProfileInt(sSection, _T("SnapshotThumbHeight"), m_nSnapshotThumbHeight);
	pApp->WriteProfileInt(sSection, _T("CaptureAudio"), m_bCaptureAudio);
	pApp->WriteProfileInt(sSection, _T("CaptureAudioFromStream"), m_bCaptureAudioFromStream);
	pApp->WriteProfileInt(sSection, _T("AudioListen"), (int)m_bAudioListen);
	pApp->WriteProfileInt(sSection, _T("AudioCaptureDeviceID"), m_dwCaptureAudioDeviceID);
	pApp->WriteProfileString(sSection, _T("AudioCaptureDeviceName"), m_sCaptureAudioDeviceName);
	pApp->WriteProfileInt(sSection, _T("VideoCaptureDeviceInputID"), m_nDeviceInputId);
	pApp->WriteProfileInt(sSection, _T("VideoCaptureDeviceFormatID"), m_nDeviceFormatId);
	pApp->WriteProfileInt(sSection, _T("VideoCaptureDeviceFormatWidth"), m_nDeviceFormatWidth);
	pApp->WriteProfileInt(sSection, _T("VideoCaptureDeviceFormatHeight"), m_nDeviceFormatHeight);
	pApp->WriteProfileInt(sSection, _T("MilliSecondsRecBeforeMovementBegin"), m_nMilliSecondsRecBeforeMovementBegin);
	pApp->WriteProfileInt(sSection, _T("MilliSecondsRecAfterMovementEnd"), m_nMilliSecondsRecAfterMovementEnd);
	pApp->WriteProfileInt(sSection, _T("DetectionMinLengthMilliSeconds"), m_nDetectionMinLengthMilliSeconds);
	pApp->WriteProfileInt(sSection, _T("DetectionMaxFrames"), m_nDetectionMaxFrames);
	pApp->WriteProfileInt(sSection, _T("DetectionZoneSize"), m_nDetectionZoneSize);
	pApp->WriteProfileInt(sSection, _T("SaveVideoMovementDetection"), m_bSaveVideo);
	pApp->WriteProfileInt(sSection, _T("SaveAnimGIFMovementDetection"), m_bSaveAnimGIF);
	pApp->WriteProfileInt(sSection, _T("SendMailMalfunction"), m_bSendMailMalfunction);
	pApp->WriteProfileInt(sSection, _T("SendMailMovementDetection"), m_bSendMailRecording);
	pApp->WriteProfileInt(sSection, _T("DoExecCommandMovementDetection"), m_bExecCommand);
	pApp->WriteProfileInt(sSection, _T("ExecModeMovementDetection"), m_nExecCommandMode);
		
	// Attention: GetPrivateProfileString() used by GetProfileString() for INI files
	// strips quotes -> encode quotes here!
	CString sExecCommand(m_sExecCommand); 
	sExecCommand.Replace(_T("\'"), _T("%singlequote%"));
	sExecCommand.Replace(_T("\""), _T("%doublequote%"));
	pApp->WriteProfileString(sSection, _T("ExecCommandMovementDetection"), sExecCommand);
	CString sExecParams(m_sExecParams); 
	sExecParams.Replace(_T("\'"), _T("%singlequote%"));
	sExecParams.Replace(_T("\""), _T("%doublequote%"));
	pApp->WriteProfileString(sSection, _T("ExecParamsMovementDetection"), sExecParams);
		
	pApp->WriteProfileInt(sSection, _T("HideExecCommandMovementDetection"), m_bHideExecCommand);
	pApp->WriteProfileInt(sSection, _T("WaitExecCommandMovementDetection"), m_bWaitExecCommand);
	pApp->WriteProfileInt(sSection, _T("VideoRecQuality"), (int)m_fVideoRecQuality);
	pApp->WriteProfileInt(sSection, _T("ObscureRemovedZones"), (int)m_bObscureRemovedZones);
	pApp->WriteProfileString(sSection, _T("FrameAnnotation"), m_szFrameAnnotation);
	pApp->WriteProfileInt(sSection, _T("ShowFrameTime"), (int)m_bShowFrameTime);
	pApp->WriteProfileInt(sSection, _T("ShowFrameUptime"), (int)m_bShowFrameUptime);
	pApp->WriteProfileInt(sSection, _T("RefFontSize"), m_nRefFontSize);
	pApp->WriteProfileInt(sSection, _T("AnimatedGifWidth"), m_dwAnimatedGifWidth);
	pApp->WriteProfileInt(sSection, _T("AnimatedGifHeight"), m_dwAnimatedGifHeight);
	pApp->WriteProfileInt(sSection, _T("DeleteRecordingsOlderThanDays"), m_nDeleteRecordingsOlderThanDays);
	pApp->WriteProfileInt(sSection, _T("MaxCameraFolderSizeMB"), m_nMaxCameraFolderSizeMB);
	pApp->WriteProfileInt(sSection, _T("MinDiskFreePermillion"), m_nMinDiskFreePermillion);

	// Frame-rate
	unsigned int nSize = sizeof(m_dFrameRate);
	pApp->WriteProfileBinary(sSection, _T("FrameRate"), (LPBYTE)&m_dFrameRate, nSize);
}

void CVideoDeviceDoc::OpenDxVideoDevice(int nId, CString sDevicePathName, CString sDeviceName)
{
	BOOL bOK = FALSE;

	// Allocate
	ASSERT(!m_pDxCapture);
	m_pDxCapture = new CDxCapture;
	m_pDxCapture->SetDoc(this);

	// Device Id
	if (nId < 0)
	{
		CString sDev(sDevicePathName);
		sDev.Replace(_T('/'), _T('\\'));
		nId = CDxCapture::GetDeviceID(sDev);
		if (nId < 0)
		{
			m_pDxCapture->SetDevicePath(sDev);
			m_pDxCapture->SetDeviceName(sDeviceName);
		}
	}

	// Device Pathname
	if (sDevicePathName.IsEmpty())
	{
		sDevicePathName = CDxCapture::GetDevicePath(nId);
		sDevicePathName.Replace(_T('\\'), _T('/'));
	}

	// Device Name
	if (sDeviceName.IsEmpty())
		sDeviceName = CDxCapture::GetDeviceName(nId);

	// Load Settings
	LoadSettings(DEFAULT_FRAMERATE, FALSE, sDevicePathName, sDeviceName);
	::SetTimer(GetView()->GetSafeHwnd(), ID_TIMER_RELOAD, RELOAD_TIMER_MS, NULL);

	// Start Delete Thread
	if (!m_DeleteThread.IsAlive())
		m_DeleteThread.Start(THREAD_PRIORITY_LOWEST);

	// Reset vars
	m_dwFrameCountUp = 0U;
	m_dwNextSnapshotUpTime = ::timeGetTime();
	::InterlockedExchange(&m_lCurrentInitUpTime, (LONG)m_dwNextSnapshotUpTime);

	// Open Dx Capture
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
		if (bOK = m_pDxCapture->Run())
		{
			// Select Input Id for Capture Devices with multiple inputs (S-Video, TV-Tuner,...)
			if (m_nDeviceInputId >= 0 && m_nDeviceInputId < m_pDxCapture->GetInputsCount())
			{
				if (!m_pDxCapture->SetCurrentInput(m_nDeviceInputId))
					m_nDeviceInputId = -1;
			}
			else
				m_nDeviceInputId = m_pDxCapture->SetDefaultInput();

			// Restart process frame
			StartProcessFrame(PROCESSFRAME_DXOPEN);

			// Start Audio Capture Thread
			if (m_bCaptureAudio && !m_bCaptureAudioFromStream)
				m_CaptureAudioThread.Start();
		}
	}
	
	// Title
	SetDocumentTitle();

	// Show error message?
	if (!bOK)
	{
		if (nId < 0)
			ConnectErr(ML_STRING(1568, "Unplugged"), sDeviceName);
		else
			ConnectErr(ML_STRING(1466, "In use or not compatible"), sDeviceName);
	}
}

void CVideoDeviceDoc::InitHttpGetFrameLocations()
{
	// Free (always leave first element!)
	while (m_HttpGetFrameLocations.GetSize() > 1)
		m_HttpGetFrameLocations.RemoveAt(m_HttpGetFrameLocations.GetUpperBound());

	// MJPEG
	if (m_nNetworkDeviceTypeMode == GENERIC_SP)
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

		m_HttpGetFrameLocations.Add(_T("/cgi-bin/mjpg/video.cgi"));			// Dahua (cheap and HD)
		
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

		m_HttpGetFrameLocations.Add(_T("/cgi-bin/video.cgi?msubmenu=mjpg"));// Samsung
		
		m_HttpGetFrameLocations.Add(_T("/-wvhttp-01-/GetOneShot?frame_count=0")); // Canon, NuSpectra
		
		m_HttpGetFrameLocations.Add(_T("/video2.mjpg"));					// Cisco, D-Link
		
		m_HttpGetFrameLocations.Add(_T("/cgi-bin/getimage.cgi?motion=1"));	// GadSpot
		
		m_HttpGetFrameLocations.Add(_T("/nph-mjpeg.cgi"));					// Stardot

		m_HttpGetFrameLocations.Add(_T("/cgi-bin/cmd/system?GET_STREAM"));	// ACTi

		m_HttpGetFrameLocations.Add(_T("/media/?action=stream"));			// A-Tion, Tenvis

		m_HttpGetFrameLocations.Add(_T("/control/faststream.jpg?stream=full"));	// Mobotix

		m_HttpGetFrameLocations.Add(_T("/Streaming/channels/1/httppreview"));	// Hikvision, ABUS (set main-stream to MJPEG in web interface)

		m_HttpGetFrameLocations.Add(_T("/Streaming/channels/2/httppreview"));	// Hikvision, ABUS (set sub-stream to MJPEG in web interface)
		
		m_HttpGetFrameLocations.Add(CString(_T("/image.cgi?mode=http")) +
									_T("&id=") + HTTP_USERNAME_PLACEHOLDER + 
									_T("&passwd=") + HTTP_PASSWORD_PLACEHOLDER);// Intellinet

		m_HttpGetFrameLocations.Add(CString(_T("/cgi-bin/CGIProxy.fcgi?cmd=setSubStreamFormat&format=1")) +
									_T("&usr=") + HTTP_USERNAME_PLACEHOLDER + 
									_T("&pwd=") + HTTP_PASSWORD_PLACEHOLDER);	// Foscam HD, enable MJPG stream
		m_HttpGetFrameLocations.Add(CString(_T("/cgi-bin/CGIStream.cgi?cmd=GetMJStream")) +
									_T("&usr=") + HTTP_USERNAME_PLACEHOLDER + 
									_T("&pwd=") + HTTP_PASSWORD_PLACEHOLDER);	// Foscam HD, get MJPG stream in VGA resolution only
																				// (ATTENTION: when getting the MJPG stream Foscam doesn't url-decode the usr & pwd params)
	}
	// JPEG
	else if (m_nNetworkDeviceTypeMode == GENERIC_CP)
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
		
		m_HttpGetFrameLocations.Add(_T("/cgi-bin/snapshot.cgi"));			// Dahua (cheap and HD)

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

		m_HttpGetFrameLocations.Add(_T("/tmpfs/snap.jpg"));					// Apexis, Astak, Aztech, Dannovo, Dericam, Ensidio, Foscam, iCam+,
		m_HttpGetFrameLocations.Add(_T("/tmpfs/auto.jpg"));					// Instar, IPCMontor, Loftek, Maygion, Niceview, PROCCTV, SmartEye,
																			// Suneyes, Tenvis, Vonnic, Vstarcam, Wansview, Xenta
																			// Note: if both are supported prefer snap.jpg because it returns
																			//       a larger image than auto.jpg

		m_HttpGetFrameLocations.Add(_T("/cgi-bin/net_jpeg.cgi"));			// 3com, 4XEM, Abelcam, ABS, ADT, Amovision, ANRAN, Aposonic, ArcVision,
																			// Beward, Dericam, Digitus, GadSpot, HiSilicon, Mega-pixel, Netwave,
																			// Polaroid, Rayvision, Wanscam, ZyXEL

		m_HttpGetFrameLocations.Add(_T("/img/snapshot.cgi"));				// Allnet, Cisco, Clas, LevelOne, Linksys, NorthQ, Sercomm, Sitecom

		m_HttpGetFrameLocations.Add(_T("/image/jpeg.cgi"));					// D-Link, Sparklan, TrendNet
		
		m_HttpGetFrameLocations.Add(_T("/cgi-bin/viewer/video.jpg"));		// ABUS, D-Link, Toshiba, Vivotek
		
		m_HttpGetFrameLocations.Add(_T("/media/?action=snapshot"));			// A-Tion, Tenvis

		m_HttpGetFrameLocations.Add(_T("/record/current.jpg"));				// Mobotix
		
		m_HttpGetFrameLocations.Add(_T("/cgi-bin/getimage.cgi?motion=0"));	// GadSpot

		m_HttpGetFrameLocations.Add(_T("/oneshotimage.jpg"));				// Sony
		
		m_HttpGetFrameLocations.Add(_T("/-wvhttp-01-/GetLiveImage"));		// Canon

		m_HttpGetFrameLocations.Add(_T("/netcam.jpg"));						// Stardot

		m_HttpGetFrameLocations.Add(_T("/cgi-bin/encoder?SNAPSHOT"));		// ACTi

		m_HttpGetFrameLocations.Add(_T("/Streaming/channels/1/picture"));	// Hikvision, ABUS

		m_HttpGetFrameLocations.Add(_T("/capture1.jpg"));					// Active WebCam Video Surveillance Software

		m_HttpGetFrameLocations.Add(CString(_T("/cgi-bin/CGIProxy.fcgi?cmd=snapPicture2")) +
									_T("&usr=") + HTTP_USERNAME_PLACEHOLDER + 
									_T("&pwd=") + HTTP_PASSWORD_PLACEHOLDER); // Foscam HD
	}

	// Finally add the mixed commands (it depends from the maker whether those commands return JPEG or MJPEG)
	if (m_nNetworkDeviceTypeMode == GENERIC_SP ||
		m_nNetworkDeviceTypeMode == GENERIC_CP)
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
		case GENERIC_SP :	return HTTPSERVERPUSH_DEFAULT_FRAMERATE;
		case GENERIC_CP :	return HTTPCLIENTPOLL_DEFAULT_FRAMERATE;
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
		default :			return DEFAULT_FRAMERATE;
	}
}

// sAddress format
// IP:Port:FrameLocation:NetworkDeviceTypeMode
// or
// HostName:Port:FrameLocation:NetworkDeviceTypeMode
BOOL CVideoDeviceDoc::OpenNetVideoDevice(CString sAddress)
{
	ASSERT(!m_pVideoNetCom);
	ASSERT(!m_pHttpVideoParseProcess);

	// Init Host, Port, FrameLocation and NetworkDeviceTypeMode
	// Fail if sAddress has not the network device address format
	if (!ParseNetworkDevicePathName(sAddress,
									m_sGetFrameVideoHost,
									m_nGetFrameVideoPort,
									m_HttpGetFrameLocations[0],
									m_nNetworkDeviceTypeMode))
		return FALSE;

	// Init http
	if (m_nNetworkDeviceTypeMode < CVideoDeviceDoc::URL_RTSP)
	{
		// Init http get frame locations array
		InitHttpGetFrameLocations();

		// Allocate
		m_pVideoNetCom = new CNetCom;
		m_pHttpVideoParseProcess = new CHttpParseProcess(this);
	}

	// Load Settings
	LoadSettings(	GetDefaultNetworkFrameRate(m_nNetworkDeviceTypeMode),
					m_nNetworkDeviceTypeMode >= CVideoDeviceDoc::URL_RTSP,
					GetDevicePathName(),
					GetDeviceName());
	::SetTimer(GetView()->GetSafeHwnd(), ID_TIMER_RELOAD, RELOAD_TIMER_MS, NULL);

	// Start Delete Thread
	if (!m_DeleteThread.IsAlive())
		m_DeleteThread.Start(THREAD_PRIORITY_LOWEST);

	// Reset vars
	m_dwFrameCountUp = 0U;
	m_dwNextSnapshotUpTime = ::timeGetTime();
	::InterlockedExchange(&m_lCurrentInitUpTime, (LONG)m_dwNextSnapshotUpTime);

	// The Title is set here before starting to connect
	// and also later on when we are connected
	SetDocumentTitle();

	// Connect
	if (m_nNetworkDeviceTypeMode < CVideoDeviceDoc::URL_RTSP)
		ConnectHttp();
	else
		ConnectRtsp();

	return TRUE;
}

void CVideoDeviceDoc::OpenNetVideoDevice(CHostPortDlg* pDlg)
{
	ASSERT(!m_pVideoNetCom);
	ASSERT(!m_pHttpVideoParseProcess);
	ASSERT(pDlg);

	// Init Host, Port, FrameLocation and NetworkDeviceTypeMode
	CHostPortDlg::ParseUrl(	pDlg->m_sHost, pDlg->m_nPort, pDlg->m_nDeviceTypeMode,
							m_sGetFrameVideoHost,
							(int&)m_nGetFrameVideoPort,
							m_HttpGetFrameLocations[0],
							(int&)m_nNetworkDeviceTypeMode);

	// Init http
	if (m_nNetworkDeviceTypeMode < CVideoDeviceDoc::URL_RTSP)
	{
		// Init http get frame locations array
		InitHttpGetFrameLocations();

		// Allocate
		m_pVideoNetCom = new CNetCom;
		m_pHttpVideoParseProcess = new CHttpParseProcess(this);
	}

	// Load Settings
	LoadSettings(	GetDefaultNetworkFrameRate(m_nNetworkDeviceTypeMode),
					m_nNetworkDeviceTypeMode >= CVideoDeviceDoc::URL_RTSP,
					GetDevicePathName(),
					GetDeviceName());
	::SetTimer(GetView()->GetSafeHwnd(), ID_TIMER_RELOAD, RELOAD_TIMER_MS, NULL);

	// Start Delete Thread
	if (!m_DeleteThread.IsAlive())
		m_DeleteThread.Start(THREAD_PRIORITY_LOWEST);

	// Reset vars
	m_dwFrameCountUp = 0U;
	m_dwNextSnapshotUpTime = ::timeGetTime();
	::InterlockedExchange(&m_lCurrentInitUpTime, (LONG)m_dwNextSnapshotUpTime);

	// The Title is set here before starting to connect
	// and also later on when we are connected
	SetDocumentTitle();

	// Connect
	if (m_nNetworkDeviceTypeMode < CVideoDeviceDoc::URL_RTSP)
		ConnectHttp();
	else
		ConnectRtsp();
}

void CVideoDeviceDoc::WaveInitFormat(WORD wCh, DWORD dwSampleRate, WORD wBitsPerSample, LPWAVEFORMATEX pWaveFormat)
{
	if (pWaveFormat)
	{
		pWaveFormat->wFormatTag = WAVE_FORMAT_PCM;
		pWaveFormat->nChannels = wCh;
		pWaveFormat->nSamplesPerSec = dwSampleRate;
		pWaveFormat->nBlockAlign = wCh * wBitsPerSample/8;
		pWaveFormat->nAvgBytesPerSec = dwSampleRate * pWaveFormat->nBlockAlign;
		pWaveFormat->wBitsPerSample = wBitsPerSample;
		pWaveFormat->cbSize = 0;
	}
}

void CVideoDeviceDoc::OnUpdateCaptureRecord(CCmdUI* pCmdUI)
{
	// Note: ON_COMMAND(ID_CAPTURE_RECORD, OnCaptureRecord) is never called,
	//       CVideoDeviceChildFrame::OnToolbarDropDown() is responsible for the menu popup
	pCmdUI->Enable();
	pCmdUI->SetCheck(m_nDetectionLevel != 0 ? 1 : 0);
}

void CVideoDeviceDoc::OnMovDetSensitivity0()
{
	if (WriteDetectionLevelToFile(0, m_sRecordAutoSaveDir))
		m_nDetectionLevel = 0; // always after the write as in OnTimer() it gets polled!
}

void CVideoDeviceDoc::OnUpdateMovDetSensitivity0(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nDetectionLevel == 0 ? 1 : 0);
}

void CVideoDeviceDoc::OnMovDetSensitivity10()
{
	if (WriteDetectionLevelToFile(10, m_sRecordAutoSaveDir))
		m_nDetectionLevel = 10; // always after the write as in OnTimer() it gets polled!
}

void CVideoDeviceDoc::OnUpdateMovDetSensitivity10(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nDetectionLevel == 10 ? 1 : 0);
}

void CVideoDeviceDoc::OnMovDetSensitivity20()
{
	if (WriteDetectionLevelToFile(20, m_sRecordAutoSaveDir))
		m_nDetectionLevel = 20; // always after the write as in OnTimer() it gets polled!
}

void CVideoDeviceDoc::OnUpdateMovDetSensitivity20(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nDetectionLevel == 20 ? 1 : 0);
}

void CVideoDeviceDoc::OnMovDetSensitivity30()
{
	if (WriteDetectionLevelToFile(30, m_sRecordAutoSaveDir))
		m_nDetectionLevel = 30; // always after the write as in OnTimer() it gets polled!
}

void CVideoDeviceDoc::OnUpdateMovDetSensitivity30(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nDetectionLevel == 30 ? 1 : 0);
}

void CVideoDeviceDoc::OnMovDetSensitivity40()
{
	if (WriteDetectionLevelToFile(40, m_sRecordAutoSaveDir))
		m_nDetectionLevel = 40; // always after the write as in OnTimer() it gets polled!
}

void CVideoDeviceDoc::OnUpdateMovDetSensitivity40(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nDetectionLevel == 40 ? 1 : 0);
}

void CVideoDeviceDoc::OnMovDetSensitivity50()
{
	if (WriteDetectionLevelToFile(50, m_sRecordAutoSaveDir))
		m_nDetectionLevel = 50; // always after the write as in OnTimer() it gets polled!
}

void CVideoDeviceDoc::OnUpdateMovDetSensitivity50(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nDetectionLevel == 50 ? 1 : 0);
}

void CVideoDeviceDoc::OnMovDetSensitivity60()
{
	if (WriteDetectionLevelToFile(60, m_sRecordAutoSaveDir))
		m_nDetectionLevel = 60; // always after the write as in OnTimer() it gets polled!
}

void CVideoDeviceDoc::OnUpdateMovDetSensitivity60(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nDetectionLevel == 60 ? 1 : 0);
}

void CVideoDeviceDoc::OnMovDetSensitivity70()
{
	if (WriteDetectionLevelToFile(70, m_sRecordAutoSaveDir))
		m_nDetectionLevel = 70; // always after the write as in OnTimer() it gets polled!
}

void CVideoDeviceDoc::OnUpdateMovDetSensitivity70(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nDetectionLevel == 70 ? 1 : 0);
}

void CVideoDeviceDoc::OnMovDetSensitivity80()
{
	if (WriteDetectionLevelToFile(80, m_sRecordAutoSaveDir))
		m_nDetectionLevel = 80; // always after the write as in OnTimer() it gets polled!
}

void CVideoDeviceDoc::OnUpdateMovDetSensitivity80(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nDetectionLevel == 80 ? 1 : 0);
}

void CVideoDeviceDoc::OnMovDetSensitivity90()
{
	if (WriteDetectionLevelToFile(90, m_sRecordAutoSaveDir))
		m_nDetectionLevel = 90; // always after the write as in OnTimer() it gets polled!
}

void CVideoDeviceDoc::OnUpdateMovDetSensitivity90(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nDetectionLevel == 90 ? 1 : 0);
}

void CVideoDeviceDoc::OnMovDetSensitivity100()
{
	if (WriteDetectionLevelToFile(100, m_sRecordAutoSaveDir))
		m_nDetectionLevel = 100; // always after the write as in OnTimer() it gets polled!
}

void CVideoDeviceDoc::OnUpdateMovDetSensitivity100(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nDetectionLevel == 100 ? 1 : 0);
}

void CVideoDeviceDoc::OnCaptureObscureSource()
{
	CString sFileName;
	if (m_sRecordAutoSaveDir != _T(""))
	{
		sFileName = m_sRecordAutoSaveDir;
		sFileName.TrimRight(_T('\\'));
		sFileName += CString(_T("\\")) + CAMERA_IS_OBSCURED_FILENAME;
	}
	if (m_bObscureSource)
	{
		::DeleteFile(sFileName);
		m_bObscureSource = FALSE;
	}
	else
	{
		FILE* pf = _tfopen(sFileName, _T("wb"));
		if (pf)
		{
			const char msg[] = "Note: delete this file to re-enable the camera";
			fwrite(msg, sizeof(char), strlen(msg), pf);
			fclose(pf);
		}
		m_bObscureSource = TRUE;
	}
}

void CVideoDeviceDoc::OnUpdateCaptureObscureSource(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bObscureSource ? 1 : 0);
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
			else
			{
				::LogLine(_T("%s"), GetAssignedDeviceName() + _T(", error getting DV video size!"));
				return;
			}
		}
		else
		{
			AM_MEDIA_TYPE* pmtConfig = NULL;
			if (!m_pDxCapture->GetCurrentFormat(&pmtConfig))
			{
				::LogLine(_T("%s"), GetAssignedDeviceName() + _T(", error getting current video format!"));
				return;
			}
			if (pmtConfig->formattype != FORMAT_VideoInfo	||
				!pmtConfig->pbFormat						||
				pmtConfig->cbFormat < sizeof(VIDEOINFOHEADER))
			{
				m_pDxCapture->DeleteMediaType(pmtConfig);
				::LogLine(_T("%s"), GetAssignedDeviceName() + _T(", unsupported video format!"));
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

void CVideoDeviceDoc::OnCaptureCameraBasicSettings() 
{
	CaptureCameraBasicSettings();
}

void CVideoDeviceDoc::OnUpdateCaptureCameraBasicSettings(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!m_bClosing);
}

void CVideoDeviceDoc::CaptureCameraBasicSettings()
{
	GetView()->ForceCursor();
	CCameraBasicSettingsDlg dlg(this, GetView());
	dlg.DoModal();
	GetView()->ForceCursor(FALSE);
}

void CVideoDeviceDoc::OnCaptureCameraAdvancedSettings() 
{
	// Create if First Time
	if (!m_pCameraAdvancedSettingsDlg)
	{
		m_pCameraAdvancedSettingsDlg = new CCameraAdvancedSettingsDlg(GetView());
		m_pCameraAdvancedSettingsDlg->Show();
	}
	// Toggle Visible / Invisible State
	else
	{
		if (m_pCameraAdvancedSettingsDlg->IsWindowVisible())
			m_pCameraAdvancedSettingsDlg->Hide();
		else
			m_pCameraAdvancedSettingsDlg->Show();
	}
}

void CVideoDeviceDoc::OnUpdateCaptureCameraAdvancedSettings(CCmdUI* pCmdUI) 
{
	if (m_pCameraAdvancedSettingsDlg)
		pCmdUI->SetCheck(m_pCameraAdvancedSettingsDlg->IsWindowVisible() ? 1 : 0);
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
			if (!m_bStopAndChangeDVFormat)
			{
				m_bStopAndChangeDVFormat = TRUE;
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
	else
	{
		if (m_nNetworkDeviceTypeMode >= URL_RTSP)
		{
			BeginWaitCursor();
			::ShellExecute(NULL, _T("open"), ::UrlEncode(_T("http://") + m_sGetFrameVideoHost, FALSE), NULL, NULL, SW_SHOWNORMAL);
			EndWaitCursor();
		}
		else if (	m_nNetworkDeviceTypeMode == GENERIC_SP	||
					m_nNetworkDeviceTypeMode == GENERIC_CP	||
					m_nNetworkDeviceTypeMode == TPLINK_SP	||
					m_nNetworkDeviceTypeMode == TPLINK_CP)
		{
			CString sUrl;
			if (m_nGetFrameVideoPort != 80)
				sUrl.Format(_T("http://%s:%d"), m_sGetFrameVideoHost, m_nGetFrameVideoPort);
			else
				sUrl.Format(_T("http://%s"), m_sGetFrameVideoHost);
			BeginWaitCursor();
			::ShellExecute(NULL, _T("open"), ::UrlEncode(sUrl, FALSE), NULL, NULL, SW_SHOWNORMAL);
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

void CVideoDeviceDoc::MicroApacheUpdateMainFiles()
{
	// Copy index_rootdir.php, favicon.ico, contaware_256x256.png and manifest.json to Doc Root
	CString sDocRoot = ((CUImagerApp*)::AfxGetApp())->m_sMicroApacheDocRoot;
	sDocRoot.TrimRight(_T('\\'));
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szProgramName[MAX_PATH];
	if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
		return;
	_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
	::CopyFile(	CString(szDrive) + CString(szDir) + MICROAPACHE_HTDOCS + _T("\\") + PHP_INDEXROOTDIRNAME_EXT,
				sDocRoot + _T("\\") + PHP_INDEXNAME_EXT,
				FALSE); // overwrite if existing
	::CopyFile(	CString(szDrive) + CString(szDir) + MICROAPACHE_HTDOCS + _T("\\") + IMAGE_FAVICONNAME_EXT,
				sDocRoot + _T("\\") + IMAGE_FAVICONNAME_EXT,
				FALSE); // overwrite if existing
	::CopyFile(	CString(szDrive) + CString(szDir) + MICROAPACHE_HTDOCS + _T("\\") + IMAGE_CONTAWAREICONNAME_EXT,
				sDocRoot + _T("\\") + IMAGE_CONTAWAREICONNAME_EXT,
				FALSE); // overwrite if existing
	::CopyFile(	CString(szDrive) + CString(szDir) + MICROAPACHE_HTDOCS + _T("\\") + JSON_MANIFESTNAME_EXT,
				sDocRoot + _T("\\") + JSON_MANIFESTNAME_EXT,
				FALSE); // overwrite if existing

	// Warning
	CString sConfig, sFormat;
	sConfig =  _T("# DO NOT MODIFY THIS FILE (ContaCam will overwrite your changes)\r\n");
	sConfig += _T("# Make your customizations in ") + CString(MICROAPACHE_EDITABLE_CONFIGNAME_EXT) + _T("\r\n\r\n");

	// Listen Ports
	sFormat.Format(_T("Listen %d\r\n"), ((CUImagerApp*)::AfxGetApp())->m_nMicroApachePort);
	sConfig += sFormat;
	sFormat.Format(_T("Listen %d\r\n"), ((CUImagerApp*)::AfxGetApp())->m_nMicroApachePortSSL);
	sConfig += sFormat;

	// Server name, root and admin email
	sConfig += _T("ServerName localhost\r\n");
	sConfig += _T("ServerRoot .\r\n");
	sConfig += _T("ServerAdmin webmaster@nowhere.com\r\n");

	// DocumentRoot
	// Note: APACHE resolves a path to its long version before passing it to PHP.
	// PHP file functions use the ANSI API of Windows and are unable to deal with
	// unicode characters in filenames. So even if configuring here the 8.3 short path
	// name, PHP will get the long path and will fail if it contains non-ASCII
	// characters. In conclusion we have to make sure that the DocumentRoot contains
	// only ASCII characters because the following GetASCIICompatiblePath(sDocRoot)
	// call is not enough for non-static pages.
	sDocRoot = ::GetASCIICompatiblePath(sDocRoot);				// directory must exist!
	sDocRoot.Replace(_T('\\'), _T('/'));						// change path from \ to / (otherwise apache is not happy)
	sFormat.Format(_T("DocumentRoot \"%s/\"\r\n"), sDocRoot);	// even if the apache docu says that the DocumentRoot
	sConfig += sFormat;											// should be specified without a trailing slash, add it
																// anyway, otherwise it is not working when the root
																// directory is the drive itself (c: for example)

	// Global settings
	sConfig += _T("ThreadsPerChild 128\r\n");
	sConfig += _T("AcceptFilter http none\r\n");										// the default "connect" will use the AcceptEx() API, "none" uses accept() and will 
	sConfig += _T("AcceptFilter https none\r\n");										// not recycle sockets between connections. This is useful for network adapters with
																						// broken driver support, as well as some virtual network providers!
	sConfig += _T("EnableMMAP off\r\n");												// do not use memory mapping to read files during delivery because deleting or truncating
																						// a file while httpd has it memory-mapped can cause httpd to crash!
	sConfig += _T("LoadFile php/libeay32.dll\r\n");										// for PHP but also for mod_ssl.so
	sConfig += _T("LoadFile php/ssleay32.dll\r\n");										// for PHP but also for mod_ssl.so
	sConfig += _T("LoadModule auth_basic_module modules/mod_auth_basic.so\r\n");		// for basic auth support
	sConfig += _T("LoadModule authn_core_module modules/mod_authn_core.so\r\n");		// for AuthType and AuthName Directives support
	sConfig += _T("LoadModule authn_file_module modules/mod_authn_file.so\r\n");		// for AuthUserFile Directive support
	sConfig += _T("LoadModule authz_core_module modules/mod_authz_core.so\r\n");		// for Require Directive support
	sConfig += _T("LoadModule authz_user_module modules/mod_authz_user.so\r\n");		// for user support in Require Directive
	sConfig += _T("LoadModule dir_module modules/mod_dir.so\r\n");						// for DirectorySlash Directive (On by default) support, it appends the trailing slashes
	sConfig += _T("LoadModule mime_module modules/mod_mime.so\r\n");
	sConfig += _T("LoadModule rewrite_module modules/mod_rewrite.so\r\n");
	sConfig += _T("LoadModule ssl_module modules/mod_ssl.so\r\n");
	sConfig += _T("LoadModule php5_module php/php5apache2_4.dll\r\n");
	sConfig += _T("AddHandler application/x-httpd-php .php\r\n");
	sConfig += _T("PHPIniDir php\r\n");
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
	sConfigDir.Replace(_T('\\'), _T('/')); // change path from \ to / (otherwise apache is not happy)
	sConfig += _T("ErrorLog \"") + sConfigDir + MICROAPACHE_LOGNAME_EXT + _T("\"\r\n");
	sConfig += _T("PidFile \"") + sConfigDir + MICROAPACHE_PIDNAME_EXT + _T("\"\r\n");
	
	// PHP session
	// 1. session.gc_maxlifetime specifies the number of seconds after which inactive session data will be seen as garbage.
	//    For performance reasons session_start() calls the garbage collector (which cleans-up the above mentioned garbage)
	//    with a frequency of session.gc_probability / session.gc_divisor.
	//    Note1: on heavily used servers set session.gc_divisor to a bigger value (session.gc_probability is usually left 1).
	//    Note2: it's not possible to expire its own session (on the same browser) because session_start() first refreshes its
	//           own session updating the session file timestamp and only after that the garbage collector is called. To expire
	//           sessions another user/browser must access the server.
	//    Note3: in ContaCam web interface sessions will not expire as long as the page polls snapshots or refreshes with the
	//           html meta tag, only if a video is open the session can be expired by another user/browser.
	//
	// 2. session.cookie_lifetime (default 0, which means until the browser's next restart) defines how long (in seconds) a
	//    session cookie will live. Sounds similar to session.gc_maxlifetime, but it's a completely different approach. This
	//    value defines the absolute maximum lifetime of a session, whether the user is active or not.
	CString sTempDir = ((CUImagerApp*)::AfxGetApp())->GetAppTempDir(); // directory has been created in InitInstance()
	sTempDir = ::GetASCIICompatiblePath(sTempDir); // directory must exist!
	sTempDir.Replace(_T('\\'), _T('/')); // change path from \ to / (otherwise apache is not happy)
	sConfig += _T("php_value session.save_path \"") + sTempDir + _T("\"\r\n");
	sConfig += _T("php_value session.gc_probability 1\r\n");	// session_start() has 1 / 10 or 10% probability (default is 1)
	sConfig += _T("php_value session.gc_divisor 10\r\n");		// to clean-up old session files (default is 100)
	sConfig += _T("php_value session.gc_maxlifetime 3600\r\n");	// session maximum lifetime of 1 hour (default is 1440 seconds)
	sConfig += _T("php_value session.cookie_lifetime 0\r\n");	// the default of 0 means until the browser is closed

	// SSL
	CString sMicroApacheCertFileSSL = ::GetASCIICompatiblePath(((CUImagerApp*)::AfxGetApp())->m_sMicroApacheCertFileSSL); // file must exist!
	sMicroApacheCertFileSSL.Replace(_T('\\'), _T('/')); // change path from \ to / (otherwise apache is not happy)
	CString sMicroApacheKeyFileSSL = ::GetASCIICompatiblePath(((CUImagerApp*)::AfxGetApp())->m_sMicroApacheKeyFileSSL); // file must exist!
	sMicroApacheKeyFileSSL.Replace(_T('\\'), _T('/')); // change path from \ to / (otherwise apache is not happy)
	sFormat.Format(_T("<VirtualHost *:%d>\r\n"), ((CUImagerApp*)::AfxGetApp())->m_nMicroApachePortSSL);
	sConfig += sFormat;
	sConfig += _T("ServerName localhost\r\n");
	sConfig += _T("SSLEngine on\r\n");
	sConfig += _T("SSLCertificateFile \"") + sMicroApacheCertFileSSL + _T("\"\r\n");
	sConfig += _T("SSLCertificateKeyFile \"") + sMicroApacheKeyFileSSL + _T("\"\r\n");
	sConfig += _T("</VirtualHost>\r\n");

	// <Directory />
	sConfig += _T("<Directory />\r\n");
	sConfig += _T("AllowOverride None\r\n"); // do not allow .htaccess files
	if (((CUImagerApp*)::AfxGetApp())->m_sMicroApacheUsername != _T("") ||
		((CUImagerApp*)::AfxGetApp())->m_sMicroApachePassword != _T(""))
	{
		// Do not allow direct .mp4, .gif, .jpg and .recycled accesses
		// Attention: cannot use those file types in css!
		// Case insensitive regular expression "not match" (Apache 2.4 or higher with mod_authz_core.so)
		// Note: %{REQUEST_URI} is only the URI path part without the query string!
		sConfig += _T("Require expr %{REQUEST_URI} !~ m#\\.(mp4|gif|jpg|recycled)$#i\r\n");
	}
	else
	{
		// Do not allow direct .recycled accesses
		// Case insensitive regular expression "not match" (Apache 2.4 or higher with mod_authz_core.so)
		// Note: %{REQUEST_URI} is only the URI path part without the query string!
		sConfig += _T("Require expr %{REQUEST_URI} !~ m#\\.recycled$#i\r\n");
	}
	sConfig += _T("</Directory>\r\n");
	
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

	// Save config file (overwrite if existing)
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

	// Make or delete authentication php file?
	CString sAuthenticateFilePath(sDocRoot + _T("\\") + PHP_AUTHENTICATENAME_EXT);
	if (((CUImagerApp*)::AfxGetApp())->m_sMicroApacheUsername != _T("") ||
		((CUImagerApp*)::AfxGetApp())->m_sMicroApachePassword != _T(""))
	{
		CSHA256 SHA256Username;
		CSHA256Hash SHA256SaltUsername;
		CSHA256Hash SHA256HashUsername;
		CSHA256 SHA256Password;
		CSHA256Hash SHA256SaltPassword;
		CSHA256Hash SHA256HashPassword;
	
		if (SHA256Username.Random(SHA256SaltUsername) &&
			SHA256Password.Random(SHA256SaltPassword))
		{
			LPBYTE pUsername = NULL;
			LPBYTE pPassword = NULL;
			int nUsernameSize = ::ToUTF8(SHA256SaltUsername.Format(FALSE) + ((CUImagerApp*)::AfxGetApp())->m_sMicroApacheUsername, &pUsername);
			int nPasswordSize = ::ToUTF8(SHA256SaltPassword.Format(FALSE) + ((CUImagerApp*)::AfxGetApp())->m_sMicroApachePassword, &pPassword);
			if (SHA256Username.Hash(pUsername, nUsernameSize, SHA256HashUsername) &&	// pointers and/or sizes can be zero
				SHA256Password.Hash(pPassword, nPasswordSize, SHA256HashPassword))		// (for that case the empty-string-hash is returned)
			{
				// Prepare php file
				CString sAuthenticate;
				sAuthenticate.Format(
					_T("<?php\r\n")
					_T("if (count(get_included_files()) == 1) { // logout if directly accessing this script\r\n")
					_T("	session_start();\r\n")
					_T("	unset($_SESSION['username']);\r\n")
					_T("	$host     = (!empty($_SERVER['HTTP_X_FORWARDED_HOST'])) ? $_SERVER['HTTP_X_FORWARDED_HOST'] : $_SERVER['HTTP_HOST'];\r\n")
					_T("	$rel_path = dirname($_SERVER['PHP_SELF']);\r\n")
					_T("	$rel_path = str_replace(\"\\\\\", \"/\", $rel_path);\r\n")
					_T("	$rel_path = rtrim($rel_path, \"/\");\r\n")
					_T("	if (!empty($_SERVER['HTTPS']) && (strtolower($_SERVER['HTTPS']) != 'off'))\r\n")
					_T("		header(\"Location: https://$host$rel_path/\");\r\n")
					_T("	else\r\n")
					_T("		header(\"Location: http://$host$rel_path/\");\r\n")
					_T("	exit;\r\n")
					_T("}\r\n")
					_T("$username_salt = '%s';\r\n")
					_T("$username_hash = '%s';\r\n")
					_T("$password_salt = '%s';\r\n")
					_T("$password_hash = '%s';\r\n"),
					SHA256SaltUsername.Format(FALSE),
					SHA256HashUsername.Format(FALSE),
					SHA256SaltPassword.Format(FALSE),
					SHA256HashPassword.Format(FALSE));
				sAuthenticate += _T("$invalid_login = 0;\r\n");
				sAuthenticate += _T("if (isset($_POST['username']) && isset($_POST['password'])) {\r\n");
				sAuthenticate += _T("	if (hash('sha256', $username_salt . $_POST['username']) == \"$username_hash\" &&\r\n");
				sAuthenticate += _T("		hash('sha256', $password_salt . $_POST['password']) == \"$password_hash\") {\r\n");
				sAuthenticate += _T("		$_SESSION['username'] = \"$username_hash\";\r\n");
				sAuthenticate += _T("		return;\r\n");
				sAuthenticate += _T("	} else {\r\n");
				sAuthenticate += _T("		unset($_SESSION['username']);\r\n");
				sAuthenticate += _T("		$invalid_login = 1;\r\n");
				sAuthenticate += _T("		sleep(1);\r\n");
				sAuthenticate += _T("	}\r\n");
				sAuthenticate += _T("}\r\n");
				sAuthenticate += _T("else if (isset($_SERVER['PHP_AUTH_USER']) && isset($_SERVER['PHP_AUTH_PW'])) {\r\n");
				sAuthenticate += _T("	if (hash('sha256', $username_salt . $_SERVER['PHP_AUTH_USER']) == \"$username_hash\" &&\r\n");
				sAuthenticate += _T("		hash('sha256', $password_salt . $_SERVER['PHP_AUTH_PW']) == \"$password_hash\") {\r\n");
				sAuthenticate += _T("		$_SESSION['username'] = \"$username_hash\";\r\n");
				sAuthenticate += _T("		return;\r\n");
				sAuthenticate += _T("	} else {\r\n");
				sAuthenticate += _T("		unset($_SESSION['username']);\r\n");
				sAuthenticate += _T("		$invalid_login = 1;\r\n");
				sAuthenticate += _T("		sleep(1);\r\n");
				sAuthenticate += _T("	}\r\n");
				sAuthenticate += _T("}\r\n");
				sAuthenticate += _T("else if (isset($_SESSION['username']) && $_SESSION['username'] == \"$username_hash\")\r\n");
				sAuthenticate += _T("	return;\r\n");
				sAuthenticate += _T("if (isset($httpbasicauth) && $httpbasicauth == 1) {\r\n");
				sAuthenticate += _T("	header(\'WWW-Authenticate: Basic realm=\"ContaCam\"\');\r\n");
				sAuthenticate += _T("	header(\'HTTP/1.0 401 Unauthorized\');\r\n");
				sAuthenticate += _T("	die(\'Not authorized\');\r\n");
				sAuthenticate += _T("}\r\n");
				sAuthenticate += _T("?>\r\n");
				sAuthenticate += _T("<!DOCTYPE html>\r\n");
				sAuthenticate += _T("<html>\r\n");
				sAuthenticate += _T("<head>\r\n");
				sAuthenticate += _T("<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\" />\r\n");
				sAuthenticate += _T("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\r\n");
				sAuthenticate += _T("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\r\n");
				sAuthenticate += _T("<meta name=\"author\" content=\"Oliver Pfister\" />\r\n");
				sAuthenticate += _T("<title>Login</title>\r\n");
				sAuthenticate += _T("<style type=\"text/css\">\r\n");
				sAuthenticate += _T("/*<![CDATA[*/\r\n");
				sAuthenticate += _T("* {\r\n");
				sAuthenticate += _T("	margin: 0;\r\n");
				sAuthenticate += _T("	padding: 0;\r\n");
				sAuthenticate += _T("}\r\n");
				sAuthenticate += _T("body {\r\n");
				sAuthenticate += _T("	font-family: Helvetica,Arial,sans-serif;\r\n");
				sAuthenticate += _T("	font-size: 16px;\r\n");
				sAuthenticate += _T("	background-color: #333333;\r\n");
				sAuthenticate += _T("	color: #bbbbbb;\r\n");
				sAuthenticate += _T("}\r\n");
				sAuthenticate += _T("form span {\r\n");
				sAuthenticate += _T("	mix-blend-mode: difference;\r\n");
				sAuthenticate += _T("}\r\n");
				sAuthenticate += _T("input[type=\'text\'], input[type=\'password\'], input[type=\'submit\'] {\r\n");
				sAuthenticate += _T("	font-size: 14px;\r\n");
				sAuthenticate += _T("	font-style: normal;\r\n");
				sAuthenticate += _T("	font-weight: bold;\r\n");
				sAuthenticate += _T("	font-family: sans-serif,Arial,Helvetica;\r\n");
				sAuthenticate += _T("	padding: 4px;\r\n");
				sAuthenticate += _T("	margin: 2px 0;\r\n");
				sAuthenticate += _T("	color: #111111;\r\n");
				sAuthenticate += _T("	background-color: #ededed;\r\n");
				sAuthenticate += _T("	border: 1px solid;\r\n");
				sAuthenticate += _T("	border-color: #33ccff #039ccf #039ccf #33ccff;\r\n");
				sAuthenticate += _T("}\r\n");
				sAuthenticate += _T("input[type=\'text\'], input[type=\'password\'] {\r\n");
				sAuthenticate += _T("	text-align: left;\r\n");
				sAuthenticate += _T("}\r\n");
				sAuthenticate += _T("input[type=\'submit\'] {\r\n");
				sAuthenticate += _T("	text-align: center;\r\n");
				sAuthenticate += _T("}\r\n");
				sAuthenticate += _T("/*]]>*/\r\n");
				sAuthenticate += _T("</style>\r\n");
				sAuthenticate += _T("</head>\r\n");
				sAuthenticate += _T("<body>\r\n");
				sAuthenticate += _T("<div style=\"text-align: center\">\r\n");
				sAuthenticate += _T("<form name=\"login\" action=\"\" method=\"post\">\r\n");
				sAuthenticate += _T("<?php if ($invalid_login): ?>\r\n");
				sAuthenticate += _T("<span>&#x1F464;</span><input style=\"border:1px solid #f00;\" type=\"text\" name=\"username\" autocapitalize=\"none\" autofocus /><br />\r\n");
				sAuthenticate += _T("<span>&#x1f512;</span><input style=\"border:1px solid #f00;\" type=\"password\" name=\"password\" /><br />\r\n");
				sAuthenticate += _T("<input type=\"submit\" name=\"submit\" value=\"Login\" />\r\n");
				sAuthenticate += _T("<?php else: ?>\r\n");
				sAuthenticate += _T("<span>&#x1F464;</span><input type=\"text\" name=\"username\" autocapitalize=\"none\" /><br />\r\n");
				sAuthenticate += _T("<span>&#x1f512;</span><input type=\"password\" name=\"password\" /><br />\r\n");
				sAuthenticate += _T("<input type=\"submit\" name=\"submit\" value=\"Login\" autofocus />\r\n");
				sAuthenticate += _T("<?php endif; ?>\r\n");
				sAuthenticate += _T("</form>\r\n");
				sAuthenticate += _T("</div>\r\n");
				sAuthenticate += _T("</body>\r\n");
				sAuthenticate += _T("</html>\r\n");
				sAuthenticate += _T("<?php\r\n");
				sAuthenticate += _T("exit;\r\n");

				// Save php file (overwrite if existing)
				LPBYTE pAuthenticateFileContent = NULL;
				int nAuthenticateFileContentSize = ::ToUTF8(sAuthenticate, &pAuthenticateFileContent);
				if (nAuthenticateFileContentSize > 0 && pAuthenticateFileContent)
				{
					try
					{
						CFile f(sAuthenticateFilePath,
							CFile::modeCreate |
							CFile::modeWrite |
							CFile::shareDenyWrite);
						f.Write(pAuthenticateFileContent, nAuthenticateFileContentSize);
					}
					catch (CFileException* e)
					{
						e->Delete();
					}
				}
				if (pAuthenticateFileContent)
					delete[] pAuthenticateFileContent;
			}

			// Free
			if (pUsername)
				delete[] pUsername;
			if (pPassword)
				delete[] pPassword;
		}
	}
	else
		::DeleteFile(sAuthenticateFilePath);
}

BOOL CVideoDeviceDoc::MicroApacheUpdateWebFiles(CString sAutoSaveDir)
{
	// Init source directory file find
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szProgramName[MAX_PATH];
	if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
		return FALSE;
	_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
	CSortableFileFind FileFind;
	if (!FileFind.InitRecursive(CString(szDrive) + CString(szDir) + MICROAPACHE_HTDOCS + _T("\\*"), FALSE))
		return FALSE;
	if (FileFind.WaitRecursiveDone() != 1)
		return FALSE;

	// Create dirs
	sAutoSaveDir.TrimRight(_T('\\'));
	sAutoSaveDir += _T('\\');
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

	// Copy files
	for (pos = 0 ; pos < FileFind.GetFilesCount() ; pos++)
	{
		CString sName = FileFind.GetFileName(pos);
		CString sShortName = ::GetShortFileName(sName);
		CString sRelName = sName.Mid(nRootDirNameSize);
		if (sShortName.CompareNoCase(PHP_INDEXROOTDIRNAME_EXT) != 0		&&
			sShortName.CompareNoCase(IMAGE_FAVICONNAME_EXT) != 0		&&
			sShortName.CompareNoCase(IMAGE_CONTAWAREICONNAME_EXT) != 0	&&
			sShortName.CompareNoCase(JSON_MANIFESTNAME_EXT) != 0		&&
			sShortName.CompareNoCase(THUMBS_DB) != 0)
		{
			if (sShortName.CompareNoCase(PHP_CONFIGNAME_EXT) == 0)
				::CopyFile(sName, sAutoSaveDir + sRelName, TRUE);	// Never overwrite the configuration file!
			else
				::CopyFile(sName, sAutoSaveDir + sRelName, FALSE);	// Always overwrite to get new version!
		}
	}

	// Remove old unused files
	::DeleteFile(sAutoSaveDir + _T("snapshotmobile.php"));
	::DeleteFile(sAutoSaveDir + _T("jpeglive.php"));
	::DeleteFile(sAutoSaveDir + _T("summarylive.php"));
	::DeleteFile(sAutoSaveDir + _T("summary.php"));
	::DeleteFile(sAutoSaveDir + _T("summarynav.php"));
	::DeleteFile(sAutoSaveDir + _T("separator.php"));
	::DeleteFile(sAutoSaveDir + _T("swf.php"));
	::DeleteFile(sAutoSaveDir + _T("avi.php"));
	::DeleteFile(sAutoSaveDir + _T("jpeg.php"));
	::DeleteFile(sAutoSaveDir + _T("movtrigger.bat"));
	::DeleteFile(sAutoSaveDir + _T("movtrigger.txt"));
	::DeleteFile(sAutoSaveDir + _T("styles\\black.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\darkgray.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\gray.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\white.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\show_black.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\hide_black.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\show_white.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\hide_white.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\show_darkgray.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\hide_darkgray.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\show_gray.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\hide_gray.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\reload_black.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\reload_white.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\reload_darkgray.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\reload_gray.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\camon_black.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\camon_white.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\camon_darkgray.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\camon_gray.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\back_black.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\back_white.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\back_darkgray.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\back_gray.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\home_black.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\home_white.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\home_darkgray.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\home_gray.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\save_black.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\save_white.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\save_darkgray.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\save_gray.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\scaler_slider_black.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\scaler_slider_track_black.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\scaler_slider_white.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\scaler_slider_track_white.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\scaler_slider_darkgray.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\scaler_slider_track_darkgray.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\scaler_slider_gray.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\scaler_slider_track_gray.gif"));
	::DeleteFile(sAutoSaveDir + _T("styles\\trash.png"));
	::DeleteFile(sAutoSaveDir + _T("styles\\trashhover.png"));
	::DeleteDir(sAutoSaveDir + _T("js"));

	return TRUE;
}

CString CVideoDeviceDoc::MicroApacheGetConfigFileName()
{
	CString sMicroapacheConfigFile = CUImagerApp::GetConfigFilesDir();
	sMicroapacheConfigFile += CString(_T("\\")) + MICROAPACHE_CONFIGNAME_EXT;
	return sMicroapacheConfigFile;
}

CString CVideoDeviceDoc::MicroApacheGetEditableConfigFileName()
{
	CString sMicroapacheEditableConfigFile = CUImagerApp::GetConfigFilesDir();
	sMicroapacheEditableConfigFile += CString(_T("\\")) + MICROAPACHE_EDITABLE_CONFIGNAME_EXT;
	return sMicroapacheEditableConfigFile;
}

CString CVideoDeviceDoc::MicroApacheGetLogFileName()
{
	CString sMicroapacheLogFile = CUImagerApp::GetConfigFilesDir();
	sMicroapacheLogFile += CString(_T("\\")) + MICROAPACHE_LOGNAME_EXT;
	return sMicroapacheLogFile;
}

CString CVideoDeviceDoc::MicroApacheGetPidFileName()
{
	CString sMicroapachePidFile = CUImagerApp::GetConfigFilesDir();
	sMicroapachePidFile += CString(_T("\\")) + MICROAPACHE_PIDNAME_EXT;
	return sMicroapachePidFile;
}

BOOL CVideoDeviceDoc::MicroApacheStart(DWORD dwTimeoutMs)
{
	// Config file path
	CString sMicroapacheConfigFile = MicroApacheGetConfigFileName();
	if (!::IsExistingFile(sMicroapacheConfigFile))
		return FALSE;
	sMicroapacheConfigFile = ::GetASCIICompatiblePath(sMicroapacheConfigFile); // file must exist!
	CString sConfigDir = ::GetDriveAndDirName(sMicroapacheConfigFile);

	// Delete old pid file from crashed / killed processes
	CString sMicroapachePidFile = sConfigDir + MICROAPACHE_PIDNAME_EXT;
	::DeleteFile(sMicroapachePidFile);

	// Delete old log file to avoid growing it too much
	CString sMicroapacheLogFile = sConfigDir + MICROAPACHE_LOGNAME_EXT;
	::DeleteFile(sMicroapacheLogFile);
	
	// Startup parameters
	sMicroapacheConfigFile.Replace(_T('\\'), _T('/'));	// change path from \ to / (otherwise apache is not happy)
	sMicroapacheLogFile.Replace(_T('\\'), _T('/'));		// change path from \ to / (otherwise apache is not happy)
	CString sParams = _T("-f \"") + sMicroapacheConfigFile + _T("\" -e debug -E \"") + sMicroapacheLogFile + _T("\"");

	// Executable path
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
	
	// Start mapache.exe
	//
	// Note: do not use ShellExecuteEx because it creates a separate thread
	//       and does not return until that thread completes. During this time
	//       ShellExecuteEx will pump window messages to prevent windows
	//       owned by the calling thread from appearing hung. We do not want
	//       that messages are pumped when starting ContaCam!
	//       We also need the STARTF_FORCEOFFFEEDBACK flag, which is not available
	//       with ShellExecuteEx, this flag avoids showing the busy cursor while
	//       starting the mapache.exe process.
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);
	si.dwFlags =	STARTF_FORCEOFFFEEDBACK |	// do not display the busy cursor
					STARTF_USESHOWWINDOW;		// use the following wShowWindow
	si.wShowWindow = SW_HIDE;
	TCHAR lpCommandLine[32768];
	_tcscpy_s(lpCommandLine, _T("\"") + sMicroapacheStartFile + _T("\"") + _T(" ") + sParams);
	DWORD dwStartUpTimeMs = ::GetTickCount();
	BOOL bStarted = ::CreateProcess(sMicroapacheStartFile, lpCommandLine,
									NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL,
									::GetDriveAndDirName(sMicroapacheStartFile), &si, &pi);
	if (pi.hProcess)
		::CloseHandle(pi.hProcess);
	if (pi.hThread)
		::CloseHandle(pi.hThread);

	// Wait a max of dwTimeoutMs
	if (bStarted)
	{
		do
		{
			if (::IsExistingFile(sMicroapachePidFile))
				return TRUE;
			::Sleep(MICROAPACHE_WAITTIME_MS);
		}
		while ((::GetTickCount() - dwStartUpTimeMs) < dwTimeoutMs);
	}

	return FALSE;
}

void CVideoDeviceDoc::MicroApacheShutdown(DWORD dwTimeoutMs)
{
	// Init with an invalid process id
	// see: https://blogs.msdn.microsoft.com/oldnewthing/20040223-00/?p=40503
	DWORD dwPid = 0; // this is the System Idle Process
	
	// Shutdown by event signalisation
	DWORD dwShutdownTimeMs = ::GetTickCount();
	BOOL bShutdownSignaled = FALSE;
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
			pData = new BYTE[dwLength + 1];
			if (pData)
			{
				// Read Pid
				dwLength = f.Read(pData, dwLength);
				pData[dwLength] = '\0';
				CString sPid;
				sPid = CString((LPCSTR)pData);
				sPid.TrimLeft();
				sPid.TrimRight();
				delete[] pData;
				dwPid = _tcstol(sPid.GetBuffer(0), NULL, 10);
				sPid.ReleaseBuffer();

				// Get the existing event
				CString sEventName;
				sEventName = _T("ap") + sPid + _T("_shutdown");
				HANDLE hShutdownEvent = ::OpenEvent(EVENT_MODIFY_STATE, FALSE, sEventName);
				if (hShutdownEvent)
				{
					// Set the event
					bShutdownSignaled = ::SetEvent(hShutdownEvent);

					// Clean-up
					::CloseHandle(hShutdownEvent);
				}
			}
		}
	}
	catch (CFileException* e)
	{
		if (pData)
			delete [] pData;
		e->Delete();
	}

	// Kill if shutdown signalisation failed
	if (!bShutdownSignaled)
	{
		// Kill the parent process (the apache child terminates when there is no parent)
		// Note: KillProcByPID() correctly fails if passing 0
		if (!::KillProcByPID(dwPid))
			::EnumKillProcByName(MICROAPACHE_FILENAME, TRUE); // kill by name
	}

	// Wait a max of dwTimeoutMs
	do
	{
		if (::EnumKillProcByName(MICROAPACHE_FILENAME) == 0)
			return;
		::Sleep(MICROAPACHE_WAITTIME_MS);
	}
	while ((::GetTickCount() - dwShutdownTimeMs) < dwTimeoutMs);
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
	// Get config file name
	CString sPhpConfigFile = PhpGetConfigFileName();
	if (sPhpConfigFile == _T(""))
		return FALSE;
	CString sPath = ::GetDriveAndDirName(sPhpConfigFile);
	if (!::IsExistingDir(sPath))
	{
		if (!::CreateDir(sPath))
			return FALSE;
	}

	// Convert given data to ANSI
	LPSTR pData = NULL;
	int nLen = ::ToANSI(sConfig, &pData);
	if (nLen <= 0 || !pData)
	{
		if (pData)
			delete [] pData;
		return FALSE;
	}

	// Make a unique temp file on the same volume so that we are sure MoveFileEx() is atomic
	CString sTmpPhpConfigFile(::MakeTempFileName(::GetDriveAndDirName(sPhpConfigFile),
												::GetFileNameNoExt(sPhpConfigFile) + _T(".tmp")));
	
	// Write to temp file and free data
	try
	{
		CFile f(sTmpPhpConfigFile,
				CFile::modeCreate		|
				CFile::modeWrite		|
				CFile::shareDenyWrite);
		f.Write(pData, nLen);
		delete [] pData;
	}
	catch (CFileException* e)
	{
		delete [] pData;
		e->Delete();
		return FALSE;
	}

	// Try moving for two seconds
	BOOL bOK = FALSE;
	int nRetry = 20;
	while (nRetry-- > 0)
	{
		bOK = ::MoveFileEx(	sTmpPhpConfigFile, sPhpConfigFile,
							MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		if (bOK)
			break;
		else
			::Sleep(100);
	}

	// On failure clean-up temp file
	if (!bOK)
		::DeleteFile(sTmpPhpConfigFile);

	return bOK;
}

BOOL CVideoDeviceDoc::PhpConfigFileSetParam(const CString& sParam, const CString& sValue)
{
	// Load Config File
	CString sConfig = PhpLoadConfigFile();

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

	// If not found -> insert after <?php
	int nIndexInsert = sConfigLowerCase.Find(_T("<?php"));
	if (nIndexInsert >= 0)
	{
		nIndexInsert += 5; // Skip <?php
		sConfig.Insert(nIndexInsert, _T("\r\n") + sDefine + _T(" (\"") + sParam + _T("\",\"") + sValue + _T("\");"));
		return PhpSaveConfigFile(sConfig);
	}
	else
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

// Scale a font size starting from a minimum reference
int CVideoDeviceDoc::ScaleFont(	int nWidth, int nHeight,
								int nMinRefFontSize,
								int nMinRefWidth, int nMinRefHeight)
{
	// Check
	if (nMinRefWidth <= 0 || nMinRefHeight <= 0)
		return nMinRefFontSize;

	// Scale
	double dFactorX = (double)nWidth / nMinRefWidth;
	if (dFactorX < 1.0)
		dFactorX = 1.0;
	double dFactorY = (double)nHeight / nMinRefHeight;
	if (dFactorY < 1.0)
		dFactorY = 1.0;
	if (dFactorX > dFactorY)
		return Round(nMinRefFontSize * dFactorX);
	else
		return Round(nMinRefFontSize * dFactorY);
}

void CVideoDeviceDoc::AddFrameTime(CDib* pDib, CTime RefTime, DWORD dwRefUpTime, const CString& sFrameAnnotation, int nRefFontSize, BOOL bShowFrameUptime)
{
	// Check
	if (!pDib)
		return;

	// Calc. date and time
	RefTime = CalcTime(pDib->GetUpTime(), RefTime, dwRefUpTime);

	// Calc. rectangle
	CRect rcRect;
	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = pDib->GetWidth();
	rcRect.bottom = pDib->GetHeight();

	// Create font
	CFont Font;
	int nFontSize = ScaleFont(rcRect.right, rcRect.bottom, nRefFontSize, FRAMETAG_REFWIDTH, FRAMETAG_REFHEIGHT);
	LOGFONT lf = {};
	_tcscpy(lf.lfFaceName, g_szDefaultFontFace);
	lf.lfHeight = -MulDiv(nFontSize, 96, 72); // use 96 and not GetDeviceCaps(hDC, LOGPIXELSY) otherwise it scales with DPI changes!
	lf.lfWeight = FW_NORMAL;
	Font.CreateFontIndirect(&lf);

	// Time text
	CString sTime;
	if (bShowFrameUptime)
	{
		DWORD dwTickCount = pDib->GetUpTime();
		const DWORD dwMillisecondsPerDay = 24U * 60U * 60U * 1000U;
		DWORD dwDays = dwTickCount / dwMillisecondsPerDay;
		dwTickCount = dwTickCount % dwMillisecondsPerDay;
		const DWORD dwMillisecondsPerHour = 60U * 60U * 1000U;
		DWORD dwHours = dwTickCount / dwMillisecondsPerHour;
		dwTickCount = dwTickCount % dwMillisecondsPerHour;
		const DWORD dwMillisecondsPerMin = 60U * 1000U;
		DWORD dwMin = dwTickCount / dwMillisecondsPerMin;
		dwTickCount = dwTickCount % dwMillisecondsPerMin;
		const DWORD dwMillisecondsPerSec = 1000U;
		DWORD dwSec = dwTickCount / dwMillisecondsPerSec;
		dwTickCount = dwTickCount % dwMillisecondsPerSec;
		sTime.Format(_T(" (%u:%02u:%02u:%02u.%03u)"), dwDays, dwHours, dwMin, dwSec, dwTickCount);
		sTime = ::MakeTimeLocalFormat(RefTime, TRUE) + sTime;
	}
	else
		sTime = ::MakeTimeLocalFormat(RefTime, TRUE);
	pDib->AddSingleLineText(sTime,
							rcRect,
							&Font,
							(DT_LEFT | DT_TOP),
							FRAMETIME_COLOR,
							OPAQUE,
							DRAW_BKG_COLOR);

	// Date text
	CString sDate = ::MakeDateLocalFormat(RefTime);
	pDib->AddSingleLineText(sDate,
							rcRect,
							&Font,
							(DT_LEFT | DT_BOTTOM),
							FRAMEDATE_COLOR,
							OPAQUE,
							DRAW_BKG_COLOR);

	// Frame annotation text
	if (!sFrameAnnotation.IsEmpty())
	{
		pDib->AddSingleLineText(sFrameAnnotation,
								rcRect,
								&Font,
								(DT_RIGHT | DT_TOP),
								FRAMEANNOTATION_COLOR,
								OPAQUE,
								DRAW_BKG_COLOR);
	}
}

void CVideoDeviceDoc::AddFrameCount(CDib* pDib, const CString& sCount, int nRefFontSize)
{
	// Check
	if (!pDib)
		return;

	// Calc. rectangle
	CRect rcRect;
	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = pDib->GetWidth();
	rcRect.bottom = pDib->GetHeight();

	// Create font
	CFont Font;
	int nFontSize = ScaleFont(rcRect.right, rcRect.bottom, nRefFontSize, FRAMETAG_REFWIDTH, FRAMETAG_REFHEIGHT);
	LOGFONT lf = {};
	_tcscpy(lf.lfFaceName, g_szDefaultFontFace);
	lf.lfHeight = -MulDiv(nFontSize, 96, 72); // use 96 and not GetDeviceCaps(hDC, LOGPIXELSY) otherwise it scales with DPI changes!
	lf.lfWeight = FW_NORMAL;
	Font.CreateFontIndirect(&lf);

	// Frame count text
	pDib->AddSingleLineText(sCount,
							rcRect,
							&Font,
							(DT_RIGHT | DT_BOTTOM),
							FRAMECOUNT_COLOR,
							OPAQUE,
							DRAW_BKG_COLOR);
}

void CVideoDeviceDoc::AddNoDonationTag(CDib* pDib, int nRefFontSize)
{
	// Check
	if (!pDib)
		return;

	// Calc. rectangle
	CRect rcRect;
	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = pDib->GetWidth();
	rcRect.bottom = pDib->GetHeight();

	// Create font
	CFont Font;
	int nFontSize = ScaleFont(rcRect.right, rcRect.bottom, nRefFontSize, FRAMETAG_REFWIDTH, FRAMETAG_REFHEIGHT);
	LOGFONT lf = {};
	_tcscpy(lf.lfFaceName, g_szDefaultFontFace);
	lf.lfHeight = -MulDiv(nFontSize, 96, 72); // use 96 and not GetDeviceCaps(hDC, LOGPIXELSY) otherwise it scales with DPI changes!
	lf.lfWeight = FW_NORMAL;
	Font.CreateFontIndirect(&lf);

	// No donation text
	CString sNoDonation(ML_STRING(1734, "NO DONATION: see Help menu"));
	if (sNoDonation.GetLength() < 10)
		sNoDonation = _T("NO DONATION: see Help menu");
	pDib->AddSingleLineText(sNoDonation,
							rcRect,
							&Font,
							(DT_RIGHT | DT_BOTTOM),
							NODONATION_MESSAGE_COLOR,
							OPAQUE,
							DRAW_BKG_COLOR);
}

void CVideoDeviceDoc::AddRecSymbol(CDib* pDib, int nRefFontSize)
{
	// Check
	if (!pDib)
		return;

	// Calc. rectangle
	CRect rcRect;
	rcRect.left = 0;
	rcRect.top = 0;
	rcRect.right = pDib->GetWidth();
	rcRect.bottom = pDib->GetHeight();

	// Create font
	CFont Font;
	int nFontSize = ScaleFont(rcRect.right, rcRect.bottom, nRefFontSize, FRAMETAG_REFWIDTH, FRAMETAG_REFHEIGHT);
	LOGFONT lf = {};
	_tcscpy(lf.lfFaceName, g_szDefaultFontFace);
	lf.lfHeight = -MulDiv(nFontSize, 96, 72); // use 96 and not GetDeviceCaps(hDC, LOGPIXELSY) otherwise it scales with DPI changes!
	lf.lfWeight = FW_NORMAL;
	Font.CreateFontIndirect(&lf);

	// Record symbol
	pDib->AddSingleLineText(_T("\u25cf"), // note: if using more than 16 bits, use a uppercase U (for example \U0001F3C3)
							rcRect,
							&Font,
							(DT_RIGHT | DT_BOTTOM),
							REC_MESSAGE_COLOR,
							TRANSPARENT,
							DRAW_BKG_COLOR);
}

// Inspired by https://github.com/Wunkolo/qreverse/
// Note: when migrating to a 64-bit application we can add _byteswap_uint64()
BOOL CVideoDeviceDoc::FlipH(CDib* pDib)
{
	// Check
	if (!pDib || !pDib->GetBMI() || !pDib->GetBits())
		return FALSE;

	BYTE ucSingle;
	WORD wStart, wEnd;
	DWORD dwStart, dwEnd;
	int nCurLine;
	int nWidth = pDib->GetWidth();
	int nWidth2 = nWidth / 2;
	int nWidth4 = nWidth2 / 2;
	int nWidth8 = nWidth4 / 2;
	int nWidth16 = nWidth8 / 2;
	int nHeight = pDib->GetHeight();
	int nHeight2 = nHeight / 2;
	LPBYTE lpBitsY = pDib->GetBits();
	LPBYTE lpBitsU = pDib->GetBits() + nWidth * nHeight;
	LPBYTE lpBitsV = pDib->GetBits() + nWidth * nHeight + nWidth2 * nHeight2;

	// Horizontal Flip Y
	for (nCurLine = 0; nCurLine < nHeight; nCurLine++)
	{
		int i = 0;

		for (int j = i / 4; j < nWidth8; j++) // j = (i / 4) is the DWORD index
		{
			// Get swapped versions of start and end
			dwStart = _byteswap_ulong(*(LPDWORD)&lpBitsY[i]);
			dwEnd = _byteswap_ulong(*(LPDWORD)&lpBitsY[nWidth - i - 4]);

			// Place them at their position
			*(LPDWORD)&lpBitsY[i] = dwEnd;
			*(LPDWORD)&lpBitsY[nWidth - i - 4] = dwStart;

			// Four elements at a time
			i += 4;
		}

		for (int j = i / 2; j < nWidth4; j++) // j = (i / 2) is the WORD index
		{
			// Get swapped versions of start and end
			wStart = _byteswap_ushort(*(LPWORD)&lpBitsY[i]);
			wEnd = _byteswap_ushort(*(LPWORD)&lpBitsY[nWidth - i - 2]);

			// Place them at their position
			*(LPWORD)&lpBitsY[i] = wEnd;
			*(LPWORD)&lpBitsY[nWidth - i - 2] = wStart;

			// Two elements at a time
			i += 2;
		}

		for (; i < nWidth2; i++) // i is the BYTE index
		{
			ucSingle = lpBitsY[i];
			lpBitsY[i] = lpBitsY[nWidth - i - 1];
			lpBitsY[nWidth - i - 1] = ucSingle;
		}

		lpBitsY += nWidth;
	}

	// Horizontal Flip U
	for (nCurLine = 0; nCurLine < nHeight2; nCurLine++)
	{
		int i = 0;

		for (int j = i / 4; j < nWidth16; j++) // j = (i / 4) is the DWORD index
		{
			// Get swapped versions of start and end
			dwStart = _byteswap_ulong(*(LPDWORD)&lpBitsU[i]);
			dwEnd = _byteswap_ulong(*(LPDWORD)&lpBitsU[nWidth2 - i - 4]);

			// Place them at their position
			*(LPDWORD)&lpBitsU[i] = dwEnd;
			*(LPDWORD)&lpBitsU[nWidth2 - i - 4] = dwStart;

			// Four elements at a time
			i += 4;
		}

		for (int j = i / 2; j < nWidth8; j++) // j = (i / 2) is the WORD index
		{
			// Get swapped versions of start and end
			wStart = _byteswap_ushort(*(LPWORD)&lpBitsU[i]);
			wEnd = _byteswap_ushort(*(LPWORD)&lpBitsU[nWidth2 - i - 2]);

			// Place them at their position
			*(LPWORD)&lpBitsU[i] = wEnd;
			*(LPWORD)&lpBitsU[nWidth2 - i - 2] = wStart;

			// Two elements at a time
			i += 2;
		}

		for (; i < nWidth4; i++) // i is the BYTE index
		{
			ucSingle = lpBitsU[i];
			lpBitsU[i] = lpBitsU[nWidth2 - i - 1];
			lpBitsU[nWidth2 - i - 1] = ucSingle;
		}

		lpBitsU += nWidth2;
	}

	// Horizontal Flip V
	for (nCurLine = 0; nCurLine < nHeight2; nCurLine++)
	{
		int i = 0;

		for (int j = i / 4; j < nWidth16; j++) // j = (i / 4) is the DWORD index
		{
			// Get swapped versions of start and end
			dwStart = _byteswap_ulong(*(LPDWORD)&lpBitsV[i]);
			dwEnd = _byteswap_ulong(*(LPDWORD)&lpBitsV[nWidth2 - i - 4]);

			// Place them at their position
			*(LPDWORD)&lpBitsV[i] = dwEnd;
			*(LPDWORD)&lpBitsV[nWidth2 - i - 4] = dwStart;

			// Four elements at a time
			i += 4;
		}

		for (int j = i / 2; j < nWidth8; j++) // j = (i / 2) is the WORD index
		{
			// Get swapped versions of start and end
			wStart = _byteswap_ushort(*(LPWORD)&lpBitsV[i]);
			wEnd = _byteswap_ushort(*(LPWORD)&lpBitsV[nWidth2 - i - 2]);

			// Place them at their position
			*(LPWORD)&lpBitsV[i] = wEnd;
			*(LPWORD)&lpBitsV[nWidth2 - i - 2] = wStart;

			// Two elements at a time
			i += 2;
		}

		for (; i < nWidth4; i++) // i is the BYTE index
		{
			ucSingle = lpBitsV[i];
			lpBitsV[i] = lpBitsV[nWidth2 - i - 1];
			lpBitsV[nWidth2 - i - 1] = ucSingle;
		}

		lpBitsV += nWidth2;
	}

	return TRUE;
}

BOOL CVideoDeviceDoc::FlipV(CDib* pDib)
{
	// Check
	if (!pDib || !pDib->GetBMI() || !pDib->GetBits())
		return FALSE;

	int nCurLine;
	int nWidth = pDib->GetWidth();
	int nWidth2 = nWidth / 2;
	int nHeight = pDib->GetHeight();
	int nHeight2 = nHeight / 2;
	int nHeight4 = nHeight2 / 2;
	LPBYTE lpSrcBitsY = pDib->GetBits();
	LPBYTE lpDstBitsY = lpSrcBitsY + nWidth * (nHeight - 1);
	LPBYTE lpSrcBitsU = pDib->GetBits() + nWidth * nHeight;
	LPBYTE lpDstBitsU = lpSrcBitsU + nWidth2 * (nHeight2 - 1);
	LPBYTE lpSrcBitsV = pDib->GetBits() + nWidth * nHeight + nWidth2 * nHeight2;
	LPBYTE lpDstBitsV = lpSrcBitsV + nWidth2 * (nHeight2 - 1);
	LPBYTE pTemp = new BYTE[nWidth];

	// Vertical Flip Y
	for (nCurLine = 0; nCurLine < nHeight2; nCurLine++)
	{
		memmove(pTemp, lpSrcBitsY, nWidth);
		memmove(lpSrcBitsY, lpDstBitsY, nWidth);
		memmove(lpDstBitsY, pTemp, nWidth);
		lpDstBitsY -= nWidth;
		lpSrcBitsY += nWidth;
	}

	// Vertical Flip U
	for (nCurLine = 0; nCurLine < nHeight4; nCurLine++)
	{
		memmove(pTemp, lpSrcBitsU, nWidth2);
		memmove(lpSrcBitsU, lpDstBitsU, nWidth2);
		memmove(lpDstBitsU, pTemp, nWidth2);
		lpDstBitsU -= nWidth2;
		lpSrcBitsU += nWidth2;
	}

	// Vertical Flip V
	for (nCurLine = 0; nCurLine < nHeight4; nCurLine++)
	{
		memmove(pTemp, lpSrcBitsV, nWidth2);
		memmove(lpSrcBitsV, lpDstBitsV, nWidth2);
		memmove(lpDstBitsV, pTemp, nWidth2);
		lpDstBitsV -= nWidth2;
		lpSrcBitsV += nWidth2;
	}

	// Clean-up
	delete[] pTemp;

	return TRUE;
}

void CVideoDeviceDoc::ProcessOtherFrame(LPBYTE pData, DWORD dwSize)
{	
	// Decode ffmpeg supported formats
	m_pProcessFrameExtraDib->SetBMI((LPBITMAPINFO)&m_ProcessFrameBMI);
	if (m_AVDecoder.Decode(	(LPBITMAPINFO)&m_CaptureBMI,
							pData,
							dwSize,
							m_pProcessFrameExtraDib)) // this function will allocate the dst bits if necessary
	{
		ProcessI420Frame(m_pProcessFrameExtraDib->GetBits(), m_pProcessFrameExtraDib->GetImageSize());
	}
	// In case that avcodec_decode_video2 fails try LoadJPEG
	else if (m_CaptureBMI.bmiHeader.biCompression == FCC('MJPG'))
	{
		if (g_nLogLevel > 0)
			::LogLine(_T("%s, m_AVDecoder.Decode() FAILURE, trying LoadJPEG() + Compress(I420)"), GetAssignedDeviceName());
		if (m_pProcessFrameExtraDib->LoadJPEG(pData, dwSize, 1, TRUE) && m_pProcessFrameExtraDib->Compress(FCC('I420')))
		{
			if (g_nLogLevel > 0)
				::LogLine(_T("%s, LoadJPEG() + Compress(I420) success"), GetAssignedDeviceName());
			ProcessI420Frame(m_pProcessFrameExtraDib->GetBits(), m_pProcessFrameExtraDib->GetImageSize());
		}
		else
		{
			if (g_nLogLevel > 0)
				::LogLine(_T("%s, LoadJPEG() + Compress(I420) FAILURE"), GetAssignedDeviceName());
		}
	}
	// Other formats
	else
	{
		m_pProcessFrameExtraDib->SetBMI((LPBITMAPINFO)&m_CaptureBMI);
		m_pProcessFrameExtraDib->SetBits(pData, dwSize);
		if (m_pProcessFrameExtraDib->IsCompressed())
		{
			if (g_nLogLevel > 0)
				::LogLine(_T("%s, m_AVDecoder.Decode() FAILURE, trying Decompress(32) + Compress(I420)"), GetAssignedDeviceName());
			m_pProcessFrameExtraDib->Decompress(32);
		}
		else
		{
			if (g_nLogLevel > 0)
				::LogLine(_T("%s, m_AVDecoder.Decode() FAILURE, trying Compress(I420)"), GetAssignedDeviceName());
		}
		if (m_pProcessFrameExtraDib->Compress(FCC('I420')))
		{
			if (g_nLogLevel > 0)
				::LogLine(_T("%s, Compress(I420) success"), GetAssignedDeviceName());
			ProcessI420Frame(m_pProcessFrameExtraDib->GetBits(), m_pProcessFrameExtraDib->GetImageSize());
		}
		else
		{
			if (g_nLogLevel > 0)
				::LogLine(_T("%s, Compress(I420) FAILURE"), GetAssignedDeviceName());
		}
	}
}

#define ISALIGNED(x, a) (0==((size_t)(x)&((a)-1)))

/*
NV12 is a YUV 4:2:0 format, in which all Y samples are found first in memory as an array
of unsigned char with an even number of lines. This is followed immediately by an array
of unsigned char containing interleaved U and V samples. NV21 is the same as NV12,
except that U and V samples are swapped.
Note1:
ffmpeg has also NV12 and NV21 decoding, but it crashes for some resolutions
Note2:
When using __asm to write assembly language in C/C++ functions, you don't need to
preserve the EAX, EBX, ECX, EDX, ESI, or EDI registers. However, using these registers
will affect code quality because the register allocator cannot use them to store values
across __asm blocks. The compiler avoids enregistering variables across an __asm block
if the register's contents would be changed by the __asm block. In addition, by using
EBX, ESI or EDI in inline assembly code, you force the compiler to save and restore
those registers in the function prologue and epilogue.
*/
void CVideoDeviceDoc::ProcessNV12Frame(LPBYTE pData, DWORD dwSize, BOOL bFlipUV)
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
		return;
	if (!m_pProcessFrameExtraDib->GetBits())
	{
		if (!m_pProcessFrameExtraDib->AllocateBitsFast(	m_pProcessFrameExtraDib->GetBitCount(),
														m_pProcessFrameExtraDib->GetCompression(),
														m_pProcessFrameExtraDib->GetWidth(),
														m_pProcessFrameExtraDib->GetHeight()))
			return;
	}

	// Init conversion vars
	int width = m_ProcessFrameBMI.bmiHeader.biWidth;
	int height = m_ProcessFrameBMI.bmiHeader.biHeight;
	int halfwidth = width >> 1;
	int halfheight = height >> 1;
	LPBYTE src_y = pData;
	LPBYTE src_uv = src_y + width * height; 
	LPBYTE dst_y = m_pProcessFrameExtraDib->GetBits();
	LPBYTE dst_u;
	LPBYTE dst_v;
	if (bFlipUV)
	{
		dst_v = dst_y + width * height;
		dst_u = dst_v + halfwidth * halfheight;
	}
	else
	{
		dst_u = dst_y + width * height;
		dst_v = dst_u + halfwidth * halfheight;
	}
	int y;

	// Copy Y plane
	memcpy(dst_y, src_y, width * height);

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
			src_uv += width;
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
		}
	}

	// Call Process Frame
	ProcessI420Frame(m_pProcessFrameExtraDib->GetBits(), m_pProcessFrameExtraDib->GetImageSize());
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
		return;
	if (!m_pProcessFrameExtraDib->GetBits())
	{
		if (!m_pProcessFrameExtraDib->AllocateBitsFast(	m_pProcessFrameExtraDib->GetBitCount(),
														m_pProcessFrameExtraDib->GetCompression(),
														m_pProcessFrameExtraDib->GetWidth(),
														m_pProcessFrameExtraDib->GetHeight()))
			return;
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
	BOOL b1SecTick = FALSE;
	if (TimeDiff1.GetTotalSeconds() >= 1 || TimeDiff1.GetTotalSeconds() < 0)
	{
		b1SecTick = TRUE;
		m_1SecTime = CurrentTime;
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
	
	// Motion Detect, Copy, Snapshot and finally Draw
	CDib* pDib = m_pProcessFrameDib;
	if (bDoProcessFrame && pData && dwSize > 0 && pDib &&
		pDib->SetBMI((LPBITMAPINFO)&m_ProcessFrameBMI) &&
		pDib->SetBits(pData, dwSize))
	{
		// Did the video size change?
		if (m_dwLastVideoWidth != pDib->GetWidth() || m_dwLastVideoHeight != pDib->GetHeight())
		{
			m_pCamOffDib->Free();
			m_dwLastVideoWidth = pDib->GetWidth();
			m_dwLastVideoHeight = pDib->GetHeight();
		}

		// Clear the user flag from any previous content
		pDib->SetUserFlag(0);

		// Flip
		if (m_bFlipH)
			FlipH(pDib);
		if (m_bFlipV)
			FlipV(pDib);

		// Obscure the video source
		if (m_bObscureSource)
		{
			// Make frame black
			memset(pDib->GetBits(), 16, pDib->GetWidth() * pDib->GetHeight()); // set Y plane
			memset(pDib->GetBits() + pDib->GetWidth() * pDib->GetHeight(), 0x80, pDib->GetWidth() * pDib->GetHeight() / 2); // set Chroma planes
			
			// Init Dib
			if (!m_pCamOffDib->IsValid())
			{
				m_pCamOffDib->LoadDibSectionRes(GetModuleHandle(NULL), IDB_CAMOFF);
				m_pCamOffDib->ConvertTo(32);
				m_pCamOffDib->StretchBitsFitRect((pDib->GetWidth() / 4) & ~0x1, (pDib->GetHeight() / 4) & ~0x1);
				m_pCamOffDib->Compress(FCC('I420'));
			}

			// Copy Dib bits
			CDib::CopyBits(	FCC('I420'),
							12,
							(pDib->GetWidth() - m_pCamOffDib->GetWidth()) / 2, // dst X
							(pDib->GetHeight() - m_pCamOffDib->GetHeight()) / 2, // dst Y
							0,
							0,
							m_pCamOffDib->GetWidth(),
							m_pCamOffDib->GetHeight(),
							pDib->GetHeight(),
							m_pCamOffDib->GetHeight(),
							pDib->GetBits(),
							m_pCamOffDib->GetBits(),
							pDib->GetWidth(),
							m_pCamOffDib->GetWidth());
		}
		// Draw the privacy mask
		else if (m_bObscureRemovedZones && m_lMovDetTotalZones > 0)
		{
			// Note: m_lMovDetXZonesCount and m_lMovDetYZonesCount will never be set to 0 and will never
			//       be updated (updated in OnThreadSafeInitMovDet) while executing the following code.
			//       When switching resolution and before m_lMovDetXZonesCount and m_lMovDetYZonesCount
			//       get updated the new dib width and height may not be exactly divisible, that's not a
			//       problem because the resulting integer zone width and height are truncated and cannot
			//       overflow lpYPlaneTopLeftBits, lpUPlaneTopLeftBits and lpVPlaneTopLeftBits.
			int nWidth2 = pDib->GetWidth() / 2;
			int nHeight2 = pDib->GetHeight() / 2;
			LPBYTE lpYPlaneTopLeftBits = pDib->GetBits();
			LPBYTE lpUPlaneTopLeftBits = lpYPlaneTopLeftBits + pDib->GetWidth() * pDib->GetHeight();
			LPBYTE lpVPlaneTopLeftBits = lpUPlaneTopLeftBits + nWidth2 * nHeight2;
			int nZoneWidth = pDib->GetWidth() / m_lMovDetXZonesCount;
			int nZoneHeight = pDib->GetHeight() / m_lMovDetYZonesCount;
			int nZoneWidth2 = nZoneWidth / 2;
			int nZoneHeight2 = nZoneHeight / 2;
			for (int y = 0; y < m_lMovDetYZonesCount; y++)
			{
				for (int x = 0; x < m_lMovDetXZonesCount; x++)
				{
					if (m_DoMovementDetection[x + y * m_lMovDetXZonesCount] == 0)
					{
						for (int h = 0; h < nZoneHeight; h++)
							memset(lpYPlaneTopLeftBits + x * nZoneWidth + h * pDib->GetWidth(), 16, nZoneWidth);
						for (int h = 0; h < nZoneHeight2; h++)
						{
							memset(lpUPlaneTopLeftBits + x * nZoneWidth2 + h * nWidth2, 0x80, nZoneWidth2);
							memset(lpVPlaneTopLeftBits + x * nZoneWidth2 + h * nWidth2, 0x80, nZoneWidth2);
						}
					}
				}
				lpYPlaneTopLeftBits += nZoneHeight * pDib->GetWidth();
				lpUPlaneTopLeftBits += nZoneHeight2 * nWidth2;
				lpVPlaneTopLeftBits += nZoneHeight2 * nWidth2;
			}
		}

		// Set the UpTime Var
		pDib->SetUpTime(dwCurrentInitUpTime);

		// Capture audio?
		if (m_bCaptureAudio)
		{
			// Move samples from audio queue to Dib
			::EnterCriticalSection(&m_csAudioList);
			pDib->MoveUserList(m_AudioList);
			::LeaveCriticalSection(&m_csAudioList);

			// Obscure the audio source?
			if (m_bObscureSource)
			{
				POSITION pos = pDib->m_UserList.GetHeadPosition();
				while (pos)
				{
					CUserBuf UserBuf = pDib->m_UserList.GetNext(pos);
					if (UserBuf.m_pBuf && UserBuf.m_dwSize > 0)
					{
						if (m_pSrcWaveFormat->wBitsPerSample == 8)
							memset(UserBuf.m_pBuf, 128, UserBuf.m_dwSize);
						else
							memset(UserBuf.m_pBuf, 0, UserBuf.m_dwSize);
					}
				}
			}
		}
		else
			pDib->FreeUserList();

		// Do Motion Detection Processing
		MovementDetectionProcessing(pDib, CurrentTime, b1SecTick);

		// Copy to Clipboard
		if (m_bDoEditCopy)
			EditCopy(pDib, CurrentTime);

		// Manual Snapshot
		if (m_bDoEditSnapshot)
			EditSnapshot(pDib, CurrentTime);

		// Timed Snapshot
		Snapshot(pDib, CurrentTime);

		// Add Frame Time if User Wants it
		if (m_bShowFrameTime)
			AddFrameTime(pDib, CurrentTime, dwCurrentInitUpTime, m_szFrameAnnotation, m_nRefFontSize, m_bShowFrameUptime);

		// Add "NO DONATION" tag
		if (g_DonorEmailValidateThread.m_bNoDonation)
			AddNoDonationTag(pDib, m_nRefFontSize);

		// Swap Dib pointers, convert to RGB32 and invalidate to draw
		::EnterCriticalSection(&m_csDib);
		m_pProcessFrameDib = m_pDib;
		m_pDib = pDib;
		if (!((CUImagerApp*)::AfxGetApp())->m_bServiceProcess &&
			(!((CUImagerApp*)::AfxGetApp())->m_bTrayIcon || !::AfxGetMainFrame()->m_TrayIcon.IsMinimizedToTray()))
		{
			BITMAPINFO BmiRgb32;
			memset(&BmiRgb32, 0, sizeof(BITMAPINFOHEADER));
			BmiRgb32.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			BmiRgb32.bmiHeader.biWidth = m_pDib->GetWidth();
			BmiRgb32.bmiHeader.biHeight = m_pDib->GetHeight();
			BmiRgb32.bmiHeader.biPlanes = 1;
			BmiRgb32.bmiHeader.biCompression = BI_RGB;
			BmiRgb32.bmiHeader.biBitCount = 32;
			BmiRgb32.bmiHeader.biSizeImage = DWALIGNEDWIDTHBYTES(BmiRgb32.bmiHeader.biWidth * BmiRgb32.bmiHeader.biBitCount) * BmiRgb32.bmiHeader.biHeight;
			m_pDrawDibRGB32->SetBMI(&BmiRgb32);
			m_DrawDecoder.Decode(	m_pDib->GetBMI(),
									m_pDib->GetBits(),
									m_pDib->GetImageSize(),
									m_pDrawDibRGB32);
			::LeaveCriticalSection(&m_csDib);
			::PostMessage(	GetView()->GetSafeHwnd(),
							WM_THREADSAFE_UPDATEWINDOWSIZES,
							(WPARAM)UPDATEWINDOWSIZES_INVALIDATE,
							(LPARAM)0);
		}
		else
			::LeaveCriticalSection(&m_csDib);

		// Set start time, flag, open the Camera Basic Settings dialog and log the starting
		if (!m_bCaptureStarted)
		{
			// Do not invert the order of the following two assignments!
			m_CaptureStartTime = CurrentTime;
			m_bCaptureStarted = TRUE;
			if (m_bDeviceFirstRun)
			{
				AutorunAddDevice(GetDevicePathName());
				::PostMessage(	GetView()->GetSafeHwnd(),
								WM_THREADSAFE_CAPTURECAMERABASICSETTINGS,
								0, 0);
			}
			::LogLine(_T("%s starting, %dx%d, %s"), GetAssignedDeviceName(), m_DocRect.Width(), m_DocRect.Height(), GetDeviceFormat());
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
				if (m_lEffectiveDataRateSum > 0)
				{
					m_lEffectiveDataRate = Round(1000.0 * (double)m_lEffectiveDataRateSum / m_dEffectiveFrameTimeSum);
					m_lEffectiveDataRateSum = 0;
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
				if (m_lEffectiveDataRateSum > 0)
				{
					m_lEffectiveDataRate = Round(1000.0 * (double)m_lEffectiveDataRateSum / m_dEffectiveFrameTimeSum);
					m_lEffectiveDataRateSum = 0;
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
				if (m_lEffectiveDataRateSum > 0)
				{
					m_lEffectiveDataRate = Round(1000.0 * (double)m_lEffectiveDataRateSum / m_dEffectiveFrameTimeSum);
					m_lEffectiveDataRateSum = 0;
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
				if (m_lEffectiveDataRateSum > 0)
				{
					m_lEffectiveDataRate = Round(1000.0 * (double)m_lEffectiveDataRateSum / m_dEffectiveFrameTimeSum);
					m_lEffectiveDataRateSum = 0;
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
				if (m_lEffectiveDataRateSum > 0)
				{
					m_lEffectiveDataRate = Round(1000.0 * (double)m_lEffectiveDataRateSum / m_dEffectiveFrameTimeSum);
					m_lEffectiveDataRateSum = 0;
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
				if (m_lEffectiveDataRateSum > 0)
				{
					m_lEffectiveDataRate = Round(1000.0 * (double)m_lEffectiveDataRateSum / m_dEffectiveFrameTimeSum);
					m_lEffectiveDataRateSum = 0;
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
}

void CVideoDeviceDoc::Snapshot(CDib* pDib, const CTime& Time)
{
	// Snapshot Thread
	if (!m_SaveSnapshotThread.IsAlive())
	{
		// Check the elapsed time to update bDoSnapshot
		BOOL bDoSnapshot = FALSE;
		DWORD dwUpTime = pDib->GetUpTime();
		int nFrameTime = Round(1000.0 / m_dFrameRate);
		if (m_dEffectiveFrameRate > 0.0)
			nFrameTime = Round(1000.0 / m_dEffectiveFrameRate);
		int nSnapshotRateMs = MAX(DEFAULT_SERVERPUSH_POLLRATE_MS, 1000 * m_nSnapshotRate); // Minimum rate is DEFAULT_SERVERPUSH_POLLRATE_MS
		if (nFrameTime >= nSnapshotRateMs)
		{
			m_dwNextSnapshotUpTime = dwUpTime;
			bDoSnapshot = TRUE;
		}
		else
		{
			DWORD dwMaxUpTimeDiff = (DWORD)(3 * nSnapshotRateMs);
			DWORD dwCurrentUpTimeDiff = dwUpTime - m_dwNextSnapshotUpTime;
			DWORD dwCurrentUpTimeDiffInv = m_dwNextSnapshotUpTime - dwUpTime;
			if (dwCurrentUpTimeDiff >= 0x80000000U)
			{
				if (dwCurrentUpTimeDiffInv >= dwMaxUpTimeDiff)	// m_dwNextSnapshotUpTime is too much in the future
					m_dwNextSnapshotUpTime = dwUpTime;			// reset it!
			}
			else
			{
				if (dwCurrentUpTimeDiff >= dwMaxUpTimeDiff)		// m_dwNextSnapshotUpTime is too old
					m_dwNextSnapshotUpTime = dwUpTime;			// reset it!
				else
				{
					m_dwNextSnapshotUpTime += (DWORD)nSnapshotRateMs;
					bDoSnapshot = TRUE;
				}
			}
		}

		// Start Thread?
		if (bDoSnapshot)
		{
			m_SaveSnapshotThread.m_Dib = *pDib;
			m_SaveSnapshotThread.m_Time = Time;
			m_SaveSnapshotThread.Start();
		}
	}

	// Snapshot Video Thread
	if (m_bSnapshotHistoryVideo && !m_SaveSnapshotVideoThread.IsAlive())
	{
		// Yesterday time
		CTime Yesterday = Time - CTimeSpan(1, 0, 0, 0);	// - 1 day
		Yesterday = CTime(	Yesterday.GetYear(),
							Yesterday.GetMonth(),
							Yesterday.GetDay(),
							0, 0, 0);					// Back to midnight

		// Start Thread if not already executed for Yesterday
		if (m_SaveSnapshotVideoThread.m_TaskCompletedForTime < Yesterday)
		{
			m_SaveSnapshotVideoThread.m_Time = Yesterday;
			m_SaveSnapshotVideoThread.Start();
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
		AddFrameTime(&Dib, Time, dwUpTime, m_szFrameAnnotation, m_nRefFontSize, m_bShowFrameUptime);

	// Add "NO DONATION" tag
	if (g_DonorEmailValidateThread.m_bNoDonation)
		AddNoDonationTag(&Dib, m_nRefFontSize);

	// Copy to clipboard
	Dib.EditCopy();

	// Clear flag
	m_bDoEditCopy = FALSE;

	return TRUE;
}

void CVideoDeviceDoc::EditSnapshot(CDib* pDib, const CTime& Time)
{
	// Create directory
	CString sBaseYearMonthDayDir;
	CreateBaseYearMonthDaySubDir(m_sRecordAutoSaveDir, Time, _T(""), sBaseYearMonthDayDir);

	// Make File Name
	CString sFileName;
	if (sBaseYearMonthDayDir.IsEmpty())
		sFileName = _T("manualshot_") + Time.Format(_T("%Y_%m_%d_%H_%M_%S")) + _T(".jpg");
	else
		sFileName = sBaseYearMonthDayDir + _T("\\") + _T("manualshot_") + Time.Format(_T("%Y_%m_%d_%H_%M_%S")) + _T(".jpg");

	// Do not overwrite existing because of below PopupNotificationWnd()
	if (::IsExistingFile(sFileName))
	{
		m_bDoEditSnapshot = FALSE;
		return;
	}

	// Get uptime
	DWORD dwUpTime = pDib->GetUpTime();

	// Dib
	CDib Dib(*pDib);

	// Add frame time
	if (m_bShowFrameTime)
		AddFrameTime(&Dib, Time, dwUpTime, m_szFrameAnnotation, m_nRefFontSize, m_bShowFrameUptime);

	// Add "NO DONATION" tag
	if (g_DonorEmailValidateThread.m_bNoDonation)
		AddNoDonationTag(&Dib, m_nRefFontSize);

	// Save to JPEG File
	CMJPEGEncoder MJPEGEncoder;
	BOOL res = CVideoDeviceDoc::SaveJpegFast(&Dib, &MJPEGEncoder, sFileName, GOOD_SNAPSHOT_COMPR_QUALITY);

	// Clear flag
	m_bDoEditSnapshot = FALSE;

	// Show message
	if (res)
		::AfxGetMainFrame()->PopupNotificationWnd(CString(APPNAME_NOEXT) + _T(" ") + ML_STRING(1849, "Saved"), sFileName);
	else
		::AfxGetMainFrame()->PopupNotificationWnd(APPNAME_NOEXT, ML_STRING(1850, "Save Failed!"), 0);
}

CString CVideoDeviceDoc::SaveJpegRec(CDib* pDib, const CTime& Time)
{
	// Create directory
	CString sBaseYearMonthDayDir;
	CreateBaseYearMonthDaySubDir(m_sRecordAutoSaveDir, Time, _T(""), sBaseYearMonthDayDir);

	// Make File Name
	CString sFileName;
	if (sBaseYearMonthDayDir.IsEmpty())
		sFileName = _T("rec_") + Time.Format(_T("%Y_%m_%d_%H_%M_%S")) + _T(".jpg");
	else
		sFileName = sBaseYearMonthDayDir + _T("\\") + _T("rec_") + Time.Format(_T("%Y_%m_%d_%H_%M_%S")) + _T(".jpg");

	// Do not overwrite previous save, use it.
	// Attention: it's ok to call this function several times per second,
	//            but only from the same thread!
	if (::IsExistingFile(sFileName))
		return sFileName;

	// Get uptime
	DWORD dwUpTime = pDib->GetUpTime();

	// Dib
	CDib Dib(*pDib);

	// Add frame time
	if (m_bShowFrameTime)
		AddFrameTime(&Dib, Time, dwUpTime, m_szFrameAnnotation, m_nRefFontSize, m_bShowFrameUptime);

	// Add "NO DONATION" tag
	if (g_DonorEmailValidateThread.m_bNoDonation)
		AddNoDonationTag(&Dib, m_nRefFontSize);

	// Save to JPEG File
	CMJPEGEncoder MJPEGEncoder;
	if (CVideoDeviceDoc::SaveJpegFast(&Dib, &MJPEGEncoder, sFileName, GOOD_SNAPSHOT_COMPR_QUALITY))
		return sFileName;
	else
		return _T("");
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
// nDetectionLevel	: 1 - 100 (1 - > low movement sensitivity, 100 -> high movement sensitivity)
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

__forceinline void CVideoDeviceDoc::OneEmptyFrameList()
{
	::EnterCriticalSection(&m_csMovementDetectionsList);
	if (m_MovementDetectionsList.IsEmpty())
	{
		CDib::LIST* pNewList = new CDib::LIST;
		if (pNewList)
			m_MovementDetectionsList.AddTail(pNewList);
	}
	else
	{
		while (m_MovementDetectionsList.GetCount() > 1)
		{
			CDib::LIST* pFrameList = m_MovementDetectionsList.GetTail();
			if (pFrameList)
			{
				CDib::FreeList(*pFrameList);
				delete pFrameList;
			}
			m_MovementDetectionsList.RemoveTail();
		}
		CDib::LIST* pFrameList = m_MovementDetectionsList.GetHead();
		if (pFrameList)
			CDib::FreeList(*pFrameList);
	}
	::LeaveCriticalSection(&m_csMovementDetectionsList);
}

__forceinline void CVideoDeviceDoc::ClearMovementDetectionsList()
{
	::EnterCriticalSection(&m_csMovementDetectionsList);
	while (!m_MovementDetectionsList.IsEmpty())
	{
		CDib::LIST* pFrameList = m_MovementDetectionsList.GetTail();
		if (pFrameList)
		{
			CDib::FreeList(*pFrameList);
			delete pFrameList;
		}
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
		if (pFrameList)
		{
			CDib::FreeList(*pFrameList);
			delete pFrameList;
		}
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

__forceinline CDib* CVideoDeviceDoc::AllocDetFrame(CDib* pDib)
{
	CDib* pNewDib = NULL;
	if (pDib)
	{
		pNewDib = new CDib;
		if (pNewDib)
		{
			pNewDib->SetShowMessageBoxOnError(FALSE);
			pNewDib->SetBMI(pDib->GetBMI());			// set BMI	
			pNewDib->SetBits((LPBYTE)pDib->GetBits());	// copy bits
			pNewDib->SetUpTime(pDib->GetUpTime());		// copy frame uptime
			pNewDib->SetUserFlag(pDib->GetUserFlag());	// copy motion, detection sequence start and stop flags
			pNewDib->CopyUserList(pDib->m_UserList);	// copy audio bufs if any
		}
	}
	return pNewDib;
}

__forceinline DWORD CVideoDeviceDoc::AddNewFrameToNewestList(BOOL bMarkStart, CDib* pDib)
{
	DWORD dwError = 0x20100000;	// custom error code
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

				// Allocate the new frame
				CDib* pNewDib = AllocDetFrame(pDib);
				if (pNewDib)
				{
					// Add the new frame
					if ((dwError = pNewDib->BitsToSharedMemory()) == ERROR_SUCCESS)
						pTail->AddTail(pNewDib);
					else
						delete pNewDib;
				}

				// Mark start?
				if (bMarkStart && !pTail->IsEmpty())
				{
					CDib* pHeadDib = pTail->GetHead();
					if (pHeadDib)
					{
						// Mark the frame as being the first frame of the detection sequence
						pHeadDib->SetUserFlag(pHeadDib->GetUserFlag() | FRAME_USER_FLAG_START);
					}
				}
			}
		}
		::LeaveCriticalSection(&m_csMovementDetectionsList);
	}
	return dwError;
}

__forceinline DWORD CVideoDeviceDoc::AddNewFrameToNewestListAndShrink(CDib* pDib)
{
	DWORD dwError = 0x20110000;	// custom error code
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

				// Allocate the new frame
				CDib* pNewDib = AllocDetFrame(pDib);
				if (pNewDib)
				{
					// Add the new frame
					if ((dwError = pNewDib->BitsToSharedMemory()) == ERROR_SUCCESS)
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

					// Free
					if (dwError != ERROR_SUCCESS)
						delete pNewDib;
				}
			}
		}
		::LeaveCriticalSection(&m_csMovementDetectionsList);
	}
	return dwError;
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

__forceinline void CVideoDeviceDoc::SaveFrameList(BOOL bMarkEnd)
{
	::EnterCriticalSection(&m_csMovementDetectionsList);
	if (bMarkEnd && !m_MovementDetectionsList.IsEmpty())
	{
		CDib::LIST* pTail = m_MovementDetectionsList.GetTail();
		if (pTail && !pTail->IsEmpty())
		{
			CDib* pTailDib = pTail->GetTail();
			if (pTailDib)
			{
				// Mark the frame as being the last frame of the detection sequence
				pTailDib->SetUserFlag(pTailDib->GetUserFlag() | FRAME_USER_FLAG_END);
			}
		}
	}
	CDib::LIST* pNewList = new CDib::LIST;
	if (pNewList)
		m_MovementDetectionsList.AddTail(pNewList);
	::LeaveCriticalSection(&m_csMovementDetectionsList);
}

void CVideoDeviceDoc::OnViewFrameAnnotation()
{
	CTextEntryDlg dlg;
	dlg.m_sText = m_szFrameAnnotation;
	if (dlg.Show(::AfxGetMainFrame(), GetView(), ML_STRING(1878, "Annotation")) == IDOK)
	{
		m_szFrameAnnotation[MAX_PATH - 1] = _T('\0');				// first make sure it is NULL terminated
		_tcsncpy(m_szFrameAnnotation, dlg.m_sText, MAX_PATH - 1);	// and then copy a maximum of (MAX_PATH - 1) chars
		::AfxGetApp()->WriteProfileString(GetDevicePathName(), _T("FrameAnnotation"), m_szFrameAnnotation);
		if (!m_bShowFrameTime)
			OnViewFrametime();
	}
}

void CVideoDeviceDoc::OnViewFrametime() 
{
	m_bShowFrameTime = !m_bShowFrameTime;
	::AfxGetApp()->WriteProfileInt(GetDevicePathName(), _T("ShowFrameTime"), m_bShowFrameTime);
}

void CVideoDeviceDoc::OnUpdateViewFrametime(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bShowFrameTime ? 1 : 0);	
}

void CVideoDeviceDoc::OnViewFrameUptime()
{
	m_bShowFrameUptime = !m_bShowFrameUptime;
	::AfxGetApp()->WriteProfileInt(GetDevicePathName(), _T("ShowFrameUptime"), m_bShowFrameUptime);
}

void CVideoDeviceDoc::OnUpdateViewFrameUptime(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bShowFrameUptime ? 1 : 0);
}

void CVideoDeviceDoc::HideDetectionZones()
{
	if (m_nShowEditDetectionZones)
	{
		m_nShowEditDetectionZones = 0;
		GetView()->UpdateCursor();
		GetView()->ForceCursor(FALSE);
		GetView()->Invalidate(FALSE);
		::AfxGetMainFrame()->StatusText();
	}
}

void CVideoDeviceDoc::OnUpdateEditZone(CCmdUI* pCmdUI)
{
	// Note: ON_COMMAND(ID_EDIT_ZONE, OnEditZone) is never called,
	//       CVideoDeviceChildFrame::OnToolbarDropDown() is responsible for the menu popup
	pCmdUI->Enable();
	pCmdUI->SetCheck(m_nShowEditDetectionZones != 0 ? 1 : 0);
}

void CVideoDeviceDoc::OnEditZonesHide()
{
	HideDetectionZones();
}

void CVideoDeviceDoc::OnUpdateEditZonesHide(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nShowEditDetectionZones == 0 ? 1 : 0);
}

void CVideoDeviceDoc::OnEditZoneSensitivity(int nSingleZoneSensitivity)
{
	// Enable Add with nSingleZoneSensitivity
	if (m_nShowEditDetectionZones == 0)
	{
		m_nShowEditDetectionZones = 1;
		GetView()->m_MovDetSingleZoneSensitivity = nSingleZoneSensitivity;
		GetView()->ForceCursor();
		GetView()->Invalidate(FALSE);
		::AfxGetMainFrame()->StatusText(ML_STRING(1483, "*** Ctrl: Draw <-> Erase ***"));
	}
	// Switch to nSingleZoneSensitivity
	else if (m_nShowEditDetectionZones == 1)
	{
		GetView()->m_MovDetSingleZoneSensitivity = nSingleZoneSensitivity;
	}
	// Switch from Remove to Add with nSingleZoneSensitivity
	else if (m_nShowEditDetectionZones == 2)
	{
		m_nShowEditDetectionZones = 1;
		GetView()->m_MovDetSingleZoneSensitivity = nSingleZoneSensitivity;
		GetView()->Invalidate(FALSE);
	}
}

void CVideoDeviceDoc::OnEditZoneSensitivity100()
{
	OnEditZoneSensitivity(1);
}

void CVideoDeviceDoc::OnUpdateEditZoneSensitivity100(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nShowEditDetectionZones == 1 && GetView()->m_MovDetSingleZoneSensitivity == 1 ? 1 : 0);
}

void CVideoDeviceDoc::OnEditZoneSensitivity50()
{
	OnEditZoneSensitivity(2);
}

void CVideoDeviceDoc::OnUpdateEditZoneSensitivity50(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nShowEditDetectionZones == 1 && GetView()->m_MovDetSingleZoneSensitivity == 2 ? 1 : 0);
}

void CVideoDeviceDoc::OnEditZoneSensitivity25()
{
	OnEditZoneSensitivity(4);
}

void CVideoDeviceDoc::OnUpdateEditZoneSensitivity25(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nShowEditDetectionZones == 1 && GetView()->m_MovDetSingleZoneSensitivity == 4 ? 1 : 0);
}

void CVideoDeviceDoc::OnEditZoneSensitivity10()
{
	OnEditZoneSensitivity(10);
}

void CVideoDeviceDoc::OnUpdateEditZoneSensitivity10(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nShowEditDetectionZones == 1 && GetView()->m_MovDetSingleZoneSensitivity == 10 ? 1 : 0);
}

void CVideoDeviceDoc::OnEditZoneSensitivity5()
{
	OnEditZoneSensitivity(20);
}

void CVideoDeviceDoc::OnUpdateEditZoneSensitivity5(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nShowEditDetectionZones == 1 && GetView()->m_MovDetSingleZoneSensitivity == 20 ? 1 : 0);
}

void CVideoDeviceDoc::OnEditZoneRemove()
{
	// Enable Remove
	if (m_nShowEditDetectionZones == 0)
	{
		m_nShowEditDetectionZones = 2;
		GetView()->ForceCursor();
		GetView()->Invalidate(FALSE);
		::AfxGetMainFrame()->StatusText(ML_STRING(1483, "*** Ctrl: Draw <-> Erase ***"));
	}
	// Switch from Add to Remove
	else if (m_nShowEditDetectionZones == 1)
	{
		m_nShowEditDetectionZones = 2;
		GetView()->Invalidate(FALSE);
	}
}

void CVideoDeviceDoc::OnUpdateEditZoneRemove(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nShowEditDetectionZones == 2 ? 1 : 0);
}

void CVideoDeviceDoc::OnEditZoneObscureRemoved()
{
	m_bObscureRemovedZones = !m_bObscureRemovedZones;
	::AfxGetApp()->WriteProfileInt(GetDevicePathName(), _T("ObscureRemovedZones"), m_bObscureRemovedZones);
}

void CVideoDeviceDoc::OnUpdateEditZoneObscureRemoved(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bObscureRemovedZones ? 1 : 0);
}

void CVideoDeviceDoc::OnEditZoneBig()
{
	m_nDetectionZoneSize = 0; // changing this triggers OnThreadSafeInitMovDet() which calls WriteProfileInt()
}

void CVideoDeviceDoc::OnUpdateEditZoneBig(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nDetectionZoneSize == 0 ? 1 : 0);
}

void CVideoDeviceDoc::OnEditZoneMedium()
{
	m_nDetectionZoneSize = 1; // changing this triggers OnThreadSafeInitMovDet() which calls WriteProfileInt()
}

void CVideoDeviceDoc::OnUpdateEditZoneMedium(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nDetectionZoneSize == 1 ? 1 : 0);
}

void CVideoDeviceDoc::OnEditZoneSmall()
{
	m_nDetectionZoneSize = 2; // changing this triggers OnThreadSafeInitMovDet() which calls WriteProfileInt()
}

void CVideoDeviceDoc::OnUpdateEditZoneSmall(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nDetectionZoneSize == 2 ? 1 : 0);
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

void CVideoDeviceDoc::OnViewWeb()
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
		BeginWaitCursor();
		::ShellExecute(NULL, _T("open"), sUrl, NULL, NULL, SW_SHOWNORMAL);
		EndWaitCursor();
	}
}

void CVideoDeviceDoc::OnViewFiles()
{
	CString sBaseYearMonthDayDir;
	CreateBaseYearMonthDaySubDir(m_sRecordAutoSaveDir, CTime::GetCurrentTime(), _T(""), sBaseYearMonthDayDir);
	::ShellExecute(NULL, _T("open"), sBaseYearMonthDayDir, NULL, NULL, SW_SHOWNORMAL);
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
	GetFrame()->PostMessage(WM_CLOSE, 0, 0);
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


PIXORD
------

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


FOSCAM
------

JPEG
_T("GET /snapshot.cgi?user=<user>&pwd=<password>&resolution=<resolution> HTTP/1.1\r\n")

MJPEG
_T("GET /videostream.cgi?user=<user>&pwd=<password>&resolution=<resolution>&rate=<rate> HTTP/1.1\r\n")

resolutions:
8  -> 320*240
32 -> 640*480

rates (value range 0-23):
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
void CVideoDeviceDoc::ConnectHttp()
{
	ASSERT(m_pVideoNetCom);
	ASSERT(m_pHttpVideoParseProcess);

	// Init Video
	switch (m_nNetworkDeviceTypeMode)
	{
		case GENERIC_SP :	// Generic HTTP motion jpeg devices
		case GENERIC_CP :	// Generic HTTP jpeg snapshots devices
			// Format not yet known because there are ambivalent connection strings in m_HttpGetFrameLocations
			m_pHttpVideoParseProcess->m_FormatType = CHttpParseProcess::FORMATVIDEO_UNKNOWN;
			m_nHttpGetFrameLocationPos = 0;
			break;

		case AXIS_SP :		// Axis HTTP motion jpeg
			m_pHttpVideoParseProcess->m_bQueryVideoProperties = TRUE;
			m_pHttpVideoParseProcess->m_FormatType = CHttpParseProcess::FORMATVIDEO_MJPEG;
			break;

		case AXIS_CP :		// Axis HTTP jpeg snapshots
			m_pHttpVideoParseProcess->m_bQueryVideoProperties = TRUE;
			m_pHttpVideoParseProcess->m_FormatType = CHttpParseProcess::FORMATVIDEO_JPEG;
			break;

		case PANASONIC_SP :	// Panasonic HTTP motion jpeg
			m_pHttpVideoParseProcess->m_FormatType = CHttpParseProcess::FORMATVIDEO_MJPEG;
			m_pHttpVideoParseProcess->m_Sizes.RemoveAll();			
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(160, 120));
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(192, 144));
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(320, 240));
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(640, 480));
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(1280, 1024)); // Support models such as BB-HCM515
			break;

		case PANASONIC_CP :	// Panasonic HTTP jpeg snapshots
			m_pHttpVideoParseProcess->m_FormatType = CHttpParseProcess::FORMATVIDEO_JPEG;
			m_pHttpVideoParseProcess->m_Sizes.RemoveAll();			
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(160, 120));
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(192, 144));
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(320, 240));
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(640, 480));
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(1280, 1024)); // Support models such as BB-HCM515
			break;

		case PIXORD_SP :	// Pixord HTTP motion jpeg
			m_pHttpVideoParseProcess->m_FormatType = CHttpParseProcess::FORMATVIDEO_MJPEG;
			m_pHttpVideoParseProcess->m_Sizes.RemoveAll();			
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(176, 112));
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(352, 240));
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(704, 480));
			break;

		case PIXORD_CP :	// Pixord HTTP jpeg snapshots
			m_pHttpVideoParseProcess->m_FormatType = CHttpParseProcess::FORMATVIDEO_JPEG;
			m_pHttpVideoParseProcess->m_Sizes.RemoveAll();			
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(176, 112));
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(352, 240));
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(704, 480));
			break;

		case EDIMAX_SP :	// Edimax HTTP motion jpeg
			m_pHttpVideoParseProcess->m_bQueryVideoProperties = TRUE;
			m_pHttpVideoParseProcess->m_bSetVideoResolution = TRUE;
			m_pHttpVideoParseProcess->m_bSetVideoCompression = TRUE;
			m_pHttpVideoParseProcess->m_bSetVideoFramerate = TRUE;
			m_pHttpVideoParseProcess->m_FormatType = CHttpParseProcess::FORMATVIDEO_MJPEG;
			break;

		case EDIMAX_CP :	// Edimax HTTP jpeg snapshots
			m_pHttpVideoParseProcess->m_bQueryVideoProperties = TRUE;
			m_pHttpVideoParseProcess->m_bSetVideoResolution = TRUE;
			m_pHttpVideoParseProcess->m_bSetVideoCompression = TRUE;
			m_pHttpVideoParseProcess->m_FormatType = CHttpParseProcess::FORMATVIDEO_JPEG;
			break;

		case TPLINK_SP :	// TP-Link HTTP motion jpeg
			m_pHttpVideoParseProcess->m_FormatType = CHttpParseProcess::FORMATVIDEO_MJPEG;
			break;

		case TPLINK_CP :	// TP-Link HTTP jpeg snapshots
			m_pHttpVideoParseProcess->m_FormatType = CHttpParseProcess::FORMATVIDEO_JPEG;
			break;

		case FOSCAM_SP :	// Foscam HTTP motion jpeg
			m_pHttpVideoParseProcess->m_FormatType = CHttpParseProcess::FORMATVIDEO_MJPEG;
			m_pHttpVideoParseProcess->m_Sizes.RemoveAll();			
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(320, 240));
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(640, 480));
			break;

		case FOSCAM_CP :	// Foscam HTTP jpeg snapshots
			m_pHttpVideoParseProcess->m_FormatType = CHttpParseProcess::FORMATVIDEO_JPEG;
			m_pHttpVideoParseProcess->m_Sizes.RemoveAll();			
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(320, 240));
			m_pHttpVideoParseProcess->m_Sizes.Add(CSize(640, 480));
			break;

		default :
			ASSERT(FALSE);
			break;
	}

	// Start Http Thread
	m_HttpThread.Start();

	// Connect video
	m_HttpThread.SetEventVideoConnect();
}

void CVideoDeviceDoc::ConnectRtsp()
{
	// Prepare path and optional query
	CString sPathAndQuery;
	BOOL bCredentialAlreadyAdded = FALSE;
	switch (m_nNetworkDeviceTypeMode)
	{
		case URL_RTSP:				
			sPathAndQuery = m_HttpGetFrameLocations[0];
			break;

		case GENERIC_1_RTSP:		// Ctronics, Escam, FDT, HooToo, IdeaNext, INSTAR, KKmoon, Microseven, SV3C, Wansview, WBox
		case WANSCAM_RTSP:
			sPathAndQuery = _T("/11");
			break;

		case GENERIC_2_RTSP:		// Escam, iZtouch, Techege
		case DIGOO_RTSP:
		case HIKAM_RTSP:
		case SRICAM_RTSP:			
			sPathAndQuery = _T("/onvif1");
			break;

		case GENERIC_3_RTSP:
		case GEMBIRD_RTSP:
		case KUCAM_RTSP:
		case NEXGADGET_RTSP:
		case UOKOO_RTSP:
		case WANSVIEW_RTSP:			
			sPathAndQuery = _T("/live/ch0");
			break;

		case GENERIC_4_RTSP:		// Chinavasion, Cotier, Ctronics, Escam, Xvision
		case FALCONEYE_RTSP:		
			sPathAndQuery = m_bPreferTcpforRtsp ? _T("/ch01.264?ptype=tcp") : _T("/ch01.264?ptype=udp");
			break;

		case GENERIC_5_RTSP:		// Note: it is legal to have ampersands and equal signs in the path part of an url
			sPathAndQuery = CString(_T("/user=")) + (m_sHttpGetFrameUsername.IsEmpty() ? _T("admin") : ::UrlEncode(m_sHttpGetFrameUsername, TRUE)) +
							_T("&password=") + ::UrlEncode(m_sHttpGetFramePassword, TRUE) + 
							_T("&channel=1&stream=0.sdp");
			bCredentialAlreadyAdded = TRUE;
			break;

		case GENERIC_6_RTSP:		// Note: it is legal to have equal signs in the path part of an url
			sPathAndQuery = CString(_T("/user=")) + (m_sHttpGetFrameUsername.IsEmpty() ? _T("admin") : ::UrlEncode(m_sHttpGetFrameUsername, TRUE)) +
							_T("_password=") + ::UrlEncode(m_sHttpGetFramePassword, TRUE) +
							_T("_channel=1_stream=0.sdp");
			bCredentialAlreadyAdded = TRUE;
			break;

		case SEVENLINKS_RTSP:
		case FOSCAM_RTSP:			
			sPathAndQuery = _T("/videoMain");
			break;

		case ABUS_RTSP:
		case TPLINK_RTSP:			
			sPathAndQuery = _T("/video.mp4");
			break;

		case ACTI_RTSP:
		case BOSCH_RTSP:			
			sPathAndQuery = _T("/h264");
			break;

		case AMCREST_RTSP:			
			sPathAndQuery = _T("/cam/realmonitor?channel=1&subtype=0");
			break;

		case ARECONT_RTSP:			
			sPathAndQuery = _T("/h264.sdp");
			break;

		case AXIS_RTSP:				
			sPathAndQuery = _T("/axis-media/media.amp");
			break;

		case CANON_RTSP:			
			sPathAndQuery = _T("/stream/profile0=r");
			break;

		case DLINK_LIVE1_RTSP:		
			sPathAndQuery = _T("/live1.sdp");
			break;

		case DLINK_PLAY1_RTSP:		
			sPathAndQuery = _T("/play1.sdp");
			break;

		case DAHUA_RTSP:			
			sPathAndQuery = _T("/live");
			break;

		case EDIMAX_H264_RTSP:		
			sPathAndQuery = _T("/ipcam_h264.sdp");
			break;

		case EDIMAX_MPEG4_RTSP:		
			sPathAndQuery = _T("/ipcam.sdp");
			break;

		case FLOUREON_RTSP:
		case JIDETECH_RTSP:
			sPathAndQuery = _T("/1/h264major");
			break;

		case GEOVISION_RTSP:		
			sPathAndQuery = _T("/CH001.sdp");
			break;

		case HIKVISION_RTSP:
		case TRENDNET_RTSP:			
			sPathAndQuery = _T("/Streaming/Channels/1");
			break;

		// av_h264_jpeg_ulaw.sdp will serve H.264 if OpenH264 has been downloaded
		// in the IP Webcam (Pro) for Android App and if not it will serve MJPEG
		// (jpeg_ulaw.sdp will server MJPEG only and h264_ulaw.sdp H.264 only)
		case IPWEBCAM_ANDROID_RTSP:	
			sPathAndQuery = _T("/av_h264_jpeg_ulaw.sdp");
			break;

		case LINKSYS_RTSP:			
			sPathAndQuery = _T("/img/media.sav");
			break;

		case LOGITECH_RTSP:			
			sPathAndQuery = _T("/HighResolutionVideo");
			break;

		case MONACOR_RTSP:			
			sPathAndQuery = _T("/ch00/0");
			break;

		case MONACOR_2MP_RTSP:		
			sPathAndQuery = _T("/0");
			break;

		case PANASONIC_RTSP:		
			sPathAndQuery = _T("/MediaInput/h264");
			break;

		case PIXORD_RTSP:			
			sPathAndQuery = _T("/v00");
			break;

		case PLANET_RTSP:			
			sPathAndQuery = _T("/stream1");
			break;

		case REOLINK_RTSP:			
			sPathAndQuery = _T("/h264Preview_01_main");
			break;

		case SAMSUNG_RTSP:			
			sPathAndQuery = _T("/profile1/media.smp");
			break;

		case SONY_RTSP:				
			sPathAndQuery = _T("/media/video1");
			break;

		case SUMPPLE_RTSP:
			sPathAndQuery = CString(_T("/live/av0?user=")) + (m_sHttpGetFrameUsername.IsEmpty() ? _T("admin") : ::UrlEncode(m_sHttpGetFrameUsername, TRUE)) +
							_T("&passwd=") + (m_sHttpGetFramePassword.IsEmpty() ? _T("sumpple") : ::UrlEncode(m_sHttpGetFramePassword, TRUE));
			bCredentialAlreadyAdded = TRUE;
			break;

		case TOSHIBA_RTSP:
		case VIVOTEK_RTSP:			
			sPathAndQuery = _T("/live.sdp");
			break;

		case UBIQUITI_RTSP:			
			sPathAndQuery = _T("/live/ch00_0");
			break;
		
		case VSTARCAM_RTSP:
		case ZMODO_RTSP:			
			sPathAndQuery = m_bPreferTcpforRtsp ? _T("/tcp/av0_0") : _T("/udp/av0_0");
			break;

		case XIAOMI_RTSP:			
			sPathAndQuery = _T("/ch0_0.h264");
			break;

		case YCAM_RTSP:				
			sPathAndQuery = _T("/live_mpeg4.sdp");
			break;

		case ZAVIO_RTSP:			
			sPathAndQuery = _T("/video.pro1");
			break;
		
		default:					
			ASSERT(FALSE);
			break;
	}

	// Start thread with given url
	CString sHost(m_sGetFrameVideoHost);
	if ((sHost.Find(_T(':'))) >= 0) // IPv6?
		sHost = _T("[") + sHost + _T("]");
	if (bCredentialAlreadyAdded || (m_sHttpGetFrameUsername.IsEmpty() && m_sHttpGetFramePassword.IsEmpty()))
	{
		m_RtspThread.m_sURL.Format(_T("rtsp://%s:%d%s"), sHost, m_nGetFrameVideoPort, sPathAndQuery);
	}
	else
	{
		m_RtspThread.m_sURL.Format(_T("rtsp://%s:%s@%s:%d%s"),	::UrlEncode(m_sHttpGetFrameUsername, TRUE),
																::UrlEncode(m_sHttpGetFramePassword, TRUE),
																sHost, m_nGetFrameVideoPort, sPathAndQuery);
	}
	if (g_nLogLevel > 1)
		::LogLine(_T("%s, rtsp://%s:%d%s"), GetAssignedDeviceName(), sHost, m_nGetFrameVideoPort, sPathAndQuery);
	m_RtspThread.Start();
}

BOOL CVideoDeviceDoc::CHttpParseProcess::SendRawRequest(CString sRequest)
{
	// Store last request untouched (before url decoding + encoding)
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

			// Do not encode reserved chars like '?' or '&' or '=' used by uri parameters or '[' and ']'
			// found in HTTP_USERNAME_PLACEHOLDER or HTTP_PASSWORD_PLACEHOLDER and replaced below.
			// ATTENTION: never provide credentials directly in url, use the placeholders so that the
			//            url encoding of the credentials can be performed correctly below!
			sUri = ::UrlEncode(sUri, FALSE);

			// Replace uri parameters placeholders with fully url encoded username and password
			sUri.Replace(HTTP_USERNAME_PLACEHOLDER, ::UrlEncode(m_pDoc->m_sHttpGetFrameUsername, TRUE));
			sUri.Replace(HTTP_PASSWORD_PLACEHOLDER, ::UrlEncode(m_pDoc->m_sHttpGetFramePassword, TRUE));

			// Make request
			sRequest = sMethod + _T(" ") + sUri + sRequest.Mid(nPosEnd);
		}
	}

	// Host
	CString sHost;
	if (m_pDoc->m_nGetFrameVideoPort != 80)
		sHost.Format(_T("Host: %s:%d\r\n"), m_pDoc->m_sGetFrameVideoHost, m_pDoc->m_nGetFrameVideoPort);
	else
		sHost.Format(_T("Host: %s\r\n"), m_pDoc->m_sGetFrameVideoHost);

	CString sMsg;
	if (m_AnswerAuthorizationType == AUTHBASIC) // http://tools.ietf.org/html/rfc2617
	{
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
		// run out of buffer or do not parse well if we send too much!
		sMsg.Format(_T("User-Agent: %s/%s\r\n")
					_T("Connection: keep-alive\r\n")
					_T("Authorization: Basic %s\r\n\r\n"),
					APPNAME_NOEXT,
					APPVERSION,
					lpszAuthorization);
	}
	else if (m_AnswerAuthorizationType == AUTHDIGEST) // http://tools.ietf.org/html/rfc2617
	{
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
		CMD5 hmac;
		CMD5Hash hash;

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

		// We can reuse the server nonce value (the server only issues a new nonce for each "401" response),
		// but for each request we must provide a new cnonce and increment the request counter (nc).
		// The server may remember when each nonce value was issued, expiring them after a certain amount of
		// time by sending a "401" status code, when that happens we must responde with the new nonce.

		// With Qop, nc and cnonce
		if (bQop)
		{
			m_dwCNonceCount++;
			CString sCNonceCount;
			sCNonceCount.Format(_T("%08x"), m_dwCNonceCount);
			std::random_device TrueRandom; // non-deterministic generator implemented as crypto-secure in Visual C++
			DWORD dwCNonce = TrueRandom(); // returns value in the range [0, 0xFFFFFFFF]
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
			// run out of buffer or do not parse well if we send too much!
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
			// run out of buffer or do not parse well if we send too much!
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
		// Keep it short because some stupid ip cams (like Planet)
		// run out of buffer or do not parse well if we send too much!
		sMsg.Format(_T("User-Agent: %s/%s\r\n")
					_T("Connection: keep-alive\r\n\r\n"),
					APPNAME_NOEXT,
					APPVERSION);
	}

	// Send
	if (g_nLogLevel > 1)
		::LogLine(_T("%s, %s"), m_pDoc->GetAssignedDeviceName(), ::SingleLine(sRequest + sHost + sMsg));
	return (m_pNetCom->WriteStr(sRequest + sHost + sMsg) > 0);
}

BOOL CVideoDeviceDoc::CHttpParseProcess::SendRequest()
{
	CString sLocation;
	CString sRequest;

	::EnterCriticalSection(&m_pDoc->m_csHttpParams);
	switch (m_pDoc->m_nNetworkDeviceTypeMode)
	{
		case GENERIC_SP :	// Generic HTTP motion jpeg devices
		case GENERIC_CP :	// Generic HTTP jpeg snapshots devices
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
		case AXIS_SP :		// Axis HTTP motion jpeg
		{
			if (m_bQueryVideoProperties)
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
		case AXIS_CP :		// Axis HTTP jpeg snapshots
		{
			if (m_bQueryVideoProperties)
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
		case PANASONIC_SP :	// Panasonic HTTP motion jpeg
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
		case PANASONIC_CP :	// Panasonic HTTP jpeg snapshots
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
		case PIXORD_SP :	// Pixord HTTP motion jpeg
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
		case PIXORD_CP :	// Pixord HTTP jpeg snapshots
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
		case EDIMAX_SP :	// Edimax HTTP motion jpeg
		{
			if (m_bQueryVideoProperties)
			{
				sLocation = _T("/camera-cgi/admin/param.cgi?action=list&group=Properties.Image.I0");
				sRequest.Format(_T("GET %s HTTP/%s\r\n"),
								sLocation,
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			else if (m_bSetVideoResolution)
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
									HTTP_DEFAULT_VIDEO_SIZE_CX,
									HTTP_DEFAULT_VIDEO_SIZE_CY,
									m_bOldVersion ? _T("1.0") : _T("1.1"));
					m_pDoc->m_nHttpVideoSizeX = HTTP_DEFAULT_VIDEO_SIZE_CX;
					m_pDoc->m_nHttpVideoSizeY = HTTP_DEFAULT_VIDEO_SIZE_CY;
				}
			}
			else if (m_bSetVideoCompression)
			{
				sLocation = _T("/camera-cgi/admin/param.cgi");
				sRequest.Format(_T("GET %s?action=update&Image.I0.Appearance.Compression=%d HTTP/%s\r\n"),
								sLocation,
								(100 - m_pDoc->m_nHttpVideoQuality) / 25, // value range is 0-4
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			else if (m_bSetVideoFramerate)
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
		case EDIMAX_CP :	// Edimax HTTP jpeg snapshots
		{
			if (m_bQueryVideoProperties)
			{
				sLocation = _T("/camera-cgi/admin/param.cgi?action=list&group=Properties.Image.I0");
				sRequest.Format(_T("GET %s HTTP/%s\r\n"),
								sLocation,
								m_bOldVersion ? _T("1.0") : _T("1.1"));
			}
			else if (m_bSetVideoResolution)
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
									HTTP_DEFAULT_VIDEO_SIZE_CX,
									HTTP_DEFAULT_VIDEO_SIZE_CY,
									m_bOldVersion ? _T("1.0") : _T("1.1"));
					m_pDoc->m_nHttpVideoSizeX = HTTP_DEFAULT_VIDEO_SIZE_CX;
					m_pDoc->m_nHttpVideoSizeY = HTTP_DEFAULT_VIDEO_SIZE_CY;
				}
			}
			else if (m_bSetVideoCompression)
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
		case TPLINK_SP :	// TP-Link HTTP motion jpeg
		{
			sLocation = _T("/video.mjpg");
			sRequest.Format(_T("GET %s HTTP/%s\r\n"),
						sLocation,
						m_bOldVersion ? _T("1.0") : _T("1.1"));
			break;
		}
		case TPLINK_CP :	// TP-Link HTTP jpeg snapshots
		{
			sLocation = _T("/jpg/image.jpg");
			sRequest.Format(_T("GET %s HTTP/%s\r\n"),
						sLocation,
						m_bOldVersion ? _T("1.0") : _T("1.1"));
			break;
		}
		case FOSCAM_SP :	// Foscam HTTP motion jpeg
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
						_T("?user=") + HTTP_USERNAME_PLACEHOLDER +
						_T("&pwd=") + HTTP_PASSWORD_PLACEHOLDER;
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
		case FOSCAM_CP :	// Foscam HTTP jpeg snapshots
		{
			sLocation = CString(_T("/snapshot.cgi")) +
						_T("?user=") + HTTP_USERNAME_PLACEHOLDER +
						_T("&pwd=") + HTTP_PASSWORD_PLACEHOLDER;
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

__forceinline int CVideoDeviceDoc::CHttpParseProcess::FindMultipartBoundary(int nPos,
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

__forceinline int CVideoDeviceDoc::CHttpParseProcess::FindSOI(	int nPos,
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

__forceinline int CVideoDeviceDoc::CHttpParseProcess::FindEndOfLine(const CString& s, int nStart)
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

/*
Content-Type: multipart/x-mixed-replace; boundary="FrameBoundary"<CRLF>
<CRLF>
--FrameBoundary<CRLF>
Content-Type: image/jpeg<CRLF>
Content-Length: 12436<CRLF>
<CRLF>
<jpeg bytes>
<CRLF>
--FrameBoundary<CRLF>
Content-Type: image/jpeg<CRLF>
Content-Length: 21436<CRLF>
<CRLF>
<jpeg bytes>
<CRLF>
--FrameBoundary<CRLF>
...

Note: the optional double-quotes around the boundary delimiter parameter are necessary
      if using special characters, like a column for example
Attention: some servers erroneously set boundary=--FrameBoundary (with the two hyphens)

https://tools.ietf.org/html/rfc2046#section-5.1.1
https://tools.ietf.org/html/rfc822
*/
BOOL CVideoDeviceDoc::CHttpParseProcess::ParseMultipart(CNetCom* pNetCom,
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

		// Content Length
		if ((nPos = sMsgLowerCase.Find(_T("content-length:"), nPos)) < 0)
		{
			if (nSize > HTTP_MIN_MULTIPART_SIZE)
				m_bMultipartNoLength = TRUE;
			return FALSE;
		}
		nPos += 15;
		if ((nPosEnd = FindEndOfLine(sMsg, nPos)) < 0)
			return FALSE;
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
		// Remove unknown stream
		if (sContentType != _T("") && sContentType.Find(_T("image/jpeg")) < 0)
		{
			pNetCom->Read(NULL, m_nProcessOffset + m_nProcessSize);
			return FALSE;
		}
		else
		{
			m_FormatType = FORMATVIDEO_MJPEG;
			return TRUE;
		}
	}
	else
		return FALSE;
}

BOOL CVideoDeviceDoc::CHttpParseProcess::ParseSingle(	BOOL bLastCall,
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
		m_FormatType = FORMATVIDEO_JPEG;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CVideoDeviceDoc::CHttpParseProcess::HasResolution(const CSize& Size)
{
	for (int nSize = 0 ; nSize < m_Sizes.GetSize() ; nSize++)
	{
		if (Size == m_Sizes[nSize])
			return TRUE;
	}
	return FALSE;
}

/*
HTTP Header Field
-----------------
field-name ":" OWS field-value
- OWS is optional white spaces
- No whitespaces are allowed between the field-name and the colon
- Reference: https://greenbytes.de/tech/webdav/draft-ietf-httpbis-p1-messaging-21.html#rfc.section.3.2
*/
int CVideoDeviceDoc::CHttpParseProcess::FindHttpHeader(	const CString& sFieldNameLowerCase,
														const CString& sMsgLowerCase)
{
	int nOffset = sMsgLowerCase.Find(sFieldNameLowerCase + _T(":"));
	if (nOffset >= 0)
	{
		// Skip header name and colon
		nOffset += sFieldNameLowerCase.GetLength() + 1;

		// Skip optional spaces
		int nLenght = sMsgLowerCase.GetLength();
		while (nOffset < nLenght && _istspace(sMsgLowerCase[nOffset]))
			nOffset++;

		// Return beginning of header value
		return nOffset;
	}
	else
		return -1;
}
BOOL CVideoDeviceDoc::CHttpParseProcess::CheckHttpHeaderValue(	const CString& sFieldValueLowerCase,
																int nOffset,
																const CString& sMsgLowerCase)
{
	// Check
	if (nOffset < 0)
		return FALSE;

	// Compare
	int nLenght = sMsgLowerCase.GetLength();
	int nValueLength = sFieldValueLowerCase.GetLength();
	if (nLenght - nOffset >= nValueLength &&
		wmemcmp((LPCWSTR)sMsgLowerCase + nOffset, (LPCWSTR)sFieldValueLowerCase, nValueLength) == 0) // correctly returns 0 in case that nValueLength is 0
		return TRUE;
	else
		return FALSE;
}

/*
HTTP defines the sequence CRLF as the end-of-line marker
for all protocol elements except the Entity-Body.
The end-of-line marker within an Entity-Body is defined by
its associated media type.
The end of the header fields is indicated by an empty field,
resulting in the transmission of two consecutive CRLF pairs.
Attention 1: Using LFLF instead of CRLFCRLF violates RFC 2616
             but must be supported!
Attention 2: I have seen unauthorized (401) replies with only
             one CRLF ending pair!
http://tools.ietf.org/html/rfc2616
*/
BOOL CVideoDeviceDoc::CHttpParseProcess::Parse(CNetCom* pNetCom, BOOL bLastCall)
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
			if (CheckHttpHeaderValue(_T("keep-alive"), FindHttpHeader(_T("connection"), sMsgLowerCase), sMsgLowerCase))
				m_bConnectionKeepAlive = TRUE;
			else
				m_bConnectionKeepAlive = FALSE;
		}
		else
		{
			if (CheckHttpHeaderValue(_T("close"), FindHttpHeader(_T("connection"), sMsgLowerCase), sMsgLowerCase))
				m_bConnectionKeepAlive = FALSE;
			else	
				m_bConnectionKeepAlive = TRUE;
		}

		// OK
		if (sCode == _T("200"))
		{
			// Set flag
			m_bAuthorized = TRUE;

			// Find content-type
			if ((nPos = FindHttpHeader(_T("content-type"), sMsgLowerCase)) < 0)
			{
				delete[] pMsg;
				return FALSE; // Do not call Processor
			}

			// Multipart image
			if (CheckHttpHeaderValue(_T("multipart/x-mixed-replace"), nPos, sMsgLowerCase))
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

				// Flag
				m_bFirstProcessing = TRUE;

				// Call mjpeg parser
				res = ParseMultipart(pNetCom, nPosEndLine, nSize, pMsg, sMsg, sMsgLowerCase);
				delete [] pMsg;
				if (g_nLogLevel > 1 && res && ((nPosEnd = sMsg.Find(_T("\r\n\r\n"))) > 0 || (nPosEnd = sMsg.Find(_T("\n\n"))) > 0))
					::LogLine(_T("%s, %s"), m_pDoc->GetAssignedDeviceName(), ::SingleLine(sMsg.Left(nPosEnd)));
				return res;
			}
			// Single image
			else if (CheckHttpHeaderValue(_T("image/jpeg"), nPos, sMsgLowerCase))
			{
				// Flag
				m_bFirstProcessing = TRUE;

				// Call jpeg parser
				res = ParseSingle(bLastCall, nSize, sMsg, sMsgLowerCase);
				delete [] pMsg;
				if (g_nLogLevel > 1 && res && ((nPosEnd = sMsg.Find(_T("\r\n\r\n"))) > 0 || (nPosEnd = sMsg.Find(_T("\n\n"))) > 0))
					::LogLine(_T("%s, %s"), m_pDoc->GetAssignedDeviceName(), ::SingleLine(sMsg.Left(nPosEnd)));
				return res;
			}
			// Text
			else if (CheckHttpHeaderValue(_T("text/plain"), nPos, sMsgLowerCase))
			{
				if ((m_pDoc->m_nNetworkDeviceTypeMode == GENERIC_SP	||
					m_pDoc->m_nNetworkDeviceTypeMode == GENERIC_CP)	&&
					++m_pDoc->m_nHttpGetFrameLocationPos < m_pDoc->m_HttpGetFrameLocations.GetSize())
				{
					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Try next possible video device location string
					m_pDoc->m_HttpThread.SetEventVideoConnect();
				}
				else if (m_bQueryVideoProperties)
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
					m_bQueryVideoProperties = FALSE;

					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Start Connection
					m_pDoc->m_HttpThread.SetEventVideoConnect();
				}
				else if (m_bSetVideoResolution)
				{
					// Reset flag
					m_bSetVideoResolution = FALSE;

					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Start Connection
					m_pDoc->m_HttpThread.SetEventVideoConnect();
				}
				else if (m_bSetVideoCompression)
				{
					// Reset flag
					m_bSetVideoCompression = FALSE;

					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Start Connection
					m_pDoc->m_HttpThread.SetEventVideoConnect();
				}
				else if (m_bSetVideoFramerate)
				{
					// Reset flag
					m_bSetVideoFramerate = FALSE;

					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Start Connection
					m_pDoc->m_HttpThread.SetEventVideoConnect();
				}
				else
				{
					// Msg
					m_pDoc->ConnectErr(ML_STRING(1488, "First open camera in browser"), m_pDoc->GetDeviceName());
					
					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();
				}
				delete [] pMsg;
				if (g_nLogLevel > 1 && ((nPosEnd = sMsg.Find(_T("\r\n\r\n"))) > 0 || (nPosEnd = sMsg.Find(_T("\n\n"))) > 0 || (nPosEnd = sMsg.GetLength()) > 0))
					::LogLine(_T("%s, %s"), m_pDoc->GetAssignedDeviceName(), ::SingleLine(sMsg.Left(nPosEnd)));
				return FALSE; // Do not call Processor
			}
			// Html
			else if (CheckHttpHeaderValue(_T("text/html"), nPos, sMsgLowerCase))
			{
				if ((m_pDoc->m_nNetworkDeviceTypeMode == GENERIC_SP	||
					m_pDoc->m_nNetworkDeviceTypeMode == GENERIC_CP)	&&
					++m_pDoc->m_nHttpGetFrameLocationPos < m_pDoc->m_HttpGetFrameLocations.GetSize())
				{
					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();

					// Try next possible video device location string
					m_pDoc->m_HttpThread.SetEventVideoConnect();
				}
				else
				{
					// Msg
					m_pDoc->ConnectErr(ML_STRING(1488, "First open camera in browser"), m_pDoc->GetDeviceName());
					
					// Empty the buffers, so that parser stops calling us!
					pNetCom->Read();
				}
				delete [] pMsg;
				if (g_nLogLevel > 1 && ((nPosEnd = sMsg.Find(_T("\r\n\r\n"))) > 0 || (nPosEnd = sMsg.Find(_T("\n\n"))) > 0 || (nPosEnd = sMsg.GetLength()) > 0))
					::LogLine(_T("%s, %s"), m_pDoc->GetAssignedDeviceName(), ::SingleLine(sMsg.Left(nPosEnd)));
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
			m_pDoc->m_HttpThread.SetEventVideoConnect(sNewRequest);
	
			delete [] pMsg;
			if (g_nLogLevel > 1 && ((nPosEnd = sMsg.Find(_T("\r\n\r\n"))) > 0 || (nPosEnd = sMsg.Find(_T("\n\n"))) > 0 || (nPosEnd = sMsg.GetLength()) > 0))
				::LogLine(_T("%s, %s"), m_pDoc->GetAssignedDeviceName(), ::SingleLine(sMsg.Left(nPosEnd)));
			return FALSE; // Do not call Processor
		}
		// Unauthorized
		else if (sCode == _T("401"))
		{
			// Wait until whole header is received
			if (!bLastCall && sMsg.Find(_T("\r\n\r\n"), 0) < 0 && sMsg.Find(_T("\n\n"), 0) < 0)
			{
				delete [] pMsg;
				return FALSE; // Do not call Processor
			}

			// Choose the first supported authentication type
			// (there may be multiple www-authenticate lines)
			CString sChosenAuthLineLowerCase;
			CString sChosenAuthLine;
			AUTHTYPE ChosenAuthorizationType = AUTHBASIC; // in case of missing www-authenticate line
			nPosEnd = 0;
			while ((nPos = sMsgLowerCase.Find(_T("www-authenticate:"), nPosEnd)) >= 0 &&
					(nPosEnd = FindEndOfLine(sMsgLowerCase, nPos)) >= 0)
			{
				sChosenAuthLineLowerCase = sMsgLowerCase.Mid(nPos, nPosEnd - nPos);
				sChosenAuthLine = sMsg.Mid(nPos, nPosEnd - nPos);
				if ((nPos = sChosenAuthLineLowerCase.Find(_T("basic"), 0)) >= 0)
				{
					sChosenAuthLineLowerCase = sChosenAuthLineLowerCase.Mid(nPos);
					sChosenAuthLine = sChosenAuthLine.Mid(nPos);
					ChosenAuthorizationType = AUTHBASIC;
					break;
				}
				else if ((nPos = sChosenAuthLineLowerCase.Find(_T("digest"), 0)) >= 0)
				{
					sChosenAuthLineLowerCase = sChosenAuthLineLowerCase.Mid(nPos);
					sChosenAuthLine = sChosenAuthLine.Mid(nPos);
					ChosenAuthorizationType = AUTHDIGEST;
					break;
				}
			}
			pNetCom->Read(); // Empty the buffers, so that parser stops calling us!

			// Authentication failed?
			// Note: an ip cam may also decide to issue a new nonce by replying with a 401,
			//       for this reason it's important to check the m_bAuthorized flag
			if (!m_bAuthorized && m_AnswerAuthorizationType != AUTHNONE)
			{
				m_AnswerAuthorizationType = AUTHNONE; // reset
				m_pDoc->ConnectErr(ML_STRING(1780, "Authorization failed"), m_pDoc->GetDeviceName());
				delete[] pMsg;
				return FALSE; // Do not call Processor
			}

			// Set the Authorization Type and clear the flag
			m_AnswerAuthorizationType = ChosenAuthorizationType;
			m_bAuthorized = FALSE;

			// Realm (m_sRealm will not contain the double quotes)
			if ((nPos = sChosenAuthLineLowerCase.Find(_T("realm=\""), 0)) >= 0)
			{
				nPos += 7;
				if ((nPosEnd = sChosenAuthLineLowerCase.Find(_T('\"'), nPos)) >= 0)
					m_sRealm = sChosenAuthLine.Mid(nPos, nPosEnd - nPos);
			}

			// Additional parameters for digest auth
			if (m_AnswerAuthorizationType == AUTHDIGEST)
			{
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
			}

			// Reset Client Nonce Count
			m_dwCNonceCount = 0U;

			// Start Connection with Last Request
			m_pDoc->m_HttpThread.SetEventVideoConnect(m_sLastRequest);
	
			delete [] pMsg;
			if (g_nLogLevel > 1 && ((nPosEnd = sMsg.Find(_T("\r\n\r\n"))) > 0 || (nPosEnd = sMsg.Find(_T("\n\n"))) > 0 || (nPosEnd = sMsg.GetLength()) > 0))
				::LogLine(_T("%s, %s"), m_pDoc->GetAssignedDeviceName(), ::SingleLine(sMsg.Left(nPosEnd)));
			return FALSE; // Do not call Processor
		}
		else
		{
			if ((m_pDoc->m_nNetworkDeviceTypeMode == GENERIC_SP	||
				m_pDoc->m_nNetworkDeviceTypeMode == GENERIC_CP)	&&
				++m_pDoc->m_nHttpGetFrameLocationPos < m_pDoc->m_HttpGetFrameLocations.GetSize())
			{
				// Empty the buffers, so that parser stops calling us!
				pNetCom->Read();

				// Try next possible video device location string
				m_pDoc->m_HttpThread.SetEventVideoConnect();
			}
			else
			{
				// Msg
				m_pDoc->ConnectErr(ML_STRING(1465, "Cannot connect to camera"), m_pDoc->GetDeviceName());

				// Empty the buffers, so that parser stops calling us!
				pNetCom->Read();
			}
			delete [] pMsg;
			if (g_nLogLevel > 1 && ((nPosEnd = sMsg.Find(_T("\r\n\r\n"))) > 0 || (nPosEnd = sMsg.Find(_T("\n\n"))) > 0 || (nPosEnd = sMsg.GetLength()) > 0))
				::LogLine(_T("%s, %s"), m_pDoc->GetAssignedDeviceName(), ::SingleLine(sMsg.Left(nPosEnd)));
			return FALSE; // Do not call Processor
		}
	}
	// Multipart or something more received before an above
	// set connection event gets executed. That's ok
	// because ParseMultipart() is robust!
	else
	{
		res = ParseMultipart(pNetCom, 0, nSize, pMsg, sMsg, sMsgLowerCase);
		delete [] pMsg;
		if (g_nLogLevel > 1 && res && ((nPosEnd = sMsg.Find(_T("\r\n\r\n"))) > 0 || (nPosEnd = sMsg.Find(_T("\n\n"))) > 0))
			::LogLine(_T("%s, %s"), m_pDoc->GetAssignedDeviceName(), ::SingleLine(sMsg.Left(nPosEnd)));
		return res;
	}
}

BOOL CVideoDeviceDoc::CHttpParseProcess::DecodeVideo(AVPacket* avpkt)
{
	int got_picture = 0;
	int len = avcodec_decode_video2(m_pCodecCtx,
									m_pFrame,
									&got_picture,
									avpkt);
    if (len < 0)
	{
		if (g_nLogLevel > 0)
			::LogLine(_T("%s, avcodec_decode_video2 FAILURE (returned=%d)"), m_pDoc->GetAssignedDeviceName(), len);
		return FALSE;
	}

	// Re-init if the size changed externally
	if (m_pDoc->m_DocRect.right != m_pCodecCtx->width ||
		m_pDoc->m_DocRect.bottom != m_pCodecCtx->height)
		m_bFirstProcessing = TRUE;

	// Init
	if (m_bFirstProcessing)
	{
		// Init Image Convert Context
		if (!InitImgConvert())
			return FALSE;

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

	// Convert and Process Frame
	// (first try fast conversion, if source format not supported fall back to sws_scale)
	if (got_picture && m_pImgConvertCtx)
	{
		BOOL bOk = ITU601JPEGConvert(m_pCodecCtx->pix_fmt,	// Source Format
									AV_PIX_FMT_YUV420P,		// Destination Format
									m_pFrame->data,			// Source Data
									m_pFrame->linesize,		// Source Stride
									m_pFrameI420->data,		// Destination Data
									m_pFrameI420->linesize,	// Destination Stride
									m_pCodecCtx->width,		// Width
									m_pCodecCtx->height);	// Height
		if (!bOk)
		{
			bOk = sws_scale(m_pImgConvertCtx,				// Image Convert Context
							m_pFrame->data,					// Source Data
							m_pFrame->linesize,				// Source Stride
							0,								// Source Slice Y
							m_pCodecCtx->height,			// Source Height
							m_pFrameI420->data,				// Destination Data
							m_pFrameI420->linesize) > 0;	// Destination Stride
		}
		if (bOk)
		{
			m_pDoc->m_lEffectiveDataRateSum += avpkt->size;
			m_pDoc->ProcessI420Frame(m_pI420Buf, m_dwI420ImageSize);
		}
	}
	// In case that avcodec_decode_video2 fails try LoadJPEG
	else
	{
		CDib Dib;
		Dib.SetShowMessageBoxOnError(FALSE);
		if (g_nLogLevel > 0)
			::LogLine(_T("%s, avcodec_decode_video2 FAILURE, trying LoadJPEG() + Compress(I420)"), m_pDoc->GetAssignedDeviceName());
		if (Dib.LoadJPEG(avpkt->data, avpkt->size, 1, TRUE) && Dib.Compress(FCC('I420')))
		{
			if (g_nLogLevel > 0)
				::LogLine(_T("%s, LoadJPEG() + Compress(I420) success"), m_pDoc->GetAssignedDeviceName());
			m_pDoc->m_lEffectiveDataRateSum += avpkt->size;
			m_pDoc->ProcessI420Frame(Dib.GetBits(), Dib.GetImageSize());
		}
		else
		{
			if (g_nLogLevel > 0)
				::LogLine(_T("%s, LoadJPEG() + Compress(I420) FAILURE"), m_pDoc->GetAssignedDeviceName());
		}
	}

	return TRUE;
}

// pLinBuf is a correctly aligned buffer ending
// with FF_INPUT_BUFFER_PADDING_SIZE zero bytes
void CVideoDeviceDoc::CHttpParseProcess::Process(unsigned char* pLinBuf, int nSize)
{
	// Enter CS
	::EnterCriticalSection(&m_pDoc->m_csHttpProcess);

	// Poll Next Frame
	if (m_FormatType == FORMATVIDEO_JPEG)
		m_bPollNextJpeg = TRUE;

	// If new sequence do a reset
	if (m_bFirstProcessing)
		OpenAVCodec();

	// Check
	if (!m_pCodecCtx)
	{
		::LeaveCriticalSection(&m_pDoc->m_csHttpProcess);
		return;
	}

	// Init source data
	AVPacket avpkt;
	av_init_packet(&avpkt);
	avpkt.data = pLinBuf;
	avpkt.size = nSize;

	// Reset structure 
	av_frame_unref(m_pFrame);

	// Decode Video
	if (!DecodeVideo(&avpkt))
	{
		av_free_packet(&avpkt);
		::LeaveCriticalSection(&m_pDoc->m_csHttpProcess);
		return;
	}

	// Reset flag
	m_bFirstProcessing = FALSE;
	
	// Free packet
	av_free_packet(&avpkt);

	// Leave CS
	::LeaveCriticalSection(&m_pDoc->m_csHttpProcess);
}

BOOL CVideoDeviceDoc::CHttpParseProcess::OpenAVCodec()
{
	// Free
	FreeAVCodec();

	// Find the decoder for the video stream
	m_pCodec = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
	if (!m_pCodec)
		goto error;
	m_pDoc->m_CaptureBMI.bmiHeader.biCompression = FCC('MJPG');

	// Allocate Context
	/* if m_pCodec non-NULL, allocate private data and initialize defaults
		* for the given codec. It is illegal to then call avcodec_open2()
		* with a different codec.
		* If NULL, then the codec-specific defaults won't be initialized,
		* which may result in suboptimal default settings (this is
		* important mainly for encoders, e.g. libx264).
		*/
	m_pCodecCtx = avcodec_alloc_context3(m_pCodec);
	if (!m_pCodecCtx)
		goto error;

	// Width and Height Unknown at this point
	m_pCodecCtx->coded_width = 0;
	m_pCodecCtx->coded_height = 0;

	// Set FourCC
	m_pCodecCtx->codec_tag = FCC('MJPG');

	// Open codec
	if (avcodec_open2(m_pCodecCtx, m_pCodec, 0) < 0)
		goto error;

	// Allocate I420 frame
	m_pFrameI420 = av_frame_alloc();
	if (!m_pFrameI420)
		goto error;

	// Allocate frame
	m_pFrame = av_frame_alloc();
	if (!m_pFrame)
		goto error;

	return TRUE;

error:
	FreeAVCodec();
	return FALSE;
}

void CVideoDeviceDoc::CHttpParseProcess::FreeAVCodec()
{
	if (m_pCodecCtx)
	{
		/*
		Close a given AVCodecContext and free all the data associated with it
		(but not the AVCodecContext itself).
		Calling this function on an AVCodecContext that hasn't been opened will free
		the codec-specific data allocated in avcodec_alloc_context3() /
		avcodec_get_context_defaults3() with a non-NULL codec. Subsequent calls will
		do nothing.
		*/
		avcodec_close(m_pCodecCtx);
		av_freep(&m_pCodecCtx);
	}
	m_pCodec = NULL;

	if (m_pFrame)
		av_frame_free(&m_pFrame);
	if (m_pFrameI420)
		av_frame_free(&m_pFrameI420);

	if (m_pImgConvertCtx)
	{
		sws_freeContext(m_pImgConvertCtx);
		m_pImgConvertCtx = NULL;
	}

	if (m_pI420Buf)
	{
		av_free(m_pI420Buf);
		m_pI420Buf = NULL;
	}
	m_dwI420BufSize = 0;
	m_dwI420ImageSize = 0;
}

BOOL CVideoDeviceDoc::CHttpParseProcess::InitImgConvert()
{
	// Check
	if (m_pCodecCtx->width <= 0 || m_pCodecCtx->height <= 0	||
		m_pCodecCtx->pix_fmt < 0 || m_pCodecCtx->pix_fmt >= AV_PIX_FMT_NB)
	{
		if (g_nLogLevel > 0)
		{
			::LogLine(	_T("%s, CVideoDeviceDoc::CHttpParseProcess::InitImgConvert() wrong format: m_pCodecCtx->width=%d, m_pCodecCtx->height=%d, m_pCodecCtx->pix_fmt=%d"),
						m_pDoc->GetAssignedDeviceName(), m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt);
		}
		return FALSE;
	}

	// Determine required buffer size and allocate buffer if necessary
	m_dwI420ImageSize = avpicture_get_size(	AV_PIX_FMT_YUV420P,
											m_pCodecCtx->width,
											m_pCodecCtx->height);
	if (m_dwI420BufSize < m_dwI420ImageSize || m_pI420Buf == NULL)
	{
		if (m_pI420Buf)
			av_free(m_pI420Buf);
		m_pI420Buf = (LPBYTE)av_malloc(m_dwI420ImageSize + FF_INPUT_BUFFER_PADDING_SIZE);
		if (!m_pI420Buf)
			return FALSE;
		m_dwI420BufSize = m_dwI420ImageSize;
	}

	// Assign appropriate parts of buffer to image planes
	avpicture_fill((AVPicture*)m_pFrameI420,
					(unsigned __int8 *)m_pI420Buf,
					AV_PIX_FMT_YUV420P,
					m_pCodecCtx->width,
					m_pCodecCtx->height);

	// Prepare Image Conversion Context
	m_pImgConvertCtx = sws_getCachedContextHelper(	m_pImgConvertCtx,		// Re-use if already allocated
													m_pCodecCtx->width,		// Source Width
													m_pCodecCtx->height,	// Source Height
													m_pCodecCtx->pix_fmt,	// Source Format
													m_pCodecCtx->width,		// Destination Width
													m_pCodecCtx->height,	// Destination Height
													AV_PIX_FMT_YUV420P,		// Destination Format
													SWS_BICUBIC);			// Interpolation (add SWS_PRINT_INFO to debug)

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

#pragma warning(pop)

#endif
