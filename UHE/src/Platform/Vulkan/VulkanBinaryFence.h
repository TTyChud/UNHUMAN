#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace UHE::RHI::VULKAN
{
class VulkanContext;

class VulkanBinaryFence
{
public:
    VulkanBinaryFence() = default;
    ~VulkanBinaryFence() = default; // vk::raii::Fence automatically cleans itself up!

    void Init(VulkanContext* context, bool signaled = false);
    void Shutdown();

    void WaitOnCpuIn() const;
    void ResetIn() const;

    [[nodiscard]] bool IsSignaled() const;
    [[nodiscard]] inline vk::Fence GetFence() const noexcept { return *m_Fence; }

private:
    VulkanContext* m_context = nullptr;
    vk::raii::Fence m_Fence = nullptr; 
};

} // namespace UHE::RHI::VULKAN
