#if !defined(AFX_PICTUREDOC_H__24956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_)
#define AFX_PICTUREDOC_H__24956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_

#pragma once

#include "uImagerDoc.h"
#include "WorkerThread.h"
#include "RotationFlippingDlg.h"
#include "MonochromeConversionDlg.h"
#include "SharpenDlg.h"
#include "SoftenDlg.h"
#include "SoftBordersDlg.h"
#include "OsdDlg.h"
#include "ImageInfoDlg.h"
#include "HLSDlgModeless.h"
#include "RedEyeDlg.h"
#include "GifAnimationThread.h"
#include "TryEnterCriticalSection.h"
#include "SortableFileFind.h"
#include "PaletteWnd.h"
#include "LayeredDlg.h"
#include <random>

//////////////////////////////////////////////
// NOTE:                                    //
// The Dib Critical Section is not used in  //
// the Picture Classes because all changing //
// operations to the dib are performed      //
// by the main UI Thread!                   //
//////////////////////////////////////////////

// Forward Declaration
class CPictureView;
class CPictureChildFrame;

// Default Save As file extension for new files
#define DEFAULT_SAVEAS_EXT										_T(".jpg")

// The Initial Compression value used for the Jpeg Compression Calculation
#define INIT_JPEG_COMPRESSION_CALCULATION						80

// CJpegThread waits that the Load Pictures
// Thread finishes, with a maximum wait time specified here in ms
#define JPEG_COMPRESSION_CALCULATION_THREADWAIT_TIMEOUT			5000U

// Load Full Jpeg Transition Delay in ms
#define JPEG_LOADFULL_TRANSITION_DELAY							200

// Full Jpeg Transition Critical Section Timeout in ms
#define CS_LOADFULLJPEGDIB_TIMEOUT								30

// Default Slideshow Delay in ms
#define DEFAULT_SLIDESHOW_DELAY									4000

// The Slideshow Thread checks each given ms
// to see whether the Picture has been loaded
#define SLIDESHOW_LOADPICTURE_WAIT								20

// System-wide Resolution of the Multimedia Timer
// A lower value increases the responsiveness of the system to periodic events.
// This increased responsiveness means the system scheduler must run more often,
// so that the system spends more time scheduling tasks, context switching, etc.
// This can ultimately reduce overall system performance, since every clock
// cycle the system is processing "system stuff" is a clock cycle that isn't
// being spent running this application.
// APIs:    timeBeginPeriod to set in ms units and NtSetTimerResolution / 
//          NtQueryTimerResolution to set / get in 100 ns units
//          (timeSetEvent calls timeBeginPeriod with the passed resolution)
// Default: 15.625 (1/64) ms
// Range:   0.5 .. 15.625 ms
// Tool:    Use Clockres.exe from sysinternals to read the current value
#define DEFAULT_TIMER_RESOLUTION								16

// Transition Delay in ms
#define TRANSITION_DELAY										60

// Transition Critical Section Timeout in ms
#define CS_TRANSITION_TIMEOUT									30

// Each given ms the Slideshow thread checks that the
// current dir still exists!
#define SLIDESHOW_DIRCHECK_TIMEOUT								2500U

// The Crop Background Image Brightness and Contrast
#define CROP_BKG_BRIGHTNESS										-110
#define CROP_BKG_CONTRAST										-40

// Layered dialog parameters
#define LAYERED_DLG_LEFTBORDER									0	// In pixels
#define LAYERED_DLG_RIGHTBORDER									0	// In pixels
#define LAYERED_DLG_TOPBORDER									0	// In pixels
#define LAYERED_DLG_BOTTOMBORDER								0	// In pixels
#define MIN_LAYERED_DLG_MAXSIZE_PERCENT							25	// m_nLayeredDlgMaxsizePercent init value
#define LAYERED_DLG_BOUNDARY_PERCENT							90	// Upper boundary value in percent of screen size before cropping
																	// (only for exact size settings <-> m_nLayeredDlgMaxsizePercent = 0)
#define MIN_LAYERED_DLG_OPACITY									51	// 20%
#define MAX_LAYERED_DLG_OPACITY									255	// 100%

// The Picture Document Class
class CPictureDoc : public CUImagerDoc
{
protected: // create from serialization only
	DECLARE_DYNCREATE(CPictureDoc)
	CPictureDoc();
	virtual ~CPictureDoc();

public:
	// The SlideShow Thread Class
	class CSlideShowThread : public CWorkerThread
	{
		public:
			CSlideShowThread();
			virtual ~CSlideShowThread();

			// Restart timer if it is running
			void RestartRunningTimer();

			// Run Slideshow
			void RunSlideshow();

			// Pause Slideshow
			void PauseSlideshow();

			// Is Slideshow Running?
			BOOL IsSlideshowRunning() {return ((m_uiSlideshowTimerId > 0) && IsRunning());};

			// Called by the Main UI Thread to go to the next picture
			void NextPicture();

			// Called by the Main UI Thread to go to the previous picture
			void PreviousPicture();

			// Called by the Main UI Thread to go to the first picture
			void FirstPicture();

			// Called by the Main UI Thread to go to the last picture
			void LastPicture();

			// Get / Set Functions
			void SetDoc(CPictureDoc* pDoc) {m_pDoc = pDoc;};
			void SetStartName(CString sStartName) {m_sStartName = sStartName;};
			void SetMilliSecondsDelay(int nMilliSecondsDelay) {	m_nMilliSecondsDelay = nMilliSecondsDelay;
																if (IsSlideshowRunning()) RunSlideshow();};
			int  GetMilliSecondsDelay() {return m_nMilliSecondsDelay;};
			void SetRecursive(BOOL bRecursive) {m_bRecursive = bRecursive;};
			void SetLoop(BOOL bLoop) {m_bLoop = bLoop;};
			void SetRandom(BOOL bRandom) {m_bRandom = bRandom;};
			
			// Is Functions
			BOOL IsRecursive() const {return m_bRecursive;};
			BOOL IsLoop() const {return m_bLoop;};
			BOOL IsRandom() const {return m_bRandom;};

			// The Slideshow Timer Event Handle
			HANDLE m_hSlideshowTimerEvent;

			// Slideshow Load Picture Done Flag
			volatile BOOL m_bSlideshowLoadPictureDone;

			// If set after picture load the
			// RunSlideshow() function is called again!
			BOOL m_bDoRunSlideshow;

		protected:
			// Worker Thread Entry
			int Work();

			// Exiting Thread When an Error Occurs
			int OnError();

			// SlideShow returns FALSE if an error occurs and
			// TRUE if it's time to Shutdown the Thread
			BOOL SlideShow(CString sStartFileName);

			// Posts a Message to the Main UI Thread to load the picture
			void LoadPicture(LPCTSTR sFileName, BOOL bNext);

			// The Slideshow Timer
			UINT m_uiSlideshowTimerId;

			// Next, Previous, First and Last Picture Event Handles,
			// triggered by NextPicture(), PreviousPicture(),
			// FirstPicture() and LastPicture()
			HANDLE m_hNextPictureEvent;
			HANDLE m_hPreviousPictureEvent;
			HANDLE m_hFirstPictureEvent;
			HANDLE m_hLastPictureEvent;

			// Events
			HANDLE m_hEventArray[6];

			// The Doc Pointer
			CPictureDoc* m_pDoc;

			// File Name or Dir Name
			CString m_sStartName;

			// Slideshow Delay
			volatile int m_nMilliSecondsDelay;

			// Recursive Slideshow?
			volatile BOOL m_bRecursive;

			// Loop Slideshow?
			volatile BOOL m_bLoop;

			// Random Slideshow?
			volatile BOOL m_bRandom;
	};

	// The LayeredDlgThread Thread Class
	class CLayeredDlgThread : public CWorkerThread
	{
		public:
			CLayeredDlgThread() {m_pDoc = NULL; m_nMaxsizePercent = MIN_LAYERED_DLG_MAXSIZE_PERCENT; m_nSizePerthousand = 0;};
			virtual ~CLayeredDlgThread() {Kill();};

			// Get / Set Functions
			void SetDoc(CPictureDoc* pDoc) {m_pDoc = pDoc;};

			// Resize and display
			BOOL DoIt(CWorkerThread* pThread = NULL); 

			// Dib to resize and show
			CDib m_Dib;

			// Resize factors
			int m_nMaxsizePercent;
			int m_nSizePerthousand;

		protected:
			// Worker Thread Entry
			int Work() {DoIt(this); return 0;};

			// The Doc Pointer
			CPictureDoc* m_pDoc;
	};

	// The Calculate Jpeg Compression Quality
	// and load the full sized jpeg Thread Class
	class CJpegThread : public CWorkerThread
	{
		public:
			CJpegThread(){	m_pDoc = NULL;
							m_lJpegCompressionQuality = -1;
							m_bDoFullLoad = FALSE;
							m_sFileName = _T("");
							m_hTimerEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);};
			virtual ~CJpegThread(){Kill(); ::CloseHandle(m_hTimerEvent);};
			void SetDoc(CPictureDoc* pDoc) {m_pDoc = pDoc;};
			__forceinline void SetDoFullLoad(BOOL bDoFullLoad) { m_bDoFullLoad = bDoFullLoad; };
			__forceinline void SetFileName(CString sFileName) {m_sFileName = sFileName;};
			__forceinline void ResetJpegCompressionQuality() {::InterlockedExchange(&m_lJpegCompressionQuality, -1);};
			__forceinline int GetJpegCompressionQuality() const {return (int)m_lJpegCompressionQuality;};
			__forceinline int GetJpegCompressionQualityBlocking() {WaitDone_Blocking(); return (int)m_lJpegCompressionQuality;};

		protected:
			int Work();
			void OnExit();
			void CleanUp();
			CPictureDoc* m_pDoc;
			HANDLE m_hEventArray[2];
			HANDLE m_hTimerEvent;
			volatile LONG m_lJpegCompressionQuality;
			volatile BOOL m_bDoFullLoad;
			CString m_sFileName;
	};

	// The Load Next & Previous Picture Thread Class
	class CLoadPicturesThread : public CWorkerThread
	{
		public:
			CLoadPicturesThread(){m_pDoc = NULL;};
			virtual ~CLoadPicturesThread(){Kill();};
			void SetDoc(CPictureDoc* pDoc) {m_pDoc = pDoc;};
			BOOL InitFileNames();

		protected:
			int Work();
			CPictureDoc* m_pDoc;
	};

	// The Gif Animation Thread Class
	class CMyGifAnimationThread : public CGifAnimationThread
	{
		public:
			CMyGifAnimationThread() {m_pDoc = NULL;};
			void SetDoc(CPictureDoc* pDoc) {m_pDoc = pDoc;};

		protected:
			CPictureDoc* m_pDoc;
			virtual void OnNewFrame();
			virtual void OnPlayTimesDone();
	};

	// The Picture Transition Effect Thread Class
	class CTransitionThread : public CWorkerThread
	{
		public:
			CTransitionThread();
			virtual ~CTransitionThread();
			void SetDoc(CPictureDoc* pDoc) {m_pDoc = pDoc;};

		protected:
			void OnExit();
			int Work();
			CPictureDoc* m_pDoc;
			HANDLE m_hTimerEvent;
			HANDLE m_hEventArray[2];
			std::mt19937 m_PseudoRandomTransitionType;
			std::mt19937 m_PseudoRandomTransitionChessX;
			std::mt19937 m_PseudoRandomTransitionChessY;
			BOOL m_bPseudoRandomInited; // pseudo-randoms are initialized on first thread run
	};

	// Alpha Blend Transition Function Type
	typedef BOOL (WINAPI * FPALPHABLEND)(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION);

// Public Functions
public:
	void CloseDocument();			// Close Document by sending a WM_CLOSE to the Parent Frame
	void CloseDocumentForce();		// Closes Without Asking To Save!

	// Check whether the dib has been fully loaded,
	// if not the command is scheduled for execution
	// after the full load.
	BOOL IsDibReadyForCommand(DWORD dwCommand);

	// Enable Command?
	BOOL DoEnableCommand();

	// Set The Document Title
	void SetDocumentTitle();

	// Setings
	void LoadSettings();
	
	// Get / Set View / Frame Functions
	CPictureView* GetView() const {return m_pView;};
	void SetView(CPictureView* pView) {m_pView = pView;};
	CPictureChildFrame* GetFrame() const {return m_pFrame;};
	void SetFrame(CPictureChildFrame* pFrame) {m_pFrame = pFrame;};

	// Do Show MessageBox on Error?
	BOOL SetShowMessageBoxOnError(BOOL bShow) {	BOOL res = m_bShowMessageBoxOnError; 
												m_bShowMessageBoxOnError = bShow;
												if (m_pDib) m_pDib->SetShowMessageBoxOnError(bShow);
												if (m_pNextDib) m_pNextDib->SetShowMessageBoxOnError(bShow);
												if (m_pPrevDib) m_pPrevDib->SetShowMessageBoxOnError(bShow);
												m_AlphaRenderedDib.SetShowMessageBoxOnError(bShow);
												return res;};
	BOOL IsShowMessageBoxOnError() const {return m_bShowMessageBoxOnError;};

	// Undo / Redo Functions
	void ClearUndoArray();
	CDib* AddUndo(CDib* pDib = NULL);	// If NULL uses m_pDib. Returns the new allocated dib pointer
	void Undo(BOOL bUpdate = TRUE);		// Always uses m_pDib, if TRUE updates the View
	void Redo(BOOL bUpdate = TRUE);		// Always uses m_pDib, if TRUE updates the View

	// Multi-page tiff check function
	__forceinline BOOL IsMultiPageTIFF()
			{return 	m_pDib &&
						m_pDib->m_FileInfo.m_nImageCount > 1 &&
						CDib::IsTIFF(m_sFileName);};

	// Gets the Pixel Alignment and starts the Jpeg Thread
	// (compression calculation and full size jpeg load)
	void JPEGGet();

	// Losslessly Rotate-Flip jpeg
	//
	// Return values:
	// 0 = save before doing a lossless transformation
	// 1 = ok done
	// 2 = cannot do a lossless transformation
	int LossLessRotateFlip(BOOL bShowMessageBoxOnError, CRotationFlippingDlg* pDlg);

	// Update Alpha Rendered Dib
	void UpdateAlphaRenderedDib();

	// Update the Image Info Dialog
	void UpdateImageInfo(BOOL bUpdateFileInfoOnly = FALSE);

	// Delete Current Page or Picture and Load Next One
	void EditDelete(BOOL bPrompt);

	// Clear the Previous and Next Loaded Pictures
	// Used in Save As (to avoid the save as file is the next or previous),
	// Crop, FileMoveTo, EditRename, DeleteDocFile
	void ClearPrevNextPictures();

	// Create Preview Dib from pDib
	// (use the Monitor Size to decide the Preview Size)
	static BOOL CreatePreviewDib(CDib* pDib);

	// Picture Loading Function
	BOOL LoadPicture(	CDib *volatile *ppDib,
						CString sFileName,
						BOOL bLoadOnly = FALSE,
						BOOL bDoPrevNextPreload = TRUE,
						BOOL bOnlyHeader = FALSE);
	
	// Picture Slideshow
	BOOL SlideShow(	BOOL bRecursive,
					BOOL bRunSlideshow);

	// Compress / Decompress RLE Bmp's
	// nCompression may be:
	// BI_RGB  : Decompress
	// BI_RLE8 : Compress to RLE8
	// BI_RLE4 : Compress to RLE4
	BOOL CompressRLE(int nCompression);

	// Edit Palette
	BOOL EditPalette();

	// Rotations
	BOOL Rotate90cw();
	BOOL Rotate90ccw();
	BOOL Rotate180();

	// Resize
	BOOL EditResize(BOOL bShowMessageBoxOnError);

	// Grayscale
	BOOL EditGrayscale(BOOL bShowMessageBoxOnError);

	// Negative
	BOOL EditNegative(BOOL bShowMessageBoxOnError);
	
	// Borders Add
	BOOL EditAddBorders();

	// Layered Dialog
	BOOL ViewLayeredDlg();

	// Paste into
	// nCorner:      0 Top-Left
	// nCorner:      1 Top-Right
	// nCorner:      2 Bottom-Left
	// nCorner:      3 Bottom-Right
	// pt:           in top-down client coordinates if bClientCoordinates set,
	//               otherwise in top-down dib pixels
	// nAlphaOffset: -255 .. +255
	BOOL EditPasteInto(int nCorner, CPoint pt, int nAlphaOffset = 0, BOOL bClientCoordinates = TRUE);

	// Unique Colors Count
	BOOL EditColorsCount();

	// Is the Clicked Point Inside a Red-Eye?
	BOOL IsClickPointRedEye(CPoint point, CDib* pSrcDib);

	// Transitions
	static BOOL TransitionChess(HDC hSrcDC, int xs, int ys,
							HDC hDestDC, int xd, int yd,
							int width, int height,
							int nStep, int nMaxSteps,
							std::mt19937& PseudoRandomX,
							std::mt19937& PseudoRandomY);

	static BOOL TransitionLRCurtain(HDC hSrcDC, int xs, int ys,
							HDC hDestDC, int xd, int yd,
							int width, int height,
							int nStep, int nMaxSteps);

	static BOOL TransitionLRRoll(HDC hSrcDC, int xs, int ys,
							HDC hDestDC, int xd, int yd,
							int width, int height,
							int nStep, int nMaxSteps);
	static BOOL TransitionLRSlide(HDC hSrcDC, int xs, int ys,
							HDC hDestDC, int xd, int yd,
							int width, int height,
							int nStep, int nMaxSteps);
	BOOL TransitionBlend(HDC hSrcDC, int xs, int ys,
							HDC hDestDC, int xd, int yd,
							int width, int height,
							int nStep);
	BOOL TransitionLoadFullJpeg(HDC hSrcDC, int xs, int ys,
								HDC hDestDC, int xd, int yd,
								int width, int height,
								int nStep);

	// Cancel Transition
	void CancelTransition() {	m_bTransitionUI = FALSE;
								m_TransitionThread.Kill_NoBlocking();};

	// Cancel Load Full Jpeg Transition
	void CancelLoadFullJpegTransition() {	if (m_bCancelLoadFullJpegTransitionAllowed)
												m_bCancelLoadFullJpegTransition = TRUE;};

	// Crop/Copy/Cut
	void CancelCrop();
	void DoCropRect();

	// Show Page/Frame
	BOOL ViewFirstPageFrame();
	BOOL ViewLastPageFrame();
	BOOL ViewNextPageFrame();
	BOOL ViewPreviousPageFrame();

	// View position in google map
	void ViewMap();

	// Enable / Disable Borders
	void ViewNoBorders();

	// Show Background Color Dialog
	void ViewBackgroundColorDlg();

	// Enable / disable zoom tool
	void ViewZoomTool();
	void CancelZoomTool();

	// Show / Hide OSD Dialog
	void ShowOsd(BOOL bShow);

	// Show / Hide File Info Dialog
	void FileInfo();

	// Enable / Disable Halftone Stretch Mode
	void StretchHalftone();

	// Save
	BOOL Save();

	// Save As
	// If bSaveCopyAs is TRUE a new document is opened with the saved file.
	// If bSaveCopyAs is FALSE the current document is cleared and the
	// saved file is opened in it.
	BOOL SaveAs(BOOL bSaveCopyAs,					
				CString sDlgTitle = _T(""));		// Open File Dialog Title
													// If _T("") the default is used

	// Save as GIF is taking into account:
	// - Alpha channel to transparency conversion
	// - Conversion to 8bpp
	static BOOL SaveGIF(const CString& sFileName,			
						CDib* pDib,
						CWnd* pProgressWnd = NULL,
						BOOL bProgressSend = TRUE,
						CWorkerThread* pThread = NULL);

	// Save as PNG is taking into account:
	// - if more than two alpha levels save to RGBA,
	//   otherwise use transparency conversion
	//   but only if 256 or less colors.
	// - Conversion to 8bpp or less if 256 or less colors
	static BOOL SavePNG(	const CString& sFileName,
							CDib* pDib,
							BOOL bStoreBackgroundColor,
							CWnd* pProgressWnd = NULL,
							BOOL bProgressSend = TRUE,
							CWorkerThread* pThread = NULL);

	// Update Layered Dialog with given Dib
	BOOL UpdateLayeredDlg(CDib* pDib);

// Public Variables
public:
	CDib m_AlphaRenderedDib;		// Alpha Rendered With Background Color
	CDib* volatile m_pNextDib;		// Next Picture
	CDib* volatile m_pPrevDib;		// Previous Picture
	CString m_sNextDibName;			// Next Picture Name
	CString m_sPrevDibName;			// Previous Picture Name
	CString m_sDirName;				// Current Directory Name
	CString m_sCopyOrMoveDirName;	// Current Copy or Move Directory Name
	CSortableFileFind m_FileFind;	// File Find Object
	CDib* m_pCropBkgDib;			// The darker dib for cropping
	CDib* volatile m_pLoadFullJpegDib;// Used To Load the Full Size Jpeg
	CTryEnterCriticalSection m_csLoadFullJpegDib;
	volatile DWORD m_dwIDAfterFullLoadCommand;// PostMessage a WM_COMMAND of the given ID
	BOOL m_bMetadataModified;		// Metadata have been modified
	BOOL m_nPageNum;				// Current displayed page number of a multi-page TIFF file

	// Threads
	CSlideShowThread m_SlideShowThread; // Thread which shows pictures in an automated sequence
	CJpegThread m_JpegThread;			// Thread which calculates the jpeg compression quality
										// and loads the full sized jpeg
	CLoadPicturesThread m_LoadPicturesThread;
	CLayeredDlgThread m_LayeredDlgThread; // Thread which shrinks the current picture for layered display
	CMyGifAnimationThread m_GifAnimationThread;

	// Do Not Draw Flag
	volatile BOOL m_bNoDrawing;

	// Transition Thread and Vars
	CTransitionThread m_TransitionThread;
	CTryEnterCriticalSection m_csTransition;
	volatile int m_nTransitionType;		// 0 -> No Transition, 1 -> Random, 2.. -> Transitions
	volatile BOOL m_bTransitionUI;
	volatile BOOL m_bTransitionWorker;
	
	// Load Full Jpeg Transition Vars
	// work happening in CJpegThread
	volatile BOOL m_bLoadFullJpegTransitionUI;
	volatile BOOL m_bLoadFullJpegTransitionWorker;
	volatile BOOL m_bLoadFullJpegTransitionWorkerDone;
	volatile BOOL m_bFirstLoadFullJpegTransition;
	volatile BOOL m_bCancelLoadFullJpegTransition;
	volatile BOOL m_bCancelLoadFullJpegTransitionAllowed;

	// OSD Dialog
	BOOL m_bEnableOsd;
	BOOL m_bOSDWasEnabled; // Before Print Preview OSD was enabled flag
	COsdDlg* m_pOsdDlg;

	// The Image Info Dialog
	CImageInfoDlg* m_pImageInfoDlg;
	
	// Transparency
	typedef BOOL (WINAPI *lpfnSetLayeredWindowAttributes)
					(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
	lpfnSetLayeredWindowAttributes m_pSetLayeredWindowAttributes;
	typedef BOOL (WINAPI *lpfnUpdateLayeredWindow)
					(HWND hwnd,
					HDC hdcDst,
					POINT *pptDst,
					SIZE *psize,
					HDC hdcSrc,
					POINT *pptSrc,
					COLORREF crKey,
					BLENDFUNCTION *pblend,
					DWORD dwFlags);
	lpfnUpdateLayeredWindow m_pUpdateLayeredWindow;

	// Brightness, Contrast, ... Dialog
	CHLSDlgModeless* m_pHLSDlg;

	// Palette Wnd
	CPaletteWnd* m_pWndPalette;

	// Rotation / Flipping Dialog
	CRotationFlippingDlg* m_pRotationFlippingDlg;
	BOOL m_bDoRotationColorPickup;

	// RedEye Reduction Dialog
	CRedEyeDlg* m_pRedEyeDlg;
	BOOL m_bDoRedEyeColorPickup;

	// Monochrome Conversion Dialog
	CMonochromeConversionDlg* m_pMonochromeConversionDlg;

	// Sharpen Dialog
	CSharpenDlg* m_pSharpenDlg;

	// Soften Dialog
	CSoftenDlg* m_pSoftenDlg;

	// Soft Borders Dialog
	CSoftBordersDlg* m_pSoftBordersDlg;

	// Layered Dialog
	CLayeredDlg* volatile m_pLayeredDlg;
	volatile BOOL m_bDoUpdateLayeredDlg;
	BOOL m_bFirstLayeredDlgUpdate;
	int m_nLayeredDlgWidth;
	int m_nLayeredDlgHeight;
	volatile int m_nLayeredDlgMaxsizePercent;	// 1 .. 100
	volatile int m_nLayeredDlgSizePerthousand;	// 1 .. 16000
	volatile int m_nLayeredDlgOrigin;			// 0 .. 3
	volatile int m_nLayeredDlgOpacity;			// 0 .. 255

	// Force Lossy Jpeg Transformations
	BOOL m_bForceLossyTrafo;

	// Make Lossless Jpeg Crop
	BOOL m_bLosslessCrop;

	// Get Pixel Align, Compression
	// and load full sized jpeg.
	// Note: Slideshow disables this!
	volatile BOOL m_bDoJPEGGet;

	// If the Image Has a Defined Background Color,
	// in all the code m_crImageBackgroundColor
	// is used instead of m_crBackgroundColor!
	BOOL m_bImageBackgroundColor;
	COLORREF m_crImageBackgroundColor;

	// Restart Paused Slideshow With Menu Closing
	BOOL m_bDoRestartSlideshow;

	// AlphaBlend Function Pointer,
	// included in msimg32.dll,
	// only available for win98, win2000 and higher
	FPALPHABLEND m_fpAlphaBlend;

	// The Minimum Unit in pixels for an image,
	// usually it is 1 for none jpeg files.
	// For jpeg files it is either 8 or 16.
	// This is used by the lossless jpeg cropping.
	int m_nPixelAlignX, m_nPixelAlignY;

	// Printing flag
	BOOL m_bPrinting;

	// Print Preview Flag
	BOOL m_bPrintPreviewMode;

	// Crop
	CRect m_CropDocRect;		// The Crop Document Size
	BOOL m_bCrop;				// Crop Tool Enabled
	CRect m_rcCropDelta;		// Amount of Cropping
	CRect m_rcCropCenter;		// Crop State When Left Button
								// Pressed For Centered Cropping

	// Paint Stretch Mode Flag
	BOOL m_bStretchModeHalftone;

	// Print
	double m_dPrintScale;		// 1.0 is 100%
	BOOL m_bPrintSizeFit;		// If TRUE the image is fitted inside the print page
	BOOL m_bPrintMargin;		// Print Margin
	CPoint m_ptPrintOffset;		// Print Offset Position
	CPoint m_ptLastPrintOffset;	// Last Print Offset

	// Loaded from Settings Zoom ComboBox Index
	int m_nZoomComboBoxIndex;

	// ZoomTool Vars
	BOOL m_bZoomTool;
	BOOL m_bZoomToolMinus;

// Protected Functions
protected:
	// Delete Current File
	BOOL DeleteDocFile();

	// Save File
	BOOL SaveAsPdf();
	BOOL SaveAsFromAnimGIF(	BOOL bSaveCopyAs,
							CString sDlgTitle = _T(""));
	BOOL SaveAsFromAnimGIFToAnimGIF(const CString& sFileName,
									UINT uiPlayTimes,
									CArray<int, int>* pDelaysArray);
	CString ExtractFromAnimGIFToBMP(const CString& sFileName);

	// Crop/Copy/Cut
	void EditCrop();
	void DoCopyRect();
	void DoCutRect();
	BOOL CopyDelCrop(BOOL bShowMessageBoxOnError, BOOL bCopy, BOOL bDel, BOOL bCrop);

// Protected Variables
protected:
	// Undo / Redo Vars
	BOOL m_bRedo;				// Has Redo
	CDib::ARRAY m_DibUndoArray;	// The Undo / Redo Array of Dibs
	int m_nDibUndoPos;			// Current Undo Pos

	// This Pointers point to the first view and first frame,
	// this works because in the current implementation
	// we always have only one view and one frame!
	CPictureView* m_pView;		// The Only Attached View
	CPictureChildFrame* m_pFrame;// The Only Attached Frame

	// Do Show Message Box on Dib Errors:
	// like loading / saving errors.
	// Disabled by Slideshow!
	volatile BOOL m_bShowMessageBoxOnError;

	// Do Dither when color converting from
	// RGB to 256 or less colors
	BOOL m_bDitherColorConversion;

	// The Maximum Number of Colors to use when
	// converting from RGB to 8bpp
	BOOL m_uiMaxColors256;

	// The Maximum Number of Colors to use when
	// converting from RGB to 4bpp
	BOOL m_uiMaxColors16;

	// msimg32.dll for AlphaBlend Function
	HINSTANCE m_hMSIMG32;

	// user32.dll for Window Transparency
	HINSTANCE m_hUser32;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPictureDoc)
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

protected:
	//{{AFX_MSG(CPictureDoc)
	afx_msg void OnFileSave();
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveAs();
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateEditDelete(CCmdUI* pCmdUI);
	afx_msg void OnSlideshowDelay2();
	afx_msg void OnUpdateSlideshowDelay2(CCmdUI* pCmdUI);
	afx_msg void OnSlideshowDelay3();
	afx_msg void OnUpdateSlideshowDelay3(CCmdUI* pCmdUI);
	afx_msg void OnSlideshowDelay4();
	afx_msg void OnUpdateSlideshowDelay4(CCmdUI* pCmdUI);
	afx_msg void OnSlideshowDelay5();
	afx_msg void OnUpdateSlideshowDelay5(CCmdUI* pCmdUI);
	afx_msg void OnSlideshowDelay10();
	afx_msg void OnUpdateSlideshowDelay10(CCmdUI* pCmdUI);
	afx_msg void OnSlideshowDelay30();
	afx_msg void OnUpdateSlideshowDelay30(CCmdUI* pCmdUI);
	afx_msg void OnSlideshowDelay60();
	afx_msg void OnUpdateSlideshowDelay60(CCmdUI* pCmdUI);
	afx_msg void OnEditRotate90cw();
	afx_msg void OnUpdateEditRotate90cw(CCmdUI* pCmdUI);
	afx_msg void OnEditRotate90ccw();
	afx_msg void OnUpdateEditRotate90ccw(CCmdUI* pCmdUI);
	afx_msg void OnEditRotateFlip();
	afx_msg void OnUpdateEditRotateFlip(CCmdUI* pCmdUI);
	afx_msg void OnEditResize();
	afx_msg void OnUpdateEditResize(CCmdUI* pCmdUI);
	afx_msg void OnEditHls();
	afx_msg void OnUpdateEditHls(CCmdUI* pCmdUI);
	afx_msg void OnEditGrayscale();
	afx_msg void OnUpdateEditGrayscale(CCmdUI* pCmdUI);
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnPlayPlay();
	afx_msg void OnUpdatePlayPlay(CCmdUI* pCmdUI);
	afx_msg void OnPlayStop();
	afx_msg void OnUpdatePlayStop(CCmdUI* pCmdUI);
	afx_msg void OnPlayLoop();
	afx_msg void OnUpdatePlayLoop(CCmdUI* pCmdUI);
	afx_msg void OnViewNextPicture();
	afx_msg void OnViewPreviousPicture();
	afx_msg void OnFileInfo();
	afx_msg void OnUpdateFileInfo(CCmdUI* pCmdUI);
	afx_msg void OnPlayTransitionOff();
	afx_msg void OnUpdatePlayTransitionOff(CCmdUI* pCmdUI);
	afx_msg void OnPlayTransitionRandom();
	afx_msg void OnUpdatePlayTransitionRandom(CCmdUI* pCmdUI);
	afx_msg void OnPlayTransitionCurtainin();
	afx_msg void OnUpdatePlayTransitionCurtainin(CCmdUI* pCmdUI);
	afx_msg void OnPlayTransitionRoolin();
	afx_msg void OnUpdatePlayTransitionRoolin(CCmdUI* pCmdUI);
	afx_msg void OnPlayTransitionSlidein();
	afx_msg void OnUpdatePlayTransitionSlidein(CCmdUI* pCmdUI);
	afx_msg void OnPlayTransitionChessboard();
	afx_msg void OnUpdatePlayTransitionChessboard(CCmdUI* pCmdUI);
	afx_msg void OnEditCrop();
	afx_msg void OnUpdateEditCrop(CCmdUI* pCmdUI);
	afx_msg void OnEditCropApply();
	afx_msg void OnUpdateEditCropApply(CCmdUI* pCmdUI);
	afx_msg void OnEditCropCancel();
	afx_msg void OnUpdateEditCropCancel(CCmdUI* pCmdUI);
	afx_msg void OnEditTo24bits();
	afx_msg void OnUpdateEditTo24bits(CCmdUI* pCmdUI);
	afx_msg void OnEditTo32bits();
	afx_msg void OnUpdateEditTo32bits(CCmdUI* pCmdUI);
	afx_msg void OnEditTo16bits();
	afx_msg void OnUpdateEditTo16bits(CCmdUI* pCmdUI);
	afx_msg void OnEditTo1bit();
	afx_msg void OnUpdateEditTo1bit(CCmdUI* pCmdUI);
	afx_msg void OnPlayAnimation();
	afx_msg void OnUpdatePlayAnimation(CCmdUI* pCmdUI);
	afx_msg void OnPlayStopAnimation();
	afx_msg void OnUpdatePlayStopAnimation(CCmdUI* pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnViewNextPageFrame();
	afx_msg void OnUpdateViewNextPageFrame(CCmdUI* pCmdUI);
	afx_msg void OnViewPreviousPageFrame();
	afx_msg void OnUpdateViewPreviousPageFrame(CCmdUI* pCmdUI);
	afx_msg void OnEditTo1bitDitherErrDiff();
	afx_msg void OnUpdateEditTo1bitDitherErrDiff(CCmdUI* pCmdUI);
	afx_msg void OnEditTo8bits();
	afx_msg void OnUpdateEditTo8bits(CCmdUI* pCmdUI);
	afx_msg void OnEditTo4bits();
	afx_msg void OnUpdateEditTo4bits(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnPlayTransitionBlend();
	afx_msg void OnUpdatePlayTransitionBlend(CCmdUI* pCmdUI);
	afx_msg void OnEditRedeye();
	afx_msg void OnUpdateEditRedeye(CCmdUI* pCmdUI);
	afx_msg void OnEditUpdateExifthumb();
	afx_msg void OnUpdateEditUpdateExifthumb(CCmdUI* pCmdUI);
	afx_msg void OnEditAddExifthumb();
	afx_msg void OnUpdateEditAddExifthumb(CCmdUI* pCmdUI);
	afx_msg void OnEditRemoveExifthumb();
	afx_msg void OnUpdateEditRemoveExifthumb(CCmdUI* pCmdUI);
	afx_msg void OnEditRemoveExif();
	afx_msg void OnUpdateEditRemoveExif(CCmdUI* pCmdUI);
	afx_msg void OnEditRemoveJfif();
	afx_msg void OnUpdateEditRemoveJfif(CCmdUI* pCmdUI);
	afx_msg void OnEditRemoveOtherApp();
	afx_msg void OnUpdateEditRemoveOtherApp(CCmdUI* pCmdUI);
	afx_msg void OnEditRemoveCom();
	afx_msg void OnUpdateEditRemoveCom(CCmdUI* pCmdUI);
	afx_msg void OnEditClearExifOrientate();
	afx_msg void OnUpdateEditClearExifOrientate(CCmdUI* pCmdUI);
	afx_msg void OnBackgroundColor();
	afx_msg void OnUpdateBackgroundColor(CCmdUI* pCmdUI);
	afx_msg void OnFileClose();
	afx_msg void OnEditCropLossless();
	afx_msg void OnUpdateEditCropLossless(CCmdUI* pCmdUI);
	afx_msg void OnViewStretchHalftone();
	afx_msg void OnUpdateViewStretchHalftone(CCmdUI* pCmdUI);
	afx_msg void OnEditFilterSharpen();
	afx_msg void OnUpdateEditFilterSharpen(CCmdUI* pCmdUI);
	afx_msg void OnEditFilterSoften();
	afx_msg void OnUpdateEditFilterSoften(CCmdUI* pCmdUI);
	afx_msg void OnEditRemoveIptc();
	afx_msg void OnUpdateEditRemoveIptc(CCmdUI* pCmdUI);
	afx_msg void OnViewBackgroundColorMenu();
	afx_msg void OnUpdateViewBackgroundColorMenu(CCmdUI* pCmdUI);
	afx_msg void OnEditAddBorders();
	afx_msg void OnUpdateEditAddBorders(CCmdUI* pCmdUI);
	afx_msg void OnEditSoftBorders();
	afx_msg void OnUpdateEditSoftBorders(CCmdUI* pCmdUI);
	afx_msg void OnEditNegative();
	afx_msg void OnUpdateEditNegative(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveCopyAs();
	afx_msg void OnUpdateFileSaveCopyAs(CCmdUI* pCmdUI);
	afx_msg void OnEditRemoveXmp();
	afx_msg void OnUpdateEditRemoveXmp(CCmdUI* pCmdUI);
	afx_msg void OnEditRemoveIcc();
	afx_msg void OnUpdateEditRemoveIcc(CCmdUI* pCmdUI);
	afx_msg void OnViewEnableOsd();
	afx_msg void OnUpdateViewEnableOsd(CCmdUI* pCmdUI);
	afx_msg void OnOsdBkgcolorImage();
	afx_msg void OnOsdBkgcolorSelect();
	afx_msg void OnOsdFontsizeSmall();
	afx_msg void OnUpdateOsdFontsizeSmall(CCmdUI* pCmdUI);
	afx_msg void OnOsdFontsizeMedium();
	afx_msg void OnUpdateOsdFontsizeMedium(CCmdUI* pCmdUI);
	afx_msg void OnOsdFontsizeBig();
	afx_msg void OnUpdateOsdFontsizeBig(CCmdUI* pCmdUI);
	afx_msg void OnOsdOfftimeout3();
	afx_msg void OnUpdateOsdOfftimeout3(CCmdUI* pCmdUI);
	afx_msg void OnOsdOfftimeout4();
	afx_msg void OnUpdateOsdOfftimeout4(CCmdUI* pCmdUI);
	afx_msg void OnOsdOfftimeout5();
	afx_msg void OnUpdateOsdOfftimeout5(CCmdUI* pCmdUI);
	afx_msg void OnOsdOfftimeout6();
	afx_msg void OnUpdateOsdOfftimeout6(CCmdUI* pCmdUI);
	afx_msg void OnOsdOfftimeout7();
	afx_msg void OnUpdateOsdOfftimeout7(CCmdUI* pCmdUI);
	afx_msg void OnOsdOfftimeoutInfinite();
	afx_msg void OnUpdateOsdOfftimeoutInfinite(CCmdUI* pCmdUI);
	afx_msg void OnOsdFontcolorSelect();
	afx_msg void OnUpdateOsdBkgcolorImage(CCmdUI* pCmdUI);
	afx_msg void OnOsdOpacity100();
	afx_msg void OnUpdateOsdOpacity100(CCmdUI* pCmdUI);
	afx_msg void OnOsdOpacity80();
	afx_msg void OnUpdateOsdOpacity80(CCmdUI* pCmdUI);
	afx_msg void OnOsdOpacity60();
	afx_msg void OnUpdateOsdOpacity60(CCmdUI* pCmdUI);
	afx_msg void OnOsdOpacity40();
	afx_msg void OnUpdateOsdOpacity40(CCmdUI* pCmdUI);
	afx_msg void OnOsdDisplayDate();
	afx_msg void OnUpdateOsdDisplayDate(CCmdUI* pCmdUI);
	afx_msg void OnOsdDisplayLocation();
	afx_msg void OnUpdateOsdDisplayLocation(CCmdUI* pCmdUI);
	afx_msg void OnOsdDisplayHeadlineDescription();
	afx_msg void OnUpdateOsdDisplayHeadlineDescription(CCmdUI* pCmdUI);
	afx_msg void OnOsdDisplayFlash();
	afx_msg void OnUpdateOsdDisplayFlash(CCmdUI* pCmdUI);
	afx_msg void OnOsdDisplayExposuretime();
	afx_msg void OnUpdateOsdDisplayExposuretime(CCmdUI* pCmdUI);
	afx_msg void OnOsdDisplayAperture();
	afx_msg void OnUpdateOsdDisplayAperture(CCmdUI* pCmdUI);
	afx_msg void OnOsdClose();
	afx_msg void OnOsdDisplayFilename();
	afx_msg void OnUpdateOsdDisplayFilename(CCmdUI* pCmdUI);
	afx_msg void OnOsdDisplaySizescompression();
	afx_msg void OnUpdateOsdDisplaySizescompression(CCmdUI* pCmdUI);
	afx_msg void OnOsdDefaults();
	afx_msg void OnOsdDisplayMetadatadate();
	afx_msg void OnUpdateOsdDisplayMetadatadate(CCmdUI* pCmdUI);
	afx_msg void OnOsdAutosize();
	afx_msg void OnUpdateOsdAutosize(CCmdUI* pCmdUI);
	afx_msg void OnEditTo15bits();
	afx_msg void OnUpdateEditTo15bits(CCmdUI* pCmdUI);
	afx_msg void OnViewNoBorders();
	afx_msg void OnUpdateViewNoBorders(CCmdUI* pCmdUI);
	afx_msg void OnUpdateOsdDefaults(CCmdUI* pCmdUI);
	afx_msg void OnUpdateOsdFontcolorSelect(CCmdUI* pCmdUI);
	afx_msg void OnUpdateOsdBkgcolorSelect(CCmdUI* pCmdUI);
	afx_msg void OnEditPalette();
	afx_msg void OnUpdateEditPalette(CCmdUI* pCmdUI);
	afx_msg void OnViewLayeredDlg();
	afx_msg void OnUpdateViewLayeredDlg(CCmdUI* pCmdUI);
	afx_msg void OnLayereddlgMaxsize066();
	afx_msg void OnLayereddlgMaxsize050();
	afx_msg void OnLayereddlgMaxsize033();
	afx_msg void OnLayereddlgMaxsize025();
	afx_msg void OnLayereddlgOpacity100();
	afx_msg void OnLayereddlgOpacity80();
	afx_msg void OnLayereddlgOpacity60();
	afx_msg void OnLayereddlgOpacity40();
	afx_msg void OnLayereddlgClose();
	afx_msg void OnUpdateLayereddlgMaxsize066(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLayereddlgMaxsize050(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLayereddlgMaxsize033(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLayereddlgMaxsize025(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLayereddlgOpacity100(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLayereddlgOpacity80(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLayereddlgOpacity60(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLayereddlgOpacity40(CCmdUI* pCmdUI);
	afx_msg void OnEditColorsCount();
	afx_msg void OnUpdateEditColorsCount(CCmdUI* pCmdUI);
	afx_msg void OnEditTextToAlpha();
	afx_msg void OnUpdateEditTextToAlpha(CCmdUI* pCmdUI);
	afx_msg void OnLayereddlgPaste();
	afx_msg void OnLayereddlgOpacity20();
	afx_msg void OnUpdateLayereddlgOpacity20(CCmdUI* pCmdUI);
	afx_msg void OnEditTo32bitsAlpha();
	afx_msg void OnUpdateEditTo32bitsAlpha(CCmdUI* pCmdUI);
	afx_msg void OnFileCopyTo();
	afx_msg void OnFileMoveTo();
	afx_msg void OnLayereddlgSize0125();
	afx_msg void OnUpdateLayereddlgSize0125(CCmdUI* pCmdUI);
	afx_msg void OnLayereddlgSize025();
	afx_msg void OnUpdateLayereddlgSize025(CCmdUI* pCmdUI);
	afx_msg void OnLayereddlgSize050();
	afx_msg void OnUpdateLayereddlgSize050(CCmdUI* pCmdUI);
	afx_msg void OnLayereddlgSize100();
	afx_msg void OnUpdateLayereddlgSize100(CCmdUI* pCmdUI);
	afx_msg void OnLayereddlgSize200();
	afx_msg void OnUpdateLayereddlgSize200(CCmdUI* pCmdUI);
	afx_msg void OnLayereddlgSize400();
	afx_msg void OnUpdateLayereddlgSize400(CCmdUI* pCmdUI);
	afx_msg void OnLayereddlgSize800();
	afx_msg void OnUpdateLayereddlgSize800(CCmdUI* pCmdUI);
	afx_msg void OnLayereddlgSizeTopleft();
	afx_msg void OnUpdateLayereddlgSizeTopleft(CCmdUI* pCmdUI);
	afx_msg void OnLayereddlgSizeTopright();
	afx_msg void OnUpdateLayereddlgSizeTopright(CCmdUI* pCmdUI);
	afx_msg void OnLayereddlgSizeBottomleft();
	afx_msg void OnUpdateLayereddlgSizeBottomleft(CCmdUI* pCmdUI);
	afx_msg void OnLayereddlgSizeBottomright();
	afx_msg void OnUpdateLayereddlgSizeBottomright(CCmdUI* pCmdUI);
	afx_msg void OnLayereddlgSize1600();
	afx_msg void OnUpdateLayereddlgSize1600(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveAsPdf();
	afx_msg void OnUpdateFileSaveAsPdf(CCmdUI* pCmdUI);
	afx_msg void OnEditPasteIntoTopleft();
	afx_msg void OnEditPasteIntoTopright();
	afx_msg void OnEditPasteIntoBottomleft();
	afx_msg void OnEditPasteIntoBottomright();
	afx_msg void OnUpdateEditPasteIntoTopleft(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPasteIntoTopright(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPasteIntoBottomleft(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPasteIntoBottomright(CCmdUI* pCmdUI);
	afx_msg void OnEditPasteIntoFile();
	afx_msg void OnUpdateEditPasteIntoFile(CCmdUI* pCmdUI);
	afx_msg void OnEditRotate180();
	afx_msg void OnUpdateEditRotate180(CCmdUI* pCmdUI);
	afx_msg void OnEditPasteIntoFileHelp();
	afx_msg void OnPlayRandom();
	afx_msg void OnUpdatePlayRandom(CCmdUI* pCmdUI);
	afx_msg void OnViewMap();
	afx_msg void OnUpdateViewMap(CCmdUI* pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnEditRename();
	afx_msg void OnFileExtract();
	afx_msg void OnUpdateFileExtract(CCmdUI* pCmdUI);
	afx_msg void OnFileReload();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PICTUREDOC_H__24956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_)
