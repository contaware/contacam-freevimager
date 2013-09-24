#if !defined(AFX_NUMEDIT_H__7482F7FF_A479_11D2_A6A7_00600844997A__INCLUDED_)
#define AFX_NUMEDIT_H__7482F7FF_A479_11D2_A6A7_00600844997A__INCLUDED_

#pragma once

// NumEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNumEdit window

#ifndef CLASS_EXPORT
	#define CLASS_EXPORT
#endif

class CLASS_EXPORT CNumEdit : public CEdit
{
	DECLARE_DYNAMIC(CNumEdit)
// Construction
public:
	CNumEdit();

// Attributes
public:
	enum {	VALID = 0x00,
			OUT_OF_RANGE = 0x01,
			INVALID_CHAR = 0x02,
			MINUS_PLUS_POINT_ZERO = 0x03,
			EMPTY = 0x04};
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNumEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual void ChangeAmount(int step);
	virtual double GetDelta();
	virtual void SetDelta(double delta);
	virtual void GetRange(double &min, double &max)const; 
	virtual void SetRange(double min, double max);
	virtual void SetMinNumberOfNumberAfterPoint(int nMinNumberOfNumberAfterPoint);
	virtual void SetMaxNumberOfNumberAfterPoint(int nMaxNumberOfNumberAfterPoint);
	virtual void Verbose(BOOL v);
	virtual BOOL Verbose()const; 
	virtual int IsValid(BOOL bVerbose = TRUE)const; 
	virtual int IsValid(const CString &str, BOOL bVerbose = TRUE)const; 
	virtual void RestoreOldValue();
	virtual BOOL SetValue(double val);
	virtual double GetValue()const; 
	virtual ~CNumEdit();

	// Generated message map functions
protected:
	virtual CString& ConstructFormat(CString &str, double num);
	int m_nNumberOfNumberAfterPoint;
	int m_nMaxNumberOfNumberAfterPoint;
	BOOL m_bVerbose;
	double m_dDelta, m_dMinValue, m_dMaxValue, m_dOldValue;
	CString m_sOldValue;
	BOOL m_bOldValueValid;
	//{{AFX_MSG(CNumEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NUMEDIT_H__7482F7FF_A479_11D2_A6A7_00600844997A__INCLUDED_)
