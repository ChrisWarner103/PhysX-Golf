#include "VisualDebugger.h"
#include <vector>
#include "Extras\Camera.h"
#include "Extras\Renderer.h"
#include "Extras\HUD.h"

namespace VisualDebugger
{
	using namespace physx;

	enum RenderMode
	{
		DEBUG,
		NORMAL,
		BOTH
	};

	enum HUDState
	{
		EMPTY = 0,
		HELP = 1,
		PAUSE = 2,
		WIN = 3
	};

	//function declarations
	void KeyHold();
	void KeySpecial(int key, int x, int y);
	void KeyRelease(unsigned char key, int x, int y);
	void KeyPress(unsigned char key, int x, int y);
	void ForceInput(int key);


	void MouseHold();
	void MouseDown(int button);
	void MouseRelease(int button);
	void motionCallback(int x, int y);
	void mouseCallback(int button, int state, int x, int y);
	void exitCallback(void);

	void continuousMotionCallback(int x, int y);

	void RenderScene();
	void ToggleRenderMode();
	void HUDInit();

	void UpdateCamera(PxRigidBody* currentActor);
	void CameraInput(int key);

	///simulation objects
	Camera* camera;
	PhysicsEngine::MyScene* scene;
	PxReal delta_time = 1.f / 60.f;
	PxReal gForceStrength = 1000;
	RenderMode render_mode = NORMAL;
	const int MAX_KEYS = 256;
	const int MAX_MOUSE_KEYS = 10;
	bool key_state[MAX_KEYS];
	bool mouse_state[MAX_MOUSE_KEYS];
	bool hud_show = true;
	HUD hud;

	///Mouse Handling
	int mMouseX = 0;
	int mMouseY = 0;
	int dX = 0;
	int dY = 0;

	float respawnTimer = 3.f;

	//Mouse Clamped Values between -1 .. 1
	float clampedMX;
	float clampedMY;

	//Store the screen width and height
	int screenWidth = 0;
	int screenHeight = 0;

	//Bool States
	//If the ball is moving.
	bool ballMoving = false;
	//If the player is zooming the camera into the ball
	bool zooming = false;
	//If the player has completed the course.
	bool holeComplete = false;

	//Amount of shots that have been taken for this course.
	int shotsTaken = 0;

	//The value of how powerful the shot is that is being taken.
	float shotPower = 0.f;

	//Position before the ball has been shot. Default to the spawn point of the ball on the current course.
	PxVec3 lastPosition = PxVec3(0, 0.5f, 0);

	//Final score for the course.
	string courseScore;


	//----------------------------------
	//Initilisation
	void Init(const char* window_name, int width, int height)
	{
		///Init PhysX
		PhysicsEngine::PxInit();
		scene = new PhysicsEngine::MyScene();
		scene->Init();



		screenWidth = width;
		screenHeight = height;

		///Init renderer
		Renderer::BackgroundColor(PxVec3(150.f / 255.f, 150.f / 255.f, 150.f / 255.f));
		Renderer::SetRenderDetail(40);
		Renderer::InitWindow(window_name, width, height);
		Renderer::Init();

		camera = new Camera(PxVec3(0.0f, 5.0f, 15.0f), PxVec3(0.f, -.5f, -1.f), 5.f);

		//initialise HUD
		HUDInit();

		///Assign callbacks
		//render
		glutDisplayFunc(RenderScene);

		//keyboard
		glutKeyboardFunc(KeyPress);
		glutSpecialFunc(KeySpecial);
		glutKeyboardUpFunc(KeyRelease);

		//mouse
		glutMouseFunc(mouseCallback);
		glutMotionFunc(motionCallback);
		glutPassiveMotionFunc(continuousMotionCallback);


		//exit
		atexit(exitCallback);

		//init motion callback
		motionCallback(0, 0);
	}

	void HUDInit()
	{

		string amountOfShotsSTR = to_string(shotsTaken);
		string powerOfShot = to_string(shotPower);

		//Clear the hud before rendering.
		hud.Clear();

		//initialise HUD
		//add an empty screen
		hud.AddLine(EMPTY, "");
		//add a help screen
		hud.AddLine(HELP, "");
		hud.AddLine(HELP, " Gameplay Controls");
		hud.AddLine(HELP, "    LMouse + Click - Shoot the ball in the direction of the mouse");
		hud.AddLine(HELP, "");
		hud.AddLine(HELP, " Display");
		hud.AddLine(HELP, "    F5 - help on/off");
		hud.AddLine(HELP, "    F6 - shadows on/off");
		hud.AddLine(HELP, "    F7 - render mode");
		hud.AddLine(HELP, "");
		hud.AddLine(HELP, " Camera");
		hud.AddLine(HELP, "    mouse + click - change orientation");
		hud.AddLine(HELP, "    F8 - reset view");
		hud.AddLine(HELP, "");
		hud.AddLine(HELP, " Special");
		hud.AddLine(HELP, "    F1 - Change the current ball that is being played with");
		hud.AddLine(HELP, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n Power of Shot: " + powerOfShot);
		hud.AddLine(HELP, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n Par 5");
		hud.AddLine(HELP, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n Amount of Shots: " + amountOfShotsSTR);
		//add a pause screen
		hud.AddLine(PAUSE, "");
		hud.AddLine(PAUSE, "");
		hud.AddLine(PAUSE, "");
		hud.AddLine(PAUSE, "   Simulation paused. Press F10 to continue.");

		//Winning Screen
		hud.AddLine(WIN, "\n\n\n\n\n    " + courseScore);

		switch (hud.ActiveScreen())
		{
		case EMPTY:
			//set font size for all screens
			hud.FontSize(0.02f);
			//set font color for all screens
			hud.Color(PxVec3(0.f, 0.f, 0.f));
			break;
		case HELP:
			//set font size for all screens
			hud.FontSize(0.02f);
			//set font color for all screens
			hud.Color(PxVec3(0.f, 0.f, 0.f));
			break;
		case PAUSE:
			//set font size for all screens
			hud.FontSize(0.02f);
			//set font color for all screens
			hud.Color(PxVec3(0.f, 0.f, 0.f));
			break;
		case WIN:
			//set font size for all screens
			hud.FontSize(0.1f);
			//set font color for all screens
			hud.Color(PxVec3(0.f, 0.f, 0.f));
			break;
		}
	}

	//----------------------------------

	//Start the main loop
	void Start()
	{
		glutMainLoop();
	}

	void Update()
	{
		//Check if the ball is not moving to update how much the power of the shot is.
		//This is so that it keeps the power of the previous shot on screen before you can shoot again.
		if (!ballMoving)
		{
			//Calculate shot power
			shotPower = abs(clampedMX) + abs(clampedMY) * 100;
		}

		if (holeComplete)
		{
			switch (shotsTaken)
			{
			case 1:
				courseScore = "HOLE IN ONE!";
				break;
			case 2:
				courseScore = "ALBATROSS";
				break;
			case 3:
				courseScore = "EAGLE";
				break;
			case 4:
				courseScore = "BIRDY";
				break;
			case 5:
				courseScore = "PAR";
				break;
			case 6:
				courseScore = "BOGEY";
				break;
			case 7:
				courseScore = "DOUBLE BOGEY";
				break;
			case 8:
				courseScore = "TRIPPLE BOGEY";
				break;
			case 9:
				courseScore = "BOGEY +1";
				break;
			case 10:
				courseScore = "BOGEY +2";
				break;
			case 11:
				courseScore = "BOGEY +3";
				break;
			case 12:
				courseScore = "BOGEY +4";
				break;
			}
		}

		HUDInit();

		if (scene->my_callback->trigger || scene->my_callback->collision)
		{
			//Enter
			if (scene->my_callback->otherObj != NULL && !scene->triggered)
			{
				string triggerObj = scene->my_callback->triggerObj->getName();

				if (triggerObj == "Hole1")
				{
					scene->triggered = true;
					scene->GetSelectedActor()->addForce(PxVec3(0, 0, -250) * 10, PxForceMode::eACCELERATION);
					printf("Landed in Hole1");
				}
				else if (triggerObj == "Hole2")
				{
					scene->triggered = true;
					scene->GetSelectedActor()->addForce(PxVec3(0, 5, 0) * 2, PxForceMode::eIMPULSE);
					printf("Landed in Hole2");
				}
				else if (triggerObj == "Hole3")
				{
					scene->triggered = true;
					scene->GetSelectedActor()->addForce(PxVec3(0, 0, 200) * 10, PxForceMode::eACCELERATION);
					printf("Landed in Hole3");
				}
				else if (triggerObj == "PipeExit")
				{
					scene->triggered = true;
					scene->GetSelectedActor()->addForce(PxVec3(100, 0, 0) * 10, PxForceMode::eACCELERATION);
					printf("Triggered PipeExit");
				}
				else if (triggerObj == "HoleFinal")
				{
					scene->triggered = true;
					holeComplete = true;
					printf("Finished Hole!");
				}
			}

			if (scene->my_callback->otherObj != NULL && !scene->contact)
			{
				string otherObj = scene->my_callback->otherObj->getName();
				
				if (scene->my_callback->triggerObj == scene->GetSelectedActor())
				{

					if (otherObj == "Floor")
					{
						scene->contact = true;

						if (respawnTimer > 0)
							respawnTimer -= delta_time;
						else
						{
							//Increase the amount of shots taken because it goes out of bounds!
							shotsTaken++;

							//Put the actor to sleep to stop all forms of velocity.
							scene->GetSelectedActor()->putToSleep();

							//Move the actor to the last known position on the course.
							scene->GetSelectedActor()->setGlobalPose(PxTransform(lastPosition));

							respawnTimer = 2.f;
						}
					}
				}
			}else
				scene->contact = false;
		}
		else
			scene->triggered = false;
	}

	//Render the scene and perform a single simulation step
	void RenderScene()
	{
		//Update Loops
		//FixedUpdate(); // Physics

		//Handle Input States
		KeyHold();
		MouseHold();

		//Core Mechanics
		//perform a single simulation step
		scene->Update(delta_time);
		Update();

		//Ran after all the other updates.
		//LateUpdate();

		//Handle Locking the Camera to the ball
		if (!zooming)
			UpdateCamera(scene->GetSelectedActor());

		//Rendering
		Renderer::Start(camera->getEye(), camera->getDir());

		if ((render_mode == DEBUG) || (render_mode == BOTH))
		{
			Renderer::Render(scene->Get()->getRenderBuffer());
		}

		if ((render_mode == NORMAL) || (render_mode == BOTH))
		{
			std::vector<PxActor*> actors = scene->GetAllActors();
			if (actors.size())
				Renderer::Render(&actors[0], (PxU32)actors.size());
		}

		//adjust the HUD state
		if (hud_show)
		{
			if (scene->Pause())
				hud.ActiveScreen(PAUSE);
			else if (holeComplete)
				hud.ActiveScreen(WIN);
			else
				hud.ActiveScreen(HELP);
		}
		else
			hud.ActiveScreen(EMPTY);

		//render HUD
		hud.Render();

		//finish rendering
		Renderer::Finish();

		//Stop the balls velocity when it is slowing down so it doesn't take too long to shoot again.
		if (scene->GetSelectedActor()->getLinearVelocity().normalize() <= 0.1f && ballMoving)
		{
			if (scene->my_callback->otherObj == NULL)
			{
				lastPosition = scene->GetSelectedActor()->getGlobalPose().p;

				scene->GetSelectedActor()->setLinearVelocity(PxVec3(0, 0, 0));
				scene->GetSelectedActor()->setAngularVelocity(PxVec3(0, 0, 0));
				ballMoving = false;
			}
		}
	}

	//----------------------------------
	//user defined keyboard & mouse handlers
	//----------------------------------

	void UserKeyPress(int key)
	{
		switch (toupper(key))
		{
			//implement your own
		case 'R':
			break;
		default:
			break;
		}
	}

	void UserKeyRelease(int key)
	{
		switch (toupper(key))
		{
			//implement your own
		case 'R':
			break;
		default:
			break;
		}
	}

	void UserKeyHold(int key)
	{
		switch (toupper(key))
		{
			//implement your own
		case 'R':
			break;
		default:
			break;
		}
	}

	void UserMouseDown(int button)
	{
		switch (button)
		{
		case 0: //Left
			break;
		case 1: //Middle
			break;
		case 2: //Right
			break;
		default:
			break;
		}
	}

	void UserMouseRelease(int button)
	{
		switch (button)
		{
		case 0: //Left
			if (!ballMoving) // Checking if the ball is currently rolling otherwise the player can't shoot!
			{
				//Need to sort out rotation of the camera and lock the ball in the centre of the screen otherwise
				//shooting will be off centred as it uses the centre of the screen to shoot the ball with power.

				scene->GetSelectedActor()->addForce(PxVec3(clampedMX, 0, clampedMY) * 8, PxForceMode::eIMPULSE);

				//Increase the amount of shots that have been taken on this hole.
				shotsTaken++;

				//Toggle the ball to be moving so that the player can no longer shoot.
				ballMoving = true;
			}
			break;
		case 1: //Middle
			break;
		case 2: //Right
			zooming = false; // Enabled the camera to follow the ball again rather then zooming.
			break;
		default:
			break;
		}
	}

	void UserMouseHold(int button)
	{
		switch (button)
		{
		case 0: //Left
			break;
		case 1: //Middle
			break;
		case 2: //Right

			if (!ballMoving) // If the ball isn't moving allow the player to zoom in and out of the ball.
			{
				zooming = true; //Toggle zooming state to true so the camera follow doesn't mess with the zooming.

				camera->Zoom(dY, scene->GetSelectedActor()->getGlobalPose().p, delta_time);
			}

			break;
		}
	}

	//---------------------------------------
	//			Camera controlls
	//---------------------------------------
	//Locking the camera to an actor
	void UpdateCamera(PxRigidBody* currentActor)
	{
		camera->Move(delta_time, currentActor->getGlobalPose().p, PxVec3(0, 5, 10));
	}

	//---------------------------------------

	///handle special keys
	void KeySpecial(int key, int x, int y)
	{
		//simulation control
		switch (key)
		{
			//Switch through the balls that you can play with.
		case GLUT_KEY_F1:
			scene->GetSelectedActor()->setGlobalPose(PxTransform(PxVec3(0, 1, 100)));
			scene->SelectNextActor();
			scene->GetSelectedActor()->setGlobalPose(PxTransform(lastPosition));
			break;
			//display control
		case GLUT_KEY_F5:
			//hud on/off
			hud_show = !hud_show;
			break;
		case GLUT_KEY_F6:
			//shadows on/off
			Renderer::ShowShadows(!Renderer::ShowShadows());
			break;
		case GLUT_KEY_F7:
			//toggle render mode
			ToggleRenderMode();
			break;
		case GLUT_KEY_F8:
			//reset camera view
			camera->Reset();
			break;

			//simulation control
		case GLUT_KEY_F9:
			//select next actor
			scene->SelectNextActor();
			break;
		case GLUT_KEY_F10:
			//toggle scene pause
			scene->Pause(!scene->Pause());
			break;
		case GLUT_KEY_F12:
			//resect scene
			scene->Reset();
			break;
		default:
			break;
		}
	}

	//handle single key presses
	void KeyPress(unsigned char key, int x, int y)
	{
		//do it only once
		if (key_state[key] == true)
			return;

		key_state[key] = true;

		//exit
		if (key == 27)
			exit(0);

		UserKeyPress(key);
	}

	//handle key release
	void KeyRelease(unsigned char key, int x, int y)
	{
		key_state[key] = false;
		UserKeyRelease(key);
	}

	//handle holded keys
	void KeyHold()
	{
		for (int i = 0; i < MAX_KEYS; i++)
		{
			if (key_state[i]) // if key down
			{
				CameraInput(i);
				ForceInput(i);
				UserKeyHold(i);
			}
		}
	}

	//Handle Mouse Down Keys
	void MouseDown(int button)
	{
		//do it only once
		if (mouse_state[button] == true)
			return;

		mouse_state[button] = true;
		UserMouseDown(button);
	}

	//Handle Mouse Release Keys
	void MouseRelease(int button)
	{
		mouse_state[button] = false;
		UserMouseRelease(button);
	}

	//Handle Mouse Hold Keys
	void MouseHold()
	{
		for (int i = 0; i < MAX_MOUSE_KEYS; i++)
		{
			if (mouse_state[i]) // if mouse down
			{
				UserMouseHold(i);
			}
		}
	}

	//-----------------------------
	//			Callbacks
	//-----------------------------

	//Handle Mouse Movement when mouse button is pushed. Also handles mouse position
	void motionCallback(int x, int y)
	{
		int dx = mMouseX - x;
		int dy = mMouseY - y;

		mMouseX = x;
		mMouseY = y;

		dX = dx;
		dY = dy;
	}

	//Handle Mouse Movement continuously. Also handles mouse position
	void continuousMotionCallback(int x, int y)
	{
		mMouseX = x;
		mMouseY = y;

		//Clamps the mouse position values between the screen width and height
		clampedMX = PxClamp(mMouseX, 0, screenWidth);
		clampedMY = PxClamp(mMouseY, 0, screenHeight);

		//Uses the screen width and height to divide it into a 0..1 value, then remaps it to -1..1
		clampedMX = PxClamp((clampedMX / screenWidth) * 2 - 1, -1.0f, 1.0f);
		clampedMY = PxClamp((clampedMY / screenHeight) * 2 - 1, -1.0f, 1.0f);
	}

	//Handle mouse buttons and states, same with positioning
	void mouseCallback(int button, int state, int x, int y)
	{
		//Gets the position of the mouse in screen coords at the point of clicking a button.
		mMouseX = x;
		mMouseY = y;

		//Handle Mouse States
		switch (state)
		{
		case 0: //Mouse State Down
			MouseDown(button);
			break;
		case 1: //Mouse State Up
			MouseRelease(button);
			break;
		}
	}

	//---------------------------------

	//Handles toggling render modes
	void ToggleRenderMode()
	{
		if (render_mode == NORMAL)
			render_mode = DEBUG;
		else if (render_mode == DEBUG)
			render_mode = BOTH;
		else if (render_mode == BOTH)
			render_mode = NORMAL;
	}

	//---------------------------------
	//			No Longer Use
	//---------------------------------
	void CameraInput(int key)
	{
		//No Longer Use!
		switch (toupper(key))
		{
		case 'W':
			camera->MoveForward(delta_time);
			break;
		case 'S':
			camera->MoveBackward(delta_time);
			break;
		case 'A':
			camera->MoveLeft(delta_time);
			break;
		case 'D':
			camera->MoveRight(delta_time);
			break;
		case 'Q':
			camera->MoveUp(delta_time);
			break;
		case 'Z':
			camera->MoveDown(delta_time);
			break;
		default:
			break;
		}
	}

	//handle force control keys
	void ForceInput(int key)
	{
		if (!scene->GetSelectedActor())
			return;

		switch (toupper(key))
		{
			// Force controls on the selected actor
		case 'I': //forward
			scene->GetSelectedActor()->addForce(PxVec3(0, 0, -1) * gForceStrength);
			break;
		case 'K': //backward
			scene->GetSelectedActor()->addForce(PxVec3(0, 0, 1) * gForceStrength);
			break;
		case 'J': //left
			scene->GetSelectedActor()->addForce(PxVec3(-1, 0, 0) * gForceStrength);
			break;
		case 'L': //right
			scene->GetSelectedActor()->addForce(PxVec3(1, 0, 0) * gForceStrength);
			break;
		case 'U': //up
			scene->GetSelectedActor()->addForce(PxVec3(0, 1, 0) * gForceStrength);
			break;
		case 'M': //down
			scene->GetSelectedActor()->addForce(PxVec3(0, -1, 0) * gForceStrength);
			break;
		default:
			break;
		}
	}

	//--------------------------------
	//		Exit Callback
	//--------------------------------
	void exitCallback(void)
	{
		delete camera;
		delete scene;
		PhysicsEngine::PxRelease();
	}
}