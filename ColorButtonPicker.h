//***************************************************************************
//
// AUTHOR:  James White (feel free to remove or otherwise mangle any part)
//
// DESCRIPTION: This class is alarmingly similar to the CColourPicker control
//	created by Chris Maunder of www.codeproject.com. It is so as it was blatantly
//	copied from that class and is entirely dependant on his other great work
//  in CColourPopup. I was hoping for (cough.. gag..) a more Microsoft look
//  and I think this is pretty close. Hope you like it.
//
// ORIGINAL: http://www.codeproject.com/miscctrl/colour_picker.asp
//
//***************************************************************************
#ifndef COLORBUTTONPICKER_INCLUDED
#define COLORBUTTONPICKER_INCLUDED
#pragma once

#ifndef COLOURPOPUP_INCLUDED
#include "ColourPopup.h"
#endif//COLOURPOPUP_INCLUDED

#include "uxtheme.h"
#include "OddButton.h"

void AFXAPI DDX_ColorButtonPicker(CDataExchange *pDX, int nIDC, COLORREF& crColour);

#define UNSELECT_TIMER_ID	200
#define UNSELECT_TIME_MS	100

class CColorButtonPicker : public COddButton
{
public:
	DECLARE_DYNCREATE(CColorButtonPicker);

	//***********************************************************************
	// Name:		CColorButtonPicker
	// Description:	Default constructor.
	// Parameters:	None.
	// Return:		None.	
	// Notes:		None.
	//***********************************************************************
	CColorButtonPicker(void);

	//***********************************************************************
	// Name:		CColorButtonPicker
	// Description:	Destructor.
	// Parameters:	None.
	// Return:		None.		
	// Notes:		None.
	//***********************************************************************
	virtual ~CColorButtonPicker(void);

	//***********************************************************************
	//**                        Property Accessors                         **
	//***********************************************************************	
	__declspec(property(get=GetColor,put=SetColor))						COLORREF	Color;
	__declspec(property(get=GetDefaultColor,put=SetDefaultColor))		COLORREF	DefaultColor;
	__declspec(property(get=GetTrackSelection,put=SetTrackSelection))	BOOL		TrackSelection;
	__declspec(property(put=SetCustomText))								LPCTSTR		CustomText;
	__declspec(property(put=SetDefaultText))							LPCTSTR		DefaultText;

	BOOL IsThemed();

	//***********************************************************************
	// Name:		GetColor
	// Description:	Returns the current color selected in the control.
	// Parameters:	void
	// Return:		COLORREF 
	// Notes:		None.
	//***********************************************************************
	COLORREF GetColor(void) const;

	//***********************************************************************
	// Name:		SetColor
	// Description:	Sets the current color selected in the control.
	// Parameters:	COLORREF Color
	// Return:		None. 
	// Notes:		None.
	//***********************************************************************
	void SetColor(COLORREF Color);


	//***********************************************************************
	// Name:		GetDefaultColor
	// Description:	Returns the color associated with the 'default' selection.
	// Parameters:	void
	// Return:		COLORREF 
	// Notes:		None.
	//***********************************************************************
	COLORREF GetDefaultColor(void) const;

	//***********************************************************************
	// Name:		SetDefaultColor
	// Description:	Sets the color associated with the 'default' selection.
	//				The default value is COLOR_APPWORKSPACE.
	// Parameters:	COLORREF Color
	// Return:		None. 
	// Notes:		None.
	//***********************************************************************
	void SetDefaultColor(COLORREF Color);

	//***********************************************************************
	// Name:		SetCustomText
	// Description:	Sets the text to display in the 'Custom' selection of the
	//				CColourPicker control, the default text is "More Colors...".
	// Parameters:	LPCTSTR tszText
	// Return:		None. 
	// Notes:		None.
	//***********************************************************************
	void SetCustomText(LPCTSTR tszText);

	//***********************************************************************
	// Name:		SetDefaultText
	// Description:	Sets the text to display in the 'Default' selection of the
	//				CColourPicker control, the default text is "Automatic". If
	//				this value is set to "", the 'Default' selection will not
	//				be shown.
	// Parameters:	LPCTSTR tszText
	// Return:		None. 
	// Notes:		None.
	//***********************************************************************
	void SetDefaultText(LPCTSTR tszText);

	//***********************************************************************
	// Name:		SetProfileSection
	// Description:	Sets the profile section where to write custom colours.
	//				Set this to _T("") to disable.
	// Parameters:	LPCTSTR tszProfileSection
	// Return:		None. 
	// Notes:		None.
	//***********************************************************************
	void SetProfileSection(LPCTSTR tszProfileSection = _T(""));

	//***********************************************************************
	// Name:		SetTrackSelection
	// Description:	Turns on/off the 'Track Selection' option of the control
	//				which shows the colors during the process of selection.
	// Parameters:	BOOL bTrack
	// Return:		None. 
	// Notes:		None.
	//***********************************************************************
	void SetTrackSelection(BOOL bTrack);

	//***********************************************************************
	// Name:		GetTrackSelection
	// Description:	Returns the state of the 'Track Selection' option.
	// Parameters:	void
	// Return:		BOOL 
	// Notes:		None.
	//***********************************************************************
	BOOL GetTrackSelection(void) const;

	//***********************************************************************
	// Name:		SetToolbarButton
	// Description:	Turns on/off the Toolbar Button drawing option
	//				of the control.
	// Parameters:	BOOL bToolbarButton
	// Return:		None. 
	// Notes:		None.
	//***********************************************************************
	void SetToolbarButton(BOOL bToolbarButton);

	//***********************************************************************
	// Name:		GetToolbarButton
	// Description:	Returns the state of the 'Toolbar Button' option.
	// Parameters:	void
	// Return:		BOOL 
	// Notes:		None.
	//***********************************************************************
	BOOL GetToolbarButton(void) const;

	//{{AFX_VIRTUAL(CColorButtonPicker)
    public:
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    //}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CColorButtonPicker)
    afx_msg BOOL OnClicked();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    //}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnMouseHover(WPARAM wparam, LPARAM lparam);
	afx_msg LONG OnSelEndOK(UINT lParam, LONG wParam);
    afx_msg LONG OnSelEndCancel(UINT lParam, LONG wParam);
    afx_msg LONG OnSelChange(UINT lParam, LONG wParam);

	//***********************************************************************
	// Name:		DrawArrow
	// Description:	None.
	// Parameters:	CDC* pDC
	//				RECT* pRect
	//				int iDirection
	//					0 - Down
	//					1 - Up
	//					2 - Left
	//					3 - Right
	// Return:		static None. 
	// Notes:		None.
	//***********************************************************************
	static void DrawArrow(CDC* pDC, 
						  RECT* pRect, 
						  int iDirection = 0,
						  COLORREF clrArrow = RGB(0,0,0));

	void DrawLine(CDC *pDC, long sx, long sy, long ex, long ey, COLORREF color);

	DECLARE_MESSAGE_MAP()

	HTHEME m_hTheme;
	BOOL m_bFirstTime;
	COLORREF m_Color;
	COLORREF m_DefaultColor;
	CString m_strDefaultText;
	CString m_strCustomText;
	CString	m_strProfileSection;
	BOOL	m_bPopupActive;
	BOOL	m_bTrackSelection;
	BOOL	m_bToolbarButton;
	BOOL	m_bTracking;
	BOOL	m_bHover;
};

#endif //!COLORBUTTONPICKER_INCLUDED

