// ZipProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "ZipProgressDlg.h"
#include "YesNoAllCancelDlg.h"
#include "SortableFileFind.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CZipProgressDlg dialog

CZipProgressDlg::CZipProgressDlg(CWnd* pParent, BOOL bExtract)
	: CDialog(CZipProgressDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CZipProgressDlg)
	//}}AFX_DATA_INIT
	m_bExtract = bExtract;
	m_sZipFileName = _T("");
	m_sPath = _T("");
}

void CZipProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CZipProgressDlg)
	DDX_Control(pDX, IDC_ZIP_PROGRESS, m_ZipProgress);
	//}}AFX_DATA_MAP
}

int CZipProgressDlg::CExtractThread::Work() 
{
	// Check
	ASSERT(m_pDlg);

	try
	{
		int nPrevPercentDone = 0;

		// Dialog
		CYesNoAllCancelDlg YesNoAllCancelDlg(m_pDlg);

		// Open Zip File
		((CUImagerApp*)::AfxGetApp())->m_Zip.Open(m_pDlg->m_sZipFileName, CZipArchive::openReadOnly, 0);

		// Extract All
		for (int i = 0 ; i < ((CUImagerApp*)::AfxGetApp())->m_Zip.GetNoEntries() ; i++)
		{
			// Do Exit Thread?
			if (DoExit())
			{
				((CUImagerApp*)::AfxGetApp())->m_Zip.Close();
				return 0;
			}

			// Extract, fail if file exists
			bool res = ((CUImagerApp*)::AfxGetApp())->m_Zip.ExtractFile(i, m_pDlg->m_sPath, true, false);

			// If File Existing
			if (!res)
			{
				// Dialog already called -> Replace All has been selected
				if (YesNoAllCancelDlg.m_nOperation == CYesNoAllCancelDlg::REPLACEALL_FILES)
					res = ((CUImagerApp*)::AfxGetApp())->m_Zip.ExtractFile(i, m_pDlg->m_sPath, false, false);
				// Dialog already called -> Rename All has been selected
				else if (YesNoAllCancelDlg.m_nOperation == CYesNoAllCancelDlg::RENAMEALL_FILES)
					res = ((CUImagerApp*)::AfxGetApp())->m_Zip.ExtractFile(i, m_pDlg->m_sPath, false, true);
				else
				{
					// Get File Info
					CFileHeader fh;
					((CUImagerApp*)::AfxGetApp())->m_Zip.GetFileInfo(fh, (WORD)i);

					// Popup Dialog
					YesNoAllCancelDlg.m_sFileName = ::GetShortFileName(fh.m_szFileName);
					YesNoAllCancelDlg.DoModal();
					if (YesNoAllCancelDlg.m_nOperation == CYesNoAllCancelDlg::REPLACE_FILE ||
						YesNoAllCancelDlg.m_nOperation == CYesNoAllCancelDlg::REPLACEALL_FILES)
						res = ((CUImagerApp*)::AfxGetApp())->m_Zip.ExtractFile(i, m_pDlg->m_sPath, false, false);
					else if (YesNoAllCancelDlg.m_nOperation == CYesNoAllCancelDlg::RENAME_FILE ||
							YesNoAllCancelDlg.m_nOperation == CYesNoAllCancelDlg::RENAMEALL_FILES)
						res = ((CUImagerApp*)::AfxGetApp())->m_Zip.ExtractFile(i, m_pDlg->m_sPath, false, true);
					else // = Cancel Pressed
					{
						((CUImagerApp*)::AfxGetApp())->m_Zip.Close();
						m_pDlg->PostMessage(WM_COMMAND, IDCANCEL);
						return 0;
					}
				}
			}

			// Update Percent Done
			int nPercentDone = i * 100 / ((CUImagerApp*)::AfxGetApp())->m_Zip.GetNoEntries();
			if (nPercentDone > nPrevPercentDone)
			{
				m_pDlg->m_ZipProgress.PostMessage(PBM_SETPOS, (WPARAM)nPercentDone, 0);
				nPrevPercentDone = nPercentDone;
			}
		}

		// Close
		((CUImagerApp*)::AfxGetApp())->m_Zip.Close();
		m_pDlg->PostMessage(WM_COMMAND, IDOK);
		return 0;
	}
	catch (CZipException* e)
	{
		e->ReportZipError();
		e->Delete();
		((CUImagerApp*)::AfxGetApp())->m_Zip.Close(true);
		m_pDlg->PostMessage(WM_COMMAND, IDCANCEL);
		return 0;
	}
	catch (CException* e)
	{
		e->ReportError(MB_ICONSTOP);
		e->Delete();
		((CUImagerApp*)::AfxGetApp())->m_Zip.Close(true);
		m_pDlg->PostMessage(WM_COMMAND, IDCANCEL);
		return 0;
	}
}

int CZipProgressDlg::CCompressThread::Work() 
{
	// Check
	ASSERT(m_pDlg);

	try
	{
		// Only 1x File to Compress?
		if (::IsExistingFile(m_pDlg->m_sPath))
		{
			// Create Zip File
			((CUImagerApp*)::AfxGetApp())->m_Zip.Open(m_pDlg->m_sZipFileName, CZipArchive::create, 0);

			// Add File to Zip File
			if (!((CUImagerApp*)::AfxGetApp())->m_Zip.AddNewFile(m_pDlg->m_sPath, false)) // Do not store path
			{
				((CUImagerApp*)::AfxGetApp())->m_Zip.Close();
				::DeleteFile(m_pDlg->m_sZipFileName);
				m_pDlg->PostMessage(WM_COMMAND, IDCANCEL);
				return 0;
			}
		}
		// Compress Directory Content Recursively
		else
		{
			int pos;
			int nPrevPercentDone = 0;
			CSortableFileFind FileFind;

			// Adjust Directory Path
			m_pDlg->m_sPath.TrimRight(_T('\\'));
		
			// Start Recursive File Find
			if (!FileFind.InitRecursive(m_pDlg->m_sPath + _T("\\*"), FALSE))
			{
				m_pDlg->PostMessage(WM_COMMAND, IDCANCEL);
				return 0;
			}

			// Create Zip File
			((CUImagerApp*)::AfxGetApp())->m_Zip.Open(m_pDlg->m_sZipFileName, CZipArchive::create, 0);

			// Wait that the thread has finished
			if (FileFind.WaitRecursiveDone(GetKillEvent()) != 1)
			{
				((CUImagerApp*)::AfxGetApp())->m_Zip.Close();
				::DeleteFile(m_pDlg->m_sZipFileName);
				if (!DoExit())
					m_pDlg->PostMessage(WM_COMMAND, IDCANCEL);
				return 0;
			}

			// Loop Through Files and Compress them
			for (pos = 0 ; pos < FileFind.GetFilesCount() ; pos++)
			{
				// Do Exit Thread?
				if (DoExit())
				{
					((CUImagerApp*)::AfxGetApp())->m_Zip.Close();
					::DeleteFile(m_pDlg->m_sZipFileName);
					return 0;
				}

				// Source File Name
				CString sSrcFileName = FileFind.GetFileName(pos);
				sSrcFileName.TrimRight(_T('\\'));

				// In Zip File Stored File Path
				CString sSrcStoredFileName = sSrcFileName;
				sSrcStoredFileName = sSrcStoredFileName.Mid(m_pDlg->m_sPath.GetLength() + 1);

				// Add File to Zip File
				if (!((CUImagerApp*)::AfxGetApp())->m_Zip.AddNewFile(sSrcFileName, sSrcStoredFileName))
				{
					((CUImagerApp*)::AfxGetApp())->m_Zip.Close();
					::DeleteFile(m_pDlg->m_sZipFileName);
					m_pDlg->PostMessage(WM_COMMAND, IDCANCEL);
					return 0;
				}

				// Update Percent Done
				int nPercentDone = pos * 100 / FileFind.GetFilesCount();
				if (nPercentDone > nPrevPercentDone)
				{
					m_pDlg->m_ZipProgress.PostMessage(PBM_SETPOS, (WPARAM)nPercentDone, 0);
					nPrevPercentDone = nPercentDone;
				}
			}
		}

		// Close
		((CUImagerApp*)::AfxGetApp())->m_Zip.Close();
		m_pDlg->PostMessage(WM_COMMAND, IDOK);
		return 0;
	}
	catch (CZipException* e)
	{
		e->ReportZipError();
		e->Delete();
		((CUImagerApp*)::AfxGetApp())->m_Zip.Close(true);
		::DeleteFile(m_pDlg->m_sZipFileName);
		m_pDlg->PostMessage(WM_COMMAND, IDCANCEL);
		return 0;
	}
	catch (CException* e)
	{
		e->ReportError(MB_ICONSTOP);
		e->Delete();
		((CUImagerApp*)::AfxGetApp())->m_Zip.Close(true);
		::DeleteFile(m_pDlg->m_sZipFileName);
		m_pDlg->PostMessage(WM_COMMAND, IDCANCEL);
		return 0;
	}
}

BEGIN_MESSAGE_MAP(CZipProgressDlg, CDialog)
	//{{AFX_MSG_MAP(CZipProgressDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZipProgressDlg message handlers

BOOL CZipProgressDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_ZipProgress.SetRange(0, 100);

	if (m_bExtract)
	{
		SetWindowText(ML_STRING(1457, "Zip Extraction Progress"));
		m_ExtractThread.SetDlg(this);
		m_ExtractThread.Start();
	}
	else
	{
		SetWindowText(ML_STRING(1458, "Zip Compression Progress"));
		m_CompressThread.SetDlg(this);
		m_CompressThread.Start();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CZipProgressDlg::OnCancel() 
{
	m_ExtractThread.Kill();
	m_CompressThread.Kill();
	CDialog::OnCancel();
}
