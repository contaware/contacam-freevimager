// AudioMCIView.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "AudioMCIView.h"
#include "AudioMCIDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAudioMCIView

IMPLEMENT_DYNCREATE(CAudioMCIView, CFormView)

CAudioMCIView::CAudioMCIView()
	: CFormView(CAudioMCIView::IDD)
{
	//{{AFX_DATA_INIT(CAudioMCIView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CAudioMCIView::~CAudioMCIView()
{
}

void CAudioMCIView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAudioMCIView)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAudioMCIView, CFormView)
	//{{AFX_MSG_MAP(CAudioMCIView)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAudioMCIView diagnostics

#ifdef _DEBUG
void CAudioMCIView::AssertValid() const
{
	CFormView::AssertValid();
}

void CAudioMCIView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CAudioMCIDoc* CAudioMCIView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAudioMCIDoc)));
	return (CAudioMCIDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAudioMCIView message handlers

void CAudioMCIView::OnInitialUpdate() 
{
	CAudioMCIDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Set Pointers
	pDoc->SetView(this);
	pDoc->SetFrame((CAudioMCIChildFrame*)GetParentFrame());

	CFormView::OnInitialUpdate();
	
	m_DibStatic.SubclassDlgItem(IDC_DIBSTATIC, this);

	// Resize to dialog template size. Call ResizeParentToFit()
	// two times because if the mainframe is really small
	// ResizeParentToFit() is not working the first time...
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit(FALSE);
	ResizeParentToFit(FALSE);
}

void CAudioMCIView::OnTimer(UINT nIDEvent) 
{
	CAudioMCIDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (((CUImagerApp*)::AfxGetApp())->m_bCloseAfterAudioPlayDone &&
		!m_DibStatic.IsMusicPlaying())
		::AfxGetMainFrame()->PostMessage(WM_CLOSE, 0, 0);
	CFormView::OnTimer(nIDEvent);
}

BOOL CAudioMCIView::PreTranslateMessage(MSG* pMsg) 
{
	CAudioMCIDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	BOOL bRemoveFocusFromMCISlider = FALSE;

	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			if (::GetKeyState(VK_SHIFT) < 0)
				::AfxGetMainFrame()->PostMessage(WM_CLOSE, 0, 0);
			else
				GetParentFrame()->PostMessage(WM_CLOSE, 0, 0);
			return TRUE;
		}
		else if (pMsg->wParam ==  VK_DELETE)
		{
			if (::GetKeyState(VK_CONTROL) < 0)
				pDoc->EditDelete(FALSE);// Delete without prompting
			else
				pDoc->EditDelete(TRUE);	// Delete with prompting
			return TRUE;
		}
	}
	else if (pMsg->message == WM_LBUTTONDOWN ||
			pMsg->message == WM_LBUTTONUP)
		bRemoveFocusFromMCISlider = TRUE;
	else if (pMsg->message == WM_MOUSEWHEEL)
	{
		return TRUE;	// Do Not Dispatch otherwise it will
						// bring the MCI slider out of sync.!
	}

	BOOL res = CFormView::PreTranslateMessage(pMsg);

	if (bRemoveFocusFromMCISlider)
		SetFocus();

	return res;
}
