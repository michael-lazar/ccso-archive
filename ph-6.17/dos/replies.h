/*
 * This software is Copyright (C) 1988 by Steven Dorner and the
 * University of Illinois Board of Trustees.  No warranties of any
 * kind are expressed or implied.  No support will be provided.
 * This software may not be redistributed for commercial purposes.
 * You may direct questions to nameserv@uiuc.edu
 */

#ifndef REPLIES_H
#define REPLIES_H
/*
 * Reply codes
 */
#define LR_PROGRESS	100	/* in progress */
#define LR_ECHO		101	/* echoing cmd */
#define LR_NUMRET	102	/* how many entries are being returned */

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
#define	LR_NOMATCH	501	/* no matches to request */
#define LR_TOOMANY	502	/* too many matches to request */
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
#define LR_AUTH		516	/* no authorization for request */
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
#define LR_OFFCAMPUS	590	/* remote queries not allowed */
#define LR_NOCMD	598	/* no such command */
#define LR_SYNTAX	599	/* syntax error */
#endif
