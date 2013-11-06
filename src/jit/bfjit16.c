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

#include "hpgcc49.h"

#define out(c) (*pout++ = (c))
#define in() (*pin++)

// fixed size for now
#define BUF_SIZE (1024*16)

typedef unsigned short op_t;
static op_t jit[BUF_SIZE];
op_t* jitPc = jit;
op_t* jitStart = jit;

int emit(op_t op) {
	*jitPc++ = op;
	return ((int) jitPc) - sizeof(short);
}
void emitAt(int addr, op_t op) {
	*(short*)addr = op;
}
int emitPc() {
	return ((int) jitPc) - sizeof(short);
}

/* for backpatch */
int stack[256]; // []'s nest depth must < 256 (maybe it's ok)
// TODO unmatched [] handling: eg. ++++][.  (for now maybe crashes)
int* stackp = &stack[0];
#define push(x) (*stackp++ = (x))
#define pop()   (*--stackp)

#define isOp(c) ((c)=='+'||(c)=='-'||(c)=='<'||(c)=='>'||(c)=='.'||(c)==','||(c)=='['||(c)==']')


int main() {
	int pc, xc, prog_len;
	int x[BUF_SIZE], p[BUF_SIZE];
	char buf_out[BUF_SIZE], *pout = &buf_out[0];
	char buf_in[BUF_SIZE], *pin = &buf_in[0];

	if (sat_stack_depth() > 1) {
		strcpy(buf_in, sat_stack_pop_string_alloc());
		strcat(buf_in, "\n");
	}
	strcpy(p, sat_stack_pop_string_alloc());
	strcat(p, ">+"); // dummy
	prog_len = strlen(p);

	sys_slowOff();
	
	pc = 0;

	for(xc = 0; xc < BUF_SIZE; xc++)
		x[xc] = 0;

	xc = 0;
	

	emit(0xb4f8); 									/* push {r3,v1,v2,v3,v4} */
	// v1 := ${x}
	emit(0x2400 | ((int)x & 0xff));					/* movs v1, ${x & 0x000000ff} */
	emit(0x2700 | (((int)x >> 8) & 0xff));		 	/* movs v4, ${(x>>8) & 0x000000ff} */
	emit(0x023f);									/* lsls v4, v4, #8 */
	emit(0x433c);									/* orrs v1, v4 */
	emit(0x2700 | (((int)x >> 16) & 0xff));		 	/* movs v4, ${(x>>16) & 0x000000ff} */
	emit(0x043f);									/* lsls v4, v4, #16 */
	emit(0x433c);									/* orrs v1, v4 */
	emit(0x2700 | (((int)x >> 24) & 0xff));		 	/* movs v4, ${(x>>24) & 0x000000ff} */
	emit(0x063f);									/* lsls v4, v4, #24 */
	emit(0x433c);									/* orrs v1, v4 */	
	// v2 := ${pin}
	emit(0x2500 | ((int)pin & 0xff));				/* movs v2, ${pin & 0x000000ff} */
	emit(0x2700 | (((int)pin >> 8) & 0xff));		/* movs v4, ${(pin>>8) & 0x000000ff} */
	emit(0x023f);									/* lsls v4, v4, #8 */
	emit(0x433d);									/* orrs v2, v4 */
	emit(0x2700 | (((int)pin >> 16) & 0xff));		/* movs v4, ${(pin>>16) & 0x000000ff} */
	emit(0x043f);									/* lsls v4, v4, #16 */
	emit(0x433d);									/* orrs v2, v4 */
	emit(0x2700 | (((int)pin >> 24) & 0xff));		/* movs v4, ${(pin>>24) & 0x000000ff} */
	emit(0x063f);									/* lsls v4, v4, #24 */
	emit(0x433d);									/* orrs v2, v4 */	
	// v3 := ${pout}
	emit(0x2600 | ((int)pout & 0xff));				/* movs v3, ${pout & 0xff} */
	emit(0x2700 | (((int)pout >> 8) & 0xff));		/* movs v4, ${(pout>>8) & 0xff} */
	emit(0x023f);									/* lsls v4, v4, #8 */
	emit(0x433e);									/* orrs v3, v4 */
	emit(0x2700 | (((int)pout >> 16) & 0xff));		/* movs v4, ${(pout>>16) & 0xff} */
	emit(0x043f);									/* lsls v4, v4, #16 */
	emit(0x433e);									/* orrs v3, v4 */
	emit(0x2700 | (((int)pout >> 24) & 0xff));		/* movs v4, ${(pout>>24) & 0xff} */
	emit(0x063f);									/* lsls v4, v4, #24 */
	emit(0x433e);									/* orrs v3, v4 */	
	

//int Q = jitPc;
	
	int v4Opt = 0; /* 1=v4 is [v1] 0=otherwise */
	for(pc = 0; pc < prog_len; pc++) {
		// '+', '-'
		if (p[pc] == 43 || p[pc] == 45) {
			// sequence of +- can optimize
			int d = 0;
			while (1) {
				if (p[pc] == 43) d++;
				else if (p[pc] == 45) d--;
				else if (!isOp(p[pc]) && pc < prog_len) {
					pc--;
					break;
				}

				pc++;
				if (d == 255 || d == -255) break;
			}
			if (d == 0) {
				// empty
			} else if (d > 0) {
				emit(0x7827);						/* ldrb v4, [v1] */
				emit(0x3700 | (d & 0xff));			/* add v4, v4, #{d} */
				emit(0x7027);						/* strb v4, [v1] */
			} else if (d < 0) {
				emit(0x7827);						/* ldrb v4, [v1] */
				emit(0x3f00 | ((-d) & 0xff));		/* sub v4, v4, #{d} */
				emit(0x7027);						/* strb v4, [v1] */
			}
			
			v4Opt = 1;
		}
		// '.'
		else if (p[pc] == 46) {
			if (!v4Opt) emit(0x7827);				/* ldrb v4, [v1] */
			emit(0x7037);							/* strb v4, [v3] */
			emit(0x3601);							/* add v3, v3, #1 */
		
			v4Opt = 1;
		}
		// ','
		else if (p[pc] == 44) {
			emit(0x782f);							/* ldrb v4, [v2] */
			emit(0x3501);							/* add v2, v2, #1 */
			emit(0x7027);							/* strb v4, [v1] */

			v4Opt = 1;
		}
		// '>'
		// '<'
		else if (p[pc] == 62 || p[pc] == 60) {
			// sequence of >< can optimize
			int d = 0;
			while (1) {
				if (p[pc] == 62) d++;
				else if (p[pc] == 60) d--;
				else if (!isOp(p[pc]) && pc < prog_len) {
					pc--;
					break;
				}
				pc++;
				
				if (d == 255 || d == -255) break;
			}
			if (d == 0) {
				// empty
			} else if (d > 0) {
				emit(0x3400 | (d & 0xff));				/* add v1, v1, #{d} */
				v4Opt = 0;
			} else if (d < 0) {
				emit(0x3c00 | ((-d) & 0xff));			/* sub v1, v1, #{d} */
				v4Opt = 0;
			}
		}
		// '['
		else if (p[pc] == 91) {
			if (emitPc() % 4 == 0) {
				if (!v4Opt) {
					emit(0x7827);					// ldr v4, [v1]
					push(emit(0x433f));				/* orrs v4, v4 */
					emit(0xd104);					/* bne 8 (l:) */
					emit(0x4b01);					/* ldr r3, [pc+#4] */
					emit(0x4738);					/* bx r3 */
					emit(0x46c0);					/* nop */
					push(emit(0x46c0));				/* nop */
					emit(0x46c0);					/* nop */
				} else {
					push(emit(0x433f));				/* orrs v4, v4 */
					emit(0xd103);					/* bne 6 (l:) */
					emit(0x4b00);					/* ldr r3, [pc+0] */
					emit(0x4738);					/* bx r3 */
					push(emit(0x46c0));				/* nop */
					emit(0x46c0);					/* nop */
				}
			} else {
				if (!v4Opt) {
					emit(0x7827);					// ldr v4, [v1]
					push(emit(0x433f));				/* orrs v4, v4 */
					emit(0xd103);					/* bne 6 (l:) */
					emit(0x4b00);					/* ldr r3, [pc+#0] */
					emit(0x4738);					/* bx r3 */
					push(emit(0x46c0));				/* nop */
					emit(0x46c0);					/* nop */
				} else {
					push(emit(0x433f));				/* orrs v4, v4 */
					emit(0xd103);					/* bne 6 (l:) */
					emit(0x4b01);					/* ldr r3, [pc+#4] */
					emit(0x4738);					/* bx r3 */
					emit(0x46c0);					/* nop */
					push(emit(0x46c0));				/* nop */
					emit(0x46c0);					/* nop */
				}
			}
													/* l: */
			v4Opt = 1;
		}
		// ']'
		else if (p[pc] == 93) {
			int addr = pop();
			int ret = pop();
			if (emitPc() % 4 == 0) {
				if (!v4Opt) {
					emit(0x7827);					// ldr v4, [v1]
					emit(0x4b01);					// ldr r3, [pc + #4]
					emit(0x4718);					// bx r3 
					emit(0x46c0);					/* nop */
					addr = emit((ret | 1) & 0xffff);
					emit(((ret >> 16) | 1) & 0xffff);
				} else {
					emit(0x4b00);					// ldr r3, [pc]
					emit(0x4718);					// bx r3 
					addr = emit((ret | 1) & 0xffff);
					emit(((ret >> 16) | 1) & 0xffff);
				}
			} else {
				if (!v4Opt) {
					emit(0x7827);					// ldr v4, [v1]
					emit(0x4b00);					// ldr r3, [pc + #0]
					emit(0x4718);					// bx r3 
					addr = emit((ret | 1) & 0xffff);
					emit(((ret >> 16) | 1) & 0xffff);
				} else {
					emit(0x4b01);					// ldr r3, [pc+#4]
					emit(0x4718);					// bx r3 
					emit(0x46c0);					/* nop */
					addr = emit((ret | 1) & 0xffff);
					emit(((ret >> 16) | 1) & 0xffff);
				}
			}
			int ret2 = (emitPc() + 2) | 1;
			emitAt(addr + 0, ret2 & 0xffff);
			emitAt(addr + 2, (ret2 >> 16) & 0xffff);

			v4Opt = 1;
		}
	}
	
	emit(0x2700); 								/* mov v4, #0 */
	emit(0x7037); 								/* strb v4, [v3] */
	emit(0x3601);								/* add v3, v3, #1 */
	
	emit(0xbcf8);								/* pop {r3,v1,v2,v3,v4} */
	emit(0x4770);								/* bx lr */

/*	
	short* i;
	for (i = Q; i < Q + 40; ++i) printf("[%x]%x:", i, *(short*)i & 0xffff);
	WAIT_CANCEL;
*/	
	beep();
	typedef void (*funcp)();
	(*(funcp)((unsigned)jitStart|1))();			// 1 means the callee is thumb code
	beep();
	
	sys_slowOn();
	
	sat_stack_push_string(buf_out);
	
	return 0;
}
