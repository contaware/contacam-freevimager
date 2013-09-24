#if !defined(AFX_PRINTPREVIEWSCALEEDIT_H__BE1F6DD1_AF2F_4964_9BFD_149F48B10EFE__INCLUDED_)
#define AFX_PRINTPREVIEWSCALEEDIT_H__BE1F6DD1_AF2F_4964_9BFD_149F48B10EFE__INCLUDED_

#pragma once

// PrintPreviewScaleEdit.h : header file
//

#include "NumEdit.h"

// Forward Declaration
class CPictureDoc;
class CPicturePrintPreviewView;


/////////////////////////////////////////////////////////////////////////////
// CPrintPreviewScaleEdit window

class CPrintPreviewScaleEdit : public CNumEdit
{
// Construction
public:
	CPrintPreviewScaleEdit(	CPictureDoc* pDoc,
							CPicturePrintPreviewView* pPrintPreview);

	BOOL SetPrintScale(double dPrintScale);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrintPreviewScaleEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPrintPreviewScaleEdit();

protected:
	CPictureDoc* m_pDoc;
	CPicturePrintPreviewView* m_pPrintPreview;
	BOOL m_bUpdate;
	
	// Generated message map functions
protected:
	//{{AFX_MSG(CPrintPreviewScaleEdit)
	afx_msg void OnChange();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRINTPREVIEWSCALEEDIT_H__BE1F6DD1_AF2F_4964_9BFD_149F48B10EFE__INCLUDED_)
