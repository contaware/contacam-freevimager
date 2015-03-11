// CDAudioDoc.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "CDAudioDoc.h"
#include "CDAudioView.h"
#include "ToolBarChildFrm.h"
#include "CDAudio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCDAudioDoc

IMPLEMENT_DYNCREATE(CCDAudioDoc, CDocument)

BEGIN_MESSAGE_MAP(CCDAudioDoc, CDocument)
	//{{AFX_MSG_MAP(CCDAudioDoc)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CCDAudioDoc::CCDAudioDoc()
{
	// The Only View and Frame
	m_pView = NULL;
	m_pFrame = NULL;
	m_pCDAudio = NULL;
	m_nFileMenuPos = -2;
	m_nEditMenuPos = -2;
	m_nViewMenuPos = -2;
	m_nCaptureMenuPos = -2;
	m_nPlayMenuPos = -2;
	m_nSettingsMenuPos = -2;
	m_nWindowsPos = -2;
	m_nHelpMenuPos = -2;
}

BOOL CCDAudioDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CCDAudioDoc::~CCDAudioDoc()
{
	if (m_pCDAudio)
	{
		delete m_pCDAudio;
		m_pCDAudio = NULL;
	}
}

void CCDAudioDoc::CloseDocument()
{
	GetFrame()->PostMessage(WM_CLOSE, 0, 0);
}

void CCDAudioDoc::CloseDocumentForce()
{
	SetModifiedFlag(FALSE);
	GetFrame()->PostMessage(WM_CLOSE, 0, 0);
}

BOOL CCDAudioDoc::LoadCD(LPCTSTR lpszFileName)
{
	// Check
	if (!::IsExistingFile(lpszFileName))
		return FALSE;

	if (m_pCDAudio)
	{
		delete m_pCDAudio;
		m_pCDAudio = NULL;
	}
	m_pCDAudio = new CCDAudio(lpszFileName);
	if (!m_pCDAudio || (m_pCDAudio->GetErrorCode() != 0))
		return FALSE;
	else
	{
		SetPathName(lpszFileName, TRUE);
		return TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCDAudioDoc serialization

void CCDAudioDoc::Serialize(CArchive& ar)
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

/////////////////////////////////////////////////////////////////////////////
// CCDAudioDoc commands

void CCDAudioDoc::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(FALSE);	
}

void CCDAudioDoc::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(FALSE);	
}

/////////////////////////////////////////////////////////////////////////////
// CCDAudioDoc diagnostics

#ifdef _DEBUG
void CCDAudioDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCDAudioDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG
