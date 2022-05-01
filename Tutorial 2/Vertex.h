#pragma once
#include "PhysicsEngine.h"

namespace PhysicsEngine
{
	struct Vertex
	{
		PxVec3 position;
		PxVec3 color;
		PxVec2 texcoord;
		PxVec3 normal;
	};
}