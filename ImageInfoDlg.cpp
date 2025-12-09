// ImageInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "ImageInfoDlg.h"
#include "PictureDoc.h"
#include "PictureView.h"
#include "XMLite.h"
#include "NoVistaFileDlg.h"
extern "C"
{
#include "icc.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImageInfoDlg dialog

CImageInfoDlg::CImageInfoDlg(CPictureDoc* pDoc) : cdxCDynamicDialog(IDD = IDD_IMAGEINFO, NULL)
{
	//{{AFX_DATA_INIT(CImageInfoDlg)
	m_nMetadataGroupView = EXIF;
	//}}AFX_DATA_INIT
	m_nMetadataType = FILE_COMMENT;
	m_pDoc = pDoc;
	ASSERT_VALID(m_pDoc);
	CPictureView* pView = m_pDoc->GetView();
	pView->ForceCursor();

	// I do not see a difference -> disable them!
	ModifyFlags(0, flSWPCopyBits);
	ModifyFlags(0, flAntiFlicker);

	// Enable bottom-right size icon
	ModifyFlags(flSizeIcon, 0);
	
	// Load Settings
	LoadSettings();

	cdxCDynamicDialog::Create(CImageInfoDlg::IDD, NULL);
}

CImageInfoDlg::~CImageInfoDlg()
{
	
}

void CImageInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	cdxCDynamicDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImageInfoDlg)
	DDX_Radio(pDX, IDC_RADIO_METADATA, m_nMetadataGroupView);
	DDX_Control(pDX, IDC_COMBO_METADATA, m_cbMetadata);
	//}}AFX_DATA_MAP
	DDX_CBIndex(pDX, IDC_COMBO_METADATA, m_nMetadataType);
}

BEGIN_MESSAGE_MAP(CImageInfoDlg, cdxCDynamicDialog)
	//{{AFX_MSG_MAP(CImageInfoDlg)
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_EDIT_METADATA, OnChangeMetadata)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_METADATA, OnButtonSaveMetadata)
	ON_CBN_SELCHANGE(IDC_COMBO_METADATA, OnSelchangeComboMetadata)
	ON_BN_CLICKED(IDC_RADIO_METADATA, OnRadioMetadata)
	ON_BN_CLICKED(IDC_RADIO_EXIF, OnRadioExif)
	ON_BN_CLICKED(IDC_RADIO_IPTC_LEGACY, OnRadioIptcLegacy)
	ON_BN_CLICKED(IDC_RADIO_XMP, OnRadioXmp)
	ON_BN_CLICKED(IDC_RADIO_ICC, OnRadioIcc)
	ON_BN_CLICKED(IDC_BUTTON_PREV_METADATA, OnButtonPrevMetadata)
	ON_BN_CLICKED(IDC_BUTTON_NEXT_METADATA, OnButtonNextMetadata)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT_METADATA, OnButtonImportMetadata)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_METADATA, OnButtonExportMetadata)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageInfoDlg message handlers

BOOL CImageInfoDlg::OnInitDialog() 
{
	cdxCDynamicDialog::OnInitDialog();

	// Add Resizing Controls
	
	AddSzControl(IDC_FILEINFO, mdResize, mdNone);
	AddSzControl(IDC_FILEINFO_BORDER, mdResize, mdNone);
	
	AddSzControl(IDC_METADATA, mdResize, mdResize);
	AddSzControl(IDC_METADATA_BORDER, mdResize, mdResize);

	AddSzControl(IDC_RADIO_METADATA, mdNone, mdNone);
	AddSzControl(IDC_RADIO_EXIF, mdNone, mdNone);
	AddSzControl(IDC_RADIO_IPTC_LEGACY, mdNone, mdNone);
	AddSzControl(IDC_RADIO_XMP, mdNone, mdNone);
	AddSzControl(IDC_EDIT_METADATA, mdResize, mdRepos);
	AddSzControl(IDC_EDIT_METADATA_BORDER, mdResize, mdRepos);
	AddSzControl(IDC_BUTTON_PREV_METADATA, mdRepos, mdRepos);
	AddSzControl(IDC_BUTTON_NEXT_METADATA, mdRepos, mdRepos);
	AddSzControl(IDC_BUTTON_IMPORT_METADATA, mdRepos, mdRepos);
	AddSzControl(IDC_BUTTON_EXPORT_METADATA, mdRepos, mdRepos);
	AddSzControl(IDC_BUTTON_SAVE_METADATA, mdRepos, mdRepos);
	AddSzControl(IDC_COMBO_METADATA, mdResize, mdRepos);

	// Set tab stop
	CEdit* pMetadataBox = (CEdit*)GetDlgItem(IDC_METADATA);
	pMetadataBox->SetTabStops(110); // in dialog units, which are based on the current system font

	// File Comment
	m_cbMetadata.AddString(ML_STRING(1581, "File: Comment (Not part of Iptc or Xmp)"));

	// Contact
	CString sContact = ML_STRING(1577, "Contact");
	m_cbMetadata.AddString(sContact + _T(": ") + ML_STRING(1582, "Creator (Byline)"));
	m_cbMetadata.AddString(sContact + _T(": ") + ML_STRING(1583, "Creator's job title (Byline title)"));
	m_cbMetadata.AddString(sContact + _T(": ") + ML_STRING(1584, "Address"));
	m_cbMetadata.AddString(sContact + _T(": ") + ML_STRING(1585, "City"));
	m_cbMetadata.AddString(sContact + _T(": ") + ML_STRING(1586, "State/Province"));
	m_cbMetadata.AddString(sContact + _T(": ") + ML_STRING(1587, "Postal code"));
	m_cbMetadata.AddString(sContact + _T(": ") + ML_STRING(1588, "Country"));
	m_cbMetadata.AddString(sContact + _T(": ") + ML_STRING(1589, "Phone(s)"));
	m_cbMetadata.AddString(sContact + _T(": ") + ML_STRING(1590, "E-Mail(s)"));
	m_cbMetadata.AddString(sContact + _T(": ") + ML_STRING(1591, "Website(s)"));
	
	// Content
	CString sContent = ML_STRING(1578, "Content");
	m_cbMetadata.AddString(sContent + _T(": ") + ML_STRING(1592, "Headline"));
	m_cbMetadata.AddString(sContent + _T(": ") + ML_STRING(1593, "Description (Caption)"));
	m_cbMetadata.AddString(sContent + _T(": ") + ML_STRING(1594, "Keywords"));
	m_cbMetadata.AddString(sContent + _T(": ") + ML_STRING(1596, "IPTC subject code(s)") + _T(" (www.newscodes.org)"));
	m_cbMetadata.AddString(sContent + _T(": ") + ML_STRING(1597, "Category (Deprecated!)"));
	m_cbMetadata.AddString(sContent + _T(": ") + ML_STRING(1606, "Supplemental cat. (Deprecated!)"));
	m_cbMetadata.AddString(sContent + _T(": ") + ML_STRING(1598, "Description writer (Caption writer)"));
	
	// Image
	CString sImage = ML_STRING(1579, "Image");
	m_cbMetadata.AddString(sImage + _T(": ") + ML_STRING(1599, "Date created"));
	m_cbMetadata.AddString(sImage + _T(": ") + ML_STRING(1600, "Intellectual genre"));
	m_cbMetadata.AddString(sImage + _T(": ") + ML_STRING(1602, "IPTC scene(s)") + _T(" (www.newscodes.org)"));
	m_cbMetadata.AddString(sImage + _T(": ") + ML_STRING(1603, "Location"));
	m_cbMetadata.AddString(sImage + _T(": ") + ML_STRING(1585, "City"));
	m_cbMetadata.AddString(sImage + _T(": ") + ML_STRING(1586, "State/Province"));
	m_cbMetadata.AddString(sImage + _T(": ") + ML_STRING(1588, "Country"));
	m_cbMetadata.AddString(sImage + _T(": ") + ML_STRING(1605, "ISO country code (2 or 3 letters, ISO 3166)"));

	// Status
	CString sStatus = ML_STRING(1580, "Status");
	m_cbMetadata.AddString(sStatus + _T(": ") + ML_STRING(1607, "Title (Object name, it's a short readable ID)"));
	m_cbMetadata.AddString(sStatus + _T(": ") + ML_STRING(1608, "Job identifier"));
	m_cbMetadata.AddString(sStatus + _T(": ") + ML_STRING(1609, "Instructions"));
	m_cbMetadata.AddString(sStatus + _T(": ") + ML_STRING(1610, "Provider (Credits)"));
	m_cbMetadata.AddString(sStatus + _T(": ") + ML_STRING(1611, "Source"));
	m_cbMetadata.AddString(sStatus + _T(": ") + ML_STRING(1612, "Copyright notice"));
	m_cbMetadata.AddString(sStatus + _T(": ") + ML_STRING(1614, "Rights usage terms"));
	m_cbMetadata.AddString(sStatus + _T(": ") + ML_STRING(1615, "Copyright URL"));
	m_cbMetadata.AddString(sStatus + _T(": ") + ML_STRING(1616, "Copyright status") + _T(" (Copyrighted or Public Domain)"));
	m_cbMetadata.AddString(sStatus + _T(": ") + ML_STRING(1613, "Urgency (1=high, 5=normal, 8=low, Deprecated!)"));

	// Complete Date Combo Box Entry with Local Short Date Format
	CString sLBText;
	CString sDateFormat = ::GetDateLocalFormat();
	sDateFormat.MakeUpper();
	m_cbMetadata.GetLBText(IMAGE_DATECREATED, sLBText);
	m_cbMetadata.InsertString(IMAGE_DATECREATED, sLBText + _T("  (") + sDateFormat + _T(")"));
	m_cbMetadata.DeleteString(IMAGE_DATECREATED + 1);

	// Set Combo Box Selection
	m_cbMetadata.SetCurSel(m_nMetadataType);

	// Set Edit Metadata Text
	SetEditMetadataText();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CImageInfoDlg::UpdateDlgTitle()
{
	CString sTitle;
	if (m_pDoc->m_sFileName == _T(""))
		sTitle = ML_STRING(1653, "File Info");
	else
		sTitle.Format(ML_STRING(1654, "File Info of %s"), ::GetShortFileName(m_pDoc->m_sFileName));	
	if (m_pDoc->m_bMetadataModified)
		sTitle += _T(" *");
	SetWindowText(sTitle);
}

void CImageInfoDlg::OnClose() 
{
	// Prompt to Save
	if (SaveModified())
	{
		// Save Settings
		SaveSettings();

		// Destroy Window
		DestroyWindow();
	}
}

BOOL CImageInfoDlg::SaveModified(BOOL bPrompt/*=TRUE*/)
{
	if (m_pDoc->m_bMetadataModified)
	{
		int nRet = IDYES;
		if (bPrompt)
		{
			CString Str;
			Str.Format(ML_STRING(1390, "Save edited metadata to %s?"),
									m_pDoc->m_sFileName);
			nRet = ::AfxMessageBox(Str, MB_YESNOCANCEL);
		}
		if (nRet == IDYES)
		{
			if (!SaveMetadata())
				return FALSE;
		}
		else if (nRet == IDNO)
			m_pDoc->m_bMetadataModified = FALSE;
		else
			return FALSE;
	}
	
	return TRUE;
}

void CImageInfoDlg::Close()
{
	OnClose();
}

void CImageInfoDlg::PostNcDestroy() 
{
	m_pDoc->m_pImageInfoDlg = NULL;
	m_pDoc->GetView()->ForceCursor(FALSE);
	delete this;	
	cdxCDynamicDialog::PostNcDestroy();
}

BOOL CImageInfoDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (HIWORD (wParam) == BN_CLICKED)
	{
		switch (LOWORD (wParam))
		{
			case IDOK:
				OnClose();
				return TRUE;
			case IDCANCEL:
				OnClose();
				return TRUE;
			default:
				return cdxCDynamicDialog::OnCommand(wParam, lParam);
		}
	}
	return cdxCDynamicDialog::OnCommand(wParam, lParam);
}

BOOL CImageInfoDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		int nVirtKey = (int)pMsg->wParam;
		if (nVirtKey == VK_TAB)
		{
			if (pMsg->hwnd == ::GetDlgItem(	this->GetSafeHwnd(),
											IDC_EDIT_METADATA))
			{
				if (::GetKeyState(VK_SHIFT) < 0)
				{
					if (PrevMetadata())
						return TRUE;
				}
				else
				{
					if (NextMetadata())
						return TRUE;
				}
			}
		}
	}
	return cdxCDynamicDialog::PreTranslateMessage(pMsg);
}

void CImageInfoDlg::OnSelchangeComboMetadata() 
{
	UpdateData(TRUE);
	SetEditMetadataText();
}

BOOL CImageInfoDlg::PrevMetadata()
{
	BOOL res = FALSE;
	int nPos = m_cbMetadata.GetCurSel();
	if (nPos > 0)
	{
		res = TRUE;
		nPos--;
		m_cbMetadata.SetCurSel(nPos);
		UpdateData(TRUE);
		SetEditMetadataText();
	}

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_METADATA);
	pEdit->SetFocus();

	return res;
}

void CImageInfoDlg::OnButtonPrevMetadata() 
{
	PrevMetadata();
}

BOOL CImageInfoDlg::NextMetadata()
{
	BOOL res = FALSE;
	int nPos = m_cbMetadata.GetCurSel();
	if (nPos < (m_cbMetadata.GetCount() - 1))
	{
		res = TRUE;
		nPos++;
		m_cbMetadata.SetCurSel(nPos);
		UpdateData(TRUE);
		SetEditMetadataText();
	}

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_METADATA);	
	pEdit->SetFocus();

	return res;
}

void CImageInfoDlg::OnButtonNextMetadata() 
{
	NextMetadata();
}

void CImageInfoDlg::OnChangeMetadata() 
{
	// Check whether file is Modified
	if (m_pDoc->IsModified())
	{
		// Message Box
		::AfxMessageBox(ML_STRING(1391, "Please save the image before proceeding\n") +
						ML_STRING(1392, "with the Metadata editing."),
						MB_OK | MB_ICONEXCLAMATION);

		// Reset Edit Metadata Text
		SetEditMetadataText();
	}
	else
	{
		// Get Edit Metadata Text
		GetEditMetadataText();

		// Update Metadata Modified Flag
		UpdateMetadataModifiedFlag();

		// Enable / disable save button for gif files.
		// For jpegs & tiffs the button is always enabled,
		// for other file types it is disabled.
		if (::GetFileExtLower(m_pDoc->m_sFileName) == _T(".gif"))
		{
			CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_SAVE_METADATA);
			if (pButton)
				pButton->EnableWindow(m_pDoc->m_bMetadataModified);
		}
	}
}

void CImageInfoDlg::UpdateMetadataModifiedFlag()
{
	// Update Flag
	if (m_sOrigComment != m_sCurrentComment ||
		m_OrigIptc != m_CurrentIptc			||
		m_OrigXmp != m_CurrentXmp)
		m_pDoc->m_bMetadataModified = TRUE;
	else
		m_pDoc->m_bMetadataModified = FALSE;

	// Update Dialog Title
	UpdateDlgTitle();
}

void CImageInfoDlg::SetEditMetadataText()
{
	int i;
	CString s;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_METADATA);
	BOOL bEnableComment = FALSE;
	BOOL bEnableIptc = FALSE;

	// Enable Comment
	if (CDib::IsJPEG(m_pDoc->m_sFileName) ||
		::GetFileExtLower(m_pDoc->m_sFileName) == _T(".gif"))
		bEnableComment = TRUE;

	// Enable Iptc
	if (CDib::IsJPEG(m_pDoc->m_sFileName) || CDib::IsTIFF(m_pDoc->m_sFileName))
		bEnableIptc = TRUE;

	switch (m_nMetadataType)
	{
		// File
		case FILE_COMMENT :
			pEdit->SetWindowText(m_sCurrentComment);
			pEdit->EnableWindow(bEnableComment);
			break;

		// Contact
		case CONTACT_CREATOR :
			pEdit->SetWindowText(m_CurrentIptc.Byline);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case CONTACT_CREATORSJOBTITLE :
			pEdit->SetWindowText(m_CurrentIptc.BylineTitle);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case CONTACT_ADDRESS :
			pEdit->SetWindowText(m_CurrentXmp.CiAdrExtadr);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case CONTACT_CITY :
			pEdit->SetWindowText(m_CurrentXmp.CiAdrCity);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case CONTACT_STATEPROVINCE :
			pEdit->SetWindowText(m_CurrentXmp.CiAdrRegion);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case CONTACT_POSTALCODE :
			pEdit->SetWindowText(m_CurrentXmp.CiAdrPcode);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case CONTACT_COUNTRY :
			pEdit->SetWindowText(m_CurrentXmp.CiAdrCtry);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case CONTACT_PHONE :
			pEdit->SetWindowText(m_CurrentXmp.CiTelWork);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case CONTACT_EMAIL :
			pEdit->SetWindowText(m_CurrentXmp.CiEmailWork);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case CONTACT_WEBSITE :
			pEdit->SetWindowText(m_CurrentXmp.CiUrlWork);
			pEdit->EnableWindow(bEnableIptc);
			break;

		// Content
		case CONTENT_HEADLINE :
			pEdit->SetWindowText(m_CurrentIptc.Headline);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case CONTENT_DESCRIPTION :
			pEdit->SetWindowText(m_CurrentIptc.Caption);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case CONTENT_KEYWORDS :
			for (i = 0 ; i < m_CurrentIptc.Keywords.GetSize() ; i++)
			{
				s += m_CurrentIptc.Keywords[i];
				if (i != m_CurrentIptc.Keywords.GetUpperBound())
					s += _T("\r\n");
			}
			pEdit->SetWindowText(s);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case CONTENT_IPTCSUBJECTCODE :
			for (i = 0 ; i < m_CurrentXmp.SubjectCode.GetSize() ; i++)
			{
				s += m_CurrentXmp.SubjectCode[i];
				if (i != m_CurrentXmp.SubjectCode.GetUpperBound())
					s += _T("\r\n");
			}
			pEdit->SetWindowText(s);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case CONTENT_CATEGORY :
			pEdit->SetWindowText(m_CurrentIptc.Category);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case CONTENT_SUPPLEMENTALCATEGORIES :
			for (i = 0 ; i < m_CurrentIptc.SupplementalCategories.GetSize() ; i++)
			{
				s += m_CurrentIptc.SupplementalCategories[i];
				if (i != m_CurrentIptc.SupplementalCategories.GetUpperBound())
					s += _T("\r\n");
			}
			pEdit->SetWindowText(s);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case CONTENT_DESCRIPTIONWRITER :
			pEdit->SetWindowText(m_CurrentIptc.CaptionWriter);
			pEdit->EnableWindow(bEnableIptc);
			break;

		// Image
		case IMAGE_DATECREATED :
		{
			if (m_CurrentIptc.DateCreated != _T(""))
			{
				if (m_CurrentIptc.DateCreated == m_OrigIptc.DateCreated &&
					m_OutOfSyncIptc.DateCreated == _T("1"))
				{
					CTime Time = CMetadata::GetDateFromIptcLegacyString(m_CurrentIptc.DateCreated.Left(8));
					s = ::MakeDateLocalFormat(Time);
					s += m_CurrentIptc.DateCreated.Mid(8, m_CurrentIptc.DateCreated.GetLength() - 8 - 8);
					Time = CMetadata::GetDateFromIptcLegacyString(m_CurrentIptc.DateCreated.Right(8));
					s += ::MakeDateLocalFormat(Time);
					pEdit->SetWindowText(s);
				}
				else
				{
					CTime Time = CMetadata::GetDateFromIptcLegacyString(m_CurrentIptc.DateCreated);
					pEdit->SetWindowText(::MakeDateLocalFormat(Time));
				}
			}
			else
				pEdit->SetWindowText(_T(""));
			pEdit->EnableWindow(bEnableIptc);
			break;
		}
		case IMAGE_INTELLECTUALGENRE :
			pEdit->SetWindowText(m_CurrentXmp.IntellectualGenre);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case IMAGE_IPTCSCENE :
			for (i = 0 ; i < m_CurrentXmp.Scene.GetSize() ; i++)
			{
				s += m_CurrentXmp.Scene[i];
				if (i != m_CurrentXmp.Scene.GetUpperBound())
					s += _T("\r\n");
			}
			pEdit->SetWindowText(s);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case IMAGE_LOCATION :
			pEdit->SetWindowText(m_CurrentXmp.Location);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case IMAGE_CITY :
			pEdit->SetWindowText(m_CurrentIptc.City);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case IMAGE_STATEPROVINCE :
			pEdit->SetWindowText(m_CurrentIptc.ProvinceState);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case IMAGE_COUNTRY :
			pEdit->SetWindowText(m_CurrentIptc.Country);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case IMAGE_ISOCOUNTRYCODE :
			pEdit->SetWindowText(m_CurrentXmp.CountryCode);
			pEdit->EnableWindow(bEnableIptc);
			break;
		
		// Status
		case STATUS_TITLE :
			pEdit->SetWindowText(m_CurrentIptc.ObjectName);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case STATUS_JOBIDENTIFIER :
			pEdit->SetWindowText(m_CurrentIptc.OriginalTransmissionReference);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case STATUS_INSTRUCTIONS :
			pEdit->SetWindowText(m_CurrentIptc.SpecialInstructions);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case STATUS_PROVIDER :
			pEdit->SetWindowText(m_CurrentIptc.Credits);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case STATUS_SOURCE :
			pEdit->SetWindowText(m_CurrentIptc.Source);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case STATUS_COPYRIGHTNOTICE :
			pEdit->SetWindowText(m_CurrentIptc.CopyrightNotice);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case STATUS_RIGHTSUSAGETERMS :
			pEdit->SetWindowText(m_CurrentXmp.UsageTerms);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case STATUS_COPYRIGHTURL :
			pEdit->SetWindowText(m_CurrentXmp.CopyrightUrl);
			pEdit->EnableWindow(bEnableIptc);
			break;
		case STATUS_COPYRIGHTSTATUS :
			if (m_CurrentXmp.CopyrightMarked == _T("True"))
				pEdit->SetWindowText(_T("Copyrighted"));
			else if (m_CurrentXmp.CopyrightMarked == _T("False"))
				pEdit->SetWindowText(_T("Public Domain"));
			else
				pEdit->SetWindowText(_T(""));
			pEdit->EnableWindow(bEnableIptc);
			break;
		case STATUS_URGENCY :
			pEdit->SetWindowText(m_CurrentIptc.Urgency);
			pEdit->EnableWindow(bEnableIptc);
			break;
		default :
			pEdit->SetWindowText(_T(""));
			pEdit->EnableWindow(FALSE);
			break;
	}
}

void CImageInfoDlg::GetEditMetadataText()
{
	int i;
	CString s, t;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_METADATA);

	switch (m_nMetadataType)
	{
		// File
		case FILE_COMMENT :
			pEdit->GetWindowText(m_sCurrentComment);
			break;

		// Contact
		case CONTACT_CREATOR :
			pEdit->GetWindowText(m_CurrentIptc.Byline);
			break;
		case CONTACT_CREATORSJOBTITLE :
			pEdit->GetWindowText(m_CurrentIptc.BylineTitle);
			break;
		case CONTACT_ADDRESS :
			pEdit->GetWindowText(m_CurrentXmp.CiAdrExtadr);
			break;
		case CONTACT_CITY :
			pEdit->GetWindowText(m_CurrentXmp.CiAdrCity);
			break;
		case CONTACT_STATEPROVINCE :
			pEdit->GetWindowText(m_CurrentXmp.CiAdrRegion);
			break;
		case CONTACT_POSTALCODE :
			pEdit->GetWindowText(m_CurrentXmp.CiAdrPcode);
			break;
		case CONTACT_COUNTRY :
			pEdit->GetWindowText(m_CurrentXmp.CiAdrCtry);
			break;
		case CONTACT_PHONE :
			pEdit->GetWindowText(m_CurrentXmp.CiTelWork);
			break;
		case CONTACT_EMAIL :
			pEdit->GetWindowText(m_CurrentXmp.CiEmailWork);
			break;
		case CONTACT_WEBSITE :
			pEdit->GetWindowText(m_CurrentXmp.CiUrlWork);
			break;

		// Content
		case CONTENT_HEADLINE :
			pEdit->GetWindowText(m_CurrentIptc.Headline);
			break;
		case CONTENT_DESCRIPTION :
			pEdit->GetWindowText(m_CurrentIptc.Caption);
			break;
		case CONTENT_KEYWORDS :
			m_CurrentIptc.Keywords.RemoveAll();
			pEdit->GetWindowText(s);
			while ((i = s.Find(_T("\r\n"))) >= 0)
			{
				t = s.Left(i);
				if (t != _T(""))
					m_CurrentIptc.Keywords.Add(t);
				s = s.Right(s.GetLength() - i - 2);
			}
			if (s != _T(""))
				m_CurrentIptc.Keywords.Add(s);
			break;
		case CONTENT_IPTCSUBJECTCODE :
			m_CurrentXmp.SubjectCode.RemoveAll();
			pEdit->GetWindowText(s);
			while ((i = s.Find(_T("\r\n"))) >= 0)
			{
				t = s.Left(i);
				if (t != _T(""))
					m_CurrentXmp.SubjectCode.Add(t);
				s = s.Right(s.GetLength() - i - 2);
			}
			if (s != _T(""))
				m_CurrentXmp.SubjectCode.Add(s);
			break;
		case CONTENT_CATEGORY :
			pEdit->GetWindowText(m_CurrentIptc.Category);
			break;
		case CONTENT_SUPPLEMENTALCATEGORIES :
			m_CurrentIptc.SupplementalCategories.RemoveAll();
			pEdit->GetWindowText(s);
			while ((i = s.Find(_T("\r\n"))) >= 0)
			{
				t = s.Left(i);
				if (t != _T(""))
					m_CurrentIptc.SupplementalCategories.Add(t);
				s = s.Right(s.GetLength() - i - 2);
			}
			if (s != _T(""))
				m_CurrentIptc.SupplementalCategories.Add(s);
			break;
		case CONTENT_DESCRIPTIONWRITER :
			pEdit->GetWindowText(m_CurrentIptc.CaptionWriter);
			break;

		// Image
		case IMAGE_DATECREATED :
		{
			pEdit->GetWindowText(s);
			if (s != _T(""))
			{
				CTime Time = ::ParseShortDateLocalFormat(s);
				t.Format(_T("%d%02d%02d"),	Time.GetYear(),
											Time.GetMonth(),
											Time.GetDay());
				m_CurrentIptc.DateCreated = t;
			}
			else
				m_CurrentIptc.DateCreated = _T("");
			break;
		}
		case IMAGE_INTELLECTUALGENRE :
			pEdit->GetWindowText(m_CurrentXmp.IntellectualGenre);
			break;
		case IMAGE_IPTCSCENE :
			m_CurrentXmp.Scene.RemoveAll();
			pEdit->GetWindowText(s);
			while ((i = s.Find(_T("\r\n"))) >= 0)
			{
				t = s.Left(i);
				if (t != _T(""))
					m_CurrentXmp.Scene.Add(t);
				s = s.Right(s.GetLength() - i - 2);
			}
			if (s != _T(""))
				m_CurrentXmp.Scene.Add(s);
			break;
		case IMAGE_LOCATION :
			pEdit->GetWindowText(m_CurrentXmp.Location);
			break;
		case IMAGE_CITY :
			pEdit->GetWindowText(m_CurrentIptc.City);
			break;
		case IMAGE_STATEPROVINCE :
			pEdit->GetWindowText(m_CurrentIptc.ProvinceState);
			break;
		case IMAGE_COUNTRY :
			pEdit->GetWindowText(m_CurrentIptc.Country);
			break;
		case IMAGE_ISOCOUNTRYCODE :
			pEdit->GetWindowText(m_CurrentXmp.CountryCode);
			break;

		// Status
		case STATUS_TITLE :
			pEdit->GetWindowText(m_CurrentIptc.ObjectName);
			break;
		case STATUS_JOBIDENTIFIER :
			pEdit->GetWindowText(m_CurrentIptc.OriginalTransmissionReference);
			break;
		case STATUS_INSTRUCTIONS :
			pEdit->GetWindowText(m_CurrentIptc.SpecialInstructions);
			break;
		case STATUS_PROVIDER :
			pEdit->GetWindowText(m_CurrentIptc.Credits);
			break;
		case STATUS_SOURCE :
			pEdit->GetWindowText(m_CurrentIptc.Source);
			break;
		case STATUS_COPYRIGHTNOTICE :
			pEdit->GetWindowText(m_CurrentIptc.CopyrightNotice);
			break;
		case STATUS_RIGHTSUSAGETERMS :
			pEdit->GetWindowText(m_CurrentXmp.UsageTerms);
			break;
		case STATUS_COPYRIGHTURL :
			pEdit->GetWindowText(m_CurrentXmp.CopyrightUrl);
			break;
		case STATUS_COPYRIGHTSTATUS :
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
				m_CurrentXmp.CopyrightMarked = s;
			}
			else
				m_CurrentXmp.CopyrightMarked = _T("");
			break;
		case STATUS_URGENCY :
			pEdit->GetWindowText(m_CurrentIptc.Urgency);
			break;
		default :
			break;
	}
}

void CImageInfoDlg::OnButtonSaveMetadata() 
{
	SaveMetadata();
}

BOOL CImageInfoDlg::SaveMetadata()
{
	// Reset Result Var
	BOOL res = FALSE;

	// Save Comment
	if (m_sOrigComment != m_sCurrentComment)
	{
		if (CDib::IsJPEG(m_pDoc->m_sFileName))
		{
			BeginWaitCursor();

			// Be Sure We Are Not Working On This File
			BOOL bWasRunning = FALSE;
			if (m_pDoc->m_JpegThread.IsRunning())
			{
				bWasRunning = TRUE;
				m_pDoc->m_JpegThread.Kill();
			}

			// Write Comment with UTF-8 Encoding
			LPBYTE pUTF8 = NULL;
			::ToUTF8(m_sCurrentComment, &pUTF8);
			res = CDib::JPEGWriteComment(	m_pDoc->m_sFileName,
											((CUImagerApp*)AfxGetApp())->GetAppTempDir(),		
											(LPCSTR)pUTF8,
											TRUE); // Show Message Box on error!
			if (pUTF8)
				delete [] pUTF8;
			
			// Update Info
			if (res)
			{
				m_pDoc->m_pDib->JPEGLoadMetadata(m_pDoc->m_sFileName);
				m_pDoc->UpdateImageInfo();
				if (bWasRunning && m_pDoc->m_bDoJPEGGet)	
					m_pDoc->JPEGGet();
				EndWaitCursor();
			}
			else
			{
				if (bWasRunning && m_pDoc->m_bDoJPEGGet)	
					m_pDoc->JPEGGet();
				EndWaitCursor();
				return FALSE;
			}
		}
		else if (::GetFileExtLower(m_pDoc->m_sFileName) == _T(".gif"))
		{
			BeginWaitCursor();

			// Write Comment to current shown frame in case of an animated gif
			int nFrame;
			if (m_pDoc->m_GifAnimationThread.IsAlive()					&&
				m_pDoc->m_GifAnimationThread.m_dwDibAnimationCount > 1	&&
				!m_pDoc->m_GifAnimationThread.IsRunning())
				nFrame = m_pDoc->m_GifAnimationThread.m_dwDibAnimationPos;
			else
				nFrame = 0;
			CString sOrigFileComment;
			if (m_pDoc->m_GifAnimationThread.m_DibAnimationArray.GetSize() > 0)
			{
				sOrigFileComment = m_pDoc->m_GifAnimationThread.m_DibAnimationArray[nFrame]->GetGif()->GetComment();
				m_pDoc->m_GifAnimationThread.m_DibAnimationArray[nFrame]->GetGif()->SetComment(m_sCurrentComment);
			}
			else
				sOrigFileComment = m_pDoc->m_pDib->GetGif()->GetComment();
			if (nFrame == 0)
				m_pDoc->m_pDib->GetGif()->SetComment(m_sCurrentComment);
			res = CDib::GIFWriteComment(nFrame,
										m_pDoc->m_sFileName,
										((CUImagerApp*)AfxGetApp())->GetAppTempDir(),		
										m_sCurrentComment,
										TRUE); // Show Message Box on error!
			
			// Update Info
			if (res)
			{
				m_pDoc->UpdateImageInfo();
				EndWaitCursor();
			}
			else
			{
				if (m_pDoc->m_GifAnimationThread.m_DibAnimationArray.GetSize() > 0)
					m_pDoc->m_GifAnimationThread.m_DibAnimationArray[nFrame]->GetGif()->SetComment(sOrigFileComment);
				if (nFrame == 0)
					m_pDoc->m_pDib->GetGif()->SetComment(sOrigFileComment);
				EndWaitCursor();
				return FALSE;
			}
		}
	}

	// Do not check m_pDoc->m_bMetadataModified, always save
	// Iptc Legacy & Xmp, because m_pDoc->m_bMetadataModified
	// may be cleared but the auto sync. of Keywords or Supplemental
	// Categories could have set the metadata edit box with the merged
	// Keywords or Supplemental Categories.
	if (CDib::IsJPEG(m_pDoc->m_sFileName))
	{
		BeginWaitCursor();

		// Be Sure We Are Not Working On This File
		BOOL bWasRunning = FALSE;
		if (m_pDoc->m_JpegThread.IsRunning())
		{
			bWasRunning = TRUE;
			m_pDoc->m_JpegThread.Kill();
		}

		// Prepare Iptc for Save
		SetIptc();

		// Write Iptc Legacy
		LPBYTE pData = NULL;
		int nSize = m_pDoc->m_pDib->GetMetadata()->MakeIptcLegacySection(&pData);
		if (nSize < 0)
		{
			if (pData)
				delete [] pData;
			::AfxMessageBox(ML_STRING(1393, "Could Not Save The Metadata."), MB_OK | MB_ICONSTOP);
			if (bWasRunning && m_pDoc->m_bDoJPEGGet)	
				m_pDoc->JPEGGet();
			EndWaitCursor();
			return FALSE;
		}
		res = CDib::JPEGWriteSection(	M_IPTC,
										NULL,
										0,
										m_pDoc->m_sFileName,
										((CUImagerApp*)AfxGetApp())->GetAppTempDir(),	
										nSize,
										pData,
										TRUE); // Show Message Box on error!

		// Free
		if (pData)
		{
			delete [] pData;
			pData = NULL;
		}

		// Write Xmp
		if (res)
		{
			nSize = m_pDoc->m_pDib->GetMetadata()->MakeXmpSection(&pData);
			if (nSize < 0)
			{
				if (pData)
					delete [] pData;
				::AfxMessageBox(ML_STRING(1393, "Could Not Save The Metadata."), MB_OK | MB_ICONSTOP);
				if (bWasRunning && m_pDoc->m_bDoJPEGGet)	
					m_pDoc->JPEGGet();
				EndWaitCursor();
				return FALSE;
			}
			res = CDib::JPEGWriteSection(	M_EXIF_XMP,
											CMetadata::m_XmpHeader,
											XMP_HEADER_SIZE,
											m_pDoc->m_sFileName,
											((CUImagerApp*)AfxGetApp())->GetAppTempDir(),	
											nSize,
											pData,
											TRUE); // Show Message Box on error!

			// Free
			if (pData)
			{
				delete [] pData;
				pData = NULL;
			}
		}
		
		// Update Info
		m_pDoc->m_pDib->JPEGLoadMetadata(m_pDoc->m_sFileName);
		if (res)
		{
			m_pDoc->UpdateImageInfo();
			if (bWasRunning && m_pDoc->m_bDoJPEGGet)	
				m_pDoc->JPEGGet();
			EndWaitCursor();
		}
		else
		{
			if (bWasRunning && m_pDoc->m_bDoJPEGGet)	
				m_pDoc->JPEGGet();
			EndWaitCursor();
			return FALSE;
		}
	}
	else if (CDib::IsTIFF(m_pDoc->m_sFileName))
	{
		BeginWaitCursor();

		// Prepare Iptc for Save
		SetIptc();

		// Update Iptc Data
		if (m_pDoc->m_pDib->GetMetadata()->m_pIptcLegacyData)
		{
			delete [] m_pDoc->m_pDib->GetMetadata()->m_pIptcLegacyData;
			m_pDoc->m_pDib->GetMetadata()->m_pIptcLegacyData = NULL;
		}
		m_pDoc->m_pDib->GetMetadata()->m_dwIptcLegacySize =
			m_pDoc->m_pDib->GetMetadata()->MakeIptcLegacyData(&(m_pDoc->m_pDib->GetMetadata()->m_pIptcLegacyData));
		
		// Update Xmp
		m_pDoc->m_pDib->GetMetadata()->UpdateXmpData(_T("image/tiff"));

		// Save Tiff Metadatas
		if ((m_pDoc->m_pDib->GetMetadata()->m_pIptcLegacyData &&
			m_pDoc->m_pDib->GetMetadata()->m_dwIptcLegacySize > 0)	||
			(m_pDoc->m_pDib->GetMetadata()->m_pXmpData &&
			m_pDoc->m_pDib->GetMetadata()->m_dwXmpSize > 0))
		{	
			BOOL bOldMessageboxShowState = m_pDoc->m_pDib->IsShowMessageBoxOnError();
			m_pDoc->m_pDib->SetShowMessageBoxOnError(TRUE);
			res = m_pDoc->m_pDib->TIFFWriteMetadata(m_pDoc->m_sFileName,
													((CUImagerApp*)AfxGetApp())->GetAppTempDir());
			m_pDoc->m_pDib->SetShowMessageBoxOnError(bOldMessageboxShowState);
		}

		// Update Info
		CDib Dib;
		if (Dib.LoadTIFF(	m_pDoc->m_sFileName,	// File Name
							m_pDoc->m_nPageNum,		// Load the given page
							TRUE))					// Only Header & Metadatas
			*(m_pDoc->m_pDib->GetMetadata()) = *(Dib.GetMetadata());
		else
		{
			EndWaitCursor();
			return FALSE;
		}
		if (res)
		{
			m_pDoc->UpdateImageInfo();
			EndWaitCursor();
		}
		else
		{
			EndWaitCursor();
			return FALSE;
		}
	}

	// Reset Metadata Modified Flag
	m_pDoc->m_bMetadataModified = FALSE;
	
	// Update Dialog Title
	UpdateDlgTitle();

	return TRUE;
}

void CImageInfoDlg::OnRadioMetadata() 
{
	UpdateData(TRUE);

	// Display Metadata Text
	DisplayMetadata();	
}

void CImageInfoDlg::OnRadioExif() 
{
	UpdateData(TRUE);

	// Display Metadata Text
	DisplayMetadata();
}

void CImageInfoDlg::OnRadioIptcLegacy() 
{
	UpdateData(TRUE);

	// Display Metadata Text
	DisplayMetadata();
}

void CImageInfoDlg::OnRadioXmp() 
{
	UpdateData(TRUE);

	// Display Metadata Text
	DisplayMetadata();
}

void CImageInfoDlg::OnRadioIcc() 
{
	UpdateData(TRUE);

	// Display Metadata Text
	DisplayMetadata();
}

void CImageInfoDlg::OnButtonImportMetadata() 
{
	// Check whether file is Modified
	if (m_pDoc->IsModified())
	{
		// Message Box
		::AfxMessageBox(ML_STRING(1391, "Please save the image before proceeding\n") +
						ML_STRING(1394, "with the Metadata importation."),
						MB_OK | MB_ICONEXCLAMATION);
	}
	// Call Xmp Import Dialog
	else 
	{
		CString sExiDateTime(m_pDoc->m_pDib->GetExifInfo()->DateTime);
		if (sExiDateTime != _T(""))
		{
			((CUImagerApp*)::AfxGetApp())->m_pXmpImportDlg->SetExifDateCreated(
				CMetadata::GetDateTimeFromExifString(sExiDateTime));
		}
		else
			((CUImagerApp*)::AfxGetApp())->m_pXmpImportDlg->ClearExifDateCreated();
		if (((CUImagerApp*)::AfxGetApp())->m_pXmpImportDlg->DoModal() == IDOK)
		{
			// Import Xmp
			((CUImagerApp*)::AfxGetApp())->m_pXmpImportDlg->GetFields(m_CurrentIptc, m_CurrentXmp);

			// Set Edit Metadata Text
			SetEditMetadataText();

			// Update Metadata Modified Flag
			UpdateMetadataModifiedFlag();
		}
	}
}

void CImageInfoDlg::OnButtonExportMetadata() 
{
	// Check whether file is Modified
	if (m_pDoc->IsModified())
	{
		// Message Box
		::AfxMessageBox(ML_STRING(1391, "Please save the image before proceeding\n") +
						ML_STRING(1395, "with the Metadata exportation."),
						MB_OK | MB_ICONEXCLAMATION);
	}
	else if (m_pDoc->m_bMetadataModified)
	{
		// Message Box
		::AfxMessageBox(ML_STRING(1396, "Please save the metadata before proceeding\n") +
						ML_STRING(1395, "with the Metadata exportation."),
						MB_OK | MB_ICONEXCLAMATION);
	}
	else
	{
		TCHAR* InitDir = new TCHAR[MAX_FILEDLG_PATH];
		InitDir[0] = _T('\0');
		CString sXmpLastSavedDir = ::GetDriveAndDirName(((CUImagerApp*)::AfxGetApp())->m_pXmpImportDlg->m_sXmpSaveFile);
		sXmpLastSavedDir.TrimRight(_T('\\'));
		if (::IsExistingDir(sXmpLastSavedDir))
			_tcscpy(InitDir, (LPCTSTR)sXmpLastSavedDir);
		CString sXmpFileName = ::GetShortFileNameNoExt(m_pDoc->m_sFileName) + _T(".xmp");
		CNoVistaFileDlg fd(	FALSE,
							_T("xmp"),
							sXmpFileName,
							OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
							_T("Xmp Files (*.xmp)|*.xmp||"));
		fd.m_ofn.lpstrInitialDir = InitDir;
		if (fd.DoModal() == IDOK)
		{
			// Set File Name
			((CUImagerApp*)::AfxGetApp())->m_pXmpImportDlg->m_sXmpSaveFile = fd.GetPathName();

			// Export Xmp
			ExportXmp(((CUImagerApp*)::AfxGetApp())->m_pXmpImportDlg->m_sXmpSaveFile);
		}

		// Free
		delete [] InitDir;
	}
}

/*
While UTF-8 does not have byte order issues,
a BOM encoded in UTF-8 may be used to mark text as UTF-8.
Quite a lot of Windows software (including Windows Notepad)
adds one to UTF-8 files.
Representations of byte order marks (BOM) U+FEFF by encoding:
UTF-8					EF BB BF (=ï»¿)
UTF-16 Big Endian		FE FF
UTF-16 Little Endian	FF FE
UTF-32 Big Endian		00 00 FE FF
UTF-32 Little Endian	FF FE 00 00

XMP Specific:
Attribute: begin
For backward compatibility with earlier versions of the XMP Packet
specification, the value of this attribute can be the empty string,
indicating UTF-8 encoding.
*/
BOOL CImageInfoDlg::ExportXmp(LPCTSTR lpszFileName)
{
	if (m_pDoc->m_pDib->GetMetadata()->m_pXmpData && 
		m_pDoc->m_pDib->GetMetadata()->m_dwXmpSize > 0)
	{
		return ExportXmp(	lpszFileName,
							m_pDoc->m_pDib->GetMetadata()->m_pXmpData,
							m_pDoc->m_pDib->GetMetadata()->m_dwXmpSize);
	}
	else
	{
		CMetadata Metadata;
		Metadata.m_IptcFromXmpInfo = m_CurrentIptc;
		if (CDib::IsTIFF(m_pDoc->m_sFileName))
		{
			if (!Metadata.UpdateXmpData(_T("image/tiff")))
				return FALSE;
		}
		else if (CDib::IsJPEG(m_pDoc->m_sFileName))
		{
			if (!Metadata.UpdateXmpData(_T("image/jpeg")))
				return FALSE;
		}
		else
			return FALSE;
		return ExportXmp(	lpszFileName,
							Metadata.m_pXmpData,
							Metadata.m_dwXmpSize);
	}
}

BOOL CImageInfoDlg::ExportXmp(LPCTSTR lpszFileName, LPBYTE pXmpData, DWORD dwXmpSize)
{
	CString s = _T("");
	CString t;

	if (pXmpData && dwXmpSize > 0)
	{
		// Parse Xml
		CString sXml = ::FromUTF8(	(const unsigned char*)pXmpData,
									(int)dwXmpSize);

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

void CImageInfoDlg::DisplayMetadata()
{
	CString s = _T("");
	CString t;

	if (m_nMetadataGroupView == OTHERS)
	{
		if (m_pDoc->m_pDib->GetExifInfo()->JpegSOFnMarker)
		{
			t.Format(ML_STRING(1659, "Jpeg encoding:\t")); s+=t;
			switch (m_pDoc->m_pDib->GetExifInfo()->JpegSOFnMarker)
			{
				case 0xC0: //M_SOF0
					t.Format(_T("baseline\r\n")); s+=t;
					break;
				case 0xC1: //M_SOF1
					t.Format(_T("extended sequential\r\n")); s+=t;
					break;
				case 0xC2: //M_SOF2
					t.Format(_T("progressive\r\n")); s+=t;
					break;
				case 0xC3: //M_SOF3
					t.Format(_T("lossless\r\n")); s+=t;
					break;
				case 0xC5: //M_SOF5
					t.Format(_T("differential sequential\r\n")); s+=t;
					break;
				case 0xC6: //M_SOF6
					t.Format(_T("differential progressive\r\n")); s+=t;
					break;
				case 0xC7: //M_SOF7
					t.Format(_T("differential lossless\r\n")); s+=t;
					break;
				case 0xC9: //M_SOF9
					t.Format(_T("extended sequential, arithmetic coding\r\n")); s+=t;
					break;
				case 0xCA: //M_SOF10
					t.Format(_T("progressive, arithmetic coding\r\n")); s+=t;
					break;
				case 0xCB: //M_SOF11
					t.Format(_T("lossless, arithmetic coding\r\n")); s+=t;
					break;
				case 0xCD: //M_SOF13
					t.Format(_T("differential sequential, arithmetic coding\r\n")); s+=t;
					break;
				case 0xCE: //M_SOF14
					t.Format(_T("differential progressive, arithmetic coding\r\n")); s+=t;
					break;
				case 0xCF: //M_SOF0
					t.Format(_T("differential lossless, arithmetic coding\r\n")); s+=t;
					break;
				default:
					t.Format(_T("unknown\r\n")); s+=t;
					break;
			}
		}

		if (m_sOrigComment != _T(""))
		{
			t.Format(ML_STRING(1656, "%s comment:\t%s"),
				::GetFileExtLower(m_pDoc->m_sFileName) == _T(".gif") ? _T("Gif") : _T("Jpeg"),
				m_sOrigComment);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}
	}
	else if (m_nMetadataGroupView == EXIF)
	{
		if (m_pDoc->m_pDib->GetExifInfo()->CameraMake[0])
		{
			t.Format(ML_STRING(1657, "Camera make:\t%s"), CString(m_pDoc->m_pDib->GetExifInfo()->CameraMake).Trim());
			::MakeLineBreakCRLF(t);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}
		
		if (m_pDoc->m_pDib->GetExifInfo()->CameraModel[0])
		{
			t.Format(ML_STRING(1658, "Camera model:\t%s"), CString(m_pDoc->m_pDib->GetExifInfo()->CameraModel).Trim());
			::MakeLineBreakCRLF(t);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetExifInfo()->ImageDescription[0])
		{
			t.Format(ML_STRING(1660, "Image description:\t%s"), CString(m_pDoc->m_pDib->GetExifInfo()->ImageDescription).Trim());
			::MakeLineBreakCRLF(t);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetExifInfo()->UserComment[0])
		{
			t.Format(ML_STRING(1661, "User comment:\t%s"), CString(m_pDoc->m_pDib->GetExifInfo()->UserComment).Trim());
			::MakeLineBreakCRLF(t);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetExifInfo()->Software[0])
		{
			t.Format(ML_STRING(1662, "Software:\t%s"), CString(m_pDoc->m_pDib->GetExifInfo()->Software).Trim());
			::MakeLineBreakCRLF(t);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetExifInfo()->Artist[0])
		{
			t.Format(ML_STRING(1663, "Artist:\t%s"), CString(m_pDoc->m_pDib->GetExifInfo()->Artist).Trim());
			::MakeLineBreakCRLF(t);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetExifInfo()->CopyrightPhotographer[0])
		{
			CString sCopyrightPhotographer = CString(m_pDoc->m_pDib->GetExifInfo()->CopyrightPhotographer).Trim();
			if (sCopyrightPhotographer != _T(""))
			{
				t.Format(ML_STRING(1664, "Photographer©:\t%s"), sCopyrightPhotographer);
				::MakeLineBreakCRLF(t);
				t.Replace(_T("\n"), _T("\n\t"));
				t += _T("\r\n");
				s+=t;
			}
		}

		if (m_pDoc->m_pDib->GetExifInfo()->CopyrightEditor[0])
		{
			CString sCopyrightEditor = CString(m_pDoc->m_pDib->GetExifInfo()->CopyrightEditor).Trim();
			if (sCopyrightEditor != _T(""))
			{
				t.Format(ML_STRING(1665, "Editor©:\t%s"), sCopyrightEditor);
				::MakeLineBreakCRLF(t);
				t.Replace(_T("\n"), _T("\n\t"));
				t += _T("\r\n");
				s+=t;
			}
		}
		
#ifdef _DEBUG
		if (m_pDoc->m_pDib->GetExifInfo()->Version[0])
		{
			t.Format(_T("Exif version:\t%s\r\n"), CString(m_pDoc->m_pDib->GetExifInfo()->Version).Trim()); s+=t;
		}
#endif

		if (m_pDoc->m_pDib->GetExifInfo()->DateTime[0])
		{
			CTime Time = CMetadata::GetDateTimeFromExifString(CString(m_pDoc->m_pDib->GetExifInfo()->DateTime));
			CString sTime = ::MakeDateLocalFormat(Time, TRUE) + _T(" ") + ::MakeTimeLocalFormat(Time, TRUE);
			t.Format(ML_STRING(1666, "Taken on:\t%s\r\n"), sTime); s+=t;
		}
		
		/*
		Default = 1
		1 = The 0th row is at the visual top of the image, and the 0th column is the visual left-hand side.
		2 = The 0th row is at the visual top of the image, and the 0th column is the visual right-hand side.
		3 = The 0th row is at the visual bottom of the image, and the 0th column is the visual right-hand
		side.
		4 = The 0th row is at the visual bottom of the image, and the 0th column is the visual left-hand
		side.
		5 = The 0th row is the visual left-hand side of of the image, and the 0th column is the visual top.
		6 = The 0th row is the visual right -hand side of of the image, and the 0th column is the visual top.
		7 = The 0th row is the visual right -hand side of of the image, and the 0th column is the visual
		bottom.
		8 = The 0th row is the visual left-hand side of of the image, and the 0th column is the visual
		bottom.
		Other = reserved
		*/
		if (m_pDoc->m_pDib->GetExifInfo()->Orientation > 1)
		{
			t.Format(ML_STRING(1667, "Orientation:\t%s\r\n"), CMetadata::m_OrientationTab[m_pDoc->m_pDib->GetExifInfo()->Orientation]); s+=t;
		}

		if (m_pDoc->m_pDib->GetExifInfo()->Flash >= 0)
		{
			if (m_pDoc->m_pDib->GetExifInfo()->Flash & 1)
			{
				t.Format(ML_STRING(1668, "Flash used:\tyes")); s+=t;
				switch (m_pDoc->m_pDib->GetExifInfo()->Flash)
				{
					case 0x5: t.Format(_T(" (Strobe light not detected)")); s+=t; break;
					case 0x7: t.Format(_T(" (Strobe light detected) ")); s+=t; break;
					case 0x9: t.Format(_T(" (manual)")); s+=t; break;
					case 0xd: t.Format(_T(" (manual, return light not detected)")); s+=t; break;
					case 0xf: t.Format(_T(" (maual, return light  detected)")); s+=t; break;
					case 0x19:t.Format(_T(" (auto)")); s+=t; break;
					case 0x1d:t.Format(_T(" (auto, return light not detected)")); s+=t; break;
					case 0x1f:t.Format(_T(" (auto, return light detected)")); s+=t; break;
					case 0x41:t.Format(_T(" (red eye reduction mode)")); s+=t; break;
					case 0x45:t.Format(_T(" (red eye reduction mode return light not detected)")); s+=t; break;
					case 0x47:t.Format(_T(" (red eye reduction mode return light detected)")); s+=t; break;
					case 0x49:t.Format(_T(" (manual, red eye reduction mode)")); s+=t; break;
					case 0x4d:t.Format(_T(" (manual, red eye reduction mode, return light not detected)")); s+=t; break;
					case 0x4f:t.Format(_T(" (red eye reduction mode, red eye reduction mode, return light detected)")); s+=t; break;
					case 0x59:t.Format(_T(" (auto, red eye reduction mode)")); s+=t; break;
					case 0x5d:t.Format(_T(" (auto, red eye reduction mode, return light not detected)")); s+=t; break;
					case 0x5f:t.Format(_T(" (auto, red eye reduction mode, return light detected)")); s+=t; break;
					default: break;
				}
			}
			else
			{
				t.Format(ML_STRING(1669, "Flash used:\tno")); s+=t;
				switch (m_pDoc->m_pDib->GetExifInfo()->Flash)
				{
					case 0x18:t.Format(_T(" (auto)")); s+=t; break;
					default: break;
				}
			}
			t.Format(_T("\r\n")); s+=t;
		}
		
		if (m_pDoc->m_pDib->GetExifInfo()->FocalLength)
		{
			t.Format(ML_STRING(1670, "Focal length:\t%.1fmm"), (double)m_pDoc->m_pDib->GetExifInfo()->FocalLength); s+=t;
			if (m_pDoc->m_pDib->GetExifInfo()->FocalLength35mmEquiv)
			{
				t.Format(_T(" ") + ML_STRING(1671, "(35mm equivalent: %dmm)"), m_pDoc->m_pDib->GetExifInfo()->FocalLength35mmEquiv);
				s+=t;
			}
			t.Format(_T("\r\n")); s+=t;
		}

		if (m_pDoc->m_pDib->GetExifInfo()->DigitalZoomRatio > 1.0f)
		{
			t.Format(ML_STRING(1672, "Digital zoom:\t%.3fx\r\n"), (double)m_pDoc->m_pDib->GetExifInfo()->DigitalZoomRatio); s+=t;
		}
		
		if (m_pDoc->m_pDib->GetExifInfo()->ExposureTime)
		{
			if (m_pDoc->m_pDib->GetExifInfo()->ExposureTime < 0.010f)
			{
				t.Format(ML_STRING(1673, "Exposure time:\t%.4f s"), (double)m_pDoc->m_pDib->GetExifInfo()->ExposureTime); s+=t;
			}
			else
			{
				t.Format(ML_STRING(1674, "Exposure time:\t%.3f s"), (double)m_pDoc->m_pDib->GetExifInfo()->ExposureTime); s+=t;
			}
			if (m_pDoc->m_pDib->GetExifInfo()->ExposureTime <= 0.5f)
			{
				t.Format(_T(" (1/%d)"), Round(1.0 / m_pDoc->m_pDib->GetExifInfo()->ExposureTime)); s+=t;
			}
			t.Format(_T("\r\n")); s+=t;
		}
		
		if (m_pDoc->m_pDib->GetExifInfo()->Brightness)
		{
			t.Format(ML_STRING(1675, "Brightness:\t%.3f\r\n"), (double)m_pDoc->m_pDib->GetExifInfo()->Brightness); s+=t;
		}
		
		if (m_pDoc->m_pDib->GetExifInfo()->ApertureFNumber)
		{
			t.Format(ML_STRING(1676, "Aperture:\tf/%.1f\r\n"), (double)m_pDoc->m_pDib->GetExifInfo()->ApertureFNumber); s+=t;
		}
		
		if (m_pDoc->m_pDib->GetExifInfo()->Distance)
		{
			if (m_pDoc->m_pDib->GetExifInfo()->Distance < 0.0f)
			{
				t.Format(ML_STRING(1677, "Focus dist.:\tinfinite\r\n")); s+=t;
			}
			else
			{
				t.Format(ML_STRING(1678, "Focus dist.:\t%.2fm\r\n"), (double)m_pDoc->m_pDib->GetExifInfo()->Distance); s+=t;
			}
		}
		
		if (m_pDoc->m_pDib->GetExifInfo()->ISOequivalent)
		{
			t.Format(ML_STRING(1679, "ISO equiv.:\t%2d\r\n"), (int)m_pDoc->m_pDib->GetExifInfo()->ISOequivalent); s+=t;
		}
		
		if (m_pDoc->m_pDib->GetExifInfo()->ExposureBias)
		{
			t.Format(ML_STRING(1680, "Exposure bias:\t%.2f\r\n"), (double)m_pDoc->m_pDib->GetExifInfo()->ExposureBias); s+=t;
		}
		
		switch (m_pDoc->m_pDib->GetExifInfo()->WhiteBalance)
		{
			case 0:
				t.Format(ML_STRING(1681, "White balance:\tauto\r\n")); s+=t;
				break;
			case 1:
				t.Format(ML_STRING(1682, "White balance:\tmanual\r\n")); s+=t;
				break;
			default:
				break;
		}
		
		switch (m_pDoc->m_pDib->GetExifInfo()->LightSource)
		{
			case 1:
				t.Format(ML_STRING(1683, "Light source:\tdaylight\r\n")); s+=t;
				break;
			case 2:
				t.Format(ML_STRING(1684, "Light source:\tfluorescent\r\n")); s+=t;
				break;
			case 3:
				t.Format(ML_STRING(1685, "Light source:\tincandescent\r\n")); s+=t;
				break;
			case 4:
				t.Format(ML_STRING(1686, "Light source:\tflash\r\n")); s+=t;
				break;
			case 9:
				t.Format(ML_STRING(1687, "Light source:\tfine weather\r\n")); s+=t;
				break;
			case 11:
				t.Format(ML_STRING(1688, "Light source:\tshade\r\n")); s+=t;
				break;
			default:
				break; 
				// There are many more modes for this, check Exif2.2 specs
				// If it just says 'unknown' or we don't know it, then
				// don't bother showing it - it doesn't add any useful information.
		}
		
		if (m_pDoc->m_pDib->GetExifInfo()->MeteringMode)
		{
			switch(m_pDoc->m_pDib->GetExifInfo()->MeteringMode)
			{
				case 2:
					t.Format(ML_STRING(1689, "Metering mode:\tcenter weight\r\n")); s+=t;
					break;
				case 3:
					t.Format(ML_STRING(1690, "Metering mode:\tspot\r\n")); s+=t;
					break;
				case 5:
					t.Format(ML_STRING(1691, "Metering mode:\tmatrix\r\n")); s+=t;
					break;
				default:
					break;
			}
		}
		
		if (m_pDoc->m_pDib->GetExifInfo()->ExposureProgram)
		{
			switch(m_pDoc->m_pDib->GetExifInfo()->ExposureProgram)
			{
				case 1:
					t.Format(ML_STRING(1692, "Exposure program:\tmanual\r\n")); s+=t;
					break;
				case 2:
					t.Format(ML_STRING(1693, "Exposure program:\tprogram (auto)\r\n")); s+=t;
					break;
				case 3:
					t.Format(ML_STRING(1694, "Exposure program:\taperture priority (semi-auto)\r\n")); s+=t;
					break;
				case 4:
					t.Format(ML_STRING(1695, "Exposure program:\tshutter priority (semi-auto)\r\n")); s+=t;
					break;
				case 5:
					t.Format(ML_STRING(1696, "Exposure program:\tcreative program (based towards depth of field)\r\n")); s+=t;
					break;
				case 6:
					t.Format(ML_STRING(1697, "Exposure program:\taction program (based towards fast shutter speed)\r\n")); s+=t;
					break;
				case 7:
					t.Format(ML_STRING(1698, "Exposure program:\tportrait mode\r\n")); s+=t;
					break;
				case 8:
					t.Format(ML_STRING(1699, "Exposure program:\tlandscape mode\r\n")); s+=t;
					break;
				default:
					break;
			}
		}

		switch (m_pDoc->m_pDib->GetExifInfo()->ExposureMode)
		{
			case 0: // Automatic (not worth cluttering up output for)
					break;
			case 1: t.Format(ML_STRING(1700, "Exposure mode:\tmanual\r\n")); s+=t;
					break;
			case 2: t.Format(ML_STRING(1701, "Exposure mode:\tauto bracketing\r\n")); s+=t;
					break;
			default:
					break;
		}
		
		if (m_pDoc->m_pDib->GetExifInfo()->DistanceRange)
		{
			switch (m_pDoc->m_pDib->GetExifInfo()->DistanceRange)
			{
				case 1:
					t.Format(ML_STRING(1702, "Focus range:\tmacro\r\n")); s+=t;
					break;
				case 2:
					t.Format(ML_STRING(1703, "Focus range:\tclose\r\n")); s+=t;
					break;
				case 3:
					t.Format(ML_STRING(1704, "Focus range:\tdistant\r\n")); s+=t;
					break;
				default:
					break;
			}
		}

		if (m_pDoc->m_pDib->GetExifInfo()->CompressionLevel)
		{
			switch((int)(m_pDoc->m_pDib->GetExifInfo()->CompressionLevel + 0.5f))
			{
				case 1:
					t.Format(ML_STRING(1705, "Jpeg quality:\tbasic\r\n")); s+=t;
					break;
				case 2:
					t.Format(ML_STRING(1706, "Jpeg quality:\tnormal\r\n")); s+=t;
					break;
				case 3:
				case 4:
					t.Format(ML_STRING(1707, "Jpeg quality:\tfine\r\n")); s+=t;
					break;
				default:
					break;
		   }
		}

		if (!isnan(m_pDoc->m_pDib->GetExifInfo()->AmbientTemperature))
		{
			t.Format(ML_STRING(1712, "Temperature:\t%.1f°C\r\n"), m_pDoc->m_pDib->GetExifInfo()->AmbientTemperature);
			s+=t;
		}

		if (!isnan(m_pDoc->m_pDib->GetExifInfo()->Humidity))
		{
			t.Format(ML_STRING(1713, "Humidity:\t%.0f%%\r\n"), m_pDoc->m_pDib->GetExifInfo()->Humidity);
			s+=t;
		}

		if (!isnan(m_pDoc->m_pDib->GetExifInfo()->Pressure))
		{
			t.Format(ML_STRING(1714, "Pressure:\t%.0fmbar"), m_pDoc->m_pDib->GetExifInfo()->Pressure); s+=t;
			if (isnan(m_pDoc->m_pDib->GetExifInfo()->WaterDepth) || m_pDoc->m_pDib->GetExifInfo()->WaterDepth <= 0.0)
			{
				// CRC Handbook (see doc\PressureAltitude_Derived.pdf)
				double dPressurePa = 100.0 * (double)MIN(m_pDoc->m_pDib->GetExifInfo()->Pressure, 1013.25f);
				double dCalcAltitudeMeter = 44331.5 - 4946.62 * pow(dPressurePa, 0.190263);
				t.Format(_T(" (~%dm)"), Round(dCalcAltitudeMeter)); s+=t;
			}
			s+=_T("\r\n");
		}

		if (!isnan(m_pDoc->m_pDib->GetExifInfo()->WaterDepth))
		{
			t.Format(ML_STRING(1715, "Water depth:\t%.1fm\r\n"),
				m_pDoc->m_pDib->GetExifInfo()->WaterDepth == 0.0f ?
				0.0f : // avoid ugly -0.0m display
				m_pDoc->m_pDib->GetExifInfo()->WaterDepth);
			s+=t;
		}

		// Gps
		if (m_pDoc->m_pDib->GetExifInfo()->bGpsInfoPresent)
		{
			CString sGps;
			if (m_pDoc->m_pDib->GetExifInfo()->GpsLat[GPS_DEGREE] >= 0.0f	&&
				m_pDoc->m_pDib->GetExifInfo()->GpsLat[GPS_MINUTES] >= 0.0f	&&
				m_pDoc->m_pDib->GetExifInfo()->GpsLat[GPS_SECONDS] >= 0.0f	&&
				m_pDoc->m_pDib->GetExifInfo()->GpsLatRef[0])
			{
				t.Format(_T("%s %.0f° %.0f\' %.1f\""),
					CString(m_pDoc->m_pDib->GetExifInfo()->GpsLatRef),
					m_pDoc->m_pDib->GetExifInfo()->GpsLat[GPS_DEGREE],
					m_pDoc->m_pDib->GetExifInfo()->GpsLat[GPS_MINUTES],
					m_pDoc->m_pDib->GetExifInfo()->GpsLat[GPS_SECONDS]);
				sGps += t + _T("  ");
			}
			if (m_pDoc->m_pDib->GetExifInfo()->GpsLong[GPS_DEGREE] >= 0.0f	&&
				m_pDoc->m_pDib->GetExifInfo()->GpsLong[GPS_MINUTES] >= 0.0f	&&
				m_pDoc->m_pDib->GetExifInfo()->GpsLong[GPS_SECONDS] >= 0.0f &&
				m_pDoc->m_pDib->GetExifInfo()->GpsLongRef[0])
			{
				t.Format(_T("%s %.0f° %.0f\' %.1f\""),
					CString(m_pDoc->m_pDib->GetExifInfo()->GpsLongRef),
					m_pDoc->m_pDib->GetExifInfo()->GpsLong[GPS_DEGREE],
					m_pDoc->m_pDib->GetExifInfo()->GpsLong[GPS_MINUTES],
					m_pDoc->m_pDib->GetExifInfo()->GpsLong[GPS_SECONDS]);
				sGps += t + _T("  ");
			}
			if (m_pDoc->m_pDib->GetExifInfo()->GpsAlt >= 0.0f &&
				m_pDoc->m_pDib->GetExifInfo()->GpsAltRef >= 0)
			{
				if (m_pDoc->m_pDib->GetExifInfo()->GpsAltRef == 1)		// Below Sea Level
					t.Format(_T("-%.1fm"), m_pDoc->m_pDib->GetExifInfo()->GpsAlt);
				else													// Above Sea Level
					t.Format(_T("%.1fm"), m_pDoc->m_pDib->GetExifInfo()->GpsAlt);
				sGps += t + _T("  ");
			}
			if (m_pDoc->m_pDib->GetExifInfo()->GpsTime[GPS_HOUR] >= 0.0f	&&
				m_pDoc->m_pDib->GetExifInfo()->GpsTime[GPS_MINUTES] >= 0.0f	&&
				m_pDoc->m_pDib->GetExifInfo()->GpsTime[GPS_SECONDS] >= 0.0f)
			{
				t.Format(_T("%02.0f:%02.0f:%02.0f (UTC)"),
					m_pDoc->m_pDib->GetExifInfo()->GpsTime[GPS_HOUR],
					m_pDoc->m_pDib->GetExifInfo()->GpsTime[GPS_MINUTES],
					m_pDoc->m_pDib->GetExifInfo()->GpsTime[GPS_SECONDS]);
				sGps += t + _T("  ");
			}
			if (!sGps.IsEmpty())
			{
				sGps.TrimRight(_T(' '));
				t.Format(_T("GPS:\t%s\r\n"), sGps);
				s+=t;
			}
		}

		// Thumbnail
		if (m_pDoc->m_pDib->GetThumbnailDib() &&
			m_pDoc->m_pDib->GetExifInfo()->ThumbnailPointer &&
			(m_pDoc->m_pDib->GetExifInfo()->ThumbnailSize > 0))
		{
			if (m_pDoc->m_pDib->GetExifInfo()->ThumbnailCompression == 6)		// JPEG
			{
				t.Format(ML_STRING(1708, "Jpeg thumbnail:\t%ix%i\r\n"),
					m_pDoc->m_pDib->GetThumbnailDib()->GetWidth(),
					m_pDoc->m_pDib->GetThumbnailDib()->GetHeight()); s+=t;
			}
			else if (m_pDoc->m_pDib->GetExifInfo()->ThumbnailCompression == 1)	// TIFF
			{
				if (m_pDoc->m_pDib->GetExifInfo()->ThumbnailPhotometricInterpretation == 2)			// RGB Format
				{
					t.Format(ML_STRING(1709, "RGB thumbnail:\t%ix%i\r\n"),
						m_pDoc->m_pDib->GetThumbnailDib()->GetWidth(),
						m_pDoc->m_pDib->GetThumbnailDib()->GetHeight()); s+=t;
				}
				else if (m_pDoc->m_pDib->GetExifInfo()->ThumbnailPhotometricInterpretation == 6)	// YCbCr Format
				{
					t.Format(ML_STRING(1710, "YCbCr thumbnail:\t%ix%i\r\n"),
						m_pDoc->m_pDib->GetThumbnailDib()->GetWidth(),
						m_pDoc->m_pDib->GetThumbnailDib()->GetHeight()); s+=t;
				}
			}
		}
	}
	// Iptc From Xmp
	else if (m_nMetadataGroupView == XMP)
	{
		// Creator Contact Info
		t = _T("[") + ML_STRING(1577, "Contact") + _T("]\r\n");
		s+=t;
		
		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->Byline != _T(""))
		{
			t.Format(ML_STRING(1621, "Creator:\t%s"),
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->Byline);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->BylineTitle != _T(""))
		{
			t.Format(ML_STRING(1622, "Creator's job title:\t%s"),
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->BylineTitle);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}
		if (m_pDoc->m_pDib->GetXmpInfo()->CiAdrExtadr != _T(""))
		{
			t.Format(ML_STRING(1623, "Address:\t%s"),
					m_pDoc->m_pDib->GetXmpInfo()->CiAdrExtadr);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetXmpInfo()->CiAdrCity != _T(""))
		{
			t.Format(ML_STRING(1624, "City:\t%s"),
					m_pDoc->m_pDib->GetXmpInfo()->CiAdrCity);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetXmpInfo()->CiAdrRegion != _T(""))
		{
			t.Format(ML_STRING(1625, "State/Province:\t%s"),
					m_pDoc->m_pDib->GetXmpInfo()->CiAdrRegion);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetXmpInfo()->CiAdrPcode != _T(""))
		{
			t.Format(ML_STRING(1626, "Postal code:\t%s"),
					m_pDoc->m_pDib->GetXmpInfo()->CiAdrPcode);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetXmpInfo()->CiAdrCtry != _T(""))
		{
			t.Format(ML_STRING(1627, "Country:\t%s"),
					m_pDoc->m_pDib->GetXmpInfo()->CiAdrCtry);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetXmpInfo()->CiTelWork != _T(""))
		{
			t.Format(ML_STRING(1628, "Phone(s):\t%s"),
					m_pDoc->m_pDib->GetXmpInfo()->CiTelWork);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetXmpInfo()->CiEmailWork != _T(""))
		{
			t.Format(ML_STRING(1629, "E-Mail(s):\t%s"),
					m_pDoc->m_pDib->GetXmpInfo()->CiEmailWork);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetXmpInfo()->CiUrlWork != _T(""))
		{
			t.Format(ML_STRING(1630, "Website(s):\t%s"),
					m_pDoc->m_pDib->GetXmpInfo()->CiUrlWork);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		// Content
		t = _T("\r\n[") + ML_STRING(1578, "Content") + _T("]\r\n");
		s+=t;
		
		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->Headline != _T(""))
		{
			t.Format(ML_STRING(1631, "Headline:\t%s"),
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->Headline);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->Caption != _T(""))
		{
			t.Format(ML_STRING(1632, "Description:\t%s"),
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->Caption);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->Keywords.GetSize() > 0)
		{
			for (int i = 0 ; i < m_pDoc->m_pDib->GetIptcFromXmpInfo()->Keywords.GetSize() ; i++)
			{
				if (i == 0)
				{
					t.Format(ML_STRING(1633, "Keywords:\t%s"),
						m_pDoc->m_pDib->GetIptcFromXmpInfo()->Keywords[i]);
				}
				else
				{
					t.Format(_T("; %s"),
						m_pDoc->m_pDib->GetIptcFromXmpInfo()->Keywords[i]);
				}
				t.Replace(_T("\r"), _T(""));
				t.Replace(_T("\n"), _T(""));
				s+=t;
			}
			s += _T("\r\n");
		}

		if (m_pDoc->m_pDib->GetXmpInfo()->SubjectCode.GetSize() > 0)
		{
			for (int i = 0 ; i < m_pDoc->m_pDib->GetXmpInfo()->SubjectCode.GetSize() ; i++)
			{
				if (i == 0)
				{
					t.Format(ML_STRING(1634, "IPTC subject code:\t%s"),
						m_pDoc->m_pDib->GetXmpInfo()->SubjectCode[i]);
				}
				else
				{
					t.Format(_T("; %s"),
						m_pDoc->m_pDib->GetXmpInfo()->SubjectCode[i]);
				}
				t.Replace(_T("\r"), _T(""));
				t.Replace(_T("\n"), _T(""));
				s+=t;
			}
			s += _T("\r\n");
		}

		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->Category != _T(""))
		{
			t.Format(ML_STRING(1635, "Category:\t%s"),
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->Category);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->SupplementalCategories.GetSize() > 0)
		{
			for (int i = 0 ; i < m_pDoc->m_pDib->GetIptcFromXmpInfo()->SupplementalCategories.GetSize() ; i++)
			{
				if (i == 0)
				{
					t.Format(ML_STRING(1636, "Supplemental cat.:\t%s"),
						m_pDoc->m_pDib->GetIptcFromXmpInfo()->SupplementalCategories[i]);
				}
				else
				{
					t.Format(_T("; %s"),
						m_pDoc->m_pDib->GetIptcFromXmpInfo()->SupplementalCategories[i]);
				}
				t.Replace(_T("\r"), _T(""));
				t.Replace(_T("\n"), _T(""));	
				s+=t;
			}
			s += _T("\r\n");
		}

		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->CaptionWriter != _T(""))
		{
			t.Format(ML_STRING(1637, "Description writer:\t%s"),
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->CaptionWriter);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		// Image
		t = _T("\r\n[") + ML_STRING(1579, "Image") + _T("]\r\n");
		s+=t;

		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->DateCreated != _T(""))
		{
			CTime Time = CMetadata::GetDateFromIptcLegacyString(m_pDoc->m_pDib->GetIptcFromXmpInfo()->DateCreated);
			t.Format(ML_STRING(1638, "Date created:\t%s"), ::MakeDateLocalFormat(Time, TRUE));
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetXmpInfo()->IntellectualGenre != _T(""))
		{
			t.Format(ML_STRING(1639, "Intellectual genre:\t%s"),
					m_pDoc->m_pDib->GetXmpInfo()->IntellectualGenre);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetXmpInfo()->Scene.GetSize() > 0)
		{
			for (int i = 0 ; i < m_pDoc->m_pDib->GetXmpInfo()->Scene.GetSize() ; i++)
			{
				if (i == 0)
				{
					t.Format(ML_STRING(1640, "IPTC scene:\t%s"),
						m_pDoc->m_pDib->GetXmpInfo()->Scene[i]);
				}
				else
				{
					t.Format(_T("; %s"),
						m_pDoc->m_pDib->GetXmpInfo()->Scene[i]);
				}
				t.Replace(_T("\r"), _T(""));
				t.Replace(_T("\n"), _T(""));
				s+=t;
			}
			s += _T("\r\n");
		}

		if (m_pDoc->m_pDib->GetXmpInfo()->Location != _T(""))
		{
			t.Format(ML_STRING(1641, "Location:\t%s"),
					m_pDoc->m_pDib->GetXmpInfo()->Location);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}
		
		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->City != _T(""))
		{
			t.Format(ML_STRING(1624, "City:\t%s"),
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->City);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->ProvinceState != _T(""))
		{
			t.Format(ML_STRING(1625, "State/Province:\t%s"),
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->ProvinceState);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->Country != _T(""))
		{
			t.Format(ML_STRING(1627, "Country:\t%s"),
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->Country);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetXmpInfo()->CountryCode != _T(""))
		{
			t.Format(ML_STRING(1642, "ISO country code:\t%s"),
					m_pDoc->m_pDib->GetXmpInfo()->CountryCode);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		// Status
		t = _T("\r\n[") + ML_STRING(1580, "Status") + _T("]\r\n");
		s+=t;

		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->ObjectName != _T(""))
		{
			t.Format(ML_STRING(1643, "Title:\t%s"),
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->ObjectName);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->OriginalTransmissionReference != _T(""))
		{
			t.Format(ML_STRING(1644, "Job identifier:\t%s"),
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->OriginalTransmissionReference);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->SpecialInstructions != _T(""))
		{
			t.Format(ML_STRING(1645, "Instructions:\t%s"),
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->SpecialInstructions);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->Credits != _T(""))
		{
			t.Format(ML_STRING(1646, "Provider:\t%s"),
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->Credits);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->Source != _T(""))
		{
			t.Format(ML_STRING(1647, "Source:\t%s"),
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->Source);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->CopyrightNotice != _T(""))
		{
			t.Format(ML_STRING(1648, "Copyright notice:\t%s"),
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->CopyrightNotice);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetXmpInfo()->UsageTerms != _T(""))
		{
			t.Format(ML_STRING(1649, "RightsUsageTerms:\t%s"),
					m_pDoc->m_pDib->GetXmpInfo()->UsageTerms);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetXmpInfo()->CopyrightUrl != _T(""))
		{
			t.Format(ML_STRING(1650, "Copyright URL:\t%s"),
					m_pDoc->m_pDib->GetXmpInfo()->CopyrightUrl);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		t.Format(ML_STRING(1651, "Copyright status:\t%s"),
			m_pDoc->m_pDib->GetXmpInfo()->CopyrightMarked == _T("True") ?
			_T("Copyrighted") : _T("Public Domain"));
		t.Replace(_T("\n"), _T("\n\t"));
		t += _T("\r\n");
		s+=t;

		if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->Urgency != _T(""))
		{
			t.Format(ML_STRING(1652, "Urgency:\t%s"),
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->Urgency);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}
	}
	// Iptc Legacy
	else if (m_nMetadataGroupView == IPTCLEGACY)
	{
		// Creator Contact Info
		t = _T("[") + ML_STRING(1577, "Contact") + _T("]\r\n");
		s+=t;

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->Byline != _T(""))
		{
			t.Format(ML_STRING(1621, "Creator:\t%s"),
					m_pDoc->m_pDib->GetIptcLegacyInfo()->Byline);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->BylineTitle != _T(""))
		{
			t.Format(ML_STRING(1622, "Creator's job title:\t%s"),
					m_pDoc->m_pDib->GetIptcLegacyInfo()->BylineTitle);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		// Content
		t = _T("\r\n[") + ML_STRING(1578, "Content") + _T("]\r\n");
		s+=t;

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->Headline != _T(""))
		{
			t.Format(ML_STRING(1631, "Headline:\t%s"),
					m_pDoc->m_pDib->GetIptcLegacyInfo()->Headline);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->Caption != _T(""))
		{
			t.Format(ML_STRING(1632, "Description:\t%s"),
					m_pDoc->m_pDib->GetIptcLegacyInfo()->Caption);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->Keywords.GetSize() > 0)
		{
			for (int i = 0 ; i < m_pDoc->m_pDib->GetIptcLegacyInfo()->Keywords.GetSize() ; i++)
			{
				if (i == 0)
				{
					t.Format(ML_STRING(1633, "Keywords:\t%s"),
						m_pDoc->m_pDib->GetIptcLegacyInfo()->Keywords[i]);
				}
				else
				{
					t.Format(_T("; %s"),
						m_pDoc->m_pDib->GetIptcLegacyInfo()->Keywords[i]);
				}
				t.Replace(_T("\r"), _T(""));
				t.Replace(_T("\n"), _T(""));
				s+=t;
			}
			s += _T("\r\n");
		}

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->Category != _T(""))
		{
			t.Format(ML_STRING(1635, "Category:\t%s"),
					m_pDoc->m_pDib->GetIptcLegacyInfo()->Category);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->SupplementalCategories.GetSize() > 0)
		{
			for (int i = 0 ; i < m_pDoc->m_pDib->GetIptcLegacyInfo()->SupplementalCategories.GetSize() ; i++)
			{
				if (i == 0)
				{
					t.Format(ML_STRING(1636, "Supplemental cat.:\t%s"),
						m_pDoc->m_pDib->GetIptcLegacyInfo()->SupplementalCategories[i]);
				}
				else
				{
					t.Format(_T("; %s"),
						m_pDoc->m_pDib->GetIptcLegacyInfo()->SupplementalCategories[i]);
				}
				t.Replace(_T("\r"), _T(""));
				t.Replace(_T("\n"), _T(""));
				s+=t;
			}
			s += _T("\r\n");
		}

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->CaptionWriter != _T(""))
		{
			t.Format(ML_STRING(1637, "Description writer:\t%s"),
					m_pDoc->m_pDib->GetIptcLegacyInfo()->CaptionWriter);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		// Image
		t = _T("\r\n[") + ML_STRING(1579, "Image") + _T("]\r\n");
		s+=t;
		
		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->DateCreated != _T(""))
		{
			CTime Time = CMetadata::GetDateFromIptcLegacyString(m_pDoc->m_pDib->GetIptcLegacyInfo()->DateCreated);
			t.Format(ML_STRING(1638, "Date created:\t%s"), ::MakeDateLocalFormat(Time, TRUE));
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->City != _T(""))
		{
			t.Format(ML_STRING(1624, "City:\t%s"),
					m_pDoc->m_pDib->GetIptcLegacyInfo()->City);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->ProvinceState != _T(""))
		{
			t.Format(ML_STRING(1625, "State/Province:\t%s"),
					m_pDoc->m_pDib->GetIptcLegacyInfo()->ProvinceState);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->Country != _T(""))
		{
			t.Format(ML_STRING(1627, "Country:\t%s"),
					m_pDoc->m_pDib->GetIptcLegacyInfo()->Country);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		// Status
		t = _T("\r\n[") + ML_STRING(1580, "Status") + _T("]\r\n");
		s+=t;

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->ObjectName != _T(""))
		{
			t.Format(ML_STRING(1643, "Title:\t%s"),
					m_pDoc->m_pDib->GetIptcLegacyInfo()->ObjectName);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->OriginalTransmissionReference != _T(""))
		{
			t.Format(ML_STRING(1644, "Job identifier:\t%s"),
					m_pDoc->m_pDib->GetIptcLegacyInfo()->OriginalTransmissionReference);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->SpecialInstructions != _T(""))
		{
			t.Format(ML_STRING(1645, "Instructions:\t%s"),
					m_pDoc->m_pDib->GetIptcLegacyInfo()->SpecialInstructions);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->Credits != _T(""))
		{
			t.Format(ML_STRING(1646, "Provider:\t%s"),
					m_pDoc->m_pDib->GetIptcLegacyInfo()->Credits);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->Source != _T(""))
		{
			t.Format(ML_STRING(1647, "Source:\t%s"),
					m_pDoc->m_pDib->GetIptcLegacyInfo()->Source);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->CopyrightNotice != _T(""))
		{
			t.Format(ML_STRING(1648, "Copyright notice:\t%s"),
					m_pDoc->m_pDib->GetIptcLegacyInfo()->CopyrightNotice);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}

		if (m_pDoc->m_pDib->GetIptcLegacyInfo()->Urgency != _T(""))
		{
			t.Format(ML_STRING(1652, "Urgency:\t%s"),
					m_pDoc->m_pDib->GetIptcLegacyInfo()->Urgency);
			t.Replace(_T("\n"), _T("\n\t"));
			t += _T("\r\n");
			s+=t;
		}
	}
	// Icc Color Profile
	else if (m_nMetadataGroupView == ICC)
	{
		if (m_pDoc->m_pDib->GetMetadata()->HasIcc())
		{
			int search = 1;		// Search for embedded profile
			int ecount = 1;		// Embedded count
			int offset = 0;		// Offset to read profile from
			int found;
			icmFile* fp = NULL;
			icmFile* op = NULL;
			icc* icco = NULL;
			int rv = 0;

			// Open for reading
			if ((fp = new_icmFileMem(	m_pDoc->m_pDib->GetMetadata()->m_pIccData,
										m_pDoc->m_pDib->GetMetadata()->m_dwIccSize))	&&
				(icco = new_icc())														&&
				(op = new_icmFileMem_d(NULL, 0))) // _d means that the library frees the allocated buffer when op is deleted
			{
				do
				{
					found = 0;

					// Dumb search for magic number
					if (search)
					{
						int fc = 0;
						char c;
						
						if (fp->seek(fp, offset) != 0)
							break;

						while (found == 0)
						{
							if (fp->read(fp, &c, 1, 1) != 1)
								break;
		
							offset++;
								
							switch (fc)
							{
								case 0:
									if (c == 'a')
										fc++;
									else
										fc = 0;
									break;
								case 1:
									if (c == 'c')
										fc++;
									else
										fc = 0;
									break;
								case 2:
									if (c == 's')
										fc++;
									else
										fc = 0;
									break;
								case 3:
									if (c == 'p')
									{
										found = 1;
										offset -= 40;
									}
									else
										fc = 0;
									break;
								default:
									break;
							}
						}
					}

					if (search == 0 || found != 0)
					{
						ecount++;
					
						if ((rv = icco->read(icco, fp, offset)) != 0)
						{
							if (g_nLogLevel > 0)
								::LogLine(_T("%d, %s"), rv, CString(icco->err));
						}
						
						icco->dump(icco, op, 1); // Verbose Level 1 .. 3

						offset += 128;
					}
				}
				while (found != 0);

				// Read from buffer
				unsigned char* buf = NULL;
				size_t len = 0;
				op->get_buf(op, &buf, &len); // not NULL terminated
				if (buf && len > 0)
				{
					CStringA sAnsi;
					LPSTR pAnsi = sAnsi.GetBuffer(len);
					memcpy(pAnsi, buf, len);
					sAnsi.ReleaseBuffer(len);
					s = CString(sAnsi);
					::MakeLineBreakCRLF(s);
				}
			}

			// Free
			if (icco)
				icco->del(icco);
			if (op)
				op->del(op);
			if (fp)
				fp->del(fp);
		}
	}

	// Remove terminating CR and LF
	s.TrimRight(_T("\r\n"));

	// Set Metadata Text
	CEdit* pMetadataBox = (CEdit*)GetDlgItem(IDC_METADATA);
	pMetadataBox->SetWindowText(s);
}

void CImageInfoDlg::UpdateMetadata()
{
	CString t;

	// Reset
	m_sOrigComment = _T("");
	m_sCurrentComment = _T("");
	m_OrigIptc.Clear();
	m_CurrentIptc.Clear();
	m_OrigXmp.Clear();
	m_CurrentXmp.Clear();
	m_OutOfSyncIptc.Clear();
	m_pDoc->m_bMetadataModified = FALSE;

	// Jpeg
	if (CDib::IsJPEG(m_pDoc->m_sFileName))
	{
		CString sJpegComment = ::FromUTF8(	(const unsigned char*)(LPCSTR)(m_pDoc->m_pDib->GetMetadata()->m_sJpegComment),
											m_pDoc->m_pDib->GetMetadata()->m_sJpegComment.GetLength());
		::MakeLineBreakCRLF(sJpegComment);
		m_sOrigComment = sJpegComment;
		m_sCurrentComment = sJpegComment;
		GetIptc();
	}

	// Tiff
	if (CDib::IsTIFF(m_pDoc->m_sFileName))
	{
		GetIptc();
	}

	// Gif
	if (::GetFileExtLower(m_pDoc->m_sFileName) == _T(".gif"))
	{
		CString sGifComment;
		if (m_pDoc->m_GifAnimationThread.IsAlive() &&
			m_pDoc->m_GifAnimationThread.m_dwDibAnimationCount > 1 &&
			!m_pDoc->m_GifAnimationThread.IsRunning())
		{
			CDib* pDib = m_pDoc->m_GifAnimationThread.m_DibAnimationArray.GetAt(m_pDoc->m_GifAnimationThread.m_dwDibAnimationPos);
			sGifComment = pDib->GetGif()->GetComment();
		}
		else
			sGifComment = m_pDoc->m_pDib->GetGif()->GetComment();
		::MakeLineBreakCRLF(sGifComment);
		m_sOrigComment = sGifComment;
		m_sCurrentComment = sGifComment;
	}

	// Display Metadata
	DisplayMetadata();

	// Set Edit Metadata Text
	SetEditMetadataText();
}

void CImageInfoDlg::GetIptc()
{
	// Get Xmp
	m_OrigXmp = m_CurrentXmp = *m_pDoc->m_pDib->GetXmpInfo();

	if (m_pDoc->m_pDib->GetIptcLegacyInfo()->IsEmpty())
	{
		m_OrigIptc =
		m_CurrentIptc =
		*m_pDoc->m_pDib->GetIptcFromXmpInfo();
	}
	else if (m_pDoc->m_pDib->GetIptcFromXmpInfo()->IsEmpty())
	{
		m_OrigIptc =
		m_CurrentIptc =
		*m_pDoc->m_pDib->GetIptcLegacyInfo();
	}
	else
	{
		// Check for Conflicts between Iptc Legacy and Iptc from Xmp!
		if (*m_pDoc->m_pDib->GetIptcLegacyInfo() != *m_pDoc->m_pDib->GetIptcFromXmpInfo())
		{
			// Caption
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->Caption,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->Caption,
					m_OrigIptc.Caption,
					m_CurrentIptc.Caption,
					m_OutOfSyncIptc.Caption);
		
			// Caption Writer
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->CaptionWriter,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->CaptionWriter,
					m_OrigIptc.CaptionWriter,
					m_CurrentIptc.CaptionWriter,
					m_OutOfSyncIptc.CaptionWriter);

			// Headline
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->Headline,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->Headline,
					m_OrigIptc.Headline,
					m_CurrentIptc.Headline,
					m_OutOfSyncIptc.Headline);

			// Special Instructions
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->SpecialInstructions,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->SpecialInstructions,
					m_OrigIptc.SpecialInstructions,
					m_CurrentIptc.SpecialInstructions,
					m_OutOfSyncIptc.SpecialInstructions);

			// Byline
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->Byline,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->Byline,
					m_OrigIptc.Byline,
					m_CurrentIptc.Byline,
					m_OutOfSyncIptc.Byline);

			// Byline Title
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->BylineTitle,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->BylineTitle,
					m_OrigIptc.BylineTitle,
					m_CurrentIptc.BylineTitle,
					m_OutOfSyncIptc.BylineTitle);

			// Credits
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->Credits,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->Credits,
					m_OrigIptc.Credits,
					m_CurrentIptc.Credits,
					m_OutOfSyncIptc.Credits);

			// Source
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->Source,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->Source,
					m_OrigIptc.Source,
					m_CurrentIptc.Source,
					m_OutOfSyncIptc.Source);

			// Object Name
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->ObjectName,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->ObjectName,
					m_OrigIptc.ObjectName,
					m_CurrentIptc.ObjectName,
					m_OutOfSyncIptc.ObjectName);

			// Date Created
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->DateCreated,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->DateCreated,
					m_OrigIptc.DateCreated,
					m_CurrentIptc.DateCreated,
					m_OutOfSyncIptc.DateCreated);

			// City
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->City,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->City,
					m_OrigIptc.City,
					m_CurrentIptc.City,
					m_OutOfSyncIptc.City);

			// Province-State
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->ProvinceState,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->ProvinceState,
					m_OrigIptc.ProvinceState,
					m_CurrentIptc.ProvinceState,
					m_OutOfSyncIptc.ProvinceState);

			// Country
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->Country,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->Country,
					m_OrigIptc.Country,
					m_CurrentIptc.Country,
					m_OutOfSyncIptc.Country);

			// Original Transmission Reference
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->OriginalTransmissionReference,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->OriginalTransmissionReference,
					m_OrigIptc.OriginalTransmissionReference,
					m_CurrentIptc.OriginalTransmissionReference,
					m_OutOfSyncIptc.OriginalTransmissionReference);
			
			// Category
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->Category,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->Category,
					m_OrigIptc.Category,
					m_CurrentIptc.Category,
					m_OutOfSyncIptc.Category);

			// Supplemental Categories
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->SupplementalCategories,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->SupplementalCategories,
					m_OrigIptc.SupplementalCategories,
					m_CurrentIptc.SupplementalCategories);

			// Urgency
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->Urgency,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->Urgency,
					m_OrigIptc.Urgency,
					m_CurrentIptc.Urgency,
					m_OutOfSyncIptc.Urgency);

			// Keywords
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->Keywords,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->Keywords,
					m_OrigIptc.Keywords,
					m_CurrentIptc.Keywords);

			// Copyright Notice
			GetIptc(m_pDoc->m_pDib->GetIptcLegacyInfo()->CopyrightNotice,
					m_pDoc->m_pDib->GetIptcFromXmpInfo()->CopyrightNotice,
					m_OrigIptc.CopyrightNotice,
					m_CurrentIptc.CopyrightNotice,
					m_OutOfSyncIptc.CopyrightNotice);
		}
		else
		{
			m_OrigIptc =
			m_CurrentIptc =
			*m_pDoc->m_pDib->GetIptcFromXmpInfo();
		}
	}
}

void CImageInfoDlg::GetIptc(const CString& sIptcLegacy,
							const CString& sIptcFromXmp,
							CString& sOrigIptc,
							CString& sCurrentIptc,
							CString& sOutOfSyncIptc)
{
	if (sIptcLegacy == _T(""))
	{
		sOrigIptc =
		sCurrentIptc =
		sIptcFromXmp;
	}
	else if (sIptcFromXmp == _T(""))
	{
		sOrigIptc =
		sCurrentIptc =
		sIptcLegacy;
	}
	// - Photoshop shows Xmp if out of sync.
	// - PixVue shows Iptc Legacy if out of sync.
	// -> We show both and prompt to fix the problem!
	else if (sIptcLegacy != sIptcFromXmp)
	{
		sOutOfSyncIptc = _T("1");
		sOrigIptc =
		sCurrentIptc =
		sIptcLegacy +
		_T("\r\n<--Iptc Leg. & Xmp Out of Sync-->\r\n") +
		sIptcFromXmp;
	}
	else
	{
		sOrigIptc =
		sCurrentIptc =
		sIptcFromXmp;
	}
}

void CImageInfoDlg::GetIptc(const CStringArray& IptcLegacy,
							const CStringArray& IptcFromXmp,
							CStringArray& OrigIptc,
							CStringArray& CurrentIptc)
{
	if (IptcLegacy.GetSize() == 0)
	{
		OrigIptc.Copy(IptcFromXmp);
		CurrentIptc.Copy(IptcFromXmp);
	}
	else if (IptcFromXmp.GetSize() == 0)
	{
		OrigIptc.Copy(IptcLegacy);
		CurrentIptc.Copy(IptcLegacy);
	}
	// Merge IptcLegacy with IptcFromXmp to OrigIptc and CurrentIptc
	else if (!CMetadata::IPTCINFO::IsArrayEqual(IptcLegacy,
												IptcFromXmp))
	{
		OrigIptc.Copy(IptcLegacy);
		for (int i = 0 ; i < IptcFromXmp.GetSize() ; i++)
		{
			if (!CMetadata::IPTCINFO::IsInArray(IptcFromXmp[i], OrigIptc))
				OrigIptc.Add(IptcFromXmp[i]);
		}
		CurrentIptc.Copy(OrigIptc);
	}
	else
	{
		OrigIptc.Copy(IptcFromXmp);
		CurrentIptc.Copy(IptcFromXmp);
	}
}

void CImageInfoDlg::SetIptc()
{
	// Set Xmp
	*m_pDoc->m_pDib->GetXmpInfo() = m_CurrentXmp;

	// Caption
	if (m_OutOfSyncIptc.Caption == _T("")		||
		(m_OutOfSyncIptc.Caption == _T("1")		&&
		m_CurrentIptc.Caption != m_OrigIptc.Caption))
	{
		m_pDoc->m_pDib->GetIptcLegacyInfo()->Caption = 
		m_pDoc->m_pDib->GetIptcFromXmpInfo()->Caption =
		m_CurrentIptc.Caption;
	}

	// Caption Writer
	if (m_OutOfSyncIptc.CaptionWriter == _T("")		||
		(m_OutOfSyncIptc.CaptionWriter == _T("1")		&&
		m_CurrentIptc.CaptionWriter != m_OrigIptc.CaptionWriter))
	{
		m_pDoc->m_pDib->GetIptcLegacyInfo()->CaptionWriter = 
		m_pDoc->m_pDib->GetIptcFromXmpInfo()->CaptionWriter =
		m_CurrentIptc.CaptionWriter;
	}

	// Headline
	if (m_OutOfSyncIptc.Headline == _T("")		||
		(m_OutOfSyncIptc.Headline == _T("1")		&&
		m_CurrentIptc.Headline != m_OrigIptc.Headline))
	{
		m_pDoc->m_pDib->GetIptcLegacyInfo()->Headline = 
		m_pDoc->m_pDib->GetIptcFromXmpInfo()->Headline =
		m_CurrentIptc.Headline;
	}

	// Special Instructions
	if (m_OutOfSyncIptc.SpecialInstructions == _T("")		||
		(m_OutOfSyncIptc.SpecialInstructions == _T("1")		&&
		m_CurrentIptc.SpecialInstructions != m_OrigIptc.SpecialInstructions))
	{
		m_pDoc->m_pDib->GetIptcLegacyInfo()->SpecialInstructions = 
		m_pDoc->m_pDib->GetIptcFromXmpInfo()->SpecialInstructions =
		m_CurrentIptc.SpecialInstructions;
	}

	// Byline
	if (m_OutOfSyncIptc.Byline == _T("")		||
		(m_OutOfSyncIptc.Byline == _T("1")		&&
		m_CurrentIptc.Byline != m_OrigIptc.Byline))
	{
		m_pDoc->m_pDib->GetIptcLegacyInfo()->Byline = 
		m_pDoc->m_pDib->GetIptcFromXmpInfo()->Byline =
		m_CurrentIptc.Byline;
	}

	// Byline Title
	if (m_OutOfSyncIptc.BylineTitle == _T("")		||
		(m_OutOfSyncIptc.BylineTitle == _T("1")		&&
		m_CurrentIptc.BylineTitle != m_OrigIptc.BylineTitle))
	{
		m_pDoc->m_pDib->GetIptcLegacyInfo()->BylineTitle = 
		m_pDoc->m_pDib->GetIptcFromXmpInfo()->BylineTitle =
		m_CurrentIptc.BylineTitle;
	}

	// Credits
	if (m_OutOfSyncIptc.Credits == _T("")		||
		(m_OutOfSyncIptc.Credits == _T("1")		&&
		m_CurrentIptc.Credits != m_OrigIptc.Credits))
	{
		m_pDoc->m_pDib->GetIptcLegacyInfo()->Credits = 
		m_pDoc->m_pDib->GetIptcFromXmpInfo()->Credits =
		m_CurrentIptc.Credits;
	}

	// Source
	if (m_OutOfSyncIptc.Source == _T("")		||
		(m_OutOfSyncIptc.Source == _T("1")		&&
		m_CurrentIptc.Source != m_OrigIptc.Source))
	{
		m_pDoc->m_pDib->GetIptcLegacyInfo()->Source = 
		m_pDoc->m_pDib->GetIptcFromXmpInfo()->Source =
		m_CurrentIptc.Source;
	}

	// Object Name
	if (m_OutOfSyncIptc.ObjectName == _T("")		||
		(m_OutOfSyncIptc.ObjectName == _T("1")		&&
		m_CurrentIptc.ObjectName != m_OrigIptc.ObjectName))
	{
		m_pDoc->m_pDib->GetIptcLegacyInfo()->ObjectName = 
		m_pDoc->m_pDib->GetIptcFromXmpInfo()->ObjectName =
		m_CurrentIptc.ObjectName;
	}

	// Date Created
	if (m_OutOfSyncIptc.DateCreated == _T("")		||
		(m_OutOfSyncIptc.DateCreated == _T("1")		&&
		m_CurrentIptc.DateCreated != m_OrigIptc.DateCreated))
	{
		m_pDoc->m_pDib->GetIptcLegacyInfo()->DateCreated = 
		m_pDoc->m_pDib->GetIptcFromXmpInfo()->DateCreated =
		m_CurrentIptc.DateCreated;
	}

	// City
	if (m_OutOfSyncIptc.City == _T("")		||
		(m_OutOfSyncIptc.City == _T("1")		&&
		m_CurrentIptc.City != m_OrigIptc.City))
	{
		m_pDoc->m_pDib->GetIptcLegacyInfo()->City = 
		m_pDoc->m_pDib->GetIptcFromXmpInfo()->City =
		m_CurrentIptc.City;
	}

	// Province-State
	if (m_OutOfSyncIptc.ProvinceState == _T("")		||
		(m_OutOfSyncIptc.ProvinceState == _T("1")		&&
		m_CurrentIptc.ProvinceState != m_OrigIptc.ProvinceState))
	{
		m_pDoc->m_pDib->GetIptcLegacyInfo()->ProvinceState = 
		m_pDoc->m_pDib->GetIptcFromXmpInfo()->ProvinceState =
		m_CurrentIptc.ProvinceState;
	}

	// Country
	if (m_OutOfSyncIptc.Country == _T("")		||
		(m_OutOfSyncIptc.Country == _T("1")		&&
		m_CurrentIptc.Country != m_OrigIptc.Country))
	{
		m_pDoc->m_pDib->GetIptcLegacyInfo()->Country = 
		m_pDoc->m_pDib->GetIptcFromXmpInfo()->Country =
		m_CurrentIptc.Country;
	}

	// Original Transmission Reference
	if (m_OutOfSyncIptc.OriginalTransmissionReference == _T("")		||
		(m_OutOfSyncIptc.OriginalTransmissionReference == _T("1")		&&
		m_CurrentIptc.OriginalTransmissionReference != m_OrigIptc.OriginalTransmissionReference))
	{
		m_pDoc->m_pDib->GetIptcLegacyInfo()->OriginalTransmissionReference = 
		m_pDoc->m_pDib->GetIptcFromXmpInfo()->OriginalTransmissionReference =
		m_CurrentIptc.OriginalTransmissionReference;
	}
	
	// Category
	if (m_OutOfSyncIptc.Category == _T("")		||
		(m_OutOfSyncIptc.Category == _T("1")		&&
		m_CurrentIptc.Category != m_OrigIptc.Category))
	{
		m_pDoc->m_pDib->GetIptcLegacyInfo()->Category = 
		m_pDoc->m_pDib->GetIptcFromXmpInfo()->Category =
		m_CurrentIptc.Category;
	}

	// Supplemental Categories
	m_pDoc->m_pDib->GetIptcLegacyInfo()->SupplementalCategories.Copy(m_CurrentIptc.SupplementalCategories); 
	m_pDoc->m_pDib->GetIptcFromXmpInfo()->SupplementalCategories.Copy(m_CurrentIptc.SupplementalCategories);

	// Urgency
	if (m_OutOfSyncIptc.Urgency == _T("")		||
		(m_OutOfSyncIptc.Urgency == _T("1")		&&
		m_CurrentIptc.Urgency != m_OrigIptc.Urgency))
	{
		m_pDoc->m_pDib->GetIptcLegacyInfo()->Urgency = 
		m_pDoc->m_pDib->GetIptcFromXmpInfo()->Urgency =
		m_CurrentIptc.Urgency;
	}

	// Keywords
	m_pDoc->m_pDib->GetIptcLegacyInfo()->Keywords.Copy(m_CurrentIptc.Keywords); 
	m_pDoc->m_pDib->GetIptcFromXmpInfo()->Keywords.Copy(m_CurrentIptc.Keywords);

	// Copyright Notice
	if (m_OutOfSyncIptc.CopyrightNotice == _T("")		||
		(m_OutOfSyncIptc.CopyrightNotice == _T("1")		&&
		m_CurrentIptc.CopyrightNotice != m_OrigIptc.CopyrightNotice))
	{
		m_pDoc->m_pDib->GetIptcLegacyInfo()->CopyrightNotice = 
		m_pDoc->m_pDib->GetIptcFromXmpInfo()->CopyrightNotice =
		m_CurrentIptc.CopyrightNotice;
	}
}

void CImageInfoDlg::LoadSettings()
{
	CWinApp* pApp = ::AfxGetApp();
	CString sSection(_T("FileInfoDlg"));

	// Date Created Source
	m_nMetadataGroupView = pApp->GetProfileInt(sSection, _T("MetadataGroupView"), EXIF);
}

void CImageInfoDlg::SaveSettings()
{
	CWinApp* pApp = ::AfxGetApp();
	CString sSection(_T("FileInfoDlg"));

	// Date Created Source
	pApp->WriteProfileInt(sSection, _T("MetadataGroupView"), m_nMetadataGroupView);
}