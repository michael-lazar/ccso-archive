#ifndef CONF_H
#define CONF_H
#define KERBEROS
#define EMAIL_AUTH
#define NO_READ_LOCK
#define OWNER "nameserv"
#define GROUP "nameserv"
extern char *OkAddrs[];
extern char *LocalAddrs[];
extern char *Strings[];
#define TEMPFILE (Strings[1])
#define NOHELP (Strings[3])
#define HELPDIR (Strings[5])
#define MAILBOX (Strings[7])
#define ADMIN (Strings[9])
#define MAILDOMAIN (Strings[11])
#define DATABASE (Strings[13])
#define RUNDIR (Strings[15])
#define PASSW (Strings[17])
#define MAILFIELD (Strings[19])
#define NATIVESUBDIR (Strings[21])
#define PERSONLIMIT 25
#define NICHARS 32
#define DOVRSIZE 400
#define NOCHARS 1024
#define MAX_KEY_LEN 16
#define MEM_TYPE char
#define DRECSIZE 400
#define LOG_QILOG LOG_LOCAL0
#define CPU_LIMIT 20
#define SIG_TYPE void
extern char *Database;
#endif
