#pragma once

#include <SDL.h>
#include <vector>

#include "Log.h"

namespace Profiler
{
  struct ScopeData
  {
    ScopeData(const char* name, double time) : name(name), time(time) {}
    const char* name;
    double time{ 0 };
  };

  void ClearFrameData();

  void PushProfile(const char* name, double time);

  std::vector<ScopeData>& GetFrameData();
}


#define PROFILE() AutoProfile p(__FUNCTION__)

struct AutoProfile
{
  AutoProfile(const char* name)
  {
    name = name;
    start = SDL_GetPerformanceCounter();;
  }

  ~AutoProfile()
  {
    double duration = double(SDL_GetPerformanceCounter() - start) / SDL_GetPerformanceFrequency();
    Profiler::PushProfile(name, duration);
  }

  const char *name;
  Uint64 start;
};