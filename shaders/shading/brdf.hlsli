#ifndef __BRDF__HLSLI__
#define __BRDF__HLSLI__

#include "utils.hlsli"

// Compute the ratio of reflected light vs how much it refracts.
// As the viewing angle increases, this ratio increases as well (quickly approaching one when angle becomes more and
// more oblique). f0 is the base reflectivity : the surface reflection at zero incidence. For non metals, it will just
// be a singular value in a float3 (v, v, v), but this is tinted for metals. Most dielectrics have a value of 0.04 as
// f0, but depending on how metallic a surface is it will be between 0.04 (metalness = 0) and the surface color
// (metalness = 1). cosTheta here is the angle between the halfway vector and the view direction. If the angle is 0.0,
// then said ratio is 1, and the light will be brightest here. Also acts as the kS term (where kS + kD = 1, due to
// energy conservation).

float3 fresnel_schlick_function(const float v_dot_h, const float3 f0)
{
    return f0 + (1.0f - f0) * pow(clamp(1.0f - v_dot_h, 0.0f, 1.0f), 5.0f);
}

// Approximates the number of microfacts on the surface whose local normals are aligned with the half way vector. For
// light to reflect from the surface (diffuse or specular) and reach our camera, the normal and halfway vector have to
// be aligned. More rough a surface is, more chaotically aligned the surface normals will be, producing large and dim
// highlights, while very smooth surfaces will produce very sharp and bright highlights since majority of microfacet
// normals are aligned to half way vector. This is the GGX TrowBridge Reitx model.
float ggx_trowbridge_normal_distribution_function(const float3 normal, const float3 h, const float roughness_factor)
{
    float alpha = roughness_factor * roughness_factor;
    float alpha_square = alpha * alpha;

    float n_dot_h = saturate(dot(normal, h));

    return alpha_square / (max(PI * pow((n_dot_h * n_dot_h * (alpha_square - 1.0f) + 1.0f), 2.0f), MIN_FLOAT_VALUE));
}

// Geometry function : approximates the number / relative surface area of the surface which is actually visible to us.
// If the surface is rough, several microfacets could overshadow and block others, because of which the light reaching
// us will be occluded. Using Smith's method, by changing the angle, we can approximate both self shadowing and geometry
// obstruction. Source :https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf if x
// is viewDirection, then we are calculating geometric obstruction, and if light direction, we are calculating self
// shadowing.
float schlick_beckmann_geometry_function(const float3 normal, const float3 x, const float roughness_factor)
{
    float k = roughness_factor / 2.0f;
    float n_dot_x = saturate(dot(normal, x));

    return n_dot_x / (max((n_dot_x * (1.0f - k) + k), MIN_FLOAT_VALUE));
}

// Smiths method is used for approximation of geometry (both self shadowing and geometry obstruction). (ShlickGGX
// model). Uses SchlickBeckman formula to calculate both geometry obstruction, where the camera cannot see a point as
// some other microfacet is blocking it, or Self shadowing, where the light ray from a point is not able to reach the
// camera.
float smith_geometry_function(const float3 normal, const float3 view_direction, const float3 light_direction, const float roughness_factor)
{
    return schlick_beckmann_geometry_function(normal, view_direction, roughness_factor) *
           schlick_beckmann_geometry_function(normal, light_direction, roughness_factor);
}

// BRDF used here : Lambertian Diffuse BRDF + Cook Torrence Specular BRDF.
// BRDF = kD * diffuseBRDF + kS * specularBRDF. (Note : kS + kD = 1).

float3 lambertian_diffuse_BRDF(const float3 normal, const float3 view_direction, const float3 pixel_to_light_direction, const float3 albedo, const float roughness_factor,
            const float metallic_factor)
{
    const float3 h = normalize(view_direction + pixel_to_light_direction);

    const float3 f0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo.xyz, metallic_factor);

    const float3 fresnel = fresnel_schlick_function(max(dot(view_direction, h), 0.0f), f0);
    
    float3 kS = fresnel;

    // Metals have kD as 0.0f, so more metallic a surface is, closes kS ~ 1 and kD ~ 0.
    // Using lambertian model for diffuse light now.
    float3 kD = lerp(float3(1.0f, 1.0f, 1.0f) - fresnel, float3(0.0f, 0.0f, 0.0f), metallic_factor);

    return (kD * albedo) / PI;
}

float3 cook_torrence_specular_BRDF(const float3 normal, const float3 view_direction, const float3 pixel_to_light_direction, const float3 albedo, const float roughness_factor,
            const float metallic_factor)
{
    const float3 h = normalize(view_direction + pixel_to_light_direction);

    const float3 f0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo.xyz, metallic_factor);

    const float3 fresnel = fresnel_schlick_function(max(dot(view_direction, h), 0.0f), f0);
    
    const float n = ggx_trowbridge_normal_distribution_function(normal, h, roughness_factor);
    const float g = smith_geometry_function(normal, view_direction, pixel_to_light_direction, roughness_factor);

    return 
        (n * g * fresnel) /
        max(4.0f * saturate(dot(view_direction, normal)) * saturate(dot(pixel_to_light_direction, normal)),
            MIN_FLOAT_VALUE);
}

#endif