#include "stdafx.h"
#include "resource.h"
#include "BrowseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CBrowseDlg, CWnd)
  //{{AFX_MSG_MAP(CBrowseDlg)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK_BROWSEDLG, OnCheckClicked)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CBrowseDlg, CWnd)

CBrowseDlg::CBrowseDlg(	CWnd* pParentWnd,
						CString* pDir,
						CString sTitle/*=_T("")*/,
						BOOL bNewFolderButton/*=FALSE*/,
						BOOL bCheckbox/*=FALSE*/,
						CString sCheckboxCaption/*=_T("")*/,
						BOOL bChecked/*=FALSE*/) :
	m_bChecked(bChecked),
	m_sCheckboxCaption(sCheckboxCaption),
	m_bCheckbox(bCheckbox),
	m_sTitle(sTitle),
	m_pItemIdList(NULL),
	m_pParentWnd(pParentWnd),
	m_pRoot(NULL)
{
	// Init Flags
	m_nFlags = (BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS | BIF_NEWDIALOGSTYLE);
	if (!bNewFolderButton)
		m_nFlags |= BIF_NONEWFOLDERBUTTON;
	else
		m_bCheckbox = FALSE;

	// Init COM: COM may fail if its already been inited with a different 
    // concurrency model. And if it fails you shouldn't release it!
	// Typically, the COM library is initialized on an apartment only once.
	// Subsequent calls will succeed, as long as they do not attempt to change
	// the concurrency model, but will return S_FALSE. To close the COM
	// library gracefully, each successful call to CoInitialize or CoInitializeEx,
	// including those that return S_FALSE, must be balanced by a corresponding
	// call to CoUninitialize.
	// Once the concurrency model for an apartment is set, it cannot be changed.
	// A call to CoInitialize on an apartment that was previously initialized as
	// multithreaded will fail and return RPC_E_CHANGED_MODE.
    HRESULT hr = ::CoInitialize(NULL);
    m_bCleanupCOM = ((hr == S_OK) || (hr == S_FALSE));
	m_pDir = pDir;
}

CBrowseDlg::~CBrowseDlg()
{
	HRESULT hr;
	IMalloc* pMalloc;
	if (FAILED(hr = ::SHGetMalloc(&pMalloc)))
		AfxThrowOleException(hr);
	pMalloc->Free(m_pRoot);
	FreeItemIdList(pMalloc);
	if (m_bCleanupCOM)
		::CoUninitialize();
}

int CBrowseDlg::DoModal()
{
	HRESULT hr;
	IMalloc* pMalloc;
	if (FAILED(hr = ::SHGetMalloc(&pMalloc)))
		::AfxThrowOleException(hr);

	// Fill out a 'BROWSEINFO' structure
	BROWSEINFO browseInfo;
	::ZeroMemory(&browseInfo, sizeof(BROWSEINFO));
	browseInfo.hwndOwner = (m_pParentWnd == NULL) ? NULL : m_pParentWnd->m_hWnd;
	browseInfo.pidlRoot = m_pRoot;
	browseInfo.pszDisplayName = m_strDisplayName.GetBufferSetLength(MAX_PATH);
	browseInfo.lpszTitle = m_sTitle;
	browseInfo.ulFlags = m_nFlags;
	browseInfo.lpfn = BrowseCallbackProc;
	browseInfo.lParam = (long)this;

	if (m_pItemIdList != NULL)
		FreeItemIdList();

	if ((m_pItemIdList = ::SHBrowseForFolder(&browseInfo)) == NULL)
	{
		// User Cancelled
		m_strDisplayName.ReleaseBuffer();
		pMalloc->Release();
		return IDCANCEL;
	}

	// Right - if we're here, the user actually selected an item.
	// Try to get a full path.  This will fail if the selected item
	// is not part of the FileSystem.
	if (m_pDir)
	{
		::SHGetPathFromIDList(	m_pItemIdList,
								m_pDir->GetBufferSetLength(MAX_PATH));
		m_pDir->ReleaseBuffer();
	}

	// Cleanup
	m_strDisplayName.ReleaseBuffer();
	pMalloc->Release();

	// Note: m_pItemIdList has *not* been freed! We keep around in
	// case the caller wants to retrieve it later. It will
	// ultimately be free-d in the destructor.
	return IDOK;
}

void CBrowseDlg::EnableOk(BOOL bEnable)
{
	ASSERT(NULL != m_hWnd);
	SendMessage(BFFM_ENABLEOK, 0U, bEnable ? 1L : 0L);
}

void CBrowseDlg::SetSelection(CString strPath)
{
	/*
	If the folder is the root of a drive, then we have to make sure that the
	trailing backslash is present. If the folder is not the root, then
	we have to strip off the trailing backslash instead (this behavior is not
	documented by Microsoft)
	*/
	ASSERT(NULL != m_hWnd);
	strPath.TrimRight(_T('\\'));
	if (strPath[strPath.GetLength() - 1] == _T(':'))
		strPath += _T("\\");
	SendMessage(BFFM_SETSELECTION, (WPARAM)TRUE, (LPARAM)(LPCTSTR)strPath);
}

void CBrowseDlg::OnCheckClicked()
{
	m_bChecked = !m_bChecked;
	m_Checkbox.SetCheck(m_bChecked ? 1 : 0);
}

void CBrowseDlg::OnInitDialog()
{
	if (m_pDir && !m_pDir->IsEmpty())
		SetSelection(*m_pDir);

	if (m_bCheckbox)
	{
		// Get checkbox's text extent
		CSize sz;
		CFont* pFont = GetFont();
		CDC* pDC = GetDC();
		CFont* pOldFont = (CFont*)pDC->SelectObject(pFont);
		::GetTextExtentPoint32(	pDC->m_hDC,
								m_sCheckboxCaption,
								m_sCheckboxCaption.GetLength(),
								&sz);
		pDC->SelectObject(pOldFont);
		ReleaseDC(pDC);

		// From MSDN Layout Specifications the dimensions of a checkbox is 12 dialog units
		// from the left edge of the control to the start of the text and 10 dialog units height
		// (http://msdn.microsoft.com/en-us/library/windows/desktop/bb226818%28v=vs.85%29.aspx)
		CRect rcCheckbox(0, 0, 12, 10);
		::MapDialogRect(GetSafeHwnd(), &rcCheckbox); // DLUs to pixels

		// Checkbox's total width
		m_nCheckboxWidth = sz.cx + rcCheckbox.Width();

		// Create it
		static const UINT BUTTON_STYLE = BS_CHECKBOX | WS_CHILD | WS_VISIBLE | WS_TABSTOP;
		CRect rc;
		CalcCheckRect(rc);
		if (!m_Checkbox.Create(	m_sCheckboxCaption,
								BUTTON_STYLE,
								rc,
								this,
								IDC_CHECK_BROWSEDLG))
			AfxThrowOleException(HRESULT_FROM_WIN32(::GetLastError()));

		// Let's change the font
		m_Checkbox.SetFont(pFont, TRUE);

		// Checked?
		m_Checkbox.SetCheck(m_bChecked ? 1 : 0);
	}
}

void CBrowseDlg::OnSelChanged(LPITEMIDLIST pNewSel)
{
	pNewSel;
}

// strBadName Invalid item name provided by the user
// Return:
// TRUE to indicate that the dialog may be dismissed.
// FALSE to force the dialog to remain displayed.
BOOL CBrowseDlg::OnValidateFailed(const CString& /*strBadName*/)
{
	return TRUE;
}

int CBrowseDlg::BrowseCallbackProc(	HWND hWnd,
									UINT nMsg,
									LPARAM lParam,
									LPARAM lpData)
{
	CBrowseDlg* pWnd = reinterpret_cast<CBrowseDlg*>(lpData);
	ASSERT_VALID(pWnd);
	ASSERT(NULL == pWnd->m_hWnd || hWnd == pWnd->m_hWnd);

	if (NULL == pWnd->m_hWnd && !pWnd->SubclassWindow(hWnd))
		AfxThrowOleException(HRESULT_FROM_WIN32(::GetLastError()));

	switch (nMsg)
	{
		case BFFM_INITIALIZED:
		// Indicates the browse dialog box has finished
		// initializing. The lParam value is zero.
		{
			pWnd->OnInitDialog();
			return 0;
		}

		case BFFM_SELCHANGED:
		// Indicates the selection has changed. The lParam parameter
		// points to the item identifier list for the newly selected
		// item.
		{
			LPITEMIDLIST p = reinterpret_cast<LPITEMIDLIST>(lParam);
			ASSERT_POINTER(p, ITEMIDLIST);
			pWnd->OnSelChanged(p);
			return 0;
		}

		case BFFM_VALIDATEFAILED:
		// Indicates the user typed an invalid name into the edit box
		// of the browse dialog box. The lParam parameter is the
		// address of a character buffer that contains the invalid
		// name. An application can use this message to inform the
		// user that the name entered was not valid. Return zero to
		// allow the dialog to be dismissed or nonzero to keep the
		// dialog displayed.
		{
			LPTSTR p = reinterpret_cast<LPTSTR>(lParam);
			ASSERT(!::IsBadStringPtr(p, UINT_MAX));
			BOOL bDismissOk = pWnd->OnValidateFailed(CString(p));
			return bDismissOk ? 0 : 1;
		}

		default:
			return 0;
	}
}

void CBrowseDlg::FreeItemIdList(IMalloc* pMalloc /*=NULL*/)
{
	if (m_pItemIdList == NULL)
		return;

	bool bWeRelease = false;

	if (pMalloc == NULL)
	{
		bWeRelease = true;
		HRESULT hr;
		if (FAILED(hr = ::SHGetMalloc(&pMalloc)))
			AfxThrowOleException(hr);
	}

	if (pMalloc)
	{
		pMalloc->Free(m_pItemIdList);
		if (bWeRelease)
			pMalloc->Release();
	}

	m_pItemIdList = NULL;
}

void CBrowseDlg::CalcCheckRect(CRect& rc)
{
	CRect rcDlg, rcWindowDlg;
	GetClientRect(&rcDlg);
	GetWindowRect(&rcWindowDlg);

	CRect rcOk, rcWindowOk;
	CButton* pOk = (CButton*)(GetDlgItem(IDOK));
	pOk->GetClientRect(&rcOk);
	pOk->GetWindowRect(&rcWindowOk);

	int nYBottomAlign = rcWindowDlg.bottom - rcWindowOk.bottom;

	// rc will be the rectangle, relative to rcDlg,
	// occupied by the Checkbox button.
	rc.left = rcDlg.left + ::SystemDPIScale(BROWSEFILEDLG_CHECKBOX_LEFT_OFFSET);
	rc.right = rc.left + m_nCheckboxWidth;
	rc.bottom = rcDlg.bottom - nYBottomAlign;
	rc.top = rc.bottom - rcOk.Height();
}

void CBrowseDlg::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	if (m_bCheckbox)
	{
		CRect rcButton;
		CalcCheckRect(rcButton);
		m_Checkbox.MoveWindow(&rcButton, TRUE);
	}
}
