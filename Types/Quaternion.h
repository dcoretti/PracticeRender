#pragma once
#include <math.h>
#include "Math.h"

// Unit quaternion only! will break if ||q|| != 1!
// TODO add some debug assert which asserts normalization
struct Quat {
	Quat() {
		w = 1.0f;	// or -1 given dual representation of same angular displacement vector
		v = Vec3(0.0f, 0.0f, 0.0f);
	}

	Quat(Vec3 unitVec, float theta) {
		// test that unitVec is unit?
		w = cosf(theta / 2.0f);
		float sTheta = sinf(theta / 2.0f);

		v = unitVec.scale(sTheta);
	}

	// From Euler angle in radians.
	Quat(float xRot, float yRot, float zRot) {
		float c1 = cosf(yRot / 2.0f);
		float c2 = cosf(xRot / 2.0f);
		float c3 = cosf(zRot / 2.0f);
		float s1 = sinf(yRot / 2.0f);
		float s2 = sinf(xRot / 2.0f);
		float s3 = sinf(zRot / 2.0f);
		float c1c2 = c1 * c2;
		float s1s2 = s1 * s2;
		w = c1c2 * c3 - s1s2 * c3;
		v.x = c1c2 * s3 + s1s2 * c3;
		v.y = s1*s2*s3 + c1 * s2 *s3;
		v.z = c1 * s2 * c3 - s1 * c2 * s3;
	
		v.normalize();
	}

	Quat conjugate() {
		return Quat(-v, w);
		// equivalent is to negate just w since it keeps the axis of rotation but negates the angular displacement

	}

	// conjugate is also the multiplicative inverse for unit quaternions (q*/||q|| = q*/1) 
	Quat inverse() {
		return conjugate();
	}

	Mat4 toMat4() {
		Mat4 mat;
		mat.m[0][0] = 1.0f - 2.0f *( v.y * v.y) - 2.0f * (v.z * v.z);
		mat.m[0][1] = 2.0f * v.x*v.y + 2.0f * w*v.z;
		mat.m[0][2] = 2.0f * v.x*v.z - 2.0f * w*v.y;
		
		mat.m[1][0] = 2.0f * v.x * v.y - 2.0f * w * v.y;
		mat.m[1][1] = 1.0f - 2.0f * (v.x*v.x) - 2.0f * (v.z*v.z);
		mat.m[1][2] = 2.0f * v.y*v.z + 2.0f * w * v.x;
		
		mat.m[2][0] = 2.0f * v.x*v.z + 2.0f * w * v.y;
		mat.m[2][1] = 2.0f * v.y * v.z - 2.0f * w * v.x;
		mat.m[2][2] = 1.0f - 2.0f * (v.x * v.x) - 2.0f * (v.y * v.y);

		return mat;
	}

	Vec3 rotate(Vec3 &p);

	Vec3 v;
	float w;
};

Quat product(const Quat &l, const Quat& r);
float dot(const Quat &l, const Quat &r);


// From pp.262 3d math primer for graphics and game development
Quat slerp(Quat q0, const Quat&q1, float t);