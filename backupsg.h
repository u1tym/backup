#ifndef _BACKUPSG_H
#define _BACKUPSG_H

typedef struct tag_BKREC
{
	char cMaster[ 512 ];
	char cBackup[ 512 ];
} BKREC;

typedef struct tag_BKSG
{
	char  cLogPath[ 512 ];
	char  cDelimiter[ 32 ];

	int   iDoBackup;

	int   iMax;
	BKREC *ptRec;
} BKSG;

static BKSG gs_tBackupSg;

int   BKSG_Init( void );
char *BKSG_GetLogPath( void );
char *BKSG_GetDelimiter( void );
int   BKSG_GetDoBackup( void );
int   BKSG_GetBackupMax( void );
int   BKSG_GetBackupRec( char *, char *, int );

#endif

