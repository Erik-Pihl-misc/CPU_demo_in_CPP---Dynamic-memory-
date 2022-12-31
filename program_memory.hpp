#ifndef PROGRAM_MEMORY_HPP_
#define PROGRAM_MEMORY_HPP_

#include "cpu.hpp"

struct cpu::program_memory
{
   static constexpr auto LED1 = 0;
   static constexpr auto BUTTON1 = 5;
   static constexpr auto led_enabled = 100; 

   static constexpr auto RESET_vect = 0x00;
   static constexpr auto PCINT0_vect = 0x02;
   static constexpr auto ISR_vect_end = PCINT0_vect + 2;

   static constexpr auto ISR_PCINT0 = ISR_vect_end;
   static constexpr auto ISR_PCINT0_end = ISR_PCINT0 + 4;

   static constexpr auto ISR_end = ISR_PCINT0 + 5;

   static constexpr auto main = ISR_end;
   static constexpr auto main_loop = main + 1;
   static constexpr auto led_toggle = main_loop + 1;
   static constexpr auto led_toggle_end = led_toggle + 4;
   static constexpr auto led_on = led_toggle + 5;
   static constexpr auto led_off = led_on + 6;

   static constexpr auto setup = led_off + 6;
   static constexpr auto init_ports = setup;
   static constexpr auto init_interrupts = init_ports + 4;
   static constexpr auto init_globals = init_interrupts + 5;

   static constexpr auto button_is_pressed = init_globals + 3;
   static constexpr auto end = button_is_pressed + 3;

   static std::uint32_t assemble(const std::uint8_t op_code,
                                 const std::uint8_t op1 = 0x00,
                                 const std::uint8_t op2 = 0x00)
   {
      std::uint32_t instruction = op_code << 16;
      instruction |= op1 << 8;
      instruction |= op2;
      return instruction;
   }

   const std::vector<std::uint32_t> data =
   {
      /* RESET_vect: */
      assemble(JMP, main),                 /* JMP main */
      assemble(NOP),                       /* NOP */

      /* PCINT0_vect: */
      assemble(JMP, ISR_PCINT0),           /* JMP ISR_PCINT0 */
      assemble(NOP),                       /* NOP */

      /* ISR_PCINT0: */
      assemble(CALL, button_is_pressed),   /* CALL button_is_pressed */
      assemble(CPI, R24, 0x00),            /* CPI R24, 0x00 */
      assemble(BREQ, ISR_PCINT0_end),      /* BRNE ISR_PCINT0_end */
      assemble(CALL, led_toggle),          /* CALL led_toggle */
      /* ISR_PCINT0_end: */
      assemble(RETI),                      /* RETI */

      /* main: */
      assemble(CALL, setup),               /* CALL setup */
      /* main_loop: */ 
      assemble(JMP, main_loop),            /* JMP main_loop */

      /* led_toggle: */
      assemble(LDS, R16, led_enabled),     /* LDS R16, led_enabled */
      assemble(CPI, R16, 0x00),            /* CPI R16, 0x00 */
      assemble(BREQ, led_on),              /* BREQ led_on */
      assemble(JMP, led_off),              /* JMP led_off */
      /* led_toggle_end: */
      assemble(RET),                       /* RET */

      /* led_on: */
      assemble(IN, R16, PORTB),            /* IN R16, PORTB */
      assemble(ORI, R16, (1 << LED1)),     /* ORI R16, (1 << LED1) */
      assemble(OUT, PORTB, R16),           /* OUT PORTB, R16 */
      assemble(LDI, R16, 0x01),            /* LDI R16, 0x01 */
      assemble(STS, led_enabled, R16),     /* STS led_enabled, R16 */
      assemble(JMP, led_toggle_end),       /* JMP_led_toggle_end */

      /* led_off: */
      assemble(IN, R16, PORTB),            /* IN R16, PORTB */
      assemble(ANDI, R16, ~(1 << LED1)),   /* ANDI R16, ~(1 << LED1) */
      assemble(OUT, PORTB, R16),           /* OUT PORTB, R16 */
      assemble(LDI, R16, 0x00),            /* LDI R16, 0x00 */
      assemble(STS, led_enabled, R16),     /* STS led_enabled, R16 */
      assemble(JMP, led_toggle_end),       /* JMP_led_toggle_end */

      /* setup: */
      assemble(LDI, R16, (1 << LED1)),     /* LDI R16, (1 << LED1) */
      assemble(OUT, DDRB, R16),            /* OUT DDRB, R16 */
      assemble(LDI, R16, (1 << BUTTON1)),  /* LDI R16, (1 << BUTTON1) */
      assemble(OUT, PORTB, R16),           /* OUT PORTB, R16 */
      /* init_interrupts: */
      assemble(SEI),                       /* SEI */
      assemble(LDI, R16, (1 << PCIE0)),    /* LDI R16, (1 << PCIE0) */
      assemble(OUT, PCICR, R16),           /* OUT PCICR, R16 */
      assemble(LDI, R16, (1 << BUTTON1)),  /* LDI R16, (1 << BUTTON1) */
      assemble(OUT, PCMSK0, R16),          /* OUT PCMSK0, R16 */
      /* init_globals: */
      assemble(CLR, R16),                  /* CLR R16 */
      assemble(STS, led_enabled, R16),     /* STS led_enabled, R16 */
      assemble(RET),                       /* RET */

      /* button_is_pressed: */
      assemble(IN, R24, PINB),             /* IN R16, PINB */
      assemble(ANDI, R24, (1 << BUTTON1)), /* ANDI R24, (1 << BUTTON1) */
      assemble(RET)                        /* RET */
   };

   program_memory(void) { }

   std::size_t address_width(void) const
   {
      return data.size();
   }

   std::uint32_t read(const std::uint32_t address)
   {
      if (address < address_width())
      {
         return data[address];
      }
      else
      {
         return 0;
      }
   }

   const char* subroutine_name(const std::uint8_t address) const
   {
      if (address == RESET_vect) return "RESET_vect";
      else if (address == PCINT0_vect) return "PCINT0_vect";
      else if (address >= ISR_PCINT0 && address < main) return "ISR (PCINT0_vect)";
      else if (address >= main && address < led_toggle) return "main";
      else if (address >= led_toggle && address < led_on) return "led_toggle";
      else if (address >= led_on && address < led_off) return "led_on";
      else if (address >= led_off && address < setup) return "led_off";
      else if (address >= setup && address < button_is_pressed) return "setup";
      else if (address >= button_is_pressed && address < end) return "button_is_pressed";
      else return "Unknown";
   }
};

#endif /* PROGRAM_MEMORY_HPP_ */