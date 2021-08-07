// MDISnapper.h: interface for the CMDISnapper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MDISNAPPER_H__C7094C4B_78CF_4B26_BF01_26C80F1A0F0F__INCLUDED_)
#define AFX_MDISNAPPER_H__C7094C4B_78CF_4B26_BF01_26C80F1A0F0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


struct SNAPINFO
{
	enum { MAXDELTA = (1<<30) };

	DWORD	snapWidth;

	RECT	rold;
	RECT	rnew;
	RECT	rdelta;

	RECT	rout;

	void	Init(RECT const & oldRect, RECT const & newRect, DWORD snapwidth);
	void	Init(RECT const & r, DWORD snapWidth, bool moveOnly = false);
	void	SnapHLine(long y);		// snap to a horizontal line
	void	SnapVLine(long x);		// snap to a vertical line

	RECT &	EndSnap();  // final coords in rout
};





class CMDISnapper  
{
public:

protected:
	DWORD				m_snapWidth;
    bool                m_bSizeMoveIsSysCommand;    ///< size/move was selected from the menu

	// helper for "move" coordinates
	CWnd *				m_wndMoving; 
	POINT				m_mouseStart;
	RECT                m_rectStart;

    // helper for swap
    CWnd *              m_swapOther;
    RECT                m_swapOtherRect;

    CWnd * PickSwapWindow(CWnd * base, POINT mouse);


public:
			 CMDISnapper(DWORD snapWidth = 8);
	virtual ~CMDISnapper();

	void	SetSnapWidth(DWORD snapWidth);
	DWORD	GetSnapWidth() const		{ return m_snapWidth; }

	LRESULT	OnMessage(CWnd * wnd, UINT msg, WPARAM wp, LPARAM lp);

  	void	Sizing(CWnd * wnd, RECT & rnew);

	void	OnEnterSizeMove(CWnd * wnd);
	void	OnExitSizeMove();
	void	OnSizing(CWnd * wnd, WPARAM edge, LPRECT newrect);
	void	OnMoving(CWnd * wnd, WPARAM edge, LPRECT newrect);

    virtual bool AllowSnap();



};

#endif // !defined(AFX_MDISNAPPER_H__C7094C4B_78CF_4B26_BF01_26C80F1A0F0F__INCLUDED_)


/*

  XSgn is required for the following setup:

		  Snap 1        Snap 2
			|             |
			|        |  | |
                     |<-|
					 |<-|
					 |  |
				  rnew  rold

		although rnew is closer to snap2, we should snap to snap1
		since user wants "move to left"


*/