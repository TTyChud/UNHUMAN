#include "uhepch.h"
#include "VulkanImGuiBackend.h"
#include <GLFW/glfw3.h>
// clang-format off
#include <imgui.h>
#include <ImGuizmo.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
// clang-format on
#include "Platform/Vulkan/UI/VulkanImGuiPass.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanLogicalDevice.h"
#include "UHE/Core/Application.h"

namespace UHE::RHI::VULKAN
{
VulkanImGuiLayer::VulkanImGuiLayer(VulkanDevice* device) : ImGuiLayer(), m_Device(device) {}

void VulkanImGuiLayer::OnAttach()
{

    ImGuiLayer::OnAttach();

    auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
    ImGui_ImplGlfw_InitForVulkan(window, true);

    VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                         {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                         {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                         {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    vk::DescriptorPoolCreateInfo poolInfo{.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                          .maxSets = static_cast<uint32_t>(1000 * IM_ARRAYSIZE(pool_sizes)),
                                          .poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes)),
                                          .pPoolSizes = reinterpret_cast<vk::DescriptorPoolSize*>(pool_sizes)};
    m_DescriptorPool =
        std::make_unique<vk::raii::DescriptorPool>(m_Device->getLogicalDevClass().getLogicalDevice(), poolInfo);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = *m_Device->getInstanceClass().getInstance();
    init_info.PhysicalDevice = *m_Device->getPhysicalDevClass().getPhysicalDevice();
    init_info.Device = *m_Device->getLogicalDevClass().getLogicalDevice();
    init_info.QueueFamily = m_Device->getPhysicalDevClass().getQueueFamilyIndices().graphicsFamily.value();
    init_info.Queue = *m_Device->GetGraphicsQueue();
    init_info.DescriptorPool = **m_DescriptorPool;
    init_info.MinImageCount = 3; // Typically 3 for triple buffering
    init_info.ImageCount = 3;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    init_info.UseDynamicRendering = true;

    VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo = {};
    pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    static auto colorFormat = static_cast<VkFormat>(m_Device->getSwapChainClass().GetSurfaceFormat().format);
    pipelineRenderingCreateInfo.colorAttachmentCount = 1;
    pipelineRenderingCreateInfo.pColorAttachmentFormats = &colorFormat;

    init_info.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineRenderingCreateInfo;

    ImGui_ImplVulkan_Init(&init_info);
}

void VulkanImGuiLayer::OnDetach()
{
    m_Device->WaitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    m_DescriptorPool.reset();

    // Base class destroys ImGui Context
    ImGuiLayer::OnDetach();
}

void VulkanImGuiLayer::Begin()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

void VulkanImGuiLayer::End()
{
    // §14 step 5: host state only. The old body hardcoded Undefined→
    // ColorAttachment and ColorAttachment→Present barriers plus a Clear-load
    // swapchain scope — all three belong to the graph now (the compiler emits
    // the transitions; Load preserves the frame's earlier passes).
    //
    // The ImGui pass is declared exactly once per frame — HERE, after the draw
    // data is ready. (VulkanDevice::EndFrameGraph does NOT declare it: its old
    // Reset-then-declare ordering wiped feature passes and duplicated this
    // pass every frame.) EndFrameGraph then compiles whatever the layers
    // declared, ImGui last — which is exactly the required declaration order.
    VulkanImGuiPass::EndHostFrame();

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    m_Device->BeginImGuiPass("ImGui");
}
} // namespace UHE::RHI::VULKAN
