#pragma once
#include <span>
#include <vulkan/vulkan_raii.hpp>
#include "Platform/Vulkan/VulkanTypes.h"

namespace UHE::RHI::VULKAN
{
class VulkanContext;

using CommandBufferRef = vk::CommandBuffer;

struct SemaphoreWait
{
    vk::Semaphore Semaphore = nullptr;
    Stage WaitStage = Stage::None;
    u64 Value = 0;
};

struct SemaphoreSignal
{
    vk::Semaphore Semaphore = nullptr;
    u64 Value = 0;
};

// Neutral description of one queue submission: what to wait on, what to record,
// what to signal. Submit() turns this into vkQueueSubmit2 or vkQueueSubmit.
struct SubmitInfo
{
    std::span<const SemaphoreWait> Waits;
    std::span<const CommandBufferRef> CommandBuffers;
    std::span<const SemaphoreSignal> Signals;
};

class VulkanSemaphore
{
public:
    VulkanSemaphore() = default;
    ~VulkanSemaphore() = default;
    VulkanSemaphore(const VulkanSemaphore&) = delete;
    VulkanSemaphore& operator=(const VulkanSemaphore&) = delete;

    void Init(bool requestTimeline = false, u64 initialValue = 0, VulkanContext* context = nullptr);
    void ShutDown();
    // CPU wait on a timeline value. Bounded by timeoutNs (§9.1.5 — UINT64_MAX
    // waits are banned); returns false on timeout. Legacy tier: no-op, true.
    [[nodiscard]] bool WaitCPU(u64 value, u64 timeoutNs);
    u64 GetValue();

    static void Submit(VulkanContext* context, vk::raii::Queue& queue, const SubmitInfo& info, vk::Fence fence = nullptr);

    [[nodiscard]] inline vk::Semaphore GetHandle() const { return *m_Semaphore; }
    [[nodiscard]] inline bool IsTimeline() const { return m_IsTimeline; }

    [[nodiscard]] SemaphoreWait GetWait(Stage stage, u64 value = 0) const
    {
        return SemaphoreWait{ .Semaphore = *m_Semaphore, .WaitStage = stage, .Value = value };
    }
    [[nodiscard]] SemaphoreSignal GetSignal(u64 value = 0) const
    {
        return SemaphoreSignal{ .Semaphore = *m_Semaphore, .Value = value };
    }

private:
    bool m_IsTimeline = false; // true only if request AND hardware
    VulkanContext* ctx = nullptr;
    vk::raii::Semaphore m_Semaphore = nullptr;

    // TODO:  fallback state for emulating timeline using multiple binary semaphore
};

} // namespace UHE::RHI::VULKAN
