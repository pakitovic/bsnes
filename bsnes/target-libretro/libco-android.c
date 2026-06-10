/*
  libco backend for Android/AArch64.

  libco's stock aarch64 backend keeps its context-switch code in a data array
  and mprotect()s it executable at runtime, which Android's SELinux forbids
  (execmem), and its sjlj fallback builds stacks with POSIX signals, which
  collides with ART's signal chaining inside the RetroArch process. This
  backend assembles the context switch into the .text section at compile time
  instead, so no runtime code generation or signals are needed.

  Based on the public-domain libco.aarch64 by webgeek1234 (bsnes-mercury),
  extended with: d8-d15 (callee-saved FP registers, required by AAPCS64 and
  exercised by bsnes), co_derive(), co_serializable(), and a BTI-safe entry.

  Context layout at the start of the cothread buffer (the stack grows down
  from the end of the same buffer, so co_serializable() can return 1 and
  bsnes' fast save states keep working):
    [0]   x19 x20 x21 x22 x23 x24 x25 x26 x27 x28   (#0..#72)
    [80]  x29 (frame pointer)
    [88]  x30 (resume address)
    [96]  sp
    [112] d8 d9 d10 d11 d12 d13 d14 d15             (#112..#168)
    [192] end of context, start of usable region
*/

#define LIBCO_C
#include "../../libco/libco.h"

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { CONTEXT_SIZE = 192 };

static _Thread_local uint64_t co_primary_buffer[CONTEXT_SIZE / 8];
static _Thread_local cothread_t co_running = 0;

__asm__(
  ".text\n"
  ".align 4\n"
  ".globl co_swap_aarch64\n"
  ".hidden co_swap_aarch64\n"
  ".type co_swap_aarch64, %function\n"
  "co_swap_aarch64:\n"        /* x0 = to, x1 = from */
  "  bti c\n"                 /* hint/nop on cores without BTI */
  "  stp x19, x20, [x1, #0]\n"
  "  stp x21, x22, [x1, #16]\n"
  "  stp x23, x24, [x1, #32]\n"
  "  stp x25, x26, [x1, #48]\n"
  "  stp x27, x28, [x1, #64]\n"
  "  stp x29, x30, [x1, #80]\n"
  "  mov x2, sp\n"
  "  str x2, [x1, #96]\n"
  "  stp d8,  d9,  [x1, #112]\n"
  "  stp d10, d11, [x1, #128]\n"
  "  stp d12, d13, [x1, #144]\n"
  "  stp d14, d15, [x1, #160]\n"

  "  ldp x19, x20, [x0, #0]\n"
  "  ldp x21, x22, [x0, #16]\n"
  "  ldp x23, x24, [x0, #32]\n"
  "  ldp x25, x26, [x0, #48]\n"
  "  ldp x27, x28, [x0, #64]\n"
  "  ldp x29, x30, [x0, #80]\n"
  "  ldr x2, [x0, #96]\n"
  "  mov sp, x2\n"
  "  ldp d8,  d9,  [x0, #112]\n"
  "  ldp d10, d11, [x0, #128]\n"
  "  ldp d12, d13, [x0, #144]\n"
  "  ldp d14, d15, [x0, #160]\n"
  "  ret\n"                   /* resumes at the restored x30 */
  ".size co_swap_aarch64, . - co_swap_aarch64\n"
);

void co_swap_aarch64(cothread_t to, cothread_t from);

static void co_init_context(uint64_t* ptr, unsigned int size, void (*entrypoint)(void)) {
  for(unsigned int n = 0; n < CONTEXT_SIZE / 8; n++) ptr[n] = 0;
  uint64_t top = ((uint64_t)ptr + size) & ~(uint64_t)15;
  ptr[10] = top;                    /* x29: frame pointer            */
  ptr[11] = (uint64_t)entrypoint;   /* x30: first resume jumps here  */
  ptr[12] = top;                    /* sp                            */
}

cothread_t co_active(void) {
  if(!co_running) co_running = co_primary_buffer;
  return co_running;
}

cothread_t co_derive(void* memory, unsigned int size, void (*entrypoint)(void)) {
  if(!memory) return 0;
  co_init_context((uint64_t*)memory, size, entrypoint);
  return (cothread_t)memory;
}

cothread_t co_create(unsigned int size, void (*entrypoint)(void)) {
  void* memory = malloc(size);
  if(!memory) return 0;
  return co_derive(memory, size, entrypoint);
}

void co_delete(cothread_t handle) {
  free(handle);
}

void co_switch(cothread_t handle) {
  cothread_t from = co_active();
  co_running = handle;
  co_swap_aarch64(handle, from);
}

int co_serializable(void) {
  return 1;
}

#ifdef __cplusplus
}
#endif
