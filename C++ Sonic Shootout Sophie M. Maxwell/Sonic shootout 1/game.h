//Header file for GAME

//Compile the code once and allow it to be accessed multiple times
#ifndef _GAME_H
#define _GAME_H


//Windows DirectX header files for graphics, audio, input ect 
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr.h>
#include <dsound.h>
#include <Windows.h>
#include <d3dx9math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>


//Header files for the game framework (self-created) 
#include "dxaudio.h"
#include "dxgraphics.h"
#include "dxinput.h"

//title of the main application to be displayed at the top of the game window 
#define APPTITLE "Sonic Shootout - Sophie M. Maxwell"

//screen setup
#define FULLSCREEN 0 //Use 0 when testing, 1 for fullscreen (yes(1)/no(0))
#define SCREEN_WIDTH 640 //Width of game window
#define SCREEN_HEIGHT 480 //height of game window

//Scrolling background setup
#define TILEWIDTH 640 //Width of the visible area
#define TILEHEIGHT 480 //height of the visible area
#define MAPWIDTH 6 //width of the map
#define MAPHEIGHT 1 //height of the map
#define GAMEWORLDWIDTH (TILEWIDTH*MAPWIDTH) //Width of the entire world
#define GAMEWORLDHEIGHT (TILEHEIGHT*MAPHEIGHT) //height of the entire world

//data for object placement (game grid) Values based on screen size
#define PLAYSEGMENTWIDTH 64  //screen width is 640 - this creates 10 columns
#define PLAYSEGMENTHEIGHT 32 //screen width is 480 - this created 15 rows
#define PLAYTILEWIDTH 10 //10 columns across
#define PLAYTILEHEIGHT 15 //15 rows down 
#define PLAYWORLDWIDTH (PLAYSEGMENTWIDTH*PLAYTILEWIDTH) //the full width
#define PLAYWORLDHEIGHT (PLAYSEGMENTHEIGHT*PLAYTILEHEIGHT) //full height

//game states
#define GAMEPLAY_STATE 0 //This is when the game is actually running
#define PAUSE_STATE 1 //This is where the game is paused
#define TITLE_STATE 2 //This is when the game is on the title screen
#define GAME_OVER 3 //This is when the game is over

//sonic states
#define SONIC_PLAY 0
#define SONIC_INV 1
#define SONIC_DIE 2
#define SONIC_OVER 3

//scrolling map functions 
void ScrollScreen(); //Function to make the screen scroll 
void BuildGameWorld(); //Function to create the game world (load everything)
void DrawTile(LPDIRECT3DSURFACE9, 
	int, int, int, int,
	LPDIRECT3DSURFACE9, int, int); //Fill the game world with bitmap tiles

//function prototype
int Game_Init(HWND); //Initialize the game
void Game_Run(HWND); //Run the game
void Game_End(HWND); //End the game

//define sprite struct, define integers 
typedef struct {
	int x, y; 
	int width, height;
	int movex, movey;
	int curframe, lastframe;
	int animdelay, animcount;
	int scalex, scaley;
	int rotation, rototerate;
} 
SPRITE;
//End of game header 
#endif