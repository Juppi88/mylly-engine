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

const int GridSize = 6;

uniform float Alpha; // Colour selector alpha
uniform float Brightness; // Colour selector brightness (in HSV space)

// World position of the widget (in pixels)
uniform float WidgetPosX;
uniform float WidgetPosY;

vec3 hsv2rgb(vec3 c)
{
	vec4 k = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + k.xyz) * 6.0 - k.www);

	return c.z * mix(k.xxx, clamp(p - k.xxx, 0.0, 1.0), c.y);
}

void main()
{
	// Discard off-sheet pixels.
	if (texCoord.s < 0 || texCoord.t < 0 ||
		texCoord.s > 1 || texCoord.t > 1) {
		
		discard;
	}

	// Change the brightness of the texture.
	vec3 hsvBrightness = vec3(0, 0, Brightness);
	vec3 rgbBrightness = hsv2rgb(hsvBrightness);

	// Generate the background squares.
	int x = int(gl_FragCoord.x - WidgetPosX);
	int y = int(gl_FragCoord.y - WidgetPosY);
	int evenRow = ((y / GridSize) & 1);

	float square = 1 - 0.5 * (evenRow + (x / GridSize) & 1);

	vec3 bgColour = vec3(square);
	vec3 texColour = colour.rgb * rgbBrightness * texture2D(TextureMain(), texCoord.st).xyz;

	// Alpha blend the widget's texture with the background squares.
	vec3 outColour = (1.0 - Alpha) * bgColour + Alpha * texColour;

	emit(vec4(outColour, 1));
}

#endif
