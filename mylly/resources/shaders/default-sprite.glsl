#pragma include inc/mylly.glinc
#pragma queue TRANSPARENT

uniform vec4 Colour;

varying vec2 texCoord;

#if defined(VERTEX_SHADER)

void main()
{
	gl_Position = toclipspace(Vertex);
	texCoord = TexCoord;
}

#elif defined(FRAGMENT_SHADER)

void main()
{
	vec4 colour = Colour * texture2D(TextureMain(), texCoord);

	emit(colour);

	// Alpha cutoff
	alphacut(colour.a, 0.01);
}

#endif
