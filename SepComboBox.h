/****************************************************************************************************************
(C) Copyright 2004 Zuoliu Ding.  All Rights Reserved.
SepComboBox.h: Interface for the CSeparatorComboBox class.
Created by:    01/30/2004, Zuoliu Ding
Note:          For a Combo box with Separators

Usage:   if CSeparatorComboBox cb; you have to call:
      For example,
         cb.SetSeparator(0);                 // Set Separator after the first one
         cb.SetSeparator(-2);                // Set Separator before last two items

      These methods are optional:
         cb.AdjustItemHeight();              // Adjust item height for Separators, Def: Inc=3
         cb.SetSepLineStyle(PS_DASH);        // Set Separator to dash Lines, Def: dot line
         cb.SetSepLineColor(RGB(0, 0, 128)); // Set Separator Color to blue, Def: dark gray 64
         cb.SetSepLineWidth(2);              // Set Separator Line Width, Def: 1 pixel
         cb.SetBottomMargin(3);              // Set Separator Bottom Margin, Def: 2 pixels
         cb.SetHorizontalMargin(1);          // Set Separator Horizontal Margin, Def: 2 pixels
****************************************************************************************************************/

#pragma once

#include <afxtempl.h>

// CSeparatorComboBox
class CSeparatorComboBox : public CComboBox
{
   DECLARE_DYNAMIC(CSeparatorComboBox)

   CListBox    m_listbox;
   CArray<int, int> m_arySeparators;

   int         m_nHorizontalMargin;
   int         m_nBottomMargin;
   int         m_nSepWidth;
   int         m_nPenStyle;
   COLORREF    m_crColor;

public:
   CSeparatorComboBox();
   virtual ~CSeparatorComboBox();

   void SetSeparator(int iSep);
   void AdjustItemHeight(int nInc=3);

   void SetSepLineStyle(int iSep) { m_nPenStyle = iSep; }
   void SetSepLineColor(COLORREF crColor) { m_crColor = crColor; }
   void SetSepLineWidth(int iWidth) { m_nSepWidth = iWidth; }
   void SetBottomMargin(int iMargin) { m_nBottomMargin = iMargin; }
   void SetHorizontalMargin(int iMargin) { m_nHorizontalMargin = iMargin; }

protected:
   DECLARE_MESSAGE_MAP()

public:
   afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
   afx_msg void OnDestroy();
};


