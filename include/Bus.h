#pragma once

#include <cstdint>
#include <array>
#include <memory>

#include "nes6502.h"
#include "nes2C02.h"
#include "Cartridge.h"

class Bus
{
public:
  Bus();
  ~Bus();

public:// Devices on the bus
  // The 6502 CPU
  nes6502 cpu;

  // The 2C02 PPU (picture processing unit)
  nes2C02 ppu;
  // Fake RAM for now
  std::array<uint8_t, 2048> cpuRam;

public:// Bus read/write
  void cpuWrite(uint16_t addr, uint8_t data);
  uint8_t cpuRead(uint16_t addr, bool bReadOnly = false);

public:// System Interface
  void insertCartridge(const std::shared_ptr<Cartridge> &cartridge);
  void reset();
  void clock();

private:
  // count of how many clocks have passed
  uint32_t nSystemClockCounter = 0;
  // Cartridge or "GamePak"
  std::shared_ptr<Cartridge> cart;
};
