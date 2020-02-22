
#include "Profiler.h"

namespace {
  std::vector<Profiler::ScopeData> singleFrameData; // cleared at the end of every frame
}

void Profiler::ClearFrameData()
{
  singleFrameData.clear();
}

void Profiler::PushProfile(const char* name, double time)
{
  singleFrameData.emplace_back(name, time);
}

std::vector<Profiler::ScopeData>& Profiler::GetFrameData()
{
  return singleFrameData;
}