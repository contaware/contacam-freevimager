// SoftenDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "SoftenDlg.h"
#include "PictureDoc.h"
#include "PictureView.h"
#include "NoVistaFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MIN_SOFTNESS		0
#define MAX_SOFTNESS		6
#define DEFAULT_SOFTNESS	2

/////////////////////////////////////////////////////////////////////////////
// CSoftenDlg dialog


CSoftenDlg::CSoftenDlg(CWnd* pParent)
	: CDialog(CSoftenDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSoftenDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	ASSERT_VALID(pView);
	pView->ForceCursor();
	CDialog::Create(CSoftenDlg::IDD, pParent);
}


void CSoftenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSoftenDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSoftenDlg, CDialog)
	//{{AFX_MSG_MAP(CSoftenDlg)
	ON_WM_CLOSE()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSoftenDlg message handlers

BOOL CSoftenDlg::OnInitDialog() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pDib =	pDoc->m_bBigPicture ?
					pDoc->m_pDib->GetPreviewDib() :
					pDoc->m_pDib;

	CDialog::OnInitDialog();
	
	// Init Slider
	CSliderCtrl* pSlider;
	pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SOFTNESS);
	pSlider->SetRange(MIN_SOFTNESS, MAX_SOFTNESS, TRUE);
	pSlider->SetLineSize(1);
	pSlider->SetPageSize(1);
	int nSoften = DEFAULT_SOFTNESS;
	if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
		nSoften = ::AfxGetApp()->GetProfileInt(_T("PictureDoc"), _T("Soften"), DEFAULT_SOFTNESS);
	pSlider->SetPos(nSoften);

	// Init Preview Undo Dib
	if (pDib)
		m_PreviewUndoDib = *pDib;

	// Soften
	BeginWaitCursor();
	Soften(pDib, NULL, nSoften);
	pDoc->UpdateAlphaRenderedDib();
	EndWaitCursor();
	pDoc->InvalidateAllViews(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CSoftenDlg::Soften(CDib* pDib, CDib* pSrcDib, int nSoftness)
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	switch (nSoftness)
	{
		case 1 : nSoftness = 28; break;
		case 2 : nSoftness = 14; break;
		case 3 : nSoftness = 8; break;
		case 4 : nSoftness = 4; break;
		case 5 : nSoftness = 2; break;
		case 6 : nSoftness = 1; break;
		default: return FALSE;
	}

	int Kernel[] = {1,1,1,
					1,nSoftness,1,
					1,1,1};

	return pDib->FilterFast(Kernel,
							nSoftness + 8,
							pSrcDib,
							pView,
							TRUE);
}

void CSoftenDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pDib =	pDoc->m_bBigPicture ?
					pDoc->m_pDib->GetPreviewDib() :
					pDoc->m_pDib;

	// Better to use Slider Pos Directly than the nPos parameter,
	// this because with the Line and Page Message nPos is always 0...
	if (pScrollBar)
	{
		CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;
		if ((SB_THUMBPOSITION == nSBCode) ||// Wheel On Mouse And End Of Dragging Slider
			(SB_LINEUP  == nSBCode) ||		// Keyboard Arrow
			(SB_LINEDOWN  == nSBCode) ||	// Keyboard Arrow
			(SB_PAGEUP == nSBCode) ||		// Mouse Press Above Slider
			(SB_PAGEDOWN == nSBCode) ||		// Mouse Press Belove Slider
			(SB_LEFT == nSBCode)	||		// Home Button
			(SB_RIGHT == nSBCode))			// End Button  
		{
			if (pDib)
			{
				BeginWaitCursor();
				Undo();
				int nSoften = pSlider->GetPos();
				Soften(pDib, NULL, nSoften);
				if (((CUImagerApp*)::AfxGetApp())->m_bUseSettings)
					::AfxGetApp()->WriteProfileInt(_T("PictureDoc"), _T("Soften"), nSoften);
				pDoc->UpdateAlphaRenderedDib();
				EndWaitCursor();
				pDoc->InvalidateAllViews(FALSE);
			}
		}
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CSoftenDlg::Close()
{
	OnClose();
}

void CSoftenDlg::OnClose() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	Undo();
	pDoc->UpdateAlphaRenderedDib();
	pDoc->InvalidateAllViews(FALSE);

	DestroyWindow();
}

BOOL CSoftenDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (HIWORD (wParam) == BN_CLICKED)
	{
		switch (LOWORD (wParam))
		{
			case IDOK:
				DoIt();
				return TRUE;
			case IDCANCEL:
				Close();
				return TRUE;
			default:
				return CDialog::OnCommand(wParam, lParam);
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}

void CSoftenDlg::PostNcDestroy() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();

	pDoc->m_pSoftenDlg = NULL;
	pView->ForceCursor(FALSE);
	delete this;	
	CDialog::PostNcDestroy();
}

void CSoftenDlg::Undo() 
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CDib* pDib =	pDoc->m_bBigPicture ?
					pDoc->m_pDib->GetPreviewDib() :
					pDoc->m_pDib;

	if (pDib && m_PreviewUndoDib.IsValid())
		*pDib = m_PreviewUndoDib;
}

void CSoftenDlg::DoIt()
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SOFTNESS);

	if (!pSlider || pSlider->GetPos() <= 0)
		Close();
	else if (pDoc->m_bBigPicture)
	{
		if (!DoItBigPicture())
			Close();
		else
			DestroyWindow();
	}
	else
	{
		pDoc->AddUndo(&m_PreviewUndoDib);
		pDoc->SetModifiedFlag();
		pDoc->SetDocumentTitle();
		pDoc->UpdateImageInfo();
		DestroyWindow();
	}
}

BOOL CSoftenDlg::DoItBigPicture()
{
	CPictureView* pView = (CPictureView*)m_pParentWnd;
	CPictureDoc* pDoc = (CPictureDoc*)pView->GetDocument();
	CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SOFTNESS);

	if (pDoc->m_pDib && pSlider)
	{
		CString sFilteredFileName;
		int nID;
		if (pDoc->m_pDib->IsMMReadOnly())
			nID = IDYES;
		else
			nID = ::AfxMessageBox(ML_STRING(1421, "Do You Want To Save The Softened Image To A New File?"), MB_YESNOCANCEL);
		if (nID == IDYES)
		{
			// Display the Save As Dialog
			TCHAR szFileName[MAX_PATH];
			CNoVistaFileDlg dlgFile(FALSE);
			sFilteredFileName = pDoc->m_sFileName;
			int index = sFilteredFileName.ReverseFind(_T('.'));	
			if (index > 0)
				sFilteredFileName.Insert(index, _T("_softened"));
			else
				return FALSE;
	
			_tcscpy(szFileName, sFilteredFileName);
			dlgFile.m_ofn.lpstrFile = szFileName;
			dlgFile.m_ofn.nMaxFile = MAX_PATH;
			dlgFile.m_ofn.lpstrCustomFilter = NULL;
			dlgFile.m_ofn.Flags |= OFN_EXPLORER;
			dlgFile.m_ofn.lpstrFilter = _T("Windows Bitmap (*.bmp;*.dib)\0*.bmp;*.dib\0");
			dlgFile.m_ofn.lpstrDefExt = _T("bmp");
			if (dlgFile.DoModal() == IDOK)
			{
				sFilteredFileName = szFileName;
				if (pDoc->m_pDib->IsMMReadOnly() && sFilteredFileName == pDoc->m_sFileName)
				{
					::AfxMessageBox(ML_STRING(1275, "Cannot save to ourself"), MB_ICONSTOP);
					return FALSE;
				}
			}
			else
				return FALSE;
		}
		else if (nID == IDNO)
		{
			// Temporary File
			sFilteredFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), pDoc->m_sFileName);
		}
		else // Cancel
			return FALSE;

		// Check
		if (sFilteredFileName == pDoc->m_sFileName)
		{
			nID = IDNO;
			sFilteredFileName = ::MakeTempFileName(((CUImagerApp*)::AfxGetApp())->GetAppTempDir(), pDoc->m_sFileName);
		}

		// Create New Filtered Dib in File
		CDib FilteredDib;
		FilteredDib.SetBMI(pDoc->m_pDib->GetBMI());
		if (!FilteredDib.MMCreateBMP(sFilteredFileName))
			return FALSE;

		// Begin Wait Cursor
		BeginWaitCursor();

		// Do Softening
		BOOL res = Soften(&FilteredDib, pDoc->m_pDib, pSlider->GetPos());
		
		// Free (This Closes the Memory Mapped Files!)
		pDoc->m_pDib->Free();
		FilteredDib.Free();
		
		// If Ok
		if (res)
		{
			// Remove and Rename File
			if (nID == IDNO)
			{
				try
				{
					CFile::Remove(pDoc->m_sFileName);
					CFile::Rename(sFilteredFileName, pDoc->m_sFileName);
				}
				catch (CFileException* e)
				{
					EndWaitCursor();
					::DeleteFile(sFilteredFileName);

					DWORD dwAttrib = ::GetFileAttributes(pDoc->m_sFileName);
					if ((dwAttrib != 0xFFFFFFFF) && (dwAttrib & FILE_ATTRIBUTE_READONLY))
					{
						CString str(ML_STRING(1255, "The file is read only\n"));
						TRACE(str);
						if (pDoc->IsShowMessageBoxOnError())
							::AfxMessageBox(str, MB_ICONSTOP);
					}
					else
						::ShowError(e->m_lOsError, pDoc->IsShowMessageBoxOnError());

					e->Delete();
					return FALSE;
				}
			}
		}
		else
		{		
			EndWaitCursor();

			CString str;
			str = ML_STRING(1420, "Error while softening the big picture\n");
			TRACE(str);
			if (pDoc->IsShowMessageBoxOnError())
				::AfxMessageBox(str, MB_ICONSTOP);

			return FALSE;
		}

		// Load
		if (nID == IDNO)
			pDoc->LoadBigPicture(pDoc->m_sFileName);
		else
			pDoc->LoadBigPicture(sFilteredFileName);

		EndWaitCursor();

		return TRUE;
	}
	else
		return FALSE;
}