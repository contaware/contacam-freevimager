#if !defined(AFX_IMAGEINFODLG_H__B14182E4_3AF2_4D7A_AED5_B77789819D79__INCLUDED_)
#define AFX_IMAGEINFODLG_H__B14182E4_3AF2_4D7A_AED5_B77789819D79__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImageInfoDlg.h : header file
//

#include "resource.h"
#include "Dib.h"
#include "cdxCDynamicDialog.h"
#include "SepComboBox.h"

#define	ICC_TEXT_MAX_SIZE		65536

// Forward Declaration
class CPictureDoc;

/////////////////////////////////////////////////////////////////////////////
// CImageInfoDlg dialog

class CImageInfoDlg : public cdxCDynamicDialog
{
public:
	enum {	TRANSPARENCY_TIMER_MS	= 25,
			MAX_OPACITY				= 100,
			MIN_OPACITY				= 45};

	enum METADATA
	{	
		FILE_COMMENT,
		
		CONTACT_CREATOR,
		CONTACT_CREATORSJOBTITLE,
		CONTACT_ADDRESS,
		CONTACT_CITY,
		CONTACT_STATEPROVINCE,
		CONTACT_POSTALCODE,
		CONTACT_COUNTRY,
		CONTACT_PHONE,
		CONTACT_EMAIL,
		CONTACT_WEBSITE,

		CONTENT_HEADLINE,
		CONTENT_DESCRIPTION,
		CONTENT_KEYWORDS,
		CONTENT_IPTCSUBJECTCODE,
		CONTENT_CATEGORY,
		CONTENT_SUPPLEMENTALCATEGORIES,
		CONTENT_DESCRIPTIONWRITER,
		
		IMAGE_DATECREATED,
		IMAGE_INTELLECTUALGENRE,
		IMAGE_IPTCSCENE,
		IMAGE_LOCATION,
		IMAGE_CITY,
		IMAGE_STATEPROVINCE,
		IMAGE_COUNTRY,
		IMAGE_ISOCOUNTRYCODE,

		STATUS_TITLE,
		STATUS_JOBIDENTIFIER,
		STATUS_INSTRUCTIONS,
		STATUS_PROVIDER,
		STATUS_SOURCE,
		STATUS_COPYRIGHTNOTICE,
		STATUS_RIGHTSUSAGETERMS,
		STATUS_COPYRIGHTURL,
		STATUS_COPYRIGHTSTATUS,
		STATUS_URGENCY
	};

	enum METADATAVIEW
	{
		OTHERS,
		EXIF,
		IPTCLEGACY,
		XMP,
		ICC
	};

	CImageInfoDlg(CPictureDoc* pDoc);
	virtual ~CImageInfoDlg();
	void SetOpacity(int nPercent);	// 0 Fully Transparent, 100 Opaque
	void SetLayered();
	void RemoveLayered();
	void Close();
	void UpdateMetadata();
	BOOL SaveModified(BOOL bPrompt = TRUE);
	void UpdateDlgTitle();

	CString m_sOrigComment;
	CMetadata::IPTCINFO m_OrigIptc;
	CMetadata::XMPINFO m_OrigXmp;
	CString m_sCurrentComment;
	CMetadata::IPTCINFO m_CurrentIptc;
	CMetadata::XMPINFO m_CurrentXmp;
	CMetadata::IPTCINFO m_OutOfSyncIptc; // Out of Sync. Fields are marked with a _T("1")
	int m_nMetadataType;

// Dialog Data
	//{{AFX_DATA(CImageInfoDlg)
	CSliderCtrl	m_TransparencySlider;
	int		m_nMetadataGroupView;
	CSeparatorComboBox m_cbMetadata;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageInfoDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void LoadSettings();
	void SaveSettings();
	BOOL SaveMetadata();
	void UpdateMetadataModifiedFlag();
	BOOL PrevMetadata();
	BOOL NextMetadata();
	void SetEditMetadataText();
	void GetEditMetadataText();
	void DisplayMetadata();
	void DisplayXmp();
	BOOL ExportXmp(LPCTSTR lpzsFileName);
	static BOOL ExportXmp(LPCTSTR lpszFileName, LPBYTE pXmpData, DWORD dwXmpSize);
	void GetIptc();
	void GetIptc(	const CString& sIptcLegacy,
					const CString& sIptcFromXmp,
					CString& sOrigIptc,
					CString& sCurrentIptc,
					CString& sOutOfSyncIptc);
	void GetIptc(	const CStringArray& IptcLegacy,
					const CStringArray& IptcFromXmp,
					CStringArray& OrigIptc,
					CStringArray& CurrentIptc);
	void SetIptc();

	BOOL m_bLayered;
	int m_nOpacity;
	int m_nMinOpacity;
	UINT m_uiTimerID;
	CPictureDoc* m_pDoc;
	BOOL m_bDisableTimer;
	int IDD;

	// Generated message map functions
	//{{AFX_MSG(CImageInfoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnChangeMetadata();
	afx_msg void OnButtonSaveMetadata();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnSelchangeComboMetadata();
	afx_msg void OnRadioMetadata();
	afx_msg void OnRadioExif();
	afx_msg void OnRadioIptcLegacy();
	afx_msg void OnRadioXmp();
	afx_msg void OnRadioIcc();
	afx_msg void OnButtonPrevMetadata();
	afx_msg void OnButtonNextMetadata();
	afx_msg void OnButtonImportMetadata();
	afx_msg void OnButtonExportMetadata();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGEINFODLG_H__B14182E4_3AF2_4D7A_AED5_B77789819D79__INCLUDED_)
