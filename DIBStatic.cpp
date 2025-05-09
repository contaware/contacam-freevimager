#include "stdafx.h"
#include "uImager.h"
#include "MainFrm.h"
#include "DibStatic.h"
#include "PictureDoc.h"
#include "PostDelayedMessage.h"
#include "MyMemDC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CDibStatic::CMyGifAnimationThread::OnNewFrame()
{	
	ASSERT(m_pDibStatic);
	m_pDibStatic->PaintDib();
}

int CDibStatic::CThumbLoadThread::Work() 
{
	ASSERT(m_pDibStatic);
	if (m_pDibStatic->m_bOnlyHeader)
		return WorkHdr();
	else
		return WorkFull();
}

void CDibStatic::CThumbLoadThread::EnterAllCS()
{
	EnterFullCS();
	EnterHdrCS();
}

void CDibStatic::CThumbLoadThread::EnterHdrCS()
{
	if (m_pDibStatic->m_pcsDibHdr)
		m_bEnteredDibHdrCS = m_pDibStatic->m_pcsDibHdr->EnterCriticalSection();
}

void CDibStatic::CThumbLoadThread::EnterFullCS()
{
	if (m_pDibStatic->m_pcsDibFull)
		m_bEnteredDibFullCS = m_pDibStatic->m_pcsDibFull->EnterCriticalSection();
}

void CDibStatic::CThumbLoadThread::LeaveAllCS()
{
	LeaveHdrCS();
	LeaveFullCS();
}

void CDibStatic::CThumbLoadThread::LeaveHdrCS()
{
	if (m_pDibStatic->m_pcsDibHdr && m_bEnteredDibHdrCS)
	{
		m_pDibStatic->m_pcsDibHdr->LeaveCriticalSection();
		m_bEnteredDibHdrCS = FALSE;
	}
}

void CDibStatic::CThumbLoadThread::LeaveFullCS()
{
	if (m_pDibStatic->m_pcsDibFull && m_bEnteredDibFullCS)
	{
		m_pDibStatic->m_pcsDibFull->LeaveCriticalSection();
		m_bEnteredDibFullCS = FALSE;
	}
}

int CDibStatic::CThumbLoadThread::WorkHdr()
{
	BOOL bOldDibHdrShowMessageBoxOnError;

	// Enter Hdr CS
	EnterHdrCS();

	// Dib Pointer Check
	if (m_pDibStatic->m_pDibHdr == NULL)
	{
		m_pDibStatic->m_bLoadHdrTerminated = TRUE;
		m_pDibStatic->FreeDibs();
		if (m_bPaint)
			m_pDibStatic->PaintDib(FALSE);
		LeaveHdrCS();
		if (::IsWindow(m_pDibStatic->m_hNotifyWnd))
			::PostMessage(	m_pDibStatic->m_hNotifyWnd,
							WM_LOADDONE,
							(WPARAM)HDRLOAD_ERROR,
							(LPARAM)m_pDibStatic);
		return 0;
	}	

	try
	{
		// Do Not Show Error Messages!
		bOldDibHdrShowMessageBoxOnError = m_pDibStatic->m_pDibHdr->IsShowMessageBoxOnError();
		m_pDibStatic->m_pDibHdr->SetShowMessageBoxOnError(FALSE);

		// Do Exit?
		if (DoExit())
			throw (int)HDRLOAD_DOEXIT;

		// Is it a Picture?
		if (CUImagerApp::IsSupportedPictureFile(m_sFileName))
		{
			// Load Header (for Jpeg this loads also exif data)
			if (!m_pDibStatic->m_pDibHdr->LoadImage(m_sFileName, 0, 0, 0, TRUE, TRUE))
				throw (int)HDRLOAD_ERROR;
			m_pDibStatic->m_bLoadHdrTerminated = TRUE;
			m_pDibStatic->m_pDibHdr->SetShowMessageBoxOnError(bOldDibHdrShowMessageBoxOnError);
			CDib::AutoOrientateDib(m_pDibStatic->m_pDibHdr);
			if (m_bPaint)
				m_pDibStatic->PaintDib(FALSE);
			LeaveHdrCS();
			if (::IsWindow(m_pDibStatic->m_hNotifyWnd))
				::PostMessage(	m_pDibStatic->m_hNotifyWnd,
								WM_LOADDONE,
								(WPARAM)(HDRLOAD_HDRDONE),
								(LPARAM)m_pDibStatic);

			return 1;
		}
		else
			throw (int)HDRLOAD_ERROR;
	}
	catch (int nCause)
	{
		m_pDibStatic->m_bLoadHdrTerminated = TRUE;
		m_pDibStatic->m_pDibHdr->SetShowMessageBoxOnError(bOldDibHdrShowMessageBoxOnError);
		m_pDibStatic->FreeDibs();
		if (m_bPaint)
			m_pDibStatic->PaintDib(FALSE);
		LeaveHdrCS();
		if (::IsWindow(m_pDibStatic->m_hNotifyWnd))
			::PostMessage(	m_pDibStatic->m_hNotifyWnd,
							WM_LOADDONE,
							(WPARAM)nCause,
							(LPARAM)m_pDibStatic);
		return 0;
	}
}

int CDibStatic::CThumbLoadThread::WorkFull() 
{
	BOOL bOldDibHdrShowMessageBoxOnError;
	BOOL bOldDibFullShowMessageBoxOnError;

	// Enter All CS
	EnterAllCS();

	// Dib Pointers Check
	if (m_pDibStatic->m_pDibHdr == NULL ||
		m_pDibStatic->m_pDibFull == NULL)
	{
		m_pDibStatic->m_bLoadFullTerminated = TRUE;
		m_pDibStatic->FreeDibs();
		if (m_bPaint)
			m_pDibStatic->PaintDib(FALSE);
		LeaveAllCS();
		if (::IsWindow(m_pDibStatic->m_hNotifyWnd))
			::PostMessage(	m_pDibStatic->m_hNotifyWnd,
							WM_LOADDONE,
							(WPARAM)FULLLOAD_HDRERROR,
							(LPARAM)m_pDibStatic);
		return 0;
	}	

	try
	{
		// Do Not Show Error Messages!
		bOldDibHdrShowMessageBoxOnError = m_pDibStatic->m_pDibHdr->IsShowMessageBoxOnError();
		bOldDibFullShowMessageBoxOnError = m_pDibStatic->m_pDibFull->IsShowMessageBoxOnError();
		m_pDibStatic->m_pDibHdr->SetShowMessageBoxOnError(FALSE);
		m_pDibStatic->m_pDibFull->SetShowMessageBoxOnError(FALSE);

		// Do Exit?
		if (DoExit())
			throw (int)FULLLOAD_DOEXIT;

		// Free Alpha Rendered Dib
		if (m_pDibStatic->m_pAlphaRenderedDib)
			m_pDibStatic->m_pAlphaRenderedDib->Free();

		// Is it a Picture?
		if (CUImagerApp::IsSupportedPictureFile(m_sFileName))
		{
			// Load Header (for Jpeg this loads also exif data)
			if (!m_pDibStatic->m_pDibHdr->LoadImage(m_sFileName, 0, 0, 0, TRUE, TRUE))
				throw (int)FULLLOAD_HDRERROR;

			// Restore Old ShowMessageBoxOnError
			m_pDibStatic->m_pDibHdr->SetShowMessageBoxOnError(bOldDibHdrShowMessageBoxOnError);

			// Post Paint Busy Text Message
			m_pDibStatic->m_dwBusyTextUpTime = ::GetTickCount();
			CPostDelayedMessageThread::PostDelayedMessage(	m_pDibStatic->GetSafeHwnd(),
															WM_PAINT_BUSYTEXT,
															PAINT_BUSYTEXT_DELAY,
															(WPARAM)m_pDibStatic->m_dwBusyTextUpTime,
															(LPARAM)0);

			// Leave Hdr CS
			LeaveHdrCS();

			// Post Header Load Done
			if (::IsWindow(m_pDibStatic->m_hNotifyWnd))
				::PostMessage(	m_pDibStatic->m_hNotifyWnd,
								WM_LOADDONE,
								(WPARAM)(FULLLOAD_HDRDONE),
								(LPARAM)m_pDibStatic);

			// Check File (This Loads also Headers)
			if (!m_pDibStatic->m_pDibFull->FileCheck(m_sFileName))
				throw (int)FULLLOAD_HDRERROR;	// Throw a Hdr Error because the Picture Hdr
												// has a problem, even if loading the full dib!

			// Do Exit?
			if (DoExit())
				throw (int)FULLLOAD_DOEXIT;

			// No Thumbnail?
			if (!m_pDibStatic->m_pDibFull->GetThumbnailDib())
			{
				if (!m_pDibStatic->m_pDibFull->CreateThumbnailDibFromJPEG(	m_sFileName,
																			m_rcClient.Width(),
																			m_rcClient.Height(),
																			NULL,
																			TRUE,
																			this))
				{
					if (DoExit())
						throw (int)FULLLOAD_DOEXIT;
					CreateThumbnail();
				}
				else
					CDib::AutoOrientateDib(m_pDibStatic->m_pDibFull);
			}
			else
				CDib::AutoOrientateDib(m_pDibStatic->m_pDibFull);

			// Alpha
			if (m_pDibStatic->m_pAlphaRenderedDib &&
				m_pDibStatic->m_pDibFull->HasAlpha() &&
				m_pDibStatic->m_pDibFull->GetBitCount() == 32)
			{
				if (m_pDibStatic->m_pDibFull->GetThumbnailDib() && m_pDibStatic->m_pDibFull->GetThumbnailDib()->IsValid())
					m_pDibStatic->m_pAlphaRenderedDib->RenderAlphaWithSrcBackground(m_pDibStatic->m_pDibFull->GetThumbnailDib());
				else
					m_pDibStatic->m_pAlphaRenderedDib->RenderAlphaWithSrcBackground(m_pDibStatic->m_pDibFull);
			}

			// Stretch
			if (m_pDibStatic->m_pDibFull->GetThumbnailDib())
			{
				if (!m_pDibStatic->m_pDibFull->GetThumbnailDib()->StretchBitsFitRect(	m_rcClient.Width(),
																						m_rcClient.Height(),
																						NULL,
																						NULL,
																						TRUE,
																						this))
				{
					if (DoExit())
						throw (int)FULLLOAD_DOEXIT;
					else
						throw (int)FULLLOAD_FULLERROR;
				}
			}
		}
		else
			throw (int)FULLLOAD_HDRERROR;

		// Set Stretch Mode
		if (m_pDibStatic->m_pDibFull->GetBitCount() > 8)
		{
			m_pDibStatic->m_pDibFull->SetStretchMode(HALFTONE);
			if (m_pDibStatic->m_pDibFull->GetThumbnailDib())
				m_pDibStatic->m_pDibFull->GetThumbnailDib()->SetStretchMode(HALFTONE);
			if (m_pDibStatic->m_pAlphaRenderedDib)
				m_pDibStatic->m_pAlphaRenderedDib->SetStretchMode(HALFTONE);
		}
		else
		{
			m_pDibStatic->m_pDibFull->SetStretchMode(COLORONCOLOR);
			if (m_pDibStatic->m_pDibFull->GetThumbnailDib())
				m_pDibStatic->m_pDibFull->GetThumbnailDib()->SetStretchMode(COLORONCOLOR);
			if (m_pDibStatic->m_pAlphaRenderedDib)
				m_pDibStatic->m_pAlphaRenderedDib->SetStretchMode(COLORONCOLOR);
		}

		// Realize Palette
		if (m_bPaint)
			m_pDibStatic->DoRealizePalette(FALSE);

		// Do Exit?
		if (DoExit())
			throw (int)FULLLOAD_DOEXIT;

		// Wait until the Gif Animation Thread Stops
		if (m_pDibStatic->m_GifAnimationThread.IsAlive())
			m_pDibStatic->m_GifAnimationThread.WaitDone_Blocking();
			
		// Clear all Animation Frames
		m_pDibStatic->m_GifAnimationThread.ClearAnimationArrays();

		// Restore Old ShowMessageBoxOnError
		m_pDibStatic->m_pDibFull->SetShowMessageBoxOnError(bOldDibFullShowMessageBoxOnError);

		// Load All Frames of the Animated File to m_DibAnimationArray
		if (m_pDibStatic->m_bAnimatedGif && m_bLoadAndPlayAnimatedGif)
		{
			// Load Gifs
			m_pDibStatic->m_GifAnimationThread.SetFileName(m_sFileName);
			if (m_pDibStatic->m_GifAnimationThread.Load(FALSE) > 1)
			{
				// Clear Paint Busy Text
				m_pDibStatic->m_dwBusyTextUpTime = ::GetTickCount();

				// Loaded Flag
				m_pDibStatic->m_bLoadFullTerminated = TRUE;

				// Leave Full CS
				LeaveFullCS();

				// Start Thread
				m_pDibStatic->m_GifAnimationThread.Start();

				// Done
				if (::IsWindow(m_pDibStatic->m_hNotifyWnd))
					::PostMessage(	m_pDibStatic->m_hNotifyWnd,
									WM_LOADDONE,
									(WPARAM)(FULLLOAD_FULLDONE),
									(LPARAM)m_pDibStatic);

				return 1;
			}
		}

		// Clear Paint Busy Text
		m_pDibStatic->m_dwBusyTextUpTime = ::GetTickCount();

		// Loaded Flag
		m_pDibStatic->m_bLoadFullTerminated = TRUE;

		// Paint Dib (must be after m_bLoadFullTerminated is set
		// because of the busy text paint logic)
		if (m_bPaint)
			m_pDibStatic->PaintDib(FALSE);

		// Leave Full CS
		LeaveFullCS();

		// Done
		if (::IsWindow(m_pDibStatic->m_hNotifyWnd))
			::PostMessage(	m_pDibStatic->m_hNotifyWnd,
							WM_LOADDONE,
							(WPARAM)(FULLLOAD_FULLDONE),
							(LPARAM)m_pDibStatic);

		return 1;	
	}
	catch (int nCause)
	{
		m_pDibStatic->m_dwBusyTextUpTime = ::GetTickCount();
		m_pDibStatic->m_bLoadFullTerminated = TRUE;
		m_pDibStatic->m_pDibFull->SetShowMessageBoxOnError(bOldDibFullShowMessageBoxOnError);
		m_pDibStatic->m_pDibHdr->SetShowMessageBoxOnError(bOldDibHdrShowMessageBoxOnError);
		m_pDibStatic->FreeDibs();
		if (m_bPaint && (nCause != FULLLOAD_DOEXIT)) // Only Paint if not killed
			m_pDibStatic->PaintDib(FALSE);
		LeaveAllCS();
		if (::IsWindow(m_pDibStatic->m_hNotifyWnd))
			::PostMessage(	m_pDibStatic->m_hNotifyWnd,
							WM_LOADDONE,
							(WPARAM)nCause,
							(LPARAM)m_pDibStatic);

		return 0;
	}
}

void CDibStatic::CThumbLoadThread::CreateThumbnail()
{
	BOOL res = FALSE;

	// Use Memory Mapped Load if Not Compressed and Not Old OS/2 Bmp and Not top-down Bmp
	if (::GetFileExt(m_sFileName) == _T(".bmp") ||
		::GetFileExt(m_sFileName) == _T(".dib"))
	{
		if (!m_pDibStatic->m_pDibFull->IsCompressed()			&&
			!m_pDibStatic->m_pDibFull->m_FileInfo.m_bBmpOS2Hdr	&&
			m_pDibStatic->m_pDibFull->m_FileInfo.m_nHeight > 0)
			res = m_pDibStatic->m_pDibFull->MapBMP(m_sFileName, TRUE);
	}
	
	// Normal Load
	if (res == FALSE)
	{
		if (!m_pDibStatic->m_pDibFull->LoadImage(	m_sFileName,
													0, 0, 0,
													TRUE,
													FALSE,
													NULL,
													TRUE,
													this))
		{
			if (DoExit())
				throw (int)FULLLOAD_DOEXIT;
			else
				throw (int)FULLLOAD_FULLERROR;
		}
	}

	// Background Color: if the image has one, use it,
	// otherwise paint with the default m_crBackgroundColor
	if (m_pDibStatic->m_pDibFull->m_FileInfo.m_bHasBackgroundColor)
	{
		m_pDibStatic->m_bUseImageBackgroundColor = TRUE;
		m_pDibStatic->m_crImageBackgroundColor = m_pDibStatic->m_pDibFull->m_FileInfo.m_crBackgroundColor;
	}
	else
		m_pDibStatic->m_pDibFull->SetBackgroundColor(m_pDibStatic->m_crBackgroundColor);

	// Do Exit?
	if (DoExit())
		throw (int)FULLLOAD_DOEXIT;

	// Create Thumbnail Dib if not animated GIF or if no load and play wanted
	if (!m_bLoadAndPlayAnimatedGif ||
		!(m_pDibStatic->m_bAnimatedGif = CDib::IsAnimatedGIF(m_sFileName, FALSE)))
	{
		if (!m_pDibStatic->m_pDibFull->CreateThumbnailDib(	m_rcClient.Width(),
															m_rcClient.Height(),
															NULL,
															NULL,
															TRUE,
															this))
		{
			if (DoExit())
				throw (int)FULLLOAD_DOEXIT;
			else
				throw (int)FULLLOAD_FULLERROR;
		}

		// Free Some Memory
		m_pDibStatic->m_pDibFull->Free(	FALSE,  // Do Not LeavePalette
										TRUE,	// Leave Header
										FALSE,	// Do Not Leave Preview Dib
										TRUE,	// Leave Thumbnail Dib
										TRUE,	// Leave EXIF Info
										TRUE);	// Leave GIF Info
	}

	// Auto Orientate
	CDib::AutoOrientateDib(m_pDibStatic->m_pDibFull);
}

/////////////////////////////////////////////////////////////////////////////
// CDibStatic

BEGIN_MESSAGE_MAP(CDibStatic, CStatic)
	//{{AFX_MSG_MAP(CDibStatic)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_PAINT_BUSYTEXT, OnPaintBusyText)
END_MESSAGE_MAP()

CDibStatic::CDibStatic()
{
	m_bLoadHdrStarted = FALSE;
	m_bLoadFullStarted = FALSE;
	m_bLoadHdrTerminated = FALSE;
	m_bLoadFullTerminated = FALSE;
	m_nThumbLoadThreadPriority = THREAD_PRIORITY_NORMAL;
	m_hNotifyWnd = NULL;
	m_dwBusyTextUpTime = 0U;
	m_pDibHdr = NULL;
	m_pDibFull = NULL;
	m_pAlphaRenderedDib = NULL;
	m_bOnlyHeader = FALSE;
	m_pcsDibHdr = NULL;
	m_pcsDibFull = NULL;
	m_bAnimatedGif = FALSE;
	m_crBackgroundColor = RGB(0xFF, 0xFF, 0xFF);
	m_bUseImageBackgroundColor = FALSE;
	m_crImageBackgroundColor = RGB(0xFF, 0xFF, 0xFF);
	m_rcBorders = CRect(0, 0, 0, 0);
	m_crBordersColor = RGB(0, 0xFF, 0);
	m_crBusyTextColor = RGB(0, 0, 0);
	m_sBusyText = ML_STRING(1384, "Loading...");
	m_crCrossColor = RGB(0, 0, 0);
	m_ThumbLoadThread.SetDibStatic(this);
	m_GifAnimationThread.SetDibStatic(this);
}

CDibStatic::~CDibStatic()
{
	m_ThumbLoadThread.Kill();
	m_GifAnimationThread.Kill();
}

void CDibStatic::OnDestroy() 
{
	m_ThumbLoadThread.Kill();
	m_GifAnimationThread.Kill();
	CStatic::OnDestroy();
}

LONG CDibStatic::OnPaintBusyText(WPARAM wparam, LPARAM lparam)
{
	DWORD dwPostedBusyTextUpTime = (DWORD)wparam;
	if (m_dwBusyTextUpTime == dwPostedBusyTextUpTime)
		PaintDib();
	return 0;
}

BOOL CDibStatic::Load(	LPCTSTR lpszFileName,
						BOOL bOnlyHeader/*=FALSE*/,
						BOOL bLoadAndPlayAnimatedGif/*=TRUE*/,
						BOOL bPaint/*=TRUE*/,
						int nWidth/*=0*/,
						int nHeight/*=0*/)
{
	// Client Area
	CRect rcClient;
	if (nWidth != 0 && nHeight != 0)
		rcClient = CRect(0, 0, nWidth, nHeight);
	else
		GetClientRect(&rcClient);

	// Borders
	rcClient.DeflateRect(&m_rcBorders);

	// Kill Thumb Load Thread
	if (m_ThumbLoadThread.IsAlive())
		m_ThumbLoadThread.Kill();

	// Start Killing Gif Animation Thread
	if (m_GifAnimationThread.IsAlive())
		m_GifAnimationThread.Kill_NoBlocking();

	// Clear Gif Animation Flag
	m_bAnimatedGif = FALSE;

	// Clear Image Background Color Flag
	m_bUseImageBackgroundColor = FALSE;

	// Init Only Header Var
	m_bOnlyHeader = bOnlyHeader;

	// Clear Loaded Flag
	if (bOnlyHeader)
		m_bLoadHdrTerminated = FALSE;
	else
		m_bLoadFullTerminated = FALSE;

	// Set Started Flag
	if (bOnlyHeader)
		m_bLoadHdrStarted = TRUE;
	else
		m_bLoadFullStarted = TRUE;

	// Init Thread Vars
	m_ThumbLoadThread.SetFileName(lpszFileName);
	m_ThumbLoadThread.SetLoadAndPlayAnimatedGif(bLoadAndPlayAnimatedGif);
	m_ThumbLoadThread.SetPaint(bPaint);
	m_ThumbLoadThread.SetClientRect(rcClient);

	// Start Thread, also if a invalid file,
	// this because we want WM_LOADDONE messages posted!
	if (m_ThumbLoadThread.Start(m_nThumbLoadThreadPriority) == true)
		return TRUE;
	else
	{
		// Clear Started Flag
		if (bOnlyHeader)
			m_bLoadHdrStarted = FALSE;
		else
			m_bLoadFullStarted = FALSE;
		return FALSE;
	}
}

void CDibStatic::ClearView(CDC* pDC)
{	
	ASSERT(pDC);

	// Client Rect
	CRect rcClient;
	GetClientRect(&rcClient);

	// Around Image
	CRect rcFill = rcClient;
	rcFill.DeflateRect(&m_rcBorders);

	// Paint Background
	CBrush BackGndBrush;
	BackGndBrush.CreateSolidBrush(m_crBackgroundColor);
	pDC->FillRect(rcFill, &BackGndBrush);

	// Paint Borders
	CRect rcBorderTop(rcClient.left, rcClient.top, rcClient.right, rcClient.top + m_rcBorders.top);
	CRect rcBorderLeft(rcClient.left, rcClient.top, rcClient.left + m_rcBorders.left, rcClient.bottom);
	CRect rcBorderRight = CRect(rcClient.right - m_rcBorders.right, rcClient.top, rcClient.right, rcClient.bottom);
	CRect rcBorderBottom = CRect(rcClient.left, rcClient.bottom - m_rcBorders.bottom, rcClient.right, rcClient.bottom);
	CBrush BordersBrush;
	BordersBrush.CreateSolidBrush(m_crBordersColor);
	if ((rcBorderTop.Width() > 0) && (rcBorderTop.Height() > 0))
		pDC->FillRect(rcBorderTop, &BordersBrush);
	if ((rcBorderLeft.Width() > 0) && (rcBorderLeft.Height() > 0))
		pDC->FillRect(rcBorderLeft, &BordersBrush);
	if ((rcBorderRight.Width() > 0) && (rcBorderRight.Height() > 0))
		pDC->FillRect(rcBorderRight, &BordersBrush);
	if ((rcBorderBottom.Width() > 0) && (rcBorderBottom.Height() > 0))
		pDC->FillRect(rcBorderBottom, &BordersBrush);
}

void CDibStatic::ClearBorders(CDC* pDC, const CRect& rcDib)
{
	ASSERT(pDC);

	// Client Rect
	CRect rcClient;
	GetClientRect(&rcClient);

	// Around Image
	CRect rcFill = rcClient;
	rcFill.DeflateRect(&m_rcBorders);

	// Paint Background
	CRect rcTop(rcFill.left, rcFill.top, rcFill.right, rcDib.top);
	CRect rcLeft(rcFill.left, rcDib.top, rcDib.left, rcDib.bottom);
	CRect rcRight = CRect(rcDib.right, rcDib.top, rcFill.right, rcDib.bottom);
	CRect rcBottom = CRect(rcFill.left, rcDib.bottom, rcFill.right, rcFill.bottom);
	CBrush BackGndBrush;
	BackGndBrush.CreateSolidBrush(	m_bUseImageBackgroundColor ?
									m_crImageBackgroundColor :
									m_crBackgroundColor);
	if ((rcTop.Width() > 0) && (rcTop.Height() > 0))
		pDC->FillRect(rcTop, &BackGndBrush);
	if ((rcLeft.Width() > 0) && (rcLeft.Height() > 0))
		pDC->FillRect(rcLeft, &BackGndBrush);
	if ((rcRight.Width() > 0) && (rcRight.Height() > 0))
		pDC->FillRect(rcRight, &BackGndBrush);
	if ((rcBottom.Width() > 0) && (rcBottom.Height() > 0))
		pDC->FillRect(rcBottom, &BackGndBrush);

	// Paint Borders
	CRect rcBorderTop(rcClient.left, rcClient.top, rcClient.right, rcClient.top + m_rcBorders.top);
	CRect rcBorderLeft(rcClient.left, rcClient.top, rcClient.left + m_rcBorders.left, rcClient.bottom);
	CRect rcBorderRight = CRect(rcClient.right - m_rcBorders.right, rcClient.top, rcClient.right, rcClient.bottom);
	CRect rcBorderBottom = CRect(rcClient.left, rcClient.bottom - m_rcBorders.bottom, rcClient.right, rcClient.bottom);
	CBrush BordersBrush;
	BordersBrush.CreateSolidBrush(m_crBordersColor);
	if ((rcBorderTop.Width() > 0) && (rcBorderTop.Height() > 0))
		pDC->FillRect(rcBorderTop, &BordersBrush);
	if ((rcBorderLeft.Width() > 0) && (rcBorderLeft.Height() > 0))
		pDC->FillRect(rcBorderLeft, &BordersBrush);
	if ((rcBorderRight.Width() > 0) && (rcBorderRight.Height() > 0))
		pDC->FillRect(rcBorderRight, &BordersBrush);
	if ((rcBorderBottom.Width() > 0) && (rcBorderBottom.Height() > 0))
		pDC->FillRect(rcBorderBottom, &BordersBrush);
}

CString CDibStatic::GetFormattedTime(LONG lMilliseconds)
{
	double dLength		= (double)lMilliseconds / 1000.0;						// Total Length in Seconds
	int nLengthHour		= (int)(dLength / 3600.0);								// Hours Part
	int nLengthMin		= (int)((dLength - nLengthHour * 3600.0) / 60.0);		// Minutes Part
	double dLengthSec	= dLength - nLengthHour * 3600.0 - nLengthMin * 60.0;	// Seconds Part

	CString t;
	t.Format(_T("%02d:%02d:%02d"),	nLengthHour,
									nLengthMin,
									Round(dLengthSec));
	return t;
}

void CDibStatic::PaintDib(BOOL bUseCS/*=TRUE*/)
{
	// Check
	if (GetSafeHwnd() == NULL)
		return;
	CClientDC dc(this);
	CRect rcClient, rcPaint;
	GetClientRect(&rcClient);
	rcPaint = rcClient;
	rcPaint.DeflateRect(&m_rcBorders);

	BOOL bEnterOk = TRUE;
	if (bUseCS && m_pcsDibFull)
		bEnterOk = m_pcsDibFull->TryEnterCriticalSection();
	if (bEnterOk)
	{
		CDib* pDib;

		if (!m_bOnlyHeader &&
			((m_pDibFull && m_pDibFull->IsValid()) ||
			(m_pDibFull && m_pDibFull->GetThumbnailDib() && m_pDibFull->GetThumbnailDib()->IsValid())))
		{
			if ((m_GifAnimationThread.m_dwDibAnimationCount > 1) &&
				m_GifAnimationThread.IsAlive())
			{
				pDib = m_GifAnimationThread.m_DibAnimationArray.GetAt(m_GifAnimationThread.m_dwDibAnimationPos);
			}
			else
			{
				if (m_pAlphaRenderedDib && m_pAlphaRenderedDib->IsValid())
					pDib = m_pAlphaRenderedDib;
				else if (m_pDibFull && m_pDibFull->GetThumbnailDib() && m_pDibFull->GetThumbnailDib()->IsValid()) 
					pDib = m_pDibFull->GetThumbnailDib();
				else
					pDib = m_pDibFull;
			}
	
			int nDestX, nDestY, nDestWidth, nDestHeight;
			if (pDib->GetWidth() < (DWORD)rcPaint.Width() && pDib->GetHeight() < (DWORD)rcPaint.Height())
			{
				nDestX = rcPaint.left + (rcPaint.Width() - pDib->GetWidth())/2;
				nDestY = rcPaint.top + (rcPaint.Height() - pDib->GetHeight())/2;
				nDestWidth = pDib->GetWidth();
				nDestHeight = pDib->GetHeight();
			}
			else
			{
				if ((rcPaint.Width()/(float)pDib->GetWidth()) <= (rcPaint.Height()/(float)pDib->GetHeight()))
				{
					nDestWidth = rcPaint.Width();
					nDestHeight = (nDestWidth*pDib->GetHeight()) / pDib->GetWidth();
					nDestX = rcPaint.left;
					nDestY = rcPaint.top + (rcPaint.Height() - nDestHeight) /2;
				}
				else
				{		
					nDestHeight = rcPaint.Height();
					nDestWidth = (nDestHeight*pDib->GetWidth()) / pDib->GetHeight();
					nDestX = rcPaint.left + (rcPaint.Width() - nDestWidth) /2;
					nDestY = rcPaint.top;
				}
			}

			CRect RectDest(nDestX, nDestY, nDestX+nDestWidth, nDestY+nDestHeight);
			CRect RectDib(0, 0, pDib->GetWidth(), pDib->GetHeight());
			ClearBorders(&dc, RectDest);
			pDib->Paint(dc,
						&RectDest,
						&RectDib,
						FALSE); 
		}
		else
		{
			// Draw Cross
			CMyMemDC MemDC(&dc, &rcClient);
			ClearView(&MemDC);
			CPen Pen(PS_SOLID, 1, m_crCrossColor);
			CPen* pOldPen = MemDC.SelectObject(&Pen);
			MemDC.MoveTo(rcPaint.TopLeft());				// Start Point Included
			MemDC.LineTo(rcPaint.BottomRight());			// End Point Not Included
			MemDC.MoveTo(rcPaint.left, rcPaint.bottom - 1);	// Start Point Included
			MemDC.LineTo(rcPaint.right, rcPaint.top - 1);	// End Point Not Included
			MemDC.SelectObject(pOldPen);
		}

		// Leave CS
		if (bUseCS && m_pcsDibFull)
			m_pcsDibFull->LeaveCriticalSection();
	}
	else if (!m_bOnlyHeader && !m_bLoadFullTerminated)
	{
		// Draw m_sBusyText Text
		CMyMemDC MemDC(&dc, &rcClient);
		ClearView(&MemDC);
		COLORREF crOldTextColor = MemDC.SetTextColor(m_crBusyTextColor);
		int nOldBkMode = MemDC.SetBkMode(TRANSPARENT);
		LOGFONT lf;
		memset(&lf, 0, sizeof(lf));
		_tcscpy(lf.lfFaceName, g_szDefaultFontFace);
		lf.lfHeight = -MulDiv(10, GetDeviceCaps(MemDC.GetSafeHdc(), LOGPIXELSY), 72);
		lf.lfWeight = FW_NORMAL;
		lf.lfItalic = 0;
		lf.lfUnderline = 0;
		HFONT hFont = ::CreateFontIndirect(&lf);
		HFONT hOldFont = (HFONT)::SelectObject(MemDC.GetSafeHdc(), hFont);
		MemDC.DrawText(m_sBusyText, rcPaint, (DT_CENTER | DT_VCENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE));
		::SelectObject(MemDC.GetSafeHdc(), hOldFont);
		::DeleteObject(hFont);
		MemDC.SetBkMode(nOldBkMode);
		MemDC.SetTextColor(crOldTextColor);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDibStatic message handlers

HBRUSH CDibStatic::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	PaintDib();

	// TODO: Return a non-NULL brush if the parent's handler should not be called
	return (HBRUSH)GetStockObject(NULL_BRUSH);
}

BOOL CDibStatic::OnQueryNewPalette() 
{
	BOOL bEnterOk = TRUE;
	if (m_pcsDibFull)
		bEnterOk = m_pcsDibFull->TryEnterCriticalSection();
	if (bEnterOk)
	{
		BOOL res = DoRealizePalette(FALSE);
		if (m_pcsDibFull)
			m_pcsDibFull->LeaveCriticalSection();
		return res;
	}
	else
		return FALSE;
}

void CDibStatic::OnPaletteChanged(CWnd* pFocusWnd) 
{
	BOOL bEnterOk = TRUE;
	if (m_pcsDibFull)
		bEnterOk = m_pcsDibFull->TryEnterCriticalSection();
	if (bEnterOk)
	{
		DoRealizePalette(TRUE);
		if (m_pcsDibFull)
			m_pcsDibFull->LeaveCriticalSection();
	}
}

BOOL CDibStatic::DoRealizePalette(BOOL bForceBackGround)
{
	if (GetSafeHwnd() != NULL	&&
		((m_pDibFull && m_pDibFull->IsValid()) ||
		(m_pDibFull && m_pDibFull->GetThumbnailDib() && m_pDibFull->GetThumbnailDib()->IsValid())))
	{
		CDib* pDib;
		if ((m_GifAnimationThread.m_dwDibAnimationCount > 1) &&
			m_GifAnimationThread.IsAlive())
			pDib = m_GifAnimationThread.m_DibAnimationArray.GetAt(m_GifAnimationThread.m_dwDibAnimationPos);
		else
		{
			if (m_pDibFull->GetThumbnailDib() && m_pDibFull->GetThumbnailDib()->IsValid()) 
				pDib = m_pDibFull->GetThumbnailDib();
			else
				pDib = m_pDibFull;
		}

		CClientDC dc(this);
		if (!pDib->GetPalette())
			return FALSE;
		HPALETTE hPal = (HPALETTE)pDib->GetPalette()->GetSafeHandle();
		HPALETTE hOldPalette = SelectPalette(dc, hPal, bForceBackGround);
		UINT nChanged = dc.RealizePalette();
		SelectPalette(dc, hOldPalette, TRUE);

		if (nChanged == 0)	// no change to our mapping
			return FALSE;
		
		// Some changes have been made
		PaintDib();
	}

	return TRUE;
}

void CDibStatic::FreeDibs()
{
	if (m_bOnlyHeader)
	{
		// Free Dib
		if (m_pDibHdr) 
			m_pDibHdr->Free();
	}
	else
	{
		// Free Dib
		if (m_pDibFull) 
			m_pDibFull->Free();

		// Free Alpha Rendered Dib
		if (m_pAlphaRenderedDib)
			m_pAlphaRenderedDib->Free();
	}
}
