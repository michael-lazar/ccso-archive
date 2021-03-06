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
static char  RcsId[] = "@(#)$Id: FieldOps.c,v 1.6 1994/03/11 22:41:14 paul Exp $";
#endif

/*
 * various field related operators.
 */

#include <syslog.h>
#ifdef __STDC__
# include <string.h>
#else /* !__STDC__ */
# include <strings.h>
#endif /* __STDC__ */
#include "qiapi.h"

#define		MAXSTR		256
#define		CHNULL		('\0')
#define		CPNULL		((char *) NULL)

int QiHighKey = 0;
QIF *QiFields = QIF_NULL;

static char *xstrsep();

/*
** GetFields -- Create and fill QIF array using information from qi server.
**
**	Parameters:
**		ToQi - stream descriptor to write to
**		FromQi - stream descriptor to read from
**
**	Returns:
**		pointer to newly created field array
**
**	Side Effects:
**		allocates space for Fields and fills it.
*/

#define		FINC		20

QIF *
GetFields(ToQi, FromQi)
    FILE *ToQi, *FromQi;
{
    QIF	*Fields;
    int code = 0;
    char Temp[MAXSTR], *p, *val, **pp;
    int limit, num;

    if (QiDebug)
	fprintf(stderr, "querying for fields\r\n");

    /* Ask for available fields */
    fprintf(ToQi, "fields\n");
    (void) fflush(ToQi);

    if ((Fields = (QIF *) malloc((FINC * sizeof(QIF)))) == NULL) {
	if (QiDebug)
	    fprintf(stderr, "GetFields: malloc(%d): %s\r\n",
	      FINC * sizeof(QIF), sys_errlist[errno]);
	syslog(LOG_ERR, "GetFields: malloc(%d): %m", FINC * sizeof(QIF));
	return(QIF_NULL);
    }
    memset((char *) Fields, (char)0, (FINC * sizeof(QIF)));
    memset(Temp, (char)0, MAXSTR);
    limit = FINC - 1;

    /* Read and assign */
    do {
	if (fgets(Temp, MAXSTR-1, FromQi) == CPNULL || *Temp == CHNULL) {
	    if (QiDebug)
	        fprintf(stderr, "GetFields: premature EOF\r\n");
	    syslog(LOG_ERR, "GetFields: premature EOF");
	    return(QIF_NULL);
	}
	p = Temp;
	code = ((val = xstrsep(&p, ":")) == CPNULL) ? 0 : atoi(val);
	if (code != -(LR_OK) || code > 0) {
	    if (code == LR_PROGRESS)
		continue;
	    if (code == LR_OK)
		break;
	    if (QiDebug)
		fprintf(stderr, "GetFields: error %s\r\n", Temp);
	    syslog (LOG_ERR, "GetFields: error %s", Temp);
	    return(QIF_NULL);
	}
	num = 0;
	if (p == CPNULL)
	    break;
	num = ((val = xstrsep(&p, ":")) == CPNULL) ? 0 : atoi(val);
	if (p == CPNULL || (val = xstrsep(&p, ":")) == CPNULL) {
	    num = 0;
	    break;
	}
	if (num > QiHighKey)
	    QiHighKey = num;
	if (num > limit) {
	    int curval = limit + 1;

	    Fields = (QIF *)realloc((char *)Fields, ((num + FINC)*sizeof(QIF)));
	    if (Fields == QIF_NULL) {
		if (QiDebug)
		    fprintf(stderr, "GetFields: realloc(%d): %s\r\n",
		      (num + FINC) * sizeof(QIF), sys_errlist[errno]);
		syslog(LOG_ERR, "GetFields: realloc(%d): %m",
		  (num + FINC) * sizeof(QIF));
		return(QIF_NULL);
	    }
	    limit = num + FINC;
	    memset((char *)(Fields+curval), (char)0, (limit - curval)*sizeof(QIF));
	    limit--;
	}
	if ((Fields+num)->value && strcmp((Fields+num)->value, val)) {
	    if (QiDebug)
	        fprintf(stderr, "Fields \"%s\" and \"%s\" have same key (%d)\r\n",
		  (Fields+num)->value, val, num);
	    syslog (LOG_ERR, "Fields \"%s\" and \"%s\" have same key (%d)",
	      (Fields+num)->value, val, num);
	    return(QIF_NULL);
	}
	else
	    (Fields+num)->value = strdup(val);
    } while (1);
    if (QiDebug)
	for (num = 0; num <= QiHighKey; num++)
	    if ((Fields+num)->value)
		fprintf(stderr, "%d\t%s\t%d\n", num, (Fields+num)->value,
		    (Fields+num)->returnOK);
    return(Fields);
}
/*
**  PickField -- Find the QI_response with the named field
**
**	Cycle through a chain of QI_response's looking for one with the
**	named field.  Return a pointer to that one or NULL if not present.
**	Assumes that the last QI_response.code > 0.
**
**	Parameters:
**		qp -- QI_response chain pointer
**		field -- QI field to search for
**
**	Returns:
**		pointer to located QI_response or NULL if not found
**
**	Side Effects:
**		None
*/

QIR *
PickField (qp, field)
    QIR	*qp;
    int	field;
{
    if (qp == QIR_NULL)
	return(qp);
    do {
	if (qp->field == field)
	    return (qp);
    } while ((qp++)->code < 0);
    return (QIR_NULL);
}
/*
** FieldValue -- Locate argument in QiFields[] and return integer value
**
**	Parameters:
**		field -- character string to locate in Fields[]
**
**	Returns:
**		integer value of field or -1 if not found.
**
**	Side Effects:
**		none
*/

int
FieldValue (field)
    char *field;
{
    QIF *QFp = QiFields;

    /* Guard against stupid mistakes (so they show up somewhere else?) */
    if (QiFields == QIF_NULL || field == CPNULL || *field == CHNULL)
	return(-1);

    /* Replace this with a binary search if profiling peaks here.  XXX */
    while (QFp <= (QiFields + QiHighKey)) {
	if (QFp->value && !strcmp(field, QFp->value))
	    break;
	QFp++;
    }
    if (QFp > (QiFields + QiHighKey))
	return(-1);
    return(QFp-QiFields);
}
/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
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

/*
 * Get next token from string *stringp, where tokens are nonempty
 * strings separated by characters from delim.  
 *
 * Writes NULs into the string at *stringp to end tokens.
 * delim need not remain constant from call to call.
 * On return, *stringp points past the last NUL written (if there might
 * be further tokens), or is NULL (if there are definitely no more tokens).
 *
 * If *stringp is NULL, strtoken returns NULL.
 */
static char *
xstrsep(stringp, delim)
	register char **stringp;
	register char *delim;
{
	register char *s;
	register char *spanp;
	register int c, sc;
	char *tok;

	if ((s = *stringp) == NULL)
		return (NULL);
	for (tok = s;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}
