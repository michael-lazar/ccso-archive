/*
**  PipeQi -- Connect to the Qi server via a pipe connection
**
**	Parameters:
**		LocalQi - pathname to the qi program
**		To - pointer to stream descriptor
**		From - pointer to stream descriptor
**
**	Returns:
**		 0 if successful
**		-1 if any error
**
**	Side Effects:
**		Initializes global QiFields by calling GetFields()
**		Reports error via syslog
**
**	History:
**		Code based on OpenQi.c and qtacacsd's main program.
**		Allan Tuchman <tuchman@uiuc.edu>
*/

#include <qiapi.h>
#include <syslog.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>


/* vfork is available under AIX (and others?), but only with -lbsd */
#if (defined(_AIX)) && !defined(vfork)
# define        vfork           fork
#endif /* AIX && !vfork */

#define FILE_NULL	((FILE *) NULL)
#define CPNULL		((char *) NULL)


int
PipeQi (LocalQi, To, From)
char	*LocalQi;
FILE	**To, **From;
{
    int Pid;            /* value from fork() */
    int P1[2], P2[2];   /* Parent/child pipes for Qi */

    if (QiDebug)
	fprintf(stderr, "establishing pipe connection to %s\r\n", LocalQi);

    if (pipe (P1) == -1) {
        if (QiDebug)
	    fprintf(stderr, "PipeQi: Pipe(1): %s\r\n", sys_errlist[errno]);
	syslog(LOG_ERR, "PipeQi: pipe(1): %m");
	return(-1);
	}
    if (pipe (P2) == -1) {
        if (QiDebug)
	    fprintf(stderr, "PipeQi: Pipe(2): %s\r\n", sys_errlist[errno]);
	syslog(LOG_ERR, "PipeQi: pipe(2): %m");
	return(-1);
	}
    if ((Pid = vfork()) == -1) {
        if (QiDebug)
	    fprintf(stderr, "PipeQi: vfork(): %s\r\n", sys_errlist[errno]);
	syslog(LOG_ERR, "PipeQi: vfork(): %m");
	return(-1);
	}

    if (Pid == 0) {	/* child */
	/* Close and dup, close and dup */
	if (close (0) == -1) {
            if (QiDebug)
	        fprintf(stderr, "PipeQi: close(0): %s\r\n", sys_errlist[errno]);
	    syslog(LOG_ERR, "PipeQi: close(0): %m");
	    return(-1);
	}
	if (fcntl (P1[0], F_DUPFD, 0) == -1) {
            if (QiDebug)
	        fprintf(stderr, "PipeQi: fcntl(0): %s\r\n", sys_errlist[errno]);
	    syslog(LOG_ERR, "PipeQi: fcntl(0): %m");
	    return(-1);
	}
	if (close (1) == -1) {
            if (QiDebug)
	        fprintf(stderr, "PipeQi: close(1): %s\r\n", sys_errlist[errno]);
	    syslog(LOG_ERR, "PipeQi: close(1): %m");
	    return(-1);
	}
	if (fcntl (P2[1], F_DUPFD, 1) == -1) {
            if (QiDebug)
	        fprintf(stderr, "PipeQi: fcntl(1): %s\r\n", sys_errlist[errno]);
	    syslog(LOG_ERR, "PipeQi: fcntl(1): %m");
	    return(-1);
	}

	/* Close all open file descriptors */
	(void) close (P1[0]);
	(void) close (P1[1]);
	(void) close (P2[0]);
	(void) close (P2[1]);

	/* Fire up Qi */
	execl (LocalQi, "qi", "-q", CPNULL);
	if (QiDebug)
	    fprintf(stderr, "%s %s \r\nexecl failed: %s\r\n",
					LocalQi, "qi -q", sys_errlist[errno]);
	syslog(LOG_ERR, "%s execl: %s", LocalQi, sys_errlist[errno]);
	return(-1);
    }

    /* Assign stream descriptors to pipe components */
    if ((*To = fdopen (P1[1], "w")) == FILE_NULL) {
        if (QiDebug)
	    fprintf(stderr, "PipeQi: fdopen(To): %s\r\n", sys_errlist[errno]);
	syslog(LOG_ERR, "PipeQi: fdopen(To): %m");
	return(-1);
	}
    if ((*From = fdopen (P2[0], "r")) == FILE_NULL) {
        if (QiDebug)
	    fprintf(stderr, "PipeQi: fdopen(From): %s\r\n",
							sys_errlist[errno]);
	syslog(LOG_ERR, "PipeQi: fdopen(From): %m");
	return(-1);
	}


    /* Initialize QiFields global */
    if (QiFields == QIF_NULL &&
			(QiFields = GetFields(*To, *From)) == QIF_NULL)
	return(-1);


    /* Connection ok */
    return (0);
}
