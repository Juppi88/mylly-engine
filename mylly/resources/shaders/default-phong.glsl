#define APPLY_LIGHTING
#pragma include inc/mylly.glinc

varying vec3 worldPosition;
varying vec3 worldNormal;
varying vec2 texCoord;

#if defined(VERTEX_SHADER)

void main()
{
	mat4 modelView = MatrixArr[MAT_VIEW] * MatrixArr[MAT_MODEL];
	vec4 position = modelView * vec4(Vertex, 1);

	// Varyuing arguments to fragment shader.
	gl_Position = toclipspace(Vertex);// MatrixArr[MAT_PROJECTION] * position;
	worldPosition = position.xyz;
	texCoord = TexCoord;

	// Calculate world space normal.
	mat3 normalMatrix = transpose(inverse(mat3(modelView)));
	worldNormal = normalize(normalMatrix * Normal);
}

#elif defined(FRAGMENT_SHADER)

vec3 AmbientLight = vec3(0.5, 0.5, 0.5);

vec3 DiffuseColour = vec3(0.6, 0.6, 0.6);
vec3 SpecularColour = vec3(0.9, 0.9, 0.9);
float Shininess = 2;

vec3 ApplyLight(int light)
{
	// Calculate light intensity at the fragment.
	vec3 position = lightPosition(light).xyz;
	vec3 normal = normalize(worldNormal);
	vec3 direction;
	float intensity;

	if (lightPosition(light).w == 0) {

		// Directional light, no attenuation.
		intensity = lightIntensity(light);
		direction = normalize(position);
	}
	else {

		// Spot or point light.
		float distance = length(position - worldPosition);

		float r = distance / lightRange(light);
		intensity = lightIntensity(light) * 1.0 / (1.0 + 25.0 * r * r);

		//intensity = lightIntensity(light) * (lightRange(light) / distance - 1);
		direction = normalize(position - worldPosition);
	}

	// Apply diffuse lighting.
	float diff = max(dot(normal, direction), 0.0);
	vec3 diffuse = diff * DiffuseColour * lightColour(light) * intensity;

	// Apply specular lighting.
	vec3 viewDirection = normalize(VectorArr[VEC_VIEW_POSITION].xyz - worldPosition);
	vec3 reflectDirection = reflect(-direction, normal);
	float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), Shininess);
	vec3 specular = spec * SpecularColour * lightColour(light) * intensity;

	// Return combined diffuse and specular lighting caused by the light instance.
	return diffuse + specular;
}

void main()
{
	// Apply ambient lighting.
	vec3 colour = AmbientLight * DiffuseColour;

	// Apply each light affecting this fragment.
	for (int i = 0; i < NumLights; i++) {
		colour += ApplyLight(i);
	}

	// Apply fragment colour.
	gl_FragColor = vec4(colour, 1.0) * texture(SamplerArr[SAMPLER_MAIN], texCoord.st);
}

#endif
