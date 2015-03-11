#if !defined(AFX_UIMAGERDOC_H__24956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_)
#define AFX_UIMAGERDOC_H__24956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_

#pragma once

#include "Dib.h"

// The Base Document Class
class CUImagerDoc : public CDocument
{
protected: // create from serialization only
	DECLARE_DYNCREATE(CUImagerDoc)
	CUImagerDoc();
	virtual ~CUImagerDoc();

public:
	// General Vars
	int m_nFileMenuPos;
	int m_nEditMenuPos;
	int m_nViewMenuPos;
	int m_nCaptureMenuPos;
	int m_nPlayMenuPos;
	int m_nSettingsMenuPos;
	int m_nWindowsPos;
	int m_nHelpMenuPos;
	volatile LONG m_bClosing;		// Set in OnClose() of Frame Window
	CDib* volatile m_pDib;			// The Picture or the Current Frame
	CRITICAL_SECTION m_csDib;		// Critical Section for the Dib Object
	CRect m_DocRect;				// The Document Size
	double m_dZoomFactor;			// The Zoom Factor: 1.0 is 100% (original size)
	CString m_sFileName;			// The Document File Name
	COLORREF m_crBackgroundColor;	// Background Color
	BOOL m_bNoBorders;				// Hide / Show borders

public:
	// Set The Title of the Document,
	// implemented in the derived classes
	virtual void SetDocumentTitle() {CDocument::SetTitle(_T("Empty Doc"));};
	void InvalidateAllViews(BOOL bErase = TRUE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUImagerDoc)
	public:
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

#ifdef _DEBUG
public:
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif	

// Generated message map functions
protected:
	//{{AFX_MSG(CUImagerDoc)
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UIMAGERDOC_H__24956A13_6E82_4959_931F_F512BE9B17A0__INCLUDED_)
