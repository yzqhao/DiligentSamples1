
#ifndef FFX_SSSR_CLASSIFY_TILES
#define FFX_SSSR_CLASSIFY_TILES

Texture2D<float> g_roughness : register(t0);

RWBuffer<uint> g_tile_list : register(u0);
RWBuffer<uint> g_ray_list : register(u1);
globallycoherent RWBuffer<uint> g_tile_counter : register(u2);
globallycoherent RWBuffer<uint> g_ray_counter : register(u3);

RWTexture2D<float4> g_temporally_denoised_reflections : register(u4);
RWTexture2D<float4> g_temporally_denoised_reflections_history : register(u5);
RWTexture2D<float> g_ray_lengths : register(u6);
RWTexture2D<float> g_temporal_variance : register(u7);
RWTexture2D<float4> g_denoised_reflections : register(u8);

#include "SSSR_Common.hlsl"

groupshared uint g_ray_count;
groupshared uint g_ray_base_index;
groupshared uint g_denoise_count;

inline int2 GetDimensions(Texture2D<float> t)
{
    uint2 d;
    t.GetDimensions(d.x, d.y);
    return d;
}

[numthreads(8, 8, 1)]
void csmain(int3 did : SV_DispatchThreadID, uint group_index : SV_GroupIndex)
{
    bool is_first_lane_of_wave = WaveIsFirstLane();
    bool is_first_lane_of_threadgroup = group_index == 0;

    // First we figure out on a per thread basis if we need to shoot a reflection ray.
    uint2 screen_size = uint2(GetDimensions(g_roughness));

    // Disable offscreen pixels
    bool needs_ray = !(did.x >= screen_size.x || did.y >= screen_size.y);
    
    // Dont shoot a ray on very rough surfaces.
    float roughness = FfxSssrUnpackRoughness(g_roughness.Load(int3(did.xy, 0)));
    needs_ray = needs_ray && IsGlossy(roughness);

    // Also we dont need to run the denoiser on mirror reflections.
    bool needs_denoiser = needs_ray && !IsMirrorReflection(roughness);

    // Decide which ray to keep
    bool is_base_ray = IsBaseRay(did.xy, g_samples_per_quad);
    needs_ray = needs_ray && (!needs_denoiser || is_base_ray); // Make sure to not deactivate mirror reflection rays.

    if (g_temporal_variance_guided_tracing_enabled != 0u && needs_denoiser && !needs_ray)
    {
        float temporal_variance = g_temporal_variance[int2(did.xy)].x;
        bool has_temporal_variance = temporal_variance != 0.0;

        // If temporal variance is too high, we enforce a ray anyway.
        needs_ray = needs_ray ||
        has_temporal_variance;
    }

    // Now we know for each thread if it needs to shoot a ray and wether or not a denoiser pass has to run on this pixel.
    // Thus, we need to compact the rays and append them all at once to the ray list.
    // Also, if there is at least one pixel in that tile that needs a denoiser, we have to append that tile to the tile list.

    if (is_first_lane_of_threadgroup)
    {
        g_ray_count = 0;
        g_denoise_count = 0;
    }
    GroupMemoryBarrier(); // Wait for reset to finish

    uint local_ray_index_in_wave = WavePrefixCountBits(needs_ray);
    uint wave_ray_count = WaveActiveCountBits(needs_ray);
    bool wave_needs_denoiser = WaveActiveAnyTrue(needs_denoiser);
    uint wave_count = wave_needs_denoiser ? 1 : 0;

    uint local_ray_index_of_wave = 0;
    if (is_first_lane_of_wave)
    {
        InterlockedAdd(g_ray_count, wave_ray_count, local_ray_index_of_wave);
        uint _;
        InterlockedAdd(g_denoise_count, wave_count, _);
    }
    local_ray_index_of_wave = WaveReadLaneFirst(local_ray_index_of_wave);

    GroupMemoryBarrier(); // Wait for ray compaction to finish

    if (is_first_lane_of_threadgroup)
    {
        bool must_denoise = g_denoise_count > 0;
        uint denoise_count = must_denoise ? 1 : 0;
        uint ray_count = g_ray_count;

        uint tile_index;
        uint ray_base_index = 0;

        InterlockedAdd(g_tile_counter[0], denoise_count, tile_index);
        InterlockedAdd(g_ray_counter[0], ray_count, ray_base_index);

        int cleaned_index = must_denoise ? int(tile_index) : -1;
	    if(must_denoise)
	    {
            g_tile_list[cleaned_index] = Pack(did.xy); // Write out pixel coords of upper left pixel
	    }
        g_ray_base_index = ray_base_index;
    }
    GroupMemoryBarrier(); // Wait for ray base index to become available

    if(needs_ray)
    {
        int ray_index = int(g_ray_base_index + local_ray_index_of_wave + local_ray_index_in_wave);
        g_ray_list[ray_index] = Pack(did.xy); // Write out pixel to trace
        // Clear intersection targets as there wont be any ray that overwrites them
        g_temporally_denoised_reflections[did.xy] = f4(0.0f);
        g_ray_lengths[did.xy] = 0.0f;
    }
    g_denoised_reflections[did.xy] = f4(0.0f);
    // Re-purpose g_temporal_variance to hold the information for the spatial pass if a ray has been shot. Always write 0 if no denoiser is running.
    g_temporal_variance[did.xy] = needs_ray ? (1.0f - g_skip_denoiser) : 0.0f;
}
#endif // FFX_SSSR_CLASSIFY_TILES