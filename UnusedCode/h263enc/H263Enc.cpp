#include "stdafx.h"
#include "H263Enc.h"
#include "RgbToYuv.h"
#include "Dib.h"

// DCT & IDCT
const int CH263Enc::m_zigzag[8][8] =
{
	{0, 1, 5, 6,14,15,27,28},
	{2, 4, 7,13,16,26,29,42},
	{3, 8,12,17,25,30,41,43},
	{9,11,18,24,31,40,44,53},
	{10,19,23,32,39,45,52,54},
	{20,22,33,38,46,51,55,60},
	{21,34,37,47,50,56,59,61},
	{35,36,48,49,57,58,62,63}
};

const int CH263Enc::m_bit_set_mask[32] =
{
	0x00000001,0x00000002,0x00000004,0x00000008,
	0x00000010,0x00000020,0x00000040,0x00000080,
	0x00000100,0x00000200,0x00000400,0x00000800,
	0x00001000,0x00002000,0x00004000,0x00008000,
	0x00010000,0x00020000,0x00040000,0x00080000,
	0x00100000,0x00200000,0x00400000,0x00800000,
	0x01000000,0x02000000,0x04000000,0x08000000,
	0x10000000,0x20000000,0x40000000,0x80000000
};


/* Motion vectors */
const int CH263Enc::m_vlc_mv_coeff[] =
{
	32,13,5,
	33,13,7,
	34,12,5,
	35,12,7,
	36,12,9,
	37,12,11,
	38,12,13,
	39,12,15,
	40,11,9,
	41,11,11,
	42,11,13,
	43,11,15,
	44,11,17,
	45,11,19,
	46,11,21,
	47,11,23,
	48,11,25,
	49,11,27,
	50,11,29,
	51,11,31,
	52,11,33,
	53,11,35,
	54,10,19,
	55,10,21,
	56,10,23,
	57,8,7,
	58,8,9,
	59,8,11,
	60,7,7,
	61,5,3,
	62,4,3,
	63,3,3,
	 0,1,1,
	 1,3,2,
	 2,4,2,
	 3,5,2,
	 4,7,6,
	 5,8,10,
	 6,8,8,
	 7,8,6,
	 8,10,22,
	 9,10,20,
	10,10,18,
	11,11,34,
	12,11,32,
	13,11,30,
	14,11,28,
	15,11,26,
	16,11,24,
	17,11,22,
	18,11,20,
	19,11,18,
	20,11,16,
	21,11,14,
	22,11,12,
	23,11,10,
	24,11,8,
	25,12,14,
	26,12,12,
	27,12,10,
	28,12,8,
	29,12,6,
	30,12,4,
	31,13,6,
	-1,-1
};

/* CBPCM (MCBPC) */
const int CH263Enc::m_vlc_cbpcm_intra_coeff[] =
{
	3,1,1,
	19,3,1,
	35,3,2,
	51,3,3,
	4,4,1,
	20,6,1,
	36,6,2,
	52,6,3,
	255,9,1,
	-1,-1
};

const int CH263Enc::m_vlc_cbpcm_coeff[] =
{
	0,1,1,
	16,4,3,
	32,4,2,
	48,6,5,
	1,3,3,
	17,7,7,
	33,7,6,
	49,9,5,
	2,3,2,
	18,7,5,
	34,7,4,
	50,8,5,
	3,5,3,
	19,8,4,
	35,8,3,
	51,7,3,
	4,6,4,
	20,9,4,
	36,9,3,
	52,9,2,
	255,9,1,
	-1,-1
};


/* CBPY */
const int CH263Enc::m_vlc_cbpy_coeff[] =
{
	0,  2,3,
	8,  4,11,
	4,  4,10,
	12, 4,9,
	2,  4,8,
	10, 4,7,
	6,  6,3,
	14, 5,5,
	1,  4,6,
	9,  6,2,
	5,  4,5,
	13, 5,4,
	3,  4,4,
	11, 5,3,
	7,  5,2,
	15, 4,3,
	-1,-1
};

/* 3D VLC */
const int CH263Enc::m_vlc_3d_coeff[] =
{
	1,2,2,
	2,4,15,
	3,6,21,
	4,7,23,
	5,8,31,
	6,9,37,
	7,9,36,
	8,10,33,
	9,10,32,
	10,11,7,
	11,11,6,
	12,11,32,
	17,3,6,
	18,6,20,
	19,8,30,
	20,10,15,
	21,11,33,
	22,12,80,
	33,4,14,
	34,8,29,
	35,10,14,
	36,12,81,
	49,5,13,
	50,9,35,
	51,10,13,
	65,5,12,
	66,9,34,
	67,12,82,
	81,5,11,
	82,10,12,
	83,12,83,
	97,6,19,
	98,10,11,
	99,12,84,
	113,6,18,
	114,10,10,
	129,6,17,
	130,10,9,
	145,6,16,
	146,10,8,
	161,7,22,
	162,12,85,
	177,7,21,
	193,7,20,
	209,8,28,
	225,8,27,
	241,9,33,
	257,9,32,
	273,9,31,
	289,9,30,
	305,9,29,
	321,9,28,
	337,9,27,
	353,9,26,
	369,11,34,
	385,11,35,
	401,12,86,
	417,12,87,

	4097,4,7,                          /* Table for last coeff */
	4098,9,25,
	4099,11,5,
	4113,6,15,
	4114,11,4,
	4129,6,14,
	4145,6,13,
	4161,6,12,
	4177,7,19,
	4193,7,18,
	4209,7,17,
	4225,7,16,
	4241,8,26,
	4257,8,25,
	4273,8,24,
	4289,8,23,
	4305,8,22,
	4321,8,21,
	4337,8,20,
	4353,8,19,
	4369,9,24,
	4385,9,23,
	4401,9,22,
	4417,9,21,
	4433,9,20,
	4449,9,19,
	4465,9,18,
	4481,9,17,
	4497,10,7,
	4513,10,6,
	4529,10,5,
	4545,10,4,
	4561,11,36,
	4577,11,37,
	4593,11,38,
	4609,11,39,
	4625,12,88,
	4641,12,89,
	4657,12,90,
	4673,12,91,
	4689,12,92,
	4705,12,93,
	4721,12,94,
	4737,12,95,
	7167,7,3,               /* escape */
	-1,-1
};

const unsigned int CH263Enc::m_movement_coords[16][2] =
{	
	{0,0},{2,2},{0,3},{1,1},
	{3,0},{1,2},{3,3},{2,1},
	{0,2},{3,1},{2,3},{1,0},
	{3,2},{0,1},{2,0},{1,3}
};


void CH263Enc::WriteByteFunction(int byte)
{
	if (m_cDataCount < COMPRESSION_BUFFER_SIZE)
		m_cData[m_cDataCount] = (unsigned char)byte;
	m_cDataCount++;
}

CH263Enc::CH263Enc()
{
#ifdef FASTIDCT
		/* Nothing To Init */
#elif VERYFASTIDCT
		init_idct();
#else
		init_idct();
#endif

	memset(&m_cParams, 0, sizeof(CParam));
	m_pYUV = NULL;
	m_nYUVBufSize = 0;
	memset(&m_Bits, 0, sizeof(Bits));
	ResetEncodedDataBuf();

	m_cDataCount = 0;
	m_find_motion_call_time = 0;
	m_advanced = 0;
	m_cpels = 0;
	m_headerlength = 0;
	m_lines = 0;
	m_long_vectors = 0;
	m_mbc = 0;
	m_mbr = 0;
	m_mv_outside_frame = 0;
	m_pb_frames = 0;
	m_pels = 0;
	m_search_p_frames = 0;
	m_sizeof_frame = 0;
	m_target_framerate = 0;
	m_tf = 0;
	m_trace = 0;
	m_uskip = 0;
	m_vskip = 0;
	m_pic = NULL;
}

CH263Enc::CH263Enc(int nCIFFormat)
{
#ifdef FASTIDCT
		/* Nothing To Init */
#elif VERYFASTIDCT
		init_idct();
#else
		init_idct();
#endif

	memset(&m_cParams, 0, sizeof(CParam));
	m_pYUV = NULL;
	m_nYUVBufSize = 0;
	memset(&m_Bits, 0, sizeof(Bits));
	ResetEncodedDataBuf();

	m_cDataCount = 0;
	m_find_motion_call_time = 0;
	m_advanced = 0;
	m_cpels = 0;
	m_headerlength = 0;
	m_lines = 0;
	m_long_vectors = 0;
	m_mbc = 0;
	m_mbr = 0;
	m_mv_outside_frame = 0;
	m_pb_frames = 0;
	m_pels = 0;
	m_search_p_frames = 0;
	m_sizeof_frame = 0;
	m_target_framerate = 0;
	m_tf = 0;
	m_trace = 0;
	m_uskip = 0;
	m_vskip = 0;
	m_pic = NULL;

	// Set CIF Format
	SetCIFFormat(nCIFFormat);
}

CH263Enc::~CH263Enc()
{
	// Finally terminate the encoder 
	Free(&m_cParams);

	// Clean-Up
	if (m_pYUV)
	{
		delete [] m_pYUV;
		m_pYUV = NULL;
	}
}

unsigned char* CH263Enc::GetEncodedDataBuf()
{
	return m_cData;
}

int CH263Enc::GetEncodedDataSize()
{
	return m_cDataCount;
}

void CH263Enc::ResetEncodedDataBuf()
{
	m_cDataCount = 0;
	memset(m_cData, 0, m_cDataCount);
}

/*
CPARAM_SQCIF:
      horizontal_size = 128;
      vertical_size = 96;
CPARAM_QCIF:
      horizontal_size = 176;
      vertical_size = 144;
CPARAM_CIF:
      horizontal_size = 352;
      vertical_size = 288;
      break;
CPARAM_4CIF:
      horizontal_size = 704;
      vertical_size = 576;
      break;
CPARAM_16CIF:
      horizontal_size = 1408;
      vertical_size = 1152;
*/
bool CH263Enc::SetCIFFormat(int nCIFFormat)
{
	switch (nCIFFormat)
	{
		case CPARAM_SQCIF :
			m_uiWidth = 128;
			m_uiHeight = 96;
			break;

		case CPARAM_QCIF :
			m_uiWidth = 176;
			m_uiHeight = 144;
			break;

		case CPARAM_CIF :
			m_uiWidth = 352;
			m_uiHeight = 288;
			break;

		case CPARAM_4CIF :
			m_uiWidth = 704;
			m_uiHeight = 576;
			break;

		case CPARAM_16CIF :
			m_uiWidth = 1408;
			m_uiHeight = 1152;
			break;

		default :
			m_uiWidth = 0;
			m_uiHeight = 0;
			if (m_pYUV)
			{
				delete [] m_pYUV;
				m_pYUV = NULL;
			}
			return false;
	}

	// Initialize the compressor 
	m_cParams.format = nCIFFormat;
	Init(&m_cParams);

	// Allocate YUV Buffer
	if (m_pYUV)
		delete [] m_pYUV;
	m_nYUVBufSize = m_uiWidth * m_uiHeight + (m_uiWidth * m_uiHeight) / 2;
	m_pYUV = new unsigned int[m_nYUVBufSize];

	// Set Compression Param
	m_cParams.half_pixel_searchwindow = CPARAM_DEFAULT_SEARCHWINDOW; 
	m_cParams.inter = CPARAM_INTRA;					//CPARAM_INTER 
	m_cParams.Q_intra = CPARAM_DEFAULT_INTRA_Q; 
	m_cParams.Q_inter = CPARAM_DEFAULT_INTER_Q;
	m_cParams.data = m_pYUV;
	m_cParams.search_method = CPARAM_LOGARITHMIC;	// CPARAM_EXHAUSTIVE
	m_cParams.advanced_method = CPARAM_NOADVANCED;	// CPARAM_ADVANCED :Use array to determine 
													//					macroblocks in INTER frame
													//					mode to be encoded

	return true;
}

/*
CPARAM_SQCIF:
      horizontal_size = 128;
      vertical_size = 96;
CPARAM_QCIF:
      horizontal_size = 176;
      vertical_size = 144;
CPARAM_CIF:
      horizontal_size = 352;
      vertical_size = 288;
      break;
CPARAM_4CIF:
      horizontal_size = 704;
      vertical_size = 576;
      break;
CPARAM_16CIF:
      horizontal_size = 1408;
      vertical_size = 1152;
*/
int CH263Enc::GetCIFFormatFromSize(unsigned int uiWidth, unsigned int uiHeight)
{
	if (uiWidth < 176 && uiHeight < 144)
		return CPARAM_SQCIF;
	else if (uiWidth < 352 && uiHeight < 288)
		return CPARAM_QCIF;
	else if (uiWidth < 704 && uiHeight < 576)
		return CPARAM_CIF;
	else
		return CPARAM_4CIF;
}

bool CH263Enc::EncodeH263(CDib* pOrigDib)
{
	if (!pOrigDib)
		return false;

	if (m_uiWidth == 0 ||
		m_uiHeight == 0 ||
		m_pYUV == NULL)
		return false;

	CDib DibCopy;
	CDib* pDib = pOrigDib;
	if (!IsI420(pOrigDib->GetBMIH()->biCompression))
	{
		// Make a Copy?
		if (pDib->IsCompressed() ||
			pDib->GetBitCount() != 24 ||
			pDib->GetWidth() != m_uiWidth ||
			pDib->GetHeight() != m_uiHeight)
		{
			DibCopy = *pOrigDib;
			pDib = &DibCopy;
		}

		// Decompress to 24 bpp
		if (pDib->IsCompressed())
		{
			if (!pDib->Decompress(24))
				return false;
		}
		// Convert to 24 bpp
		else if (pDib->GetBitCount() != 24)
		{
			if (!pDib->ConvertTo24bits())
				return false;
		}

		// Resize
		if (!pDib->NearestNeighborResizeBits(m_uiWidth, m_uiHeight))
			return false;

		// Convert To YUV
		if (m_uiWidth > 0 &&
			m_uiHeight > 0 &&
			pDib->GetBits() &&
			m_pYUV)
			::RGB24ToYUV420(pDib->GetBits(), m_pYUV, m_uiWidth, m_uiHeight);
		else
			return false;
	}
	else
	{
		if (pOrigDib->GetBMIH()->biWidth != (int)m_uiWidth ||
			pOrigDib->GetBMIH()->biHeight != (int)m_uiHeight)
		{
			// Make a Copy!
			DibCopy = *pOrigDib;
			pDib = &DibCopy;

			// Decompress For Resizing
			if (!pDib->Decompress(24))
				return false;

			// Resize
			if (!pDib->NearestNeighborResizeBits(m_uiWidth, m_uiHeight))
				return false;

			// Reconvert To YUV
			if (m_uiWidth > 0 &&
				m_uiHeight > 0 &&
				pDib->GetBits() &&
				m_pYUV)
				::RGB24ToYUV420(pDib->GetBits(), m_pYUV, m_uiWidth, m_uiHeight);
			else
				return false;
		}
		else
		{
			ASSERT(m_nYUVBufSize <= (int)pOrigDib->GetImageSize());
			memcpy(m_pYUV, pOrigDib->GetBits(), pOrigDib->GetImageSize());
		}
	}

	// Compress
	m_cDataCount = 0;
	CompressFrame(&m_cParams, &m_Bits);
	
	return true;
}

CString CH263Enc::FourCCToString(unsigned int uiFourCC)
{
	char ch0 = (char)(uiFourCC & 0xFF);
	char ch1 = (char)((uiFourCC >> 8) & 0xFF);
	char ch2 = (char)((uiFourCC >> 16) & 0xFF);
	char ch3 = (char)((uiFourCC >> 24) & 0xFF);
#ifdef _UNICODE
	WCHAR wch0, wch1, wch2, wch3;
	mbtowc(&wch0, &ch0, sizeof(WCHAR));
	mbtowc(&wch1, &ch1, sizeof(WCHAR));
	mbtowc(&wch2, &ch2, sizeof(WCHAR));
	mbtowc(&wch3, &ch3, sizeof(WCHAR));
	return (CString(wch0) + CString(wch1) + CString(wch2) + CString(wch3));
#else
	return (CString(ch0) + CString(ch1) + CString(ch2) + CString(ch3));
#endif
}

unsigned int CH263Enc::StringToFourCC(CString sFourCC)
{
#ifdef _UNICODE
	char FourCC[5];
	wcstombs(FourCC, sFourCC, 5);
	return ((DWORD)(BYTE)(FourCC[0]) |
			((DWORD)(BYTE)(FourCC[1]) << 8) |
			((DWORD)(BYTE)(FourCC[2]) << 16) |
			((DWORD)(BYTE)(FourCC[3]) << 24 ));
#else	
    return ((DWORD)(BYTE)(sFourCC[0]) |
			((DWORD)(BYTE)(sFourCC[1]) << 8) |
			((DWORD)(BYTE)(sFourCC[2]) << 16) |
			((DWORD)(BYTE)(sFourCC[3]) << 24 ));
#endif
}

unsigned int CH263Enc::FourCCMakeLowerCase(unsigned int uiFourCC)
{
	CString s = FourCCToString(uiFourCC);
	s.MakeLower();
	return StringToFourCC(s);
}

bool CH263Enc::IsI420(unsigned int uiFourCC)
{
	// Note:
	// The i420 (or yuv420) from the webcam is not working
	// -> Always make conversion i420 -> RGB24 -> i420 ...
	return false;

	unsigned int uiLowerCaseFourCC = FourCCMakeLowerCase(uiFourCC);
	unsigned int uiI420FourCC = StringToFourCC(_T("i420"));
	return (uiI420FourCC == uiLowerCaseFourCC);
}
