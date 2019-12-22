// BkgColEdit.cpp : implementation file
//

#include "stdafx.h"
#include "BkgColEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBkgColEdit

CBkgColEdit::CBkgColEdit()
{
}

CBkgColEdit::~CBkgColEdit()
{
}

BEGIN_MESSAGE_MAP(CBkgColEdit, CEdit)
	//{{AFX_MSG_MAP(CBkgColEdit)
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBkgColEdit message handlers

HBRUSH CBkgColEdit::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	return ::GetSysColorBrush(COLOR_WINDOW);
}
