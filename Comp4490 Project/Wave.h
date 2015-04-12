#pragma once

#include <glm\glm.hpp>

using namespace glm;

class Wave{
public:
	float waveLength;
	float amplitude;
	float speed;
	vec2 direction;
	float phaseConstant;
	float frequency;
	Wave(float w, float a, float s, vec2 d)
	{
		waveLength = w;
		amplitude = a;
		speed = s;
		direction = d;
		frequency = 2.0f / waveLength;
		phaseConstant = speed * frequency;
	}
};