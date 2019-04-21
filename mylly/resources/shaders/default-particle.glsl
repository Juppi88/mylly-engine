#define VERTEX_PARTICLE
#pragma include inc/mylly.glinc
#pragma queue TRANSPARENT

varying vec2 texCoord;
varying vec4 colour;

#if defined(VERTEX_SHADER)

void main()
{
	vec3 position = Vertex;
	position *= ParticleSize;
	position += ParticleCentre;

	gl_Position = toclipspace(position);

	// TODO: Orient towards the camera!
	
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
