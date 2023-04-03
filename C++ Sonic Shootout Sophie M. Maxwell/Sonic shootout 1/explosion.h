//Header file for EXPLOSION

//Compile the file once and allow it to be accessed multiple times
#ifndef _EXPLOSION_H
#define _EXPLOSION_H

//header files to include
#include "dxaudio.h"
#include "dxgraphics.h"
#include "game.h"

//EXPLOSION class
class EXPLOSION {
private:
	SPRITE explosionSprite;
	RECT srcRect;
	LPDIRECT3DTEXTURE9 explosionTexture;
public:
	EXPLOSION();
	EXPLOSION(int, int);

	RECT getSrRect();//Get the coordinate of the reptangle the player is in 
	int getHealth();//Runs the function to get the players health	
	void setHealth(int);//Runs the function to adjust the players health	
	void setSprite(SPRITE sExplosionSprite);//Changes the player sprite	
	SPRITE getSprite();//Gets the sprite	
	void Update();//Runs the function to update the game	
	void Draw(LPD3DXSPRITE);//Draws the new information to the game screen

	D3DXVECTOR3 getPosition();//get the position

	//set the sprite X and Y location
	void setSpriteX(int);
	void setSpriteY(int);
	//get the sprite X and Y location
	int getX();
	int getY();
	//get the frame 
	int getFrame();
	//end of explosion
	~EXPLOSION();

};
//end of compiled code
#endif 
