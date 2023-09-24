// clang-format off

#ifndef __UTILS_HLSLI__
#define __UTILS_HLSLI__

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

#endif