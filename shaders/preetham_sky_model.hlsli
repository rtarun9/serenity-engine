// clang-format off

#ifndef __PREETHAM_SKY_MODEL_HLSLI__
#define __PREETHAM_SKY_MODEL_HLSLI__

// An implementaiton of the paper "A practical analytic model for daylight"
// by A.J.Preetham, Peter Shirley, and Brian Smits (https://courses.cs.duke.edu/fall01/cps124/resources/p91-preetham.pdf).

// The paper uses Perez et al.'s model to get the sky luminance distribution.
// It is a five parameter model, where the params A, B, C, D and E are : 
// A : Darkening or Brightening of the Horizion
// B : Luminance gradient near the horizon.
// C : Relative intensity of Circumsolar region.
// D : Width of circumsolar region.
// E : Relative backscattered light.
// The formula : 
// F(theta, gamma) = (1 + A * e^(B / cos(theta)))(1 + C*e^(D*gamma) + E(cos(gamma)^2)). [3 in paper]
// Gamma and theta are described in the paper in Figure 4.
// Gamma is angle between viewing direction and sun direction.
// Theta is angle between zenith line (i.e line above user to atmosphere) and view direction.
// The luminance Y for sky in any viewing direction is given by : 
// Y = (Zenith Luminance) * F(theta, gamma) / F(0, thetas) [4 in paper]
// Zenith Luminance is represented by Yz, and thetas is the angle between sun direction and zenith line.

// [8 in paper] gives the integral which upon solving we get the total light scattered into viewing directection, given by:
// L(in) = [Integral]S1(..)(u1(x))T(0..x)dx +  [Integral]S2(..)(u2(x))T(0..x)dx 
// S1 is for molecules (wavelength smaller than light, due to which scattering varies per wavelength)
// while S2 is for Haze.
// T(0..x) is the extinction factor, which basically means the amount of light scattered from source to reaching us
// u(x) and T both work on principle that scattering is inversely proportional to height (as number of molecules / other particle decreases with increase in elevation).
// Table 2 in appendix gives various values for S1, S2, etc.

float4 preetham_sky_model(const float3 camera_position, const float3 sun_position, const float3 world_space_pixel_position)
{
    // Code and angles are from Figure [4] in the paper.
    // The pixel_position is in world space, so the zenight line is line going above the pixel in 3D coords.
    const float3 zenith = normalize(float3(0.0f, world_space_pixel_position.y, 0.0f));
    const float3 pixel_to_sun_direction = normalize(sun_position - world_space_pixel_position);
    const float3 pixel_to_camera_direction = normalize(camera_position - world_space_pixel_position);

    // Angle from zenith to view direction (v) is given by theta.
    const float theta = acos(dot(zenith, pixel_to_camera_direction));

    // Angle from zenith to sun is given by theta(s).
    const float theta_s = acos(dot(zenith, pixel_to_sun_direction));

    // Gamma is the angle between sun direction and camera (view) direction.
    const float gamma = acos(dot(pixel_to_camera_direction, pixel_to_sun_direction));
}

#endif