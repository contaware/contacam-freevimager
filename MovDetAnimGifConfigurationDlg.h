#if !defined(AFX_MOVDETANIMGIFCONFIGURATIONDLG_H__F1B48863_D6E1_4937_9740_2CDE8D26762F__INCLUDED_)
#define AFX_MOVDETANIMGIFCONFIGURATIONDLG_H__F1B48863_D6E1_4937_9740_2CDE8D26762F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MovDetAnimGifConfigurationDlg.h : header file
//

#ifdef VIDEODEVICEDOC

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CMovDetAnimGifConfigurationDlg dialog

class CMovDetAnimGifConfigurationDlg : public CDialog
{
// Construction
public:
	CMovDetAnimGifConfigurationDlg(	int nFullWidth, int nFullHeight,
					int nCurrentWidth, int nCurrentHeight,
					int nSpeedMulIndex, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CMovDetAnimGifConfigurationDlg)
	enum { IDD = IDD_MOVDET_ANIMGIF_CONFIGURATION };
	int		m_nPixelsWidth;
	int		m_nPixelsHeight;
	int		m_nPercentWidth;
	int		m_nPercentHeight;
	BOOL	m_bRetainAspect;
	int		m_nSpeedMulIndex;
	//}}AFX_DATA

	int m_nFullWidth;
	int m_nFullHeight;
	double m_dFullRatio;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMovDetAnimGifConfigurationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL m_bDlgInitialized;
	BOOL m_bNormalValidate;
	double m_dMaxResizeFactor;

	// Generated message map functions
	//{{AFX_MSG(CMovDetAnimGifConfigurationDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditPercentHeight();
	afx_msg void OnChangeEditPercentWidth();
	afx_msg void OnChangeEditPixelsHeight();
	afx_msg void OnChangeEditPixelsWidth();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOVDETANIMGIFCONFIGURATIONDLG_H__F1B48863_D6E1_4937_9740_2CDE8D26762F__INCLUDED_)
