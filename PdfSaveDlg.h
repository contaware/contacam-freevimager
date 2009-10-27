#if !defined(AFX_PDFSAVEDLG_H__0090AD6C_1B3A_4C18_BD0C_0892F529D0CD__INCLUDED_)
#define AFX_PDFSAVEDLG_H__0090AD6C_1B3A_4C18_BD0C_0892F529D0CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PdfSaveDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPdfSaveDlg dialog

class CPdfSaveDlg : public CDialog
{
// Construction
public:
	CPdfSaveDlg(CWnd* pParent = NULL);   // standard constructor
	int m_nCompressionQuality;
	CString m_sPdfScanPaperSize;

// Dialog Data
	//{{AFX_DATA(CPdfSaveDlg)
	enum { IDD = IDD_PDF_SAVE };
	CSliderCtrl	m_CompressionQualitySlider;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPdfSaveDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_nLastPos;
	// Generated message map functions
	//{{AFX_MSG(CPdfSaveDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PDFSAVEDLG_H__0090AD6C_1B3A_4C18_BD0C_0892F529D0CD__INCLUDED_)
