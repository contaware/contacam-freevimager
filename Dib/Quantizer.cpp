#include "stdafx.h"
#include "Quantizer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
CQuantizer::CQuantizer(UINT nMaxColors, UINT nColorBits)
{
	m_nColorBits = nColorBits < 8 ? nColorBits : 8;

	m_pTree	= NULL;
	m_nLeafCount = 0;
	for	(int i = 0 ; i <= (int)m_nColorBits ; i++)
		m_pReducibleNodes[i] = NULL;
	m_nMaxColors = m_nOutputMaxColors = nMaxColors;
	if (m_nMaxColors < 16)
		m_nMaxColors = 16;
}
/////////////////////////////////////////////////////////////////////////////
CQuantizer::~CQuantizer()
{
	if (m_pTree	!= NULL)
		DeleteTree(&m_pTree);
}
/////////////////////////////////////////////////////////////////////////////
BOOL CQuantizer::ProcessImage(	CDib* pDib,
								CWnd* pProgressWnd/*=NULL*/,
								BOOL bProgressSend/*=TRUE*/)
{
	BYTE R,	G, B, A;
	unsigned int line, lineinc, i;

	if (!pDib)
		return FALSE;

	if (!pDib->IsValid())
		return FALSE;

	if (pDib->IsCompressed())
	{
		if (!pDib->Decompress(pDib->GetBitCount())) // Decompress
			return FALSE;
	}
	
	DWORD uiDIBSourceScanLineSize = DWALIGNEDWIDTHBYTES(pDib->GetWidth() * pDib->GetBitCount());
	LPBYTE lpBits = pDib->GetBits();

	// Calculate the line increment to skip lines if image is too big.
	// The reason for that is to avoid overflowing NODE::nRedSum, NODE::nGreenSum, 
	// NODE::nBlueSum and NODE::nAlphaSum by limiting the added pixels to 16000000
	// (16000000 * 256 < UINT_MAX)
	lineinc = 1;
	if (pDib->GetWidth() > 0)
	{
		unsigned int uiHeightLimit = 16000000 / pDib->GetWidth();
		if (uiHeightLimit > 0)
			lineinc = pDib->GetHeight() / uiHeightLimit + 1;
	}

	DIB_INIT_PROGRESS;

	switch (pDib->GetBitCount())
	{
		case 1:	// 1-bit DIB
		case 4:	// 4-bit DIB
		case 8:	// 8-bit DIB
			for (line = 0 ; line < pDib->GetHeight() ; line += lineinc)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, pDib->GetHeight());

				for (i = 0 ; i < pDib->GetWidth() ; i++)
				{
					BYTE idx = GetPixelIndex(i, line, pDib->GetBitCount(), uiDIBSourceScanLineSize, lpBits);
					BYTE* pal = (BYTE*)pDib->GetColors();
					long ldx = idx * sizeof(RGBQUAD);
					B = pal[ldx];
					G = pal[ldx+1];
					R = pal[ldx+2];
					A = pal[ldx+3];
					AddColor(&m_pTree, R, G, B, A, m_nColorBits, 0, &m_nLeafCount, m_pReducibleNodes);
					while (m_nLeafCount	> m_nMaxColors)
						ReduceTree(m_nColorBits, &m_nLeafCount, m_pReducibleNodes);
				}
			}
			break;

		case 16:
			for (line = 0 ; line < pDib->GetHeight() ; line += lineinc)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, pDib->GetHeight());

				for (i = 0 ; i < pDib->GetWidth() ; i++)
				{
					pDib->DIB16ToRGB(((WORD*)lpBits)[i], &R, &G, &B);
					AddColor(&m_pTree,	R, G, B, 0, m_nColorBits, 0, &m_nLeafCount, m_pReducibleNodes);
					while (m_nLeafCount	> m_nMaxColors)
						ReduceTree(m_nColorBits, &m_nLeafCount, m_pReducibleNodes);
				}
				lpBits += lineinc * uiDIBSourceScanLineSize;
			}
			break;
	
		case 24:
			for (line = 0 ; line < pDib->GetHeight() ; line += lineinc)
			{
				DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, pDib->GetHeight());

				for (i = 0 ; i < pDib->GetWidth() ; i++)
				{
					B =	lpBits[3*i];
					G = lpBits[3*i+1];
					R =	lpBits[3*i+2];
					AddColor(&m_pTree,	R, G, B, 0, m_nColorBits, 0, &m_nLeafCount, m_pReducibleNodes);
					while (m_nLeafCount	> m_nMaxColors)
						ReduceTree(m_nColorBits, &m_nLeafCount, m_pReducibleNodes);
				}
				lpBits += lineinc * uiDIBSourceScanLineSize;
			}
			break;

		case 32 :
			if (!pDib->HasAlpha() && pDib->IsFast32bpp())
			{
				for (line = 0 ; line < pDib->GetHeight() ; line += lineinc)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, pDib->GetHeight());

					for (i = 0 ; i < pDib->GetWidth() ; i++)
					{
						B = lpBits[4*i];
						G = lpBits[4*i+1];
						R = lpBits[4*i+2];
						AddColor(&m_pTree,	R, G, B, 0, m_nColorBits, 0, &m_nLeafCount, m_pReducibleNodes);
						while (m_nLeafCount	> m_nMaxColors)
							ReduceTree(m_nColorBits, &m_nLeafCount, m_pReducibleNodes);
					}
					lpBits += lineinc * uiDIBSourceScanLineSize;
				}
			}
			else if (pDib->HasAlpha())
			{
				for (line = 0 ; line < pDib->GetHeight() ; line += lineinc)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, pDib->GetHeight());

					for (i = 0 ; i < pDib->GetWidth() ; i++)
					{
						B = lpBits[4*i];
						G = lpBits[4*i+1];
						R = lpBits[4*i+2];
						A = lpBits[4*i+3];
						if (A > 0) // Only Add if not transparent!
						{
							AddColor(&m_pTree,	R, G, B, 0, m_nColorBits, 0, &m_nLeafCount, m_pReducibleNodes);
							while (m_nLeafCount	> m_nMaxColors)
								ReduceTree(m_nColorBits, &m_nLeafCount, m_pReducibleNodes);
						}
					}
					lpBits += lineinc * uiDIBSourceScanLineSize;
				}
			}
			else
			{
				for (line = 0 ; line < pDib->GetHeight() ; line += lineinc)
				{
					DIB_PROGRESS(pProgressWnd->GetSafeHwnd(), bProgressSend, line, pDib->GetHeight());

					for (i = 0 ; i < pDib->GetWidth() ; i++)
					{
						pDib->DIB32ToRGB(((DWORD*)lpBits)[i], &R, &G, &B);
						AddColor(&m_pTree,	R, G, B, 0, m_nColorBits, 0, &m_nLeafCount, m_pReducibleNodes);
						while (m_nLeafCount	> m_nMaxColors)
							ReduceTree(m_nColorBits, &m_nLeafCount, m_pReducibleNodes);
					}
					lpBits += lineinc * uiDIBSourceScanLineSize;
				}
			}
			break;

		default:
			DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());
			return FALSE;
	}

	DIB_END_PROGRESS(pProgressWnd->GetSafeHwnd());

	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
__forceinline void CQuantizer::AddColor(NODE** ppNode, BYTE r, BYTE g, BYTE b, BYTE a,
	UINT nColorBits, UINT nLevel, UINT*	pLeafCount,	NODE** pReducibleNodes)
{
	static BYTE	mask[8]	= {	0x80, 0x40,	0x20, 0x10,	0x08, 0x04,	0x02, 0x01 };

	// If the node doesn't exist, create it.
	if (*ppNode	== NULL)
		*ppNode	= (NODE*)CreateNode (nLevel, nColorBits, pLeafCount, pReducibleNodes);

	// Update color	information	if it's	a leaf node.
	if ((*ppNode)->bIsLeaf)
	{
		(*ppNode)->nPixelCount++;
		(*ppNode)->nRedSum += r;
		(*ppNode)->nGreenSum +=	g;
		(*ppNode)->nBlueSum	+= b;
		(*ppNode)->nAlphaSum += a;
	}
	// Recurse a level deeper if the node is not a leaf.
	else
	{
		int	shift =	7 -	nLevel;
		int	nIndex =(((r & mask[nLevel]) >> shift) << 2) |
					(((g & mask[nLevel]) >>	shift) << 1) |
					(( b & mask[nLevel]) >> shift);
		AddColor (&((*ppNode)->pChild[nIndex]),	r, g, b, a, nColorBits,
					nLevel + 1,	pLeafCount,	pReducibleNodes);
	}
}
/////////////////////////////////////////////////////////////////////////////
__forceinline void* CQuantizer::CreateNode(UINT nLevel, UINT	nColorBits,	UINT* pLeafCount,
	NODE** pReducibleNodes)
{
	NODE* pNode = (NODE*)calloc(1,sizeof(NODE));

	if (pNode == NULL)
		return NULL;

	pNode->bIsLeaf = (nLevel ==	nColorBits)	? TRUE : FALSE;
	if (pNode->bIsLeaf)
		(*pLeafCount)++;
	else
	{
		pNode->pNext = pReducibleNodes[nLevel];
		pReducibleNodes[nLevel]	= pNode;
	}
	return pNode;
}
/////////////////////////////////////////////////////////////////////////////
__forceinline void CQuantizer::ReduceTree(UINT nColorBits, UINT*	pLeafCount,
	NODE** pReducibleNodes)
{
	// Find	the	deepest	level containing at	least one reducible	node.
	int i;
	for	(i=nColorBits -	1; (i>0) &&	(pReducibleNodes[i]	== NULL); i--);

	// Reduce the node most	recently added to the list at level	i.
	NODE* pNode	= pReducibleNodes[i];
	pReducibleNodes[i] = pNode->pNext;

	UINT nRedSum = 0;
	UINT nGreenSum = 0;
	UINT nBlueSum =	0;
	UINT nAlphaSum = 0;
	UINT nChildren = 0;

	for	(i=0; i<8; i++)
	{
		if (pNode->pChild[i] !=	NULL)
		{
			nRedSum	+= pNode->pChild[i]->nRedSum;
			nGreenSum += pNode->pChild[i]->nGreenSum;
			nBlueSum +=	pNode->pChild[i]->nBlueSum;
			nAlphaSum += pNode->pChild[i]->nAlphaSum;
			pNode->nPixelCount += pNode->pChild[i]->nPixelCount;
			free(pNode->pChild[i]);
			pNode->pChild[i] = NULL;
			nChildren++;
		}
	}

	pNode->bIsLeaf = TRUE;
	pNode->nRedSum = nRedSum;
	pNode->nGreenSum = nGreenSum;
	pNode->nBlueSum	= nBlueSum;
	pNode->nAlphaSum	= nAlphaSum;
	*pLeafCount	-= (nChildren -	1);
}
/////////////////////////////////////////////////////////////////////////////
__forceinline void CQuantizer::DeleteTree(NODE**	ppNode)
{
	for	(int i=0; i<8; i++)
	{
		if ((*ppNode)->pChild[i] !=	NULL)
			DeleteTree (&((*ppNode)->pChild[i]));
	}
	free(*ppNode);
	*ppNode	= NULL;
}
/////////////////////////////////////////////////////////////////////////////
__forceinline void CQuantizer::GetPaletteColors(NODE* pTree, RGBQUAD* prgb, UINT* pIndex, UINT* pSum)
{
	if (pTree)
	{
		if (pTree->bIsLeaf)
		{
			prgb[*pIndex].rgbRed   = (BYTE)MIN(255, Round((double)pTree->nRedSum   / (double)pTree->nPixelCount));
			prgb[*pIndex].rgbGreen = (BYTE)MIN(255, Round((double)pTree->nGreenSum / (double)pTree->nPixelCount));
			prgb[*pIndex].rgbBlue  = (BYTE)MIN(255, Round((double)pTree->nBlueSum  / (double)pTree->nPixelCount));
			prgb[*pIndex].rgbReserved =	(BYTE)MIN(255, Round((double)pTree->nAlphaSum / (double)pTree->nPixelCount));
			if (pSum)
				pSum[*pIndex] = pTree->nPixelCount;
			(*pIndex)++;
		} 
		else
		{
			for	(int i=0; i<8; i++)
			{
				if (pTree->pChild[i] !=	NULL)
					GetPaletteColors (pTree->pChild[i],	prgb, pIndex, pSum);
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
void CQuantizer::SetColorTable(RGBQUAD* prgb)
{
	UINT nIndex	= 0;
	if (m_nOutputMaxColors<16)
	{
		UINT nSum[16];
		RGBQUAD tmppal[16];
		GetPaletteColors(m_pTree, tmppal, &nIndex, nSum);
		if (m_nLeafCount>m_nOutputMaxColors)
		{
			UINT j,k,nr,ng,nb,na,ns,a,b;
			for (j=0;j<m_nOutputMaxColors;j++)
			{
				a=(j*m_nLeafCount)/m_nOutputMaxColors;
				b=((j+1)*m_nLeafCount)/m_nOutputMaxColors;
				nr=ng=nb=na=ns=0;
				for (k=a;k<b;k++)
				{
					nr+=tmppal[k].rgbRed * nSum[k];
					ng+=tmppal[k].rgbGreen * nSum[k];
					nb+=tmppal[k].rgbBlue * nSum[k];
					na+=tmppal[k].rgbReserved * nSum[k];
					ns+= nSum[k];
				}
				prgb[j].rgbRed   = (BYTE)MIN(255, Round((double)nr / (double)ns));
				prgb[j].rgbGreen = (BYTE)MIN(255, Round((double)ng / (double)ns));
				prgb[j].rgbBlue  = (BYTE)MIN(255, Round((double)nb / (double)ns));
				prgb[j].rgbReserved = (BYTE)MIN(255, Round((double)na / (double)ns));
			}
		}
		else
			memcpy(prgb,tmppal,m_nLeafCount * sizeof(RGBQUAD));
	}
	else
		GetPaletteColors(m_pTree, prgb, &nIndex, 0);
}
/////////////////////////////////////////////////////////////////////////////
__forceinline BYTE CQuantizer::GetPixelIndex(long x, long y, int nbit, long effwdt, BYTE *pimage)
{
	if (nbit==8)
		return pimage[y*effwdt + x];
	else
	{
		BYTE pos;
		BYTE iDst= pimage[y*effwdt + (x*nbit >> 3)];
		if (nbit==4)
		{
			pos = (BYTE)(4*(1-x%2));
			iDst &= (0x0F<<pos);
			return (iDst >> pos);
		}
		else if (nbit==1)
		{
			pos = (BYTE)(7-x%8);
			iDst &= (0x01<<pos);
			return (iDst >> pos);
		}
	}
	return 0;
}
