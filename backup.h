#ifndef _BACKUP_H
#define _BACKUP_H

#include "fmmod.h"
#include "backuplog.h"

typedef struct tag_BKUP
{
    char cMaster[ DEF_PATH_MAX ];
    char cBackup[ DEF_PATH_MAX ];
} BKUP;

#ifdef _BACKUP_MAIN
int gs_iTest = 1;
#else
extern int gs_iTest;
#endif

int     BKUP_Do( BKUP * );
PATHES *BKUP_MakeList( char * );
int     BKUP_Do_Proc( BKUP *, PATHES *, PATHES * );
int     BKUP_Do_Proc_Update( BKUP *, PATHES *, PATHES * );
int     BKUP_Do_Proc_DeleteFile( BKUP *, PATHES * );
int     BKUP_Do_Proc_DeleteDir( BKUP *, PATHES * );

int     BKUP_Del_Dir( BKUP *, char * );
int     BKUP_Mak_Dir( BKUP *, char * );
int     BKUP_Upd_Fil( BKUP *, char * );
int     BKUP_Del_Fil( BKUP *, char * );

#endif

