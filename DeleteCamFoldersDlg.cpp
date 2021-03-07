// DeleteCamFoldersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "ToolBarChildFrm.h"
#include "VideoDeviceDoc.h"
#include "HostPortDlg.h"
#include "DeleteCamFoldersDlg.h"
#include "PostDelayedMessage.h"

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

BOOL CDeleteCamFoldersDlg::IsCamClosed(CString sFolderPath)
{
	// Note: if miss-configured there could be more than one camera
	//       running per folder, this function checks all of them!
	sFolderPath.TrimRight(_T('\\'));
	CUImagerMultiDocTemplate* pVideoDeviceDocTemplate = ((CUImagerApp*)::AfxGetApp())->GetVideoDeviceDocTemplate();
	POSITION posVideoDeviceDoc = pVideoDeviceDocTemplate->GetFirstDocPosition();
	while (posVideoDeviceDoc)
	{
		CVideoDeviceDoc* pVideoDeviceDoc = (CVideoDeviceDoc*)(pVideoDeviceDocTemplate->GetNextDoc(posVideoDeviceDoc));
		if (pVideoDeviceDoc)
		{
			CString sRecordAutoSaveDir = pVideoDeviceDoc->m_sRecordAutoSaveDir;
			sRecordAutoSaveDir.TrimRight(_T('\\'));
			if (sFolderPath.CompareNoCase(sRecordAutoSaveDir) == 0)
				return FALSE;
		}
	}
	return TRUE;
}

void CDeleteCamFoldersDlg::CloseCam(CString sFolderPath)
{
	// Note: if miss-configured there could be more than one camera
	//       running per folder, this function closes all of them!
	sFolderPath.TrimRight(_T('\\'));
	CUImagerMultiDocTemplate* pVideoDeviceDocTemplate = ((CUImagerApp*)::AfxGetApp())->GetVideoDeviceDocTemplate();
	POSITION posVideoDeviceDoc = pVideoDeviceDocTemplate->GetFirstDocPosition();
	while (posVideoDeviceDoc)
	{
		CVideoDeviceDoc* pVideoDeviceDoc = (CVideoDeviceDoc*)(pVideoDeviceDocTemplate->GetNextDoc(posVideoDeviceDoc));
		if (pVideoDeviceDoc)
		{
			CString sRecordAutoSaveDir = pVideoDeviceDoc->m_sRecordAutoSaveDir;
			sRecordAutoSaveDir.TrimRight(_T('\\'));
			if (sFolderPath.CompareNoCase(sRecordAutoSaveDir) == 0)
				pVideoDeviceDoc->GetFrame()->PostMessage(WM_CLOSE, 0, 0);
		}
	}
}

void CDeleteCamFoldersDlg::EnableDisableAllCtrls(BOOL bEnable)
{
	CWnd* pwndChild = GetWindow(GW_CHILD);
	while (pwndChild)
	{
		// Note: close X button, Esc and Alt+F4 are working according to the
		//       IDCANCEL button state which is Enabled/Disabled in this loop
		TCHAR szClassName[8]; // one extra char to make sure it is exactly "Static"
		::GetClassName(pwndChild->GetSafeHwnd(), szClassName, 8);
		szClassName[7] = _T('\0');
		if (_tcsicmp(szClassName, _T("Static")) != 0)
			pwndChild->EnableWindow(bEnable);
		pwndChild = pwndChild->GetNextWindow();
	}
}

BEGIN_MESSAGE_MAP(CDeleteCamFoldersDlg, CDialog)
	//{{AFX_MSG_MAP(CDeleteCamFoldersDlg)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BUTTON_LIST_SELECTALL, OnButtonListSelectall)
	ON_BN_CLICKED(IDC_BUTTON_LIST_SELECTNONE, OnButtonListSelectnone)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_APPLY_CAMS_DELETION, OnApplyDeletion)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeleteCamFoldersDlg message handlers

BOOL CDeleteCamFoldersDlg::OnInitDialog() 
{
	// This calls UpdateData(FALSE) which transfers data
	// into the dialog window from the member variables
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
	((CUImagerApp*)::AfxGetApp())->EnumConfiguredDevicePathNames(m_DevicePathNames);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LONG CDeleteCamFoldersDlg::OnApplyDeletion(WPARAM wparam, LPARAM lparam)
{
	for (int pos = 0 ; pos < m_DirFind.GetDirsCount() ; pos++)
	{
		if (m_CamFolders.GetSel(pos) > 0)
		{
			CString sDirName(m_DirFind.GetDirName(pos));
			sDirName.TrimRight(_T('\\'));

			// Make sure camera is not running
			if (!IsCamClosed(sDirName))
			{
				// Note that's not a problem if we call CloseCam() many
				// times, the device close logic itself is to call
				// CVideoDeviceChildFrame::OnClose() many times
				CloseCam(sDirName);
				CPostDelayedMessageThread::PostDelayedMessage(GetSafeHwnd(), WM_APPLY_CAMS_DELETION, 1000U, 0, 0);
				return 0;
			}

			// Delete folder
			if (::IsExistingDir(sDirName))
				::DeleteToRecycleBin(sDirName);
			
			// Clear autorun and delete device configuration
			if (!::IsExistingDir(sDirName)) // make sure dir has been deleted
			{
				for (int i = 0 ; i < m_DevicePathNames.GetSize() ; i++)
				{
					CString sRecordAutoSaveDir = ::AfxGetApp()->GetProfileString(m_DevicePathNames[i], _T("RecordAutoSaveDir"), _T(""));
					sRecordAutoSaveDir.TrimRight(_T('\\'));
					if (sDirName.CompareNoCase(sRecordAutoSaveDir) == 0)
					{
						CVideoDeviceDoc::AutorunRemoveDevice(m_DevicePathNames[i]);
						if (::AfxGetApp()->m_pszRegistryKey)
							::DeleteRegistryKey(HKEY_CURRENT_USER,	_T("Software\\") +
																	CString(MYCOMPANY) + CString(_T("\\")) +
																	CString(APPNAME_NOEXT) + _T("\\") +
																	m_DevicePathNames[i]);
						else
							::WritePrivateProfileString(m_DevicePathNames[i], NULL, NULL, ::AfxGetApp()->m_pszProfileName);
					}
				}
			}
		}
	}
	EndWaitCursor();
	EndDialog(IDOK);
	return 0;
}

BOOL CDeleteCamFoldersDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// If Wait Cursor leave it!
	if (((CUImagerApp*)::AfxGetApp())->IsWaitCursor())
	{
		RestoreWaitCursor();
		return TRUE;
	}
	else
		return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CDeleteCamFoldersDlg::OnOK() 
{
	// This transfers data from the dialog window to
	// the member variables validating it
	if (!UpdateData(TRUE))
		return;

	// Check
	// Note: the following alert is the same as when
	//       clicking outside a modal dialog
	if (m_CamFolders.GetSelCount() < 1)
	{
		::AlertUser(GetSafeHwnd());
		return;
	}

	// Prompt
	if (::AfxMessageBox(ML_STRING(1807, "For the selected cameras:\n\n1. Settings will be permanently deleted.\n2. Recordings are moved to the Recycle Bin.\n\nDo You really want to PROCEED?"),
						MB_YESNO | MB_DEFBUTTON2) == IDNO)
		return;

	// Begin wait cursor
	BeginWaitCursor();

	// Disable all
	EnableDisableAllCtrls(FALSE);

	// Start deletion
	CPostDelayedMessageThread::PostDelayedMessage(GetSafeHwnd(), WM_APPLY_CAMS_DELETION, 500U, 0, 0);
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
