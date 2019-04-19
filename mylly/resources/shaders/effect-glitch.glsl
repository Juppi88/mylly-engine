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

// Total glitch effect amount. Only the fractional part will matter.
float GlitchEffect = 0;

// Chromatic aberration effect strength.
float ChromaticAberrX = 0.01;
float ChromaticAberrY = 0.0025;

// Displacement and waves. First two elements of the vector affect stripe displacement, the last two
// are for wave displacement.
vec4 Displacement = vec4(-0.01, 0.01, -0.0025, 0.0025);
float WaveEffectFreq = 200;

// Horizontal stripes. The elements are: Left/right stripes frequency, left/right stripes fill.
vec4 Stripes = vec4(100, 70, 0.3, 0.4);

float rand(vec2 co)
{
	return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
	GlitchEffect = 0.5 + 0.5 * sin(4.5 * time()) + time();

	float rightStripesFill = 0;
	float leftStripesFill = 0;
	vec2 chromaticAberr = vec2(ChromaticAberrX, ChromaticAberrY);
	vec2 chromAberrAmount = vec2(0, 0);
	vec4 displAmount = vec4(0, 0, 0, 0);

	// Calculate multipliers for different effects based on the overall glitch factor.
	float factor = fract(GlitchEffect);

	if (factor < 0.7) {
		leftStripesFill =  mix(0, Stripes.z, factor * 2);
		rightStripesFill = mix(0, Stripes.w, factor * 2);
	}
	if (factor < 0.5) {
		chromAberrAmount = mix(vec2(0, 0), chromaticAberr, factor * 2);
	}
	if (factor < 0.5) {
		displAmount = mix(vec4(0, 0, 0, 0), Displacement, factor * 2);
	}

	// Calculate stripes.
	float stripesLeft = floor(texCoord.y * Stripes.x);
	stripesLeft = step(leftStripesFill, rand(vec2(stripesLeft, stripesLeft)));

	float stripesRight = floor(texCoord.y * Stripes.y);
	stripesRight = step(rightStripesFill, rand(vec2(stripesRight, stripesRight)));

	vec4 wavyDispl = mix(
		vec4(1, 0, 0, 1),
		vec4(0, 1, 0, 1),
		(sin(texCoord.y * WaveEffectFreq) + 1) / 2
	);

	// Calculate displacement.
	vec2 displCoord = (displAmount.xy * stripesRight) - (displAmount.xy * stripesLeft);
	displCoord += (displAmount.zw * wavyDispl.r) - (displAmount.zw * wavyDispl.g);

	// Calculate chromatic aberration.
	float r = texture2D(getTexture(), texCoord + displCoord + chromAberrAmount).r;
	float g = texture2D(getTexture(), texCoord + displCoord).g;
	float b = texture2D(getTexture(), texCoord + displCoord - chromAberrAmount).b;

	gl_FragColor = vec4(r, g, b, 1);
}

#endif
