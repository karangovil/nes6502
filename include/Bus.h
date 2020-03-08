#pragma once

#include <cstdint>
#include <array>

#include "nes6502.h"
 
class Bus
{
public:
  Bus();
  ~Bus();

public: // Devices on the bus
  nes6502 cpu;

  // Fake RAM for now
  std::array<uint8_t, 64 * 1024> ram;

public: // Bus read/write
  void write(uint16_t addr, uint8_t data);
  uint8_t read(uint16_t addr, bool bReadOnly = false);
};
