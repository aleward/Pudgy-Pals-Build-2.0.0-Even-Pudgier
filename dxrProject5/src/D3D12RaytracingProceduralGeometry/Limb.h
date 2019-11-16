#pragma once
#include "Creature.h"

class Limb
{
private:
	std::vector<XMFLOAT3> jointPos;
	std::vector<float> jointRadii;
	bool isLeg;
public:
	Limb(bool isLeg);
	~Limb();
	void generate(XMFLOAT3 startPos, float startRadius);
};

