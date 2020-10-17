#pragma once

#include "Log.h"

namespace Profiler
{
  struct ScopeData
  {
    ScopeData() : name(""), time(0) {}
    ScopeData(const char* _name, double _time) : name(_name), time(_time) {}
    const char* name;
    double time{ 0 };
  };

  void ClearFrameData();

  void PushProfile(const char* _name, double _time);

  void GetFrameData(Profiler::ScopeData** pOutData, int& outNumElements);
}


#define PROFILE() AutoProfile p(__FUNCTION__)

struct AutoProfile
{
  AutoProfile(const char* _name);
  ~AutoProfile();
  const char *name;
  uint64_t start;
};