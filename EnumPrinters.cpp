#include "stdafx.h"
#include "EnumPrinters.h"
#include <Winspool.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEnumPrinters::CEnumPrinters()
{
	// Initialise
	m_NumPrinters = 0;
	m_PrinterName.RemoveAll();
	m_PrinterLocation.RemoveAll();
	m_PrinterShareName.RemoveAll();
	ReadLocalPrinters();
	ReadRemotePrinters();
}

CEnumPrinters::~CEnumPrinters()
{

}

CString CEnumPrinters::GetPrinterName(int index) const
{
	ASSERT(index >= 0 && index < m_NumPrinters);
	POSITION pos;

	pos = m_PrinterName.GetHeadPosition();
	while (pos && index > 0)
	{
		// Traverse the list
		m_PrinterName.GetNext(pos);
		index--;
	}
	if (pos)
		return m_PrinterName.GetAt(pos);
	return CString(_T("Error"));
}

CString CEnumPrinters::GetPrinterLocation(int index) const
{
	ASSERT(index >= 0 && index < m_NumPrinters);
	POSITION pos;

	pos = m_PrinterLocation.GetHeadPosition();
	while (pos && index > 0)
	{
		// Traverse the list
		m_PrinterLocation.GetNext(pos);
		index--;
	}
	if (pos)
		return m_PrinterLocation.GetAt(pos);
	return CString(_T("Error"));
}

CString CEnumPrinters::GetPrinterShareName(int index) const
{
	ASSERT(index >= 0 && index < m_NumPrinters);
	POSITION pos;

	pos = m_PrinterShareName.GetHeadPosition();
	while (pos && index > 0)
	{
		// Traverse the list
		m_PrinterShareName.GetNext(pos);
		index--;
	}
	if (pos)
		return m_PrinterShareName.GetAt(pos);
	return CString(_T("Error"));
}

CString CEnumPrinters::GetPrinterPortName(int index) const
{
	ASSERT(index >= 0 && index < m_NumPrinters);
	POSITION pos;

	pos = m_PrinterPort.GetHeadPosition();
	while (pos && index > 0)
	{
		// Traverse the list
		m_PrinterPort.GetNext(pos);
		index--;
	}
	if (pos)
		return m_PrinterPort.GetAt(pos);
	return CString(_T("Error"));
}

CString CEnumPrinters::GetDefaultPrinterName()
{
	CString sPrinterName;

	HINSTANCE h = ::LoadLibrary(_T("winspool.drv"));
	typedef BOOL (WINAPI * FPGETDEFAULTPRINTERW)(LPWSTR pszBuffer, LPDWORD pcchBuffer);
	FPGETDEFAULTPRINTERW fpGetDefaultPrinter;
	if (h)
		fpGetDefaultPrinter = (FPGETDEFAULTPRINTERW)::GetProcAddress(h, "GetDefaultPrinterW");
	else
		fpGetDefaultPrinter = NULL;
	if (fpGetDefaultPrinter)
	{
		// I had a problem in win 2003 server with no printer installed:
		// DWORD size was not inited to zero before, so that
		// fpGetDefaultPrinter(NULL, &size) did not set size to zero
		// -> out of memory exception because size was very big in
		// release build.
		DWORD size = 0;
		fpGetDefaultPrinter(NULL, &size);
		if (size)
		{
			TCHAR* buffer = sPrinterName.GetBuffer(size);
			if (fpGetDefaultPrinter(buffer, &size))
			{
				sPrinterName.ReleaseBuffer();
				::FreeLibrary(h);
				return sPrinterName;
			}
			sPrinterName.ReleaseBuffer();
		}
		::FreeLibrary(h);
		return _T("");
	}
	else
	{
		TCHAR* buffer = sPrinterName.GetBuffer(1024);
		if (::GetProfileString(	_T("windows"),
								_T("device"),
								_T(""),
								buffer,
								1024) <= 0)
		{
			sPrinterName.ReleaseBuffer();
			if (h)
				::FreeLibrary(h);
			return _T("");
		}
		sPrinterName.ReleaseBuffer();
		int pos = sPrinterName.Find(_T(','));
		if (pos < 0)
		{
			if (h)
				::FreeLibrary(h);
			return _T("");
		}
		else
		{
			if (h)
				::FreeLibrary(h);
			return sPrinterName.Left(pos);
		}
	}
}

// This procedure returns the index of the default printer,
// or -1 if the printer could not be identified
int CEnumPrinters::GetDefaultPrinterIndex() const
{
	CString sDefaultPrinterName = GetDefaultPrinterName();
	for (int i = 0 ; i < GetPrinterCount() ; i++)
	{
		if (sDefaultPrinterName == GetPrinterName(i))
			return i; // this is the printer
	}
	return -1;
}

// This procedure returns the index of the currently selected printer,
// or -1 if the printer could not be identified
int CEnumPrinters::GetPrinterIndex(HANDLE &hDevMode, HANDLE& hDevNames) const
{
	// Extract the name and port of the selected printer
	if (hDevNames == NULL || hDevNames == INVALID_HANDLE_VALUE)
	{
		if (hDevMode != NULL && hDevMode != INVALID_HANDLE_VALUE)
		{
			// Just in case one handle is valid when the other is invalid
			// we need to release it
			ASSERT(::GlobalFlags(hDevMode) != GMEM_INVALID_HANDLE);
			UINT nCount = ::GlobalFlags(hDevMode) & GMEM_LOCKCOUNT;
			while (nCount--)
				::GlobalUnlock(hDevMode);

			// Finally, really free the handle
			::GlobalFree(hDevMode);
			hDevMode = INVALID_HANDLE_VALUE;
		}
		// No current printer is selected, get the system wide default
		// printer using the CPrintDialog
		CPrintDialog pd(TRUE);
		pd.GetDefaults();
		hDevMode = pd.m_pd.hDevMode;
		hDevNames = pd.m_pd.hDevNames;
	}

	// To identify the current printer we need the info from the hDevNames object
	LPDEVNAMES lpDevNames = (LPDEVNAMES)::GlobalLock(hDevNames);	// lock it
	
	// We only need the name of the printer to identify it
	CString name;
	if (lpDevNames)
		name = CString(((TCHAR*)((TCHAR*)lpDevNames + lpDevNames->wDeviceOffset))); 
	::GlobalUnlock(hDevNames); // unlock it

	for (int i = 0 ; i < GetPrinterCount() ; i++)
	{
		if (name == GetPrinterName(i))
			return i; // this is the printer
	}

	return -1; // failed to identify
}

void CEnumPrinters::ReadLocalPrinters()
{
	DWORD		Flags = PRINTER_ENUM_LOCAL; // Local printers
	DWORD		cbBuf = 0;
	DWORD		pcReturned = 0;
	DWORD		index;
	DWORD		Level = 2;
	LPPRINTER_INFO_2 pPrinterEnum = NULL;

	::EnumPrinters(Flags, NULL, Level, NULL, 0, &cbBuf, &pcReturned);
	pPrinterEnum = (LPPRINTER_INFO_2)::LocalAlloc(LPTR, cbBuf);

	if (!pPrinterEnum)
	{
		TRACE(_T("Error %1d\n"), ::GetLastError());
		goto clean_up;
	}

	if (!::EnumPrinters(Flags,							// DWORD Flags, printer object types 
						NULL,							// LPTSTR Name, name of printer object 
						Level,							// DWORD Level, information level 
						(LPBYTE)pPrinterEnum,			// LPBYTE pPrinterEnum, printer information buffer 
						cbBuf,							// DWORD cbBuf, size of printer information buffer
						&cbBuf,							// LPDWORD pcbNeeded, bytes received or required 
						&pcReturned)					// LPDWORD pcReturned number of printers enumerated 
						)
	{
		TRACE(_T("Error %1d\n"), ::GetLastError());
		goto clean_up;
	}

	if (pcReturned > 0)
	{
		for (index = 0 ; index < pcReturned ; index++)
		{
			m_PrinterName.AddTail((pPrinterEnum + index)->pPrinterName);
			m_PrinterShareName.AddTail((pPrinterEnum + index)->pShareName);
			m_PrinterLocation.AddTail((pPrinterEnum + index)->pLocation);
			m_PrinterPort.AddTail((pPrinterEnum + index)->pPortName);
			m_NumPrinters++;
		}
	}

clean_up:
	::LocalFree(LocalHandle(pPrinterEnum));
}

void CEnumPrinters::ReadRemotePrinters()
{
	DWORD		Flags = PRINTER_ENUM_CONNECTIONS; // Remote Printers that have been connected in the past
	DWORD		cbBuf = 0;
	DWORD		pcReturned = 0;
	DWORD		index;
	DWORD		Level = 4;
	LPPRINTER_INFO_4 pPrinterEnum = NULL;

	::EnumPrinters(Flags, NULL, Level, NULL, 0, &cbBuf, &pcReturned);
	pPrinterEnum = (LPPRINTER_INFO_4)::LocalAlloc(LPTR, cbBuf);

	if (!pPrinterEnum)
	{
		TRACE(_T("Error %1d\n"), ::GetLastError());
		goto clean_up;
	}

	if (!::EnumPrinters(Flags,							// DWORD Flags, printer object types 
						NULL,							// LPTSTR Name, name of printer object 
						Level,							// DWORD Level, information level 
						(LPBYTE)pPrinterEnum,			// LPBYTE pPrinterEnum, printer information buffer 
						cbBuf,							// DWORD cbBuf, size of printer information buffer
						&cbBuf,							// LPDWORD pcbNeeded, bytes received or required 
						&pcReturned)					// LPDWORD pcReturned number of printers enumerated 
						)
	{
		TRACE(_T("Error %1d\n"), ::GetLastError());
		goto clean_up;
	}

	if (pcReturned > 0)
	{
		for (index = 0 ; index < pcReturned ; index++)
		{
			m_PrinterName.AddTail((pPrinterEnum + index)->pPrinterName);
			m_PrinterShareName.AddTail(_T(""));
			m_PrinterLocation.AddTail(_T(""));
			m_PrinterPort.AddTail(_T(""));
			m_NumPrinters++;
		}
	}

clean_up:
	::LocalFree(LocalHandle(pPrinterEnum));
}

bool CEnumPrinters::SetDefault(	HANDLE& hDevMode,
								HANDLE& hDevNames)
{
	int index = GetDefaultPrinterIndex();
	if (index >= 0)
		return SetNewPrinter(hDevMode, hDevNames, index, true);
	else
		return false;
}

bool CEnumPrinters::SetNewPrinter(	HANDLE& hDevMode,
									HANDLE& hDevNames,
									const CString& PrinterName,
									const CString& PrinterSpooler,
									const CString& PrinterPort,
									bool bDefault)
// hDeMode - Handle to the current DEVMODE structure
// hDevNames - Handle to the current DEVNAMES structure
// PrinterName - E.g. HP LaserJet 4L
// PrinterSpooler - e.g. "winspool"
// PrinterPort - e.g. "LPT1:"
{
	// We only update the existing hDevMode and hDevNames objects
	// if we can successfgully setup the
	// new hDevMode and hDevNames objects
	HANDLE local_hDevMode = INVALID_HANDLE_VALUE;
	HANDLE local_hDevNames = INVALID_HANDLE_VALUE;

	// To setup the new local_hDevMode object we need
	// to open the printer name to get the information
	HANDLE hPrinter;
	TCHAR* szPrinter = new TCHAR[PrinterName.GetLength() + 1];
	ASSERT(szPrinter);
	_tcscpy(szPrinter, PrinterName);
	if (!::OpenPrinter(szPrinter, &hPrinter, NULL))
	{
		delete [] szPrinter;
		return false;
	}

	// A zero for last param returns the size of buffer needed for the information to be returned
	int nSize = ::DocumentProperties(NULL, hPrinter, szPrinter, NULL, NULL, 0);
	ASSERT(nSize >= 0);
	local_hDevMode = ::GlobalAlloc(GHND, nSize); // Allocate on heap
	LPDEVMODE lpDevMode = (LPDEVMODE)::GlobalLock(local_hDevMode); // Lock it

	// Fill in the rest of the structure.
	if (::DocumentProperties(NULL, hPrinter, szPrinter, lpDevMode, NULL, DM_OUT_BUFFER) != IDOK)
	{
		// Failed to read printer properties, abort
		ASSERT(::GlobalFlags(local_hDevMode) != GMEM_INVALID_HANDLE);
		UINT nCount = ::GlobalFlags(local_hDevMode) & GMEM_LOCKCOUNT;
		while (nCount--)
			::GlobalUnlock(local_hDevMode);

		// Finally, really free the handle
		::GlobalFree(local_hDevMode);
		local_hDevMode = NULL;
		::ClosePrinter(hPrinter);
		delete [] szPrinter;
		return false;
	}
	// Finished interrogating for DEVMODE structure
	::GlobalUnlock(local_hDevMode);
	::ClosePrinter(hPrinter);
	delete [] szPrinter;

	// We need to allocate a new DEVNAMES object on the global heap
	// we also need the size to include the strings PrinterName, PrinterSpooler and PrinterPort
	// Layout is:
	// DEVNAMES structure
	// PrinterSpooler\0
	// PrinterName\0
	// PrinterPort\0
	int	size =	sizeof(DEVNAMES) +
				sizeof(TCHAR) * (PrinterName.GetLength() + 1 +
								PrinterSpooler.GetLength() + 1 +
								PrinterPort.GetLength() + 1);
	local_hDevNames = ::GlobalAlloc(GHND, size); // allocate on heap
	LPDEVNAMES pNewDevNames = (LPDEVNAMES)::GlobalLock(local_hDevNames);	// lock it
	if (pNewDevNames)
	{
		memset(pNewDevNames, 0, size);											// init to 0
		
		// Add the 3 strings to the end of the structure
		// Offsets are all in TCHAR units!
		pNewDevNames->wDriverOffset = sizeof(DEVNAMES) / sizeof(TCHAR);
		_tcscpy((TCHAR*)pNewDevNames + pNewDevNames->wDriverOffset, PrinterSpooler);
		pNewDevNames->wDeviceOffset = pNewDevNames->wDriverOffset + PrinterSpooler.GetLength() + 1;
		_tcscpy((TCHAR*)pNewDevNames + pNewDevNames->wDeviceOffset, PrinterName);
		pNewDevNames->wOutputOffset = pNewDevNames->wDeviceOffset + PrinterName.GetLength() + 1;
		_tcscpy((TCHAR*)pNewDevNames + pNewDevNames->wOutputOffset, PrinterPort);

		// Default Flag
		if (bDefault)
			pNewDevNames->wDefault = DN_DEFAULTPRN;
	}

	::GlobalUnlock(local_hDevNames); // free it

	// Now update the handles that were passed in
	// free the existing handles if they exist first
	if (hDevMode != NULL && hDevMode != INVALID_HANDLE_VALUE)
	{
		ASSERT(::GlobalFlags(hDevMode) != GMEM_INVALID_HANDLE);
		UINT nCount = ::GlobalFlags(hDevMode) & GMEM_LOCKCOUNT;
		while (nCount--)
			::GlobalUnlock(hDevMode);

		// Finally, really free the handle
		::GlobalFree(hDevMode);
		hDevMode = INVALID_HANDLE_VALUE;
	}
	if (hDevNames != NULL && hDevNames != INVALID_HANDLE_VALUE)
	{
		ASSERT(::GlobalFlags(hDevNames) != GMEM_INVALID_HANDLE);
		UINT nCount = ::GlobalFlags(hDevNames) & GMEM_LOCKCOUNT;
		while (nCount--)
			::GlobalUnlock(hDevNames);

		// Finally, really free the handle
		::GlobalFree(hDevNames);
		hDevNames = INVALID_HANDLE_VALUE;
	}
	hDevMode = local_hDevMode;
	hDevNames = local_hDevNames;

	return true;
}

bool CEnumPrinters::SetNewPrinter(HANDLE& hDevMode,
								  HANDLE& hDevNames,
								  int index,
								  bool bDefault)
{
	CString	printer = GetPrinterName(index);
	CString spooler(_T("winspool"));
	CString	port = GetPrinterPortName(index);

	return SetNewPrinter(	hDevMode,
							hDevNames,
							printer,
							spooler,
							port,
							bDefault);
}

bool CEnumPrinters::SavePrinterSelection(HANDLE &hDevMode, HANDLE& hDevNames)
{
	CWinApp* pApp = ::AfxGetApp();
	ASSERT(pApp);

	CString	printer;
	CString	spooler;
	CString	port;
	int	landscape = DMORIENT_PORTRAIT;
	int papersize = DMPAPER_LETTER;
	CString papersizename;

	// Get Printer Name, Spooler and Port
	if (hDevNames != INVALID_HANDLE_VALUE &&
		hDevNames != NULL)
	{
		LPDEVNAMES lpDevNames = (LPDEVNAMES)::GlobalLock(hDevNames);
		if (lpDevNames)
		{
			printer = (TCHAR*)((TCHAR*)lpDevNames + lpDevNames->wDeviceOffset);
			spooler = (TCHAR*)((TCHAR*)lpDevNames + lpDevNames->wDriverOffset);
			port = (TCHAR*)((TCHAR*)lpDevNames + lpDevNames->wOutputOffset);
		}
		::GlobalUnlock(hDevNames);
	}
	else
		return false; // not setup!

	// Get Printer Settings
	if (hDevMode != INVALID_HANDLE_VALUE &&
		hDevMode != NULL)
	{
		LPDEVMODE lpDevMode = (LPDEVMODE)::GlobalLock(hDevMode);
		if (lpDevMode)
		{
			landscape = lpDevMode->dmOrientation;
			papersize = lpDevMode->dmPaperSize;
			papersizename = lpDevMode->dmFormName;
		}
		::GlobalUnlock(hDevMode);
	}
	
	VERIFY(pApp->WriteProfileString(_T("PrinterConfig"), _T("PrinterName"), printer));
	VERIFY(pApp->WriteProfileString(_T("PrinterConfig"), _T("Spooler"), spooler));
	VERIFY(pApp->WriteProfileString(_T("PrinterConfig"), _T("Port"), port));
	VERIFY(pApp->WriteProfileInt(_T("PrinterConfig"), _T("Landscape"), landscape));
	VERIFY(pApp->WriteProfileInt(_T("PrinterConfig"), _T("PaperSize"), papersize));
	VERIFY(pApp->WriteProfileString(_T("PrinterConfig"), _T("PaperSizeName"), papersizename));

	return true;
}

bool CEnumPrinters::RestorePrinterSelection(HANDLE &hDevMode, HANDLE& hDevNames)
{
	CWinApp* pApp = ::AfxGetApp();
	ASSERT(pApp);

	CString	printer;
	CString	spooler;
	CString	port;
	int	landscape = DMORIENT_PORTRAIT;
	int papersize = DMPAPER_LETTER;
	CString papersizename;

	printer = pApp->GetProfileString(_T("PrinterConfig"), _T("PrinterName"), _T(""));
	spooler = pApp->GetProfileString(_T("PrinterConfig"), _T("Spooler"), _T(""));
	port = pApp->GetProfileString(_T("PrinterConfig"), _T("Port"), _T(""));
	landscape = pApp->GetProfileInt(_T("PrinterConfig"), _T("Landscape"), DMORIENT_PORTRAIT);
	papersize = pApp->GetProfileInt(_T("PrinterConfig"), _T("PaperSize"), DMPAPER_LETTER);
	papersizename = pApp->GetProfileString(_T("PrinterConfig"), _T("PaperSizeName"), _T(""));

	if (printer == _T("") || spooler == _T("") || port == _T(""))
	{
		SetDefault(hDevMode, hDevNames);
		return false; // not setup
	}

	// Make sure the selected printer is in the list available
	int i;
	for (i = 0 ; i < m_NumPrinters ; i++)
	{
		if (printer == GetPrinterName(i))
			break; // found!
	}
	if (i >= m_NumPrinters)
	{
		// The selected printer is no longer available
		TRACE(	_T("Warning : Unable to re-select your preferred printer\n")
				_T("as it is no longer available. The system default printer\n")
				_T("will be used.")) ;
		SetDefault(hDevMode, hDevNames);
		return false;
	}

	if (printer == GetDefaultPrinterName())
		VERIFY(SetNewPrinter(hDevMode, hDevNames, printer, spooler, port, true));
	else
		VERIFY(SetNewPrinter(hDevMode, hDevNames, printer, spooler, port, false));
	VERIFY(SetPrintOrientation(hDevMode, landscape));
	VERIFY(SetPrintPaperSize(hDevMode, papersize));
	VERIFY(SetPrintPaperSizeName(hDevMode, papersizename));

	return true;
}

bool CEnumPrinters::SetPrintOrientation(HANDLE &hDevMode, int mode)
{
	if (hDevMode == INVALID_HANDLE_VALUE ||
		hDevMode == NULL)
		return false;

	switch (mode)
	{
		case DMORIENT_PORTRAIT :
		{
			// Portrait mode
			LPDEVMODE pDevMode = (LPDEVMODE)::GlobalLock(hDevMode);
			// Set orientation to portrait
			if (pDevMode)
				pDevMode->dmOrientation = DMORIENT_PORTRAIT;
			::GlobalUnlock(hDevMode);
			break;
		}
		case DMORIENT_LANDSCAPE :
		{
			// Landscape mode
			LPDEVMODE pDevMode = (LPDEVMODE)::GlobalLock(hDevMode);
			// Set orientation to landscape
			if (pDevMode)
				pDevMode->dmOrientation = DMORIENT_LANDSCAPE;
			::GlobalUnlock(hDevMode);
			break;
		}
		default :	
			ASSERT(FALSE); // invalid parameter
			return false;
	}
	return true;
}

bool CEnumPrinters::SetPrintPaperSize(HANDLE &hDevMode, int papersize)
{
	if (hDevMode == INVALID_HANDLE_VALUE ||
		hDevMode == NULL)
		return false;

	LPDEVMODE pDevMode = (LPDEVMODE)::GlobalLock(hDevMode);
	if (pDevMode)
		pDevMode->dmPaperSize = papersize;
	::GlobalUnlock(hDevMode);
		
	return true;
}

bool CEnumPrinters::SetPrintPaperSizeName(HANDLE &hDevMode, CString papersizename)
{
	if (hDevMode == INVALID_HANDLE_VALUE ||
		hDevMode == NULL)
		return false;

	LPDEVMODE pDevMode = (LPDEVMODE)::GlobalLock(hDevMode);
	if (pDevMode)
	{
		_tcsncpy((TCHAR*)pDevMode->dmFormName, papersizename, CCHFORMNAME);
		pDevMode->dmFormName[CCHFORMNAME-1] = _T('\0');
	}
	::GlobalUnlock(hDevMode);	
		
	return true;
}

#ifdef _DEBUG

void CEnumPrinters::DumpHandles(HANDLE& hDevMode, HANDLE& hDevNames)
{
	// Dump the content of the handles to the debug output
	TRACE(_T("===================Dumping Print Object handles==============\n"));
	if (hDevMode != INVALID_HANDLE_VALUE && hDevMode != NULL)
	{
		LPDEVMODE lpDevMode = (LPDEVMODE)::GlobalLock(hDevMode); // lock it
		if (lpDevMode)
		{
			TRACE(_T("------hDevMode---------------------------\n"));
			TRACE(_T("Device name          : %s\n"), lpDevMode->dmDeviceName);
			TRACE(_T("dmSpecVersion        : %d\n"), lpDevMode->dmSpecVersion);
			TRACE(_T("dmDriverVersion      : %d\n"), lpDevMode->dmDriverVersion);
			TRACE(_T("dmSize               : %d\n"), lpDevMode->dmSize);
			TRACE(_T("dmDriverExtra        : %d\n"), lpDevMode->dmDriverExtra);
			TRACE(_T("dmFIelds             : %x\n"), lpDevMode->dmFields);
			TRACE(_T("dmScale              : %d\n"), lpDevMode->dmScale);
			TRACE(_T("dmCopies             : %d\n"), lpDevMode->dmCopies);
			TRACE(_T("dmDefaultSource      : %d\n"), lpDevMode->dmDefaultSource);
			TRACE(_T("dmPrintQuality       : %d\n"), lpDevMode->dmPrintQuality);
			TRACE(_T("dmColor              : %d\n"), lpDevMode->dmColor);
			TRACE(_T("dmDuplex             : %d\n"), lpDevMode->dmDuplex);
			TRACE(_T("dmYResolution        : %d\n"), lpDevMode->dmYResolution);
			TRACE(_T("dmTTOption           : %d\n"), lpDevMode->dmTTOption);
			TRACE(_T("dmCollate            : %d\n"), lpDevMode->dmCollate);
			TRACE(_T("dmFormName           : %s\n"), lpDevMode->dmFormName);
			TRACE(_T("dmLogPixels          : %d\n"), lpDevMode->dmLogPixels);
			TRACE(_T("dmBitsPerPel         : %d\n"), lpDevMode->dmBitsPerPel);
			TRACE(_T("dmPelsWidth          : %d\n"), lpDevMode->dmPelsWidth);
			TRACE(_T("dmPelsHeight         : %d\n"), lpDevMode->dmPelsHeight);
			TRACE(_T("dmNup/dmDisplayFlags : %d\n"), lpDevMode->dmDisplayFlags);
			TRACE(_T("dmDisplayFrequency   : %d\n"), lpDevMode->dmDisplayFrequency);
		}
		::GlobalUnlock(hDevMode);
	}
	else
		TRACE(_T("hDevMode             : INVALID_HANDLE_VALUE\n"));

	if (hDevNames != INVALID_HANDLE_VALUE && hDevNames != NULL)
	{
		LPDEVNAMES lpDevNames = (LPDEVNAMES)::GlobalLock(hDevNames);
		if (lpDevNames)
		{
			TRACE(_T("------hDevNames--------------------------\n"));
			TRACE(_T("wDriverOffset   : %d\n"), lpDevNames->wDriverOffset);
			TRACE(_T("wDeviceOffset   : %d\n"), lpDevNames->wDeviceOffset);
			TRACE(_T("wOutputOffset   : %d\n"), lpDevNames->wOutputOffset);
			TRACE(_T("wDefault        : %x\n"), lpDevNames->wDefault);
			TRACE(_T("DriverName      : %s\n"), (TCHAR*)((TCHAR*)lpDevNames + lpDevNames->wDriverOffset));
			TRACE(_T("DeviceName      : %s\n"), (TCHAR*)((TCHAR*)lpDevNames + lpDevNames->wDeviceOffset));
			TRACE(_T("OutputName      : %s\n"), (TCHAR*)((TCHAR*)lpDevNames + lpDevNames->wOutputOffset));
		}
		::GlobalUnlock(hDevNames);
	}
	else
		TRACE(_T("hDevNames            : INVALID_HANDLE_VALUE\n"));
	TRACE(_T("===================Dump Complete=============================\n"));
}
#endif
