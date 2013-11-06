/*
 * BFI
 * Copyright (C) 2003 Thomas Cort
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Program Name:  BFI
 * Version:       1.1
 * Date:          2003-04-29
 * Description:   Interpreter for the Brainfuck Programming Language
 * License:       GPL
 * Web page:      http://www.brainfuck.ca
 * Download:      http://www.brainfuck.ca/BFI.c
 * Source Info:   http://www.brainfuck.ca/downloads.html
 * Latest Ver:    http://www.brainfuck.ca/downloads.html
 * Documentation: None
 * Help:          tom@brainfuck.ca
 * Developement:  tom@brainfuck.ca
 * Bugs:          tom@brainfuck.ca
 * Maintainer:    Thomas Cort <tom@brainfuck.ca>
 * Developer:     Thomas Cort <tom@brainfuck.ca>
 * Interfaces:    Command Line
 * Source Lang:   C
 * Build Prereq:  None
 * Related Progs: BFIwDebug
 * Category:      Software Development > Programming languages
 */

/* modified by kariya@kariya.cc */

#include "hpgcc49.h"

#define out(c) (*pout++ = (c))
#define in() (*pin++)


int main() {
	int pc, xc, prog_len, l = 0;
	int x[32768];
	char buf_out[1024*32], *pout = &buf_out;
	char buf_in[1024*32], *pin = &buf_in;
	char* p;

	if (sat_stack_depth() > 1) {
	} else {
		strcpy(buf_in, sat_stack_pop_string_alloc());
		strcat(buf_in, "\n");
	}
	p = sat_stack_pop_string_alloc();
	prog_len = strlen(p);

	sys_slowOff();
	
	for(xc = 0; xc < 32768; xc++)
		x[xc] = 0;

	xc = 0;

	for(pc = 0; pc < prog_len; pc++) {
//		if (keyb_isAnyKeyPressed()) {
//		break;
//	}
	
		// '+'
		if (p[pc] == 43) x[xc]++;

		// '-'
		else if (p[pc] == 45) x[xc]--;

		// '.'
		else if (p[pc] == 46) out(x[xc]);

		// ','
		else if (p[pc] == 44) x[xc] = in();

		// '>'
		else if (p[pc] == 62) xc++;

		// '<'
		else if (p[pc] == 60) xc--;

		// '['
		else if (p[pc] == 91) {
			if (x[xc] == 0) {
				pc++;
				while (l > 0 || p[pc] != 93) {
					if (p[pc] == 91) l++;
					if (p[pc] == 93) l--;
					pc++;
				}
			}
		}

		// ']'
		else if (p[pc] == 93) {
			pc--;
			while (l > 0 || p[pc] != 91) {
				if (p[pc] == 93) l++;
				if (p[pc] == 91) l--;
				pc--;
			}
			pc--;
		}
	}
	
	/*out(10);*/
	
	out('\0');
	
	sys_slowOn();

	sat_stack_push_string(buf_out);
	
	return 0;
}

