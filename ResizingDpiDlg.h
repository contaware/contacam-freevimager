#if !defined(AFX_RESIZINGDPIDLG_H__F1B48863_D6E1_4937_9740_2CDE8D26762F__INCLUDED_)
#define AFX_RESIZINGDPIDLG_H__F1B48863_D6E1_4937_9740_2CDE8D26762F__INCLUDED_

#pragma once

// ResizingDpiDlg.h : header file
//

#include "resource.h"
#include "NumSpinCtrl.h"
#include "NumEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CResizingDpiDlg dialog

class CResizingDpiDlg : public CDialog
{
// Construction
public:
	enum {AVG = 0, NEARESTNEIGHBOR, BILINEAR, BICUBIC};
	double m_dRatio;
	CResizingDpiDlg(int nWidth, int nHeight,
					int nXDpi, int nYDpi,
					int nResizingMethod,
					CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CResizingDpiDlg)
	enum { IDD = IDD_RESIZING_DPI };
	CNumEdit m_EditPhysHeight;
	CNumEdit m_EditPhysWidth;
	CNumSpinCtrl m_SpinPhysWidth;
	CNumSpinCtrl m_SpinPhysHeight;
	int		m_nPercentHeight;
	int		m_nPercentWidth;
	int		m_nPixelsHeight;
	int		m_nPixelsWidth;
	BOOL	m_bRetainAspect;
	int		m_ResizingMethod;
	int		m_PhysUnit;
	int		m_nXDpi;
	int		m_nYDpi;
	//}}AFX_DATA

	int m_nInitResizingMethod;
	int m_nInitWidth;
	int m_nInitHeight;
	double m_dMaxResizeFactor;
	int m_nInitXDpi;
	int m_nInitYDpi;
	BOOL m_bPixAndPercentAreChanging;
	BOOL m_bPhysOrDpiAreChanging;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResizingDpiDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL m_bDlgInitialized;
	BOOL m_bNormalValidate;

	void Reset();
	double GetMaxPhysWidth();
	double GetMaxPhysHeight();
	void FromPixToPhys();
	void FromPixToPhysX();
	void FromPixToPhysY();
	void FromPixToDpi();
	void FromPixToDpiX();
	void FromPixToDpiY();
	void FromPhysAndDpiToPixAndPercent();
	void FromPhysAndDpiToPixAndPercentX();
	void FromPhysAndDpiToPixAndPercentY();

	// Generated message map functions
	//{{AFX_MSG(CResizingDpiDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditPercentHeight();
	afx_msg void OnChangeEditPercentWidth();
	afx_msg void OnChangeEditPixelsHeight();
	afx_msg void OnChangeEditPixelsWidth();
	virtual void OnOK();
	afx_msg void OnChangeEditPhysHeight();
	afx_msg void OnChangeEditPhysWidth();
	afx_msg void OnChangeEditXdpi();
	afx_msg void OnChangeEditYdpi();
	afx_msg void OnSelchangeComboPhysUnit();
	afx_msg void OnCheckRetainAspect();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESIZINGDPIDLG_H__F1B48863_D6E1_4937_9740_2CDE8D26762F__INCLUDED_)
