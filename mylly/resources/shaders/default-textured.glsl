#pragma include inc/mylly.glinc

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
	gl_FragColor = texture2D(TextureMain(), texCoord.st);
}

#endif
