#ifndef _BACKUPLOG_H
#define _BACKUPLOG_H

#include  "ulog.h"

#ifdef _MAIN
#define	BACKUPLOG_EXTERN
#else
#define	BACKUPLOG_EXTERN	extern
#endif

BACKUPLOG_EXTERN UL_DATA *gs_ptLog;

#define     ERR             "ERR"
#define     INF             "INF"
#define     DBG             "DBG"

#endif

