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

float _ChromAberrAmountX = 0.0;
float _ChromAberrAmountY = 0.0;
vec4 _DisplacementAmount = vec4(0.1, 0.1, 0.1, 0.1);
float _DesaturationAmount = 0.1;
float _RightStripesAmount = 0.1;
float _RightStripesFill = 0.7;
float _LeftStripesAmount = 0.1;
float _LeftStripesFill = 0.7;
float _WavyDisplFreq = 0.1;
float _GlitchEffect = 0.3;

float rand(vec2 co)
{
	return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
	vec2 _ChromAberrAmount = vec2(_ChromAberrAmountX, _ChromAberrAmountY);

	vec4 displAmount = vec4(0, 0, 0, 0);
	vec2 chromAberrAmount = vec2(0, 0);
	float rightStripesFill = 0;
	float leftStripesFill = 0;
	//Glitch control
	if (fract(_GlitchEffect) < 0.8) {
		rightStripesFill = mix(0, _RightStripesFill, fract(_GlitchEffect) * 2);
		leftStripesFill = mix(0, _LeftStripesFill, fract(_GlitchEffect) * 2);
	}
	if (fract(_GlitchEffect) < 0.5) {
		chromAberrAmount = mix(vec2(0, 0), _ChromAberrAmount.xy, fract(_GlitchEffect) * 2);
	}
	if (fract(_GlitchEffect) < 0.33) {
		displAmount = mix(vec4(0,0,0,0), _DisplacementAmount, fract(_GlitchEffect) * 3);
	}

	//Stripes section
	float stripesRight = floor(texCoord.y * _RightStripesAmount);
	stripesRight = step(rightStripesFill, rand(vec2(stripesRight, stripesRight)));

	float stripesLeft = floor(texCoord.y * _LeftStripesAmount);
	stripesLeft = step(leftStripesFill, rand(vec2(stripesLeft, stripesLeft)));
	//Stripes section

	vec4 wavyDispl = mix(vec4(1,0,0,1), vec4(0,1,0,1), (sin(texCoord.y * _WavyDisplFreq) + 1) / 2);

	//Displacement section
	vec2 displUV = (displAmount.xy * stripesRight) - (displAmount.xy * stripesLeft);
	displUV += (displAmount.zw * wavyDispl.r) - (displAmount.zw * wavyDispl.g);
	//Displacement section

	//Chromatic aberration section
	float chromR = texture2D(getTexture(), texCoord + displUV + chromAberrAmount).r;
	float chromG = texture2D(getTexture(), texCoord + displUV).g;
	float chromB = texture2D(getTexture(), texCoord + displUV - chromAberrAmount).b;
	//Chromatic aberration section

	gl_FragColor = vec4(chromR, chromG, chromB, 1);
}

#endif
