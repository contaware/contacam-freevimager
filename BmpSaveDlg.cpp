// BmpSaveDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "BmpSaveDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBmpSaveDlg dialog


CBmpSaveDlg::CBmpSaveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBmpSaveDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBmpSaveDlg)
	m_bRleEncode = FALSE;
	//}}AFX_DATA_INIT
}


void CBmpSaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBmpSaveDlg)
	DDX_Check(pDX, IDC_CHECK_RLE_ENCODE, m_bRleEncode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBmpSaveDlg, CDialog)
	//{{AFX_MSG_MAP(CBmpSaveDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBmpSaveDlg message handlers
