#if !defined(AFX_NOVISTAFILEDLG_H__8F42E4A5_F751_11D4_B4C9_00010223337E__INCLUDED_)
#define AFX_NOVISTAFILEDLG_H__8F42E4A5_F751_11D4_B4C9_00010223337E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NoVistaFileDlg.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CNoVistaFileDlg dialog

class CNoVistaFileDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CNoVistaFileDlg)

public:
	CNoVistaFileDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

protected:
	//{{AFX_MSG(CNoVistaFileDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NOVISTAFILEDLG_H__8F42E4A5_F751_11D4_B4C9_00010223337E__INCLUDED_)
