#define DEFERRED_LIGHTING
#pragma include inc/mylly.glinc

varying vec2 texCoord;
varying vec3 normal;

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

// Uniforms for Phong lighting.
uniform vec4 DiffuseColour;
uniform vec4 SpecularColour;
uniform float Shininess;

void main()
{
	emit(
		texture2D(TextureMain(), texCoord) * vec4(DiffuseColour.rgb, 1),
		encodenormal(normal),
		vec4(SpecularColour.rgb, encodeshininess(Shininess))
	);
}

#endif
