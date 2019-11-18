#pragma once
#include "Appendages.h"

class Appendages
{
public:
    std::vector<float> appendageData; // In order, num appendages and positions of each	// Send to shader

	Appendages();
	~Appendages();
	void generate(std::vector<float> jointsPerLimb, std::vector<float> jointPos);
};

