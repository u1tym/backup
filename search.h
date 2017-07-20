#ifndef _SEARCH_H
#define _SEARCH_H

#include "fmmod.h"

PATHES *SRC_Init( void );
void    SRC_Fin( PATHES * );
int     SRC_SearchPath( PATHES **, char * );
int     SRC_SearchPathCore( PATHES **, char *, char * );
int     SRC_Disp( PATHES ** );

#endif

