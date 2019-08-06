#pragma include inc/mylly.glinc

uniform vec4 DiffuseColour;
uniform vec4 SpecularColour;
uniform float Shininess;
uniform float Opacity;

varying vec2 texCoord;
varying mat3 tangentMatrix;

#ifndef DEFERRED_LIGHTING

// -------------------------------------------------------------------------------------------------
// Forward lighting path 
// -------------------------------------------------------------------------------------------------

#pragma include inc/lighting.glinc

varying vec3 worldPosition;

#if defined(VERTEX_SHADER)

void main()
{
	// Calculate vertex clip position.
	gl_Position = toclipspace(Vertex);

	// Varying arguments to fragment shader.
	worldPosition = (MatrixModel() * vec4(Vertex, 1)).xyz;
	texCoord = TexCoord;

	// Calculate world space tangent matrix for normal map calculations.
	vec3 tangentVec = normalize(vec3(MatrixModel() * vec4(Tangent, 0)));
	vec3 normalVec = normalize(vec3(MatrixModel() * vec4(Normal, 0)));
	tangentVec = normalize(tangentVec - dot(tangentVec, normalVec) * normalVec);
	vec3 biTangentVec = cross(tangentVec, normalVec);

	tangentMatrix = mat3(tangentVec, biTangentVec, normalVec);
}

#elif defined(FRAGMENT_SHADER)

void main()
{
	// Apply ambient lighting.
	vec3 colour = ApplyAmbientLight(DiffuseColour.rgb);

	// Retrieve surface normal from the normal map and transform it to world space coordinates.
	vec3 normal = texture2D(TextureNormal(), texCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(tangentMatrix * normal);

	// Apply each light emitter affecting this fragment.
	for (int i = 0; i < NumLights; i++) {

		colour += ApplyPhongLight(i, worldPosition, normal,
		                          DiffuseColour.rgb, SpecularColour.rgb, Shininess);
	}

	// Apply fragment colour.
	vec4 result = vec4(colour, Opacity) * texture2D(TextureMain(), texCoord);
	emit(result);
}

#endif

#else // DEFERRED_LIGHTING

// -------------------------------------------------------------------------------------------------
// Deferred lighting path 
// -------------------------------------------------------------------------------------------------

#if defined(VERTEX_SHADER)

void main()
{
	gl_Position = toclipspace(Vertex);
	texCoord = TexCoord;

	// Calculate world space tangent matrix for normal map calculations.
	vec3 tangentVec = normalize(vec3(MatrixModel() * vec4(Tangent, 0)));
	vec3 normalVec = normalize(vec3(MatrixModel() * vec4(Normal, 0)));
	tangentVec = normalize(tangentVec - dot(tangentVec, normalVec) * normalVec);
	vec3 biTangentVec = cross(tangentVec, normalVec);

	tangentMatrix = mat3(tangentVec, biTangentVec, normalVec);
}

#elif defined(FRAGMENT_SHADER)

void main()
{
	// Retrieve surface normal from the normal map and transform it to world space coordinates.
	vec3 normal = texture2D(TextureNormal(), texCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(tangentMatrix * normal);

	emit(
		texture2D(TextureMain(), texCoord) * vec4(DiffuseColour.rgb, 1),
		encodenormal(normal),
		vec4(SpecularColour.rgb, encodeshininess(Shininess))
	);
}

#endif

#endif // DEFERRED_LIGHTING
