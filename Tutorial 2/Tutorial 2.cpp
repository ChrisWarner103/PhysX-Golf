#include <iostream>
#include "VisualDebugger.h"

/*
* In this assignment I wanted to push my limits of my programming ability to get the most out of it. So I aimed to make a high detailed golf course
* which required me to implement my own solutions for certain things such as model loading, custom inputs and camera handling and different types
* of static and dynamic actors. The first thing that I needed to implement was the custom inputs for the player to shoot the ball. Since one of my
* favourite golf games that I play is Tower Unite, I wanted to get my simulation to be as close to that as I could, with the same shooting mechanics.
* I hit a few road blocks but I managed to get around them, such as the input manager functions in the VisualDebugger needed to be completely changed
* to accommodate the controls I wanted to use. I had to implement my own mouse functions such as MouseUp, Down and Hold and tracking each button that
* is pressed. Once this was done I had to edit the camera so that it would follow the ball constantly. After this I could implement my own shooting
* mechanic, which uses the screen coordinates of where the mouse is currently located and shoots it in the direction your mouse is pointed with different
* ranges of power.
*
* Once I had managed to finish the controls shooting the golf ball, I wanted a course to play on. So I modelled my own course in Blender and exported
* it as a obj file, which I could then write my own model importing script. Which can be found in ModelLoader.h. There was an issue though, I needed
* to set up the project so that I could import my own models. So I had to modify the different types of actors you could create and make my own
* MeshStatic and MeshDynamic classes which use the TriangleMesh and ConvexMesh types. I tried to implement a DyanamicTriangle mesh class but it kept
* throwing exceptions and I couldn't work out why. So some things that I wanted to do like implement a spike ball as a ball type wasn't possible.
* 
* The controls for the game are very simple, you aim with your mouse on the screen in the direction you want to shoot and the power indication is in
* the bottom left corner. When you release the Left Mouse button it will shoot the ball in the direction aimed towards. To switch the ball you can
* cycle between them all by pressing the F1 key. The camera controls have changed so holding down Right Mouse button and moving the mouse either forward
* or back will zoom in and out with the camera. That's mainly all the controls that are needed for the simulation to play.
* 
* I tried to indicate the theme of the assignment through the use of different balls you can play with, such as there being a Diamond, D20 for tabletop fantasy
* roleplaying and a barrel. As well as some environmental objects such as crystals and monster teeth for the traps.
* 
* Also to run the program, either use Visual Studio or open the .exe in the Release folder under x64.
*/

using namespace std;

int main()
{
	try 
	{ 
		VisualDebugger::Init("Tutorial 2", 800, 800); 
	}
	catch (Exception exc) 
	{ 
		cerr << exc.what() << endl;
		return 0; 
	}

	VisualDebugger::Start();

	return 0;
}