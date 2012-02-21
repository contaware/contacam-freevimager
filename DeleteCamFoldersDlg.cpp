// DeleteCamFoldersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "DeleteCamFoldersDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CDeleteCamFoldersDlg dialog


CDeleteCamFoldersDlg::CDeleteCamFoldersDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDeleteCamFoldersDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDeleteCamFoldersDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDeleteCamFoldersDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeleteCamFoldersDlg)
	DDX_Control(pDX, IDC_CAM_FOLDERS, m_CamFolders);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDeleteCamFoldersDlg, CDialog)
	//{{AFX_MSG_MAP(CDeleteCamFoldersDlg)
	ON_BN_CLICKED(IDC_BUTTON_LIST_SELECTALL, OnButtonListSelectall)
	ON_BN_CLICKED(IDC_BUTTON_LIST_SELECTNONE, OnButtonListSelectnone)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeleteCamFoldersDlg message handlers

BOOL CDeleteCamFoldersDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CString sMicroApacheDocRoot = ((CUImagerApp*)::AfxGetApp())->m_sMicroApacheDocRoot;
	sMicroApacheDocRoot.TrimRight(_T('\\'));
	m_DirFind.Init(sMicroApacheDocRoot + _T("\\*"));
	for (int pos = 0 ; pos < m_DirFind.GetDirsCount() ; pos++)
	{
		CString sDirName(m_DirFind.GetDirName(pos));
		sDirName.TrimRight(_T('\\'));
		int index;
		if ((index = sDirName.ReverseFind(_T('\\'))) >= 0)
			sDirName = sDirName.Right(sDirName.GetLength() - index - 1);
		m_CamFolders.AddString(sDirName);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDeleteCamFoldersDlg::OnOK() 
{
	for (int pos = 0 ; pos < m_DirFind.GetDirsCount() ; pos++)
	{
		if (m_CamFolders.GetSel(pos) > 0)
			::DeleteToRecycleBin(m_DirFind.GetDirName(pos), FALSE);
	}
	
	CDialog::OnOK();
}

void CDeleteCamFoldersDlg::OnButtonListSelectall() 
{
	m_CamFolders.SetSel(-1);
}

void CDeleteCamFoldersDlg::OnButtonListSelectnone() 
{
	m_CamFolders.SetSel(-1, FALSE);
}

#endif
