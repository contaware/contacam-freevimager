//
// SortableFileFind.h
//

#if !defined(AFX_SORTABLEFILEFIND_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
#define AFX_SORTABLEFILEFIND_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SortableStringArray.h"
#include "WorkerThread.h"

#define	WM_RECURSIVEFILEFIND_STARTED			WM_USER + 1100
#define	WM_RECURSIVEFILEFIND_DONE				WM_USER + 1101

class CSortableFileFind
{
	public:
		// The File Find Thread Class
		class CFileFindThread : public CWorkerThread
		{
			public:
				CFileFindThread(){m_p = NULL; m_bDoSort = FALSE; m_sStartName = _T("");};
				virtual ~CFileFindThread(){Kill();};
				void SetPointer(CSortableFileFind* p) {m_p = p;};
				void SetSort(BOOL bDoSort) {m_bDoSort = bDoSort;};
				void SetStartName(const CString& sStartName) {m_sStartName = sStartName;};

			protected:
				int Work();
				BOOL Recurse(const CString& strName);
				CSortableFileFind* m_p;
				BOOL m_bDoSort;
				CString m_sStartName;
		};
		friend CFileFindThread;

	public:
		CSortableFileFind() {	m_FileFindThread.SetPointer(this);
								::InitializeCriticalSection(&m_csFileFindArray);
								m_hRecursiveFileFindStarted = ::CreateEvent(NULL, TRUE, FALSE, NULL);
								m_hRecursiveFileFindNoFiles = ::CreateEvent(NULL, TRUE, FALSE, NULL);
								m_bRecursiveFileFindStarted = FALSE;
								m_hWnd = NULL;
								m_sRootDirName = _T("");
								m_sFileName = _T("");
								m_sDirName = _T("");
								m_nDirPos = -1;
								m_nFilePos = -1;};
		CSortableFileFind(CString strName, BOOL bDoSort = TRUE){m_FileFindThread.SetPointer(this);
																::InitializeCriticalSection(&m_csFileFindArray);
																m_hRecursiveFileFindStarted = ::CreateEvent(NULL, TRUE, FALSE, NULL);
																m_hRecursiveFileFindNoFiles = ::CreateEvent(NULL, TRUE, FALSE, NULL);
																m_bRecursiveFileFindStarted = FALSE;
																m_hWnd = NULL;
																Init(strName, bDoSort);};
		virtual ~CSortableFileFind() {	Close();
										ClearAllowedExtensions();
										::CloseHandle(m_hRecursiveFileFindStarted);
										::CloseHandle(m_hRecursiveFileFindNoFiles);
										::DeleteCriticalSection(&m_csFileFindArray);};

		// Pass A string containing the name of the file to find.
		// Call this member function to open a file search.
		// If bDoSort is set, files and dirs are sorted
		BOOL Init(const CString& strName, BOOL bDoSort = TRUE, CWorkerThread* pThread = NULL);

		// Pass A string containing the name of the file to find.
		// Call this member function to open a recursive file search.
		// If bDoSort is set, files and dirs are sorted inside
		// the parent dir
		BOOL InitRecursive(const CString& strName, BOOL bDoSort = TRUE);

		// Wait that the top dir has been searched (and ev. sorted).
		// Return values:
		// 1  : Ok started
		// 0  : No files
		// -1 : Kill event
		// -2 : Error
		int WaitRecursiveStarted(HANDLE hKillEvent = NULL);

		// Wait that the recursive file find finishes.
		// Return values:
		// 1  : Ok done
		// 0  : No files
		// -1 : Kill event
		// -2 : Error
		int WaitRecursiveDone(HANDLE hKillEvent = NULL);

		// Add an Allowed File Extension
		void AddAllowedExtension(CString sExt);

		// Clear All Allowed File Extensions
		// -> All extensions are allowed
		void ClearAllowedExtensions();

		// Set Window for Messages Send
		void SetWnd(HWND hWnd) {m_hWnd = hWnd;};

		// Call FindPreviousFile to retrieve files
		BOOL FindPreviousFile();

		// Call FindPreviousDir to retrieve dirs
		BOOL FindPreviousDir();

		// Get a random file
		BOOL FindRandomFile();

		// Call FindNextFile to retrieve files
		BOOL FindNextFile();

		// Call FindNextDir to retrieve dirs
		BOOL FindNextDir();

		// Get The First File,
		// no need to call this function after Init,
		// call FindNextFile()
		BOOL FindFirstFile();

		// Get The First Dir
		// no need to call this function after Init,
		// call FindNextDir()
		BOOL FindFirstDir();

		// Get The First File
		BOOL FindLastFile();

		// Get The First Dir
		BOOL FindLastDir();

		// Circulary returns the Previous File Name without
		// changing the current internal file position;
		// start position for previous position is the given
		// pos parameter which will be decremented by the
		// function.
		CString GetPreviousFileName(int& pos);

		// Circulary returns the Next File Name without
		// changing the current internal file position;
		// start position for next position is the given
		// pos parameter which will be incremented by the
		// function.
		CString GetNextFileName(int& pos);

		// Inserts the file name at the specified position
		// by moving the old file name down by one index.
		// If pos == m_Files.GetSize() it adds
		// the file name to the end.
		BOOL AddFileName(int pos, const CString& strName);

		// Delete File Name at Specified Position
		BOOL DeleteFileName(int pos);

		// Call this member function to end the search,
		// reset the context, and release all resources.
		// After calling Close, you do not have to create
		// a new CSortableFileFind instance before calling
		// Init to begin a new search.
		void Close();

		// Get the File Find Thread Pointer
		CFileFindThread* GetFileFindThread() {return &m_FileFindThread;};

		// Returns the most-recently-found file path
		CString GetFileName() const {return m_sFileName;};

		// Returns the file path at given position
		CString GetFileName(int pos);

		// Returns the name (with extension) of the most-recently-found file
		CString GetShortFileName();

		// Returns the name (with extension) at given position
		CString GetShortFileName(int pos);

		// Returns the most-recently-found dir path
		CString GetDirName() const {return m_sDirName;};

		// Returns the dir path at given position
		CString GetDirName(int pos);

		// Returns the root search dir path
		CString GetRootDirName() const {return m_sRootDirName;};

		// The length of the found file, in bytes
		ULARGE_INTEGER GetFileSize();

		// Get Current Fiel Position
		int GetFilePosition();

		// Get Current Directory Position
		int GetDirPosition();

		// Get the Number of Files in the String Array
		int GetFilesCount();

		// Get the Number of Directories in the String Array
		int GetDirsCount();

		// The File Attributes:
		// FILE_ATTRIBUTE_ARCHIVE The file or directory is an archive file or directory. Applications use this attribute to mark files for backup or removal. 
		// FILE_ATTRIBUTE_COMPRESSED The file or directory is compressed. For a file, this means that all of the data in the file is compressed. For a directory, this means that compression is the default for newly created files and subdirectories. 
		// FILE_ATTRIBUTE_DIRECTORY The handle identifies a directory. 
		// FILE_ATTRIBUTE_ENCRYPTED The file or directory is encrypted. For a file, this means that all data streams are encrypted. For a directory, this means that encryption is the default for newly created files and subdirectories. 
		// FILE_ATTRIBUTE_HIDDEN The file or directory is hidden. It is not included in an ordinary directory listing. 
		// FILE_ATTRIBUTE_NORMAL The file or directory has no other attributes set. This attribute is valid only if used alone. 
		// FILE_ATTRIBUTE_OFFLINE The data of the file is not immediately available. Indicates that the file data has been physically moved to offline storage. 
		// FILE_ATTRIBUTE_READONLY The file or directory is read-only. Applications can read the file but cannot write to it or delete it. In the case of a directory, applications cannot delete it. 
		// FILE_ATTRIBUTE_REPARSE_POINT The file has an associated reparse point. 
		// FILE_ATTRIBUTE_SPARSE_FILE The file is a sparse file. 
		// FILE_ATTRIBUTE_SYSTEM The file or directory is part of, or is used exclusively by, the operating system. 
		// FILE_ATTRIBUTE_TEMPORARY The file is being used for temporary storage. File systems attempt to keep all of the data in memory for quicker access rather than flushing the data back to mass storage. A temporary file should be deleted by the application as soon as it is no longer needed. 
		DWORD GetFileAttributes() const {return ::GetFileAttributes(m_sFileName);};
		DWORD GetDirAttributes() const {return ::GetFileAttributes(m_sDirName);};

	protected:
		CRITICAL_SECTION m_csFileFindArray;
		CFileFindThread m_FileFindThread;
		CString m_sRootDirName;
		CString m_sFileName;
		CString m_sDirName;
		CSortableStringArray m_Dirs;
		CSortableStringArray m_Files;
		CSortableStringArray m_AllowedExtensions;
		int m_nDirPos;
		int m_nFilePos;
		BOOL m_bRecursiveFileFindStarted;
		HANDLE m_hRecursiveFileFindStarted;
		HANDLE m_hRecursiveFileFindNoFiles;
		HWND m_hWnd;
};


#endif // !defined(AFX_SORTABLEFILEFIND_H__0A6BEBCA_829C_4085_8A12_19A15A12F62C__INCLUDED_)
