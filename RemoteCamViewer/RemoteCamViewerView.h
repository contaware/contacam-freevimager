// RemoteCamViewerView.h : interface of the CRemoteCamViewerView class
//
/////////////////////////////////////////////////////////////////////////////
//{{AFX_INCLUDES()
#include "remotecam.h"
//}}AFX_INCLUDES

#if !defined(AFX_REMOTECAMVIEWERVIEW_H__D85B1F9B_EC9B_40D3_B8F3_E0C0AF07755B__INCLUDED_)
#define AFX_REMOTECAMVIEWERVIEW_H__D85B1F9B_EC9B_40D3_B8F3_E0C0AF07755B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CRemoteCamViewerView : public CFormView
{
protected: // create from serialization only
	CRemoteCamViewerView();
	DECLARE_DYNCREATE(CRemoteCamViewerView)

public:
	//{{AFX_DATA(CRemoteCamViewerView)
	enum { IDD = IDD_REMOTECAMVIEWER_FORM };
	CRemoteCam	m_RemoteCam0;
	CRemoteCam	m_RemoteCam1;
	CRemoteCam	m_RemoteCam2;
	CRemoteCam	m_RemoteCam3;
	//}}AFX_DATA

// Attributes
public:
	CRemoteCamViewerDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRemoteCamViewerView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRemoteCamViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CSize m_szRemoteCamSize0;
	CSize m_szRemoteCamSize1;
	CSize m_szRemoteCamSize2;
	CSize m_szRemoteCamSize3;
	BOOL m_bInit0, m_bInit1, m_bInit2, m_bInit3;
	CRect m_rc0, m_rc1, m_rc2, m_rc3;
	CString m_sTitle;
	CString m_sLabel0;
	CString m_sLabel1;
	CString m_sLabel2;
	CString m_sLabel3;
	CFont m_LabelFont;
	int m_nTitleHeight;
	CFont m_TitleFont;
	void ClipToView(LPRECT lpRect) const;
	void RepositionCams();

// Generated message map functions
protected:
	//{{AFX_MSG(CRemoteCamViewerView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnHelpAboutctrl();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnFileShowprop0();
	afx_msg void OnFileShowprop1();
	afx_msg void OnFileShowprop2();
	afx_msg void OnFileShowprop3();
	afx_msg void OnFileSettings();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in RemoteCamViewerView.cpp
inline CRemoteCamViewerDoc* CRemoteCamViewerView::GetDocument()
   { return (CRemoteCamViewerDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REMOTECAMVIEWERVIEW_H__D85B1F9B_EC9B_40D3_B8F3_E0C0AF07755B__INCLUDED_)
