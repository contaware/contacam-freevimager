/****************************************************************************************************************
(C) Copyright 2004 Zuoliu Ding.  All Rights Reserved.
SepComboBox.cpp: Implementation file
Created by:      01/30/2004, Zuoliu Ding
Note:            For a Combo box with Separators
Ref:             http://support.microsoft.com/default.aspx?scid=kb;en-us;Q174667
****************************************************************************************************************/

#include "stdafx.h"
#include "SepComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CSeparatorComboBox

IMPLEMENT_DYNAMIC(CSeparatorComboBox, CComboBox)
CSeparatorComboBox::CSeparatorComboBox(): 
   m_nPenStyle(PS_DOT), 
   m_crColor(RGB(64, 64, 64)),
   m_nBottomMargin(2),
   m_nSepWidth(1),
   m_nHorizontalMargin(2)
{
}

CSeparatorComboBox::~CSeparatorComboBox()
{
}


BEGIN_MESSAGE_MAP(CSeparatorComboBox, CComboBox)
   ON_WM_CTLCOLOR()
   ON_WM_DESTROY()
END_MESSAGE_MAP()

// CSeparatorComboBox message handlers

HBRUSH CSeparatorComboBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
   if (nCtlColor == CTLCOLOR_LISTBOX)
   {
      if (m_listbox.GetSafeHwnd() ==NULL)
      {
         m_listbox.SubclassWindow(pWnd->GetSafeHwnd());
      }

      CRect r;
      int   nIndex, n = m_listbox.GetCount();

      CPen pen(m_nPenStyle, m_nSepWidth, m_crColor), *pOldPen;
      pOldPen = pDC->SelectObject(&pen);

      for (int i=0; i< m_arySeparators.GetSize(); i++)
      {
         nIndex = m_arySeparators[i];
         if (nIndex<0) nIndex += n-1;

         if (nIndex < n-1)
         {
            m_listbox.GetItemRect(nIndex, &r );
            pDC->MoveTo(r.left+m_nHorizontalMargin, r.bottom-m_nBottomMargin);
            pDC->LineTo(r.right-m_nHorizontalMargin, r.bottom-m_nBottomMargin);
         }
      }

      pDC->SelectObject(pOldPen);
   }

   // TODO:  Return a different brush if the default is not desired
   //HBRUSH hbr = CComboBox::OnCtlColor(pDC, pWnd, nCtlColor);
   //return hbr;      // (HBRUSH)COLOR_WINDOW;   
   return CComboBox::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CSeparatorComboBox::OnDestroy()
{
   if (m_listbox.GetSafeHwnd() !=NULL)
      m_listbox.UnsubclassWindow();

   CComboBox::OnDestroy();
}

   
void CSeparatorComboBox::SetSeparator(int iSep) 
{ 
   if (!m_arySeparators.GetSize())
      AdjustItemHeight();

   m_arySeparators.Add(iSep); 
}


void CSeparatorComboBox::AdjustItemHeight(int nInc/*=3*/)
{
   SetItemHeight(0, nInc+ GetItemHeight(0));
}
