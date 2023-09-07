#pragma once

namespace serenity::asset
{
    // A utility namespace that helps in loading texture from file.
    // The output contains a vector of floats or uint8_t's (based on the texture) from which GPU textures are to be
    // created (not done here). This design is taken so as to reduce dependency between the process of loading texture
    // files and actually constructing data from them. Texture loader currently uses stbi.

    struct TextureData
    {
        Uint2 dimension{};
        std::variant<std::vector<uint8_t>, std::vector<float>> data{};
    };

    namespace TextureLoader
    {
        [[nodiscard]] TextureData load_texture(const std::string_view texture_path, const int num_channels = 4);

        // Internally uses stbi_load_from_memory.
        [[nodiscard]] TextureData load_texture(const std::byte* data, const uint32_t size, const int num_channels = 4);
    }
} // namespace serenity::asset