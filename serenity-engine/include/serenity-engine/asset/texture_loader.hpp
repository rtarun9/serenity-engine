#pragma once

namespace serenity::asset
{
    // A utility namespace that helps in loading texture from file.
    // The output contains a vector of floats or uint8_t's (based on the texture) from which GPU textures are to be
    // created (not done here). This design is taken so as to reduce dependency between the process of loading texture
    // files and actually constructing GPU resources from them. Texture loader internally uses stbi.

    struct TextureData
    {
        Uint2 dimension{};
        std::variant<std::vector<uint8_t>, std::vector<float>> data{};
    };

    namespace TextureLoader
    {
        // Load data from file on disk and the texture path is known.
        [[nodiscard]] TextureData load_texture(const std::string_view texture_path, const uint32_t num_channels = 4);

        // Load data which is in memory.
        // Internally uses stbi_load_from_memory.
        [[nodiscard]] TextureData load_texture(const std::byte *data, const uint32_t size,
                                               const uint32_t num_channels = 4);
    } // namespace TextureLoader
} // namespace serenity::asset