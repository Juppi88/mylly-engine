#pragma include inc/mylly.glinc

varying vec2 texCoord;
varying vec4 colour;

#if defined(VERTEX_SHADER)

void main()
{
	gl_Position = MatrixMVP * Vertex;
	
	texCoord = TexCoord;
	colour = Colour;
}

#elif defined(FRAGMENT_SHADER)

void main()
{
	if (texCoord.s < 0.01 || texCoord.s > 0.99 ||
		texCoord.t < 0.01 || texCoord.t > 0.99) {
		gl_FragColor = vec4(0, 0, 0, 1);
	}
	else {
		gl_FragColor = vec4(abs(Time.y), abs(Time.z), 0, 1);
	}
}

#endif
