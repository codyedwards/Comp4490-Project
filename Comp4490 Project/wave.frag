#version 330 core



in vec2 TexCoords;
in vec3 pos;
in mat3 TBN;
in vec2 texConst;

vec4 Ka = vec4(0.31f, 0.54f, 0.75f, 1.0f);
vec4 Kd = vec4(0.5f, 0.65f, 0.75f, 1.0f);
vec4 Ks = vec4(1.0f, 1.0f, 1.0f,  1.0f);
float Shininess = 7000.0f;

vec4 waterColor = vec4(0.2f, 0.4f, 0.75f, 1.0f);

uniform vec3 LightPosition;

out vec4 color;

uniform sampler2D texture_diffuse1;
uniform samplerCube skybox;

void main()
{
	float k = 2.0f;

	float lookUp = pow((sin(2*3.14159*TexCoords.x) + 1) / 2.0f, k - 1) * cos(2*3.14159*TexCoords.x);

	vec2 tex = lookUp * texConst;

	tex *= 1/15;

	tex = normalize(tex);

	vec3 L = normalize(-LightPosition );
	vec3 E = normalize(-pos);
	
	vec3 N = normalize(texture2D( texture_diffuse1, vec2(tex.x, TexCoords.y) ).rgb*2.0 - 1.0);

	L = normalize(TBN * L);
	E = normalize(TBN * E);

	vec3 H = normalize(L + E);

	vec4 AmbientProduct = Ka;
	float DiffuseProduct = max(dot(L, N), 0.0);
	vec4 diffuse = Kd*DiffuseProduct;
	float SpecularProduct = pow(max(dot(N, H), 0.0), Shininess);

	vec4 specular = Ks * SpecularProduct;

	if(dot(L, N) < 0.0f)
		specular = vec4(0.0f, 0.0f, 0.0f, 1.0f);

	// refractive index of water is 1.33
	float n1 = 1.0f;
	float n2 = 1.33f;
	float ratio = n1 / n2;

	vec3 Reflect = reflect(H, N);
	vec3 Refract = refract(H, N, ratio);

	vec4 reflectColor = Ks * texture(skybox, Reflect);
	vec4 refractColor = texture(skybox, Refract);

	// calc fresnal term
	float cosi = dot(normalize(-LightPosition), N);
	float cost = dot(Refract, N);

	float Rs = ( (n1*cosi)-(n2*cost) ) / ( (n1*cosi)+(n2*cost) );
	Rs = Rs * Rs;

	float Rp = ( (n1*cost)-(n2*cosi) ) / ( (n1*cost)+(n2*cosi) );
	Rp = Rp * Rp;

	float R = ( Rs + Rp ) / 2.0f;

	// Schlick's approximation
	//float R0 = (pow(ratio-1, 2)/pow(ratio+1, 2));
	//float R = R0 + ((1 - R0) * pow(1-dot(H, E), 5));

	// reflect/refract color
	vec4 refColor = mix(refractColor, reflectColor, R*R);

	//color = AmbientProduct + DiffuseProduct + specular;
	//color = refractColor;
	color = waterColor * (AmbientProduct + DiffuseProduct + specular + refColor);
}