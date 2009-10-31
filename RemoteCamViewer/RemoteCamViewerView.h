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
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
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
	void ClipToView(LPRECT lpRect) const;
	void RepositionCams();

// Generated message map functions
protected:
	//{{AFX_MSG(CRemoteCamViewerView)
	afx_msg void OnHelpAboutctrl();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnToolsShowprop0();
	afx_msg void OnToolsShowprop1();
	afx_msg void OnToolsShowprop2();
	afx_msg void OnToolsShowprop3();
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
