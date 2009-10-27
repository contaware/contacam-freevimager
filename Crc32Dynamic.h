#ifdef CRACKCHECK
#ifndef _CRC32DYNAMIC_H_
#define _CRC32DYNAMIC_H_

class CCrc32Dynamic
{
public:
	CCrc32Dynamic();
	virtual ~CCrc32Dynamic();
	static BOOL CalcC(LPBYTE pData, int nSize, DWORD& dwCrc32);
	static BOOL CalcAssembly(LPBYTE pData, int nSize, DWORD& dwCrc32);
	
protected:
	void Init();
	void Free();
	BOOL Crc32C(LPBYTE pData, int nSize, DWORD& dwCrc32);
	BOOL Crc32Assembly(LPBYTE pData, int nSize, LPDWORD pdwCrc32);
	__forceinline void CalcCrc32(const BYTE byte, DWORD &dwCrc32) const;
	DWORD *m_pdwCrc32Table;
};

#endif
#endif
