#if !defined(AFX_AUDIOMCIDOC_H__CDB9A65F_0E23_4DB1_A99F_BDC77E607F42__INCLUDED_)
#define AFX_AUDIOMCIDOC_H__CDB9A65F_0E23_4DB1_A99F_BDC77E607F42__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AudioMCIDoc.h : header file
//

// Forward Declaration
class CAudioMCIView;
class CAudioMCIChildFrame;

/////////////////////////////////////////////////////////////////////////////
// CAudioMCIDoc document

class CAudioMCIDoc : public CDocument
{
protected:
	CAudioMCIDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CAudioMCIDoc)

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioMCIDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAudioMCIDoc();
	
	// Get / Set View / Frame Functions
	CAudioMCIView* GetView() const {return m_pView;};
	void SetView(CAudioMCIView* pView) {m_pView = pView;};
	CAudioMCIChildFrame* GetFrame() const {return m_pFrame;};
	void SetFrame(CAudioMCIChildFrame* pFrame) {m_pFrame = pFrame;};

	// Closing Functions
	void CloseDocument();
	void CloseDocumentForce();

	// Delete File
	void EditDelete(BOOL bPrompt);

	// Audio Load Function
	BOOL LoadAudio(LPCTSTR lpszFileName);

	// Timer Id
	UINT m_uiTimerId;

	// File pos vars
	int m_nFileMenuPos;
	int m_nEditMenuPos;
	int m_nViewMenuPos;
	int m_nCaptureMenuPos;
	int m_nPlayMenuPos;
	int m_nToolsMenuPos;
	int m_nWindowsPos;
	int m_nHelpMenuPos;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void DeleteDocFile();
	// This Pointers point to the first view and first frame,
	// this works because in the current implementation
	// we always have only one view and one frame!
	CAudioMCIView* m_pView;		// The Only Attached View
	CAudioMCIChildFrame* m_pFrame;// The Only Attached Frame

	// Generated message map functions
protected:
	//{{AFX_MSG(CAudioMCIDoc)
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnEditRename();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDIOMCIDOC_H__CDB9A65F_0E23_4DB1_A99F_BDC77E607F42__INCLUDED_)
