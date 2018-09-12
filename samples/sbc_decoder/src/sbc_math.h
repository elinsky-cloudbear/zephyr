/*
 *
 *  Bluetooth low-complexity, subband codec (SBC) library
 *
 *  Copyright (C) 2008-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2004-2005  Henryk Ploetz <henryk@ploetzli.ch>
 *  Copyright (C) 2005-2008  Brad Midgley <bmidgley@xmission.com>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/* C does not provide an explicit arithmetic shift right but this will
   always be correct and every compiler *should* generate optimal code */
#define ASR(val, bits) ((-2 >> 1 == -1) ? \
		 ((int32_t)(val)) >> (bits) : ((int32_t) (val)) / (1 << (bits)))

#define SCALE_SPROTO4_TBL	12
#define SCALE_SPROTO8_TBL	14
#define SCALE_NPROTO4_TBL	11
#define SCALE_NPROTO8_TBL	11
#define SCALE4_STAGED1_BITS	15
#define SCALE8_STAGED1_BITS	15

#define SCALE4_STAGED1(src) ASR(src, SCALE4_STAGED1_BITS)
#define SCALE8_STAGED1(src) ASR(src, SCALE8_STAGED1_BITS)

#define MUL(a, b)        ((a) * (b))
#define MULA(a, b, res)  ((a) * (b) + (res))
