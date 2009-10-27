// RemoteCamViewerView.cpp : implementation of the CRemoteCamViewerView class
//

#include "stdafx.h"
#include "RemoteCamViewer.h"
#include "RemoteCamViewerDoc.h"
#include "RemoteCamViewerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerView

IMPLEMENT_DYNCREATE(CRemoteCamViewerView, CFormView)

BEGIN_MESSAGE_MAP(CRemoteCamViewerView, CFormView)
	//{{AFX_MSG_MAP(CRemoteCamViewerView)
	ON_COMMAND(ID_HELP_ABOUTCTRL, OnHelpAboutctrl)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_COMMAND(ID_TOOLS_SHOWPROP, OnToolsShowprop)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CFormView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerView construction/destruction

CRemoteCamViewerView::CRemoteCamViewerView()
	: CFormView(CRemoteCamViewerView::IDD)
{
	//{{AFX_DATA_INIT(CRemoteCamViewerView)
	//}}AFX_DATA_INIT
	// TODO: add construction code here

}

CRemoteCamViewerView::~CRemoteCamViewerView()
{
}

void CRemoteCamViewerView::DoDataExchange(CDataExchange* pDX)
{
	try
	{
		CFormView::DoDataExchange(pDX);
		//{{AFX_DATA_MAP(CRemoteCamViewerView)
		DDX_Control(pDX, IDC_REMOTECAMCTRL, m_RemoteCam);
		//}}AFX_DATA_MAP
	}
	catch (CException* e)
	{
		e->Delete();
		::AfxMessageBox(_T("ActiveX is not registered!"), MB_OK | MB_ICONERROR);
	}
}

BOOL CRemoteCamViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CRemoteCamViewerView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// For resizing
	CRect rc;
	if (::IsWindow(m_RemoteCam.m_hWnd))
	{
		m_RemoteCam.GetClientRect(&rc);
		m_szRemoteCamSize = rc.Size();
	}
	SetTimer(1, 500, NULL);

	// For flicker free window painting (resizing)
	ModifyStyle(NULL, WS_CLIPCHILDREN, 0);
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerView printing

BOOL CRemoteCamViewerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CRemoteCamViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CRemoteCamViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CRemoteCamViewerView::OnPrint(CDC* pDC, CPrintInfo* /*pInfo*/)
{
	// TODO: add customized printing code here
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerView diagnostics

#ifdef _DEBUG
void CRemoteCamViewerView::AssertValid() const
{
	CFormView::AssertValid();
}

void CRemoteCamViewerView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CRemoteCamViewerDoc* CRemoteCamViewerView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRemoteCamViewerDoc)));
	return (CRemoteCamViewerDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerView message handlers

void CRemoteCamViewerView::OnHelpAboutctrl() 
{
	if (::IsWindow(m_RemoteCam.m_hWnd))
		m_RemoteCam.AboutBox();
}

void CRemoteCamViewerView::OnToolsShowprop() 
{
	if (::IsWindow(m_RemoteCam.m_hWnd))
		m_RemoteCam.PropertyPage(GetSafeHwnd());
}

void CRemoteCamViewerView::OnTimer(UINT nIDEvent) 
{
	CRect rc;
	if (::IsWindow(m_RemoteCam.m_hWnd))
	{
		m_RemoteCam.GetClientRect(&rc);
		if (rc.Size() != m_szRemoteCamSize)
		{
			m_szRemoteCamSize = rc.Size();
			SetScrollSizes(MM_TEXT, rc.Size());
		}
	}
	CFormView::OnTimer(nIDEvent);
}

void CRemoteCamViewerView::OnDestroy() 
{
	CFormView::OnDestroy();
	KillTimer(1);	
}
