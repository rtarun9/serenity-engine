#pragma once

#include "d3d_utils.hpp"

#include "serenity-engine/core/singleton_instance.hpp"

namespace serenity::renderer::rhi
{
    // Abstraction for bindless root signature, which specifies that the shader expects 32 bit root constants (64 of
    // them) and a bunch of static samplers.
    class RootSignature : public core::SingletonInstance<RootSignature>
    {
      public:
        explicit RootSignature(const comptr<ID3D12Device> &device);
        ~RootSignature();

        comptr<ID3D12RootSignature> &get_root_signature()
        {
            return m_root_signature;
        }

      public:
        static constexpr uint32_t NUM_32_BIT_ROOT_CONSTANTS = 64u;

      private:
        comptr<ID3D12RootSignature> m_root_signature{};
    };
} // namespace serenity::renderer::rhi