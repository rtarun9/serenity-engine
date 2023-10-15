// clang-format off

#ifndef __UTILS_HLSLI__
#define __UTILS_HLSLI__

static const float MIN_FLOAT_VALUE = 0.00001f;
static const float EPSILON = 1.0e-4;

static const float PI = 3.14159265359;
static const float TWO_PI = 2.0f * PI;
static const float INV_PI = 1.0f / PI;
static const float INV_TWO_PI = 1.0f / TWO_PI;
static const float INVALID_INDEX = 4294967295; // UINT32_MAX;

float3 get_sampling_vector_cubemap(float2 uv, uint3 dispatch_thread_id)
{
    uv = uv = 2.0f * float2(uv.x, 1.0f - uv.y) - float2(1.0f, 1.0f);

    float3 samplingVector = float3(0.0f, 0.0f, 0.0f);

    // Based on cube face 'index', choose a vector.
    switch (dispatch_thread_id.z)
    {
    case 0:
        return normalize(float3(1.0, uv.y, -uv.x));
        break;
    case 1:
        return normalize(float3(-1.0, uv.y, uv.x));
        break;
    case 2:
        return normalize(float3(uv.x, 1.0, -uv.y));
        break;
    case 3:
        return normalize(float3(uv.x, -1.0, uv.y));
        break;
    case 4:
        return normalize(float3(uv.x, uv.y, 1.0));
        break;
    case 5:
        return normalize(float3(-uv.x, uv.y, -1.0));
        break;
    }

    // Code control flow should never reach here.
    return float3(0.0f, 0.0f, 0.0f);
}

// Source : https://www.reedbeta.com/blog/quick-and-easy-gpu-random-numbers-in-d3d11/
// and : https://github.com/acmarrs/ColorBanding/blob/master/shaders/Common.hlsl
uint wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

uint rand_xor_shift(uint seed)
{
    // Xorshift algorithm from George Marsaglia's paper
    seed ^= (seed << 13);
    seed ^= (seed >> 17);
    seed ^= (seed << 5);
    return seed;
}

float random_float_in_range_0_1(inout uint seed)
{
    seed = wang_hash(seed);
    return float(rand_xor_shift(seed)) * (1.f / 4294967296.f);
}
#endif