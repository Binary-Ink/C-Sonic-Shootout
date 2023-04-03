//Direct3D framework source code

#include <d3d9.h>
#include <d3dx9.h>
#include "dxgraphics.h"

//variables
LPDIRECT3D9 d3d = NULL;
LPDIRECT3DDEVICE9 d3ddev = NULL;
LPDIRECT3DSURFACE9 backbuffer=NULL;

int Init_Direct3D(HWND hwnd, int width, int height,  int fullscreen)
{
	d3d=Direct3DCreate9(D3D_SDK_VERSION);
	if(d3d==NULL)
	{
		MessageBoxA(hwnd,"Error initialising Direct3D","Error",MB_OK);
		return 0;
	}
	//set Direct3D parameters
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	d3dpp.Windowed = 1;
	d3dpp.SwapEffect =D3DSWAPEFFECT_COPY;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount=1;
	d3dpp.BackBufferWidth=width;
	d3dpp.BackBufferHeight=height;
	d3dpp.hDeviceWindow = hwnd;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	//create Direct3D device
	d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hwnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&d3ddev
		);

	if(d3ddev==NULL)
	{
		MessageBoxA(hwnd, "Error creating Direct3D device", "Error", MB_OK);
		return 0;
	}

	//clear backbuffer to 000 (black)
	d3ddev->Clear(0,NULL,D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0),1.0f,0);

	//create pointer tobackbuffer
	d3ddev->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&backbuffer);

	return 1;
}

LPDIRECT3DSURFACE9 LoadSurface(wchar_t *filename, D3DCOLOR transcolor)
{
	LPDIRECT3DSURFACE9 image=NULL;
	D3DXIMAGE_INFO info;
	HRESULT result;

	//get height/width of bitmap
	result = D3DXGetImageInfoFromFile(filename,&info);
	if(result!=D3D_OK)
		return NULL;

	//create surface
	result=d3ddev->CreateOffscreenPlainSurface(
		info.Width,		//width of the surface
		info.Height,	//height of the surface
		D3DFMT_X8R8G8B8,//surface format
		D3DPOOL_DEFAULT,//memory pool 
		&image,			//pointer to surface
		NULL
		);

	if(result!=D3D_OK)
		return NULL;

	//load surface from file into new surface
	result=D3DXLoadSurfaceFromFile(
		image, //destination surface
		NULL, //destination palette
		NULL, //destination rectangle
		filename, //source filename
		NULL, //source rectangle
		D3DX_DEFAULT, //controls how image is filtered
		transcolor, //transparency 
		NULL); //source image info (NULL)

	//check file loaded 
	if(result!=D3D_OK)
		return NULL;

	return image;
}

LPDIRECT3DTEXTURE9 LoadTexture(char* filename, D3DCOLOR transcolor)
{
	//texture pointr
	LPDIRECT3DTEXTURE9 texture = NULL;

	//the struct reading bitmap info
	D3DXIMAGE_INFO info;

	//standard win return value
	HRESULT result;

	//get width/height bitmap
	result = D3DXGetImageInfoFromFileA(filename,&info);
	if(result!=D3D_OK)
		return NULL;

	//create the new texture through loading bitmap
	D3DXCreateTextureFromFileExA(
		d3ddev,						//Direct3D device object
		filename,					//bitmap filename
		info.Width,					//bitmap image width
		info.Height,				//bitmap image height
		1,							//mip-map levels (1 for no chain)
		D3DPOOL_DEFAULT,			//the type of surface (standard)
		D3DFMT_UNKNOWN,				//surface format (default)
		D3DPOOL_DEFAULT,			//memory class for the texture
		D3DX_DEFAULT,				//image filter
		D3DX_DEFAULT,				//mip filter
		transcolor,					//color key for transparfancy
		&info,						//bitmap file info from loaded file
		NULL,						//colour palette
		&texture					//destination texture
		);

	//check loaded correctly
	if(result!=D3D_OK)
		return NULL;

	return texture;
}