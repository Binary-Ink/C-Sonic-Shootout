#ifndef _DXAUDIO_H
#define _DXAUDIO_H 1

#include "dsutil.h"

//primary DirectSound obj
extern CSoundManager *dsound;

//function prototypes
int Init_DirectSound(HWND);
CSound *LoadSound(wchar_t *);
void PlaySound(CSound*);

void LoopSound(CSound*);
void StopSound(CSound*);

#endif