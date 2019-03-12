#ifndef lint
static char	*sccsid = "@(#)fakesyslog.c	1.3	(Berkeley) 2/6/88";
#endif

/*
 * Fake syslog routines for systems that don't have syslog.
 * Taken from an idea by Paul McKenny, <mckenny@sri-unix.arpa>.
 * (Unfortunately, Paul, I can't distribute the real syslog code
 * as you suggested ... sigh.)
 *
 * Warning: this file contains joe code that may offend you.
 */

#include <stdio.h>

/*#include "../common/conf.h" */

#define FAKESYSLOG "/emds/db/qi.log"

#ifdef FAKESYSLOG

extern	int	errno;
extern	int	sys_nerr;
extern	char	*sys_errlist[];

static FILE	*logfp;

char	*strcpy(), *strcat(), *ctime();

openlog()
{
	logfp = fopen(FAKESYSLOG, "a");
}


syslog(pri, msg, x1, x2, x3, x4, x5, x6)
	int	pri;
	char	*msg, *x1, *x2, *x3, *x4, *x5, *x6;
{
	char		buf[1024];
	char		*cp, *bp;
	long		clock;
	static int	failed = 0;

	if (failed)
		return;

	if (logfp == NULL) {
		openlog();
		if (logfp == NULL) {
			failed = 1;
			return;
		}
	}

	(void) time(&clock);
	(void) strcpy(buf, ctime(&clock));

	bp = buf + strlen(buf)-1;
	*bp++ = ' ';
	*bp = '\0';
	for (cp = msg; *cp; cp++) {
		if (*cp == '%' && cp[1] == 'm') {
			*bp = '\0';
			if (errno >= sys_nerr || errno < 0) {
				char	work[32];
				sprintf(work, "unknown error #%d", errno);
				(void) strcat(bp, work);
			} else
				(void) strcat(bp, sys_errlist[errno]);
			bp = buf + strlen(buf);
			cp++;
		} else {
			*bp++ = *cp;
		}
	}
	*bp = '\0';
	/* Ah, the semantic security of C ... */
	if (bp[-1] != '\n')
		(void) strcat(bp, "\n");

	fprintf(logfp, buf, x1, x2, x3, x4, x5, x6);
}

#endif
