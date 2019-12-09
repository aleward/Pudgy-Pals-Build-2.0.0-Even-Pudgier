#pragma once

#include "stdafx.h"
#include <cmath>
#include "RaytracingHlslCompat.h"
#include "DXR-Structs.h"

/*class vec4 {
private:
	///The set of floats representing the coordinates of the vector
	float data[4];
public:
	/// Constructors
	vec4() : data{ 0,0,0,0 } {}
	vec4(float x, float y, float z, float w) : data{ x,y,z,w } {}
	vec4(const vec4 &v2) : vec4(v2[0], v2[1], v2[2], v2[3]) {}
	/// Getters/Setters
	float operator[](unsigned int index) const {
		if (index > 3) { std::out_of_range("Index must be between 0 and 3"); }
		return data[index];
	}
	float& operator[](unsigned int index) {
		if (index > 3) { std::out_of_range("Index must be between 0 and 3"); }
		return data[index];
	}
	/// Operator Functions 
	vec4& operator=(const vec4 &v2) {
		data[0] = v2[0];
		data[1] = v2[1];
		data[2] = v2[2];
		data[3] = v2[3];
		return *this;
	}
	/// Arithmetic:
	vec4& operator+=(const vec4 &v2) {
		data[0] = data[0] + v2[0];
		data[1] = data[1] + v2[1];
		data[2] = data[2] + v2[2];
		data[3] = data[3] + v2[3];
		return *this;
	}
	vec4& operator-=(const vec4 &v2) {
		data[0] = data[0] - v2[0];
		data[1] = data[1] - v2[1];
		data[2] = data[2] - v2[2];
		data[3] = data[3] - v2[3];
		return *this;
	}
	vec4& operator*=(float c) {
		data[0] = c * data[0];
		data[1] = c * data[1];
		data[2] = c * data[2];
		data[3] = c * data[3];
		return *this;
	}
	vec4& operator/=(float c) {
		data[0] = data[0] / c;
		data[1] = data[1] / c;
		data[2] = data[2] / c;
		data[3] = data[3] / c;
		return *this;
	}
	vec4  operator+(const vec4 &v2) const {
		return vec4(data[0] + v2[0], data[1] + v2[1], data[2] + v2[2], data[3] + v2[3]);
	}
	vec4  operator-(const vec4 &v2) const {
		return vec4(data[0] - v2[0], data[1] - v2[1], data[2] - v2[2], data[3] - v2[3]);
	}
	vec4  operator*(float c) const {
		return vec4(data[0] * c, data[1] * c, data[2] * c, data[3] * c);
	}
	vec4  operator/(float c) const {
		return vec4(data[0] / c, data[1] / c, data[2] / c, data[3] / c);
	}
};
/// Dot Product
float dot(const vec4 &v1, const vec4 &v2) {
	return (v1[0] * v2[0]) + (v1[1] * v2[1]) + (v1[2] * v2[2]) + (v1[3] * v2[3]);
}
/// Returns the geometric length of the input vector
float length(const vec4 &v) {
	return sqrt(pow(v[0], 2) + pow(v[1], 2) + pow(v[2], 2) + pow(v[3], 2));
}
/// Scalar Multiplication (c * v)
vec4 operator*(float c, const vec4 &v) {
	return vec4(v[0] * c, v[1] * c, v[2] * c, v[3] * c);
}*/



// VEC3

class vec3 {
private:
	float data[3];
public:
	/// Constructors
	vec3();
	vec3(float x, float y, float z);
	vec3(const vec3 &v2);

	/// Getters/Setters
	float operator[](unsigned int index) const;
	float& operator[](unsigned int index);

	/// Operator Functions 
	vec3& operator=(const vec3 &v2);

	/// Arithmetic:
	vec3& operator+=(const vec3 &v2);
	vec3& operator-=(const vec3 &v2);
	vec3& operator*=(float c);
	vec3& operator/=(float c);

	vec3 operator+(const vec3 &v2) const;
	vec3 operator-(const vec3 &v2) const;
	vec3 operator*(float c) const;
	vec3 operator/(float c) const;
};

vec3 abs(const vec3 &v);
vec3 maxV(const vec3 &v, float m);
/// Dot Product
float dot(const vec3 &v1, const vec3 &v2);
/// Cross Product
vec3 cross(const vec3 &v1, const vec3 &v2);
/// Returns the geometric length of the input vector
float length(const vec3 &v);
vec3 normalize(const vec3 &v);
/// Scalar Multiplication (c * v)
vec3 operator*(float c, const vec3 &v);
/// Print statements
std::string printVec(const vec3 &v);


// VEC2
class vec2 {
private:
	float data[2];
public:
	/// Constructors
	vec2();
	vec2(float x, float y);
	vec2(const vec2 &v2);

	/// Getters/Setters
	float operator[](unsigned int index) const;
	float& operator[](unsigned int index);

	/// Operator Functions 
	vec2& operator=(const vec2 &v2);

	/// Arithmetic:
	vec2& operator+=(const vec2 &v2);
	vec2& operator-=(const vec2 &v2);
	vec2& operator*=(float c);
	vec2& operator/=(float c);

	vec2  operator+(const vec2 &v2) const;
	vec2  operator-(const vec2 &v2) const;
	vec2  operator*(float c) const;
	vec2  operator/(float c) const;
	vec2  operator*(vec2 v) const;
	vec2  operator/(vec2 v) const;
};

vec2 abs(const vec2 &v);
vec2 maxV(const vec2 &v, float m);
/// Dot Product
float dot(const vec2 &v1, const vec2 &v2);
/// Returns the geometric length of the input vector
float length(const vec2 &v);
/// Scalar Multiplication (c * v)
vec2 operator*(float c, const vec2 &v);


//************************

// MAT3
class mat3 {
private:
	vec3 data[3];
public:
	/// Constructors
	mat3(float diag);
	mat3();
	mat3(const vec3 &col0, const vec3 &col1, const vec3 &col2);
	mat3(const mat3 &m2);

	/// Getters
	vec3  operator[](unsigned int index) const;

	/// Returns a reference to the column at the index
	vec3& operator[](unsigned int index);

	/// Static Initializers   TAKE IN RADIANS
	static mat3 rotateX(float angle);
	static mat3 rotateY(float angle);
	static mat3 rotateZ(float angle);
	static mat3 scale(float x, float y, float z);
	static mat3 identity();

	/// Operator Functions
	mat3& operator=(const mat3 &m2);

	/// Matrix multiplication (m1 * m2)
	mat3 operator*(const mat3 &m2) const;

	/// Matrix/vector multiplication (m * v)
	/// Assume v is a column vector (ie. a 4x1 matrix)
	vec3 operator*(const vec3 &v) const;
};

// Matrix Operations
/// Returns a row of the input matrix
vec3 row(const mat3 &m, unsigned int index);

mat3 transpose(const mat3 &m);

/// Vector/matrix multiplication (v * m)
vec3 operator*(const vec3 &v, const mat3 &m);



// MAT2
class mat2 {
private:
	vec2 data[2];
public:
	/// Constructors
	mat2(float diag);
	mat2();
	mat2(const vec2 &col0, const vec2 &col1);
	mat2(const mat2 &m2);

	/// Getters
	/// Returns the values of the column at the index
	vec2  operator[](unsigned int index) const;

	/// Returns a reference to the column at the index
	vec2& operator[](unsigned int index);

	/// Static Initializers   TAKE IN RADIANS
	static mat2 scale(float x, float y, float z);
	static mat2 identity();

	/// Operator Functions
	mat2& operator=(const mat2 &m2);

	/// Matrix multiplication (m1 * m2)
	mat2 operator*(const mat2 &m2) const;

	/// Matrix/vector multiplication (m * v)
	/// Assume v is a column vector (ie. a 4x1 matrix)
	vec2 operator*(const vec2 &v) const;
};

// Matrix Operations
/// Returns a row of the input matrix
vec2 row(const mat2 &m, unsigned int index);

mat2 transpose(const mat2 &m);

/// Vector/matrix multiplication (v * m)
vec2 operator*(const vec2 &v, const mat2 &m);


//***************************************************************************************

// SDF TIME

class SDF {
public:
	StructuredBuffer<HeadSpineInfoBuffer> g_headSpineBuffer;
	StructuredBuffer<AppendageInfoBuffer> g_appenBuffer;
	StructuredBuffer<LimbInfoBuffer> g_limbBuffer;
	StructuredBuffer<RotationInfoBuffer> g_rotBuffer;
	
	SDF() {}
	SDF(StructuredBuffer<HeadSpineInfoBuffer> m_headSpineBuffer,
		StructuredBuffer<AppendageInfoBuffer> m_appenBuffer,
		StructuredBuffer<LimbInfoBuffer> m_limbBuffer,
		StructuredBuffer<RotationInfoBuffer> m_rotBuffer) :
		g_headSpineBuffer(m_headSpineBuffer), g_appenBuffer(m_appenBuffer),
		g_limbBuffer(m_limbBuffer), g_rotBuffer(m_rotBuffer)
	{}
	~SDF() {}

	void setHeadBuffer(StructuredBuffer<HeadSpineInfoBuffer> m_headSpineBuffer) {
		g_headSpineBuffer = m_headSpineBuffer;
	}
	void setAppenBuffer(StructuredBuffer<AppendageInfoBuffer> m_appenBuffer) {
		g_appenBuffer = m_appenBuffer;
	}
	void setLimbBuffer(StructuredBuffer<LimbInfoBuffer> m_limbBuffer) {
		g_limbBuffer = m_limbBuffer;
	}
	void setLimbBuffer(StructuredBuffer<RotationInfoBuffer> m_rotBuffer) {
		g_rotBuffer = m_rotBuffer;
	}
	
	
	
	float radians(float angle) {
		return angle * 3.14159265358979323846 / 180.0;
	}
	
	vec3 rotateX(vec3 p, float angle) {
		float rad = radians(angle);
		float co = cos(rad);
		float si = sin(rad);
		mat2 mat = mat2(vec2(co, si), vec2(-si, co));
		vec2 p_yz = mat * vec2(p[1], p[2]);
		return vec3(p[0], p_yz[0], p_yz[1]);
	}
	
	vec3 rotateY(vec3 p, float angle) {
		float rad = radians(angle);
		float co = cos(rad);
		float si = sin(rad);
		mat2 mat = mat2(vec2(co, -si), vec2(si, co));
		vec2 p_xz = mat * vec2(p[0], p[2]);
		return vec3(p_xz[0], p[1], p_xz[1]);
	}
	
	vec3 rotateZ(vec3 p, float angle) {
		float rad = radians(angle);
		float co = cos(rad);
		float si = sin(rad);
		mat2 mat = mat2(vec2(co, si), vec2(-si, co));
		vec2 p_xy = mat * vec2(p[0], p[1]);
		return vec3(p_xy[0], p_xy[1], p[2]);
	}
	
	vec3 rotateInverseAxisAngle(float angle, float x, float y, float z, vec3 p)
	{
		float c, s;
		//sincos(angle, s, c);
		c = cos(angle);
		s = sin(angle);
	
		float t = 1 - c;
	
		mat3 m = mat3(
			vec3(t * x * x + c, t * x * y - s * z, t * x * z + s * y),
			vec3(t * x * y + s * z, t * y * y + c, t * y * z - s * x),
			vec3(t * x * z - s * y, t * y * z + s * x, t * z * z + c));
	
		return p * transpose(m);//mul(transpose(m), p);
	}
	
	mat3 scaleMat(float amt) {
		return mat3(
			vec3(amt, 0.0, 0.0),
			vec3(0.0, amt, 0.0),
			vec3(0.0, 0.0, amt)
		);
	}
	
	
	float clamp(float c, float mi, float ma) {
		return max(min(c, ma), mi);
	}
	
	float lerp(float first, float second, float t) {
		return (1.0 - t) * first + t * second;
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
	
	float sphereSDF(vec3 p, float r) {
		return length(p) - r;
	}
	
	float cubeSDF(vec3 p, float r) {
		vec3 d = abs(p) - vec3(r, r, r);
		float insideDistance = min(max(d[0], max(d[1], d[2])), 0.0);
		float outsideDistance = length(maxV(d, 0.0));
		return insideDistance + outsideDistance;
	}
	
	float sdCappedCylinder(vec3 p, vec2 h) {
		vec2 d = abs(vec2(length(vec2(p[0], p[2])), p[1])) - h;
		return min(max(d[0], d[1]), 0.0) + length(maxV(d, 0.0));
	}
	
	float udBox(vec3 p, vec3 b) {
		return length(maxV(abs(p) - b, 0.0));
	}
	
	float sdCone(vec3 p, vec2 c) {
		// c must be normalized
		float q = length(vec2(p[0], p[1]));
		return dot(c, vec2(q, p[2]));
	}
	
	float sign(float c) {
		if (c < 0) {
			return -1;
		}
		else if (c == 0) {
			return 0;
		}
		else {
			return 1;
		}
	}
	
	float sdCappedCone(vec3 p, vec3 c) {
		vec2 q = vec2(length(vec2(p[0], p[2])), p[1]);
		vec2 v = vec2(c[2]*c[1] / c[0], -c[2]);
		vec2 w = v - q;
		vec2 vv = vec2(dot(v, v), v[0]*v[0]);
		vec2 qv = vec2(dot(v, w), v[0]*w[0]);
		vec2 d = maxV(qv, 0.0)*qv / vv;
		return sqrt(dot(w, w) - max(d[0], d[1])) * sign(max(q[1]*v[0] - q[0]*v[1], w[1]));
	}
	
	float udRoundBox(vec3 p, vec3 b, float r) {
		return length(maxV(abs(p) - b, 0.0)) - r;
	}
	
	float sdCapsule(vec3 p, vec3 a, vec3 b, float r) {
		vec3 pa = p - a, ba = b - a;
		float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
		return length(pa - ba * h) - r;
	}
	
	// a better capped cone function (like what)
	float sdConeSection(vec3 p, float h, float r1, float r2) {
		float d1 = -p[1] - h;
		float q = p[1] - h;
		float si = 0.5*(r1 - r2) / h;
		vec2 p_xz = vec2(p[0], p[2]);
		float d2 = max(sqrt(dot(p_xz, p_xz)*(1.0 - si * si)) + q * si - r2, q);
		return length(maxV(vec2(d1, d2), 0.0)) + min(max(d1, d2), 0.);
	}
	
	//~~~~~HEAD SDFs~~~~~///
	float bugHeadSDF(vec3 p, float u_Head[HEAD_COUNT]) {
		p = p + vec3(u_Head[0], u_Head[1], u_Head[2]);
		p = rotateY(p, -90.0);
		float base = sphereSDF(p, u_Head[3]);
		float eyes = min(sphereSDF(p + u_Head[3] * vec3(0.55, -0.35, -.71), u_Head[3] * .2), sphereSDF(p + u_Head[3] * vec3(-0.55, -0.35, -.71), u_Head[3] * .2));
		float mandibleBase = sdCappedCylinder(p + u_Head[3] * vec3(0.0, 0.001, -.9), u_Head[3] * vec2(1.2, 0.1));
		float mandibles = max(mandibleBase, -sphereSDF(p + u_Head[3] * vec3(0.0, 0.0, -0.60), .7 * u_Head[3]));
		mandibles = max(mandibles, -sphereSDF(p + u_Head[3] * vec3(0.0, 0.0, -1.70), .7 * u_Head[3]));
		float head = smin(min(base, eyes), mandibles, .05);
		return head;
	}
	
	float dinoHeadSDF(vec3 p, float u_Head[HEAD_COUNT]) {
		p = p + vec3(u_Head[0], u_Head[1], u_Head[2]);
		p = rotateY(p, -90.0);
		float base = sphereSDF(p, u_Head[3]);
		float topJaw = sphereSDF(p + u_Head[3] * vec3(0.0, 0.3, -1.4), u_Head[3] * 1.08);
		topJaw = max(topJaw, -cubeSDF(p + u_Head[3] * vec3(0.0, 1.4, -1.4), u_Head[3] * 1.2));
		float bottomJaw = sphereSDF(p + u_Head[3] * vec3(0.0, 0.6, -1.0), u_Head[3] * .7);
		bottomJaw = max(bottomJaw, -cubeSDF(rotateX((p + u_Head[3] * vec3(0.0, -.4, -1.7)), 45.0), u_Head[3] * 1.1));
		float combine = smin(base, topJaw, .04);
		combine = smin(combine, bottomJaw, .08);
	
		float eyes = min(sphereSDF(p + u_Head[3] * vec3(.9, 0.0, 0.0), u_Head[3] * .3), sphereSDF(p + u_Head[3] * vec3(-0.9, 0.0, 0.0), u_Head[3] * .3));
		combine = min(combine, eyes);
		float brows = min(udBox(rotateX((p + u_Head[3] * vec3(.85, -0.35, 0.0)), -20.0), u_Head[3] * vec3(.3, .2, .5)),
			udBox(rotateX((p + u_Head[3] * vec3(-0.85, -0.35, 0.0)), -20.0), u_Head[3] * vec3(.3, .2, .5)));
		combine = min(combine, brows);
	
		float teeth = sdCappedCone(rotateX((p + u_Head[3] * vec3(0.4, 0.7, -1.8)), 180.0), u_Head[3] * vec3(3.0, 1.0, 1.0));
		teeth = min(teeth, sdCappedCone(rotateX((p + u_Head[3] * vec3(-0.4, 0.7, -1.8)), 180.0), u_Head[3] * vec3(3.0, 1.0, 1.0)));
		teeth = min(teeth, sdCappedCone(rotateX((p + u_Head[3] * vec3(-0.4, 0.7, -1.3)), 180.0), u_Head[3] * vec3(2.7, 1.0, 1.0)));
		teeth = min(teeth, sdCappedCone(rotateX((p + u_Head[3] * vec3(0.4, 0.7, -1.3)), 180.0), u_Head[3] * vec3(2.7, 1.0, 1.0)));
		combine = min(combine, teeth);
		return combine;
	}
	
	float trollHeadSDF(vec3 p, float u_Head[HEAD_COUNT]) {
		p = p + vec3(u_Head[0], u_Head[1], u_Head[2]);
		p = rotateY(p, -270.0);
		float base = sphereSDF(p, u_Head[3]);
		float bottomJaw = sphereSDF(p + u_Head[3] * vec3(0.0, 0.3, .62), u_Head[3] * 1.08);
		bottomJaw = max(bottomJaw, -cubeSDF(p + u_Head[3] * vec3(0.0, -1.0, .45), u_Head[3] * 1.3));
		float combine = smin(base, bottomJaw, .04);
		float teeth = sdCappedCone(p + u_Head[3] * vec3(0.65, -0.7, 1.1), u_Head[3] * vec3(4.0, 1.0, 1.0));
		teeth = min(teeth, sdCappedCone(p + u_Head[3] * vec3(-0.65, -0.7, 1.1), u_Head[3] * vec3(4.0, 1.0, 1.0)));
		teeth = min(teeth, sdCappedCone(p + u_Head[3] * vec3(-0.25, -0.2, 1.4), u_Head[3] * vec3(3.4, .5, .5)));
		teeth = min(teeth, sdCappedCone(p + u_Head[3] * vec3(0.25, -0.2, 1.4), u_Head[3] * vec3(3.4, .5, .5)));
		combine = min(combine, teeth);
		float eyes = min(sphereSDF(p + u_Head[3] * vec3(.3, -0.5, 0.7), u_Head[3] * .2), sphereSDF(p + u_Head[3] * vec3(-.3, -0.5, 0.7), u_Head[3] * .2));
		float monobrow = udBox(rotateX((p + u_Head[3] * vec3(0.0, -0.7, .65)), -20.0), u_Head[3] * vec3(.6, .2, .2));
		combine = min(min(combine, eyes), monobrow);
		return combine;
	}
	
	//~~~~HAND/FEET SDFs~~~~~//
	
	// For now, size is based on head size, but later make it the average joint size
	float clawFootSDF(vec3 p, float size) {
		size = size * 2.0;
		float base = udRoundBox(p, size * vec3(.6, .6, .3), .001);
		float fingees = sdConeSection(rotateZ(p + size * vec3(0.5, -0.9, 0.3), -20.0), size, size * .3, size * .05);
		fingees = min(fingees, sdConeSection(rotateZ(p + size * vec3(-0.5, -0.9, 0.3), 20.0), size, size * .3, size * .05));
		fingees = min(fingees, sdConeSection(p + size * vec3(0.0, -1.1, 0.3), size, size * .3, size * .05));
		float combine = smin(base, fingees, .13); // final foot
		return combine;
	}
	
	
	float handSDF(vec3 p, float size) {
		//float size = u_Head[3] / 1.5;
		//size = 1.0;
		float base = udRoundBox(p, size * vec3(.6, .6, .2), .08);
		float fingee1 = sdConeSection(rotateZ(p + size * vec3(1.1, -0.7, 0.0), -30.0), size, size * .5, size * .2);
		float fingee2 = sdConeSection((p + size * vec3(0.45, -1.9, 0.0)), size, size * .5, size * .2);
		float fingee3 = sdConeSection((p + size * vec3(-0.45, -1.9, 0.0)), size, size * .5, size * .2);
		fingee1 = min(fingee1, fingee2);
		fingee1 = min(fingee1, fingee3);
		float combine = smin(base, fingee1, .09);
		return combine;
	}
	
	float appendagesSDF(vec3 p) {
		AppendageInfoBuffer appenAttr = g_appenBuffer[0];
		LimbInfoBuffer limbAttr = g_limbBuffer[0];
		RotationInfoBuffer rotAttr = g_rotBuffer[0];
	
		float all = MAX_DIST;
		float angle;
		float angle1 = -35.0;
		float angle2 = 35.0;
	
		int armsNow = 0;
		int numAppen = 0;
	
		int startPos = 0;
		int startRot = 0;
		for (int i = 0; i < appenAttr.numAppen; i++) {
			int thisPos = startPos + (3 * ((limbAttr.limbLengths[i] - 1)));
			vec3 offset = vec3(limbAttr.jointLocData[thisPos], limbAttr.jointLocData[thisPos + 1], limbAttr.jointLocData[thisPos + 2]);
	
			if ((i % 2) == 0) {
				angle = angle1;
			}
			else
			{
				angle = angle2;
			}
			float foot;
	
			if (appenAttr.appenBools[numAppen] == 1) {
				armsNow = 1;
			}
	
			int thisRot = startRot + (4 * ((limbAttr.limbLengths[i] - 1)));
			if (armsNow == 0) {
				vec3 rotP = rotateZ((p + offset), 90.0);
				rotP = rotateY(rotP, 90.0);
				rotP = rotateZ(rotP, angle);
				foot = clawFootSDF(rotP, appenAttr.appenRads[numAppen]);
			}
			else {
				vec3 q = rotateInverseAxisAngle(rotAttr.rotations[thisRot], rotAttr.rotations[thisRot + 1], rotAttr.rotations[thisRot + 2], rotAttr.rotations[thisRot + 3],
					p + offset);
				foot = handSDF(rotateZ(q, 180.0), appenAttr.appenRads[numAppen]);
			}
	
			numAppen = numAppen + 1;
			startPos = thisPos + 3;
			startRot = thisRot + 4;
	
			all = min(all, foot);
		}
	
		return all;
	}
	
	float armSDF(vec3 p) {
	
		LimbInfoBuffer limbAttr = g_limbBuffer[0];
		RotationInfoBuffer rotAttr = g_rotBuffer[0];
	
		int countSegs = 0;
	
		float allLimbs = MAX_DIST;
		int incr = 0;
		int numLimbs = 0;
		int jointNum = 0;
		for (int i = 0; i < LIMBLEN_COUNT; i++) {
			jointNum += limbAttr.limbLengths[i];
		}
	
		//this is for each limb
		for (int j = 0; j < (jointNum * 3); j = j + incr) {
			numLimbs++;
	
			int count = 0;
	
			// NEED joint number to do the below operations...
	
			//count is number of joints in this limb
			count = int(limbAttr.limbLengths[numLimbs - 1]);
	
			float arm = MAX_DIST;
			// all joint positions for a LIM (jointNum * 3)
			int endJoint = (j + (count * 3));
			for (int i = j; i < endJoint; i = i + 3) {
				vec3 pTemp = p + vec3(limbAttr.jointLocData[i], limbAttr.jointLocData[i + 1], limbAttr.jointLocData[i + 2]);
				arm = min(arm, sphereSDF(pTemp, limbAttr.jointRadData[i / 3]));
			}
	
			// for 3 * (jointNum(per limb) - 1), each joint until last one
			float segments = MAX_DIST;
	
			endJoint = (j + ((count - 1) * 3));
			for (int i = j; i < endJoint; i = i + 3) {
				vec3 point0 = vec3(limbAttr.jointLocData[i], limbAttr.jointLocData[i + 1], limbAttr.jointLocData[i + 2]);
				vec3 point1 = vec3(limbAttr.jointLocData[i + 3], limbAttr.jointLocData[i + 4], limbAttr.jointLocData[i + 5]);
				vec3 midpoint = vec3((point0[0] + point1[0]) / 2.0, (point0[1] + point1[1]) / 2.0, (point0[2] + point1[2]) / 2.0);
				float len = length(point1 - point0);//distance(point0, point1);
	
				vec3 dir = point1 - point0; //dir is correct
	
				int r = (i / 3) * 4;
				vec3 q = rotateInverseAxisAngle(rotAttr.rotations[r], rotAttr.rotations[r + 1], rotAttr.rotations[r + 2], rotAttr.rotations[r + 3],
					p + midpoint);
	
				float part = sdConeSection(q, len / 2.0, limbAttr.jointRadData[(i + 3) / 3], limbAttr.jointRadData[i / 3]);
				segments = min(segments, part);
				countSegs++;
			}
	
			float combine = smin(arm, segments, .2); // this is one arm
			allLimbs = min(allLimbs, combine); //merge with all other limbs
	
			incr = count * 3;
		}
	
		return allLimbs;
	}
	
	
	float spineSDF(vec3 p) {
		HeadSpineInfoBuffer headSpineAttr = g_headSpineBuffer[0];
	
		float spine = MAX_DIST;
		for (int i = 0; i < SPINE_LOC_COUNT; i += 3) {
			if (headSpineAttr.spineLocData[i] == 0. && headSpineAttr.spineLocData[i + 1] == 0. && headSpineAttr.spineLocData[i + 2] == 0.) continue;
			vec3 pTemp = p + vec3(headSpineAttr.spineLocData[i], headSpineAttr.spineLocData[i + 1], headSpineAttr.spineLocData[i + 2]);
			spine = smin(spine, sphereSDF(pTemp, headSpineAttr.spineRadData[i / 3]), 0.06);
		}
		return spine;
	}
	
	// OVERALL SCENE SDF -- rotates about z-axis (turn-table style)
	float sceneSDF(vec3 p) {
		HeadSpineInfoBuffer headSpineAttr = g_headSpineBuffer[0];
	
		float headSDF = 0;
		int headType = headSpineAttr.headData[4];
		if (headType == 0) {
			headSDF = bugHeadSDF(p, headSpineAttr.headData);
		}
		else if (headType == 1) {
			headSDF = dinoHeadSDF(p, headSpineAttr.headData);
		}
		else if (headType == 2) {
			headSDF = trollHeadSDF(p, headSpineAttr.headData);
		}
		float spine = spineSDF(p);
		float headSpine = smin(spine, headSDF, .1);
		float limbs = armSDF(p);
		float appendages = appendagesSDF(p);
		return smin(smin(limbs, appendages, .2), headSpine, .1);
	}
};