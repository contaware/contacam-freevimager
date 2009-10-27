/*
Module : CFILE64.CPP
Purpose: Interface for an MFC class to provide access to the 64bit'isms of the Win32 file system APIs
Created: PJN / 11-02-1998

Copyright (c) 1998 - 2000 by PJ Naughter.  
All rights reserved.

*/


#ifndef __CFILE64_H__
#define __CFILE64_H__

#if (_MSC_VER <= 1200)

///////////////////// classes ////////////////////////////////////////


//some typedefs to make using 64 bit ints more Windows friendly
typedef unsigned __int64 UINT64;
typedef signed __int64 INT64;
typedef UINT64* LPUINT64;
typedef INT64* LPINT64;


//the actual 64 bit file class
class CFile64 : public CObject
{
public:

//constructors / Destructors
  CFile64();
  CFile64(HANDLE hFile, BOOL bAutoClose = TRUE);
  ~CFile64();
  
//methods
  operator HANDLE();
  BOOL Open(LPCTSTR lpFileName,
            DWORD dwDesiredAccess,
            DWORD dwShareMode,
            DWORD dwCreationDistribution,
            CFileException* pError = NULL,
            LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL,
            DWORD dwFlagsAndAttributes = 0,
            HANDLE hTemplateFile = NULL);
  BOOL IsOpen() const;
  BOOL IsClosed() const;
  void Attach(HANDLE hFile, BOOL bAutoClose = TRUE);
  void Detach();
  void Close();
  void Write(LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite);
  DWORD Read(LPVOID lpBuffer, DWORD dwNumberOfBytesToRead);	
  UINT64 Seek(const UINT64& lDistanceToMove, UINT MoveMethod, BOOL bForward = TRUE);
  UINT64 SeekToEnd();
	void SeekToBegin();
  void SetLength(const UINT64& lNewLen);
  UINT64 GetLength() const;
  UINT64 GetPosition() const;
  void Flush();
  CFile64* Duplicate() const;
  void Abort();
  void LockRange(const UINT64& lPos, const UINT64& lCount);
  void UnlockRange(const UINT64& lPos, const UINT64& lCount);
  DWORD GetFileType();
  CString GetFileTitle() const;
  CString GetFileName() const;
  CString GetFilePath() const;
  void SetFilePath(LPCTSTR lpszNewName); 


//diagnostics
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

//helper functions to manage 64 integer conversion
  static UINT64 MakeUnsignedInt64(DWORD nHigh, DWORD nLow);
  static void SplitUnsignedInt64(const UINT64& nBigInt, DWORD& nHigh, DWORD& nLow);

  HANDLE  m_hFile;

protected:
	DECLARE_DYNAMIC(CFile64)

//member variables
  BOOL    m_bAutoClose;
  CString m_sFilename;
};

#endif

#endif // __CFILE64_H__
