#pragma once
#include "Appendages.h"

class Appendages
{
private:
	std::vector<float> appendageData; // In order, num appendages and positions of each	// Send to shader
public:
	Appendages();
	~Appendages();
	void generate(std::vector<float> jointsPerLimb, std::vector<float> jointPos);
};

