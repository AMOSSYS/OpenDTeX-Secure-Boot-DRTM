#pragma once

#define E820_RAM        1
#define E820_RESERVED   2
#define E820_ACPI       3
#define E820_NVS        4
#define E820_BADRAM     5

struct e820_mmap {
  uint64_t addr;
  uint64_t size;
  uint32_t type;
} __attribute__((packed));


