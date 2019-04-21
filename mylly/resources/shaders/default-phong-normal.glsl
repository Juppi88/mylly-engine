#pragma include inc/mylly.glinc
#pragma include inc/lighting.glinc

varying vec3 worldPosition;
varying vec2 texCoord;
varying mat3 tangentMatrix;

#if defined(VERTEX_SHADER)

void main()
{
	// Calculate vertex clip position.
	gl_Position = toclipspace(Vertex);

	// Varying arguments to fragment shader.
	worldPosition = (MatrixModel() * vec4(Vertex, 1)).xyz;
	texCoord = TexCoord;

	// Calculate tangent matrix for normal map calculations.
	vec3 tangentVec = normalize(vec3(MatrixModel() * vec4(Tangent, 0)));
	vec3 normalVec = normalize(vec3(MatrixModel() * vec4(Normal, 0)));
	tangentVec = normalize(tangentVec - dot(tangentVec, normalVec) * normalVec);
	vec3 biTangentVec = cross(normalVec, tangentVec);

	tangentMatrix = mat3(tangentVec, biTangentVec, normalVec);
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

	// Apply each light emitter affecting this fragment.
	for (int i = 0; i < NumLights; i++) {

		// Retrieve surface normal from the normal map and transform it to the model's orientation.
		vec3 normal = texture2D(TextureNormalMap(), texCoord).rgb;
		normal = normalize(normal * 2.0 - 1.0);
		normal = normalize(tangentMatrix * normal);

		colour += ApplyPhongLight(i, worldPosition, normal,
		                          DiffuseColour.rgb, SpecularColour.rgb, Shininess);
	}

	// Apply fragment colour.
	gl_FragColor = vec4(colour, Opacity) * texture2D(TextureMain(), texCoord.st);
}

#endif
