/*
 * memtester version 4.0.0
 *
 * Very simple but very effective user-space memory tester.
 * Originally by Simon Kirby <sim@stormix.com> <sim@neato.org>
 * Version 2 by Charles Cazabon <memtest@discworld.dyndns.org>
 * Version 3 not publicly released.
 * Version 4 rewrite:
 * Copyright (C) 2004 Charles Cazabon <memtest@discworld.dyndns.org>
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 */

#define __version__ "4.0.0"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#include "types.h"
#include "sizes.h"
#include "tests.h"

struct test tests[] = {
	{ "Random Value", test_random_value },
	{ "Compare XOR", test_xor_comparison },
	{ "Compare SUB", test_sub_comparison },
	{ "Compare MUL", test_mul_comparison },
	{ "Compare DIV",test_div_comparison },
	{ "Compare OR", test_or_comparison },
	{ "Compare AND", test_and_comparison },
	{ "Sequential Increment", test_seqinc_comparison },
	{ "Solid Bits", test_solidbits_comparison },
	{ "Block Sequential", test_blockseq_comparison },
	{ "Checkerboard", test_checkerboard_comparison },
	{ "Bit Spread", test_bitspread_comparison },
	{ "Bit Flip", test_bitflip_comparison },
	{ "Walking Ones", test_walkbits1_comparison },
	{ "Walking Zeroes", test_walkbits0_comparison },
	{ NULL, NULL }
};

int main(int argc, char **argv) {
    long pagesize;
    ul pagesizemask, loops, loop, i;
    size_t wantmb, wantbytes, bufsize, halflen, count;
    void volatile *buf, *aligned;
    ulv *bufa, *bufb;
    int do_mlock = 1;

    printf("memtester version " __version__ " (%d-bit)\n", UL_LEN);
    printf("Copyright (C) 2004 Charles Cazabon.\n");
    printf("Licensed under the GNU General Public License version 2 (only).\n");
    printf("\n");
    if (sysconf(_SC_VERSION) < 199009L) {
        fprintf(stderr, "A POSIX system is required.  Don't be surprised if "
            "this craps out.\n");
        fprintf(stderr, "_SC_VERSION is %lu\n", sysconf(_SC_VERSION));
    }

#ifdef _SC_PAGE_SIZE
    pagesize = sysconf(_SC_PAGE_SIZE);
    if (pagesize == -1) {
        perror("get page size failed");
        exit(1);
    }
    printf("pagesize is %ld\n", pagesize);
#else
    pagesize = 8192;
    printf("sysconf(_SC_PAGE_SIZE) not supported; using pagesize of %ld\n", pagesize);
#endif
    pagesizemask = ~(pagesize - 1);
    printf("pagesizemask is 0x%lx\n", pagesizemask);

    if (argc < 2) {
        fprintf(stderr, "need memory argument, in MB\n");
        exit(1);
    }
    wantmb = (size_t) strtoul(argv[1], NULL, 0);
    wantbytes = (size_t) (wantmb << 20);
    if (wantbytes < pagesize) {
        fprintf(stderr, "bytes < pagesize -- memory argument too large?\n");
        exit(1);
    }

    if (argc < 3) {
        loops = 0;
    } else {
        loops = strtoul(argv[2], NULL, 0);
    }
    
    printf("want %lluMB (%llu bytes)\n", (ull) wantmb, (ull) wantbytes);
    buf = NULL;
    while (!buf && wantbytes) {
        buf = (void volatile *) malloc(wantbytes);
        if (!buf) wantbytes -= pagesize;
    }

    printf("got  %lluMB (%llu bytes)\n", (ull) wantbytes >> 20, 
        wantbytes);
    bufsize = wantbytes;

    if ((size_t) buf % pagesize) {
        printf("aligning to page\n");
        aligned = (void volatile *) ((size_t) buf & pagesizemask);
        bufsize -= ((size_t) aligned - (size_t) buf);
    } else {
        aligned = buf;
    }
    
    /* Try memlock */
    if (mlock((void *) aligned, bufsize) < 0) {
        do_mlock = 0;
        switch(errno) {
            case ENOMEM:
                printf("mlock() failed: too many pages\n");
                break;
            case EPERM:
                printf("mlock() failed: insufficient permission\n");
                break;
            default:
                printf("mlock() failed: unknown reason\n");
        }
    }
    if (!do_mlock) fprintf(stderr, "Continuing with unlocked memory; testing "
        "will be slower and less reliable.\n");

    halflen = bufsize / 2;
    count = halflen / sizeof(ul);
    bufa = (ulv *) aligned;
    bufb = (ulv *) ((size_t) aligned + halflen);

    for(loop=1; ((!loops) || loop <= loops); loop++) {
        printf("Loop %lu", loop);
        if (loops) {
            printf("/%lu", loops);
        }
        printf(":\n");
    	printf("  %-20s: ", "Stuck Address");
    	fflush(stdout);
    	if (!test_stuck_address(aligned, bufsize / sizeof(ul))) printf("ok\n");
    	for (i=0;;i++) {
    		if (!tests[i].name) break;
		    printf("  %-20s: ", tests[i].name);
            if (!tests[i].fp(bufa, bufb, count)) printf("ok\n");
			fflush(stdout);
    	}
    	printf("\n");
    	fflush(stdout);
    }
    if (do_mlock) munlock((void *) aligned, bufsize);
	printf("Done.\n");
	fflush(stdout);
    exit(0);
}
