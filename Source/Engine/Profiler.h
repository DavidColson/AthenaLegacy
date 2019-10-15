#pragma once

#include <SDL.h>
#include <vector>

#include "Log.h"

namespace Profiler
{
  struct ScopeData
  {
    ScopeData(const char* name, double time) : m_name(name), m_time(time) {}
    const char* m_name;
    double m_time{ 0 };
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
    m_name = name;
    m_start = SDL_GetPerformanceCounter();;
  }

  ~AutoProfile()
  {
    double duration = double(SDL_GetPerformanceCounter() - m_start) / SDL_GetPerformanceFrequency();
    Profiler::PushProfile(m_name, duration);
  }

  const char *m_name;
  Uint64 m_start;
};