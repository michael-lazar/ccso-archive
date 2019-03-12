# This Makefile is automatically generated.  Do NOT edit it here.
######################################################################
# 
######################################################################
CC	= cc
CFLAGS	= -g -DPH -DHASSTRTOK -DNO_MEMMOVE -DWAIT_INT -I.  -DEMAIL_AUTH=\"1\" -DMAILDOMAIN=\"uiuc.edu\" -DFALLBACKPORT=\"105\" -DHOST=\"ns.uiuc.edu\" -DNSSERVICE=\"ns\" -DFALLBACKADDR=\"128.174.5.58\" # -DKERBEROS -DKRBNSREALM=\"UIUC.EDU\"
LFLAGS	= 
######################################################################
# other variables
######################################################################
FLGS	= 
SRCS	= ph.c cryptit.c fcrypt.c LoginQi.c ReadQi.c FieldOps.c
OBJS	= ph.o cryptit.o fcrypt.o LoginQi.o ReadQi.o FieldOps.o
LIBS	= -lresolv # -lsocket -lnsl -lkrb -ldes
BINS	= ph
DESTDIR = /usr/local/bin
DESTMAN = /usr/local/man
MANSUFF = 1
all:$(BINS)
######################################################################
# specific targets
######################################################################
ph:	$(OBJS)
	$(CC) $(CFLAGS) -o ph $(OBJS) $(LIBS)

######################################################################
# standard targets
######################################################################
install:$(BINS)
	for b in $? ; do \
	  install -c -m 0755 $$b $(DESTDIR); done
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
	chmod 4755 $(DESTDIR)/ph
	@echo "Don't forget to install and/or update the /etc/services"
	@echo "for nameservice.  Typical values are"
	@echo ""
	@echo "csnet-ns	105/tcp				# CSnet nameserver"
	@echo "	or"
	@echo "ns		105/tcp		nameservice	# CSO nameserver"
	@echo ""
	@echo "The service name must match NSSERVICE in this Makefile.  It's"
	@echo "also used in /etc/inetd.conf.  Example entry is"
	@echo ""
	@echo "ns	stream	tcp	nowait	qiserv	/nameserv/bin/qi	qi -d -t30"
	@touch install

installman: ph.1
	cp ph.1 $(DESTMAN)/man$(MANSUFF)/ph.$(MANSUFF)
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

