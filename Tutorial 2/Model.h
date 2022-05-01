#pragma once
#include <vector>
#include "PhysicsEngine.h"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

namespace PhysicsEngine {

	using namespace std;

	class Model {
	public:

		Model(char* path)
		{
			this->loadModel(path);
		}

		vector<PxVec3> vertices;
		vector<PxU32> indices;
		vector<PxVec2> texCords;
		vector<PxVec3> normals;

		vector<PxReal> modelData;

	private:
		//vector<Mesh> meshes;
		string directory;

		void loadModel(string path)
		{
			std::ifstream in(path, std::ios::in);
			if (!in)
			{
				std::cout << ".obj file cannot be opened " << path << std::endl;
				exit(1);
			}

			this->directory = path.substr(0, (path.find_last_of('/') + 1));

			
		}
	};
}