// clang-format off

#ifndef __TONE_MAPPERS_HLSLI__
#define __TONE_MAPPERS_HLSLI__

// Aces Narkowicz approximation (to convert HDR to LDR).
// Formula from : https://64.github.io/tonemapping/
float3 aces_approximation(float3 hdr_color)
{
	hdr_color *= 0.6f;

	const float a = 2.51f;
	const float b = 0.03f;
	const float c = 2.43f;
	const float d = 0.59f;
	const float e = 0.14f;

	return clamp((hdr_color * (a * hdr_color + b)) / (hdr_color * (c * hdr_color + d) + e), 0.0f, 1.0f);
}

// Reinhard-Jodie tone mapping
// Formula from : https://64.github.io/tonemapping/
float3 reinhard_jodie(float3 hdr_color)
{
	float luminance = dot(hdr_color, float3(0.2126f, 0.7152f, 0.0722f));
	float3 tv = hdr_color / (1.0f + hdr_color);

	return lerp(hdr_color / (1.0f + luminance), tv, tv);
}

// Aces
// Formula from : https://64.github.io/tonemapping/
float3 aces(float3 v)
{
    // aces_input_matrix multiplied with v
	v = float3(dot(float3(0.59719f, 0.35458f, 0.04823f), v), dot(float3(0.07600f, 0.90834f, 0.01566f), v),
               dot(float3(0.02840f, 0.13383f, 0.83777f), v));

    // rtt and odt fit
	float3 a = v * (v + 0.0245786f) - 0.000090537f;
	float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;

	v = a / b;

    // aces_output_matrix multiplied with v
	v = float3(dot(float3(1.60475f, -0.53108f, -0.07367f), v), dot(float3(-0.10208f, 1.10813f, -0.00605f), v),
               dot(float3(-0.00327f, -0.07276f, 1.07602f), v));

	return v;
}

#endif