# This Makefile is automatically generated.  Do NOT edit it here.
# Copyright (c) 1985 Corporation for Research and Educational Networking
# Copyright (c) 1988 University of Illinois Board of Trustees, Steven
#		Dorner, and Paul Pomes
# All rights reserved.
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
#	This product includes software developed by the Corporation for
#	Research and Educational Networking (CREN), the University of
#	Illinois at Urbana, and their contributors.
# 4. Neither the name of CREN, the University nor the names of their
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
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
# @(#)$Id: Makefile.templ,v 1.2 1994/03/12 01:04:25 paul Exp $

######################################################################
# variables set by Configure
######################################################################
CC	= cc
CFLAGS	= -I/pub/unix/net/qi/include  -DFCNTL_LOCK -DSYSV -DWAIT_INT -DKRBNSREALM=\"UIUC.EDU\" -g -I/usr/local/include
LFLAGS	= 
######################################################################
# other variables
######################################################################
SRCS=	cryptit.c fcrypt.c CloseQi.c FreeQIR.c OpenQi.c FieldOps.c ReadQi.c LoginQi.c
OBJS=	cryptit.o fcrypt.o CloseQi.o FreeQIR.o OpenQi.o FieldOps.o ReadQi.o LoginQi.o
OTHERS	= 
BINS	= libqiapi.a /pub/unix/net/qi/lib/libqiapi.a apitest

all: $(BINS)
######################################################################
# specific targets
######################################################################
libqiapi.a:	$(OBJS)
	rm -f $@
	ar crv $@ $(OBJS)
	echo $@

/pub/unix/net/qi/lib/libqiapi.a: libqiapi.a
	rm -f $@
	cp libqiapi.a $@
	echo $@

apitest: apitest.c
	$(CC) $(CFLAGS) $(LFLAGS) -o apitest apitest.c libqiapi.a -ll -L/usr/local/lib -lresolv -lsocket -lnsl -L/usr/local/lib -lkrb -ldes

######################################################################
# standard targets
######################################################################
rcs:	$(SRCS)
	ci $?
	touch rcs

clean:
	rm -f $(BINS) $(OBJS) $(OTHERS) tags LINT install

depend: 
	perl -i -e 'while(<>){print;if (/^# DO NOT DELETE/) {last;}}' Makefile
	$(CC)  $(CFLAGS) $(SRCS) >> Makefile

install: $(BINS)
	-mkdir /usr/local/lib
	/usr/ucb/install -c -m 444 -o nameserv -g  nameserv libqiapi.a /usr/local/lib
	echo /usr/local/lib/libqiapi.a
	touch install
# DO NOT DELETE THIS LINE.

