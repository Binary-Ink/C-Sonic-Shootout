//main code for enemy

//header files to be included 
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9math.h>
#include "dxaudio.h"
#include "dxgraphics.h"
#include "game.h"
#include "player.h"
#include "kamikaze.h"

KAMIKAZE::KAMIKAZE(int x, int y) {

	//load textures with origin transparacy
	kamikazeTexture = LoadTexture("kamikazetexture.png", 0); //load the kamikaze texture

	kamikazeSprite.x = x; //set the enemy x sprite location to X
	kamikazeSprite.y = y; //set the enemy Y location to Y

	kamikazeSprite.width = 55; //set the width of the sprite
	kamikazeSprite.height = 18; //set the height of the sprite 
	kamikazeSprite.movex = 0;
	kamikazeSprite.movey = 0;
	kamikazeSprite.curframe = 1; //set the first subimage to frame 1
	kamikazeSprite.lastframe = 2; //set the last subimage to frame 2
}

RECT  KAMIKAZE::getSrRect() { //get the location
	int columns = 2;  //set the number of columns to 2 (there are 2 frames)
	srcRect.left = (kamikazeSprite.curframe%columns)*kamikazeSprite.width; //get the left location
	srcRect.top = 0; //set the top location to 0
	srcRect.right = srcRect.left + kamikazeSprite.width; //get the right location
	srcRect.bottom - srcRect.top + kamikazeSprite.height; //get the bottom location 
	return srcRect; //return the location
}

D3DXVECTOR2 KAMIKAZE::getPosition() { //get the position of the enemy
	D3DXVECTOR2 position(0, 0); 
	position.x = (float)kamikazeSprite.x; //get the x location
	position.y = (float)kamikazeSprite.y; //get the y location
	return position; //return the position
}

void KAMIKAZE::setSprite(SPRITE sKamikazeSprite) {  //function for setting the sprite
	kamikazeSprite = sKamikazeSprite; //set the sprite
}

SPRITE KAMIKAZE::getSprite() { //get the sprite
	return kamikazeSprite; //return the sprite
}

void KAMIKAZE::Update(PLAYER* sonic) { //update the enemy 
	

	if (kamikazeSprite.curframe > kamikazeSprite.lastframe) //if the sprite has gone past the last frame
		kamikazeSprite.curframe = 1; //set the frame to 1

	//Has the animation delay reached the threshhold? 
	if (++kamikazeSprite.animcount > kamikazeSprite.animdelay)
	{
		//reset the counter
		kamikazeSprite.animcount = 0;

		//animate the sprite
		if (++kamikazeSprite.curframe > kamikazeSprite.lastframe) 
			kamikazeSprite.curframe = 0;
	}

	//get random speed between 1 and 5 
	randomspeed = 1 + rand() % 5;
	//get sonic position
	D3DXVECTOR2 TargetPos = sonic->getPosition();
	//get the current position
	D3DXVECTOR2 currentPos = this->getPosition();
	//calculate the direction 
	D3DXVECTOR2 seekBehaviour = (TargetPos - currentPos);
	//empty vector to store normalised vector 
	D3DXVECTOR2 DesiredVelocity(0, 0);
	//normalise seekBehaviour and stores in desired velocity
	D3DXVec2Normalize(&DesiredVelocity, &seekBehaviour);

	kamikazeSprite.x -= 5; //move kamikaze sprite -5 on the X axis to move
	kamikazeSprite.y += DesiredVelocity.y*randomspeed; //move the sprite by a random speed on the Y axis for the velocity
}

void KAMIKAZE::Draw(LPD3DXSPRITE sprite_handler) { //Draw the kamikazi sprite 

	//set up vector position so enemy can be drawn
	D3DXVECTOR3 position(0, 0, 0);
	position.x = (float)kamikazeSprite.x; //get X vector position
	position.y = (float)kamikazeSprite.y; //get Y vector position

	RECT srcRect; 
	int columns = 2; //set the columns to 2
	srcRect.left = (kamikazeSprite.curframe%columns)*kamikazeSprite.width; //get the left position
	srcRect.top = 0; //set the top to 0
	srcRect.right = srcRect.left + kamikazeSprite.width; //get the right position
	srcRect.bottom = srcRect.top + kamikazeSprite.height; //get the bottom position

	sprite_handler->Draw( //draw the actual texture
		kamikazeTexture,
		&srcRect,
		NULL,
		&position,
		D3DCOLOR_XRGB(255, 255, 255)
	);

}
//get X location
int KAMIKAZE::getX() { 
	return kamikazeSprite.x;
}
//get Y location
int KAMIKAZE::getY() {
	return kamikazeSprite.y;
}

KAMIKAZE::~KAMIKAZE() {

}
//function to get new x location
void KAMIKAZE::setSpriteX(int newX) {
	kamikazeSprite.x = newX;
}
//function to get new y location
void KAMIKAZE::setSpriteY(int newY) {
	kamikazeSprite.x = newY;
}