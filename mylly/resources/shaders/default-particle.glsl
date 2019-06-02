#define VERTEX_PARTICLE
#pragma include inc/mylly.glinc
#pragma queue TRANSPARENT

varying vec2 texCoord;
varying vec4 colour;

#if defined(VERTEX_SHADER)

void main()
{
	// Convert particle centre position to world space.
	vec3 position = mat3(MatrixModel()) * ParticleCentre;

	// Rotate towards the camera (billboard).
	mat4 view = MatrixView();
	vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
	vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]);

	position +=
	    cameraRight * Vertex.x * ParticleSize +
	    cameraUp * Vertex.y * ParticleSize;

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
