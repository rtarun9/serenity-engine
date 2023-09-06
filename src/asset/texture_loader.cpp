#include "serenity-engine/asset/texture_loader.hpp"

#include "serenity-engine/core/file_system.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace serenity::asset::TextureLoader
{
    TextureData load_texture(const std::string_view texture_path, const int num_channels)
    {
        auto texture_data = TextureData{};

        // If the texture extension is 'hdr', that means we need to load the texture as vector of floats. Else, a vector
        // of uint8_t's is used.

        auto path = core::FileSystem::instance().get_relative_path(texture_path);
        if (const auto extension = std::filesystem::path(path).extension(); extension == ".hdr")
        {
            core::Log::instance().critical("This function is not implemented yet!");
        }
        else
        {
            auto width = static_cast<int>(0);
            auto height = static_cast<int>(0);

            auto data = stbi_load(path.c_str(), &width, &height, nullptr, num_channels);

            texture_data.dimension = Uint2{
                .x = static_cast<uint32_t>(width),
                .y = static_cast<uint32_t>(height),
            };

            if (!data)
            {
                core::Log::instance().critical(std::format("Failed to load texture from path : {}", texture_path));
                return {};
            }
            else
            {
                auto data_vector = std::vector<uint8_t>(static_cast<size_t>(width * height * num_channels));
                std::memcpy(data_vector.data(), data, data_vector.size());

                texture_data.data = data_vector;
            }
        }

        core::Log::instance().info(std::format("Loaded texture from path :  {}", texture_path));

        return texture_data;
    }
} // namespace serenity::asset::TextureLoader