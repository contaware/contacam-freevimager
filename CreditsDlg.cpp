// CreditsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "CreditsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreditsDlg dialog


CCreditsDlg::CCreditsDlg(CWnd* pParent, UINT idd)
	: CDialog(idd, pParent)
{
	//{{AFX_DATA_INIT(CCreditsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCreditsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreditsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreditsDlg, CDialog)
	//{{AFX_MSG_MAP(CCreditsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreditsDlg message handlers

BOOL CCreditsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (GetDlgItem(IDC_LINK_FFMPEG))
		m_FfmpegLink.SubclassDlgItem(IDC_LINK_FFMPEG, this);
	if (GetDlgItem(IDC_LINK_MICROAPACHE))
		m_MicroApacheLink.SubclassDlgItem(IDC_LINK_MICROAPACHE, this);
	m_IjgLink.SubclassDlgItem(IDC_LINK_IJG, this);
	m_LibTiffLink.SubclassDlgItem(IDC_LINK_LIBTIFF, this);
	m_LibPngLink.SubclassDlgItem(IDC_LINK_LIBPNG, this);
	m_ZlibLink.SubclassDlgItem(IDC_LINK_ZLIB, this);
	m_GifLibLink.SubclassDlgItem(IDC_LINK_GIFLIB, this);
	m_IccLibLink.SubclassDlgItem(IDC_LINK_ICCLIB, this);
	m_WinkLink.SubclassDlgItem(IDC_LINK_WINK, this);
	m_NsisLink.SubclassDlgItem(IDC_LINK_NSIS, this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
