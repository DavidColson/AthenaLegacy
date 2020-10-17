#include "FrameStats.h"
#include "Profiler.h"
#include "Engine.h"

#include <Imgui/imgui.h>

FrameStats::FrameStats()
{
    menuName = "Frame Stats";
}

// ***********************************************************************

void FrameStats::Update(Scene& scene)
{
    double realFrameTime;
    double observedFrameTime;
    Engine::GetFrameRates(realFrameTime, observedFrameTime);

    if (++frameStatsCounter > 30)
    {
        oldRealFrameTime = realFrameTime;
        oldObservedFrameTime = observedFrameTime;
        frameStatsCounter = 0;
    }

    ImGui::Begin("Frame Stats", &open);

    ImGui::Text("Real frame time %.6f ms/frame (%.3f FPS)", oldRealFrameTime * 1000.0, 1.0 / oldRealFrameTime);
    ImGui::Text("Observed frame time %.6f ms/frame (%.3f FPS)", oldObservedFrameTime * 1000.0, 1.0 / oldObservedFrameTime);

    ImGui::Separator();

    int elements = 0;
    Profiler::ScopeData* pFrameData = nullptr;
    Profiler::GetFrameData(&pFrameData, elements);
    for (size_t i = 0; i < elements; ++i)
    {
        // Might want to add some smoothing and history to this data? Can be noisey, especially for functions not called every frame
        // Also maybe sort so we can see most expensive things at the top? Lots of expansion possibility here really
        double inMs = pFrameData[i].time * 1000.0;
        eastl::string str;
        str.sprintf("%s - %fms/frame", pFrameData[i].name, inMs);
        ImGui::Text(str.c_str());
    }

    ImGui::End();
}