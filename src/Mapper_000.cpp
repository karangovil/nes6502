#include "Mapper_000.h"

Mapper_000::Mapper_000(uint8_t prgBanks, uint8_t chrBanks)
  : Mapper(prgBanks, chrBanks)
{}

bool Mapper_000::cpuMapRead(uint16_t addr, uint32_t &mapped_addr)
{
  // if PRGROM is 16KB
  //   CPU address bus    PRG ROM
  //   0x8000 -> 0xBFFF:Map    0x0000->0x3FFF
  //   0xC000 -> 0xFFFF:Mirror 0x0000->0x3FF
  // if PRGROM is 32KB
  //   CPU address bus    PRG ROM
  //   0x8000 -> 0xBFFF:Map    0x0000->0x7FFF
  if (addr >= 0x8000 && addr <= 0xFFFF) {
    mapped_addr = addr & (nPRGBanks > 1 ? 0x7FFF : 0x3FFF);
    return true;
  }

  return false;
}

bool Mapper_000::cpuMapWrite(uint16_t addr, uint32_t &mapped_addr)
{
  if (addr >= 0x8000 && addr <= 0xFFFF) {
    mapped_addr = addr & (nPRGBanks > 1 ? 0x7FFF : 0x3FFF);
    return true;
  }

  return false;
}

bool Mapper_000::ppuMapRead(uint16_t addr, uint32_t &mapped_addr)
{
  if (addr >= 0x0000 && addr <= 0x1FFF) {
    mapped_addr = addr;
    return true;
  }

  return false;
}

bool Mapper_000::ppuMapWrite(uint16_t addr, uint32_t &mapped_addr)
{
  if (addr >= 0x0000 && addr <= 0x1FFF) {
    if (nCHRBanks == 0) {
      // treat as RAM
      mapped_addr = addr;
      return true;
    }
  }

  return false;
}
