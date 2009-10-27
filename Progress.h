#ifndef _INC_PROGRESS
#define _INC_PROGRESS

#include <windows.h>
#include "Round.h"

// Progress
#ifndef WM_PROGRESS
#define WM_PROGRESS		WM_USER + 113
#endif

#ifndef DIB_INIT_PROGRESS
#define DIB_INIT_PROGRESS 	int nProgress_PercentDone;\
							int nProgress_PrevPercentDone = -5;
#endif

#ifndef DIB_PROGRESS
#define DIB_PROGRESS(hWnd, bSend, current, total)\
if (hWnd)\
{\
	nProgress_PercentDone = Round((double)(current) * 100.0 / (double)(total));\
	if ((nProgress_PercentDone >= (nProgress_PrevPercentDone + 5)) ||\
		(nProgress_PercentDone < nProgress_PrevPercentDone))\
	{\
		if (bSend)\
		{\
			SendMessage(	hWnd,\
							WM_PROGRESS,\
							(WPARAM)nProgress_PercentDone,\
							(LPARAM)NULL);\
		}\
		else\
		{\
			PostMessage(	hWnd,\
							WM_PROGRESS,\
							(WPARAM)nProgress_PercentDone,\
							(LPARAM)NULL);\
		}\
		nProgress_PrevPercentDone = nProgress_PercentDone;\
	}\
}
#endif

#ifndef DIB_END_PROGRESS
#define DIB_END_PROGRESS(hWnd)\
if (hWnd)\
{\
	PostMessage(	hWnd,\
					WM_PROGRESS,\
					(WPARAM)100,\
					(LPARAM)NULL);\
	nProgress_PrevPercentDone = -5;\
}
#endif

#endif
