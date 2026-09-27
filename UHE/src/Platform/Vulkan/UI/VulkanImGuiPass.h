#pragma once
#include "Platform/Vulkan/RenderGraph/VulkanRenderGraphExecutor.h"

struct ImDrawData;

namespace UHE::RHI::VULKAN
{

// The ImGui pass as a graph pass (§14 step 5, "ImGui becomes a pass").
//
// Split of responsibilities:
//   - Host state (NewFrame/Render/draw data) stays with the layer.
//   - The DEVICE declares the pass into the frame graph (BeginImGuiPass) with
//     Load/Store ops and the swapchain import's states. The import starts at
//     Present when legacy scene rendering (not yet graph-resident) already drew
//     to the swapchain this frame, otherwise at Undefined; it flips to the
//     graph's own Present acquire once scene passes are graph-resident
//     (ROADMAP M5 step 4).
//   - The COMPILER owns the layout transitions (entry → ColorAttachment,
//     ColorAttachment → Present exit). The hardcoded Undefined→ColorAttachment
//     and ColorAttachment→Present barriers the legacy End() emitted are gone
//     (§13.5 item 28 defect).
//   - The EXECUTOR owns the dynamic-rendering scope (Load preserves the scene).
//   - This file only records ImGui's draw data inside that scope.
class VulkanImGuiPass
{
public:
    // Host-side hook: ImGui::Render() + draw-data pointer (no GPU work).
    static ImDrawData* EndHostFrame();

    // GPU recording inside the executor-opened swapchain scope, from the pass
    // callback. No scope, no barriers — those are the executor's/compiler's.
    static void RecordInContext(RGPassContext& context);
};

} // namespace UHE::RHI::VULKAN
