#include "stdafx.h"
#include "Spine.h"


Spine::Spine()
{
}


Spine::~Spine()
{
}

void randomizeSpline(std::vector<XMFLOAT3> *splinePoints) {
	int numSplinePoints = 4;
	for (int i = 0; i < numSplinePoints; i++) {
		XMFLOAT3 newPoint = XMFLOAT3(0.5 * i + 0.3 * (std::rand() / RAND_MAX), 1. * (2 * (std::rand() / RAND_MAX) - 1), 0);
		splinePoints->push_back(newPoint);
	}
}

XMFLOAT3 getPosOnSpline(float t, std::vector<XMFLOAT3> splinePoints) {
	std::vector<XMFLOAT3> qs = std::vector<XMFLOAT3>();
	std::vector<XMFLOAT3> rs = std::vector<XMFLOAT3>();
	for (int i = 0; i < 3; i++) {
		XMFLOAT3 newPoint = XMFLOAT3(0, 0, 0);
		XMVECTOR sP1 = XMLoadFloat3(&splinePoints[i + 1]);
		XMVECTOR sP0 = XMLoadFloat3(&splinePoints[i]);
		XMVECTOR v = XMVectorSubtract(sP1, sP0);
		v = XMVectorScale(v, t);
		v = XMVectorAdd(v, sP0);
		XMStoreFloat3(&newPoint, v);
		qs.push_back(newPoint);
	}
	for (int i = 0; i < 2; i++) {
		XMFLOAT3 newPoint = XMFLOAT3(0, 0, 0);
		XMVECTOR q1 = XMLoadFloat3(&qs[i + 1]);
		XMVECTOR q0 = XMLoadFloat3(&qs[i]);
		XMVECTOR v = XMVectorSubtract(q1, q0);
		v = XMVectorScale(v, t);
		v = XMVectorAdd(v, q0);
		XMStoreFloat3(&newPoint, v);
		rs.push_back(newPoint);
	}
	XMFLOAT3 pos = XMFLOAT3(0, 0, 0);
	XMVECTOR r1 = XMLoadFloat3(&rs[1]);
	XMVECTOR r0 = XMLoadFloat3(&rs[0]);
	XMVECTOR v = XMVectorSubtract(r1, r0);
	v = XMVectorScale(v, t);
	v = XMVectorAdd(v, r0);
	XMStoreFloat3(&pos, v);
	return pos;
}

void Spine::generate() {
	splinePoints = std::vector<XMFLOAT3>();
	metaBallPos = std::vector<XMFLOAT3>();
	metaBallRadii = std::vector<float>();

	int numMetaBalls = 8;

	float radius = ((maxSpineRadius - minSpineRadius) * std::pow(std::rand() / RAND_MAX, 1.2) + minSpineRadius);  //pow to bias smaller radii
	for (int i = 0; i < numMetaBalls; i++) {
		// radius = 0.2;
		metaBallRadii.push_back(radius);
		radius += 0.1 * (2 * std::pow(std::rand() / RAND_MAX, 1.2) - 1); //pow to bias shrinking over length
		if (radius < minSpineRadius) radius = minSpineRadius;
		if (radius > maxSpineRadius) radius = maxSpineRadius;
	}

	randomizeSpline(&splinePoints);
	std::vector<XMFLOAT3> positions = std::vector<XMFLOAT3>();
	float t = 0;
	for (int j = 0; j < numMetaBalls; j++) {
		float radius = metaBallRadii[j] / 0.4;
		float prevRadius = 0;
		if (j > 0) prevRadius = metaBallRadii[j - 1];
		t += 1 / numMetaBalls;
		XMFLOAT3 pos = getPosOnSpline(t);
		XMFLOAT3 posNearby = getPosOnSpline(t + 0.05);
		XMVECTOR v0 = XMLoadFloat3(&pos);
		XMVECTOR v1 = XMLoadFloat3(&posNearby);
		XMVECTOR slope = XMVectorSubtract(v0, v1);
		slope = XMVector3Normalize(slope);
		slope = XMVectorScale(slope, -1 * metaBallRadii[j]);
		XMFLOAT3 slopeF3 = XMFLOAT3(0, 0, 0);
		XMStoreFloat3(&slopeF3, slope);
		pos.y += slopeF3.y;
		pos.x += slopeF3.x;

		if (pos.x == 0 && pos.y == 0 && pos.z == 0) pos.x += 0.01;
		positions.push_back(pos);
	}

	for (int j = 1; j < numMetaBalls; j++) {
		XMFLOAT3 diff = XMFLOAT3(0, 0, 0);
		XMVECTOR p0 = XMLoadFloat3(&positions[j - 1]);
		XMVECTOR p1 = XMLoadFloat3(&positions[j]);
		XMVECTOR diffV = XMVectorSubtract(p0, p1);
		float dist = XMVectorGetByIndex(XMVector3Length(diffV), 0);
		diffV = XMVector3Normalize(diffV);
		float f = max(dist - (metaBallRadii[j - 1] / 2 + metaBallRadii[j] / 2), 0);
		diffV = XMVectorScale(diffV, f);
		diffV = XMVectorAdd(p1, diffV);
		XMStoreFloat3(&diff, diffV);
		positions[j] = diff;
	}

	XMVECTOR averagePos = XMVectorSet(0, 0, 0, 0);
	for (int j = 0; j < numMetaBalls; j++) {
		XMVECTOR p = XMLoadFloat3(&positions[j]);
		averagePos = XMVectorAdd(averagePos, p);
	}
	XMVectorScale(averagePos, 1.f / numMetaBalls);

	float creatureHeight = std::rand() / RAND_MAX * 1.5 + 0.5;

	for (int j = 0; j < numMetaBalls; j++) {
		XMVECTOR p = XMLoadFloat3(&positions[j]);
		p = XMVectorSubtract(p, averagePos);
		p = XMVectorAdd(p, XMVectorSet(0, -creatureHeight, 0, 0));
		if (XMVectorGetByIndex(p, 1) > -0.2) XMVectorSetByIndex(p, -0.2f, 1);
		XMStoreFloat3(&positions[j], p);
	}

	for (int j = 0; j < numMetaBalls; j++) {
		metaBallPos.push_back(positions[j]);
	}
}
