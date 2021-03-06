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
static char  RcsId[] = "@(#)$Id: ssnid.c,v 1.8 1994/03/12 00:59:25 paul Exp $";
#endif

/*
 * replaces ssn fields with id fields, using id database
 */

#include <stdio.h>
#include "protos.h"

char * FindId __P((char *));
char * AssignId __P((char *));

#define BUF_SIZE    8192

main(argc, argv)
	int argc;
	char **argv;
{
	char	buffer[BUF_SIZE];
	char	*ssn1, *ssn2;
	char	*id;
	int	found = 0, assigned = 0, skipped = 0;
	int	err;
	short	assignOk = 0;
	int	count = 0;

	argc--, argv++;
	if (argc >= 1 && !strcmp(argv[0], "-a"))
	{
		argc--, argv++;
		assignOk = 1;
	}
	if (argc != 1)
	{
		fprintf(stderr, "Usage: ssnid [-a] ssndb\n");
		exit(1);
	}
	if (err = IdInit(argv[0]))
	{
		perror(argv[0]);
		exit(1);
	}
	ssn1 = buffer + 2;	/* ^5:X, where X is the beginning of SSN */
	while (gets(buffer) != NULL)
	{
		if (++count % 500 == 0)
			fprintf(stderr, "%d\r", count);
		ssn2 = (char *)strchr(ssn1, '\t');	/* find end of ssn */
		if (!ssn2)
			continue;	/* skip */

		*ssn2 = '\0';
		if (id = FindId(ssn1))
			found++;
		else if (assignOk)
		{
			id = AssignId(ssn1);
			assigned++;
		} else
		{
			id = "<none>";
			skipped++;
		}
		printf("5:%s\t%s\n", id, ssn2 + 1);
	}
	fprintf(stderr, "Found %d, assigned %d, skipped %d.\n", found, assigned, skipped);
	IdDone();
	exit(0);
}
