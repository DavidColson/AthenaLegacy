#pragma once

#include <SDL.h>
#include <vector>

#include "Log.h"

namespace Profiler
{
  struct ScopeData
  {
    ScopeData(const char* _name, double _time) : name(_name), time(_time) {}
    const char* name;
    double time{ 0 };
  };

  void ClearFrameData();

  void PushProfile(const char* _name, double _time);

  std::vector<ScopeData>& GetFrameData();
}


#define PROFILE() AutoProfile p(__FUNCTION__)

struct AutoProfile
{
  AutoProfile(const char* _name)
  {
    name = _name;
    start = SDL_GetPerformanceCounter();
  }

  ~AutoProfile()
  {
    double duration = double(SDL_GetPerformanceCounter() - start) / SDL_GetPerformanceFrequency();
    Profiler::PushProfile(name, duration);
  }

  const char *name;
  Uint64 start;
};