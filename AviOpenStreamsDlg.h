#if !defined(AFX_AVIOPENSTREAMSDLG_H__EC0B4FA3_8D4D_4C61_B530_09F763A336EC__INCLUDED_)
#define AFX_AVIOPENSTREAMSDLG_H__EC0B4FA3_8D4D_4C61_B530_09F763A336EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AviOpenStreamsDlg.h : header file
//

#include "resource.h"
#include "HScrollListBox.h"

// Forward Declaration
class CVideoAviDoc;

/////////////////////////////////////////////////////////////////////////////
// CAviOpenStreamsDlg dialog

class CAviOpenStreamsDlg : public CDialog
{
// Construction
public:
	CAviOpenStreamsDlg(CVideoAviDoc* pDoc, CWnd* pParent = NULL);   // standard constructor

	CVideoAviDoc* m_pDoc;

// Dialog Data
	//{{AFX_DATA(CAviOpenStreamsDlg)
	enum { IDD = IDD_OPEN_STREAMS_SELECT };
	CHScrollListBox	m_VideoStreams;
	CHScrollListBox m_AudioStreams;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAviOpenStreamsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAviOpenStreamsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AVIOPENSTREAMSDLG_H__EC0B4FA3_8D4D_4C61_B530_09F763A336EC__INCLUDED_)
