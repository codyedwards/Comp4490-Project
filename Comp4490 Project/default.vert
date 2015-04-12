#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;
out vec3 pos;
out vec3 N;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

// Wavelength
uniform float L[4];
//float w = 2.0f/L;

// Amplitude
uniform float A[4];

// Speed
uniform float speed[4];



// Direction
uniform vec2 D[4];

float Q = 0.5;

vec3 wave(int i, float w)
{
	vec3 wave;
	
	float q = Q / (w * A[i] * 4);

	//float w = 2/L[i];
	//float s = speed[i] * w;

	float a = dot((w*D[i]), vec2(position.x, position.z)) + (speed[i] * w * time);

	//float a = dot(D[i], vec2(position.x, position.z)) * w;

	float b = cos(a);

	wave.x = q * A[i] * D[i].x * b;
	wave.y = q * A[i] * D[i].y * b;
	wave.z = A[i] * sin(a);

	//float b = time * s;
	//float c = (sin(a + b) + 1) / 2;

	return wave;
}

vec3 calcN(int i, float WA, float C, float S, float w)
{
	vec3 N;
	float q = Q / (w * A[i] * 4);

	N.x = D[i].x * WA * C;
	N.y = D[i].y * WA * C;
	N.z = q * WA * S;

	return N;
}

void main()
{
	vec3 H = vec3(0.0f, 0.0f, 0.0f);
	float WA[4];
	float w[4];
	float S[4];
	float C[4];
	N = vec3(0.0f, 0.0f, 0.0f);

	//Calc w
	for(int i = 0; i < 4; i++)
	{
		w[i] = sqrt(9.8 * ((2 * 3.14159)/L[i]));
	}

	// Calc WA
	for(int i = 0; i < 4; i++)
	{
		WA[i] = w[i] * A[i];
	}


	for(int i = 0; i < 4; i++)
	{
		H += wave(i, w[i]);
	}

	H.x = position.x + H.x;
	H.y = position.y + H.y;

	// Calc S
	for(int i = 0; i < 4; i++)
	{
		S[i] = sin(dot(w[i] * D[i], vec2(H.x, H.z)) + (speed[i] * w[i] * time));
	}

	// Calc C
	for(int i = 0; i < 4; i++)
	{
		C[i] = cos(dot(w[i] * D[i], vec2(H.x, H.z)) + (speed[i] * w[i] * time));
	}

	// Calc N
	for(int i = 0; i < 4; i++)
	{
		N += calcN(i, WA[i], S[i], C[i], w[i]);
	}

	N.x = -N.x;
	N.y = -N.y;
	N.z = 1 - N.z;

	N = normalize(N);

	pos = (view * model * vec4(position, 0.0)).xyz;

	N = normalize((view*model) * vec4(N, 0.0)).xyz;

	gl_Position = projection * view * model * vec4(H.x, H.z, position.z, 1.0f);
	TexCoords = texCoords;
}