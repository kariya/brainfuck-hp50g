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

//#define DBG
//#define DUMP 

#ifdef DUMP
#include <stdio.h>
#include <stdlib.h>
#else
#include "hpgcc49.h"
#endif

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
	int simple;
};

#define MAX(a,b) ((a)>=(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

struct loopInfo* stackLoop[256];
struct loopInfo** stackpLoop = &stackLoop[0];
#define pushLoop(x) (*stackpLoop++ = (x))
#define popLoop()   (*--stackpLoop)
#define topLoop(n)   (*(stackpLoop-(n)))

struct loopInfo* newInfo() {
	struct loopInfo* info = (struct loopInfo*) malloc(sizeof(struct loopInfo));;
	info->cur = info->max = info->min = 0;
	info->simple = -1;
	return info;
}
struct loopInfo *stackLoop[256];
struct loopInfo** stackpLoop = stackLoop;
#define pushLoop(x) (*stackpLoop++ = (x))
#define popLoop()   (*--stackpLoop)
#define topLoop(n)   (*(stackpLoop-(n)))




int main() {
	int pc, xc, prog_len;
	int x[BUF_SIZE];
	char buf_out[BUF_SIZE], *pout = buf_out;
	char buf_in[BUF_SIZE], *pin = buf_in;
	char* p;

#if DUMP
	p = malloc(1024*8);
	int c;
	char* pp = p;
	while ((c = getchar()) != EOF) *pp++ = c;
#else
	if (sat_stack_depth() > 1) {
		strcpy(buf_in, sat_stack_pop_string_alloc());
		strcat(buf_in, "\n");
	}
	p = sat_stack_pop_string_alloc();
#endif
	prog_len = strlen(p);

#ifndef DUMP
	sys_slowOff();
#endif
	
	for(xc = 0; xc < BUF_SIZE; xc++) {
		x[xc] = 0;
		buf_out[xc] = 0;
	}
	xc = 0;

#if 0
	register allocation:
	r0: general 
	r4: general2 
	r5: x
	r6: pin
	r7: pout
#endif
	
	emit([[push {r0,r4,r5,r6,r7}]]);
	emit([[nop]]);
	// r5 := ${x}
	emit([[ldr r5, [pc]]]);
	emit([[b 10]]);
	emitWord((int) x);
	// r6 := ${pin}
	emit([[ldr r6, [pc]]]);
	emit([[b 10]]);
	emitWord((int) pin);
	// r7 := ${pout}
	emit([[ldr r7, [pc]]]);
	emit([[b 10]]);
	emitWord((int) pout);

	int v4Opt = 0; /* =1 ifv4 and [v1] are the same value, =0 otherwise */
	pushLoop(newInfo());
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
				if (!v4Opt) emit([[ldrb r0, [r5]]]);
				emit([[add r0, r0, #0]] | (d & 0xff));
				emit([[strb r0, [r5]]]);
			} else if (d < 0) {
				if (!v4Opt) emit([[ldrb r0, [r5]]]);
				emit([[sub r0, r0, #0]] | ((-d) & 0xff));
				emit([[strb r0, [r5]]]);
			}
			
			v4Opt = 1;
		}
		// '.'
		else if (p[pc] == 46) {
			if (!v4Opt) emit([[ldrb r0, [r5]]]);
			emit([[strb r0, [r7]]]);
			emit([[add r7, r7, #1]]);
		
			v4Opt = 1;
		}
		// ','
		else if (p[pc] == 44) {
			emit([[ldrb r0, [r6]]]);
			emit([[add r6, r6, #1]]);
			emit([[strb r0, [r5]]]);

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
				emit([[add r5, r5, #0]] | (d & 0xff));
				v4Opt = 0;
			} else if (d < 0) {
				emit([[sub r5, r5, #0]] | ((-d) & 0xff));
				v4Opt = 0;
			}
			struct loopInfo* info = topLoop(1);
			info->cur += d;
			info->max = MAX(info->max, info->cur);
			info->min = MIN(info->min, info->cur);
		}
		// '['
		else if (p[pc] == 91) {
			if (emitPc() % 4 != 0) {
				if (!v4Opt) { 
					emit([[ldr r0, [r5]]]);
					emit([[ldr r4, [pc, #4]]]);
					emit([[bx r4]]);
					emit([[nop]]);
					push(emitWord(0));
				} else {
					emit([[ldr r4, [pc, #0]]]);
					emit([[bx r4]]);
					push(emitWord(0));
				}
			} else {
				if (!v4Opt) { 
					emit([[ldr r0, [r5]]]);
					emit([[ldr r4, [pc, #0]]]);
					emit([[bx r4]]);
					push(emitWord(0));
				} else {
					emit([[ldr r4, [pc, #4]]]);
					emit([[bx r4]]);
					emit([[nop]]);
					push(emitWord(0));
				}
			}
													/* l: */
			pushLoop(newInfo());
			v4Opt = 1;
		}
		// ']'
		else if (p[pc] == 93) {
			struct loopInfo* info = popLoop();
			if (info->cur == 0 && info->simple == -1) info->simple = 1;
			else {
				info->simple = 0;
				topLoop(1)->simple = 0;
			}
#ifdef DBG
			printf("[%d]", info->simple ? info->max - info->min : -1);
#endif
			
			int ret = pop() + 4;
			int ret2 = emitPc() + 2;
			int offset = ret - (emitPc() + (v4Opt ? 8 : 10)) ;
			if (-offset <= 0xff) {
				if (!v4Opt) {
						emit([[ldr r0, [r5]]]);
						offset -= 2;
				}
				emit([[orr r0, r0]]);
				emit([[bne 8]] | ((offset >> 1) & 0xff));
			} else {
				if (emitPc() % 4 != 0) {
					if (!v4Opt) {
						emit([[ldr r0, [r5]]]);
						emit([[orr r0, r0]]);
						emit([[ldr r4, [pc, #4]]]);
						emit([[beq 20]]);
						emit([[bx r4]]);
						emit([[nop]]);
						emitWord(ret | 1);
					} else {
						emit([[orr r0, r0]]);
						emit([[ldr r4, [pc, #4]]]);
						emit([[beq 16]]);
						emit([[bx r4]]);
						emitWord(ret | 1);
					}
				} else {
					if (!v4Opt) {
						emit([[ldr r0, [r5]]]);
						emit([[orr r0, r0]]);
						emit([[ldr r4, [pc, #4]]]);
						emit([[beq 16]]);
						emit([[bx r4]]);
						emitWord(ret | 1);
					} else {
						emit([[orr r0, r0]]);
						emit([[ldr r4, [pc, #4]]]);
						emit([[beq 20]]);
						emit([[bx r4]]);
						emit([[nop]]);
						emitWord(ret | 1);
					}
				}
			}
			emitAt(ret - 4, (ret2|1) & 0xffff);
			emitAt(ret - 2, ((ret2|1) >> 16) & 0xffff);

			v4Opt = 1;
		}
	}
	
	// put('\0') as output string terminator
	emit([[mov r0, #0]]);
	emit([[strb r0, [r7]]]);
	emit([[add r7, r7, #1]]);
	
	emit([[pop {r0,r4,r5,r6,r7}]]);
	emit([[bx lr]]);

#ifdef DUMP
	int i;
	printf("void f() {\n");
	printf("\tasm(\"" ".org 0x%x\");\n", jit);
	for (i = 0; i < BUF_SIZE; ++i) { 
		printf("\tasm(\".short 0x%x\");\n", jit[i]);
	}
	printf("}\n");
#else
	beep();
	typedef void (*funcp)();
	(*(funcp)((unsigned)jitStart|1))();			// 1 means the callee is thumb code
	beep();
	
	sys_slowOn();
	
	sat_stack_push_string(buf_out);
#endif	

	return 0;
}

