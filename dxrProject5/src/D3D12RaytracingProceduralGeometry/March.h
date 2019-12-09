#pragma once

#include "Cases.h"

class March
{
public:
	/// Member variables
	Cases* caseData;
	SDF* sdf;

	std::vector<vec3> positions; // Corners of every "cube" - the grid (""vertices"")
	std::vector<float> weights;  // The SDF values at each above corner
	std::vector< std::unique_ptr<Block>> blocks;   // The cubes to check
	std::unordered_map<std::string, std::unique_ptr<Edge>> edges;   // Edges that connect above positions
    
    // Grid data
    float divisions;
    int numVerts;
    int numBlocks;
    
    // AABB data, puts vertices in worldspace > model space
    vec3 tempRefScale;
    vec3 tempRefTrans;

    // Final triangle-vertices and normals (to pass into Mesh)
    std::vector<vec3> triVerts;
    std::vector<vec3> triNorms;

    // The end result
    //finalMesh: Mesh;


	/// FUNCTIONS
	March(vec3 scale, vec3 trans, float divs, Cases* cases, SDF* sdfS);
	~March();

	// Immediately sends this data to Mesh
	void callMeshClass();

	// Sets the "weights" for each cube-vertex, based on the sdf values at the positions
	void testVertexSDFs();

	// Helps the below function
	void resolveAmbiguities(std::vector<int> ambiguities, int blockNum);

	// Get the case numbers for each box
	void testBoxValues();

	// Check which marching-cube-edge a vertex falls on - pre-scale & translation
	std::pair<int, int> edgeCheck(vec3 point);

	// Determines the final triangle vertices for this mesh
	void setTriangles();
};

