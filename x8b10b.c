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

#include "x8b10b.h"
#include "x8b10b_tables.h"

#include <stdio.h>

#define BIT(N)  (1<<(N))
#define MASK(N) ((1<<(N))-1)
#define RD_BIT  BIT(6)

#define _TRACE

#ifdef _TRACE
static char buf[20];
static char* bits(uint16_t symbol, int n) {
	for (int i = 0; i < n; ++i)
		 buf[i] = ((symbol & BIT(n-i-1)) != 0) ? '1' : '0';
	buf[n] = '\0';
	return buf;
}
#endif

int x8b10b_enc(uint8_t u8b, uint16_t *p10b, int ctrl, int *rd)
{
	uint16_t abcdeifghj;
	uint8_t  abcdei, abcdei_rd, fghj, fghj_rd, x, y;

	x = (u8b        & MASK(5)) | ((!!ctrl) << 5); /* 5 bit */
	y = ((u8b >> 5) & MASK(3)) | ((!!ctrl) << 4); /* 3 bit */

	abcdei = ((*rd) < 0) ? b5b6_1[x] : b5b6_2[x];
	if (abcdei == 0xff) return 0;
	abcdei_rd = ((abcdei & RD_BIT) != 0x00);
	abcdei = abcdei & MASK(6);
	if (abcdei_rd) {
		if ((*rd) < 0)
			(*rd) += 2;
		else
			(*rd) -= 2;
	}

	if (y == 7 && !ctrl) {
		if ((*rd) < 0) {
			if ((x == 17) || (x == 18) || (x == 20))
					y = 8;
		} else {
			if ((x == 11) || (x == 13) || (x == 14))
					y = 8;
		}
	}

	fghj = ((*rd) < 0) ? b3b4_1[y] : b3b4_2[y];
	if (fghj == 0xff) return 0;
	fghj_rd = ((fghj & RD_BIT) != 0x00);
	fghj = fghj & MASK(4);
	if (fghj_rd) {
		if ((*rd) < 0)
			(*rd) += 2;
		else
			(*rd) -= 2;
	}

	abcdeifghj = (abcdei << 4) | fghj;
#ifdef _TRACE
	printf("_8b_2_10b: "
		   "abcdei=%s / ",     bits(abcdei,      6));
	printf("fghj=%s / ",       bits(fghj,        4));
	printf("abcdeifghj=%s / ", bits(abcdeifghj, 10));
	printf("rd=%s\n",          ((*rd)<0)?"-1":"+1" );
	if (((*rd) != -1) && ((*rd) != +1))
		printf("Error: Invalid RD=%d\n", (*rd));
#endif
	(*p10b) = abcdeifghj;
	return 1;
}
