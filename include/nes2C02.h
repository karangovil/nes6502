#pragma once

#include <cstdint>
#include <memory>

#include "Cartridge.h"
#include "olcPixelGameEngine.h"

class nes2C02
{
public:
  nes2C02();
  ~nes2C02() = default;

private:
  // VRAM - 2 kb butone full name table is 1kb
  // and NES has capability to store 2 whole name tables
  // however it can address 4 whole name tables
  uint8_t tblName[2][1024];
  // RAM to store palette information (32 entries)
  uint8_t tblPalette[32];
  // normally in NES systems, this exists on cartridge
  uint8_t tblPattern[2][4096];// TODO Future reminder

public:
  // Communications with the main bus
  uint8_t cpuRead(uint16_t addr, bool bReadOnly = false);
  void cpuWrite(uint16_t addr, uint8_t data);

  // Communications with the PPU bus
  uint8_t ppuRead(uint16_t addr, bool bReadOnly = false);
  void ppuWrite(uint16_t addr, uint8_t data);

private:
  // Cartridge or "GamePak"
  std::shared_ptr<Cartridge> cart;

public:// System Interface
  void ConnectCartridge(const std::shared_ptr<Cartridge> &cartridge);
  void clock();

private:
  olc::Pixel palScreen[0x40];
  olc::Sprite sprScreen = olc::Sprite(256, 240);
  olc::Sprite sprNameTable[2] = { olc::Sprite(256, 240), olc::Sprite(256, 240) };
  olc::Sprite sprPatternTable[2] = { olc::Sprite(128, 128), olc::Sprite(128, 128) };

public:
  // Debugging utils
  olc::Sprite &GetScreen();
  olc::Sprite &GetNameTable(uint8_t i);
  olc::Sprite &GetPatternTable(uint8_t i);
  bool frame_complete = false;

private:
  int16_t scanline = 0;// row on screen
  int16_t cycle = 0;// col on scrren
};
