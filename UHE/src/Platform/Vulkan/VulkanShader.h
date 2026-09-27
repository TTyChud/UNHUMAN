#pragma once
#include <atomic>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include "UHE/RHI/RHITypes.h"

namespace UHE::RHI::VULKAN
{

class VulkanShader
{
public:
    VulkanShader() = default;
    VulkanShader(const VulkanShader&) = delete;
    VulkanShader operator=(const VulkanShader&) = delete;
    // Create the shader module
    void Create(const vk::raii::Device& device, const ShaderDesc& desc);

    vk::ShaderModule GetHandle() const { return *m_Module; }
    ShaderStage GetStage() const { return m_Stage; }
    const char* GetEntryPoint() const { return m_EntryPoint.c_str(); }
    vk::ShaderModule GetModule() const { return *m_Module; }
    [[nodiscard]] u64 GetStableId() const { return m_StableId; }

private:
    vk::raii::ShaderModule m_Module{nullptr};
    ShaderStage m_Stage = ShaderStage::Vertex;
    std::string m_EntryPoint = "main";
    u64 m_StableId = s_NextId.fetch_add(1, std::memory_order_relaxed);
    inline static std::atomic<u64> s_NextId{1};
};

} // namespace UHE::RHI::VULKAN
