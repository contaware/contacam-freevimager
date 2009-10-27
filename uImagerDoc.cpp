#include "stdafx.h"
#include "uImager.h"
#include "MainFrm.h"
#include "uImagerDoc.h"
#include "uImagerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUImagerDoc

IMPLEMENT_DYNCREATE(CUImagerDoc, CDocument)

BEGIN_MESSAGE_MAP(CUImagerDoc, CDocument)
	//{{AFX_MSG_MAP(CUImagerDoc)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUImagerDoc construction/destruction

CUImagerDoc::CUImagerDoc()
{
	m_nFileMenuPos = -2;
	m_nEditMenuPos = -2;
	m_nViewMenuPos = -2;
	m_nCaptureMenuPos = -2;
	m_nPlayMenuPos = -2;
	m_nToolsMenuPos = -2;
	m_nWindowsPos = -2;
	m_nHelpMenuPos = -2;
	m_bClosing = FALSE;
	m_sFileName = _T("");
	m_DocRect = CRect(0,0,0,0);
	m_dZoomFactor = 1.0;
	m_crBackgroundColor = RGB(0,0,0);
	m_bNoBorders = TRUE;

	// Init Dib Critical Section
	::InitializeCriticalSection(&m_csDib);

	// New Dib
	m_pDib = new CDib;
}

CUImagerDoc::~CUImagerDoc()
{	
	// Free
	if (m_pDib)
	{
		delete m_pDib;
		m_pDib = NULL;
	}

	// Delete Dib Critical Section
	::DeleteCriticalSection(&m_csDib);
}

BOOL CUImagerDoc::OnNewDocument() 
{	
	return CDocument::OnNewDocument();
}

/////////////////////////////////////////////////////////////////////////////
// CUImagerDoc serialization

void CUImagerDoc::Serialize(CArchive& ar)
{
	// Serialization Not Used For This Project
	/*
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
	*/
}

// Necessary when Print Preview is enabled and we are editing HLS, grayscale, ...
void CUImagerDoc::InvalidateAllViews(BOOL bErase/*=TRUE*/)
{
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		pView->Invalidate(bErase);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CUImagerDoc commands

void CUImagerDoc::OnEditCopy() 
{
	::EnterCriticalSection(&m_csDib);
	if (m_pDib && m_pDib->IsValid())
		m_pDib->EditCopy();
	::LeaveCriticalSection(&m_csDib);
}

void CUImagerDoc::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CUImagerDoc diagnostics

#ifdef _DEBUG
void CUImagerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CUImagerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG
