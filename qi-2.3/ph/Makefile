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
# @(#)$Id: Makefile.templ,v 1.3 1994/03/12 04:31:21 paul Exp $

######################################################################
# 
######################################################################
CC	= cc
CFLAGS	= -DPH -DHASSTRTOK -I.  -DKERBEROS -DEMAIL_AUTH=\"1\" -DMAILDOMAIN=\"uiuc.edu\" -DFALLBACKPORT=\"105\" -DHOST=\"ns.uiuc.edu\" -DNSSERVICE=\"ns\" -DFALLBACKADDR=\"128.174.5.56\" -I/pub/unix/net/qi/include  -DFCNTL_LOCK -DSYSV -DWAIT_INT -DKRBNSREALM=\"UIUC.EDU\" -g -I/usr/local/include
LFLAGS	= 
######################################################################
# other variables
######################################################################
FLGS	= 
SRCS	= ph.c
OBJS	= ph.o
BINS	= ph
DESTDIR = /usr/local/bin
DESTMAN = /usr/man/man
MANSUFF = l
all:$(BINS)
######################################################################
# specific targets
######################################################################
ph:	$(OBJS)
	$(CC) $(CFLAGS) -o ph $(OBJS) /pub/unix/net/qi/lib/libqiapi.a -ll -L/usr/local/lib -lresolv -lsocket -lnsl -L/usr/local/lib -lkrb -ldes

######################################################################
# standard targets
######################################################################
install:$(BINS)
	for b in $? ; do \
	  /usr/ucb/install -c -m 0755 $$b $(DESTDIR); done
	-ln -s ph $(DESTDIR)/timetable
	-ln -s ph $(DESTDIR)/food
	-ln -s ph $(DESTDIR)/phone
	-ln -s ph $(DESTDIR)/unit
	-ln -s ph $(DESTDIR)/areacode
	-ln -s ph $(DESTDIR)/person
	-ln -s ph $(DESTDIR)/weather
	-ln -s ph $(DESTDIR)/spring
	-ln -s ph $(DESTDIR)/summer
	-ln -s ph $(DESTDIR)/fall
# make ph setuid root if you want to allow rlogin-like ph logins
	chown root $(DESTDIR)/ph
	chmod 4511 $(DESTDIR)/ph
	@echo "Don't forget to install and/or update the /etc/services"
	@echo "for nameservice.  A typical value is"
	@echo ""
	@echo "ns	105/tcp		csnet-ns nameservice	# CSnet nameserver"
	@echo ""
	@echo "The service name must match NSSERVICE in this Makefile.  It's"
	@echo "also used in /etc/inetd.conf.  Example entry is"
	@echo ""
	@echo "ns	stream	tcp	nowait	qiserv	/nameserv/bin/qi	qi -d -t30"
	@touch install

installman: ph.1
	cp ph.1 $(DESTMAN)$(MANSUFF)/ph.$(MANSUFF)
	touch installman

rcs:	$(SRCS)
	ci $?
	touch rcs

clean:
	rm -f $(BINS) $(OBJS) tags install
depend: 
	perl -i -e 'while(<>){print;if (/^# DO NOT DELETE/) {last;}}' Makefile
	$(CC)  $(CFLAGS) $(SRCS) >> Makefile
# DO NOT DELETE THIS LINE.

