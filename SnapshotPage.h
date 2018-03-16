#if !defined(AFX_SNAPSHOTPAGE_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_)
#define AFX_SNAPSHOTPAGE_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_

#pragma once

// SnapshotPage.h : header file
//

#ifdef VIDEODEVICEDOC

// Forward Declaration
class CVideoDeviceDoc;

class CSnapshotPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CSnapshotPage)
public:
	// Construction
	CSnapshotPage();
	void SetDoc(CVideoDeviceDoc* pDoc);
	~CSnapshotPage();

	// Dialog data
	enum { IDD = IDD_SNAPSHOT };
	CTime m_SnapshotStartTime;
	CTime m_SnapshotStopTime;

	// Public Helper functions
	void DisplaySnapshotRate();
	void ChangeThumbSize(int nNewWidth, int nNewHeight);

protected:
	// Helper vars
	CVideoDeviceDoc* m_pDoc;

	// Helper functions
	void UpdateSnapshotStartStopTimes();

	// Dialog functions
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnChangeEditSnapshotRate();
	afx_msg void OnButtonSnapshotNames();
	afx_msg void OnButtonThumbSize();
	afx_msg void OnCheckSnapshotHistoryVideo();
	afx_msg void OnCheckFtpSnapshot();
	afx_msg void OnCheckFtpSnapshotHistoryVideo();
	afx_msg void OnFtpConfigure();
	afx_msg void OnCheckSchedulerDaily();
	afx_msg void OnDatetimechangeTimeDailyStart(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeTimeDailyStop(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};

#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SNAPSHOTPAGE_H__6E88B0A4_B936_40BD_B351_397CEA44BF51__INCLUDED_)
