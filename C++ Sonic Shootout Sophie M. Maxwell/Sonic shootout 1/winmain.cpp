//GAME FRAMEWORKK

#include <d3d9.h>
#include <d3dx9.h>
#include <time.h>
#include <stdio.h>
#include <tchar.h>
#include "dxgraphics.h"
#include "game.h"
#include "dxaudio.h"
#include "dxinput.h"

//callback window function
LRESULT WINAPI WinProc(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_DESTROY:
		
	//release Direct3D device
		if(d3ddev!=NULL)
			d3ddev->Release();

	//release Direct3D object
		if(d3d!=NULL)
			d3d->Release();

	//call the front-end shutdown function
		Game_End(hWnd);

		//release input objs
		Kill_Keyboard();
		Kill_Mouse();
		if(dinput!=NULL)
			dinput->Release();
		
		//release sounds

		//tell Win to kill program
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd,msg,wParam,lParam);
}

//********helper function
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	//create window struct
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);

	//fill the struct with informatiopn
	wc.style			=CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc		=(WNDPROC)WinProc;
	wc.cbClsExtra		=0;
	wc.cbWndExtra		=0;
	wc.hInstance		=hInstance;
	wc.hIcon			=NULL;
	wc.hCursor			=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground	=(HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName		=NULL;
	wc.lpszClassName	=_T(APPTITLE);
	wc.hIconSm			=NULL;

	//set up the window w/ class info
	return RegisterClassEx(&wc);

}

//entry point for win program
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	HWND hWnd;

	//register class
	MyRegisterClass(hInstance);

	//is screen windowed or full screen?
	DWORD style;
	if(FULLSCREEN)
		style=WS_EX_TOPMOST|WS_VISIBLE|WS_POPUP;
	else
		style=WS_OVERLAPPED;

	//create a new window
	hWnd=CreateWindowA(
		APPTITLE,			//window class
		APPTITLE,			//title bar
		style,				//window style
		CW_USEDEFAULT,		//xpos of window
		CW_USEDEFAULT,		//ypos of window
		SCREEN_WIDTH,		//width of window
		SCREEN_HEIGHT,		//height of window
		NULL,				//parentwindow
		NULL,				//menu 
		hInstance,			//application instence
		NULL
		);

	//error?
	if(!hWnd)
		return FALSE;

	//display window
	ShowWindow(hWnd,nCmdShow);
	UpdateWindow(hWnd);

	if(!Init_Direct3D(hWnd,SCREEN_WIDTH,SCREEN_HEIGHT,FULLSCREEN))
		return 0;

	//initialise DirectSound
	if(!Init_DirectSound(hWnd))
	{
		MessageBoxA(hWnd, "Error initialising DirectSound","Error",MB_OK);
		return 0;
	}

	//initialise DirectInput
	if(!Init_DirectInput(hWnd))
	{
		MessageBoxA(hWnd, "Error initialising DirectInput","Error",MB_OK);
		return 0;
	}

	//initialise game
	if (!Game_Init(hWnd))
	{
		MessageBoxA(hWnd, "Error initializing the game", "Error", MB_OK);
		return 0;
	}

	//main msg loop
	int done = 0;
	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
		//quit message?
				if (msg.message == WM_QUIT)
					done = 1;
			//decode and pass message to WndProc
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
			//process loop
			Game_Run(hWnd);
		}
	return msg.wParam;
	}
