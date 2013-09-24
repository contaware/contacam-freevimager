#if !defined(AFX_BROWSEDLG_H__1D054314_0872_11D2_8A46_0000E81D3D27__INCLUDED_)
#define AFX_BROWSEDLG_H__1D054314_0872_11D2_8A46_0000E81D3D27__INCLUDED_

#pragma once

// BrowseDlg.h : header file
//

#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE		0x0040
#endif
#ifndef BIF_NONEWFOLDERBUTTON
#define BIF_NONEWFOLDERBUTTON	0x0200
#endif
#ifndef BIF_UAHINT
#define BIF_UAHINT				0x0100
#endif

class CBrowseDlg : public CWnd
{
	DECLARE_DYNAMIC(CBrowseDlg)

	typedef struct _DllVersionInfo
	{
			DWORD cbSize;
			DWORD dwMajorVersion;                   // Major version
			DWORD dwMinorVersion;                   // Minor version
			DWORD dwBuildNumber;                    // Build number
			DWORD dwPlatformID;                     // DLLVER_PLATFORM_*
	} DLLVERSIONINFO;

	// Construction
public:
	// Construct
	CBrowseDlg(	CWnd* pParentWnd,
				CString* pDir,
				CString sTitle = _T(""),
				BOOL bNewFolderButton = FALSE,
				BOOL bCheckbox = FALSE,
				CString sCheckboxCaption = _T(""),
				BOOL bChecked = FALSE);
	virtual ~CBrowseDlg();
 
	// ComCtl32 Version
	int GetComCtl32MajorVersion();
	int GetComCtl32MinorVersion();

	// Attributes
public:
	// Retrieve the display name of the selected item
	CString GetDisplayName() const {return m_strDisplayName;};

	// Retrive the Check Box State
	BOOL IsChecked() const {return m_bChecked;};

	// Operations
public:
	// Display the Dialog - returns IDOK or IDCANCEL
	int DoModal();

	// En/Dis-able the "Ok" button
	void EnableOk(BOOL bEnable);

	// Set the current selection in the tree control
	void SetSelection(const CString &strPath);

	// Overrides
public:
	// Called when the Dialog has initialized
	virtual void OnInitDialog();

	// Called when the selection has changed in the tree control
	virtual void OnSelChanged(LPITEMIDLIST pNewSel);

	// Called when the user types an invalid name
	virtual BOOL OnValidateFailed(const CString &strBadName);

	// Generated message map functions
protected:
	//{{AFX_MSG(CBrowseDlg) NOTE - the ClassWizard will add and remove
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg void OnCheckClicked();
	DECLARE_MESSAGE_MAP()

private:
	// Static method to be used for the callback
	static int CALLBACK BrowseCallbackProc(HWND hWnd, UINT nMsg,
										 LPARAM lParam,
										 LPARAM lpData);

	// Free the memory referenced by m_pItemIdList
	void FreeItemIdList(IMalloc *pMalloc = NULL);

	// Calculate the Checkbox Rectangle
	void CalcCheckRect(CRect& rc);

private:
	// Check State
	BOOL m_bChecked;

	// Display a Checkbox
	BOOL m_bCheckbox;

	// The Checkbox Button
	CButton m_Checkbox;

	// The CheckboxWidth
	int m_nCheckboxWidth;

	// The Checkbox Caption
	CString m_sCheckboxCaption;

	// Cleanup COM when finished
	BOOL m_bCleanupCOM;

	// Display name of the selected item
	CString m_strDisplayName;

	// Flags to be passed to the browse dialog
	UINT m_nFlags;

	// Dialog Title
	CString m_sTitle;

	// ITEMIDLIST identifying the selected Shell item
	LPITEMIDLIST m_pItemIdList;

	// Parent CWnd (NULL => App main window)
	CWnd* m_pParentWnd;

	// Selected path
	CString* m_pDir;

	// ITEMIDLIST identifying the root
	LPITEMIDLIST m_pRoot;
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BROWSEDLG_H__1D054314_0872_11D2_8A46_0000E81D3D27__INCLUDED_)
