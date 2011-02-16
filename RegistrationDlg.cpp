// RegistrationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "RegistrationDlg.h"
#include "NoVistaFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CRegistrationDlg dialog


CRegistrationDlg::CRegistrationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRegistrationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRegistrationDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRegistrationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRegistrationDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRegistrationDlg, CDialog)
	//{{AFX_MSG_MAP(CRegistrationDlg)
	ON_BN_CLICKED(IDC_BUTTON_BUY_LICENSE, OnButtonBuyLicense)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_KEYFILE, OnButtonOpenKeyfile)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRegistrationDlg message handlers

void CRegistrationDlg::OnButtonBuyLicense() 
{
	::ShellExecute(	NULL,
					_T("open"),
					REGISTRATION_ONLINE_PAGE,
					NULL, NULL, SW_SHOWNORMAL);
}

void CRegistrationDlg::OnButtonOpenKeyfile() 
{
	CNoVistaFileDlg fd(	TRUE,
						_T("bin"),
						_T(""),
						OFN_HIDEREADONLY, // Hides the Read Only check box
						_T("Binary Key Files (*.bin)|*.bin||"));
	if (fd.DoModal() == IDOK)
	{
		// Get App Data Folder
		CString sRSAKeyFile = ::GetSpecialFolderPath(CSIDL_APPDATA);

		// It's important to have a place to write the config file,
		// under win95 and NT4 CSIDL_APPDATA is not available
		// return the program's directory
		if (sRSAKeyFile == _T(""))
		{
			TCHAR szDrive[_MAX_DRIVE];
			TCHAR szDir[_MAX_DIR];
			TCHAR szProgramName[MAX_PATH];
			if (::GetModuleFileName(NULL, szProgramName, MAX_PATH) == 0)
				return;
			_tsplitpath(szProgramName, szDrive, szDir, NULL, NULL);
			sRSAKeyFile = CString(szDrive) + CString(szDir);
			sRSAKeyFile += RSA_KEYNAME_EXT;
		}
		else
			sRSAKeyFile += _T("\\") + RSA_KEY_FILE;
		CString sPath = ::GetDriveAndDirName(sRSAKeyFile);
		if (!::IsExistingDir(sPath))
			::CreateDir(sPath);
		if (::CopyFile(fd.GetPathName(), sRSAKeyFile, FALSE))
		{
			((CUImagerApp*)::AfxGetApp())->m_bRegistered = ((CUImagerApp*)::AfxGetApp())->RSADecrypt();
			UpdateRegCtrls();
			::AfxGetMainFrame()->m_MDIClientWnd.Invalidate();
		}
	}
}

void CRegistrationDlg::UpdateRegCtrls()
{
	CEdit* pRegNum = (CEdit*)GetDlgItem(IDC_REG_NUMBER);
	CEdit* pRegName = (CEdit*)GetDlgItem(IDC_REG_NAME);
	if (((CUImagerApp*)::AfxGetApp())->m_bRegistered)
	{
		CString sRegNum;
		sRegNum.Format(_T("%u.%u"),	((CUImagerApp*)::AfxGetApp())->m_dwPURCHASE_ID,
									(DWORD)((CUImagerApp*)::AfxGetApp())->m_wRUNNING_NO);
		pRegNum->SetWindowText(sRegNum);
		pRegName->SetWindowText(((CUImagerApp*)::AfxGetApp())->m_sREG_NAME);
	}
	else
	{
		pRegNum->SetWindowText(ML_STRING(1748, "unregistered"));
		pRegName->SetWindowText(ML_STRING(1748, "unregistered"));
	}
	Invalidate();
}

BOOL CRegistrationDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	UpdateRegCtrls();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH CRegistrationDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr; 

	switch (nCtlColor) 
	{ 
		// For Read/Write Edit Controls
		case CTLCOLOR_EDIT:
		case CTLCOLOR_MSGBOX:
			hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
			break;

		// Process Static text, READONLY controls, DISABLED * controls.
		//   NOTE: Disabled controls can NOT have their text color
		//         changed.
		//         Suggest you change all your DISABLED controls to
		//         READONLY.
		case CTLCOLOR_STATIC:
			switch (pWnd->GetDlgCtrlID())
			{     
				case IDC_REG_NUMBER :
				case IDC_REG_NAME :
					pDC->SetBkMode(TRANSPARENT);
					pDC->SetTextColor(((CUImagerApp*)::AfxGetApp())->m_bRegistered ? REGISTERED_FONT_COLOR : UNREGISTERED_FONT_COLOR_2);
					hbr = (HBRUSH)::GetStockObject(NULL_BRUSH);
					break;
		
				default:
					hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
					break;
			}
			break;

		// Otherwise, do default handling of OnCtlColor
		default:
			hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
			break;
	}

	return hbr; // return brush
}

#endif