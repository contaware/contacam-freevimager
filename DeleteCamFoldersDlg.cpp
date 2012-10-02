// DeleteCamFoldersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "VideoDeviceDoc.h"
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
	
	// Enum all folders
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
	
	// Enum all device entries in registry or ini file
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
	{
		if (((CUImagerApp*)::AfxGetApp())->m_bUseRegistry)
		{
			const int MAX_KEY_BUFFER = 257; // http://www.sepago.de/e/holger/2010/07/20/how-long-can-a-registry-key-name-really-be
			HKEY hKey;
			if (::RegOpenKeyEx(	HKEY_CURRENT_USER,
								_T("Software\\") + CString(MYCOMPANY) + CString(_T("\\")) + CString(APPNAME_NOEXT),
								0, KEY_READ, &hKey) == ERROR_SUCCESS)
			{
				DWORD cSubKeys = 0;
				::RegQueryInfoKey(hKey, NULL, NULL, NULL, &cSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
				TCHAR achKey[MAX_KEY_BUFFER];
				DWORD cbName;
				for (DWORD i = 0 ; i < cSubKeys; i++)
				{ 
					cbName = MAX_KEY_BUFFER;
					::RegEnumKeyEx(hKey, i, achKey, &cbName, NULL, NULL, NULL, NULL);
					CString sRecordAutoSaveDir = ::AfxGetApp()->GetProfileString(achKey, _T("RecordAutoSaveDir"), _T(""));
					CString sDetectionAutoSaveDir = ::AfxGetApp()->GetProfileString(achKey, _T("DetectionAutoSaveDir"), _T(""));
					CString sSnapshotAutoSaveDir = ::AfxGetApp()->GetProfileString(achKey, _T("SnapshotAutoSaveDir"), _T(""));
					if (sRecordAutoSaveDir != _T("")	&&
						sDetectionAutoSaveDir != _T("")	&&
						sSnapshotAutoSaveDir != _T(""))
						m_DevicePathNames.Add(achKey);
				}
				::RegCloseKey(hKey);
			}
		}
		else
		{
			const int MAX_SECTIONNAMES_BUFFER = 1024 * 1024; // 1 MB is enough!
			TCHAR* pSectionNames = new TCHAR[MAX_SECTIONNAMES_BUFFER];
			::GetPrivateProfileSectionNames(pSectionNames, MAX_SECTIONNAMES_BUFFER, ::AfxGetApp()->m_pszProfileName);
			TCHAR* sSource = pSectionNames;
			while (*sSource != 0) // If 0 -> end of list
			{
				CString sRecordAutoSaveDir = ::AfxGetApp()->GetProfileString(sSource, _T("RecordAutoSaveDir"), _T(""));
				CString sDetectionAutoSaveDir = ::AfxGetApp()->GetProfileString(sSource, _T("DetectionAutoSaveDir"), _T(""));
				CString sSnapshotAutoSaveDir = ::AfxGetApp()->GetProfileString(sSource, _T("SnapshotAutoSaveDir"), _T(""));
				if (sRecordAutoSaveDir != _T("")	&&
					sDetectionAutoSaveDir != _T("")	&&
					sSnapshotAutoSaveDir != _T(""))
					m_DevicePathNames.Add(sSource);
				while (*sSource != 0)
					sSource++;
				sSource++; // Skip the 0
			}
			delete [] pSectionNames;
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDeleteCamFoldersDlg::OnOK() 
{
	EnableWindow(FALSE);
	for (int pos = 0 ; pos < m_DirFind.GetDirsCount() ; pos++)
	{
		if (m_CamFolders.GetSel(pos) > 0)
		{
			CString sDirName(m_DirFind.GetDirName(pos));
			sDirName.TrimRight(_T('\\'));

			// Delete folder
			::DeleteToRecycleBin(sDirName, FALSE, GetSafeHwnd());
			
			// Web files copy, Assistant Dialog pop-up and autorun-clear for all devices which save to the above deleted folder
			if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
			{
				for (int i = 0 ; i < m_DevicePathNames.GetSize() ; i++)
				{
					CString sRecordAutoSaveDir = ::AfxGetApp()->GetProfileString(m_DevicePathNames[i], _T("RecordAutoSaveDir"), _T(""));
					CString sDetectionAutoSaveDir = ::AfxGetApp()->GetProfileString(m_DevicePathNames[i], _T("DetectionAutoSaveDir"), _T(""));
					CString sSnapshotAutoSaveDir = ::AfxGetApp()->GetProfileString(m_DevicePathNames[i], _T("SnapshotAutoSaveDir"), _T(""));
					sRecordAutoSaveDir.TrimRight(_T('\\'));
					sDetectionAutoSaveDir.TrimRight(_T('\\'));
					sSnapshotAutoSaveDir.TrimRight(_T('\\'));
					if (sDirName.CompareNoCase(sRecordAutoSaveDir) == 0		||
						sDirName.CompareNoCase(sDetectionAutoSaveDir) == 0	||
						sDirName.CompareNoCase(sSnapshotAutoSaveDir) == 0)
					{
						::AfxGetApp()->WriteProfileInt(m_DevicePathNames[i], _T("RestoreWebFiles"), TRUE);
						CVideoDeviceDoc::AutorunRemoveDevice(m_DevicePathNames[i]);
					}
				}
			}
		}
	}
	EnableWindow(TRUE);
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
