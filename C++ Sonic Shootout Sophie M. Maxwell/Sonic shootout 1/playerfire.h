//HEADER FILE FOR PLAYER FIRE

//Only compile the code once and allow it to be accessed multiple times 
#ifndef _PLAYERFIRE_H
#define _PLAYERFIRE_H

//Header files to be included 
#include "dxaudio.h"
#include "dxgraphics.h"
#include "game.h"

//Class for the player fire
class PLAYERFIRE {
private:
	SPRITE playerFireSprite;
	RECT srcRect;
	LPDIRECT3DTEXTURE9 playerFireTexture;
public:
	PLAYERFIRE();
	PLAYERFIRE(int, int);	
	RECT getSrRect();//Get the coordinate of the reptangle the player is in 
	int getHealth();//Runs the function to get the players health	
	void setHealth(int);//Runs the function to adjust the players health	
	void setSprite(SPRITE sPlayerFireSprite);//Changes the player sprite	
	SPRITE getSprite();//Gets the sprite	
	void Update();//Runs the function to update the game	
	void Draw(LPD3DXSPRITE);//Draws the new information to the game screen


	//get the position
	D3DXVECTOR3 getPosition();

	//Set the sprite X and Y location
	void setSpriteX(int);
	void setSpriteY(int);
	//Get the X and Y location 
	int getX();
	int getY();
	
	//End of player fire 
	~PLAYERFIRE();

};

//end of the compiled code 
#endif 

