#pragma once

class Head
{
public:
    std::vector<float> headData; // In order, contains head position (x,y,z), radius, then type	// Send to shader

	Head();
	~Head();
	void generate(std::vector<float> spinePos, std::vector<float> spineRadii, int type);
};

