#pragma once

#include "serenity-engine/utils/string_conversions.hpp"

namespace serenity::graphics
{
    // Helper functions for D3D12.

    template <typename T> using comptr = Microsoft::WRL::ComPtr<T>;

    static inline void throw_if_failed(const HRESULT hr,
                                       const std::source_location source_location = std::source_location::current())
    {
        if (FAILED(hr))
        {
            core::Log::get().critical("Hresult failed", source_location);
        }
    }

    static inline void set_name(ID3D12Object *const object, const std::wstring_view name)
    {
        throw_if_failed(object->SetName(name.data()));
    }

    static inline std::wstring command_list_type_to_string(const D3D12_COMMAND_LIST_TYPE command_list_type)
    {
        switch (command_list_type)
        {
        case D3D12_COMMAND_LIST_TYPE_DIRECT: {
            return L"Direct";
        }
        break;

        case D3D12_COMMAND_LIST_TYPE_COMPUTE: {
            return L"Compute";
        }
        break;

        case D3D12_COMMAND_LIST_TYPE_COPY: {
            return L"Copy";
        }
        break;
        }

        // Code should never reach here.
        return L"INVALID COMMAND QUEUE TYPE";
    }

    static inline std::wstring descriptor_heap_type_to_string(const D3D12_DESCRIPTOR_HEAP_TYPE descriptor_heap_type)
    {
        switch (descriptor_heap_type)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: {
            return L"CBV SRV UAV";
        }
        break;

        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER: {
            return L"Sampler";
        }
        break;

        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV: {
            return L"RTV";
        }
        break;

        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV: {
            return L"DSV";
        }
        break;
        }

        // Code should never reach here.
        return L"INVALID DESCRIPTOR HEAP TYPE";
    }
} // namespace serenity::graphics