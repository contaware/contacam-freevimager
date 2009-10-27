/*
Module : CFILE64.CPP
Purpose: Implementation for an MFC class to provide access to the 64bit'isms of the Win32 file system APIs
Created: PJN / 11-02-1998
History: 

Copyright (c) 1998 - 2000 by PJ Naughter.  
All rights reserved.

*/

/////////////////////////////////  Includes  //////////////////////////////////
#include "stdafx.h"
#include "cfile64.h"

#if (_MSC_VER <= 1200)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


////////////////// Implementation //////////////////////////////

IMPLEMENT_DYNAMIC(CFile64, CObject)

CFile64::CFile64()
{
  m_hFile = INVALID_HANDLE_VALUE;
  m_sFilename.Empty();
  m_bAutoClose = FALSE;   
}

CFile64::CFile64(HANDLE hFile, BOOL bAutoClose)
{
  Attach(hFile, bAutoClose);
}

CFile64::~CFile64()
{
  Detach();
}

CFile64::operator HANDLE()
{
  return m_hFile;
}

BOOL CFile64::Open(LPCTSTR lpFileName,
                   DWORD dwDesiredAccess,
                   DWORD dwShareMode,
                   DWORD dwCreationDistribution,
                   CFileException* pError,
                   LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                   DWORD dwFlagsAndAttributes,
                   HANDLE hTemplateFile)
{
	ASSERT(AfxIsValidString(lpFileName));
	ASSERT(pError == NULL || AfxIsValidAddress(pError, sizeof(CFileException)));


  HANDLE hFile = CreateFile(lpFileName,	dwDesiredAccess, dwShareMode,	
                            lpSecurityAttributes,	dwCreationDistribution,	
                            dwFlagsAndAttributes,	hTemplateFile);

  BOOL bSuccess = FALSE;
  if (hFile != INVALID_HANDLE_VALUE)
  {
    bSuccess = TRUE;
    m_hFile = hFile;
    m_sFilename = lpFileName;
    m_bAutoClose = TRUE;   
  }
  else
  {
		if (pError != NULL)
		{
			pError->m_lOsError = ::GetLastError();
			pError->m_cause =	CFileException::OsErrorToException(pError->m_lOsError);

			// use passed file name (not expanded vesion) when reporting
			// an error while opening
			pError->m_strFileName = lpFileName;
    }
  }

  return bSuccess;
}

BOOL CFile64::IsOpen() const
{
  return (m_hFile != INVALID_HANDLE_VALUE);
}

BOOL CFile64::IsClosed() const
{
  return (m_hFile == INVALID_HANDLE_VALUE);
}

void CFile64::Attach(HANDLE hFile, BOOL bAutoClose)
{
  m_hFile = hFile;
  m_sFilename.Empty(); //Currently Win32 provides no means of
                       //going from a File Handle back to a filename
  m_bAutoClose = bAutoClose;   
}

void CFile64::Detach()
{
  if (IsOpen() && m_bAutoClose)
    Close();
  else
  {
  	m_hFile = INVALID_HANDLE_VALUE;
	  m_bAutoClose = FALSE;
  	m_sFilename.Empty();
  }
}

void CFile64::Close()
{
	ASSERT_VALID(this);
	ASSERT(IsOpen());

	BOOL bError = FALSE;
	if (IsOpen())
		bError = !::CloseHandle(m_hFile);

	m_hFile = INVALID_HANDLE_VALUE;
	m_bAutoClose = FALSE;
	m_sFilename.Empty();

	if (bError)
		CFileException::ThrowOsError((LONG)::GetLastError());
}

void CFile64::Write(LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite)
{
 	ASSERT_VALID(this);
	ASSERT(IsOpen());
	ASSERT(lpBuffer != NULL);
	ASSERT(AfxIsValidAddress(lpBuffer, dwNumberOfBytesToWrite, FALSE));

	DWORD nWritten;
	if (!::WriteFile(m_hFile, lpBuffer, dwNumberOfBytesToWrite, &nWritten, NULL))
		CFileException::ThrowOsError((LONG)::GetLastError(), m_sFilename);

	if (nWritten != dwNumberOfBytesToWrite)
		AfxThrowFileException(CFileException::diskFull, -1, m_sFilename);
}

DWORD CFile64::Read(LPVOID lpBuffer, DWORD dwNumberOfBytesToRead)
{
	ASSERT_VALID(this);
	ASSERT(IsOpen());
	ASSERT(lpBuffer != NULL);
	ASSERT(AfxIsValidAddress(lpBuffer, dwNumberOfBytesToRead, TRUE));

	DWORD dwRead;
	if (!::ReadFile(m_hFile, lpBuffer, dwNumberOfBytesToRead, &dwRead, NULL))
		CFileException::ThrowOsError((LONG)::GetLastError());

	return dwRead;
}

UINT64 CFile64::MakeUnsignedInt64(DWORD nHigh, DWORD nLow)
{
  return ((((UINT64) nHigh) << 32) | nLow);
}

void CFile64::SplitUnsignedInt64(const UINT64& nBigInt, DWORD& nHigh, DWORD& nLow)
{
  nHigh = (DWORD) ((nBigInt & 0xFFFFFFFF00000000) >> 32);
  nLow = (DWORD) (nBigInt & 0x00000000FFFFFFFF);
}

UINT64 CFile64::Seek(const UINT64& lDistanceToMove, UINT MoveMethod, BOOL bForward/*=TRUE*/)
{
	ASSERT_VALID(this);
	ASSERT(IsOpen());

  UINT64 nNewPos;
  switch (MoveMethod)
  {
	case CFile::begin:   
    {
      nNewPos = lDistanceToMove; 
      break;
    }
    case CFile::current: //deliberate fall through
    {
      if (bForward)
        nNewPos = GetPosition() + lDistanceToMove; 
      else
        nNewPos = GetPosition() - lDistanceToMove;
      break;
    }
    case CFile::end:     
    {
      if (bForward)
        nNewPos = GetLength() + lDistanceToMove; 
      else
        nNewPos = GetLength() - lDistanceToMove;
      break;
    }
    default:      
    {
      ASSERT(FALSE);        
      break;
    }
  }
  DWORD dwDistanceToMoveLow;
  DWORD dwDistanceToMoveHigh;
  SplitUnsignedInt64(nNewPos, dwDistanceToMoveHigh, dwDistanceToMoveLow);

  //WINBUG: What if dwDistanceToMoveLow is > LONG_MAX, then SetFilePointer 
  //will interpret that as a negative value
	DWORD dwNewLow = ::SetFilePointer(m_hFile, dwDistanceToMoveLow, (LONG*) &dwDistanceToMoveHigh, FILE_BEGIN);
	if (dwNewLow  == (DWORD)-1)
		CFileException::ThrowOsError((LONG)::GetLastError());

	return MakeUnsignedInt64(dwDistanceToMoveHigh, dwNewLow);
}

UINT64 CFile64::SeekToEnd()
{
	return Seek(0, CFile::end, FALSE);
}

void CFile64::SeekToBegin()
{
	Seek(0, CFile::begin, TRUE);
}

void CFile64::SetLength(const UINT64& lNewLen)
{
 	ASSERT_VALID(this);
	ASSERT(IsOpen());

	Seek(lNewLen, CFile::begin, TRUE);

	if (!::SetEndOfFile(m_hFile))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

UINT64 CFile64::GetLength() const
{
	ASSERT_VALID(this);
	ASSERT(IsOpen());

  DWORD dwHighLength;
  DWORD dwLowLength = GetFileSize(m_hFile, &dwHighLength);
	if (dwLowLength  == (DWORD)-1)
		CFileException::ThrowOsError((LONG)::GetLastError());  

	return MakeUnsignedInt64(dwHighLength, dwLowLength);
}

UINT64 CFile64::GetPosition() const
{
	ASSERT_VALID(this);
	ASSERT(IsOpen());

  DWORD dwHighPos = 0;
	DWORD dwLowPos = ::SetFilePointer(m_hFile, 0, (LONG*) &dwHighPos, FILE_CURRENT);
	if (dwLowPos  == (DWORD)-1)
		CFileException::ThrowOsError((LONG)::GetLastError());

	return MakeUnsignedInt64(dwHighPos, dwLowPos);
}

void CFile64::Flush()
{
	ASSERT_VALID(this);

	if (IsClosed())
		return;

	if (!::FlushFileBuffers(m_hFile))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

CFile64* CFile64::Duplicate() const
{
	ASSERT_VALID(this);
	ASSERT(IsOpen());

	CFile64* pFile = new CFile64();
	HANDLE hFile;
  HANDLE hProcess = GetCurrentProcess();
	if (!::DuplicateHandle(hProcess, m_hFile, hProcess, &hFile, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		delete pFile;
		CFileException::ThrowOsError((LONG)::GetLastError());
	}
	pFile->m_hFile = hFile;
  pFile->m_sFilename = m_sFilename;
	ASSERT(pFile->IsOpen());
	pFile->m_bAutoClose = m_bAutoClose;
	return pFile;
}

void CFile64::Abort()
{
 	ASSERT_VALID(this);
	if (IsOpen())
	{
		// close but ignore errors
		::CloseHandle(m_hFile);
  	m_hFile = INVALID_HANDLE_VALUE;
	  m_bAutoClose = FALSE;
	  m_sFilename.Empty();
	}
}

void CFile64::LockRange(const UINT64& lPos, const UINT64& lCount)
{
	ASSERT_VALID(this);
	ASSERT(IsOpen());

  DWORD dwFileOffsetLow;
  DWORD dwFileOffsetHigh;
  SplitUnsignedInt64(lPos, dwFileOffsetHigh, dwFileOffsetLow);
  DWORD nNumberOfBytesToLockLow;
  DWORD nNumberOfBytesToLockHigh;
  SplitUnsignedInt64(lCount, nNumberOfBytesToLockHigh, nNumberOfBytesToLockLow);

	if (!::LockFile(m_hFile, dwFileOffsetLow, dwFileOffsetHigh, nNumberOfBytesToLockLow, nNumberOfBytesToLockHigh))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

void CFile64::UnlockRange(const UINT64& lPos, const UINT64& lCount)
{
	ASSERT_VALID(this);
	ASSERT(IsOpen());

  DWORD dwFileOffsetLow;
  DWORD dwFileOffsetHigh;
  SplitUnsignedInt64(lPos, dwFileOffsetHigh, dwFileOffsetLow);
  DWORD nNumberOfBytesToLockLow;
  DWORD nNumberOfBytesToLockHigh;
  SplitUnsignedInt64(lCount, nNumberOfBytesToLockHigh, nNumberOfBytesToLockLow);

	if (!::UnlockFile(m_hFile, dwFileOffsetLow, dwFileOffsetHigh, nNumberOfBytesToLockLow, nNumberOfBytesToLockHigh))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

DWORD CFile64::GetFileType()
{
	ASSERT_VALID(this);
	ASSERT(IsOpen());

  return ::GetFileType(m_hFile);
}

#ifdef _DEBUG
void CFile64::AssertValid() const
{
	CObject::AssertValid();
}

void CFile64::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "with handle " << m_hFile;
	dc << " and name \"" << m_sFilename << "\"";
	dc << "\n";
}
#endif

CString CFile64::GetFilePath() const
{
	ASSERT_VALID(this);
	return m_sFilename;
}

CString CFile64::GetFileName() const
{
	ASSERT_VALID(this);
	TCHAR szName[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];
	_tsplitpath(m_sFilename, NULL, NULL, szName, szExt);
	return CString(szName) + CString(szExt);
}

CString CFile64::GetFileTitle( ) const
{
	ASSERT_VALID(this);
	TCHAR szName[_MAX_FNAME];
	_tsplitpath(m_sFilename, NULL, NULL, szName, NULL);
	return CString(szName);
}

void CFile64::SetFilePath(LPCTSTR lpszNewName)
{
	ASSERT_VALID(this);
	ASSERT(AfxIsValidString(lpszNewName, FALSE));
	m_sFilename = lpszNewName;
}

#endif
