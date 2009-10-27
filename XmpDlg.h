#if !defined(AFX_XMPDLG_H__1F20D25C_EA66_4F02_9DD3_CE18A31969B2__INCLUDED_)
#define AFX_XMPDLG_H__1F20D25C_EA66_4F02_9DD3_CE18A31969B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// XmpDlg.h : header file
//

#include "DibMetadata.h"

/////////////////////////////////////////////////////////////////////////////
// CXmpDlg dialog

class CXmpDlg : public CDialog
{
// Construction
public:
	enum {FIELD_OVERWRITE, FIELD_SETEMPTY, FIELD_LEAVE, FIELD_DELETE};
	CXmpDlg(CWnd* pParent, UINT idd);
	void GetFields(	CMetadata::IPTCINFO& IptcInfo,
					CMetadata::XMPINFO& XmpInfo);
	void GetFields(	BOOL bXmpPriority,
					CMetadata& metadata);
	void MergeIptc(	BOOL bXmpPriority,
					CMetadata::IPTCINFO& IptcLegacyInfo,
					CMetadata::IPTCINFO& IptcFromXmpInfo);
	void MergeIptc(	BOOL bXmpPriority,
					CString& sIptcLegacy,
					CString& sIptcFromXmp);
	void MergeIptc(	CStringArray& IptcLegacy,
					CStringArray& IptcFromXmp);
	void LoadSettings();
	void SaveSettings();
	void SetExifDateCreated(const CTime& Time)
	{
		m_sExifDateCreated.Format(	_T("%d%02d%02d"),
									Time.GetYear(),
									Time.GetMonth(),
									Time.GetDay());
	};
	void ClearExifDateCreated() {m_sExifDateCreated = _T("");};

// Dialog Data
	//{{AFX_DATA(CXmpDlg)
	int		m_nRadioByline;
	int		m_nRadioBylineTitle;
	int		m_nRadioCiAdrExtadr;
	int		m_nRadioCiAdrCity;
	int		m_nRadioCiAdrRegion;
	int		m_nRadioCiAdrPcode;
	int		m_nRadioCiAdrCtry;
	int		m_nRadioCiTelWork;
	int		m_nRadioCiEmailWork;
	int		m_nRadioCiUrlWork;
	CString	m_sXmpLoadFile;
	int		m_nRadioHeadline;
	int		m_nRadioCaption;
	int		m_nRadioKeywords;
	int		m_nRadioSubjectCode;
	int		m_nRadioCategory;
	int		m_nRadioSupplementalCategories;
	int		m_nRadioCaptionWriter;
	int		m_nRadioDateCreated;
	int		m_nRadioIntellectualGenre;
	int		m_nRadioScene;
	int		m_nRadioLocation;
	int		m_nRadioCity;
	int		m_nRadioProvinceState;
	int		m_nRadioCountry;
	int		m_nRadioCountryCode;
	int		m_nRadioObjectName;
	int		m_nRadioOriginalTransmissionReference;
	int		m_nRadioSpecialInstructions;
	int		m_nRadioCredits;
	int		m_nRadioSource;
	int		m_nRadioCopyrightNotice;
	int		m_nRadioUsageTerms;
	int		m_nRadioCopyrightUrl;
	int		m_nRadioCopyrightMarked;
	int		m_nRadioUrgency;
	int		m_nRadioDateCreatedSource;
	//}}AFX_DATA
	CString	m_sXmpSaveFile;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXmpDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Implementation
protected:	
	BOOL ParseXmp();
	void ParseCopyrightStatus();
	void DisplayCopyrightStatus();
	void ParseDateCreated();
	void DisplayDateCreated();
	void DisplayMetadata();
	void AlignCtrls(int id, CRect& rcWnd, CRect& rcPrevWnd);
	void GetMetadataFromDisplay();
	__forceinline void GetField(	int nRadioField,
									const CString& sSrcField,
									CString& sDstField);
	__forceinline void GetFieldArray(	int nRadioField,
										const CStringArray& sSrcFieldArray,
										CStringArray& sDstFieldArray);
	__forceinline CString FormatString(CString s);
	__forceinline CString FormatStringArray(const CStringArray& array);
	void ParseToStringArray(CString s, CStringArray& array);
	BOOL ExportXmp(LPCTSTR lpszFileName);

	UINT IDD;
	CMetadata m_Metadata;
	CString m_sExifDateCreated;

	// Generated message map functions
	//{{AFX_MSG(CXmpDlg)
	afx_msg void OnButtonXmpload();
	afx_msg void OnButtonXmpsave();
	afx_msg void OnButtonXmpunload();
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonContactOverwrite();
	afx_msg void OnButtonContactSet();
	afx_msg void OnButtonContactLeave();
	afx_msg void OnButtonContactDel();
	afx_msg void OnButtonContentOverwrite();
	afx_msg void OnButtonContentSet();
	afx_msg void OnButtonContentLeave();
	afx_msg void OnButtonContentDel();
	afx_msg void OnButtonImageOverwrite();
	afx_msg void OnButtonImageSet();
	afx_msg void OnButtonImageLeave();
	afx_msg void OnButtonImageDel();
	afx_msg void OnButtonStatusOverwrite();
	afx_msg void OnButtonStatusSet();
	afx_msg void OnButtonStatusLeave();
	afx_msg void OnButtonStatusDel();
	afx_msg void OnRadioDatecreatedSourceExif();
	afx_msg void OnRadioDatecreatedSourceXmp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XMPDLG_H__1F20D25C_EA66_4F02_9DD3_CE18A31969B2__INCLUDED_)
