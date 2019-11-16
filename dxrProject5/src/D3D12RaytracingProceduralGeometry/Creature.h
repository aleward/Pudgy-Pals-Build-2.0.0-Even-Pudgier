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
	Spine spine;
	std::vector<float> spineLocations;
	Head head;
	Limb *limbs;
	std::vector<float> jointLocations;
	std::vector<float> jointRadii;
	std::vector<float> limbLengths;
	Appendages appendages;
	int texture1;
	int texture2;
	XMFLOAT3 color1;
	XMFLOAT3 color2;
	XMFLOAT3 color3;
	XMFLOAT3 color4;

	Creature();
	~Creature();
};

