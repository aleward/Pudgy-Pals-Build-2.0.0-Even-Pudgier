#pragma once
#include "Creature.h"

class Head
{
private:
	std::vector<float> headData; // In order, contains head position (x,y,z), radius, then type
public:
	Head();
	~Head();
	void generate(std::vector<float> spinePos, std::vector<float> spineRadii, int type);
};

