
#include "Profiler.h"

namespace {
  Profiler::ScopeData singleFrameData[100]; // cleared at the end of every frame
  int inUseSlots = 0;
}

void Profiler::ClearFrameData()
{
  inUseSlots = 0;
}

void Profiler::PushProfile(const char* name, double time)
{
  singleFrameData[inUseSlots].name = name;
  singleFrameData[inUseSlots].time = time;
  inUseSlots++;
}

void Profiler::GetFrameData(Profiler::ScopeData** pOutData, int& outNumElements)
{
  *pOutData = singleFrameData;
  outNumElements = inUseSlots;
}