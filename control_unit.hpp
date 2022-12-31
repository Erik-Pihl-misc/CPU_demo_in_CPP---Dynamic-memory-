#ifndef CONTROL_UNIT_HPP_
#define CONTROL_UNIT_HPP_

#include "program_memory.hpp"
#include "data_memory.hpp"
#include "stack.hpp"
#include "cpu.hpp"

struct cpu::control_unit
{
   static constexpr auto I = 4;
   static constexpr auto N = 3;
   static constexpr auto Z = 2;
   static constexpr auto V = 1;
   static constexpr auto C = 0;

   static constexpr auto NUM_REGISTERS = 32;
   static constexpr auto DATA_WIDTH = 8;

   program_memory prog_mem;
   data_memory<std::uint8_t> data_mem;
   stack<std::uint8_t> stack;
   std::array<std::uint8_t, NUM_REGISTERS> reg{};

   std::uint8_t pc = 0x00;
   std::uint8_t mar = 0x00;
   std::uint32_t ir = 0x00;
   std::uint8_t sr = 0x00;

   std::uint8_t op_code = 0x00;
   std::uint8_t op1 = 0x00;
   std::uint8_t op2 = 0x00;

   state current_state = state::fetch;

   control_unit(void) 
   {
      data_mem.init(2000);
      stack.init(256);
      return;
   }

   void reset(void)
   {
      data_mem.reset();
      stack.reset();
      pc = 0x00;
      mar = 0x00;
      ir = 0x00;
      sr = 0x00;

      op_code = 0x00;
      op1 = 0x00;
      op2 = 0x00;

      current_state = state::fetch;

      for (auto& i : reg)
      {
         i = 0x00;
      }

      return;
   }

   bool interrupt_enabled(void) const
   {
      return read(sr, I);
   }

   bool negative(void) const
   {
      return read(sr, N);
   }

   bool equal(void) const
   {
      return read(sr, Z);
   }

   bool greater(void) const
   {
      return !negative() && !equal();
   }

   bool lower(void) const
   {
      return negative();
   }

   void generate_interrupt(const std::uint8_t interrupt_vector)
   {
      stack.push(pc);
      stack.push(mar);
      stack.push(sr);

      stack.push(ir >> 16);
      stack.push(ir >> 8);
      stack.push(ir);

      stack.push(op_code);
      stack.push(op1);
      stack.push(op2);

      stack.push(static_cast<std::uint8_t>(current_state));
     
      for (auto& i : reg)
      {
         stack.push(i);
      }

      pc = interrupt_vector;
      current_state = state::fetch;
      return;
   }

   void return_from_interrupt(void)
   {
      std::uint8_t temp = 0x00;

      for (auto& i : reg)
      {
         stack.pop(i);
      }

      stack.pop(temp);
      current_state = static_cast<state>(temp);

      stack.pop(op2);
      stack.pop(op1);
      stack.pop(op_code);

      stack.pop(temp);
      ir = temp;
      stack.pop(temp);
      ir |= temp << 8;
      stack.pop(temp);
      ir |= temp << 16;

      stack.pop(sr);
      stack.pop(mar);
      stack.pop(pc);
      return;
   }

   void monitor_interrupts(void)
   {
      static std::uint8_t last_input = 0x00;
      const auto current_input = data_mem.read(PINB);

      if (interrupt_enabled())
      {
         if (data_mem.read(PCICR) & (1 << PCIE0))
         {
            for (auto i = 0; i < DATA_WIDTH; ++i)
            {
               if (data_mem.read(PCMSK0) & (1 << i))
               {
                  if (read(last_input, i) != read(current_input, i))
                  {
                     generate_interrupt(prog_mem.PCINT0_vect);
                  }
               }
            }
         }
      }

      last_input = current_input;
      return;
   }

   static std::uint8_t get_status_bits(const std::uint16_t result,
                                       const std::uint8_t a,
                                       const std::uint8_t b)
   {
      std::uint8_t nzvc = 0x00;

      if (read(result, 7)) set(nzvc, 3);
      if (result == 0) set(nzvc, 2);

      if (read(a, 7) == read(b, 7))
      {
         if (read(a, 7) != read(result, 7))
         {
            set(nzvc, 1);
         }
      }

      if (read(result, 9)) set(nzvc, 0);
      return nzvc;
   }


   std::uint8_t alu(const std::uint8_t a, 
                    const std::uint8_t b = 0x00)
   {
      std::uint16_t result = 0x00;

      if (op_code == ORI || op_code == OR) result = a | b;
      else if (op_code == ANDI || op_code == AND) result = a & b;
      else if (op_code == XORI || op_code == XOR) result = a ^ b;
      else if (op_code == INC) result = a + 1;
      else if (op_code == DEC) result = a - 1;
      else if (op_code == ADDI || op_code == ADD) result = a + b;
      else if (op_code == SUBI || op_code == SUB) result = a - b;
      else if (op_code == CPI || op_code == CP) result = a - b;

      sr |= get_status_bits(result, a, b);
      return static_cast<std::uint8_t>(result);
   }

   void compare(const std::uint8_t a,
                const std::uint8_t b)
   {
      const std::uint16_t result = a - b;
      sr = get_status_bits(result, a, b);
      return;
   }

   void run_next_state(void)
   {
      switch (current_state)
      {
         case state::fetch:
         {
            ir = prog_mem.read(pc);
            mar = pc;
            pc++;
            current_state = state::decode;
            break;
         }
         case state::decode:
         {
            op_code = ir >> 16;
            op1 = ir >> 8;
            op2 = ir;
            current_state = state::execute;
            break;
         }
         case state::execute:
         {
            if (op_code == LDI)
            {
               reg[op1] = op2;
            }
            else if (op_code == MOV)
            {
               reg[op1] = reg[op2];
            }
            else if (op_code == OUT)
            {
               data_mem.write(op1, reg[op2]);
            }
            else if (op_code == IN)
            {
               reg[op1] = data_mem.read(op2);
            }
            else if (op_code == STS)
            {
               data_mem.write(static_cast<std::size_t>(op1), reg[op2]);

               if (op2 < NUM_REGISTERS)
               {
                  data_mem.write(static_cast<std::size_t>(op1) + 1, reg[static_cast<std::uint8_t>(op2 + 1)]);
               }
            }
            else if (op_code == LDS)
            {
               reg[op1] = data_mem.read(op2);

               if (op1 < NUM_REGISTERS)
               {
                  reg[static_cast<std::uint8_t>(op1 + 1)] = data_mem.read(static_cast<std::size_t>(op2 + 1));
               }
            }
            else if (op_code == ORI || op_code == ANDI || op_code == XORI)
            {
               reg[op1] = alu(reg[op1], op2);
            }
            else if (op_code == OR || op_code == AND || op_code == XOR)
            {
               reg[op1] = alu(reg[op1], reg[op2]);
            }
            else if (op_code == CLR)
            {
               reg[op1] = 0x00;
            }
            else if (op_code == INC || op_code == DEC)
            {
               reg[op1] = alu(reg[op1]);
            }
            else if (op_code == CPI)
            {
               compare(reg[op1], op2);
            }
            else if (op_code == CP)
            {
               compare(reg[op1], reg[op2]);
            }
            else if (op_code == JMP)
            {
               pc = op1;
            }
            else if (op_code == BREQ)
            {
               if (equal()) pc = op1;
            }
            else if (op_code == BRNE)
            {
               if (!equal())
               {
                  pc = op1;
               }
            }
            else if (op_code == BRGE)
            {
               if (greater() || equal()) pc = op1;
            }
            else if (op_code == BRGT)
            {
               if (greater()) pc = op1;
            }
            else if (op_code == BRLE)
            {
               if (lower() || equal()) pc = op1;
            }
            else if (op_code == BRLT)
            {
               if (lower()) pc = op1;
            }
            else if (op_code == CALL)
            {
               stack.push(pc);
               pc = op1;
            }
            else if (op_code == RET)
            {
               stack.pop(pc);
            }
            else if (op_code == PUSH)
            {
               stack.push(reg[op1]);
            }
            else if (op_code == POP)
            {
               stack.pop(reg[op1]);
            }
            else if (op_code == SEI)
            {
               set(sr, I);
            }
            else if (op_code == CLI)
            {
               clr(sr, I);
            }
            else if (op_code == RETI)
            {
               return_from_interrupt();
            }

            current_state = state::fetch;
            break;
         }
         default:
         {
            reset();
            break;
         }
      }

      monitor_interrupts();
      return;
   }

   void print(std::ostream& ostream = std::cout) const
   {
      ostream << "--------------------------------------------------------------------------------\n";
      ostream << "Subroutine:\t\t\t\t\t" << prog_mem.subroutine_name(mar) << "\n";
      ostream << "Current instruction:\t\t\t\t" << cpu::instruction_name(op_code) << "\n";
      ostream << "Current state:\t\t\t\t\t" << cpu::state_name(current_state) << "\n\n";

      ostream << "Program counter:\t\t\t\t" << static_cast<int>(pc) << "\n";
      ostream << "Instruction register:\t\t\t\t" << std::hex << static_cast<int>(ir) << "\n";
      ostream << "Status register (INZVC):\t\t\t" << std::bitset<5>(sr) << "\n\n";

      ostream << "Content in CPU register R16:\t\t\t" << std::bitset<8>(reg[R16]) << "\n";
      ostream << "Content in CPU register R24:\t\t\t" << std::bitset<8>(reg[R24]) << "\n\n";

      ostream << "Content in data direction register DDRB:\t" << std::bitset<8>(data_mem.read(DDRB)) << "\n";
      ostream << "Content in data register PORTB:\t\t\t" << std::bitset<8>(data_mem.read(PORTB)) << "\n";
      ostream << "Content in pin register PINB:\t\t\t" << std::bitset<8>(data_mem.read(PINB)) << "\n";
      ostream << "--------------------------------------------------------------------------------\n\n";
   }

   static void readline(std::string& s)
   {
      std::getline(std::cin, s);
      std::cout << "\n";
      return;
   }

   template<class T = int>
   static T convert(const std::string& s)
   {
      T val{};

      std::stringstream stream(s);
      stream >> val;
      return val;
   }

   template<class T = int>
   static T get_input(void)
   {
      std::string s;
      readline(s);
      return convert<T>(s);
   }

   void print_menu(void) const
   {
      std::cout << "Please select an alternative:\n";
      std::cout << "1. Execute next instruction cycle\n";
      std::cout << "2. Execute next state\n";
      std::cout << "3. System reset\n";
      std::cout << "4. Enter input to the PINB register\n\n";
      return;
   }

   int get_selection(void) const
   {
      print_menu();

      while (1)
      {
         const auto selection = get_input();

         if (selection >= 1 && selection <= 4)
         {
            return selection;
         }
         else
         {
            std::cout << "Invalid input, try again!\n\n";
         }
      }
   }

   void execute_selection(void)
   {
      const auto selection = get_selection();

      if (selection == 1)
      {
         std::cout << "Executing next instruction cycle!\n\n";

         if (current_state == state::execute)
         {
            this->run_next_state();
         }

         while (current_state != state::execute)
         {
            this->run_next_state();
         }
      }
      if (selection == 2)
      {
         std::cout << "Executing next state!\n\n";
         run_next_state();
      }
      else if (selection == 3)
      {
         reset();
         std::cout << "System reset!\n\n";
      }
      else if (selection == 4)
      {
         std::cout << "Enter new input for the PINB register:\n";
         const auto input = get_input();
         data_mem.write(PINB, input);
         std::cout << "Wrote data " << std::bitset<8>(input) << " to register PINB!\n\n";
      }
      return;
   }

   void run_with_key_press(void)
   {
      while (1)
      {
         print();        
         execute_selection();
      }
      return;
   }
};

#endif /* CONTROL_UNIT_HPP_ */