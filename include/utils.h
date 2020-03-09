#pragma once

#include <string>

// converet variables into hex strings
inline std::string hex(uint32_t n, uint8_t d)
{
  std::string s(d, '0');
  for (int i = d - 1; i >= 0; i--, n >>= 4)
    s[i] = "0123456789ABCDEF"[n & 0xF];
  return s;
}
