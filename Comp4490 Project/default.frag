#version 330 core

in vec2 TexCoords;
in vec3 pos;
in vec3 N;

vec4 Ka = vec4(0.31f, 0.54f, 0.75f, 1.0f);
vec4 Kd = vec4(0.5f, 0.65f, 0.75f, 1.0f);
vec4 Ks = vec4(1.0f, 1.0f, 1.0f,  1.0f);
float Shininess = 200.0f;

vec3 LightPosition = vec3(10.0f, 10.0f, 10.0f);

out vec4 color;

uniform sampler2D texture_diffuse1;

void main()
{
	vec3 L = normalize(-LightPosition );
	vec3 E = normalize(-pos);
	vec3 H = normalize(L + E);

	vec4 AmbientProduct = Ka;
	float DiffuseProduct = max(dot(L, N), 0.0);
	vec4 diffuse = Kd*DiffuseProduct;
	float SpecularProduct = pow(max(dot(N, H), 0.0), Shininess);

	vec4 specular = Ks * SpecularProduct;

	if(dot(L, N) < 0.0f)
		specular = vec4(0.0f, 0.0f, 0.0f, 1.0f);

	color = AmbientProduct + DiffuseProduct + specular;

	//color = vec4(texture(texture_diffuse1, TexCoords));
}