#include "stdafx.h"
#include "Appendages.h"


Appendages::Appendages()
{
}


Appendages::~Appendages()
{
}

void Appendages::generate(std::vector<float> jointsPerLimb, std::vector<float> jointPos) {
	appendageData.push_back(jointsPerLimb.size());

	/*int start = 0;
	int first = 0;
	for (int i = 0; i < jointsPerLimb.size(); i++) {
		//need last position
		first = start + (3 * ((jointsPerLimb[i] - 1)));
		//console.log("first: " + first);
		appendageData.push_back(jointPos[first]);
		appendageData.push_back(jointPos[first + 1]);
		appendageData.push_back(jointPos[first + 2]);

		start = first + 3;

	}*/

}