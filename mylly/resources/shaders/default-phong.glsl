#pragma include inc/mylly.glinc

varying vec3 worldPosition;
varying vec3 worldNormal;
varying vec2 texCoord;

#if defined(VERTEX_SHADER)

void main()
{
	// Calculate vertex clip position.
	gl_Position = toclipspace(Vertex);

	// Varyuing arguments to fragment shader.
	vec4 position = MatrixArr[MAT_MODEL] * vec4(Vertex, 1);
	
	worldPosition = position.xyz;
	texCoord = TexCoord;

	// Calculate world space normal.
	mat3 normalMatrix = transpose(inverse(mat3(MatrixArr[MAT_MODEL])));
	worldNormal = normalize(normalMatrix * Normal);
}

#elif defined(FRAGMENT_SHADER)

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

	// Directional light, no attenuation.
	if (lightPosition(light).w == 0) {

		intensity = lightIntensity(light);
		direction = normalize(position);
	}

	// Spot or point light.
	else {
		float distance = length(position - worldPosition);
		float r = distance / lightRange(light);

		intensity = lightIntensity(light) * 1.0 / (1.0 + 25.0 * r * r);
		direction = normalize(position - worldPosition);
	}

	// Apply diffuse lighting.
	float diffuseStrength = max(dot(normal, direction), 0.0);
	vec3 diffuse = diffuseStrength * DiffuseColour;

	// Apply specular lighting.
	vec3 viewDirection = normalize(VectorArr[VEC_VIEW_POSITION].xyz - worldPosition);
	vec3 reflectDirection = reflect(-direction, normal);
	float specularStrength = pow(max(dot(viewDirection, reflectDirection), 0.0), Shininess);
	vec3 specular = specularStrength * SpecularColour;

	// Spotlight attenuation.
	if (lightCutoffAngle(light) > 0) {

		float theta = dot(-direction, normalize(lightDirection(light)));
		float epsilon = lightCutoffAngle(light) - lightCutoffOuterAngle(light);
		
		intensity *= clamp((theta - lightCutoffOuterAngle(light)) / epsilon, 0.0, 1.0);
	}

	// Return combined diffuse and specular lighting caused by the light instance.
	return intensity * (diffuse + specular) * lightColour(light);
}

void main()
{
	// Apply ambient lighting.
	vec3 colour = ambientLightColour() * DiffuseColour;

	// Apply each light affecting this fragment.
	for (int i = 0; i < NumLights; i++) {
		colour += ApplyLight(i);
	}

	if (lightPosition(1).y > 5) discard;

	// Apply fragment colour.
	gl_FragColor = vec4(colour, 1.0) * texture(SamplerArr[SAMPLER_MAIN], texCoord.st);
}

#endif
