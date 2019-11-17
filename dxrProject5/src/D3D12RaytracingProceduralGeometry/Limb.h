#pragma once
#include "Creature.h"

class Limb
{
private:

public:
	std::vector<XMFLOAT3> jointPos;
	std::vector<float> jointRadii;
	bool isLeg;

	Limb(bool isLeg);
	~Limb();
	void generate(XMFLOAT3 startPos, float startRadius);
};

