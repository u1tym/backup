#include  <stdio.h>
#include  <string.h>
#include  <stdlib.h>

#include  <zlib.h>
#include  <zconf.h>

#include  "ucomp.h"

const uInt buflimit = 1024 * 1024 * 1;  // 1MB  
const uInt tempsize = 16384;  

static char *gs_pcExeName = "C:\\\"Program Files\"\\7-Zip\\7z.exe";

int  UF_Compress_GZ( char *, char * );
void UF_Compress_ZIP0( char *, char * );

int UF_Compress( char *pcDst, char *pcSrc )
{
#if 1
	UF_Compress_GZ( pcDst, pcSrc );
#else
	char cCom[ 2048 ];

	sprintf( cCom, "%s a \"%s\" \"%s\"", gs_pcExeName, pcDst, pcSrc );

	system( cCom );
#endif
	return 0;
}

int UF_Compress_GZ( char *pcDst, char *pcSrc )
{
	int    iRetValue;
	int    iCnt;
	char   cBuff[ 256 ];
	gzFile ptSrc = ( gzFile )NULL;
	gzFile ptDst = ( gzFile )NULL;

	ptSrc = gzopen( pcSrc, "rb" );
	if( ptSrc == ( gzFile )NULL )
	{
		iRetValue = -1;
		goto LABEL_END;
	}

	ptDst = gzopen( pcDst, "w1" );
	if( ptDst == ( gzFile )NULL )
	{
		iRetValue = -1;
		goto LABEL_END;
	}

	while( ( iCnt = gzread( ptSrc, cBuff, sizeof( cBuff ) ) ) > 0 )
	{
		gzwrite( ptDst, cBuff, iCnt );
	}

	iRetValue = 0;

LABEL_END:
	if( ptSrc != ( gzFile )NULL )
	{
		gzclose( ptSrc );
		ptSrc = ( gzFile )NULL;
	}
	if( ptDst != ( gzFile )NULL )
	{
		gzclose( ptDst );
		ptDst = ( gzFile )NULL;
	}

	return iRetValue;
}

void UF_Compress_ZIP0( char *pcDst, char *pcSrc )
{
	char *infile;
	char *outfile;

	infile  = pcSrc;
	outfile = pcDst;

    // z_stream構造体の初期化  
    z_stream zs;  
    zs.zalloc = Z_NULL;  
    zs.zfree = Z_NULL;  
    zs.opaque = Z_NULL;  
    if(deflateInit(&zs, Z_DEFAULT_COMPRESSION) != Z_OK){  
        printf("初期化に失敗しました");  
        return;  
    }  
  
    FILE *ifp = fopen(infile, "rb");  
    FILE *ofp = fopen(outfile, "wb");  
  
    uInt buffersz = 0, rest;  
    int flushtype;  
    Byte intemp[tempsize], outtemp[tempsize];  
    do{  
        // 圧縮元データのサイズ  
        zs.avail_in = fread(intemp, 1, tempsize, ifp);  
        // 圧縮元データの格納されているバッファへのポインタ  
        zs.next_in = intemp;  
  
        buffersz += zs.avail_in;  
        if(buffersz > buflimit){  
            // zlibのバッファが一定量たまったら、いったん書き出す  
            flushtype = Z_SYNC_FLUSH;  
            buffersz = 0;  
        }else if(feof(ifp) != 0){  
            // ファイルの終端にきたら終了  
            flushtype = Z_FINISH;  
        }else{  
            // データの取り込みを続行  
            flushtype = Z_NO_FLUSH;  
        }  
  
  
        do{  
            // 圧縮後データのサイズ  
            zs.avail_out = tempsize;  
            // 圧縮後データを格納するバッファのポインタ  
            zs.next_out = outtemp;  
  
            // データの転送  
            deflate(&zs, flushtype);  
  
  
            // avail_outに未圧縮データのサイズが格納されるので、  
            // 差し引いた分のデータを圧縮データとして出力する  
            rest = tempsize - zs.avail_out;  
              
            // 圧縮済みデータの書き込み  
            fwrite(outtemp, 1, rest, ofp);  
        }while (zs.avail_out == 0);  
    }while(flushtype != Z_FINISH);  
  
    // zlibが使用したデータの解放  
    deflateEnd(&zs);  
 
    fclose(ifp);
    fclose(ofp);
}

