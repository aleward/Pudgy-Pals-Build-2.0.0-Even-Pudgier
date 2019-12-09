#pragma once

#include "CubePieces.h"

// Have trangles on range [-0.5, 0.5]
class Case {
public: 
	std::vector<Triangle*> triangles;
	bool canBeAmbiguous;

	// List indices for ambiguities and tests to decide which one
	int ambNum;

	Case(std::vector<Triangle*> tris, bool canBeAmb, int ambN);
};



class Cases
{
public:
	std::vector<Triangle*> cornerTris = { new Triangle({vec3(-0.5, 0.0, -0.5),
														vec3(-0.5, -0.5, 0.0),
														vec3(0.0, -0.5, -0.5)}), // 0 - Bottom Front Left
										  new Triangle({vec3(-0.5, 0.0, 0.5),
														vec3(-0.5, -0.5, 0.0),
														vec3(0.0, -0.5, 0.5)}), // 1 - Bottom Front Right
										  new Triangle({vec3(0.5, 0.0, 0.5),
														vec3(0.0, -0.5, 0.5),
														vec3(0.5, -0.5, 0.0)}), // 2 - Bottom Back Right
										  new Triangle({vec3(0.5, 0.0, -0.5),
														vec3(0.5, -0.5, 0.0),
														vec3(0.0, -0.5, -0.5)}), // 3 - Bottom Back Left
										  new Triangle({vec3(-0.5, 0.0, -0.5),
														vec3(0.0, 0.5, -0.5),
														vec3(-0.5, 0.5, 0.0)}), // 4 - Top Front Left
										  new Triangle({vec3(-0.5, 0.0, 0.5),
														vec3(0.0, 0.5, 0.5),
														vec3(-0.5, 0.5, 0.0)}), // 5 - Top Front Right
										  new Triangle({vec3(0.5, 0.0, 0.5),
														vec3(0.0, 0.5, 0.5),
														vec3(0.5, 0.5, 0.0)}), // 6 - Top Back Right
										  new Triangle({vec3(0.5, 0.0, -0.5),
														vec3(0.0, 0.5, -0.5),
														vec3(0.5, 0.5, 0.0)})  // 7 - Top Back Left
	};

	// NORMAL CASES
	const Case* Case0 = new Case({}, false, 0);
	const Case* Case1 = new Case({ cornerTris[0] }, false, 1);

	const Case* Case2 = new Case({ new Triangle({vec3(-0.5, 0.0, -0.5),
								  			     vec3(-0.5, 0.0, 0.5),
								  			     vec3(0.0, -0.5, 0.5)}), // Top One
								   new Triangle({vec3(-0.5, 0.0, -0.5),
								  			     vec3(0.0, -0.5, 0.5),
								  			     vec3(0.0, -0.5, -0.5)}) }, // Bottom one
								   false, 2); 

	const Case* Case3 = new Case({ cornerTris[0], cornerTris[5] }, true, 15);
	const Case* Case4 = new Case({ cornerTris[0], cornerTris[6] }, true, 4);

	const Case* Case5 = new Case({ new Triangle({vec3(0.5, 0.0, 0.5),
												 vec3(0.5, 0.0, -0.5),
												 vec3(-0.5, 0.0, 0.5)}), // Top Piece
								   new Triangle({vec3(-0.5, 0.0, 0.5),
												 vec3(0.5, 0.0, -0.5),
												 vec3(0.0, -0.5, -0.5)}), // Middle Piece
								   new Triangle({vec3(-0.5, 0.0, 0.5),
												 vec3(0.0, -0.5, -0.5),
												 vec3(-0.5, -0.5, 0.0)}) },  // Bottom one
								   false, 5);

	const Case* Case6 = new Case({ new Triangle({vec3(-0.5, 0.0, -0.5),
								   			     vec3(-0.5, 0.0, 0.5),
								   			     vec3(0.0, -0.5, 0.5)}), // Top One
								   new Triangle({vec3(-0.5, 0.0, -0.5),
								   			     vec3(0.0, -0.5, 0.5),
								   			     vec3(0.0, -0.5, -0.5)}), cornerTris[6] }, // Bottom one
								   true, 6); 

	const Case* Case7 = new Case({ cornerTris[1], cornerTris[6], cornerTris[4] }, true, 7);

	const Case* Case8 = new Case({ new Triangle({vec3(0.5, 0.0, 0.5),
												 vec3(0.5, 0.0, -0.5),
												 vec3(-0.5, 0.0, -0.5)}), // Back Piece
								   new Triangle({vec3(0.5, 0.0, 0.5),
												 vec3(-0.5, 0.0, -0.5),
												 vec3(-0.5, 0.0, 0.5)}) }, // Front one
									false, 8); 

	const Case* Case9 = new Case({ new Triangle({vec3(0.0, -0.5, 0.5),
												 vec3(-0.5, 0.0, -0.5),
												 vec3(-0.5, -0.5, 0.0)}), // bottom Piece
								   new Triangle({vec3(0.0, -0.5, 0.5),
												 vec3(0.0, 0.5, -0.5),
												 vec3(-0.5, 0.0, -0.5)}), // Midbottom Piece
								   new Triangle({vec3(0.0, -0.5, 0.5),
												 vec3(0.5, 0.0, 0.5),
												 vec3(0.0, 0.5, -0.5)}), // Midtop Piece
								   new Triangle({vec3(0.5, 0.0, 0.5),
												 vec3(0.5, 0.5, 0.0),
												 vec3(0.0, 0.5, -0.5)}) }, // top one
								   false, 9); 

	const Case* Case10 = new Case({ new Triangle({vec3(-0.5, 0.5, 0.0),
												 vec3(0.0, -0.5, -0.5),
												 vec3(0.0, 0.5, -0.5)}), // Left top
								   new Triangle({vec3(-0.5, 0.5, 0.0),
												 vec3(-0.5, -0.5, 0.0),
												 vec3(0.0, -0.5, -0.5)}), // Left bottom
								   new Triangle({vec3(0.0, 0.5, 0.5),
												 vec3(0.5, 0.5, 0.0),
												 vec3(0.5, -0.5, 0.0)}), // Right top
								   new Triangle({vec3(0.0, 0.5, 0.5),
												 vec3(0.5, -0.5, 0.0),
												 vec3(0.0, -0.5, 0.5)}) }, // Right bottom
								  true, 10); 

	const Case* Case11 = new Case({ new Triangle({vec3(0.0, -0.5, 0.5),
												 vec3(0.0, 0.5, 0.5),
												 vec3(-0.5, -0.5, 0.0)}), // bottom right
								   new Triangle({vec3(0.0, 0.5, 0.5),
												 vec3(0.5, 0.0, -0.5),
												 vec3(-0.5, -0.5, 0.0)}), // Mid
								   new Triangle({vec3(0.5, 0.0, -0.5),
												 vec3(-0.5, 0.0, -0.5),
												 vec3(-0.5, -0.5, 0.0)}), // bottom left
								   new Triangle({vec3(0.0, 0.5, 0.5),
												 vec3(0.5, 0.5, 0.0),
												 vec3(0.5, 0.0, -0.5)}) }, // top Piece
								   false, 11); 

	const Case* Case12 = new Case({ cornerTris[4],
							   new Triangle({vec3(0.5, 0.0, 0.5),
											 vec3(0.5, 0.0, -0.5),
											 vec3(-0.5, 0.0, 0.5)}), // Top Piece
							   new Triangle({vec3(-0.5, 0.0, 0.5),
											 vec3(0.5, 0.0, -0.5),
											 vec3(0.0, -0.5, -0.5)}), // Middle Piece
							   new Triangle({vec3(-0.5, 0.0, 0.5),
											 vec3(0.0, -0.5, -0.5),
											 vec3(-0.5, -0.5, 0.0)}) }, // Bottom one
                              true, 12); 

	const Case* Case13 = new Case({ cornerTris[0], cornerTris[2], cornerTris[5], cornerTris[7] }, true, 13);

	const Case* Case14 = new Case({ new Triangle({vec3(-0.5, -0.5, 0.0),
												 vec3(-0.5, 0.0, 0.5),
												 vec3(0.5, 0.0, 0.5)}), // bottom right
								   new Triangle({vec3(-0.5, -0.5, 0.0),
												 vec3(0.5, 0.0, 0.5),
												 vec3(0.0, 0.5, -0.5)}), // Mid
								   new Triangle({vec3(-0.5, -0.5, 0.0),
												 vec3(0.0, 0.5, -0.5),
												 vec3(0.0, -0.5, -0.5)}), // bottom left
								   new Triangle({vec3(0.5, 0.0, 0.5),
												 vec3(0.5, 0.5, 0.0),
												 vec3(0.0, 0.5, -0.5)}) }, // top Piece
								   false, 14); 

// AMBIGUOUS CASES
	const Case* Case3_2 = new Case({ new Triangle({vec3(-0.5, 0.0, -0.5),
												 vec3(-0.5, 0.5, 0.0),
												 vec3(0.0, -0.5, -0.5)}), // bottom right
								   new Triangle({vec3(-0.5, 0.5, 0.0),
												 vec3(0.0, 0.5, 0.5),
												 vec3(0.0, -0.5, -0.5)}), // Mid
								   new Triangle({vec3(0.0, -0.5, -0.5),
												 vec3(0.0, 0.5, 0.5),
												 vec3(-0.5, 0.0, 0.5)}), // bottom left
								   new Triangle({vec3(0.0, -0.5, -0.5),
												 vec3(-0.5, 0.0, 0.5),
												 vec3(-0.5, -0.5, 0.0)}) }, // top Piece
								   false, 15);

	const Case* Case6_2 = new Case({ new Triangle({vec3(-0.5, 0.0, -0.5),
											 vec3(-0.5, 0.5, 0.0),
											 vec3(0.0, -0.5, -0.5)}), // bottom right
							   new Triangle({vec3(-0.5, 0.5, 0.0),
											 vec3(0.0, 0.5, 0.5),
											 vec3(0.0, -0.5, -0.5)}), // Mid
							   new Triangle({vec3(0.0, -0.5, -0.5),
											 vec3(0.0, 0.5, 0.5),
											 vec3(-0.5, 0.0, 0.5)}), // bottom left
							   new Triangle({vec3(0.0, -0.5, -0.5),
											 vec3(-0.5, 0.0, 0.5),
											 vec3(-0.5, -0.5, 0.0)}) }, // top Piece
                               false, 15);


	// MOST IMPORTANT PARTS                          
	const std::array<const Case*, 17> caseArray = { Case0, Case1, Case2, Case3, Case4, Case5, Case6, Case7, Case8, Case9, Case10, Case11, Case12, Case13, Case14, Case3_2, Case6_2 };

	const std::array<std::pair<int, vec3>, 256> map;

	Cases();
	~Cases();

	std::array<std::pair<int, vec3>, 256> mapReturn();
};