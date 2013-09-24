#if !defined(AFX_ANIMGIFSAVEDLG_H__14AAD4C5_428C_4D47_B92C_89911CD4A390__INCLUDED_)
#define AFX_ANIMGIFSAVEDLG_H__14AAD4C5_428C_4D47_B92C_89911CD4A390__INCLUDED_

#pragma once

// AnimGifSaveDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAnimGifSaveDlg dialog

class CAnimGifSaveDlg : public CDialog
{
// Construction
public:
	CAnimGifSaveDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAnimGifSaveDlg)
	enum { IDD = IDD_ANIMGIF_SAVE };
	BOOL	m_bDitherColorConversion;
	int		m_nColorTables;
	int		m_nNumColors;
	UINT	m_uiPlayTimes;
	int		m_nLoopInfinite;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimGifSaveDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAnimGifSaveDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioLoop();
	afx_msg void OnRadioLoopInfinite();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMGIFSAVEDLG_H__14AAD4C5_428C_4D47_B92C_89911CD4A390__INCLUDED_)
