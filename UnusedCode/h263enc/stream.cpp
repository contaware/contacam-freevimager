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
//    Name    : stream.cpp
//
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

/*************************************************************
Copyright (C) 1990, 1991, 1993 Andy C. Hung, all rights reserved.
PUBLIC DOMAIN LICENSE: Stanford University Portable Video Research
Group. If you use this software, you agree to the following: This
program package is purely experimental, and is licensed "as is".
Permission is granted to use, modify, and distribute this program
without charge for any purpose, provided this license/ disclaimer
notice appears in the copies.  No warranty or maintenance is given,
either expressed or implied.  In no event shall the author(s) be
liable to you or a third party for any special, incidental,
consequential, or other damages, arising out of the use or inability
to use the program for any purpose (or the loss of data), even if we
have been advised of such possibilities.  Any public reference or
advertisement of this source code should refer to it as the Portable
Video Research Group (PVRG) code, and not by any author(s) (or
Stanford University) name.
*************************************************************/

/*
************************************************************
stream.c

This file handles all of the bit-level stream commands.

************************************************************
*/

#include "stdafx.h"
#include "H263Enc.h"

#define WRITEBYTE(x) WriteByteFunction(x) 

#define mput1()\
{m_current_write_byte|=m_bit_set_mask[m_write_position--];\
 if (m_write_position<0) {WRITEBYTE(m_current_write_byte); m_write_position=7;m_current_write_byte=0;}}

#define mput0()\
{m_write_position--;if(m_write_position<0){WRITEBYTE(m_current_write_byte);m_write_position=7;m_current_write_byte=0;}}


/*
mwinit() initializes variables for writing 
*/
void CH263Enc::mwinit()
{
    m_current_write_byte = 0;
    m_write_position = 7;
}

/*
mwcloseinit() closes the write bit-stream. It flushes the remaining byte
with ones, consistent with -1 returned on EOF.
*/
void  CH263Enc::mwcloseinit()
{
    while(m_write_position!=7)
	{
	    mput1();
	}
}

/*
zeroflush() flushes out the rest of the byte with 0's.
returns number og bits written to bitstream (kol)
*/
int CH263Enc::zeroflush()
{
    int bits = 0;
    while (m_write_position!=7)
	{
	    mput0();
	    bits++;
	}
    return bits;
}

/*

mputv() puts a n bits to the stream from byte b.

*/
void CH263Enc::mputv(int n,int b)
{
    char *bit;

    if (m_trace)
	{
		bit = BitPrint(n, b);
		fprintf(m_tf,"wrote %s = %d\n", bit, b);
		free(bit);
    }
    while(n--)
	{
	    if (b & m_bit_set_mask[n])
		{
			mput1();
		}
		else
		{
			mput0();
		}
	}
}