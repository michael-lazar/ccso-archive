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
static char  RcsId[] = "@(#)$Id: dbi.c,v 1.27 1994/03/21 13:14:24 paul Exp $";
#endif

#include "protos.h"

#define ESC '\033'		/* the special account entry marker */

#define ASIZE 32		/* granule size for long array allocation */


static int debug = 0;
static int ixfd;			/* index file desciptor */
static int ovfd;			/* overflow file desciptor */
static long IDXSIZE;
static long OVRSIZE;
static long oarry[NOPTRS];

static int getient __P((register char *, struct iindex *));
static long *getrecptrs __P((struct iindex *));
static int getx __P((long *));
static long *i_oread __P((long, long *));
static int i_owrite __P((long *, long));
static int i_read __P((struct iindex *, long));
static int i_write __P((struct iindex *, long));
static int ovrnew();
static void printarry __P((long *));
static long *un_dupi __P((long *));

extern struct dirhead DirHead;
extern int Have_head;

/*
 * This routine takes a string and searches for it in the inverted index. It
 * returns a pointer to an array of long which are the directory
 * entries that contain this string.
 */

long *
findstr(str)
	char *str;
{
	struct iindex x;

	if (getient(str, &x) <= 0)
		return (0);

	return (un_dupi(getrecptrs(&x)));
}

/*
 * this returns an array of record pointers (long) gleaned from the
 * index entry idx.
 */

static long *
getrecptrs(idx)
	struct iindex *idx;
{
	register int src, dest;
	int	array_size;
	long	*array;

	array_size = (NIPTRS + 1) * PTRSIZE;	/* One extra space for
			       * benefit of putstr() */
	array = (long *) malloc(array_size);
	memset((void *) array, (char) 0, array_size);

	/* copy recptrs into array */
	src = getx((long *) idx);
	dest = 0;
	while (src < NIPTRS)
		array[dest++] = idx->i_recptrs[src++];
	dest--;

	/* read overflow index blocks into array */
	while (array[dest])
	{
		array = (long *) realloc(array, array_size += NOPTRS * PTRSIZE);
		(void) i_oread(array[dest], &array[dest]);
		dest += NOPTRS - 1;
	}

	return (array);
}


/*
 * finds the first integer after the end of the string in buffer str.
 */

static int 
getx(str)
	long *str;
{
	/* please hide this stuff away somewhere, it's gross */
	int	i;

	for (i = 0;; i++)
	{
		if ((str[i] & 0xff) == 0 || (str[i] & 0xff00) == 0 ||
		    (str[i] & 0xff0000) == 0 || (str[i] & 0xff000000) == 0)
		{
			i++;
			break;
		}
	}
	return (i);		/* finally have index into str as if it were
				 * an array of long */
}

/*
 * This routine returns the index into the index file of string str and puts
 * a copy of the entry in xbuf. It returns 0 on failure and the negative of
 * the index of the first null entry it finds. It uses a rehashing strategy
 * for searching.
 */

int	TryCount;
int	TrySum;
int	TryTimes;

static int 
getient(str, x)
	char *str;
	struct iindex *x;
{
	long	strtix = 0;
	long	emptynode = 0;
	char	scratch[256];

	(void) strlcpy(scratch, str);
	scratch[MAX_KEY_LEN] = 0;	/* index only first MAX_KEY_LEN chars */

	TrySum += TryCount;
	TryTimes++;
	TryCount = 0;
	do
	{
		strtix = ihash(scratch, strtix, IDXSIZE);

		/*
		 * ihash() should never return an entry that goes beyond EOF,
		 * so i_read() should never fail due to that. Correct???
		 * (Otherwise, we've got a third possibility).
		 * Rick McCarty <mccarty@add.itg.ti.com>
		 */

		if (!i_read(x, strtix))
			return (0);
		if ((x->i_string)[0] == 0)
		{		/* pristine node */
			if (Have_head && TryCount < NHASH)
				DirHead.hashes[TryCount]++;
			return (emptynode ? -emptynode : -strtix);
		} else if ((x->i_string)[0] == EMPTY)
		{		/* was once a part of a hash chain */
			if (!emptynode)
				emptynode = strtix;

			/*
			 * Not sure about this but this may be a lurking bug.
			 * What is "DirHead" used for.
			 * Rick McCarty <mccarty@add.itg.ti.com>
			 */
			if (Have_head && TryCount < NHASH)
				DirHead.hashes[TryCount]++;
		} else if (!stricmp(scratch, x->i_string))
		{
			if (Have_head && TryCount < NHASH)
				DirHead.hashes[TryCount]++;
			return (strtix);
		}
	}
	while (TryCount++ < MAXTRIES);

	/*
	 * Here's where I think we can take care of the other case.  We
	 * have traveled down the maximum number of tries, but did not find
	 * a match.  We did, however, encounter an empty node along the way.
	 * Return it.   Rick McCarty <mccarty@add.itg.ti.com>
	 */
	if (emptynode)
		return (-emptynode);

	/*
	 * Well, we couldn't find it. This isn't a good way to end. It means
	 * that the index is getting full. These should be logged somehow to
	 * let someone know they should increase idx file size.
	 */
	IssueMessage(LOG_WARNING, "MAXTRIES exceeded on string \"%s\"\n", str);
	return (0);
}

/*
 * This routine takes a string and searches for it in the inverted index. If
 * it is in the index it adds the record number "recnum" to the array of
 * directory entries associated with the string. If the string is not found,
 * it is added.
 *
 * putstr() indexes on record.
 * putstrarry() indexes a list of them in one shot.
 */

int	WordsIndexed = 0;
int	MaxIdx = 0;
int	DoTree = 1;		/* bing the bintree stuff */

int
putstr(str, recnum)
	char *str;
	int recnum;
{
	long arry[2];

	arry[0] = recnum;
	arry[1] = 0;
	return putstrarry(str, arry);
}

int 
putstrarry(str, newarry)
	char *str;
	long *newarry;		/* newarry is assumed to be sorted! */
{
	struct iindex x;
	long	iloc, i, j, save, ovrflo, recnum;
	long	*arry;
	char	scratch[MAX_KEY_LEN + 1];

	if (strlen(str) < 2)
		return (1);	/* ignore, but pretend success */
	strncpy(scratch, str, sizeof (scratch));
	scratch[MAX_KEY_LEN] = 0;	/* truncate */
	iloc = getient(scratch, &x);
	if (debug)
		IssueMessage(LOG_DEBUG, "putstr: iloc= %ld", iloc);
	if (iloc == 0)
		return (0);
	if (iloc < 0)
	{			/* this is an empty entry */
		WordsIndexed++;
		(void) strlcpy(x.i_string, scratch);
		/* copy the first few recnums into the primary index entry. */
		for (i = getx((long *) &x), j = 0; i < (NIPTRS - 1) && newarry[j]; i++, j++)
		  x.i_recptrs[i] = newarry[j];
		if (newarry[j]) { /* overflow: more recnums to go into the .iov file */
			if ((ovrflo = x.i_recptrs[i] = ovrnew()) < 0) {
				IssueMessage(LOG_WARNING,"putstrarry: ovrnew failed.");
				return(0);
			}
			while (ovrflo) {
				for (i = 0; i < (NOPTRS-1) && newarry[j]; i++,j++)
				  oarry[i] = newarry[j];
				if (newarry[j]) {
					if ((oarry[i] = ovrnew()) < 0) {
						IssueMessage(LOG_WARNING,"putstrarry: ovrnew failed.");
						return(0);
					}
				} else {
					oarry[i] = oarry[NOPTRS-1] = 0;
				}
				if (!i_owrite(oarry,ovrflo)) {
					  IssueMessage(LOG_WARNING,"putstrarry: i_owrite failed.");
					  return(0);
				}
				ovrflo = oarry[NOPTRS-1];
			}
		} else {	/* no overlow: everything fits in the .idx entry */
			x.i_recptrs[i] = 0;
			x.i_recptrs[NIPTRS - 1] = 0;	/* make sure don't think we ovrflo */
		}
		if (i_write(&x, -iloc) == 0)
		{
			IssueMessage(LOG_WARNING, "putstr i_write failed.");
			return (0);
		}
		if (DoTree && *str != ESC)
			insert(x.i_string, -iloc);	/* update the sequence file */
	} else
	{			/* got one; merge arry and newarry */
		arry = getrecptrs(&x);
		if (debug)
			printarry(arry);

		/* sort the new entries in, return if already there */
		if (newarry[2])	/* if the new array is more than 1 element, realloc to make room */
		  arry = (long *) realloc(arry, (length(arry) + length(newarry) + 1) * PTRSIZE);
		if (arry == NULL) {
			IssueMessage(LOG_WARNING, "Coulnd't realloc for string \"%s\"\n", str);
			return(0);
		}
		for (j = 0; recnum=newarry[j]; j++) { /* gross hack way to merge these in */
			for (i = 0; recnum; i++)
			  {

				  if (arry[i] > recnum || arry[i] == 0)
				    {
					    save = arry[i];
					    arry[i] = recnum;
					    recnum = save;
				    }
			  }
			arry[i] = 0;	/* zero terminate */
			if (i > MaxIdx)
			  MaxIdx = i;
		}
		if (debug)
			printarry(arry);
		if (!putarry(&x, arry, iloc))
		{
			IssueMessage(LOG_WARNING, "putstrarry: putarry failed.");
			free(arry);
			return (0);
		}
		free(arry);
	}
	return (1);
}

int 
putarry(x, arry, iloc)
	struct iindex *x;
	long *arry, iloc;
{
	int	i, j;
	long	had_ovrflo, ovrdo, size, oloc;
	long	*addr;

	j = getx((long *) x);

	addr = x->i_recptrs;
	size = NIPTRS - 1;
	had_ovrflo = addr[size];
	addr[size] = 0;
	ovrdo = 0;		/* whether we are filling an ovrflo block */

	for (i = 0; arry[i]; i++, j++)
	{			/* write out array */

		if (j >= size)
		{		/* get next ovrflo block */

			if (!had_ovrflo)
				if ((had_ovrflo = ovrnew()) < 0)
				{
					IssueMessage(LOG_WARNING, "putarry: ovrnew failed.");
					return (0);
				}
			addr[size] = had_ovrflo;

			if (ovrdo)
			{	/* write out last ovrflo block */
				if (i_owrite(addr, oloc) == 0)	/* ignore lint--this is ok */
				{
					IssueMessage(LOG_WARNING, "putarry i_owrite failed");
					return (0);
				}
			}
			addr = i_oread(had_ovrflo, oarry);
			j = 0;
			ovrdo++;
			size = NOPTRS - 1;
			oloc = had_ovrflo;	/* new location in ovr file */
			had_ovrflo = addr[size];
		}
		addr[j] = arry[i];
	}
	addr[j] = 0;		/* zero terminate */

	/* write out index entry */
	if (i_write(x, iloc) == 0)
	{
		IssueMessage(LOG_WARNING, "putarry i_write failed.");
		return (0);
	}
	/* write out any partial ovrflo blocks */
	addr[size] = 0;		/* zero ovrflo block ptr */
	if (ovrdo)
	{
		if (i_owrite(addr, oloc) < 0)
		{
			IssueMessage(LOG_WARNING, "putarry i_owrite failed");
			return (0);
		}
	}
	return (1);
}

static void 
printarry(arry)
	long *arry;
{
	printf("array entries : ");
	while (*arry)
		printf(" %ld ", *arry++);
	printf("\n");
}

/*
 * This routine will delete the record recnum from the inverted index entry
 * for str. If this is the only record that str is found in, then str in
 * removed from the index.
 */

int 
deletestr(str, recnum)
	char *str;
	int recnum;
{
	struct iindex x;
	long	iloc, offset, i;
	long	*arry;
	char	scratch[MAX_KEY_LEN + 1];

	strncpy(scratch, str, MAX_KEY_LEN);
	scratch[MAX_KEY_LEN] = 0;	/* index only first MAX_KEY_LEN chars */
	str = scratch;

	if ((iloc = getient(str, &x)) <= 0)
	{
		IssueMessage(LOG_WARNING, "deletestr: no index ent for \"%s\"\n", str);
		return (0);
	}
	arry = getrecptrs(&x);

	offset = 0;

	/* get rid of the record pointer */
	for (i = 0; arry[i]; i++)
	{
		if (arry[i] == recnum)	/* is it there? */
			offset = 1;
		arry[i] = arry[i + offset];
	}
	if (!offset)
	{
		IssueMessage(LOG_WARNING, "deletestr: no recptr for \"%s\",%ld\n", str, recnum);
		free(arry);
		return (0);
	}
	/* get rid of the entry */
	if (arry[0] == 0)
	{			/* empty, remove the entry */
		x.i_string[0] = EMPTY;
		x.i_recptrs[NIPTRS - 1] = 0;
		if (i_write(&x, iloc) == 0)
		{
			IssueMessage(LOG_WARNING, "deletestr i_write failed.");
			return (0);
		}
		if (*str != ESC)
			if (delete(str) == 0)
			{
				IssueMessage(LOG_WARNING, "deletestr: delete failed.");
			}
	} else if (!putarry(&x, arry, iloc))
	{
		free(arry);
		IssueMessage(LOG_WARNING, "deletestr: putarry failed.");
		return (0);
	}
	free(arry);
	return (1);
}

/*
 * This routine returns an array of all long common to the two
 * arrays given as argument. It assumes that the arrays are sorted.
 */

long *
intersect(ary0, ary1)
	long *ary0, *ary1;
{
	long	*result;
	int	i, j, k, size0, size1;

	/* all right, who's the smallest ? */
	for (size0 = 0; ary0[size0]; size0++) ;
	for (size1 = 0; ary1[size1]; size1++) ;

	/* get result array, can't run out (right?) */

	if (size0 > size1)
		i = (((size0 * PTRSIZE) / ASIZE + 1) * ASIZE);
	else
		i = (((size1 * PTRSIZE) / ASIZE + 1) * ASIZE);
	result = (long *) malloc((unsigned) i);

	for (i = 0, j = 0, k = 0; i < size0; i++)
	{			/* foreach element in ary0 */
		while (ary1[j] < ary0[i] && ++j < size1) ;	/* no match */
		if (j >= size1)
			break;
		if (ary0[i] == ary1[j])
		{
			result[k++] = ary0[i];
			continue;
		}
	}
	result[k] = 0;
	return (result);

}

static long *
un_dupi(ary)
	long *ary;
{
	register long *ptr, *orig;

	orig = ary;

	if (!ary[0] || !ary[1])
		return (ary);

	for (ptr = ary++; *ary; ary++)
	{
		if (*ptr != *ary)
			*++ptr = *ary;
	}
	*++ptr = 0;

	return (orig);
}

/*
 * This routine opens the index and index-overflow files file is the prefix
 * of the database name.
 */

int 
dbi_init(file)
	char *file;
{
	struct iindex *x;
	char	idxname[100], iovname[100];
	static int firstTime = 1;

	/* make file names */
	strcpy(idxname, file);
	strcat(idxname, ".idx");
	strcpy(iovname, file);
	strcat(iovname, ".iov");

	if (firstTime && (ixfd = open(idxname, 2)) < 0)
	{
		IssueMessage(LOG_INFO, "dbi_init %s %s", idxname, strerror(errno));
		return (0);
	}
	IDXSIZE = lseek(ixfd, 0L, 2) / sizeof *x;

	if (firstTime && (ovfd = open(iovname, 2)) < 0)
	{
		IssueMessage(LOG_INFO, "dbi_init %s %s", iovname, strerror(errno));
		return (0);
	}
	OVRSIZE = lseek(ovfd, 0L, 2) / (PTRSIZE * NOPTRS);
	if (OVRSIZE == 0)
		if (ovrnew() < 0)
		{
			IssueMessage(LOG_WARNING, "dbi_init: overnew failed.");
			cleanup(-1);
		}
	if (debug)
		printf("init: IDXSIZE= %ld, OVRSIZE= %ld\n", IDXSIZE, OVRSIZE);
	firstTime = 0;
	return (1);
}

static int 
i_write(x, iloc)
	struct iindex *x;
	long iloc;
{
	if (lseek(ixfd, iloc * (sizeof *x), 0) != iloc * (sizeof *x))
	{
		IssueMessage(LOG_INFO, "i_write %s", strerror(errno));
		return (0);
	}
	if (write(ixfd, x, sizeof *x) < 0)
	{
		IssueMessage(LOG_WARNING, "i_write %ld, %s", iloc, strerror(errno));
		return (0);
	}
	return (1);
}

static int 
i_read(x, iloc)
	struct iindex *x;
	long iloc;
{
	if (lseek(ixfd, iloc * (sizeof *x), 0) != iloc * (sizeof *x))
	{
		IssueMessage(LOG_INFO, "i_read(lseek) %s", strerror(errno));
		return (0);
	}
	if (read(ixfd, x, sizeof *x) == -1)
	{
		IssueMessage(LOG_INFO, "i_read(read) %s", strerror(errno));
		return (0);
	}
	return (1);
}

static int 
i_owrite(x, ioloc)
	long *x, ioloc;
{
	if (lseek(ovfd, ioloc * PTRSIZE * NOPTRS, 0) != ioloc * PTRSIZE * NOPTRS)
	{
		IssueMessage(LOG_INFO, "i_owrite(lseek) %ld %s", ioloc, strerror(errno));
		return (0);
	}
	if (write(ovfd, x, PTRSIZE * NOPTRS) < 0)
	{
		IssueMessage(LOG_WARNING, "i_owrite(write) %ld: %s", ioloc, strerror(errno));
		return (0);
	}
	return (1);
}

/*
 * this routine fetches the overflow record indexed by oloc out of the
 * overflow file. It returns a pointer to a static area with the data in it,
 * therefore it can't be used recursively without copying the data out.
 */

static long *
i_oread(oloc, arry)
	long oloc, *arry;
{

	if (lseek(ovfd, (PTRSIZE * NOPTRS) * oloc, 0) < 0)
	{			/* get upset */
		IssueMessage(LOG_INFO, "i_oread: bad seek (%ld)\n", oloc);
		abort();
		exit(1);
	}
	if (read(ovfd, arry, PTRSIZE * NOPTRS) != PTRSIZE * NOPTRS)
	{			/* get upset */
		IssueMessage(LOG_INFO, "i_oread: bad read (%ld)\n", oloc);
		abort();
		exit(1);
	}
	return (arry);
}

static int 
ovrnew()
{
	char	i;

	if (lseek(ovfd, (PTRSIZE * NOPTRS * (OVRSIZE + 1)) - 1, 0) < 0)
	{
		IssueMessage(LOG_WARNING, "ovrnew lseek %s", strerror(errno));
		return (-1);
	} else if (write(ovfd, &i, 1) < 0)
	{
		IssueMessage(LOG_WARNING, "overnew write %s", strerror(errno));
		return (-1);
	}
	return (OVRSIZE++);
}

long *
get_dir_ptrs(iloc)
	long iloc;
{
	struct iindex x;

	if (i_read(&x, iloc) == 0)
	{
		return (NULL);
	}
	return (un_dupi(getrecptrs(&x)));
}
