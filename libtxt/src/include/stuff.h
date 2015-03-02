#pragma once

#include <uc/types.h>

static inline bool plus_overflow_u64(uint64_t x, uint64_t y)
{
    return ((((uint64_t)(~0)) - x) < y);
}

static inline bool plus_overflow_u32(uint32_t x, uint32_t y)
{
    return ((((uint32_t)(~0)) - x) < y);
}

/*
 * This checks to see if two numbers multiplied together are larger
 *   than the type that they are.  Returns TRUE if OVERFLOWING.
 *   If the first parameter "x" is greater than zero and
 *   if that is true, that the largest possible value 0xFFFFFFFF / "x"
 *   is less than the second parameter "y".  If "y" is zero then
 *   it will also fail because no unsigned number is less than zero.
 */
static inline bool multiply_overflow_u32(uint32_t x, uint32_t y)
{
    return (x > 0) ? ((((uint32_t)(~0))/x) < y) : false;
}

#define ARRAY_SIZE(a)    (sizeof(a) / sizeof(a[0]))

#define AP_WAKE_TRIGGER_DEF   0xffffffff

