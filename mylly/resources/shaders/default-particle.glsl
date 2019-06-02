#define VERTEX_PARTICLE
#pragma include inc/mylly.glinc
#pragma queue TRANSPARENT

varying vec2 texCoord;
varying vec4 colour;

#if defined(VERTEX_SHADER)

void main()
{
	// Convert particle centre position to world space.
	mat4 model = MatrixModel();
	vec4 worldCentre = model * vec4(ParticleCentre, 1);

	// Rotate towards the camera (billboard).
	vec3 position =
		worldCentre.xyz +
	    CameraRight() * Vertex.x * ParticleSize +
	    CameraUp() * Vertex.y * ParticleSize;

	// World space particles.
	if (ParticleEmitPosition.w != 0) {
		position += ParticleEmitPosition.xyz - ObjWorldPosition();
	}

	// Convert to clip space.
	gl_Position = MatrixProjection() * MatrixView() * vec4(position, 1);

	texCoord = TexCoord;
	colour = Colour;
}

#elif defined(FRAGMENT_SHADER)

void main()
{
	gl_FragColor = colour * texture2D(TextureMain(), texCoord.st);

	// Alpha cutoff
	alphacut(gl_FragColor.a, 0.01);
}

#endif
