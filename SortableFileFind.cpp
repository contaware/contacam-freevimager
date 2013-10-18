//
// SortableFileFind.cpp
//

#include "stdafx.h"
#include "SortableFileFind.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL CSortableFileFind::Init(const CString& strName, BOOL bDoSort/*=TRUE*/)
{
	m_sFileName = _T("");
	m_sDirName = _T("");
	Close();

	// Check
	if (strName == _T(""))
		return FALSE;

	// Set Root Dir Name
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	_tsplitpath(strName, szDrive, szDir, NULL, NULL);
	m_sRootDirName = CString(szDrive) + CString(szDir);

	// Start File Search
	WIN32_FIND_DATA Info;
	HANDLE hFileSearch = ::FindFirstFile(strName, &Info);
    if (!hFileSearch || hFileSearch == INVALID_HANDLE_VALUE)
        return FALSE;

	::EnterCriticalSection(&m_csFileFindArray);

	// Do extension filtering?
	BOOL bDoExtensionFiltering = (m_AllowedExtensions.GetSize() > 0 || m_bExtensionFilterAllowNumeric);

	do
    {
		CString sFileName = m_sRootDirName + CString(Info.cFileName);
		if (Info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (CString(Info.cFileName) != _T(".") &&
				CString(Info.cFileName) != _T(".."))
				m_Dirs.Add(sFileName);
		}
		else
		{
			if (bDoExtensionFiltering)
			{
				BOOL bAllowed = FALSE;
				CString sExt(::GetFileExt(sFileName));	// returns lower case extension with dot
				sExt.TrimLeft(_T('.'));					// extension without dot
				for (int pos = 0 ; pos < m_AllowedExtensions.GetSize() ; pos++)
				{
					if (sExt == m_AllowedExtensions[pos]) // extension in array is lower case without dot
					{
						bAllowed = TRUE;
						break;
					}
				}
				if (!bAllowed && m_bExtensionFilterAllowNumeric)
					bAllowed = ::IsNumeric(sExt);
				if (bAllowed)
					m_Files.Add(sFileName);
			}
			else
				m_Files.Add(sFileName);
		}
    }
    while (::FindNextFile(hFileSearch, &Info));

	if (bDoSort)
	{
		m_Files.Sort();
		m_Dirs.Sort();
	}

	::LeaveCriticalSection(&m_csFileFindArray);

	return ::FindClose(hFileSearch);
}

BOOL CSortableFileFind::InitRecursive(const CString& strName, BOOL bDoSort/*=TRUE*/)
{
	m_sFileName = _T("");
	m_sDirName = _T("");
	Close();

	// Check
	if (strName == _T(""))
		return FALSE;

	// Set Root Dir Name
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	_tsplitpath(strName, szDrive, szDir, NULL, NULL);
	m_sRootDirName = CString(szDrive) + CString(szDir);

	// Start Thread
	m_FileFindThread.SetStartName(strName);
	m_FileFindThread.SetSort(bDoSort);
	if (m_FileFindThread.Start(THREAD_PRIORITY_BELOW_NORMAL))
		return TRUE;
	else
		return FALSE;
}

int CSortableFileFind::WaitRecursiveStarted(HANDLE hKillEvent/*=NULL*/)
{
	HANDLE hEventArray[3];
	hEventArray[0] = m_hRecursiveFileFindStarted;
	hEventArray[1] = m_hRecursiveFileFindNoFiles;
	DWORD Event;
	if (hKillEvent)
	{
		hEventArray[2] = hKillEvent;
		Event = ::WaitForMultipleObjects(3, hEventArray, FALSE, INFINITE);
		switch (Event)
		{
			case WAIT_OBJECT_0 :		return 1;	// Started Event

			case WAIT_OBJECT_0 + 1 :	return 0;	// No Files Event

			case WAIT_OBJECT_0 + 2 :	return -1;	// Kill Event

			default:					return -2;	// Error
		}
	}
	else
	{
		Event = ::WaitForMultipleObjects(2, hEventArray, FALSE, INFINITE);
		switch (Event)
		{
			case WAIT_OBJECT_0 :		return 1;	// Started Event

			case WAIT_OBJECT_0 + 1 :	return 0;	// No Files Event

			default:					return -2;	// Error
		}
	}
}

int CSortableFileFind::WaitRecursiveDone(HANDLE hKillEvent/*=NULL*/)
{
	// Wait That The Thread Starts
	int ret = WaitRecursiveStarted(hKillEvent);
	if (ret != 1)
		return ret;

	// Wait That The Thread Exits
	DWORD Event;
	if (hKillEvent)
	{
		HANDLE hEventArray[2];
		hEventArray[0] = m_FileFindThread.GetHandle();
		hEventArray[1] = hKillEvent;
		Event = ::WaitForMultipleObjects(2, hEventArray, FALSE, INFINITE);
		switch (Event)
		{
			case WAIT_OBJECT_0 :		return 1;		// Thread Exited

			case WAIT_OBJECT_0 + 1 :	return -1;		// Kill Event

			default:					return -2;		// Error
		}

	}
	else
	{
		Event = ::WaitForSingleObject(m_FileFindThread.GetHandle(), INFINITE);
		switch (Event)
		{
			case WAIT_OBJECT_0 :		return 1;		// Thread Exited

			default:					return -2;		// Error
		}
	}
}

void CSortableFileFind::AddAllowedExtension(CString sExt)
{
	// Adjust
	sExt.MakeLower();
	sExt.TrimLeft(_T('.'));

	// Check whether extension is already in array
	BOOL bDuplicated = FALSE;
	for (int pos = 0 ; pos < m_AllowedExtensions.GetSize() ; pos++)
	{
		if (sExt == m_AllowedExtensions[pos])
		{
			bDuplicated = TRUE;
			break;
		}
	}

	// Add the extension to the end if not already present
	if (!bDuplicated)
		m_AllowedExtensions.Add(sExt);
}

void CSortableFileFind::ClearAllowedExtensions()
{
	m_AllowedExtensions.RemoveAll();
	m_bExtensionFilterAllowNumeric = FALSE;
}

BOOL CSortableFileFind::FindPreviousFile()
{
	::EnterCriticalSection(&m_csFileFindArray);
	if (m_nFilePos > 0)
	{
		m_sFileName = m_Files[--m_nFilePos];
		::LeaveCriticalSection(&m_csFileFindArray);
		return TRUE;
	}
	else
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return FALSE;
	}
}

BOOL CSortableFileFind::FindPreviousDir()
{
	::EnterCriticalSection(&m_csFileFindArray);
	if (m_nDirPos > 0)
	{
		m_sDirName = m_Dirs[--m_nDirPos];
		::LeaveCriticalSection(&m_csFileFindArray);
		return TRUE;
	}
	else
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return FALSE;
	}
}

CString CSortableFileFind::GetFileName(int pos)
{
	// Simple Cases
	::EnterCriticalSection(&m_csFileFindArray);
	if (m_Files.GetSize() == 0)
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return _T("");
	}

	// Check
	if ((pos > (m_Files.GetSize() - 1)) || (pos < 0))
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return _T("");
	}

	// Get Name
	CString sFileName = m_Files[pos];

	// Exit
	::LeaveCriticalSection(&m_csFileFindArray);

	return sFileName;
}

CString CSortableFileFind::GetDirName(int pos)
{
	// Simple Cases
	::EnterCriticalSection(&m_csFileFindArray);
	if (m_Dirs.GetSize() == 0)
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return _T("");
	}

	// Check
	if ((pos > (m_Dirs.GetSize() - 1)) || (pos < 0))
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return _T("");
	}

	// Get Name
	CString sDirName = m_Dirs[pos];

	// Exit
	::LeaveCriticalSection(&m_csFileFindArray);

	return sDirName;
}

CString CSortableFileFind::GetPreviousFileName(int& pos)
{
	// Simple Cases
	::EnterCriticalSection(&m_csFileFindArray);
	if (m_Files.GetSize() == 0)
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return _T("");
	}
	if (m_Files.GetSize() == 1)
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return m_Files[0];
	}

	// Dec Pos
	pos--;
	
	// Check and Wrap-around Pos
	if ((pos > (m_Files.GetSize() - 1)) || (pos < 0))
		pos = m_Files.GetSize() - 1;

	// Get Name
	CString sFileName = m_Files[pos];

	// Exit
	::LeaveCriticalSection(&m_csFileFindArray);

	return sFileName;
}

CString CSortableFileFind::GetNextFileName(int& pos)
{
	// Simple Cases
	::EnterCriticalSection(&m_csFileFindArray);
	if (m_Files.GetSize() == 0)
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return _T("");
	}
	if (m_Files.GetSize() == 1)
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return m_Files[0];
	}

	// Inc Pos
	pos++;
	
	// Check and Wrap-around Pos
	if ((pos > (m_Files.GetSize() - 1)) || (pos < 0))
		pos = 0;

	// Get Name
	CString sFileName = m_Files[pos];

	// Exit
	::LeaveCriticalSection(&m_csFileFindArray);

	return sFileName;
}

BOOL CSortableFileFind::AddFileName(int pos, const CString& strName)
{
	::EnterCriticalSection(&m_csFileFindArray);
	
	// Check
	if ((pos > m_Files.GetSize()) || (pos < 0))
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return FALSE;
	}

	// Add
	if (pos == m_Files.GetSize())
	{
		m_Files.Add(strName);
	}
	// Insert
	else
	{
		// Insert At
		m_Files.InsertAt(pos, strName);

		// Update File Position
		if (pos <= m_nFilePos)
			++m_nFilePos;
	}

	// Exit
	::LeaveCriticalSection(&m_csFileFindArray);

	return TRUE;
}

BOOL CSortableFileFind::DeleteFileName(int pos)
{
	::EnterCriticalSection(&m_csFileFindArray);
	
	// Check
	if ((pos > (m_Files.GetSize() - 1)) || (pos < 0))
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return FALSE;
	}

	// Remove At
	m_Files.RemoveAt(pos);

	// Update File Position
	if (pos < m_nFilePos)
		--m_nFilePos;

	// Wrap-Around File Position
	if (m_nFilePos > (m_Files.GetSize() - 1))
		m_nFilePos = 0;

	// Update File Name
	if (m_Files.GetSize() > 0)
		m_sFileName = m_Files[m_nFilePos];
	else
	{
		m_nFilePos = -1;
		m_sFileName = _T("");
	}

	// Exit
	::LeaveCriticalSection(&m_csFileFindArray);

	return TRUE;
}

BOOL CSortableFileFind::FindRandomFile()
{
	::EnterCriticalSection(&m_csFileFindArray);
	if (m_Files.GetSize() > 0)
	{
		int nOldFilePos = m_nFilePos;
		srand(::timeGetTime()); // Seed
		m_nFilePos = (int)irand(m_Files.GetSize()); // returns a random pos in the range [0,m_Files.GetSize()[
		m_sFileName = m_Files[m_nFilePos];
		if (nOldFilePos == m_nFilePos) // We do not want the same file, return next one
		{
			if (m_nFilePos < (m_Files.GetSize() - 1))
				m_sFileName = m_Files[++m_nFilePos];
			else
				m_sFileName = m_Files[m_nFilePos = 0];
		}
		::LeaveCriticalSection(&m_csFileFindArray);
		return TRUE;
	}
	else
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return FALSE;
	}
}

BOOL CSortableFileFind::FindNextFile()
{
	::EnterCriticalSection(&m_csFileFindArray);
	if (m_nFilePos < (m_Files.GetSize() - 1))
	{
		m_sFileName = m_Files[++m_nFilePos];
		::LeaveCriticalSection(&m_csFileFindArray);
		return TRUE;
	}
	else
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return FALSE;
	}
}

BOOL CSortableFileFind::FindNextDir()
{
	::EnterCriticalSection(&m_csFileFindArray);
	if (m_nDirPos < (m_Dirs.GetSize() - 1))
	{
		m_sDirName = m_Dirs[++m_nDirPos];
		::LeaveCriticalSection(&m_csFileFindArray);
		return TRUE;
	}
	else
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return FALSE;
	}
}

BOOL CSortableFileFind::FindFirstFile()
{
	::EnterCriticalSection(&m_csFileFindArray);
	if (m_Files.GetSize() >= 1)
	{
		m_nFilePos = 0;
		m_sFileName = m_Files[m_nFilePos];
		::LeaveCriticalSection(&m_csFileFindArray);
		return TRUE;
	}
	else
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return FALSE;
	}
}

BOOL CSortableFileFind::FindFirstDir()
{
	::EnterCriticalSection(&m_csFileFindArray);
	if (m_Dirs.GetSize() >= 1)
	{
		m_nDirPos = 0;
		m_sDirName = m_Dirs[m_nDirPos];
		::LeaveCriticalSection(&m_csFileFindArray);
		return TRUE;
	}
	else
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return FALSE;
	}
}

BOOL CSortableFileFind::FindLastFile()
{
	::EnterCriticalSection(&m_csFileFindArray);
	if (m_Files.GetSize() >= 1)
	{
		m_nFilePos = m_Files.GetSize() - 1;
		m_sFileName = m_Files[m_nFilePos];
		::LeaveCriticalSection(&m_csFileFindArray);
		return TRUE;
	}
	else
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return FALSE;
	}
}

BOOL CSortableFileFind::FindLastDir()
{
	::EnterCriticalSection(&m_csFileFindArray);
	if (m_Dirs.GetSize() >= 1)
	{
		m_nDirPos = m_Dirs.GetSize() - 1;
		m_sDirName = m_Dirs[m_nDirPos];
		::LeaveCriticalSection(&m_csFileFindArray);
		return TRUE;
	}
	else
	{
		::LeaveCriticalSection(&m_csFileFindArray);
		return FALSE;
	}
}

void CSortableFileFind::Close()
{
	m_FileFindThread.Kill();
	m_Files.RemoveAll();
	m_Dirs.RemoveAll();
	m_nDirPos = -1;
	m_nFilePos = -1;
	m_bRecursiveFileFindStarted = FALSE;
	::ResetEvent(m_hRecursiveFileFindStarted);
	::ResetEvent(m_hRecursiveFileFindNoFiles);
}

CString CSortableFileFind::GetShortFileName()
{
	TCHAR szExt[_MAX_EXT];
	TCHAR szName[_MAX_FNAME];
	_tsplitpath(m_sFileName, NULL, NULL, szName, szExt);
	return CString(szName) + CString(szExt);
}

CString CSortableFileFind::GetShortFileName(int pos)
{
	TCHAR szExt[_MAX_EXT];
	TCHAR szName[_MAX_FNAME];
	_tsplitpath(GetFileName(pos), NULL, NULL, szName, szExt);
	return CString(szName) + CString(szExt);
}

ULARGE_INTEGER CSortableFileFind::GetFileSize()
{
	ULARGE_INTEGER Size;
	Size.QuadPart = 0;
	WIN32_FIND_DATA fileinfo;
	HANDLE hFind = ::FindFirstFile(m_sFileName, &fileinfo);
	Size.LowPart = fileinfo.nFileSizeLow;
	Size.HighPart = fileinfo.nFileSizeHigh;
	::FindClose(hFind);
	return Size;
}

int CSortableFileFind::GetFilePosition()
{
	::EnterCriticalSection(&m_csFileFindArray);
	int nFilePos = m_nFilePos;
	::LeaveCriticalSection(&m_csFileFindArray);
	return nFilePos;
}

int CSortableFileFind::GetDirPosition()
{
	::EnterCriticalSection(&m_csFileFindArray);
	int nDirPos = m_nDirPos;
	::LeaveCriticalSection(&m_csFileFindArray);
	return nDirPos;
}

int CSortableFileFind::GetFilesCount()
{
	::EnterCriticalSection(&m_csFileFindArray);
	int nSize = m_Files.GetSize();
	::LeaveCriticalSection(&m_csFileFindArray);
	return nSize;
}

int CSortableFileFind::GetDirsCount()
{
	::EnterCriticalSection(&m_csFileFindArray);
	int nSize = m_Dirs.GetSize();
	::LeaveCriticalSection(&m_csFileFindArray);
	return nSize;
}

#define RECURSE_FREE \
if (pLocalFiles) delete pLocalFiles;\
if (pLocalDirs) delete pLocalDirs;\
if (pInfo) delete pInfo;\
if (szDrive) delete [] szDrive;\
if (szDir) delete [] szDir;\
if (szExt) delete [] szExt;\
if (szName) delete [] szName;
BOOL CSortableFileFind::CFileFindThread::Recurse(const CString& strName)
{
	int i;
	CSortableStringArray* pLocalFiles = NULL;
	CSortableStringArray* pLocalDirs = NULL;
	WIN32_FIND_DATA* pInfo = NULL;
	TCHAR* szDrive = NULL;
	TCHAR* szDir = NULL;
	TCHAR* szExt = NULL;
	TCHAR* szName = NULL;

	// Allocate on heap because we are a recursive function,
	// using the stack can overflow the stack!
	pLocalFiles = new CSortableStringArray;
	if (!pLocalFiles)
		return FALSE;
	pLocalDirs = new CSortableStringArray;
	if (!pLocalDirs)
	{
		RECURSE_FREE;
		return FALSE;
	}
	pInfo = new WIN32_FIND_DATA;
	if (!pInfo)
	{
		RECURSE_FREE;
		return FALSE;
	}
	szDrive = new TCHAR[_MAX_DRIVE];
	if (!szDrive)
	{
		RECURSE_FREE;
		return FALSE;
	}
	szDir = new TCHAR[_MAX_DIR];
	if (!szDir)
	{
		RECURSE_FREE;
		return FALSE;
	}
	szExt = new TCHAR[_MAX_EXT];
	if (!szExt)
	{
		RECURSE_FREE;
		return FALSE;
	}
	szName = new TCHAR[_MAX_FNAME];
	if (!szName)
	{
		RECURSE_FREE;
		return FALSE;
	}
	_tsplitpath(strName, szDrive, szDir, szName, szExt);

	HANDLE hFileSearch = ::FindFirstFile(strName, pInfo);
    if (!hFileSearch || hFileSearch == INVALID_HANDLE_VALUE)
	{
		// Probably no access to this directory -> continue
        RECURSE_FREE;
		return TRUE;
	}

	// Do extension filtering?
	BOOL bDoExtensionFiltering = (m_p->m_AllowedExtensions.GetSize() > 0 || m_p->m_bExtensionFilterAllowNumeric);

	do
    {
		// Do Exit?
		if (DoExit())
		{
			::FindClose(hFileSearch);
			RECURSE_FREE;
			return FALSE;
		}

		// Add to Local List
		CString sFileName = CString(szDrive) + CString(szDir) + CString(pInfo->cFileName);
		if (pInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (CString(pInfo->cFileName) != _T(".") &&
				CString(pInfo->cFileName) != _T(".."))
				pLocalDirs->Add(sFileName);
		}
		else
		{
			if (bDoExtensionFiltering)
			{
				BOOL bAllowed = FALSE;
				CString sExt(::GetFileExt(sFileName));	// returns lower case extension with dot
				sExt.TrimLeft(_T('.'));					// extension without dot
				for (int pos = 0 ; pos < m_p->m_AllowedExtensions.GetSize() ; pos++)
				{
					if (sExt == m_p->m_AllowedExtensions[pos]) // extension in array is lower case without dot
					{
						bAllowed = TRUE;
						break;
					}
				}
				if (!bAllowed && m_p->m_bExtensionFilterAllowNumeric)
					bAllowed = ::IsNumeric(sExt);
				if (bAllowed)
					pLocalFiles->Add(sFileName);
			}
			else
				pLocalFiles->Add(sFileName);
		}
    }
    while (::FindNextFile(hFileSearch, pInfo));

	// Close
	::FindClose(hFileSearch);

	// Sort
	if (m_bDoSort)
	{
		pLocalFiles->Sort();
		pLocalDirs->Sort();
	}

	// Do Exit?
	if (DoExit())
	{
		RECURSE_FREE;
		return FALSE;
	}

	// Update Main Lists
	::EnterCriticalSection(&m_p->m_csFileFindArray);
	for (i = 0 ; i < pLocalFiles->GetSize() ; i++)
		m_p->m_Files.Add((*pLocalFiles)[i]);
	for (i = 0 ; i < pLocalDirs->GetSize() ; i++)
		m_p->m_Dirs.Add((*pLocalDirs)[i]);
	::LeaveCriticalSection(&m_p->m_csFileFindArray);

	// Trigger Ready Event
	if (!m_p->m_bRecursiveFileFindStarted && (m_p->m_Files.GetSize() > 0))
	{
		m_p->m_bRecursiveFileFindStarted = TRUE;
		::SetEvent(m_p->m_hRecursiveFileFindStarted);
		if (::IsWindow(m_p->m_hWnd))
		{
			::PostMessage(	m_p->m_hWnd,
							WM_RECURSIVEFILEFIND_STARTED,
							0, 0);
		}
	}

	// Do Exit?
	if (DoExit())
	{
		RECURSE_FREE;
		return FALSE;
	}

	// Recurse
	for (i = 0 ; i < pLocalDirs->GetSize() ; i++)
	{
		if (!Recurse((*pLocalDirs)[i] + _T("\\") + CString(szName) + CString(szExt)))
		{
			RECURSE_FREE;
			return FALSE;
		}
	}

	// Free
	RECURSE_FREE;

	// Do Exit?
	if (DoExit())
		return FALSE;
	else
		return TRUE;
}

int CSortableFileFind::CFileFindThread::Work() 
{
	if (m_p == NULL)
		return 0;

	// Go!
	Recurse(m_sStartName);

	// Post Done Message
	if (::IsWindow(m_p->m_hWnd))
	{
		::PostMessage(	m_p->m_hWnd,
						WM_RECURSIVEFILEFIND_DONE,
						0, 0);
	}

	// Trigger the No Files Event
	if (m_p->m_Files.GetSize() == 0)
		::SetEvent(m_p->m_hRecursiveFileFindNoFiles);

	return 0;
}
