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
	vec3 worldPosition = decodeworldpos(texCoord);
	vec3 diffuse = texture2D(TextureMain(), texCoord).rgb;
	vec3 normal = texture2D(TextureNormal(), texCoord).rgb;
	vec4 specularData = texture2D(TextureSpecular(), texCoord);
	vec3 specular = specularData.rgb;
	float shininess = specularData.a;

	// Apply ambient lighting.
	vec3 colour = ApplyAmbientLight(diffuse);

	// Apply the effect of the light.
	colour += ApplyPhongLight(0, worldPosition, normal, diffuse, specular, shininess);

	gl_FragColor = vec4(colour, 1);
}

#endif
