#define VERTEX_UI
#pragma include inc/mylly.glinc

varying vec2 texCoord;
varying vec2 rgbNW;
varying vec2 rgbNE;
varying vec2 rgbSW;
varying vec2 rgbSE;
varying vec2 rgbM;

#if defined(VERTEX_SHADER)

void main()
{
	gl_Position = vec4(Vertex, 0, 1);
	texCoord = TexCoord;

	vec2 fragCoord = TexCoord * ScreenResolution();
	vec2 inverseVP = 1.0 / ScreenResolution();

	// Calculate texture coordinates in the vertex shader to avoid unnecessary texture reads.
	rgbNW = (fragCoord + vec2(-1.0, -1.0)) * inverseVP;
	rgbNE = (fragCoord + vec2(1.0, -1.0)) * inverseVP;
	rgbSW = (fragCoord + vec2(-1.0, 1.0)) * inverseVP;
	rgbSE = (fragCoord + vec2(1.0, 1.0)) * inverseVP;
	rgbM = vec2(fragCoord * inverseVP);
}

#elif defined(FRAGMENT_SHADER)

#define FXAA_REDUCE_MIN 1.0 / 128.0
#define FXAA_REDUCE_MUL 1.0 / 8.0
#define FXAA_SPAN_MAX   8.0

void main()
{
	vec4 colour;

	vec2 fragCoord = texCoord * ScreenResolution();
	vec2 inverseVP = vec2(1.0 / ScreenResolution().x, 1.0 / ScreenResolution().y);

	// Look up the fragment and others around it.
	vec3 rgbNW = texture2D(TextureMain(), rgbNW).xyz;
	vec3 rgbNE = texture2D(TextureMain(), rgbNE).xyz;
	vec3 rgbSW = texture2D(TextureMain(), rgbSW).xyz;
	vec3 rgbSE = texture2D(TextureMain(), rgbSE).xyz;
	vec4 texColor = texture2D(TextureMain(), rgbM);
	vec3 rgbM  = texColor.xyz;

	// Calculate luminances.
	vec3 luma = vec3(0.299, 0.587, 0.114);
	float lumaNW = dot(rgbNW, luma);
	float lumaNE = dot(rgbNE, luma);
	float lumaSW = dot(rgbSW, luma);
	float lumaSE = dot(rgbSE, luma);
	float lumaM  = dot(rgbM,  luma);
	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

	vec2 dir = vec2(
		-((lumaNW + lumaNE) - (lumaSW + lumaSE)),
		 ((lumaNW + lumaSW) - (lumaNE + lumaSE))
	);

	float dirReduce = max(
		(lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

	float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

	dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
	          dir * rcpDirMin)) * inverseVP;

	vec3 rgbA = 0.5 * (
	    texture2D(TextureMain(), fragCoord * inverseVP + dir * (1.0 / 3.0 - 0.5)).xyz +
	    texture2D(TextureMain(), fragCoord * inverseVP + dir * (2.0 / 3.0 - 0.5)).xyz);

	vec3 rgbB = rgbA * 0.5 + 0.25 * (
	    texture2D(TextureMain(), fragCoord * inverseVP + dir * -0.5).xyz +
	    texture2D(TextureMain(), fragCoord * inverseVP + dir * 0.5).xyz);

	float lumaB = dot(rgbB, luma);

	if (lumaB < lumaMin || lumaB > lumaMax) {
	    colour = vec4(rgbA, texColor.a);
	}
	else {
	    colour = vec4(rgbB, texColor.a);
	}
	
	emit(colour);
}

#endif
