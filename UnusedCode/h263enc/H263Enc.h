#ifndef _INC_H263ENCCLASS
#define _INC_H263ENCCLASS

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "def.h"
#include "config.h"

// DCT & IDCT Configuration
#undef	FASTDCT
#define FASTIDCT
#undef	VERYFASTIDCT

#define COMPRESSION_BUFFER_SIZE	500000

class CDib;

class CH263Enc
{
	public:

		/* Point structure */
		struct Point
		{
			int x;
			int y;
		};

		/* Motionvector structure */
		struct MotionVector
		{
			int x;			/* Horizontal comp. of mv 	 */
			int y;			/* Vertical comp. of mv 	 */
			int x_half;		/* Horizontal half-pel acc.	 */
			int y_half;		/* Vertical half-pel acc.	 */
			int min_error;	/* Min error for this vector	 */
			int Mode;		/* Necessary for adv. pred. mode */
		};

		/* Structure with image data */
		struct PictImage
		{
			unsigned int *lum;		/* Luminance plane		*/
			unsigned int *Cr;		/* Cr plane			*/
			unsigned int *Cb;		/* Cb plane			*/
		};

		/* Picture structure */
		struct Pict
		{
			int prev; 
			int curr;
			int TR;				/* Time reference */
			int source_format;
			int picture_coding_type;
			int spare;
			int unrestricted_mv_mode;
			int PB;
			int QUANT;
			int DQUANT;
			int MB;
			int seek_dist;		/* Motion vector search window */
			int use_gobsync;	/* flag for gob_sync */
			int MODB;			/* B-frame mode */
			int BQUANT;			/* which quantizer to use for B-MBs in PB-frame */
			int TRB;			/* Time reference for B-picture */
			int frame_inc;		/* buffer control frame_inc */
			float QP_mean;		/* mean quantizer */
		};

		/* Slice structure */
		struct Slice
		{
			unsigned int vert_pos;		/* Vertical position of slice 	*/
			unsigned int quant_scale;	/* Quantization scale			*/
		};

		/* Macroblock structure */
		struct Macroblock
		{
			int mb_address;			/* Macroblock address 	*/
			int macroblock_type;	/* Macroblock type 		*/
			int skipped;			/* 1 if skipped			*/
			MotionVector motion;	/* Motion Vector 		*/
		};

		/* Structure for macroblock data */
		struct MB_Structure
		{
			int lum[16][16];
			int Cr[8][8];
			int Cb[8][8];
		};

		/* Structure for average results and virtal buffer data */
		struct Results
		{
			float SNR_l;		/* SNR for luminance */
			float SNR_Cr;		/* SNR for chrominance */
			float SNR_Cb;
			float QP_mean;		/* Mean quantizer */
		};

		struct CParam
		{
			/*	Contains all the parameters that are needed for 
				encoding plus all the status between two encodings
			*/
			int half_pixel_searchwindow; /* size of search window in half pixels
											if this value is 0, no search is performed
											*/
			int format;
			int pels;			/* Only used when format == CPARAM_OTHER */
			int lines;			/* Only used when format == CPARAM_OTHER */
			int inter;			/*	TRUE for INTER frame encoded frames,
									FALSE for INTRA frames */
			int search_method;	/* DEF_EXHAUSTIVE or DEF_LOGARITHMIC */
			int advanced_method;/* TRUE : Use array to determine 
									macroblocks in INTER frame
									mode to be encoded */
			int Q_inter;			/* Quantization factor for INTER frames */
			int Q_intra;			/* Quantization factor for INTRA frames */
			unsigned int *data;		/* source data in qcif format */
			unsigned int *interpolated_lum;	/* intepolated recon luminance part */
			unsigned int *recon;		/* Reconstructed copy of compressed frame */
			int *EncodeThisBlock; 
										/* Array when advanced_method is used */
		};

		/* Structure for counted bits */
		struct Bits
		{
			int Y;
			int C;
			int vec;
			int CBPY;
			int CBPCM;
			int MODB;
			int CBPB;
			int COD;
			int header;
			int DQUANT;
			int total;
			int no_inter;
			int no_inter4v;
			int no_intra;
			/* NB: Remember to change AddBits(), ZeroBits() and AddBitsPicture() 
			when entries are added here */
		};


		// Video Format structure...
		struct qcif 
		{
			unsigned int Y[QCIF_YHEIGHT][QCIF_YWIDTH];
			unsigned int U[QCIF_UHEIGHT][QCIF_UWIDTH];
			unsigned int V[QCIF_VHEIGHT][QCIF_VWIDTH];
		};

		struct cif 
		{
			unsigned int Y[CIF_YHEIGHT][CIF_YWIDTH];
			unsigned int U[CIF_UHEIGHT][CIF_UWIDTH];
			unsigned int V[CIF_VHEIGHT][CIF_VWIDTH];
		};

		struct qcif8bit 
		{
			unsigned char Y[QCIF_YHEIGHT][QCIF_YWIDTH];
			unsigned char U[QCIF_UHEIGHT][QCIF_UWIDTH];
			unsigned char V[QCIF_VHEIGHT][QCIF_VWIDTH];
		};

		struct cif8bit 
		{
			unsigned char Y[CIF_YHEIGHT][CIF_YWIDTH];
			unsigned char U[CIF_UHEIGHT][CIF_UWIDTH];
			unsigned char V[CIF_VHEIGHT][CIF_VWIDTH];
		};

		struct EHUFF
		{
			int n;
			int *Hlen;
			int *Hcode;
		};

	public:
		CH263Enc();
		CH263Enc(int nCIFFormat);
		virtual ~CH263Enc();
		bool SetCIFFormat(int nCIFFormat);
		static int GetCIFFormatFromSize(unsigned int uiWidth, unsigned int uiHeight);
		bool EncodeH263(CDib* pOrigDib);
		unsigned char* GetEncodedDataBuf();
		int GetEncodedDataSize();
		
	protected:
		int CompressFrame(CParam *params, Bits *bits);
		int Init(CParam *params);
		void Free(CParam *params);
		void SkipH263Frames(int frames_to_skip);
		void ResetEncodedDataBuf();
		void WriteByteFunction(int byte);
		static inline bool IsI420(unsigned int uiFourCC);
		static inline CString FourCCToString(unsigned int uiFourCC);
		static inline unsigned int StringToFourCC(CString sFourCC);
		static inline unsigned int FourCCMakeLowerCase(unsigned int uiFourCC);

		// Motion Detection
		/* Procedure to detect motion, expects param->EncodeThisBlock is set to array. 
		   Advised values for threshold: mb_threholds = 2; pixel_threshold = 2 */
		int FindMotion(CParam *params, int mb_threshold, int pixel_threshold);
		void init_motion_detection();
		inline int Check8x8(unsigned int *orig,unsigned int *recon, int pos);
		int HasMoved(int call_time,  void *real, void *recon, int x, int y);

		// Huffman
		void InitHuff();
		void FreeHuff();
		void PrintEhuff();
		EHUFF* MakeEhuff(int n);
		void FreeEhuff(EHUFF* eh);
		void LoadETable(const int *array,EHUFF *table);
		int Encode(int val,EHUFF *huff);
		char* BitPrint(int length, int val);

		// Coder
		void ZeroMBlock(MB_Structure* data);
		void CodeIntraH263(CParam* params, Bits* bits);
		void CodeInterH263(CParam* params, Bits* bits);
		inline void Clip(MB_Structure* data);
		int *MB_EncodeAndFindCBP(MB_Structure* mb_orig, int QP, int I, int* CBP);
		int MB_Decode(int* qcoeff, MB_Structure* mb_recon, int QP, int I);
		void FullMotionEstimatePicture(unsigned int* curr, unsigned int* prev, 
						   unsigned int* prev_ipol, int seek_dist, 
						   MotionVector* MV_ptr,
						   int advanced_method,
						   int* EncodeThisBlock);
		void ReconCopyImage(int i, int j, unsigned int* recon, unsigned int* prev_recon);
		void ReconImage(int i, int j, MB_Structure *data, unsigned int* recon);
		void InterpolateImage(unsigned int* image,
						   unsigned int* ipol_image, 
						   int w, int h);
		void FillLumBlock( int x, int y, unsigned int* image, MB_Structure* data);
		void FillChromBlock(int x_curr, int y_curr, unsigned int* image,
					MB_Structure* data);

		// Count Bit
		void ZeroBits(Bits *bits);
		void ZeroRes(Results *res);
		int FindCBP(int *qcoeff, int Mode, int ncoeffs);
		void CountBitsVectors(MotionVector *MV_ptr, Bits *bits, 
					  int x, int y, int Mode, int newgob, Pict *pic);
		void FindPMV(MotionVector *MV_ptr, int x, int y, 
				 int *p0, int *p1, int block, int newgob, int half_pel);
		void CountBitsCoeff(int *qcoeff, int I, int CBP, Bits *bits, int ncoeffs);
		int CodeCoeff(int Mode, int *qcoeff, int block, int ncoeffs);
		int CountBitsPicture(Pict *pic);
		void AddBitsPicture(Bits *bits);
		void CountBitsMB(int Mode, int COD, int CBP, int CBPB, Pict *pic, Bits *bits);
		int CountBitsSlice(int slice, int quant);
		void ZeroVec(MotionVector *MV);
		void MarkVec(MotionVector *MV);
		void CopyVec(MotionVector *MV1, MotionVector *MV2);
		int EqualVec(MotionVector *MV2, MotionVector *MV1);
		void AddBits(Bits *total, Bits *bits);
		void AddRes(Results *total, Results *res, Pict *pic);
	
		// DCT
#ifdef FASTDCT
		int Dct(int* block, int* coeff);
		inline void mp_fwd_dct_fast(Block data2d, Block dest2d);
#else
		int Dct(int* block, int* coeff);
#endif

		// IDCT
#ifdef FASTIDCT
		int idct(int* coeff, int* block);
#elif VERYFASTIDCT
		int idct(int* coeff, int* block);
		void idctrow(int* blk);
		inline void idctcol(int* blk);
		void init_idct();
#else
		int idct(int* coeff, int* block);
		void init_idct();
#endif

		// Motion Estimation
		void FindMB(int x, int y, unsigned int *image, unsigned int MB[16][16]);
		void FullMotionEstimation(unsigned int *curr, unsigned int *prev_ipol, 
					 int seek_dist, MotionVector *current_MV, int x_curr, 
					 int y_curr);
		inline int SAD_HalfPixelMacroblock(unsigned int *ii,
						unsigned int *curr,
						int pixels_on_line, int Min_SAD);
		inline int SAD_HalfPixelMacroblock2(unsigned int *ii,
						unsigned int *curr,
						int pixels_on_line, int Min_SAD);
		unsigned int* LoadArea(unsigned int *im, int x, int y, 
					int x_size, int y_size, int lx);

		// Prediction
		MB_Structure *Predict_P(unsigned int *curr_image, unsigned int *prev_image,
			unsigned int *prev_ipol, int x, int y, 
			MotionVector *MV_ptr);
		void DoPredChrom_P(int x_curr, int y_curr, int dx, int dy,
				   unsigned int *curr, unsigned int *prev, 
				   MB_Structure *pred_error);
		void FindPred(int x, int y, MotionVector *fr, unsigned int *prev, 
				  int *pred); 
		MB_Structure *MB_Recon_P(unsigned int *prev_image, unsigned int *prev_ipol,
					 MB_Structure *diff, int x_curr, int y_curr, 
					 MotionVector *MV_ptr);
		void ReconLumBlock_P(int x, int y, MotionVector *fr,
					 unsigned int *prev, int *data);
		void ReconChromBlock_P(int x_curr, int y_curr, int dx, int dy,
					   unsigned int *prev, MB_Structure *data);
		void FindChromBlock_P(int x_curr, int y_curr, int dx, int dy,
					   unsigned int *prev, MB_Structure *data);
		int ChooseMode(unsigned int *curr, int x_pos, int y_pos, int min_SAD);


		// Quantization
		void Dequant(int *qcoeff, int *rcoeff, int QP, int I);
		int QuantAndFindCBP(int *coeff, int *qcoeff, int QP, int I, int CBP_Mask);


		// Writing
		void mwinit();
		void mwcloseinit();
		int zeroflush();
		void mputv(int n,int b);


		// Vars

		int m_cDataCount;
		unsigned char m_cData[COMPRESSION_BUFFER_SIZE];

		CParam m_cParams;
		unsigned int* m_pYUV;
		int m_nYUVBufSize;
		unsigned int m_uiWidth;
		unsigned int m_uiHeight;
		Bits m_Bits;

		int m_advanced;
		int m_cpels;
		int m_headerlength;
		int m_lines;
		int m_long_vectors;
		int m_mbc;
		int m_mbr;
		int m_mv_outside_frame;
		int m_pb_frames;
		int m_pels;
		int m_search_p_frames;
		size_t m_sizeof_frame;
		float m_target_framerate;
		FILE* m_tf;	// Trace File
		int m_trace;// Do Trace Flag
		int m_uskip;
		int m_vskip;
		Pict* m_pic;

		// Huffman Vars
		EHUFF* m_vlc_3d;
		EHUFF* m_vlc_cbpcm;
		EHUFF* m_vlc_cbpcm_intra;
		EHUFF* m_vlc_cbpy;
		EHUFF* m_vlc_mv;

		// DCT & IDCT
		static const int m_zigzag[8][8];

		// IDCT
#ifdef FASTIDCT
		/* Nothing */
#elif VERYFASTIDCT
		/* clipping table */
		int m_iclip[1024];
		int* m_iclp;
#else
		/* cosine transform matrix for 8x1 IDCT */
		double m_cos[8][8];
#endif

		// Motion Detection
		int m_find_motion_call_time;
		int m_global_mb_threshold;
		int m_global_pixel_threshold;
		int m_movement_detection[16][4];
		/* This array determines the order in a pixel is checked per 4x4 block */
		/* {x, y} within [0..3] */
		static const unsigned int m_movement_coords[16][2];

		// Writing
		int m_current_write_byte;
		int m_write_position;
		static const int m_bit_set_mask[32];

		/* Motion vectors */
		static const int m_vlc_mv_coeff[];

		/* CBPCM (MCBPC) */
		static const int m_vlc_cbpcm_intra_coeff[];
		static const int m_vlc_cbpcm_coeff[];

		/* CBPY */
		static const int m_vlc_cbpy_coeff[];

		/* 3D VLC */
		static const int m_vlc_3d_coeff[];
};

#endif //!_INC_H263ENCCLASS
