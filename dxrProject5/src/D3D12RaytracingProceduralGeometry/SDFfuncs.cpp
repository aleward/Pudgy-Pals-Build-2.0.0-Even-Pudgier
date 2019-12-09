#include "stdafx.h"
#include "SDFfuncs.h"


// CLASS FUNCTIONS WITHIN VEC3
vec3::vec3() : data{ 0.0, 0.0, 0.0 } {}
vec3::vec3(float x, float y, float z) : data{ x, y, z } {}
vec3::vec3(const vec3 &v2) : vec3(v2[0], v2[1], v2[2]) {}

/// Getters/Setters
float vec3::operator[](unsigned int index) const {
	if (index > 2) { std::out_of_range("Index must be between 0 and 2"); }
	return data[index];
}
float& vec3::operator[](unsigned int index) {
	if (index > 2) { std::out_of_range("Index must be between 0 and 2"); }
	return data[index];
}

/// Operator Functions 
vec3& vec3::operator=(const vec3 &v2) {
	data[0] = v2[0];
	data[1] = v2[1];
	data[2] = v2[2];
	return *this;
}

/// Arithmetic:
vec3& vec3::operator+=(const vec3 &v2) {
	data[0] = data[0] + v2[0];
	data[1] = data[1] + v2[1];
	data[2] = data[2] + v2[2];
	return *this;
}
vec3& vec3::operator-=(const vec3 &v2) {
	data[0] = data[0] - v2[0];
	data[1] = data[1] - v2[1];
	data[2] = data[2] - v2[2];
	return *this;
}
vec3& vec3::operator*=(float c) {
	data[0] = c * data[0];
	data[1] = c * data[1];
	data[2] = c * data[2];
	return *this;
}
vec3& vec3::operator/=(float c) {
	data[0] = data[0] / c;
	data[1] = data[1] / c;
	data[2] = data[2] / c;
	return *this;
}

vec3 vec3::operator+(const vec3 &v2) const {
	return vec3(data[0] + v2[0], data[1] + v2[1], data[2] + v2[2]);
}
vec3 vec3::operator-(const vec3 &v2) const {
	return vec3(data[0] - v2[0], data[1] - v2[1], data[2] - v2[2]);
}
vec3 vec3::operator*(float c) const {
	return vec3(data[0] * c, data[1] * c, data[2] * c);
}
vec3 vec3::operator/(float c) const {
	return vec3(data[0] / c, data[1] / c, data[2] / c);
}


// VEC3 RELATED FUNCTIONS OUTSIDE OF VEC3
vec3 abs(const vec3 &v) {
	return vec3(abs(v[0]), abs(v[1]), abs(v[2]));
}
vec3 maxV(const vec3 &v, float m) {
	return vec3(max(v[0], m), max(v[1], m), max(v[2], m));
}
float dot(const vec3 &v1, const vec3 &v2) {
	return (v1[0] * v2[0]) + (v1[1] * v2[1]) + (v1[2] * v2[2]);
}
vec3 cross(const vec3 &v1, const vec3 &v2) {
	return vec3(v1[1] * v2[2] - v2[1] * v1[2],
				v1[2] * v2[0] - v2[2] * v1[0],
				v1[0] * v2[1] - v2[0] * v1[1]);
}
float length(const vec3 &v) {
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}
vec3 normalize(const vec3 &v) {
	float n = length(v);
	if (n != 0) { return vec3(v[0] / n, v[1] / n, v[2] / n); }
	else { return vec3(0.0, 0.0, 0.0); }
}
vec3 operator*(float c, const vec3 &v) {
	return vec3(v[0] * c, v[1] * c, v[2] * c);
}
std::string printVec(const vec3 &v) {
	return "[" + std::to_string(v[0]) + ", " + std::to_string(v[1]) + ", " + std::to_string(v[2]) + "]\n";
}


// CLASS FUNCTIONS WITHIN VEC2
/// Constructors
vec2::vec2() : data{ 0.0, 0.0 } {}
vec2::vec2(float x, float y) : data{ x, y } {}
vec2::vec2(const vec2 &v2) : vec2(v2[0], v2[1]) {}

/// Getters/Setters
float vec2::operator[](unsigned int index) const {
	if (index > 1) { std::out_of_range("Index must be between 0 and 1"); }
	return data[index];
}
float& vec2::operator[](unsigned int index) {
	if (index > 1) { std::out_of_range("Index must be between 0 and 1"); }
	return data[index];
}

/// Operator Functions 
vec2& vec2::operator=(const vec2 &v2) {
	data[0] = v2[0];
	data[1] = v2[1];
	return *this;
}

/// Arithmetic:
vec2& vec2::operator+=(const vec2 &v2) {
	data[0] = data[0] + v2[0];
	data[1] = data[1] + v2[1];
	return *this;
}
vec2& vec2::operator-=(const vec2 &v2) {
	data[0] = data[0] - v2[0];
	data[1] = data[1] - v2[1];
	return *this;
}
vec2& vec2::operator*=(float c) {
	data[0] = c * data[0];
	data[1] = c * data[1];
	return *this;
}
vec2& vec2::operator/=(float c) {
	data[0] = data[0] / c;
	data[1] = data[1] / c;
	return *this;
}

vec2 vec2::operator+(const vec2 &v2) const {
	return vec2(data[0] + v2[0], data[1] + v2[1]);
}
vec2 vec2::operator-(const vec2 &v2) const {
	return vec2(data[0] - v2[0], data[1] - v2[1]);
}
vec2 vec2::operator*(float c) const {
	return vec2(data[0] * c, data[1] * c);
}
vec2 vec2::operator/(float c) const {
	return vec2(data[0] / c, data[1] / c);
}
vec2 vec2::operator*(vec2 v) const {
	return vec2(data[0] * v[0], data[1] * v[1]);
}
vec2 vec2::operator/(vec2 v) const {
	return vec2(data[0] / v[0], data[1] / v[1]);
}

// VEC2 RELATED FUNCTIONS
vec2 abs(const vec2 &v) {
	return vec2(abs(v[0]), abs(v[1]));
}

vec2 maxV(const vec2 &v, float m) {
	return vec2(max(v[0], m), max(v[1], m));
}

/// Dot Product
float dot(const vec2 &v1, const vec2 &v2) {
	return (v1[0] * v2[0]) + (v1[1] * v2[1]);
}

/// Returns the geometric length of the input vector
float length(const vec2 &v) {
	return sqrt(v[0] * v[0] + v[1] * v[1]);
}

/// Scalar Multiplication (c * v)
vec2 operator*(float c, const vec2 &v) {
	return vec2(v[0] * c, v[1] * c);
}



// MAT3 CLASS FUNCS
/// Constructors
mat3::mat3(float diag) : data{ vec3(diag, 0.0, 0.0), vec3(0.0, diag, 0.0), vec3(0.0, 0.0, diag) } {}
mat3::mat3() : mat3(1.0) {}
mat3::mat3(const vec3 &col0, const vec3 &col1, const vec3 &col2) : data{ col0, col1, col2 } {}
mat3::mat3(const mat3 &m2) : mat3(m2[0], m2[1], m2[2]) {}

/// Getters
vec3 mat3::operator[](unsigned int index) const {
	if (index > 2) { std::out_of_range("Index must be between 0 and 3"); }
	return data[index];
}

/// Returns a reference to the column at the index
vec3& mat3::operator[](unsigned int index) {
	if (index > 2) { std::out_of_range("Index must be between 0 and 3"); }
	return data[index];
}

/// Static Initializers   TAKE IN RADIANS
mat3 mat3::rotateX(float angle) {
	return mat3(vec3(1.0, 0.0, 0.0),
		vec3(0.0, cos(angle), sin(angle)),
		vec3(0.0, -sin(angle), cos(angle)));
}
mat3 mat3::rotateY(float angle) {
	return mat3(vec3(cos(angle), 0.0, -sin(angle)),
		vec3(0.0, 1.0, 0.0),
		vec3(sin(angle), 0.0, cos(angle)));
}
mat3 mat3::rotateZ(float angle) {
	return mat3(vec3(cos(angle), sin(angle), 0.0),
		vec3(-sin(angle), cos(angle), 0.0),
		vec3(0.0, 0.0, 1.0));
}
mat3 mat3::scale(float x, float y, float z) {
	return mat3(vec3(x, 0, 0), vec3(0, y, 0), vec3(0, 0, z));
}
mat3 mat3::identity() {
	return mat3();
}

/// Operator Functions
mat3& mat3::operator=(const mat3 &m2) {
	data[0] = m2[0];
	data[1] = m2[1];
	data[2] = m2[2];
	return *this;
}

// OPERATOR Defs
/// Matrix multiplication (m1 * m2)
mat3 mat3::operator*(const mat3 &m2) const {
	return mat3(m2 * data[0], m2 * data[1], m2 * data[2]);
}

/// Matrix/vector multiplication (m * v)
vec3 mat3::operator*(const vec3 &v) const {
	return vec3(dot(row(*this, 0), v), dot(row(*this, 1), v), dot(row(*this, 2), v));
}


// MAT3-RELATED
/// Returns a row of the input matrix
vec3 row(const mat3 &m, unsigned int index) {
	if (index > 2) { std::out_of_range("Index must be between 0 and 3"); }
	return vec3(m[0][index], m[1][index], m[2][index]);
}

mat3 transpose(const mat3 &m) {
	return mat3(row(m, 0), row(m, 1), row(m, 2));
}

/// Vector/matrix multiplication (v * m)
vec3 operator*(const vec3 &v, const mat3 &m) {
	return transpose(m) * v;
}



// MAT2 FUNCTIONS
/// Constructors
mat2::mat2(float diag) : data{ vec2(diag, 0.0), vec2(0.0, diag) } {}
mat2::mat2() : mat2(1.0) {}
mat2::mat2(const vec2 &col0, const vec2 &col1) : data{ col0, col1 } {}
mat2::mat2(const mat2 &m2) : mat2(m2[0], m2[1]) {}

/// Getters
vec2 mat2::operator[](unsigned int index) const {
	if (index > 1) { std::out_of_range("Index must be between 0 and 3"); }
	return data[index];
}

/// Returns a reference to the column at the index
vec2& mat2::operator[](unsigned int index) {
	if (index > 1) { std::out_of_range("Index must be between 0 and 3"); }
	return data[index];
}

/// Static Initializers   TAKE IN RADIANS
mat2 mat2::scale(float x, float y, float z) {
	return mat2(vec2(x, 0), vec2(0, y));
}
mat2 mat2::identity() {
	return mat2();
}

/// Operator Functions
mat2& mat2::operator=(const mat2 &m2) {
	data[0] = m2[0];
	data[1] = m2[1];
	return *this;
}

// OPERATOR Defs
/// Matrix multiplication (m1 * m2)
mat2 mat2::operator*(const mat2 &m2) const {
	return mat2(m2 * data[0], m2 * data[1]);
}

/// Matrix/vector multiplication (m * v)
vec2 mat2::operator*(const vec2 &v) const {
	return vec2(dot(row(*this, 0), v), dot(row(*this, 1), v));
}


// MAT2 RELATED
/// Returns a row of the input matrix
vec2 row(const mat2 &m, unsigned int index) {
	if (index > 1) { std::out_of_range("Index must be between 0 and 3"); }
	return vec2(m[0][index], m[1][index]);
}

mat2 transpose(const mat2 &m) {
	return mat2(row(m, 0), row(m, 1));
}

/// Vector/matrix multiplication (v * m)
vec2 operator*(const vec2 &v, const mat2 &m) {
	return transpose(m) * v;
}