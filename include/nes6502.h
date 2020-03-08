#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

class Bus;

class nes6502
{
public:
  nes6502();
  ~nes6502();

public:
  enum FLAGS6502 {
    C = (1 << 0),// Carry bit
    Z = (1 << 1),// Zero
    I = (1 << 2),// Disable interrups
    D = (1 << 3),// Decimal mode (unused in this implementation)
    B = (1 << 4),// Break
    U = (1 << 5),// Unused
    V = (1 << 6),// Overflow
    N = (1 << 7),// Negative
  };

  // CPU core registers
  uint8_t a = 0x00;// Accumulator register
  uint8_t x = 0x00;// X register
  uint8_t y = 0x00;// Y register
  uint8_t stkp = 0x00;// Stack pointer (points to location on bus)
  uint8_t pc = 0x00;// Program counter
  uint8_t status = 0x00;// Status register

  void ConnectBus(Bus *n) { bus = n; }

  // clang-format off
  // Addressing modes
  uint8_t IMP();    uint8_t IMM();
  uint8_t ZP0();    uint8_t ZPX();
  uint8_t ZPY();    uint8_t REL();
  uint8_t ABS();    uint8_t ABX();
  uint8_t ABY();    uint8_t IND();
  uint8_t IZX();    uint8_t IZY();

  // Opcodes
  uint8_t ADC();  uint8_t AND();  uint8_t ASL();  uint8_t BCC();
  uint8_t BCS();  uint8_t BEQ();  uint8_t BIT();  uint8_t BMI();
  uint8_t BNE();  uint8_t BPL();  uint8_t BRK();  uint8_t BVC();
  uint8_t BVS();  uint8_t CLC();  uint8_t CLD();  uint8_t CLI();
  uint8_t CLV();  uint8_t CMP();  uint8_t CPX();  uint8_t CPY();
  uint8_t DEC();  uint8_t DEX();  uint8_t DEY();  uint8_t EOR();
  uint8_t INC();  uint8_t INX();  uint8_t INY();  uint8_t JMP();
  uint8_t JSR();  uint8_t LDA();  uint8_t LDX();  uint8_t LDY();
  uint8_t LSR();  uint8_t NOP();  uint8_t ORA();  uint8_t PHA();
  uint8_t PHP();  uint8_t PLA();  uint8_t PLP();  uint8_t ROL();
  uint8_t ROR();  uint8_t RTI();  uint8_t RTS();  uint8_t SBC();
  uint8_t SEC();  uint8_t SED();  uint8_t SEI();  uint8_t STA();
  uint8_t STX();  uint8_t STY();  uint8_t TAX();  uint8_t TAY();
  uint8_t TSX();  uint8_t TXA();  uint8_t TXS();  uint8_t TYA();
  // clang-format on

  uint8_t XXX();

  void clock();// Perform one clock cycle's worth of update
  void reset();// Reset interrupt - Forces CPU into a known state
  void irq();// Interrupt request - Executes an instruction at a specific location
  void nmi();// Non-Maskable interrupt request - As above, but cannot be disabled

  // Indicates the current instr has completed by returning true.
  // Utility fn to enable step-by-step execution without manually
  // clocking every cycle
  bool complete();

  // Produces a map of strings, with keys equivalent to instr start
  // locations in memory, for the specified addr range
  std::map<uint16_t, std::string> disassemble(uint16_t nStart,
    uint16_t nStop);

private:
  // Read location of data an come from memory addr
  // or its immediately available as part of instr.
  // This fn decides depending on address mode of instr
  uint8_t fetch();

  uint8_t fetched = 0x00;// Represents the working input value to ALU
  uint16_t temp = 0x0000;// A convenience variable
  uint16_t addr_abs = 0x0000;// All used memory addrs end up here
  uint16_t addr_rel = 0x00;// Absolute addr following a branch
  uint8_t opcode = 0x00;
  uint8_t cycles = 0;

private:
  Bus *bus = nullptr;
  uint8_t read(uint16_t a);
  void write(uint16_t a, uint8_t d);

  // Convenience functions to access status register
  uint8_t GetFlag(FLAGS6502 f);
  void SetFlag(FLAGS6502 f, bool v);

private:
  struct INSTRUCTION
  {
    std::string name;
    uint8_t (nes6502::*operate)(void) = nullptr;
    uint8_t (nes6502::*addrmode)(void) = nullptr;
    uint8_t cycles = 0;
  };

  using n = nes6502;
  std::vector<INSTRUCTION> lookup
    // clang-format off
  {
    { "BRK", &n::BRK, &n::IMM, 7 },{ "ORA", &n::ORA, &n::IZX, 6 },{ "???", &n::XXX, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 8 },{ "???", &n::NOP, &n::IMP, 3 },{ "ORA", &n::ORA, &n::ZP0, 3 },{ "ASL", &n::ASL, &n::ZP0, 5 },{ "???", &n::XXX, &n::IMP, 5 },{ "PHP", &n::PHP, &n::IMP, 3 },{ "ORA", &n::ORA, &n::IMM, 2 },{ "ASL", &n::ASL, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 2 },{ "???", &n::NOP, &n::IMP, 4 },{ "ORA", &n::ORA, &n::ABS, 4 },{ "ASL", &n::ASL, &n::ABS, 6 },{ "???", &n::XXX, &n::IMP, 6 },
    { "BPL", &n::BPL, &n::REL, 2 },{ "ORA", &n::ORA, &n::IZY, 5 },{ "???", &n::XXX, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 8 },{ "???", &n::NOP, &n::IMP, 4 },{ "ORA", &n::ORA, &n::ZPX, 4 },{ "ASL", &n::ASL, &n::ZPX, 6 },{ "???", &n::XXX, &n::IMP, 6 },{ "CLC", &n::CLC, &n::IMP, 2 },{ "ORA", &n::ORA, &n::ABY, 4 },{ "???", &n::NOP, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 7 },{ "???", &n::NOP, &n::IMP, 4 },{ "ORA", &n::ORA, &n::ABX, 4 },{ "ASL", &n::ASL, &n::ABX, 7 },{ "???", &n::XXX, &n::IMP, 7 },
    { "JSR", &n::JSR, &n::ABS, 6 },{ "AND", &n::AND, &n::IZX, 6 },{ "???", &n::XXX, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 8 },{ "BIT", &n::BIT, &n::ZP0, 3 },{ "AND", &n::AND, &n::ZP0, 3 },{ "ROL", &n::ROL, &n::ZP0, 5 },{ "???", &n::XXX, &n::IMP, 5 },{ "PLP", &n::PLP, &n::IMP, 4 },{ "AND", &n::AND, &n::IMM, 2 },{ "ROL", &n::ROL, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 2 },{ "BIT", &n::BIT, &n::ABS, 4 },{ "AND", &n::AND, &n::ABS, 4 },{ "ROL", &n::ROL, &n::ABS, 6 },{ "???", &n::XXX, &n::IMP, 6 },
    { "BMI", &n::BMI, &n::REL, 2 },{ "AND", &n::AND, &n::IZY, 5 },{ "???", &n::XXX, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 8 },{ "???", &n::NOP, &n::IMP, 4 },{ "AND", &n::AND, &n::ZPX, 4 },{ "ROL", &n::ROL, &n::ZPX, 6 },{ "???", &n::XXX, &n::IMP, 6 },{ "SEC", &n::SEC, &n::IMP, 2 },{ "AND", &n::AND, &n::ABY, 4 },{ "???", &n::NOP, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 7 },{ "???", &n::NOP, &n::IMP, 4 },{ "AND", &n::AND, &n::ABX, 4 },{ "ROL", &n::ROL, &n::ABX, 7 },{ "???", &n::XXX, &n::IMP, 7 },
    { "RTI", &n::RTI, &n::IMP, 6 },{ "EOR", &n::EOR, &n::IZX, 6 },{ "???", &n::XXX, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 8 },{ "???", &n::NOP, &n::IMP, 3 },{ "EOR", &n::EOR, &n::ZP0, 3 },{ "LSR", &n::LSR, &n::ZP0, 5 },{ "???", &n::XXX, &n::IMP, 5 },{ "PHA", &n::PHA, &n::IMP, 3 },{ "EOR", &n::EOR, &n::IMM, 2 },{ "LSR", &n::LSR, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 2 },{ "JMP", &n::JMP, &n::ABS, 3 },{ "EOR", &n::EOR, &n::ABS, 4 },{ "LSR", &n::LSR, &n::ABS, 6 },{ "???", &n::XXX, &n::IMP, 6 },
    { "BVC", &n::BVC, &n::REL, 2 },{ "EOR", &n::EOR, &n::IZY, 5 },{ "???", &n::XXX, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 8 },{ "???", &n::NOP, &n::IMP, 4 },{ "EOR", &n::EOR, &n::ZPX, 4 },{ "LSR", &n::LSR, &n::ZPX, 6 },{ "???", &n::XXX, &n::IMP, 6 },{ "CLI", &n::CLI, &n::IMP, 2 },{ "EOR", &n::EOR, &n::ABY, 4 },{ "???", &n::NOP, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 7 },{ "???", &n::NOP, &n::IMP, 4 },{ "EOR", &n::EOR, &n::ABX, 4 },{ "LSR", &n::LSR, &n::ABX, 7 },{ "???", &n::XXX, &n::IMP, 7 },
    { "RTS", &n::RTS, &n::IMP, 6 },{ "ADC", &n::ADC, &n::IZX, 6 },{ "???", &n::XXX, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 8 },{ "???", &n::NOP, &n::IMP, 3 },{ "ADC", &n::ADC, &n::ZP0, 3 },{ "ROR", &n::ROR, &n::ZP0, 5 },{ "???", &n::XXX, &n::IMP, 5 },{ "PLA", &n::PLA, &n::IMP, 4 },{ "ADC", &n::ADC, &n::IMM, 2 },{ "ROR", &n::ROR, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 2 },{ "JMP", &n::JMP, &n::IND, 5 },{ "ADC", &n::ADC, &n::ABS, 4 },{ "ROR", &n::ROR, &n::ABS, 6 },{ "???", &n::XXX, &n::IMP, 6 },
    { "BVS", &n::BVS, &n::REL, 2 },{ "ADC", &n::ADC, &n::IZY, 5 },{ "???", &n::XXX, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 8 },{ "???", &n::NOP, &n::IMP, 4 },{ "ADC", &n::ADC, &n::ZPX, 4 },{ "ROR", &n::ROR, &n::ZPX, 6 },{ "???", &n::XXX, &n::IMP, 6 },{ "SEI", &n::SEI, &n::IMP, 2 },{ "ADC", &n::ADC, &n::ABY, 4 },{ "???", &n::NOP, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 7 },{ "???", &n::NOP, &n::IMP, 4 },{ "ADC", &n::ADC, &n::ABX, 4 },{ "ROR", &n::ROR, &n::ABX, 7 },{ "???", &n::XXX, &n::IMP, 7 },
    { "???", &n::NOP, &n::IMP, 2 },{ "STA", &n::STA, &n::IZX, 6 },{ "???", &n::NOP, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 6 },{ "STY", &n::STY, &n::ZP0, 3 },{ "STA", &n::STA, &n::ZP0, 3 },{ "STX", &n::STX, &n::ZP0, 3 },{ "???", &n::XXX, &n::IMP, 3 },{ "DEY", &n::DEY, &n::IMP, 2 },{ "???", &n::NOP, &n::IMP, 2 },{ "TXA", &n::TXA, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 2 },{ "STY", &n::STY, &n::ABS, 4 },{ "STA", &n::STA, &n::ABS, 4 },{ "STX", &n::STX, &n::ABS, 4 },{ "???", &n::XXX, &n::IMP, 4 },
    { "BCC", &n::BCC, &n::REL, 2 },{ "STA", &n::STA, &n::IZY, 6 },{ "???", &n::XXX, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 6 },{ "STY", &n::STY, &n::ZPX, 4 },{ "STA", &n::STA, &n::ZPX, 4 },{ "STX", &n::STX, &n::ZPY, 4 },{ "???", &n::XXX, &n::IMP, 4 },{ "TYA", &n::TYA, &n::IMP, 2 },{ "STA", &n::STA, &n::ABY, 5 },{ "TXS", &n::TXS, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 5 },{ "???", &n::NOP, &n::IMP, 5 },{ "STA", &n::STA, &n::ABX, 5 },{ "???", &n::XXX, &n::IMP, 5 },{ "???", &n::XXX, &n::IMP, 5 },
    { "LDY", &n::LDY, &n::IMM, 2 },{ "LDA", &n::LDA, &n::IZX, 6 },{ "LDX", &n::LDX, &n::IMM, 2 },{ "???", &n::XXX, &n::IMP, 6 },{ "LDY", &n::LDY, &n::ZP0, 3 },{ "LDA", &n::LDA, &n::ZP0, 3 },{ "LDX", &n::LDX, &n::ZP0, 3 },{ "???", &n::XXX, &n::IMP, 3 },{ "TAY", &n::TAY, &n::IMP, 2 },{ "LDA", &n::LDA, &n::IMM, 2 },{ "TAX", &n::TAX, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 2 },{ "LDY", &n::LDY, &n::ABS, 4 },{ "LDA", &n::LDA, &n::ABS, 4 },{ "LDX", &n::LDX, &n::ABS, 4 },{ "???", &n::XXX, &n::IMP, 4 },
    { "BCS", &n::BCS, &n::REL, 2 },{ "LDA", &n::LDA, &n::IZY, 5 },{ "???", &n::XXX, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 5 },{ "LDY", &n::LDY, &n::ZPX, 4 },{ "LDA", &n::LDA, &n::ZPX, 4 },{ "LDX", &n::LDX, &n::ZPY, 4 },{ "???", &n::XXX, &n::IMP, 4 },{ "CLV", &n::CLV, &n::IMP, 2 },{ "LDA", &n::LDA, &n::ABY, 4 },{ "TSX", &n::TSX, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 4 },{ "LDY", &n::LDY, &n::ABX, 4 },{ "LDA", &n::LDA, &n::ABX, 4 },{ "LDX", &n::LDX, &n::ABY, 4 },{ "???", &n::XXX, &n::IMP, 4 },
    { "CPY", &n::CPY, &n::IMM, 2 },{ "CMP", &n::CMP, &n::IZX, 6 },{ "???", &n::NOP, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 8 },{ "CPY", &n::CPY, &n::ZP0, 3 },{ "CMP", &n::CMP, &n::ZP0, 3 },{ "DEC", &n::DEC, &n::ZP0, 5 },{ "???", &n::XXX, &n::IMP, 5 },{ "INY", &n::INY, &n::IMP, 2 },{ "CMP", &n::CMP, &n::IMM, 2 },{ "DEX", &n::DEX, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 2 },{ "CPY", &n::CPY, &n::ABS, 4 },{ "CMP", &n::CMP, &n::ABS, 4 },{ "DEC", &n::DEC, &n::ABS, 6 },{ "???", &n::XXX, &n::IMP, 6 },
    { "BNE", &n::BNE, &n::REL, 2 },{ "CMP", &n::CMP, &n::IZY, 5 },{ "???", &n::XXX, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 8 },{ "???", &n::NOP, &n::IMP, 4 },{ "CMP", &n::CMP, &n::ZPX, 4 },{ "DEC", &n::DEC, &n::ZPX, 6 },{ "???", &n::XXX, &n::IMP, 6 },{ "CLD", &n::CLD, &n::IMP, 2 },{ "CMP", &n::CMP, &n::ABY, 4 },{ "NOP", &n::NOP, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 7 },{ "???", &n::NOP, &n::IMP, 4 },{ "CMP", &n::CMP, &n::ABX, 4 },{ "DEC", &n::DEC, &n::ABX, 7 },{ "???", &n::XXX, &n::IMP, 7 },
    { "CPX", &n::CPX, &n::IMM, 2 },{ "SBC", &n::SBC, &n::IZX, 6 },{ "???", &n::NOP, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 8 },{ "CPX", &n::CPX, &n::ZP0, 3 },{ "SBC", &n::SBC, &n::ZP0, 3 },{ "INC", &n::INC, &n::ZP0, 5 },{ "???", &n::XXX, &n::IMP, 5 },{ "INX", &n::INX, &n::IMP, 2 },{ "SBC", &n::SBC, &n::IMM, 2 },{ "NOP", &n::NOP, &n::IMP, 2 },{ "???", &n::SBC, &n::IMP, 2 },{ "CPX", &n::CPX, &n::ABS, 4 },{ "SBC", &n::SBC, &n::ABS, 4 },{ "INC", &n::INC, &n::ABS, 6 },{ "???", &n::XXX, &n::IMP, 6 },
    { "BEQ", &n::BEQ, &n::REL, 2 },{ "SBC", &n::SBC, &n::IZY, 5 },{ "???", &n::XXX, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 8 },{ "???", &n::NOP, &n::IMP, 4 },{ "SBC", &n::SBC, &n::ZPX, 4 },{ "INC", &n::INC, &n::ZPX, 6 },{ "???", &n::XXX, &n::IMP, 6 },{ "SED", &n::SED, &n::IMP, 2 },{ "SBC", &n::SBC, &n::ABY, 4 },{ "NOP", &n::NOP, &n::IMP, 2 },{ "???", &n::XXX, &n::IMP, 7 },{ "???", &n::NOP, &n::IMP, 4 },{ "SBC", &n::SBC, &n::ABX, 4 },{ "INC", &n::INC, &n::ABX, 7 },{ "???", &n::XXX, &n::IMP, 7 },
  };
  // clang-format on
};
