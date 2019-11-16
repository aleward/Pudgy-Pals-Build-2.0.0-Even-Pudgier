#include "stdafx.h"
#include "Limb.h"


Limb::Limb(bool isLeg)
{
	this->isLeg = isLeg;
	jointPos = std::vector<XMFLOAT3>();
	jointRadii = std::vector<float>();
}


Limb::~Limb()
{
}

void Limb::generate(XMFLOAT3 startPos, float startRadius) {
	int numJoints = std::floor((std::rand() / RAND_MAX) * 3. + 2.);

	float radius = ((std::rand() / RAND_MAX) * 2. - 1.) * 0.1 + startRadius;
	if (radius > 0.25) radius = 0.25;
	if (radius < 0.05) radius = 0.05;

	jointPos.push_back(startPos);
	jointRadii.push_back(radius);

	for (int i = 1; i < numJoints; i++) {
		float yaw = ((std::rand() / RAND_MAX)) * 3.1415926 * 0.8 + 3.1415926;
		float pitch = ((std::rand() / RAND_MAX) * 2 - 1) * 3.1415926 * 0.35;
		if (!isLeg) pitch -= 0.5;
		float r = jointRadii[i - 1] / 0.2 * ((std::rand() / RAND_MAX) * 0.5 + 0.2 + jointRadii[i - 1] / 2);

		float dx = r * std::sin(pitch) * std::cos(yaw);
		float dy = r * std::cos(pitch);
		float dz = r * std::sin(pitch) * std::sin(yaw);
		XMVECTOR newPosV = XMVectorAdd(XMLoadFloat3(&jointPos[i - 1]), XMVectorSet(dx, dy, dz, 0));
		XMFLOAT3 newPos = XMFLOAT3(0, 0, 0);
		XMStoreFloat3(&newPos, newPosV);
		if (isLeg && i + 1 >= numJoints) {
			newPos.y = 0.0;
		}
		if (newPos.z < 0.05) newPos.z = 0.1;
		if (newPos.y > -0.1) newPos.y = -0.1;
		jointPos.push_back(newPos);
		radius += ((std::rand() / RAND_MAX) * 2 - 1) * 0.05 - 0.1;
		if (radius > 0.25) radius = 0.25;
		if (radius < 0.05) radius = 0.05;
		jointRadii.push_back(radius);
	}
}
