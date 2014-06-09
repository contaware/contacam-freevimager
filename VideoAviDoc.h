#if !defined(AFX_VIDEOAVIDOC_H__24956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_)
#define AFX_VIDEOAVIDOC_H__24956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_

#pragma once

#include "uImagerDoc.h"
#include "WorkerThread.h"
#include "SortableStringArray.h"
#include "DxDraw.h"
#include "AviPlay.h"
#include "AVRec.h"

// Avi File Exceptions
#define AVI_E_ZEROPATH								0
#define AVI_E_WRONGEXTENTION						1
#define AVI_E_FILEEMPTY								2

// Player Timing Constants
#define VIDEO_PLAYER_THRESHOLD_TIME					5

// DirectDraw Full-Screen Display of GDI Dialogs Timeouts
#define DXDRAW_GDIDISPLAY_SAFEPAUSED_TIMEOUT		3000U	// ms
#define DXDRAW_GDIDISPLAY_DELAYEDRESTART_TIMEOUT	3000U	// ms

// Rendering Modes
#define RENDERING_MODE_GDI_RGB						0
#define RENDERING_MODE_GDI_YUV						1
#define RENDERING_MODE_DXDRAW_RGB					2
#define RENDERING_MODE_DXDRAW_YUV					3

// Audio only avi file
#define AUDIOONLY_DEFAULT_WIDTH						400
#define AUDIOONLY_DEFAULT_HEIGHT					40

// Forward Declaration
class CVideoAviView;
class CVideoAviChildFrame;
class CAviInfoDlg;
class COutVolDlg;
class CAudioVideoShiftDlg;
class CPlayerToolBarDlg;

//
// The synchronization between UI Thread and Worker Threads is quite complicated:
//
// This mainly because we are using a CDxDraw class that wraps the DirectDraw 7.0 COM
// Object in a STA (Single Threaded Apartment) environment.
// MTA (Multi Thread Apartment) would be much easier,
// but it is not supported by all COMs like OLE and others!
//
// STA means that each call from the Worker Thread to the COM Object passes through the
// UI Thread much like a SendMessage(). This causes a dead-locks if the Worker Thread is in
// the CDxDraw CS and calls CDxDraw functions while at the same time the UI Thread is waiting to
// enter a CDxDraw CS. To avoid that each UI function which wants to enter the CDxDraw CS tries to
// enter and if it fails a retry is scheduled through a delayed post message.
//
// To avoid the same type of dead-lock with the CDib CS all functions which call a CDxDraw
// function from a Worker Thread inside a CDib CS should be avoided because the CDib CS
// in the UI Thread is not entered through a try enter CS like the CDxDraw CS!  
//
// RESUMING THE RULES:
// 1. Each call to CDxDraw functions from the UI Thread is to be done through a Try Enter CS!
// 2. No CDxDraw calls from the Worker Thread inside a CDib CS!
//
// EXCEPTION TO THE RULES:
// Rule 1. may be broken if you are absolutely sure that the Worker Threads are not running!
//
//
// RESUME OF FUNCTIONS calling CDxDraw functions:
//
//
// Called by the UI Thread:
// ------------------------
//
// - CMainFrame::FullScreenModeOn()          No need for a Try Enter CS because the function makes sure
//                                           the Video Thread is waiting in a safe place.
//                                           While processing we cannot switch to full-screen.
//
// - CMainFrame::FullScreenModeOff()         Same Comments as above.
//
// - CVideoAviDoc::LoadActiveStreams()       No need for a Try Enter CS because the streams are loaded 
//                                           before playing and the process functions have already
//                                           terminated when reloading.
//
// - CVideoAviView::RenderingSwitch()        No need for a Try Enter CS because the function is only
//                                           called when not playing and not processing.
//
// - CVideoAviView::OnRestoreFrame()         Has Try Enter CS.
//
// - CVideoAviView::OnDraw()                 Has Try Enter CS.
//
//
// Called by CPlayVideoFileThread::Work():
// ---------------------------------------
//
// - Draw() (-> EraseBkgnd())                Normal Enter CS.
//
//
// Called by CProcessing::Work():
// ------------------------------
// 
// Nothing is called directly by this thread, all calls are through the UI Thread
// with PostMessage(). See DisplayFrame() or RestoreFrame().
//


// The Document Class
class CVideoAviDoc : public CUImagerDoc
{
public:
	// The Play Video File Thread Class
	class CPlayVideoFileThread : public CWorkerThread
	{
		public:
			CPlayVideoFileThread();
			virtual ~CPlayVideoFileThread();
			void SetDoc(CVideoAviDoc* pDoc) {m_pDoc = pDoc;};
			void SetPlaySpeedPercent(int nPlaySpeedPercent) {m_nPlaySpeedPercent = nPlaySpeedPercent;};
			int GetPlaySpeedPercent() const {return m_nPlaySpeedPercent;};
			int GetMilliSecondsCorrection() const {return m_nMilliSecondsCorrection;};
			int GetMilliSecondsCorrectionAvg() const {return m_nMilliSecondsCorrectionAvg;};
			UINT GetTimerDelay() const {return m_uiTimerDelay;};
			DWORD GetDroppedFramesCount() const {return m_dwDroppedFramesCount;};
			void ResetDroppedFramesCount() {m_dwDroppedFramesCount = 0;};
			void SetPlaySyncEvent(HANDLE hPlaySyncEvent) {m_hPlaySyncEvent = hPlaySyncEvent; m_hPlaySyncEventArray[1] = hPlaySyncEvent;};
			void SetWaitingForStart(BOOL bWaitingForStart) {m_bWaitingForStart = bWaitingForStart;}; 
			BOOL IsWaitingForStart() const	{return m_bWaitingForStart;};
			void Rew();
			double GetFrameRate() const;
			BOOL GetFrameRate(DWORD* pdwRate, DWORD* pdwScale) const;

			// Direct Draw Sync.

			// Prepares the Exclusive Full-Screen DirectDraw Mode for GDI Drawing
			// and posts the message to show the GDI Object (usually a dialog).
			void DxDrawGDIDisplay(	HWND hSafePausedMsgWnd,			// Window is notified when GDI drawing can be safely performed
									LONG lSafePausedMsgId,			// Message ID
									WPARAM wparam,					// Message W Param
									LPARAM lparam);					// Message L Param
			void SafePauseDelayedRestart(HWND hSafePausedWnd,		// If hSafePausedWnd != NULL and lSafePausedMsgId != 0
										LONG lSafePausedMsgId,		// and in safe paused state a message is posted to hSafePausedWnd
										WPARAM wparam,				// Message W Param
										LPARAM lparam,				// Message L Param
										DWORD dwSafePausedMsgTimeout,// Message Post Timeout (after this amount of time a message is posted also if we are not in the safe state)
										DWORD dwDelayedRestartTimeout,	// Video Thread sleeps in safe place a max. of this amount of time
										BOOL bDoUpdateDoFullScreenBlt);	// Should the Video Thread set the m_bDoFullScreenBlt when in safe place?
			void OnSafePauseTimeout(WPARAM wparam, LPARAM lparam);
			__forceinline BOOL DoFullScreenBlt() const {return (BOOL)m_bDoFullScreenBlt;};
			__forceinline void SetFullScreenBlt() {::InterlockedExchange(&m_bDoFullScreenBlt, 1);};
			__forceinline void ResetFullScreenBlt() {::InterlockedExchange(&m_bDoFullScreenBlt, 0);};
			__forceinline void SetSafePauseRestartEvent() {::SetEvent(m_hDelayedRestartEvent);}; // Finish sleeping in safe place
		
		protected:
			int Work();									// Thread Proc.
			void OnExit();								// Called when thread exits in response to a Thread Kill Event
			__forceinline BOOL OnSync();				// Called at the beginning the first time and with each looping
			__forceinline BOOL OnSafePause();			// Called with each frame to see whether we have to stop a moment in a safe place,
														// used mainily by Direct Draw

			volatile int m_nPlaySpeedPercent;			// 100 = normal speed
			volatile int m_nMilliSecondsCorrection;		// Instantaneous drift of frame with respect to audio samples (if available) or with respect to system time
			volatile int m_nMilliSecondsCorrectionAvg;	// Moving-average drift of frame with respect to audio samples (if available) or with respect to system time
			volatile UINT m_uiTimerDelay;				// The Frame Timer Value
			volatile DWORD m_dwDroppedFramesCount;		// The Dropped Frames
			CVideoAviDoc* m_pDoc;						// The Doc Pointer
			HANDLE m_hPlaySyncEvent;					// Sync. with Audio Event
			volatile BOOL m_bWaitingForStart;			// When finishing playing and looping enabled the thread sets this var and waits that the audio finishes as well
			HANDLE m_hPlaySyncEventArray[2];			// Event array containing the Thread Kill Event and the m_hPlaySyncEvent 
			HANDLE m_hTimerEvent;						// The Timer Event triggered each m_uiTimerDelay ms
			HANDLE m_hEventArray[2];					// Event array containing the Thread Kill Event and the m_hTimerEvent

			// Direct Draw Sync. Vars
			volatile LONG m_bDoFullScreenBlt;
			volatile BOOL m_bDoUpdateDoFullScreenBlt;
			volatile BOOL m_bDoSafePause;
			volatile DWORD m_dwDelayedRestartTimeout;
			volatile BOOL m_bDoSetSafePaused;
			volatile LONG m_lSafePausedMsgId;
			volatile WPARAM m_wSafePausedMsgWParam;
			volatile LPARAM m_lSafePausedMsgLParam;
			HWND volatile m_hSafePausedMsgWnd;
			volatile DWORD m_dwSafePausedMsgSeq;
			CRITICAL_SECTION m_csSafePauseDelayedRestart;
			HANDLE volatile m_hDelayedRestartEvent;
			HANDLE m_hDelayedRestartEventArray[2];
	};

	// The Play Audio File Thread Class
	class CPlayAudioFileThread : public CWorkerThread
	{
		public:
			class CMixerOut
			{
				public:
					CMixerOut();
					virtual ~CMixerOut();
					BOOL Open(HWAVEOUT hWaveOut, HWND hWndCallBack = NULL);
					void Close();
					BOOL IsOpen() {return (m_hMixer != NULL);};
					BOOL IsWithWndHandleOpen() {return ((m_hMixer != NULL) && (m_hWndMixerCallback != NULL));};
					HMIXER GetHandle() const {return m_hMixer;};
					
					BOOL GetMasterVolume(DWORD& dwVolumeLeft, DWORD& dwVolumeRight);
					BOOL SetMasterVolume(DWORD dwVolumeLeft, DWORD dwVolumeRight);
					DWORD GetMasterVolumeControlID() const {return m_dwMasterVolumeControlID;};
					DWORD GetMasterVolumeControlMin() const {return m_dwMasterVolumeControlMin;};
					DWORD GetMasterVolumeControlMax() const {return m_dwMasterVolumeControlMax;};

					BOOL GetWaveVolume(DWORD& dwVolumeLeft, DWORD& dwVolumeRight);
					BOOL SetWaveVolume(DWORD dwVolumeLeft, DWORD dwVolumeRight);
					DWORD GetWaveVolumeControlID() const {return m_dwWaveVolumeControlID;};
					DWORD GetWaveVolumeControlMin() const {return m_dwWaveVolumeControlMin;};
					DWORD GetWaveVolumeControlMax() const {return m_dwWaveVolumeControlMax;};

					BOOL GetMasterMute(BOOL& bCheck);
					BOOL SetMasterMute(BOOL bCheck);
					DWORD GetMasterMuteControlID() const {return m_dwMasterMuteControlID;};
					BOOL GetWaveMute(BOOL& bCheck);
					BOOL SetWaveMute(BOOL bCheck);
					DWORD GetWaveMuteControlID() const {return m_dwWaveMuteControlID;};

				protected:
					HMIXER m_hMixer;
					HWND m_hWndMixerCallback;
					UINT m_uiMixerID;
					
					DWORD m_dwMasterVolumeControlID;
					DWORD m_dwMasterVolumeControlMin;
					DWORD m_dwMasterVolumeControlMax;
					DWORD m_dwMasterMuteControlID;
					DWORD m_dwMasterChannels;

					DWORD m_dwWaveVolumeControlID;
					DWORD m_dwWaveVolumeControlMin;
					DWORD m_dwWaveVolumeControlMax;
					DWORD m_dwWaveMuteControlID;
					DWORD m_dwWaveChannels;
			};

			// General Functions
			CPlayAudioFileThread();
			virtual ~CPlayAudioFileThread();
			void SetDoc(CVideoAviDoc* pDoc) {m_pDoc = pDoc;};
			BOOL SetPlaySpeedPercent(int nPlaySpeedPercent);
			void SetPlaySyncEvent(HANDLE hPlaySyncEvent) {m_hPlaySyncEvent = hPlaySyncEvent; m_hPlaySyncEventArray[1] = hPlaySyncEvent;};
			BOOL IsWaitingForStart() const	{return m_bWaitingForStart;};
			BOOL IsSleepingVideoLeadsAudio() const {return m_bSleepingVideoLeadsAudio;};
			void SetWaitingForStart(BOOL bWaitingForStart) {m_bWaitingForStart = bWaitingForStart;}; 
			HWAVEOUT GetWaveHandle() const {return m_hWaveOut;};
			void SetWaveHandle(HWAVEOUT hWaveOut) {m_hWaveOut = hWaveOut;};
			void AudioOutDestinationDialog();
			BOOL IsOpen() const {return (m_hWaveOut != NULL);};
			BOOL IsResetting() const {return m_bResetting;};
			BOOL OpenOutAudio(LPWAVEFORMATEX pWaveFormat = NULL);
			void CloseOutAudio();
			BOOL DataOutAudio(LPBYTE lpData, DWORD dwSize);
			LONGLONG GetCurrentSamplePos() const;
			LONGLONG GetNextSamplePos() const;
			BOOL SetCurrentSamplePos(LONGLONG llCurrentSamplePos);
			BOOL SetNextSamplePos(LONGLONG llNextSamplePos);
			LONGLONG GetTotalSamples() const;
			void PlaySyncAudioFromVideo();
			void Rew();
			DWORD GetSampleRate() const; // In Hz

			// Wave Format Functions
			void SetWaveFormat(LPWAVEFORMATEX pWaveFormat);
			LPWAVEFORMATEX GetWaveFormat()					{return m_pWaveFormat;};
			WORD GetWaveFormatTag() const					{return m_pWaveFormat->wFormatTag;};		// PCM is WAVE_FORMAT_PCM
			DWORD GetWaveSampleRate() const					{return m_pWaveFormat->nSamplesPerSec;};	// In Hz
			WORD GetWaveBits() const						{return m_pWaveFormat->wBitsPerSample;};	// 8 or 16 bits
			WORD GetWaveNumOfChannels() const				{return m_pWaveFormat->nChannels;};			// Mono = 1, Stereo = 2
			WORD GetWaveSampleSize() const					{return m_pWaveFormat->nBlockAlign;};		// In Bytes
			DWORD GetWaveBytesPerSeconds() const			{return m_pWaveFormat->nAvgBytesPerSec;};	// If PCM:  nSamplesPerSec * nBlockAlign

			// The Output Mixer
			CMixerOut m_Mixer;

		protected:
			int Work();
			void OnExit();
			BOOL WaitEndOfPlayUnprepareHdr();

			LPBYTE m_pBuf[2];
			HWAVEOUT m_hWaveOut;
			volatile int m_nPlaySpeedPercent;
			CVideoAviDoc* m_pDoc;
			LPWAVEFORMATEX m_pWaveFormat;
			WAVEHDR m_WaveHeader[2];
			HANDLE m_hWaveOutEvent;
			HANDLE m_hEventArray[2];
			int m_nWaveOutToggle;
			HANDLE m_hPlaySyncEvent;
			volatile BOOL m_bWaitingForStart;
			volatile BOOL m_bResetting;
			volatile BOOL m_bSleepingVideoLeadsAudio;
			HANDLE m_hPlaySyncEventArray[2];
	};

	// Video Processing Classes
	class CFunct
	{
		public:
			CFunct(){m_pDoc = NULL;};
			void SetDoc(CVideoAviDoc* pDoc) {m_pDoc = pDoc;};
			virtual BOOL Do() = 0;
		protected:
			CVideoAviDoc* m_pDoc;
	};
	class CSaveAs : public CFunct
	{
		public:
			virtual BOOL Do() {return m_pDoc->SaveAs();};
	};
	class CExtractframes : public CFunct
	{
		public:
			virtual BOOL Do() {return m_pDoc->FileExtractframes();};
	};
	class CPercentProgress
	{
		public:
			double dVideoPercentOffset;
			double dVideoPercentSize;
			double dAudioPercentOffset;
			double dAudioPercentSize;
	};
	class CFileMergeAs : public CFunct
	{
		public:
			virtual BOOL Do() {return m_pDoc->FileMergeAs();};
	};
	class CShrinkDocTo : public CFunct
	{
		public:
			virtual BOOL Do() {return m_pDoc->ShrinkDocTo(this);};
			CString m_sOutFileName;
	};
	class CProcessingThread : public CWorkerThread
	{
		public:
			CProcessingThread(){m_pDoc = NULL; m_pFunct = NULL;};
			virtual ~CProcessingThread() {Kill();};
			void SetDoc(CVideoAviDoc* pDoc) {m_pDoc = pDoc;};
			void SetProcessingFunct(CFunct* pFunct) {m_pFunct = pFunct;};

		protected:
			int Work();
			CVideoAviDoc* m_pDoc;
			CFunct* m_pFunct;
	};
	typedef CArray<CAVIPlay*,CAVIPlay*> AVIPLAYARRAY;

protected: // create from serialization only
	DECLARE_DYNCREATE(CVideoAviDoc)
	CVideoAviDoc();
	virtual ~CVideoAviDoc();

public:

	// General Functions
	__forceinline BOOL IsAVI()
			{return (	(::GetFileExt(m_sFileName) == _T(".avi"))	||
						(::GetFileExt(m_sFileName) == _T(".divx")));};
	static BOOL IsAVI(LPCTSTR szFileName);
	void CloseDocument();			// Close Document by sending a WM_CLOSE to the Parent Frame
	void CloseDocumentForce();		// Closes Without Asking To Save!
	void SetDocumentTitle();
	CVideoAviView* GetView() const {return m_pView;};
	void SetView(CVideoAviView* pView) {m_pView = pView;};
	CVideoAviChildFrame* GetFrame() const {return m_pFrame;};
	void SetFrame(CVideoAviChildFrame* pFrame) {m_pFrame = pFrame;};
	void UpdateAviInfoDlg();
	void EditSnapshot();
	void EditDelete(BOOL bPrompt);

	// Player Control
	void PlayAVI();				// Not blocking start (starts playing if not already playing)
	void StopAVI();				// Not blocking stop playing.
	void JumpToFirstFrame();
	void JumpToLastFrame();
	void FrameBack();
	void FrameFront();
	void FrameBackFast(BOOL bVeryFast = FALSE);
	void FrameFrontFast(BOOL bVeryFast = FALSE);
	__forceinline BOOL IsLoop() const {return (BOOL)m_bLoop;};
	__forceinline void ChangeLoop() {::InterlockedExchange(&m_bLoop, m_bLoop ? 0 : 1);};
	__forceinline BOOL GetPlayFrameRate(DWORD* pdwRate, DWORD* pdwScale) const;
	__forceinline double GetPlayFrameRate() const
									{return (double)m_dwPlayRate / (double)m_dwPlayScale;};
	__forceinline void SetPlayFrameRate(double dFrameRate)
									{m_dwPlayRate = (DWORD)Round(dFrameRate * AVI_SCALE_DOUBLE);
															m_dwPlayScale = AVI_SCALE_INT;};
	__forceinline void SetPlayFrameRate(DWORD dwRate, DWORD dwScale)
									{m_dwPlayRate = dwRate; m_dwPlayScale = dwScale;};



	// Load AVI
	BOOL LoadAVI(	CString sFileName,				// AVI File Name
					DWORD dwFramePos = 0,			// Get first frame at this position
					BOOL bActiveStreams = FALSE);	// If TRUE:  Load the Set Active Streams
													// If FALSE: Prompt User to Select Streams
	BOOL LoadStreams(DWORD dwFramePos);				// Prompt User to Select Streams
	BOOL LoadActiveStreams(DWORD dwFramePos);		// Load the Set Active Streams at given position
	BOOL LoadActiveStreams();						// Load the Set Active Streams at set position

	// Close Streams
	BOOL CloseActiveStreams();

	// Video Processing
	__forceinline BOOL IsProcessing() {return	((m_ProcessingThread.IsAlive() &&
												((m_nVideoPercentDone >= 0) || (m_nAudioPercentDone >= 0)))
												||
												(m_sProcessingError != _T("")));};
	void ResetPercentDone();
	void StartShrinkDocTo(CString sOutFileName);
	BOOL ShrinkDocTo(CShrinkDocTo* pShrinkDocTo);
	void StartFileMergeAs();
	BOOL FileMergeAs();

	// Save
	BOOL Save(BOOL bNoReload = FALSE);			// NoReload is set only if Saving and Closing,
												// from inside SaveModified()

	// Save As
	BOOL SaveAs(CString sDlgTitle = _T(""));	// Open File Dialog Title
												// If _T("") the default is used

	// Extract frames
	BOOL FileExtractframes();

	// End of Position Slider Thumbtrack
	void EndThumbTrack();			// No blocking restart playing at m_nThumbTrackPos
									// if it was playing. If it wasn't playing just
									// call DisplayFrame().
									// Polls the playing state periodically and
									// restarts playing when stopped. With the current
									// defines polling is given up after 5 seconds.

	// Settings
	void LoadSettings();
	void SaveSettings();

	// Show / Hide Avi Info Dialog
	void AviInfoDlg(BOOL bCenterCursor = FALSE);

	// Show / Hide Play Volume Dialog
	void PlayVolDlg(BOOL bCenterCursor = FALSE);

	// Show / Hide Audio Video Shift Dialog
	void AudioVideoShiftDlg(BOOL bCenterCursor = FALSE);

	// Show / Hide Player Controls in FullScreen Mode
	void PlayerToolBarDlg(CPoint ptPos);

	// Enable / Disable Infinite Play Loop
	void PlayLoop();

	// Display Frame
	BOOL DisplayFrame(int nFrame, int nDelay = 0);

	// Restore Frame
	void RestoreFrame(int nDelay = 0);

// Public Variables
public:

	// General Vars
	DWORD m_dwPlayAudioDeviceID;				// Audio Play Device ID
	HANDLE m_hPlaySyncEvent;					// Audio / Video Play Synchronization
	CRITICAL_SECTION m_csPlayWaitingForStart;	// Critical Section Used In the Player For Sync Audio / Video
	BOOL m_bAboutToStopAviPlay;					// Player is stopping, just to gray the toolbar buttons
												// while waiting the threads termination
	CAVIPlay* m_pAVIPlay;						// Pointer to the opened Avi file
	volatile DWORD m_dwPlayRate;				// Play Rate
	volatile DWORD m_dwPlayScale;				// Play Scale -> FrameRate = dwRate / dwScale 
	volatile BOOL m_bTimePositionShow;			// Flag indicating whether the time position is displayed
	int m_nActiveVideoStream;					// Active Video Stream
	int m_nActiveAudioStream;					// Active Audio Stream
	int m_nEndThumbTrackRetryCountDown;			// How many times to try ending the thumbtrack
	volatile DWORD m_dwFrameCountUp;			// Played Frames Count-Up, it can wrap around!

	// Thread Vars
	CPlayVideoFileThread m_PlayVideoFileThread; // Video Thread Playing the Frames from File
	CPlayAudioFileThread m_PlayAudioFileThread; // Thread playing the Audio
	CProcessingThread m_ProcessingThread;

	// Compression Vars
	int m_nVideoCompressorKeyframesRate;		// Keyframes Rate
	float m_fVideoCompressorQuality;			// 2.0f best quality, 31.0f worst quality
	LPWAVEFORMATEX m_pAudioCompressorWaveFormat;

	// If Positive Values the Audio leads (is played before)
	// the video by the given amount of ms.
	// If Negative Values the opposite applies.
	volatile int m_nAudioLeadsVideoShift;
	volatile int m_nAudioLeadsVideoShiftInitValue;

	// Processing Vars
	volatile int m_nVideoPercentDone;
	volatile int m_nAudioPercentDone;
	CString m_sProcessingError;

	// User Zoom Rect of the previous full-screen
	CRect m_PrevUserZoomRect;

	// Draw
	CDxDraw m_DxDraw;					// Direct Draw Object
	volatile BOOL m_bUseDxDraw;			// Use Direct Draw?
	volatile BOOL m_bForceRgb;			// Force Rgb Rendering
	volatile BOOL m_bNoDrawing;			// Disable Drawing
	volatile LONG m_bAboutToRestoreFrame;// Restoring Frame Soon

	// AV Codec Priority
	bool m_bAVCodecPriority;

	// Avi Info Dialog
	CAviInfoDlg* m_pAviInfoDlg;

	// Play Volume Dialog
	COutVolDlg* m_pOutVolDlg;

	// Audio Video Shift Dialog
	CAudioVideoShiftDlg* m_pAudioVideoShiftDlg;

	// Player Controls in FullScreen Mode
	CPlayerToolBarDlg* m_pPlayerToolBarDlg;

// Protected Functions
protected:
	void DeleteDocFile();
	BOOL PlayAVI(BOOL bDoRewind, int nSpeedPercent);
	CString ExtractAsBMP(const CString& sFileName);
	CString ExtractAsJPEG(const CString& sFileName, int nCompressionQuality);
	BOOL SaveAsAnimGIF(	const CString& sFileName,
						BOOL bUniqueColorTable,
						BOOL bDitherColorConversion,
						UINT uiMaxColors,
						UINT uiPlayTimes);
	static BOOL SaveAsAVCODECMultiFile(	const CString& sDstFileName,
										CAVRec** ppAVRec,		// If first file *ppAVRec is NULL and will be allocated
										CAVIPlay* pAVIPlay,		// Already Opened Input AVI File
										DWORD dwVideoCompressorFourCC,
										int nVideoCompressorKeyframesRate,
										float fVideoCompressorQuality,
										LPWAVEFORMATEX pAudioCompressorWaveFormat,
										CWnd* pProgressWnd = NULL,
										BOOL bProgressSend = TRUE,
										CPercentProgress* pPercentProgress = NULL,
										CWorkerThread* pThread = NULL);
	static BOOL SaveAsAVCODECSingleFile(const CString& sDstFileName,
										CAVIPlay* pAVIPlay,		// Already Opened Input AVI File
										DWORD dwVideoCompressorFourCC,
										int nVideoCompressorKeyframesRate,
										float fVideoCompressorQuality,
										LPWAVEFORMATEX pAudioCompressorWaveFormat,
										CWnd* pProgressWnd = NULL,
										BOOL bProgressSend = TRUE,
										CWorkerThread* pThread = NULL);
	
	// Return Values:
	// -1: Error
	// 0 : Dlg Canceled
	// 1 : Ok
	static int SaveAsAVCODECDlgs(const CString& sDstFileName,
								CAVRec** ppAVRec,		// If first file *ppAVRec is NULL and will be allocated
								CAVIPlay* pAVIPlay,		// Already Opened Input AVI File
								DWORD& dwVideoCompressorFourCC,
								int& nVideoCompressorKeyframesRate,
								float& fVideoCompressorQuality,
								LPWAVEFORMATEX pAudioCompressorWaveFormat,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE,
								CPercentProgress* pPercentProgress = NULL,
								CWorkerThread* pThread = NULL);
	int SaveAsAVCODECDlgs(const CString& sDstFileName);

	// Static Save Function with compression params
	// called by ShrinkDocTo()
	static BOOL SaveAsAVCODEC(	const CString& sDstFileName,
								const CString& sSrcFileName,
								DWORD dwVideoCompressorFourCC,
								int nVideoCompressorKeyframesRate,
								float fVideoCompressorQuality,
								LPWAVEFORMATEX pAudioCompressorWaveFormat,
								CWnd* pProgressWnd = NULL,
								BOOL bProgressSend = TRUE,
								CWorkerThread* pThread = NULL);

	// Main Save Function called by SaveAs()
	BOOL SaveAsAVCODEC(	const CString& sFileName,
						BOOL bSaveCopyAs,
						int nCurrentFramePos);

	// Merge Function
	// Return Values:
	// -1: Error
	// 0 : Dlg Canceled
	// 1 : Ok
	int AVIFileMergeAVCODEC(CString sSaveFileName,
							CSortableStringArray* pAviFileNames,
							DWORD& dwVideoCompressorFourCC,
							int& nVideoCompressorKeyframesRate,
							float& fVideoCompressorQuality,
							LPWAVEFORMATEX pAudioCompressorWaveFormat,
							CWnd* pWnd,
							CWorkerThread* pThread,
							bool bShowMessageBoxOnError);

// Protected Variables
protected:
	// Attached View and Frame Pointers
	CVideoAviView* m_pView;
	CVideoAviChildFrame* m_pFrame;

	// Do Loop when reaching the end?
	volatile LONG m_bLoop;

	// Save FourCC
	DWORD m_dwVideoCompressorFourCC;

	// Processing Classes
	CSaveAs m_SaveAsProcessing;
	CExtractframes m_ExtractframesProcessing;
	CFileMergeAs m_FileMergeAsProcessing;
	CShrinkDocTo m_ShrinkDocToProcessing;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoAviDoc)
	public:
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	protected:
	virtual BOOL SaveModified();
	//}}AFX_VIRTUAL

#ifdef _DEBUG
public:
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	

// Generated message map functions
protected:
	//{{AFX_MSG(CVideoAviDoc)
	afx_msg void OnFileSaveAs();
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateEditDelete(CCmdUI* pCmdUI);	
	afx_msg void OnPlayAvi();
	afx_msg void OnUpdatePlayAvi(CCmdUI* pCmdUI);
	afx_msg void OnStopAvi();
	afx_msg void OnUpdateStopAvi(CCmdUI* pCmdUI);
	afx_msg void OnPlayLoop();
	afx_msg void OnUpdatePlayLoop(CCmdUI* pCmdUI);
	afx_msg void OnPlayFrameBack();
	afx_msg void OnUpdatePlayFrameBack(CCmdUI* pCmdUI);
	afx_msg void OnPlayFrameFront();
	afx_msg void OnUpdatePlayFrameFront(CCmdUI* pCmdUI);
	afx_msg void OnPlayFrameBackFast();
	afx_msg void OnUpdatePlayFrameBackFast(CCmdUI* pCmdUI);
	afx_msg void OnPlayFrameFrontFast();
	afx_msg void OnUpdatePlayFrameFrontFast(CCmdUI* pCmdUI);
	afx_msg void OnPlayFrameFirst();
	afx_msg void OnUpdatePlayFrameFirst(CCmdUI* pCmdUI);
	afx_msg void OnPlayFrameLast();
	afx_msg void OnUpdatePlayFrameLast(CCmdUI* pCmdUI);
	afx_msg void OnEditStop();
	afx_msg void OnUpdateEditStop(CCmdUI* pCmdUI);
	afx_msg void OnFileInfo();
	afx_msg void OnViewTimeposition();
	afx_msg void OnUpdateViewTimeposition(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileInfo(CCmdUI* pCmdUI);
	afx_msg void OnFileClose();
	afx_msg void OnPlayVol();
	afx_msg void OnUpdatePlayVol(CCmdUI* pCmdUI);
	afx_msg void OnPlayAudiovideoshift();
	afx_msg void OnUpdatePlayAudiovideoshift(CCmdUI* pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnPlayVfwcodecpriority();
	afx_msg void OnUpdatePlayVfwcodecpriority(CCmdUI* pCmdUI);
	afx_msg void OnPlayInternalcodecpriority();
	afx_msg void OnUpdatePlayInternalcodecpriority(CCmdUI* pCmdUI);
	afx_msg void OnFileSave();
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnEditSnapshot();
	afx_msg void OnUpdateEditSnapshot(CCmdUI* pCmdUI);
	afx_msg void OnEditRename();
	afx_msg void OnUpdateEditRename(CCmdUI* pCmdUI);
	afx_msg void OnFileExtractframes();
	afx_msg void OnUpdateFileExtractframes(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDEOAVIDOC_H__24956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_)
