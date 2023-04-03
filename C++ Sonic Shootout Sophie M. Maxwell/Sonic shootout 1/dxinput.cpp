#include "dxinput.h"

#define BUTTON_DOWN(obj, button) (obj.rgbButtons[button]&0x80)

LPDIRECTINPUT8 dinput;
LPDIRECTINPUTDEVICE8 dimouse;
LPDIRECTINPUTDEVICE8 dikeyboard;
LPDIRECTINPUTDEVICE8 dijoystick;
DIMOUSESTATE mouse_state;

//keyboard input
char keys[256];

int Init_DirectInput(HWND hwnd)
{
//initialise DirectInput obj
	HRESULT result=DirectInput8Create(
		GetModuleHandle(NULL),
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&dinput,
		NULL
		);

	if(result!=DI_OK)
		return 0;

	//initialise mouse
	result=dinput->CreateDevice(GUID_SysMouse, &dimouse, NULL);
	if(result!=DI_OK)
		return 0;

	//initilaise keys
	result=dinput->CreateDevice(GUID_SysKeyboard, &dikeyboard, NULL);
	if(result!=DI_OK)
		return 0;

	//returnv true
	return 1;
}

int Init_Mouse(HWND hwnd)
{
	//set the data format (mouse)
	HRESULT result=dimouse->SetDataFormat(&c_dfDIMouse);
	if(result!=DI_OK)
		return 0;

	//set the coop level and hide pointer
	result = dimouse->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	if(result!=DI_OK)
		return 0;

	//get mouse
	result=dimouse->Acquire();
	if(result!=DI_OK)
		return 0;

	//return true
	return 1;
}
//mouse X pos
int Mouse_X()
{
	return mouse_state.lX;
}
//mouse Y pos
int Mouse_Y()
{
	return mouse_state.lY;
}
//button click
int Mouse_Button(int button)
{
	return BUTTON_DOWN(mouse_state, button);
}
//mouse active? 
void Poll_Mouse() {
	dimouse->GetDeviceState(sizeof(mouse_state), (LPVOID)&mouse_state);
}
//kill the mouse
void Kill_Mouse()
{
	if(dimouse!=NULL)
	{
		dimouse->Unacquire();
		dimouse->Release();
		dimouse=NULL;
	}
}

int Init_Keyboard(HWND hwnd)
{
	//set the data format (keyboard)
	HRESULT result=dikeyboard->SetDataFormat(&c_dfDIKeyboard);
	if(result!=DI_OK)
		return 0;

	//set the coop lvl
	result=dikeyboard->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	if(result!=DI_OK)
		return 0;

	//get keyboard
	result=dikeyboard->Acquire();
	if(result!=DI_OK)
		return 0;

	//return true
	return 1;
}
//keyboard active?
void Poll_Keyboard()
{
	dikeyboard->GetDeviceState(sizeof(keys), (LPVOID)&keys);
}

int Key_Down(int key)
{
	return (keys[key] & 0x80);
}
//kill keyboard
void Kill_Keyboard()
{
	if (dikeyboard != NULL)
	{
		dikeyboard->Unacquire();
		dikeyboard->Release();
		dikeyboard = NULL;
	}
}