#include "plat_types.h"
#include "hal_location.h"
#include "hal_trace.h"

#define GUARD_WORD                      0xdeadbeef

#ifdef LTO_OPTIMIZE
USED
#endif
BOOT_DATA_LOC
uint32_t __stack_chk_guard = GUARD_WORD;

BOOT_TEXT_FLASH_LOC
void __stack_chk_init (void)
{
  __stack_chk_guard = GUARD_WORD;
}

#ifdef LTO_OPTIMIZE
USED
#endif
NORETURN
void __stack_chk_fail (void)
{
  const char *msg = "*** stack smashing detected ***: terminated LR=%x";
  ASSERT(0, msg, (uint32_t)__builtin_return_address(0));
}
