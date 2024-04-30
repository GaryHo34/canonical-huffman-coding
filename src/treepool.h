#ifndef TREE_POOL_H
#define TREE_POOL_H
#include <stdio.h>
#ifndef _NON_PINTOS
#include <syscall.h>
#else
#include "lib.h"
#endif

#define OUTPUT_BUFFER_SIZE 1024 * 1024

struct TreeNode
{
  bool used;
  char ch;
  int count;
  struct TreeNode *left;
  struct TreeNode *right;
};

void treenode_pool_init (struct TreeNode *tree_node_pool,
                         size_t tree_pool_size);
struct TreeNode *create_node (struct TreeNode *tree_node_pool, char ch,
                              int count, size_t tree_pool_size);
#endif /*TREE_POOL_H*/