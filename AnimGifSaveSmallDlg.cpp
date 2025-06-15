// AnimGifSaveSmallDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "AnimGifSaveSmallDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAnimGifSaveSmallDlg dialog


CAnimGifSaveSmallDlg::CAnimGifSaveSmallDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAnimGifSaveSmallDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnimGifSaveSmallDlg)
	m_uiPlayTimes = 1;
	m_nLoopInfinite = 1;
	//}}AFX_DATA_INIT
	m_pDelaysArray = NULL;
}


void CAnimGifSaveSmallDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimGifSaveSmallDlg)
	DDX_Control(pDX, IDC_LIST_DELAYS, m_ListDelays);
	DDX_Text(pDX, IDC_EDIT_PLAY_TIMES, m_uiPlayTimes);
	DDV_MinMaxUInt(pDX, m_uiPlayTimes, 1, 65536);
	DDX_Radio(pDX, IDC_RADIO_LOOP, m_nLoopInfinite);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnimGifSaveSmallDlg, CDialog)
	//{{AFX_MSG_MAP(CAnimGifSaveSmallDlg)
	ON_BN_CLICKED(IDC_RADIO_LOOP, OnRadioLoop)
	ON_BN_CLICKED(IDC_RADIO_LOOP_INFINITE, OnRadioLoopInfinite)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimGifSaveSmallDlg message handlers

BOOL CAnimGifSaveSmallDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PLAY_TIMES);
	CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PLAY_TIMES);
	pSpin->SetRange32(1, 65536);

	if (m_nLoopInfinite == 1)
	{
		pEdit->EnableWindow(FALSE);
		pSpin->EnableWindow(FALSE);
	}

	if (m_pDelaysArray)
	{
		m_ListDelays.SetExtendedStyle(m_ListDelays.GetExtendedStyle());
		m_ListDelays.InsertColumn(0, _T("Delay (ms)"), LVCFMT_LEFT, 80);
		m_ListDelays.InsertColumn(1, _T("Frame"), LVCFMT_LEFT, 60);
		for (int i = 0 ; i < m_pDelaysArray->GetSize() ; i++)
		{
			CString t;
			LVITEM lvItem;

			// Frame's Delay
			t.Format(_T("%d"), m_pDelaysArray->GetAt(i));
			lvItem.iItem = i;
			lvItem.mask = LVIF_TEXT;
			lvItem.iSubItem = 0;
			lvItem.pszText = t.GetBuffer(t.GetLength());
			m_ListDelays.InsertItem(&lvItem);
			t.ReleaseBuffer();

			// Frame Number
			t.Format(_T("%d"), i+1);
			lvItem.iItem = i;
			lvItem.mask = LVIF_TEXT;
			lvItem.iSubItem = 1;
			lvItem.pszText = t.GetBuffer(t.GetLength());
			m_ListDelays.SetItem(&lvItem);
			t.ReleaseBuffer();
		}
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAnimGifSaveSmallDlg::OnRadioLoop() 
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PLAY_TIMES);
	pEdit->EnableWindow(TRUE);

	CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PLAY_TIMES);
	pSpin->EnableWindow(TRUE);
}

void CAnimGifSaveSmallDlg::OnRadioLoopInfinite() 
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PLAY_TIMES);
	pEdit->EnableWindow(FALSE);

	CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_PLAY_TIMES);
	pSpin->EnableWindow(FALSE);
}

void CAnimGifSaveSmallDlg::OnOK() 
{
	for (int i = 0 ; i < m_pDelaysArray->GetSize() ; i++)
	{
		CString t;
		LVITEM lvItem;
		ZeroMemory(&lvItem, sizeof(LV_ITEM));
		lvItem.iItem        = i;
		lvItem.iSubItem     = 0;
		lvItem.mask			= LVIF_TEXT;
		lvItem.stateMask	= (UINT)-1;
		lvItem.pszText		= t.GetBuffer(CListCtrlEx::MAX_LABEL_SIZE + 1);
		lvItem.cchTextMax	= CListCtrlEx::MAX_LABEL_SIZE + 1;
		m_ListDelays.GetItem(&lvItem);
		if (lvItem.pszText)
		{
			int nValue = _ttoi(lvItem.pszText);
			t.ReleaseBuffer();
			if (nValue > 0)
				m_pDelaysArray->SetAt(i, nValue);
		}
		else
			t.ReleaseBuffer();
	}
	
	CDialog::OnOK();
}