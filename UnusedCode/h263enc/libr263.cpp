////////////////////////////////////////////////////////////////////////////
//
//
//    Project     : VideoNet version 1.1.
//    Description : Peer to Peer Video Conferencing over the LAN.
//	  Author      :	Nagareshwar Y Talekar ( nsry2002@yahoo.co.in)
//    Date        : 15-6-2004.
//
//    I have converted origional fast h.263 encoder library from C to C++ 
//	  so that it can be integrated into any windows application easily.
//	  I have removed some of unnecessary codes/files from the
//	  fast h263 library.Also moved definitions and declarations
//	  in their proper .h and .cpp files.
//
//    File description : 
//    Name    : libr263.cpp
//    Details : Entry point to encoder....
//
/////////////////////////////////////////////////////////////////////////////

/*************************************************
 * libr263: fast H.263 encoder library
 *
 * Copyright (C) 1996, Roalt Aalmoes, Twente University
 * SPA multimedia group
 *
 * Based on Telenor TMN 1.6 encoder (Copyright (C) 1995, Telenor R&D)
 * created by Karl Lillevold 
 *
 * Author encoder: Roalt Aalmoes, <aalmoes@huygens.nl>
 * 
 * Date: 31-07-96
 **************************************************/

#include "stdafx.h"
#include "H263Enc.h"

int CH263Enc::Init(CParam *params)
{
	m_pic = (Pict*)malloc(sizeof(Pict));
	if(!m_pic) 
		return -1;

	m_pic->unrestricted_mv_mode = DEF_UMV_MODE;
	m_pic->use_gobsync = DEF_INSERT_SYNC;
	m_pic->PB = 0;
	m_pic->TR = 0;
	m_pic->QP_mean = 0.0;

	if (params->format == CPARAM_QCIF)
	{
		m_pels = QCIF_YWIDTH;
		m_lines = QCIF_YHEIGHT;
		m_cpels = QCIF_YWIDTH/2;
		m_pic->source_format = SF_QCIF;
	}
	else if (params->format == CPARAM_CIF)
	{
		m_pels = CIF_YWIDTH;
		m_lines = CIF_YHEIGHT;
		m_cpels = CIF_YWIDTH/2;
		m_pic->source_format = SF_CIF;
	} 
	else if (params->format == CPARAM_SQCIF)
	{
		m_pels = SQCIF_YWIDTH;
		m_lines = SQCIF_YHEIGHT;
		m_cpels = SQCIF_YWIDTH/2;
		m_pic->source_format = SF_SQCIF;
	}
	else if (params->format == CPARAM_4CIF)
	{
		m_pels = CIF4_YWIDTH;
		m_lines = CIF4_YHEIGHT;
		m_cpels = CIF4_YWIDTH/2;
		m_pic->source_format = SF_4CIF;  
	}
	else if (params->format == CPARAM_16CIF)
	{
		m_pels = CIF16_YWIDTH;
		m_lines = CIF16_YHEIGHT;
		m_cpels = CIF16_YWIDTH/2;
		m_pic->source_format = SF_16CIF;
	} 
	else
	{
		m_pels = params->pels;
		m_lines = params->lines;
		m_cpels = params->pels / 2;
		m_pic->source_format = 0;	/* ILLEGAL H.263! Use it only for testing */
	}
  
	m_mbr = m_lines / MB_SIZE;
	m_mbc = m_pels / MB_SIZE;
	m_uskip = m_lines*m_pels;
	m_vskip = m_uskip + m_lines*m_pels/4;
	m_sizeof_frame = (m_vskip + m_lines*m_pels/4)*sizeof(int);

	m_headerlength = DEF_HEADERLENGTH;

	/* Initalize VLC_tables */
	InitHuff();
	mwinit();

	/* Init motion detection */
	init_motion_detection();

	/* Set internal variables */
	m_advanced = DEF_ADV_MODE;
	m_mv_outside_frame = DEF_UMV_MODE || DEF_ADV_MODE;
	m_long_vectors = DEF_UMV_MODE;
	m_pb_frames = DEF_PBF_MODE;
	m_search_p_frames = DEF_SPIRAL_SEARCH;
	m_trace = DEF_WRITE_TRACE;

	params->half_pixel_searchwindow = CPARAM_DEFAULT_SEARCHWINDOW; 
	params->inter = CPARAM_DEFAULT_INTER;
	params->search_method = CPARAM_DEFAULT_SEARCH_METHOD;
	params->advanced_method = CPARAM_DEFAULT_ADVANCED_METHOD;
	params->Q_inter = CPARAM_DEFAULT_INTER_Q;
	params->Q_intra = CPARAM_DEFAULT_INTRA_Q;

	params->interpolated_lum = (unsigned int*)malloc(m_pels*m_lines*4*sizeof(int));

	if (!params->interpolated_lum)
	{
		free(m_pic);
		FreeHuff();
		return -1;
	}

	params->recon = (unsigned int*)malloc(m_sizeof_frame);
	if (!params->recon)
	{
		free(m_pic);
		FreeHuff();
		free(params->interpolated_lum);
		return -1;
	}

	return 0;
}

void CH263Enc::SkipH263Frames(int frames_to_skip)
{
	m_pic->TR += frames_to_skip % 256;
}

int CH263Enc::CompressFrame(CParam *params, Bits *bits)
{
	if (!params->inter)
		CodeIntraH263(params, bits);
	else
		CodeInterH263(params, bits);

	bits->header += zeroflush();	/* pictures shall be byte aligned */
	m_pic->TR += 1 % 256;		/* one means 30 fps */
	return 0;
}

void CH263Enc::Free(CParam* params)
{
	mwcloseinit();
	free(params->interpolated_lum);
	free(params->recon);
	free(m_pic);
	FreeHuff();
}

void CH263Enc::init_motion_detection()
{
	unsigned int counter, pos;

	for(counter = 0; counter < 16; counter++)
	{
		pos = m_movement_coords[counter][0] + m_movement_coords[counter][1]*m_pels;
		m_movement_detection[counter][0] = pos;
		m_movement_detection[counter][1] = pos + 4;
		m_movement_detection[counter][2] = pos + m_pels*4;
		m_movement_detection[counter][3] = pos + m_pels*4 + 4;
	}
}

int CH263Enc::Check8x8(unsigned int* orig, unsigned int* recon, int pos)
{
	int value, index;
	register int thres = m_global_pixel_threshold;

	value = 0;

	/* Mark pixel changed when lum value differs more than "thres" */
	index = m_movement_detection[pos][0];
	value += abs(*(orig + index) - *(recon+index)) > thres;

	index = m_movement_detection[pos][1];
	value += abs(*(orig + index) - *(recon+index)) > thres;

	index = m_movement_detection[pos][2];				
	value += abs(*(orig + index) - *(recon+index)) > thres;

	index = m_movement_detection[pos][3];			
	value += abs(*(orig + index) - *(recon+index)) > thres;

	return value;
}

int CH263Enc::HasMoved(	int call_time,
						void *real,
						void *recon,
						int x, int y)
{
	int offset1;
	unsigned int *MB_orig;
	unsigned int *MB_recon;
	int position;
	int value = 0;

	offset1 = (y*m_pels+x)*MB_SIZE;
	position = call_time;

	/* Integration of 8x8 and 4x4 check might improve performance, 
	 but is not done here */
	MB_orig = (unsigned int *) real + offset1;
	MB_recon = (unsigned int *) recon + offset1;
	value += Check8x8(MB_orig, MB_recon, position);

	MB_orig += 8; MB_recon += 8;
	value += Check8x8(MB_orig, MB_recon, position); 

	MB_orig += 8*m_pels - 8; MB_recon += 8*m_pels - 8;
	value += Check8x8(MB_orig, MB_recon, position);

	MB_orig += 8; MB_recon += 8;
	value += Check8x8(MB_orig, MB_recon, position);

	return value > m_global_mb_threshold;	
	/* Mark MB changed if more than "m_global_mb_threshold" pixels are changed */
}

int CH263Enc::FindMotion(CParam* params, int mb_threshold, int pixel_threshold)
{
	int j,i;
	int counter = 0;

	m_global_mb_threshold = mb_threshold;
	m_global_pixel_threshold = pixel_threshold;

	for(j = 0; j < m_mbr; j++)
	{
		for(i = 0; i < m_mbc; i++)
		{
			*(params->EncodeThisBlock + j*m_mbc + i) = 
			HasMoved(m_find_motion_call_time, params->data, params->recon, i,j);

			counter += *(params->EncodeThisBlock +j*m_mbc + i);
		}
	}

	m_find_motion_call_time = (m_find_motion_call_time + 1) % 16;

	return counter;
}



