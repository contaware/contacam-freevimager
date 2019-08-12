// SettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "sinstance.h"
#include "SettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg dialog

CSettingsDlg::CSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsDlg::IDD, pParent)
{
	// Global Settings
	m_bSingleInstance =	((CUImagerApp*)::AfxGetApp())->m_bSingleInstance;
	m_bTrayIcon =		((CUImagerApp*)::AfxGetApp())->m_bTrayIcon;
	m_bAutostart =		((CUImagerApp*)::AfxGetApp())->IsAutostart();
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingsDlg)
	DDX_Check(pDX, IDC_CHECK_SINGLEINSTANCE, m_bSingleInstance);
	DDX_Check(pDX, IDC_CHECK_TRAYICON, m_bTrayIcon);
	DDX_Check(pDX, IDC_CHECK_STARTWITH_WINDOWS, m_bAutostart);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSettingsDlg, CDialog)
	//{{AFX_MSG_MAP(CSettingsDlg)
	ON_BN_CLICKED(IDC_CHECK_STARTWITH_WINDOWS, &CSettingsDlg::OnBnClickedCheckStartwithWindows)
	ON_BN_CLICKED(IDC_CHECK_TRAYICON, &CSettingsDlg::OnBnClickedCheckTrayicon)
	ON_BN_CLICKED(IDC_CHECK_SINGLEINSTANCE, &CSettingsDlg::OnBnClickedCheckSingleinstance)
	ON_BN_CLICKED(IDC_BUTTON_ASSOCIATE, &CSettingsDlg::OnBnClickedButtonAssociate)
	ON_BN_CLICKED(IDC_BUTTON_UNASSOCIATE, &CSettingsDlg::OnBnClickedButtonUnassociate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg message handlers

void CSettingsDlg::OnBnClickedCheckStartwithWindows()
{
	// Validate
	if (!UpdateData(TRUE))
		return;

	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();
	pApp->Autostart(m_bAutostart);
}

void CSettingsDlg::OnBnClickedCheckTrayicon()
{
	// Validate
	if (!UpdateData(TRUE))
		return;

	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();
	pApp->m_bTrayIcon = m_bTrayIcon;
	::AfxGetMainFrame()->TrayIcon(m_bTrayIcon);
	pApp->WriteProfileInt(_T("GeneralApp"), _T("TrayIcon"), m_bTrayIcon);
}

void CSettingsDlg::OnBnClickedCheckSingleinstance()
{
	// Validate
	if (!UpdateData(TRUE))
		return;

	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();
	if (m_bSingleInstance != pApp->m_bSingleInstance)
	{
		CInstanceChecker instanceChecker(CString(APPNAME_NOEXT) + CString(_T("_Unicode")));
		if (m_bSingleInstance)
		{
			instanceChecker.ActivateChecker();
			if (!instanceChecker.PreviousInstanceRunning())
				instanceChecker.TrackFirstInstanceRunning();
		}
		// else the constructor above closed the handle of the mm file
	}
	pApp->m_bSingleInstance = m_bSingleInstance;
	pApp->WriteProfileInt(_T("GeneralApp"), _T("SingleInstance"), m_bSingleInstance);
}

void CSettingsDlg::OnBnClickedButtonAssociate()
{
	// Associate all supported file types
	FileAssociation(TRUE);

	// Close dialog
	EndDialog(IDOK);
}

void CSettingsDlg::OnBnClickedButtonUnassociate()
{
	// Unassociate all supported file types
	FileAssociation(FALSE);

	// Close dialog
	EndDialog(IDOK);
}

void CSettingsDlg::FileAssociation(BOOL bDoAssociation)
{
	CUImagerApp* pApp = (CUImagerApp*)::AfxGetApp();

	if (bDoAssociation)
	{
		// Bmp
		pApp->AssociateFileType(_T("bmp"));

		// Jpeg
		pApp->AssociateFileType(_T("jpg"));
		pApp->AssociateFileType(_T("jpeg"));
		pApp->AssociateFileType(_T("jpe"));
		pApp->AssociateFileType(_T("jfif"));
		pApp->AssociateFileType(_T("thm"));

		// Pcx
		pApp->AssociateFileType(_T("pcx"));

		// Emf
		pApp->AssociateFileType(_T("emf"));

		// Png
		pApp->AssociateFileType(_T("png"));

		// Tiff
		pApp->AssociateFileType(_T("tif"));
		pApp->AssociateFileType(_T("tiff"));
		pApp->AssociateFileType(_T("jfx"));

		// Gif
		pApp->AssociateFileType(_T("gif"));
	}
	else
	{
		// Bmp
		pApp->UnassociateFileType(_T("bmp"));

		// Jpeg
		pApp->UnassociateFileType(_T("jpg"));
		pApp->UnassociateFileType(_T("jpeg"));
		pApp->UnassociateFileType(_T("jpe"));
		pApp->UnassociateFileType(_T("jfif"));
		pApp->UnassociateFileType(_T("thm"));

		// Pcx
		pApp->UnassociateFileType(_T("pcx"));

		// Emf
		pApp->UnassociateFileType(_T("emf"));

		// Png
		pApp->UnassociateFileType(_T("png"));

		// Tiff
		pApp->UnassociateFileType(_T("tif"));
		pApp->UnassociateFileType(_T("tiff"));
		pApp->UnassociateFileType(_T("jfx"));

		// Gif
		pApp->UnassociateFileType(_T("gif"));
	}

	// Notify Changes
	::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}

#endif
