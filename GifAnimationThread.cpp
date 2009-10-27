//
// GifAnimationThread.cpp
//

#include "stdafx.h"
#include "GifAnimationThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef SUPPORT_GIFLIB

void CGifAnimationThread::ClearAnimationArrays()
{	
	while (m_DibAnimationArray.GetSize() > 0)
	{
		delete m_DibAnimationArray.GetAt(m_DibAnimationArray.GetUpperBound());
		m_DibAnimationArray.RemoveAt(m_DibAnimationArray.GetUpperBound());
	}
	while (m_AlphaRenderedDibAnimationArray.GetSize() > 0)
	{
		delete m_AlphaRenderedDibAnimationArray.GetAt(m_AlphaRenderedDibAnimationArray.GetUpperBound());
		m_AlphaRenderedDibAnimationArray.RemoveAt(m_AlphaRenderedDibAnimationArray.GetUpperBound());
	}
	m_dwDibAnimationPos = 0;
	m_dwDibAnimationCount = 0;
}

int CGifAnimationThread::Load(BOOL bDoAlphaRender)
{
	CDib* pDib;
	CDib Dib;
	Dib.SetShowMessageBoxOnError(FALSE);

	// Call before running thread!
	if (IsRunning())
		return 0;

	// Clean-Up
	ClearAnimationArrays();

	// Load First GIF
	if (!Dib.LoadFirstGIF32(m_sFileName))
		return 0;

	// Add First GIF to Array
	pDib = (CDib*)new CDib(Dib);
	pDib->SetShowMessageBoxOnError(FALSE);
	m_DibAnimationArray.Add(pDib);

	// Load Next GIFs
	while (Dib.LoadNextGIF32() > 0)
	{
		// Add GIF to Array
		pDib = (CDib*)new CDib(Dib);
		pDib->SetShowMessageBoxOnError(FALSE);
		m_DibAnimationArray.Add(pDib);
	}

	// Close
	Dib.GetGif()->LoadClose();

	// Init Alpha Render
	if (bDoAlphaRender)
		bDoAlphaRender = InitAlphaRender();

	// Alpha Render
	if (bDoAlphaRender)
		AlphaRender(Dib.GetBackgroundColor());

	// If Only One GIF in File -> Clear & Exit
	if (m_DibAnimationArray.GetSize() <= 1)
	{
		ClearAnimationArrays();
		return 0;
	}
	// Check the to fast frames bug:
	// Some earlier softwares wrote wrong values
	// for the delay. In many cases they wrote
	// 10ms instead of 100ms.
	else
	{
		BOOL bClip = FALSE;
		for (int i = 0 ; i < m_DibAnimationArray.GetSize() ; i++)
		{
			if (m_DibAnimationArray[i]->GetGif()->GetDelay() <
								MIN_ACCETABLE_ANIMGIF_FRAMETIME)
			{
				bClip = TRUE;
				break;
			}
		}
		SetClipTo100ms(bClip);
	}

	// Set Position To First Image in Array and Set Total Images Count
	m_dwDibAnimationPos = 0;
	m_dwDibAnimationCount = m_DibAnimationArray.GetSize();

	return m_dwDibAnimationCount;
}

BOOL CGifAnimationThread::InitAlphaRender()
{
	int i;
	BOOL bAlphaUsed = FALSE;

	// Check
	if (!m_DibAnimationArray[0])
		return FALSE;
	if (!m_DibAnimationArray[0]->HasAlpha())
		return FALSE;

	// Check whether alpha rendering is necessary
	for (i = 0 ; i < m_DibAnimationArray.GetSize() ; i++)
	{
		if (m_DibAnimationArray[i]->GetGif()->m_bAlphaUsed)
		{
			bAlphaUsed = TRUE;
			break;
		}
	}

	// Allocate
	if (bAlphaUsed)
	{
		CDib* pDib;
		for (i = 0 ; i < m_DibAnimationArray.GetSize() ; i++)
		{
			pDib = (CDib*)new CDib;
			pDib->SetShowMessageBoxOnError(FALSE);
			m_AlphaRenderedDibAnimationArray.Add(pDib);
		}

		return TRUE;
	}
	else
		return FALSE;
}

void CGifAnimationThread::AlphaRender(COLORREF crBackgroundColor)
{
	int i;

	// Check
	if (!m_DibAnimationArray[0])
		return;
	if (!m_DibAnimationArray[0]->HasAlpha())
		return;
	if (m_AlphaRenderedDibAnimationArray.GetSize() <= 0)
		return;

	// Render alpha with given background color
	for (i = 0 ; i < m_DibAnimationArray.GetSize() ; i++)
	{
		// Set Background Color
		m_DibAnimationArray[i]->SetBackgroundColor(crBackgroundColor);

		// Render Alpha
		m_AlphaRenderedDibAnimationArray[i]->RenderAlphaWithSrcBackground(m_DibAnimationArray[i]);
	}
}

int CGifAnimationThread::Work()
{
	DWORD Event;

	if (m_dwDibAnimationCount <= 1)
		return 0;

	int nPlayTimes = 0;
	if (m_DibAnimationArray.GetAt(0))
		nPlayTimes = m_DibAnimationArray.GetAt(0)->GetGif()->GetPlayTimes();
	BOOL bInfinitePlay = (nPlayTimes == 0) ? TRUE : FALSE;

	// Show Them
	while (TRUE)
	{
		OnNewFrame();

		// Delay:
		// Most browsers clip everything faster than a 100ms delay (0..99ms) to 100 ms,
		// because of corrupted GIFs. IE started this, quite all browsers followed it
		// (state in year 2005). I do not know in future...
		DWORD dwDelay = m_DibAnimationArray.GetAt(m_dwDibAnimationPos)->GetGif()->GetDelay();
		if (m_bClipTo100ms && dwDelay < 100)
			dwDelay = 100;

		// Wait for GetDelay() or Exit
		Event = ::WaitForSingleObject(GetKillEvent(), dwDelay);
		switch (Event)
		{
			// Shutdown Event
			case WAIT_OBJECT_0 :	return 0;

			case WAIT_TIMEOUT :		// Inc. Pos
									m_dwDibAnimationPos = (++m_dwDibAnimationPos)%m_dwDibAnimationCount;

									// Finish playing?
									if (!bInfinitePlay &&
										(m_dwDibAnimationPos == m_dwDibAnimationCount - 1))
									{
										if (--nPlayTimes == 0)
										{
											if (m_DibAnimationArray.GetAt(0))
												nPlayTimes = m_DibAnimationArray.GetAt(0)->GetGif()->GetPlayTimes();
											OnPlayTimesDone();
										}
									}

									break;

			default:				break;
		}
	}

	return 0;
}

void CGifAnimationThread::ShowPrevious()		
{	
	DWORD dwPos = m_dwDibAnimationPos;
	if (dwPos == 0)
		dwPos = m_dwDibAnimationCount - 1;
	else
		dwPos = (--dwPos) % m_dwDibAnimationCount;
	m_dwDibAnimationPos = dwPos;
}

void CGifAnimationThread::ShowNext()			
{	
	DWORD dwPos = m_dwDibAnimationPos;
	dwPos = (++dwPos) % m_dwDibAnimationCount;
	m_dwDibAnimationPos = dwPos;
}

void CGifAnimationThread::ShowFirst()		
{	
	m_dwDibAnimationPos = 0;
}

void CGifAnimationThread::ShowLast()			
{	
	m_dwDibAnimationPos = m_dwDibAnimationCount - 1;
}

#endif