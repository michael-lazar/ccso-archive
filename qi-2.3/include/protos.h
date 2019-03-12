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
 * @(#)$Id: protos.h,v 1.32 1994/03/25 15:00:26 paul Exp $
 */

#ifndef PROTOS_H
#define PROTOS_H
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#ifdef __STDC__
# include <stdlib.h>
# include <string.h>
#else /* !__STDC__ */
# include <strings.h>
#endif /* __STDC__ */
#include "bintree.h"
#include "commands.h"
#include "conf.h"
#include "qdb.h"
#include "field.h"
#include "log.h"
#include "options.h"
#include "qi.h"
#include "qiapi.h"
#ifndef __STDC__
MEM_TYPE *malloc(), *realloc();
char	*strchr(), *strrchr(), *strtok();
#endif /* !__STDC__ */
#ifdef NO_STRDUP
char	*strdup();
#endif

/* comment out if fcrypt() bombs on your host */
#ifndef crypt
# define crypt		fcrypt
#endif /* !crypt */

extern int errno;

char	*add_string __P((char *));	/* lib/strbuild.c - not used */
void	AddValue __P((char *, int));
int	AliasIsUsed __P((char *, long));
int	AllMeta __P((char *));
IDX	allocate_leaf();
int	any __P((int, char *));		/* 1 char -> int */
int	anyof __P((register char *, register char *));
char	*BadAlias __P((QDIR, char *));
void	bintree_init __P((char *));
int	blankline __P((char *));	/* lib/str.c - not used */
int	CanAddEntries();
int	CanChange __P((QDIR, FDESC *));
int	CanDelete __P((QDIR));
int	CanLookup __P((FDESC *));
int	CanSee __P((QDIR, FDESC *, int));
int	ChangeDir __P((QDIR *, FDESC *, char *));
void	cleanup __P((int));
void	close_tree();
int	countc __P((char *, int));	/* 2 char -> int, str.c - not used */
int	CurrentDate();
int	CurrentIndex();
int	dbd_init __P((char *));
int	dbi_init __P((char *));
int	decode __P((unsigned char *, unsigned char *));
int	delete __P((char *));
int	deletestr __P((char *, int));
int	dnext_ent __P((long));
void	DoAdd __P((ARG *));
void	DoChange __P((ARG *));
void	DoCommand __P((int));
void	DoHelp __P((ARG *));
long	*DoLookup __P((ARG *));
long	*do_lookup __P((char **, long *));
void	DoQuery __P((ARG *));
void	DoQuit __P((ARG *));
void	DoReply __P((int, char *, ...));
void	DoSet __P((ARG *));
int	DoSysLog __P((int));
int	encode __P((char *, char *, int));
int	ent_dead();
void	expand __P((LEAF *, int, ITEM *));
char	*FascistCheck __P((QDIR, char *, char *));
long	*find_all __P((char *));
FDESC	*FindFD __P((char *));
FDESC	*FindFDI __P((int));
int	FindField __P((QDIR, int));
long	*findstr __P((char *));
void	FreeArgs __P((ARG *));
void	free_ary __P((long **));
void	FreeDir __P((QDIR *));
ARG	*FreshArg __P(());
QDIR	GetAliasDir __P((char *));
char	**getdata __P((QDIR *));
DREC	*getdirent __P((long));
void	get_dir_head();
long	*get_dir_ptrs __P((long));
int	GetFieldConfig();
#ifdef VMS
int	GetGood __P((char *, int, int));
#else
int	GetGood __P((char *, int, FILE *));
#endif /*VMS*/
int	GetState();
void	get_tree_head();
char	*getword __P((char *, char *));	/* lib/util.c - not used */
int	GonnaRead __P((char *));
int	GonnaWrite __P((char *));
int	icopy __P((char *, char *));
long	ihash __P((char *, long, long));
void	InitializeLogging();
void	InitializeOptions();
void	init_string();
void	insert __P((char *, IDX));
long	*intersect __P((long *, long *));
int	issub __P((char *, char *));
int	IssueMessage __P((int, char *fmt, ...));
int	length __P((long *));
void	LimitHit __P((int));
char	*ltrunc __P((char *));		/* lib/str.c - not used */
void	make_lookup __P((char *, int));
char	*make_str __P((char *));
long	*merge __P((long *, long *));
void	mkargv __P((int *, char **, char *));
int	new_ent();
int	next_ent __P((long));
char	*PasswordOf __P((QDIR));
char	*phonemify __P((char *));
int	putarry __P((struct iindex *, long *, long));
int	putdata __P((char **));
void	putdirent __P((long, DREC *));
void	put_dir_head();
int	putstr __P((char *, int));
void	put_tree_head();
int	quoted __P((char *));
void	read_index __P((char *));
void	read_leaf __P((IDX, LEAF *));
int	RateAKey __P((char *));
int	RemoveSpecials __P((char *));
int	search __P((char *, LEAF *, int *));
void	set_date __P((int));
void	SetDeleteMark();
void	store_ent();
int	strcate __P((char *, char *));
int	strcpc __P((register char *, register char *));
char	*strecat __P((char *, char *)); /* lib/str.c - not used */
char	*strecpy __P((char *, char *)); /* lib/str.c - not used */
int	stricmp __P((char *, char *));
int	strincmp __P((char *, char *, int));
char	*strlcpy __P((char *, char *));
void	Unknown __P((char *));
void	Unlock __P((char *));
void	unmake_lookup __P((char *, int));
int	UserCanChange __P((QDIR));
int	UserCanDelete();
void	write_leaf __P((LEAF *));
int	ValidQuery __P((ARG *, int));

#endif	/*PROTOS_H*/
