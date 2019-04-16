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

float strength = 0.0015;
vec2 rOffset = strength * vec2(1, 1);
vec2 gOffset = strength * vec2(1, -1);
vec2 bOffset = strength * vec2(-1, 1);

void main()
{
	vec4 col = texture(SamplerArr[SAMPLER_MAIN], texCoord.st);

	float r = texture2D(SamplerArr[SAMPLER_MAIN], texCoord - rOffset).r;
    float g = texture2D(SamplerArr[SAMPLER_MAIN], texCoord - gOffset).g;
    float b = texture2D(SamplerArr[SAMPLER_MAIN], texCoord - bOffset).b;

    gl_FragColor = vec4(r, g, b, 1.0);
}

#endif
