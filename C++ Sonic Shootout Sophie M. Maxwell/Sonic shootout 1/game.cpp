//SONIC SHOOTOUT Sophie M. Maxwell 
//Main code for GAME


//header files and libraries to be included
#include "game.h"
#include "player.h"
#include "playerfire.h"
#include "kamikaze.h"
#include "explosion.h"
#include <tchar.h>
#include <vector>
#include <algorithm>
#include <functional>

//Using the standard namespace
using namespace std;



//***********BACKGROUND
//game scroll definitions
int ScrollX, ScrollY;
int SpeedX, SpeedY;

//game map definitions
int MAPDATA[MAPWIDTH*MAPHEIGHT] =
{
	0, 1, 0, 1, 2, 3
};

//game 'areas'
LPDIRECT3DSURFACE9 gameworld; //actual game
LPDIRECT3DSURFACE9 titlescreen; //titlescreen

//***********Game object placement
//integers
int levelProgressPoint = 0;
int levelDataRow;

//game data for objects
int PLAYDATA[PLAYTILEWIDTH*PLAYTILEHEIGHT] = //define array for game sections (10 columns with 15 rows) 
{
	//grid defines first 'wave' of enemies will be in a line (1 defines enemy)
	//1 //2 //3 //4 //5 //6 //7 //8 //9 //10
	1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  //1
	1,  0,  0,  0,  0,  0,  0,  0,  1,  0,  //2
	1,  0,  0,  0,  0,  0,  0,  1,  0,  0,  //3
	1,  0,  0,  0,  0,  0,  1,  0,  0,  0,  //4
	1,  0,  0,  0,  0,  1,  0,  0,  0,  0,  //5
	1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  //6
	1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  //7
	1,  0,  1,  0,  0,  0,  0,  0,  0,  0,  //8
	1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  //9
	1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  //10
	1,  0,  0,  0,  0,  1,  0,  0,  0,  0,  //11
	1,  0,  0,  0,  0,  0,  1,  0,  0,  0,  //12
	1,  0,  0,  0,  0,  0,  0,  1,  0,  0,  //13
	1,  0,  0,  0,  0,  0,  0,  0,  1,  0,  //14
	1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  //15
};


//game sprite handler
LPD3DXSPRITE sprite_handler;

//game misc definitions
long start = GetTickCount();
long fireDelay = GetTickCount();
long levelScroll = GetTickCount();//timer for how fast objects scroll 
HRESULT result;
bool enterDownFlag = false;//If a key is held down, don't continously repeat the action 
int gameState = TITLE_STATE;//Global variable to hold the current game state

//game sounds
CSound *sound_laser;
CSound *sound_pop;

//game fonts
LPD3DXFONT m_font;

//game display functions definitions
int score = 0;
void DisplayScore();
void DisplayPause();
void DisplayHealth();
void DisplayLives();
void DisplayGameOver();

//game object definitions
vector<PLAYERFIRE> playerFireVector;
vector<KAMIKAZE> kamikazeVector;
vector<EXPLOSION> explosionVector;
PLAYER sonic;

//*************collision detection
int Collision(SPRITE* sprite1, SPRITE* sprite2)//takes 2 sprite pointers (sprite1, sprite2) 
{
	RECT rect1; //set rect1 for sprite1
	rect1.left = sprite1->x + 1; //set left
	rect1.top = sprite1->y + 1; //set top 
	rect1.right = sprite1->x + sprite1->width - 1; //set right
	rect1.bottom = sprite1->y + sprite1->height - 1; //set bottom 

	RECT rect2; //set rect 2 for sprite 2
	rect2.left = sprite2->x + 1; //set left 
	rect2.top = sprite2->y + 1; //set top
	rect2.right = sprite2->x + sprite2->width - 1; //set right
	rect2.bottom = sprite2->y + sprite2->height - 1; //set bottom

	RECT dest; //to store overlapped area 
	return IntersectRect(&dest, &rect1, &rect2); //return if the sprites have collided (1 for yes, 0 for no)
}

//************Initialise game function
int Game_Init(HWND hwnd)
{
	//set random number seed
	srand(time(NULL));

	//initailise the keyboard
	if (!Init_Keyboard(hwnd)) //if the keyboard can not be initialised
	{
		MessageBoxA(hwnd, "Error initlising Keyboard", "Error", MB_OK); //return error 
		return 0; //return false
	}

	titlescreen = LoadSurface(L"titlescreen.jpg", 0); //load the titlescreen image
	BuildGameWorld(); //run the function to generate the game world

	sound_laser = LoadSound(L"laser.wav"); //play the laser sound
	if (sound_laser == NULL) //if there is no file matching the description
	{
		MessageBoxA(hwnd, "Error crearing laser", "Error", MB_OK); //return error
		return 0; //return false
	}

	sound_pop = LoadSound(L"pop.wav"); //play the pop sound
	if (sound_pop == NULL) //if no file matching description
	{
		MessageBoxA(hwnd, "Error creating pop", "Error", MB_OK); //return error
		return 0; //return false
	}

	//create D3DX font object
	D3DXCreateFont(
		d3ddev, //d3d device
		30, //height
		0, //width
		0, //weight
		FW_NORMAL, //miplevels
		0, //italics
		DEFAULT_CHARSET, //character set
		OUT_DEFAULT_PRECIS, //precision 
		DEFAULT_QUALITY, //quality
		DEFAULT_PITCH FF_DONTCARE, //pitch and family
		TEXT("Hobo std"), //facename
		&m_font); //store for font object 

	sonic = PLAYER(); //set the player to 'sonic' 

	//create sprite handler object 
	result = D3DXCreateSprite(d3ddev, &sprite_handler);
	if (result != D3D_OK) //if the sprite can not be created
	{
		MessageBoxA(hwnd, "Error creating sprite image", "Error", MB_OK); //return an error
		return 0; //return false
	}

	titlescreen = LoadSurface(L"titlescreen.jpg", 0); //load the titlescreen image

	BuildGameWorld(); //run the build world function

	return 1;//return true 
}
//***************************** MAIN GAME LOOP *****************************
void Game_Run(HWND hwnd)
{
	if (d3ddev == NULL)
		return;
	//update keyboared 
	Poll_Keyboard();

	//dalay and next frame to keep state steady
	if (GetTickCount() - start >= 30)
	{
		//reset time
		start = GetTickCount();

		
		if (sonic.getPlayerState() == SONIC_OVER) { //checks to see if sonic is dead
			if (gameState != GAME_OVER) { //If this is not set to game over
				gameState = GAME_OVER; //the game is set to over
			}
		}
		//*********sounds
		if (gameState != GAME_OVER) { //if the game is running
			if (Key_Down(DIK_RETURN))
			{
				//update sound
				if (!enterDownFlag) { 
					if (gameState == TITLE_STATE) {
						gameState = GAMEPLAY_STATE;

					}
					else {
						//stop level sound
						if (gameState == GAMEPLAY_STATE) {
							gameState = PAUSE_STATE;

						}
						else {
							//play level sound
							gameState = GAMEPLAY_STATE;

						}
					}
				}
				enterDownFlag = true;
			}
			else {
				enterDownFlag = false;
			}

		
			ScrollScreen();
			sonic.Update();

			if (GetTickCount() - fireDelay >= 120)
			{

				//reset the timer
				fireDelay = GetTickCount();
				if (Key_Down(DIK_SPACE)) //if space is pressed
				{
					PLAYERFIRE playerFireCopy = PLAYERFIRE(sonic.getX() + 35, sonic.getY() + 5); //fire from sonics location 
					playerFireVector.push_back(playerFireCopy); 
					PlaySound(sound_laser); //play the laser sound
				}
			}

			for (vector<PLAYERFIRE>::iterator it = playerFireVector.begin();
				it != playerFireVector.end();)
			{
				it->Update();
				//if not on screen
				if (it->getX() > SCREEN_WIDTH) {
					//get next iteratopn
					it = playerFireVector.erase(it);
				}
				else {
					it++;
				}
			}
			//*Kamikaze and player collision detection 
			for (vector<KAMIKAZE>::iterator it = kamikazeVector.begin();
				it != kamikazeVector.end();)
			{
				it->Update(&sonic); //update the object 
				if (Collision(&it->getSprite(), &sonic.GetSprite())) //is kamikaze is touching the player?
				{
					//explosion
					EXPLOSION explosionCopy = EXPLOSION(it->getX(), it->getY()); //create an explosion copy
					explosionVector.push_back(explosionCopy);
					PlaySound(sound_pop); //play sound
					sonic.setHealth(sonic.getHealth() - 10); //deduct 10 from sonics health
					it->setSpriteX(-100); //move the explosion sprite off screen
				}
				else {
					if (it->getX() < -55) { //if off screen (55 is the width of kamikaze sprite)
						
						it = kamikazeVector.erase(it);//get next iteration (if erasing)
					}
					else {
						it++; //increase iterateration
					}
				}
			}

			for (vector<EXPLOSION>::iterator it = explosionVector.begin();
				it != explosionVector.end();)
			{
				it->Update();
				if (it->getFrame() > 4) {
					//if expired get next iteration if erasing 
					it = explosionVector.erase(it);
				}
				else {
					it++;
				}
			}
			//player fire + kamikaze collision detetion 
			for (vector<PLAYERFIRE>::iterator it = playerFireVector.begin();
				it != playerFireVector.end(); it++)
			{
				for (vector<KAMIKAZE>::iterator itK = kamikazeVector.begin();
					itK != kamikazeVector.end(); itK++)
				{
					if (Collision(&it->getSprite(), &itK->getSprite()))
					{

						EXPLOSION explosionCopy =
							EXPLOSION(itK->getX() + 35, itK->getY());
						explosionVector.push_back(explosionCopy);
						PlaySound(sound_pop);

						score += 1000;
						itK->setSpriteX(-100);
						it->setSpriteX(1000);
					}
				}
			}


			//*****SCROLL LEVEL
			if (GetTickCount() - levelScroll >= 3000) {

				if (levelProgressPoint < PLAYTILEWIDTH) {

					for (levelDataRow = 0; levelDataRow < PLAYTILEHEIGHT; levelDataRow++) {

						if (PLAYDATA[levelDataRow*PLAYTILEWIDTH + levelProgressPoint] == 1) {

							KAMIKAZE kamikazeCopy = KAMIKAZE(SCREEN_WIDTH + 55, levelDataRow * 32);
							kamikazeVector.push_back(kamikazeCopy);
						}
					}
					levelProgressPoint++;
					if (levelProgressPoint == PLAYTILEWIDTH)
					{
						levelProgressPoint = 0;
					}
					levelScroll = GetTickCount();
				}
			}


		}


	}

	//start rendering
	if (d3ddev->BeginScene())
	{
		//draw logics
		if (gameState == TITLE_STATE) {

			RECT srcRect;
			srcRect.top = 0; 
			srcRect.left = 0;
			srcRect.right = SCREEN_WIDTH;
			srcRect.bottom = SCREEN_HEIGHT;

			d3ddev->StretchRect(titlescreen, &srcRect,
				backbuffer, &srcRect, D3DTEXF_NONE);
		}
		else {
			//set source image size

			RECT r1 = { ScrollX,
				ScrollY,
				ScrollX + SCREEN_WIDTH,
				ScrollY + SCREEN_HEIGHT };

			//set destination rect
			RECT r2 = { 0,0,SCREEN_WIDTH, SCREEN_HEIGHT
			};

			sprite_handler->Begin(D3DXSPRITE_ALPHABLEND);

			d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
			d3ddev->SetRenderState(D3DRS_ALPHATESTENABLE, false);
			d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);


			//player
			sonic.Draw(sprite_handler);

			//player firepower
			for (vector<PLAYERFIRE>::iterator it = playerFireVector.begin();
				it != playerFireVector.end(); ++it)
			{
				it->Draw(sprite_handler);
			}

			//draw enemies 
			for (vector<KAMIKAZE>::iterator it = kamikazeVector.begin();
				it != kamikazeVector.end(); ++it)
			{
				it->Draw(sprite_handler);
			}

			//draw explosions 
			for (vector<EXPLOSION>::iterator it = explosionVector.begin();
				it != explosionVector.end(); ++it)
			{
				it->Draw(sprite_handler);
			}
			sprite_handler->End();

			if (gameState == PAUSE_STATE) { DisplayPause(); }
			if (gameState == GAMEPLAY_STATE) {
	//************HUD display
				DisplayScore();
				DisplayHealth();
				DisplayLives();
			}
			if (gameState == GAME_OVER) {
				DisplayGameOver(); //calls game over function if the game state is equal to game_over
			}

		}

		d3ddev->EndScene();

		//display backbuffer on the screen 
		d3ddev->Present(NULL, NULL, NULL, NULL);

		//check for esc pressed (ends program)
		if (Key_Down(DIK_ESCAPE))
			PostMessage(hwnd, WM_DESTROY, 0, 0);
	}

}







//frees memory before end of game
void Game_End(HWND hwnd)
{

	if (sprite_handler != NULL)
		sprite_handler->Release();

}
//function to build the game world 
void BuildGameWorld()
{
	//declare variables
	int x, y;
	LPDIRECT3DSURFACE9 tiles;

	//load the bitmaps containing tiles
	tiles = LoadSurface(L"bgtexture.png", D3DCOLOR_XRGB(0, 0, 0));
	if (tiles == NULL)
	{
		MessageBoxA(NULL, "Error getting tiles", "Error", 0);
		return;
	}
	//create the scrolling game world
	result = d3ddev->CreateOffscreenPlainSurface(
		GAMEWORLDWIDTH,      //width of surface 
		GAMEWORLDHEIGHT,     //height of surface 
		D3DFMT_X8R8G8B8,     //color mode
		D3DPOOL_DEFAULT,     //memory pool
		&gameworld,          //destination surface
		NULL                 //shared handle - usually NULL
	);

	if (result != D3D_OK)//if result is not equal to
	{
		MessageBoxA(NULL, "Error createing working surface", "Error", 0);//display error
		return;//return false
	}

	//fill the game world bitmap with tiles 
	for (y = 0; y<MAPHEIGHT; y++) {
		for (x = 0; x<MAPWIDTH; x++) {
			DrawTile(tiles, MAPDATA[y*MAPWIDTH + x], TILEWIDTH, TILEHEIGHT,
				MAPWIDTH, gameworld, x*TILEWIDTH, y*TILEHEIGHT);
		}
	}

	//release th tiles as unneeded
	tiles->Release();
}

void DrawTile(
	LPDIRECT3DSURFACE9 source, //source surface image 
	int tilenum,     //tile num
	int width,       //tile width
	int height,     //tile height
	int columns,    //olumns of titles
	LPDIRECT3DSURFACE9 dest, //destination surface 
	int destx,                //destination x
	int desty)  //destination y
{
	//set reptangle 
	RECT r1;
	r1.left = (tilenum % columns) *width;
	r1.top = (tilenum / columns) *height;
	r1.right = r1.left + width;
	r1.bottom = r1.top + height;

	//set destination rect 
	RECT r2 = { destx, desty, destx + width, desty + height };

	//draw tile 
	result = d3ddev->StretchRect(source, &r1, dest, &r2, D3DTEXF_NONE);
	if (result != D3D_OK) {
		TCHAR str[255];
		swprintf_s(str, _T("point x is %d"), tilenum);
		MessageBox(0, str, L"Error creating tile array: ", MB_OK);
	}
}
//function to scroll the screen
void ScrollScreen()
{
	SpeedX = 1;
	//udate horisontal scrolling position and speed 
	ScrollX += SpeedX;
	if (ScrollX < 0)//if ScrollX is less than 0
	{
		ScrollX = 0; //Set scrollX to 0
		SpeedX = 0; //set speed to 0
	}
	else if (ScrollX > GAMEWORLDWIDTH - SCREEN_WIDTH) //if ScrollX is more than the width of the game world minus the width of the screen
	{
		ScrollX = GAMEWORLDWIDTH - SCREEN_WIDTH; //set scrollX to the gameworld width minus the width of the screen
		SpeedX = 0;//set speed to 0
	}
	//update verticle scrolling 
	ScrollY += SpeedY;
	if (ScrollY < 0)//if scrollY is less than 0
	{
		ScrollY = 0;//set scrollY to 0
		SpeedY = 0;//set the speed to 0
	}
	else if (ScrollY > GAMEWORLDHEIGHT - SCREEN_HEIGHT)//if scrollY is more than the game world height minus the screen height
	{
		ScrollY = GAMEWORLDHEIGHT - SCREEN_HEIGHT;//set scrollY to the height of the game world minus the height of the screen
		SpeedY = 0;//set the speed to 0
	}
	//create a reptangle for the screen view
	RECT r1 = { ScrollX, 
		ScrollY,
		ScrollX + SCREEN_WIDTH,
		ScrollY + SCREEN_HEIGHT };

	//set destination rectangle
	RECT r2 = { 0,
		0,
		SCREEN_WIDTH, 
		SCREEN_HEIGHT };

	//draw the current game world view 
	d3ddev->StretchRect(gameworld, &r1, backbuffer, &r2, D3DTEXF_NONE);
}

//*********Display functions

void DisplayScore()
{
	// Create a colour for the tect - in this case grey 
	D3DCOLOR fontColor = D3DCOLOR_ARGB(190, 216, 216, 216);

	// Create a rectangle to indicate where should be drawn
	RECT rct;
	rct.left = 2;
	rct.right = 160;
	rct.top = 10;
	rct.bottom = rct.top + 30;

	TCHAR str[255];
	swprintf_s(str, _T("Score: %d"), score);
	//draw text
	m_font->DrawText(NULL, str, -1, &rct, 0, fontColor);
}

void DisplayHealth() //Display the remaining health
{
	// create text colour
	D3DCOLOR fontColor = D3DCOLOR_ARGB(190, 216, 216, 216);

	// create area to draw the text to
	RECT rct;
	rct.left = 160;
	rct.right = 320;
	rct.top = 10;
	rct.bottom = rct.top + 30; 

	TCHAR str[255];
	swprintf_s(str, _T("Health: %d"), sonic.getHealth()); //get the remaining health of the player

	//draw text to HUD
	m_font->DrawText(NULL, str, -1, &rct, 0, fontColor);
}

void DisplayLives() //display the lives on the screen
{
	// colour the text
	D3DCOLOR fontColor = D3DCOLOR_ARGB(190, 216, 216, 216);

	//create reptangle where should be drawn 
	RECT rct;
	rct.left = 320;
	rct.right = 480;
	rct.top = 10;
	rct.bottom = rct.top + 30;

	TCHAR str[255];
	swprintf_s(str, _T("Lives: %d"), sonic.getLives()); //get the remaining lives
	//draw the remaining lives to screen
	m_font->DrawText(NULL, str, -1, &rct, 0, fontColor);
}

void DisplayPause() //function to display that the game is paused
{
	D3DCOLOR fontColor = D3DCOLOR_ARGB(190, 216, 216, 216); //set the font colour

	RECT rct; //set the location where the text will be drawn
	rct.left = TILEWIDTH / 2 - 100;
	rct.right = TILEWIDTH / 2 + 100;
	rct.top = TILEHEIGHT / 2 - 20;
	rct.bottom = TILEHEIGHT / 2 + 20;

	//draw text ti screen
	m_font->DrawText(NULL, L"Game Paused", -1, &rct, 0, fontColor);
}

void DisplayGameOver() //function to draw text to the screen displaying 'game over'
{
	D3DCOLOR fontColor = D3DCOLOR_ARGB(190, 216, 216, 216); //set the font colour

	RECT rct; //set the location where the text will be drawn
	rct.left = TILEWIDTH / 2 - 100;
	rct.right = TILEWIDTH / 2 + 100;
	rct.top = TILEHEIGHT / 2 - 20;
	rct.bottom = TILEHEIGHT / 2 + 20;

	//draw text on screen
	m_font->DrawText(NULL, L"Game Over", -1, &rct, 0, fontColor);
}