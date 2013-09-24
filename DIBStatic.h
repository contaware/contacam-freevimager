#if !defined(AFX_DIBSTATIC_H__40E4BDE3_BD3A_11D1_89F4_0000E81D3D27__INCLUDED_)
#define AFX_DIBSTATIC_H__40E4BDE3_BD3A_11D1_89F4_0000E81D3D27__INCLUDED_

#pragma once

// DibStatic.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDibStatic window

#include "Dib.h"
#include "AviPlay.h"
#include "GifAnimationThread.h"
#include "TryEnterCriticalSection.h"

#define WM_MUSIC_POS				WM_USER + 201
#define	WM_LOADDONE					WM_USER + 1300
#define	WM_PAINT_BUSYTEXT			WM_USER + 1301

// Message Delay
#define PAINT_BUSYTEXT_DELAY		300U

class CDibStatic : public CStatic
{
public:
#ifdef SUPPORT_GIFLIB
	// The Gif Animation Thread Class
	class CMyGifAnimationThread : public CGifAnimationThread
	{
		public:
			CMyGifAnimationThread() {m_pDibStatic = NULL;};
			void SetDibStatic(CDibStatic* pDibStatic) {m_pDibStatic = pDibStatic;};

		protected:
			CDibStatic* m_pDibStatic;
			virtual void OnNewFrame();
	};
#endif

	// Thumbnail Loading Thread Class
	class CThumbLoadThread : public CWorkerThread
	{
		public:
			friend CDibStatic;

		public:
			CThumbLoadThread() {m_pDibStatic = NULL;
								m_bLoadAndPlayAnimatedGif = TRUE;
								m_bPaint = TRUE;
								m_rcClient = CRect(0,0,0,0);
								m_bEnteredDibHdrCS = FALSE;
								m_bEnteredDibFullCS = FALSE;};
			virtual ~CThumbLoadThread(){Kill();};
			void SetDibStatic(CDibStatic* pDibStatic) {m_pDibStatic = pDibStatic;};
			void SetFileName(CString sFileName) {m_sFileName = sFileName;};
			void SetLoadAndPlayAnimatedGif(BOOL bLoadAndPlayAnimatedGif) {m_bLoadAndPlayAnimatedGif = bLoadAndPlayAnimatedGif;};
			void SetPaint(BOOL bPaint) {m_bPaint = bPaint;};
			void SetClientRect(CRect rcClient) {m_rcClient = rcClient;};

		protected:
			int Work();
			int WorkHdr();
			int WorkFull();
			void CreateThumbnail();
			void EnterAllCS();
			void EnterHdrCS();
			void EnterFullCS();
			void LeaveAllCS();
			void LeaveHdrCS();
			void LeaveFullCS();
			BOOL m_bEnteredDibHdrCS;
			BOOL m_bEnteredDibFullCS;
			CDibStatic* m_pDibStatic;
			CString m_sFileName;
			BOOL m_bLoadAndPlayAnimatedGif;
			BOOL m_bPaint;
			CRect m_rcClient;
	};

	// The Load Started WPARAM 
	enum LOADSTARTED_TYPE
	{
		HDRLOAD_STARTED=0,
		FULLLOAD_STARTED=1
	};

	// The Load Done WPARAM 
	enum LOADDONE_TYPE
	{
		HDRLOAD_ERROR=0,
		HDRLOAD_DOEXIT=1,
		HDRLOAD_HDRDONE=2,
		FULLLOAD_HDRERROR=3,
		FULLLOAD_DOEXIT=4,
		FULLLOAD_FULLERROR=5,
		FULLLOAD_HDRDONE=6,
		FULLLOAD_FULLDONE=7
	};

	// Friends
	friend CMyGifAnimationThread;
	friend CThumbLoadThread;

public:
	// Construction & Destruction
	CDibStatic();
	virtual ~CDibStatic();

	// Post load messages to the following window
	void SetNotifyHwnd(HWND hNotifyWnd) {m_hNotifyWnd = hNotifyWnd;};

	// Threads Pointers
	CThumbLoadThread* GetThumbLoadThread() {return &m_ThumbLoadThread;};
#ifdef SUPPORT_GIFLIB
	CMyGifAnimationThread* GetGifAnimationThread() {return &m_GifAnimationThread;};
#endif
	void SetThumbLoadThreadPriority(int nThumbLoadThreadPriority) {m_nThumbLoadThreadPriority = nThumbLoadThreadPriority;};

	// Hdr Load Started Flag Means that LoadDib has been successfully called
	// with the bOnlyHeader flag set
	BOOL HasLoadHdrStarted() const {return m_bLoadHdrStarted;};
	void SetLoadHdrStarted(BOOL bLoadHdrStarted) {m_bLoadHdrStarted = bLoadHdrStarted;};

	// Full Load Started Flag Means that LoadDib has been successfully called
	// with the bOnlyHeader flag cleared
	BOOL HasLoadFullStarted() const {return m_bLoadFullStarted;};
	void SetLoadFullStarted(BOOL bLoadFullStarted) {m_bLoadFullStarted = bLoadFullStarted;};

	// Hdr Loaded Terminated Flag means that the Load Thread started
	// with the bOnlyHeader flag set runned to termination
	BOOL HasLoadHdrTerminated() const {return m_bLoadHdrTerminated;};
	void SetLoadHdrTerminated(BOOL bLoadHdrTerminated) {m_bLoadHdrTerminated = bLoadHdrTerminated;};

	// Full Loaded Terminated Flag means that the Load Thread started
	// with the bOnlyHeader flag cleared runned to termination
	BOOL HasLoadFullTerminated() const {return m_bLoadFullTerminated;};
	void SetLoadFullTerminated(BOOL bLoadFullTerminated) {m_bLoadFullTerminated = bLoadFullTerminated;};

	// Critical Sections
	CTryEnterCriticalSection* GetDibHdrCS() const {return m_pcsDibHdr;};
	void SetDibHdrCS(CTryEnterCriticalSection* pcs) {m_pcsDibHdr = pcs;};
	CTryEnterCriticalSection* GetDibFullCS() const {return m_pcsDibFull;};
	void SetDibFullCS(CTryEnterCriticalSection* pcs) {m_pcsDibFull = pcs;};

	// Dibs & AVI Pointers
	CAVIPlay* GetAVIPlayPointer() const {return m_pAVIPlay;};
	void SetAVIPlayPointer(CAVIPlay* pAVIPlay);
	CDib* GetDibHdrPointer() const {return m_pDibHdr;};
	void SetDibHdrPointer(CDib* pDib) {m_pDibHdr = pDib;};
	CDib* GetDibFullPointer() const {return m_pDibFull;};
	void SetDibFullPointer(CDib* pDib) {m_pDibFull = pDib;};
	CDib* GetAlphaRenderedDibPointer() const {return m_pAlphaRenderedDib;};
	void SetAlphaRenderedDibPointer(CDib* pAlphaRenderedDib) {m_pAlphaRenderedDib = pAlphaRenderedDib;};
	
	// Attributes
	void SetBackgroundColor(COLORREF crBackgroundColor) {m_crBackgroundColor = crBackgroundColor;};
	COLORREF GetBackgroundColor() const {return m_crBackgroundColor;};
	void SetImageBackgroundColor(COLORREF crImageBackgroundColor) {m_crImageBackgroundColor = crImageBackgroundColor;};
	COLORREF GetImageBackgroundColor() const {return m_crImageBackgroundColor;};
	void SetUseImageBackgroundColor(BOOL bUseImageBackgroundColor) {m_bUseImageBackgroundColor = bUseImageBackgroundColor;};
	BOOL GetUseImageBackgroundColor() const {return m_bUseImageBackgroundColor;};
	void SetBorders(CRect rcBorders) {m_rcBorders = rcBorders;};
	void SetBordersColor(COLORREF crBordersColor) {m_crBordersColor = crBordersColor;};
	void SetCrossColor(COLORREF crCrossColor) {m_crCrossColor = crCrossColor;};
	void SetBusyTextColor(COLORREF crTextColor) {m_crBusyTextColor = crTextColor;};
	void SetBusyText(CString sText) {m_sBusyText = sText;};
	void SetMusicFlag(BOOL bMusicFile) {m_bMusicFile = bMusicFile;};
	BOOL GetMusicFlag() const {return m_bMusicFile;};

	// Operations
	BOOL Load(	LPCTSTR lpszFileName,					// File Name to Load
				BOOL bOnlyHeader = FALSE,				// If TRUE Load only headers (included EXIF data) to Hdr Dib,
														// If FALSE Load headers to Hdr Dib and a Thumbnail to Full Dib
				BOOL bLoadAndPlayAnimatedGif = TRUE,	// If TRUE Load & Play Animated Gifs,
														// if FALSE Load only first frame
				BOOL bPaint = TRUE,						// Paint After Load
				int nWidth = 0,							// If nWidth != 0 and nHeight != 0 the 
				int nHeight = 0,						// image is loaded using the given sizes, otherwise the client rect is taken
				BOOL bStartPlayingAudio = FALSE);		// If TRUE start playing the given audio file
	
	// Dibs
	void PaintDib(BOOL bUseCS = TRUE);

	// Music
	BOOL IsMusicPlaying() const {	return m_hMCIWnd ?
									MCIWndGetMode(m_hMCIWnd, NULL, 0) == MCI_MODE_PLAY :
									FALSE;};
	BOOL LoadMusic(	LPCTSTR lpszFileName,
					int nWidth = 0,
					int nHeight = 0,
					BOOL bStartPlaying = FALSE,
					BOOL bUseShortPath = FALSE);
	void UnloadMusic();
	void FreeMusic();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDibStatic)
	//}}AFX_VIRTUAL

protected:
	void FreeDibs();
	BOOL DoRealizePalette(BOOL bForceBackGround);
	void ClearView(CDC* pDC);
	void ClearBorders(CDC* pDC, const CRect& rcDib);
	void ClearMusicView(CDC* pDC);
	void CreateMusicRgn(CRgn& rgn);
	CString GetFormattedTime(LONG lMilliseconds);

	volatile DWORD m_dwBusyTextUpTime;
	HWND volatile m_hNotifyWnd;
	CAVIPlay* volatile m_pAVIPlay;
	CDib* volatile m_pDibHdr;
	CDib* volatile m_pDibFull;
	CDib* volatile m_pAlphaRenderedDib;
	CThumbLoadThread m_ThumbLoadThread;
#ifdef SUPPORT_GIFLIB
	CMyGifAnimationThread m_GifAnimationThread;
#endif
	volatile BOOL m_bOnlyHeader;
	CTryEnterCriticalSection* volatile m_pcsDibHdr;
	CTryEnterCriticalSection* volatile m_pcsDibFull;
	volatile BOOL m_bAnimatedGif;
	volatile COLORREF m_crBackgroundColor;
	volatile BOOL m_bUseImageBackgroundColor;
	volatile COLORREF m_crImageBackgroundColor; 
	CRect m_rcBorders;
	volatile COLORREF m_crBordersColor;
	volatile COLORREF m_crCrossColor;
	volatile COLORREF m_crBusyTextColor;
	CString m_sBusyText;
	int m_nThumbLoadThreadPriority;
	volatile BOOL m_bLoadHdrStarted;
	volatile BOOL m_bLoadFullStarted;
	volatile BOOL m_bLoadHdrTerminated;
	volatile BOOL m_bLoadFullTerminated;

	// Music Vars
	HWND m_hMCIWnd;
	BOOL m_bMusicFile;
	CString m_sMusicFile;
	volatile LONG m_lMusicLength;
	volatile LONG m_lMusicPos;

	//{{AFX_MSG(CDibStatic)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg LONG OnPaintBusyText(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnMusicPos(WPARAM wparam, LPARAM lparam);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIBSTATIC_H__40E4BDE3_BD3A_11D1_89F4_0000E81D3D27__INCLUDED_)
