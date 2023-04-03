//-----------------------------------------------------------------------------
// File: DSUtil.cpp
//
// Desc: DirectSound framework classes for reading and writing wav files and
//       playing them in DirectSound buffers. Feel free to use this class 
//       as a starting point for adding extra functionality.
//
// Copyright (c) Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <mmsystem.h>
#include <dxerr.h>
#include <dsound.h>
#include "DSUtil.h"
#include "DXUtil.h"




//-----------------------------------------------------------------------------
// Name: CSoundManager::CSoundManager()
// Desc: Constructs the class
//-----------------------------------------------------------------------------
CSoundManager::CSoundManager()
{
    m_pDS = NULL;
}




//-----------------------------------------------------------------------------
// Name: CSoundManager::~CSoundManager()
// Desc: Destroys the class
//-----------------------------------------------------------------------------
CSoundManager::~CSoundManager()
{
    SAFE_RELEASE( m_pDS ); 
}




//-----------------------------------------------------------------------------
// Name: CSoundManager::Initialize()
// Desc: Initializes the IDirectSound object and also sets the primary buffer
//       format.  This function must be called before any others.
//-----------------------------------------------------------------------------
HRESULT CSoundManager::Initialize( HWND  hWnd, 
                                   DWORD dwCoopLevel )
{
    HRESULT             hr;

    SAFE_RELEASE( m_pDS );

    //create IDirectSound prim sound device
    if( FAILED( hr = DirectSoundCreate8( NULL, &m_pDS, NULL ) ) )
        return DXTRACE_ERR( TEXT("DirectSoundCreate8"), hr );

    // set DirectSound cooperative lvl
    if( FAILED( hr = m_pDS->SetCooperativeLevel( hWnd, dwCoopLevel ) ) )
        return DXTRACE_ERR( TEXT("SetCooperativeLevel"), hr );   

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CSoundManager::SetPrimaryBufferFormat()
// Desc: Set primary buffer to a specified format 
//       !WARNING! - Setting the primary buffer format and then using this
//                   same dsound object for DirectMusic messes up DirectMusic! 
//       For example, to set the primary buffer format to 22kHz stereo, 16-bit
//       then:   dwPrimaryChannels = 2
//               dwPrimaryFreq     = 22050, 
//               dwPrimaryBitRate  = 16
//-----------------------------------------------------------------------------
HRESULT CSoundManager::SetPrimaryBufferFormat( DWORD dwPrimaryChannels, 
                                               DWORD dwPrimaryFreq, 
                                               DWORD dwPrimaryBitRate )
{
    HRESULT             hr;
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

    if( m_pDS == NULL )
        return CO_E_NOTINITIALIZED;

    // get prim buffer
    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize        = sizeof(DSBUFFERDESC);
    dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
    dsbd.dwBufferBytes = 0;
    dsbd.lpwfxFormat   = NULL;
       
    if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &pDSBPrimary, NULL ) ) )
        return DXTRACE_ERR( TEXT("CreateSoundBuffer"), hr );

    WAVEFORMATEX wfx;
    ZeroMemory( &wfx, sizeof(WAVEFORMATEX) ); 
    wfx.wFormatTag      = (WORD) WAVE_FORMAT_PCM; 
    wfx.nChannels       = (WORD) dwPrimaryChannels; 
    wfx.nSamplesPerSec  = (DWORD) dwPrimaryFreq; 
    wfx.wBitsPerSample  = (WORD) dwPrimaryBitRate; 
    wfx.nBlockAlign     = (WORD) (wfx.wBitsPerSample / 8 * wfx.nChannels);
    wfx.nAvgBytesPerSec = (DWORD) (wfx.nSamplesPerSec * wfx.nBlockAlign);

    if( FAILED( hr = pDSBPrimary->SetFormat(&wfx) ) )
        return DXTRACE_ERR( TEXT("SetFormat"), hr );

    SAFE_RELEASE( pDSBPrimary );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CSoundManager::Get3DListenerInterface()
// Desc: Returns the 3D listener interface associated with primary buffer.
//-----------------------------------------------------------------------------
HRESULT CSoundManager::Get3DListenerInterface( LPDIRECTSOUND3DLISTENER* ppDSListener )
{
    HRESULT             hr;
    DSBUFFERDESC        dsbdesc;
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

    if( ppDSListener == NULL )
        return E_INVALIDARG;
    if( m_pDS == NULL )
        return CO_E_NOTINITIALIZED;

    *ppDSListener = NULL;

    //ask prim buffer for 3D control
    ZeroMemory( &dsbdesc, sizeof(DSBUFFERDESC) );
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
    if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbdesc, &pDSBPrimary, NULL ) ) )
        return DXTRACE_ERR( TEXT("CreateSoundBuffer"), hr );

    if( FAILED( hr = pDSBPrimary->QueryInterface( IID_IDirectSound3DListener, 
                                                  (VOID**)ppDSListener ) ) )
    {
        SAFE_RELEASE( pDSBPrimary );
        return DXTRACE_ERR( TEXT("QueryInterface"), hr );
    }

    //release prim buffer as not used 
    SAFE_RELEASE( pDSBPrimary );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CSoundManager::Create()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CSoundManager::Create( CSound** ppSound, 
                               LPTSTR strWaveFileName, 
                               DWORD dwCreationFlags, 
                               GUID guid3DAlgorithm,
                               DWORD dwNumBuffers )
{
    HRESULT hr;
    HRESULT hrRet = S_OK;
    DWORD   i;
    LPDIRECTSOUNDBUFFER* apDSBuffer     = NULL;
    DWORD                dwDSBufferSize = NULL;
    CWaveFile*           pWaveFile      = NULL;

    if( m_pDS == NULL )
        return CO_E_NOTINITIALIZED;
    if( strWaveFileName == NULL || ppSound == NULL || dwNumBuffers < 1 )
        return E_INVALIDARG;

    apDSBuffer = new LPDIRECTSOUNDBUFFER[dwNumBuffers];
    if( apDSBuffer == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto LFail;
    }

    pWaveFile = new CWaveFile();
    if( pWaveFile == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto LFail;
    }

    pWaveFile->Open( strWaveFileName, NULL, WAVEFILE_READ );

    if( pWaveFile->GetSize() == 0 )
    {
        //wav blank - do not create
        hr = E_FAIL;
        goto LFail;
    }

    //make DirectSound buffer same size as wav
    dwDSBufferSize = pWaveFile->GetSize();

    //make the direct sound buffer, only request the flags needed
    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize          = sizeof(DSBUFFERDESC);
    dsbd.dwFlags         = dwCreationFlags;
    dsbd.dwBufferBytes   = dwDSBufferSize;
    dsbd.guid3DAlgorithm = guid3DAlgorithm;
    dsbd.lpwfxFormat     = pWaveFile->m_pwfx;
    
    //DirectSound only guarenteed to play PCM data.
    hr = m_pDS->CreateSoundBuffer( &dsbd, &apDSBuffer[0], NULL );

    // return error
    if( hr == DS_NO_VIRTUALIZATION )
        hrRet = DS_NO_VIRTUALIZATION;
            
    if( FAILED(hr) )
    {
        // DSERR_BUFFERTOOSMALL will be returned if the buffer is less than DSBSIZE_FX_MIN and the buffer is created with DSBCAPS_CTRLFX.
      
        
        // may fail is hardware buffer missing
        DXTRACE_ERR( TEXT("CreateSoundBuffer"), hr );
                    
        goto LFail;
    }

    // Default to use DuplicateSoundBuffer() when extra buffers. DuplicateSoundBuffer() will fail if  DSBCAPS_CTRLFX used; use CreateSoundBuffer() instead
    if( (dwCreationFlags & DSBCAPS_CTRLFX) == 0 )
    {
        for( i=1; i<dwNumBuffers; i++ )
        {
            if( FAILED( hr = m_pDS->DuplicateSoundBuffer( apDSBuffer[0], &apDSBuffer[i] ) ) )
            {
                DXTRACE_ERR( TEXT("DuplicateSoundBuffer"), hr );
                goto LFail;
            }
        }
    }
    else
    {
        for( i=1; i<dwNumBuffers; i++ )
        {
            hr = m_pDS->CreateSoundBuffer( &dsbd, &apDSBuffer[i], NULL );
            if( FAILED(hr) )
            {
                DXTRACE_ERR( TEXT("CreateSoundBuffer"), hr );
                goto LFail;
            }
        }
   }
    
    //make sound 
    *ppSound = new CSound( apDSBuffer, dwDSBufferSize, dwNumBuffers, pWaveFile, dwCreationFlags );
    
    SAFE_DELETE_ARRAY( apDSBuffer );
    return hrRet;

LFail:
    //clean up
    SAFE_DELETE( pWaveFile );
    SAFE_DELETE_ARRAY( apDSBuffer );
    return hr;
}









//-----------------------------------------------------------------------------
// Name: CSoundManager::CreateFromMemory()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CSoundManager::CreateFromMemory( CSound** ppSound, 
                                        BYTE* pbData,
                                        ULONG  ulDataSize,
                                        LPWAVEFORMATEX pwfx,
                                        DWORD dwCreationFlags, 
                                        GUID guid3DAlgorithm,
                                        DWORD dwNumBuffers )
{
    HRESULT hr;
    DWORD   i;
    LPDIRECTSOUNDBUFFER* apDSBuffer     = NULL;
    DWORD                dwDSBufferSize = NULL;
    CWaveFile*           pWaveFile      = NULL;

    if( m_pDS == NULL )
        return CO_E_NOTINITIALIZED;
    if( pbData == NULL || ppSound == NULL || dwNumBuffers < 1 )
        return E_INVALIDARG;

    apDSBuffer = new LPDIRECTSOUNDBUFFER[dwNumBuffers];
    if( apDSBuffer == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto LFail;
    }

    pWaveFile = new CWaveFile();
    if( pWaveFile == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto LFail;
    }

    pWaveFile->OpenFromMemory( pbData,ulDataSize, pwfx, WAVEFILE_READ );


    // Make the DirectSound buffer same size as wav
    dwDSBufferSize = ulDataSize;

    // create buffer and request needed flags

    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize          = sizeof(DSBUFFERDESC);
    dsbd.dwFlags         = dwCreationFlags;
    dsbd.dwBufferBytes   = dwDSBufferSize;
    dsbd.guid3DAlgorithm = guid3DAlgorithm;
    dsbd.lpwfxFormat     = pwfx;

    if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &apDSBuffer[0], NULL ) ) )
    {
        DXTRACE_ERR( TEXT("CreateSoundBuffer"), hr );
        goto LFail;
    }

    //use DuplicateSoundBuffer() when extra buffers create a buffer that uses the same memory, DuplicateSoundBuffer() will fail if  DSBCAPS_CTRLFX is used, use CreateSoundBuffer() instead 
    if( (dwCreationFlags & DSBCAPS_CTRLFX) == 0 )
    {
        for( i=1; i<dwNumBuffers; i++ )
        {
            if( FAILED( hr = m_pDS->DuplicateSoundBuffer( apDSBuffer[0], &apDSBuffer[i] ) ) )
            {
                DXTRACE_ERR( TEXT("DuplicateSoundBuffer"), hr );
                goto LFail;
            }
        }
    }
    else
    {
        for( i=1; i<dwNumBuffers; i++ )
        {
            hr = m_pDS->CreateSoundBuffer( &dsbd, &apDSBuffer[i], NULL );
            if( FAILED(hr) )
            {
                DXTRACE_ERR( TEXT("CreateSoundBuffer"), hr );
                goto LFail;
            }
        }
   }

    // create sound
    *ppSound = new CSound( apDSBuffer, dwDSBufferSize, dwNumBuffers, pWaveFile, dwCreationFlags );

    SAFE_DELETE_ARRAY( apDSBuffer );
    return S_OK;

LFail:
    //clean
   
    SAFE_DELETE_ARRAY( apDSBuffer );
    return hr;
}





//-----------------------------------------------------------------------------
// Name: CSoundManager::CreateStreaming()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CSoundManager::CreateStreaming( CStreamingSound** ppStreamingSound, 
                                        LPTSTR strWaveFileName, 
                                        DWORD dwCreationFlags, 
                                        GUID guid3DAlgorithm,
                                        DWORD dwNotifyCount, 
                                        DWORD dwNotifySize, 
                                        HANDLE hNotifyEvent )
{
    HRESULT hr;

    if( m_pDS == NULL )
        return CO_E_NOTINITIALIZED;
    if( strWaveFileName == NULL || ppStreamingSound == NULL || hNotifyEvent == NULL )
        return E_INVALIDARG;

    LPDIRECTSOUNDBUFFER pDSBuffer      = NULL;
    DWORD               dwDSBufferSize = NULL;
    CWaveFile*          pWaveFile      = NULL;
    DSBPOSITIONNOTIFY*  aPosNotify     = NULL; 
    LPDIRECTSOUNDNOTIFY pDSNotify      = NULL;

    pWaveFile = new CWaveFile();
    if( pWaveFile == NULL )
        return E_OUTOFMEMORY;
    pWaveFile->Open( strWaveFileName, NULL, WAVEFILE_READ );

    // calculate how big buffer needs to be 
    dwDSBufferSize = dwNotifySize * dwNotifyCount;

    // set up buffer, request the NOTIFY flag 

    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize          = sizeof(DSBUFFERDESC);
    dsbd.dwFlags         = dwCreationFlags | 
                           DSBCAPS_CTRLPOSITIONNOTIFY | 
                           DSBCAPS_GETCURRENTPOSITION2;
    dsbd.dwBufferBytes   = dwDSBufferSize;
    dsbd.guid3DAlgorithm = guid3DAlgorithm;
    dsbd.lpwfxFormat     = pWaveFile->m_pwfx;

    if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &pDSBuffer, NULL ) ) )
    {
        // If wav format isn't then  return either DSERR_BADFORMAT/ E_INVALIDARG
        if( hr == DSERR_BADFORMAT || hr == E_INVALIDARG )
            return DXTRACE_ERR( TEXT("CreateSoundBuffer"), hr );

        return DXTRACE_ERR( TEXT("CreateSoundBuffer"), hr );
    }

    //create notificaion event
    if( FAILED( hr = pDSBuffer->QueryInterface( IID_IDirectSoundNotify, 
                                                (VOID**)&pDSNotify ) ) )
    {
        SAFE_DELETE_ARRAY( aPosNotify );
        return DXTRACE_ERR( TEXT("QueryInterface"), hr );
    }

    aPosNotify = new DSBPOSITIONNOTIFY[ dwNotifyCount ];
    if( aPosNotify == NULL )
        return E_OUTOFMEMORY;

    for( DWORD i = 0; i < dwNotifyCount; i++ )
    {
        aPosNotify[i].dwOffset     = (dwNotifySize * i) + dwNotifySize - 1;
        aPosNotify[i].hEventNotify = hNotifyEvent;             
    }
    
    //tell DirectSound when to notify
    if( FAILED( hr = pDSNotify->SetNotificationPositions( dwNotifyCount, 
                                                          aPosNotify ) ) )
    {
        SAFE_RELEASE( pDSNotify );
        SAFE_DELETE_ARRAY( aPosNotify );
        return DXTRACE_ERR( TEXT("SetNotificationPositions"), hr );
    }

    SAFE_RELEASE( pDSNotify );
    SAFE_DELETE_ARRAY( aPosNotify );

    // create sound
    *ppStreamingSound = new CStreamingSound( pDSBuffer, dwDSBufferSize, pWaveFile, dwNotifySize );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CSound::CSound()
// Desc: Constructs the class
//-----------------------------------------------------------------------------
CSound::CSound( LPDIRECTSOUNDBUFFER* apDSBuffer, DWORD dwDSBufferSize, 
                DWORD dwNumBuffers, CWaveFile* pWaveFile, DWORD dwCreationFlags )
{
    DWORD i;

    m_apDSBuffer = new LPDIRECTSOUNDBUFFER[dwNumBuffers];
    if( NULL != m_apDSBuffer )
    {
        for( i=0; i<dwNumBuffers; i++ )
            m_apDSBuffer[i] = apDSBuffer[i];
    
        m_dwDSBufferSize = dwDSBufferSize;
        m_dwNumBuffers   = dwNumBuffers;
        m_pWaveFile      = pWaveFile;
        m_dwCreationFlags = dwCreationFlags;
        
        FillBufferWithSound( m_apDSBuffer[0], FALSE );
    }
}




//-----------------------------------------------------------------------------
// Name: CSound::~CSound()
// Desc: Destroys the class
//-----------------------------------------------------------------------------
CSound::~CSound()
{
    for( DWORD i=0; i<m_dwNumBuffers; i++ )
    {
        SAFE_RELEASE( m_apDSBuffer[i] ); 
    }

    SAFE_DELETE_ARRAY( m_apDSBuffer ); 
    SAFE_DELETE( m_pWaveFile );
}




//-----------------------------------------------------------------------------
// Name: CSound::FillBufferWithSound()
// Desc: Fills a DirectSound buffer with a sound file 
//-----------------------------------------------------------------------------
HRESULT CSound::FillBufferWithSound( LPDIRECTSOUNDBUFFER pDSB, BOOL bRepeatWavIfBufferLarger )
{
    HRESULT hr; 
    VOID*   pDSLockedBuffer      = NULL; // Pointer to locked buffer memory
    DWORD   dwDSLockedBufferSize = 0;    // Size of the locked DirectSound buffer
    DWORD   dwWavDataRead        = 0;    // Amount of data read from the wav file 

    if( pDSB == NULL )
        return CO_E_NOTINITIALIZED;

    //make sure there is focus
    if( FAILED( hr = RestoreBuffer( pDSB, NULL ) ) ) 
        return DXTRACE_ERR( TEXT("RestoreBuffer"), hr );

    //lock down buffer
    if( FAILED( hr = pDSB->Lock( 0, m_dwDSBufferSize, 
                                 &pDSLockedBuffer, &dwDSLockedBufferSize, 
                                 NULL, NULL, 0L ) ) )
        return DXTRACE_ERR( TEXT("Lock"), hr );

    // Reset file to start
    m_pWaveFile->ResetFile();

    if( FAILED( hr = m_pWaveFile->Read( (BYTE*) pDSLockedBuffer,
                                        dwDSLockedBufferSize, 
                                        &dwWavDataRead ) ) )           
        return DXTRACE_ERR( TEXT("Read"), hr );

    if( dwWavDataRead == 0 )
    {
        // Wav is blank, so fill w/ silence 
        FillMemory( (BYTE*) pDSLockedBuffer, 
                    dwDSLockedBufferSize, 
                    (BYTE)(m_pWaveFile->m_pwfx->wBitsPerSample == 8 ? 128 : 0 ) );
    }
    else if( dwWavDataRead < dwDSLockedBufferSize )
    {
        // If the wav < smaller than the DirectSound buffer, fill remainder of buffer with data
        if( bRepeatWavIfBufferLarger )
        {       
            // Reset the file, fill buffer with wav data
            DWORD dwReadSoFar = dwWavDataRead;    
            while( dwReadSoFar < dwDSLockedBufferSize )
            {  
                // keep going until buffer full
                if( FAILED( hr = m_pWaveFile->ResetFile() ) )
                    return DXTRACE_ERR( TEXT("ResetFile"), hr );

                hr = m_pWaveFile->Read( (BYTE*)pDSLockedBuffer + dwReadSoFar,
                                        dwDSLockedBufferSize - dwReadSoFar,
                                        &dwWavDataRead );
                if( FAILED(hr) )
                    return DXTRACE_ERR( TEXT("Read"), hr );

                dwReadSoFar += dwWavDataRead;
            } 
        }
        else
        {
            // don't repeate wav, silence 
            FillMemory( (BYTE*) pDSLockedBuffer + dwWavDataRead, 
                        dwDSLockedBufferSize - dwWavDataRead, 
                        (BYTE)(m_pWaveFile->m_pwfx->wBitsPerSample == 8 ? 128 : 0 ) );
        }
    }

    // unlock buffer as unneeded
    pDSB->Unlock( pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0 );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CSound::RestoreBuffer()
// Desc: Restores the lost buffer. *pbWasRestored returns TRUE if the buffer was 
//       restored.  It can also NULL if the information is not needed.
//-----------------------------------------------------------------------------
HRESULT CSound::RestoreBuffer( LPDIRECTSOUNDBUFFER pDSB, BOOL* pbWasRestored )
{
    HRESULT hr;

    if( pDSB == NULL )
        return CO_E_NOTINITIALIZED;
    if( pbWasRestored )
        *pbWasRestored = FALSE;

    DWORD dwStatus;
    if( FAILED( hr = pDSB->GetStatus( &dwStatus ) ) )
        return DXTRACE_ERR( TEXT("GetStatus"), hr );

    if( dwStatus & DSBSTATUS_BUFFERLOST )
    {
        // sleep until DirectSound has control
        do 
        {
            hr = pDSB->Restore();
            if( hr == DSERR_BUFFERLOST )
                Sleep( 10 );
        }
        while( ( hr = pDSB->Restore() ) == DSERR_BUFFERLOST );

        if( pbWasRestored != NULL )
            *pbWasRestored = TRUE;

        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}




//-----------------------------------------------------------------------------
// Name: CSound::GetFreeBuffer()
// Desc: Finding the first buffer that is not playing and return a pointer to 
//       it, or if all are playing return a pointer to a randomly selected buffer.
//-----------------------------------------------------------------------------
LPDIRECTSOUNDBUFFER CSound::GetFreeBuffer()
{
    if( m_apDSBuffer == NULL )
        return FALSE; 

	DWORD i=0;

    for( i=0; i<m_dwNumBuffers; i++ )
    {
        if( m_apDSBuffer[i] )
        {  
            DWORD dwStatus = 0;
            m_apDSBuffer[i]->GetStatus( &dwStatus );
            if ( ( dwStatus & DSBSTATUS_PLAYING ) == 0 )
                break;
        }
    }

    if( i != m_dwNumBuffers )
        return m_apDSBuffer[ i ];
    else
        return m_apDSBuffer[ rand() % m_dwNumBuffers ];
}




//-----------------------------------------------------------------------------
// Name: CSound::GetBuffer()
// Desc: 
//-----------------------------------------------------------------------------
LPDIRECTSOUNDBUFFER CSound::GetBuffer( DWORD dwIndex )
{
    if( m_apDSBuffer == NULL )
        return NULL;
    if( dwIndex >= m_dwNumBuffers )
        return NULL;

    return m_apDSBuffer[dwIndex];
}




//-----------------------------------------------------------------------------
// Name: CSound::Get3DBufferInterface()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CSound::Get3DBufferInterface( DWORD dwIndex, LPDIRECTSOUND3DBUFFER* ppDS3DBuffer )
{
    if( m_apDSBuffer == NULL )
        return CO_E_NOTINITIALIZED;
    if( dwIndex >= m_dwNumBuffers )
        return E_INVALIDARG;

    *ppDS3DBuffer = NULL;

    return m_apDSBuffer[dwIndex]->QueryInterface( IID_IDirectSound3DBuffer, 
                                                  (VOID**)ppDS3DBuffer );
}


//-----------------------------------------------------------------------------
// Name: CSound::Play()
// Desc: Plays the sound using voice management flags.  Pass in DSBPLAY_LOOPING
//       in the dwFlags to loop the sound
//-----------------------------------------------------------------------------
HRESULT CSound::Play( DWORD dwPriority, DWORD dwFlags, LONG lVolume, LONG lFrequency, LONG lPan )
{
    HRESULT hr;
    BOOL    bRestored;

    if( m_apDSBuffer == NULL )
        return CO_E_NOTINITIALIZED;

    LPDIRECTSOUNDBUFFER pDSB = GetFreeBuffer();

    if( pDSB == NULL )
        return DXTRACE_ERR( TEXT("GetFreeBuffer"), E_FAIL );

    // restore buffer if lost
    if( FAILED( hr = RestoreBuffer( pDSB, &bRestored ) ) )
        return DXTRACE_ERR( TEXT("RestoreBuffer"), hr );

    if( bRestored )
    {
        // fill restored buffer witrh data
        if( FAILED( hr = FillBufferWithSound( pDSB, FALSE ) ) )
            return DXTRACE_ERR( TEXT("FillBufferWithSound"), hr );
    }

    if( m_dwCreationFlags & DSBCAPS_CTRLVOLUME )
    {
        pDSB->SetVolume( lVolume );
    }

    if( lFrequency != -1 && 
        (m_dwCreationFlags & DSBCAPS_CTRLFREQUENCY) )
    {
        pDSB->SetFrequency( lFrequency );
    }
    
    if( m_dwCreationFlags & DSBCAPS_CTRLPAN )
    {
        pDSB->SetPan( lPan );
    }
    
    return pDSB->Play( 0, dwPriority, dwFlags );
}




//-----------------------------------------------------------------------------
// Name: CSound::Play3D()
// Desc: Plays the sound using voice management flags.  Pass in DSBPLAY_LOOPING
//       in the dwFlags to loop the sound
//-----------------------------------------------------------------------------
HRESULT CSound::Play3D( LPDS3DBUFFER p3DBuffer, DWORD dwPriority, DWORD dwFlags, LONG lFrequency )
{
    HRESULT hr;
    BOOL    bRestored;
    DWORD   dwBaseFrequency;

    if( m_apDSBuffer == NULL )
        return CO_E_NOTINITIALIZED;

    LPDIRECTSOUNDBUFFER pDSB = GetFreeBuffer();
    if( pDSB == NULL )
        return DXTRACE_ERR( TEXT("GetFreeBuffer"), E_FAIL );

    // restore buffer iflost
    if( FAILED( hr = RestoreBuffer( pDSB, &bRestored ) ) )
        return DXTRACE_ERR( TEXT("RestoreBuffer"), hr );

    if( bRestored )
    {
        // fill restored buffer with data
        if( FAILED( hr = FillBufferWithSound( pDSB, FALSE ) ) )
            return DXTRACE_ERR( TEXT("FillBufferWithSound"), hr );
    }

    if( m_dwCreationFlags & DSBCAPS_CTRLFREQUENCY )
    {
        pDSB->GetFrequency( &dwBaseFrequency );
        pDSB->SetFrequency( dwBaseFrequency + lFrequency );
    }

    // QI 3D buffer
    LPDIRECTSOUND3DBUFFER pDS3DBuffer;
    hr = pDSB->QueryInterface( IID_IDirectSound3DBuffer, (VOID**) &pDS3DBuffer );
    if( SUCCEEDED( hr ) )
    {
        hr = pDS3DBuffer->SetAllParameters( p3DBuffer, DS3D_IMMEDIATE );
        if( SUCCEEDED( hr ) )
        {
            hr = pDSB->Play( 0, dwPriority, dwFlags );
        }

        pDS3DBuffer->Release();
    }

    return hr;
}



//-----------------------------------------------------------------------------
// Name: CSound::Stop()
// Desc: Stops the sound from playing
//-----------------------------------------------------------------------------
HRESULT CSound::Stop()
{
    if( m_apDSBuffer == NULL )
        return CO_E_NOTINITIALIZED;

    HRESULT hr = 0;

    for( DWORD i=0; i<m_dwNumBuffers; i++ )
        hr |= m_apDSBuffer[i]->Stop();

    return hr;
}




//-----------------------------------------------------------------------------
// Name: CSound::Reset()
// Desc: Reset all of the sound buffers
//-----------------------------------------------------------------------------
HRESULT CSound::Reset()
{
    if( m_apDSBuffer == NULL )
        return CO_E_NOTINITIALIZED;

    HRESULT hr = 0;

    for( DWORD i=0; i<m_dwNumBuffers; i++ )
        hr |= m_apDSBuffer[i]->SetCurrentPosition( 0 );

    return hr;
}




//-----------------------------------------------------------------------------
// Name: CSound::IsSoundPlaying()
// Desc: Checks to see if a buffer is playing and returns TRUE if it is.
//-----------------------------------------------------------------------------
BOOL CSound::IsSoundPlaying()
{
    BOOL bIsPlaying = FALSE;

    if( m_apDSBuffer == NULL )
        return FALSE; 

    for( DWORD i=0; i<m_dwNumBuffers; i++ )
    {
        if( m_apDSBuffer[i] )
        {  
            DWORD dwStatus = 0;
            m_apDSBuffer[i]->GetStatus( &dwStatus );
            bIsPlaying |= ( ( dwStatus & DSBSTATUS_PLAYING ) != 0 );
        }
    }

    return bIsPlaying;
}




//-----------------------------------------------------------------------------
// Name: CStreamingSound::CStreamingSound()
// Desc: Setups up a buffer so data can be streamed from the wave file into 
//       a buffer.  This is very useful for large wav files that would take a 
//       while to load.  The buffer is initially filled with data, then 
//       as sound is played the notification events are signaled and more data
//       is written into the buffer by calling HandleWaveStreamNotification()
//-----------------------------------------------------------------------------
CStreamingSound::CStreamingSound( LPDIRECTSOUNDBUFFER pDSBuffer, DWORD dwDSBufferSize, 
                                  CWaveFile* pWaveFile, DWORD dwNotifySize ) 
                : CSound( &pDSBuffer, dwDSBufferSize, 1, pWaveFile, 0 )           
{
    m_dwLastPlayPos     = 0;
    m_dwPlayProgress    = 0;
    m_dwNotifySize      = dwNotifySize;
    m_dwNextWriteOffset = 0;
    m_bFillNextNotificationWithSilence = FALSE;
}




//-----------------------------------------------------------------------------
// Name: CStreamingSound::~CStreamingSound()
// Desc: Destroys the class
//-----------------------------------------------------------------------------
CStreamingSound::~CStreamingSound()
{
}



//-----------------------------------------------------------------------------
// Name: CStreamingSound::HandleWaveStreamNotification()
// Desc: Handle the notification that tells us to put more wav data in the 
//       circular buffer
//-----------------------------------------------------------------------------
HRESULT CStreamingSound::HandleWaveStreamNotification( BOOL bLoopedPlay )
{
    HRESULT hr;
    DWORD   dwCurrentPlayPos;
    DWORD   dwPlayDelta;
    DWORD   dwBytesWrittenToBuffer;
    VOID*   pDSLockedBuffer = NULL;
    VOID*   pDSLockedBuffer2 = NULL;
    DWORD   dwDSLockedBufferSize;
    DWORD   dwDSLockedBufferSize2;

    if( m_apDSBuffer == NULL || m_pWaveFile == NULL )
        return CO_E_NOTINITIALIZED;

    // restore buffer if lost
    BOOL bRestored;
    if( FAILED( hr = RestoreBuffer( m_apDSBuffer[0], &bRestored ) ) )
        return DXTRACE_ERR( TEXT("RestoreBuffer"), hr );

    if( bRestored )
    {
        //fill restored buffer with data
        if( FAILED( hr = FillBufferWithSound( m_apDSBuffer[0], FALSE ) ) )
            return DXTRACE_ERR( TEXT("FillBufferWithSound"), hr );
        return S_OK;
    }

    // lock buffer
    if( FAILED( hr = m_apDSBuffer[0]->Lock( m_dwNextWriteOffset, m_dwNotifySize, 
                                            &pDSLockedBuffer, &dwDSLockedBufferSize, 
                                            &pDSLockedBuffer2, &dwDSLockedBufferSize2, 0L ) ) )
        return DXTRACE_ERR( TEXT("Lock"), hr );

    // m_dwDSBufferSize and m_dwNextWriteOffset both multiples of m_dwNotifySize, should be the second buffer so empty
    if( pDSLockedBuffer2 != NULL )
        return E_UNEXPECTED; 

    if( !m_bFillNextNotificationWithSilence )
    {
        //fill the DirectSound buffer with wav data
        if( FAILED( hr = m_pWaveFile->Read( (BYTE*) pDSLockedBuffer, 
                                                  dwDSLockedBufferSize, 
                                                  &dwBytesWrittenToBuffer ) ) )           
            return DXTRACE_ERR( TEXT("Read"), hr );
    }
    else
    {
        //fill DirectSound buffer with silence
        FillMemory( pDSLockedBuffer, dwDSLockedBufferSize, 
                    (BYTE)( m_pWaveFile->m_pwfx->wBitsPerSample == 8 ? 128 : 0 ) );
        dwBytesWrittenToBuffer = dwDSLockedBufferSize;
    }

    // If the number of bytes written < amount we requested,have a short file.
    if( dwBytesWrittenToBuffer < dwDSLockedBufferSize )
    {
        if( !bLoopedPlay ) 
        {
            //fill in silence for rest of buffer
            FillMemory( (BYTE*) pDSLockedBuffer + dwBytesWrittenToBuffer, 
                        dwDSLockedBufferSize - dwBytesWrittenToBuffer, 
                        (BYTE)(m_pWaveFile->m_pwfx->wBitsPerSample == 8 ? 128 : 0 ) );

            // future notifications filled with silence
            m_bFillNextNotificationWithSilence = TRUE;
        }
        else
        {
            //(looping) reset buffer and fill with new data
            DWORD dwReadSoFar = dwBytesWrittenToBuffer; 
            while( dwReadSoFar < dwDSLockedBufferSize )
            {  
                // keep reading until full
                if( FAILED( hr = m_pWaveFile->ResetFile() ) )
                    return DXTRACE_ERR( TEXT("ResetFile"), hr );

                if( FAILED( hr = m_pWaveFile->Read( (BYTE*)pDSLockedBuffer + dwReadSoFar,
                                                          dwDSLockedBufferSize - dwReadSoFar,
                                                          &dwBytesWrittenToBuffer ) ) )
                    return DXTRACE_ERR( TEXT("Read"), hr );

                dwReadSoFar += dwBytesWrittenToBuffer;
            } 
        } 
    }

    //unlock DirectSound buffer
    m_apDSBuffer[0]->Unlock( pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0 );

    // calculate data played so far. when played past end of file, fill buffer with silence or read from beginning of file
  
    if( FAILED( hr = m_apDSBuffer[0]->GetCurrentPosition( &dwCurrentPlayPos, NULL ) ) )
        return DXTRACE_ERR( TEXT("GetCurrentPosition"), hr );

    // Check pos counter looped
    if( dwCurrentPlayPos < m_dwLastPlayPos )
        dwPlayDelta = ( m_dwDSBufferSize - m_dwLastPlayPos ) + dwCurrentPlayPos;
    else
        dwPlayDelta = dwCurrentPlayPos - m_dwLastPlayPos;

    m_dwPlayProgress += dwPlayDelta;
    m_dwLastPlayPos = dwCurrentPlayPos;

    // is buffer being filled with silence, end buffer
    if( m_bFillNextNotificationWithSilence )
    {
        //don't cut off sound until finished
        if( m_dwPlayProgress >= m_pWaveFile->GetSize() )
        {
            m_apDSBuffer[0]->Stop();
        }
    }

    // update where buffer locks
    m_dwNextWriteOffset += dwDSLockedBufferSize; 
	//circle buffer
    m_dwNextWriteOffset %= m_dwDSBufferSize; 

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CStreamingSound::Reset()
// Desc: Resets the sound so it will begin playing at the beginning
//-----------------------------------------------------------------------------
HRESULT CStreamingSound::Reset()
{
    HRESULT hr;

    if( m_apDSBuffer[0] == NULL || m_pWaveFile == NULL )
        return CO_E_NOTINITIALIZED;

    m_dwLastPlayPos     = 0;
    m_dwPlayProgress    = 0;
    m_dwNextWriteOffset = 0;
    m_bFillNextNotificationWithSilence = FALSE;

    //restore buffer if lost
    BOOL bRestored;
    if( FAILED( hr = RestoreBuffer( m_apDSBuffer[0], &bRestored ) ) )
        return DXTRACE_ERR( TEXT("RestoreBuffer"), hr );

    if( bRestored )
    {
        //fill restored buffer with sound
        if( FAILED( hr = FillBufferWithSound( m_apDSBuffer[0], FALSE ) ) )
            return DXTRACE_ERR( TEXT("FillBufferWithSound"), hr );
    }

    m_pWaveFile->ResetFile();

    return m_apDSBuffer[0]->SetCurrentPosition( 0L );  
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::CWaveFile()
// Desc: Constructs the class.  Call Open() to open a wave file for reading.  
//       Then call Read() as needed.  Calling the destructor or Close() 
//       will close the file.  
//-----------------------------------------------------------------------------
CWaveFile::CWaveFile()
{
    m_pwfx    = NULL;
    m_hmmio   = NULL;
    m_pResourceBuffer = NULL;
    m_dwSize  = 0;
    m_bIsReadingFromMemory = FALSE;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::~CWaveFile()
// Desc: Destructs the class
//-----------------------------------------------------------------------------
CWaveFile::~CWaveFile()
{
    Close();

    if( !m_bIsReadingFromMemory )
        SAFE_DELETE_ARRAY( m_pwfx );
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::Open()
// Desc: Opens a wave file for reading
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Open( LPTSTR strFileName, WAVEFORMATEX* pwfx, DWORD dwFlags )
{
    HRESULT hr;

    m_dwFlags = dwFlags;
    m_bIsReadingFromMemory = FALSE;

    if( m_dwFlags == WAVEFILE_READ )
    {
        if( strFileName == NULL )
            return E_INVALIDARG;
        SAFE_DELETE_ARRAY( m_pwfx );

        m_hmmio = mmioOpen( strFileName, NULL, MMIO_ALLOCBUF | MMIO_READ );

        if( NULL == m_hmmio )
        {
            HRSRC   hResInfo;
            HGLOBAL hResData;
            DWORD   dwSize;
            VOID*   pvRes;

            //loading failed, try loading as resource
            if( NULL == ( hResInfo = FindResource( NULL, strFileName, TEXT("WAVE") ) ) )
            {
                if( NULL == ( hResInfo = FindResource( NULL, strFileName, TEXT("WAV") ) ) )
                    return DXTRACE_ERR( TEXT("FindResource"), E_FAIL );
            }

            if( NULL == ( hResData = LoadResource( NULL, hResInfo ) ) )
                return DXTRACE_ERR( TEXT("LoadResource"), E_FAIL );

            if( 0 == ( dwSize = SizeofResource( NULL, hResInfo ) ) ) 
                return DXTRACE_ERR( TEXT("SizeofResource"), E_FAIL );

            if( NULL == ( pvRes = LockResource( hResData ) ) )
                return DXTRACE_ERR( TEXT("LockResource"), E_FAIL );

            m_pResourceBuffer = new CHAR[ dwSize ];
            memcpy( m_pResourceBuffer, pvRes, dwSize );

            MMIOINFO mmioInfo;
            ZeroMemory( &mmioInfo, sizeof(mmioInfo) );
            mmioInfo.fccIOProc = FOURCC_MEM;
            mmioInfo.cchBuffer = dwSize;
            mmioInfo.pchBuffer = (CHAR*) m_pResourceBuffer;

            m_hmmio = mmioOpen( NULL, &mmioInfo, MMIO_ALLOCBUF | MMIO_READ );
        }

        if( FAILED( hr = ReadMMIO() ) )
        {
            // ReadMMIO will fail if not wav
            mmioClose( m_hmmio, 0 );
            return DXTRACE_ERR( TEXT("ReadMMIO"), hr );
        }

        if( FAILED( hr = ResetFile() ) )
            return DXTRACE_ERR( TEXT("ResetFile"), hr );

        // after reset size of wav file is m_ck.cksize - store it 
        m_dwSize = m_ck.cksize;
    }
    else
    {
        m_hmmio = mmioOpen( strFileName, NULL, MMIO_ALLOCBUF  | 
                                                  MMIO_READWRITE | 
                                                  MMIO_CREATE );
        if( NULL == m_hmmio )
            return DXTRACE_ERR( TEXT("mmioOpen"), E_FAIL );

        if( FAILED( hr = WriteMMIO( pwfx ) ) )
        {
            mmioClose( m_hmmio, 0 );
            return DXTRACE_ERR( TEXT("WriteMMIO"), hr );
        }
                        
        if( FAILED( hr = ResetFile() ) )
            return DXTRACE_ERR( TEXT("ResetFile"), hr );
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::OpenFromMemory()
// Desc: copy data to CWaveFile member variable from memory
//-----------------------------------------------------------------------------
HRESULT CWaveFile::OpenFromMemory( BYTE* pbData, ULONG ulDataSize, 
                                   WAVEFORMATEX* pwfx, DWORD dwFlags )
{
    m_pwfx       = pwfx;
    m_ulDataSize = ulDataSize;
    m_pbData     = pbData;
    m_pbDataCur  = m_pbData;
    m_bIsReadingFromMemory = TRUE;
    
    if( dwFlags != WAVEFILE_READ )
        return E_NOTIMPL;       
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::ReadMMIO()
// Desc: Support function for reading from a multimedia I/O stream.
//       m_hmmio must be valid before calling.  This function uses it to
//       update m_ckRiff, and m_pwfx. 
//-----------------------------------------------------------------------------
HRESULT CWaveFile::ReadMMIO()
{
    MMCKINFO        ckIn;           // chunk info. for general use.
    PCMWAVEFORMAT   pcmWaveFormat;  // Temp PCM structure to load in.       

    m_pwfx = NULL;

    if( ( 0 != mmioDescend( m_hmmio, &m_ckRiff, NULL, 0 ) ) )
        return DXTRACE_ERR( TEXT("mmioDescend"), E_FAIL );

    // check valid wav
    if( (m_ckRiff.ckid != FOURCC_RIFF) ||
        (m_ckRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E') ) )
        return DXTRACE_ERR( TEXT("mmioFOURCC"), E_FAIL ); 

    //search the input for the 'fmt ' chunk
    ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if( 0 != mmioDescend( m_hmmio, &ckIn, &m_ckRiff, MMIO_FINDCHUNK ) )
        return DXTRACE_ERR( TEXT("mmioDescend"), E_FAIL );

    // expect 'fmt' chunk to be large as <PCMWAVEFORMAT>
    //ignore additional parameters
       if( ckIn.cksize < (LONG) sizeof(PCMWAVEFORMAT) )
           return DXTRACE_ERR( TEXT("sizeof(PCMWAVEFORMAT)"), E_FAIL );

    //read  'fmt ' chunk into <pcmWaveFormat>.
    if( mmioRead( m_hmmio, (HPSTR) &pcmWaveFormat, 
                  sizeof(pcmWaveFormat)) != sizeof(pcmWaveFormat) )
        return DXTRACE_ERR( TEXT("mmioRead"), E_FAIL );

    // allocate waveformatex, but if  not pcm format, read the next word, allocate extra bytes
    if( pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM )
    {
        m_pwfx = (WAVEFORMATEX*)new CHAR[ sizeof(WAVEFORMATEX) ];
        if( NULL == m_pwfx )
            return DXTRACE_ERR( TEXT("m_pwfx"), E_FAIL );

        //copy bytes from the pcm structure to the waveformatex structure
        memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
        m_pwfx->cbSize = 0;
    }
    else
    {
		//read length extra bytes
        WORD cbExtraBytes = 0L;
        if( mmioRead( m_hmmio, (CHAR*)&cbExtraBytes, sizeof(WORD)) != sizeof(WORD) )
            return DXTRACE_ERR( TEXT("mmioRead"), E_FAIL );

        m_pwfx = (WAVEFORMATEX*)new CHAR[ sizeof(WAVEFORMATEX) + cbExtraBytes ];
        if( NULL == m_pwfx )
            return DXTRACE_ERR( TEXT("new"), E_FAIL );

        //copy bytes from  pcm structure to the waveformatex structure
        memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
        m_pwfx->cbSize = cbExtraBytes;

        //read extra bytes into structure if cbExtraAlloc != 0.
        if( mmioRead( m_hmmio, (CHAR*)(((BYTE*)&(m_pwfx->cbSize))+sizeof(WORD)),
                      cbExtraBytes ) != cbExtraBytes )
        {
            SAFE_DELETE( m_pwfx );
            return DXTRACE_ERR( TEXT("mmioRead"), E_FAIL );
        }
    }

    //ascendinput file out of 'fmt ' chunk
    if( 0 != mmioAscend( m_hmmio, &ckIn, 0 ) )
    {
        SAFE_DELETE( m_pwfx );
        return DXTRACE_ERR( TEXT("mmioAscend"), E_FAIL );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::GetSize()
// Desc: Retuns the size of the read access wave file 
//-----------------------------------------------------------------------------
DWORD CWaveFile::GetSize()
{
    return m_dwSize;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::ResetFile()
// Desc: Resets the internal m_ck pointer so reading starts from the 
//       beginning of the file again 
//-----------------------------------------------------------------------------
HRESULT CWaveFile::ResetFile()
{
    if( m_bIsReadingFromMemory )
    {
        m_pbDataCur = m_pbData;
    }
    else 
    {
        if( m_hmmio == NULL )
            return CO_E_NOTINITIALIZED;

        if( m_dwFlags == WAVEFILE_READ )
        {
            //seek to data
            if( -1 == mmioSeek( m_hmmio, m_ckRiff.dwDataOffset + sizeof(FOURCC),
                            SEEK_SET ) )
                return DXTRACE_ERR( TEXT("mmioSeek"), E_FAIL );
			//search input file for the 'data' chunk
            m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
            if( 0 != mmioDescend( m_hmmio, &m_ck, &m_ckRiff, MMIO_FINDCHUNK ) )
              return DXTRACE_ERR( TEXT("mmioDescend"), E_FAIL );
        }
        else
        {
            //create'data' chunk holding the waveform samples  
            m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
            m_ck.cksize = 0;

            if( 0 != mmioCreateChunk( m_hmmio, &m_ck, 0 ) ) 
                return DXTRACE_ERR( TEXT("mmioCreateChunk"), E_FAIL );

            if( 0 != mmioGetInfo( m_hmmio, &m_mmioinfoOut, 0 ) )
                return DXTRACE_ERR( TEXT("mmioGetInfo"), E_FAIL );
        }
    }
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::Read()
// Desc: Reads section of data from a wave file into pBuffer and returns 
//       how much read in pdwSizeRead, reading not more than dwSizeToRead.
//       This uses m_ck to determine where to start reading from.  So 
//       subsequent calls will be continue where the last left off unless 
//       Reset() is called.
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Read( BYTE* pBuffer, DWORD dwSizeToRead, DWORD* pdwSizeRead )
{
    if( m_bIsReadingFromMemory )
    {
        if( m_pbDataCur == NULL )
            return CO_E_NOTINITIALIZED;
        if( pdwSizeRead != NULL )
            *pdwSizeRead = 0;

        if( (BYTE*)(m_pbDataCur + dwSizeToRead) > 
            (BYTE*)(m_pbData + m_ulDataSize) )
        {
            dwSizeToRead = m_ulDataSize - (DWORD)(m_pbDataCur - m_pbData);
        }
        
        CopyMemory( pBuffer, m_pbDataCur, dwSizeToRead );
        
        if( pdwSizeRead != NULL )
            *pdwSizeRead = dwSizeToRead;

        return S_OK;
    }
    else 
    {
		//current status of m_hmmio
        MMIOINFO mmioinfoIn; 

        if( m_hmmio == NULL )
            return CO_E_NOTINITIALIZED;
        if( pBuffer == NULL || pdwSizeRead == NULL )
            return E_INVALIDARG;

        if( pdwSizeRead != NULL )
            *pdwSizeRead = 0;

        if( 0 != mmioGetInfo( m_hmmio, &mmioinfoIn, 0 ) )
            return DXTRACE_ERR( TEXT("mmioGetInfo"), E_FAIL );
                
        UINT cbDataIn = dwSizeToRead;
        if( cbDataIn > m_ck.cksize ) 
            cbDataIn = m_ck.cksize;       

        m_ck.cksize -= cbDataIn;
    
        for( DWORD cT = 0; cT < cbDataIn; cT++ )
        {
            // cpy bytes from IO to buffer
            if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
            {
                if( 0 != mmioAdvance( m_hmmio, &mmioinfoIn, MMIO_READ ) )
                    return DXTRACE_ERR( TEXT("mmioAdvance"), E_FAIL );

                if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
                    return DXTRACE_ERR( TEXT("mmioinfoIn.pchNext"), E_FAIL );
            }

            //the actual copy
            *((BYTE*)pBuffer+cT) = *((BYTE*)mmioinfoIn.pchNext);
            mmioinfoIn.pchNext++;
        }

        if( 0 != mmioSetInfo( m_hmmio, &mmioinfoIn, 0 ) )
            return DXTRACE_ERR( TEXT("mmioSetInfo"), E_FAIL );

        if( pdwSizeRead != NULL )
            *pdwSizeRead = cbDataIn;

        return S_OK;
    }
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::Close()
// Desc: Closes the wave file 
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Close()
{
    if( m_dwFlags == WAVEFILE_READ )
    {
        mmioClose( m_hmmio, 0 );
        m_hmmio = NULL;
        SAFE_DELETE_ARRAY( m_pResourceBuffer );
    }
    else
    {
        m_mmioinfoOut.dwFlags |= MMIO_DIRTY;

        if( m_hmmio == NULL )
            return CO_E_NOTINITIALIZED;

        if( 0 != mmioSetInfo( m_hmmio, &m_mmioinfoOut, 0 ) )
            return DXTRACE_ERR( TEXT("mmioSetInfo"), E_FAIL );
    
        // write data chunk size
        if( 0 != mmioAscend( m_hmmio, &m_ck, 0 ) )
            return DXTRACE_ERR( TEXT("mmioAscend"), E_FAIL );
    
        // do this instead
        if( 0 != mmioAscend( m_hmmio, &m_ckRiff, 0 ) )
            return DXTRACE_ERR( TEXT("mmioAscend"), E_FAIL );
        
        mmioSeek( m_hmmio, 0, SEEK_SET );

        if( 0 != (INT)mmioDescend( m_hmmio, &m_ckRiff, NULL, 0 ) )
            return DXTRACE_ERR( TEXT("mmioDescend"), E_FAIL );
    
        m_ck.ckid = mmioFOURCC('f', 'a', 'c', 't');

        if( 0 == mmioDescend( m_hmmio, &m_ck, &m_ckRiff, MMIO_FINDCHUNK ) ) 
        {
            DWORD dwSamples = 0;
            mmioWrite( m_hmmio, (HPSTR)&dwSamples, sizeof(DWORD) );
            mmioAscend( m_hmmio, &m_ck, 0 ); 
        }
    
        //ascend the output file out of the 'RIFF' chunk causingthe chunk size of the 'RIFF' chunk to be written
        if( 0 != mmioAscend( m_hmmio, &m_ckRiff, 0 ) )
            return DXTRACE_ERR( TEXT("mmioAscend"), E_FAIL );
    
        mmioClose( m_hmmio, 0 );
        m_hmmio = NULL;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::WriteMMIO()
// Desc: Support function for reading from a multimedia I/O stream
//       pwfxDest is the WAVEFORMATEX for this new wave file.  
//       m_hmmio must be valid before calling.  This function uses it to
//       update m_ckRiff, and m_ck.  
//-----------------------------------------------------------------------------
HRESULT CWaveFile::WriteMMIO( WAVEFORMATEX *pwfxDest )
{
    DWORD    dwFactChunk; //actual fact check
    MMCKINFO ckOut1;
    
    dwFactChunk = (DWORD)-1;

    // Create the output file RIFF chunk of form type 'WAVE'.
    m_ckRiff.fccType = mmioFOURCC('W', 'A', 'V', 'E');       
    m_ckRiff.cksize = 0;

    if( 0 != mmioCreateChunk( m_hmmio, &m_ckRiff, MMIO_CREATERIFF ) )
        return DXTRACE_ERR( TEXT("mmioCreateChunk"), E_FAIL );
    
    // We are now descended into the 'RIFF' chunk we just created.
    // Now create the 'fmt ' chunk. Since we know the size of this chunk,
    // specify it in the MMCKINFO structure so MMIO doesn't have to seek
    // back and set the chunk size after ascending from the chunk.
    m_ck.ckid = mmioFOURCC('f', 'm', 't', ' ');
    m_ck.cksize = sizeof(PCMWAVEFORMAT);   

    if( 0 != mmioCreateChunk( m_hmmio, &m_ck, 0 ) )
        return DXTRACE_ERR( TEXT("mmioCreateChunk"), E_FAIL );
    
    // writePCMWAVEFORMAT structure to the 'fmt ' chunk if same type
    if( pwfxDest->wFormatTag == WAVE_FORMAT_PCM )
    {
        if( mmioWrite( m_hmmio, (HPSTR) pwfxDest, 
                       sizeof(PCMWAVEFORMAT)) != sizeof(PCMWAVEFORMAT))
            return DXTRACE_ERR( TEXT("mmioWrite"), E_FAIL );
    }   
    else 
    {
        //writevariable length size
        if( (UINT)mmioWrite( m_hmmio, (HPSTR) pwfxDest, 
                             sizeof(*pwfxDest) + pwfxDest->cbSize ) != 
                             ( sizeof(*pwfxDest) + pwfxDest->cbSize ) )
            return DXTRACE_ERR( TEXT("mmioWrite"), E_FAIL );
    }  
    
    //ascend out of 'fmt ' chunk, back into  'RIFF' chunk
    if( 0 != mmioAscend( m_hmmio, &m_ck, 0 ) )
        return DXTRACE_ERR( TEXT("mmioAscend"), E_FAIL );
    
    //create the fact chunk, This is filled in when the close routine called
    ckOut1.ckid = mmioFOURCC('f', 'a', 'c', 't');
    ckOut1.cksize = 0;

    if( 0 != mmioCreateChunk( m_hmmio, &ckOut1, 0 ) )
        return DXTRACE_ERR( TEXT("mmioCreateChunk"), E_FAIL );
    
    if( mmioWrite( m_hmmio, (HPSTR)&dwFactChunk, sizeof(dwFactChunk)) != 
                    sizeof(dwFactChunk) )
         return DXTRACE_ERR( TEXT("mmioWrite"), E_FAIL );
    
    //ascent fact chunk
    if( 0 != mmioAscend( m_hmmio, &ckOut1, 0 ) )
        return DXTRACE_ERR( TEXT("mmioAscend"), E_FAIL );
       
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::Write()
// Desc: Writes data to the open wave file
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Write( UINT nSizeToWrite, BYTE* pbSrcData, UINT* pnSizeWrote )
{
    UINT cT;

    if( m_bIsReadingFromMemory )
        return E_NOTIMPL;
    if( m_hmmio == NULL )
        return CO_E_NOTINITIALIZED;
    if( pnSizeWrote == NULL || pbSrcData == NULL )
        return E_INVALIDARG;

    *pnSizeWrote = 0;
    
    for( cT = 0; cT < nSizeToWrite; cT++ )
    {       
        if( m_mmioinfoOut.pchNext == m_mmioinfoOut.pchEndWrite )
        {
            m_mmioinfoOut.dwFlags |= MMIO_DIRTY;
            if( 0 != mmioAdvance( m_hmmio, &m_mmioinfoOut, MMIO_WRITE ) )
                return DXTRACE_ERR( TEXT("mmioAdvance"), E_FAIL );
        }

        *((BYTE*)m_mmioinfoOut.pchNext) = *((BYTE*)pbSrcData+cT);
        (BYTE*)m_mmioinfoOut.pchNext++;

        (*pnSizeWrote)++;
    }

    return S_OK;
}




