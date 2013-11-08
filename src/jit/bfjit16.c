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

/* modified by kairya@kariya.cc */

#include "hpgcc49.h"

//#define DBG

#define out(c) (*pout++ = (c))
#define in() (*pin++)

// fixed buffer size for now
#define BUF_SIZE (1024*16)

/* JIT code and emitting procedures */
typedef unsigned short op_t;
static op_t jit[BUF_SIZE];
#define emitAt(addr, op) (*(op_t*)(addr) = (op))
#define emitPc() (((int) jitPc) - sizeof(short))
op_t* jitPc = jit;
op_t* jitStart = jit;
int emit(op_t op) {
	*jitPc++ = op;
	return emitPc();
}
int emitWord(int word) {
	// assumes little endian
	int addr = emit((op_t)(word & 0xffff));
	emit((op_t)((word >> 16) & 0xffff));
	return addr;
}

/* for backpatch */
int stack[256]; // []'s nest depth must < 256 (maybe it's ok)
// TODO unmatched [] handling: eg. ++++][.  (for now maybe crashes)
int* stackp = &stack[0];
#define push(x) (*stackp++ = (x))
#define pop()   (*--stackp)
#define top(n)   (*(stackp-(n)))

//#define isOp(c) ((c)=='+'||(c)=='-'||(c)=='<'||(c)=='>'||(c)=='.'||(c)==','||(c)=='['||(c)==']')

struct loopInfo {
	int cur;
	int max;
	int min;
	int known;
};

#define MAX(a,b) ((a)>=(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

struct loopInfo* newInfo() {
	struct loopInfo* info = malloc(sizeof(struct loopInfo));
	info->cur = info->max = info->min = 0;
	info->known = -1;
	return info;
}

int main() {
	int pc, xc, prog_len;
	int x[BUF_SIZE];
	char buf_out[BUF_SIZE], *pout = buf_out;
	char buf_in[BUF_SIZE], *pin = buf_in;
	char* p;

	if (sat_stack_depth() > 1) {
		strcpy(buf_in, sat_stack_pop_string_alloc());
		strcat(buf_in, "\n");
	}
	p = sat_stack_pop_string_alloc();
	prog_len = strlen(p);

	sys_slowOff();
	
	for(xc = 0; xc < BUF_SIZE; xc++) {
		x[xc] = 0;
		buf_out[xc] = 0;
	}
	xc = 0;
	
	emit(0xb4f8); 									/* push {r3,v1,v2,v3,v4} */
	emit(0x46c0);									/* nop */
	// v1 := ${x}
	emit(0x4c00); 									// ldr v1, [pc]
	emit(0xe001); 									// b r#1
	emitWord((int)x);								// .word x 
	// v2 := ${pin}
	emit(0x4d00); 									// ldr v2, [pc]
	emit(0xe001); 									// b r#1
	emitWord((int)pin);								// .word pin 
	// v3 := ${pout}
	emit(0x4e00);									// ldr v3, [pc]
	emit(0xe001); 									// b r#1
	emitWord((int)pout);							// .word pout 

	
	int v4Opt = 0; /* =1 ifv4 and [v1] are the same value, =0 otherwise */
	push((int) newInfo());
	for (pc = 0; pc < prog_len; pc++) {
		// '+', '-'
		if (p[pc] == 43 || p[pc] == 45) {
			// sequence of +- can optimize
			int d = 0;
			while (1) {
				if (p[pc] == 43) d++;
				else if (p[pc] == 45) d--;
				else { //if (!isOp(p[pc]) || pc > prog_len) {
					pc--;
					break;
				}

				if (d == 255 || d == -255) break;
				pc++;
			}
			if (d == 0) {
				// empty
			} else if (d > 0) {
				if (!v4Opt) emit(0x7827);			/* ldrb v4, [v1] */
				emit(0x3700 | (d & 0xff));			/* add v4, v4, #{d} */
				emit(0x7027);						/* strb v4, [v1] */
			} else if (d < 0) {
				if (!v4Opt) emit(0x7827);			/* ldrb v4, [v1] */
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
				else { //if (!isOp(p[pc]) || pc > prog_len) {
					pc--;
					break;
				}
				if (d == 255 || d == -255) break;
				pc++;
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
			struct loopInfo* info = (struct loopInfo*) top(1);
			info->cur += d;
			info->max = MAX(info->max, info->cur);
			info->min = MIN(info->min, info->cur);
		}
		// '['
		else if (p[pc] == 91) {
			if (emitPc() % 4 != 0) {
				if (!v4Opt) { 
					emit(0x7827);					// ldr v4, [v1]
					emit(0x4b01);					/* ldr r3, [pc+#4] */
					emit(0x4718);					/* bx r3 */
					emit(0x46c0);					/* nop */
					push(emitWord(0));				/* .word */
				} else {
					emit(0x4b00);					/* ldr r3, [pc+#0] */
					emit(0x4718);					/* bx r3 */
					push(emitWord(0));				/* .word */
				}
			} else {
				if (!v4Opt) { 
					emit(0x7827);					// ldr v4, [v1]
					emit(0x4b00);					/* ldr r3, [pc+#0] */
					emit(0x4718);					/* bx r3 */
					push(emitWord(0));				/* .word */
				} else {
					emit(0x4b01);					/* ldr r3, [pc+#4] */
					emit(0x4718);					/* bx r3 */
					emit(0x46c0);					/* nop */
					push(emitWord(0));				/* .word */
				}
			}
													/* l: */
			push((int) newInfo());
			v4Opt = 1;
		}
		// ']'
		else if (p[pc] == 93) {
			struct loopInfo* info = (struct loopInfo*) pop();
			if (info->cur == 0 && info->known == -1) info->known = 1;
			else {
				info->known = 0;
				((struct loopInfo*) top(1+1))->known = 0;
			}
			printf("[%d]", info->known ? info->max - info->min : -1);
			
			int ret = pop() + 4;
			int ret2 = emitPc() + 2;
			int offset = ret - (emitPc() + (v4Opt ? 8 : 10)) ;
			if (-offset <= 0xff) {
				if (!v4Opt) {
						emit(0x7827);					// ldr v4, [v1]
						offset -= 2;
				}
				emit(0x433f);							// orrs v4, v4
				emit(0xd100 | ((offset >> 1) & 0xff));	// bne #{offset}	
			} else {
				if (emitPc() % 4 != 0) {
					if (!v4Opt) {
						emit(0x7827);					// ldr v4, [v1]
						emit(0x433f);					// orrs v4, v4
						emit(0x4b01);					// ldr r3, [pc + #4]
						emit(0xd003);					// beq pc + #6 
						emit(0x4718);					// bx r3 
						emit(0x46c0);					/* nop */
						emitWord(ret | 1);				// .word (ret|1)
					} else {
						emit(0x433f);					// orrs v4, v4
						emit(0x4b01);					// ldr r3, [pc + #4]
						emit(0xd002);					// beq pc + #4 
						emit(0x4718);					// bx r3 
						emitWord(ret | 1);				// .word (ret|1)
					}
				} else {
					if (!v4Opt) {
						emit(0x7827);					// ldr v4, [v1]
						emit(0x433f);					// orrs v4, v4
						emit(0x4b01);					// ldr r3, [pc + #4]
						emit(0xd002);					// beq pc + #4 
						emit(0x4718);					// bx r3 
						emitWord(ret | 1);				// .word (ret|1)
					} else {
						emit(0x433f);					// orrs v4, v4
						emit(0x4b01);					// ldr r3, [pci + #4]
						emit(0xd003);					// beq pc + #6 
						emit(0x4718);					// bx r3 
						emit(0x46c0);					/* nop */
						emitWord(ret | 1);				// .word (ret|1)
					}
				}
			}
			emitAt(ret - 4, (ret2|1) & 0xffff);
			emitAt(ret - 2, ((ret2|1) >> 16) & 0xffff);

			v4Opt = 1;
		}
	}
	
	// put('\0') as output string terminator
	emit(0x2700); 								/* mov v4, #0 */
	emit(0x7037); 								/* strb v4, [v3] */
	emit(0x3601);								/* add v3, v3, #1 */
	
	emit(0xbcf8);								/* pop {r3,v1,v2,v3,v4} */
	emit(0x4770);								/* bx lr */

#ifdef DBG
	int i;
	char *bb = malloc(8*1024*2), bbb[16];
	strcpy(bb, "");
	for (i = 0; i < BUF_SIZE; ++i) { 
		itoa(jit[i], bbb, 16);
		strcat(bb, bbb);
		strcat(bb, "@");
		itoa(&jit[i], bbb, 16);
		strcat(bb, bbb);
		strcat(bb, "\n");
		if (jit[i] == 0) break;
	}
	sat_stack_push_string(bb);

	WAIT_CANCEL;
#endif
	
	beep();
	typedef void (*funcp)();
	(*(funcp)((unsigned)jitStart|1))();			// 1 means the callee is thumb code
	beep();
	
	sys_slowOn();
	
	sat_stack_push_string(buf_out);
	
	return 0;
}

