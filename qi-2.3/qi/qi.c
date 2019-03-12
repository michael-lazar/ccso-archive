/*
 * Copyright (c) 1985 Corporation for Research and Educational Networking
 * Copyright (c) 1988 University of Illinois Board of Trustees, Steven
 *		Dorner, and Paul Pomes
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the Corporation for
 *	Research and Educational Networking (CREN), the University of
 *	Illinois at Urbana, and their contributors.
 * 4. Neither the name of CREN, the University nor the names of their
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE TRUSTEES AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE TRUSTEES OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char  RcsId[] = "@(#)$Id: qi.c,v 1.71 1994/04/12 17:22:09 paul Exp $";
#endif

#include "protos.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <pwd.h>
#include <sys/time.h>
#include <signal.h>
#include <netinet/in.h>
#include <netdb.h>

FILE	*Input, *Output;
int	Daemon = 0;
int	ReadOnly = 0;
int	TurnedOff = 0;
int	Initializing = 1;
int	Timeout = 0;
int	LockTimeout = 30;
int	Quiet = 0;

int	OffCampus = 0;
int	LocalUser = 1;

#ifdef EMAIL_AUTH
struct hostent TrustHp;
#endif

char	*DBState;
char	*hostname = NULL;

static void SetSignals();
static void WhoAreYou();

/*
 * what am I talking to?
 */
int	InputType;
char	Foreign[256];
static char *Me;		/* the name of this program */

int
qimain(argc, argv)
	int argc;
	char **argv;
{
	char	**opt;
	char	*equal;
	int	pid = getpid();		/* available for debugging */

#ifdef USE_GID
	setgid(USE_GID);
#endif
#ifdef USE_UID
	setuid(USE_UID);
#endif
#ifdef RUNDIR
	chdir(RUNDIR);
#endif

	Input = stdin;
	Output = stdout;

	/* when you're strange, no one remembers your name */
	Me = *argv;

	for (argc--, argv++; argc && **argv == '-'; argc--, argv++)
	{
		(*argv)++;
		switch (**argv)
		{
		    case 'q':
			Quiet = 1;
			break;
		    case 'w':
			ReadOnly = 0;
			break;
		    case 'd':
			Daemon = 1;
			break;
		    case 't':
			Timeout = 60 * atoi((*argv) + 1);
			break;
		    case 'k':
			LockTimeout = 60 * atoi((*argv) + 1);
			break;
		    case 'l':
			OP_VALUE(NOLOG_OP) = strdup("");
			break;
		    default:
			if (equal = strchr(*argv, '='))
			{
				*equal++ = '\0';
				for (opt = Strings; *opt; opt += 2)
				{
					if (!strcmp(opt[0], *argv))
					{
						opt[1] = equal;
						break;
					}
				}
				if (*opt)
					break;
				fprintf(stdout, "%s: %s: unknown string.\n",
					Me, *argv);
				exit(1);
			} else
			{
				fprintf(stdout, "%s: %s: unknown option.\n",
					Me, *argv);
				exit(1);
			}
		}
	}
	Database = DATABASE;
	if (!OP_VALUE(NOLOG_OP))
	{
#ifdef LOG_DAEMON
# ifndef LOG_QILOG
#  define	LOG_QILOG	LOG_DAEMON
# endif	/* !LOG_QILOG */
		openlog("tqi", LOG_PID, LOG_QILOG);
#else	/* !LOG_DAEMON */
		openlog("tqi", LOG_PID);
#endif	/* LOG_DAEMON */
	}

	/* Quick, verify that crypt() is sane! */
	if (strcmp(crypt("fredfred", "fredfred"), "frxWbx4IRuBBA"))
	{
		IssueMessage(LOG_ERR, "Incompatible crypt() - check bit order in api/fcrypt.c");
		IssueMessage(LOG_ERR, "or recompile w.o. #define crypt fcrypt in");
		IssueMessage(LOG_ERR, "include/qiapi.h and include/protos.h");
		DoReply(LR_ERROR, "Server compiled with broken crypt() function");
		DoQuit(NULL);
	}

	WhoAreYou();
	IssueMessage(LOG_INFO, "begin %s", Foreign);
	InitializeOptions();
	SetSignals();

	/* get configuration */
	if (!GetFieldConfig())
		exit(1);
	if ((Quiet || Daemon) && GetState())
	{
		fprintf(Output, "555:Database shut off (%s).\n", DBState);
		exit(0);
	}
	Initializing = 0;
	if (!Daemon && (InputType == IT_TTY || InputType == IT_PIPE || InputType == IT_FILE))
		AmHero = 1;
	do
	{
		if (Timeout)
			alarm(0);
		fflush(Output);
		if (!Quiet && (InputType == IT_TTY || InputType == IT_PIPE || InputType == IT_FILE))
			printf("\nmqi> ");
		if (Timeout && !AmHero)
			alarm(Timeout);
	}
	while (yylex());
	DoQuit(NULL);
	IssueMessage(LOG_INFO, "Done 0");
}

void
cleanup(sig)
	int sig;
{
	int level = (sig == SIGALRM) ? LOG_INFO : LOG_ERR;

	IssueMessage(level, "Done %d", sig);
	exit(1);
}

static void
SetSignals()
{
	(void) signal(SIGALRM, cleanup);
	(void) signal(SIGHUP, cleanup);
	(void) signal(SIGINT, cleanup);
	(void) signal(SIGTERM, cleanup);
/*
	(void) signal(SIGQUIT, cleanup);
	(void) signal(SIGILL, cleanup);
	(void) signal(SIGTRAP, cleanup);
	(void) signal(SIGIOT, cleanup);
	(void) signal(SIGEMT, cleanup);
	(void) signal(SIGFPE, cleanup);
	(void) signal(SIGBUS, cleanup);
	(void) signal(SIGSEGV, cleanup);
	(void) signal(SIGSYS, cleanup);
 */
}

#ifndef L_INCR
# define L_INCR 1
#endif /* !L_INCR */

/*
 * who am I talking to?
 */
static void
WhoAreYou()
{
	struct sockaddr From;
	int	FromLen = sizeof (From);
	struct passwd *pwd = NULL;
	char	errorstr[MAXSTR];
	int	s = fileno(stdin);

	if (isatty(s))
	{
		char	*name = (char *) getlogin();
		int	uid = (int) getuid();

		InputType = IT_TTY;
		if (name != NULL)
			pwd = getpwnam(name);
		if (pwd == NULL)
			pwd = getpwuid(uid);
		if (pwd)
			(void) sprintf(Foreign, "%s %s", ttyname(s), pwd->pw_name);
		else {
			IssueMessage(LOG_ERR, "getlogin() || getpwuid() failed for uid %d\n", uid);
			exit(1);
		}
	} else if (getpeername(s, &From, &FromLen) == 0)
	{
		struct sockaddr_in *sin = (struct sockaddr_in *) (&From);
		struct hostent *hp = NULL;
		int	on = 1;
		int     hostnamep = 0;


		InputType = IT_NET;
		/* get name of connected client */
		hp = gethostbyaddr((char *)&sin->sin_addr, sizeof (sin->sin_addr), AF_INET);


		if (hp) {
			/*
			 * Attempt to verify that we haven't been fooled by
			 * someone in a remote net; look up the name and check
			 * that this address corresponds to the name.
			 */

			int	HostNameLen = strlen (hp->h_name);
			char	remotehost[2 * MAXHOSTNAMELEN + 1];

			hostnamep = 1;
			hostname = strdup(hp->h_name);
			strncpy(remotehost, hp->h_name, sizeof(remotehost) - 1);
			remotehost[sizeof(remotehost) - 1] = 0;
			hp = gethostbyname(remotehost);
			if (hp == NULL) {
				(void) sprintf(errorstr, "Couldn't look up address for %s", remotehost);
				DoReply(LR_NOADDR, errorstr);
				IssueMessage(LOG_NOTICE, errorstr);
				DoQuit(NULL);
			} else for (; ; hp->h_addr_list++) {
				if (hp->h_addr_list[0] == NULL) {
					(void) sprintf(errorstr,
					    "Host addr %s not listed for host %s",
#if defined(sparc) && __GNUC__ == 1
					    inet_ntoa(&sin->sin_addr),
#else
					    inet_ntoa(sin->sin_addr),
#endif
					    hp->h_name);
					DoReply(LR_MISMATCH, errorstr);
					IssueMessage(LOG_NOTICE, errorstr);
					DoQuit(NULL);
				}
				if (!memcmp(hp->h_addr_list[0], (caddr_t)&sin->sin_addr,
				    sizeof(sin->sin_addr))) {
					hostname = strdup(hp->h_name);
					break;
				}
			}
		} else {
#if defined(sparc) && __GNUC__ == 1
			hostname = strdup((char *) inet_ntoa(&sin->sin_addr));
#else
			hostname = strdup((char *) inet_ntoa(sin->sin_addr));
#endif
			IssueMessage(LOG_NOTICE, "Couldn't find hostname for address (%s)",
			    hostname);
#ifndef NOCHECKNET
			DoReply(LR_NONAME, "No hostname found for IP address");
			/* DoQuit(NULL);	/* How rude do we need to be? */
#endif /* !NOCHECKNET */
		}
		(void) sprintf(Foreign, "NET %s", hostname);
#ifdef EMAIL_AUTH
		memset(&TrustHp, 0, sizeof (TrustHp));
		if (hp != NULL && ntohs(sin->sin_port) < IPPORT_RESERVED)
			(void) memcpy((void*)&TrustHp, (void*)hp, sizeof (TrustHp));
#endif

		if (*OkAddrs)
		{
			char	**local;
			int	hlen;
			int	llen;

			OffCampus = 1;

			/* Try OkAddrs as IP address fragments */
			for (local = OkAddrs; *local; local++)
				if (!strncmp(Foreign+4, *local, strlen(*local)))
					OffCampus = 0;

			/* Then as domain names */
			if (OffCampus == 1 && hostnamep)
			{
				hlen  = strlen(hostname);
				for (local = OkAddrs; *local; local++)
				{
					llen = strlen(*local);
					if (hlen < llen)
						continue;
					if (!stricmp((hostname + (hlen - llen)), *local))
						OffCampus = 0;
				}
			}
		}
		if (*LocalAddrs)
		{
			char	**local;
			int	hlen;
			int	llen;

			LocalUser = 0;

			/* Try LocalAddrs as IP address fragments */
			for (local = LocalAddrs; *local; local++)
				if (!strncmp(Foreign+4, *local, strlen(*local)))
					LocalUser = 1;

			/* Then as domain names */
			if (LocalUser == 0 && hostnamep)
			{
				hlen  = strlen(hostname);
				for (local = LocalAddrs; *local; local++)
				{
					llen = strlen(*local);
					if (hlen < llen)
						continue;
					if (!stricmp ((hostname + (hlen - llen)), *local))
						LocalUser = 1;
				}
			}
		}
		if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *)&on,
		    sizeof(on)) < 0)
			IssueMessage(LOG_WARNING,"setsockopt(SO_KEEPALIVE): %s",
			    strerror(errno));
	} else if (lseek(s, 0, L_INCR) >= 0)
	{
		InputType = IT_FILE;
		pwd = getpwuid(getuid());
		(void) sprintf(Foreign, "FILE %s", pwd->pw_name);
	} else
	{
		InputType = IT_PIPE;
		pwd = getpwuid(getuid());
		(void) sprintf(Foreign, "PIPE %s", pwd->pw_name);
	}
}

/*
 * read the state of the database from a file
 */
int
GetState()
{
	char	name[80];
	FILE	*fp;
	static char state[1024];
	char	*token;

	(void) sprintf(name, "%s.sta", Database);
	if ((fp = fopen(name, "r")) == NULL)
		return (0);

	if (fgets(state, 1024, fp))
	{
		fclose(fp);
		if (DBState = strtok(state, " \t\n"))
		{
			if (!strcmp(DBState, "off"))
			{
				DBState = (token = strtok(0, "\n")) ?
				    token : "for maintenance";
				return (1);
			} else
			{
				DBState = strtok(0, "\n");
				if (!ReadOnly && !Initializing)
					fprintf(Output, "100:The database is now read-only (%s).\n",
						DBState);
				ReadOnly = 1;
			}
		}
	} else
		(void) fclose(fp);

	return (0);
}

/*
 * exit cleanly when limit hit
 */
void
LimitHit(sig)
	int sig;
{
	signal(SIGXCPU, SIG_IGN);
	fprintf(Output, "%d: CPU time limit exceeded for this session.\n", LR_XCPU);
	DoQuit(NULL);
}
