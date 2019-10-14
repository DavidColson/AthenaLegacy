#pragma once

#include <bitset>

struct CInputState
{
#define NKEYS 512
  std::bitset<NKEYS> m_keyDowns;
  std::bitset<NKEYS> m_keyUps;
  std::bitset<NKEYS> m_keyStates;
};