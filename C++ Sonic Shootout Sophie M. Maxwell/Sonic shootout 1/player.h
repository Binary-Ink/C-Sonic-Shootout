//Header file for PLAYER

//Compile file once and allow access multiple times
#ifndef _PLAYER_H
#define _PLAYER_H

//Header files to be included
#include "dxaudio.h"
#include "dxgraphics.h"
#include "game.h"

//PLAYER class
class PLAYER {
private:

	//Set variables 
	int health; //var for players health
	int lives; //var for player lives
	int playerState; //var for the state of the player (alive, dead)
	int flashFlag; 
	int invFlag; 
	SPRITE playerSprite; //define 'playerSprite' 
	RECT srcRect; //define 'srcRect' 
	SPRITE deathSprite; //define 'deathSprite' 
	LPDIRECT3DTEXTURE9 playerTexture; //define playerTexture
	LPDIRECT3DTEXTURE9 deathTexture; //define deathTexture
public:
	PLAYER(); //constructor

	PLAYER(int, int); //gets X and Y position for players actual position to create instance

	RECT getSrcRect(); //Used to get the position for collision and rendering 
	int getHealth(); //gets the health value for the player
	void setHealth(int); //sets the health for the player
	int getPlayerState();
	void setPlayerState(int);
	int getLives(); //get the lives of the player
	void setLives(int); //set the lives of the player
	void setSprite(SPRITE sPlayerSprite); //set the sprite to the correct player sprit
	SPRITE GetSprite(); //gets the sprite
	void Update(); //function to update the game
	void Draw(LPD3DXSPRITE); //render the player to the screen
	D3DXVECTOR2 getPosition(); //get the position of the player as a vector

	void setSpriteX(int); //Set the sprites X location
	void setSpriteY(int); //set the sprites Y location
	int getX(); //get the X location
	int getY(); //get the Y location 

	
	~PLAYER(); //deconstructor 

};
//end of PLAYER header
#endif 
