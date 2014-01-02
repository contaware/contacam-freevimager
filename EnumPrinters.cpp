#include "stdafx.h"
#include "EnumPrinters.h"
#include <Winspool.h>
#include <..\src\mfc\afximpl.h>

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
	// I had a problem in win 2003 server with no printer installed:
	// DWORD size was not inited to zero before, so that
	// GetDefaultPrinter(NULL, &size) did not set size to zero
	// -> out of memory exception because size was very big in
	// release build.
	DWORD size = 0;
	::GetDefaultPrinter(NULL, &size);
	if (size)
	{
		CString sPrinterName;
		TCHAR* buffer = sPrinterName.GetBuffer(size);
		if (::GetDefaultPrinter(buffer, &size))
		{
			sPrinterName.ReleaseBuffer();
			return sPrinterName;
		}
		sPrinterName.ReleaseBuffer();
	}
	return _T("");
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
			::AfxGlobalFree(hDevMode);
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
	LPDEVNAMES lpDevNames = (LPDEVNAMES)::GlobalLock(hDevNames);
	
	// We only need the name of the printer to identify it
	CString name;
	if (lpDevNames)
		name = CString(((TCHAR*)((TCHAR*)lpDevNames + lpDevNames->wDeviceOffset))); 
	::GlobalUnlock(hDevNames);

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
	{
		CString	printer = GetPrinterName(index);
		CString spooler(_T("winspool"));
		CString	port = GetPrinterPortName(index);
		return SetNewPrinter(	hDevMode,
								hDevNames,
								printer,
								spooler,
								port,
								true);
	}
	else
		return false;
}

// From KB167345:
// "Using a DEVMODE structure to modify printer settings is more difficult than
// just changing the fields of the structure. Specifically, a valid DEVMODE
// structure for a device contains private data that can only be modified by
// the DocumentProperties() function."
//
// hDeMode - Handle to the current DEVMODE structure
// hDevNames - Handle to the current DEVNAMES structure
// sPrinterName - e.g. "HP LaserJet 4L"
// sPrinterSpooler - e.g. "winspool"
// sPrinterPort - e.g. "LPT1:"
// bDefault - is this printer the default one?
// nOrientation - 0 do not set, otherwise DMORIENT_PORTRAIT or DMORIENT_LANDSCAPE
// nPaperSize - 0 do not set, otherwise DMPAPER_LETTER ...
// sFormName - _T("") do not set, otherwise the Paper Name
bool CEnumPrinters::SetNewPrinter(	HANDLE& hDevMode,
									HANDLE& hDevNames,
									CString sPrinterName,
									const CString& sPrinterSpooler,
									const CString& sPrinterPort,
									bool bDefault,
									int nOrientation/*=0*/,
									int nPaperSize/*=0*/,
									const CString& sFormName/*=_T("")*/)
{
	// We only update the existing hDevMode and hDevNames objects
	// if we can successfully setup the new hDevMode and hDevNames objects
	HANDLE local_hDevMode = INVALID_HANDLE_VALUE;
	HANDLE local_hDevNames = INVALID_HANDLE_VALUE;

	// To setup the new local_hDevMode object we need to open the printer to get the information
	HANDLE hPrinter;
	if (!::OpenPrinter(sPrinterName.GetBuffer(), &hPrinter, NULL))
		return false;

	// A zero for last param returns the size of buffer needed for the information to be returned
	int nSize = ::DocumentProperties(NULL, hPrinter, sPrinterName.GetBuffer(), NULL, NULL, 0);
	local_hDevMode = ::GlobalAlloc(GHND, nSize);
	LPDEVMODE lpDevMode = (LPDEVMODE)::GlobalLock(local_hDevMode);

	// Fill in the rest of the structure
	memset(lpDevMode, 0, nSize);
	if (::DocumentProperties(NULL, hPrinter, sPrinterName.GetBuffer(), lpDevMode, NULL, DM_OUT_BUFFER) != IDOK)
	{
		::AfxGlobalFree(local_hDevMode);
		local_hDevMode = NULL;
		::ClosePrinter(hPrinter);
		return false;
	}

	// Allocate a new DEVNAMES object on the global heap, we also need space
	// to include the strings sPrinterSpooler, sPrinterName and sPrinterPort
	// Layout is:
	// DEVNAMES structure
	// PrinterSpooler\0
	// PrinterName\0
	// PrinterPort\0
	nSize =	sizeof(DEVNAMES) +
			sizeof(TCHAR) * (sPrinterName.GetLength() + 1 +
							sPrinterSpooler.GetLength() + 1 +
							sPrinterPort.GetLength() + 1);
	local_hDevNames = ::GlobalAlloc(GHND, nSize);
	LPDEVNAMES pNewDevNames = (LPDEVNAMES)::GlobalLock(local_hDevNames);
	if (pNewDevNames)
	{
		// Init to 0
		memset(pNewDevNames, 0, nSize);
		
		// Add the 3 strings to the end of the structure, offsets are all in TCHAR units!
		pNewDevNames->wDriverOffset = sizeof(DEVNAMES) / sizeof(TCHAR);
		_tcscpy((TCHAR*)pNewDevNames + pNewDevNames->wDriverOffset, sPrinterSpooler);
		pNewDevNames->wDeviceOffset = pNewDevNames->wDriverOffset + sPrinterSpooler.GetLength() + 1;
		_tcscpy((TCHAR*)pNewDevNames + pNewDevNames->wDeviceOffset, sPrinterName);
		pNewDevNames->wOutputOffset = pNewDevNames->wDeviceOffset + sPrinterName.GetLength() + 1;
		_tcscpy((TCHAR*)pNewDevNames + pNewDevNames->wOutputOffset, sPrinterPort);

		// Default Flag
		if (bDefault)
			pNewDevNames->wDefault = DN_DEFAULTPRN;
	}

	// Merge the new settings with the old ones
	if ((lpDevMode->dmFields & DM_ORIENTATION) && nOrientation >= DMORIENT_PORTRAIT)
		lpDevMode->dmOrientation = nOrientation;
	if ((lpDevMode->dmFields & DM_PAPERSIZE) && nPaperSize >= DMPAPER_FIRST)
		lpDevMode->dmPaperSize = nPaperSize;
	if ((lpDevMode->dmFields & DM_FORMNAME) && sFormName != _T(""))
	{
		_tcsncpy((TCHAR*)lpDevMode->dmFormName, sFormName, CCHFORMNAME);
		lpDevMode->dmFormName[CCHFORMNAME-1] = _T('\0');
	}
    LONG lRet = ::DocumentProperties(NULL, hPrinter, sPrinterName.GetBuffer(),
									lpDevMode, lpDevMode,
									DM_IN_BUFFER | DM_OUT_BUFFER);

	// Free
	::GlobalUnlock(local_hDevNames);
	::GlobalUnlock(local_hDevMode);
	::ClosePrinter(hPrinter);

	// Now update the handles that were passed in
	if (hDevMode != NULL && hDevMode != INVALID_HANDLE_VALUE)
		::AfxGlobalFree(hDevMode);
	if (hDevNames != NULL && hDevNames != INVALID_HANDLE_VALUE)
		::AfxGlobalFree(hDevNames);
	hDevMode = local_hDevMode;
	hDevNames = local_hDevNames;

	return (lRet == IDOK);
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
	
	pApp->WriteProfileString(_T("PrinterConfig"), _T("PrinterName"), printer);
	pApp->WriteProfileString(_T("PrinterConfig"), _T("Spooler"), spooler);
	pApp->WriteProfileString(_T("PrinterConfig"), _T("Port"), port);
	pApp->WriteProfileInt(_T("PrinterConfig"), _T("Landscape"), landscape);
	pApp->WriteProfileInt(_T("PrinterConfig"), _T("PaperSize"), papersize);
	pApp->WriteProfileString(_T("PrinterConfig"), _T("PaperSizeName"), papersizename);

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

	return SetNewPrinter(	hDevMode, hDevNames,
							printer, spooler, port,
							printer == GetDefaultPrinterName(),
							landscape, papersize, papersizename);
}