/**************************************************************************//**
 * @file  fmmod.c
 * @brief パス管理機能（ヘッダファイル）
 *****************************************************************************/

#ifndef _FMMOD_H
#define _FMMOD_H

#include  <time.h>

/*===========================================================================*/
/* 定数定義																	 */
/*===========================================================================*/

#define	DEF_PATH_MAX	( 316 )

/* PATHのタイプ */
#define FM_TYPE_ALL		( -1 )	/**< 指定なし								 */
#define FM_TYPE_FILE    ( 1 )	/**< ファイル								 */
#define FM_TYPE_DIR     ( 3 )	/**< ディレクトリ							 */

/* 比較結果 */
#define FM_NO_EXIST		( 1 )	/**< 存在しない								 */
#define FM_EXIST_NEW	( 2 )	/**< 新たしいファイルが存在					 */
#define FM_EXIST_EQUAL	( 3 )	/**< 同一ファイルが存在						 */
#define FM_EXIST_OLD	( 4 )	/**< 古いファイルが存在						 */


/*===========================================================================*/
/* 構造体定義																 */
/*===========================================================================*/

/**
 * PATH
 */
typedef struct tag_path
{
    int    iType;					/**< PATHのタイプ						 */
    char   cName[ DEF_PATH_MAX ];	/**< PATH								 */

	/* PATHがファイルの場合 */
    size_t lSize;				/**< ファイルサイズ							 */
    time_t tTime;				/**< 更新タイムスタンプ						 */
} PATH;

/**
 * PATHの集まり
 */
typedef struct tag_pathes
{
    int  iHWMark;				/**< 確保領域レコード数						 */
    PATH *ptPath;				/**< 領域									 */
} PATHES;


/*===========================================================================*/
/* 関数のプロトタイプ宣言													 */
/*===========================================================================*/

PATHES *FM_init( void );
void   FM_finish( PATHES * );
int    FM_getHWMark( PATHES * );
int    FM_addPath( PATHES **, char *, int );
int    FM_delPath( PATHES **, char *, int );
int    FM_getCount( PATHES *, int );

char   *FM_getPath( PATHES *, int );

int    FM_getIndex( PATHES *, int );
int    FM_getIndex_Rev( PATHES *, int );
char   *FM_getPathFromIndex( PATHES *, int );
time_t FM_getTimeFromIndex( PATHES *, int );
int    FM_getTypeFromIndex( PATHES *, int );

int    FM_setAttr( PATHES **, char *, int, size_t, time_t );
void   FM_dispList( PATHES * );
int    FM_sort( PATHES ** );

int    FM_compare( PATHES *, char *, int, time_t );
PATHES *FM_lack( PATHES *, PATHES * );

char   *FM_getPathFromNum( PATHES *, int, int );
int    FM_getIndexFromNum( PATHES *, int, int );

#endif

