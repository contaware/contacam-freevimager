// PngSaveDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "PngSaveDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPngSaveDlg dialog


CPngSaveDlg::CPngSaveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPngSaveDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPngSaveDlg)
	m_bStoreBackgroundColor = FALSE;
	//}}AFX_DATA_INIT
}


void CPngSaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPngSaveDlg)
	DDX_Check(pDX, IDC_CHECK_STORE_BACKGROUND_COLOR, m_bStoreBackgroundColor);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPngSaveDlg, CDialog)
	//{{AFX_MSG_MAP(CPngSaveDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPngSaveDlg message handlers
