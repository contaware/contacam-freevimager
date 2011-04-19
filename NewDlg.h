#if !defined(AFX_NEWDLG_H__F1B48863_D6E1_4937_9740_2CDE8D26762F__INCLUDED_)
#define AFX_NEWDLG_H__F1B48863_D6E1_4937_9740_2CDE8D26762F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewDlg.h : header file
//

#include "resource.h"
#include "NumSpinCtrl.h"
#include "NumEdit.h"
#include "ColorButton.h"

/////////////////////////////////////////////////////////////////////////////
// CNewDlg dialog

class CNewDlg : public CDialog
{
// Construction
public:
	COLORREF m_crBackgroundColor;
	CNewDlg(int nWidth, int nHeight,
			int nXDpi, int nYDpi,
			int nPhysUnit,
			const CString& sPaperSize = _T(""),
			COLORREF crBackgroundColor = RGB(255,255,255),
			CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CNewDlg)
	enum { IDD = IDD_NEW };
	CColorButton m_BackgroundColorButton;
	CNumEdit m_EditPhysHeight;
	CNumEdit m_EditPhysWidth;
	CNumSpinCtrl m_SpinPhysWidth;
	CNumSpinCtrl m_SpinPhysHeight;
	int		m_nPixelsHeight;
	int		m_nPixelsWidth;
	int		m_PhysUnit;
	int		m_nXDpi;
	int		m_nYDpi;
	//}}AFX_DATA

	int m_nInitWidth;
	int m_nInitHeight;
	int m_nInitXDpi;
	int m_nInitYDpi;
	BOOL m_bPixAreChanging;
	BOOL m_bPhysOrDpiAreChanging;
	BOOL m_bPaperSizeChanging;
	CString m_sPaperSize;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL m_bDlgInitialized;
	BOOL m_bNormalValidate;

	void FromPixToPhys();
	void FromPixToPhysX();
	void FromPixToPhysY();
	void FromPixToDpi();
	void FromPixToDpiX();
	void FromPixToDpiY();
	void FromPhysAndDpiToPix();
	void FromPhysAndDpiToPixX();
	void FromPhysAndDpiToPixY();
	BOOL PaperNameToPhysSize();

	// Generated message map functions
	//{{AFX_MSG(CNewDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditPixelsHeight();
	afx_msg void OnChangeEditPixelsWidth();
	virtual void OnOK();
	afx_msg void OnChangeEditPhysHeight();
	afx_msg void OnChangeEditPhysWidth();
	afx_msg void OnChangeEditXdpi();
	afx_msg void OnChangeEditYdpi();
	afx_msg void OnSelchangeComboPhysUnit();
	afx_msg void OnCheckRetainAspect();
	afx_msg void OnSelendokComboPaperSize();
	afx_msg void OnPickColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWDLG_H__F1B48863_D6E1_4937_9740_2CDE8D26762F__INCLUDED_)
