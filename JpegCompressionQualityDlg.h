#if !defined(AFX_JPEGCOMPRESSIONQUALITYDLG_H__36C8EF9C_C1BE_45A6_9C01_E39D5BA97893__INCLUDED_)
#define AFX_JPEGCOMPRESSIONQUALITYDLG_H__36C8EF9C_C1BE_45A6_9C01_E39D5BA97893__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// JpegCompressionQualityDlg.h : header file
//

#include "resource.h"
 
/////////////////////////////////////////////////////////////////////////////
// CJpegCompressionQualityDlg dialog

class CJpegCompressionQualityDlg : public CDialog
{
// Construction
public:
	CJpegCompressionQualityDlg(CWnd* pParent = NULL);   // standard constructor
	int m_nLastPos;
	int m_nCompressionQuality;

// Dialog Data
	//{{AFX_DATA(CJpegCompressionQualityDlg)
	enum { IDD = IDD_JPEG_COMPRESSION_QUALITY };
	CSliderCtrl	m_CompressionQualitySlider;
	BOOL	m_bSaveAsGrayscale;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJpegCompressionQualityDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CJpegCompressionQualityDlg)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JPEGCOMPRESSIONQUALITYDLG_H__36C8EF9C_C1BE_45A6_9C01_E39D5BA97893__INCLUDED_)
