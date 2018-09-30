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
	gl_FragColor = colour * texture(Texture, texCoord.st);
}

#endif
