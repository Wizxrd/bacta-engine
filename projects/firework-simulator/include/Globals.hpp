#pragma once
#define _USE_MATH_DEFINES
#include <algorithm>
#include <iostream>
#include <math.h>

static float clamp(float value){
	return std::max(0.f, std::min(value, 1.f));
}

static float clamp(float value, float max){
	return std::max(0.f, std::min(value, max));
}

static float randomize(float min, float max){
	return min + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (max - min);
}

namespace _G{
	static const float GRAVITY = 100.f;
	static const float DRAG = 0.985f;
	static const float WINDOW_WIDTH = 1300.f;
	static const float WINDOW_HEIGHT = 900.f;
}