#define VERTEX_UI
#pragma include inc/mylly.glinc
#pragma queue OVERLAY

varying vec2 texCoord;
varying vec4 colour;

#if defined(VERTEX_SHADER)

void main()
{
	gl_Position = MatrixMVP * vec4(Vertex, 0, 1);
	gl_Position = pixelsnap(gl_Position);
	
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
