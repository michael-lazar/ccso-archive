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
 *
 * @(#)$Id: qiapi.h,v 1.6 1994/03/11 23:29:48 paul Exp $
 */

#ifndef _QIAPI_H
#define _QIAPI_H

/* comment out if fcrypt() bombs on your host */
#ifndef crypt
# define crypt		fcrypt
#endif /* !crypt */

#ifdef NO_MEMMOVE
# define	memmove(a,b,c)	bcopy(b,a,c)
#endif

#include <stdio.h>
#include <errno.h>
#ifdef MALLOC_DEBUG
# include <malloc_dbg.h>
#endif /* MALLOC_DEBUG */

#if defined(__STDC__)
# undef __P
# define __P(x) x
#else /* !__STDC__ */
# undef const
# define const
# undef __P
# define __P(x) ()
#endif /* __STDC__ */

struct	QI_fields {
    char *value;
    int returnOK;
};
typedef struct QI_fields QIF;
#define		QIF_NULL		((QIF *) NULL)

struct	QI_response {
    int code;
    int subcode;
    int field;
    char *message;
};
typedef struct QI_response QIR;
#define		QIR_NULL	((QIR *) NULL)

/*
** BSD 4.4 systems and others using the shadow password system have
** getpass() routines that can return up to 128 characters.  To assure
** uniform behavior across all systems, qi/ph should only use the
** first 8.
*/
#define		PH_PW_LEN	8

/*
** Reply codes:
**	1XX - status
**	2XX - information
**	3XX - additional information or action needed
**	4XX - temporary errors
**	5XX - permanent errors
**	6XX - phquery specific codes
*/
#define LR_PROGRESS	100	/* in progress */
#define LR_ECHO		101	/* echoing cmd */
#define LR_NUMRET	102	/* how many entries are being returned */
#define LR_NONAME	103	/* no hostname found for IP address */

#define LR_OK		200	/* success */
#define LR_RONLY	201	/* database ready in read only mode */

#define LR_MORE		300	/* need more info */
#define LR_LOGIN	301	/* encrypt this string */

#define LR_TEMP		400	/* temporary error */
#define LR_INTERNAL	401	/* database error, possibly temporary */
#define LR_LOCK		402	/* lock not obtained within timeout period */
#define LR_COULDA_BEEN	403	/* login would have been ok but db read only */
#define LR_DOWN		475	/* database unavailable; try again later */

#define LR_ERROR	500	/* hard error; general */
#define	LR_NOMATCH	501	/* no matches to query */
#define LR_TOOMANY	502	/* too many matches to query */
#define LR_AINFO	503	/* may not see that field */
#define LR_ASEARCH	504	/* may not search on that field */
#define LR_ACHANGE	505	/* may not change field */
#define LR_NOTLOG	506	/* must be logged in */
#define LR_FIELD	507	/* field unknown */
#define LR_ABSENT	508	/* field not present in entry */
#define LR_ALIAS	509	/* requested alias is already in use */
#define LR_AENTRY	510	/* may not change entry */
#define LR_ADD		511	/* may not add entries */
#define LR_VALUE	512	/* illegal value */
#define LR_OPTION	513	/* unknown option */
#define LR_UNKNOWN	514	/* unknown command */
#define LR_NOKEY	515	/* no indexed field found in query */
#define LR_AUTH		516	/* no authorization for query */
#define LR_READONLY	517	/* operation failed; database is read-only */
#define LR_LIMIT	518	/* too many entries selected for change */
#define LR_HISTORY	519	/* history substitution failed (obsolete) */
#define LR_XCPU		520	/* too much cpu used */
#define LR_ADDONLY	521	/* addonly option set and change command
				   applied to a field with data */
#define LR_ISCRYPT	522	/* attempt to view encrypted field */
#define LR_NOANSWER	523	/* "answer" was expected but not gotten */
#define LR_BADHELP	524	/* help topics cannot contain slashes */
#define LR_NOEMAIL	525	/* email authentication failed */
#define LR_NOADDR	526	/* host name address not found in DNS */
#define LR_MISMATCH	527	/* host = gethostbyaddr(foo); foo != gethostbyname(host) */
#define LR_OFFCAMPUS	590	/* remote queries not allowed */
#define LR_NOCMD	598	/* no such command */
#define LR_SYNTAX	599	/* syntax error */

#define LR_AMBIGUOUS	600	/* ambiguous or multiple match */

/* LoginQi options */
#define LQ_AUTO		0x01	/* attempt auto login */
#define LQ_INTERACTIVE	0x02	/* allowed to be interactive if needed. */
#define LQ_KERBEROS	0x10	/* use Kerberos login, if available */
#define LQ_EMAIL	0x20	/* use email login, if available */
#define LQ_LOGIN	0x40	/* use conventional QI login */
#define LQ_ALL	(LQ_KERBEROS|LQ_EMAIL|LQ_LOGIN)	/* try all of them */

struct QiReplyCode {
	int	key;
	char	*value;
};

extern int errno, sys_nerr;
extern char *sys_errlist[];

extern QIF *QiFields;		/* FieldOps.c: fields available from qi */
extern int QiHighKey;		/* FieldOps.c: highest field key */
extern int QiDebug;		/* FieldOps.c: print debugging info */

void	crypt_start __P((char *));
char	*decrypt __P((char *, char *));
void	decrypt_end();
int	encryptit __P((char *, char *));
char	*fcrypt __P((const char *, const char *));
void	CloseQi __P((FILE *, FILE *));
void	FreeQIR __P((QIR *));
int	FieldValue __P((char *));
QIF	*GetFields __P((FILE *, FILE *));
int	OpenQi __P((char *, FILE **, FILE **));
QIR	*PickField __P((QIR *, int));
QIR	*ReadQi __P((FILE *, int *));
char 	*LoginQi __P((const char *, FILE *, FILE *, int, const char *, const char *));
int	LogoutQi __P((FILE *, FILE *));
#endif /* !_QIAPI_H */
