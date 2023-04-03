#ifndef _DXGRAPHICS_H
#define _DXGRAPHICS_H


//function prototype
int Init_Direct3D(HWND, int, int, int);
LPDIRECT3DSURFACE9 LoadSurface(wchar_t*,D3DCOLOR);
LPDIRECT3DTEXTURE9 LoadTexture(char*,D3DCOLOR);

//variables
extern LPDIRECT3D9 d3d;
extern LPDIRECT3DDEVICE9 d3ddev;
extern LPDIRECT3DSURFACE9 backbuffer;

#endif

