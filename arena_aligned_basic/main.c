// sanitized: gcc main.c -g3 -std=c11 -Wall -Wextra -pedantic -fsanitize=address,undefined -o arenaaligned

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

typedef int32_t S32;
typedef uint8_t U8;
typedef uint32_t U32;
typedef uint64_t U64;
typedef size_t USIZE;

#define DEFAULT_ALIGNMENT (sizeof(void *))

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
 USIZE alloc_pos = arena->position;

 // -
 U64 aligned_size = 0;

 // -
 // Alignment
 if(size == DEFAULT_ALIGNMENT)
 {
  aligned_size = size;
 }
 else if(size < DEFAULT_ALIGNMENT)
 {
  U64 padding = DEFAULT_ALIGNMENT - size;
  aligned_size = size + padding;
 }
 else if(size > DEFAULT_ALIGNMENT)
 {
  if(size % DEFAULT_ALIGNMENT)
  {
   U64 base_size = (size / DEFAULT_ALIGNMENT) * DEFAULT_ALIGNMENT;
   aligned_size = base_size + DEFAULT_ALIGNMENT;
  }
  else
  {
   aligned_size = (size / DEFAULT_ALIGNMENT) * DEFAULT_ALIGNMENT;
  }
 }

 // -
 if((arena->position + aligned_size) > arena->size)
 {
  printf("Out of memory in the Arena!\n");
  return NULL;
 }
 
 // -
 arena->position = arena->position + aligned_size;

 // -
 for(USIZE i = alloc_pos; i < arena->position; i++)
 {
  arena->memory[i] = 0;
 }

 // -
 void *ptr = &arena->memory[alloc_pos];

 // -
 return ptr;
}

void arena_free(Arena *arena)
{
 // -
 for(USIZE i = 0; i < arena->size; i++)
 {
  arena->memory[i] = 0;
 }

 // -
 arena->position = 0;
}

typedef struct Object Object;
struct Object {
 U32 number;
 U64 large_number;
 bool boolean;
};

S32 main(void)
{
 // -
 printf("\nMACHINE ALIGNMENT: %zu\n\n", DEFAULT_ALIGNMENT);

 // -
 U64 unaligned_bytes_count = 0;

 for(USIZE i = 0; i < 3; i++)
 {
  // -
  printf("ITERATION: %zu\n", i);

  // -
  U8 arena_memory[64];
  Arena arena = {0};
  arena_init(&arena, arena_memory, 64);

  // Allocate a U32 (4 bytes)
  U32 *number = arena_alloc(&arena, sizeof(U32));
  *number = 0xFFFFFFFF;
  printf("number: %u\n", *number);
  assert(*number == UINT32_MAX);
  unaligned_bytes_count += sizeof(U32);

  // Allocate a U64 (8 bytes)
  U64 *large_number = arena_alloc(&arena, sizeof(U64));
  *large_number = 0xFFFFFFFFFFFFFFFF;
  printf("large_number: %zu\n", *large_number);
  assert(*large_number == UINT64_MAX);
  unaligned_bytes_count += sizeof(U64);

  // Allocate a bool (1 bytes)
  bool *boolean = arena_alloc(&arena, sizeof(bool));
  *boolean = true;
  printf("boolean: %d\n", *boolean);
  unaligned_bytes_count += sizeof(bool);

  // Allocate struct (24 bytes)
  Object *obj = arena_alloc(&arena, sizeof(Object));
  obj->number = 0xAAAA0000;
  obj->large_number = 0xBBBBBBBB00000000;
  obj->boolean = true;
  printf("obj: number: 0x%X, large_number: 0x%lX, boolean: %d\n", obj->number, obj->large_number, obj->boolean);
  unaligned_bytes_count += sizeof(Object);

  // Allocate a U64 (8 bytes)
  U64 *another_large_number = arena_alloc(&arena, sizeof(U64));
  *another_large_number = 0xCCCCCCCC00000000;
  printf("another_large_number: 0x%lX\n", *another_large_number);
  unaligned_bytes_count += sizeof(U64);

  // -
  printf("--------------------\n");
  if(arena.position >= unaligned_bytes_count)
  {
   printf("OK, %zu bytes allocted in the Arena allocator.\n", arena.position);
  }
  else
  {
   printf("Arena allocation alignment bug!\n");
   printf("Unaligned bytes count: %zu\n", unaligned_bytes_count);
   printf("Aligned bytes allocated: %zu\n", arena.position);
  }

  // -
  U64 current_pos = arena.position;
  arena_free(&arena);
  printf("Freed %zu bytes from the Arena\n", current_pos);

  // -
  unaligned_bytes_count = 0;

  // -
  printf("\n");
 }

 return 0;
}

