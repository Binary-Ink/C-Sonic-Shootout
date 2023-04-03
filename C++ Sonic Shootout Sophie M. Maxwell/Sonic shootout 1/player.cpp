//Main code for PLAYER 


//things to be included 
#include <d3dx9math.h>
#include "dxaudio.h"
#include "dxgraphics.h"
#include "game.h"
#include "player.h"

//****************Constructors for PLAYER object
PLAYER::PLAYER() {

	playerTexture = LoadTexture("sonictexture.png", 0xFFFFFFFF); //load the player texture with original transparency
	deathTexture = LoadTexture("sonicdeath.png", 0xFFFFFFFF); //load the death texture with original transparency
	
	health = 100; //set player health to '100'
	lives = 3; //set the players lives to '3'

	playerState = SONIC_PLAY; //sets the4 state of the game to PLAY
	playerSprite.x = 0; //sets the location of the X value of the sprite to 0 (far left) 
	playerSprite.y = 220; //sets the location of the Y value of the texture to 220 (half way up the screen)
	flashFlag = 0;
	invFlag = 0; 
	
	playerSprite.width = 52; //sets the width of the sprite to 52
	playerSprite.height = 28; //sets the height of the sprite to 28
	playerSprite.movex = 0; 
	playerSprite.movey = 0;
	playerSprite.curframe = 1; //sets the first subimage to frame 1
	playerSprite.lastframe = 4; //sets the last subimage to frame 4
	
	deathSprite.width = 34; //sets the width of the death sprite to 34
	deathSprite.height = 43; //sets the height of the death sprite to 43
}

PLAYER::PLAYER(int x, int y) {

	playerTexture = LoadTexture("sonictexture.png", 0xFFFFFFFF); //Load player texture with original transparency
	deathTexture = LoadTexture("sonicdeath.png", 0xFFFFFFFF); //load player death texture with original transparency
	
	health = 100; //set the health to '100'
	lives = 3; //set the lives to '3'
	playerState = SONIC_PLAY; //set the game state to PLAY
	
	playerSprite.x = x; //set player sprite x location to 'x'
	playerSprite.y = y; //set player sprite y location to 'y'
	flashFlag = 0;
	invFlag = 0;
	
	playerSprite.width = 52; //set the player sprite width to 52
	playerSprite.height = 28; //set the player sprite height to 28
	playerSprite.movex = 0; 
	playerSprite.movey = 0;
	playerSprite.curframe = 1; //set the first subimage to frame 1
	playerSprite.lastframe = 4; //set the last subimage to frame 4
	
	deathSprite.width = 34; //set the width of the death sprite
	deathSprite.height = 43; //set the height of the death sprite
}

RECT  PLAYER::getSrcRect() {

	//make sonic fall off screen if player die
	if (playerState != SONIC_DIE) { //If the game state changes to DIE
		int columns = 4; //set the columns to 4
		srcRect.left = (playerSprite.curframe%columns)*playerSprite.width; //Get the left location
		srcRect.top = 0; //set the top location 
		srcRect.right = srcRect.left + playerSprite.width; //get the right location
		srcRect.bottom = srcRect.top + playerSprite.height; //get the bottom location 
		return srcRect;//return
	}
	else { //if the game state does not change 
		srcRect.left = 0; //set left to 0
		srcRect.top = 0; //set top to 0
		srcRect.right = srcRect.left + deathSprite.width; //get the right location
		srcRect.bottom = srcRect.top + deathSprite.height; //get the bottom location
		return srcRect; //return
	}
}

D3DXVECTOR2 PLAYER::getPosition() {
	D3DXVECTOR2 position(0, 0);
	if (playerState != SONIC_DIE) {
		position.x = (float)playerSprite.x;
		position.y = (float)playerSprite.y;
		return position;
	}
	else {
		position.x = (float)deathSprite.x;
		position.y = (float)deathSprite.y;
		return position;
	}
}
//get the players health
int PLAYER::getHealth() {
	return health;
}
//if health = 0, sonic dies
void PLAYER::setHealth(int nHealth) {
	health = nHealth;
	if (health <= 0) {
		health = 0;
		this->setPlayerState(SONIC_DIE);
	}
}
//get the number of lives the player has 
int PLAYER::getLives() {
	return lives;
}
//return the state of the player
int PLAYER::getPlayerState() {
	return playerState;
}
//if sonic is dead, death sprite
void PLAYER::setPlayerState(int nState) {
	playerState = nState;
	if (playerState == SONIC_DIE) {
		deathSprite.x = playerSprite.x;
		deathSprite.y = playerSprite.y;
	}
}

void PLAYER::setSprite(SPRITE sPlayerSprite) {
	playerSprite = sPlayerSprite;
}

SPRITE PLAYER::GetSprite() {
	return playerSprite;
}

//Update the player animation
void PLAYER::Update() {

	if (playerState != SONIC_DIE) {}
	
	
	if (playerSprite.curframe > playerSprite.lastframe) { //if the current animation frame is more than the last frame in the subimages
		playerSprite.curframe = 1; //set the frame back to 1
	}

	if (++playerSprite.animcount > playerSprite.animdelay) //Has the threshhold been reached for the animation delay
	{
		playerSprite.animcount = 0; //set the counter back to 0
 
		if (++playerSprite.curframe > playerSprite.lastframe) //if current frame is more than the last frame
			playerSprite.curframe = 0; //set frame to 0 to animate sprite
	}

//**************player movement

	//check left arrow pressed
	if (Key_Down(DIK_LEFT))
	{
		playerSprite.x -= 5; //move the player location by -5 on the x axis
	}

	//check right arrow pressed
	if (Key_Down(DIK_RIGHT))
	{
		playerSprite.x += 5; //move player location by +5 on he X axis
	}

	//check up arrow pressed
	if (Key_Down(DIK_UP))
	{
		playerSprite.y -= 5; //move player location by -5 on Y axis
	}

	//check down arrow pressed
	if (Key_Down(DIK_DOWN))
	{
		playerSprite.y += 5; //move player location by +5 on Y axis 
	}

	if (playerSprite.x > SCREEN_WIDTH - playerSprite.width) //If the player is at the side (width) of the screen
	{
		playerSprite.x = SCREEN_WIDTH - playerSprite.width; // Do not move the player
	}
	else if (playerSprite.x < 0) //if the player is not at the edge of the screen
	{
		playerSprite.x = 0; //move the player
	}

	if (playerSprite.y > SCREEN_HEIGHT - playerSprite.height) //if the player is at the side (height) of the screen
	{
		playerSprite.y = SCREEN_HEIGHT - playerSprite.height; // do not move the player
	}
	else if (playerSprite.y < 0) //if the player is not at the edge of the screen
	{
		playerSprite.y = 0; //move the player
	}

	//check invisible
	if (playerState == SONIC_INV) {
		invFlag++;
		if (invFlag > 100)
		{
			invFlag = 0;
			setPlayerState(SONIC_PLAY);
		}
	}
	else {
		//move the death sprite
		deathSprite.y += 10; 
		if (deathSprite.y > SCREEN_HEIGHT) {
			//deduce 1 life
			lives -= 1; 
			//if no lives remaining, end game
			if (lives == 0) {
				this->setPlayerState(SONIC_OVER);  
			}
			else { 
				//reset 
				this->setPlayerState(SONIC_INV);
				this->setSpriteX(0);
				this->setSpriteY(250);
				this->setHealth(100);
			}
		}
	} 
}

//*********Drawing sprites
void PLAYER::Draw(LPD3DXSPRITE sprite_handler) {
	D3DXVECTOR3 position(0, 0, 0);
	RECT srcRect = this->getSrcRect();
	
	if (playerState != SONIC_DIE&&playerState != SONIC_OVER) { //if sonic is not dead and the game is not over

	//set up vector position so sprite can be drawn
		position.x = (float)playerSprite.x; //set the X position to the x of playerSprite
		position.y = (float)playerSprite.y; //set the Y position to the Y of playerSprite
		
		if (playerState == SONIC_INV) {
			//draw sprite
			if (flashFlag < 750) {
				sprite_handler->Draw(
					playerTexture,
					&srcRect,
					NULL,
					&position,
					D3DCOLOR_XRGB(255, 255, 255)
					);
				flashFlag++;
			}
			else {
				if (flashFlag < 1500) {
					flashFlag++;
				}
				else {
					flashFlag = 0;
				}
			}
		}
		
		else {
			sprite_handler->Draw( //draws the playerTexture 
				playerTexture,
				&srcRect,
				NULL,
				&position,
				D3DCOLOR_XRGB(255, 255, 255)
				);
		}
	}
	else {
	//create vector so death sprite can be drawn
		position.x = (float)deathSprite.x; //set the X position to the x of deathSprite
		position.y = (float)deathSprite.y; //set the Y position to the Y of deathSprite
		sprite_handler->Draw( //draw the texture
			deathTexture, 
			&srcRect,
			NULL,
			&position,
			D3DCOLOR_XRGB(255, 255, 255)
			);
	}

}
//get x location
int PLAYER::getX() {
	return playerSprite.x;
}
//get y location
int PLAYER::getY() {
	return playerSprite.y;
}

//PLAYER deconstruct
PLAYER::~PLAYER() {

}
//Function to get new X coordinate
void PLAYER::setSpriteX(int newX) { 
	playerSprite.x = newX;
}
//Function to get new Y coordinate
void PLAYER::setSpriteY(int newY) {
	playerSprite.x = newY;
}