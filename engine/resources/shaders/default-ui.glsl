uniform sampler2D Texture;
uniform mat4 MatrixMVP;

varying vec2 texCoord;
varying vec4 colour;

#if defined(VERTEX_SHADER)

// Attributes for UI widget quads.
attribute vec2 Vertex;
attribute vec4 Colour;
attribute vec2 TexCoord;

void main()
{
	gl_Position = MatrixMVP * vec4(Vertex, 0, 1);
	
	texCoord = TexCoord;
	colour = Colour;
}

#elif defined(FRAGMENT_SHADER)

void main()
{
	// Discard off-sheet pixels.
	if (texCoord.s < 0 || texCoord.t < 0 ||
		texCoord.s > 1 || texCoord.t > 1) {
		
		discard;
	}

	gl_FragColor = colour * texture(Texture, texCoord.st);
}

#endif
