#if !defined(AFX_ENUMPRINTERS_H__496315A4_5B64_11D6_AD8C_00B0D0652E95__INCLUDED_)
#define AFX_ENUMPRINTERS_H__496315A4_5B64_11D6_AD8C_00B0D0652E95__INCLUDED_

#pragma once

class CEnumPrinters  
{
public:
						CEnumPrinters();
	virtual				~CEnumPrinters();

	// Returning gathered printer information
	__forceinline int	GetPrinterCount() const {return m_NumPrinters;};
	CString				GetPrinterName(int index) const;
	CString				GetPrinterLocation(int index) const;
	CString				GetPrinterShareName(int index)const;
	CString				GetPrinterPortName(int index) const;
	static CString		GetDefaultPrinterName();
	int					GetDefaultPrinterIndex() const;
	int					GetPrinterIndex(HANDLE &hDevMode, HANDLE& hDevNames) const;

	// Enumerating printers
	void				ReadLocalPrinters();
	void				ReadRemotePrinters();

	// Actually changing or configuring the printer
	bool				SetDefault(	HANDLE& hDevMode,
									HANDLE& hDevNames);

	// Note: As Parameter for bDefault pass the following:
	// sPrinterName == GetDefaultPrinterName()
	bool				SetNewPrinter(	HANDLE& hDevMode,
										HANDLE& hDevNames,
										CString sPrinterName,
										const CString& sPrinterSpooler,
										const CString& sPrinterPort,
										bool bDefault,
										int nOrientation = 0,
										int nPaperSize = 0,
										const CString& sFormName = _T(""));

	// Saving settings too/from the registry
	bool				SavePrinterSelection(HANDLE &hDevMode, HANDLE& hDevNames);
	bool				RestorePrinterSelection(HANDLE &hDevMode, HANDLE& hDevNames);

private:
	int					m_NumPrinters;
	CStringList			m_PrinterName;
	CStringList			m_PrinterLocation;
	CStringList			m_PrinterShareName;
	CStringList			m_PrinterPort;
};

#endif // !defined(AFX_ENUMPRINTERS_H__496315A4_5B64_11D6_AD8C_00B0D0652E95__INCLUDED_)
