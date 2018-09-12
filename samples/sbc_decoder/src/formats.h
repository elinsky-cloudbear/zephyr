/*
 *
 *  Bluetooth low-complexity, subband codec (SBC) library
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
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
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <misc/byteorder.h>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define COMPOSE_ID(a,b,c,d)	((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define COMPOSE_ID(a,b,c,d)	((d) | ((c)<<8) | ((b)<<16) | ((a)<<24))
#else
#error "Wrong endian"
#endif

#define AU_MAGIC		COMPOSE_ID('.','s','n','d')
#define AU_FMT_LIN16	3

struct au_header {
	uint32_t magic;		/* '.snd' */
	uint32_t hdr_size;	/* size of header (min 24) */
	uint32_t data_size;	/* size of data */
	uint32_t encoding;	/* see to AU_FMT_XXXX */
	uint32_t sample_rate;	/* sample rate */
	uint32_t channels;	/* number of channels (voices) */
};
