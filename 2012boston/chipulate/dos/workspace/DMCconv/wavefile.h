//
// DX8SDK‚ÌDSUtil‚ð‰ü‘¢
//
#ifndef	__CWAVEFILE_INCLUDED__
#define	__CWAVEFILE_INCLUDED__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>

class	CWaveFile
{
public:
	WAVEFORMATEX*	m_pwfx;
	HMMIO		m_hmmio;
	MMCKINFO	m_ck;
	MMCKINFO	m_ckRiff;
	DWORD		m_dwSize;

public:
	CWaveFile();
	~CWaveFile();

	BOOL	Open( LPSTR strFileName );
	BOOL	Close();

	BOOL	Read( BYTE* pBuffer, DWORD dwSizeToRead, DWORD* pdwSizeRead );

	BOOL	ResetFile();
	DWORD	GetSize();
	WAVEFORMATEX*	GetFormat() { return m_pwfx; };

protected:
	BOOL	ReadMMIO();
};

#endif	// !__CWAVEFILE_INCLUDED__
