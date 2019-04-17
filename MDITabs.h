// Credits: inspired by Christian Rodemeyer's CMDITabs

#pragma once

#ifndef __MDITABS_H
#define __MDITABS_H

class CMainFrame;

enum EMDITabStyles
{
	MT_BOTTOM        = 0x0000, // places tabs at bottom (default)
	MT_TOP           = 0x0001, // place tabs at top
	MT_IMAGES        = 0x0002, // show images
	MT_HIDEWLT2VIEWS = 0x0004, // Hide Tabs when less than two views are open (default is one view)
	MT_TOOLTIPS      = 0x0008, // not implemented (a tooltip can appear about a tab) 
	MT_BUTTONS       = 0x0010, // not implemented (show tabs as buttons)
	MT_AUTOSIZE      = 0x0020, // not implemented (tabs are sized to fit the entire width of the control)
	MT_TASKBAR       = 0x0038  // MT_TOOLTIPS|MT_BUTTONS|MT_AUTOSIZE
};

enum EWMMTGetInfo
{
	WM_GETTABTIPTEXT = WM_APP + 0x0393, // send to associated view to get a tip text
	WM_GETTABSYSMENU = WM_APP + 0x0394  // if neccessary, the view can provide a different systemmenu by answering this message
};

class CMDITabs : public CTabCtrl
{
public:
	CMDITabs();
	void Create(CMainFrame* pMainFrame, DWORD dwStyle = MT_BOTTOM | MT_IMAGES);
	void Update(); // sync the tabctrl with all views
	void SetMinViews(int nMinViews) {m_nMinViews = nMinViews;}

private:
	int        m_height;
	int        m_width;
	CImageList m_images;
	int        m_nMinViews;
	BOOL       m_bImages;
	BOOL       m_bTop;
	BOOL       m_bTracking;
	int        m_nCloseHotTabIndex;
	CFont      m_Font;
	BOOL       m_bInUpdate;

protected:
	CRect GetCloseBkgRect(int nTabIndex);
	afx_msg LRESULT OnSizeParent(WPARAM, LPARAM lParam);
	afx_msg void OnSelChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPaint();
	afx_msg void OnNcPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	DECLARE_MESSAGE_MAP()
};

#endif
