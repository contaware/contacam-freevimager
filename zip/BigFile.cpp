#include "BigFile.h"

#if (_MSC_VER <= 1200)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CBigFile, CFile)

ULONGLONG CBigFile::Seek(LONGLONG lOff, UINT nFrom)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != hFileNull);
	ASSERT(nFrom == begin || nFrom == end || nFrom == current);
	ASSERT(begin == FILE_BEGIN && end == FILE_END && current == FILE_CURRENT);
	LARGE_INTEGER li;
	li.QuadPart = lOff;

	li.LowPart  = ::SetFilePointer((HANDLE)m_hFile, li.LowPart, &li.HighPart, (DWORD)nFrom);
	DWORD dw = ::GetLastError();
	if ((li.LowPart == (DWORD)-1) && (dw != NO_ERROR))
		CFileException::ThrowOsError((LONG)dw);

	return (ULONGLONG)li.QuadPart;
}

ULONGLONG CBigFile::GetPosition64() const
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != hFileNull);
	LARGE_INTEGER li;
	li.QuadPart = 0;

	li.LowPart = ::SetFilePointer((HANDLE)m_hFile, 0, &li.HighPart, FILE_CURRENT);
	DWORD dw = ::GetLastError();
	if ((li.LowPart == (DWORD)-1) && (dw != NO_ERROR))
		CFileException::ThrowOsError((LONG)dw);

	return (ULONGLONG)li.QuadPart;
}

ULONGLONG CBigFile::GetLength64() const
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != hFileNull);
	ULARGE_INTEGER li;
	li.QuadPart = 0;

	li.LowPart = ::GetFileSize((HANDLE)m_hFile, &li.HighPart);
	DWORD dw = ::GetLastError();
	if ((li.LowPart == (DWORD)-1) && (dw != NO_ERROR))
		CFileException::ThrowOsError((LONG)dw);  

	return li.QuadPart;
}

#endif