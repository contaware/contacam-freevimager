#if !defined(AFX_TIFFSAVEDLG_H__0090AD6C_1B3A_4C18_BD0C_0892F529D0CD__INCLUDED_)
#define AFX_TIFFSAVEDLG_H__0090AD6C_1B3A_4C18_BD0C_0892F529D0CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TiffSaveDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTiffSaveDlg dialog

class CTiffSaveDlg : public CDialog
{
// Construction
public:
	CTiffSaveDlg(CWnd* pParent = NULL);   // standard constructor
	int m_nCompressionQuality;

// Dialog Data
	//{{AFX_DATA(CTiffSaveDlg)
	enum { IDD = IDD_TIFF_SAVE };
	CSliderCtrl	m_CompressionQualitySlider;
	int		m_nCompression;
	//}}AFX_DATA
	int m_nBpp;
	BOOL m_bGrayscale;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTiffSaveDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_nLastPos;
	// Generated message map functions
	//{{AFX_MSG(CTiffSaveDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnRadioCompression();
	afx_msg void OnRadioCompressionCcittfax4();
	afx_msg void OnRadioCompressionLzw();
	afx_msg void OnRadioCompressionJpeg();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TIFFSAVEDLG_H__0090AD6C_1B3A_4C18_BD0C_0892F529D0CD__INCLUDED_)
