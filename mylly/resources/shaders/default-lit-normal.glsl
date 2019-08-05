#define DEFERRED_LIGHTING
#pragma include inc/mylly.glinc

varying vec2 texCoord;
varying mat3 tangentMatrix;

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

// Uniforms for Phong lighting.
uniform vec4 DiffuseColour;
uniform vec4 SpecularColour;
uniform float Shininess;

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
