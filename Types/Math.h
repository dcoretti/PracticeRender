#pragma once
#include <math.h>
float lerp(float rangeA, float rangeB, float amnt);
float invLerp(float val, float rangeA, float rangeB);
float clamp1(float f);

struct Vec3 {
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3() = default;

    Vec3 cross(const Vec3 &other) {
        return Vec3(y * other.z - z * other.y,
                    z * other.x - x * other.z,
                    x * other.y - y * other.x);
    }

    Vec3& operator=(const Vec3& r) {
        x = r.x;
        y = r.y;
        z = r.z;
        return *this;
    }
    Vec3& operator+=(const Vec3& r) {
        x += r.x;
        y += r.y;
        z += r.z;
        return *this;
    }
    Vec3& operator-=(const Vec3 &r) {
        x -= r.x;
        y -= r.y;
        z -= r.z;
        return *this;
    }
    Vec3& operator*=(const Vec3 &r) {
        x *= r.x;
        y *= r.y;
        z *= r.z;
        return *this;
    }
    Vec3& operator/=(const Vec3 &r) {
        x /= r.x;
        y /= r.y;
        z /= r.z;
        return *this;
    }

    void scale(float f) {
        x *= f;
        y *= f;
        z *= f;
    }

    float dot(const Vec3 &v) {
        return x * v.x + y * v.y + z * v.z;
    }

    float x{ 0.0f };
    float y{ 0.0f };
    float z{ 0.0f };
};

Vec3 cross(const Vec3 &l, const Vec3&r);
Vec3 operator+(Vec3 l, const Vec3 &r);
Vec3 operator-(Vec3 l, const Vec3 &r);
Vec3 operator*(Vec3 l, const Vec3 &r);
Vec3 operator/(Vec3 l, const Vec3 &r);
Vec3 operator-(Vec3 l);

struct Vec2 {

    float x, y;

    void scale(float f) {
        x *= f;
        y *= f;
    }
};

struct Vec4 {
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vec4(const Vec4 &other) :x(other.x), y(other.y), z(other.z), w(other.w) {
    }

    Vec4() = default;

    void normalize() {
        float length = len();
        if (length == 0) {
            return;
        }
        x = x / length;
        y = y / length;
        z = z / length;
        w = w / length;
    }

    float len() {
        return sqrtf((x * x) + (y * y) + (z * z) + (w * w));
    }

    float dot(const Vec4& b) {
        return x * b.x + y * b.y + z * b.z + w * b.w;
    }

    Vec4& operator=(const Vec4 &r) {
        x = r.x;
        y = r.y;
        z = r.z;
        w = r.w;
        return *this;
    }

    Vec4& operator+=(const Vec4 &r) {
        x += r.x;
        y += r.y;
        z += r.z;
        w += r.w;
        return *this;
    }

    Vec4& operator-=(const Vec4 &r) {
        x -= r.x;
        y -= r.y;
        z -= r.z;
        w -= r.w;
        return *this;
    }

    Vec4& operator*=(const Vec4 &r) {
        x *= r.x;
        y *= r.y;
        z *= r.z;
        w *= r.w;
        return *this;
    }

    Vec4& operator/=(const Vec4 &r) {
        x /= r.x;
        y /= r.y;
        z /= r.z;
        w /= r.w;
        return *this;
    }


    float * ptr() { return &x; }

    float x, y, z, w;
};


Vec4 operator+(Vec4 l, const Vec4 &r);
Vec4 operator-(Vec4 l, const Vec4 &r);
Vec4 operator*(Vec4 l, const Vec4 &r);
Vec4 operator/(Vec4 l, const Vec4 &r);
Vec4 operator-(Vec4 r);


struct Mat4 {
    Mat4() = default;
    Mat4(const Mat4& other) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                m[i][j] = other.m[i][j];
            }
        }
    }

    Mat4& operator=(const Mat4& other) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                m[i][j] = other.m[i][j];
            }
        }
        return *this;
    }

    Mat4& operator*=(const Mat4 &r) {
        Mat4 res;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                // A * B  row a col b
                res.m[i][j] = m[i][0] * r.m[0][j] + m[i][1] * r.m[1][j] + m[i][2] * r.m[2][j] + m[i][3] * r.m[3][j];
            }
        }
        *this = res;
        return *this;
    }

    static Mat4 scale(float f) {
        Mat4 mat;
        mat.m[0][0] = f;
        mat.m[1][1] = f;
        mat.m[2][2] = f;
        return mat;
    }

    static Mat4 translate(const Vec3 &v) {
        Mat4 mat;

        mat.m[0][3] = v.x;
        mat.m[1][3] = v.y;
        mat.m[2][3] = v.z;
        return mat;
    }

    static Mat4 rotx(float deg) {
        Mat4 mat;

        mat.m[1][1] = cosf(deg);
        mat.m[1][2] = -sinf(deg);
        mat.m[2][1] = sinf(deg);
        mat.m[2][2] = cosf(deg);
        return mat;
    }

    static Mat4 roty(float deg) {
        Mat4 mat;

        mat.m[0][0] = cosf(deg);
        mat.m[0][2] = sinf(deg);
        mat.m[2][0] = -sinf(deg);
        mat.m[2][2] = cosf(deg);
        return mat;
    }

    static Mat4 rotz(float deg) {
        Mat4 mat;

        mat.m[0][0] = cosf(deg);
        mat.m[0][1] = -sinf(deg);
        mat.m[1][0] = sinf(deg);
        mat.m[1][1] = cosf(deg);
        return mat;
    }

    float m[4][4]{
        { 1,0,0,0 },
        { 0,1,0,0 },
        { 0,0,1,0 },
        { 0,0,0,1 } // point
    };


};

Mat4 operator*(Mat4 l, const Mat4 &r);

Vec4 operator*(const Mat4 &mat, const Vec4 &v);