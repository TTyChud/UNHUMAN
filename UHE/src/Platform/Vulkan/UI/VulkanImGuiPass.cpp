#include "uhepch.h"
#include "VulkanImGuiPass.h"
// clang-format off
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
// clang-format on
#include "Platform/Vulkan/RenderGraph/VulkanRenderGraphExecutor.h"

namespace UHE::RHI::VULKAN
{

ImDrawData* VulkanImGuiPass::EndHostFrame()
{
    ImGui::Render();
    return ImGui::GetDrawData();
}

void VulkanImGuiPass::RecordInContext(RGPassContext& context)
{
    ImDrawData* drawData = ImGui::GetDrawData();
    if (drawData == nullptr)
        return;

    // The executor opened the swapchain scope (Load/Store from the graph) and
    // the compiler emitted the layout transitions around it — just draw.
    ImGui_ImplVulkan_RenderDrawData(drawData, context.Cmd());
}

} // namespace UHE::RHI::VULKAN
