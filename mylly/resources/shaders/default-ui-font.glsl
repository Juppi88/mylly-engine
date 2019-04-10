#define VERTEX_UI
#pragma include inc/mylly.glinc
#pragma queue OVERLAY

varying vec2 texCoord;
varying vec4 colour;

#if defined(VERTEX_SHADER)

void main()
{
	gl_Position = pixelsnap(toclipspace2(Vertex));
	
	texCoord = TexCoord;
	colour = Colour;
}

#elif defined(FRAGMENT_SHADER)

void main()
{
	// The font texture is a greyscale texture where the data is encoded into the red channel.
	// Use the red channel as alpha and keep every other channel as 1.
	gl_FragColor = colour * vec4(1, 1, 1, texture(SamplerArr[SAMPLER_MAIN], texCoord.st).r);
}

#endif
