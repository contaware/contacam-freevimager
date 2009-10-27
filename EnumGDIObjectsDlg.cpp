// EnumGDIObjectsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "uimager.h"
#include "EnumGDIObjectsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEnumGDIObjectsDlg dialog


CEnumGDIObjectsDlg::CEnumGDIObjectsDlg(CWnd* pParent)
	: CDialog(CEnumGDIObjectsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEnumGDIObjectsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	CDialog::Create(CEnumGDIObjectsDlg::IDD, pParent);
}


void CEnumGDIObjectsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEnumGDIObjectsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEnumGDIObjectsDlg, CDialog)
	//{{AFX_MSG_MAP(CEnumGDIObjectsDlg)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEnumGDIObjectsDlg message handlers

BOOL CEnumGDIObjectsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetTimer(ID_TIMER_GDI_OBJECTS_DLG, GDIOBJECTSDLG_TIMER_MS, NULL);

	EnumGDIs();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEnumGDIObjectsDlg::OnClose() 
{
	KillTimer(ID_TIMER_GDI_OBJECTS_DLG);
	DestroyWindow();
}

void CEnumGDIObjectsDlg::Close()
{
	OnClose();
}

void CEnumGDIObjectsDlg::PostNcDestroy() 
{
	::AfxGetMainFrame()->m_pEnumGDIObjectsDlg = NULL;
	delete this;	
	CDialog::PostNcDestroy();
}

BOOL CEnumGDIObjectsDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (HIWORD (wParam) == BN_CLICKED)
	{
		switch (LOWORD (wParam))
		{
			case IDOK:
				KillTimer(ID_TIMER_GDI_OBJECTS_DLG);
				DestroyWindow();
				return TRUE;
			case IDCANCEL:
				KillTimer(ID_TIMER_GDI_OBJECTS_DLG);
				DestroyWindow();
				return TRUE;
			default:
				return CDialog::OnCommand(wParam, lParam);
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}

BOOL CEnumGDIObjectsDlg::GDIHandleSeemsValid(DWORD hGDI, DWORD ObjectType)
{
   switch (ObjectType)
   {
   // stored in 16-bit GDI heap
   // --> handle value must end in 2, 6, A or E
   //     and must be less than 0x6000
      case OBJ_BITMAP:
      case OBJ_BRUSH:
      case OBJ_PAL:
      case OBJ_PEN:
      case OBJ_DC:
      case OBJ_MEMDC:
      case OBJ_EXTPEN:
      case OBJ_METADC:
      case OBJ_ENHMETADC:
         return(
            (hGDI & 2) && 
            ((hGDI % 2) == 0) &&
            (hGDI <= 0x6000)
            );
      break;

   // special case
   // --> undocumented...
      case OBJ_METAFILE:
      case OBJ_ENHMETAFILE:
      case OBJ_COLORSPACE:
         return(TRUE);
      break;

   // stored in 32-bit GDI heap
   // --> handle value is a multiple of 4
      case OBJ_FONT:
      case OBJ_REGION:
         return((hGDI % 4) == 0);
      break;

      default:
      return(FALSE);
   }
}

void CEnumGDIObjectsDlg::EnumGDIs()
{
	const int dim1 = 14;
	const int dim2 = 65536; // 65536 is the max num of handles for a process
	DWORD* handlesArray = new DWORD[dim1*dim2];
	CString str(_T(""));
	DWORD nBitmapCount = 0;
	DWORD nBrushCount = 0;
	DWORD nFontCount = 0;
	DWORD nPalCount = 0;
	DWORD nPenCount = 0;
	DWORD nExtpenCount = 0;
	DWORD nRegionCount = 0;
	DWORD nDCCount = 0;
	DWORD nMemDCCount = 0;
	DWORD nMetafileCount = 0;
	DWORD nMetafileDCCount = 0;
	DWORD nEnhMetafileCount = 0;
	DWORD nEnhMetafileDCCount = 0;
	DWORD nUnknownResCount = 0;
	DWORD nTotal = 0;

	DWORD minBitmapAddr = 0;
	DWORD maxBitmapAddr = 0;
	DWORD minBrushAddr = 0;
	DWORD maxBrushAddr = 0;
	DWORD minFontAddr = 0;
	DWORD maxFontAddr = 0;
	DWORD minPalAddr = 0;
	DWORD maxPalAddr = 0;
	DWORD minPenAddr = 0;
	DWORD maxPenAddr = 0;
	DWORD minExtpenAddr = 0;
	DWORD maxExtpenAddr = 0;
	DWORD minRegionAddr = 0;
	DWORD maxRegionAddr = 0;
	DWORD minDCAddr = 0;
	DWORD maxDCAddr = 0;
	DWORD minMemDCAddr = 0;
	DWORD maxMemDCAddr = 0;
	DWORD minMetafileAddr = 0;
	DWORD maxMetafileAddr = 0;
	DWORD minMetafileDCAddr = 0;
	DWORD maxMetafileDCAddr = 0;
	DWORD minEnhMetafileAddr = 0;
	DWORD maxEnhMetafileAddr = 0;
	DWORD minEnhMetafileDCAddr = 0;
	DWORD maxEnhMetafileDCAddr = 0;
	DWORD minUnknownResAddr = 0;
	DWORD maxUnknownResAddr = 0;

	DWORD i = 1;
	DWORD dwObjectType;
	do 
	{
		dwObjectType = ::GetObjectType((HGDIOBJ)i);
		if (GDIHandleSeemsValid(i, dwObjectType))
		{
			switch (dwObjectType)
			{
				case 0 :
					break;
				case OBJ_BITMAP :		handlesArray[nBitmapCount++] = i;
					break;
				case OBJ_BRUSH :		handlesArray[1*dim2 + nBrushCount++] = i;
					break;
				case OBJ_FONT :			handlesArray[2*dim2 + nFontCount++] = i;
					break;
				case OBJ_PAL :			handlesArray[3*dim2 + nPalCount++] = i;
					break;
				case OBJ_PEN :			handlesArray[4*dim2 + nPenCount++] = i;
					break;
				case OBJ_EXTPEN :		handlesArray[5*dim2 + nExtpenCount++] = i;
					break;
				case OBJ_REGION :		handlesArray[6*dim2 + nRegionCount++] = i;
					break;
				case OBJ_DC :			handlesArray[7*dim2 + nDCCount++] = i;
					break;
				case OBJ_MEMDC :		handlesArray[8*dim2 + nMemDCCount++] = i;
					break;
				case OBJ_METAFILE :		handlesArray[9*dim2 + nMetafileCount++] = i;
					break;
				case OBJ_METADC :		handlesArray[10*dim2 + nMetafileDCCount++] = i;
					break;
				case OBJ_ENHMETAFILE :	handlesArray[11*dim2 + nEnhMetafileCount++] = i;
					break;
				case OBJ_ENHMETADC :	handlesArray[12*dim2 + nEnhMetafileDCCount++] = i;
					break;
				default :				handlesArray[13*dim2 + nUnknownResCount++] = i;
					break;
			}
		}
		i++;
	}
	while (i <= 0xFFFF);
	
	nTotal =	nBitmapCount + nBrushCount + nFontCount + nPalCount +
				nPenCount + nExtpenCount + nRegionCount + nDCCount +
				nMemDCCount + nMetafileCount + nMetafileDCCount + nEnhMetafileCount +
				nEnhMetafileDCCount + nUnknownResCount;


	minBitmapAddr = maxBitmapAddr = handlesArray[0];
	for(i = 1 ; i < nBitmapCount ; i++)
	{
		minBitmapAddr = min(minBitmapAddr, handlesArray[i]);
		maxBitmapAddr = max(maxBitmapAddr, handlesArray[i]);
	}
	minBrushAddr = maxBrushAddr = handlesArray[1*dim2 + 0];
	for(i = 1 ; i < nBrushCount ; i++)
	{
		minBrushAddr = min(minBrushAddr, handlesArray[1*dim2 + i]);
		maxBrushAddr = max(maxBrushAddr, handlesArray[1*dim2 + i]);
	}
	minFontAddr = maxFontAddr = handlesArray[2*dim2 + 0];
	for(i = 1 ; i < nFontCount ; i++)
	{
		minFontAddr = min(minFontAddr, handlesArray[2*dim2 + i]);
		maxFontAddr = max(maxFontAddr, handlesArray[2*dim2 + i]);
	}
	minPalAddr = maxPalAddr = handlesArray[3*dim2 + 0];
	for(i = 1 ; i < nPalCount ; i++)
	{
		minPalAddr = min(minPalAddr, handlesArray[3*dim2 + i]);
		maxPalAddr = max(maxPalAddr, handlesArray[3*dim2 + i]);
	}
	minPenAddr = maxPenAddr = handlesArray[4*dim2 + 0];
	for(i = 1 ; i < nPenCount ; i++)
	{
		minPenAddr = min(minPenAddr, handlesArray[4*dim2 + i]);
		maxPenAddr = max(maxPenAddr, handlesArray[4*dim2 + i]);
	}
	minExtpenAddr = maxExtpenAddr = handlesArray[5*dim2 + 0];
	for(i = 1 ; i < nExtpenCount ; i++)
	{
		minExtpenAddr = min(minExtpenAddr, handlesArray[5*dim2 + i]);
		maxExtpenAddr = max(maxExtpenAddr, handlesArray[5*dim2 + i]);
	}
	minRegionAddr = maxRegionAddr = handlesArray[6*dim2 + 0];
	for(i = 1 ; i < nRegionCount ; i++)
	{
		minRegionAddr = min(minRegionAddr, handlesArray[6*dim2 + i]);
		maxRegionAddr = max(maxRegionAddr, handlesArray[6*dim2 + i]);
	}
	minDCAddr = maxDCAddr = handlesArray[7*dim2 + 0];
	for(i = 1 ; i < nDCCount ; i++)
	{
		minDCAddr = min(minDCAddr, handlesArray[7*dim2 + i]);
		maxDCAddr = max(maxDCAddr, handlesArray[7*dim2 + i]);
	}
	minMemDCAddr = maxMemDCAddr = handlesArray[8*dim2 + 0];
	for(i = 1 ; i < nMemDCCount ; i++)
	{
		minMemDCAddr = min(minMemDCAddr, handlesArray[8*dim2 + i]);
		maxMemDCAddr = max(maxMemDCAddr, handlesArray[8*dim2 + i]);
	}
	minMetafileAddr = maxMetafileAddr = handlesArray[9*dim2 + 0];
	for(i = 1 ; i < nMetafileCount ; i++)
	{
		minMetafileAddr = min(minMetafileAddr, handlesArray[9*dim2 + i]);
		maxMetafileAddr = max(maxMetafileAddr, handlesArray[9*dim2 + i]);
	}
	minMetafileDCAddr = maxMetafileDCAddr = handlesArray[10*dim2 + 0];
	for(i = 1 ; i < nMetafileDCCount ; i++)
	{
		minMetafileDCAddr = min(minMetafileDCAddr, handlesArray[10*dim2 + i]);
		maxMetafileDCAddr = max(maxMetafileDCAddr, handlesArray[10*dim2 + i]);
	}
	minEnhMetafileAddr = maxEnhMetafileAddr = handlesArray[11*dim2 + 0];
	for(i = 1 ; i < nEnhMetafileCount ; i++)
	{
		minEnhMetafileAddr = min(minEnhMetafileAddr, handlesArray[11*dim2 + i]);
		maxEnhMetafileAddr = max(maxEnhMetafileAddr, handlesArray[11*dim2 + i]);
	}
	minEnhMetafileDCAddr = maxEnhMetafileDCAddr = handlesArray[12*dim2 + 0];
	for(i = 1 ; i < nEnhMetafileDCCount ; i++)
	{
		minEnhMetafileDCAddr = min(minEnhMetafileDCAddr, handlesArray[12*dim2 + i]);
		maxEnhMetafileDCAddr = max(maxEnhMetafileDCAddr, handlesArray[12*dim2 + i]);
	}
	minUnknownResAddr = maxUnknownResAddr = handlesArray[13*dim2 + 0];
	for(i = 1 ; i < nUnknownResCount ; i++)
	{
		minUnknownResAddr = min(minUnknownResAddr, handlesArray[13*dim2 + i]);
		maxUnknownResAddr = max(maxUnknownResAddr, handlesArray[13*dim2 + i]);
	}


	str.Format(_T("Bitmap : %d (0x%08X - 0x%08X)\n\
Brush : %d (0x%08X - 0x%08X)\n\
Font : %d (0x%08X - 0x%08X)\n\
Palette : %d (0x%08X - 0x%08X)\n\
Pen : %d (0x%08X - 0x%08X)\n\
Extended pen : %d (0x%08X - 0x%08X)\n\
Region : %d (0x%08X - 0x%08X)\n\
Device context : %d (0x%08X - 0x%08X)\n\
Memory device context : %d (0x%08X - 0x%08X)\n\
Metafile : %d (0x%08X - 0x%08X)\n\
Metafile device context : %d (0x%08X - 0x%08X)\n\
Enhanced metafile : %d (0x%08X - 0x%08X)\n\
Enhanceded metafile dc : %d (0x%08X - 0x%08X)\n\
Unknown resource object : %d (0x%08X - 0x%08X)\n\n\
Total : %d"),
nBitmapCount, minBitmapAddr, maxBitmapAddr,
nBrushCount, minBrushAddr, maxBrushAddr,
nFontCount, minFontAddr, maxFontAddr,
nPalCount, minPalAddr, maxPalAddr,
nPenCount, minPenAddr, maxPenAddr,
nExtpenCount, minExtpenAddr, maxExtpenAddr,
nRegionCount, minRegionAddr, maxRegionAddr,
nDCCount, minDCAddr, maxDCAddr,
nMemDCCount, minMemDCAddr, maxMemDCAddr,
nMetafileCount, minMetafileAddr, maxMetafileAddr,
nMetafileDCCount, minMetafileDCAddr, maxMetafileDCAddr,
nEnhMetafileCount, minEnhMetafileAddr, maxEnhMetafileAddr,
nEnhMetafileDCCount, minEnhMetafileDCAddr, maxEnhMetafileDCAddr,
nUnknownResCount, minUnknownResAddr, maxUnknownResAddr,
nTotal);

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_GDIOBJECTS);
	pEdit->SetWindowText(str);

	delete [] handlesArray;
}

void CEnumGDIObjectsDlg::OnTimer(UINT nIDEvent) 
{
	EnumGDIs();	
	CDialog::OnTimer(nIDEvent);
}
