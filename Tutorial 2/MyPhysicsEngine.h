#pragma once

#include "BasicActors.h"
#include "ModelLoader.h"
#include "Model.h"
#include <iostream>
#include <string>
#include <iomanip>

namespace PhysicsEngine
{
	using namespace std;

	//a list of colours: Circus Palette
	static const PxVec3 color_palette[] = { PxVec3(46.f / 255.f,9.f / 255.f,39.f / 255.f),PxVec3(217.f / 255.f,0.f / 255.f,0.f / 255.f),
		PxVec3(255.f / 255.f,45.f / 255.f,0.f / 255.f),PxVec3(255.f / 255.f,140.f / 255.f,54.f / 255.f),PxVec3(4.f / 255.f,117.f / 255.f,111.f / 255.f) };

	//pyramid vertices
	static std::vector<PxVec3> pyramid_verts;
	static PxVec3 pyramid_verts_temp[] = { {-3.238439f, 0.118574f, 1.000000f}, {-3.238439f, 0.005762f, 1.000000}, {-3.238439f,  0.118574f,  -6.500750f} };
	//pyramid triangles: a list of three vertices for each triangle e.g. the first triangle consists of vertices 1, 4 and 0
	//vertices have to be specified in a counter-clockwise order to assure the correct shading in rendering
	static std::vector<PxU32> pyramid_trigs;

	class MeshDynamic : public ConvexMesh
	{
	public:
		MeshDynamic(PxTransform pose = PxTransform(PxIdentity), PxReal density = 1.f) :
			ConvexMesh(vector<PxVec3>(begin(pyramid_verts), end(pyramid_verts)), pose, density)
		{
		}
	};

	class Mesh : public TriangleMesh
	{
	public:
		Mesh(PxTransform pose = PxTransform(PxIdentity)) :
			TriangleMesh(vector<PxVec3>(begin(pyramid_verts), end(pyramid_verts)), vector<PxU32>(begin(pyramid_trigs), end(pyramid_trigs)), pose)
		{
		}
	};

	struct TriggerTypes {
		enum MyEnum
		{
			Hole1 = 0,
			Hole2 = 1,
			Hole3 = 2,
			HoleFinal = 4,
		};
	};

	struct FilterGroup
	{
		enum Enum
		{
			BALL = (1 << 0),
			HOLES = (1 << 1),
			TRAPS = (1 << 2)
		};
	};

	///A customised collision class, implemneting various callbacks
	class MySimulationEventCallback : public PxSimulationEventCallback
	{
	public:
		//an example variable that will be checked in the main simulation loop
		bool trigger;
		bool collision;
		PxRigidActor* otherObj, * triggerObj;

		MySimulationEventCallback() : trigger(false) {}

		///Method called when the contact with the trigger object is detected.
		virtual void onTrigger(PxTriggerPair* pairs, PxU32 count)
		{
			//you can read the trigger information here
			for (PxU32 i = 0; i < count; i++)
			{
				//filter out contact with the planes
				if (pairs[i].otherShape->getGeometryType() != PxGeometryType::ePLANE)
				{
					//check if eNOTIFY_TOUCH_FOUND trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
					{
						cerr << "onTrigger::eNOTIFY_TOUCH_FOUND" << endl;

						otherObj = pairs[i].otherShape->getActor();
						triggerObj = pairs[i].triggerShape->getActor();

						trigger = true;
					}
					//check if eNOTIFY_TOUCH_LOST trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_LOST)
					{
						cerr << "onTrigger::eNOTIFY_TOUCH_LOST" << endl;

						otherObj = NULL;
						triggerObj = NULL;

						trigger = false;
					}
					if (!strcmp(pairs[i].otherActor->getName(), "Ball"))
						printf("Ball has collided!");


					cout << pairs[i].otherShape->getActor()->getName() << " collided with " << pairs[i].triggerShape->getActor()->getName() << endl;
				}
			}
		}

		///Method called when the contact by the filter shader is detected.
		virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
		{
			cerr << "Contact found between " << pairHeader.actors[0]->getName() << " " << pairHeader.actors[1]->getName() << endl;

			//check all pairs
			for (PxU32 i = 0; i < nbPairs; i++)
			{
				//check eNOTIFY_TOUCH_FOUND
				if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
				{
					cerr << "onContact::eNOTIFY_TOUCH_FOUND" << endl;
					collision = true;

					triggerObj = pairHeader.actors[0];
					otherObj = pairHeader.actors[1];
				}
				//check eNOTIFY_TOUCH_LOST
				if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_LOST)
				{
					cerr << "onContact::eNOTIFY_TOUCH_LOST" << endl;
					collision = false;

					triggerObj = NULL;
					otherObj = NULL;
				}
			}
		}

		virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) {}
		virtual void onWake(PxActor** actors, PxU32 count) {}
		virtual void onSleep(PxActor** actors, PxU32 count) {}
#if PX_PHYSICS_VERSION >= 0x304000
		virtual void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) {}
#endif
	};

	//A simple filter shader based on PxDefaultSimulationFilterShader - without group filtering
	static PxFilterFlags CustomFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		// let triggers through
		if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlags();
		}

		pairFlags = PxPairFlag::eCONTACT_DEFAULT;
		//enable continous collision detection
//		pairFlags |= PxPairFlag::eCCD_LINEAR;


		//customise collision filtering here
		//e.g.

		// trigger the contact callback for pairs (A,B) where 
		// the filtermask of A contains the ID of B and vice versa.
		if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		{
			//trigger onContact callback for this pair of objects
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;
			//			pairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS;
		}

		return PxFilterFlags();
	};

	///Custom scene class
	class MyScene : public Scene
	{
		Plane* plane;
		Sphere* ball;
		BoxStatic* hole1, * hole2, * hole3, * pipeExit, * holeFinal;
		PxMaterial* course_phys_mat, * rail_phys_mat, * ballMaterial, * ice_phys_mat;
		Mesh* course, * rail, * iceFloor, *ballHolder, *environment;
		MeshDynamic* diamond, * d20, *barrel;
		ModelImport modelImporter;
		BoxRigid* joint1, * jointBlade;


	public:

		bool triggered = false;
		bool contact = false;

		//Made public to be accessed from other classes.
		MySimulationEventCallback* my_callback;

		//Specify your custom filter shader here!
		//PxDefaultSimulationFilterShader by default
		MyScene() : Scene(CustomFilterShader)
		{

		};

		///A custom scene class
		void SetVisualisation()
		{
			px_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
		}

		//Custom scene initialisation
		virtual void CustomInit()
		{

			SetVisualisation();

			///Initialise and set the customised event callback
			my_callback = new MySimulationEventCallback();
			px_scene->setSimulationEventCallback(my_callback);

			GetMaterial()->setDynamicFriction(0.34f);

			ballMaterial = CreateMaterial(0.6f, 0.5f, 0.3f);
			course_phys_mat = CreateMaterial(0.4f, 0.4f, 0.02f);
			rail_phys_mat = CreateMaterial(0.1f, 0.1f, 1);
			ice_phys_mat = CreateMaterial(0, 0, 0.3f);

			ObjectInit();

			//RevoluteJoint joint(joint1, PxTransform(PxVec3(0, 0, 0)), jointBlade, PxTransform(PxVec3(0, -5, 0), PxQuat(PxPiDivTwo, PxVec3(0, 0, 1))));
		}

		//Custom udpate function
		virtual void CustomUpdate()
		{
			//float rotationValue = PxSin(0.01f * px_scene->getTimestamp());

			//joint1->GetShape()->setLocalPose(PxTransform(PxQuat(rotationValue, PxVec3(0, 0, 1.f))));
		}

		void ObjectInit()
		{
			plane = new Plane();
			plane->Color(PxVec3(210.f / 255.f, 210.f / 255.f, 210.f / 255.f));
			//plane->Material(groundMaterial);
			plane->SetupFiltering(FilterGroup::TRAPS, FilterGroup::BALL, 0);
			plane->Name("Floor");
			Add(plane);

			ball = new Sphere(PxTransform(PxVec3(0.f, .5f, 0.f)), .25f, 2.5f);
			ball->Color(color_palette[0]);
			ball->Material(ballMaterial);
			ball->SetAngularDamping(2.0f);
			ball->SetupFiltering(FilterGroup::BALL, FilterGroup::HOLES | FilterGroup::TRAPS, 0);
			ball->Name("Ball");
			Add(ball);

			hole1 = new BoxStatic(PxTransform(PxVec3(-12.9f, 0.8f, -31.9f)), PxVec3(.25f, .25f, .25f));
			hole1->Color(color_palette[2]);
			hole1->SetupFiltering(FilterGroup::HOLES, FilterGroup::BALL, 0);
			hole1->Name("Hole1");
			hole1->SetTrigger(true);
			Add(hole1);

			hole2 = new BoxStatic(PxTransform(PxVec3(-19.3f, 2.5f, -30.3f)), PxVec3(.25f, .25f, .25f));
			hole2->Color(color_palette[2]);
			hole2->SetupFiltering(FilterGroup::HOLES, FilterGroup::BALL, 0);
			hole2->Name("Hole2");
			hole2->SetTrigger(true);
			Add(hole2);

			hole3 = new BoxStatic(PxTransform(PxVec3(-26.1f, 0.9f, -28.1f)), PxVec3(.25f, .25f, .25f));
			hole3->Color(color_palette[2]);
			hole3->SetupFiltering(FilterGroup::HOLES, FilterGroup::BALL, 0);
			hole3->Name("Hole3");
			hole3->SetTrigger(true);
			Add(hole3);

			holeFinal = new BoxStatic(PxTransform(PxVec3(-6.8f, 2.6f, -55.6f)), PxVec3(.25f, .25f, .25f));
			holeFinal->Color(color_palette[2]);
			holeFinal->SetupFiltering(FilterGroup::HOLES, FilterGroup::BALL, 0);
			holeFinal->Name("HoleFinal");
			holeFinal->SetTrigger(true);
			Add(holeFinal);

			pipeExit = new BoxStatic(PxTransform(PxVec3(-24.f, 0.8f, -0.96f)), PxVec3(.25f, .25f, .25f));
			pipeExit->SetupFiltering(FilterGroup::HOLES, FilterGroup::BALL, 0);
			pipeExit->Name("PipeExit");
			pipeExit->SetTrigger(true);
			Add(pipeExit);

			modelImporter.LoadOBJ2("..//Assets//Models//Course.obj", pyramid_verts, pyramid_trigs);

			course = new Mesh(PxTransform(0, 0, 0));
			course->Color(color_palette[2]);
			course->Material(course_phys_mat);
			course->Name("Course");
			course->SetupFiltering(FilterGroup::BALL, FilterGroup::HOLES, 0);
			Add(course);

			pyramid_verts.clear();
			pyramid_trigs.clear();

			modelImporter.LoadOBJ2("..//Assets//Models//Railing.obj", pyramid_verts, pyramid_trigs);

			rail = new Mesh(PxTransform(0, 0, 0));
			rail->Color(color_palette[3]);
			rail->Material(rail_phys_mat);
			course->SetupFiltering(FilterGroup::BALL, FilterGroup::HOLES, 0);
			rail->Name("Railing");
			Add(rail);

			pyramid_verts.clear();
			pyramid_trigs.clear();

			modelImporter.LoadOBJ2("..//Assets//Models//Ice Floor.obj", pyramid_verts, pyramid_trigs);

			iceFloor = new Mesh(PxTransform(0, 0, 0));
			iceFloor->Color(color_palette[4]);
			iceFloor->Material(ice_phys_mat);
			iceFloor->SetupFiltering(FilterGroup::BALL, FilterGroup::HOLES, 0);
			iceFloor->Name("Ice Floor");
			Add(iceFloor);


			pyramid_verts.clear();
			pyramid_trigs.clear();

			modelImporter.LoadOBJ2("..//Assets//Models//Environment.obj", pyramid_verts, pyramid_trigs);

			environment = new Mesh(PxTransform(0, 0, 0));
			environment->Color(color_palette[4]);
			environment->Material(ice_phys_mat);
			environment->SetupFiltering(FilterGroup::BALL, FilterGroup::HOLES, 0);
			environment->Name("Environment Detail");
			Add(environment);

			pyramid_verts.clear();
			pyramid_trigs.clear();

			modelImporter.LoadOBJ2("..//Assets//Models//Ball Holder.obj", pyramid_verts, pyramid_trigs);

			ballHolder = new Mesh(PxTransform(0, 0, 100));
			Add(ballHolder);

			pyramid_verts.clear();
			pyramid_trigs.clear();

			modelImporter.LoadOBJ2("..//Assets//Models//Diamond.obj", pyramid_verts, pyramid_trigs);

			diamond = new MeshDynamic(PxTransform(0, 1, 100), 6.3f);
			diamond->Color(color_palette[0]);
			diamond->Material(ballMaterial);
			diamond->SetAngularDamping(2.0f);
			diamond->SetupFiltering(FilterGroup::BALL, FilterGroup::HOLES | FilterGroup::TRAPS, 0);
			diamond->Name("Diamond");
			Add(diamond);

			pyramid_verts.clear();
			pyramid_trigs.clear();

			modelImporter.LoadOBJ2("..//Assets//Models//D20.obj", pyramid_verts, pyramid_trigs);

			d20 = new MeshDynamic(PxTransform(0, 1, 100), 4.1f);
			d20->Color(color_palette[0]);
			d20->Material(ballMaterial);
			d20->SetAngularDamping(2.0f);
			d20->SetupFiltering(FilterGroup::BALL, FilterGroup::HOLES | FilterGroup::TRAPS, 0);
			d20->Name("D20");
			Add(d20);

			pyramid_verts.clear();
			pyramid_trigs.clear();
			modelImporter.LoadOBJ2("..//Assets//Models//Barrel.obj", pyramid_verts, pyramid_trigs);

			barrel = new MeshDynamic(PxTransform(0, 1, 100), 1.f);
			barrel->Color(color_palette[0]);
			barrel->Material(ballMaterial);
			barrel->SetAngularDamping(2.0f);
			barrel->SetupFiltering(FilterGroup::BALL, FilterGroup::HOLES | FilterGroup::TRAPS, 0);
			barrel->Name("Barrel");
			Add(barrel);

			//joint1 = new BoxRigid(PxTransform(0, 15, -40));
			//joint1->SetKinematic(true);
			//Add(joint1);

			//jointBlade = new BoxRigid(PxTransform(0, 10, -40), PxVec3(5, 2.5f, .1f));
			////jointBlade->SetKinematic(true);
			//Add(jointBlade);
		}
	};
}
