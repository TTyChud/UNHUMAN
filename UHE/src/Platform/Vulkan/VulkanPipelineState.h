#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vulkan/vulkan_raii.hpp>
#include "UHE/RHI/RHITypes.h"

namespace UHE::RHI::VULKAN
{
class VulkanPipelineState
{
public:
    VulkanPipelineState() = default;
    virtual ~VulkanPipelineState() = default;
    VulkanPipelineState(const VulkanPipelineState&) = delete;
    VulkanPipelineState& operator=(const VulkanPipelineState&) = delete;

    [[nodiscard]] virtual vk::Pipeline GetPipeline() const = 0;
    [[nodiscard]] virtual vk::PipelineLayout GetPipelineLayout() const = 0;
    [[nodiscard]] virtual vk::PipelineBindPoint GetBindPoint() const = 0;
};

class VulkanPipelineStateCache
{
public:
    using CreateFn = std::function<PipelineHandle()>;

    [[nodiscard]] PipelineHandle Acquire(const GraphicsPipelineDesc& desc, const CreateFn& create);
    [[nodiscard]] PipelineHandle Acquire(const ComputePipelineDesc& desc, const CreateFn& create);

    [[nodiscard]] bool Release(PipelineHandle handle);

    [[nodiscard]] static u64 Hash(const GraphicsPipelineDesc& desc);
    [[nodiscard]] static u64 Hash(const ComputePipelineDesc& desc);

private:
    struct CacheKey
    {
        u64 hash = 0;
        u64 shaderA = 0;
        u64 shaderB = 0;
        bool operator==(const CacheKey& other) const = default;
    };
    struct CacheKeyHash
    {
        size_t operator()(const CacheKey& key) const noexcept
        {
            size_t combined = std::hash<u64>{}(key.hash);
            combined ^= std::hash<u64>{}(key.shaderA) + 0x9e3779b97f4a7c15ULL + (combined << 6) + (combined >> 2);
            combined ^= std::hash<u64>{}(key.shaderB) + 0x9e3779b97f4a7c15ULL + (combined << 6) + (combined >> 2);
            return combined;
        }
    };
    struct Entry
    {
        PipelineHandle handle = nullptr;
        u32 refCount = 0;
        bool pending = false;
    };

    PipelineHandle AcquireHashed(const CacheKey& key, bool compute, const CreateFn& create);

    std::mutex m_Mutex;
    std::condition_variable m_Cond;
    std::unordered_map<CacheKey, Entry, CacheKeyHash> m_Graphics;
    std::unordered_map<CacheKey, Entry, CacheKeyHash> m_Compute;
};
} // namespace UHE::RHI::VULKAN
