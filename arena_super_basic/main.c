// sanitized: gcc main.c -g3 -std=c11 -Wall -Wextra -pedantic -fsanitize=address,undefined -o arenasuperbasic

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

typedef int32_t S32;
typedef uint8_t U8;
typedef uint32_t U32;
typedef uint64_t U64;
typedef size_t USIZE;

typedef struct Arena Arena;
struct Arena {
 U8 *memory;
 USIZE size;
 USIZE position;
};

void arena_init(Arena *arena, U8 *arena_memory, USIZE size)
{
 arena->memory = arena_memory;
 arena->size = size;
 arena->position = 0;
}

void *arena_alloc(Arena *arena, USIZE size)
{

 // -
 // Save where our allocated void pointer should point to in memory.
 USIZE alloc_pos = arena->position;

 // TODO: Check that we have not run out of memory.
 
 // -
 arena->position = arena->position + size;

 // -
 void *ptr = &arena->memory[alloc_pos];

 // -
 // "memset" to zero by hand =). Or use: memset(ptr, 0, size);
 for(USIZE i = 0; i < size; i++)
 {
  ((U8*)ptr)[i] = 0;
 }

 // -
 return ptr;
}

S32 main(void)
{
 // -
 // Very simple arena that "works".
 U8 arena_memory[1000];
 Arena arena = {0};
 arena_init(&arena, arena_memory, 1000);

 // Allocate a U64 (8 bytes)
 U64 *large_number = arena_alloc(&arena, 8);
 *large_number = 0xFFFFFFFFFFFFFFFF;
 printf("large_number: %zu\n", *large_number);
 assert(*large_number == UINT64_MAX);

 // Allocate a U32 (4 bytes)
 U32 *number = arena_alloc(&arena, 4);
 *number = 0xFFFFFFFF;
 printf("number: %u\n", *number);
 assert(*number == UINT32_MAX);


 // -
 printf("\nOK!\n\n");
 printf("But by allocating the U32 (4 bytes) first we run into this problem! (provided compiled with -fsanitize=undefined)\n\n");

 /* EXAMPLE alignment issue
  main.c:92:24: runtime error: store to misaligned address 0x7f0614c00514 for type 'U64', which requires 8 byte alignment 0x7f0614c00514: 
  note: pointer points here
  00 00 ff ff 00 00 00 00  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
              ^
  main.c:93:2: runtime error: load of misaligned address 0x7f0614c00514 for type 'U64', which requires 8 byte alignment 0x7f0614c00514: 
  note: pointer points here
  00 00 ff ff 00 00 00 00  ff ff ff ff 00 00 00 00  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
  */
 
 // -
 // Well this does not work!
 U8 another_arena_memory[1000];
 Arena another_arena = {0};
 arena_init(&another_arena, another_arena_memory, 1000);

 // Allocate a U32 (4 bytes)
 U32 *another_number = arena_alloc(&another_arena, 4);
 *another_number = 0xFFFF0000;
 printf("another_number: %u\n", *another_number);
 
 // Allocate a U64 (8 bytes)
 U64 *another_large_number = arena_alloc(&another_arena, 8);
 *another_large_number = 0xFFFFFFFF00000000;
 printf("another_large_number: %zu\n", *another_large_number);

 // -
 printf("\nWE NEED ALIGNED MEMORY FROM THE ARENA ALLOCATOR!\n\n");

 return 0;
}

