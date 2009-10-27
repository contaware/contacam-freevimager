// RemoteCamViewerDoc.h : interface of the CRemoteCamViewerDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_REMOTECAMVIEWERDOC_H__06025360_F038_47B9_8F02_45A59EB4C5A6__INCLUDED_)
#define AFX_REMOTECAMVIEWERDOC_H__06025360_F038_47B9_8F02_45A59EB4C5A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CRemoteCamViewerDoc : public CDocument
{
protected: // create from serialization only
	CRemoteCamViewerDoc();
	DECLARE_DYNCREATE(CRemoteCamViewerDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRemoteCamViewerDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRemoteCamViewerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CRemoteCamViewerDoc)
	afx_msg void OnToolsRegister();
	afx_msg void OnToolsUnregister();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REMOTECAMVIEWERDOC_H__06025360_F038_47B9_8F02_45A59EB4C5A6__INCLUDED_)
