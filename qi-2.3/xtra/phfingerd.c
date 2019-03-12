/*
 * Copyright (c) 1983 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Replacement for fingerd that calls ph instead of finger.
 *
 * Install in your favorite location (/usr/local/libexec or /usr/local/etc
 * are favorites), edit /etc/inetd.conf, and restart inetd.  /etc/inetd.conf
 * will need the pathname and execution name of the finger daemon changed to
 * that of phfingerd.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)fingerd.c	5.3 (Berkeley) 11/3/88";
#endif /* not lint */

/*
 * Finger server.
 */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>

int debug = 0;

main(argc, argv)
	int argc;
	char *argv[];
{
	register char *sp;
	char line[512];
	struct sockaddr_in sin;
	int i, p[2], pid, status;
	FILE *fp;
	char *av[5];
	struct rlimit rlp;

	if (argc > 1 && strcmp(argv[1], "-d") == 0)
		debug++;
	i = sizeof (sin);
	if (getpeername(0, &sin, &i) < 0 && !debug)
		fatal(argv[0], "getpeername");
	if (fgets(line, sizeof(line), stdin) == NULL)
		exit(1);
	rlp.rlim_cur = 5000;	/* 5 CPU seconds */
	rlp.rlim_max = 5000;	/* 5 CPU seconds */
	if (setrlimit(RLIMIT_CPU, &rlp) < 0)
		fatal(argv[0], "setrlimit");
	alarm(60);
	if ((sp = index(line, '\n')) != NULL)
		*sp = '\0';
	if ((sp = index(line, '-')) != NULL)
		*sp = ' ';
	sp = line;
	/* openlog("phfingerd", LOG_PID, LOG_DAEMON); */
	i = 0;
	av[i++] = "ph";
	av[i++] = line;
	av[i++] = "return alias name department title email phone address other";
	av[i++] = 0;
#ifdef notdef
	for (i = 1;;) {
		while (isspace(*sp))
			sp++;
		if (!*sp)
			break;
		if (*sp && !isspace(*sp)) {
			av[i++] = sp;
			while (*sp && !isspace(*sp))
				sp++;
			*sp++ = '\0';
			/* syslog(LOG_INFO, "%s", av[i-1]); */
		}
	}
	av[i] = 0;
#endif
	if (pipe(p) < 0)
		fatal(argv[0], "pipe");
	if ((pid = fork()) == 0) {
		close(p[0]);
		if (p[1] != 1) {
			dup2(p[1], 1);
			close(p[1]);
		}
		if (av[1] != NULL)
			execv("/usr/local/bin/ph", av);
		_exit(1);
	}
	if (pid == -1)
		fatal(argv[0], "fork");
	close(p[1]);
	if ((fp = fdopen(p[0], "r")) == NULL)
		fatal(argv[0], "fdopen");
	while ((i = getc(fp)) != EOF) {
		if (i == '\n')
			putchar('\r');
		putchar(i);
	}
	fclose(fp);
	while ((i = wait(&status)) != pid && i != -1)
		;
	return(0);
}

fatal(prog, s)
	char *prog, *s;
{
	fprintf(stderr, "%s: ", prog);
	perror(s);
	exit(1);
}
