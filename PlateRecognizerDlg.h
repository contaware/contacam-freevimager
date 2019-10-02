#if !defined(AFX_PLATERECOGNIZERDLG_H__AAD75600_3201_49D1_A5D4_3AE518917430__INCLUDED_)
#define AFX_PLATERECOGNIZERDLG_H__AAD75600_3201_49D1_A5D4_3AE518917430__INCLUDED_

#pragma once

// PlateRecognizerDlg.h : header file
//

#ifdef VIDEODEVICEDOC

class CPlateRecognizerDlg : public CDialog
{
	DECLARE_DYNAMIC(CPlateRecognizerDlg)

public:
	CPlateRecognizerDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CPlateRecognizerDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PLATERECOGNIZER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSyslinkPlateRecognizerHelp(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCheckPlateRecognizerCloud();
	afx_msg void OnCheckPlateRecognizerOnPremise();
	afx_msg void OnSyslinkPlateRecognizerRegionsHelp(NMHDR *pNMHDR, LRESULT *pResult);
	int m_nMode;
	CString m_sToken;
	CString m_sUrl;
	CString m_sRegions;
	virtual BOOL OnInitDialog();
};

#endif

#endif // !defined(AFX_PLATERECOGNIZERDLG_H__AAD75600_3201_49D1_A5D4_3AE518917430__INCLUDED_)
