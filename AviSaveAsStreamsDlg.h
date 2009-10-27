#if !defined(AFX_AVISAVEASSTREAMSDLG_H__E8E3E27D_ECC2_48B4_BD41_EB2633391B17__INCLUDED_)
#define AFX_AVISAVEASSTREAMSDLG_H__E8E3E27D_ECC2_48B4_BD41_EB2633391B17__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AviSaveAsStreamsDlg.h : header file
//

#include "resource.h"
#include "HScrollListBox.h"

// Forward Declaration
class CAVIPlay;

/////////////////////////////////////////////////////////////////////////////
// CAviSaveAsStreamsDlg dialog

class CAviSaveAsStreamsDlg : public CDialog
{
// Construction
public:
	CAviSaveAsStreamsDlg(	CAVIPlay* pAVIPlay,
							int nDlgID,
							CDWordArray* pVideoStreamsSave,
							CDWordArray* pVideoStreamsChange,
							CDWordArray* pAudioStreamsSave,
							CDWordArray* pAudioStreamsChange,
							CWnd* pParent = NULL);   // standard constructor
	BOOL m_bUseVfwCodecs;

// Dialog Data
	//{{AFX_DATA(CAviSaveAsStreamsDlg)
	CHScrollListBox	m_StreamSave;
	CHScrollListBox	m_StreamChange;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAviSaveAsStreamsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CAVIPlay* m_pAVIPlay;
	CDWordArray* m_pVideoStreamsSave;
	CDWordArray* m_pVideoStreamsChange;
	CDWordArray* m_pAudioStreamsSave;
	CDWordArray* m_pAudioStreamsChange;

	// Generated message map functions
	//{{AFX_MSG(CAviSaveAsStreamsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelchangeStreamSave();
	afx_msg void OnSelchangeStreamChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AVISAVEASSTREAMSDLG_H__E8E3E27D_ECC2_48B4_BD41_EB2633391B17__INCLUDED_)
