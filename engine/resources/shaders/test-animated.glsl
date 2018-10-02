#pragma include inc/mylly.glinc

varying vec2 texCoord;
varying vec4 colour;

#if defined(VERTEX_SHADER)

const int frames = 4;
const float animSpeed = 2.0;

void main()
{
	gl_Position = MatrixMVP * Vertex;
	
	texCoord = TexCoord;
	colour = Colour;

	// Calculate animation frame.
	int frame = (int(frames * animSpeed * Time.x) % frames);
	float width = 1.0 / frames;

	texCoord.x = (frame * width) + texCoord.x * width;
}

#elif defined(FRAGMENT_SHADER)

void main()
{
	gl_FragColor = colour * texture(Texture, texCoord.st);
}

#endif
