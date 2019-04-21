#define VERTEX_UI
#pragma include inc/mylly.glinc

varying vec2 texCoord;

#if defined(VERTEX_SHADER)

void main()
{
	gl_Position = vec4(Vertex, 0, 1);
	texCoord = TexCoord;
}

#elif defined(FRAGMENT_SHADER)

float strength = 0.0025;

void main()
{
	vec2 offset = vec2(strength, strength);

	float r = texture2D(TextureMain(), texCoord - offset).r;
	float g = texture2D(TextureMain(), texCoord).g;
	float b = texture2D(TextureMain(), texCoord + offset).b;

	gl_FragColor = vec4(r, g, b, 1);
}

#endif
