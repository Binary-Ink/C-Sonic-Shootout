#include "dxaudio.h"

CSoundManager *dsound;

int Init_DirectSound(HWND hwnd) {
	HRESULT result;

	//create DirectSound managed obj
	dsound = new CSoundManager();

	//initialise DirectSound
	result=dsound->Initialize(hwnd,DSSCL_PRIORITY);
	if(result!=DS_OK)
		return 0;

	//set primary buffer format
	result=dsound->SetPrimaryBufferFormat(2,22050,16);

	//return false
	if(result!=DS_OK)
		return 0;

	//return true
	return 1;
}

CSound *LoadSound(wchar_t* filename)
{
	HRESULT result;

	//create local ref for wave data
	CSound *wave;

	//try loadingthe wav file
	result=dsound->Create(&wave,filename);
	if(result!=DS_OK)
		return NULL;

	//return wave
	return wave;
}

void PlaySound(CSound *sound)
{
	sound->Play();
}

void LoopSound(CSound *sound)
{
	sound->Play(0, DSBPLAY_LOOPING);
}

void StopSound(CSound *sound) 
{
	sound->Stop();
}