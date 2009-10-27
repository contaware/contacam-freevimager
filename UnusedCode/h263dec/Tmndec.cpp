////////////////////////////////////////////////////////////////////////////
//
//
//    Project     : VideoNet version 1.1.
//    Description : Peer to Peer Video Conferencing over the LAN.
//	  Author      :	Nagareshwar Y Talekar ( nsry2002@yahoo.co.in)
//    Date        : 15-6-2004.
//
//    This is the modified version of tmndecode (H.263 decoder) 
//    written by Karl & Robert.It was in ANSI C. I have converted into C++
//    so that it can be integrated into any windows application. I have 
//    removed some of the files which had display and file storing 
//    functions.I have removed the unnecessary code and also added some
//    new files..
//	  Original library dealt with files. Input & Output , both were files.
//    I have done some major changes so that it can be used for real time 
//    decoding process. Now one can use this library for decoding H263 frames. 
//
//
//    File description : 
//    Name    : Tmndec.cpp
//    Details : Main entry point for decoder.
//
//    Usage : 
//			1) InitH263Decoder
//			2) Invoke DecompressFrame with parameters
//			      compressed data and output data pointers.
//			  Note > Output data must be large enough to hold RGB data.
//                 > Change the DecompressFrame function if you want 
//					 output in YUV420 format only. Just comment out conversion routine
//          3) Repeat the step 2 for each compressed frame , for exit do 4
//			4) ExitH263Decoder  .
//
/////////////////////////////////////////////////////////////////////////////



/************************************************************************
 *
 *  tmndec.c, main(), initialization, options for tmndecode (H.263 decoder)
 *  Copyright (C) 1996  Telenor R&D, Norway
 *        Karl Olav Lillevold <Karl.Lillevold@nta.no>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Karl Olav Lillevold               <Karl.Lillevold@nta.no>
 *  Telenor Research and Development
 *  P.O.Box 83                        tel.:   +47 63 84 84 00
 *  N-2007 Kjeller, Norway            fax.:   +47 63 81 00 76
 *
 *  Robert Danielsen                  e-mail: Robert.Danielsen@nta.no
 *  Telenor Research and Development  www:    http://www.nta.no/brukere/DVC/
 *  P.O.Box 83                        tel.:   +47 63 84 84 00
 *  N-2007 Kjeller, Norway            fax.:   +47 63 81 00 76
 *  
 ************************************************************************/

/*
 * based on mpeg2decode, (C) 1994, MPEG Software Simulation Group
 * and mpeg2play, (C) 1994 Stefan Eckart
 *                         <stefan@lis.e-technik.tu-muenchen.de>
 *
 */
#include "stdafx.h"
#include "H263Dec.h"
#include "YuvToRgb.h"

void CH263Dec::Init()
{
	Free();

	for (int i = 0 ; i < 3 ; i++)
	{
		m_refframe[i] = NULL;
		m_oldrefframe[i] = NULL;
		m_bframe[i] = NULL;
		m_newframe[i] = NULL;
		m_edgeframe[i] = NULL;
		m_edgeframeorig[i] = NULL;
		m_exnewframe[i] = NULL;
	}
	memset(m_MV, 0, sizeof(m_MV));
	memset(m_modemap, 0, sizeof(m_modemap));
	m_clp_base = NULL; 
	m_clp = NULL;
	m_horizontal_size = 0;
	m_vertical_size = 0;
	m_mb_width = 0;
	m_mb_height = 0;
	m_coded_picture_width = 0;
	m_coded_picture_height = 0;
	m_chrom_width = 0;
	m_chrom_height = 0;
	m_blk_cnt = 0;
	m_pict_type = 0;
	m_newgob = 0;
	m_mv_outside_frame = 0;
	m_syntax_arith_coding = 0;
	m_adv_pred_mode = 0;
	m_pb_frame = 0;
	m_long_vectors = 0;
	m_fault = 0;
	m_expand = 0;
	m_refidct = 0;
	m_matrix_coefficients = 0;
	m_temp_ref = 0;
	m_prev_temp_ref = -1;
	m_quant = 0;
	m_source_format = 0;
	m_cframe = NULL;
	m_csize = 0;
	m_cindex = 0;
	m_yp = NULL;
	m_up = NULL;
	m_vp = NULL;
	memset(&m_base, 0, sizeof(ld_struct));
	m_outtype = 0;
	m_trd = 0;
	m_trb = 0;
	m_bscan = 0;
	m_bquant = 0;
	m_iclp = NULL;
	m_sac_low = 0;
	m_sac_high = 0;
	m_sac_code_value = 0;
	m_sac_bit = 0;
	m_sac_length = 0;
	m_sac_index = 0;
	m_sac_cum = 0;
	m_sac_zerorun = 0;
	m_ld = &m_base;
	m_nFirst = 1;
	m_FrameNum = 0;
	m_quiet = 1;  // keep quiet
	m_trace = 0;  // don't trace

    // Open log file
	if (m_trace == 1 ||
		m_quiet == 0)
		m_dlog = fopen("decoder.log","w");
	if (m_dlog == NULL)
	{
		m_trace=0;
		m_quiet=1;
	}
	
	initbits();    // Setup source buffer pointers
}

/**
*    Decompress the frame.....
*    > After decompression 
*    > yp ,up and vp pointers will point to respective Y , U , V frames...
*  
*  Input 
*      cdata    compressed data
*      size     size of the compressed frame
*      outdata  pointer to rgb buffer (size must be enough to store data)  
*  
*  Output
*      return 1  success  , outdata contains rgbdata
*	   return 0  operation failed...
*/

int CH263Dec::DecompressFrame(unsigned char *cdata,
							  int size,
							  unsigned char *outdata,
							  int outsize,
							  int& first,
							  unsigned __int64& framenum)
{	
	// Initialize output pointers...
	m_yp=NULL;
	m_up=NULL;
	m_vp=NULL;

	// The Frame Buffer and Size	 
	m_cindex=0;       // index into the cframe....
	m_csize=size;     // size of compressed frame 
	m_cframe=cdata;   // pointer to compressed frame

	// Reset the pointers
	initbits();

	// Get the frame header
	getheader();

	// If first time...then call initdecoder....
	// This has to be done after calling getheader()
	// because getheader() setup some variables using the information
	// from header...
	if (first) 
	{
	   initdecoder();
	   first = 0;
	}

	// Decompress the frame and get the picture frame in YUV format
	getpicture(&framenum);
	framenum++;

	if (m_yp==NULL || m_up==NULL || m_vp==NULL)
	{
		if (m_trace)
			fputs("decompression failed...",m_dlog);
		return  0;
	}

   	// convert YUV to RGB
	// *******  TODO *********
	// check if outdata is enough to store rgb data
	//
	int totalsize = m_horizontal_size * m_vertical_size *3;
	if (outsize < totalsize || outdata == NULL)
	{
		if (m_dlog)
			fputs("Output buffer is not sufficient",m_dlog);
		return 0;			
	}

	::YUVToRGB24(m_yp, m_up, m_vp, outdata, m_horizontal_size, m_vertical_size);
 
	return 1;
}


/**
*    Do the clean up here
*
*/
void CH263Dec::Free()
{
	int i;
    
	for (i = 0 ; i < 3 ; i++)
	{
		if (m_refframe[i])
		{
			free(m_refframe[i]);
			m_refframe[i] = NULL;
		}

		if (m_oldrefframe[i])
		{
			free(m_oldrefframe[i]);
			m_oldrefframe[i] = NULL;
		}

		if (m_bframe[i])
		{
			free(m_bframe[i]);
			m_bframe[i] = NULL;
		}

		if (m_edgeframeorig[i])
		{
			free(m_edgeframeorig[i]);
			m_edgeframeorig[i] = NULL;
		}
	}

	if (m_clp_base)
	{
		free(m_clp_base);
		m_clp_base = NULL;
		m_clp = NULL;
	}

	// Close Log files
	if (m_dlog)
		fclose(m_dlog);
}


/**
*    Initialize decoder
*
*
*/
void CH263Dec::initdecoder()
{
	int i, cc, size;


	/* clip table */
	if (!(m_clp_base=(unsigned char *)malloc(1024)))
		error("malloc failed\n");
	m_clp = m_clp_base;

	m_clp += 384;

	for (i=-384; i<640; i++)
		m_clp[i] = (i<0) ? 0 : ((i>255) ? 255 : i);


	/* MPEG-1 = TMN parameters */
	m_matrix_coefficients = 5;


	switch (m_source_format) 
	{
		case (SF_SQCIF):
			m_horizontal_size = 128;
			m_vertical_size = 96;
			break;
		case (SF_QCIF):
			m_horizontal_size = 176;
			m_vertical_size = 144;
			break;
		case (SF_CIF):
			m_horizontal_size = 352;
			m_vertical_size = 288;
			break;
		case (SF_4CIF):
			m_horizontal_size = 704;
			m_vertical_size = 576;
			break;
		case (SF_16CIF):
			m_horizontal_size = 1408;
			m_vertical_size = 1152;
			break;
		default:
			if (m_trace)
				fputs("ERROR: Illegal input format\n",m_dlog);
			return;
	}

	m_mb_width = m_horizontal_size/16;
	m_mb_height = m_vertical_size/16;
	m_coded_picture_width = m_horizontal_size;
	m_coded_picture_height = m_vertical_size;
	m_chrom_width =  m_coded_picture_width>>1;
	m_chrom_height = m_coded_picture_height>>1;
	m_blk_cnt = 6;

	for (cc=0; cc<3; cc++)
	{
		// Find out size...
		if (cc==0)
			size = m_coded_picture_width*m_coded_picture_height;
		else
			size = m_chrom_width*m_chrom_height;

		// Allocate memory
		if (!(m_refframe[cc] = (unsigned char *)malloc(size)))
			error("malloc failed\n");

		if (!(m_oldrefframe[cc] = (unsigned char *)malloc(size)))
			error("malloc failed\n");

		if (!(m_bframe[cc] = (unsigned char *)malloc(size)))
			error("malloc failed\n");
	}

	for (cc=0; cc<3; cc++) 
	{
		if (cc==0) 
		{
			size = (m_coded_picture_width+64)*(m_coded_picture_height+64);

			if (!(m_edgeframeorig[cc] = (unsigned char *)malloc(size)))
				error("malloc failed\n");

			m_edgeframe[cc] = m_edgeframeorig[cc] + (m_coded_picture_width+64) * 32 + 32;
		}
		else 
		{
			size = (m_chrom_width+32)*(m_chrom_height+32);

			if (!(m_edgeframeorig[cc] = (unsigned char *)malloc(size)))
				error("malloc failed\n");

			m_edgeframe[cc] = m_edgeframeorig[cc] + (m_chrom_width+32) * 16 + 16;
		}
	}

  
	/* IDCT */
	if (m_refidct)
		init_idctref();
	else
		init_idct();
}

// Error routine
void CH263Dec::error(char *text)
{
	if(m_dlog)
		fputs(text,m_dlog);
}

// trace output
void CH263Dec::printbits(int code,int bits,int len)
{
   if(m_dlog)
	   fputs("printbits : remove comment..!!!",m_dlog);
}






