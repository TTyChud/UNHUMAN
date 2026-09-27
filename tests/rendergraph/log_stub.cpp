// Stub for the standalone RenderGraph test only: provides the two loggers the
// engine Log.h expects, backed by a default spdlog stderr logger. Replaces
// Log.cpp (which pulls ImGuiLogSink and the ImGui include tree).
#include "UHE/Core/Log.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace UHE
{
std::shared_ptr<spdlog::logger> Log::s_CoreLogger = spdlog::stderr_color_mt("uhe-core-stub");
std::shared_ptr<spdlog::logger> Log::s_ClientLogger = spdlog::stderr_color_mt("uhe-client-stub");
std::shared_ptr<ImGuiLogSink<std::mutex>> Log::s_ImGuiSink{};
void Log::Init() {}
} // namespace UHE
