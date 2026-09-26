// Test-only translation unit for the rendergraph harness:
//
// 1. The vulkan-hpp dynamic dispatcher storage, which the engine defines in
//    VulkanDevice.cpp. No loader is initialized — the Resolve path never calls
//    into Vulkan.
// 2. VulkanExtensionCheck::Supports — real VulkanExtensionCheck.cpp queries
//    physical-device properties via the loader; the tests have no device. The
//    executor only reads GetSyncTier() (never calls Supports on a live object),
//    so a never-called stub returning false (Legacy tier) is faithful.
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include "Platform/Vulkan/VulkanExtensionCheck.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace UHE::RHI::VULKAN
{
bool VulkanExtensionCheck::Supports(Extension) const noexcept
{
    return false; // never called in the harness; Legacy tier
}
} // namespace UHE::RHI::VULKAN
