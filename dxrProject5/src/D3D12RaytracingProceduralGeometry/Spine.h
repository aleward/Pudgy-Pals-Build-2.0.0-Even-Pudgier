#pragma once
#include "Creature.h"

class Spine
{
private:
	float maxSpineRadius = 0.4;
	float minSpineRadius = 0.1;

public:
	std::vector<XMFLOAT3> splinePoints;
	std::vector<XMFLOAT3> metaBallPos;
	std::vector<float> metaBallRadii;

	Spine();
	~Spine();
	void generate();
};

