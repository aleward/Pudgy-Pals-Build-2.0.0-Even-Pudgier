#include "stdafx.h"
#include "Head.h"


Head::Head()
{
}


Head::~Head()
{
}

void Head::generate(std::vector<float> spinePos, std::vector<float> spineRadii, int type) {
	headData = std::vector<float>();

	std::vector<float> firstPos;
	firstPos.push_back(spinePos[0]);
	firstPos.push_back(spinePos[1]);
	firstPos.push_back(spinePos[2]);

	float firstRadii = spineRadii[0];

	// new head position is the first metaball position plus the radius to the left (for now)
	headData.push_back(firstPos[0] - firstRadii);
	headData.push_back(firstPos[1]);
	headData.push_back(firstPos[2]);

	// Find radius of head
	float sum = 0;
	float avg = 0;

	for (int i = 0; i < spineRadii.size(); i++) {
		sum += spineRadii[i];
	}

	avg = sum / spineRadii.size();

	headData.push_back(avg);

	if (type == -1) {
		float rand = std::rand() / RAND_MAX;
		if (rand < .33) {
			headData.push_back(0.0);
		}
		else if (.33 < rand && rand < .66) {
			headData.push_back(1.0);
		}
		else {
			headData.push_back(2.0);
		}
	}
	else {
		headData.push_back(type);
	}
}