#include "treepool.h"

void
treenode_pool_init (struct TreeNode *tree_node_pool, size_t tree_pool_size)
{
  for (size_t i = 0; i < tree_pool_size; i++)
    {
      tree_node_pool[i].ch = '\0';
      tree_node_pool[i].count = 0;
      tree_node_pool[i].left = NULL;
      tree_node_pool[i].right = NULL;
      tree_node_pool[i].used = false;
    }
}

struct TreeNode *
create_node (struct TreeNode *tree_node_pool, char ch, int count,
             size_t tree_pool_size)
{
  for (size_t i = 0; i < tree_pool_size; i++)
    {
      if (!tree_node_pool[i].used)
        {
          tree_node_pool[i].used = true;
          tree_node_pool[i].ch = ch;
          tree_node_pool[i].count = count;
          return &tree_node_pool[i];
        }
    }
  return NULL;
}