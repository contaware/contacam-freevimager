// SettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "remotecamviewer.h"
#include "SettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg dialog


CSettingsDlg::CSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSettingsDlg)
	m_sTitle = _T("");
	m_sLabel0 = _T("");
	m_sLabel1 = _T("");
	m_sLabel2 = _T("");
	m_sLabel3 = _T("");
	//}}AFX_DATA_INIT
}


void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingsDlg)
	DDX_Text(pDX, IDC_EDIT_TITLE, m_sTitle);
	DDX_Text(pDX, IDC_EDIT_LABEL0, m_sLabel0);
	DDX_Text(pDX, IDC_EDIT_LABEL1, m_sLabel1);
	DDX_Text(pDX, IDC_EDIT_LABEL2, m_sLabel2);
	DDX_Text(pDX, IDC_EDIT_LABEL3, m_sLabel3);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSettingsDlg, CDialog)
	//{{AFX_MSG_MAP(CSettingsDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg message handlers
