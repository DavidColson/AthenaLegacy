
#include "Profiler.h"

namespace {
  std::vector<Profiler::ScopeData> m_singleFrameData; // cleared at the end of every frame
}

void Profiler::ClearFrameData()
{
  m_singleFrameData.clear();
}

void Profiler::PushProfile(const char* name, double time)
{
  m_singleFrameData.emplace_back(name, time);
}

std::vector<Profiler::ScopeData>& Profiler::GetFrameData()
{
  return m_singleFrameData;
}