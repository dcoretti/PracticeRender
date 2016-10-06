#pragma once
#include <cstdint>
#include <Windows.h>
static double counterFrequency;

void initTimers() {
	LARGE_INTEGER perfFrequency;
	QueryPerformanceFrequency(&perfFrequency);
	counterFrequency = (double)perfFrequency.QuadPart;
}

uint64_t getPerfCounter() {
	LARGE_INTEGER res;
	QueryPerformanceCounter(&res);
	return res.QuadPart;
}


double getElapsedTimeSeconds(uint64_t t0, uint64_t t1) {
	return  (t1 - t0) / counterFrequency;
}