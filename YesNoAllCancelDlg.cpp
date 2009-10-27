// YesNoAllCancelDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "YesNoAllCancelDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CYesNoAllCancelDlg dialog


CYesNoAllCancelDlg::CYesNoAllCancelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CYesNoAllCancelDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CYesNoAllCancelDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nOperation = NOT_DEFINED;
}

CString CYesNoAllCancelDlg::MakeRenamedFileName(CString sFileName)
{
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szName[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];
	_tsplitpath(sFileName, szDrive, szDir, szName, szExt);
	CString sRenamedFileName = sFileName;
	int i = 2;
	while(::IsExistingFile(sRenamedFileName))
	{
		sRenamedFileName.Format(_T("%s%s%s_%i%s"), szDrive, szDir, szName, i++, szExt);
	}
	return sRenamedFileName;
}

void CYesNoAllCancelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CYesNoAllCancelDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CYesNoAllCancelDlg, CDialog)
	//{{AFX_MSG_MAP(CYesNoAllCancelDlg)
	ON_BN_CLICKED(IDC_BUTTON_RENAME, OnButtonRename)
	ON_BN_CLICKED(IDC_BUTTON_RENAMEALL, OnButtonRenameall)
	ON_BN_CLICKED(IDC_BUTTON_REPLACEALL, OnButtonReplaceall)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CYesNoAllCancelDlg message handlers

void CYesNoAllCancelDlg::OnOK() // = Replace
{
	m_nOperation = REPLACE_FILE;
	CDialog::OnOK();
}

void CYesNoAllCancelDlg::OnButtonReplaceall() 
{
	m_nOperation = REPLACEALL_FILES;
	CDialog::OnOK();
}

void CYesNoAllCancelDlg::OnButtonRename() 
{
	m_nOperation = RENAME_FILE;
	CDialog::OnOK();
}

void CYesNoAllCancelDlg::OnButtonRenameall() 
{
	m_nOperation = RENAMEALL_FILES;
	CDialog::OnOK();
}

void CYesNoAllCancelDlg::OnCancel() 
{	
	m_nOperation = NOT_DEFINED;
	CDialog::OnCancel();
}

BOOL CYesNoAllCancelDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Set the Dialog Title to the Application Name
	SetWindowText(APPNAME_NOEXT);

	// Set File Name
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_FILENAME);
	CString sText;
	sText.Format(ML_STRING(1456, "%s already existing."), m_sFileName);
	pEdit->SetWindowText(sText);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
