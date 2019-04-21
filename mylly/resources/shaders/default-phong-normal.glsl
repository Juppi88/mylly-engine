#pragma include inc/mylly.glinc

varying vec3 worldPosition;
varying vec2 texCoord;
varying mat3 tangentMatrix;

#if defined(VERTEX_SHADER)

void main()
{
	// Calculate vertex clip position.
	gl_Position = toclipspace(Vertex);

	// Varyuing arguments to fragment shader.
	vec4 position = MatrixArr[MAT_MODEL] * vec4(Vertex, 1);
	
	worldPosition = position.xyz;
	texCoord = TexCoord;

	// Calculate tangent matrix for normal map calculations.
	vec3 tangentVec = normalize(vec3(MatrixArr[MAT_MODEL] * vec4(Tangent, 0)));
	vec3 normalVec = normalize(vec3(MatrixArr[MAT_MODEL] * vec4(Normal, 0)));
	tangentVec = normalize(tangentVec - dot(tangentVec, normalVec) * normalVec);
	vec3 biTangentVec = cross(normalVec, tangentVec);

	tangentMatrix = mat3(tangentVec, biTangentVec, normalVec);
}

#elif defined(FRAGMENT_SHADER)

uniform vec4 DiffuseColour;
uniform vec4 SpecularColour;
uniform float Shininess;
uniform float Opacity;

vec3 ApplyLight(int light)
{
	// Calculate light intensity at the fragment.
	vec3 position = lightPosition(light).xyz;
	vec3 direction;
	float intensity;

	// Get normal from normal map.
	vec3 normal = texture(SamplerArr[SAMPLER_NORMAL], texCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(tangentMatrix * normal);

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
	vec3 diffuse = diffuseStrength * DiffuseColour.rgb;

	// Apply specular lighting.
	vec3 viewDirection = normalize(VectorArr[VEC_VIEW_POSITION].xyz - worldPosition);
	vec3 reflectDirection = reflect(-direction, normal);
	float specularStrength = pow(max(dot(viewDirection, reflectDirection), 0.0), Shininess);
	vec3 specular = specularStrength * SpecularColour.rgb;

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
	vec3 colour = ambientLightColour() * DiffuseColour.rgb;

	// Apply each light affecting this fragment.
	for (int i = 0; i < NumLights; i++) {
		colour += ApplyLight(i);
	}

	// Apply fragment colour.
	gl_FragColor = vec4(colour, Opacity) * texture2D(getTexture(), texCoord.st);
 }

#endif
