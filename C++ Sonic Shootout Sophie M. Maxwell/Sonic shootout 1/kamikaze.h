//Enemy header file

//compile file once and allow access multiple times
#ifndef _KAMIKAZE_H
#define _KAMIKAZE_H

//header files to be included
#include "dxaudio.h"
#include "dxgraphics.h"
#include "game.h"
#include "player.h" 
#include <d3dx9math.h> //for vector calculations for AI

//define the KAMIKAZE class (enemy)
class KAMIKAZE {
private:
	SPRITE kamikazeSprite; //define the sprite
	RECT srcRect; 
	int randomspeed; 
	LPDIRECT3DTEXTURE9 kamikazeTexture; //define the texture
public:
	KAMIKAZE(int,int);
	RECT getSrRect(); //get the location
	void setSprite(SPRITE sKamiKazeSprite); //set the sprite
	SPRITE getSprite(); //get the sprite
	void Update(PLAYER*);  
	void Draw(LPD3DXSPRITE); //draw the sprite
	D3DXVECTOR2 getPosition(); //get the position of the vector

	void setSpriteX(int); //function for setting the sprite X
	void setSpriteY(int); //function for setting the sprite Y
	int getX(); //variable for X value
	int getY(); //variable for Y value

	//deconstruct
	~KAMIKAZE();

};

//end of KAMIKAZE
#endif 
