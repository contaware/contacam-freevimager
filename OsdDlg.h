#if !defined(AFX_OSDDLG_H__132EC743_2DD4_400E_8D77_BDACD793ADB6__INCLUDED_)
#define AFX_OSDDLG_H__132EC743_2DD4_400E_8D77_BDACD793ADB6__INCLUDED_

#pragma once

// OsdDlg.h : header file
//

// Forward Declaration
class CPictureDoc;

/////////////////////////////////////////////////////////////////////////////
// COsdDlg dialog

class COsdDlg : public CDialog
{
public:
	enum {	TRANSPARENCY_TIMER_MS	= 25,
			NOTRANSPARENCY_TIMER_MS	= 200,
			DEFAULT_MAX_OPACITY		= 80,
			MIN_OPACITY				= 0};
	enum {	FONT_SMALL=12, FONT_MEDIUM=14, FONT_BIG=18};
	enum {	TIMEOUT_INFINITE=0,
			TIMEOUT_3SEC=3000,
			TIMEOUT_4SEC=4000,
			TIMEOUT_5SEC=5000,
			TIMEOUT_6SEC=6000,
			TIMEOUT_7SEC=7000};
	enum {	DEFAULT_DLG_HEIGHT=58};
	enum {	DISPLAY_FILENAME=1,
			DISPLAY_SIZESCOMPRESSION=2,
			DISPLAY_DATE=4,
			DISPLAY_METADATADATE=8,
			DISPLAY_LOCATION=16,
			DISPLAY_HEADLINEDESCRIPTION=32,
			DISPLAY_FLASH=64,
			DISPLAY_EXPOSURETIME=128,
			DISPLAY_APERTURE=256};
	
	COsdDlg(CPictureDoc* pDoc);
	virtual ~COsdDlg() {m_Font.DeleteObject(); m_FontDesc.DeleteObject();};

// Dialog Data
	//{{AFX_DATA(COsdDlg)
	enum { IDD = IDD_OSD };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	void UpdateDisplay();
	void MonitorSwitch();	// Call this if the monitor has been switched

	void Defaults();
	void SetOpacity(int nPercent);	// 0 Fully Transparent, 100 Opaque
	void SetLayered();
	void RemoveLayered();
	void Close();
	void ForceShow(BOOL bForce = TRUE);

	void SetTimeout(int nTimeout);
	int GetTimeout() const {return m_nTimeout;};
	void RestartTimeout();

	void SetAutoSize(BOOL bAutoSize) {m_bAutoSize = bAutoSize;};
	BOOL DoAutoSize() const {return m_bAutoSize;};
	void SetFontSize(int nFontSize);
	int GetFontSize() const {return m_nFontSize;};
	void SetFontColor(COLORREF crFontColor);
	COLORREF GetFontColor() const {return m_crFontColor;};
	void SetBackgroundColor(COLORREF crBackgroundColor);
	COLORREF GetBackgroundColor() const {return m_crBackgroundColor;};
	void SetUsePictureDocBackgroundColor(BOOL bUse) {m_bUsePictureDocBackgroundColor = bUse;};
	BOOL DoUsePictureDocBackgroundColor() const {return m_bUsePictureDocBackgroundColor;};
	void SetMaxOpacity(int nMaxOpacity) {m_nMaxOpacity = nMaxOpacity; SetOpacity(m_nOpacity);};
	int GetMaxOpacity() const {return m_nMaxOpacity;};
	
	void SetDisplayState(DWORD dwState) {m_dwDisplayState |= dwState; UpdateDisplay();};
	void ClearDisplayState(DWORD dwState) {m_dwDisplayState &= ~dwState; UpdateDisplay();};
	void ToggleDisplayState(DWORD dwState) {DoDisplayState(dwState) ?
											ClearDisplayState(dwState) :
											SetDisplayState(dwState);};
	BOOL DoDisplayState(DWORD dwState) const {return m_dwDisplayState & dwState;};
	CStringArray m_TopArray;
	CString m_sHeadline;
	CString m_sDescription;
	CStringArray m_ExifArray;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COsdDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void ActivateDrag();
	void DeactivateDrag();
	void LoadSettings();
	BOOL LoadPlacementSettings();
	void ClipToMonitorRect(RECT& rc);
	BOOL SetPlacement(WINDOWPLACEMENT&	wpl, const CRect& rcOrigMonitor);
	void SaveSettings();
	void PaintArray(CDC* pDC, CStringArray& a, CRect& rcDraw, CRect& rcDrawPrev, int nLineHeight);
	BOOL FitsOneLine(CDC* pDC, CString s, const CRect& rc);
	CSize GetOneLineTextSize(CDC* pDC, CString s);
	void SetDlgHeight(int nHeight);

	CFont m_Font;
	CFont m_FontDesc;
	CPictureDoc* m_pDoc;
	int m_nCountdown;
	int m_nForceShowCount;
	int m_nTimeout;
	int m_nOpacity;
	int m_nOpacityInc;
	int m_nOpacityDec;
	int m_nMinOpacity;
	int m_nMaxOpacity;
	BOOL m_bLayered;
	BOOL m_bDataToDisplay;
	UINT m_uiTimerID;
	BOOL m_bDrag;
	int m_nOldMouseX;
	int m_nOldMouseY;
	CRect m_rcMonitor;
	int m_nMinDlgSizeX;
	int m_nMinDlgSizeY;
	BOOL m_bFontCreated;
	int m_nFontSize;
	COLORREF m_crFontColor;
	COLORREF m_crBackgroundColor;
	BOOL m_bAutoSize;
	BOOL m_bAutoSizeNow;
	BOOL m_bUsePictureDocBackgroundColor;
	DWORD m_dwDisplayState;
	
	// Generated message map functions
	//{{AFX_MSG(COsdDlg)
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
    afx_msg LRESULT OnEnterSizeMove(WPARAM, LPARAM);
	afx_msg LRESULT OnExitSizeMove(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OSDDLG_H__132EC743_2DD4_400E_8D77_BDACD793ADB6__INCLUDED_)
