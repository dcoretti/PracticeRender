#pragma once
#include <cstdint>

struct AnimationInstance {
	double fps;
	bool enabled;
	bool loop;
	uint64_t localTime;

	int curClip;
	int numClips;
};