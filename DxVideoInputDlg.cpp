// DxVideoInputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "DxVideoInputDlg.h"
#include "VideoDeviceDoc.h"
#include "VideoDeviceView.h"
#include "DxCapture.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef VIDEODEVICEDOC

/////////////////////////////////////////////////////////////////////////////
// CDxVideoInputDlg dialog


CDxVideoInputDlg::CDxVideoInputDlg(CVideoDeviceDoc* pDoc)
	: CDialog(CDxVideoInputDlg::IDD, NULL)
{
	//{{AFX_DATA_INIT(CDxVideoInputDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	ASSERT_VALID(pDoc);
	m_pDoc = pDoc;
}


void CDxVideoInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDxVideoInputDlg)
	DDX_Control(pDX, IDC_COMBO_VIDEOINPUT, m_VideoInput);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDxVideoInputDlg, CDialog)
	//{{AFX_MSG_MAP(CDxVideoInputDlg)
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDxVideoInputDlg message handlers

BOOL CDxVideoInputDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	int nInputsCount = 0;
	if (m_pDoc->m_pDxCapture)
		nInputsCount = m_pDoc->m_pDxCapture->EnumInputs(m_Inputs);
	NumberDuplicateNames();

	for (int nInput = 0 ; nInput < nInputsCount ; nInput++)
		m_VideoInput.AddString(m_Inputs[nInput]);

	int nCurrentInput = -1;
	if (m_pDoc->m_pDxCapture)
		nCurrentInput = m_pDoc->m_pDxCapture->GetCurrentInputID();

	if (nCurrentInput >= 0 && nCurrentInput <= m_Inputs.GetUpperBound())
		m_VideoInput.SetCurSel(nCurrentInput);
	else
		m_VideoInput.SetCurSel(0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDxVideoInputDlg::IsAlreadyInInputs(int nIndex)
{
	// Check
	if (nIndex < 0 || nIndex >= m_Inputs.GetSize())
		return FALSE;
	
	for (int i = 0 ; i < nIndex ; i++)
	{
		if (m_Inputs[i] == m_Inputs[nIndex])
			return TRUE;
	}

	return FALSE;
}

void CDxVideoInputDlg::NumberDuplicateNames()
{
	for (int i = 0 ; i < m_Inputs.GetSize() ; i++)
	{
		CString t;
		int k = 2;
		CString sInput = m_Inputs[i];
		while (IsAlreadyInInputs(i))
		{
			t.Format(_T("_%d"), k++);
			m_Inputs[i] = sInput + t;
		}
	}
}

void CDxVideoInputDlg::Apply()
{
	if (m_pDoc->m_pDxCapture)
	{
		if (m_pDoc->m_pDxCapture->SetCurrentInput(m_VideoInput.GetCurSel()))
			m_pDoc->m_nDeviceInputId = m_VideoInput.GetCurSel();
		else
			m_pDoc->m_nDeviceInputId = -1;
	}
}

void CDxVideoInputDlg::OnOK() 
{
	Apply();
	CDialog::OnOK();
}

void CDxVideoInputDlg::OnApply() 
{
	Apply();
}

#endif
