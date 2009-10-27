// DirectX7Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "DirectX7Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDirectX7Dlg dialog


CDirectX7Dlg::CDirectX7Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDirectX7Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDirectX7Dlg)
	m_sTextRow1 = _T("");
	m_sTextRow2 = _T("");
	m_sTextRow3 = _T("");
	//}}AFX_DATA_INIT
}


void CDirectX7Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDirectX7Dlg)
	DDX_Text(pDX, IDC_EDIT_TEXTROW1, m_sTextRow1);
	DDX_Text(pDX, IDC_EDIT_TEXTROW2, m_sTextRow2);
	DDX_Text(pDX, IDC_EDIT_TEXTROW3, m_sTextRow3);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDirectX7Dlg, CDialog)
	//{{AFX_MSG_MAP(CDirectX7Dlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDirectX7Dlg message handlers

BOOL CDirectX7Dlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Title
	SetWindowText(APPNAME_NOEXT);

	// Subclass Link Edit Box
	m_TextLink.SubclassDlgItem(IDC_EDIT_LINK, this);
	m_TextLink.SetWindowText(m_sTextLink);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
