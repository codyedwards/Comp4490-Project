#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;
out vec3 pos;
out mat3 TBN;
out vec2 texConst;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform float kAmpOverLen;
uniform float tex_kAmpOverLen;
uniform vec3 cameraPos;

// Wavelength
uniform float L[4];

// Amplitude
uniform float A[4];

// Speed
uniform float speed[4];

// Direction
uniform vec2 D[4];

// Texture Waves

// Wavelength
uniform float texL[15];

// Amplitude
uniform float texA[15];

// Speed
uniform float texSpeed[15];

// Direction
uniform vec2 texD[15];

uniform float Q;
float k = 2.0f;

vec3 wave(int i, float w, float q)
{
	vec3 wave;
	

	//float w = 2/L[i];
	//float s = speed[i] * w;

	float a = dot((w*D[i]), vec2(position.x, position.z)) + (speed[i] * w * time);

	//float a = dot(D[i], vec2(position.x, position.z)) * w;

	float b = cos(a);

	wave.x = q * A[i] * D[i].x * b;
	//wave.y = q * A[i] * D[i].y * b;
	wave.z = A[i] * sin(a);

	//float b = time * s;
	//float c = (sin(a + b) + 1) / 2;

	return wave;
}

vec3 calcN(int i, float WA, float C, float S, float q)
{
	vec3 N;

	N.x = D[i].x * WA * C;
	N.y = D[i].y * WA * C;
	N.z = q * WA * S;

	return N;
}

vec3 calcT(int i, float WA, float C, float S, float q)
{
	vec3 T;

	T.x = q * D[i].x * D[i].y * WA * S;
	T.y = q * (D[i].y * D[i].y) * WA * S; 
	T.z = D[i].y * WA * C;

	return T;
}

vec3 calcB(int i, float WA, float C, float S, float q)
{
	vec3 B;

	B.x = q * (D[i].x*D[i].x) * WA * S;
	B.y = q * D[i].x * D[i].y * WA * S;
	B.z = D[i].x * WA * C;

	return B;
}

void main()
{
	vec3 H = vec3(0.0f, 0.0f, 0.0f);
	vec3 T = vec3(0.0f, 0.0f, 0.0f);
	float WA[4];
	float w[4];
	float S[4];
	float C[4];
	float q[4];
	vec3 N = vec3(0.0f, 0.0f, 0.0f);
	vec3 B = vec3(0.0f, 0.0f, 0.0f);


	//Calc w
	for(int i = 0; i < 4; i++)
	{
		w[i] = sqrt(9.8 * ((2 * 3.14159)/L[i]));
	}

	// Calc q
	for(int i = 0; i < 4; i++)
	{
		q[i] = Q / (w[i] * A[i] * 4);
	}

	// Calc WA
	for(int i = 0; i < 4; i++)
	{
		WA[i] = w[i] * A[i];
	}


	for(int i = 0; i < 4; i++)
	{
		H += wave(i, w[i], q[i]);
	}

	H.x = position.x + H.x;
	//H.y = position.y + H.y;

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
		N += calcN(i, WA[i], S[i], C[i], q[i]);
	}

	N.x = -N.x;
	N.y = -N.y;
	N.z = 1 - N.z;

	//N = normalize(N);

	// Calc T
	for(int i = 0; i < 4; i++)
	{
		T += calcT(i, WA[i], S[i], C[i], q[i]);
	}

	T.x = -T.x;
	T.y = 1 - T.y;

	//T = normalize(T);

	// Calc B
	for(int i = 0; i < 4; i++)
	{
		B += calcB(i, WA[i], S[i], C[i], q[i]);
	}

	B.x = 1 - B.x;
	B.y = -B.y;

	//B = normalize(B);

	mat3 Surf2World = mat3(B.x, B.z, B.y, 
							T.x, T.z, T.y, 
							N.x, N.z, N.y);

	vec3 biT = cross(N, T);

	pos = (view * model * vec4(position, 0.0)).xyz;

	N = normalize((view*model) * vec4(N, 0.0)).xyz;

	biT = normalize((view*model) * vec4(biT, 0.0)).xyz;

	T = normalize((view*model) * vec4(T, 0.0)).xyz;

	mat3 Tex2Surf = transpose(mat3(T, biT, N));

	float scale = 15 * 2 * tex_kAmpOverLen;

	mat3 Rescale = mat3(scale, 0.0f, 0.0f, 
						0.0f, scale, 0.0f, 
						0.0f, 0.0f, 1.0f);

	TBN = Surf2World * Tex2Surf * Rescale;

	// Calculate texture waves

	float u = 0.0f;

	// Calc u
	for(int i = 0; i < 15; i++)
	{
		float phase = texSpeed[i] * (2/texL[i]);

		u += dot(texD[i], texCoords) + (phase*time);
	}

	texConst = vec2(0.0f, 0.0f);

	// Calc texConst
	for(int i = 0; i < 15; i++)
	{
		float freq = sqrt(9.8 * ((2 * 3.14159)/texL[i]));

		texConst += (texD[i] * freq * texA[i] * k);
	}

	gl_Position = projection * view * model * vec4(H.x, H.z, position.z, 1.0f);
	TexCoords = vec2(u, texCoords.y);
	//TexCoords = texCoords;
}