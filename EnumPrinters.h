// EnumPrinters.h: interface for the CEnumPrinters class.
//
// Written By : R.I.Allen
// 3rd May 2002
// Roger.Allen@sirius-analytical.com
// You can use this source as you like, but without any warranties of any kind!
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ENUMPRINTERS_H__496315A4_5B64_11D6_AD8C_00B0D0652E95__INCLUDED_)
#define AFX_ENUMPRINTERS_H__496315A4_5B64_11D6_AD8C_00B0D0652E95__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
	void				ReadLocalPrinters();	// For Win9x and Me this enumarates also Network Printers
	void				ReadRemotePrinters();	// For NT, 2000, XP, Vista

	// Actually changing or configuring the printer
	bool				SetDefault(	HANDLE& hDevMode,
									HANDLE& hDevNames);

	// Note: As Parameter for bDefault pass the following:
	// PrinterName == GetDefaultPrinterName()
	bool				SetNewPrinter(	HANDLE& hDevMode,
										HANDLE& hDevNames,
										const CString& PrinterName,
										const CString& PrinterSpooler,
										const CString& PrinterPort,
										bool bDefault);

	// Note: As Parameter for bDefault pass the following:
	// index == GetDefaultPrinterIndex()
	bool				SetNewPrinter(	HANDLE& hDevMode,
										HANDLE& hDevNames,
										int index,
										bool bDefault);

	// Printer Settings
	bool				SetPrintOrientation(HANDLE &hDevMode, int mode);
	bool				SetPrintPaperSize(HANDLE& hDevMode, int papersize);
	bool				SetPrintPaperSizeName(HANDLE &hDevMode, CString papersizename);

	// Saving settings too/from the registry
	bool				SavePrinterSelection(HANDLE &hDevMode, HANDLE& hDevNames);
	bool				RestorePrinterSelection(HANDLE &hDevMode, HANDLE& hDevNames);

	// Debug options only
#ifdef _DEBUG
	void				DumpHandles(HANDLE& hDevMode, HANDLE& hDevNames);
#endif

private:
	int					m_NumPrinters;
	CStringList			m_PrinterName;
	CStringList			m_PrinterLocation;
	CStringList			m_PrinterShareName;
	CStringList			m_PrinterPort;
};

#endif // !defined(AFX_ENUMPRINTERS_H__496315A4_5B64_11D6_AD8C_00B0D0652E95__INCLUDED_)
