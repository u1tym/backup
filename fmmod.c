/**************************************************************************//**
 * @file  fmmod.c
 * @brief パス管理機能
 *****************************************************************************/

#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>

#include  "ulog.h"
#include  "fmmod.h"

#define FM_ADD_COUNT  ( 10000 )

static int    _FM_Search( PATHES *, char *, int );
static int    _FM_Add( PATHES **, char *, int );
static int    _FM_Del( PATHES **, char *, int );
static int    _FM_Sort_Core( const void *, const void * );
static PATHES *_FM_Copy( PATHES * );

/* ログ出力用 === ここから === */
#define FM_NFN  "NFN"	/* 通常関数（数回しか呼び出されない）				 */
#define FM_CFN  "CFN"	/* CORE関数（何回も呼び出される）					 */
#define FM_BFN	"STC"	/* 内部関数											 */

#define FM_NTC	"NTC"	/* 通知												 */
#define FM_ERR  "ERR"	/* 処理異常発生										 */

static int     gs_iFirst = 1;
static UL_DATA *gs_ptLog = ( UL_DATA * )NULL;
/* ログ出力用 === ここまで === */


/**
 *
 * @fn 初期化処理
 *
 * @brief  ファイルパス格納領域を確保し、管理用のポインタを返す。
 *
 * @param  なし
 * @return 管理用ポインタ
 *
 */
PATHES *FM_init( void )
{
	PATHES *ptArea;


	/*====================*/
	/* 初回用ログ設定処理 */
	/*====================*/

	if( gs_iFirst == 1 )
	{
		gs_iFirst = 0;

		gs_ptLog = ULOG_Open( "c:\\tmp\\fmlog.txt" );

		ULOG_SetDeny( gs_ptLog, FM_CFN );
		ULOG_SetDeny( gs_ptLog, FM_BFN );
	}


	/*==========*/
	/* 処理開始 */
	/*==========*/

	ULOG_Output( gs_ptLog, FM_NFN, "FN-START" );


	/*==========*/
	/* 領域確保 */
	/*==========*/
	ptArea = ( PATHES * )malloc( sizeof( PATHES ) );
	if( ptArea == ( PATHES * )NULL )
	{
		/* 領域確保異常 */

		ULOG_Output( gs_ptLog, FM_ERR, "malloc error" );
		return ( PATHES * )NULL;
	}

	/* 初期化処理 */
    ptArea->iHWMark = 0;
    ptArea->ptPath  = ( PATH * )NULL;


	/*==========*/
	/* 処理終了 */
	/*==========*/

	ULOG_Output( gs_ptLog, FM_NFN, "FN-END ret=0x%x", ptArea );

    return ptArea;
}

/**
 *
 * @fn 終了処理
 *
 * @brief  ファイルパス格納領域を解放する。
 *
 * @param  ( ptArea ) FM_init()で取得した管理用ポインタ
 * @return なし
 *
 */
void FM_finish( PATHES *ptArea )
{
	/*==========*/
	/* 処理開始 */
	/*==========*/

	ULOG_Output( gs_ptLog, FM_NFN, "FN-START" );


	/*==========*/
	/* 領域解放 */
	/*==========*/

	if( ptArea->ptPath != ( PATH * )NULL )
	{
		free( ptArea->ptPath );
		ptArea->ptPath = ( PATH * )NULL;
	}

	ptArea->iHWMark = 0;

	free( ptArea );


	/*==========*/
	/* 処理終了 */
	/*==========*/

	ULOG_Output( gs_ptLog, FM_NFN, "FN-END" );
	return;
}

/**
 *
 * @fn 確保領域レコード数取得処理
 *
 * @brief  ファイルパス格納領域として確保したレコード数を返す。
 *
 * @param  ( ptArea ) FM_init()で取得した管理用ポインタ
 * @return 確保レコード数
 *
 */
int  FM_getHWMark( PATHES *ptArea )
{
    return ptArea->iHWMark;
}

/**
 *
 * @fn ファイルパス追加処理
 *
 * @brief  指定されたファイルパスを追加する。
 *
 * @param  ( pptArea ) FM_init()で取得した管理用ポインタのアドレス
 * @param  ( pcPath )  追加するパス
 * @param  ( iType )   追加するパスの種類
 *
 * @return  0 : 正常終了
 * @return  1 : 正常終了（同一パス追加済み）
 * @return -9 : 異常終了（領域拡張不正など）
 *
 */
int FM_addPath( PATHES **pptArea, char *pcPath, int iType )
{
    int  iRetValue;
    int  iRet;
    int  iIndex;


	/*==========*/
	/* 処理開始 */
	/*==========*/

	ULOG_Output( gs_ptLog, FM_CFN, "FN-START" );


    /* 登録済みチェック */
    iIndex = _FM_Search( *pptArea, pcPath, iType );
    if( iIndex >= 0 )
    {
        iRetValue = 1;
        goto LABEL_END;
    }

    /* 登録 */
    iRet = _FM_Add( pptArea, pcPath, iType );
    if( iRet < 0 )
    {
        iRetValue = -9;
        goto LABEL_END;
    }
    iRetValue = 0;


	/*==========*/
	/* 処理終了 */
	/*==========*/

LABEL_END:

	ULOG_Output( gs_ptLog, FM_CFN, "FN-END ret=%d", iRetValue );
    return iRetValue;
}

/**
 *
 * @fn ファイルパス削除処理
 *
 * @brief  指定されたファイルパスを削除する。
 *
 * @param  ( pptArea ) FM_init()で取得した管理用ポインタのアドレス
 * @param  ( pcPath )  削除するパス
 * @param  ( iType )   削除するパスの種類
 *
 * @return  0 : 正常終了
 * @return  1 : 正常終了（同一パスが存在しない）
 *
 */
int FM_delPath( PATHES **pptArea, char *pcPath, int iType )
{
    int  iRet;
    int  iRetValue;

	/*==========*/
	/* 処理開始 */
	/*==========*/

	ULOG_Output( gs_ptLog, FM_CFN, "FN-START" );


    /* 削除 */
    iRet = _FM_Del( pptArea, pcPath, iType );
    if( iRet != 0 )
    {
		ULOG_Output( gs_ptLog, FM_NFN,
		             "not found path=[%s] type=[%d]",
		             pcPath, iType );

        iRetValue = 1;
        goto LABEL_END;
    }

    iRetValue = 0;


	/*==========*/
	/* 処理終了 */
	/*==========*/

LABEL_END:

	ULOG_Output( gs_ptLog, FM_CFN, "FN-END ret=%d", iRetValue );
    return iRetValue;
}

/**
 *
 * @fn パス数取得処理
 *
 * @brief  指定された種類のパス数を返す。
 *
 * @param  ( ptArea ) FM_init()で取得した管理用ポインタ
 * @param  ( iType )  パスの種類
 *
 * @return  >=0 : パスの数
 *
 */
int FM_getCount( PATHES *ptArea, int iType )
{
    int  iIndex;
    int  iRetValue;
    PATH *ptRec;

    iRetValue = 0;
    for( iIndex = 0; iIndex < ptArea->iHWMark; ++iIndex )
    {
        ptRec = ( PATH * )&( ptArea->ptPath[ iIndex ] );

        if( ptRec->iType == iType )
        {
            ++iRetValue;
        }
    }

    return iRetValue;
}

/**
 *
 * @fn パス取得処理
 *
 * @brief  指定された種類のパスを1つ返す。
 *
 * @param  ( ptArea ) FM_init()で取得した管理用ポインタ
 * @param  ( iType )  パスの種類
 *
 * @return パス（charのアドレス）
 *
 */
char *FM_getPath( PATHES *ptArea, int iType )
{
	int  iIndex;
	char *pcRetValue;
	PATH *ptRec;

	pcRetValue = ( char * )NULL;

	iIndex = FM_getIndex( ptArea, iType );

	if( iIndex >= 0 )
	{
		ptRec = ( PATH * )&( ptArea->ptPath[ iIndex ] );
		pcRetValue = ptRec->cName;
	}

    return pcRetValue;
}

int FM_getIndex( PATHES *ptArea, int iType )
{
	int  iIndex;
	int  iRetValue;
    PATH *ptRec;

	iRetValue = -1;

	for( iIndex = 0; iIndex < ptArea->iHWMark; ++iIndex )
	{
		ptRec = ( PATH * )&( ptArea->ptPath[ iIndex ] );

		if( ptRec->cName[ 0 ] == '\0' )
		{
			continue;
		}

		if( iType >= 0 && ptRec->iType != iType )
		{
			continue;
		}

		iRetValue = iIndex;
		goto LABEL_END;
	}

LABEL_END:
	return iRetValue;
}

int FM_getIndex_Rev( PATHES *ptArea, int iType )
{
	int  iIndex;
	int  iRetValue;
    PATH *ptRec;

	iRetValue = -1;

	for( iIndex = ptArea->iHWMark - 1; iIndex >= 0; --iIndex )
	{
		ptRec = ( PATH * )&( ptArea->ptPath[ iIndex ] );

		if( ptRec->cName[ 0 ] == '\0' )
		{
			continue;
		}

		if( iType >= 0 && ptRec->iType != iType )
		{
			continue;
		}

		iRetValue = iIndex;
		goto LABEL_END;
	}

LABEL_END:
	return iRetValue;
}




char *FM_getPathFromIndex( PATHES *ptArea, int iIndex )
{
	char *pcRetValue;
	PATH *ptRec;

	ptRec = ( PATH * )&( ptArea->ptPath[ iIndex ] );
	pcRetValue = ptRec->cName;

	return pcRetValue;
}

time_t FM_getTimeFromIndex( PATHES *ptArea, int iIndex )
{
	time_t tRetValue;
	PATH   *ptRec;

	ptRec = ( PATH * )&( ptArea->ptPath[ iIndex ] );
	tRetValue = ptRec->tTime;

	return tRetValue;
}

int FM_getTypeFromIndex( PATHES *ptArea, int iIndex )
{
	int  iRetValue;
	PATH *ptRec;

	ptRec = ( PATH * )&( ptArea->ptPath[ iIndex ] );
	iRetValue = ptRec->iType;

	return iRetValue;
}

char *FM_getPathFromNum( PATHES *ptArea, int iNum, int iType )
{
	int  iIndex;
	char *pcRetValue;

	iIndex = FM_getIndexFromNum( ptArea, iNum, iType );
	if( iIndex < 0 )
	{
		pcRetValue = ( char * )NULL;
		goto LABEL_END;
	}

	pcRetValue = FM_getPathFromIndex( ptArea, iIndex );

LABEL_END:
	return pcRetValue;
}

int FM_getIndexFromNum( PATHES *ptArea, int iNum, int iType )
{
	int  iIndex;
	int  iCount;
	int  iRetValue;
	PATH *ptRec;

	iRetValue = -1;

	for( iIndex = 0, iCount = 0; iIndex < ptArea->iHWMark; ++iIndex )
	{
		ptRec = ( PATH * )&( ptArea->ptPath[ iIndex ] );

		if( ptRec->iType != iType )
		{
			continue;
		}

		if( iCount == iNum )
		{
			iRetValue = iIndex;
			goto LABEL_END;
		}

		++iCount;
	}

LABEL_END:
	return iRetValue;
}


/**
 *
 * @fn パスの属性設定処理
 *
 * @brief  指定されたパスに、指定された属性を付加する。
 *
 * @param  ( pptArea ) FM_init()で取得した管理用ポインタのアドレス
 * @param  ( pcPath )  パス
 * @param  ( lSize )   サイズ
 * @param  ( tTime )   タイムスタンプ
 *
 * @return  0 : 正常終了
 * @return -1 : 異常終了（指定されたパスが存在しない）
 *
 */
int  FM_setAttr( PATHES **pptArea, char *pcPath, int iType, size_t lSize, time_t tTime )
{
    int  iRet;
    int  iRetValue;
    int  iIndex;
    PATH *ptRec;

    iRet = _FM_Search( *pptArea, pcPath, iType );
    if( iRet < 0 )
    {
        iRetValue = -1;
        goto LABEL_END;
    }
    iIndex = iRet;

    ptRec = ( PATH * )&( ( *pptArea )->ptPath[ iIndex ] );

    ptRec->lSize = lSize;
    ptRec->tTime = tTime;

    iRetValue = 0;

LABEL_END:
    return iRetValue;
}

/**
 *
 * @fn パス表示処理（デバック用）
 *
 * @brief  全パスを表示する。
 *
 * @param  ( ptArea ) FM_init()で取得した管理用ポインタ
 *
 * @return なし
 *
 */
void FM_dispList( PATHES *ptArea )
{
    int  iIndex;
    int  iCount;
    char cKind[ 16 ];
    PATH *ptRec;

    iCount = 0;
    for( iIndex = 0; iIndex < ptArea->iHWMark; ++iIndex )
    {
        ptRec = ( PATH * )&( ptArea->ptPath[ iIndex ] );

        if( ptRec->cName[ 0 ] == '\0' )
        {
            continue;
        }

        sprintf( cKind, "[-]" );
        if( ptRec->iType == FM_TYPE_DIR )
        {
            sprintf( cKind, "[D]" );
        }
        else if( ptRec->iType == FM_TYPE_FILE )
        {
            sprintf( cKind, "[F]" );
        }

        struct tm *ptTime;
        char      cTime[ 64 ];
        ptTime = localtime( &( ptRec->tTime ) );

        memset( cTime, 0x00, sizeof( cTime ) );
        strftime( cTime, sizeof( cTime ), "%Y/%m/%d %H:%M:%S", ptTime );

        fprintf( stdout, "(%d) %s,\"%s\",%s,%d\n", iIndex, cKind, ptRec->cName, cTime, ptRec->lSize );
        ++iCount;
    }
    fprintf( stdout, "count=[%d]\n", iCount );
    fflush( stdout );


    return;
}

/**
 *
 * @fn ソート処理
 *
 * @brief  パス名でソートする。
 *
 * @param  ( pptArea ) FM_init()で取得した管理用ポインタのアドレス
 *
 * @return 0 : 正常終了
 *
 */
int FM_sort( PATHES **pptArea )
{
	qsort( ( *pptArea )->ptPath,
	       ( *pptArea )->iHWMark, sizeof( PATH ),
	       _FM_Sort_Core );

	return 0;
}

/**
 *
 * @fn  存在チェック処理
 *
 * @brief 指定したファイルが存在するか否かを確認する。
 *
 * @param ( ptArea ) FM_init()で取得した管理用ポインタ
 * @param ( pcPath ) チェックするファイルのパス
 * @param ( tTime )  チェックするファイルのタイムスタンプ
 *
 * @return FM_NO_EXIST    : 存在しない
 * @return FM_EXIST_NEW   : 存在し、ptArea内のものの方が新しい
 * @return FM_EXIST_EQUAL : 存在し、タイムスタンプも同一
 * @return FM_EXIST_OLD   : 存在し、ptArea内のものの方が古い
 *
 */
int FM_compare( PATHES *ptArea, char *pcPath, int iType, time_t tTime )
{
	int  iIndex;			/**< インデックス番号							 */
	int  iRetValue;			/**< 戻り値設定用								 */
	PATH *ptRec;			/**< 該当レコード								 */


	/*==================*/
	/* 対象ファイル走査 */
	/*==================*/

	iIndex = _FM_Search( ptArea, pcPath, iType );
	if( iIndex < 0 )
	{
		/* 該当なし */
		iRetValue = FM_NO_EXIST;
		goto LABEL_END;
	}

	/* 該当するファイルが存在 */

	ptRec = ( PATH * )&( ptArea->ptPath[ iIndex ] );

	if( tTime < ptRec->tTime )
	{
		/* ptArea内のものの方が新しい */
		iRetValue = FM_EXIST_NEW;
		goto LABEL_END;
	}
	else if( tTime == ptRec->tTime )
	{
		/* タイムスタンプが同一 */
		iRetValue = FM_EXIST_EQUAL;
		goto LABEL_END;
	}
	else
	{
		/* ptArea内のものの方が古い */
		iRetValue = FM_EXIST_OLD;
		goto LABEL_END;
	}

LABEL_END:
	return iRetValue;
}

/**
 *
 * @fn  不足ファイル抽出処理
 *      PATHES *FM_lack( PATHES *ptArea1, PATHES *ptArea2 );
 *
 * @brief ptArea1内にあって、ptArea2内にないファイルを抽出する。
 *
 * @param ( ptArea1 ) FM_init()で取得した管理用ポインタ
 * @param ( ptArea2 ) FM_init()で取得した管理用ポインタ
 *
 * @return addr : 正常終了
 * @return NULL : 異常終了
 *
 * @notice 抽出数は、getCount()を使用すること。
 * @notice FM_finish()での領域解放をわすれないこと。
 *
 */
PATHES *FM_lack( PATHES *ptArea1, PATHES *ptArea2 )
{
	int    iRet;
	char   *pcFile;
	PATHES *ptDriven;
	PATHES *ptResult;


	ptResult = ( PATHES * )NULL;
	ptDriven = ( PATHES * )NULL;

	/*==========================*/
	/* 軸となる方のコピーを作成 */
	/*==========================*/

	ptDriven = _FM_Copy( ptArea1 );
	if( ptDriven == ( PATHES * )NULL )
	{
		goto LABEL_END;
	}


	/*======================*/
	/* 結果格納用領域を作成 */
	/*======================*/

	ptResult = ( PATHES * )malloc( sizeof( PATHES ) );
	if( ptResult == ( PATHES * )NULL )
	{
		goto LABEL_END;
	}


	for( ;; )
	{
		pcFile = FM_getPath( ptDriven, FM_TYPE_FILE );
		if( pcFile == ( char * )NULL )
		{
			break;
		}

		/* ptArea2内に、当該ファイルがあるかないかチェック */
		iRet = _FM_Search( ptArea2, pcFile, FM_TYPE_FILE );
		if( iRet < 0 )
		{
			/* 無い -> 結果に追加 */

			FM_addPath( &ptResult, pcFile, FM_TYPE_FILE );
		}

		/* 駆動軸から削除 */
		FM_delPath( &ptDriven, pcFile, FM_TYPE_FILE );
	}

		
LABEL_END:
	if( ptDriven != ( PATHES * )NULL )
	{
		FM_finish( ptDriven );
		ptDriven = ( PATHES * )NULL;
	}

	return ptResult;
}

/*===========================================================================*/

static int _FM_Sort_Core( const void *pvRec1, const void *pvRec2 )
{
	PATH *ptPath1;
	PATH *ptPath2;

	ptPath1 = ( PATH * )pvRec1;
	ptPath2 = ( PATH * )pvRec2;

	if( ptPath1->cName[ 0 ] == '\0' )
	{
		return 1;
	}
	else if( ptPath2->cName[ 0 ] == '\0' )
	{
		return -1;
	}

	return strcmp(  ptPath1->cName, ptPath2->cName );
}

static int _FM_Search( PATHES *ptArea, char *pcPath, int iType )
{
    int  iIndex;
    int  iRetValue;
    PATH *ptRec;

    iRetValue = -1;
    for( iIndex = 0; iIndex < ptArea->iHWMark; ++iIndex )
    {
        ptRec = ( PATH * )&( ptArea->ptPath[ iIndex ] );

        if( strcmp( ptRec->cName, pcPath ) == 0
            && ptRec->iType == iType )
        {
            iRetValue = iIndex;
            goto LABEL_END;
        }
    }

LABEL_END:
    return iRetValue;
}

static int _FM_Add( PATHES **pptArea, char *pcPath, int iType )
{
    int  iIndex;
    int  iRetValue;
    PATH *ptEmpty;
    PATH *ptRec;

    iRetValue = -1;

    if( ( *pptArea )->iHWMark == 0 )
    {
        ptRec = ( PATH * )malloc( sizeof( PATH ) * FM_ADD_COUNT );
        if( ptRec == ( PATH * )NULL )
        {
            iRetValue = -9;
            goto LABEL_END;
        }

        memset( ptRec, 0x00, sizeof( PATH ) * FM_ADD_COUNT );

        ( *pptArea )->iHWMark = FM_ADD_COUNT;
        ( *pptArea )->ptPath  = ptRec;
    }

    ptEmpty = ( PATH * )NULL;
    for( ;; )
    {
        
        for( iIndex = 0; iIndex < ( *pptArea )->iHWMark; ++iIndex )
        {
            ptRec = ( PATH * )&( ( *pptArea )->ptPath[ iIndex ] );

            if( ptRec->cName[ 0 ] == '\0' )
            {
                ptEmpty = ptRec;
                break;
            }
        }

        if( ptEmpty != ( PATH * )NULL )
        {
            break;
        }

        ptRec = ( PATH * )realloc( ( *pptArea )->ptPath,
                                   sizeof( PATH ) * ( ( *pptArea )->iHWMark + FM_ADD_COUNT ) );
        if( ptRec == ( PATH * )NULL )
        {
            iRetValue = -9;
            goto LABEL_END;
        }

        memset( &( ptRec[ ( *pptArea )->iHWMark ] ), 0x00, sizeof( PATH ) * FM_ADD_COUNT );

        ( *pptArea )->iHWMark += FM_ADD_COUNT;
        ( *pptArea )->ptPath   = ptRec;

    }


    strcpy( ptEmpty->cName, pcPath );
    ptEmpty->iType = iType;

    iRetValue = 0;    
    
LABEL_END:
    return iRetValue;
}

static int _FM_Del( PATHES **pptArea, char *pcPath, int iType )
{
    int  iRet;
    int  iRetValue;
    int  iDelIndex;
    PATH *ptDelRec;

    iRetValue = -1;

    iRet = _FM_Search( *pptArea, pcPath, iType );
    if( iRet < 0 )
    {
        iRetValue = 1;
        goto LABEL_END;
    }
    iDelIndex = iRet;

    ptDelRec = ( PATH * )&( ( *pptArea )->ptPath[ iDelIndex ] );
    memset( ptDelRec, 0x00, sizeof( PATH ) );

    iRetValue = 0;

LABEL_END:
    return iRetValue;
}

static PATHES *_FM_Copy( PATHES *ptArea )
{
	PATH   *ptTmp;
	PATHES *ptRetValue;

	ptRetValue = ( PATHES * )NULL;

	if( ptArea == ( PATHES * )NULL )
	{
		goto LABEL_END;
	}

	ptRetValue = ( PATHES * )malloc( sizeof( PATHES ) );
	if( ptRetValue == ( PATHES * )NULL )
	{
		goto LABEL_END;
	}

	ptRetValue->iHWMark = 0;
	ptRetValue->ptPath  = ( PATH * )NULL;

	if( ptArea->ptPath == ( PATH * )NULL )
	{
		goto LABEL_END;
	}

	ptTmp = ( PATH * )malloc( sizeof( PATH ) * ptArea->iHWMark );
	if( ptTmp == ( PATH * )NULL )
	{
		goto LABEL_END;
	}

	ptRetValue->iHWMark = ptArea->iHWMark;
	ptRetValue->ptPath  = ptTmp;
	memcpy( ptRetValue->ptPath, ptArea->ptPath,
	        sizeof( PATH ) * ptRetValue->iHWMark );

LABEL_END:
	return ptRetValue;
}

