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
 * 4. Neither the name of CREN, the University nor the names of its
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
static char  RcsId[] = "@(#)$Id: LoginQi.c,v 1.8 1994/03/12 04:27:19 paul Exp $";
#endif

/*
** Login and Logout functions, using the many flavors of password
**  protocols (original recipe, Kerberos, email etc.)
**
**
** LoginQi - Login to QI server, optionally prompting for username/password.
**
**	Parameters:
**		UseHost - name of Qi server host
**		ToQI - stream descriptor to write to
**		FromQI - stream descriptor to read from
**		Options - see qiapi.h/LQ_* defines
**		Username - pointer to name to login as (alias) or NULL
**		Password - pointer to password or NULL
**
**	Returns:
**		alias logged in as or NULL.
**
**	Side Effects:
**		possibly obtains and caches Kerberos tickets.
**		username/password prompts are written/read to/from stdin/out,
**		 iff Options&LQ_INTERACTIVE.
**
** (most of this code lifted out of ph 6.5)
*/

#include <syslog.h>

#ifdef __STDC__
# include <unistd.h>
# include <stdlib.h>
# include <string.h>
#else /* !__STDC__ */
# include <strings.h>
char	*malloc();
char	*getenv();
char	*strtok();
#endif /* __STDC__ */
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#ifdef SYSV
#include <fcntl.h>
#endif
#include <arpa/inet.h>
#include <pwd.h>
#include <sys/param.h>
#include <errno.h>
#include "conf.h"
#include "qiapi.h"
#ifdef KERBEROS
#include <des.h>
#include <krb.h>
#endif /* KERBEROS */

char	*getpass __P((const char *));
char	*strdup __P((const char *));

#ifndef NAMEPROMPT
# define NAMEPROMPT "Enter nameserver alias: "
#endif
#ifndef PASSPROMPT
# define PASSPROMPT "Enter nameserver password: "
#endif 
#ifndef CLIENT
# define CLIENT "ph"
#endif 
#ifndef NSSERVICE
# define	NSSERVICE	"ns"
#endif

#define MAXSTR		2048	/*max string length */

int QiDebug = 0;
static char MsgBuf[MAXSTR];	/*messages from qi*/

#ifdef KERBEROS
static int LoginKRB __P((const char *, FILE *, FILE *, int, char *, char **, char **));
#endif /*KERBEROS*/
#ifdef EMAIL_AUTH
static int LoginEmail __P((const char *, FILE *, FILE *, int, char *, char **, char **));
#endif /*EMAIL_AUTH*/
static int LoginOriginal __P((const char *, FILE *, FILE *, int, char *, char **, char **));

/*
 * try each kind of login protocol in turn 'til one succeeds or we run
 * out of choices.  To avoid reprompting for Username or Password, it
 * is the responsiblity of each routine to malloc up the result of 
 * obtaining the username/password, and the responsibility of this
 * routine to clean them up -- unless they were passed in from caller of
 * course.
 */
char *
LoginQi(UseHost, ToQI, FromQI, Options, Username, Password)
const char *UseHost;
FILE *ToQI, *FromQI;
int Options;
const char *Username, *Password;
{
  char *U = (char *)Username, *P = (char *)Password;
  static char MyAlias[MAXSTR];

  memset(MyAlias,0,sizeof MyAlias);
#ifdef EMAIL_AUTH
  if (Options&LQ_EMAIL &&
      LoginEmail(UseHost, ToQI, FromQI, Options, MyAlias, &U, &P) == LR_OK)
    goto LoggedIn;
#endif /* EMAIL_AUTH */
#ifdef KERBEROS
  if (Options&LQ_KERBEROS && 
      LoginKRB(UseHost, ToQI, FromQI, Options, MyAlias, &U, &P) == LR_OK)
    goto LoggedIn;
#endif /* KERBEROS */
  if (Options&LQ_LOGIN)
    (void) LoginOriginal(UseHost, ToQI, FromQI, Options, MyAlias, &U, &P);
 LoggedIn:
  if (!Username && U)		/* username was not passed in */
    free(U);			/* so free malloc'd string */
  if (!Password && P) {		/* ditto for password */
    memset(P,0,strlen(P));
    free(P);
  }
  fputs(MsgBuf, stdout);
  return (*MyAlias)? MyAlias : NULL;
}


/*
 * Original recipe login, based on shared secret (password) between QI
 * server and user.  If autologin is selected, .netrc is tried first.
 * If LoginQiEmailAuth is true, then email auth is attempted.
 */
static int LoginQiEmailAuth = 0; /* a dirty little secret */

static int
LoginOriginal(UseHost, ToQI, FromQI, Options, MyAlias, Up, Pp)
const char *UseHost;
FILE *ToQI, *FromQI;
int Options;
char *MyAlias, **Up, **Pp;
{
  int	code;
  char	scratch[MAXSTR];
  static void GetAutoLogin __P((char **,char **));
  
  /* if LQ_AUTO option selection and a username is not supplied,
     try getting the login info from .netrc */
  if (Options&LQ_AUTO && !*Up) { /* try autologin w/.netrc */
    GetAutoLogin(Up,Pp);
    if (QiDebug)
      fprintf(stderr,"autologin: .netrc user=%s, pass=%s\n",
	      (*Up)?*Up:"(none)", (*Pp)?*Pp:"(none)");
  }
  if (!*Up) {			/* username not supplied */
    if (!(Options&LQ_INTERACTIVE))  /* sorry, I can't ask you. */
      return (LR_ERROR);
    printf(NAMEPROMPT);	/* ask for missing alias */
    fgets(scratch, sizeof (scratch), stdin);
    scratch[strlen(scratch)] = '\0';	/* zap the \n */
    if (!*scratch)
      return (LR_ERROR);
    *Up = strdup(scratch);
  }
  if (!*Pp &&!LoginQiEmailAuth && !(Options&LQ_INTERACTIVE))
    return (LR_ERROR);		/* I can't ask your password */
  if (QiDebug)
    fprintf(stderr, "sent=login %s\n", *Up); /*send login request */
  fprintf(ToQI, "login %s\n", *Up); /*send login request */
  fflush(ToQI);
  
  for (;;)		/*read the response */
    {
      if (!GetGood(MsgBuf, MAXSTR, FromQI))
	{
	  fprintf(stderr, "Whoops--the nameserver died.\n");
	  return LR_ERROR;
	}
      code = atoi(MsgBuf);
      if (code != LR_LOGIN) /*intermediate or strange response */
	fputs(MsgBuf, stdout);
      if (code >= LR_OK)	/*final response */
	break;
    }
  
  if (code == LR_LOGIN)
    {
      if (LoginQiEmailAuth) {	/* try email login */
	char *me = getpwuid(getuid())->pw_name;
	
	if (QiDebug)
	  fprintf(stderr, "sent=email %s\n",me);
	fprintf(ToQI, "email %s\n", me);
      } else {
	if (!*Pp) {		/* password not supplied */
	  char *newp;
	  newp = getpass(PASSPROMPT);
	  *Pp = strdup(newp);
	}
	if (strlen(*Pp) > PH_PW_LEN) {
	  char *cp = &(*Pp)[PH_PW_LEN];
	  while (*cp)
	    *cp++ = '\0'; /* null out *all* the extras */
	}
	crypt_start(*Pp);
	
	/*encrypt the challenge with the password */
	MsgBuf[strlen(MsgBuf) - 1] = '\0';	/*strip linefeed */
	scratch[encryptit(scratch, (char *)strchr(MsgBuf, ':') + 1)] = '\0';
	
	/*send the encrypted text to qi */
	if (QiDebug)
	  fprintf(stderr, "sent=answer %s\n", scratch);
	fprintf(ToQI, "answer %s\n", scratch);
      }
    }
  fflush(ToQI);

  /*get the final response */
  for (;;)
    {
      if (!GetGood(MsgBuf, MAXSTR, FromQI))
	{
	  fprintf(stderr, "Whoops--the nameserver died.\n");
	  return LR_ERROR;
	}
      code = atoi(MsgBuf);
      if (code >= LR_OK)	/*final response */
	break;
    }

  if (code == LR_OK)	/*logged in */
    {
      strcpy(MyAlias, (char *)strchr(MsgBuf, ':') + 1);
      *(char *)strchr(MyAlias, ':') = '\0';
    } else
      *MyAlias = '\0';
  return (code);
}
/*
 * check .netrc to for username and password to try to login with.
 */
static void 
GetAutoLogin(alias,pw)
char **alias, **pw;		/* filled in from .netrc */
{
  FILE	*netrc;		/*the .netrc file */
  char	path[1024];	/*pathname of .netrc file */
  struct stat statbuf;	/*permissions, etc. of .netrc file */
  char	key[80], val[80];	/*line from the .netrc file */
  char	*token;		/*token (word) from the line from the .netrc file */
  static void SkipMacdef __P((FILE *));
  
  /*
   * manufacture the pathname of the user's .netrc file
   */
  sprintf(path, "%s/.netrc", getenv("HOME"));
  
  /*
   * make sure its permissions are ok
   */
  if (stat(path, &statbuf) < 0)
    return;
  if (statbuf.st_mode & 077)
    return;		/*refuse insecure files */
  
  /*
   * try to open it
   */
  if (!(netrc = fopen(path, "r")))
    return;
  
  /*
   * look for a ``machine'' named ``ph''
   */
  while (2 == fscanf(netrc, "%s %s", key, val))
    {
      if (!strcmp(key, "machine") && !strcmp(val, CLIENT))
	{
	  /*
	   * found an entry for ph.  look now for other items
	   */
	  while (2 == fscanf(netrc, "%s %s", key, val))
	    {
	      if (!strcmp(key, "machine"))	/*new machine */
		goto out;
	      else if (!strcmp(key, "login"))
		*alias = strdup(val);
	      else if (!strcmp(key, "password"))
		*pw = strdup(val);
	      else if (!strcmp(key, "macdef"))
		SkipMacdef(netrc);
	    }
	} else if (!strcmp(key, "macdef"))
	  SkipMacdef(netrc);
    }
 out:
  return;
}

/*
 * skip a macdef in the .netrc file
 */
static void 
SkipMacdef(netrc)
FILE	*netrc;
{
	int	c, wasNl;

	for (wasNl = 0; (c = getc(netrc)) != EOF; wasNl = (c == '\n'))
		if (wasNl && c == '\n')
			break;
}

#ifdef KERBEROS
/*
 * Extra-Krispy recipe, using a trusted third party with a strange name.
 */
static int
LoginKRB(UseHost, ToQI, FromQI, Options, MyAlias, Up, Pp)
const char *UseHost;
FILE *ToQI, *FromQI;
int Options;
char *MyAlias, **Up, **Pp;
{
  struct sockaddr_in sin, lsin;
  int	status;
  int	sock = fileno(ToQI);
  int	namelen;
  KTEXT_ST ticket;
  long	authopts;
  MSG_DAT	msg_data;
  CREDENTIALS cred;
  Key_schedule sched;
  static char scratch[MAXSTR];
  char	principal[ANAME_SZ];
  char	instance[INST_SZ];
  char	realm[REALM_SZ], *hrealm;
  int	code;
  char	krbtkfile[MAXPATHLEN];
  char	okrbtkfile[MAXPATHLEN];
  static char kpass[BUFSIZ];

  /* find out who I am */
  namelen = sizeof (lsin);
  if (getsockname(sock, (struct sockaddr *) & lsin, &namelen) < 0)
    {
      perror("getsockname");
      return (LR_ERROR);
    }
  
  /* find out who the other side is */
  namelen = sizeof (sin);
  if (getpeername(sock, (struct sockaddr *) & sin, &namelen) < 0)
    {
      perror("getpeername");
      return (LR_ERROR);
    }
  
  /*
   * Did the user specify a username?  Has autologin been requested?
   * If not, and if we're not logged in to kerberos, prompt for one.
   */
  if (!*Up)
    {
      struct stat dummy;
      
      if (!(Options&LQ_AUTO))	/* no user, no autologin */
	return (LR_ERROR);	/* no deal */
      if (stat(TKT_FILE, &dummy)) /* no ticket cache */
	{
	  if (!(Options&LQ_INTERACTIVE)) /* can't ask */
	    return (LR_ERROR);
	  printf(NAMEPROMPT);
	  fgets(scratch, sizeof (scratch), stdin);
	  if (!*scratch)
	    return (LR_ERROR);
	  else
	    {
	      /* zap newline*/
	      scratch[strlen(scratch) - 1] = 0;
	      *Up = strdup(scratch);
	    }
	}
    }
 /* If we're not already logged in with kerberos then do so (get a TGT).
    (NULL username at this point implies we already have a TGT). */
  if (*Up)
    {
      *principal = *instance = *realm = '\0';
      status = kname_parse(principal, instance, realm, *Up);
      if (status != KSUCCESS)
	{
	  fprintf(stderr, "%s\n", krb_err_txt[status]);
	  return LR_ERROR;
	}
      if (!*realm && krb_get_lrealm(realm, 1))
	{
	  fprintf(stderr, "Unable to get realm.\n");
	  return LR_ERROR;
	}
      /* set tkt file we'll use */
      strcpy(okrbtkfile, TKT_FILE);
      sprintf(krbtkfile, "/tmp/tkt_ph_%d", getpid());
      krb_set_tkt_string(krbtkfile);
      
      if (!*Pp) {		/* no password supplied */
	if (!(Options&LQ_INTERACTIVE))	/* I can't ask */
	  return LR_ERROR;
	if (des_read_pw_string(kpass,sizeof(kpass),PASSPROMPT,0) != 0) {
	  fprintf(stderr, "Unable to read password.\n");
	  return LR_ERROR;
	}
	*Pp = strdup(kpass);
      }
      /* login */
      status = krb_get_pw_in_tkt(principal, instance, realm,
				 "krbtgt", realm, 96, *Pp);
      if (QiDebug)
	fprintf(stderr, "%s getting kerberos ticket granting ticket.\n",
		(status == KSUCCESS)?"success":"failure");
      if (status != KSUCCESS)
	{
	  if (*Up)
	    {
	      krb_set_tkt_string(okrbtkfile);
	    }
	  return LR_ERROR;
	}
    }
  if (QiDebug)
    fprintf(stderr, "sent=klogin\n");	/* send login request */
  fprintf(ToQI, "klogin\n");	/* send login request */
  fflush(ToQI);

  for (;;)		/* read the response */
    {
      if (!GetGood(MsgBuf, MAXSTR, FromQI))
	{
	  fprintf(stderr, "Whoops--the nameserver died.\n");
	  if (*Up)
	    dest_tkt();	/* destroy temp tickets for
			   specified username */
	  memset(kpass,0,sizeof(kpass)); /* paranoia */
	  return(LR_ERROR);
	}
      code = atoi(MsgBuf);
      if (code != LR_LOGIN)	/* intermediate or strange response */
	fputs(MsgBuf, stdout);
      if (code >= LR_OK)	/* final response */
	break;
    }
  
  if (code == LR_LOGIN)
    {
      /*
       * call Kerberos library routine to obtain an authenticator,
       * pass it over the socket to the server, and obtain mutual
       * authentication.
       */
      
#ifdef KRBNSREALM
      hrealm = KRBNSREALM;
#else
      hrealm = krb_realmofhost(UseHost);
#endif
      authopts = KOPT_DO_MUTUAL;
      status = krb_sendauth(authopts, sock, &ticket,
			    NSSERVICE, UseHost, hrealm,
			    0, &msg_data, &cred,
			    sched, &lsin, &sin, "VERSION9");
      if (QiDebug)
	fprintf(stderr, "%s doing kerberos mutual authentication of %s with %s in realm %s.\n",
		(status == KSUCCESS)?"success":"failure", NSSERVICE,  UseHost, hrealm);
      if (*Up)		/* ???? */
	dest_tkt();	/* destroy special tickets as soon as
			   possible */
	  
      /* get the final response (even if mutual failed)*/
      for (;;)
	{
	  if (!GetGood(MsgBuf, MAXSTR, FromQI))
	    {
	      fprintf(stderr, "Whoops--the nameserver died.\n");
	      return LR_ERROR;
	    }
	  code = atoi(MsgBuf);
	  if (code >= LR_OK)	/* final response */
	    break;
	}
      
      if (*Up)
	{
	  krb_set_tkt_string(okrbtkfile);
	}

      if (status == KSUCCESS && code == LR_OK)	/* logged in */
	{
	  memset(kpass,0,sizeof(kpass)); /* Don't need to fall thru */
	  strcpy(MyAlias, (char *)strchr(MsgBuf, ':') + 1);
	  *(char *)strchr(MyAlias, ':') = '\0';
	} else
	  *MyAlias = '\0';
      return (code);
    }
}
#endif /*KERBEROS*/

#ifdef EMAIL_AUTH
/*
 * Bean sprout recipe, using Berkeley r-command ingredients.
 * (actually just calls LoginOriginal since I stole this code
 *  out of ph....)
 */
static int
LoginEmail(UseHost, ToQI, FromQI, Options, MyAlias, Up, Pp)
const char *UseHost;
FILE *ToQI, *FromQI;
int Options;
char *MyAlias, **Up, **Pp;
{
    int rc;
    LoginQiEmailAuth = 1;	/* set our secret internal flag */
    if (QiDebug)
      fprintf(stderr,"attempting email login.\n");
    rc = LoginOriginal(UseHost, ToQI, FromQI, Options, MyAlias, Up, Pp);
    LoginQiEmailAuth = 0;
    return rc;
}
#endif /*EMAIL_AUTH*/

/*
** LogoutQi - Logout from QI server.
**
**	Parameters:
**		ToQI - stream descriptor to write to
**		FromQI - stream descriptor to read from
**
**	Returns:
**		success(LR_OK) or failure indication
**
*/

int
LogoutQi(ToQI, FromQI)
FILE *ToQI, *FromQI;
{
  QIR *r;
  int n;

  fprintf(ToQI,"logout\n");
  fflush(ToQI);
  if ((r = ReadQi(FromQI, &n)) == NULL)
    return LR_ERROR;
  n = r->code;

  /* Accept the memory leak to simplify standalone compilation of ph */
  /* FreeQIR(r); */
  return n;
}

/*
 * get a non-comment line from a stream
 * a comment is a line beginning with a # sign
 */
int 
GetGood(str, maxc, fp)
	char	*str;		/*space to put the chars */
	int	maxc;		/*max # of chars we want */

#ifdef VMS
	int	fp;	/*stream to read them from */
{
	static char Qbuf[MAXSTR + 4] =
	{'\0'};
	static int pos =
	{0},	end =
	{0},	len =
	{0};
	char	*linp;

	for (;;)
	{
		if (pos >= len)
		{
			len = netread(fp, Qbuf, maxc);
			if (len <= 0)
				return (0);
			Qbuf[len] = '\0';
			pos = 0;
		}
		linp = strchr(Qbuf + pos, '\n'); /*find next newline char */
		if (linp == NULL)
			end = len;		/*no newline chars left */
		else
			end = linp - Qbuf;	/*convert pointer to index */

		strncpy(str, Qbuf + pos, end - pos + 1);
		*(str + end - pos + 1) = '\0';
		pos = end + 1;		/*save new position for next time */

		if (!*str)
#else
	FILE	*fp;			/*stream to read them from */
{
	errno = 0;
	for (;;)
	{
		if (! fgets(str, maxc, fp))
#endif
		{
			fputs("Oops; lost connection to server.\n", stderr);
			if (errno)
				perror("");
			exit(1);
		} else if (*str != '#')
		{
			if (QiDebug)
				fprintf(stderr, "read =%s", str);
			return (1);	/*not a comment; success! */
		}
	}
	/* NOTREACHED */
}

#ifdef NO_STRDUP
char *
strdup(str)
	const char *str;
{
	int len;
	char *copy;

	len = strlen(str) + 1;
	if (!(copy = malloc((unsigned int)len)))
		return((char *)NULL);
	memcpy(copy, str, len);
	return(copy);
}
#endif
