#if !defined(AFX_VIDEOFORMATDLG_H__D6D9A878_9585_44A0_95D8_1FB0677FB155__INCLUDED_)
#define AFX_VIDEOFORMATDLG_H__D6D9A878_9585_44A0_95D8_1FB0677FB155__INCLUDED_

#pragma once

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
	int			m_nVideoCompressorKeyframesRate;
	//}}AFX_DATA
	DWORD m_dwVideoCompressorFourCC;
	float m_fVideoCompressorQuality;
	enum FILETYPE {FILETYPE_AVI, FILETYPE_SWF, FILETYPE_MP4};
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
	void UpdateVideoQualityInfo();
	__forceinline int GetRevertedPos(CSliderCtrl& SliderCtrl) {return SliderCtrl.GetRangeMin() + (SliderCtrl.GetRangeMax() - SliderCtrl.GetPos());};
	__forceinline void SetRevertedPos(CSliderCtrl& SliderCtrl, int nPos) {SliderCtrl.SetPos(SliderCtrl.GetRangeMin() + (SliderCtrl.GetRangeMax() - nPos));};
	CDWordArray m_VideoCompressionFcc;
	CDWordArray m_VideoCompressionKeyframesRateSupport;
	CDWordArray m_VideoCompressionQualitySupport;
	// Generated message map functions
	//{{AFX_MSG(CVideoFormatDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeVideoCompressionChoose();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDEOFORMATDLG_H__D6D9A878_9585_44A0_95D8_1FB0677FB155__INCLUDED_)
