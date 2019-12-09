#pragma once

#include "./SDFfucns.h"
#include "stdafx.h"


// STORES POSITIONS FOR CASES
class Triangle
{
public:
	std::array<vec3, 3> vertices; // Store counter clockwise
	std::array<vec3, 3> normVecs;

	Triangle(std::array<vec3, 3> verts);
	~Triangle();
};

// JUST STORES INDICES
class Triangle2
{
public:
	/*std::vector<int>*/std::array<int, 3> indices;

	Triangle2(std::array<int, 3> inds);//std::vector<int> indices);
	~Triangle2();

	// Helper functions when analyzing a triangle
	vec3 getNormal(vec3 p0, vec3 p1, vec3 p2);
	vec3 averageNormal(vec3 avgNorm, vec3 newNorm, float currNum);
};


class Edge 
{
public:
	int vertexIndex;
	float numVerts; // Number of marched vertices that land here
	Edge() {
		vertexIndex = -1;
		numVerts = 0;
	}
	~Edge() {}
};


class Block 
{            
public:															  //  [   0    ,    1    ,    2    ,    3    ,    4    ,    5    ,    6    ,    7   ]
	std::array<int, 8> vertIdxs; // Its gonna have verts w displacement [0, 0, 0] [0, 0, 1] [1, 0, 1] [1, 0, 0] [0, 1, 0] [0, 1, 1] [1, 1, 1] [1, 1, 0]
	vec3 pos;
	vec3 scale;
	
	std::vector<Triangle2*> triangles; // 2 for EDGEMODE
	int caseNum;
	vec3 rotation;
	bool invert;

	Block(std::array<int, 8> verts, vec3 position, vec3 scl);
	~Block();
};