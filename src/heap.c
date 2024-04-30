#include "heap.h"

static void shif_up (struct Heap *heap, int index);
static void shif_down (struct Heap *heap, int index);
static size_t get_parent (size_t child);
static size_t get_left_child (size_t parent);
static size_t get_right_child (size_t parent);
static void swap_node (void **a, void **b);

void
heap_init (struct Heap *heap, heap_less_func *less, size_t capacity)
{
  heap->less = less;
  heap->capacity = capacity;
  for (size_t i = 0; i < capacity; i++)
    {
      heap->queue[i] = NULL;
    }
}

void
heap_push (struct Heap *heap, void *node)
{
  if (heap->size == heap->capacity)
    return;

  heap->queue[heap->size] = node;
  heap->size++;
  shif_up (heap, heap->size - 1);
}

void *
heap_pop (struct Heap *heap)
{
  if (heap->size == 0)
    return NULL;
  void *temp = heap->queue[0];
  heap->queue[0] = heap->queue[heap->size - 1];
  heap->queue[heap->size - 1] = NULL;
  heap->size--;
  shif_down (heap, 0);
  return temp;
}

void *
heap_top (struct Heap *heap)
{
  return heap->queue[0];
}

/* static helper functions */
static void
shif_up (struct Heap *heap, int child)
{
  if (child == 0)
    return;
  size_t parent = get_parent (child);

  if (heap->less (heap->queue[parent], heap->queue[child]))
    {
      swap_node (&heap->queue[parent], &heap->queue[child]);
      shif_up (heap, parent);
    }
}

static void
shif_down (struct Heap *heap, int parent)
{
  size_t right_child = get_right_child (parent);
  size_t left_child = get_left_child (parent);

  size_t max_child = 0;
  if (right_child >= heap->size)
    {
      if (left_child >= heap->size)
        return;
      else
        max_child = left_child;
    }
  else
    {
      if (!heap->less (heap->queue[left_child], heap->queue[right_child]))
        max_child = left_child;
      else
        max_child = right_child;
    }

  if (heap->less (heap->queue[parent], heap->queue[max_child]))
    {
      swap_node (&heap->queue[parent], &heap->queue[max_child]);
      shif_down (heap, max_child);
    }
}

static size_t
get_parent (size_t child)
{
  if (child == 0)
    return 0;
  return (child - 1) >> 1;
}

static size_t
get_left_child (size_t parent)
{
  return (parent << 1) + 1;
}

static size_t
get_right_child (size_t parent)
{
  return (parent + 1) << 1;
}

static void
swap_node (void **a, void **b)
{
  void *temp = *a;
  *a = *b;
  *b = temp;
}