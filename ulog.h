/**************************************************************************//**
 * @file  fmmod.c
 * @brief パス管理機能（ヘッダファイル）
 *****************************************************************************/

#ifndef _ULOG_H
#define _ULOG_H

#include  <stdarg.h>

#ifdef _UL_MAIN
#define	UL_EXTERN
#else
#define	UL_EXTERN	extern
#endif

UL_EXTERN char *g_pcFile;
UL_EXTERN int  g_iLine;

/** @def
 *  ログオープンマクロ
 */
#define	ULOG_Open		UL_LogOpen

/** @def
 *  ログ出力マクロ
 */
#define	ULOG_Output		g_pcFile = __FILE__ ;								  \
						g_iLine  = __LINE__;								  \
						UL_LogOutput

/** @def
 *  ログ出力抑制設定マクロ
 */
#define	ULOG_SetDeny	UL_SetDeny

/** @def
 *  ログクローズマクロ
 */
#define ULOG_Close		UL_LogClose

typedef struct tag_UL_DATA
{
	FILE *ptFile;			/**< ログファイルのファイルポインタ				 */
	char *pcDeny;			/**< 出力拒否するログレベル一覧					 */
} UL_DATA;


/*===========================================================================*/
/* 関数のプロトタイプ宣言													 */
/*===========================================================================*/
UL_DATA *UL_LogOpen( char * );
void    UL_LogOutput( UL_DATA *, char *, char *, ... );
void    UL_LogClose( UL_DATA * );
void    UL_SetDeny( UL_DATA *, char * );

#endif

