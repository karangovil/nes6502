#include <iostream>
#include <sstream>
#include <map>
#include <cstdint>
#include <string>

#include "Bus.h"
#include "nes6502.h"
#include "utils.h"

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

class Demo_nes6502
  : public olc::PixelGameEngine
{
public:
  Demo_nes6502() { sAppName = "nes6502 Demonstration"; }

  Bus bus;

  std::map<uint16_t, std::string> mapAsm;

  void DrawRam(int x, int y, uint16_t nAddr, int nRows, int nCols)
  {
    int nRamX = x;
    int nRamY = y;
    for (int row = 0; row < nRows; ++row) {
      std::string sOffset = "$" + hex(nAddr, 4) + ":";
      for (int col = 0; col < nCols; ++col) {
        sOffset += " " + hex(bus.read(nAddr, true), 2);
        nAddr += 1;
      }

      DrawString(nRamX, nRamY, sOffset);
      nRamY += 10;
    }
  }

  void DrawCpu(int x, int y)
  {
    std::string status = "STATUS";
    DrawString(x, y, "STATUS", olc::WHITE);
    DrawString(x + 64, y, "N", bus.cpu.status & nes6502::N ? olc::GREEN : olc::RED);
    DrawString(x + 80, y, "V", bus.cpu.status & nes6502::V ? olc::GREEN : olc::RED);
    DrawString(x + 96, y, "-", bus.cpu.status & nes6502::U ? olc::GREEN : olc::RED);
    DrawString(x + 112, y, "B", bus.cpu.status & nes6502::B ? olc::GREEN : olc::RED);
    DrawString(x + 128, y, "D", bus.cpu.status & nes6502::D ? olc::GREEN : olc::RED);
    DrawString(x + 144, y, "I", bus.cpu.status & nes6502::I ? olc::GREEN : olc::RED);
    DrawString(x + 160, y, "Z", bus.cpu.status & nes6502::Z ? olc::GREEN : olc::RED);
    DrawString(x + 178, y, "C", bus.cpu.status & nes6502::C ? olc::GREEN : olc::RED);
    DrawString(x, y + 10, "PC: $" + hex(bus.cpu.pc, 4));
    DrawString(x, y + 20, "A: $" + hex(bus.cpu.a, 2) + " [" + std::to_string(bus.cpu.a) + "]");
    DrawString(x, y + 30, "X: $" + hex(bus.cpu.x, 2) + " [" + std::to_string(bus.cpu.x) + "]");
    DrawString(x, y + 40, "Y: $" + hex(bus.cpu.y, 2) + " [" + std::to_string(bus.cpu.y) + "]");
    DrawString(x, y + 50, "Stack P: $" + hex(bus.cpu.stkp, 4));
  }

  void DrawCode(int x, int y, int nLines)
  {
    auto it_a = mapAsm.find(bus.cpu.pc);
    int nLineY = (nLines >> 1) * 10 + y;
    if (it_a != mapAsm.end()) {
      DrawString(x, nLineY, (*it_a).second, olc::CYAN);
      while (nLineY < (nLines * 10) + y) {
        nLineY += 10;
        if (++it_a != mapAsm.end()) {
          DrawString(x, nLineY, (*it_a).second);
        }
      }
    }

    it_a = mapAsm.find(bus.cpu.pc);
    nLineY = (nLines >> 1) * 10 + y;
    if (it_a != mapAsm.end()) {
      while (nLineY > y) {
        nLineY -= 10;
        if (--it_a != mapAsm.end()) {
          DrawString(x, nLineY, (*it_a).second);
        }
      }
    }
  }

  bool OnUserCreate() override
  {
    // clang-format off
            // Load program (assembled at https://www.masswerk.at/6502/assembler.html)
            /*
                *=$8000
                LDX #10
                STX $0000
                LDX #3
                STX #0001
                LDY #0000
                LDA #0
                CLC
                loop
                ADC $0001
                DEY
                BNE loop
                STA $0002
                NOP
                NOP
                NOP
            */
    // clang-format on

    std::stringstream ss;
    ss << "A2 0A 8E 00 00 A2 03 8E 01 00 AC 00 00 A9 00 18 6D 01 00 88 D0 FA 8D 02 00 EA EA EA";
    uint16_t nOffset = 0x8000;
    while (!ss.eof()) {
      std::string b;
      ss >> b;
      bus.ram[nOffset++] = (uint8_t)std::stoul(b, nullptr, 16);
    }

    // Set reset vector
    bus.ram[0xFFFC] = 0x00;
    bus.ram[0xFFFD] = 0x80;

    // Extract disassembly
    mapAsm = bus.cpu.disassemble(0x0000, 0xFFFF);

    // Reset
    bus.cpu.reset();
    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    Clear(olc::DARK_BLUE);

    if (GetKey(olc::Key::SPACE).bPressed) {
      do {
        bus.cpu.clock();
      } while (!bus.cpu.complete());
    }

    if (GetKey(olc::Key::R).bPressed)
      bus.cpu.reset();

    if (GetKey(olc::Key::I).bPressed)
      bus.cpu.irq();

    if (GetKey(olc::Key::N).bPressed)
      bus.cpu.nmi();

    // Draw Ram page 0x00
    DrawRam(2, 2, 0x0000, 16, 16);
    DrawRam(2, 182, 0x8000, 16, 16);
    DrawCpu(448, 2);
    DrawCode(448, 72, 26);

    DrawString(10, 370, "SPACE = Step Instruction  R = RESET  I = IRQ  N = NMI");

    return true;
  }
};

int main()
{
  Demo_nes6502 demo;
  demo.Construct(680, 480, 2, 2);
  demo.Start();

  return 0;
}
