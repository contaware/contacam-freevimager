#if !defined(AFX_CDAUDIOVIEW_H__DAC2D8F9_1ED2_42BD_8BFF_73ECE3445B8E__INCLUDED_)
#define AFX_CDAUDIOVIEW_H__DAC2D8F9_1ED2_42BD_8BFF_73ECE3445B8E__INCLUDED_

#pragma once

// CDAudioView.h : header file
//

#include "DibStatic.h"
#include "CDAudioDoc.h"
#include "XButtonXP.h"

/////////////////////////////////////////////////////////////////////////////
// CCDAudioView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CCDAudioView : public CFormView
{
protected:
	CCDAudioView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCDAudioView)

public:
	CCDAudioDoc* GetDocument();

	//{{AFX_DATA(CCDAudioView)
	enum { IDD = IDD_CDAUDIOPLAYER };
	CXButtonXP	m_PauseButton;
	CXButtonXP	m_StopButton;
	CXButtonXP  m_PlayButton;
	CSliderCtrl	m_SliderPos;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCDAudioView)
	public:
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CCDAudioView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	int m_nLastTrackNum;

	// Generated message map functions
	//{{AFX_MSG(CCDAudioView)
	afx_msg void OnButtonPlay();
	afx_msg void OnButtonNext();
	afx_msg void OnButtonPrevious();
	afx_msg void OnButtonStop();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnButtonTrack();
	afx_msg void OnButtonPause();
	//}}AFX_MSG
	afx_msg void OnTrackChange(UINT nID);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG
inline CCDAudioDoc* CCDAudioView::GetDocument()
   { return (CCDAudioDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDAUDIOVIEW_H__DAC2D8F9_1ED2_42BD_8BFF_73ECE3445B8E__INCLUDED_)
