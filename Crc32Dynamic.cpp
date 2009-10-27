#include "stdafx.h"
#include "Crc32Dynamic.h"

#ifdef CRACKCHECK

#define  CRC32_MAX_BUFFER_SIZE		4096

//***********************************************
CCrc32Dynamic::CCrc32Dynamic() : m_pdwCrc32Table(NULL)
{
	Init();
}

//***********************************************
CCrc32Dynamic::~CCrc32Dynamic()
{
	Free();
}

//***********************************************
void CCrc32Dynamic::Init()
{
	// This is the official polynomial used by CRC32 in PKZip.
	// Often times the polynomial shown reversed as 0x04C11DB7.
	DWORD dwPolynomial = 0xEDB88320;
	int i, j;

	Free();
	m_pdwCrc32Table = new DWORD[256];

	DWORD dwCrc;
	for (i = 0; i < 256; i++)
	{
		dwCrc = i;
		for (j = 8; j > 0; j--)
		{
			if(dwCrc & 1)
				dwCrc = (dwCrc >> 1) ^ dwPolynomial;
			else
				dwCrc >>= 1;
		}
		m_pdwCrc32Table[i] = dwCrc;
	}
}

//***********************************************
void CCrc32Dynamic::Free()
{
	if (m_pdwCrc32Table)
	{
		delete [] m_pdwCrc32Table;
		m_pdwCrc32Table = NULL;
	}
}

__forceinline void CCrc32Dynamic::CalcCrc32(const BYTE byte, DWORD &dwCrc32) const
{
	dwCrc32 = ((dwCrc32) >> 8) ^ m_pdwCrc32Table[(byte) ^ ((dwCrc32) & 0x000000FF)];
}

BOOL CCrc32Dynamic::Crc32C(LPBYTE pData, int nSize, DWORD& dwCrc32)
{
	dwCrc32 = 0xFFFFFFFF;

	// Is the table initialized?
	if (!m_pdwCrc32Table)
		return FALSE;

	for (int nLoop = 0; nLoop < nSize ; nLoop++)
		CalcCrc32(pData[nLoop], dwCrc32);

	dwCrc32 = ~dwCrc32;

	return TRUE;
}

BOOL CCrc32Dynamic::CalcC(LPBYTE pData, int nSize, DWORD& dwCrc32)
{
	if (!pData)
		return FALSE;
	if (nSize <= 0)
		return FALSE;
	dwCrc32 = 0xFFFFFFFF;
	CCrc32Dynamic* pobCrc32Dynamic = new CCrc32Dynamic;
	if (!pobCrc32Dynamic)
		return FALSE;
	BOOL res = pobCrc32Dynamic->Crc32C(pData, nSize, dwCrc32);
	delete pobCrc32Dynamic;
	return res;
}

BOOL CCrc32Dynamic::CalcAssembly(LPBYTE pData, int nSize, DWORD& dwCrc32)
{
	if (!pData)
		return FALSE;
	if (nSize <= 0)
		return FALSE;
	dwCrc32 = 0xFFFFFFFF;
	CCrc32Dynamic* pobCrc32Dynamic = new CCrc32Dynamic;
	if (!pobCrc32Dynamic)
		return FALSE;
	BOOL res = pobCrc32Dynamic->Crc32Assembly(pData, nSize, &dwCrc32);
	delete pobCrc32Dynamic;
	return res;
}

BOOL CCrc32Dynamic::Crc32Assembly(LPBYTE pData, int nSize, LPDWORD pdwCrc32)
{
	if (!pdwCrc32)
		return FALSE;
	*pdwCrc32 = 0xFFFFFFFF;

	try
	{
		// Is the table initialized?
		if (!m_pdwCrc32Table)
			throw 0;
		
		BYTE buffer[CRC32_MAX_BUFFER_SIZE];
		DWORD dwBytesToRead = CRC32_MAX_BUFFER_SIZE;
		if (dwBytesToRead > (DWORD)nSize)
			dwBytesToRead = (DWORD)nSize;
		DWORD dwTotalRead = dwBytesToRead;
		LPBYTE p = pData;
		memcpy(buffer, p, dwBytesToRead);
		while (TRUE)
		{
			// Register use:
			//		eax - CRC32 value
			//		ebx - a lot of things
			//		ecx - CRC32 value
			//		edx - address of end of buffer
			//		esi - address of start of buffer
			//		edi - CRC32 table
			__asm
			{
				// Save the esi and edi registers
				push esi
				push edi

				mov eax, pdwCrc32			// Load the pointer
				mov ecx, [eax]				// Dereference the pointer

				mov ebx, this				// Load the CRC32 table
				mov edi, [ebx]CCrc32Dynamic.m_pdwCrc32Table

				lea esi, buffer					// Load buffer
				mov ebx, dwBytesToRead		// Load dwSize
				lea edx, [esi + ebx]		// Calculate the end of the buffer

			crc32loop:
				xor eax, eax				// Clear the eax register
				mov bl, byte ptr [esi]		// Load the current source byte
				
				mov al, cl					// Copy crc value into eax
				inc esi						// Advance the source pointer

				xor al, bl					// Create the index into the CRC32 table
				shr ecx, 8

				mov ebx, [edi + eax * 4]	// Get the value out of the table
				xor ecx, ebx				// xor with the current byte

				cmp edx, esi				// Have we reached the end of the buffer?
				jne crc32loop

				// Restore the edi and esi registers
				pop edi
				pop esi

				mov eax, pdwCrc32			// Load the pointer
				mov [eax], ecx				// Write the result
			}

			// Done?
			if (dwTotalRead == (DWORD)nSize)
				break;
			
			// Inc. pointer
			p += dwBytesToRead;

			// Calc. next bytes to read
			dwTotalRead += CRC32_MAX_BUFFER_SIZE;
			if (dwTotalRead > (DWORD)nSize)
			{
				dwBytesToRead = CRC32_MAX_BUFFER_SIZE - (dwTotalRead - (DWORD)nSize);
				dwTotalRead = (DWORD)nSize;
			}
			memcpy(buffer, p, dwBytesToRead);
		}
	}
	catch(...)
	{
		return FALSE;
	}

	*pdwCrc32 = ~(*pdwCrc32);

	return TRUE;
}

#endif
