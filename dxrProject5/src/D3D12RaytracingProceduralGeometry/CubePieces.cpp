#include "stdafx.h"
#include "CubePieces.h"


// ****** TRIANGLE *******

Triangle::Triangle(std::array<vec3, 3> verts) :
	vertices(verts), normVecs()
{}

Triangle::~Triangle()
{}

Triangle2::Triangle2(std::array<int, 3> inds) :
	indices(inds)
{}

Triangle2::~Triangle2()
{}

// Cross products the edges between the given vertices
vec3 Triangle2::getNormal(vec3 p0, vec3 p1, vec3 p2) {
	vec3 v1 = p1 - p0;
	vec3 v2 = p2 - p0;

	vec3 norm = cross(v1, v2);
	return normalize(norm);
}

vec3 Triangle2::averageNormal(vec3 avgNorm, vec3 newNorm, float currNum)
{
	// Initialize base value
	vec3 average = avgNorm;

	// Set weight for given normal
	average *= currNum;
	// And add the new normal
	average += newNorm;

	return normalize(average);
}

// Block Constructor
Block::Block(std::array<int, 8> verts, vec3 position, vec3 scl) :
	vertIdxs(verts), pos(position), scale(scl),
	triangles(), caseNum(-1), rotation(vec3(0.0, 0.0, 0.0)), invert(false)
{
	//triangles = std::vector<Triangle2>();
}

Block::~Block() { 
	/*for (int i = 0; i < triangles.size(); i++) {
		delete triangles[i];
	}*/
	triangles.clear(); 
}