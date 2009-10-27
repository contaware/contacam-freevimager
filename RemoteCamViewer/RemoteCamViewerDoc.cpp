// RemoteCamViewerDoc.cpp : implementation of the CRemoteCamViewerDoc class
//

#include "stdafx.h"
#include "RemoteCamViewer.h"
#include "RemoteCamViewerDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerDoc

IMPLEMENT_DYNCREATE(CRemoteCamViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CRemoteCamViewerDoc, CDocument)
	//{{AFX_MSG_MAP(CRemoteCamViewerDoc)
	ON_COMMAND(ID_TOOLS_REGISTER, OnToolsRegister)
	ON_COMMAND(ID_TOOLS_UNREGISTER, OnToolsUnregister)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerDoc construction/destruction

CRemoteCamViewerDoc::CRemoteCamViewerDoc()
{
	// TODO: add one-time construction code here

}

CRemoteCamViewerDoc::~CRemoteCamViewerDoc()
{
}

BOOL CRemoteCamViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	SetTitle(_T("Contaware"));
	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerDoc serialization

void CRemoteCamViewerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerDoc diagnostics

#ifdef _DEBUG
void CRemoteCamViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CRemoteCamViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CRemoteCamViewerDoc commands

void CRemoteCamViewerDoc::OnToolsRegister() 
{
	if (((CRemoteCamViewerApp*)::AfxGetApp())->Register())
		::AfxMessageBox(_T("Registration succeeded: restart the program!"), MB_OK | MB_ICONINFORMATION);
}

void CRemoteCamViewerDoc::OnToolsUnregister() 
{
	if (((CRemoteCamViewerApp*)::AfxGetApp())->Unregister())
		::AfxMessageBox(_T("Unregistration succeeded."), MB_OK | MB_ICONINFORMATION);
}
