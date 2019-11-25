#include "stdafx.h"
#include "Creature.h"
#include <random>


Creature::Creature()
{
}


Creature::~Creature()
{
}

void Creature::generate(int numTextures, int numLimbSets, int headType) {
	texture1 = std::floor((std::rand() / RAND_MAX) * numTextures);
	texture2 = std::floor((std::rand() / RAND_MAX) * numTextures);
	color1 = XMFLOAT3((std::rand() / RAND_MAX), (std::rand() / RAND_MAX), (std::rand() / RAND_MAX));
	color2 = XMFLOAT3((std::rand() / RAND_MAX), (std::rand() / RAND_MAX), (std::rand() / RAND_MAX));
	color3 = XMFLOAT3((std::rand() / RAND_MAX), (std::rand() / RAND_MAX), (std::rand() / RAND_MAX));
	color4 = XMFLOAT3((std::rand() / RAND_MAX), (std::rand() / RAND_MAX), (std::rand() / RAND_MAX));

	spine = new Spine();
	head = new Head();

	appendages = new Appendages();

	spine->generate();
	for (int i = 0; i < spine->metaBallPos.size(); i++) {
		XMFLOAT3 spinePos = spine->metaBallPos[i];
		spineLocations.push_back(spinePos.x);
		spineLocations.push_back(spinePos.y);
		spineLocations.push_back(spinePos.z);
	}

	// Head takes information from the spine that is made previously
	head->generate(spineLocations, spine->metaBallRadii, headType);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distrib(0, 1);

	//Leg generation and parsing
	int numLimbs;
	if (numLimbSets == 0) {
		numLimbs = std::pow(distrib(gen), 1.7) * 2 + 1;
	}
	else {
		numLimbs = numLimbSets;
	}

	bool generatingArms = false;
	for (int i = 0; i < numLimbs; i++) {
		Limb limb1 = Limb(!generatingArms);
		float offset = (distrib(gen) * spine->metaBallPos.size() / numLimbs) * 0.8;
		int spineIndex = spine->metaBallPos.size() - 1 - max(min(std::floor(i * spine->metaBallPos.size() / numLimbs + offset), spine->metaBallPos.size() - 1), 0);
		XMFLOAT3 startPos = spine->metaBallPos[spineIndex];
		startPos.z += spine->metaBallRadii[spineIndex] / 2.0 + 0.1;
		limb1.generate(startPos, spine->metaBallRadii[spineIndex] * 0.7);

		Limb limb2 = Limb(true);
		for (int j = 0; j < limb1.jointPos.size(); j++) {
			XMFLOAT3 mirrorJointPos = XMFLOAT3(limb1.jointPos[j].x, limb1.jointPos[j].y, -limb1.jointPos[j].z);
			limb2.jointPos.push_back(mirrorJointPos);
			limb2.jointRadii.push_back(limb1.jointRadii[j]);
		}

		limbs.push_back(limb1);
		limbs.push_back(limb2);

		if (distrib(gen) > 0.6) generatingArms = true;
	}

	for (int i = 0; i < limbs.size(); i++) {

		Limb leg = limbs[i];
		limbLengths.push_back(leg.jointPos.size());
		for (int j = 0; j < leg.jointPos.size(); j++) {
			XMFLOAT3 joint = leg.jointPos[j];
			jointLocations.push_back(joint.x);
			jointLocations.push_back(joint.y);
			jointLocations.push_back(joint.z);
			jointRadii.push_back(leg.jointRadii[j]);
		}
	}

    appendages->generate(limbLengths, jointLocations);

    // set appendage bools
    bool armsNow = false;
    for (int i = 0; i < limbs.size(); i++) {
        if (armsNow) {
            appenBools.push_back(1);
            continue;
        }
        if (limbs[i].isLeg) {
            appenBools.push_back(0);
        }
        else {
            armsNow = true;
            appenBools.push_back(1);
        }
    }


    //go through limbs and set joint and appendage rotations, appendage radii
    int start = 0;
    //need to do separately for each limb
    for (int k = 0; k < limbLengths.size(); k++) {
        //for as many joints in each limb
        for (int l = 0; l < limbLengths[k] - 1; l++) {

            XMVECTOR a = XMVectorSet(0.0, 1.0, 0.0, 0.0);
            XMVECTOR b = XMVectorSet(jointLocations[start + 3] - jointLocations[start],
                jointLocations[start + 4] - jointLocations[start + 1],
                jointLocations[start + 5] - jointLocations[start + 2], 0.0);
            b = XMVector3Normalize(b);

            /*XMVECTOR q = XMVector3Cross(a, b);
            float w = pow(XMVectorGetX(XMVector3Length(a)), 2) * pow(XMVectorGetX(XMVector3Length(b)), 2) + XMVectorGetX(XMVector3Dot(a, b));
            q = XMVectorSetW(q, sqrt(w));
            XMMATRIX m4 = XMMatrixRotationQuaternion(q);*/

            float angle = acos(XMVectorGetX(XMVector3Dot(a, b)));
            XMVECTOR axis = XMVector3Normalize(XMVector3Cross(a, b));

            jointRots.push_back(angle);
            jointRots.push_back(XMVectorGetX(axis));
            jointRots.push_back(XMVectorGetY(axis));
            jointRots.push_back(XMVectorGetZ(axis));
            if (l == limbLengths[k] - 2) {
                jointRots.push_back(angle);
                jointRots.push_back(XMVectorGetX(axis));
                jointRots.push_back(XMVectorGetY(axis));
                jointRots.push_back(XMVectorGetZ(axis));
                appenRads.push_back(jointRadii[start / 3]);
            }

            start = start + 3;

        }
        start = start + 3;

    }
}
