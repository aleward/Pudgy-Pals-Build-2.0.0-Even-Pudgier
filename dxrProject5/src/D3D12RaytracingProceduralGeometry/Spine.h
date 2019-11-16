#pragma once

using namespace DirectX;
class Spine
{
private:
	std::vector<XMFLOAT3> splinePoints;
	std::vector<XMFLOAT3> metaBallPos;
	std::vector<float> metaBallRadii;

	float maxSpineRadius = 0.4;
	float minSpineRadius = 0.1;

public:
	Spine();
	~Spine();
	void generate();
};

