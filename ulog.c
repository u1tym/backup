/**************************************************************************//**
 * @file  fmmod.c
 * @brief パス管理機能
 *****************************************************************************/

#define _UL_MAIN

#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <time.h>
#include  <stdarg.h>

#include  "ulog.h"

static int _UL_IsOutput( UL_DATA *, char * );

/**
 *
 * @fn ログファイルオープン処理
 *
 * @brief  ログファイルをオープンしファイルポインタを返す。
 *
 * @param  ( pcPath ) ログファイルのパス
 *
 * @return ファイルポインタ
 *
 */
UL_DATA *UL_LogOpen( char *pcPath )
{
	int     iRet;			/**< 戻り値参照用								 */
	char    *pcMode;		/**< ファイルオープンモード						 */
	FILE    *ptFile;		/**< ファイルポインタ							 */
	UL_DATA *ptRetValue;	/**< 戻り値										 */


	/*==================================*/
	/* ログファイル有無によるモード設定 */
	/*==================================*/

	pcMode = "a";

	iRet = access( pcPath, W_OK );
	if( iRet != 0 )
	{
		pcMode = "w";
	}


	/*======================*/
	/* ログファイルオープン */
	/*======================*/

	ptFile = fopen( pcPath, pcMode );

	if( ptFile == ( FILE * )NULL )
	{
		ptRetValue = ( UL_DATA * )NULL;
		goto LABEL_END;
	}


	/*==========*/
	/* 領域確保 */
	/*==========*/

	ptRetValue = ( UL_DATA * )malloc( sizeof( UL_DATA ) );
	if( ptRetValue == ( UL_DATA * )NULL )
	{
		ptRetValue = ( UL_DATA * )NULL;
		goto LABEL_END;
	}

	ptRetValue->ptFile = ptFile;
	ptRetValue->pcDeny = ( char * )NULL;

	g_pcFile = "system";
	g_iLine  = 0;
	UL_LogOutput( ptRetValue, "SYS", "*** log start ***" );


LABEL_END:
	return ptRetValue;
}


void UL_LogOutput( UL_DATA *ptData, char *pcLevel, char *pcFormat, ... )
{
	int       iRet;					/**< 戻り値参照用						 */
	char      cMessage[ 256 ];		/**< ログ出力文字列						 */
	char      cNow[ 64 ];			/**< タイムスタンプ（文字列）			 */
	time_t    lNow;					/**< タイムスタンプ（time_t型）			 */
	struct tm *ptNow;				/**< タイムスタンプ（構造体型）			 */
	FILE      *ptFile;				/**< ファイルポインタ					 */
	va_list   tArgs;				/**< 可変引数整形用						 */


	/*====================*/
	/* パラメータチェック */
	/*====================*/

	if( ptData == ( UL_DATA * )NULL )
	{
		return;
	}

	ptFile = ptData->ptFile;

	if( ptFile == ( FILE * )NULL )
	{
		return;
	}


	/*==================*/
	/* 出力要否チェック */
	/*==================*/

	iRet = _UL_IsOutput( ptData, pcLevel );
	if( iRet != 1 )
	{
		return;
	}


	/*==============*/
	/* 現在時刻取得 */
	/*==============*/

	lNow = time( NULL );
	ptNow = localtime( &lNow );
	strftime( cNow, sizeof( cNow ), "%Y/%m/%d %H:%M:%S", ptNow );


	/*====================*/
	/* 出力メッセージ整形 */
	/*====================*/

	va_start( tArgs, pcFormat );
	vsnprintf( cMessage, sizeof( cMessage ), pcFormat, tArgs );
	va_end( tArgs );


	/*======*/
	/* 出力 */
	/*======*/
	fprintf( ptFile, "[%s] %s [%-16.16s:%4d] %s\n", pcLevel, cNow, g_pcFile, g_iLine, cMessage );
	fflush( ptFile );

	return;
}

void UL_LogClose( UL_DATA *ptData )
{
	FILE      *ptFile;				/**< ファイルポインタ					 */

	/*====================*/
	/* パラメータチェック */
	/*====================*/

	if( ptData == ( UL_DATA * )NULL )
	{
		return;
	}

	ptFile = ptData->ptFile;

	if( ptFile == ( FILE * )NULL )
	{
		return;
	}


	/*======================*/
	/* ログファイルクローズ */
	/*======================*/

	fclose( ptFile );


	return;
}

void UL_SetDeny( UL_DATA *ptData, char *pcLevel )
{
	char *pcTmp;
	char cCheckToken[ 32 ];

	if( ptData == ( UL_DATA * )NULL )
	{
		return;
	}

	sprintf( cCheckToken, "%s,", pcLevel );

	if( ptData->pcDeny == ( char * )NULL )
	{
		pcTmp = ( char * )malloc( strlen( cCheckToken ) + 1 );
		if( pcTmp == ( char * )NULL )
		{
			return;
		}

		ptData->pcDeny = pcTmp;

		ptData->pcDeny[0] = '\0';
	}
	else
	{
		pcTmp = ( char * )realloc( ptData->pcDeny,
		                           strlen( ptData->pcDeny )
		                           + strlen( cCheckToken )
		                           + 1 );
		if( pcTmp == ( char * )NULL )
		{
			return;
		}

		ptData->pcDeny = pcTmp;
	}

	strcat( ptData->pcDeny, cCheckToken );

	return;
}

/**
 * @return 1 : 出力する
 * @return 0 : 出力しない
 */
static int _UL_IsOutput( UL_DATA *ptData, char *pcLevel )
{
	int  iRetValue;
	char *pcRet;
	char cCheckToken[ 32 ];

	if( ptData->pcDeny == ( char * )NULL )
	{
		iRetValue = 1;
		goto LABEL_END;
	};

	sprintf( cCheckToken, "%s,", pcLevel );

	iRetValue = 1;

	pcRet = strstr( ptData->pcDeny, cCheckToken );
	if( pcRet != ( char * )NULL )
	{
		iRetValue = 0;
	};

LABEL_END:
	return iRetValue;
}



