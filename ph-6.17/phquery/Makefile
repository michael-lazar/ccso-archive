# Makefile for UIUC sendmail-related programs.
#
# Written by Paul Pomes, University of Illinois, Computing Services Office
# Copyright (c) 1991 by Paul Pomes and the University of Illinois Board
# of Trustees.  
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this software
#    must display the following acknowledgement:
#	This product includes software developed by the University of
#	Illinois, Urbana and its contributors.
# 4. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE TRUSTEES AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE TRUSTEES OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# Email:	Paul-Pomes@uiuc.edu	USMail:	Paul Pomes
# ICBM:	40 06 47 N / 88 13 35 W			University of Illinois - CSO
#						1304 West Springfield Avenue
#						Urbana, Illinois,  61801-2910
# @(#)$Id: Makefile,v 1.9 1993/06/24 04:51:46 paul Exp $

# .PREFIXES:	./RCS
.SUFFIXES:	.0 .8

# QI_HOST	The fully qualified domain name of the local qi/ph server
# QI_ALT	The fully qualified domain name of the backup qi/ph server
# DOMAIN	The domain name to append to Reply-To: aliases.
# SYSLOG	Syslog logging facility.
# SYSV		Installation under System 5.
# STANDALONE	If compiling without including sendmail.h

# Reasonable defaults.  A CPU type must usually be specified with ANSI C
# compilers in order to select the correct byte-ordering in <arpa/nameser.h>.
# IT'S LIKELY THAT THE -Dvax BELOW WILL BE WRONG FOR YOUR MACHINE.
CC=	cc -g
#CC=	gcc -Dvax -ansi -g
DEFS   = -DQI_HOST=\"ns.uiuc.edu\" \
	 -DQI_ALT=\"ns1.uiuc.edu\" \
	 -DDOMAIN=\"uiuc.edu\" \
	 -DSYSLOG=LOG_MAIL \
	 -DSTANDALONE
# -I/usr/include/local to pick up qiapi.h and qicode.h
CFLAGS = ${DEFS} -I../src -I/usr/include/local
FFLAGS = ${DEFS}
PFLAGS = ${DEFS}
LFLAGS = 
YFLAGS = 
LDFLAGS= 
LIBS   = -lqiapi		# From qi package on uxc.cso.uiuc.edu

DESTBIN= /bin
DESTETC= /usr/local/etc
DESTLIB= /usr/lib/mail
CAT8DIR = /usr/local/man/cat8
DOCDIR = /usr/local/doc

HDRS   = messages.h phquery.h
SRCS   = dconn.c malloc.c phquery.c
OBJS   = phquery.o # mktemp.o malloc.o
OTHERS = Makefile README phquery.8
VERS   = 

.8.0:
	/usr/man/manroff $*.8 > $*.0

all:	phquery 

# If your /lib/libc.a doesn't have mkstemp(), copy ../support/mktemp.c to this
# directory and insert into the SRCS/OBJS list above.  Also add mktemp.o below.
phquery:	${OBJS}
	$(CC) -o phquery ${CFLAGS} ${LDFLAGS} ${OBJS} $(LIBS)

install:	phquery phquery.0
	install -s -o daemon -m 4511 phquery {DESTLIB}/phquery
	install -c -m 444 phquery.0 ${CAT8DIR}/phquery.0

uninstall:	/tmp
	rm -f ${DESTLIB}/phquery ${CAT8DIR}/phquery.0

lint:	${HDRS} ${SRCS}
	lint -chpb ${DEFS} phquery.c

shar:	${HDRS} ${SRCS} ${OTHERS}
	shar ${HDRS} ${SRCS} ${OTHERS} > phquery.shar

tags:	${HDRS} ${SRCS}
	ctags ${HDRS} ${SRCS}

clean:
	@echo "Removing object and junk files."
	rm -f phquery *.o core a.out make.log lint.out \
		Makefile.bak phquery.shar

clobber:
	@echo "Removing read-only source files that have RCS parents."
	@echo "Error code 1 indicates last file in SRCS list was not removed."
	@echo "(Which is OK)"
	make clean
	-if [ `whoami` != root ]; then \
	   for i in ${HDRS} ${SRCS}; do \
	      if [ ! -w $$i ]; then \
	         ( if   [ -f $$i,v ];     then rm -f $$i; \
	           elif [ -f RCS/$$i,v ]; then rm -f $$i; \
	         fi ); \
	      else echo $$i "writeable, not removed"; \
	      fi; \
	   done; \
	else echo "Running \"make clobber\" as root will zap ALL SRCS,\
		RCS'ed or not (not done)."; \
	fi

compress:
	make clean
	@echo "Compressing source and RCS files."
	find . -size +2 \( -name \*.h -o -name \*.c -o -name \*.f \
		-o -name \*.p -o -name \*.l -o -name \*.y -o -name \*,v \) \
		-exec compress {} \;

uncompress:
	uncompressdir .

# RCS stuff

ci:		${HDRS} ${SRCS}
		-ci $?
		@touch ci

coall:
		co -l ${HDRS} ${SRCS}

update:
		ci -sDist -u -f${VERS} ${HDRS} ${SRCS}
		@touch ci

depend:
	mkdep ${DEFS} ${SRCS}

# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
