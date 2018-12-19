/*--------------------------------------------//
Game Defintion
This is the scene controller for the physics world
and the ui interface.
//--------------------------------------------*/
#ifndef GAME
#define GAME
	/*--------------------------------------------//
	Includes
	//--------------------------------------------*/
		#include "game.h"
	
	/*--------------------------------------------//
	Constructors
	//--------------------------------------------*/
		game::game(){
			//set default view angles
			viewerAzimuth = INITIAL_VIEWER_AZIMUTH;
			viewerAltitude = INITIAL_VIEWER_ALTITUDE;
			//initialize world and interface
			worldspace = new world(TIMESTEP, TIMEOUT, VERTEXRAD, GRAVOBJMASS, GRAVITYCONSTANT, FRICTIONDIST, FRICTIONCONSTANT, DEFORMCONSTANT);
			display = new interface();

			//setup frame delay display
			FrameDelay* frame;
			frame = new FrameDelay();
			display->add(frame);

			//add model to world
			mesh* obj = new mesh();
			vertex* a = new vertex(vec3(0,1,0), new vec3(1,0,0), new vec4(1,1,1,1), new vec2(0.0, 0.0), new double(0));
			vertex* b = new vertex(vec3(0,0,1), new vec3(1,0,0), new vec4(1,1,1,1), new vec2(1.0, 0.0), new double(0.5));
			vertex* c = new vertex(vec3(0,-1,0), new vec3(1,0,0), new vec4(1,1,1,1), new vec2(0.0, 1.0), new double(1));
			vertex* d = new vertex(vec3(0,0,-1), new vec3(1,0,0), new vec4(1,1,1,1), new vec2(1.0, 1.0), new double(0.5));
			obj->addTri(a,b,c);
			obj->addTri(a,c,d);
			obj->setGlobalMat("textures/test.png", 1, 0);
			obj->setGlobalMat("textures/test2.png", 1, 1);
			worldspace->addMesh(obj);
		}

	/*--------------------------------------------//
	Destructor
	//--------------------------------------------*/
		game::~game(){
			delete worldspace;
			delete display;
		}
		
	/*--------------------------------------------//
	Predraw
	Passes the function call down the stack for 
	drawing elements before the lighting has been 
	added.
	//--------------------------------------------*/
		void game::predraw(){
			//Draw frame delay text
			glPushMatrix();
				glDisable( GL_DEPTH_TEST );
				display->draw();
				glEnable( GL_DEPTH_TEST );
			glPopMatrix();
		}

	/*--------------------------------------------//
	Postdraw
	Performs player camera related operations and 
	passes down the stack for drawing objects after
	lighting has been done.
	//--------------------------------------------*/
		void game::postdraw(float aspect){
			//Position and orient camera.
			float position[] = { 0.0f, 0.0f, 0.0f };
			float camera[] = { position[0] + VIEWER_DISTANCE * sin(viewerAltitude) * sin(viewerAzimuth), 
				position[1] + VIEWER_DISTANCE * cos(viewerAltitude), 
				position[2] + VIEWER_DISTANCE * sin(viewerAltitude) * cos(viewerAzimuth) 
			};
			gluLookAt(camera[0], camera[1], camera[2], position[0], position[1], position[2], 0.0, 1.0, 0.0);

			//Draw the world
			worldspace->draw(position, camera, aspect);
		}

	/*--------------------------------------------//
	Update
	Perform any changes or updates to the game 
	caused by a passage of time.
	//--------------------------------------------*/
		int game::update(int value){
			//Update value and check if we are on a update cycle for the physics simulation
			if (value % worldspace->getTimeStep() == 0){
				//worldspace->update();
				value = 0;
			}
			return value++;
		}

	/*--------------------------------------------//
	Keyboard Press Event Handler
	Handles any keyboard key inputs from user
	//--------------------------------------------*/
		void game::keypressASCII(unsigned char pressedKey, int mouseXPosition, int mouseYPosition){
			switch(pressedKey){
				//switch based upon keys pressed to perform actions desired
				//none yet - placeholder
			}
		}

	/*--------------------------------------------//
	Non ASCII Keyboard Press Event Handler
	Handles any Non ASCII keyboard key inputs from user
	This will catch certain events unable to be caught
	normally
	//--------------------------------------------*/
		void game::keypressNonASCII(int pressedKey, int mouseXPosition, int mouseYPosition){
			switch (pressedKey){
				case GLUT_KEY_RIGHT:{
					viewerAzimuth += VIEWER_ANGLE_INCREMENT;
					if (viewerAzimuth > 2 * PI)
						viewerAzimuth -= 2 * PI;
					break;
				}
				case GLUT_KEY_LEFT:{
					viewerAzimuth -= VIEWER_ANGLE_INCREMENT;
					if (viewerAzimuth < 0.0)
						viewerAzimuth += 2 * PI;
					break;
				}
				case GLUT_KEY_UP:{
					viewerAltitude += VIEWER_ANGLE_INCREMENT;
					if (viewerAltitude > PI - VIEWER_ANGLE_INCREMENT)
						viewerAltitude = PI - VIEWER_ANGLE_INCREMENT;
					break;
				}
				case GLUT_KEY_DOWN:{
					viewerAltitude -= VIEWER_ANGLE_INCREMENT;
					if (viewerAltitude < VIEWER_ANGLE_INCREMENT)
						viewerAltitude = VIEWER_ANGLE_INCREMENT;
					break;
				}
			}
		}
#endif