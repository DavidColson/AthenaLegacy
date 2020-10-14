#include "Profiler.h"

#include <SDL_timer.h>

namespace {
  Profiler::ScopeData singleFrameData[100]; // cleared at the end of every frame
  int inUseSlots = 0;
}

// ***********************************************************************

void Profiler::ClearFrameData()
{
  inUseSlots = 0;
}

// ***********************************************************************

void Profiler::PushProfile(const char* name, double time)
{
  singleFrameData[inUseSlots].name = name;
  singleFrameData[inUseSlots].time = time;
  inUseSlots++;
}

// ***********************************************************************

void Profiler::GetFrameData(Profiler::ScopeData** pOutData, int& outNumElements)
{
  *pOutData = singleFrameData;
  outNumElements = inUseSlots;
}

// ***********************************************************************

AutoProfile::AutoProfile(const char* _name)
{
  name = _name;
  start = SDL_GetPerformanceCounter();
}

// ***********************************************************************

AutoProfile::~AutoProfile()
{
  double duration = double(SDL_GetPerformanceCounter() - start) / SDL_GetPerformanceFrequency();
  Profiler::PushProfile(name, duration);
}