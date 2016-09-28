#include "Math.h"

float lerp(float rangeA, float rangeB, float amnt) {
    return rangeA + (rangeB - rangeA) * amnt;
}

float invLerp(float val, float rangeA, float rangeB) {
    return (val - rangeA) / (rangeB - rangeA);
}


float clamp1(float f) {
    if (f < 0.0f) {
        return 0.0f;
    }
    if (f > 1.0f) {
        return 1.0f;
    }
    return f;
}

Vec3 scale(const Vec3 &l, float s) {
	return Vec3(l.x * s, l.y * s, l.z * s);
}

float dot(const Vec3 &l, const Vec3 &r) {
	return l.x * r.x + l.y * r.y + l.z * r.z;
}

Vec3 cross(const Vec3 &l, const Vec3&r) {
    return Vec3(l.y * r.z - l.z * r.y,
                l.z * r.x - l.x * r.z,
                l.x * r.y - l.y * r.x);
}

Vec3 operator+(Vec3 l, const Vec3 &r) {
    return l += r;
}

Vec3 operator-(Vec3 l, const Vec3 &r) {
    return l -= r;

}

Vec3 operator-(Vec3 l) {
    return Vec3(-l.x, -l.y, -l.z);
}

Vec4 operator+(Vec4 l, const Vec4 &r) {
    Vec4 res = l += r;
    return res;

}

Vec4 operator-(Vec4 l, const Vec4 &r) {
    Vec4 res = l -= r;
    return res;
}


Vec4 operator-(Vec4 r) {
    Vec4 res;
    res.x = -r.x;
    res.y = -r.y;
    res.z = -r.z;
    res.w = -r.w;
    return res;
}

Mat4 operator*(Mat4 l, const Mat4 &r) {
    l *= r;
    return l;
}

Vec4 operator*(const Mat4 &mat, const Vec4 &v) {
    return Vec4(mat.m[0][0] * v.x + mat.m[0][1] * v.y + mat.m[0][2] * v.z + mat.m[0][3] * v.w,
        mat.m[1][0] * v.x + mat.m[1][1] * v.y + mat.m[1][2] * v.z + mat.m[1][3] * v.w,
        mat.m[2][0] * v.x + mat.m[2][1] * v.y + mat.m[2][2] * v.z + mat.m[2][3] * v.w,
        mat.m[3][0] * v.x + mat.m[3][1] * v.y + mat.m[3][2] * v.z + mat.m[3][3] * v.w);
}


float degToRad(float deg) {
	return deg * (float)M_PI / 180.0f;
}


Mat4 transpose(Mat4 m) {
	Mat4 r;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			r.m[i][j] = m.m[j][i];
		}
	}
	return r;
}

Mat4 fromRot(Vec3 &r) {
	Mat4 mRot;
	float c = cosf(r.x);

	mRot.m[0][0] = cosf(r.y) * cosf(r.z); 
	mRot.m[0][1] = cosf(r.z)*sinf(r.x)*sinf(r.y) - cosf(r.x)*sinf(r.z);
	mRot.m[0][2] = cosf(r.x) * cosf(r.z) * sinf(r.y) + sinf(r.x) * sinf(r.z);
	mRot.m[1][0] = cosf(r.y) * sinf(r.z); 
	mRot.m[1][1] = cosf(r.x)*cosf(r.z) + sinf(r.x) * sinf(r.y) * sinf(r.z);
	mRot.m[1][2] = cosf(r.x) * sinf(r.y) * sinf(r.z) - cosf(r.z) * sinf(r.x);
	mRot.m[2][0] = -sinf(r.y);
	mRot.m[2][1] = cosf(r.y) * sinf(r.x);
	mRot.m[2][2] = cosf(r.x) * cosf(r.y);
	return mRot;
}

Mat4 invRotTrans(Vec3 translation, Vec3 rot) {
	// 1. Covnert rotation to a matrix (assume rot in radians) and transpose.
	// 2. add in opposite translation
	Mat4 m = transpose(fromRot(rot));
	m.m[0][3] = -translation.x;
	m.m[1][3] = -translation.y;
	m.m[2][3] = -translation.z;
	
	return m;
}