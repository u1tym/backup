#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>

#include  "usg.h"


static void _SG_Add( SGTBL *, char *, char * );
static void _SG_CutSpace( char *, char * );

/** @function
 *
 * @fn     SGファイル読み込み処理
 *         int SG_Init( char *pcPath, SGTBL *ptSg );
 *
 * @brief  SGファイルを読み込み、SGテーブルに格納する。
 *
 * @param  [I/ ] pcPath  : SGファイル
 * @param  [I/O] ptSg    : SGテーブル
 *
 * @return >=0           : 正常終了
 * @return <0            : 異常終了
 *
 */
int SG_Init( char *pcPath, SGTBL *ptSg )
{
	int  iRet;
	int  iRetValue;
	char *pcTmp;
	char cBuff[ 512 ];
	FILE *ptFile;

	char cKey[ DEF_KEY_MAX ];
	char cVal[ DEF_VAL_MAX ];
	char cKey_Conv[ DEF_KEY_MAX ];
	char cVal_Conv[ DEF_VAL_MAX ];


	ptFile = ( FILE * )NULL;


	/*====================*/
	/* パラメータチェック */
	/*====================*/

	if( pcPath == ( char * )NULL || ptSg == ( SGTBL * )NULL )
	{
		iRetValue = -1;
		goto LABEL_END;
	}


	/*========*/
	/* 初期化 */
	/*========*/

	ptSg->iMax  = 0;
	ptSg->ptRec = ( SGREC * )NULL;


	/*====================*/
	/* SGファイルオープン */
	/*====================*/

	ptFile = fopen( pcPath, "r" );
	if( ptFile == ( FILE * )NULL )
	{
		iRetValue = -2;
		goto LABEL_END;
	}


	/*==========*/
	/* 読み込み */
	/*==========*/

	for( ;; )
	{
		/* 終了判定 */
		iRet = feof( ptFile );
		if( iRet != 0 )
		{
			break;
		}

		/* 読み込み */
		memset( cBuff, 0x00, sizeof( cBuff ) );
		fgets( cBuff, sizeof( cBuff ), ptFile );

		/* 改行、コメント削除 */
		pcTmp = strstr( cBuff, "\n" );
		if( pcTmp != ( char * )NULL )
		{
			*pcTmp = '\0';
		}
		pcTmp = strstr( cBuff, "#" );
		if( pcTmp != ( char * )NULL )
		{
			*pcTmp = '\0';
		}

		if( cBuff[ 0 ] == '\0' )
		{
			continue;
		}

		/* 項目値の取得 */
		memset( cKey, 0x00, sizeof( cKey ) );
		memset( cVal, 0x00, sizeof( cVal ) );

		SG_GetDiv( cBuff, '=', 0, cKey );
		SG_GetDiv( cBuff, '=', 1, cVal );

		if( strlen( cKey ) == 0 )
		{
			continue;
		}

		memset( cKey_Conv, 0x00, sizeof( cKey_Conv ) );
		memset( cVal_Conv, 0x00, sizeof( cVal_Conv ) );

		_SG_CutSpace( cKey, cKey_Conv );
		_SG_CutSpace( cVal, cVal_Conv );
		
		_SG_Add( ptSg, cKey_Conv, cVal_Conv );
	}

	iRetValue = 0;

LABEL_END:

	if( ptFile != ( FILE * )NULL )
	{
		fclose( ptFile );
		ptFile = ( FILE * )NULL;
	}

	return iRetValue;
}

/** @function
 *
 * @fn     SGファイル件数取得処理
 *         int SG_GetCount( SGTBL *ptSg, char *pcKey );
 *
 * @brief  指定したキーのレコード数を返す。
 *
 * @param  [I/ ] ptSg    : SGテーブル
 * @param  [I/ ] pcKey   : キー
 *
 * @return >=0           : 正常終了
 * @return <0            : 異常終了
 *
 */
int SG_GetCount( SGTBL *ptSg, char *pcKey )
{
	int   iRetValue;
	int   iIndex;
	SGREC *ptRec;


	/*====================*/
	/* パラメータチェック */
	/*====================*/

	if( ptSg == ( SGTBL * )NULL || pcKey == ( char * )NULL )
	{
		iRetValue = -1;
		goto LABEL_END;
	}
	if( strlen( pcKey ) == 0 )
	{
		iRetValue = 0;
		goto LABEL_END;
	}


	/*==========*/
	/* カウント */
	/*==========*/

	iRetValue = 0;

	for( iIndex = 0; iIndex < ptSg->iMax; ++iIndex )
	{
		ptRec = ( SGREC * )&( ptSg->ptRec )[ iIndex ];

		if( strcmp( ptRec->cKey, pcKey ) != 0 )
		{
			continue;
		}

		++iRetValue;
	}

LABEL_END:
	return iRetValue;
}

/** @function
 *
 * @fn     SGファイル項目値取得処理
 *         int SG_GetValue( SGTBL *ptSg, char *pcKey, int iOrderIndex );
 *
 * @brief  指定したキーの値を返す。
 *
 * @param  [I/ ] ptSg    : SGテーブル
 * @param  [I/ ] pcKey   : キー
 * @param  [I/ ] iOrderIndex : インデックス（0～）
 *
 * @return アドレス      : 項目値
 *
 */
char *SG_GetValue( SGTBL *ptSg, char *pcKey, int iOrderIndex )
{
	int   iIndex;
	int   iCount;
	SGREC *ptRec;

	char  *pcRetValue;


	/*====================*/
	/* パラメータチェック */
	/*====================*/

	if( ptSg == ( SGTBL * )NULL || pcKey == ( char * )NULL )
	{
		pcRetValue = ( char * )NULL;
		goto LABEL_END;
	}
	if( strlen( pcKey ) == 0 )
	{
		pcRetValue = ( char * )NULL;
		goto LABEL_END;
	}


	/*======*/
	/* 検索 */
	/*======*/

	iCount     = 0;
	pcRetValue = ( char * )NULL;

	for( iIndex = 0; iIndex < ptSg->iMax; ++iIndex )
	{
		ptRec = ( SGREC * )&( ptSg->ptRec[ iIndex ] );

		if( strcmp( ptRec->cKey, pcKey ) != 0 )
		{
			continue;
		}

		if( iCount == iOrderIndex )
		{
			pcRetValue = &( ptRec->cVal[ 0 ] );
			break;
		}

		++iCount;
	}

LABEL_END:

	return pcRetValue;
}

void SG_Finish( SGTBL *ptSg )
{
	if( ptSg == ( SGTBL * )NULL )
	{
		goto LABEL_END;
	}

	if( ptSg->ptRec != ( SGREC * )NULL )
	{
		free( ptSg->ptRec );
		ptSg->ptRec = ( SGREC * )NULL;
	}

LABEL_END:
	return;
}

void SG_GetDiv( char *pcString, char cToken, int iIndex, char *pcOutput )
{
	int  iSt;
	int  iEd;
	int  iPos;
	int  iCnt;

	iSt = 0;


	/*================*/
	/* 開始地点を探す */
	/*================*/

	iPos = 0;
	iCnt = 0;
	for( iPos = 0; iCnt < iIndex && iPos < strlen( pcString ); ++iPos )
	{
		if( *( pcString + iPos ) != cToken )
		{
			continue;
		}

		++iCnt;

		if( iCnt == iIndex )
		{
			iSt = ( iPos + 1 );
			break;
		}
	}

	if( iPos >= strlen( pcString ) )
	{
		return;
	}


	/*================*/
	/* 終了地点を探す */
	/*================*/

	for( iPos = iSt; iPos < strlen( pcString ); ++iPos )
	{
		if( *( pcString + iPos ) != cToken )
		{
			continue;
		}

		iEd = iPos - 1;
		break;
	}

	if( iPos >= strlen( pcString ) )
	{
		iEd = strlen( pcString ) - 1;
	}

	/* 開始地点＞終了地点（＝デリミタが連続）の場合 */
	if( iSt > iEd )
	{
		return;
	}


	/*========*/
	/* コピー */
	/*========*/

	memcpy( pcOutput, ( pcString + iSt ), ( iEd - iSt + 1 ) );

	return;
}

void SG_CutSp( char *input, char *output )
{
	_SG_CutSpace( input, output );
	return;
}

static void _SG_Add( SGTBL *ptSg, char *pcKey, char *pcVal )
{
	SGREC *ptRec;

	if( ptSg->ptRec == ( SGREC * )NULL )
	{
		ptRec = ( SGREC * )malloc( sizeof( SGREC ) );
	}
	else
	{
		ptRec = ( SGREC * )realloc( ptSg->ptRec,
		                            sizeof( SGREC ) * ( ptSg->iMax + 1 ) );
	}
	if( ptRec == ( SGREC * )NULL )
	{
		/* 領域確保異常 */
		return;
	}

	++( ptSg->iMax );
	ptSg->ptRec = ptRec;

	ptRec = &( ptSg->ptRec )[ ptSg->iMax - 1 ];

	memset( ptRec, 0x00, sizeof( *ptRec ) );
	strcpy( ptRec->cKey, pcKey );
	strcpy( ptRec->cVal, pcVal );

	fprintf( stderr, "set key=%s, val=%s\n", ptRec->cKey, ptRec->cVal );
	fflush( stderr );

	return;
}

static void _SG_CutSpace( char *pcSrc, char *pcDst )
{
	int  iInd;
	int  iTop;
	int  iBtm;

	for( iInd = 0; iInd < strlen( pcSrc ); ++iInd )
	{
		if( *( pcSrc + iInd ) == ' ' )
		{
			continue;
		}

		iTop = iInd;
		break;
	}

	for( iInd = strlen( pcSrc ) - 1; iInd >= 0; --iInd )
	{
		if( *( pcSrc + iInd ) == ' ' )
		{
			continue;
		}

		iBtm = iInd;
		break;
	}

	memcpy( pcDst, ( pcSrc + iTop ), ( iBtm - iTop + 1 ) );

	return;
}

