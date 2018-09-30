uniform mat4 MatrixModel;
uniform mat4 MatrixMVP;
uniform sampler2D Texture;

varying vec2 texCoord;
varying vec4 colour;

#if defined(VERTEX_SHADER)

attribute vec4 Vertex;
attribute vec3 Normal;
attribute vec4 Colour;
attribute vec2 TexCoord;

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
		gl_FragColor = vec4(1, 0, 0, 1);
	}
	else {
		gl_FragColor = vec4(1, 1, 1, 1);
	}
}

#endif
