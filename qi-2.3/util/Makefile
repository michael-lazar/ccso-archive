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
# @(#)$Id: Makefile.templ,v 1.1 1994/03/12 01:10:30 paul Exp $

######################################################################
# variables set by Configure
######################################################################
CC	= cc
CFLAGS	= -I/pub/unix/net/qi/include  -DFCNTL_LOCK -DSYSV -DWAIT_INT -DKRBNSREALM=\"UIUC.EDU\" -g -I/usr/local/include
LFLAGS	= 
######################################################################
# other variables
######################################################################
SRCS	= border.c build.c credb.c id.c maked.c \
	  makei.c maket.c mdump.c nsck.c ssnid.c ssnload.c ssndump.c

OBJS	= \
  border.o build.o credb.o id.o \
  maked.o makei.o maket.o mdump.o \
  nsck.o ssnid.o ssnload.o ssndump.o brute2.o

BINS	= \
  border build credb maked	\
  makei mdump nsck ssnid	\
  ssnload ssndump brute

SCRIPTS = addnickname age aliasassign aliasprepare f.unblock \
	froshlog h.unblock merge3 pg phify phoneaddr qierrs \
	s.unblock sizedb zaplist

ALLLIB = /pub/unix/net/qi/lib/libqi.a /pub/unix/net/qi/lib/libu.a /pub/unix/net/qi/lib/libqiapi.a
all:$(BINS)
######################################################################
# specific targets
######################################################################
brute: brute2.o
	$(CC) $(CFLAGS) -o brute brute2.o
	
build: build.o maket.o $(ALLLIB)
	$(CC) $(CFLAGS) -o build build.o maket.o $(ALLLIB) -ll -L/usr/local/lib -lresolv -lsocket -lnsl -L/usr/local/lib -lkrb -ldes
	
maked : maked.o $(ALLLIB)
	$(CC) $(CFLAGS) -o maked  maked.o $(ALLLIB) -ll -L/usr/local/lib -lresolv -lsocket -lnsl -L/usr/local/lib -lkrb -ldes

mdump : mdump.o $(ALLLIB)
	$(CC) $(CFLAGS) -o mdump  mdump.o $(ALLLIB) -ll -L/usr/local/lib -lresolv -lsocket -lnsl -L/usr/local/lib -lkrb -ldes

nsck : nsck.o ../qi/conf.o $(ALLLIB)
	$(CC) $(CFLAGS) -o nsck  nsck.o $(ALLLIB) -ll -L/usr/local/lib -lresolv -lsocket -lnsl -L/usr/local/lib -lkrb -ldes

border : border.o
	$(CC) $(CFLAGS) -o border  border.o

ssnid : ssnid.o id.o
	$(CC) $(CFLAGS) -o ssnid  ssnid.o id.o 

ssndump : ssndump.o id.o
	$(CC) $(CFLAGS) -o ssndump  ssndump.o id.o 

ssnload : ssnload.o id.o
	$(CC) $(CFLAGS) -o ssnload  ssnload.o id.o 

makei : makei.o $(ALLLIB)
	$(CC) $(CFLAGS) -o makei  makei.o $(ALLLIB) -ll -L/usr/local/lib -lresolv -lsocket -lnsl -L/usr/local/lib -lkrb -ldes

credb :	credb.o
	$(CC) $(CFLAGS) -o credb  credb.o $(ALLLIB) -ll -L/usr/local/lib -lresolv -lsocket -lnsl -L/usr/local/lib -lkrb -ldes

######################################################################
# standard targets
######################################################################
install:$(BINS) $(SCRIPTS)
	for b in $(BINS) $(SCRIPTS) ; do \
	  /usr/ucb/install -c -m 0755 -o nameserv -g  nameserv $$b /var/apps/nameserv/bin; done
	touch install

rcs:	$(SRCS)
	ci $?
	touch rcs

clean:
	rm -f $(BINS) $(OBJS) tags install
depend: 
	perl -i -e 'while(<>){print;if (/^# DO NOT DELETE/) {last;}}' Makefile
	$(CC)  $(CFLAGS) $(SRCS) >> Makefile
# DO NOT DELETE THIS LINE.

