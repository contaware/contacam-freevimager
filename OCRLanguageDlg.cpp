// OCRLanguageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "OCRLanguageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COCRLanguageDlg dialog


COCRLanguageDlg::COCRLanguageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COCRLanguageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COCRLanguageDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLangId = miLANG_ENGLISH;
}


void COCRLanguageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COCRLanguageDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COCRLanguageDlg, CDialog)
	//{{AFX_MSG_MAP(COCRLanguageDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COCRLanguageDlg message handlers

/*
Simplified Chinese
Traditional Chinese
Czech
Danish
Dutch
English
Finnish
French
German
Greek
Hungarian
Italian
Japanese
Korean
Norwegian
Polish
Portuguese
Russian
Spanish	
Swedish
Turkish
*/

BOOL COCRLanguageDlg::OnInitDialog() 
{
	// Init Combo Box
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_LANGUAGE);
	if (pComboBox)
	{
		pComboBox->AddString(ML_STRING(1520, "English"));
		pComboBox->AddString(ML_STRING(1521, "French"));
		pComboBox->AddString(ML_STRING(1522, "German"));
		pComboBox->AddString(ML_STRING(1523, "Italian"));
		pComboBox->AddString(ML_STRING(1524, "Spanish"));
		pComboBox->AddString(ML_STRING(1525, "Portuguese"));
		pComboBox->AddString(ML_STRING(1526, "Czech"));
		pComboBox->AddString(ML_STRING(1527, "Greek"));
		pComboBox->AddString(ML_STRING(1528, "Hungarian"));
		pComboBox->AddString(ML_STRING(1529, "Polish"));
		pComboBox->AddString(ML_STRING(1530, "Dutch"));
		pComboBox->AddString(ML_STRING(1531, "Danish"));
		pComboBox->AddString(ML_STRING(1532, "Finnish"));
		pComboBox->AddString(ML_STRING(1533, "Swedish"));
		pComboBox->AddString(ML_STRING(1534, "Norwegian"));
		pComboBox->AddString(ML_STRING(1535, "Russian"));
		pComboBox->AddString(ML_STRING(1536, "Turkish"));
		pComboBox->AddString(ML_STRING(1537, "Japanese"));
		pComboBox->AddString(ML_STRING(1538, "Korean"));
		pComboBox->AddString(ML_STRING(1539, "Simplified Chinese"));
		pComboBox->AddString(ML_STRING(1540, "Traditional Chinese"));
	}

	CDialog::OnInitDialog();
	
	CString sLang;
	switch (m_lLangId)
	{
		case miLANG_CHINESE_SIMPLIFIED :	sLang = _T("Simplified Chinese");
			break;
		case miLANG_CHINESE_TRADITIONAL :	sLang = _T("Traditional Chinese");
			break;
		case miLANG_CZECH :					sLang = _T("Czech");
			break;
		case miLANG_DANISH :				sLang = _T("Danish");
			break;
		case miLANG_DUTCH :					sLang = _T("Dutch");
			break;
		case miLANG_ENGLISH :				sLang = _T("English");
			break;
		case miLANG_FINNISH :				sLang = _T("Finnish");
			break;
		case miLANG_FRENCH :				sLang = _T("French");
			break;
		case miLANG_GERMAN :				sLang = _T("German");
			break;
		case miLANG_GREEK :					sLang = _T("Greek");
			break;
		case miLANG_HUNGARIAN :				sLang = _T("Hungarian");
			break;
		case miLANG_ITALIAN :				sLang = _T("Italian");
			break;
		case miLANG_JAPANESE :				sLang = _T("Japanese");
			break;
		case miLANG_KOREAN :				sLang = _T("Korean");
			break;
		case miLANG_NORWEGIAN :				sLang = _T("Norwegian");
			break;
		case miLANG_POLISH :				sLang = _T("Polish");
			break;
		case miLANG_PORTUGUESE :			sLang = _T("Portuguese");
			break;
		case miLANG_RUSSIAN :				sLang = _T("Russian");
			break;
		case miLANG_SPANISH :				sLang = _T("Spanish");
			break;
		case miLANG_SWEDISH :				sLang = _T("Swedish");
			break;
		case miLANG_TURKISH :				sLang = _T("Turkish");
			break;
	}

	// Select Language
	if (pComboBox)
	{
		int nIndex = pComboBox->FindStringExact(-1, sLang);
		if (nIndex == CB_ERR)
			pComboBox->SetCurSel(0);
		else
			pComboBox->SetCurSel(nIndex);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COCRLanguageDlg::OnOK() 
{	
	SelToLangId();
	CDialog::OnOK();
}

void COCRLanguageDlg::SelToLangId()
{
	CString sLang;
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_LANGUAGE);
	if (pComboBox)
	{
		if (pComboBox->GetCurSel() <= 0)
			sLang = _T("English");
		else
			pComboBox->GetLBText(pComboBox->GetCurSel(), sLang);
	}
	if (sLang == _T("Simplified Chinese"))
		m_lLangId = miLANG_CHINESE_SIMPLIFIED;
	else if (sLang == _T("Traditional Chinese"))
		m_lLangId = miLANG_CHINESE_TRADITIONAL;
	else if (sLang == _T("Czech"))
		m_lLangId = miLANG_CZECH;
	else if (sLang == _T("Danish"))
		m_lLangId = miLANG_DANISH;
	else if (sLang == _T("Dutch"))
		m_lLangId = miLANG_DUTCH;
	else if (sLang == _T("Finnish"))
		m_lLangId = miLANG_FINNISH;
	else if (sLang == _T("French"))
		m_lLangId = miLANG_FRENCH;
	else if (sLang == _T("German"))
		m_lLangId = miLANG_GERMAN;
	else if (sLang == _T("Greek"))
		m_lLangId = miLANG_GREEK;
	else if (sLang == _T("Hungarian"))
		m_lLangId = miLANG_HUNGARIAN;
	else if (sLang == _T("Italian"))
		m_lLangId = miLANG_ITALIAN;
	else if (sLang == _T("Japanese"))
		m_lLangId = miLANG_JAPANESE;
	else if (sLang == _T("Korean"))
		m_lLangId = miLANG_KOREAN;
	else if (sLang == _T("Norwegian"))
		m_lLangId = miLANG_NORWEGIAN;
	else if (sLang == _T("Polish"))
		m_lLangId = miLANG_POLISH;
	else if (sLang == _T("Portuguese"))
		m_lLangId = miLANG_PORTUGUESE;
	else if (sLang == _T("Russian"))
		m_lLangId = miLANG_RUSSIAN;
	else if (sLang == _T("Spanish"))
		m_lLangId = miLANG_SPANISH;
	else if (sLang == _T("Swedish"))
		m_lLangId = miLANG_SWEDISH;
	else if (sLang == __T("Turkish"))
		m_lLangId = miLANG_TURKISH;
	else
		m_lLangId = miLANG_ENGLISH;
}
