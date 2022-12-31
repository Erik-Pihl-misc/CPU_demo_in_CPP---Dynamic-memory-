#ifndef STACK_HPP_
#define STACK_HPP_

#include "cpu.hpp"

template<class T>
struct cpu::stack
{
   std::vector<T> data;
   std::size_t sp = 0;
   bool stack_empty = true;

   void reset(void)
   {
      for (auto& i : data)
      {
         i = 0x00;
      }

      sp = address_width() - 1;
      stack_empty = true;
      return;
   }

   stack(void) { }

   stack(const std::size_t address_width)
   {
      init(address_width);
      return;
   }

   void init(const std::size_t address_width = 256)
   {
      data.resize(address_width, 0);
      sp = address_width - 1;
      return;
   }

   std::size_t address_width(void) const
   {
      return data.size();
   }

   int push(const T& new_element)
   {
      if (stack_empty)
      {
         data[sp] = new_element;
         stack_empty = false;
         return 0;
      }
      else
      {
         if (sp > 0)
         {
            data[--sp] = new_element;
            return 0;
         }
         else
         {
            return 1;
         }
      }
   }

   int pop(T& retrieved_value)
   {
      if (stack_empty) return 1;
      retrieved_value = data[sp];

      if (sp < address_width() - 1)
      {
         sp++;
      }
      else
      {
         stack_empty = true;
      }
      return 0;
   }

   T first_element(void) const
   {
      return data[address_width() - 1];
   }

   T last_element(void) const
   {
      return data[sp];
   }

   auto num_elements(void) const
   {
      return address_width() - 1 - sp;
   }
};

#endif /* STACK_HPP_ */