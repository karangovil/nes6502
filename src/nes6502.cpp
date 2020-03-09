#include "nes6502.h"
#include "Bus.h"
#include "utils.h"

nes6502::nes6502()
{
}

nes6502::~nes6502()
{
}

uint8_t nes6502::read(uint16_t addr)
{
  return bus->read(addr, false);
}

void nes6502::write(uint16_t addr, uint8_t data)
{
  bus->write(addr, data);
}

void nes6502::clock()
{
  if (cycles == 0) {
    opcode = read(pc);
    pc++;

    // Get starting number of cycles
    cycles = lookup[opcode].cycles;

    uint8_t additional_cycle1 = (this->*lookup[opcode].addrmode)();

    uint8_t additional_cycle2 = (this->*lookup[opcode].operate)();

    cycles += (additional_cycle1 & additional_cycle2);
  }

  cycles--;
}

// Forces the CPU into a known state.
// Registers are set to 0x00, status register is cleared
// except for unused bit. An absolute address is read from
// location 0xFFFC which contains a second address that the
// program counter is set to. This is known to the programmer
void nes6502::reset()
{
  addr_abs = 0xFFFC;
  uint16_t lo = read(addr_abs + 0);
  uint16_t hi = read(addr_abs + 1);
  pc = (hi << 8) | lo;

  a = 0;
  x = 0;
  y = 0;
  stkp = 0xFD;
  status = 0x00 | U;

  addr_rel = 0x0000;
  addr_abs = 0x0000;
  fetched = 0x00;

  cycles = 8;
}

// Interrupt reqs only happen if the "disable interrupt"
// flag is 0. IRQs can be happen anytime but the current
// instr is allowed to finish and then the current pc and
// status register is stored on stack. When the routine that
// services the interrupt is finished, status register and pc
// can be restored (RTI instr). Once IRQ is finished, a
// programmable addr is read from a hard coded location, 0x00FE
void nes6502::irq()
{
  if (GetFlag(I) == 0) {
    write(0x0100 + stkp, (pc << 8) & 0x00FF);
    stkp--;
    write(0x0100 + stkp, pc & 0x00FF);
    stkp--;

    SetFlag(B, 0);
    SetFlag(U, 1);
    SetFlag(I, 1);
    write(0x0100 + stkp, status);
    stkp--;

    addr_abs = 0xFFFE;
    uint16_t lo = read(addr_abs + 0);
    uint16_t hi = read(addr_abs + 1);
    pc = (hi << 8) | lo;

    cycles = 7;
  }
}

// A non maskable interrupt cannot be ignored.
// Same as irq except reads the new pc from 0xFFFA
void nes6502::nmi()
{
  write(0x0100 + stkp, (pc << 8) & 0x00FF);
  stkp--;
  write(0x0100 + stkp, pc & 0x00FF);
  stkp--;

  SetFlag(B, 0);
  SetFlag(U, 1);
  SetFlag(I, 1);
  write(0x0100 + stkp, status);
  stkp--;

  addr_abs = 0xFFFE;
  uint16_t lo = read(addr_abs + 0);
  uint16_t hi = read(addr_abs + 1);
  pc = (hi << 8) | lo;

  cycles = 8;
}

// Flag functions

// Get the value of the given flag
uint8_t nes6502::GetFlag(FLAGS6502 f)
{
  return ((status & f) > 0) ? 1 : 0;
}

// Set or clear thje value of the given flag
void nes6502::SetFlag(FLAGS6502 f, bool v)
{
  if (v)
    status |= f;
  else
    status &= ~f;
}

// Addressing Modes

// Implied: There is no additional data required for this instr
// The instr does something very simple let sets a status bit
// We will target the accumulator, for instrs like PHA
uint8_t nes6502::IMP()
{
  fetched = a;
  return 0;
}

// Immediate: The instr expects the next byte to be used as a
// value, so we'll prep the read addr to point to next byte
uint8_t nes6502::IMM()
{
  addr_abs = pc++;
  return 0;
}

// Zero page: To save program bytes, zero page addressing
// allows to absolutely address a location in the first 0xFF
// bytes of addr range which only requires one byte instead
// of the usual two.
uint8_t nes6502::ZP0()
{
  addr_abs = read(pc);
  pc++;
  addr_abs &= 0x00FF;// extract LO 8 bytes of the addr
  return 0;
}

// Zero page with X offset: Similar to ZP0 but contents of X
// register are added to the supplied single byte addr. Useful
// for iterating through range within first page
uint8_t nes6502::ZPX()
{
  addr_abs = (read(pc) + x);
  pc++;
  addr_abs &= 0x00FF;
  return 0;
}

// Zero page with Y offset
uint8_t nes6502::ZPY()
{
  addr_abs = (read(pc) + y);
  pc++;
  addr_abs &= 0x00FF;
  return 0;
}

// Relative: Exclusive to branch instrs. The addr must
// reside within -128 to 127 of the branch instr i.e.
// cannot directly branch of any addr in the addressable range
uint8_t nes6502::REL()
{
  addr_rel = read(pc);
  pc++;
  if (addr_rel & 0x80)
    addr_rel |= 0xFF00;
  return 0;
}

// Absolute: Full 16 bit addr is loaded and used
uint8_t nes6502::ABS()
{
  uint16_t lo = read(pc);
  pc++;
  uint16_t hi = read(pc);
  pc++;

  addr_abs = (hi << 8) | lo;

  return 0;
}

// Absolute with X Offset: Same as absolute but X register
// is added to the supplied 2 byte addr. If the resulting addr
// changes the page, addition cycle is needed
uint8_t nes6502::ABX()
{
  uint16_t lo = read(pc);
  pc++;
  uint16_t hi = read(pc);
  pc++;

  addr_abs = (hi << 8) | lo;
  addr_abs += x;

  // check if the HI byte has changed after adding X
  if ((addr_abs & 0xFF00) != (hi << 8))
    return 1;
  else
    return 0;
}

// Absolute with Y Offset
uint8_t nes6502::ABY()
{
  uint16_t lo = read(pc);
  pc++;
  uint16_t hi = read(pc);
  pc++;

  addr_abs = (hi << 8) | lo;
  addr_abs += y;

  if ((addr_abs & 0xFF00) != (hi << 8))
    return 1;
  else
    return 0;
}

// Indirect: The supplied 16-bit addr is read to get the
// actual 16-bit addr. Bug: If the LO byte of the supplied
// addr is 0xFF, then to read the HI byte of the actual addr
// we need to cross a page bdry. This doesn't actually work
// on the chip as designed, instead it wraps back around in
// the same page, yielding in invalid actual addr
uint8_t nes6502::IND()
{
  uint16_t ptr_lo = read(pc);
  pc++;
  uint16_t ptr_hi = read(pc);
  pc++;

  uint16_t ptr = (ptr_hi << 8) | ptr_lo;

  if (ptr_lo == 0x00FF)// Simulate page boundary hardware bug
  {
    addr_abs = (read(ptr & 0xFF00) << 8) | read(ptr + 0);
  } else// behave normally
  {
    addr_abs = (read(ptr + 1) << 8) | read(ptr + 0);
  }

  return 0;
}

// Indirect X: Supplied 8 bit addr is offset by X register
// to a location in page 0x00. The actual 16 bit addr is read
// from this location
uint8_t nes6502::IZX()
{
  uint16_t t = read(pc);
  pc++;

  uint16_t lo = read((uint16_t)(t + (uint16_t)x) & 0x00FF);
  uint16_t hi = read((uint16_t)(t + (uint16_t)x + 1) & 0x00FF);

  addr_abs = (hi << 8) | lo;

  return 0;
}

// Indirect Y: Supplied 8 bit addr indexes a location in page 0x00
// From here, the actual 16 bit addr is read, and the contents of Y
// register is added to offset it. If the offset causes a change in
// page, then an additional clock cycle is needed
uint8_t nes6502::IZY()
{
  uint16_t t = read(pc);
  pc++;

  uint16_t lo = read(t & 0x00FF);
  uint16_t hi = read((t + 1) & 0x00FF);

  addr_abs = (hi << 8) | lo;
  addr_abs += y;

  if ((addr_abs & 0xFF00) != (hi << 8))
    return 1;
  else
    return 0;
}

// Instructions

// Function to source the data required by the instr
uint8_t nes6502::fetch()
{
  if (!(lookup[opcode].addrmode == &nes6502::IMP))
    fetched = read(addr_abs);
  return fetched;
}

// Add with carry in
uint8_t nes6502::ADC()
{
  fetch();
  // add in 16 bit to capture any carry bit which
  // will exist in bit 8 of 16 bit word
  temp = (uint16_t)a + (uint16_t)fetched + (uint16_t)GetFlag(C);
  // carry flag out exists in the HI bit 0
  SetFlag(C, temp > 255);
  SetFlag(Z, (temp & 0x00FF) == 0x0000);
  // set the signed overflow flag
  SetFlag(V, (~((uint16_t)a ^ (uint16_t)fetched) & ((uint16_t)a ^ (uint16_t)temp)) & 0x0080);
  SetFlag(N, temp & 0x80);
  // load the result into accumulator (8-bit)
  a = temp & 0x00FF;

  return 1;
}

uint8_t nes6502::SBC()
{
  fetch();
  uint16_t value = ((uint16_t)fetched) ^ 0x00FF;
  temp = (uint16_t)a + value + (uint16_t)GetFlag(C);
  SetFlag(C, temp < 255);
  SetFlag(Z, (temp & 0x00FF) == 0x0000);
  SetFlag(V, (temp ^ (uint16_t)a) & (temp ^ value) & 0x0080);
  SetFlag(N, temp & 0x80);
  a = temp & 0x00FF;

  return 1;
}

// Typical order of events for most instrs
// 1. Fetch the data
// 2. Perform computation
// 3. Store the results in appropriate places
// 4. Set flags of the status register
// 5. Return if the instr has potential to require
//    additional clock cycle

// Bitwise logic AND
// Flags out: N, Z
uint8_t nes6502::AND()
{
  fetch();
  a = a & fetched;
  SetFlag(Z, a == 0x00);
  SetFlag(N, a & 0x80);
  return 1;
}

// Bitwise logic XOR
// Flags out: N, Z
uint8_t nes6502::EOR()
{
  fetch();
  a = a ^ fetched;
  SetFlag(Z, a == 0x00);
  SetFlag(N, a & 0x80);
  return 1;
}

// Bitwise logic OR
// Flags out: N, Z
uint8_t nes6502::ORA()
{
  fetch();
  a = a | fetched;
  SetFlag(Z, a == 0x00);
  SetFlag(N, a & 0x80);
  return 1;
}

// Test bits in memory with accumulator
uint8_t nes6502::BIT()
{
  fetch();
  temp = a & fetched;
  SetFlag(Z, (temp & 0x00FF) == 0x00);
  SetFlag(N, fetched & (1 << 7));
  SetFlag(V, fetched & (1 << 6));
  return 0;
}

// Branch if Carry set
uint8_t nes6502::BCS()
{
  if (GetFlag(C) == 1) {
    cycles++;
    addr_abs = pc + addr_rel;

    if ((addr_abs & 0xFF00) != (pc & 0xFF00))
      cycles++;

    pc = addr_abs;
  }
  return 0;
}

// Branch if Carry clear
uint8_t nes6502::BCC()
{
  if (GetFlag(C) == 0) {
    cycles++;
    addr_abs = pc + addr_rel;

    if ((addr_abs & 0xFF00) != (pc & 0xFF00))
      cycles++;

    pc = addr_abs;
  }
  return 0;
}

// Branch if Equal
uint8_t nes6502::BEQ()
{
  if (GetFlag(Z) == 1) {
    cycles++;
    addr_abs = pc + addr_rel;

    if ((addr_abs & 0xFF00) != (pc & 0xFF00))
      cycles++;

    pc = addr_abs;
  }
  return 0;
}

// Branch if Not Equal
uint8_t nes6502::BNE()
{
  if (GetFlag(Z) == 0) {
    cycles++;
    addr_abs = pc + addr_rel;

    if ((addr_abs & 0xFF00) != (pc & 0xFF00))
      cycles++;

    pc = addr_abs;
  }
  return 0;
}

// Branch if Negative
uint8_t nes6502::BMI()
{
  if (GetFlag(N) == 1) {
    cycles++;
    addr_abs = pc + addr_rel;

    if ((addr_abs & 0xFF00) != (pc & 0xFF00))
      cycles++;

    pc = addr_abs;
  }
  return 0;
}

// Branch if Positive
uint8_t nes6502::BPL()
{
  if (GetFlag(N) == 0) {
    cycles++;
    addr_abs = pc + addr_rel;

    if ((addr_abs & 0xFF00) != (pc & 0xFF00))
      cycles++;

    pc = addr_abs;
  }
  return 0;
}

// Branch if Overflow Clear
uint8_t nes6502::BVC()
{
  if (GetFlag(V) == 0) {
    cycles++;
    addr_abs = pc + addr_rel;

    if ((addr_abs & 0xFF00) != (pc & 0xFF00))
      cycles++;

    pc = addr_abs;
  }
  return 0;
}

// Branch if Overflow Set
uint8_t nes6502::BVS()
{
  if (GetFlag(V) == 1) {
    cycles++;
    addr_abs = pc + addr_rel;

    if ((addr_abs & 0xFF00) != (pc & 0xFF00))
      cycles++;

    pc = addr_abs;
  }
  return 0;
}

// Program sourced interrupts
uint8_t nes6502::BRK()
{
  pc++;

  SetFlag(I, 1);
  write(0x0100 + stkp, (pc >> 8) & 0x00FF);
  stkp--;
  write(0x0100 + stkp, pc & 0x00FF);

  SetFlag(B, 1);
  write(0x0100 + stkp, status);
  stkp--;
  SetFlag(B, 0);

  pc = (uint16_t)read(0xFFFE) | ((uint16_t)read(0xFFFF) << 8);

  return 0;
}

// Clear Carry flag
uint8_t nes6502::CLC()
{
  SetFlag(C, false);
  return 0;
}

// Clear Decimal flag
uint8_t nes6502::CLD()
{
  SetFlag(D, false);
  return 0;
}

// Disable Interrupts / Clear Interrupts flag
uint8_t nes6502::CLI()
{
  SetFlag(I, false);
  return 0;
}

// Clear Overflow flag
uint8_t nes6502::CLV()
{
  SetFlag(V, false);
  return 0;
}

// Compare Accumulator
// Flags out: N, C, Z
uint8_t nes6502::CMP()
{
  fetch();
  temp = (uint16_t)a - (uint16_t)fetched;
  SetFlag(C, a >= fetched);
  SetFlag(Z, (temp & 0x00FF) == 0x0000);
  SetFlag(N, temp & 0x0080);
  return 0;
}

// Compare X register
// Flags out: N, C, Z
uint8_t nes6502::CPX()
{
  fetch();
  temp = (uint16_t)x - (uint16_t)fetched;
  SetFlag(C, x >= fetched);
  SetFlag(Z, (temp & 0x00FF) == 0x0000);
  SetFlag(N, temp & 0x0080);
  return 0;
}

// Compare Y register
// Flags out: N, C, Z
uint8_t nes6502::CPY()
{
  fetch();
  temp = (uint16_t)y - (uint16_t)fetched;
  SetFlag(C, y >= fetched);
  SetFlag(Z, (temp & 0x00FF) == 0x0000);
  SetFlag(N, temp & 0x0080);
  return 0;
}

// Decrement value at memory location
// Flags out: N, Z
uint8_t nes6502::DEC()
{
  fetch();
  temp = fetched - 1;
  write(addr_abs, temp & 0x00FF);
  SetFlag(Z, (temp & 0x00FF) == 0x0000);
  SetFlag(N, temp & 0x0080);
  return 0;
}

// Decrement X register
// Flags out: N, Z
uint8_t nes6502::DEX()
{
  x--;
  SetFlag(Z, x == 0x00);
  SetFlag(N, temp & 0x80);
  return 0;
}

// Decrement Y register
// Flags out: N, Z
uint8_t nes6502::DEY()
{
  y--;
  SetFlag(Z, y == 0x00);
  SetFlag(N, y & 0x80);
  return 0;
}

// Increment value at memory location
// Flags out: N, Z
uint8_t nes6502::INC()
{
  fetch();
  temp = fetched + 1;
  write(addr_abs, temp & 0x00FF);
  SetFlag(Z, (temp & 0x00FF) == 0x0000);
  SetFlag(N, temp & 0x0080);
  return 0;
}

// Increment X register
// Flags out: N, Z
uint8_t nes6502::INX()
{
  x++;
  SetFlag(Z, x == 0x00);
  SetFlag(N, temp & 0x80);
  return 0;
}

// Increment Y register
// Flags out: N, Z
uint8_t nes6502::INY()
{
  y++;
  SetFlag(Z, y == 0x00);
  SetFlag(N, y & 0x80);
  return 0;
}

// Jump to location
uint8_t nes6502::JMP()
{
  pc = addr_abs;
  return 0;
}

// Jump to Sub-Routine
// Push current pc to stack, pc = addr
uint8_t nes6502::JSR()
{
  pc--;

  write(0x0100 + stkp, (pc >> 8) & 0x00FF);
  stkp--;
  write(0x0100 + stkp, pc & 0x00FF);
  stkp--;

  pc = addr_abs;
  return 0;
}

// Load the accumulator
// Flags out: N, Z
uint8_t nes6502::LDA()
{
  fetch();
  a = fetched;
  SetFlag(Z, a == 0x00);
  SetFlag(N, a & 0x80);
  return 1;
}

// Load the X register
// Flags out: N, Z
uint8_t nes6502::LDX()
{
  fetch();
  x = fetched;
  SetFlag(Z, x == 0x00);
  SetFlag(N, x & 0x80);
  return 1;
}

// Load the Y register
// Flags out: N, Z
uint8_t nes6502::LDY()
{
  fetch();
  y = fetched;
  SetFlag(Z, y == 0x00);
  SetFlag(N, y & 0x80);
  return 1;
}

// Shift left one bit
// shift one bt right (memory or accumulator)
uint8_t nes6502::ASL()
{
  fetch();
  temp = (uint16_t)fetched << 1;
  SetFlag(C, (temp & 0xFF00) > 0);
  SetFlag(Z, (temp & 0x00FF) == 0x00);
  SetFlag(N, temp & 0x80);
  if (lookup[opcode].addrmode == &nes6502::IMP)
    a = temp & 0x00FF;
  else
    write(addr_abs, temp & 0x00FF);
  return 0;
}

// Logical Shift right
// shift one bt right (memory or accumulator)
uint8_t nes6502::LSR()
{
  fetch();
  SetFlag(C, fetched & 0x0001);
  temp = fetched >> 1;
  SetFlag(Z, (temp & 0x00FF) == 0x0000);
  SetFlag(N, temp & 0x0080);

  if (lookup[opcode].addrmode == &nes6502::IMP)
    a = temp & 0x00FF;
  else
    write(addr_abs, temp & 0x00FF);

  return 0;
}

uint8_t nes6502::NOP()
{
  switch (opcode) {
  case 0x1C:
  case 0x3C:
  case 0x5C:
  case 0x7C:
  case 0xDC:
  case 0xFC:
    return 1;
    break;
  }
  return 0;
}

// Push accumulator to stack
// Note that addr 0x0100 is hard coded in 6502
// as the base location for the stack pointer
// stkp is an offset to that base location
uint8_t nes6502::PHA()
{
  write(0x0100 + stkp, a);
  stkp--;
  return 0;
}

// Push status register to stack
// Break flag is set to 1 before push
uint8_t nes6502::PHP()
{
  write(0x0100 + stkp, status | B | U);
  SetFlag(B, 0);
  SetFlag(U, 0);
  stkp--;
  return 0;
}

// Pop accumulator off the stack
// Flags set: N, Z
uint8_t nes6502::PLA()
{
  stkp++;
  a = read(0x0100 + stkp);
  SetFlag(Z, a = 0x00);
  SetFlag(N, a & 0x80);
  return 0;
}

// Pop status register off the stack
uint8_t nes6502::PLP()
{
  stkp++;
  status = read(0x0100 + stkp);
  SetFlag(U, 1);
  return 0;
}

// Rotate one bit left (memory or accumulator)
uint8_t nes6502::ROL()
{
  fetch();
  temp = (uint16_t)(fetched << 1) | GetFlag(C);
  SetFlag(C, temp & 0x01);
  SetFlag(Z, (temp & 0x00FF) == 0x00);
  SetFlag(N, temp & 0x0080);
  if (lookup[opcode].addrmode == &nes6502::IMP)
    a = temp & 0x00FF;
  else
    write(addr_abs, temp & 0x00FF);

  return 0;
}

// Rotate one bit right (memory or accumulator)
uint8_t nes6502::ROR()
{
  fetch();
  temp = (uint16_t)(GetFlag(C) << 7) | (fetched << 1);
  SetFlag(C, fetched & 0x01);
  SetFlag(Z, (temp & 0x00FF) == 0x00);
  SetFlag(N, temp & 0x0080);
  if (lookup[opcode].addrmode == &nes6502::IMP)
    a = temp & 0x00FF;
  else
    write(addr_abs, temp & 0x00FF);

  return 0;
}

// Return from interrupt
uint8_t nes6502::RTI()
{

  stkp++;
  status = read(0x0100 + stkp);
  status &= ~B;
  status &= ~U;

  stkp++;
  pc = (uint16_t)read(0x0100 + stkp);
  stkp++;
  pc |= (uint16_t)read(0x0100 + stkp) << 8;

  return 0;
}

uint8_t nes6502::RTS()
{
  stkp++;
  pc = (uint16_t)read(0x0100 + stkp);
  stkp++;
  pc |= (uint16_t)read(0x0100 + stkp) << 8;

  pc++;

  return 0;
}

// Set Carry flag
uint8_t nes6502::SEC()
{
  SetFlag(C, true);
  return 0;
}

// Set Decimal flag
uint8_t nes6502::SED()
{
  SetFlag(D, true);
  return 0;
}

// Set Interrupt flag / Enable interrupts
uint8_t nes6502::SEI()
{
  SetFlag(I, true);
  return 0;
}

// Store Accumulator at addr
uint8_t nes6502::STA()
{
  write(addr_abs, a);
  return 0;
}

// Store X register at addr
uint8_t nes6502::STX()
{
  write(addr_abs, x);
  return 0;
}

// Store Y register at addr
uint8_t nes6502::STY()
{
  write(addr_abs, y);
  return 0;
}

// Transfer Accumulator to X register
// Flags out: N, Z
uint8_t nes6502::TAX()
{
  x = a;
  SetFlag(Z, x == 0x00);
  SetFlag(N, x & 0x80);
  return 0;
}

// Transfer Accumulator to Y register
// Flags out: N, Z
uint8_t nes6502::TAY()
{
  y = a;
  SetFlag(Z, y == 0x00);
  SetFlag(N, y & 0x80);
  return 0;
}

// Transfer X register to Accumulator
// Flags out: N, Z
uint8_t nes6502::TXA()
{
  a = x;
  SetFlag(Z, a == 0x00);
  SetFlag(N, a & 0x80);
  return 0;
}

// Transfer Y register to Accumulator
// Flags out: N, Z
uint8_t nes6502::TYA()
{
  a = y;
  SetFlag(Z, a == 0x00);
  SetFlag(N, a & 0x80);
  return 0;
}

// Transfer X register to Stack pointer
uint8_t nes6502::TXS()
{
  stkp = x;
  return 0;
}

// Transfer Stack register to X register
// Flags out: N, Z
uint8_t nes6502::TSX()
{
  x = stkp;
  SetFlag(Z, x == 0x00);
  SetFlag(N, x & 0x80);
  return 0;
}

// Capture illegal opcodes
uint8_t nes6502::XXX()
{
  return 0;
}

// Helper functions

bool nes6502::complete()
{
  return cycles == 0;
}

std::map<uint16_t, std::string> nes6502::disassemble(uint16_t nStart, uint16_t nStop)
{
  uint32_t addr = nStart;
  uint8_t value = 0x00;
  uint8_t lo = 0x00;
  uint8_t hi = 0x00;
  std::map<uint16_t, std::string> mapLines;
  uint16_t line_addr = 0;

  while (addr <= (uint32_t)nStop) {
    line_addr = addr;

    // Prefix line with instr addr
    std::string sInst = "$" + hex(addr, 4) + ": ";

    // read instr and get its readable name
    uint8_t opcode = bus->read(addr, true);
    addr++;
    sInst += lookup[opcode].name + " ";

    // get operands from desired locations, and form the
    // instr based upon its addressing mode. These routines
    // mimmick the actual fetch routine of the 6502 in
    // order to get accurate data as part of the instr
    if (lookup[opcode].addrmode == &nes6502::IMP) {
      sInst += " {IMP}";
    } else if (lookup[opcode].addrmode == &nes6502::IMM) {
      value = bus->read(addr, true);
      addr++;
      sInst += "#$" + hex(value, 2) + " {IMM}";
    } else if (lookup[opcode].addrmode == &nes6502::ZP0) {
      lo = bus->read(addr, true);
      addr++;
      hi = 0x00;
      sInst += "$" + hex(lo, 2) + " {ZP0}";
    } else if (lookup[opcode].addrmode == &nes6502::ZPX) {
      lo = bus->read(addr, true);
      addr++;
      hi = 0x00;
      sInst += "$" + hex(lo, 2) + ", X {ZPX}";
    } else if (lookup[opcode].addrmode == &nes6502::ZPY) {
      lo = bus->read(addr, true);
      addr++;
      hi = 0x00;
      sInst += "$" + hex(lo, 2) + ", Y {ZPY}";
    } else if (lookup[opcode].addrmode == &nes6502::IZX) {
      lo = bus->read(addr, true);
      addr++;
      hi = 0x00;
      sInst += "($" + hex(lo, 2) + ", X) {IZX}";
    } else if (lookup[opcode].addrmode == &nes6502::IZY) {
      lo = bus->read(addr, true);
      addr++;
      hi = 0x00;
      sInst += "($" + hex(lo, 2) + ", Y) {IZY}";
    } else if (lookup[opcode].addrmode == &nes6502::ABS) {
      lo = bus->read(addr, true);
      addr++;
      hi = bus->read(addr, true);
      addr++;
      sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + " {ABS}";
    } else if (lookup[opcode].addrmode == &nes6502::ABX) {
      lo = bus->read(addr, true);
      addr++;
      hi = bus->read(addr, true);
      addr++;
      sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", X {ABX}";
    } else if (lookup[opcode].addrmode == &nes6502::ABY) {
      lo = bus->read(addr, true);
      addr++;
      hi = bus->read(addr, true);
      addr++;
      sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", Y {ABY}";
    } else if (lookup[opcode].addrmode == &nes6502::IND) {
      lo = bus->read(addr, true);
      addr++;
      hi = bus->read(addr, true);
      addr++;
      sInst += "($" + hex((uint16_t)(hi << 8) | lo, 4) + ") {IND}";
    } else if (lookup[opcode].addrmode == &nes6502::REL) {
      value = bus->read(addr, true);
      addr++;
      sInst += "$" + hex(value, 2) + "[$" + hex(addr + value, 4) + "] {REL}";
    }

    // Add the formed string to the map using the instr's addr
    // as the key for lookup later as the instrs as variable
    // in length, so a straight up incremental index is not sufficient
    mapLines[line_addr] = sInst;
  }

  return mapLines;
}
