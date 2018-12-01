#if !defined(AFX_GETDIRCONTENTSIZE_H__8FD88286_7192_47B9_B311_4C2F27BF8B86__INCLUDED_)
#define AFX_GETDIRCONTENTSIZE_H__8FD88286_7192_47B9_B311_4C2F27BF8B86__INCLUDED_

#pragma once

#include "WorkerThread.h"

extern ULARGE_INTEGER GetDirContentSize(LPCTSTR szDirName,
										int* pFilesCount = NULL, // optional files count
										CWorkerThread* pThread = NULL);

#endif // !defined(AFX_GETDIRCONTENTSIZE_H__8FD88286_7192_47B9_B311_4C2F27BF8B86__INCLUDED_)