#include "Creature.h"


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
	spineLocations = std::vector<float>();
	head = new Head();
	limbs = std::vector<Limb>();

	jointLocations = std::vector<float>();
	jointRadii = std::vector<float>();
	limbLengths = std::vector<float>();


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

	appendages->generate(limbLengths, jointLocations);

	//Leg generation and parsing
	int numLimbs;
	if (numLimbSets == 0) {
		numLimbs = std::pow((std::rand() / RAND_MAX), 1.7) * 2 + 1;
	}
	else {
		numLimbs = numLimbSets;
	}

	bool generatingArms = false;
	for (int i = 0; i < numLimbs; i++) {
		Limb limb1 = Limb(!generatingArms);
		float offset = ((std::rand() / RAND_MAX) * spine->metaBallPos.size() / numLimbs) * 0.8;
		int spineIndex = spine->metaBallPos.size() - 1 - max(min(std::floor(i * spine->metaBallPos.size() / numLimbs + offset), spine->metaBallPos.size() - 1), 0);
		XMFLOAT3 startPos = spine->metaBallPos[spineIndex];
		startPos.z += spine->metaBallRadii[spineIndex] / 2 + 0.1;
		limb1.generate(startPos, spine->metaBallRadii[spineIndex] * 0.7);

		Limb limb2 = Limb(true);
		for (int j = 0; j < limb1.jointPos.size(); j++) {
			XMFLOAT3 mirrorJointPos = XMFLOAT3(limb1.jointPos[j].x, limb1.jointPos[j].y, -limb1.jointPos[j].z);
			limb2.jointPos.push_back(mirrorJointPos);
			limb2.jointRadii.push_back(limb1.jointRadii[j]);
		}

		limbs.push_back(limb1);
		limbs.push_back(limb2);

		if ((std::rand() / RAND_MAX) > 0.6) generatingArms = true;
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
}
