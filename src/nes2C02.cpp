#include "nes2C02.h"

nes2C02::nes2C02()
{
  palScreen[0x00] = olc::Pixel(84, 84, 84);
  palScreen[0x01] = olc::Pixel(0, 30, 116);
  palScreen[0x02] = olc::Pixel(8, 16, 144);
  palScreen[0x03] = olc::Pixel(48, 0, 136);
  palScreen[0x04] = olc::Pixel(68, 0, 100);
  palScreen[0x05] = olc::Pixel(92, 0, 48);
  palScreen[0x06] = olc::Pixel(84, 4, 0);
  palScreen[0x07] = olc::Pixel(60, 24, 0);
  palScreen[0x08] = olc::Pixel(32, 42, 0);
  palScreen[0x09] = olc::Pixel(8, 58, 0);
  palScreen[0x0A] = olc::Pixel(0, 64, 0);
  palScreen[0x0B] = olc::Pixel(0, 60, 0);
  palScreen[0x0C] = olc::Pixel(0, 50, 60);
  palScreen[0x0D] = olc::Pixel(0, 0, 0);
  palScreen[0x0E] = olc::Pixel(0, 0, 0);
  palScreen[0x0F] = olc::Pixel(0, 0, 0);

  palScreen[0x10] = olc::Pixel(152, 150, 152);
  palScreen[0x11] = olc::Pixel(8, 76, 196);
  palScreen[0x12] = olc::Pixel(48, 50, 236);
  palScreen[0x13] = olc::Pixel(92, 30, 228);
  palScreen[0x14] = olc::Pixel(136, 20, 176);
  palScreen[0x15] = olc::Pixel(160, 20, 100);
  palScreen[0x16] = olc::Pixel(152, 34, 32);
  palScreen[0x17] = olc::Pixel(120, 60, 0);
  palScreen[0x18] = olc::Pixel(84, 90, 0);
  palScreen[0x19] = olc::Pixel(40, 114, 0);
  palScreen[0x1A] = olc::Pixel(8, 124, 0);
  palScreen[0x1B] = olc::Pixel(0, 118, 40);
  palScreen[0x1C] = olc::Pixel(0, 102, 120);
  palScreen[0x1D] = olc::Pixel(0, 0, 0);
  palScreen[0x1E] = olc::Pixel(0, 0, 0);
  palScreen[0x1F] = olc::Pixel(0, 0, 0);

  palScreen[0x20] = olc::Pixel(236, 238, 236);
  palScreen[0x21] = olc::Pixel(76, 154, 236);
  palScreen[0x22] = olc::Pixel(120, 124, 236);
  palScreen[0x23] = olc::Pixel(176, 98, 236);
  palScreen[0x24] = olc::Pixel(228, 84, 236);
  palScreen[0x25] = olc::Pixel(236, 88, 180);
  palScreen[0x26] = olc::Pixel(236, 106, 100);
  palScreen[0x27] = olc::Pixel(212, 136, 32);
  palScreen[0x28] = olc::Pixel(160, 170, 0);
  palScreen[0x29] = olc::Pixel(116, 196, 0);
  palScreen[0x2A] = olc::Pixel(76, 208, 32);
  palScreen[0x2B] = olc::Pixel(56, 204, 108);
  palScreen[0x2C] = olc::Pixel(56, 180, 204);
  palScreen[0x2D] = olc::Pixel(60, 60, 60);
  palScreen[0x2E] = olc::Pixel(0, 0, 0);
  palScreen[0x2F] = olc::Pixel(0, 0, 0);

  palScreen[0x30] = olc::Pixel(236, 238, 236);
  palScreen[0x31] = olc::Pixel(168, 204, 236);
  palScreen[0x32] = olc::Pixel(188, 188, 236);
  palScreen[0x33] = olc::Pixel(212, 178, 236);
  palScreen[0x34] = olc::Pixel(236, 174, 236);
  palScreen[0x35] = olc::Pixel(236, 174, 212);
  palScreen[0x36] = olc::Pixel(236, 180, 176);
  palScreen[0x37] = olc::Pixel(228, 196, 144);
  palScreen[0x38] = olc::Pixel(204, 210, 120);
  palScreen[0x39] = olc::Pixel(180, 222, 120);
  palScreen[0x3A] = olc::Pixel(168, 226, 144);
  palScreen[0x3B] = olc::Pixel(152, 226, 180);
  palScreen[0x3C] = olc::Pixel(160, 214, 228);
  palScreen[0x3D] = olc::Pixel(160, 162, 160);
  palScreen[0x3E] = olc::Pixel(0, 0, 0);
  palScreen[0x3F] = olc::Pixel(0, 0, 0);
}

olc::Sprite &nes2C02::GetScreen()
{
  return sprScreen;
}

olc::Sprite &nes2C02::GetNameTable(uint8_t i)
{
  return sprNameTable[i];
}

olc::Sprite &nes2C02::GetPatternTable(uint8_t i)
{
  return sprPatternTable[i];
}

void nes2C02::clock()
{
  // Fake some noise for now
  sprScreen.SetPixel(cycle - 1, scanline, palScreen[(rand() % 2) ? 0x3F : 0x30]);

  // Advance renderer - it never stops
  cycle++;
  if (cycle >= 341) {
    cycle = 0;
    scanline++;
    if (scanline >= 261) {
      scanline = -1;
      frame_complete = true;
    }
  }
}

uint8_t nes2C02::cpuRead(uint16_t addr, bool bReadOnly)
{
  uint8_t data = 0x00;

  switch (addr) {
  case 0x0000:// Control
    break;
  case 0x0001:// Mask
    break;
  case 0x0002:// Status
    break;
  case 0x0003:// OAM Address
    break;
  case 0x0004:// OAM data
    break;
  case 0x0005:// Scroll
    break;
  case 0x0006:// PPU Address
    break;
  case 0x0007:// PPU data
    break;
  }
  return data;
}

void nes2C02::cpuWrite(uint16_t addr, uint8_t data)
{
  switch (addr) {
  case 0x0000:// Control
    break;
  case 0x0001:// Mask
    break;
  case 0x0002:// Status
    break;
  case 0x0003:// OAM Address
    break;
  case 0x0004:// OAM data
    break;
  case 0x0005:// Scroll
    break;
  case 0x0006:// PPU Address
    break;
  case 0x0007:// PPU data
    break;
  }
}

// Communications with the PPU bus
uint8_t nes2C02::ppuRead(uint16_t addr, bool bReadOnly)
{
  uint8_t data = 0x00;
  addr &= 0x3FFF;

  if (cart->ppuRead(addr, data)) {
    // Cartridge address range
  }
  return data;
}

void nes2C02::ppuWrite(uint16_t addr, uint8_t data)
{
  addr &= 0x3FFF;
  if (cart->ppuWrite(addr, data)) {
    // Cartridge address range
  }
}

void nes2C02::ConnectCartridge(const std::shared_ptr<Cartridge> &cartridge)
{
  this->cart = cartridge;
}
