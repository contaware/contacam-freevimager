// PlateRecognizerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uImager.h"
#include "PlateRecognizerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

IMPLEMENT_DYNAMIC(CPlateRecognizerDlg, CDialog)

CPlateRecognizerDlg::CPlateRecognizerDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_PLATERECOGNIZER, pParent)
	, m_nMode(0)
	, m_sToken(_T(""))
	, m_sUrl(_T("http://localhost:8080/alpr"))
	, m_sRegions(_T(""))
{

}

CPlateRecognizerDlg::~CPlateRecognizerDlg()
{
}

void CPlateRecognizerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_CHECK_PLATERECOGNIZER_CLOUD, m_nMode);
	DDX_Text(pDX, IDC_EDIT_PLATERECOGNIZER_TOKEN, m_sToken);
	DDX_Text(pDX, IDC_EDIT_PLATERECOGNIZER_URL, m_sUrl);
	DDX_Text(pDX, IDC_EDIT_PLATERECOGNIZER_REGIONS, m_sRegions);
}

BEGIN_MESSAGE_MAP(CPlateRecognizerDlg, CDialog)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_PLATERECOGNIZER_HELP, OnSyslinkPlateRecognizerHelp)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_PLATERECOGNIZER_HELP, OnSyslinkPlateRecognizerHelp)
	ON_BN_CLICKED(IDC_CHECK_PLATERECOGNIZER_CLOUD, OnCheckPlateRecognizerCloud)
	ON_BN_CLICKED(IDC_CHECK_PLATERECOGNIZER_ONPREMISE, OnCheckPlateRecognizerOnPremise)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_PLATERECOGNIZER_REGIONS_HELP, OnSyslinkPlateRecognizerRegionsHelp)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_PLATERECOGNIZER_REGIONS_HELP, OnSyslinkPlateRecognizerRegionsHelp)
END_MESSAGE_MAP()

BOOL CPlateRecognizerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the input focus to the token edit control and disable the url edit control
	GotoDlgCtrl(GetDlgItem(IDC_EDIT_PLATERECOGNIZER_TOKEN));
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PLATERECOGNIZER_URL);
	pEdit->EnableWindow(FALSE);

	// Return FALSE because we manually set the focus to a control
	return FALSE;
}

void CPlateRecognizerDlg::OnSyslinkPlateRecognizerHelp(NMHDR *pNMHDR, LRESULT *pResult)
{
	::ShellExecute(	NULL,
					_T("open"),
					PLATERECOGNIZER_ONLINE_PAGE,
					NULL, NULL, SW_SHOWNORMAL);
	*pResult = 0;
}

void CPlateRecognizerDlg::OnCheckPlateRecognizerCloud()
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PLATERECOGNIZER_TOKEN);
	pEdit->EnableWindow(TRUE);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PLATERECOGNIZER_URL);
	pEdit->EnableWindow(FALSE);
}

void CPlateRecognizerDlg::OnCheckPlateRecognizerOnPremise()
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PLATERECOGNIZER_TOKEN);
	pEdit->EnableWindow(FALSE);
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PLATERECOGNIZER_URL);
	pEdit->EnableWindow(TRUE);
}

void CPlateRecognizerDlg::OnSyslinkPlateRecognizerRegionsHelp(NMHDR *pNMHDR, LRESULT *pResult)
{
	::ShellExecute(	NULL,
					_T("open"),
					PLATERECOGNIZER_REGIONS_ONLINE_PAGE,
					NULL, NULL, SW_SHOWNORMAL);
	*pResult = 0;
}

#endif
