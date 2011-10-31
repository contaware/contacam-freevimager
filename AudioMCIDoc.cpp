// AudioMCIDoc.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "AudioMCIDoc.h"
#include "AudioMCIView.h"
#include "ToolBarChildFrm.h"
#include "RenameDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MCIPLAY_POLL_TIMER_DELAY	1000		// ms

/////////////////////////////////////////////////////////////////////////////
// CAudioMCIDoc

IMPLEMENT_DYNCREATE(CAudioMCIDoc, CDocument)

BEGIN_MESSAGE_MAP(CAudioMCIDoc, CDocument)
	//{{AFX_MSG_MAP(CAudioMCIDoc)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_COMMAND(ID_EDIT_RENAME, OnEditRename)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CAudioMCIDoc::CAudioMCIDoc()
{
	// The Only View and Frame
	m_pView = NULL;
	m_pFrame = NULL;
	m_uiTimerId = 0U;
	m_nFileMenuPos = -2;
	m_nEditMenuPos = -2;
	m_nViewMenuPos = -2;
	m_nCaptureMenuPos = -2;
	m_nPlayMenuPos = -2;
	m_nToolsMenuPos = -2;
	m_nWindowsPos = -2;
	m_nHelpMenuPos = -2;
}

BOOL CAudioMCIDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CAudioMCIDoc::~CAudioMCIDoc()
{

}

void CAudioMCIDoc::CloseDocument()
{
	GetFrame()->PostMessage(WM_CLOSE, 0, 0);
}

void CAudioMCIDoc::CloseDocumentForce()
{
	SetModifiedFlag(FALSE);
	GetFrame()->PostMessage(WM_CLOSE, 0, 0);
}

BOOL CAudioMCIDoc::LoadAudio(LPCTSTR lpszFileName)
{
	// Check
	if (!::IsExistingFile(lpszFileName))
		return FALSE;

	// Kill Timer
	if (m_uiTimerId)
	{
		GetView()->KillTimer(m_uiTimerId);
		m_uiTimerId = 0U;
	}

	// Load Audio
	if (!GetView()->m_DibStatic.Load(	lpszFileName,									// File Name to Load
										FALSE,
										TRUE,
										TRUE,
										0,												// client width is taken
										0,												// client height is taken
										((CUImagerApp*)::AfxGetApp())->m_bStartPlay))	// Start playing audio file
	{
		((CUImagerApp*)::AfxGetApp())->m_bStartPlay = FALSE;
		return FALSE;
	}
	else
	{
		((CUImagerApp*)::AfxGetApp())->m_bStartPlay = FALSE;
		SetPathName(lpszFileName, TRUE);
		m_uiTimerId = GetView()->SetTimer(ID_TIMER_MCIAUDIO, MCIPLAY_POLL_TIMER_DELAY, NULL);
		return TRUE;
	}
}

void CAudioMCIDoc::EditDelete(BOOL bPrompt)
{
	if (bPrompt)
	{
		CString sMsg;
		sMsg.Format(ML_STRING(1266, "Are you sure you want to delete %s ?"), GetPathName());
		if (::AfxMessageBox(sMsg, MB_YESNO) == IDYES)
			DeleteDocFile();
	}
	else
		DeleteDocFile();
}

void CAudioMCIDoc::DeleteDocFile()
{
	// Store File Name
	CString sFileNameToDelete = GetPathName();

	if (::IsReadonly(sFileNameToDelete))
	{
		CString str;
		str.Format(ML_STRING(1250, "The file %s\nis read only"), sFileNameToDelete);
		::AfxMessageBox(str, MB_ICONSTOP);
		return;
	}

	// Destroy MCI Wnd
	GetView()->m_DibStatic.FreeMusic();

	// Delete It
	if (!::DeleteToRecycleBin(sFileNameToDelete))
	{
		CString str;
		str.Format(ML_STRING(1267, "Cannot delete the %s file"), sFileNameToDelete);
		::AfxMessageBox(str, MB_ICONSTOP);
	}
	CloseDocument();
}

void CAudioMCIDoc::EditRename()
{
	CRenameDlg dlg;
	dlg.m_sFileName = ::GetShortFileNameNoExt(GetPathName());
	if (dlg.DoModal() == IDOK)
	{	
		// New file name
		CString sNewFileName =	::GetDriveName(GetPathName()) +
								::GetDirName(GetPathName()) +
								dlg.m_sFileName +
								::GetFileExt(GetPathName());
		
		// Destroy MCI Wnd
		GetView()->m_DibStatic.FreeMusic();

		// Rename
		if (!::MoveFile(GetPathName(), sNewFileName))
		{
			::ShowLastError(TRUE);
			sNewFileName = GetPathName();
		}
		
		// Reload
		LoadAudio(sNewFileName);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CAudioMCIDoc serialization

void CAudioMCIDoc::Serialize(CArchive& ar)
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
// CAudioMCIDoc commands

void CAudioMCIDoc::OnEditDelete() 
{
	EditDelete(TRUE);	
}

void CAudioMCIDoc::OnEditRename() 
{
	EditRename();
}

void CAudioMCIDoc::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(FALSE);
}

void CAudioMCIDoc::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CAudioMCIDoc diagnostics

#ifdef _DEBUG
void CAudioMCIDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CAudioMCIDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG
