#pragma include inc/mylly.glinc

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
	gl_FragColor = colour * texture(SamplerArr[SAMPLER_MAIN], texCoord.st);
}

#endif
