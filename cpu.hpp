#ifndef CPU_HPP_
#define CPU_HPP_

#include <iostream>
#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <bitset>
#include <sstream>

namespace cpu
{
   static constexpr auto NOP  = 0x00;
   static constexpr auto LDI  = 0x01;
   static constexpr auto MOV  = 0x02;
   static constexpr auto OUT  = 0x03;
   static constexpr auto IN   = 0x04;
   static constexpr auto STS  = 0x05;
   static constexpr auto LDS  = 0x06;
   static constexpr auto ORI  = 0x07;
   static constexpr auto ANDI = 0x08;
   static constexpr auto XORI = 0x09;
   static constexpr auto OR   = 0x0A;
   static constexpr auto AND  = 0x0B;
   static constexpr auto XOR  = 0x0C;
   static constexpr auto CLR  = 0x0D;
   static constexpr auto INC  = 0x0E;
   static constexpr auto DEC  = 0x0F;

   static constexpr auto ADDI = 0x10;
   static constexpr auto SUBI = 0x11;
   static constexpr auto ADD  = 0x12;
   static constexpr auto SUB  = 0x13;
   static constexpr auto CPI  = 0x14;
   static constexpr auto CP   = 0x15;
   static constexpr auto JMP  = 0x16;
   static constexpr auto CALL = 0x17;
   static constexpr auto RET  = 0x18;
   static constexpr auto BREQ = 0x19;
   static constexpr auto BRNE = 0x1A;
   static constexpr auto BRGT = 0x1B;
   static constexpr auto BRGE = 0x1C;
   static constexpr auto BRLT = 0x1D;
   static constexpr auto BRLE = 0x1E;
   static constexpr auto PUSH = 0x1F;

   static constexpr auto POP  = 0x20;
   static constexpr auto SEI  = 0x21;
   static constexpr auto CLI  = 0x22;
   static constexpr auto RETI = 0x23;

   static constexpr auto DDRB   = 0x00;
   static constexpr auto PORTB  = 0x01;
   static constexpr auto PINB   = 0x02;
   static constexpr auto PCICR  = 0x03;
   static constexpr auto PCMSK0 = 0x04;

   static constexpr auto PCIE0 = 0x00;

   static constexpr auto R0 = 0x00;
   static constexpr auto R1 = 0x01;
   static constexpr auto R2 = 0x02;
   static constexpr auto R3 = 0x03;
   static constexpr auto R4 = 0x04;
   static constexpr auto R5 = 0x05;
   static constexpr auto R6 = 0x06;
   static constexpr auto R7 = 0x07;
   static constexpr auto R8 = 0x08;
   static constexpr auto R9 = 0x09;
   static constexpr auto R10 = 0x0A;
   static constexpr auto R11 = 0x0B;
   static constexpr auto R12 = 0x0C;
   static constexpr auto R13 = 0x0D;
   static constexpr auto R14 = 0x0E;
   static constexpr auto R15 = 0x0F;

   static constexpr auto R16 = 0x10;
   static constexpr auto R17 = 0x11;
   static constexpr auto R18 = 0x12;
   static constexpr auto R19 = 0x13;
   static constexpr auto R20 = 0x14;
   static constexpr auto R21 = 0x15;
   static constexpr auto R22 = 0x16;
   static constexpr auto R23 = 0x17;
   static constexpr auto R24 = 0x18;
   static constexpr auto R25 = 0x19;
   static constexpr auto R26 = 0x1A;
   static constexpr auto R27 = 0x1B;
   static constexpr auto R28 = 0x1C;
   static constexpr auto R29 = 0x1D;
   static constexpr auto R30 = 0x1E;
   static constexpr auto R31 = 0x1F;

   enum class state
   {
      fetch,
      decode,
      execute
   };

   template<class T = std::uint8_t>
   static inline void set(T& reg, const std::uint8_t bit)
   {
      reg |= (1 << bit);
      return;
   }

   template<class T = std::uint8_t>
   static inline void clr(T& reg, const std::uint8_t bit)
   {
      reg &= ~(1 << bit);
      return;
   }

   template<class T = std::uint8_t>
   static inline T read(const T data, const std::uint8_t bit)
   {
      return (data & (1 << bit));
   }

   static const char* instruction_name(const std::uint8_t instruction)
   {
      if (instruction == NOP) return "NOP";
      else if (instruction == LDI) return "LDI";
      else if (instruction == MOV) return "MOV";
      else if (instruction == IN) return "IN";
      else if (instruction == OUT) return "OUT";
      else if (instruction == STS) return "STS";
      else if (instruction == LDS) return "LDS";
      else if (instruction == ORI) return "ORI";
      else if (instruction == ANDI) return "ANDI";
      else if (instruction == XORI) return "XORI";
      else if (instruction == OR) return "OR";
      else if (instruction == AND) return "AND";
      else if (instruction == XOR) return "XOR";
      else if (instruction == CLR) return "CLR";
      else if (instruction == INC) return "INC";
      else if (instruction == DEC) return "DEC";
      else if (instruction == ADDI) return "ADDI";
      else if (instruction == SUBI) return "SUBI";
      else if (instruction == ADDI) return "ADD";
      else if (instruction == SUBI) return "SUB";
      else if (instruction == CPI) return "CPI";
      else if (instruction == CP) return "CP";
      else if (instruction == BREQ) return "BREQ";
      else if (instruction == BRNE) return "BRNE";
      else if (instruction == BRGE) return "BRGE";
      else if (instruction == BRGT) return "BRGT";
      else if (instruction == BRLE) return "BRLE";
      else if (instruction == BRLT) return "BRLT";
      else if (instruction == JMP) return "JMP";
      else if (instruction == CALL) return "CALL";
      else if (instruction == RET) return "RET";
      else if (instruction == PUSH) return "PUSH";
      else if (instruction == POP) return "POP";
      else if (instruction == SEI) return "SEI";
      else if (instruction == CLI) return "CLI";
      else if (instruction == RETI) return "RETI";
      else return "Unknown";
   }

   static const char* state_name(const enum state state)
   {
      if (state == state::fetch) return "Fetch";
      else if (state == state::decode) return "Decode";
      else if (state == state::execute) return "Execute";
      else return "Unknown";
   }

   struct control_unit;
   struct program_memory;

   template<class T = std::uint8_t>
   struct data_memory;

   template<class T = std::uint8_t>
   struct stack;
}

#include "program_memory.hpp"
#include "data_memory.hpp"
#include "control_unit.hpp"
#include "stack.hpp"

#endif /* CPU_HPP_ */