#
# Makefile for memtester by Charles Cazabon.
#
# Copyright (C) 1999 Simon Kirby.
# Copyright (C) 1999-2005 Charles Cazabon.
# Licensed under the GNU General Public License version 2.  See the file
# COPYING for details.
#

CC			= $(shell head -n 1 conf-cc)
LD			= $(shell head -n 1 conf-ld)

SOURCES		= memtester.c tests.c
OBJECTS		= $(SOURCES:.c=.o)
HEADERS		= memtester.h
TARGETS     = *.o compile load auto-ccld.sh find-systype make-compile make-load systype extra-libs

#
# Targets
#
all: memtester


auto-ccld.sh: \
conf-cc conf-ld warn-auto.sh
	( cat warn-auto.sh; \
	echo CC=\'`head -1 conf-cc`\'; \
	echo LD=\'`head -1 conf-ld`\' \
	) > auto-ccld.sh

compile: \
make-compile warn-auto.sh systype
	( cat warn-auto.sh; ./make-compile "`cat systype`" ) > \
	compile
	chmod 755 compile

find-systype: \
find-systype.sh auto-ccld.sh
	cat auto-ccld.sh find-systype.sh > find-systype
	chmod 755 find-systype

make-compile: \
make-compile.sh auto-ccld.sh
	cat auto-ccld.sh make-compile.sh > make-compile
	chmod 755 make-compile

make-load: \
make-load.sh auto-ccld.sh
	cat auto-ccld.sh make-load.sh > make-load
	chmod 755 make-load

systype: \
find-systype trycpp.c
	./find-systype > systype

extra-libs: \
extra-libs.sh systype
	./extra-libs.sh "`cat systype`" >extra-libs

load: \
make-load warn-auto.sh systype
	( cat warn-auto.sh; ./make-load "`cat systype`" ) > load
	chmod 755 load

clean:
	rm -f memtester $(TARGETS) $(OBJECTS) core

memtester: \
$(OBJECTS) memtester.c tests.h tests.c tests.h conf-cc Makefile load extra-libs
	./load memtester tests.o `cat extra-libs`

memtester.o: memtester.c tests.h conf-cc Makefile compile
	./compile memtester.c

tests.o: tests.c tests.h conf-cc Makefile compile
	./compile tests.c
