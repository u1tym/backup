
#define _BACKUP_MAIN

#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>

#include  <sys/types.h>
#include  <sys/stat.h>
#include  <unistd.h>

#include  "search.h"
#include  "backup.h"
#include  "ucomp.h"

#define DEF_EXTRA   ".zip"

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
#ifdef Linux
		mkdir( cTgtPath, S_IRUSR | S_IWUSR | S_IXUSR
						| S_IRGRP | S_IWGRP | S_IXGRP
						| S_IROTH | S_IWOTH | S_IXOTH );
#else
		mkdir( cTgtPath );
#endif
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


