#include "uhepch.h"
#include "VulkanSemaphore.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanExtensionCheck.h"

namespace UHE::RHI::VULKAN
{

void VulkanSemaphore::Init(bool requestTimeline, u64 initialValue, VulkanContext* context)
{
    ctx = context;

    m_IsTimeline = requestTimeline && ctx->CheckExtensions->Supports(Extension::TimelineSemaphore);

    vk::SemaphoreTypeCreateInfo timelineInfo;
    timelineInfo.semaphoreType = m_IsTimeline ? vk::SemaphoreType::eTimeline : vk::SemaphoreType::eBinary;
    timelineInfo.initialValue = initialValue;

    vk::SemaphoreCreateInfo createInfo;
    createInfo.pNext = &timelineInfo;

    m_Semaphore = vk::raii::Semaphore(*ctx->logicalDeviceHandle, createInfo);
}

void VulkanSemaphore::ShutDown()
{
    m_Semaphore = nullptr;
    ctx = nullptr;
}

bool VulkanSemaphore::WaitCPU(u64 value, u64 timeoutNs)
{
    if (!m_IsTimeline)
        return true; // Legacy tier: binary pacing is a no-op (fence is the gate)

    vk::SemaphoreWaitInfo waitInfo;
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores = &m_Semaphore.operator*();
    waitInfo.pValues = &value;

    // §9.1.5: bounded waits only — a timeout means device loss or a sync bug,
    // and the caller must see it instead of hanging forever.
    const vk::Result waitResult = ctx->logicalDeviceHandle->waitSemaphores(waitInfo, timeoutNs);
    return waitResult == vk::Result::eSuccess;
}

u64 VulkanSemaphore::GetValue()
{
    if (!m_IsTimeline)
        return 0;

    return m_Semaphore.getCounterValue();
}

void VulkanSemaphore::Submit(VulkanContext* context, vk::raii::Queue& queue, const SubmitInfo& info, vk::Fence fence)
{
    const SyncTier tier = context != nullptr ? context->CheckExtensions->GetSyncTier() : SyncTier::Legacy;

    if (tier == SyncTier::Legacy)
    {
        // v1 submit: binary semaphores and a per-wait stage mask. Timeline values
        // have no representation here and are ignored.
        std::vector<vk::Semaphore> waitSemaphores;
        std::vector<vk::PipelineStageFlags> waitStages;
        waitSemaphores.reserve(info.Waits.size());
        waitStages.reserve(info.Waits.size());
        for (const SemaphoreWait& wait : info.Waits)
        {
            waitSemaphores.push_back(wait.Semaphore);
            waitStages.push_back(ToVkPipelineStage1(wait.WaitStage));
        }

        std::vector<vk::Semaphore> signalSemaphores;
        signalSemaphores.reserve(info.Signals.size());
        for (const SemaphoreSignal& signal : info.Signals)
            signalSemaphores.push_back(signal.Semaphore);

        const std::vector<vk::CommandBuffer> commandBuffers(info.CommandBuffers.begin(), info.CommandBuffers.end());

        const vk::SubmitInfo submitInfo{
            .waitSemaphoreCount = static_cast<u32>(waitSemaphores.size()),
            .pWaitSemaphores = waitSemaphores.data(),
            .pWaitDstStageMask = waitStages.data(),
            .commandBufferCount = static_cast<u32>(commandBuffers.size()),
            .pCommandBuffers = commandBuffers.data(),
            .signalSemaphoreCount = static_cast<u32>(signalSemaphores.size()),
            .pSignalSemaphores = signalSemaphores.data(),
        };

        queue.submit(submitInfo, fence);
        return;
    }

    std::vector<vk::SemaphoreSubmitInfo> waits;
    waits.reserve(info.Waits.size());
    for (const SemaphoreWait& wait : info.Waits)
    {
        waits.push_back({
            .semaphore = wait.Semaphore,
            .value = wait.Value,
            .stageMask = ToVkPipelineStage2(wait.WaitStage),
        });
    }

    std::vector<vk::SemaphoreSubmitInfo> signals;
    signals.reserve(info.Signals.size());
    for (const SemaphoreSignal& signal : info.Signals)
    {
        signals.push_back({
            .semaphore = signal.Semaphore,
            .value = signal.Value,
            .stageMask = vk::PipelineStageFlagBits2::eAllCommands,
        });
    }

    std::vector<vk::CommandBufferSubmitInfo> commandInfos;
    commandInfos.reserve(info.CommandBuffers.size());
    for (const vk::CommandBuffer& commandBuffer : info.CommandBuffers)
        commandInfos.push_back({.commandBuffer = commandBuffer});

    const vk::SubmitInfo2 submitInfo{
        .waitSemaphoreInfoCount = static_cast<u32>(waits.size()),
        .pWaitSemaphoreInfos = waits.data(),
        .commandBufferInfoCount = static_cast<u32>(commandInfos.size()),
        .pCommandBufferInfos = commandInfos.data(),
        .signalSemaphoreInfoCount = static_cast<u32>(signals.size()),
        .pSignalSemaphoreInfos = signals.data(),
    };

    queue.submit2(submitInfo, fence);
}

} // namespace UHE::RHI::VULKAN
