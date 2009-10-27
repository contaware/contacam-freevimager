// PrintPreviewScaleEdit.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "PrintPreviewScaleEdit.h"
#include "NumSpinCtrl.h"
#include "PictureDoc.h"
#include "PicturePrintPreviewView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPrintPreviewScaleEdit

CPrintPreviewScaleEdit::CPrintPreviewScaleEdit(	CPictureDoc* pDoc,
												CPicturePrintPreviewView* pPrintPreview)
{
	m_pDoc = pDoc;
	m_pPrintPreview = pPrintPreview;
	m_bUpdate = TRUE;
}

CPrintPreviewScaleEdit::~CPrintPreviewScaleEdit()
{

}

BOOL CPrintPreviewScaleEdit::SetPrintScale(double dPrintScale)
{
	if ((dPrintScale * 100.0) >= MIN_PRINT_SCALE &&
		(dPrintScale * 100.0) <= MAX_PRINT_SCALE)
	{
		m_bUpdate = FALSE;

		SetValue(dPrintScale * 100.0);

		m_bUpdate = TRUE;

		return TRUE;
	}
	else
		return FALSE;
}

BEGIN_MESSAGE_MAP(CPrintPreviewScaleEdit, CEdit)
	//{{AFX_MSG_MAP(CPrintPreviewScaleEdit)
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrintPreviewScaleEdit message handlers

void CPrintPreviewScaleEdit::OnChange() 
{
	if (m_bUpdate)
	{
		double dValue = GetValue();
		if (dValue >= MIN_PRINT_SCALE && dValue <= MAX_PRINT_SCALE)
		{
			m_pDoc->m_dPrintScale = dValue / 100.0;
			m_pDoc->m_bPrintSizeFit = FALSE;
			m_pPrintPreview->Invalidate(FALSE);
			CButton* pCheckPrintSizeFit = (CButton*)m_pPrintPreview->GetToolBar()->GetDlgItem(IDC_CHECK_SIZE_FIT);
			pCheckPrintSizeFit->SetCheck(0);
		}
	}
}
