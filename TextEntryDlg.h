#if !defined(AFX_TEXTENTRYDLG_H__6A02A506_A463_4BDD_B9D6_711968602729__INCLUDED_)
#define AFX_TEXTENTRYDLG_H__6A02A506_A463_4BDD_B9D6_711968602729__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTextEntryDlg : protected CWnd
{
public:
	CTextEntryDlg();
	virtual ~CTextEntryDlg();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	
	// Returns: 0=Error, 1=IDOK, 2=IDCANCEL
	int Show(CWnd* pParent, CWnd* pCenterIn, LPCTSTR pszTitle, BOOL bPassword = FALSE);
	CString m_sText;

protected:
	CEdit m_ctlEdit;
	CFont m_Font;
	CButton m_ctlCancel;
	CButton m_ctlOK;

	int LoopIt(CWnd *pParent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnClose();
	afx_msg void OnCancel();
	afx_msg void OnOK();

	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_TEXTENTRYDLG_H__6A02A506_A463_4BDD_B9D6_711968602729__INCLUDED_)
