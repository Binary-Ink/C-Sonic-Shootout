//Main code for EXPLOSION

//header files to include
#include <d3dx9math.h>
#include "dxaudio.h"
#include "dxgraphics.h"
#include "game.h"
#include "explosion.h"


EXPLOSION::EXPLOSION(int x, int y) {

//load textures with original transparency 
	explosionTexture = LoadTexture("explosiontexture.png", 0xFFFFFFFF); //load the explosion texture

	explosionSprite.x=x; //set the explosion sprite X location to X
	explosionSprite.y=y; //set the explosion sprite Y location to y

	explosionSprite.width = 25; //set the explosion sprite width
	explosionSprite.height = 22; //set the explosion sprite height
	explosionSprite.movex = 0;
	explosionSprite.movey = 0;
	explosionSprite.curframe = 1; //set the first subimage to frame 1
	explosionSprite.lastframe = 4; //set the last subimage to frame 4
	explosionSprite.animcount = 0; //set the animation count to 0
	explosionSprite.animdelay = 2; //set the delay to 2
}


RECT EXPLOSION::getSrRect() { //Get the location
	int columns = 4; //Set the columns to 4 (there are 4 frames)
	srcRect.left = (explosionSprite.curframe%columns)*explosionSprite.width; //Get the left location
	srcRect.top = 0; //set the top location to 0
	srcRect.right = srcRect.left + explosionSprite.width; //get the right location
	srcRect.bottom = srcRect.top + explosionSprite.height; //get the bottom location
	return srcRect; //return the location
}

D3DXVECTOR3 EXPLOSION::getPosition() { //get the position of explosion 
	D3DXVECTOR3 position(0, 0, 0);
	position.x = (float)explosionSprite.x; //get the X location
	position.y = (float)explosionSprite.y; // get the Y location
	return position; //return the position of the explosion 
}

void EXPLOSION::setSprite(SPRITE sExplosionSprite) { //function for setting the explosion sprite 
	explosionSprite=sExplosionSprite;
}

SPRITE EXPLOSION::getSprite() { //get and return the sprite
	return explosionSprite;
}

void EXPLOSION::Update() { // function for updating the explosion 

//if the animation count is over the threshhold 
	if (++explosionSprite.animcount > explosionSprite.animdelay)
	{
		//reset the counter
		explosionSprite.animcount = 0;

		//animate the sprite
		explosionSprite.curframe++;
	}

}

void EXPLOSION::Draw(LPD3DXSPRITE sprite_handler) {//function draw the sprite

	//Set up vector for explosion 
	D3DXVECTOR3 position(0, 0, 0);
	position.x = (float)explosionSprite.x+2; //+2 for placement
	position.y = (float)explosionSprite.y+2;

	RECT srcRect;
	int columns = 4; //Set the columns to 4 
	srcRect.left = (explosionSprite.curframe%columns)*explosionSprite.width; //get the left location
	srcRect.top = 0; //set the top to 0
	srcRect.right = srcRect.left + explosionSprite.width; //get the right location
	srcRect.bottom = srcRect.top + explosionSprite.height; //get the bottom location

	sprite_handler->Draw( //sprite handeler to draw the sprites 
		explosionTexture,
		&srcRect,
		NULL,
		&position,
		D3DCOLOR_XRGB(255, 255, 255)
	);

}
//Get the X location
int EXPLOSION::getX() {
	return explosionSprite.x; //return the X location as explosionSprite.x
}

//get the Y location
int EXPLOSION::getY() {
	return explosionSprite.y;//return the Y location as explosionSprite.y
}

EXPLOSION::~EXPLOSION() {

}

//function to get new X location
void EXPLOSION::setSpriteX(int newX) {
	explosionSprite.x = newX;
}

//function to get new Y location
void EXPLOSION::setSpriteY(int newY) {
	explosionSprite.x = newY;
}

//get the current frame of the explosion animation 
int EXPLOSION::getFrame() {
	return explosionSprite.curframe;
}