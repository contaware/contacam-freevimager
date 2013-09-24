/* === C R E D I T S  &  D I S C L A I M E R S ==============
 * Permission is given by the author to freely redistribute and include
 * this code in any program as long as this credit is given where due.
 *
 * CQuantizer (c)  1996-1997 Jeff Prosise
 *
 * 31/08/2003 Davide Pizzolato - www.xdp.it
 * - fixed minor bug in ProcessImage when bpp<=8
 * - better color reduction to less than 16 colors
 *
 * COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
 * OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
 * THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
 * OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
 * CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
 * THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
 * SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
 * PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
 * THIS DISCLAIMER.
 *
 * Use at your own risk!
 * ==========================================================
 */

/*

There are several improvements I would like to point out:

1) bIsLeaf flag is unnecessary as any node with (PixelCount > 0) can be considered a leaf.
   That saves some memory.

2) Picking up most recently added nodes for reduction causes visible artefacts.
   Better results can be obtained if we find the node with minimum PixelCount at the deepest level.

3) rgbRed += pTree->nRedSum + (pTree->nPixelCount >> 1) / pTree->nPixelCount; gives proper rounding

4) If memory is not a concern, we can reduce tree after adding whole pixel
   row so that new pixels get a chance to survive.

Also, be careful when assigning colour from the palette to the original colour.
Traversing octree to find the first leaf is not enough! Other leafs can give better results in
terms of RGB or perceptual distance. So we should create a linked list of leafs during GetPaletteColors()
and then search through the whole list for the best match.

*/

#if !defined(AFX_QUANTIZER_H__C2E689E1_C05D_11D4_B4BC_00010223337E__INCLUDED_)
#define AFX_QUANTIZER_H__C2E689E1_C05D_11D4_B4BC_00010223337E__INCLUDED_

#pragma once

// Quantizer.h : header file
//

#include "Dib.h"

class CQuantizer
{
	struct NODE
	{
		BOOL bIsLeaf;       // TRUE if node has no children
		UINT nPixelCount;   // Number of pixels represented by this leaf
		UINT nRedSum;       // Sum of red components
		UINT nGreenSum;     // Sum of green components
		UINT nBlueSum;      // Sum of blue components
		UINT nAlphaSum;     // Sum of alpha components
		NODE* pChild[8];    // Pointers to child nodes
		NODE* pNext;        // Pointer to next reducible node
	};

protected:
    NODE* m_pTree;
    UINT m_nLeafCount;
    NODE* m_pReducibleNodes[9];
    UINT m_nMaxColors;
    UINT m_nOutputMaxColors;
    UINT m_nColorBits;

public:
    CQuantizer(UINT nMaxColors, UINT nColorBits);
    virtual ~CQuantizer();
    BOOL ProcessImage(	CDib* pDib,
						CWnd* pProgressWnd = NULL,
						BOOL bProgressSend = TRUE);
    __forceinline UINT GetColorCount() const {return m_nLeafCount;};
    void SetColorTable(RGBQUAD* prgb);

protected:
    __forceinline void AddColor(NODE** ppNode, BYTE r, BYTE g, BYTE b, BYTE a, UINT nColorBits,
        UINT nLevel, UINT* pLeafCount, NODE** pReducibleNodes);
    __forceinline void* CreateNode(UINT nLevel, UINT nColorBits, UINT* pLeafCount,
        NODE** pReducibleNodes);
    __forceinline void ReduceTree(UINT nColorBits, UINT* pLeafCount,
        NODE** pReducibleNodes);
    __forceinline void DeleteTree(NODE** ppNode);
    __forceinline void GetPaletteColors(NODE* pTree, RGBQUAD* prgb, UINT* pIndex, UINT* pSum);
	__forceinline BYTE GetPixelIndex(long x,long y, int nbit, long effwdt, BYTE *pimage);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUANTIZER_H__C2E689E1_C05D_11D4_B4BC_00010223337E__INCLUDED_)