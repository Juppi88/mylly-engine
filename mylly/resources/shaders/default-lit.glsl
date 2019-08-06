#pragma include inc/mylly.glinc

uniform vec4 DiffuseColour;
uniform vec4 SpecularColour;
uniform float Shininess;
uniform float Opacity;

varying vec2 texCoord;
varying vec3 normal;

#ifndef DEFERRED_LIGHTING

// -------------------------------------------------------------------------------------------------
// Forward lighting path 
// -------------------------------------------------------------------------------------------------

#pragma include inc/lighting.glinc

varying vec3 worldPosition;
varying vec3 worldNormal;

#if defined(VERTEX_SHADER)

void main()
{
	// Calculate vertex clip position.
	gl_Position = toclipspace(Vertex);

	// Varying arguments to fragment shader.
	worldPosition = (MatrixModel() * vec4(Vertex, 1)).xyz;
	texCoord = TexCoord;

	// Calculate world space normal.
	mat3 normalMatrix = transpose(inverse(mat3(MatrixModel())));
	worldNormal = normalize(normalMatrix * Normal);
}

#elif defined(FRAGMENT_SHADER)

void main()
{
	// Apply ambient lighting.
	vec3 colour = ApplyAmbientLight(DiffuseColour.rgb);

	// Apply each light affecting this fragment.
	for (int i = 0; i < NumLights; i++) {

		colour += ApplyPhongLight(i, worldPosition, normalize(worldNormal),
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

	// Calculate world space normal.
	mat3 normalMatrix = transpose(inverse(mat3(MatrixModel())));
	normal = normalize(normalMatrix * Normal);
}

#elif defined(FRAGMENT_SHADER)

void main()
{
	emit(
		texture2D(TextureMain(), texCoord) * vec4(DiffuseColour.rgb, 1),
		encodenormal(normal),
		vec4(SpecularColour.rgb, encodeshininess(Shininess))
	);
}

#endif

#endif // DEFERRED_LIGHTING
