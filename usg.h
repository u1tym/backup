#ifndef _USG_H
#define _USG_H

#define	DEF_KEY_MAX			( 128 )
#define	DEF_VAL_MAX			( 512 )

typedef struct tag_sgrec
{
	char cKey[ DEF_KEY_MAX ];
	char cVal[ DEF_VAL_MAX ];
} SGREC;

typedef struct tag_sgtbl
{
	int   iMax;
	SGREC *ptRec;
} SGTBL;

int   SG_Init( char *, SGTBL * );
int   SG_GetCount( SGTBL *, char * );
char *SG_GetValue( SGTBL *, char *, int );
void  SG_Finish( SGTBL * );

void  SG_GetDiv( char *, char, int, char * );
void  SG_CutSp( char *, char * );

#endif
