// XmpDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "XmpDlg.h"
#include "NoVistaFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CXmpDlg dialog


CXmpDlg::CXmpDlg(CWnd* pParent, UINT idd)
	: CDialog(idd, pParent)
{
	//{{AFX_DATA_INIT(CXmpDlg)
	m_sXmpLoadFile = _T("");
	m_nRadioByline = 1;
	m_nRadioBylineTitle = 1;
	m_nRadioCiAdrExtadr = 1;
	m_nRadioCiAdrCity = 1;
	m_nRadioCiAdrRegion = 1;
	m_nRadioCiAdrPcode = 1;
	m_nRadioCiAdrCtry = 1;
	m_nRadioCiTelWork = 1;
	m_nRadioCiEmailWork = 1;
	m_nRadioCiUrlWork = 1;
	m_nRadioHeadline = 1;
	m_nRadioCaption = 1;
	m_nRadioKeywords = 1;
	m_nRadioSubjectCode = 1;
	m_nRadioCategory = 1;
	m_nRadioSupplementalCategories = 1;
	m_nRadioCaptionWriter = 1;
	m_nRadioDateCreated = 1;
	m_nRadioIntellectualGenre = 1;
	m_nRadioScene = 1;
	m_nRadioLocation = 1;
	m_nRadioCity = 1;
	m_nRadioProvinceState = 1;
	m_nRadioCountry = 1;
	m_nRadioCountryCode = 1;
	m_nRadioObjectName = 1;
	m_nRadioOriginalTransmissionReference = 1;
	m_nRadioSpecialInstructions = 1;
	m_nRadioCredits = 1;
	m_nRadioSource = 1;
	m_nRadioCopyrightNotice = 1;
	m_nRadioUsageTerms = 1;
	m_nRadioCopyrightUrl = 1;
	m_nRadioCopyrightMarked = 1;
	m_nRadioUrgency = 1;
	m_nRadioDateCreatedSource = 1;
	//}}AFX_DATA_INIT
	IDD = idd;
	m_sXmpSaveFile = _T("");
	LoadSettings();
}


void CXmpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CXmpDlg)
	DDX_Radio(pDX, IDC_RADIO_CONTACT_CREATOR, m_nRadioByline);
	DDX_Radio(pDX, IDC_RADIO_CONTACT_CREATORSJOB, m_nRadioBylineTitle);
	DDX_Radio(pDX, IDC_RADIO_CONTACT_ADDRESS, m_nRadioCiAdrExtadr);
	DDX_Radio(pDX, IDC_RADIO_CONTACT_CITY, m_nRadioCiAdrCity);
	DDX_Radio(pDX, IDC_RADIO_CONTACT_STATEPROVINCE, m_nRadioCiAdrRegion);
	DDX_Radio(pDX, IDC_RADIO_CONTACT_POSTALCODE, m_nRadioCiAdrPcode);
	DDX_Radio(pDX, IDC_RADIO_CONTACT_COUNTRY, m_nRadioCiAdrCtry);
	DDX_Radio(pDX, IDC_RADIO_CONTACT_PHONE, m_nRadioCiTelWork);
	DDX_Radio(pDX, IDC_RADIO_CONTACT_EMAIL, m_nRadioCiEmailWork);
	DDX_Radio(pDX, IDC_RADIO_CONTACT_WEBSITE, m_nRadioCiUrlWork);
	DDX_Text(pDX, IDC_EDIT_XMPLOAD, m_sXmpLoadFile);
	DDX_Radio(pDX, IDC_RADIO_CONTENT_HEADLINE, m_nRadioHeadline);
	DDX_Radio(pDX, IDC_RADIO_CONTENT_DESCRIPTION, m_nRadioCaption);
	DDX_Radio(pDX, IDC_RADIO_CONTENT_KEYWORDS, m_nRadioKeywords);
	DDX_Radio(pDX, IDC_RADIO_CONTENT_IPTCSUBJECTCODE, m_nRadioSubjectCode);
	DDX_Radio(pDX, IDC_RADIO_CONTENT_CATEGORY, m_nRadioCategory);
	DDX_Radio(pDX, IDC_RADIO_CONTENT_SUPPLEMENTALCATEGORIES, m_nRadioSupplementalCategories);
	DDX_Radio(pDX, IDC_RADIO_CONTENT_DESCRIPTIONWRITER, m_nRadioCaptionWriter);
	DDX_Radio(pDX, IDC_RADIO_IMAGE_DATECREATED, m_nRadioDateCreated);
	DDX_Radio(pDX, IDC_RADIO_IMAGE_INTELLECTUALGENRE, m_nRadioIntellectualGenre);
	DDX_Radio(pDX, IDC_RADIO_IMAGE_IPTCSCENE, m_nRadioScene);
	DDX_Radio(pDX, IDC_RADIO_IMAGE_LOCATION, m_nRadioLocation);
	DDX_Radio(pDX, IDC_RADIO_IMAGE_CITY, m_nRadioCity);
	DDX_Radio(pDX, IDC_RADIO_IMAGE_STATEPROVINCE, m_nRadioProvinceState);
	DDX_Radio(pDX, IDC_RADIO_IMAGE_COUNTRY, m_nRadioCountry);
	DDX_Radio(pDX, IDC_RADIO_IMAGE_ISOCOUNTRYCODE, m_nRadioCountryCode);
	DDX_Radio(pDX, IDC_RADIO_STATUS_TITLE, m_nRadioObjectName);
	DDX_Radio(pDX, IDC_RADIO_STATUS_JOBIDENTIFIER, m_nRadioOriginalTransmissionReference);
	DDX_Radio(pDX, IDC_RADIO_STATUS_INSTRUCTIONS, m_nRadioSpecialInstructions);
	DDX_Radio(pDX, IDC_RADIO_STATUS_PROVIDER, m_nRadioCredits);
	DDX_Radio(pDX, IDC_RADIO_STATUS_SOURCE, m_nRadioSource);
	DDX_Radio(pDX, IDC_RADIO_STATUS_COPYRIGHTNOTICE, m_nRadioCopyrightNotice);
	DDX_Radio(pDX, IDC_RADIO_STATUS_RIGHTSUSAGETERMS, m_nRadioUsageTerms);
	DDX_Radio(pDX, IDC_RADIO_STATUS_COPYRIGHTURL, m_nRadioCopyrightUrl);
	DDX_Radio(pDX, IDC_RADIO_STATUS_COPYRIGHTSTATUS, m_nRadioCopyrightMarked);
	DDX_Radio(pDX, IDC_RADIO_STATUS_URGENCY, m_nRadioUrgency);
	DDX_Radio(pDX, IDC_RADIO_DATECREATED_SOURCE_XMP, m_nRadioDateCreatedSource);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CXmpDlg, CDialog)
	//{{AFX_MSG_MAP(CXmpDlg)
	ON_BN_CLICKED(IDC_BUTTON_XMPLOAD, OnButtonXmpload)
	ON_BN_CLICKED(IDC_BUTTON_XMPSAVE, OnButtonXmpsave)
	ON_BN_CLICKED(IDC_BUTTON_XMPUNLOAD, OnButtonXmpunload)
	ON_BN_CLICKED(IDC_BUTTON_CONTACT_OVERWRITE, OnButtonContactOverwrite)
	ON_BN_CLICKED(IDC_BUTTON_CONTACT_SET, OnButtonContactSet)
	ON_BN_CLICKED(IDC_BUTTON_CONTACT_LEAVE, OnButtonContactLeave)
	ON_BN_CLICKED(IDC_BUTTON_CONTACT_DEL, OnButtonContactDel)
	ON_BN_CLICKED(IDC_BUTTON_CONTENT_OVERWRITE, OnButtonContentOverwrite)
	ON_BN_CLICKED(IDC_BUTTON_CONTENT_SET, OnButtonContentSet)
	ON_BN_CLICKED(IDC_BUTTON_CONTENT_LEAVE, OnButtonContentLeave)
	ON_BN_CLICKED(IDC_BUTTON_CONTENT_DEL, OnButtonContentDel)
	ON_BN_CLICKED(IDC_BUTTON_IMAGE_OVERWRITE, OnButtonImageOverwrite)
	ON_BN_CLICKED(IDC_BUTTON_IMAGE_SET, OnButtonImageSet)
	ON_BN_CLICKED(IDC_BUTTON_IMAGE_LEAVE, OnButtonImageLeave)
	ON_BN_CLICKED(IDC_BUTTON_IMAGE_DEL, OnButtonImageDel)
	ON_BN_CLICKED(IDC_BUTTON_STATUS_OVERWRITE, OnButtonStatusOverwrite)
	ON_BN_CLICKED(IDC_BUTTON_STATUS_SET, OnButtonStatusSet)
	ON_BN_CLICKED(IDC_BUTTON_STATUS_LEAVE, OnButtonStatusLeave)
	ON_BN_CLICKED(IDC_BUTTON_STATUS_DEL, OnButtonStatusDel)
	ON_BN_CLICKED(IDC_RADIO_DATECREATED_SOURCE_EXIF, OnRadioDatecreatedSourceExif)
	ON_BN_CLICKED(IDC_RADIO_DATECREATED_SOURCE_XMP, OnRadioDatecreatedSourceXmp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXmpDlg message handlers

void CXmpDlg::AlignCtrls(int id, CRect& rcWnd, CRect& rcPrevWnd)
{
	CEdit* pEdit = (CEdit*)GetDlgItem(id);
	pEdit->GetWindowRect(&rcWnd);
	if (rcWnd.top != rcPrevWnd.bottom)
	{
		rcWnd.OffsetRect(0, rcPrevWnd.bottom - rcWnd.top);
		rcPrevWnd = rcWnd;
		ScreenToClient(&rcWnd);
		pEdit->SetWindowPos(NULL, rcWnd.left, rcWnd.top, 0, 0, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);
	}
	else
	{
		rcPrevWnd = rcWnd;
		ScreenToClient(&rcWnd);
	}

	CRect rcRadio;
	CButton* pRadio = (CButton*)GetDlgItem(id-4);
	pRadio->GetWindowRect(&rcRadio);
	ScreenToClient(&rcRadio);
	pRadio->SetWindowPos(NULL, rcRadio.left, rcWnd.top, rcRadio.Width(), rcWnd.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

	pRadio = (CButton*)GetDlgItem(id-3);
	pRadio->GetWindowRect(&rcRadio);
	ScreenToClient(&rcRadio);
	pRadio->SetWindowPos(NULL, rcRadio.left, rcWnd.top, rcRadio.Width(), rcWnd.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

	pRadio = (CButton*)GetDlgItem(id-2);
	pRadio->GetWindowRect(&rcRadio);
	ScreenToClient(&rcRadio);
	pRadio->SetWindowPos(NULL, rcRadio.left, rcWnd.top, rcRadio.Width(), rcWnd.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

	pRadio = (CButton*)GetDlgItem(id-1);
	pRadio->GetWindowRect(&rcRadio);
	ScreenToClient(&rcRadio);
	pRadio->SetWindowPos(NULL, rcRadio.left, rcWnd.top, rcRadio.Width(), rcWnd.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
}

BOOL CXmpDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Align the 10 Contact Entries
	int i;
	CRect rcWnd, rcPrevWnd;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_CREATOR);
	pEdit->GetWindowRect(&rcWnd);
	rcPrevWnd.bottom = rcWnd.top;
	for (i = 0 ; i < 10 ; i++)
		AlignCtrls(IDC_EDIT_CONTACT_CREATOR + 5*i, rcWnd, rcPrevWnd);

	// Align the 7 Content Entries
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTENT_HEADLINE);
	pEdit->GetWindowRect(&rcWnd);
	rcPrevWnd.bottom = rcWnd.top;
	for (i = 0 ; i < 7 ; i++)
		AlignCtrls(IDC_EDIT_CONTENT_HEADLINE + 5*i, rcWnd, rcPrevWnd);
	
	// Align the 8 Image Entries
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_DATECREATED);
	pEdit->GetWindowRect(&rcWnd);
	rcPrevWnd.bottom = rcWnd.top;
	for (i = 0 ; i < 8 ; i++)
		AlignCtrls(IDC_EDIT_IMAGE_DATECREATED + 5*i, rcWnd, rcPrevWnd);

	// Align the 10 Status Entries
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_TITLE);
	pEdit->GetWindowRect(&rcWnd);
	rcPrevWnd.bottom = rcWnd.top;
	for (i = 0 ; i < 10 ; i++)
		AlignCtrls(IDC_EDIT_STATUS_TITLE + 5*i, rcWnd, rcPrevWnd);

	// Display Metadata
	DisplayMetadata();

	// Disable DateCreated EditBox if Source is from Exif
	if (IDD == IDD_XMP)
	{
		pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_DATECREATED);
		if (m_nRadioDateCreatedSource == 1)
			pEdit->EnableWindow(FALSE);
	}

	// Set Xmp File Edit Box
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_XMPLOAD);
	pEdit->SetWindowText(m_sXmpLoadFile);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CXmpDlg::ParseXmp()
{
	CString s = _T("");
	CString t;
	LPBYTE pData = NULL;
	int nSize;
	BOOL res = FALSE;

	// Open the Xmp File
	try
	{
		CFile f(m_sXmpLoadFile, CFile::modeRead | CFile::shareDenyNone);
		nSize = (int)f.GetLength();
		pData = new BYTE[nSize];
		f.Read(pData, nSize);
		f.Close();
	}
	catch (CFileException* e)
	{
		if (pData)
			delete [] pData;
		e->ReportError();
		e->Delete();
		return FALSE;
	}

	// Parse Xmp
	if (pData)
	{
		// Parse
		if (nSize > 0)
			res = m_Metadata.ParseXMP(pData, nSize);

		// Free
		if (pData)
			delete [] pData;
	}
	
	return res;
}

__forceinline CString CXmpDlg::FormatString(CString s)
{
	s.Replace(_T("\r\n"), _T(" "));
	s.Replace(_T("\r"), _T(" "));
	s.Replace(_T("\n"), _T(" "));
	s.Replace(_T("\t"), _T(" "));
	return s;
}

__forceinline CString CXmpDlg::FormatStringArray(const CStringArray& array)
{
	CString s;
	for (int i = 0 ; i < array.GetSize() ; i++)
	{
		if (i)
			s += _T("; ") + array[i];
		else
			s = array[0];
	}
	return s;
}

void CXmpDlg::ParseToStringArray(CString s, CStringArray& array)
{
	CString t;
	int i;
	array.RemoveAll();
	while ((i = s.Find(_T(";"))) >= 0)
	{
		t = s.Left(i);
		t.TrimLeft();
		t.TrimRight();
		if (t != _T(""))
			array.Add(t);
		s = s.Right(s.GetLength() - i - 1);
	}
	s.TrimLeft();
	s.TrimRight();
	if (s != _T(""))
		array.Add(s);
}

void CXmpDlg::OnRadioDatecreatedSourceExif() 
{
	UpdateData(TRUE);
	if (IDD == IDD_XMP)
	{
		ParseDateCreated();
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_DATECREATED);
		pEdit->EnableWindow(FALSE);
	}
	DisplayDateCreated();
}

void CXmpDlg::OnRadioDatecreatedSourceXmp() 
{
	UpdateData(TRUE);
	if (IDD == IDD_XMP)
	{
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_DATECREATED);
		pEdit->EnableWindow(TRUE);
	}
	DisplayDateCreated();
}

void CXmpDlg::ParseCopyrightStatus()
{
	CString s;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_COPYRIGHTSTATUS);
	pEdit->GetWindowText(s);
	if (s != _T(""))
	{
		s.MakeLower();
		if (s.Find(_T("true")) >= 0			||
			s.Find(_T("copyright")) >= 0)
			s = _T("True");
		else if (s.Find(_T("false")) >= 0	||
				s.Find(_T("public")) >= 0	||
				s.Find(_T("domain")) >= 0)
			s = _T("False");
		else
			s = _T("");
		m_Metadata.m_XmpInfo.CopyrightMarked = s;
	}
	else
		m_Metadata.m_XmpInfo.CopyrightMarked = _T("");
}

void CXmpDlg::DisplayCopyrightStatus()
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_COPYRIGHTSTATUS);
	if (m_Metadata.m_XmpInfo.CopyrightMarked == _T("True"))
		pEdit->SetWindowText(_T("Copyrighted"));
	else if (m_Metadata.m_XmpInfo.CopyrightMarked == _T("False"))
		pEdit->SetWindowText(_T("Public Domain"));
	else
		pEdit->SetWindowText(_T(""));
}

void CXmpDlg::ParseDateCreated()
{
	CString t, sDateCreated;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_DATECREATED);
	pEdit->GetWindowText(sDateCreated);
	if (sDateCreated != _T(""))
	{
		CTime Time = ::ParseShortDateLocalFormat(sDateCreated);
		t.Format(_T("%d%02d%02d"),	Time.GetYear(),
									Time.GetMonth(),
									Time.GetDay());
		m_Metadata.m_IptcFromXmpInfo.DateCreated = t;
	}
	else
		m_Metadata.m_IptcFromXmpInfo.DateCreated = _T("");
}

void CXmpDlg::DisplayDateCreated()
{
	CString sDateCreated;
	if (m_nRadioDateCreatedSource == 0)
		sDateCreated = m_Metadata.m_IptcFromXmpInfo.DateCreated;
	else
		sDateCreated = m_sExifDateCreated;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_DATECREATED);
	if (sDateCreated != _T(""))
	{
		CTime Time = CMetadata::GetDateFromIptcLegacyString(sDateCreated);
		pEdit->SetWindowText(::MakeDateLocalFormat(Time));
	}
	else
		pEdit->SetWindowText(_T(""));
}

void CXmpDlg::DisplayMetadata()
{
	CEdit* pEdit;

	// Contact
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_CREATOR);
	pEdit->SetWindowText(FormatString(m_Metadata.m_IptcFromXmpInfo.Byline));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_CREATORSJOB);
	pEdit->SetWindowText(FormatString(m_Metadata.m_IptcFromXmpInfo.BylineTitle));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_ADDRESS);
	pEdit->SetWindowText(FormatString(m_Metadata.m_XmpInfo.CiAdrExtadr));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_CITY);
	pEdit->SetWindowText(FormatString(m_Metadata.m_XmpInfo.CiAdrCity));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_STATEPROVINCE);
	pEdit->SetWindowText(FormatString(m_Metadata.m_XmpInfo.CiAdrRegion));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_POSTALCODE);
	pEdit->SetWindowText(FormatString(m_Metadata.m_XmpInfo.CiAdrPcode));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_COUNTRY);
	pEdit->SetWindowText(FormatString(m_Metadata.m_XmpInfo.CiAdrCtry));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_PHONE);
	pEdit->SetWindowText(FormatString(m_Metadata.m_XmpInfo.CiTelWork));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_EMAIL);
	pEdit->SetWindowText(FormatString(m_Metadata.m_XmpInfo.CiEmailWork));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_WEBSITE);
	pEdit->SetWindowText(FormatString(m_Metadata.m_XmpInfo.CiUrlWork));

	// Content
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTENT_HEADLINE);
	pEdit->SetWindowText(FormatString(m_Metadata.m_IptcFromXmpInfo.Headline));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTENT_DESCRIPTION);
	pEdit->SetWindowText(FormatString(m_Metadata.m_IptcFromXmpInfo.Caption));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTENT_KEYWORDS);
	pEdit->SetWindowText(FormatStringArray(m_Metadata.m_IptcFromXmpInfo.Keywords));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTENT_IPTCSUBJECTCODE);
	pEdit->SetWindowText(FormatStringArray(m_Metadata.m_XmpInfo.SubjectCode));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTENT_CATEGORY);
	pEdit->SetWindowText(FormatString(m_Metadata.m_IptcFromXmpInfo.Category));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTENT_SUPPLEMENTALCATEGORIES);
	pEdit->SetWindowText(FormatStringArray(m_Metadata.m_IptcFromXmpInfo.SupplementalCategories));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTENT_DESCRIPTIONWRITER);
	pEdit->SetWindowText(FormatString(m_Metadata.m_IptcFromXmpInfo.CaptionWriter));

	// Image
	DisplayDateCreated();
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_INTELLECTUALGENRE);
	pEdit->SetWindowText(FormatString(m_Metadata.m_XmpInfo.IntellectualGenre));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_IPTCSCENE);
	pEdit->SetWindowText(FormatStringArray(m_Metadata.m_XmpInfo.Scene));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_LOCATION);
	pEdit->SetWindowText(FormatString(m_Metadata.m_XmpInfo.Location));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_CITY);
	pEdit->SetWindowText(FormatString(m_Metadata.m_IptcFromXmpInfo.City));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_STATEPROVINCE);
	pEdit->SetWindowText(FormatString(m_Metadata.m_IptcFromXmpInfo.ProvinceState));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_COUNTRY);
	pEdit->SetWindowText(FormatString(m_Metadata.m_IptcFromXmpInfo.Country));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_ISOCOUNTRYCODE);
	pEdit->SetWindowText(FormatString(m_Metadata.m_XmpInfo.CountryCode));

	// Status
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_TITLE);
	pEdit->SetWindowText(FormatString(m_Metadata.m_IptcFromXmpInfo.ObjectName));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_JOBIDENTIFIER);
	pEdit->SetWindowText(FormatString(m_Metadata.m_IptcFromXmpInfo.OriginalTransmissionReference));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_INSTRUCTIONS);
	pEdit->SetWindowText(FormatString(m_Metadata.m_IptcFromXmpInfo.SpecialInstructions));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_PROVIDER);
	pEdit->SetWindowText(FormatString(m_Metadata.m_IptcFromXmpInfo.Credits));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_SOURCE);
	pEdit->SetWindowText(FormatString(m_Metadata.m_IptcFromXmpInfo.Source));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_COPYRIGHTNOTICE);
	pEdit->SetWindowText(FormatString(m_Metadata.m_IptcFromXmpInfo.CopyrightNotice));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_RIGHTSUSAGETERMS);
	pEdit->SetWindowText(FormatString(m_Metadata.m_XmpInfo.UsageTerms));
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_COPYRIGHTURL);
	pEdit->SetWindowText(FormatString(m_Metadata.m_XmpInfo.CopyrightUrl));
	DisplayCopyrightStatus();
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_URGENCY);
	pEdit->SetWindowText(FormatString(m_Metadata.m_IptcFromXmpInfo.Urgency));
}

void CXmpDlg::GetMetadataFromDisplay()
{
	CEdit* pEdit;
	CString s;

	// Contact
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_CREATOR);
	pEdit->GetWindowText(m_Metadata.m_IptcFromXmpInfo.Byline);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_CREATORSJOB);
	pEdit->GetWindowText(m_Metadata.m_IptcFromXmpInfo.BylineTitle);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_ADDRESS);
	pEdit->GetWindowText(m_Metadata.m_XmpInfo.CiAdrExtadr);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_CITY);
	pEdit->GetWindowText(m_Metadata.m_XmpInfo.CiAdrCity);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_STATEPROVINCE);
	pEdit->GetWindowText(m_Metadata.m_XmpInfo.CiAdrRegion);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_POSTALCODE);
	pEdit->GetWindowText(m_Metadata.m_XmpInfo.CiAdrPcode);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_COUNTRY);
	pEdit->GetWindowText(m_Metadata.m_XmpInfo.CiAdrCtry);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_PHONE);
	pEdit->GetWindowText(m_Metadata.m_XmpInfo.CiTelWork);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_EMAIL);
	pEdit->GetWindowText(m_Metadata.m_XmpInfo.CiEmailWork);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTACT_WEBSITE);
	pEdit->GetWindowText(m_Metadata.m_XmpInfo.CiUrlWork);

	// Content
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTENT_HEADLINE);
	pEdit->GetWindowText(m_Metadata.m_IptcFromXmpInfo.Headline);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTENT_DESCRIPTION);
	pEdit->GetWindowText(m_Metadata.m_IptcFromXmpInfo.Caption);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTENT_KEYWORDS);
	pEdit->GetWindowText(s);
	ParseToStringArray(s, m_Metadata.m_IptcFromXmpInfo.Keywords);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTENT_IPTCSUBJECTCODE);
	pEdit->GetWindowText(s);
	ParseToStringArray(s, m_Metadata.m_XmpInfo.SubjectCode);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTENT_CATEGORY);
	pEdit->GetWindowText(m_Metadata.m_IptcFromXmpInfo.Category);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTENT_SUPPLEMENTALCATEGORIES);
	pEdit->GetWindowText(s);
	ParseToStringArray(s, m_Metadata.m_IptcFromXmpInfo.SupplementalCategories);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONTENT_DESCRIPTIONWRITER);
	pEdit->GetWindowText(m_Metadata.m_IptcFromXmpInfo.CaptionWriter);

	// Image
	ParseDateCreated();
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_INTELLECTUALGENRE);
	pEdit->GetWindowText(m_Metadata.m_XmpInfo.IntellectualGenre);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_IPTCSCENE);
	pEdit->GetWindowText(s);
	ParseToStringArray(s, m_Metadata.m_XmpInfo.Scene);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_LOCATION);
	pEdit->GetWindowText(m_Metadata.m_XmpInfo.Location);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_CITY);
	pEdit->GetWindowText(m_Metadata.m_IptcFromXmpInfo.City);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_STATEPROVINCE);
	pEdit->GetWindowText(m_Metadata.m_IptcFromXmpInfo.ProvinceState);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_COUNTRY);
	pEdit->GetWindowText(m_Metadata.m_IptcFromXmpInfo.Country);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_IMAGE_ISOCOUNTRYCODE);
	pEdit->GetWindowText(m_Metadata.m_XmpInfo.CountryCode);

	// Status
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_TITLE);
	pEdit->GetWindowText(m_Metadata.m_IptcFromXmpInfo.ObjectName);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_JOBIDENTIFIER);
	pEdit->GetWindowText(m_Metadata.m_IptcFromXmpInfo.OriginalTransmissionReference);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_INSTRUCTIONS);
	pEdit->GetWindowText(m_Metadata.m_IptcFromXmpInfo.SpecialInstructions);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_PROVIDER);
	pEdit->GetWindowText(m_Metadata.m_IptcFromXmpInfo.Credits);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_SOURCE);
	pEdit->GetWindowText(m_Metadata.m_IptcFromXmpInfo.Source);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_COPYRIGHTNOTICE);
	pEdit->GetWindowText(m_Metadata.m_IptcFromXmpInfo.CopyrightNotice);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_RIGHTSUSAGETERMS);
	pEdit->GetWindowText(m_Metadata.m_XmpInfo.UsageTerms);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_COPYRIGHTURL);
	pEdit->GetWindowText(m_Metadata.m_XmpInfo.CopyrightUrl);
	ParseCopyrightStatus();
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_STATUS_URGENCY);
	pEdit->GetWindowText(m_Metadata.m_IptcFromXmpInfo.Urgency);
}

void CXmpDlg::OnOK()
{
	// Get Data From Edit Controls
	if (IDD == IDD_XMP)
		GetMetadataFromDisplay();
	CDialog::OnOK();
	SaveSettings();
}

void CXmpDlg::OnCancel()
{
	// Get Data From Edit Controls
	if (IDD == IDD_XMP)
		GetMetadataFromDisplay();
	CDialog::OnCancel();
	SaveSettings();
}

void CXmpDlg::GetFields(CMetadata::IPTCINFO& IptcInfo,
						CMetadata::XMPINFO& XmpInfo)
{
	// Contact
	GetField(	m_nRadioByline,
				m_Metadata.m_IptcFromXmpInfo.Byline,
				IptcInfo.Byline);
	GetField(	m_nRadioBylineTitle,
				m_Metadata.m_IptcFromXmpInfo.BylineTitle,
				IptcInfo.BylineTitle);
	GetField(	m_nRadioCiAdrExtadr,
				m_Metadata.m_XmpInfo.CiAdrExtadr,
				XmpInfo.CiAdrExtadr);
	GetField(	m_nRadioCiAdrCity,
				m_Metadata.m_XmpInfo.CiAdrCity,
				XmpInfo.CiAdrCity);
	GetField(	m_nRadioCiAdrRegion,
				m_Metadata.m_XmpInfo.CiAdrRegion,
				XmpInfo.CiAdrRegion);
	GetField(	m_nRadioCiAdrPcode,
				m_Metadata.m_XmpInfo.CiAdrPcode,
				XmpInfo.CiAdrPcode);
	GetField(	m_nRadioCiAdrCtry,
				m_Metadata.m_XmpInfo.CiAdrCtry,
				XmpInfo.CiAdrCtry);
	GetField(	m_nRadioCiTelWork,
				m_Metadata.m_XmpInfo.CiTelWork,
				XmpInfo.CiTelWork);
	GetField(	m_nRadioCiEmailWork,
				m_Metadata.m_XmpInfo.CiEmailWork,
				XmpInfo.CiEmailWork);
	GetField(	m_nRadioCiUrlWork,
				m_Metadata.m_XmpInfo.CiUrlWork,
				XmpInfo.CiUrlWork);

	// Content
	GetField(	m_nRadioHeadline,
				m_Metadata.m_IptcFromXmpInfo.Headline,
				IptcInfo.Headline);
	GetField(	m_nRadioCaption,
				m_Metadata.m_IptcFromXmpInfo.Caption,
				IptcInfo.Caption);
	GetFieldArray(	m_nRadioKeywords,
					m_Metadata.m_IptcFromXmpInfo.Keywords,
					IptcInfo.Keywords);
	GetFieldArray(	m_nRadioSubjectCode,
					m_Metadata.m_XmpInfo.SubjectCode,
					XmpInfo.SubjectCode);
	GetField(	m_nRadioCategory,
				m_Metadata.m_IptcFromXmpInfo.Category,
				IptcInfo.Category);
	GetFieldArray(	m_nRadioSupplementalCategories,
					m_Metadata.m_IptcFromXmpInfo.SupplementalCategories,
					IptcInfo.SupplementalCategories);
	GetField(	m_nRadioCaptionWriter,
				m_Metadata.m_IptcFromXmpInfo.CaptionWriter,
				IptcInfo.CaptionWriter);

	// Image
	if (m_nRadioDateCreatedSource == 0)
	{
		GetField(	m_nRadioDateCreated,
					m_Metadata.m_IptcFromXmpInfo.DateCreated,
					IptcInfo.DateCreated);
	}
	else
	{
		GetField(	m_nRadioDateCreated,
					m_sExifDateCreated,
					IptcInfo.DateCreated);
	}
	GetField(	m_nRadioIntellectualGenre,
				m_Metadata.m_XmpInfo.IntellectualGenre,
				XmpInfo.IntellectualGenre);
	GetFieldArray(	m_nRadioScene,
					m_Metadata.m_XmpInfo.Scene,
					XmpInfo.Scene);
	GetField(	m_nRadioLocation,
				m_Metadata.m_XmpInfo.Location,
				XmpInfo.Location);
	GetField(	m_nRadioCity,
				m_Metadata.m_IptcFromXmpInfo.City,
				IptcInfo.City);
	GetField(	m_nRadioProvinceState,
				m_Metadata.m_IptcFromXmpInfo.ProvinceState,
				IptcInfo.ProvinceState);
	GetField(	m_nRadioCountry,
				m_Metadata.m_IptcFromXmpInfo.Country,
				IptcInfo.Country);
	GetField(	m_nRadioCountryCode,
				m_Metadata.m_XmpInfo.CountryCode,
				XmpInfo.CountryCode);

	// Status
	GetField(	m_nRadioObjectName,
				m_Metadata.m_IptcFromXmpInfo.ObjectName,
				IptcInfo.ObjectName);
	GetField(	m_nRadioOriginalTransmissionReference,
				m_Metadata.m_IptcFromXmpInfo.OriginalTransmissionReference,
				IptcInfo.OriginalTransmissionReference);
	GetField(	m_nRadioSpecialInstructions,
				m_Metadata.m_IptcFromXmpInfo.SpecialInstructions,
				IptcInfo.SpecialInstructions);
	GetField(	m_nRadioCredits,
				m_Metadata.m_IptcFromXmpInfo.Credits,
				IptcInfo.Credits);
	GetField(	m_nRadioSource,
				m_Metadata.m_IptcFromXmpInfo.Source,
				IptcInfo.Source);
	GetField(	m_nRadioCopyrightNotice,
				m_Metadata.m_IptcFromXmpInfo.CopyrightNotice,
				IptcInfo.CopyrightNotice);
	GetField(	m_nRadioUsageTerms,
				m_Metadata.m_XmpInfo.UsageTerms,
				XmpInfo.UsageTerms);
	GetField(	m_nRadioCopyrightUrl,
				m_Metadata.m_XmpInfo.CopyrightUrl,
				XmpInfo.CopyrightUrl);
	GetField(	m_nRadioCopyrightMarked,
				m_Metadata.m_XmpInfo.CopyrightMarked,
				XmpInfo.CopyrightMarked);
	GetField(	m_nRadioUrgency,
				m_Metadata.m_IptcFromXmpInfo.Urgency,
				IptcInfo.Urgency);
}

void CXmpDlg::GetFields(BOOL bXmpPriority, CMetadata& metadata)
{
	// Merge metadata.m_IptcLegacyInfo with metadata.m_IptcFromXmpInfo,
	// on conflicts give the priority to metadata.m_IptcFromXmpInfo
	// if bXmpPriority is set, otherwise to metadata.m_IptcLegacyInfo.
	MergeIptc(	bXmpPriority,
				metadata.m_IptcLegacyInfo,
				metadata.m_IptcFromXmpInfo);

	// Get Fields 
	GetFields(	metadata.m_IptcFromXmpInfo,
				metadata.m_XmpInfo);
	metadata.m_IptcLegacyInfo = metadata.m_IptcFromXmpInfo;
}

__forceinline void CXmpDlg::GetField(	int nRadioField,
										const CString& sSrcField,
										CString& sDstField)
{
	switch (nRadioField)
	{
		case FIELD_OVERWRITE :
			if (sSrcField != _T(""))
				sDstField = sSrcField;
			break;
			
		case FIELD_SETEMPTY :
			if (sDstField == _T(""))
				sDstField = sSrcField;
			break;
			
		case FIELD_LEAVE :
			break;
			
		case FIELD_DELETE :
			sDstField = _T("");
			break;

		default :
			break;
	}
}

__forceinline void CXmpDlg::GetFieldArray(	int nRadioField,
											const CStringArray& sSrcFieldArray,
											CStringArray& sDstFieldArray)
{
	int i;

	switch (nRadioField)
	{
		case FIELD_OVERWRITE :
			if (sSrcFieldArray.GetSize() > 0)
			{
				sDstFieldArray.RemoveAll();
				for (i = 0 ; i < sSrcFieldArray.GetSize() ; i++)
					sDstFieldArray.Add(sSrcFieldArray[i]);
			}
			break;
			
		case FIELD_SETEMPTY : // Append
			for (i = 0 ; i < sSrcFieldArray.GetSize() ; i++)
			{
				if (!CMetadata::IPTCINFO::IsInArray(sSrcFieldArray[i], sDstFieldArray))
					sDstFieldArray.Add(sSrcFieldArray[i]);
			}
			break;
			
		case FIELD_LEAVE :
			break;
			
		case FIELD_DELETE :
			sDstFieldArray.RemoveAll();
			break;

		default :
			break;
	}
}

void CXmpDlg::MergeIptc(BOOL bXmpPriority,
						CMetadata::IPTCINFO& IptcLegacyInfo,
						CMetadata::IPTCINFO& IptcFromXmpInfo)
{
	if (IptcLegacyInfo.IsEmpty())
	{
		IptcLegacyInfo = IptcFromXmpInfo;
	}
	else if (IptcFromXmpInfo.IsEmpty())
	{
		IptcFromXmpInfo = IptcLegacyInfo;
	}
	else
	{
		// Check for Conflicts between Iptc Legacy and Iptc from Xmp!
		if (IptcLegacyInfo != IptcFromXmpInfo)
		{
			// Caption
			MergeIptc(	bXmpPriority,
						IptcLegacyInfo.Caption,
						IptcFromXmpInfo.Caption);
		
			// Caption Writer
			MergeIptc(	bXmpPriority,
						IptcLegacyInfo.CaptionWriter,
						IptcFromXmpInfo.CaptionWriter);

			// Headline
			MergeIptc(	bXmpPriority,
						IptcLegacyInfo.Headline,
						IptcFromXmpInfo.Headline);

			// Special Instructions
			MergeIptc(	bXmpPriority,
						IptcLegacyInfo.SpecialInstructions,
						IptcFromXmpInfo.SpecialInstructions);

			// Byline
			MergeIptc(	bXmpPriority,
						IptcLegacyInfo.Byline,
						IptcFromXmpInfo.Byline);

			// Byline Title
			MergeIptc(	bXmpPriority,
						IptcLegacyInfo.BylineTitle,
						IptcFromXmpInfo.BylineTitle);

			// Credits
			MergeIptc(	bXmpPriority,
						IptcLegacyInfo.Credits,
						IptcFromXmpInfo.Credits);

			// Source
			MergeIptc(	bXmpPriority,
						IptcLegacyInfo.Source,
						IptcFromXmpInfo.Source);

			// Object Name
			MergeIptc(	bXmpPriority,
						IptcLegacyInfo.ObjectName,
						IptcFromXmpInfo.ObjectName);

			// Date Created
			MergeIptc(	bXmpPriority,
						IptcLegacyInfo.DateCreated,
						IptcFromXmpInfo.DateCreated);

			// City
			MergeIptc(	bXmpPriority,
						IptcLegacyInfo.City,
						IptcFromXmpInfo.City);

			// Province-State
			MergeIptc(	bXmpPriority,
						IptcLegacyInfo.ProvinceState,
						IptcFromXmpInfo.ProvinceState);

			// Country
			MergeIptc(	bXmpPriority,
						IptcLegacyInfo.Country,
						IptcFromXmpInfo.Country);

			// Original Transmission Reference
			MergeIptc(	bXmpPriority,
						IptcLegacyInfo.OriginalTransmissionReference,
						IptcFromXmpInfo.OriginalTransmissionReference);
			
			// Category
			MergeIptc(	bXmpPriority,
						IptcLegacyInfo.Category,
						IptcFromXmpInfo.Category);

			// Supplemental Categories
			MergeIptc(	IptcLegacyInfo.SupplementalCategories,
						IptcFromXmpInfo.SupplementalCategories);

			// Urgency
			MergeIptc(	bXmpPriority,
						IptcLegacyInfo.Urgency,
						IptcFromXmpInfo.Urgency);

			// Keywords
			MergeIptc(	IptcLegacyInfo.Keywords,
						IptcFromXmpInfo.Keywords);

			// Copyright Notice
			MergeIptc(	bXmpPriority,
						IptcLegacyInfo.CopyrightNotice,
						IptcFromXmpInfo.CopyrightNotice);
		}
	}
}

void CXmpDlg::MergeIptc(BOOL bXmpPriority,
						CString& sIptcLegacy,
						CString& sIptcFromXmp)
{
	if (sIptcLegacy == _T(""))
	{
		sIptcLegacy = sIptcFromXmp;
	}
	else if (sIptcFromXmp == _T(""))
	{
		sIptcFromXmp = sIptcLegacy;
	}
	// - Photoshop shows Xmp if out of sync.
	// - PixVue shows Iptc Legacy if out of sync.
	else if (sIptcLegacy != sIptcFromXmp)
	{
		if (bXmpPriority)
			sIptcLegacy = sIptcFromXmp;
		else
			sIptcFromXmp = sIptcLegacy;
	}
}

void CXmpDlg::MergeIptc(CStringArray& IptcLegacy,
						CStringArray& IptcFromXmp)
{
	if (IptcLegacy.GetSize() == 0)
	{
		IptcLegacy.Copy(IptcFromXmp);
	}
	else if (IptcFromXmp.GetSize() == 0)
	{
		IptcFromXmp.Copy(IptcLegacy);
	}
	// Merge IptcLegacy with IptcFromXmp
	else if (!CMetadata::IPTCINFO::IsArrayEqual(IptcLegacy,
												IptcFromXmp))
	{
		for (int i = 0 ; i < IptcFromXmp.GetSize() ; i++)
		{
			if (!CMetadata::IPTCINFO::IsInArray(IptcFromXmp[i], IptcLegacy))
				IptcLegacy.Add(IptcFromXmp[i]);
		}
		IptcFromXmp.RemoveAll();
		IptcFromXmp.Copy(IptcLegacy);
	}
}

void CXmpDlg::OnButtonXmpload() 
{
	TCHAR* InitDir = new TCHAR[MAX_FILEDLG_PATH];
	InitDir[0] = _T('\0');
	CString sXmpLastOpenedDir = ::GetDriveAndDirName(m_sXmpLoadFile);
	sXmpLastOpenedDir.TrimRight(_T('\\'));
	if (::IsExistingDir(sXmpLastOpenedDir))
		_tcscpy(InitDir, (LPCTSTR)sXmpLastOpenedDir);
	CNoVistaFileDlg fd(	TRUE,
						_T("xmp"),
						_T(""),
						OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, // file must exist and hide the read-only check box
						_T("Xmp Files (*.xmp)|*.xmp||"));
	fd.m_ofn.lpstrInitialDir = InitDir;
	if (fd.DoModal() == IDOK)
	{
		// Set File Name
		m_sXmpLoadFile = fd.GetPathName();
		
		// Parse Xmp
		ParseXmp();

		// Display Metadata
		DisplayMetadata();

		// Set Xmp File Edit Box
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_XMPLOAD);
		pEdit->SetWindowText(m_sXmpLoadFile);
	}

	// Free
	delete [] InitDir;
}

void CXmpDlg::OnButtonXmpsave() 
{
	TCHAR* InitDir = new TCHAR[MAX_FILEDLG_PATH];
	InitDir[0] = _T('\0');
	CString sXmpLastSavedDir = ::GetDriveAndDirName(m_sXmpSaveFile);
	sXmpLastSavedDir.TrimRight(_T('\\'));
	if (::IsExistingDir(sXmpLastSavedDir))
		_tcscpy(InitDir, (LPCTSTR)sXmpLastSavedDir);
	CString sXmpFileName = ::GetShortFileName(m_sXmpSaveFile);
	CNoVistaFileDlg fd(	FALSE,
						_T("xmp"),
						sXmpFileName,
						OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
						_T("Xmp Files (*.xmp)|*.xmp||"));
	fd.m_ofn.lpstrInitialDir = InitDir;
	if (fd.DoModal() == IDOK)
	{
		// Set File Name
		m_sXmpSaveFile = fd.GetPathName();

		// Export Xmp
		ExportXmp(m_sXmpSaveFile);
	}

	// Free
	delete [] InitDir;
}

BOOL CXmpDlg::ExportXmp(LPCTSTR lpszFileName)
{
	CString s = _T("");
	CString t;

	// Get Data From Edit Controls
	GetMetadataFromDisplay();
	
	// Update m_pXmpData & m_dwXmpSize
	m_Metadata.UpdateXmpData(_T("image/jpeg"));

	// Save
	if (m_Metadata.m_pXmpData && 
		m_Metadata.m_dwXmpSize > 0)
	{
		// Parse Xml
		CString sXml = ::FromUTF8(	(const unsigned char*)m_Metadata.m_pXmpData,
									(int)m_Metadata.m_dwXmpSize);

		XDoc xml;
		PARSEINFO pi;
		pi.trim_value = true;				// trim value
		sXml.Replace(_T("\\"), _T("\\\\"));	// escape
		if (xml.Load(sXml, &pi))
		{
			// Change BOM
			LPXAttr pAttr = xml.GetChildAttr(_T("xpacket"), _T("begin"));
			if (pAttr && pAttr->value != _T(""))
				pAttr->value = _T("123");

			// Get Xml
			DISP_OPT opt;
			opt.newline = true;
			_tcsncpy(opt.newline_type, _T("\n"), 3);
			opt.value_quotation_mark = _T('\'');
			sXml = xml.GetXML(&opt);
			sXml.TrimLeft(_T('\n')); // Remove initial \n
			LPBYTE pData = NULL;
			int nSize = ::ToUTF8(sXml, &pData);

			// Restore BOM
			int nBOMPos = 0;
			while (nBOMPos < ((int)nSize - 6) && memcmp(&pData[nBOMPos], "begin=", 6) != 0)
				nBOMPos++;
			if (nBOMPos < ((int)nSize - 11) && memcmp(&pData[nBOMPos], "begin='123'", 11) == 0)
			{
				pData[nBOMPos+7] = 0xEF;
				pData[nBOMPos+8] = 0xBB;
				pData[nBOMPos+9] = 0xBF;
			}

			// Create the Xmp File
			try
			{
				CFile f(lpszFileName, CFile::modeCreate | CFile::modeWrite);
				f.Write(pData, nSize);
				f.Close();
			}
			catch (CFileException* e)
			{
				if (pData)
					delete [] pData;
				e->ReportError();
				e->Delete();
				return FALSE;
			}

			// Free
			if (pData)
				delete [] pData;

			return TRUE;
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}

void CXmpDlg::OnButtonXmpunload() 
{
	// Reset File Name
	m_sXmpLoadFile = _T("");
	
	// Free Metadata
	m_Metadata.Free();

	// Reset Display
	DisplayMetadata();

	// Reset Xmp File Edit Box
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_XMPLOAD);
	pEdit->SetWindowText(_T(""));
}

void CXmpDlg::LoadSettings()
{
	CWinApp* pApp = ::AfxGetApp();
	CString sSection;

	if (IDD == IDD_XMP_IMPORT)
		sSection = _T("XmpImportDlg");
	else
		sSection = _T("XmpDlg");
	
	// Contact
	m_nRadioByline =					pApp->GetProfileInt(sSection, _T("RadioByline"), 1);
	m_nRadioBylineTitle =				pApp->GetProfileInt(sSection, _T("RadioBylineTitle"), 1); 
	m_nRadioCiAdrExtadr =				pApp->GetProfileInt(sSection, _T("RadioCiAdrExtadr"), 1);
	m_nRadioCiAdrCity =					pApp->GetProfileInt(sSection, _T("RadioCiAdrCity"),	1); 
	m_nRadioCiAdrRegion =				pApp->GetProfileInt(sSection, _T("RadioCiAdrRegion"), 1); 
	m_nRadioCiAdrPcode =				pApp->GetProfileInt(sSection, _T("RadioCiAdrPcode"), 1); 
	m_nRadioCiAdrCtry =					pApp->GetProfileInt(sSection, _T("RadioCiAdrCtry"),	1); 
	m_nRadioCiTelWork =					pApp->GetProfileInt(sSection, _T("RadioCiTelWork"), 1); 
	m_nRadioCiEmailWork =				pApp->GetProfileInt(sSection, _T("RadioCiEmailWork"), 1); 
	m_nRadioCiUrlWork =					pApp->GetProfileInt(sSection, _T("RadioCiUrlWork"),	1);

	// Content
	m_nRadioHeadline =					pApp->GetProfileInt(sSection, _T("RadioHeadline"), 1);
	m_nRadioCaption =					pApp->GetProfileInt(sSection, _T("RadioCaption"), 1);
	m_nRadioKeywords =					pApp->GetProfileInt(sSection, _T("RadioKeywords"), 1);
	m_nRadioSubjectCode =				pApp->GetProfileInt(sSection, _T("RadioSubjectCode"), 1);
	m_nRadioCategory =					pApp->GetProfileInt(sSection, _T("RadioCategory"), 1);
	m_nRadioSupplementalCategories =	pApp->GetProfileInt(sSection, _T("RadioSupplementalCategories"), 1);
	m_nRadioCaptionWriter =				pApp->GetProfileInt(sSection, _T("RadioCaptionWriter"), 1);

	// Image
	m_nRadioDateCreated =				pApp->GetProfileInt(sSection, _T("RadioDateCreated"), 1);
	m_nRadioIntellectualGenre =			pApp->GetProfileInt(sSection, _T("RadioIntellectualGenre"), 1);
	m_nRadioScene =						pApp->GetProfileInt(sSection, _T("RadioScene"), 1);
	m_nRadioLocation =					pApp->GetProfileInt(sSection, _T("RadioLocation"), 1);
	m_nRadioCity =						pApp->GetProfileInt(sSection, _T("RadioCity"), 1);
	m_nRadioProvinceState =				pApp->GetProfileInt(sSection, _T("RadioProvinceState"), 1);
	m_nRadioCountry =					pApp->GetProfileInt(sSection, _T("RadioCountry"), 1);
	m_nRadioCountryCode =				pApp->GetProfileInt(sSection, _T("RadioCountryCode"), 1);

	// Status
	m_nRadioObjectName =				pApp->GetProfileInt(sSection, _T("RadioObjectName"), 1);
	m_nRadioOriginalTransmissionReference = pApp->GetProfileInt(sSection, _T("RadioOriginalTransmissionReference"), 1);
	m_nRadioSpecialInstructions =		pApp->GetProfileInt(sSection, _T("RadioSpecialInstructions"), 1);
	m_nRadioCredits =					pApp->GetProfileInt(sSection, _T("RadioCredits"), 1);
	m_nRadioSource =					pApp->GetProfileInt(sSection, _T("RadioSource"), 1);
	m_nRadioCopyrightNotice =			pApp->GetProfileInt(sSection, _T("RadioCopyrightNotice"), 1);
	m_nRadioUsageTerms =				pApp->GetProfileInt(sSection, _T("RadioUsageTerms"), 1);
	m_nRadioCopyrightUrl =				pApp->GetProfileInt(sSection, _T("RadioCopyrightUrl"), 1);
	m_nRadioCopyrightMarked =			pApp->GetProfileInt(sSection, _T("RadioCopyrightMarked"), 1);
	m_nRadioUrgency =					pApp->GetProfileInt(sSection, _T("RadioUrgency"), 1);

	// Date Created Source
	m_nRadioDateCreatedSource =			pApp->GetProfileInt(sSection, _T("RadioDateCreatedSource"), 1);

	// Xmp File
	m_sXmpLoadFile =					pApp->GetProfileString(sSection, _T("XmpLoadFile"), _T(""));
	if (!::IsExistingFile(m_sXmpLoadFile))
		m_sXmpLoadFile = _T("");
	if (m_sXmpLoadFile != _T(""))
		ParseXmp();
	m_sXmpSaveFile =					pApp->GetProfileString(sSection, _T("XmpSaveFile"), _T(""));
}

void CXmpDlg::SaveSettings()
{
	CWinApp* pApp = ::AfxGetApp();
	CString sSection;

	if (IDD == IDD_XMP_IMPORT)
		sSection = _T("XmpImportDlg");
	else
		sSection = _T("XmpDlg");

	// Contact
	pApp->WriteProfileInt(sSection, _T("RadioByline"),		m_nRadioByline);
	pApp->WriteProfileInt(sSection, _T("RadioBylineTitle"), m_nRadioBylineTitle); 
	pApp->WriteProfileInt(sSection, _T("RadioCiAdrExtadr"), m_nRadioCiAdrExtadr);
	pApp->WriteProfileInt(sSection, _T("RadioCiAdrCity"),	m_nRadioCiAdrCity); 
	pApp->WriteProfileInt(sSection, _T("RadioCiAdrRegion"), m_nRadioCiAdrRegion); 
	pApp->WriteProfileInt(sSection, _T("RadioCiAdrPcode"),	m_nRadioCiAdrPcode); 
	pApp->WriteProfileInt(sSection, _T("RadioCiAdrCtry"),	m_nRadioCiAdrCtry); 
	pApp->WriteProfileInt(sSection, _T("RadioCiTelWork"),	m_nRadioCiTelWork); 
	pApp->WriteProfileInt(sSection, _T("RadioCiEmailWork"), m_nRadioCiEmailWork); 
	pApp->WriteProfileInt(sSection, _T("RadioCiUrlWork"),	m_nRadioCiUrlWork); 

	// Content
	pApp->WriteProfileInt(sSection, _T("RadioHeadline"),	m_nRadioHeadline);
	pApp->WriteProfileInt(sSection, _T("RadioCaption"),		m_nRadioCaption);
	pApp->WriteProfileInt(sSection, _T("RadioKeywords"),	m_nRadioKeywords);
	pApp->WriteProfileInt(sSection, _T("RadioSubjectCode"), m_nRadioSubjectCode);
	pApp->WriteProfileInt(sSection, _T("RadioCategory"),	m_nRadioCategory);
	pApp->WriteProfileInt(sSection, _T("RadioSupplementalCategories"), m_nRadioSupplementalCategories);
	pApp->WriteProfileInt(sSection, _T("RadioCaptionWriter"), m_nRadioCaptionWriter);

	// Image
	pApp->WriteProfileInt(sSection, _T("RadioDateCreated"),	m_nRadioDateCreated);
	pApp->WriteProfileInt(sSection, _T("RadioIntellectualGenre"), m_nRadioIntellectualGenre);
	pApp->WriteProfileInt(sSection, _T("RadioScene"),		m_nRadioScene);
	pApp->WriteProfileInt(sSection, _T("RadioLocation"),	m_nRadioLocation);
	pApp->WriteProfileInt(sSection, _T("RadioCity"),		m_nRadioCity);
	pApp->WriteProfileInt(sSection, _T("RadioProvinceState"), m_nRadioProvinceState);
	pApp->WriteProfileInt(sSection, _T("RadioCountry"),		m_nRadioCountry);
	pApp->WriteProfileInt(sSection, _T("RadioCountryCode"),	m_nRadioCountryCode);

	// Status
	pApp->WriteProfileInt(sSection, _T("RadioObjectName"),	m_nRadioObjectName);
	pApp->WriteProfileInt(sSection, _T("RadioOriginalTransmissionReference"), m_nRadioOriginalTransmissionReference);
	pApp->WriteProfileInt(sSection, _T("RadioSpecialInstructions"), m_nRadioSpecialInstructions);
	pApp->WriteProfileInt(sSection, _T("RadioCredits"),		m_nRadioCredits);
	pApp->WriteProfileInt(sSection, _T("RadioSource"),		m_nRadioSource);
	pApp->WriteProfileInt(sSection, _T("RadioCopyrightNotice"), m_nRadioCopyrightNotice);
	pApp->WriteProfileInt(sSection, _T("RadioUsageTerms"),	m_nRadioUsageTerms);
	pApp->WriteProfileInt(sSection, _T("RadioCopyrightUrl"),	m_nRadioCopyrightUrl);
	pApp->WriteProfileInt(sSection, _T("RadioCopyrightMarked"), m_nRadioCopyrightMarked);
	pApp->WriteProfileInt(sSection, _T("RadioUrgency"),		m_nRadioUrgency);

	// Date Created Source
	pApp->WriteProfileInt(sSection, _T("RadioDateCreatedSource"), m_nRadioDateCreatedSource);

	// Xmp File
	pApp->WriteProfileString(sSection, _T("XmpLoadFile"), m_sXmpLoadFile);
	pApp->WriteProfileString(sSection, _T("XmpSaveFile"), m_sXmpSaveFile);
}

void CXmpDlg::OnButtonContactOverwrite() 
{
	m_nRadioByline = 0;
	m_nRadioBylineTitle = 0;
	m_nRadioCiAdrExtadr = 0;
	m_nRadioCiAdrCity = 0;
	m_nRadioCiAdrRegion = 0;
	m_nRadioCiAdrPcode = 0;
	m_nRadioCiAdrCtry = 0;
	m_nRadioCiTelWork = 0;
	m_nRadioCiEmailWork = 0;
	m_nRadioCiUrlWork = 0;

	UpdateData(FALSE);
}

void CXmpDlg::OnButtonContactSet() 
{
	m_nRadioByline = 1;
	m_nRadioBylineTitle = 1;
	m_nRadioCiAdrExtadr = 1;
	m_nRadioCiAdrCity = 1;
	m_nRadioCiAdrRegion = 1;
	m_nRadioCiAdrPcode = 1;
	m_nRadioCiAdrCtry = 1;
	m_nRadioCiTelWork = 1;
	m_nRadioCiEmailWork = 1;
	m_nRadioCiUrlWork = 1;

	UpdateData(FALSE);
}

void CXmpDlg::OnButtonContactLeave() 
{
	m_nRadioByline = 2;
	m_nRadioBylineTitle = 2;
	m_nRadioCiAdrExtadr = 2;
	m_nRadioCiAdrCity = 2;
	m_nRadioCiAdrRegion = 2;
	m_nRadioCiAdrPcode = 2;
	m_nRadioCiAdrCtry = 2;
	m_nRadioCiTelWork = 2;
	m_nRadioCiEmailWork = 2;
	m_nRadioCiUrlWork = 2;

	UpdateData(FALSE);
}

void CXmpDlg::OnButtonContactDel() 
{
	m_nRadioByline = 3;
	m_nRadioBylineTitle = 3;
	m_nRadioCiAdrExtadr = 3;
	m_nRadioCiAdrCity = 3;
	m_nRadioCiAdrRegion = 3;
	m_nRadioCiAdrPcode = 3;
	m_nRadioCiAdrCtry = 3;
	m_nRadioCiTelWork = 3;
	m_nRadioCiEmailWork = 3;
	m_nRadioCiUrlWork = 3;

	UpdateData(FALSE);
}

void CXmpDlg::OnButtonContentOverwrite() 
{
	m_nRadioHeadline = 0;
	m_nRadioCaption = 0;
	m_nRadioKeywords = 0;
	m_nRadioSubjectCode = 0;
	m_nRadioCategory = 0;
	m_nRadioSupplementalCategories = 0;
	m_nRadioCaptionWriter = 0;

	UpdateData(FALSE);
}

void CXmpDlg::OnButtonContentSet() 
{
	m_nRadioHeadline = 1;
	m_nRadioCaption = 1;
	m_nRadioKeywords = 1;
	m_nRadioSubjectCode = 1;
	m_nRadioCategory = 1;
	m_nRadioSupplementalCategories = 1;
	m_nRadioCaptionWriter = 1;

	UpdateData(FALSE);
}

void CXmpDlg::OnButtonContentLeave() 
{
	m_nRadioHeadline = 2;
	m_nRadioCaption = 2;
	m_nRadioKeywords = 2;
	m_nRadioSubjectCode = 2;
	m_nRadioCategory = 2;
	m_nRadioSupplementalCategories = 2;
	m_nRadioCaptionWriter = 2;

	UpdateData(FALSE);
}

void CXmpDlg::OnButtonContentDel() 
{
	m_nRadioHeadline = 3;
	m_nRadioCaption = 3;
	m_nRadioKeywords = 3;
	m_nRadioSubjectCode = 3;
	m_nRadioCategory = 3;
	m_nRadioSupplementalCategories = 3;
	m_nRadioCaptionWriter = 3;

	UpdateData(FALSE);
}

void CXmpDlg::OnButtonImageOverwrite() 
{
	m_nRadioDateCreated = 0;
	m_nRadioIntellectualGenre = 0;
	m_nRadioScene = 0;
	m_nRadioLocation = 0;
	m_nRadioCity = 0;
	m_nRadioProvinceState = 0;
	m_nRadioCountry = 0;
	m_nRadioCountryCode = 0;

	UpdateData(FALSE);
}

void CXmpDlg::OnButtonImageSet() 
{
	m_nRadioDateCreated = 1;
	m_nRadioIntellectualGenre = 1;
	m_nRadioScene = 1;
	m_nRadioLocation = 1;
	m_nRadioCity = 1;
	m_nRadioProvinceState = 1;
	m_nRadioCountry = 1;
	m_nRadioCountryCode = 1;
	
	UpdateData(FALSE);
}

void CXmpDlg::OnButtonImageLeave() 
{
	m_nRadioDateCreated = 2;
	m_nRadioIntellectualGenre = 2;
	m_nRadioScene = 2;
	m_nRadioLocation = 2;
	m_nRadioCity = 2;
	m_nRadioProvinceState = 2;
	m_nRadioCountry = 2;
	m_nRadioCountryCode = 2;

	UpdateData(FALSE);
}

void CXmpDlg::OnButtonImageDel() 
{
	m_nRadioDateCreated = 3;
	m_nRadioIntellectualGenre = 3;
	m_nRadioScene = 3;
	m_nRadioLocation = 3;
	m_nRadioCity = 3;
	m_nRadioProvinceState = 3;
	m_nRadioCountry = 3;
	m_nRadioCountryCode = 3;
	
	UpdateData(FALSE);
}

void CXmpDlg::OnButtonStatusOverwrite() 
{
	m_nRadioObjectName = 0;
	m_nRadioOriginalTransmissionReference = 0;
	m_nRadioSpecialInstructions = 0;
	m_nRadioCredits = 0;
	m_nRadioSource = 0;
	m_nRadioCopyrightNotice = 0;
	m_nRadioUsageTerms = 0;
	m_nRadioCopyrightUrl = 0;
	m_nRadioCopyrightMarked = 0;
	m_nRadioUrgency = 0;
	
	UpdateData(FALSE);
}

void CXmpDlg::OnButtonStatusSet() 
{
	m_nRadioObjectName = 1;
	m_nRadioOriginalTransmissionReference = 1;
	m_nRadioSpecialInstructions = 1;
	m_nRadioCredits = 1;
	m_nRadioSource = 1;
	m_nRadioCopyrightNotice = 1;
	m_nRadioUsageTerms = 1;
	m_nRadioCopyrightUrl = 1;
	m_nRadioCopyrightMarked = 1;
	m_nRadioUrgency = 1;
	
	UpdateData(FALSE);
}

void CXmpDlg::OnButtonStatusLeave() 
{
	m_nRadioObjectName = 2;
	m_nRadioOriginalTransmissionReference = 2;
	m_nRadioSpecialInstructions = 2;
	m_nRadioCredits = 2;
	m_nRadioSource = 2;
	m_nRadioCopyrightNotice = 2;
	m_nRadioUsageTerms = 2;
	m_nRadioCopyrightUrl = 2;
	m_nRadioCopyrightMarked = 2;
	m_nRadioUrgency = 2;

	UpdateData(FALSE);
}

void CXmpDlg::OnButtonStatusDel() 
{
	m_nRadioObjectName = 3;
	m_nRadioOriginalTransmissionReference = 3;
	m_nRadioSpecialInstructions = 3;
	m_nRadioCredits = 3;
	m_nRadioSource = 3;
	m_nRadioCopyrightNotice = 3;
	m_nRadioUsageTerms = 3;
	m_nRadioCopyrightUrl = 3;
	m_nRadioCopyrightMarked = 3;
	m_nRadioUrgency = 3;

	UpdateData(FALSE);
}
