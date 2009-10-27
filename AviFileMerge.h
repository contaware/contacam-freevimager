#ifndef _INC_AVIFILEMERGE
#define _INC_AVIFILEMERGE

#include "SortableStringArray.h"

extern bool DoExit(HANDLE hKillEvent);

extern bool AVIFileMergeSerialVfW(	CString sSaveFileName,
									CSortableStringArray* pAviFileNames,
									bool bReCompressVideo,
									bool bReCompressAudio,
									CWnd* pWnd,
									HANDLE hKillEvent,
									bool bShowMessageBoxOnError);

extern bool AVIFileMergeParallelVfW(	CString sSaveFileName,
										CSortableStringArray* pAviFileNames,
										CWnd* pWnd,
										HANDLE hKillEvent,
										bool bShowMessageBoxOnError);

#endif