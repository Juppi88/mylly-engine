#pragma include inc/mylly.glinc
#pragma queue TRANSPARENT

varying vec2 texCoord;
varying vec4 colour;

#if defined(VERTEX_SHADER)

void main()
{
	gl_Position = toclipspace(Vertex);
	
	texCoord = TexCoord;
	colour = vec4(1, 1, 1, 1);
}

#elif defined(FRAGMENT_SHADER)

void main()
{
	gl_FragColor = colour * texture(Texture, texCoord.st);

	// Alpha cutoff
	alphacut(gl_FragColor.a, 0.01);
}

#endif
