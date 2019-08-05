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
	// Fetch material properties from G-buffer data.
	vec3 source = texture2D(TextureMain(), texCoord).rgb;
	vec3 worldPosition = decodeworldpos(texCoord);
	vec3 normal = decodenormal(texture2D(TextureNormal(), texCoord).rgb);
	vec3 diffuse = texture2D(TextureDiffuse(), texCoord).rgb;
	vec4 specularData = texture2D(TextureSpecular(), texCoord);
	vec3 specular = specularData.rgb;
	float shininess = decodeshininess(specularData.a);

	// Apply the effect of the light.
	vec3 colour = source + ApplyPhongLight(0, worldPosition, normal, diffuse, specular, shininess);

	gl_FragColor = vec4(colour, 1);
}

#endif