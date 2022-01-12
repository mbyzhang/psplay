/* Copyright 2017 Tania Hagn.
 * Copyright 2021 Peter Zhang.
 *
 * This file is part of x8b10b.
 *
 *    Daisy is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Daisy is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Daisy.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __X8B10B_TABLES_H__
#define __X8B10B_TABLES_H__

#include <stdint.h>

const uint8_t b5b6_1[64] = {
 /*D.00:+2N*/ 0xe7, /*D.01:+2N*/ 0xdd, /*D.02:+2N*/ 0xed, /*D.03:   */ 0x31,
 /*D.04:+2N*/ 0xf5, /*D.05:   */ 0x29, /*D.06:   */ 0x19, /*D.07:  N*/ 0xb8,
 /*D.08:+2N*/ 0xf9, /*D.09:   */ 0x25, /*D.10:   */ 0x15, /*D.11:   */ 0x34,
 /*D.12:   */ 0x0d, /*D.13:   */ 0x2c, /*D.14:   */ 0x1c, /*D.15:+2N*/ 0xd7,
 /*D.16:+2N*/ 0xdb, /*D.17:   */ 0x23, /*D.18:   */ 0x13, /*D.19:   */ 0x32,
 /*D.20:   */ 0x0b, /*D.21:   */ 0x2a, /*D.22:   */ 0x1a, /*D.23:+2N*/ 0xfa,
 /*D.24:+2N*/ 0xf3, /*D.25:   */ 0x26, /*D.26:   */ 0x16, /*D.27:+2N*/ 0xf6,
 /*D.28:   */ 0x0e, /*D.29:+2N*/ 0xee, /*D.30:+2N*/ 0xde, /*D.31:+2N*/ 0xeb,

 /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff,
 /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff,
 /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff,
 /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff,
 /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff,
 /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*K.23:+2N*/ 0xfa,
 /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*K.27:+2N*/ 0xf6,
 /*K.28:+2N*/ 0xcf, /*K.29:+2N*/ 0xee, /*K.30:+2N*/ 0xde, /*NOTM:   */ 0xff,
};

const uint8_t b3b4_1[32] = {
 /*D.x.0 :+2N*/ 0xcb, /*D.x.1 :   */ 0x09, /*D.x.2 :   */ 0x05, /*D.x.3 :  N*/ 0x8c,
 /*D.x.4 :+2N*/ 0xcd, /*D.x.5 :   */ 0x0a, /*D.x.6 :   */ 0x06, /*D.x.P7:+2N*/ 0xce,
 /*D.x.A7:+2N*/ 0xc7, /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff,
 /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff,

 /*K.x.0 :+2N*/ 0xcb, /*K.x.1 :  N*/ 0x86, /*K.x.2 :  N*/ 0x8a, /*K.x.3 :  N*/ 0x8c,
 /*K.x.4 :+2N*/ 0xcd, /*K.x.5 :  N*/ 0x85, /*K.x.6 :  N*/ 0x89, /*K.x.7 :+2N*/ 0xc7,
 /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff,
 /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff,
};

const uint8_t b5b6_2[64] = {
 /*D.00:-2N*/ 0xd8, /*D.01:-2N*/ 0xe2, /*D.02:-2N*/ 0xd2, /*D.03:   */ 0x31,
 /*D.04:-2N*/ 0xca, /*D.05:   */ 0x29, /*D.06:   */ 0x19, /*D.07:  N*/ 0x87,
 /*D.08:-2N*/ 0xc6, /*D.09:   */ 0x25, /*D.10:   */ 0x15, /*D.11:   */ 0x34,
 /*D.12:   */ 0x0d, /*D.13:   */ 0x2c, /*D.14:   */ 0x1c, /*D.15:-2N*/ 0xe8,
 /*D.16:-2N*/ 0xe4, /*D.17:   */ 0x23, /*D.18:   */ 0x13, /*D.19:   */ 0x32,
 /*D.20:   */ 0x0b, /*D.21:   */ 0x2a, /*D.22:   */ 0x1a, /*D.23:-2N*/ 0xc5,
 /*D.24:-2N*/ 0xcc, /*D.25:   */ 0x26, /*D.26:   */ 0x16, /*D.27:-2N*/ 0xc9,
 /*D.28:   */ 0x0e, /*D.29:-2N*/ 0xd1, /*D.30:-2N*/ 0xe1, /*D.31:-2N*/ 0xd4,

 /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff,
 /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff,
 /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff,
 /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff,
 /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff,
 /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*K.23:-2N*/ 0xc5,
 /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*NOTM:   */ 0xff, /*K.27:-2N*/ 0xc9,
 /*K.28:-2N*/ 0xf0, /*K.29:-2N*/ 0xd1, /*K.30:-2N*/ 0xe1, /*NOTM:   */ 0xff,
};

const uint8_t b3b4_2[32] = {
 /*D.x.0 :-2N*/ 0xc4, /*D.x.1 :   */ 0x09, /*D.x.2 :   */ 0x85, /*D.x.3 :  N*/ 0x83,
 /*D.x.4 :-2N*/ 0xc2, /*D.x.5 :   */ 0x0a, /*D.x.6 :   */ 0x06, /*D.x.P7:-2N*/ 0xc1,
 /*D.x.A7:-2N*/ 0xc8, /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff,
 /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff,

 /*K.x.0 :-2N*/ 0xc4, /*K.x.1 :  N*/ 0x89, /*K.x.2 :  N*/ 0x8a, /*K.x.3 :  N*/ 0x83,
 /*K.x.4 :+2N*/ 0xc2, /*K.x.5 :  N*/ 0x8a, /*K.x.6 :  N*/ 0x86, /*K.x.7 :+2N*/ 0xc8,
 /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff,
 /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff, /*NOTM  :   */ 0xff,
};

#endif /* X8B10B_TABLES_H_ */
