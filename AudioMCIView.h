#if !defined(AFX_AUDIOMCIVIEW_H__DAC2D8F9_1ED2_42BD_8BFF_73ECE3445B8E__INCLUDED_)
#define AFX_AUDIOMCIVIEW_H__DAC2D8F9_1ED2_42BD_8BFF_73ECE3445B8E__INCLUDED_

#pragma once

// AudioMCIView.h : header file
//

#include "DibStatic.h"
#include "AudioMCIDoc.h"

/////////////////////////////////////////////////////////////////////////////
// CAudioMCIView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CAudioMCIView : public CFormView
{
protected:
	CAudioMCIView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CAudioMCIView)

public:
	CAudioMCIDoc* GetDocument();

	CDibStatic m_DibStatic;

	//{{AFX_DATA(CAudioMCIView)
	enum { IDD = IDD_MCIAUDIOPLAYER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioMCIView)
	public:
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CAudioMCIView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CAudioMCIView)
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG
inline CAudioMCIDoc* CAudioMCIView::GetDocument()
   { return (CAudioMCIDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDIOMCIVIEW_H__DAC2D8F9_1ED2_42BD_8BFF_73ECE3445B8E__INCLUDED_)
