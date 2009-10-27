#if !defined(AFX_SNAPSHOTPAGE_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_)
#define AFX_SNAPSHOTPAGE_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SnapshotPage.h : header file
//

#ifdef VIDEODEVICEDOC

// Forward Declaration
class CVideoDeviceDoc;

#include "StaticLink.h"

/////////////////////////////////////////////////////////////////////////////
// CSnapshotPage dialog

class CSnapshotPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CSnapshotPage)

// Construction
public:
	CSnapshotPage();
	void SetDoc(CVideoDeviceDoc* pDoc);
	~CSnapshotPage();
	void EnableDisableCriticalControls(BOOL bEnable);
	void ChangeThumbSize(int nNewWidth, int nNewHeight);
	CStaticLink m_FileLabel;
	CStaticLink m_DirLabel;

// Dialog Data
	//{{AFX_DATA(CSnapshotPage)
	enum { IDD = IDD_SNAPSHOT };
	CSliderCtrl	m_VideoCompressorQuality;
	CSliderCtrl	m_CompressionQuality;
	CTime	m_SnapshotStartTime;
	CTime	m_SnapshotStopTime;
	int		m_nDeleteSnapshotsOlderThanDays;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSnapshotPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateSnapshotStartStopTimes();
	CVideoDeviceDoc* m_pDoc;
	// Generated message map functions
	//{{AFX_MSG(CSnapshotPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnChangeEditSnapshotRate();
	afx_msg void OnButtonSetDir();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnButtonThumbSize();
	afx_msg void OnCheckSnapshotThumb();
	afx_msg void OnChangeEditSnapshotHistoryFramerate();
	afx_msg void OnCheckSnapshotHistoryDeinterlace();
	afx_msg void OnCheckSnapshotLiveJpeg();
	afx_msg void OnCheckSnapshotHistoryJpeg();
	afx_msg void OnCheckSnapshotHistorySwf();
	afx_msg void OnCheckSchedulerDaily();
	afx_msg void OnDatetimechangeTimeDailyStart(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeTimeDailyStop(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeEditDeleteSnapshotsDays();
	afx_msg void OnFtpConfigure();
	afx_msg void OnCheckFtpSnapshot();
	afx_msg void OnCheckFtpSnapshotHistoryJpeg();
	afx_msg void OnCheckFtpSnapshotHistorySwf();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SNAPSHOTPAGE_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_)
