//
// DX8SDK‚ÌDSUtil‚ð‰ü‘¢
//
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include "wavefile.h"

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }

CWaveFile::CWaveFile()
{
	m_pwfx    = NULL;
	m_hmmio   = NULL;
	m_dwSize  = 0;
}

CWaveFile::~CWaveFile()
{
	Close();
	SAFE_DELETE_ARRAY( m_pwfx );
}

BOOL	CWaveFile::Open( LPSTR strFileName )
{
	if( strFileName == NULL )
		return	FALSE;
	SAFE_DELETE_ARRAY( m_pwfx );

	if( !(m_hmmio = mmioOpen( strFileName, NULL, MMIO_ALLOCBUF | MMIO_READ )) ) {
		return	FALSE;
	}

	if( !ReadMMIO() ) {
		mmioClose( m_hmmio, 0 );
		return	FALSE;
	}

	if( !ResetFile() )
		return	FALSE;

	m_dwSize = m_ck.cksize;

	return	TRUE;
}

BOOL	CWaveFile::ReadMMIO()
{
	MMCKINFO	ckIn;
	PCMWAVEFORMAT	pcmWaveFormat;

	m_pwfx = NULL;

	if( mmioDescend( m_hmmio, &m_ckRiff, NULL, 0 ) )
		return	FALSE;

	if( (m_ckRiff.ckid != FOURCC_RIFF) ||
	    (m_ckRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E') ) )
		return	FALSE;

	ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if( mmioDescend( m_hmmio, &ckIn, &m_ckRiff, MMIO_FINDCHUNK ) )
		return	FALSE;

	if( ckIn.cksize < (LONG) sizeof(PCMWAVEFORMAT) )
		return	FALSE;

	if( mmioRead( m_hmmio, (HPSTR) &pcmWaveFormat, sizeof(PCMWAVEFORMAT)) != sizeof(PCMWAVEFORMAT) )
		return	FALSE;

	if( pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM ) {
		m_pwfx = (WAVEFORMATEX*)new CHAR[ sizeof(WAVEFORMATEX) ];
		if( NULL == m_pwfx )
			return	FALSE;

		memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
		m_pwfx->cbSize = 0;
	} else {
		WORD cbExtraBytes = 0L;
		if( mmioRead( m_hmmio, (CHAR*)&cbExtraBytes, sizeof(WORD)) != sizeof(WORD) )
			return	FALSE;

		m_pwfx = (WAVEFORMATEX*)new CHAR[ sizeof(WAVEFORMATEX) + cbExtraBytes ];
		if( NULL == m_pwfx )
			return	FALSE;

		memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
		m_pwfx->cbSize = cbExtraBytes;

		if( mmioRead( m_hmmio, (CHAR*)(((BYTE*)&(m_pwfx->cbSize))+sizeof(WORD)), cbExtraBytes ) != cbExtraBytes ) {
			SAFE_DELETE( m_pwfx );
			return	FALSE;
		}
	}

	if( mmioAscend( m_hmmio, &ckIn, 0 ) ) {
		SAFE_DELETE( m_pwfx );
		return	FALSE;
	}

	return	TRUE;
}

DWORD	CWaveFile::GetSize()
{
	return	m_dwSize;
}

BOOL	CWaveFile::ResetFile()
{
	if( m_hmmio == NULL )
		return	FALSE;

	if( -1 == mmioSeek( m_hmmio, m_ckRiff.dwDataOffset + sizeof(FOURCC), SEEK_SET ) )
		return	FALSE;

	m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if( 0 != mmioDescend( m_hmmio, &m_ck, &m_ckRiff, MMIO_FINDCHUNK ) )
		return	FALSE;

	return	TRUE;
}

BOOL	CWaveFile::Read( BYTE* pBuffer, DWORD dwSizeToRead, DWORD* pdwSizeRead )
{
	MMIOINFO mmioinfoIn;

	if( m_hmmio == NULL )
		return	FALSE;

	if( pBuffer == NULL || pdwSizeRead == NULL )
		return	FALSE;

	if( pdwSizeRead != NULL )
		*pdwSizeRead = 0;

	if( mmioGetInfo( m_hmmio, &mmioinfoIn, 0 ) )
		return	FALSE;

	UINT	cbDataIn = dwSizeToRead;
	if( cbDataIn > m_ck.cksize )
		cbDataIn = m_ck.cksize;

	m_ck.cksize -= cbDataIn;

	for( DWORD cT = 0; cT < cbDataIn; cT++ ) {
		if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead ) {
			if( mmioAdvance( m_hmmio, &mmioinfoIn, MMIO_READ ) )
				return	FALSE;

			if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
				return	FALSE;
		}

		*((BYTE*)pBuffer+cT) = *((BYTE*)mmioinfoIn.pchNext);
		mmioinfoIn.pchNext++;
	}

        if( mmioSetInfo( m_hmmio, &mmioinfoIn, 0 ) )
		return	FALSE;

	if( pdwSizeRead != NULL )
		*pdwSizeRead = cbDataIn;

	return	TRUE;
}

BOOL	CWaveFile::Close()
{
	if( m_hmmio ) {
		mmioClose( m_hmmio, 0 );
		m_hmmio = NULL;
	}

	return	TRUE;
}

