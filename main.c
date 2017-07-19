#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/stat.h>
#include  <dirent.h>
#include  <time.h>

#include  "usg.h"
#include  "ulog.h"
#include  "ucomp.h"
#include  "fmmod.h"


#define DEF_EXTRA	".zip"

/** ファイル一覧作成処理部分 */
PATHES *SRC_Init( void );
void    SRC_Fin( PATHES * );
int     SRC_SearchPath( PATHES **, char * );
int     SRC_SearchPathCore( PATHES **, char *, char * );
int     SRC_Disp( PATHES ** );


/** バックアップ処理部分 */

typedef struct tag_BKUP
{
	char cMaster[ DEF_PATH_MAX ];
	char cBackup[ DEF_PATH_MAX ];
} BKUP;

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

static UL_DATA *gs_ptLog = ( UL_DATA * )NULL;

#define		ERR				"ERR"
#define		INF				"INF"
#define		DBG				"DBG"

#define		SG_FILE			"backup.conf"
#define		SG_KEY_LOG		"log_path"
#define		SG_KEY_BACKUP	"backup"
#define		SG_KEY_DO		"DO"


static int gs_iTest = 1;

int main( int iArgc, char *pcArgv[] )
{
	int   iRet;					/**< 戻り値参照用							 */
	SGTBL tSg;					/**< SG値保持用								 */

	char  *pcLogPath;			/**< ログ出力先								 */

	int   iBackupCnt;			/**< バックアップ用カウンタ					 */
	int   iBackupMax;			/**< バックアップ用カウンタ（最大数）		 */
	char  *pcBackup;			/**< バックアップ対象用文字列				 */
	char  cMaster_Tmp[ 512 ];   /**< バックアップ対象用文字列				 */
	char  cBackup_Tmp[ 512 ];   /**< バックアップ対象用文字列				 */
	BKUP  tBackup;				/**< バックアップ対象						 */

	char  *pcDo;


	/*====================*/
	/* SGファイル読み込み */
	/*====================*/

	iRet = SG_Init( SG_FILE, &tSg );
	if( iRet != 0 )
	{
		fprintf( stderr, "sg open error\n" );
		fflush( stderr );
		return 1;
	}

	pcDo = SG_GetValue( &tSg, SG_KEY_DO, 0 );
	if( pcDo != ( char * )NULL && strcmp( pcDo, "1" ) == 0 )
	{
		gs_iTest = 0;
	}


	/*==============*/
	/* ログ出力設定 */
	/*==============*/

	/* ログ出力先 */
	iRet = SG_GetCount( &tSg, SG_KEY_LOG );
	if( iRet < 1 )
	{
		fprintf( stderr, "ログ出力未指定\n" );
		fflush( stderr );
		return 1;
	}

	/* ログ出力設定 */
	pcLogPath = SG_GetValue( &tSg, SG_KEY_LOG, 0 );

	fprintf( stderr, "log-path=[%s]\n", pcLogPath );
	fflush( stderr );

	gs_ptLog  = ULOG_Open( pcLogPath );

	ULOG_SetDeny( gs_ptLog, DBG );


	/*==================*/
	/* バックアップ処理 */
	/*==================*/

	iBackupMax = SG_GetCount( &tSg, SG_KEY_BACKUP );

	ULOG_Output( gs_ptLog, INF, "backup=[%d]", iBackupMax );

	for( iBackupCnt = 0; iBackupCnt < iBackupMax; ++iBackupCnt )
	{
		pcBackup = SG_GetValue( &tSg, SG_KEY_BACKUP, iBackupCnt );


		memset( cMaster_Tmp, 0x00, sizeof( cMaster_Tmp ) );
		memset( cBackup_Tmp, 0x00, sizeof( cBackup_Tmp ) );

		SG_GetDiv( pcBackup, ',', 0, cMaster_Tmp );
		SG_GetDiv( pcBackup, ',', 1, cBackup_Tmp );

		memset( &tBackup, 0x00, sizeof( tBackup ) );

		SG_CutSp( cMaster_Tmp, tBackup.cMaster );
		SG_CutSp( cBackup_Tmp, tBackup.cBackup );

		( void )BKUP_Do( &tBackup );
	}

    return 0;
}


int BKUP_Do( BKUP *ptData )
{
	int    iRet;				/**< 戻り値参照用							 */
	PATHES *ptMaster;			/**< マスターファイル一覧					 */
	PATHES *ptBackup;			/**< バックアップファイル一覧				 */


	/*==========*/
	/* 初期処理 */
	/*==========*/

	ptMaster = ( PATHES * )NULL;
	ptBackup = ( PATHES * )NULL;

	ULOG_Output( gs_ptLog, INF, "=== DO BACKUP ===" );
	ULOG_Output( gs_ptLog, INF, "master=[%s]", ptData->cMaster );
	ULOG_Output( gs_ptLog, INF, "backup=[%s]", ptData->cBackup );

	for( ;; )
	{
		/*==========================*/
		/* マスターファイル一覧作成 */
		/*==========================*/

		ptMaster = BKUP_MakeList( ptData->cMaster );
		if( ptMaster == ( PATHES * )NULL )
		{
			break;
		}


		/*==============================*/
		/* バックアップファイル一覧作成 */
		/*==============================*/

		ptBackup = BKUP_MakeList( ptData->cBackup );
		if( ptBackup == ( PATHES * )NULL )
		{
			break;
		}


		/*======================*/
		/* バックアップ処理実行 */
		/*======================*/

		iRet = BKUP_Do_Proc( ptData, ptMaster, ptBackup );
		if( iRet != 0 )
		{
			break;
		}

		break;
	}


	/*==========*/
	/* 領域解放 */
	/*==========*/

	if( ptMaster != ( PATHES * )NULL )
	{
		FM_finish( ptMaster );
		ptMaster = ( PATHES * )NULL;
	}
	if( ptBackup != ( PATHES * )NULL )
	{
		FM_finish( ptBackup );
		ptBackup  = ( PATHES * )NULL;
	}

	return 0;
}

PATHES *BKUP_MakeList( char *pcPath )
{
	int    iRet;
	PATHES *ptRetValue;


	ULOG_Output( gs_ptLog, INF, "ファイル一覧作成 path=[%s]", pcPath );


	/*========================*/
	/* ファイル検索処理初期化 */
	/*========================*/

	ptRetValue = SRC_Init();
	if( ptRetValue == ( PATHES * )NULL )
	{
		ULOG_Output( gs_ptLog, "ERR", "SRC_Init() error" );
		ptRetValue = ( PATHES * )NULL;
		goto LABEL_END;
	}


	/*==================*/
	/* ファイル検索処理 */
	/*==================*/

	iRet = SRC_SearchPath( &ptRetValue, pcPath );
	if( iRet < 0 )
	{
		ULOG_Output( gs_ptLog, "ERR", "SRC_SearchPath() error" );

		/* 領域解放 */
		SRC_Fin( ptRetValue );

		ptRetValue = ( PATHES * )NULL;
		goto LABEL_END;
	}

	ULOG_Output( gs_ptLog, INF, "ファイル一覧作成完了" );

LABEL_END:
	return ptRetValue;
}

int BKUP_Do_Proc( BKUP *ptData, PATHES *ptMaster, PATHES *ptBackup )
{
	ULOG_Output( gs_ptLog, INF, "バックアップ処理開始" );


	/*========================*/
	/* 要更新のファイルを更新 */
	/*========================*/

	( void )BKUP_Do_Proc_Update( ptData, ptMaster, ptBackup );


	/*====================*/
	/* 不要ファイルを削除 */
	/*====================*/

	( void )BKUP_Do_Proc_DeleteFile( ptData, ptBackup );


	/*========================*/
	/* 不要ディレクトリを削除 */
	/*========================*/

	( void )BKUP_Do_Proc_DeleteDir( ptData, ptBackup );


	ULOG_Output( gs_ptLog, INF, "バックアップ処理完了" );
	return 0;
}

int BKUP_Do_Proc_Update( BKUP *ptData, PATHES *ptMaster, PATHES *ptBackup )
{
	int    iIndex;							/**< インデックス番号( MASTER )	 */

	char   cPath[ DEF_PATH_MAX ];			/**< パス名						 */
	char   cPath_Check[ DEF_PATH_MAX ];		/**< パス名（チェック用）		 */
	int    iType;							/**< 種別						 */
	time_t tTime;							/**< タイムスタンプ				 */

	int    iResult;							/**< 判定結果					 */
	char   *pcResult;						/**< 判定結果（日本語）			 */

	int    iDelFile;						/**< 要削除ファイル数			 */
	int    iDelDir;							/**< 要削除ディレクトリ数		 */


	ULOG_Output( gs_ptLog, INF, "更新分チェック処理開始" );


	/*====================================*/
	/* マスターファイル一覧を軸として処理 */
	/*====================================*/

	for( ;; )
	{
		/* マスターから、パスを1つ取得 */
		iIndex = FM_getIndex( ptMaster, FM_TYPE_ALL );

		ULOG_Output( gs_ptLog, "DBG", "index=%d", iIndex );

		if( iIndex < 0 )
		{
			/* 終了判定 */
			break;
		}

		/* パス名、種別、タイムスタンプ取得 */
		strcpy( cPath,  FM_getPathFromIndex( ptMaster, iIndex ) );
		iType = FM_getTypeFromIndex( ptMaster, iIndex );
		tTime = FM_getTimeFromIndex( ptMaster, iIndex );

		if( iType == FM_TYPE_DIR )
		{
			/* ディレクトリは、そのままチェック */
			strcpy( cPath_Check, cPath );
		}
		else if( iType == FM_TYPE_FILE )
		{
			/* ファイルは、末尾に拡張子を付加したものでチェック */
			strcpy( cPath_Check, cPath );
			strcat( cPath_Check, DEF_EXTRA );
		}

		ULOG_Output( gs_ptLog, "DBG", "存在チェック path=%s", cPath_Check );

		/* バックアップファイル一覧と比較 */
		iResult = FM_compare( ptBackup, cPath_Check, iType, tTime );

		if( iType == FM_TYPE_DIR && iResult == FM_NO_EXIST )
		{
			/* ディレクトリ作成 */
			BKUP_Mak_Dir( ptData, cPath_Check );
		}
		else if( iType == FM_TYPE_FILE
		         && ( iResult == FM_NO_EXIST || iResult == FM_EXIST_OLD ) )
		{
			/* ファイルバックアップ */
			BKUP_Upd_Fil( ptData, cPath );
		}

		FM_delPath( &ptMaster, cPath, iType );
		FM_delPath( &ptBackup, cPath_Check, iType );

	}

	ULOG_Output( gs_ptLog, INF, "更新分チェック処理完了" );
	return 0;
}

int BKUP_Do_Proc_DeleteFile( BKUP *ptData, PATHES *ptBackup )
{
	int  iIndex;
	char cPath[ DEF_PATH_MAX ];

	ULOG_Output( gs_ptLog, INF, "不要ファイルのチェック処理開始" );

	for( ;; )
	{
		/* マスターから、ファイルのパスを1つ取得 */
		iIndex = FM_getIndex( ptBackup, FM_TYPE_FILE );

		ULOG_Output( gs_ptLog, "DBG", "index=%d", iIndex );

		if( iIndex < 0 )
		{
			/* 終了判定 */
			break;
		}

		/* パス名、種別、タイムスタンプ取得 */
		strcpy( cPath,  FM_getPathFromIndex( ptBackup, iIndex ) );

		/* ファイルを削除 */
		BKUP_Del_Fil( ptData, cPath );

		FM_delPath( &ptBackup, cPath, FM_TYPE_FILE );
	}

	ULOG_Output( gs_ptLog, INF, "不要ファイルのチェック処理完了" );
	return 0;
}

int BKUP_Do_Proc_DeleteDir( BKUP *ptData, PATHES *ptBackup )
{
	int  iIndex;
	char cPath[ DEF_PATH_MAX ];


	ULOG_Output( gs_ptLog, INF, "不要ディレクトリのチェック処理開始" );

	for( ;; )
	{
		/* マスターから、ディレクトリのパスを1つ取得 */
		iIndex = FM_getIndex_Rev( ptBackup, FM_TYPE_DIR );

		ULOG_Output( gs_ptLog, "DBG", "index=%d", iIndex );

		if( iIndex < 0 )
		{
			/* 終了判定 */
			break;
		}

		/* パス名、種別、タイムスタンプ取得 */
		strcpy( cPath,  FM_getPathFromIndex( ptBackup, iIndex ) );

		/* ディレクトリを削除 */
		BKUP_Del_Dir( ptData, cPath );

		FM_delPath( &ptBackup, cPath, FM_TYPE_DIR );
	}

	ULOG_Output( gs_ptLog, INF, "不要ディレクトリのチェック処理完了" );
	return 0;
}

int BKUP_Del_Dir( BKUP *ptData, char *pcPath )
{
	char cTgtPath[ DEF_PATH_MAX ];

	sprintf( cTgtPath, "%s\\%s", ptData->cBackup, pcPath );

	if( gs_iTest == 1 )
	{
		/* テスト時は、何もしない */
		;
	}
	else
	{
		rmdir( cTgtPath );
	}

	ULOG_Output( gs_ptLog, INF, "DEL-DIR  : %s", cTgtPath );

	return 0;
}

int BKUP_Mak_Dir( BKUP *ptData, char *pcPath )
{
	char cTgtPath[ DEF_PATH_MAX ];

	sprintf( cTgtPath, "%s\\%s", ptData->cBackup, pcPath );

	if( gs_iTest == 1 )
	{
		/* テスト時は、何もしない */
		;
	}
	else
	{
		mkdir( cTgtPath );
	}

	ULOG_Output( gs_ptLog, INF, "MAK-DIR  : %s", cTgtPath );

	return 0;
}

int BKUP_Upd_Fil( BKUP *ptData, char *pcPath )
{
	char cMotoPath[ DEF_PATH_MAX ];
	char cSakiPath[ DEF_PATH_MAX ];

	sprintf( cMotoPath, "%s\\%s",     ptData->cMaster, pcPath );
	sprintf( cSakiPath, "%s\\%s%s", ptData->cBackup, pcPath, DEF_EXTRA );

	if( gs_iTest == 1 )
	{
		/* テスト時は、何もしない */
		;
	}
	else
	{
		UF_Compress( cSakiPath, cMotoPath );
	}

	ULOG_Output( gs_ptLog, INF, "UPD-FILE : %s ( <- %s )", cSakiPath, cMotoPath );

	return 0;
}

int BKUP_Del_Fil( BKUP *ptData, char *pcPath )
{
	char cTgtPath[ DEF_PATH_MAX ];

	sprintf( cTgtPath, "%s\\%s", ptData->cBackup, pcPath );

	if( gs_iTest == 1 )
	{
		/* テスト時は、何もしない */
		;
	}
	else
	{
		remove( cTgtPath );
	}

	ULOG_Output( gs_ptLog, INF, "DEL-FILE : %s", cTgtPath );

	return 0;
}





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


