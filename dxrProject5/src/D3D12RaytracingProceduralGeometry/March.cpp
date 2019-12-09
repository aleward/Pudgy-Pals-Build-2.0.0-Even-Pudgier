#include "stdafx.h"
#include "March.h"

std::string edgeLabel(int n1, int n2) {
	return "[" + std::to_string(n1) + ", " + std::to_string(n2) + "]";
}

March::March(vec3 scale, vec3 trans, float divs, Cases* cases, SDF* sdfS) :
	caseData(cases),
	sdf(sdfS),

	tempRefScale(scale),
	tempRefTrans(trans),

	divisions(divs),
	numBlocks(divs * divs * divs),
	numVerts((divs + 1) * (divs + 1) * (divs + 1)),

	positions(), //new Array<vec3>(this.numVerts);
	weights(),   //new Array<number>(this.numVerts);
	blocks(),	 //new Array<Block>(this.numBlocks);
	edges(),	 //new Map<string, Edge>();

	triVerts(),
	triNorms(),
	triIndxVBO()
{
	// Allocate space
	positions.resize(numVerts);
	weights.resize(numVerts);
	blocks.resize(numBlocks);

	float delta = 2.0 / divisions;

	// Vertex loop
	for (float x = 0.0; x <= divisions; x++) {
		for (float y = 0.0; y <= divisions; y++) {
			for (float z= 0.0; z <= divisions; z++) {
				// Find vertex location for this index
				vec3 temp = vec3((x * delta - 1.0) * tempRefScale[0] + tempRefTrans[0],
								 (y * delta - 1.0) * tempRefScale[1] + tempRefTrans[1],
								 (z * delta - 1.0) * tempRefScale[2] + tempRefTrans[2]);
				//Translate 3D indices->1D
				int index = z +
					(divisions + 1) * y +
					(divisions + 1) * (divisions + 1) * x;

				positions[index] = temp;
			}
		}
	}

	// Edge loop
	for (float x = 0.0; x <= divisions; x++) {
		for (float y = 0.0; y <= divisions; y++) {
			for (float z = 0.0; z <= divisions; z++) {
				// Current vertex index
				int index = z +
					(divisions + 1) * y +
					(divisions + 1) * (divisions + 1) * x;

				// Write up to three new edges per point
				// x-axis edge
				if (x < divisions) {
					int index2 = z +
								(divisions + 1) * y +
								(divisions + 1) * (divisions + 1) * (x + 1);
					std::string eL = edgeLabel(index, index2);
					//Edge* e = new Edge();
					edges.insert({ eL, std::make_unique<Edge>()});//e}); //.set(eL, e);
				}

				// y-axis edge
				if (y < divisions) {
					int index2 = z +
								(divisions + 1) * (y + 1) +
								(divisions + 1) * (divisions + 1) * x;
					std::string eL = edgeLabel(index, index2);
					edges.insert({ eL, std::make_unique<Edge>() });
				}

				// z-axis edge
				if (y < divisions) {
					int index2 = (z + 1) +
								(divisions + 1) * y +
								(divisions + 1) * (divisions + 1) * x;
					std::string eL = edgeLabel(index, index2);
					edges.insert({ eL, std::make_unique<Edge>() });
				}
			}
		}
	}


	vec3 newScale = vec3(tempRefTrans[0] * 2.0 / divisions,
						 tempRefTrans[1] * 2.0 / divisions,
						 tempRefTrans[2] * 2.0 / divisions);
	// Block loop
	for (float x = 0.0; x <= divisions; x++) {
		for (float y = 0.0; y <= divisions; y++) {
			for (float z = 0.0; z <= divisions; z++) {
				vec3 newPos = vec3(((x * delta - 1.0) + delta / 2) * tempRefScale[0] + tempRefTrans[0],
								   ((y * delta - 1.0) + delta / 2) * tempRefScale[1] + tempRefTrans[1],
								   ((z * delta - 1.0) + delta / 2) * tempRefScale[2] + tempRefTrans[2]);

				// FILL ARRAY
				std::array<int, 8> vertArr = std::array<int, 8>();
				int idxNum = 0;
				// Its gonna have verts w displacement [0, 0, 0] [0, 0, 1] [1, 0, 1] [1, 0, 0] [0, 1, 0] [0, 1, 1] [1, 1, 1] [1, 1, 0]
				for (float iY = 0; iY < 2; iY++) {
					for (float iX = 0; iX < 2; iX++) {
						for (float iZ = 0; iZ < 2; iZ++) {
							float tempZ = iZ;
							if (iX == 1) { tempZ = 1 - iZ; }

							int index = (z + tempZ) +
										(divisions + 1) * (y + iY) +
										(divisions + 1) * (divisions + 1) * (x + iX);

							vertArr[idxNum] = index;
							idxNum++;
						}
					}
				}

				int index = z +
					(divisions + 1) * y +
					(divisions + 1) * (divisions + 1) * x;
				if (index < numBlocks) {
					blocks[index] = std::make_unique<Block>(vertArr, newPos, newScale);
				}
			}
		}
	}
}


March::~March()
{
	/*for (int i = 0; i < blocks.size(); i++) {
		delete blocks[i];
	}
	blocks.clear();*/
	//edges.clear();

	/*positions.clear();
	weights.clear();
	triVerts.clear();
	triNorms.clear();*/
	//std::vector<Block*>().swap(blocks);
}

void March::callMeshClass()
{
	for (int i = 0; i < blocks.size(); i++) {
		Block& currBlock = *blocks[i];
		for (int j = 0; j < currBlock.triangles.size(); j++) {
			for (int k = 0; k < 3; k++) {
				triIndxVBO.push_back(currBlock.triangles[j]->indices[k]);
			}
		}
	}
}

void March::testVertexSDFs()
{
	for (int i = 0; i < numVerts; i++) {
		weights[i] = sdf->sceneSDF(positions[i]);
	}
}

void March::resolveAmbiguities(std::vector<int> ambiguities, int blockNum)
{
	if (blocks[blockNum]->caseNum != caseData->caseArray[blocks[blockNum]->caseNum]->ambNum) {
		vec3 avg = vec3(0.0, 0.0, 0.0);
		for (int i = 0; i < ambiguities.size(); i++) {
			avg += positions[ambiguities[i]];
		}
		avg /= ambiguities.size();

		float result = sdf->sceneSDF(avg);
		if (result <= 0) {
			int cNum = blocks[blockNum]->caseNum;
			blocks[blockNum]->caseNum = caseData->caseArray[cNum]->ambNum;
		}
	}
}

void March::testBoxValues()
{
	int i = 0;
	for (int b = 0; b < blocks.size(); b++) {
		// Vertices to check for VERY BASIC ambiguity testing
		std::vector<int> ambiguities = std::vector<int>();

		int binary = 0; // End 8-bit value
		int bit = 128;  // Current value to "or" by

		// Each bit corresponds to one of 8 cube vertices
		for (int v = 0; v < 8; v++) {
			if (weights[blocks[b]->vertIdxs[v]] <= 0.0) {
				binary = binary | bit;

				// For ambiguities
				ambiguities.push_back(blocks[b]->vertIdxs[v]);
			}
			bit /= 2;
		}

		// Variable to check inverse
		int opposite = 255 ^ binary;

		// For NORMAL CASES
		if (std::get<int>(caseData->map[binary]) != -1) {
			blocks[b]->caseNum = std::get<int>(caseData->map[binary]);
			blocks[b]->rotation = std::get<vec3>(caseData->map[binary]);
		}
		// FOR OPPOSITES OF NORMAL CASES
		else if (std::get<int>(caseData->map[opposite]) != -1) {
			blocks[b]->caseNum = std::get<int>(caseData->map[opposite]);
			blocks[b]->rotation = std::get<vec3>(caseData->map[opposite]);
			blocks[b]->invert = true;
			i++;
		}

		// RESOLUTION FOR AMBIGUOUS CASES
		int cNum = blocks[b]->caseNum;
		if (cNum != -1 && caseData->caseArray[cNum]->canBeAmbiguous) {
			resolveAmbiguities(ambiguities, b);
		}
	}
}

std::pair<int, int> March::edgeCheck(vec3 point)
{
	if (point[1] == -0.5 && point[2] == -0.5) { return std::make_pair<int, int>(0, 3); }
    if (point[1] == -0.5 && point[2] ==  0.5) { return std::make_pair<int, int>(1, 2); }
    if (point[1] ==  0.5 && point[2] ==  0.5) { return std::make_pair<int, int>(5, 6); }
    if (point[1] ==  0.5 && point[2] == -0.5) { return std::make_pair<int, int>(4, 7); }

    if (point[0] == -0.5 && point[2] == -0.5) { return std::make_pair<int, int>(0, 4); }
    if (point[0] == -0.5 && point[2] ==  0.5) { return std::make_pair<int, int>(1, 5); }
    if (point[0] ==  0.5 && point[2] ==  0.5) { return std::make_pair<int, int>(2, 6); }
    if (point[0] ==  0.5 && point[2] == -0.5) { return std::make_pair<int, int>(3, 7); }

    if (point[0] == -0.5 && point[1] == -0.5) { return std::make_pair<int, int>(0, 1); }
    if (point[0] == -0.5 && point[1] ==  0.5) { return std::make_pair<int, int>(4, 5); }
    if (point[0] ==  0.5 && point[1] ==  0.5) { return std::make_pair<int, int>(7, 6); }
    if (point[0] ==  0.5 && point[1] == -0.5) { return std::make_pair<int, int>(3, 2); }

	return std::make_pair<int, int>(0, 0);
}

void March::setTriangles()
{
	vec3 center = vec3(0, 0, 0);
    vec3 scale = vec3(tempRefScale[0] * 2.0 / divisions,
                      tempRefScale[1] * 2.0 / divisions,
                      tempRefScale[2] * 2.0 / divisions);

    for (int i = 0; i < blocks.size(); i++) {
        int c = blocks[i]->caseNum;
        if (c != -1) {
            for (int t = 0; t < caseData->caseArray[c]->triangles.size(); t++) {
                // Track the current triangle indices for triVerts
                std::array<int, 3> currTriangle = std::array<int, 3>();
				std::array<int, 3> currWeights  = std::array<int, 3>();

                for (int v = 0; v < 3; v++) {
                    // Copy vertex from current case
                    vec3 vert = vec3();
                    // ... in reverse order if needed
                    if (blocks[i]->invert) {
                        vert = caseData->caseArray[c]->triangles[t]->vertices[2 - v];
                    } else {
						vert = caseData->caseArray[c]->triangles[t]->vertices[v];
                    }
                    // Rotate each vertex
					vert = mat3::rotateZ(sdf->radians(blocks[i]->rotation[2])) * vert;///vec3.rotateZ(vert, vert, center, this.blocks[i].rotation[2] * rad);
					vert = mat3::rotateY(sdf->radians(blocks[i]->rotation[1])) * vert;///vec3.rotateY(vert, vert, center, this.blocks[i].rotation[1] * rad);
					vert = mat3::rotateX(sdf->radians(blocks[i]->rotation[0])) * vert;///vec3.rotateX(vert, vert, center, this.blocks[i].rotation[0] * rad);
                    // - Check and record which edge it is on
					std::pair<int, int> e = edgeCheck(vert);
                    int e0 = blocks[i]->vertIdxs[std::get<0>(e)];
                    int e1 = blocks[i]->vertIdxs[std::get<1>(e)];
                    std::string eL = edgeLabel(e0, e1);

                    if (edges.count(eL) > 0) {
                        Edge& currEdge = *edges.at(eL);
                        // If the edge has no existing vertex, create a new one
                        if (currEdge.vertexIndex == -1) {
                            // Interpolate between the existing vertices
                            vec3 pos1 = positions[e0];
                            vec3 pos2 = positions[e1];
                            // With existing weights
                            float weight1 = weights[e0];
                            float weight2 = weights[e1];
                            float lerp = -weight1 / (weight2 - weight1);

                            vert = vec3(pos1[0] + lerp * (pos2[0] - pos1[0]), 
                                        pos1[1] + lerp * (pos2[1] - pos1[1]), 
                                        pos1[2] + lerp * (pos2[2] - pos1[2]));

                            currTriangle[v] = triVerts.size();
                            currWeights[v] = currEdge.numVerts;

                            currEdge.numVerts = 1;
                            currEdge.vertexIndex = triVerts.size();

                            triVerts.push_back(vert);
                            // Fill with an empty normal, to be handled later
                            triNorms.push_back(vec3(0.0, 0.0, 0.0));
                        } 
                        // If the edge already has a vertex, blend normal values
                        else {
                            // Update triangle and edge
                            currTriangle[v] = currEdge.vertexIndex;
                            currWeights[v]  = currEdge.numVerts;
                            currEdge.numVerts++;
                        }
                    } 
                }

				std::unique_ptr<Triangle2> newTri = std::make_unique<Triangle2>(currTriangle); //Triangle2* newTri = new Triangle2(currTriangle);
                // Flat normal of this triangle face
                vec3 faceNorm = newTri->getNormal(triVerts[currTriangle[0]],
                                                  triVerts[currTriangle[1]],
                                                  triVerts[currTriangle[2]]);

                // Update stored normals
                for (int n = 0; n < 3; n++) {
                    int currIdx = currTriangle[n]; // indices for normals and vertices match

                    // If this is the first vertex on its edge
                    if (currWeights[n] == 0) {
                        triNorms[currIdx] = faceNorm;
                    } 
                    // If this is one of many vertices in the same place
                    else {
                        // Average possible normals for smooth shading
                        vec3 avg = triNorms[currIdx];
                        triNorms[currIdx] = newTri->averageNormal(avg, faceNorm, currWeights[n]);
                    }
                }

                // Every three vertices, push() back a new Triangle into this block's triangle array
                blocks[i]->triangles.push_back(std::move(newTri));
            }
        }
    }
}