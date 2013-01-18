// SnapshotNamesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "SnapshotNamesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSnapshotNamesDlg dialog


CSnapshotNamesDlg::CSnapshotNamesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSnapshotNamesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSnapshotNamesDlg)
	m_sSnapshotLiveJpegName = _T("");
	m_sSnapshotLiveJpegThumbName = _T("");
	//}}AFX_DATA_INIT
}


void CSnapshotNamesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSnapshotNamesDlg)
	DDX_Text(pDX, IDC_EDIT_SNAPSHOT_LIVE_JPEGNAME, m_sSnapshotLiveJpegName);
	DDX_Text(pDX, IDC_EDIT_SNAPSHOT_LIVE_JPEGTHUMBNAME, m_sSnapshotLiveJpegThumbName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSnapshotNamesDlg, CDialog)
	//{{AFX_MSG_MAP(CSnapshotNamesDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSnapshotNamesDlg message handlers
