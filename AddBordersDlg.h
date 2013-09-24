#if !defined(AFX_ADDBORDERSDLG_H__051426CD_80E8_4699_A3E6_D9CC3BDC19B9__INCLUDED_)
#define AFX_ADDBORDERSDLG_H__051426CD_80E8_4699_A3E6_D9CC3BDC19B9__INCLUDED_

#pragma once

// AddBordersDlg.h : header file
//

#include "ColorButton.h"
#include "PaletteWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CAddBordersDlg dialog

class CAddBordersDlg : public CDialog
{
// Construction
public:
	CAddBordersDlg(CWnd* pParent);

// Dialog Data
	//{{AFX_DATA(CAddBordersDlg)
	enum { IDD = IDD_ADD_BORDERS };
	CColorButton	m_PickColor;
	UINT	m_uiLeftBorder;
	UINT	m_uiBottomBorder;
	UINT	m_uiRightBorder;
	UINT	m_uiTopBorder;
	//}}AFX_DATA

	COLORREF m_crBorder;
	UINT m_uiBorderIndex;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddBordersDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CPaletteWnd* m_pWndPalette;

	// Generated message map functions
	//{{AFX_MSG(CAddBordersDlg)
	afx_msg void OnPickColor();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	afx_msg LRESULT OnColorPicked(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnColorPickerClosed(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDBORDERSDLG_H__051426CD_80E8_4699_A3E6_D9CC3BDC19B9__INCLUDED_)
