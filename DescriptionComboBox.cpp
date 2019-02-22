#include "stdafx.h"
#include "DescriptionComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDescriptionComboBox::CDescriptionComboBox()
{
	m_bEditHeightSet = FALSE;
}

CDescriptionComboBox::~CDescriptionComboBox()
{

}

// Called for each entry when adding the string
// (as per microsoft doc we do not need to set lpMeasureItemStruct->itemWidth)
void CDescriptionComboBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	int itemID = lpMeasureItemStruct->itemID;
	if (itemID != CB_ERR)
	{
		CDC* pDC = GetDC();

		// Ensure the DC is using the combobox font when getting the text metrics
		CFont* pOldFont = pDC->SelectObject(GetFont());

		// Get the text height
		TEXTMETRIC tm;
		pDC->GetTextMetrics(&tm);
		lpMeasureItemStruct->itemHeight = tm.tmHeight + tm.tmExternalLeading;

		// Adjust the edit part of the combobox
		if (!m_bEditHeightSet)
		{
			m_bEditHeightSet = TRUE;
			SetItemHeight(-1, lpMeasureItemStruct->itemHeight + 2);
		}

		// Set the list item height
		lpMeasureItemStruct->itemHeight *= 2;
		lpMeasureItemStruct->itemHeight += 2;

		// Clean-up
		if (pOldFont)
			pDC->SelectObject(pOldFont);
		ReleaseDC(pDC);
	}
}

// Draw the row in the list box part of the combobox
void CDescriptionComboBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
    int itemID = lpDrawItemStruct->itemID;
	if (itemID != CB_ERR)
	{
		CDC dc;
		dc.Attach(lpDrawItemStruct->hDC);
		CRect rc = lpDrawItemStruct->rcItem;
		COLORREF crOldTextColor = dc.GetTextColor();
		COLORREF crOldBkColor = dc.GetBkColor();

		// If this item is selected, set the background color 
		// and the text color to appropriate values. Erase
		// the rect by filling it with the background color
		if ((lpDrawItemStruct->itemAction & ODA_SELECT) &&
			(lpDrawItemStruct->itemState  & ODS_SELECTED))
		{
			dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
			dc.SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
			dc.FillSolidRect(&lpDrawItemStruct->rcItem, ::GetSysColor(COLOR_HIGHLIGHT));
		}
		else
			dc.FillSolidRect(&lpDrawItemStruct->rcItem, crOldBkColor);

		// Draw name
		CString sName;
		int i = lpDrawItemStruct->itemData;
		if (i >= 0 && i < m_NameArray.GetCount())
			sName = m_NameArray[i];
		if (sName.IsEmpty())
			sName = _T("-------");
		dc.DrawText(sName, sName.GetLength(), &rc, DT_SINGLELINE | DT_LEFT | DT_TOP);
		
		// Draw host
		rc.bottom -= 2;
		if (!(HFONT)m_HostFont)
		{
			LOGFONT lf;
			GetFont()->GetLogFont(&lf);
			lf.lfItalic = TRUE;
			m_HostFont.CreateFontIndirect(&lf);
		}
		CFont* pOldFont = dc.SelectObject(&m_HostFont);
		dc.SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
		CString sHost;
		GetLBText(itemID, sHost);
		dc.DrawText(sHost, sHost.GetLength(), &rc, DT_SINGLELINE | DT_LEFT | DT_BOTTOM);
		if (pOldFont)
			dc.SelectObject(pOldFont);

		// Draw separator
		rc.bottom += 1;
		if (itemID < GetCount() - 1)
		{
			CPen Pen(PS_SOLID, 1, RGB(172, 172, 172));
			CPen* pOldPen = dc.SelectObject(&Pen);
			dc.MoveTo(rc.left, rc.bottom);
			dc.LineTo(rc.right, rc.bottom);
			if (pOldPen)
				dc.SelectObject(pOldPen);
		}

		// Clean-up
		dc.SetBkColor(crOldBkColor);
		dc.SetTextColor(crOldTextColor);
		dc.Detach();
	}
}
