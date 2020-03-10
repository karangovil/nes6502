#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

#include "Mapper_000.h"

class Cartridge
{
public:
  Cartridge(const std::string &sFileName);
  ~Cartridge() = default;

public:
  bool ImageValid();

  enum MIRROR {
    HORIZONTAL,
    VERTICAL,
    ONESCREEN_LO,
    ONESCREEN_HI,
  } mirror = HORIZONTAL;

private:
  bool bImageValid;
  std::vector<uint8_t> vPRGMemory;
  std::vector<uint8_t> vCHRMemory;

  uint8_t nMapperID = 0;
  uint8_t nPRGBanks = 0;
  uint8_t nCHRBanks = 0;

  std::shared_ptr<Mapper> pMapper;

public:
  // Communications with the main bus
  bool cpuRead(uint16_t addr, uint8_t &data);
  bool cpuWrite(uint16_t addr, uint8_t data);

  // Communications with the PPU bus
  bool ppuRead(uint16_t addr, uint8_t &data);
  bool ppuWrite(uint16_t addr, uint8_t data);
};
