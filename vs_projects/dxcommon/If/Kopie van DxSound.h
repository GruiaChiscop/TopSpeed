#ifndef __DXCOMMON_DXSOUND_H__
#define __DXCOMMON_DXSOUND_H__

#include <DxCommon/If/DxCommon.h>
#include <dsound.h>


namespace DirectX
{

class SoundManager;
class Sound;
class WaveFile;

//-----------------------------------------------------------------------------
// Name: class CSoundManager
// Desc: 
//-----------------------------------------------------------------------------
class SoundManager
{
public:
    _dxcommon_ SoundManager();
    _dxcommon_ ~SoundManager();

    _dxcommon_ Int Initialize( HWND hWnd, DWORD dwCoopLevel, DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate );
    _dxcommon_ inline  LPDIRECTSOUND8 GetDirectSound() { return m_pDS; }
    _dxcommon_ Int SetPrimaryBufferFormat(UInt dwPrimaryChannels, UInt dwPrimaryFreq, UInt dwPrimaryBitRate );
    //HRESULT Get3DListenerInterface( LPDIRECTSOUND3DLISTENER* ppDSListener );

    _dxcommon_ Int Create( Sound** ppSound, Char* strWaveFileName, UInt dwCreationFlags = 0, GUID guid3DAlgorithm = GUID_NULL, UInt dwNumBuffers = 1 );
    _dxcommon_ Int CreateFromMemory( Sound** ppSound, UByte* pbData, UInt ulDataSize, LPWAVEFORMATEX pwfx, UInt dwCreationFlags = 0, GUID guid3DAlgorithm = GUID_NULL, UInt dwNumBuffers = 1 );
    //HRESULT CreateStreaming( CStreamingSound** ppStreamingSound, LPTSTR strWaveFileName, DWORD dwCreationFlags, GUID guid3DAlgorithm, DWORD dwNotifyCount, DWORD dwNotifySize, HANDLE hNotifyEvent );

private:
    LPDIRECTSOUND8 m_pDS;
};




//-----------------------------------------------------------------------------
// Name: class CSound
// Desc: Encapsulates functionality of a DirectSound buffer.
//-----------------------------------------------------------------------------
class Sound
{
protected:
    LPDIRECTSOUNDBUFFER* m_apDSBuffer;
    DWORD                m_dwDSBufferSize;
    WaveFile*            m_pWaveFile;
    DWORD                m_dwNumBuffers;

    Int RestoreBuffer( LPDIRECTSOUNDBUFFER pDSB, Boolean* pbWasRestored );

public:
    _dxcommon_ Sound(LPDIRECTSOUNDBUFFER* apDSBuffer, UInt dwDSBufferSize, UInt dwNumBuffers, WaveFile* pWaveFile);
    _dxcommon_ virtual ~Sound();

    _dxcommon_ Int Get3DBufferInterface( UInt dwIndex, LPDIRECTSOUND3DBUFFER* ppDS3DBuffer );
    _dxcommon_ Int FillBufferWithSound( LPDIRECTSOUNDBUFFER pDSB, Boolean bRepeatWavIfBufferLarger );
    _dxcommon_ LPDIRECTSOUNDBUFFER GetFreeBuffer();
    _dxcommon_ LPDIRECTSOUNDBUFFER GetBuffer(UInt dwIndex );

    _dxcommon_ Int Play( UInt dwPriority = 0, UInt dwFlags = 0);
    _dxcommon_ Int Stop();
    _dxcommon_ Int Reset();
    _dxcommon_ Boolean    IsSoundPlaying();

    _dxcommon_ LPDIRECTSOUNDBUFFER* buffer( )  { return m_apDSBuffer; }
};



//-----------------------------------------------------------------------------
// Name: class CWaveFile
// Desc: Encapsulates reading or writing sound data to or from a wave file
//-----------------------------------------------------------------------------
class WaveFile
{
public:
    WAVEFORMATEX* m_pwfx;        // Pointer to WAVEFORMATEX structure
    HMMIO         m_hmmio;       // MM I/O handle for the WAVE
    MMCKINFO      m_ck;          // Multimedia RIFF chunk
    MMCKINFO      m_ckRiff;      // Use in opening a WAVE file
    DWORD         m_dwSize;      // The size of the wave file
    MMIOINFO      m_mmioinfoOut;
    DWORD         m_dwFlags;
    BOOL          m_bIsReadingFromMemory;
    BYTE*         m_pbData;
    BYTE*         m_pbDataCur;
    ULONG         m_ulDataSize;
    CHAR*         m_pResourceBuffer;

protected:
    _dxcommon_ Int ReadMMIO();
    _dxcommon_ Int WriteMMIO( WAVEFORMATEX *pwfxDest );

public:
    _dxcommon_ WaveFile();
    _dxcommon_ virtual ~WaveFile();

    _dxcommon_ Int Open( Char* strFileName, WAVEFORMATEX* pwfx, UInt dwFlags );
    _dxcommon_ Int OpenFromMemory( UByte* pbData, UInt ulDataSize, WAVEFORMATEX* pwfx, UInt dwFlags );
    _dxcommon_ Int Close();

    _dxcommon_ Int Read( UByte* pBuffer, UInt dwSizeToRead, UInt* pdwSizeRead );
    _dxcommon_ Int Write( UInt nSizeToWrite, UByte* pbData, UInt* pnSizeWrote );

    _dxcommon_ UInt GetSize();
    _dxcommon_ Int ResetFile();
    _dxcommon_ WAVEFORMATEX* GetFormat() { return m_pwfx; };
};


} // namespace DirectX






#endif /* __DXCOMMON_DXSOUND_H__ */