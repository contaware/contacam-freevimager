#include "stdafx.h"
#include "CaptureDeviceDlg.h"
#include "VideoDeviceDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

CCaptureDeviceDlg::CCaptureDeviceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCaptureDeviceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCaptureDeviceDlg)
	//}}AFX_DATA_INIT
	m_wDeviceID = 0;
}


void CCaptureDeviceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCaptureDeviceDlg)
	DDX_Control(pDX, IDC_DEVICES, m_ctrlDevices);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		int nIndex = m_ctrlDevices.GetCurSel();
		m_wDeviceID = (WORD) m_ctrlDevices.GetItemData(nIndex);
	}
}


BEGIN_MESSAGE_MAP(CCaptureDeviceDlg, CDialog)
	//{{AFX_MSG_MAP(CCaptureDeviceDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CCaptureDeviceDlg::OnInitDialog() 
{
	// Let the parent class do its thing
	CDialog::OnInitDialog();

	// Add the capture devices to the list box
	for (WORD i = 0 ; i < 10 ; i++)
	{
		CString sName, sVersion;
		if (CVideoDeviceDoc::GetCaptureDriverDescription(i, sName, sVersion))
		{
			CString sText;
			if (sVersion.GetLength())
				sText.Format(_T("%s\t%s"), sName, sVersion);
			else
				sText = sName;
			int nIndex = m_ctrlDevices.AddString(sText);
			m_ctrlDevices.SetItemData(nIndex, i);
		}
	}

	// Now set the selection to the first item
	m_ctrlDevices.SetCurSel(0);
	
	return TRUE;
}

#endif
