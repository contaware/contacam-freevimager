#if !defined(AFX_LISTCTRLEX_H__4786AB0E_4866_46B5_BE12_B75B467F7AD0__INCLUDED_)
#define AFX_LISTCTRLEX_H__4786AB0E_4866_46B5_BE12_B75B467F7AD0__INCLUDED_

#pragma once

// ListCtrlEx.h : header file
//

class CListCtrlEx : public CListCtrl
{
public:
	enum {MAX_LABEL_SIZE=127};
	CListCtrlEx();
	virtual ~CListCtrlEx();

protected:
	int GetSelectedItem();

public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListCtrlEx)
	//}}AFX_VIRTUAL	

	// Generated message map functions
protected:
	//{{AFX_MSG(CListCtrlEx)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnBeginLabelEditList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnEndLabelEditList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnDblClickEx(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTCTRLEX_H__4786AB0E_4866_46B5_BE12_B75B467F7AD0__INCLUDED_)
