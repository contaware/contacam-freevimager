//
// GifAnimationThread.h
//

#if !defined(AFX_GIFANIMATIONTHREAD_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
#define AFX_GIFANIMATIONTHREAD_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_

#pragma once

#include "WorkerThread.h"
#include "Dib.h"

// If there are frames which are less than 20ms
// the animated gif is clipped to 100ms
#define MIN_ACCETABLE_ANIMGIF_FRAMETIME		20

// The Gif Animation Thread Class
class CGifAnimationThread : public CWorkerThread
{
	public:
		CGifAnimationThread(){	m_sFileName = _T(""); m_bClipTo100ms = FALSE;
								m_bShowPrevious = FALSE; m_bShowNext = FALSE;
								m_dwDibAnimationPos = 0; m_dwDibAnimationCount = 0;};
		virtual ~CGifAnimationThread(){Kill(); ClearAnimationArrays();};
		void SetFileName(CString sFileName) {m_sFileName = sFileName;};
		void SetClipTo100ms(BOOL bDoClip = TRUE) {m_bClipTo100ms = bDoClip;};
		BOOL IsClipTo100ms() const {return m_bClipTo100ms;};
		void ClearAnimationArrays();
		int Load(BOOL bDoAlphaRender);
		void ShowPrevious();
		void ShowNext();
		void ShowFirst();
		void ShowLast();
		BOOL InitAlphaRender();
		void AlphaRender(COLORREF crBackgroundColor);

		CDib::ARRAY m_DibAnimationArray;
		CDib::ARRAY m_AlphaRenderedDibAnimationArray;
		DWORD m_dwDibAnimationPos;
		DWORD m_dwDibAnimationCount;
	
	protected:
		virtual void OnNewFrame(){;};
		virtual void OnPlayTimesDone(){;};
		int Work();

		CString m_sFileName;
		BOOL m_bShowPrevious;
		BOOL m_bShowNext;
		// Most browsers clip everything faster than a 100ms delay (0..99ms) to 100 ms,
		// because of corrupted GIFs. IE started this, quite all browsers followed it
		// (state in year 2005). I do not know in future...
		BOOL m_bClipTo100ms;
};

#endif // !defined(AFX_GIFANIMATIONTHREAD_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
