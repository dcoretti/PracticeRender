#include "Quaternion.h"

Vec3 Quat::rotate(Vec3 &p) {
	Quat pQuat(p, 0.0f); // <-- works since we never use anything that requires pQuat to be unit length
	return product(product(*this, pQuat), inverse()).v;
}



Quat product(const Quat &l, const Quat& r){
	return Quat(scale(r.v, l.w) + scale(l.v, r.w) + cross(l.v, r.v), l.w * r.w - dot(l.v, r.v));

}

float dot(const Quat &l, const Quat &r) {
	return l.w * r.w + dot(l.v, r.v);
}


Quat slerp(Quat q0, const Quat&q1, float t) {
	// ((q1q0^-1)^t)*q0	// multiply the original orientation by the interpolated orientation
	// or sin(1-t)w/sin(w) * q0 + sin(tw)/sin(w) * q1
	// where cos(w) = dot(q0,q1)

	float cOmega = dot(q0, q1);
	if (cOmega < 0.0f) {
		q0.w = -q0.w;
		q0.v = -q0.v;
		cOmega = -cOmega;
	}
	float k0, k1;
	if (cOmega > 0.9999f) {
		// linear interpolation for very close orientations
		k0 = 1.0f - t;
		k1 = t;
	} else {
		float sOmega = sqrtf(1.0f - cOmega * cOmega);
		float omega = atan2f(sOmega, cOmega);
		float invSOmega = 1.0f / sOmega;
		k0 = sinf((1.0f - t) * omega) * invSOmega;
		k1 = sinf(t * omega) * invSOmega;
	}

	return Quat(Vec3(q0.v.x * k0 + q1.v.x * k1,
		q0.v.y * k0 + q1.v.y * k1,
		q0.v.z * k0 + q1.v.z * k1),
		q0.w * k0 + q1.w * k1);
}