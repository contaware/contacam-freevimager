// DeletePageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "DeletePageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDeletePageDlg dialog


CDeletePageDlg::CDeletePageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDeletePageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDeletePageDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDeletePageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeletePageDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDeletePageDlg, CDialog)
	//{{AFX_MSG_MAP(CDeletePageDlg)
	ON_BN_CLICKED(IDNO, OnNo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeletePageDlg message handlers

void CDeletePageDlg::OnNo() 
{
	EndDialog(IDNO);
}