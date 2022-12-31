#ifndef DATA_MEMORY_HPP_
#define DATA_MEMORY_HPP_

#include "cpu.hpp"

template<class T>
struct cpu::data_memory
{
   std::vector<T> data;

   data_memory(void) { }

   data_memory(const std::size_t address_width)
   {
      init(address_width);
      return;
   }

   std::size_t address_width(void) const
   {
      return data.size();
   }

   void init(const std::size_t address_width = 2000)
   {
      data.resize(address_width, 0x00);
      return;
   }

   void reset(void)
   {
      for (auto& i : data)
      {
         i = 0x00;
      }
      return;
   }

   int write(const std::uint16_t address, const T& new_element)
   {
      if (address < address_width())
      {
         data[address] = new_element;
         return 0;
      }
      else
      {
         return 1;
      }
   }

   T read(const std::uint16_t address) const
   {
      if (address < address_width())
      {
         return data[address];
      }
      else
      {
         return static_cast<T>(0);
      }
   }
};

#endif /* DATA_MEMORY_HPP_ */