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
//    Name    : GetBits.cpp
//
/////////////////////////////////////////////////////////////////////////////


/************************************************************************
 *
 *  getbits.c, bit level routines for tmndecode (H.263 decoder)
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

/* initialize buffer, call once before first getbits or showbits */
void CH263Dec::initbits()
{
	m_ld->incnt = 0;
	m_ld->rdptr = m_ld->rdbfr + 2048;
	m_ld->bitcnt = 0;
}

/*
*		Read bytes from buffer
*/
void CH263Dec::fillbfr()
{
	int l;

	// Store prev 4 bytes into first 4 bytes....
	// new data is appended to these prev bytes....
	m_ld->inbfr[0] = m_ld->inbfr[8];
	m_ld->inbfr[1] = m_ld->inbfr[9];
	m_ld->inbfr[2] = m_ld->inbfr[10];
	m_ld->inbfr[3] = m_ld->inbfr[11];

	// Check if whether we have to read the data. ..or it already exist

	if (m_ld->rdptr>=m_ld->rdbfr+2048)
	{
		// copy the data from buffer..
		// ....2048 bytes at a time...
		if(m_csize>=2048)
		{
			memcpy(m_ld->rdbfr,m_cframe+m_cindex,2048);
			l=2048;
			m_cindex+=2048;
			m_csize-=2048;
		}
		else
		{
			// Read all available data from buffer
			if(m_csize>0)
			{
				memcpy(m_ld->rdbfr,m_cframe+m_cindex,m_csize);
				l=m_csize;
				m_cindex+=m_csize;
				m_csize=0;
			}
			else  // buffer is empty
			{
				l=0;
				m_csize=0;
			}
		}
   
		// reset the pointer...to start of data
		m_ld->rdptr = m_ld->rdbfr;
    
		// if less data is read....then append "End Sequence code"
		if (l<2048)
		{
			if (l<0)
			l = 0;

			while (l<2048)   /* Add recognizable sequence end code */
			{
				m_ld->rdbfr[l++] = 0;
				m_ld->rdbfr[l++] = 0;
				m_ld->rdbfr[l++] = (1<<7) | (SE_CODE<<2);
			}
		}
	}

	// Store 8 byte of data into in buffer
	for (l=0; l<8; l++)
	m_ld->inbfr[l+4] = m_ld->rdptr[l];

	// Update pointers..
	m_ld->rdptr+= 8;		// 8 bytes..
	m_ld->incnt+= 64;       // 64 bits...
}


/* return next n bits (right adjusted) without advancing */
unsigned int CH263Dec::showbits(int n)
{
	unsigned char *v;
	unsigned int b;
	int c;

	// if inbuf contains less bits...then read again....
	if (m_ld->incnt<n)
		fillbfr();

	// get pointer to start data in inbuffer
	//
	v = m_ld->inbfr + ((96 - m_ld->incnt)>>3);

	// combine bytes to form int
	b = (v[0]<<24) | (v[1]<<16) | (v[2]<<8) | v[3];

	c = ((m_ld->incnt-1) & 7) + 25;

	// send the righ adjusted value....
	return (b>>(c-n)) & m_msk[n];
}


/* return next bit (could be made faster than getbits(1)) */
unsigned int CH263Dec::getbits1()
{
	return getbits(1);
}


/* advance by n bits */
// Update the pointer....
// === Remove data from the buffer...
void CH263Dec::flushbits(int n)
{
	m_ld->bitcnt+= n;
	m_ld->incnt-= n;
	if (m_ld->incnt < 0)
		fillbfr();
}


/* return next n bits (right adjusted) */
//
// Get next n bits from the file header...
//
unsigned int CH263Dec::getbits(int n)
{
	unsigned int l;

	l = showbits(n);
	flushbits(n);

	return l;
}