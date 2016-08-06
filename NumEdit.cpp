// NumEdit.cpp : implementation file
//

#include "stdafx.h"
#include "NumEdit.h"
#include "Float.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNumEdit

IMPLEMENT_DYNAMIC(CNumEdit, CEdit)

CNumEdit::CNumEdit()
{
	m_nNumberOfNumberAfterPoint = 0;
	m_nMaxNumberOfNumberAfterPoint = 4;
	m_bVerbose = TRUE;
	m_dMinValue = -DBL_MAX;
	m_dMaxValue = DBL_MAX;
	m_dDelta = _DBL_ROUNDS;
	m_dOldValue = 0.0;
	m_bOldValueValid = FALSE;
	m_sOldValue = _T("");
}

CNumEdit::~CNumEdit()
{

}

BEGIN_MESSAGE_MAP(CNumEdit, CEdit)
	//{{AFX_MSG_MAP(CNumEdit)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNumEdit message handlers

double CNumEdit::GetValue() const
{
	double d;
	if (m_bOldValueValid)
		d = m_dOldValue;
	else
		d = m_dMinValue;
	if (IsValid(FALSE) == VALID)
	{
		CString str;
		GetWindowText(str);
		_stscanf(str, _T("%lf"), &d);
	}
	return d;
}

BOOL CNumEdit::SetValue(double val)
{
	if (val > m_dMaxValue || val < m_dMinValue)
		return FALSE;
	CString str, s;
	str.Format(ConstructFormat(s, val), val);
	SetWindowText(str);
	m_bOldValueValid = TRUE;
	m_sOldValue = str;
	m_dOldValue = val;
	return TRUE;
}

void CNumEdit::RestoreOldValue()
{
	SetWindowText(m_sOldValue);
}

int CNumEdit::IsValid(BOOL bVerbose/*=TRUE*/) const
{
	CString str;
	GetWindowText(str);
	return IsValid(str, bVerbose);
}

int CNumEdit::IsValid(const CString &str, BOOL bVerbose/*=TRUE*/) const
{
	int res = VALID;
	double d;
	CString sRemaining;
	LPTSTR lpzsRemaining = sRemaining.GetBuffer(str.GetLength());
	if (str.GetLength() == 0)
		res = EMPTY;
	else if ((str.GetLength() == 1) 
					&&
			(
			(str[0] == _T('+')) ||
			(str[0] == _T('-')) ||
			(str[0] == _T('.')) ||
			(str[0] == _T('0'))
			)
			)
		res = MINUS_PLUS_POINT_ZERO;
	else if ((str.GetLength() == 2)
					&&
			(
			(str[0] == _T('+')) ||
			(str[0] == _T('-')) ||
			(str[0] == _T('.')) ||
			(str[0] == _T('0'))
			)		&&
			(
			(str[1] == _T('.')) ||
			(str[1] == _T('0'))))
				res = MINUS_PLUS_POINT_ZERO;
	else if ((str.GetLength() == 3)
					&&
			(
			(str[0] == _T('+')) ||
			(str[0] == _T('-')) ||
			(str[0] == _T('.')) ||
			(str[0] == _T('0'))
			)		&&
			(
			(str[1] == _T('.')) ||
			(str[1] == _T('0'))
					&&
			(
			(str[2] == _T('.')))))
					res = MINUS_PLUS_POINT_ZERO;
	else if (_stscanf(str, _T("%lf%s"), &d, lpzsRemaining) != 1)
		res = INVALID_CHAR;
	else if (d > m_dMaxValue || d < m_dMinValue)
		res = OUT_OF_RANGE;
	if (m_bVerbose && bVerbose &&
		(res != VALID) &&
		(res != MINUS_PLUS_POINT_ZERO) &&
		(res != EMPTY))
	{
		if (res & OUT_OF_RANGE)
		{
			CString msg, format1, format2;
			format1.Format(_T("%%.%df"), m_nMaxNumberOfNumberAfterPoint);
			format2.Format(ML_STRING(1400, "Please enter a number between %s and %s"), format1, format1);
			msg.Format(format2, m_dMinValue, m_dMaxValue);
			::AfxMessageBox(msg, MB_OK);
		}
	}
	sRemaining.ReleaseBuffer();
	return res;
}

void CNumEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == _T(' '))
		return;
	int nOldPos = CharFromPos(GetCaretPos());
	DWORD dwOldSel = GetSel();
	CEdit::OnChar(nChar, nRepCnt, nFlags);	// Gets New Value
	int val = IsValid();
	MSG msg;
	switch (val)
	{
		case VALID:
			GetWindowText(m_sOldValue);
			m_dOldValue = GetValue();
			m_bOldValueValid = TRUE;
			break;
		case MINUS_PLUS_POINT_ZERO:
		case EMPTY: 
			break;
		case INVALID_CHAR:
			::MessageBeep((DWORD)-1);
			RestoreOldValue();
			if (dwOldSel)
				SetSel(dwOldSel);
			else
				SetSel(nOldPos, nOldPos);
			while (::PeekMessage(&msg, m_hWnd, WM_CHAR, WM_CHAR, PM_REMOVE));
			break;
		default:
			if (m_bOldValueValid)
				SetValue(m_dOldValue);
			else
				SetValue(m_dMinValue);
			SetSel(0, -1);
			while (::PeekMessage(&msg, m_hWnd, WM_CHAR, WM_CHAR, PM_REMOVE));
			break;
	}
}

BOOL CNumEdit::Verbose() const
{
	return m_bVerbose;
}

void CNumEdit::Verbose(BOOL v)
{
	m_bVerbose = v;
}

void CNumEdit::SetRange(double min, double max)
{
	m_dMinValue = min;
	m_dMaxValue = max;
}

void CNumEdit::SetMinNumberOfNumberAfterPoint(int nMinNumberOfNumberAfterPoint)
{
	m_nNumberOfNumberAfterPoint = nMinNumberOfNumberAfterPoint;
}

void CNumEdit::SetMaxNumberOfNumberAfterPoint(int nMaxNumberOfNumberAfterPoint)
{
	m_nMaxNumberOfNumberAfterPoint = nMaxNumberOfNumberAfterPoint;
}

void CNumEdit::GetRange(double & min, double & max) const
{
	min = m_dMinValue;
	max = m_dMaxValue;
}

void CNumEdit::SetDelta(double delta)
{
	m_dDelta = delta;
}

double CNumEdit::GetDelta()
{
	return m_dDelta;
}

void CNumEdit::ChangeAmount(int step)
{
	double d = GetValue() + step * m_dDelta;
	if (d > m_dMaxValue)
		d = m_dMaxValue;
	else if (d < m_dMinValue)
		d = m_dMinValue;
	SetValue(d);
}

CString& CNumEdit::ConstructFormat(CString &str, double num)
{
	str.Format(_T("%G"), num);
	int n = str.Find(_T('.'));
	if (n >= 0)
	{
		n = str.GetLength() - n - 1;

		if (n > m_nMaxNumberOfNumberAfterPoint)
			n = m_nMaxNumberOfNumberAfterPoint;

		if (n > m_nNumberOfNumberAfterPoint)
			m_nNumberOfNumberAfterPoint = n;
	}
	str.Format(_T("%%.%df"), m_nNumberOfNumberAfterPoint);
	return str;
}
