#pragma include inc/mylly.glinc
#pragma include inc/lighting.glinc

varying vec3 worldPosition;
varying vec3 worldNormal;
varying vec2 texCoord;

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

// Uniforms for Phong lighting.
uniform vec4 DiffuseColour;
uniform vec4 SpecularColour;
uniform float Shininess;
uniform float Opacity;

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
	gl_FragColor = vec4(colour, Opacity) * texture2D(TextureMain(), texCoord.st);
}

#endif
