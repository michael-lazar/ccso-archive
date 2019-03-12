/*
    WHO - A very simplistic PH client written in C/370, allows
          you to perform simple queries against a CSO nameserver.
          No fancy edit/login/password changes or anything like
          that - just straight queries.


        Syntax:   WHO  <name1> [<name2>]


          <name1> and/or <name2> can be used to specify first or
          last names for the 'name' field. Wild cards are OK.


          All default fields are returned.


Building:    CC WHO
             LOAD WHO
             GENMOD WHO


          Remember to access the C runtime libraries in the front-end
          EXEC!


Author:  Steve Howie, Computing and Communication Services,
                      University of Guelph
                      (email: scotty@compcen.ccs.uoguelph.ca)


Disclaimers: Do what you like with this code, just don't
             hold me or the University liable for support and/or
             damages ..
*/


#define  VM
#define  MAXLINE     256
#define  DIRECTORY   "directory.cs.uoguelph.ca"


#include <manifest.h>
#include <bsdtypes.h>
#include <bsdtime.h>
#include <in.h>
#include <socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


#include "xlate.h"


void   xvert(char *, int);
void   zapcrlf(char *);
int    more;


main(int argc, char **argv)
{
    double indx, old_indx;
    int  sock,i;
    int  lenbuf;
    char buf[MAXLINE], outbuf[MAXLINE], outbuf1[MAXLINE];
    char *stop;
    int  num_bytes;
    unsigned short port;
    struct    sockaddr_in server;
    struct    hostent  *hp;
    char      *tpr;
    char      args[256];


/* Concatenate all arguments into a string */


    strcpy(args,"");
    for ( i = 1; i<  argc; i++) {
        strcat(args, argv[i]);
        strcat(args, " ");
    }
    args[strlen(args)-1] = '\0';      /* Remove trailing blank */


/* Find directory server machine */


   hp = gethostbyname(DIRECTORY);
   if (!hp) {
      printf("** Error: Can't contact directory service ....\n");
      exit(1);
   }


/* Create socket endpoint */


   port = 105;
   server.sin_family = AF_INET;
   server.sin_port = htons(port);
   server.sin_addr.s_addr = *((unsigned long *) hp->h_addr);


   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error %d\n", errno);
        exit(0);
   }


/* Connect to it */


   if (connect(sock, &server, sizeof(server)) <0) {
        printf("Connect error %d\n", errno);
        exit(0);
   }


/* Build the query */


   printf("\nConnected to directory service ..\n");
   strcpy(buf,"ph ");
   strcat(buf, args);
   lenbuf = strlen(buf);
   xvert(buf, lenbuf);
   buf[lenbuf] = '\x0d';
   buf[lenbuf+1] = '\x0a';
   lenbuf = lenbuf + 2;


/* Send the query to the server */


   if (( num_bytes = write(sock, buf, lenbuf)) < 0) {
        printf("Error %d writing to server\n", errno);
        exit(0);
   }


/* Read response and reformat it */


   more = 1;
   puts("");
   old_indx = 0;
   while(more) {
       strcpy(buf,"");
       if (( num_bytes = readline(sock, buf, MAXLINE)) < 0) {
            printf("Error %d reading from server\n", errno);
            exit(0);
       }
       buf[num_bytes] = 0;
       zapcrlf(buf);


       if (!strncmp(buf,"200",3)) more = 0;
       if (!strncmp(buf,"501",3)) {
            more = 0;
            printf("No matches found\n");
       }
       tpr = strstr(buf, ":");
       strcpy(outbuf, ++tpr);
       if (strstr(outbuf,":") != NULL) {
          indx = strtod(outbuf, &stop);
          if (indx != old_indx) {
             puts("");
             old_indx = indx;
          }
          tpr = strstr(outbuf,":");
          strcpy(outbuf1, ++tpr);
          printf("%s\n", outbuf1);
       }
   }


/* Clean up */


   puts("");
   shutdown(sock,2);
   close(sock);
   exit(0);
}


/* Read the data from the socket without losing any */


int readline(fd, ptr, maxlen)
int fd;
char     *ptr;
int      maxlen;
{
    int n;
    int rc;
    char c;


    for (n=1; n < maxlen; n++) {
         if ( (rc = read(fd, &c, 1)) == 1) {
              c = asciitoebcdic[c];
              *ptr++ = c;
              if (c == '\x25')
                   break;
         }
         else {
            if (rc == 0) {
                 if (n == 1)
                      return(0);     /* EOF, no data read */
                 else
                      break;         /* EOF, some data was read */
             }
             else
                 return(-1);         /* error */
         }
    }
    *ptr = 0;                     /* Tack a NULL on the end */
    return(n);
}


void xvert(char *buf, int len)
{
    int i;


    for (i=0; i<len;i++)
        buf[i] = ebcdictoascii[buf[i]];
}


/* Slice out CR-LF characters */


void zapcrlf(char *inputline)
{
     char *cp;


     cp = strstr(inputline, "\r");    /* Zap CR-LF */
     if (cp != NULL)
          *cp = '\0';
     else {
          cp = strstr(inputline, "\n");
          if (cp != NULL)
               *cp = '\0';
     }


}
