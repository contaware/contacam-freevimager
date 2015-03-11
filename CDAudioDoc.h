#if !defined(AFX_CDAUDIODOC_H__CDB9A65F_0E23_4DB1_A99F_BDC77E607F42__INCLUDED_)
#define AFX_CDAUDIODOC_H__CDB9A65F_0E23_4DB1_A99F_BDC77E607F42__INCLUDED_

#pragma once

// CDAudioDoc.h : header file
//

// Forward Declaration
class CCDAudioView;
class CCDAudioChildFrame;
class CCDAudio;

/////////////////////////////////////////////////////////////////////////////
// CCDAudioDoc document

class CCDAudioDoc : public CDocument
{
protected:
	CCDAudioDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CCDAudioDoc)

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCDAudioDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCDAudioDoc();
	
	// Get / Set View / Frame Functions
	CCDAudioView* GetView() const {return m_pView;};
	void SetView(CCDAudioView* pView) {m_pView = pView;};
	CCDAudioChildFrame* GetFrame() const {return m_pFrame;};
	void SetFrame(CCDAudioChildFrame* pFrame) {m_pFrame = pFrame;};

	// Closing Functions
	void CloseDocument();
	void CloseDocumentForce();

	// CD Audio Load Function
	BOOL LoadCD(LPCTSTR lpszFileName);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CCDAudio* m_pCDAudio;

	// File pos vars
	int m_nFileMenuPos;
	int m_nEditMenuPos;
	int m_nViewMenuPos;
	int m_nCaptureMenuPos;
	int m_nPlayMenuPos;
	int m_nSettingsMenuPos;
	int m_nWindowsPos;
	int m_nHelpMenuPos;

protected:
	// This Pointers point to the first view and first frame,
	// this works because in the current implementation
	// we always have only one view and one frame!
	CCDAudioView* m_pView;		// The Only Attached View
	CCDAudioChildFrame* m_pFrame;// The Only Attached Frame

	// Generated message map functions
protected:
	//{{AFX_MSG(CCDAudioDoc)
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDAUDIODOC_H__CDB9A65F_0E23_4DB1_A99F_BDC77E607F42__INCLUDED_)
