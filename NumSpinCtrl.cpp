// NumSpinCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "NumSpinCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNumSpinCtrl

IMPLEMENT_DYNAMIC(CNumSpinCtrl, CSpinButtonCtrl)

CNumSpinCtrl::CNumSpinCtrl()
{
}

CNumSpinCtrl::~CNumSpinCtrl()
{
}


BEGIN_MESSAGE_MAP(CNumSpinCtrl, CSpinButtonCtrl)
	//{{AFX_MSG_MAP(CNumSpinCtrl)
	ON_NOTIFY_REFLECT(UDN_DELTAPOS, OnDeltapos)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNumSpinCtrl message handlers

void CNumSpinCtrl::SetPos(double val)
{
	CSpinButtonCtrl::SetPos(500);
	CNumEdit *m_Edit = GetBuddy();
	if (m_Edit)
		if (m_Edit->IsKindOf(RUNTIME_CLASS(CNumEdit)))
			m_Edit->SetValue(val);
}

double CNumSpinCtrl::GetPos()
{
	CNumEdit *m_Edit = GetBuddy();
	if (m_Edit)
		if (m_Edit->IsKindOf(RUNTIME_CLASS(CNumEdit)))
			return m_Edit->GetValue();
	return 0.0;
}

void CNumSpinCtrl::SetRange(double lower, double upper)
{
	CSpinButtonCtrl::SetRange(0, 1000);
	CNumEdit* m_Edit = GetBuddy();
	if (m_Edit)
	{
		if (m_Edit->IsKindOf(RUNTIME_CLASS(CNumEdit)))
		{
			m_Edit->SetRange(lower, upper);
		}
	}
}

void CNumSpinCtrl::GetRange(double & lower, double & upper) const
{
	CNumEdit *m_Edit = GetBuddy();
	if (m_Edit)
		if (m_Edit->IsKindOf(RUNTIME_CLASS(CNumEdit)))
			m_Edit->GetRange(lower, upper);
}

CNumEdit* CNumSpinCtrl::GetBuddy() const
{
	return (CNumEdit *)CSpinButtonCtrl::GetBuddy();
}

void CNumSpinCtrl::SetBuddy(CNumEdit * edit)
{
	CSpinButtonCtrl::SetBuddy(edit);
	if (edit)
		if (edit->IsKindOf(RUNTIME_CLASS(CNumEdit)))
			edit->SetValue(0.0);
}

void CNumSpinCtrl::OnDeltapos(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	
	if ((pNMUpDown->iPos <= pNMUpDown->iDelta) ||
		(pNMUpDown->iPos >= 1000 - pNMUpDown->iDelta)) pNMUpDown->iPos = 500;

	CNumEdit *m_Edit = GetBuddy();
	if (m_Edit)
		if (m_Edit->IsKindOf(RUNTIME_CLASS(CNumEdit)))
			m_Edit->ChangeAmount(pNMUpDown->iDelta);

	*pResult = 0;
}

double CNumSpinCtrl::GetDelta()
{
	CNumEdit *m_Edit = GetBuddy();
	if (m_Edit)
		if (m_Edit->IsKindOf(RUNTIME_CLASS(CNumEdit)))
			return m_Edit->GetDelta();
	return 0.0;
}

void CNumSpinCtrl::SetDelta(double delta)
{
	CNumEdit *m_Edit = GetBuddy();
	if (m_Edit)
		if (m_Edit->IsKindOf(RUNTIME_CLASS(CNumEdit)))
			m_Edit->SetDelta(delta);
}
