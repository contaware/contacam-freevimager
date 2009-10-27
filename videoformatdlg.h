#if !defined(AFX_VIDEOFORMATDLG_H__D6D9A878_9585_44A0_95D8_1FB0677FB155__INCLUDED_)
#define AFX_VIDEOFORMATDLG_H__D6D9A878_9585_44A0_95D8_1FB0677FB155__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VideoFormatDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVideoFormatDlg dialog

class CVideoFormatDlg : public CDialog
{
// Construction
public:
	CVideoFormatDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CVideoFormatDlg)
	enum { IDD = IDD_VIDEOFORMAT };
	CSliderCtrl	m_VideoCompressorQuality;
	CComboBox	m_VideoCompressionChoose;
	int		m_nVideoCompressorDataRate;		// Data Rate in kbps
	int		m_nVideoCompressorKeyframesRate;
	BOOL	m_bDeinterlace;
	int		m_nRawCompressionIndex;
	int		m_nQualityBitrate;
	//}}AFX_DATA
	DWORD m_dwVideoCompressorFourCC;
	float m_fVideoCompressorQuality;
	double m_dVideoLength;
	BOOL m_bShowDeinterlaceCheck;
	BOOL m_bShowRawChoose;
	LONGLONG m_llTotalAudioBytes;
	enum FILETYPE {FILETYPE_AVI, FILETYPE_SWF};
	FILETYPE m_nFileType;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVideoFormatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void ShowHideCtrls();
	void UpdateLength();
	CDWordArray m_VideoCompressionFcc;
	CDWordArray m_VideoCompressionKeyframesRateSupport;
	CDWordArray m_VideoCompressionDataRateSupport;
	CDWordArray m_VideoCompressionQualitySupport;
	// Generated message map functions
	//{{AFX_MSG(CVideoFormatDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeVideoCompressionChoose();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnChangeEditDatarate();
	virtual void OnOK();
	afx_msg void OnRadioQuality();
	afx_msg void OnRadioBitrate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDEOFORMATDLG_H__D6D9A878_9585_44A0_95D8_1FB0677FB155__INCLUDED_)
