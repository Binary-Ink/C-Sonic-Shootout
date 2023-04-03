#include <d3dx9math.h>
#include "dxaudio.h"
#include "dxgraphics.h"
#include "game.h"
#include "playerfire.h"

PLAYERFIRE::PLAYERFIRE(int x, int y) {

//load textures with original transparency 
	playerFireTexture = LoadTexture("sonicbullet.png", 0xFFFFFFFF);//load the bullet texture

	playerFireSprite.x = x;//set playerFireSprite.x to x
	playerFireSprite.y = y;//set playerFireSprite.y to y

	playerFireSprite.width = 10; //set the sprite width
	playerFireSprite.height = 10; //set the sprite height
	playerFireSprite.movex = 0; 
	playerFireSprite.movey = 0;
	playerFireSprite.animcount = 0; //set the animation count to 0
	playerFireSprite.animdelay = 3; //set the animation delay to 3
	playerFireSprite.curframe = 1; //set the first frame to 1
	playerFireSprite.lastframe = 2; //set the last frame to 2
}

RECT  PLAYERFIRE::getSrRect() { 
	int columns = 2;
	srcRect.left = (playerFireSprite.curframe%columns)*playerFireSprite.width;
	srcRect.top = 0;
	srcRect.right = srcRect.left + playerFireSprite.width;
	srcRect.bottom - srcRect.top + playerFireSprite.height;
	return srcRect;
}
//get the location of the player
D3DXVECTOR3 PLAYERFIRE::getPosition() {
	D3DXVECTOR3 position(0, 0, 0);
	position.x = (float)playerFireSprite.x;
	position.y = (float)playerFireSprite.y;
	return position;
}
//set the sprite
void PLAYERFIRE::setSprite(SPRITE sPlayerFireSprite) {
	playerFireSprite = sPlayerFireSprite;
}
//get the sprite
SPRITE PLAYERFIRE::getSprite() {
	return playerFireSprite;
}
//update animation
void PLAYERFIRE::Update() {
	

	if (playerFireSprite.curframe > playerFireSprite.lastframe) //if the current frame is more than the last frame
		playerFireSprite.curframe = 1; //set the current frame to 1

	//animation over threshhold? 
	if (++playerFireSprite.animcount > playerFireSprite.animdelay) //if the animation count is over the delay threshhold
	{
		//reset the counter
		playerFireSprite.animcount = 0;

		//animate the sprite 
		if (++playerFireSprite.curframe > playerFireSprite.lastframe)
			playerFireSprite.curframe = 0;
	}

	//move the player
	playerFireSprite.x += 10;
}
//draw player fire
void PLAYERFIRE::Draw(LPD3DXSPRITE sprite_handler) {
	//create a vector to store the object
	D3DXVECTOR3 position(0, 0, 0); 
	position.x = (float)playerFireSprite.x; //set x to playerFireSprite.x
	position.y = (float)playerFireSprite.y; //set y to playerFireSprite.y
	//create a reptangle 
	RECT srcRect;
	int columns = 2; //set the columns to 2
	srcRect.left = (playerFireSprite.curframe%columns)*playerFireSprite.width;//set left
	srcRect.top = 0; //set top
	srcRect.right = srcRect.left + playerFireSprite.width; //set right
	srcRect.bottom = srcRect.top + playerFireSprite.height; //set left
	//draw playerfire
	sprite_handler->Draw(
		playerFireTexture,//draw the texture
		&srcRect,//draw in the reptangle
		NULL,
		&position,
		D3DCOLOR_XRGB(255, 255, 255)//set the colour
	);

}
//get X 
int PLAYERFIRE::getX() {
	return playerFireSprite.x;
}
//get Y
int PLAYERFIRE::getY() {
	return playerFireSprite.y;
}

PLAYERFIRE::~PLAYERFIRE() {

}
//get new X
void PLAYERFIRE::setSpriteX(int newX) {
	playerFireSprite.x = newX;
}
//get new Y
void PLAYERFIRE::setSpriteY(int newY) {
	playerFireSprite.x = newY;
}