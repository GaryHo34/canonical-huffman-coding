#ifndef HEAP_H
#define HEAP_H
#include <stdio.h>

#ifdef _NON_PINTOS
#include "lib.h"
#endif

#define MAX_HEAP_SIZE 512
typedef bool heap_less_func (void *a, void *b);

struct Heap
{
  void *queue[MAX_HEAP_SIZE];
  size_t size;
  size_t capacity;
  heap_less_func *less;
};

void heap_init (struct Heap *heap, heap_less_func *less, size_t capacity);
void heap_push (struct Heap *heap, void *node);
void *heap_pop (struct Heap *heap);
void *heap_top (struct Heap *heap);

#endif // HEAP_H