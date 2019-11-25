#pragma once
#include "Spine.h"
#include "Head.h"
#include "Appendages.h"
#include "Limb.h"
#include <iostream>
#include <vector>
#include "stdafx.h"
#include "DXProceduralProject.h"

class Creature
{
public:
	Spine *spine;
	std::vector<float> spineLocations;	// Send to shader
	Head *head;	// Send headData to shader
	std::vector<Limb> limbs;
	std::vector<float> jointLocations;	// Send to shader
	std::vector<float> jointRadii;	// Send to shader
	std::vector<float> limbLengths;	// Send to shader
	Appendages *appendages;	// Send appendageData to shader
    std::vector<float> appenBools; // 0 for foot, 1 for hand
    std::vector<float> appenRads;
    std::vector<float> jointRots;
	int texture1;	// Send to shader
	int texture2;	// Send to shader
	XMFLOAT3 color1;	// Send to shader
	XMFLOAT3 color2;	// Send to shader
	XMFLOAT3 color3;	// Send to shader
	XMFLOAT3 color4;	// Send to shader

	Creature();
	~Creature();
	void generate(int numTextures, int numLimbSets, int headType);
};

