// LOOKAT-1.9.1: RaytracingShaderHelper.hlsli
// Contains various helper functions used from the shader code in Raytracing.hlsl.

#ifndef SDFSHADERHELPER_H
#define SDFSHADERHELPER_H

// Procedural geometry resources
StructuredBuffer<PrimitiveInstancePerFrameBuffer> g_AABBPrimitiveAttributes : register(t3, space0); // transforms per procedural
ConstantBuffer<PrimitiveConstantBuffer> l_materialCB : register(b1); // material data per procedural
ConstantBuffer<PrimitiveInstanceConstantBuffer> l_aabbCB: register(b2); // other meta-data: type, instance indices

StructuredBuffer<HeadSpineInfoBuffer> g_headSpineBuffer: register(t4, space0);
StructuredBuffer<AppendageInfoBuffer> g_appenBuffer: register(t5, space0);
StructuredBuffer<LimbInfoBuffer> g_limbBuffer: register(t6, space0);
StructuredBuffer<RotationInfoBuffer> g_rotBuffer: register(t7, space0);

float3x3 rotateMatX(float angle) {
	float rad = radians(angle);
	return float3x3(
		float3(1.0, 0.0, 0.0),
		float3(0.0, cos(rad), -sin(rad)),
		float3(0.0, sin(rad), cos(rad))
		);
}

float3x3 rotateMatY(float angle) {
	float rad = radians(angle);
	return float3x3(
		float3(cos(rad), 0.0, sin(rad)),
		float3(0.0, 1.0, 0.0),
		float3(-sin(rad), 0.0, cos(rad))
		);
}

float3x3 rotateMatZ(float angle) {
	float rad = radians(angle);
	return float3x3(
		float3(cos(rad), -sin(rad), 0.0),
		float3(sin(rad), cos(rad), 0.0),
		float3(0.0, 0.0, 1.0)
		);
}

float3x3 scaleMat(float amt) {
	return float3x3(
		float3(amt, 0.0, 0.0),
		float3(0.0, amt, 0.0),
		float3(0.0, 0.0, amt)
		);
}

// polynomial smooth min
float smin(float a, float b, float k) {
	float h = clamp(0.5 + 0.5*(b - a) / k, 0.0, 1.0);
	return lerp(b, a, h) - k * h*(1.0 - h);
}

float sminExp(float a, float b, float k) {
	float res = exp(-k * a) + exp(-k * b);
	return -log(res) / k;
}

float sminPow(float a, float b, float k) {
	a = pow(a, k); b = pow(b, k);
	return pow((a*b) / (a + b), 1.0 / k);
}



float sphereSDF(float3 p, float r) {
	return length(p) - r;
}

float cubeSDF(float3 p, float r) {
	float3 d = abs(p) - float3(r, r, r);
	float insideDistance = min(max(d.x, max(d.y, d.z)), 0.0);
	float outsideDistance = length(max(d, 0.0));
	return insideDistance + outsideDistance;
}

float sdCappedCylinder(float3 p, float2 h) {
	float2 d = abs(float2(length(p.xz), p.y)) - h;
	return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

float udBox(float3 p, float3 b) {
	return length(max(abs(p) - b, 0.0));
}

float sdCone(float3 p, float2 c) {
	// c must be normalized
	float q = length(p.xy);
	return dot(c, float2(q, p.z));
}

float sdCappedCone(in float3 p, in float3 c) {
	float2 q = float2(length(p.xz), p.y);
	float2 v = float2(c.z*c.y / c.x, -c.z);
	float2 w = v - q;
	float2 vv = float2(dot(v, v), v.x*v.x);
	float2 qv = float2(dot(v, w), v.x*w.x);
	float2 d = max(qv, 0.0)*qv / vv;
	return sqrt(dot(w, w) - max(d.x, d.y)) * sign(max(q.y*v.x - q.x*v.y, w.y));
}

float udRoundBox(float3 p, float3 b, float r) {
	return length(max(abs(p) - b, 0.0)) - r;
}

float sdCapsule(float3 p, float3 a, float3 b, float r) {
	float3 pa = p - a, ba = b - a;
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
	return length(pa - ba * h) - r;
}

// a better capped cone function (like what)
float sdConeSection(in float3 p, in float h, in float r1, in float r2) {
	float d1 = -p.y - h;
	float q = p.y - h;
	float si = 0.5*(r1 - r2) / h;
	float d2 = max(sqrt(dot(p.xz, p.xz)*(1.0 - si * si)) + q * si - r2, q);
	return length(max(float2(d1, d2), 0.0)) + min(max(d1, d2), 0.);
}

//~~~~~HEAD SDFs~~~~~///
float bugHeadSDF(float3 p, float u_Head[HEAD_COUNT]) {
	p = mul(rotateMatY(-90.0), p);
	float base = sphereSDF(p, u_Head[3]);
	float eyes = min(sphereSDF(p + u_Head[3] * float3(0.55, -0.35, -.71), u_Head[3] * .2), sphereSDF(p + u_Head[3] * float3(-0.55, -0.35, -.71), u_Head[3] * .2));
	float mandibleBase = sdCappedCylinder(p + u_Head[3] * float3(0.0, 0.001, -.9), u_Head[3] * float2(1.2, 0.1));
	float mandibles = max(mandibleBase, -sphereSDF(p + u_Head[3] * float3(0.0, 0.0, -0.60), .7 * u_Head[3]));
	mandibles = max(mandibles, -sphereSDF(p + u_Head[3] * float3(0.0, 0.0, -1.70), .7 * u_Head[3]));
	float head = smin(min(base, eyes), mandibles, .05);
	return head;
}

float dinoHeadSDF(float3 p, float u_Head[HEAD_COUNT]) {
	p = mul(rotateMatY(-90.0), p);
	float base = sphereSDF(p, u_Head[3]);
	float topJaw = sphereSDF(p + u_Head[3] * float3(0.0, 0.3, -1.4), u_Head[3] * 1.08);
	topJaw = max(topJaw, -cubeSDF(p + u_Head[3] * float3(0.0, 1.4, -1.4), u_Head[3] * 1.2));
	float bottomJaw = sphereSDF(p + u_Head[3] * float3(0.0, 0.6, -1.0), u_Head[3] * .7);
	bottomJaw = max(bottomJaw, -cubeSDF(mul(rotateMatX(45.0), (p + u_Head[3] * float3(0.0, -.4, -1.7))), u_Head[3] * 1.1));
	float combine = smin(base, topJaw, .04);
	combine = smin(combine, bottomJaw, .08);

	float eyes = min(sphereSDF(p + u_Head[3] * float3(.9, 0.0, 0.0), u_Head[3] * .3), sphereSDF(p + u_Head[3] * float3(-0.9, 0.0, 0.0), u_Head[3] * .3));
	combine = min(combine, eyes);
	float brows = min(udBox(mul(rotateMatX(-20.0), (p + u_Head[3] * float3(.85, -0.35, 0.0))), u_Head[3] * float3(.3, .2, .5)),
		udBox(mul(rotateMatX(-20.0), (p + u_Head[3] * float3(-0.85, -0.35, 0.0))), u_Head[3] * float3(.3, .2, .5)));
	combine = min(combine, brows);

	float teeth = sdCappedCone(mul(rotateMatX(180.0), (p + u_Head[3] * float3(0.4, 0.7, -1.8))), u_Head[3] * float3(3.0, 1.0, 1.0));
	teeth = min(teeth, sdCappedCone(mul(rotateMatX(180.0), (p + u_Head[3] * float3(-0.4, 0.7, -1.8))), u_Head[3] * float3(3.0, 1.0, 1.0)));
	teeth = min(teeth, sdCappedCone(mul(rotateMatX(180.0), (p + u_Head[3] * float3(-0.4, 0.7, -1.3))), u_Head[3] * float3(2.7, 1.0, 1.0)));
	teeth = min(teeth, sdCappedCone(mul(rotateMatX(180.0), (p + u_Head[3] * float3(0.4, 0.7, -1.3))), u_Head[3] * float3(2.7, 1.0, 1.0)));
	combine = min(combine, teeth);
	return combine;
}

float trollHeadSDF(float3 p, float u_Head[HEAD_COUNT]) {
	p = mul(rotateMatY(-270.0), p);
	float base = sphereSDF(p, u_Head[3]);
	float bottomJaw = sphereSDF(p + u_Head[3] * float3(0.0, 0.3, .62), u_Head[3] * 1.08);
	bottomJaw = max(bottomJaw, -cubeSDF(p + u_Head[3] * float3(0.0, -1.0, .45), u_Head[3] * 1.3));
	float combine = smin(base, bottomJaw, .04);
	float teeth = sdCappedCone(p + u_Head[3] * float3(0.65, -0.7, 1.1), u_Head[3] * float3(4.0, 1.0, 1.0));
	teeth = min(teeth, sdCappedCone(p + u_Head[3] * float3(-0.65, -0.7, 1.1), u_Head[3] * float3(4.0, 1.0, 1.0)));
	teeth = min(teeth, sdCappedCone(p + u_Head[3] * float3(-0.25, -0.2, 1.4), u_Head[3] * float3(3.4, .5, .5)));
	teeth = min(teeth, sdCappedCone(p + u_Head[3] * float3(0.25, -0.2, 1.4), u_Head[3] * float3(3.4, .5, .5)));
	combine = min(combine, teeth);
	float eyes = min(sphereSDF(p + u_Head[3] * float3(.3, -0.5, 0.7), u_Head[3] * .2), sphereSDF(p + u_Head[3] * float3(-.3, -0.5, 0.7), u_Head[3] * .2));
	float monobrow = udBox(mul(rotateMatX(-20.0), (p + u_Head[3] * float3(0.0, -0.7, .65))), u_Head[3] * float3(.6, .2, .2));
	combine = min(min(combine, eyes), monobrow);
	return combine;
}

//~~~~HAND/FEET SDFs~~~~~//

// For now, size is based on head size, but later make it the average joint size
float clawFootSDF(float3 p, float size) {
	size = size * 2.0;
	float base = udRoundBox(p, size * float3(.6, .6, .3), .001);
	float fingees = sdConeSection(mul(rotateMatZ(-20.0), (p + size * float3(0.5, -0.9, 0.3))), size * 1.0, size * .3, size * .05);
	fingees = min(fingees, sdConeSection(mul(rotateMatZ(20.0), (p + size * float3(-0.5, -0.9, 0.3))), size * 1.0, size * .3, size * .05));
	fingees = min(fingees, sdConeSection(p + size * float3(0.0, -1.1, 0.3), size * 1.0, size * .3, size * .05));
	float combine = smin(base, fingees, .13); // final foot
	return combine;
}


float handSDF(float3 p, float size) {
	//float size = u_Head[3] / 1.5;
	//size = 1.0;
	float base = udRoundBox(p, size * float3(.6, .6, .2), .08);
	float fingee1 = sdConeSection(mul(rotateMatZ(-30.0), (p + size * float3(1.1, -0.7, 0.0))), size * 1.0, size * .5, size * .2);
	float fingee2 = sdConeSection((p + size * float3(0.45, -1.9, 0.0)), size * 1.0, size * .5, size * .2);
	float fingee3 = sdConeSection((p + size * float3(-0.45, -1.9, 0.0)), size * 1.0, size * .5, size * .2);
	fingee1 = min(fingee1, fingee2);
	fingee1 = min(fingee1, fingee3);
	float combine = smin(base, fingee1, .09);
	return combine;
}

float appendagesSDF(float3 p, float u_AppenData[APPEN_COUNT], float u_AppenRad[APPEN_COUNT], float u_AppenBools[APPEN_COUNT], XMMATRIX u_AppenRots[APPEN_COUNT]) {
	//AppendageInfoBuffer appenAttr = g_appenBuffer[l_aabbCB.instanceIndex];

	float all = MAX_DIST;
	float angle;
	//angle to slightly offset each foot
	float angle1 = -35.0;
	float angle2 = 35.0;

	int armsNow = 0;
	int numAppen = 0;

	for (int i = 0; i < int(u_AppenData[0]); i++) {
		int start = (i * 3) + 1;
		float3 offset = float3(u_AppenData[start], u_AppenData[start + 1], u_AppenData[start + 2]);

		if ((i % 2) == 0) {
			angle = angle1;
		}
		else {
			angle = angle2;
		}
		float foot;

		if (u_AppenBools[numAppen] == 1) {
			armsNow = 1;
		}

		if (armsNow == 0) {
			float3 rotP = mul(rotateMatZ(angle), mul(rotateMatY(90.0), mul(rotateMatZ(90.0), (p + offset))));
			foot = clawFootSDF(rotP, u_AppenRad[numAppen]);
		}
		else {
			float3 q = mul(float4((p + offset), 1.0), transpose(u_AppenRots[numAppen])).xyz;
			foot = handSDF(mul(rotateMatZ(180.0), q), u_AppenRad[numAppen]);
		}

		// //if(u_AppenBools[numAppen] != 0.0) {
		// if((numAppen % 2) == 0) {
		// 	foot = clawFootSDF((p + offset)*rotateMatZ(90.0) * rotateMatY(90.0) * rotateMatZ(angle));		
		// }
		// else {
		// 	foot = handSDF((p + offset) * rotateMatZ(135.0));
		// }
		// else if(u_AppenBools[2] == 0){			
		// 	
		// }

		numAppen = numAppen + 1;

		// if((p + offset).y == 100.0) { //should be 0, but does this work
		// 	foot = handSDF((p + offset) * rotateMatZ(135.0));
		// }
		// else {
		// 	foot = clawFootSDF((p + offset)*rotateMatZ(90.0) * rotateMatY(90.0) * rotateMatZ(angle));
		// }
		all = min(all, foot);
	}

	return all;
}

float armSDF(float3 p, float u_LimbLengths[LIMBLEN_COUNT], float u_JointLoc[JOINT_LOC_COUNT], float u_JointRad[JOINT_RAD_COUNT], XMMATRIX u_Rotations[ROT_COUNT]) {

	int countSegs = 0;

	float allLimbs = MAX_DIST;
	int incr = 0;
	int numLimbs = 0;
	int jointNum = 0;
	for (int i = 0; i < LIMBLEN_COUNT; i++) {
		jointNum += u_LimbLengths[i];
	}

	//this is for each limb
	for (int j = 0; j < (jointNum * 3); j = j + incr) {
		numLimbs++;

		int count = 0;

		// NEED joint number to do the below operations...

		//count is number of joints in this limb
		count = int(u_LimbLengths[numLimbs - 1]);

		float arm = MAX_DIST;
		// all joint positions for a LIM (jointNum * 3)
		for (int i = j; i < (j + (count * 3)); i = i + 3) {
			float3 pTemp = p + float3(u_JointLoc[i], u_JointLoc[i + 1], u_JointLoc[i + 2]);
			arm = min(arm, sphereSDF(pTemp, u_JointRad[i / 3]));

		}

		// for 3 * (jointNum(per limb) - 1), each joint until last one
		float segments = MAX_DIST;

		for (i = j; i < (j + ((count - 1) * 3)); i = i + 3) {
			float3 point0 = float3(u_JointLoc[i], u_JointLoc[i + 1], u_JointLoc[i + 2]);
			float3 point1 = float3(u_JointLoc[i + 3], u_JointLoc[i + 4], u_JointLoc[i + 5]);
			float3 midpoint = float3((point0.x + point1.x) / 2.0, (point0.y + point1.y) / 2.0, (point0.z + point1.z) / 2.0);
			float len = distance(point0, point1);

			float3 dir = point1 - point0; //dir is correct



			float4x4 outMat4 = transpose(u_Rotations[countSegs]);
			countSegs++;

			float3 q = mul(float4((p + midpoint), 1.0), outMat4).xyz;



			float part = sdConeSection(q, len / 2.0, u_JointRad[(i + 3) / 3], u_JointRad[i / 3]);
			segments = min(segments, part);
		}

		float combine = smin(arm, segments, .2); // this is one arm
		allLimbs = min(allLimbs, combine); //merge with all other limbs

		incr = count * 3;

	}

	return allLimbs;
}


float spineSDF(float3 p, float u_SpineLoc[SPINE_LOC_COUNT], float u_SpineRad[SPINE_RAD_COUNT]) {
	float spine = MAX_DIST;
	for (int i = 0; i < SPINE_LOC_COUNT; i += 3) {
		if (u_SpineLoc[i] == 0. && u_SpineLoc[i + 1] == 0. && u_SpineLoc[i + 2] == 0.) continue;
		float3 pTemp = p + float3(u_SpineLoc[i], u_SpineLoc[i + 1], u_SpineLoc[i + 2]);
		spine = smin(spine, sphereSDF(pTemp, u_SpineRad[i / 3]), 0.06);
	}
	return spine;
}

float boxTest(float3 p, float3 b) {
	float3 d = abs(p) - b;
	return min(max(d[0], max(d[1], d[2])), 0.0) + length(max(d, 0.0));
}

// OVERALL SCENE SDF -- rotates about z-axis (turn-table style)
float sceneSDF(float3 p) {
	HeadSpineInfoBuffer headSpineAttr = g_headSpineBuffer[l_aabbCB.instanceIndex];
	AppendageInfoBuffer appenAttr = g_appenBuffer[l_aabbCB.instanceIndex];
	//LimbInfoBuffer limbAttr = g_limbBuffer[l_aabbCB.instanceIndex];
	//RotationInfoBuffer rotAttr = g_rotBuffer[l_aabbCB.instanceIndex];

	float u_Head[HEAD_COUNT] = headSpineAttr.headData;

	p += float3(0, 0, 0);
	// p = p * rotateMatY(u_Time) ; // rotates creature
	//return sphereSDF(p, 1.0);
	//return dinoHeadSDF(p + float3(u_Head[0], u_Head[1], u_Head[2]), u_Head);
	float headSDF = 0;
	if (u_Head[4] == 0) {
		headSDF = bugHeadSDF(p + float3(u_Head[0], u_Head[1], u_Head[2]), u_Head);
	}
	else if (u_Head[4] == 1) {
		headSDF = dinoHeadSDF(p + float3(u_Head[0], u_Head[1], u_Head[2]), u_Head);
	}
	else if (u_Head[4] == 2) {
		headSDF = trollHeadSDF(p + float3(u_Head[0], u_Head[1], u_Head[2]), u_Head);
	}
	float headSpine = smin(spineSDF(p, headSpineAttr.spineLocData, headSpineAttr.spineRadData), headSDF, .1);
	//float plusArm = smin(appendagesSDF(p, appenAttr.appenData, appenAttr.appenRads, appenAttr.appenBools, appenAttr.appenRots), headSpine, .1);
	
	// Marching cubes size test
	//return min(headSpine, boxTest(p + float3(-0.4, 0.2, 0.0), float3(1.6, 0.7, 0.5)));
	
	return headSpine;//smin(smin(armSDF(p, limbAttr.limbLengths, limbAttr.jointLocData, limbAttr.jointRadData, rotAttr.rotations),
		//appendagesSDF(p, appenAttr.appenData, appenAttr.appenRads, appenAttr.appenBools, appenAttr.appenRots), .2), dist, .1);

	//return min(handSDF(p+ float3(-1.0,0.0,0.0), u_AppenRad[0]), clawFootSDF(p + float3(1.0,0.0,0.0), u_AppenRad[0]));
}

#endif // SDFSHADERHELPER_H