// Written by Marius Bancila (mbancila@yahoo.com)
// Copyright (c) 2004.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. If 
// the source code in  this file is used in any commercial application 
// then acknowledgement must be made to the author of this file 
// (in whatever form you wish).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
// IN THE SOFTWARE.
// 
// Please use and enjoy. Please let me know of any bugs/mods/improvements 
// that you have found/implemented and I will fix/incorporate them into this
// file. 


#if !defined(AFX_COLORBUTTON_H__EB5FACDB_272F_4883_A997_659DDA42FD38__INCLUDED_)
#define AFX_COLORBUTTON_H__EB5FACDB_272F_4883_A997_659DDA42FD38__INCLUDED_

#pragma once

// ColorButton.h : header file
//

#include "uxtheme.h"
#include "OddButton.h"

/////////////////////////////////////////////////////////////////////////////
// CColorButton window

class CColorButton : public COddButton
{
private:
	enum {	BUTTON_IN			= 0x01,
			BUTTON_OUT			= 0x02,
			BUTTON_BLACK_BORDER	= 0x04,};
	
// Construction
public:
	CColorButton();
	CColorButton(COLORREF text, COLORREF bkgnd);
	CColorButton(COLORREF text, COLORREF bkgnd, COLORREF disabled);
	CColorButton(COLORREF text, COLORREF bkgnd, COLORREF disabled, COLORREF light, COLORREF highlight, COLORREF shadow, COLORREF darkShadow);
	
// Attributes
public:

private:
	BOOL		m_bTracking;
	BOOL		m_bHover;
	HTHEME		m_hTheme;
	BOOL		m_bFirstTime;
	BOOL		m_bToolbarButton;
	BOOL		m_bUseDefaultColor;
	COLORREF	m_TextColor;
	COLORREF	m_BkgndColor;
	COLORREF	m_DisabledBkgndColor;
	COLORREF	m_Light;
	COLORREF	m_Highlight;
	COLORREF	m_Shadow;
	COLORREF	m_DarkShadow;
	
// Operations
public:
	void SetColor(COLORREF text, COLORREF bkgnd);
	void SetColor(COLORREF text, COLORREF bkgnd, COLORREF disabled);
	void SetColor(COLORREF text, COLORREF bkgnd, COLORREF disabled, COLORREF light, COLORREF highlight, COLORREF shadow, COLORREF darkShadow);
	void SetColorToWindowsDefault();
	BOOL IsDefaultColor() {return m_bUseDefaultColor;};
	BOOL IsThemed();
	void SetToolbarButton(BOOL bToolbarButton) {m_bToolbarButton = bToolbarButton; RedrawWindow();};
	COLORREF GetTextColor() const {return m_TextColor;};
	COLORREF GetBkgColor() const {return m_BkgndColor;};
	
private:
	void DrawFrame(CDC *pDC, CRect rc, int state);
	void DrawFilledRect(CDC *pDC, CRect rc, COLORREF color);
	void DrawLine(CDC *pDC, long sx, long sy, long ex, long ey, COLORREF color);
	void DrawButtonText(CDC *pDC, CRect rc, CString strCaption, COLORREF textcolor);
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColorButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorButton)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnMouseHover(WPARAM wparam, LPARAM lparam);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORBUTTON_H__EB5FACDB_272F_4883_A997_659DDA42FD38__INCLUDED_)
