#pragma include inc/mylly.glinc
#pragma queue TRANSPARENT

varying vec2 texCoord;
varying vec4 colour;

#if defined(VERTEX_SHADER)

void main()
{
	vec4 position = Vertex;
	position *= 0.1;
	position.w = 1;
	position += vec4(Normal, 0);

	gl_Position = MatrixMVP * position;
	
	texCoord = TexCoord;
	colour = Colour;

	//colour = vec4(Normal.x / 10, Normal.y / 10, 0, 1);
}

#elif defined(FRAGMENT_SHADER)

void main()
{
	gl_FragColor = colour * texture(Texture, texCoord.st);

	// Alpha cutoff
	alphacut(gl_FragColor.a, 0.01);
}

#endif
