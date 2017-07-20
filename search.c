#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>

#include  <sys/stat.h>
#include  <dirent.h>
#include  <unistd.h>
#include  <sys/types.h>

#include  "fmmod.h"
#include  "search.h"


PATHES *SRC_Init( void )
{
	return FM_init();
}

void SRC_Fin( PATHES *ptPathes )
{
	FM_finish( ptPathes );
	return;
}

int SRC_SearchPath( PATHES **pptPathes, char *pcBasePath )
{
    int  iRet;
    int  iDirCnt;
    int  iDirNum;
    int  iHWMark;
    char *pcRet;
    char cDirPath[ DEF_PATH_MAX ];

    ( void )SRC_SearchPathCore( pptPathes, pcBasePath, NULL );

	iDirNum = 0;
    for( ;; )
    {
        iDirCnt = FM_getCount( *pptPathes, FM_TYPE_DIR );
        if( iDirNum >= iDirCnt )
        {
            break;
        }

		pcRet = FM_getPathFromNum( *pptPathes, iDirNum, FM_TYPE_DIR );
        // pcRet = FM_getPath( *pptPathes, FM_TYPE_DIR );
        strcpy( cDirPath, pcRet );

        // FM_delPath( pptPathes, cDirPath, FM_TYPE_DIR );
        ++iDirNum;

        // iHWMark = FM_getHWMark( *pptPathes );

        // fprintf( stderr, "dir-cnt=%d/%d, next-path=%s\n", iDirCnt, iHWMark, cDirPath );
        // fflush( stderr );

        SRC_SearchPathCore( pptPathes, pcBasePath, cDirPath );
    }

    return 0;
}

int SRC_SearchPathCore( PATHES **pptPathes, char *pcBasePath, char *pcAddPath )
{
    int           iRet;
    int           iRetValue;
    int           iType;
    char          cSearchPath[ DEF_PATH_MAX ];
    char          cResultFullPath[ DEF_PATH_MAX ];
    char          cForAddPath[ DEF_PATH_MAX ];
    DIR           *ptDir = ( DIR * )NULL;
    struct dirent *ptDirEnt;
    struct stat   tStat;    

    /* 走査パスの設定 */
    if( pcAddPath == ( char * )NULL )
    {
        strcpy( cSearchPath, pcBasePath );
    }
    else
    {
        sprintf( cSearchPath, "%s\\%s", pcBasePath, pcAddPath );
    }

    ptDir = opendir( cSearchPath );
    if( ptDir == ( DIR * )NULL )
    {
        iRetValue = -1;
        goto LABEL_END;
    }

    for( ;; )
    {
        ptDirEnt = readdir( ptDir );
        if( ptDirEnt == ( struct dirent * )NULL )
        {
            break;
        }

        if( strcmp( ptDirEnt->d_name, "." ) == 0
            || strcmp( ptDirEnt->d_name, ".." ) == 0 )
        {
            continue;
        }

        sprintf( cResultFullPath, "%s\\%s", cSearchPath, ptDirEnt->d_name );

        iRet = stat( cResultFullPath, &tStat );
        if( iRet != 0 )
        {
            /* ファイル異常 */
            continue;
        }

        if( ( tStat.st_mode & S_IFMT ) == S_IFDIR )
        {
            /* ディレクトリ */
            iType = FM_TYPE_DIR;
        }
        else
        {
            /* ファイル */
            iType = FM_TYPE_FILE;
        }

        if( pcAddPath == ( char * )NULL )
        {
            sprintf( cForAddPath, "%s", ptDirEnt->d_name );
        }
        else
        {
            sprintf( cForAddPath, "%s\\%s", pcAddPath, ptDirEnt->d_name );
        }

        FM_addPath( pptPathes, cForAddPath, iType );
        
        FM_setAttr( pptPathes, cForAddPath, iType, tStat.st_size, tStat.st_mtime );

    }

LABEL_END:
    return 0;
}

int SRC_Disp( PATHES **pptPathes )
{
	FM_sort( pptPathes );
    FM_dispList( *pptPathes );
    return 0;
}
