#include "StdAfx.h"

const CString ml_string(int StringId)
{
	CString s;
	BOOL bOK = s.LoadString(StringId);
	if (bOK)
		return s;
	else
	{
		ASSERT(FALSE);
		return _T("???");
	}
}
