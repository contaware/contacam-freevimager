#if !defined(AFX_SAVEFILEDLG_H__8F42E4A5_F751_11D4_B4C9_00010223337E__INCLUDED_)
#define AFX_SAVEFILEDLG_H__8F42E4A5_F751_11D4_B4C9_00010223337E__INCLUDED_

#pragma once

// SaveFileDlg.h : header file
//

#include "resource.h"

#define SAVEFILEDLG_LEFT_OFFSET			9
#define SAVEFILEDLG_RIGHT_OFFSET		40
#define SAVEFILEDLG_BOTTOM_OFFSET		10
#define SAVEFILEDLG_SLIDERLABEL_WIDTH	160
#define SAVEFILEDLG_SLIDER_HEIGHT		30
#define SAVEFILEDLG_SLIDERTEXT_WIDTH	25

/////////////////////////////////////////////////////////////////////////////
// CSaveFileDlg dialog

class CSaveFileDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CSaveFileDlg)

public:
	CSaveFileDlg(	BOOL bShowJPEGCompression,
					LPCTSTR lpszDefExt = NULL,
					LPCTSTR lpszFileName = NULL,
					DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					LPCTSTR lpszFilter = NULL,
					CWnd* pParentWnd = NULL);
	__forceinline int GetJpegCompressionQuality() const {return m_nJpegCompressionQuality;};

protected:
	//{{AFX_MSG(CSaveFileDlg)
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	BOOL m_bShowJPEGCompression;
	int m_nJpegCompressionQuality;
	CStatic m_SliderLabel;
	CStatic m_SliderText;
	CSliderCtrl m_SliderCtrl;
	virtual void OnInitDone();
	virtual void OnTypeChange();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAVEFILEDLG_H__8F42E4A5_F751_11D4_B4C9_00010223337E__INCLUDED_)
