#pragma include inc/mylly.glinc

varying vec2 texCoord;
varying vec4 colour;

#if defined(VERTEX_SHADER)

void main()
{
	gl_Position = MatrixMVP * Vertex;
	
	texCoord = TexCoord;
	colour = Colour;
}

#elif defined(FRAGMENT_SHADER)

void main()
{
	gl_FragColor = colour * texture(Texture, texCoord.st);

	// Alpha cutoff
	if (gl_FragColor.a < 0.01) {
		discard;
	}
}

#endif
