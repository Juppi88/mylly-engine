#define VERTEX_UI
#define DEFERRED_LIGHTING
#pragma include inc/mylly.glinc
#pragma include inc/lighting.glinc

varying vec2 texCoord;

#if defined(VERTEX_SHADER)

void main()
{
	gl_Position = vec4(Vertex, 0, 1);
	texCoord = TexCoord;
}

#elif defined(FRAGMENT_SHADER)

void main()
{
	// Apply ambient lighting.
	vec3 diffuse = texture2D(TextureMain(), texCoord).rgb;
	vec3 colour = ApplyAmbientLight(diffuse);

	gl_FragColor = vec4(colour, 1);
}

#endif
