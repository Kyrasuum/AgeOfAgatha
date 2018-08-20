/*--------------------------------------------//
Game Header
This is the scene controller for the physics world
and the ui interface.
//--------------------------------------------*/
#ifndef GAME_H
#define GAME_H

/*--------------------------------------------//
Includes
//--------------------------------------------*/
	#include "../globals.h"
	#include "../objects/world.h"
	#include "../interface/interface.h"
	#include "../interface/text/frameDelay.h"

class game{
	private:
	protected:
	public:
		/*--------------------------------------------//
		Class Variables
		//--------------------------------------------*/
			world* worldspace;//Stores the physical world, handles all interactions between discrete objects
			interface* display;//Interacts directly with the player through 2D elements
			float viewerAzimuth;//Player view azimuth
			float viewerAltitude;//Player view altitude

		/*--------------------------------------------//
		Constructors
		//--------------------------------------------*/
			game();

		/*--------------------------------------------//
		Destructor
		//--------------------------------------------*/
			~game();

		/*--------------------------------------------//
		Functions
		//--------------------------------------------*/
			void predraw();
			void postdraw();
			
			int update(int value);
			void keypressASCII(unsigned char pressedKey, int mouseXPosition, int mouseYPosition);
			void keypressNonASCII(int pressedKey, int mouseXPosition, int mouseYPosition);
};
#endif