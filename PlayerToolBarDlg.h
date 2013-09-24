#if !defined(AFX_PLAYERTOOLBARDLG_H__D42904B2_961F_4BBE_8368_F4B63810B415__INCLUDED_)
#define AFX_PLAYERTOOLBARDLG_H__D42904B2_961F_4BBE_8368_F4B63810B415__INCLUDED_

#pragma once

// PlayerToolBarDlg.h : header file
//

#include "ToolBarChildFrm.h"

// Width of ToolBar will be:
// Width-Of-Screen / TOOLBAR_SCREENSIZE_FRACTION
#define TOOLBAR_SCREENSIZE_FRACTION		2.7f
#define TOOLBAR_MIN_SIZE				400

/////////////////////////////////////////////////////////////////////////////
// CPlayerToolBarDlg dialog

class CPlayerToolBarDlg : public CDialog
{
// Construction
public:
	CPlayerToolBarDlg(CWnd* pParent = NULL);
	virtual ~CPlayerToolBarDlg();
	void Close();
	CVideoAviToolBar m_VideoAviToolBar;

// Dialog Data
	//{{AFX_DATA(CPlayerToolBarDlg)
	enum { IDD = IDD_PLAYER_TOOLBAR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayerToolBarDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL IsThemed();
	HTHEME m_hTheme;

	// Generated message map functions
	//{{AFX_MSG(CPlayerToolBarDlg)
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYERTOOLBARDLG_H__D42904B2_961F_4BBE_8368_F4B63810B415__INCLUDED_)
