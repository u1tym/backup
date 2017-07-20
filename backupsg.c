#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>

#include  "usg.h"
#include  "backupsg.h"

#define		SG_FILE				"backup.conf"

#define		SG_KEY_LOG			"log_path"
#define		SG_KEY_BACKUP		"backup"
#define		SG_KEY_DELIMITER	"delimiter"
#define		SG_KEY_DO			"DO"

static BKSG gs_tBackupSg;

int BKSG_Init( void )
{
	int   iRet;
	int   iRetValue;
	SGTBL tSg;
	char  *pcVal;
	int   iMax;
	int   iCnt;
	BKREC *ptRec;
	char  *pcBackup;
	char  cMaster_Tmp[ 512 ];
	char  cBackup_Tmp[ 512 ];
	char  cMaster[ 512 ];
	char  cBackup[ 512 ];


	/*========*/
	/* 初期化 */
	/*========*/

	if( gs_tBackupSg.ptRec != ( BKREC * )NULL )
	{
		free( gs_tBackupSg.ptRec );
		gs_tBackupSg.ptRec = ( BKREC * )NULL;
	}

	memset( &gs_tBackupSg, 0x00, sizeof( gs_tBackupSg ) );


	/*======================*/
	/* SGファイルの読み込み */
	/*======================*/

	iRet = SG_Init( SG_FILE, &tSg );
	if( iRet != 0 )
	{
		/* 読み込み異常 */
		iRetValue = -1;
		goto LABEL_END;
	}


	/*================*/
	/* SG値取得、格納 */
	/*================*/

	/* ログファイル名 */
	pcVal = SG_GetValue( &tSg, SG_KEY_LOG, 0 );
	if( pcVal == ( char * )NULL )
	{
		iRetValue = -1;
		goto LABEL_END;
	}
	strcpy( gs_tBackupSg.cLogPath, pcVal );

	/* デリミタ */
	pcVal = SG_GetValue( &tSg, SG_KEY_DELIMITER, 0 );
	if( pcVal == ( char * )NULL )
	{
		iRetValue = -1;
		goto LABEL_END;
	}
	strcpy( gs_tBackupSg.cDelimiter, pcVal );

	/* バックアップ実行設定 */
	pcVal = SG_GetValue( &tSg, SG_KEY_DO, 0 );
	if( pcVal == ( char * )NULL )
	{
		iRetValue = -1;
		goto LABEL_END;
	}
	gs_tBackupSg.iDoBackup = atoi( pcVal );

	/* バックアップ対象 */
	iMax = SG_GetCount( &tSg, SG_KEY_BACKUP );
	if( iMax <= 0 )
	{
		iRetValue = -1;
		goto LABEL_END;
	}

	/* バックアップ対象 : 領域確保 */
	ptRec = ( BKREC * )malloc( sizeof( BKREC ) * iMax );
	if( ptRec == ( BKREC * )NULL )
	{
		iRetValue = -1;
		goto LABEL_END;
	}

	gs_tBackupSg.iMax  = iMax;
	gs_tBackupSg.ptRec = ptRec;

	/* バックアップ対象 : 値設定 */
	for( iCnt = 0; iCnt < iMax; ++iCnt )
	{
		/* 設定先のレコードにキャスト */
		ptRec = &( gs_tBackupSg.ptRec )[ iCnt ];

		/* 設定値を取得 */
		pcBackup = SG_GetValue( &tSg, SG_KEY_BACKUP, iCnt );
		if( pcBackup == ( char * )NULL )
		{
			iRetValue = -1;
			goto LABEL_END;
		}

		/* カンマ区切りで分割 */
		memset( cMaster_Tmp, 0x00, sizeof( cMaster_Tmp ) );
		memset( cBackup_Tmp, 0x00, sizeof( cBackup_Tmp ) );
		SG_GetDiv( pcBackup, ',', 0, cMaster_Tmp );
		SG_GetDiv( pcBackup, ',', 1, cBackup_Tmp );

		/* 空白を除去 */
		memset( cMaster, 0x00, sizeof( cMaster ) );
		memset( cBackup, 0x00, sizeof( cBackup ) );
		SG_CutSp( cMaster_Tmp, cMaster );
		SG_CutSp( cBackup_Tmp, cBackup );

		strcpy( ptRec->cMaster, cMaster );
		strcpy( ptRec->cBackup, cBackup );
	}

	iRetValue = 0;

LABEL_END:
	return iRetValue;
}

char *BKSG_GetLogPath( void )
{
	return &( gs_tBackupSg.cLogPath[ 0 ] );
}

char *BKSG_GetDelimiter( void )
{
	return &( gs_tBackupSg.cDelimiter[ 0 ] );
}

int BKSG_GetDoBackup( void )
{
	return gs_tBackupSg.iDoBackup;
}

int BKSG_GetBackupMax( void )
{
	return gs_tBackupSg.iMax;
}

int BKSG_GetBackupRec( char *pcMaster, char *pcBackup, int iIndex )
{
	BKREC *ptRec;

	if( iIndex >= gs_tBackupSg.iMax )
	{
		return -1;
	}

	ptRec = &( gs_tBackupSg.ptRec )[ iIndex ];

	strcpy( pcMaster, ptRec->cMaster );
	strcpy( pcBackup, ptRec->cBackup );

	return 0;
}



