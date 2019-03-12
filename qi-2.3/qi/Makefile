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
# @(#)$Id: Makefile.templ,v 1.1 1994/03/12 01:09:13 paul Exp $

######################################################################
# variables set by Configure
######################################################################
CC	= cc
CFLAGS	= -I/pub/unix/net/qi/include  -DFCNTL_LOCK -DSYSV -DWAIT_INT -DKRBNSREALM=\"UIUC.EDU\" -g -I/usr/local/include
LFLAGS	= 
######################################################################
# other variables
######################################################################
SRCS	=\
    add.c auth.c bintree.c change.c commands.c conf.c \
    dbd.c dbi.c dbm.c field.c help.c lock.c lookup.c \
    phoneme.c qi.c query.c rules.c set.c version.c
OBJS	=\
    add.o auth.o bintree.o change.o commands.o conf.o \
    dbd.o dbi.o dbm.o field.o help.o lex.yy.o lock.o lookup.o \
    phoneme.o qi.o query.o rules.o set.o version.o
BINS	= qi

all:$(BINS) /pub/unix/net/qi/lib/libqi.a
######################################################################
# specific targets
######################################################################
qi	: $(OBJS) main.o /pub/unix/net/qi/lib/libu.a /pub/unix/net/qi/lib/libqiapi.a
	$(CC) $(LFLAGS) -o qi main.o $(OBJS) /pub/unix/net/qi/lib/libu.a /pub/unix/net/qi/lib/libqiapi.a -ll -L/usr/local/lib -lresolv -lsocket -lnsl -L/usr/local/lib -lkrb -ldes
	chmod 770 $@

/pub/unix/net/qi/lib/libqi.a : $(OBJS)
	rm -f $@
	ar crv $@ $(OBJS)
	echo $@

lex.yy.c: language.l
	cp language.l temp.l  
	lex -e temp.l
	perl -i -p -e 's/define YYLMAX .*/define YYLMAX 4096/;' lex.yy.c
lex.yy.o: ../include/commands.h
######################################################################
# standard targets
######################################################################
install:$(BINS)
	for b in $? ; do \
	  /usr/ucb/install -c -m 0755 -o nameserv -g  nameserv $$b /usr/local/libexec; done
	touch install

rcs:	$(SRCS) language.l
	ci $?
	touch rcs

clean:
	rm -f $(BINS) $(OBJS) /pub/unix/net/qi/lib/libqi.a tags LINT lex.yy.c temp.l main.o install

depend: 
	perl -i -e 'while(<>){print;if (/^# DO NOT DELETE/) {last;}}' Makefile
	$(CC)  $(CFLAGS) $(SRCS) >> Makefile
# DO NOT DELETE THIS LINE.

