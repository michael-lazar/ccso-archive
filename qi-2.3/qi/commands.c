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
static char  RcsId[] = "@(#)$Id: commands.c,v 1.65 1994/03/21 13:21:31 paul Exp $";
#endif

#include "protos.h"

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <netdb.h>
#include <sys/param.h>

#ifdef KERBEROS
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <des.h>
#include <krb.h>
#endif /* KERBEROS */

static ARG *s_args = NULL;

static void DoKLogin __P((ARG *));
static void DoAnswer __P((ARG *));
static char *RandomString __P((int));
static int UserMatch __P((char *));
static void DoDelete __P((ARG *));
static void DoFields __P((ARG *));
static void DoId __P((ARG *));
static void DoInfo __P((ARG *));
static void DoLogin __P((ARG *));
static void DoLogout __P((ARG *));
static void DoStatus __P((ARG *));
static void DumpOutput();
static void ListAllFields();
static void ListField __P((FDESC *));
static void NotImplemented __P((ARG *));
static int OkByEmail __P((QDIR, char *));
static int OkFields __P((ARG *));
static int OpenTempOut();
static void PrintCommand __P((int, ARG *));
static void SleepTil __P((long));
#ifdef NO_STRSTR
       char *strstr __P((char *, char *));
#endif

extern FILE *Input;		/* qi.c */
extern FILE *Output;		/* qi.c */
extern int InputType;		/* qi.c */
extern char Foreign[80];	/* qi.c */
extern char CommandText[];	/* language.l */
extern char *CommandSpot;	/* language.l */
extern int Daemon;		/* qi.c */
extern int Quiet;		/* qi.c */
extern int LocalUser;		/* qi.c */
extern char *DBState;		/* qi.c */
extern char Revision[];		/* version.c */

#ifdef EMAIL_AUTH
extern struct hostent TrustHp;	/* qi.c */

#endif

FILE	*TempOutput = NULL;
static char *TFile = NULL;

int	DiscardIt;
void	(*CommandTable[]) __P((ARG *)) =
{
	DoQuery,
	DoChange,
	DoLogin,
	DoAnswer,
	DoLogout,
	DoFields,
	DoAdd,
	DoDelete,
	DoSet,
	DoQuit,
	DoStatus,
	DoId,
	DoHelp,
	DoAnswer,
	DoInfo,
	DoAnswer,
	DoKLogin,	
	(void (*) __P((ARG *))) 0
};

QDIR	User = NULL;		/* the currently logged in user */
int	AmHero = 0;		/* is currently logged in user the Hero? */
int	AmOpr = 0;		/* is currently logged in user the Operator? */
char	*UserAlias = NULL;	/* the alias of the currently logged in user */
int	UserEnt = 0;		/* entry number of current user */

char	*ClaimUser = NULL;	/* the identity the user has claimed, but not
				 * yet verified */
char	*Challenge = NULL;	/* challenge string */
int	State = S_IDLE;

/*
 * Add a value to an argument list
 */
void 
AddValue(value, ttype)
	char *value;
	int ttype;
{
	static ARG *LastArg = NULL;

	if (!s_args)
	{
		LastArg = s_args = FreshArg();
		LastArg->aFirst = strdup(value);
		LastArg->aType = ttype;
	} else
	{
		if (LastArg->aType == VALUE && ttype & EQUAL)
			LastArg->aType |= ttype;
		else if (LastArg->aType & EQUAL && !(LastArg->aType & VALUE2))
		{
			LastArg->aType |= VALUE2;
			LastArg->aSecond = strdup(value);
		} else
		{
			LastArg->aNext = FreshArg();
			LastArg = LastArg->aNext;
			LastArg->aType = ttype;
			LastArg->aFirst = strdup(value);
		}
	}
}

/*
 * Complain about extraneous junk
 */
void 
Unknown(junk)
	char *junk;
{
	fprintf(Output, "%d:%s:%s\n", LR_NOCMD, junk, "Command not recognized.");
}

/*
 * Execute a command
 */
void 
DoCommand(cmd)
	int cmd;
{
#define MAX_SYSLOG 100
	char	c;
	int	which = cmd == C_CLEAR ? 6 : MAX_SYSLOG;

	c = CommandText[which];
	CommandText[which] = '\0';
	*CommandSpot = '\0';
	if (*CommandText && strstr(CommandText, "password=") == NULL)
		IssueMessage(LOG_INFO, "%s", CommandText);
	CommandText[which] = c;
	if (!Quiet && (InputType == IT_FILE || InputType == IT_PIPE || OP_VALUE(ECHO_OP)))
		PrintCommand(cmd, s_args);	/* echo the cmd */

	if (Daemon && GetState())
	{
		fprintf(Output, "555:Database shut off (%s).\n", DBState);
		exit(0);
	}
	if (TempOutput == NULL && !OpenTempOut())
	{
		fprintf(Output, "%d:couldn't open temp file.\n", LR_INTERNAL);
		IssueMessage(LOG_INFO, "Couldn't open temp file.\n");
		goto done;
	}
	if (State == S_E_PENDING)
	{
		if (DiscardIt || cmd != C_ANSWER && cmd != C_CLEAR && cmd != C_EMAIL)
		{
			DoReply(-LR_NOANSWER, "Expecting answer, clear, or email; login discarded.");
			State = S_IDLE;
			free(ClaimUser);
			ClaimUser = NULL;
			free(Challenge);
			Challenge = NULL;
		}
	}
	if (DiscardIt)
		DoReply(LR_SYNTAX, "Command not understood.");
	else
	{
		/* (*CommandTable[cmd - 1]) (s_args); */
		void (*cmdP) __P((ARG *)) = *CommandTable[cmd - 1];

		if (cmdP != NULL)
			(*cmdP) (s_args);
		else
			DoReply(LR_SYNTAX, "Command not understood.");
	}

	DumpOutput();

      done:
	FreeArgs(s_args);
	s_args = NULL;
	DiscardIt = 0;
}

/*
 * Free the argument list
 */
void 
FreeArgs(arg)
	ARG *arg;
{
	ARG	*narg;

	if (arg)
		for (narg = arg->aNext; arg; arg = narg)
		{
			narg = arg->aNext;
			if (arg->aFirst)
				free(arg->aFirst);
			if (arg->aSecond)
				free(arg->aSecond);
			free((char *) arg);
			arg = NULL;
		}
}

/*
 * create a fresh argument structure
 */
ARG *
FreshArg()
{
	ARG	*arg;

	arg = (ARG *)malloc(sizeof(ARG));
	memset((void *) arg, (char)0, sizeof (ARG));
	return (arg);
}

/*
 * status--give the database status
 */
/*ARGSUSED*/
static void 
DoStatus(arg)
	ARG *arg;
{
	char	banner[MAXPATHLEN], vstr[MAXSTR];
	FILE	*bfp;

	(void)sprintf(banner, "%s.bnr", Database);
	if (bfp = fopen(banner, "r"))
	{
		(void)sprintf(vstr, "Qi server %s", Revision);
		DoReply(LR_PROGRESS, vstr);
		while (fgets(banner, sizeof (banner) - 1, bfp))
		{
			char	*nl = strchr(banner, '\n');

			if (nl)
				*nl = '\0';
			DoReply(LR_PROGRESS, banner);
		}
		(void) fclose(bfp);
	}
	if (ReadOnly)
		DoReply(LR_RONLY, "Database ready, read only (%s).", DBState);
	else
		DoReply(LR_OK, "Database ready.");
}

/*
 * id--this command is a no-op; the client issues it only to put
 * the name of the calling user into the nameserver's logfiles
 */
/*ARGSUSED*/
static void 
DoId(arg)
	ARG *arg;
{
	DoReply(LR_OK, "Thanks.");
}

/*
 * quit
 */
/*ARGSUSED*/
void 
DoQuit(arg)
	ARG *arg;
{
	fprintf(Output, "%d:%s\n", LR_OK, "Bye!");
	fflush(Output);
	IssueMessage(LOG_INFO, "Done 0");
	closelog();
	exit(0);
}

/*
 * info
 */
/*ARGSUSED*/
static void 
DoInfo(arg)
	ARG *arg;
{
	short	n = 0;

#ifdef MAILDOMAIN
	DoReply(-LR_OK, "%d:maildomain:%s", ++n, MAILDOMAIN);
#endif
	DoReply(-LR_OK, "%d:mailfield:%s", ++n, MAILFIELD);
	DoReply(-LR_OK, "%d:administrator:%s", ++n, ADMIN);
	DoReply(-LR_OK, "%d:passwords:%s", ++n, PASSW);
	DoReply(-LR_OK, "%d:mailbox:%s", ++n, MAILBOX);
#ifdef CHARSET
	DoReply(-LR_OK, "%d:charset:%s", ++n, CHARSET);
#endif
	DoReply(LR_OK, "Ok.");
}

/*
 * Not implemented
 */
static void 
NotImplemented(arg)
	ARG *arg;
{
	DoReply(500, "%s:command not implemented.", arg->aFirst);
}

/*
 * make a reply to a command
 */
/*VARARGS2*/
void 
#ifdef __STDC__
DoReply(int code, char *fmt,...)
#else /* !__STDC__ */
DoReply(code, fmt, va_alist)
	int code;
	char *fmt;
va_dcl
#endif /* __STDC__ */
{
	char	scratchFormat[256];
	va_list	args;

	(void) sprintf(scratchFormat, "%d:%s\n", code, fmt);

#ifdef __STDC__
	va_start(args, fmt);
#else /* !__STDC__ */
	va_start(args);
#endif /* __STDC__ */
	if (TempOutput == NULL && !OpenTempOut())
	{
		fprintf(Output, "%d:couldn't open temp file.\n", LR_INTERNAL);
		IssueMessage(LOG_INFO, "Couldn't open temp file.\n");
		return;
	}
	vfprintf(TempOutput, scratchFormat, args);
	va_end(args);
	{
		char	buf[4096];

#ifdef __STDC__
		va_start(args, fmt);
#else /* !__STDC__ */
		va_start(args);
#endif /* __STDC__ */
		vsprintf(buf, scratchFormat, args);
		va_end(args);
		/* IssueMessage(LOG_DEBUG, "%s", buf); */
	}
}

/*
 * identify user, using Kerberos authentication.
 */
#ifndef KERBEROS
static void 
DoKLogin(arg)
	ARG *arg;
{
	DoReply(LR_SYNTAX, "%s:Kerberos login not available.", arg->aFirst);
}
#else
static void 
DoKLogin(arg)
	ARG *arg;
{
	char	*me;

	me = arg->aFirst;
	if (arg->aNext)
		DoReply(LR_SYNTAX, "%s:extra arguments.", me);
	else
	{
		struct sockaddr_in peername, myname;
		int	namelen = sizeof (peername);
		int	status;
		long	authopts;
		AUTH_DAT auth_data;
		KTEXT_ST clt_ticket;
		Key_schedule sched;
		char	instance[INST_SZ];
		char	version[9];
#ifndef KRBSRVTAB
#define KRBSRVTAB ""		/* use the kerberos default (/etc/srvtab) */
#endif
#ifndef KRBHERO
#define KRBHERO "phhero"	/* use the default instance */
#endif
		char	*srvtab = KRBSRVTAB;
		char	*heroinst = KRBHERO;


		fprintf(Output, "%d:Login started; send kerberos mutual authenticator.\n", LR_LOGIN);
		fflush(Output);

		/*
		 * To verify authenticity, we need to know the address of the
		 * client.
		 */
		if (getpeername(0, (struct sockaddr *) &peername, &namelen) < 0)
		{
			IssueMessage(LOG_ERR, "getpeername: %m");
			DoReply(LR_INTERNAL, "getpeername: %m");
			return;
		}

		/* for mutual authentication, we need to know our address */
		namelen = sizeof (myname);
		if (getsockname(0, (struct sockaddr *) &myname, &namelen) < 0)
		{
			IssueMessage(LOG_ERR, "getsockname: %m");
			DoReply(LR_INTERNAL, "getsockname: %m");
			return;
		}

		/*
		 * Read the authenticator and decode it.  Since we don't care
		 * what the instance is, we use "*" so that krb_rd_req
		 * will fill it in from the authenticator.
		 */
		(void) strcpy(instance, "*");

		/* we want mutual authentication */
		authopts = KOPT_DO_MUTUAL;
		status = krb_recvauth(authopts, 0, &clt_ticket, "ns",
				instance, &peername, &myname, &auth_data, 
				 srvtab, sched, version);
		if (status != KSUCCESS)
		{
			IssueMessage(LOG_INFO, "Login failed: (%d) %s", status, krb_err_txt[status]);
			DoReply(LR_ERROR, "Login failed: (%d) %s", status, krb_err_txt[status]);
		} else
		{
			char	realm[REALM_SZ];

			/* Check the version string (8 chars) */
			if (strncmp(version, "VERSION9", 8))
			{
				/*
				 * didn't match the expected version.
				 * could do something different, but we just
				 * log an error and continue.
				 */
				version[8] = '\0';	/* make null term */
				IssueMessage(LOG_ERR, "Version mismatch: '%s' isn't 'VERSION9'",
					    version);
			}

			/* Check to make sure it's in our local realm */
			krb_get_lrealm(realm, 1);
			if (strcmp(auth_data.prealm, realm))
			{	/* if not equal */
				DoReply(LR_ERROR, "Login failed.");
				IssueMessage(LOG_INFO, "login: realm %s not local realm.",
					    auth_data.prealm);
				return;
			}
			/*
			 * Make sure the instance is a reasonable one (null or KRBHERO).
			 * Dissallow other instances, since who knows what level of privilege
			 * a particular instance means at a given site?
			 */
			if (*auth_data.pinst && strcmp(auth_data.pinst,KRBHERO)) {	/* not null or phhero */
				DoReply(LR_ERROR, "Login failed.");
				IssueMessage(LOG_INFO, "login: %s.%s not accepted.", 
							 auth_data.pname,auth_data.pinst);
				return;
			}

			/*
			 * The user has successfully authenticated, so check
			 * for matching alias, log in and send reply.
			 *
			 * kerberos principal == our alias is auth_data.pname
			 * kerberos instance auth_data.pinst says whether it's
			 * a ph hero.
			 */
			if (User)
				FreeDir(&User);
#define WAIT_SECS	1
			if (!GonnaRead("DoKlogin"))
				/* Lock routines give their own errors */ ;
			else
			{
				long	xtime = time((long *) NULL);

				AmHero = 0;
				UserAlias = NULL;
				User = GetAliasDir(auth_data.pname);
				Unlock("DoKlogin");
				if (!User)
				{
					SleepTil(xtime + WAIT_SECS);
					DoReply(LR_ERROR, "Login failed.");
					IssueMessage(LOG_INFO, "login: alias %s does not exist.", auth_data.pname);
				} else
				{
					SleepTil(xtime + WAIT_SECS);
					DoReply(LR_OK, "%s:Hi how are you?", auth_data.pname);
					LocalUser = 1;
					AmHero = !strcmp(auth_data.pinst, KRBHERO) &&
					   *FINDVALUE(User, F_HERO);
					UserAlias = FINDVALUE(User, F_ALIAS);
					UserEnt = CurrentIndex();
					IssueMessage(LOG_INFO, "%s logged in.", UserAlias);
				}
			}
		}
	}
}

#endif /*KERBEROS*/
/*
 * identify user
 */
static void 
DoLogin(arg)
	ARG *arg;
{
	char	*me;

	me = arg->aFirst;
	arg = arg->aNext;	/* skip the command name */
	if (!arg)
		DoReply(LR_SYNTAX, "%s:no name given.", me);
	else if (arg->aType != VALUE)
		DoReply(LR_SYNTAX, "%s:argument invalid.", me);
	else if (arg->aNext)
		DoReply(LR_SYNTAX, "%s:extra arguments.", me);
	else
	{
		Challenge = strdup(RandomString(42));
		DoReply(LR_LOGIN, "%s", Challenge);
		ClaimUser = strdup(arg->aFirst);
		AmHero = 0;
		AmOpr = 0;
		State = S_E_PENDING;
		if (User)
			FreeDir(&User);
	}
}

/*
 * handle the answer to a challenge
 */
#define WAIT_SECS 1
static void 
DoAnswer(arg)
	ARG *arg;
{
	char	*me, *pass;
	long	xtime;

	me = arg->aFirst;
	arg = arg->aNext;

	if (!ClaimUser)
		DoReply(LR_SYNTAX, "%s:there is no outstanding login.", me);
	else if (!arg)
		DoReply(LR_SYNTAX, "%s:no argument given.", me);
	else if (arg->aType != VALUE)
		DoReply(LR_SYNTAX, "%s:invalid argument type.", me);
	else if (!GonnaRead("DoAnswer"))
		/* Lock routines give their own errors */ ;
	else
	{
		AmHero = 0;
		AmOpr = 0;
		UserAlias = NULL;
		xtime = time((long *) NULL);
		if (User = GetAliasDir(ClaimUser))
			pass = PasswordOf(User);
		Unlock("DoAnswer");
		if (!User)
		{
			SleepTil(xtime + WAIT_SECS);
			DoReply(LR_ERROR, "Login failed.");
			IssueMessage(LOG_INFO, "login: alias %s does not exist.", ClaimUser);
		} else if (((*me == 'a' || *me == 'A') && !UserMatch(arg->aFirst)) ||
#ifdef EMAIL_AUTH
			   ((*me == 'e' || *me == 'E') && !OkByEmail(User, arg->aFirst)) ||
#endif
#ifndef PRE_ENCRYPT
			   (((*me == 'c' || *me == 'C') && *pass) &&
			    strcmp(arg->aFirst, pass))
#else
			   (((*me == 'c' || *me == 'C') && *pass) &&
			    strncmp(crypt(arg->aFirst, arg->aFirst), pass, 13))
#endif
		    )
		{
			SleepTil(xtime + WAIT_SECS);
			if (*me != 'e' && *me != 'E')
			{
				DoReply(LR_ERROR, "Login failed.");
				IssueMessage(LOG_INFO, "Password incorrect for %s.", ClaimUser);
			}
			FreeDir(&User);
		} else
		{
			char *tpnt = FINDVALUE(User, F_HERO);

			SleepTil(xtime + WAIT_SECS);
			DoReply(LR_OK, "%s:Hi how are you?", ClaimUser);
			if (*tpnt != '\0')
			{
				if (stricmp(tpnt, "opr") == 0 ||
				    stricmp(tpnt, "oper") == 0 ||
				    stricmp(tpnt, "operator") == 0)
					AmOpr = 1;
				else
					AmHero = 1;
			}
			LocalUser = 1;
			UserAlias = FINDVALUE(User, F_ALIAS);
			UserEnt = CurrentIndex();
			IssueMessage(LOG_INFO, "%s logged in.", UserAlias);
		}
	}
	if (ClaimUser)
	{
		free(ClaimUser);
		ClaimUser = NULL;
	}
	if (Challenge)
	{
		free(Challenge);
		Challenge = NULL;
	}
	State = S_IDLE;
}

/*
 * sleep til a given time
 */
static void 
SleepTil(xtime)
	long xtime;
{
	unsigned span;

	span = xtime - time((long *) 0);
	if (0 < span && span < 10000)
		sleep(span);
}

/*
 * return the dir entry of the requested alias
 */
QDIR 
GetAliasDir(fname)
	char *fname;
{
	ARG	*Alist;
	ARG	*arg;
	long	*entry;
	QDIR	dirp;

	arg = Alist = FreshArg();

	arg->aType = COMMAND;
	arg->aFirst = strdup("query");
	arg->aNext = FreshArg();
	arg = arg->aNext;
	arg->aType = VALUE | EQUAL | VALUE2;
	arg->aFirst = strdup("alias");	/* should be alias */
	arg->aSecond = strdup(fname);
	(void) ValidQuery(Alist, C_QUERY);

	if ((entry = DoLookup(Alist)) != NULL && length(entry) == 1 &&
	    next_ent(*entry))
		getdata(&dirp);
	else
		dirp = NULL;

	if (entry)
		free((char *) entry);
	FreeArgs(Alist);

	return (dirp);
}

/*
 * de-identify the current user
 */
static void 
DoLogout(arg)
	ARG *arg;
{
	if (arg->aNext)
		DoReply(LR_SYNTAX, "argument given on logout command.");
	else if (!User)
		DoReply(LR_ERROR, "Not logged in.");
	else
	{
		FreeDir(&User);
		AmHero = 0;
		AmOpr = 0;
		UserAlias = NULL;
		UserEnt = 0;
		DoReply(LR_OK, "Ok.");
	}
}

/*
 * list fields
 */
static void 
DoFields(arg)
	ARG *arg;
{
	if (arg->aNext == NULL)
	{
		ListAllFields();
		DoReply(LR_OK, "Ok.");
	} else if (OkFields(arg->aNext))
	{
		for (arg = arg->aNext; arg; arg = arg->aNext)
			if (arg->aFD)
				ListField(arg->aFD);
		DoReply(LR_OK, "Ok.");
	} else
		DoReply(LR_SYNTAX, "Invalid field request.");
}

/*
 * List a single field
 */
static void 
ListField(fd)
	FDESC *fd;
{
	char	scratch[MAX_LEN];
	char	*cp;

	(void) sprintf(scratch, "%d:%s:max %d", fd->fdId, fd->fdName, fd->fdMax);
	cp = scratch + strlen(scratch);
	if (fd->fdIndexed)
		cp += strcpc(cp, " Indexed");
	if (fd->fdLookup)
		cp += strcpc(cp, " Lookup");
	if (fd->fdPublic)
		cp += strcpc(cp, " Public");
	if (fd->fdLocalPub)
		cp += strcpc(cp, " LocalPub");
	if (fd->fdDefault)
		cp += strcpc(cp, " Default");
	if (fd->fdAlways)
		cp += strcpc(cp, " Always");
	if (fd->fdAny)
		cp += strcpc(cp, " Any");
	if (fd->fdChange)
		cp += strcpc(cp, " Change");
	if (fd->fdSacred)
		cp += strcpc(cp, " Sacred");
	if (fd->fdTurn)
		cp += strcpc(cp, " Turn");
	if (fd->fdEncrypt)
		cp += strcpc(cp, " Encrypt");
	if (fd->fdNoPeople)
		cp += strcpc(cp, " NoPeople");
	*cp = '\0';
	DoReply(-LR_OK, scratch);
	strcpy(scratch, fd->fdHelp);
	for (cp = strtok(scratch, "\n"); cp; cp = strtok((char *) NULL, "\n"))
		DoReply(-LR_OK, "%d:%s:%s", fd->fdId, fd->fdName, cp);
}

/*
 * list all fields
 */
static void 
ListAllFields()
{
	FDESC **fd;

	for (fd = FieldDescriptors; *fd; fd++) {
		if ((*fd)->fdLocalPub && !LocalUser)
			continue;
		ListField(*fd);
	}
}

/*
 * validate arguments for field names
 */
static int 
OkFields(arg)
	ARG *arg;
{
	int	bad = 0;
	int	count = 0;
	FDESC *fd;

	for (; arg; arg = arg->aNext)
	{
		count++;
		if (arg->aType != VALUE)
		{
			DoReply(-LR_SYNTAX, "argument %d:is not a field name.", count);
			bad = 1;
		} else if (!(fd = FindFD(arg->aFirst)) || (!LocalUser && fd->fdLocalPub))
		{
			DoReply(-LR_FIELD, "%s:unknown field.", arg->aFirst);
			bad = 1;
		} else
			arg->aFD = fd;
	}
	return (!bad);
}

/*
 * delete entries
 */
static void 
DoDelete(arg)
	ARG *arg;
{
	long	*entries, *entp;
	int	haveError = 0;
	int	count;
	int	done;
	QDIR	dirp;

	if (!AmHero && !User)
	{
		DoReply(LR_NOTLOG, "Must be logged in to delete.");
		return;
	} else if (!UserCanDelete())
	{
		DoReply(LR_ERROR, "You may not delete entries.");
		IssueMessage(LOG_INFO, "%s is not authorized to delete entries.", UserAlias);
		return;
	}
	if (!ValidQuery(arg, C_DELETE))
	{
		DoReply(LR_SYNTAX, "Delete command not understood.");
		return;
	}
	if (!GonnaWrite("DoDelete"))
	{
		/* GonnaWrite will issue an error message */ ;
		return;
	}
	if ((entries = DoLookup(arg)) == NULL)
	{
		Unlock("DoDelete");
		DoReply(LR_NOMATCH, "No entries matched specifications.");
		return;
	}
	for (count = 1, done = 0, entp = entries; *entp; count++, entp++)
	{
		if (!next_ent(*entp))
		{
			DoReply(-LR_TEMP, "Internal error.");
			haveError = 1;
			continue;
		}
		getdata(&dirp);
		if (!CanDelete(dirp))
		{
			DoReply(-LR_ERROR, "%d:%s: you may not delete this entry.",
				count, FINDVALUE(dirp, F_ALIAS));
			haveError = 1;
			IssueMessage(LOG_INFO, "%s may not delete %s.",
				     UserAlias, FINDVALUE(dirp, F_ALIAS));
			FreeDir(&dirp);		/* XXX */
			continue;
		}
		/* delete the index entries */
		MakeLookup(dirp, *entp, unmake_lookup);
		FreeDir(&dirp);

		/* mark it as dead and put it out to pasture */
		SetDeleteMark();
		set_date(1);
		store_ent();
		done++;
	}

	free((char *) entries);
	Unlock("DoDelete");

	if (haveError)
		DoReply(LR_ERROR, "%d errors, %d successes on delete command.",
			count - done, done);
	else
		DoReply(LR_OK, "%d entries deleted.", done);
}

/*
 * open a temp file for output
 */
static int 
OpenTempOut()
{
	if (TFile == NULL)
	{
		TFile = strdup(TEMPFILE);
		mktemp(TFile);
	}
	if ((TempOutput = fopen(TFile, "w+")) == NULL)
	{
		free(TFile);
		TFile = NULL;
		return (0);
	}
	unlink(TFile);

	return (1);
}

/*
 * Dump a the stuff in TFile to output
 */
static void 
DumpOutput()
{
	int	c;

	rewind(TempOutput);	/* back to the beginning */
	{
		while ((c = getc(TempOutput)) != EOF)
			putc(c, Output);
	}
	fclose(TempOutput);	/* close; already unlinked */
	TempOutput = NULL;
	fflush(Output);
}

/*
 * print the current command
 */
static void 
PrintCommand(cmd, arg)
	int cmd;
	ARG *arg;
{
	fprintf(Output, "%d: %s", LR_ECHO, arg->aFirst);
	for (arg = arg->aNext;
	     arg;
	     arg = arg->aNext)
	{
		putc(' ', Output);
		if (arg->aType == RETURN)
			fputs(cmd == C_QUERY ? "return" : "make", Output);
		else
		{
			if (arg->aType & VALUE)
				fputs(arg->aFirst, Output);
#ifdef DO_TILDE
			if (arg->aType & TILD_E)
				putc('~', Output);
			else
#endif
			if (arg->aType & EQUAL)
				putc('=', Output);
			if (arg->aType & VALUE2)
				fputs(arg->aSecond, Output);
		}
	}
	putc('\n', Output);
}

/*
 * was the returned string encrypted with the appropriate password?
 */
static int 
UserMatch(string)
	char *string;
{
	char	decrypted[MAXSTR];
	char	*pw = PasswordOf(User);

	if (!*pw)
		return (0);
	crypt_start(pw);
	(void) decrypt(decrypted, string);
	return (!strcmp(decrypted, Challenge));
}

/*
 * generate a random string
 */
static char *
RandomString(byteCount)
	int byteCount;
{
	static char string[MAXSTR];
	char	*cp;
	static int seeded = 0;

	if (!seeded)
	{
		seeded = 1;
		srand((int) time((long *) NULL) ^ getpid());
	}
	for (cp = string; byteCount; cp++, byteCount--)
		*cp = (rand() & 0x3f) + 0x21;

	return (string);
}

/*
 * extract the password from a dir
 */
char *
PasswordOf(User)
	QDIR User;
{
	int	len;
	char	*password;

	/* find the user's password */
	if (!*(password = FINDVALUE(User, F_PASSWORD)))
	{
#ifdef ID_FALLBACK
		if (*(password = FINDVALUE(User, F_UNIVID)))
		{
			len = strlen(password);
			if (len > 8)
				password += len - 8;
		} else
#endif
			password = "";
	}
	return (password);
}

#ifdef EMAIL_AUTH
/*
 * figure out if a user is ok by his email address
 */
static int 
OkByEmail(User, username)
	QDIR User;
	char *username;
{
	char	buf[256];
	char	*email = FINDVALUE(User, F_EMAIL);
	char	*new, *spnt, *epnt;
	int	result;

	/*
	 * Fix up email field by omitting leading whitespace and
	 * terminating it at the first white space.
	 */
	new = spnt = strdup(email);
	while (isspace(*spnt))
		spnt++;
	epnt = spnt;
	while (*epnt && !isspace(*epnt))
		epnt++;
	*epnt = '\0';
	if (!TrustHp.h_name || !*spnt)
		result = 1;
	else
	{
		(void) sprintf(buf, "%s@%s", username, TrustHp.h_name);
		result = stricmp(spnt, buf);
	}
	free(new);
	if (result)
		DoReply(LR_NOEMAIL, "You can't login that way.");
	return (!result);
}

#endif

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strstr.c	5.1 (Berkeley) 5/15/90";
#endif /* LIBC_SCCS and not lint */

#ifdef NO_STRSTR

/*
 * Find the first occurrence of find in s.
 */
char *
strstr(s, find)
	register char *s, *find;
{
	register char c, sc;
	register int len;

	if ((c = *find++) != 0) {
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == 0)
					return ((char *) 0);
			} while (sc != c);
		} while (strncmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}
#endif
