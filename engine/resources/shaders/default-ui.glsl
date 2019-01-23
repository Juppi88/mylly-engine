uniform sampler2D Texture;

varying vec2 texCoord;
varying vec4 colour;

#if defined(VERTEX_SHADER)

// Attributes for UI widget quads.
attribute vec2 Vertex;
attribute vec4 Colour;
attribute vec2 TexCoord;

void main()
{
	// TODO: Move to uniform
	vec2 viewport = vec2(2560, 1440);

	gl_Position = vec4(2 * Vertex / viewport - 1, 0, 1);
	
	texCoord = TexCoord;
	colour = Colour;
}

#elif defined(FRAGMENT_SHADER)

void main()
{
	gl_FragColor = colour * texture(Texture, texCoord.st);
}

#endif
