//
// WAVE-DMC変換プログラム
//
// 汚いソースだ…
//
#define WIN32_LEAN_AND_MEAN	// おまじない
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"
#include "macro.h"
#include "wavefile.h"

INT	DMCtbl[] = {
	0xD60, 0xBE0, 0xAA0, 0xA00, 0x8F0, 0x7F0, 0x710, 0x6B0,
	0x5F0, 0x500, 0x470, 0x400, 0x350, 0x2A0, 0x240, 0x1B0,
};

void	main( INT argc, CHAR* argv[] )
{
FILE*	wfp = NULL;
FILE*	ofp = NULL;
LPBYTE	lpBuffer = NULL;
INT	i, j;
INT	DMCrate = 0x0F;
INT	nVolume = 100;
BOOL	bVolume = TRUE;
BOOL	bSize   = TRUE;
BOOL	bPadding = FALSE;
CHAR*	pInFile = NULL;
CHAR*	pOutFile = NULL;

	try {
		printf( "DMC sample converter v0.05 programmed by Norix\n" );

		if( argc < 2 ) {
			printf( "Usage: DMCconv wavefile outfile <option>\n" );
			printf( " Options\n" );
			printf( " -r?  DMC Sampling rate(0-F) (Default:F 33.14KHz)\n" );
			printf( "      0: 4.18KHz  1: 4.71KHz  2: 5.26KHz  3: 5.59KHz\n" );
			printf( "      4: 6.26KHz  5: 7.05KHz  6: 7.92KHz  7: 8.36KHz\n" );
			printf( "      8: 9.42KHz  9:11.18KHz  A:12.60KHz  B:13.98KHz\n" );
			printf( "      C:16.88KHz  D:21.30KHz  E:24.86KHz  F:33.14KHz\n" );
			printf( " -v?  Volume(Default:100) \n" );
			printf( " -n   Volume not adjust(Default:Adjust)\n" );
			printf( " -b   Bank size padding(Default:No padding)\n" );
			printf( " -s   Size correct(Default:Correct)\n" );

			return;
		}

		// オプション解析
		for( i = 1; i < argc; i++ ) {
			if( argv[i][0] == '-' || argv[i][0] == '/' ) {
				switch( toupper(argv[i][1]) ) {
					case	'R':
						DMCrate = strtol( &argv[i][2], NULL, 16 );
						break;
					case	'V':
						nVolume = atoi( &argv[i][2] );
						bVolume = TRUE;
						break;
					case	'B':
						bPadding = TRUE;
						break;
					case	'N':
						bVolume = FALSE;
						break;
					case	'S':
						bSize = FALSE;
						break;
				}
			} else if( !pInFile ) {
				pInFile = argv[i];
			} else {
				pOutFile = argv[i];
			}
		}

		if( !pInFile ) {
			printf( "ERROR: require input file.\n" );
			return;
		}
		if( !pOutFile ) {
			printf( "ERROR: require output file.\n" );
			return;
		}

		if( DMCrate < 0 || DMCrate > 0x0F )
			throw	"rateは0-Fの範囲です。";

		CWaveFile	wavfile;
		WAVEFORMATEX*	pwfex;

		if( !wavfile.Open( pInFile ) )
			throw	"ファイルが開けないかWAVEファイルではありません。";

		pwfex = wavfile.GetFormat();

		WORD	nChannels       = pwfex->nChannels;
		DWORD	nSamplesPerSec  = pwfex->nSamplesPerSec;
		DWORD	nAvgBytesPerSec = pwfex->nAvgBytesPerSec;
		WORD	nBlockAlign     = pwfex->nBlockAlign;
		WORD	wBitsPerSample  = pwfex->wBitsPerSample;
		DWORD	nDataSize       = wavfile.GetSize();

		if( pwfex->nChannels > 1 )
			throw	"ステレオフォーマットはサポートしていません。";
		if( pwfex->wBitsPerSample != 16 )
			throw	"8bitフォーマットはサポートしていません。";

		if( !(lpBuffer = (LPBYTE)malloc( nDataSize )) )
			throw	"メモリを確保出来ません。";

		if( !wavfile.ResetFile() )
			throw	"読み込みエラーが発生しました。";

		DWORD	dwReadSize = 0;
		if( !wavfile.Read( lpBuffer, nDataSize, &dwReadSize ) )
			throw	"読み込みエラーが発生しました。";

		if( nDataSize != dwReadSize )
			throw	"読み込みエラーが発生しました。";

		wavfile.Close();

		LONG	nSampleSize = nDataSize/(wBitsPerSample/8);

		printf( "Wave File SamplingRate: %dHz\n", nSamplesPerSec );
		printf( "DMC       SamplingRate: %7.3fHz\n", ((1789772.5*8)/(double)DMCtbl[DMCrate]) );

		printf( "Smaple size: %d samples\n", nSampleSize );

		double	SampleStep, SampleCount;

		// サンプリングレートとDMCサンプリングレートのステップ値
		SampleStep = ((1789772.5*8)/(double)DMCtbl[DMCrate])/(double)nSamplesPerSec;

//		if( SampleStep < 1.0 )
//			throw	"WAVEファイルのサンプリングレートが高すぎます。\n";

		LPSHORT	pW;
		INT	Min, Max;
		INT	temp;

		// WAVEファイルのデータのMAX値を求める
		pW = (SHORT*)lpBuffer;
		Min =  32767;
		Max = -32768;
		for( i = 0; i < nSampleSize; i++ ) {
			if( Min > (INT)*pW )
				Min = (INT)*pW;
			if( Max < (INT)*pW )
				Max = (INT)*pW;
			pW++;
		}

		// 平均値の中心点
		INT	DivPoint = (Min+Max)/2;
		pW = (SHORT*)lpBuffer;

		// でっかい方を最大値として取る
		INT	Multi;
		if( abs( Min ) > abs( Max ) ) {
			Multi = abs(Min);
		} else {
			Multi = abs(Max);
		}

		// 変換
		INT	Delta;
		if( bVolume && Multi ) {
			for( i = 0; i < nSampleSize; i++ ) {
				// 中心点に移動してボリューム調整
				temp = nVolume*((INT)pW[i]-(Min+Max)/2)/100;
				// 最大値を0x1000とする
				temp = 0x800*temp/Multi + 0x800;
				if( temp < 0 )
					temp = 0;
				if( temp > 0x0FFF )
					temp = 0xFFF;

				pW[i] = (SHORT)temp;
			}
			Delta = 0x3F;	// DPCM初期値
		} else {
			Delta = (pW[0]+0x8000)>>9;	// DPCM初期値
		}
		printf( "DMC start value: $%02X\n", Delta );

		// DMCデータへの変換
		if( (ofp = fopen( pOutFile, "wb" )) == NULL )
			throw	"ファイルが開けません。";

		i = 0;
		INT	DMCshift = 8;
		INT	DMCsize = 0;
		INT	DMCbits = 0;

		pW = (SHORT*)lpBuffer;

		SampleCount = SampleStep;

		do {
			DMCbits >>= 1;

			if( pW[i] > Delta*(0x1000/0x40) ) {
				if( ++Delta > 0x3F )
					Delta = 0x3F;
				DMCbits |= 0x80;
			} else {
				if( --Delta < 0 )
					Delta = 0;
			}

			if( --DMCshift == 0 ) {
				fputc( DMCbits, ofp );

				DMCshift = 8;
				DMCbits = 0;
				DMCsize++;

				if( bPadding ) {
					// パディング
					if( (DMCsize & 0x0FFF) == 0x0FF1 ) {
						for( j = 0; j < 0xF; j++ ) {
							fputc( 0, ofp );
							DMCsize++;
						}
					}
				}
			}

			SampleCount -= 1.0;
			while( SampleCount < 0.0 ) {
				SampleCount += SampleStep;
				if( ++i >= nSampleSize )
					break;
			}
		} while( i < nSampleSize );

		if( bPadding ) {
			// バンクサイズになる様にケツにデータを埋める
			INT	nTotalSize = (DMCsize+16383)&~16383;

			do {
				fputc( 0, ofp );
				DMCsize++;
			} while( DMCsize < nTotalSize );

			printf( "Total: %dBanks(8K) $%02XBanks(16KHex) %dKBytes\n", DMCsize/8192, DMCsize/16384, DMCsize/1024 );
		} else if( bSize ) {
			// 再生サイズに合うようにデータを埋める
			if( DMCsize > 0xFF1 ) {
				printf( "Warning: DMCの最大再生可能バイト数を超えています。\n" );
			}
			INT	nTotalSize = ((DMCsize+15)&0xFFFFFFF0)+1;
			do {
				fputc( 0x55, ofp );
				DMCsize++;
			} while( DMCsize < nTotalSize );

			printf( "Total: %d bytes\n", DMCsize );
			printf( "Len-reg value: %d ($%X)\n", nTotalSize>>4, nTotalSize>>4 );
		} else {
			printf( "Total: %d bytes\n", DMCsize );
		}

		FREE( lpBuffer );
		FCLOSE( ofp );

		printf( "Completed!!\n" );
	} catch( CHAR* pStr ) {
		printf( "ERROR: %s\n", pStr );

		FREE( lpBuffer );
		FCLOSE( wfp );
		FCLOSE( ofp );
		return;
#ifndef	_DEBUG
	} catch( ... ) {
		printf( "ERROR: 不明なエラーが発生しました。\n" );

		FREE( lpBuffer );
		FCLOSE( wfp );
		FCLOSE( ofp );
		return;
#endif
	}
}


