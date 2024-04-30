#include "codeword.h"
#include "heap.h"
#include "treepool.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#ifndef _NON_PINTOS
#include <syscall.h>
#else
#include "lib.h"
#endif

// pintos -v -k --smp 4 --kvm  --filesys-size=8 -p ../../examples/zip -a zip -p
// ../../examples/extreme.txt -a sample.txt --swap-size=8 -- -q -f run 'zip -z
// sample.txt' > ret.txt

#define MAX(_a, _b) ((_a) > (_b) ? (_a) : (_b))
#define MIN(_a, _b) ((_a) < (_b) ? (_a) : (_b))

#define TREE_POOL_SIZE 512
#define CODE_LOOKUP_TABLE_WIDTH 8
#define CHAR_SET_SIZE 256

static void zip (char *filename);
static void unzip (char *filename);
static void array_init (int *array, int size);
static void charmap_init (struct Codeword *char_mapping, int size);
static bool tree_heap_less (void *a, void *b);
static void build_char_map (struct Codeword *char_mapping,
                            struct TreeNode *root, uint8_t length);
static bool codeword_heap_less (void *a, void *b);

int
main (int argc, char *argv[])
{
  if (argc < 3 || argc > 3)
    {
      printf ("\n"
              "Usage: zip [OPTION] <target_file>\n"
              "  -z:               zip the file\n"
              "  -u:             unzip the file\n"
              "the default output has .zip     \n");
      exit (EXIT_FAILURE);
    }

  if (!strcmp ("-z", argv[1]))
    {
      zip (argv[2]);
    }
  else if (!strcmp ("-u", argv[1]))
    {
      unzip (argv[2]);
    }
  else
    {
      printf ("invalid option\n"
              "Usage: zip [OPTION] <target_file>\n"
              "  -z:               zip the file\n"
              "  -u:             unzip the file\n"
              "the default output has .zip     \n");
      exit (EXIT_FAILURE);
    }

  return EXIT_SUCCESS;
}

void
zip (char *filename)
{
  int input_fd;
  if ((input_fd = open (filename)) < 0)
    {
      printf ("%s: open failed\n", filename);
      exit (EXIT_FAILURE);
    }
  
  int input_filesize = filesize (input_fd);
  seek (input_fd, 0);

  int char_count[CHAR_SET_SIZE];
  array_init (char_count, CHAR_SET_SIZE);
  for (;;)
    {
      uint8_t buffer[1024];
      int bytes_read = read (input_fd, buffer, sizeof buffer);
      if (bytes_read == 0)
        break;
      for (int i = 0; i < bytes_read; i++)
        {
          char_count[buffer[i]]++;
        }
    }
  char_count[0xff]++; // a special char for EOF

  struct TreeNode tree_node_pool[TREE_POOL_SIZE];
  treenode_pool_init (tree_node_pool, TREE_POOL_SIZE);

  struct Heap tree_heap;
  heap_init (&tree_heap, tree_heap_less, MAX_HEAP_SIZE);

  // create and insert treenode to heap
  for (int i = 0; i < 256; i++)
    {
      if (char_count[i] == 0)
        continue;
      struct TreeNode *node = create_node (tree_node_pool, (char)i,
                                           char_count[i], TREE_POOL_SIZE);
      heap_push (&tree_heap, node);
    }

  // build the huffman tree
  while (tree_heap.size > 1)
    {
      struct TreeNode *node1 = heap_pop (&tree_heap);
      struct TreeNode *node2 = heap_pop (&tree_heap);
      struct TreeNode *new_node = create_node (
          tree_node_pool, '\0', node1->count + node2->count, TREE_POOL_SIZE);
      new_node->left = node1;
      new_node->right = node2;
      heap_push (&tree_heap, new_node);
    }

  struct TreeNode *root = heap_pop (&tree_heap);

  struct Codeword char_mapping[CHAR_SET_SIZE];
  charmap_init (char_mapping, CHAR_SET_SIZE);

  // traverse the tree and calculate the codeword lenght for each char
  build_char_map (char_mapping, root, 0);

  struct Heap codeword_heap;
  heap_init (&codeword_heap, codeword_heap_less, MAX_HEAP_SIZE);

  // these 2 variable is used to calculate the max codeword length
  // which will be used to write the codeword length array to header
  uint8_t word_count = 0;
  uint8_t max_cw_length = 0;

  // push the codeword to heap, sorting by length and then by char
  for (int i = 0; i < CHAR_SET_SIZE; i++)
    {
      int length = char_mapping[i].length;
      if (length == 0)
        continue;
      word_count++;
      max_cw_length = MAX (max_cw_length, length);
      heap_push (&codeword_heap, &char_mapping[i]);
    }

  int cw_count_by_length[max_cw_length + 1];
  array_init (cw_count_by_length, max_cw_length + 1);

  char cw_char[word_count];
  int ptr = 0;
  int cur_length = 0;
  int code[8];
  code_clean (code);

  // build the canonical huffman code
  while (codeword_heap.size)
    {
      struct Codeword *cw = heap_pop (&codeword_heap);
      cw_char[ptr++] = cw->ch;
      code_shift_right (code, (cw->length - cur_length));
      cur_length = cw->length;
      cw_count_by_length[cur_length]++;
      code_cp (code, cw->code);
      code_inc_one (code);
    }

  char write_buffer[input_filesize * 2];
  int buffer_size = 0;

  // write 1 byte the length of the cw_length array
  write_buffer[buffer_size++] = max_cw_length + 1;

  // write the cw_length array
  for (int i = 0; i <= max_cw_length; i++)
    {
      uint8_t cw_length = (uint8_t)cw_count_by_length[i];
      write_buffer[buffer_size++] = cw_length;
    }

  // write the cw_char array
  for (int i = 0; i < word_count; i++)
    {
      write_buffer[buffer_size++] = cw_char[i];
    }

  // load and encode
  seek (input_fd, 0);
  char built_char = 0;
  // once built_cnt == 0, we write the built_char to the file
  int built_cnt = 8;
  for (;;)
    {
      uint8_t buffer[1024];
      int bytes_read = read (input_fd, buffer, sizeof buffer);
      if (bytes_read == 0)
        break;
      // each char, translate to bit code
      for (int i = 0; i < bytes_read; i++)
        {
          int *code = char_mapping[buffer[i]].code;
          int length = char_mapping[buffer[i]].length;
          for (int j = length - 1; j >= 0; j--)
            {
              int bit = (code[j / 32] >> (j % 32)) & 1;
              built_char += (bit << (built_cnt - 1));
              built_cnt--;
              if (built_cnt == 0)
                {
                  write_buffer[buffer_size++] = built_char;
                  built_cnt = 8;
                  built_char = 0;
                }
            }
        }
    }
  // write 0xff as EOF
  int *eof_code = char_mapping[0xff].code;
  int eof_length = char_mapping[0xff].length;
  for (int j = eof_length - 1; j >= 0; j--)
    {
      int bit = (eof_code[j / 32] >> (j % 32)) & 1;
      built_char += (bit << (built_cnt - 1));
      built_cnt--;
      if (built_cnt == 0)
        {
          write_buffer[buffer_size++] = built_char;
          built_cnt = 8;
          built_char = 0;
        }
    }
  if (built_cnt != 8)
    {
      write_buffer[buffer_size++] = built_char;
    }
  char filename_cp[24];
  strlcpy (filename_cp, filename, strlen (filename));

  char *save_ptr;
  char *token = strtok_r (filename_cp, ".", &save_ptr);
  char new_filename[24];
  snprintf (new_filename, strlen (token) + 5, "%s.cmp", token);

  // create the output file
  if (!create (new_filename, buffer_size))
    {
      printf ("%s: create failed\n", new_filename);
      exit (EXIT_FAILURE);
    }

  int output_fd;
  if ((output_fd = open (new_filename)) < 0)
    {
      printf ("%s: open failed\n", new_filename);
      exit (EXIT_FAILURE);
    }

  seek (output_fd, 0);
  write (output_fd, write_buffer, buffer_size);

  printf ("successfully compressed:        %s\n"
          "create cmp file:                %s\n"
          "original file size:             %d\n"
          "cmp file size:                  %d\n",
          filename, new_filename, filesize (input_fd), filesize (output_fd));

  close (input_fd);
  close (output_fd);
}

void
unzip (char *filename)
{
  int input_fd;
  if ((input_fd = open (filename)) < 0)
    {
      printf ("%s: open failed\n", filename);
      exit (EXIT_FAILURE);
    }
  int input_filesize = filesize (input_fd);
  seek (input_fd, 0);

  struct Codeword char_mapping[CHAR_SET_SIZE];
  struct Codeword code_lookup_table[CHAR_SET_SIZE]; // O(1) lookup table
  charmap_init (char_mapping, CHAR_SET_SIZE);
  charmap_init (code_lookup_table, CHAR_SET_SIZE);

  // first byte will be the length of the codeword length array
  uint8_t max_cw_length;
  read (input_fd, &max_cw_length, sizeof max_cw_length);

  int word_count = 0;
  uint8_t cw_count_by_length[max_cw_length];
  for (int i = 0; i < max_cw_length; i++)
    {
      read (input_fd, &(cw_count_by_length[i]), sizeof cw_count_by_length[i]);
      word_count += cw_count_by_length[i];
    }

  int cur_length = 0;
  int pre_length = 0;
  int code[8];
  code_clean (code);

  for (; cw_count_by_length[cur_length] == 0; cur_length++)
    ;

  for (int i = 0; i < word_count; i++)
    {
      uint8_t ch;
      read (input_fd, &ch, sizeof ch);
      struct Codeword *cw = &char_mapping[ch];
      cw->ch = ch;
      cw->length = cur_length;
      code_clean (cw->code);
      code_shift_right (code, (cw->length - pre_length));
      pre_length = cw->length;
      code_cp (code, cw->code);
      cw_count_by_length[cur_length]--;
      // we store the information of cw which length <= 8 in the lookup table
      if (cw->length <= CODE_LOOKUP_TABLE_WIDTH)
        {
          // For example, a codeword has length 3 and code 101
          // then stored in a lookup table with width 8
          // we left shif it to 8 bits long -> 101xxxxx
          // In the canonical huffman code, the prefix of the
          // codeword is unique, so we can assign all the codeword
          // index from 10100000 to 10111111 to the same char
          int _code = cw->code[0] << (CODE_LOOKUP_TABLE_WIDTH - cw->length);
          int padding = 1 << (CODE_LOOKUP_TABLE_WIDTH - cw->length);
          for (int i = 0; i < padding; i++)
            {
              code_lookup_table[_code + i].ch = ch;
              code_lookup_table[_code + i].length = cw->length;
              code_cp (cw->code, code_lookup_table[_code + i].code);
            }
        }
      code_inc_one (code);
      for (; cw_count_by_length[cur_length] == 0; cur_length++)
        ;
    }

  char write_buffer[input_filesize * 2];
  int buffer_size = 0;

  cur_length = 0;
  char decode_char;
  bool is_end = false;
  code_clean (code);
  for (;;)
    {
      uint8_t buffer[1024];
      int bytes_read = read (input_fd, buffer, sizeof buffer);
      if (bytes_read == 0)
        break;
      // each char, translate to bit code
      for (int i = 0; i < bytes_read && !is_end; i++)
        {
          char encode_char = buffer[i];
          for (int j = 0; j < 8; j++)
            {
              code_shift_right (code, 1);
              if ((encode_char >> (7 - j)) & 1)
                code_inc_one (code);
              cur_length++;
              if (cur_length == CODE_LOOKUP_TABLE_WIDTH)
                {
                  if (code_lookup_table[code[0]].length != 0)
                    {
                      int cw_length = code_lookup_table[code[0]].length;
                      decode_char = code_lookup_table[code[0]].ch;
                      is_end = (decode_char == (char)0xff);
                      if (!is_end)
                        {
                          write_buffer[buffer_size++] = decode_char;
                          code[0] = code[0] / (1 << cw_length);
                          cur_length -= cw_length;
                        }
                    }
                }
              else
                {
                  for (int k = 0; k < 256; k++)
                    {
                      if (codeword_match (&char_mapping[k], code, cur_length))
                        {
                          decode_char = char_mapping[k].ch;
                          is_end = (decode_char == (char)0xff);
                          if (!is_end)
                            {
                              write_buffer[buffer_size++] = decode_char;
                              code_clean (code);
                              cur_length = 0;
                            }
                          break;
                        }
                    }
                }
              if (is_end)
                break;
            }
        }
    }
  char filename_cp[24];
  strlcpy (filename_cp, filename, strlen (filename));

  char new_filename[24];
  char *save_ptr;
  char *token = strtok_r (filename_cp, ".", &save_ptr);

  snprintf (new_filename, strlen (token) + 6, "%s.ucmp", token);
  if (!create (new_filename, buffer_size))
    {
      printf ("%s: create failed\n", new_filename);
      exit (EXIT_FAILURE);
    }

  int output_fd;
  if ((output_fd = open (new_filename)) < 0)
    {
      printf ("%s: open failed\n", new_filename);
      exit (EXIT_FAILURE);
    }
  seek (output_fd, 0);
  write (output_fd, write_buffer, buffer_size);

#ifdef _NON_PINTOS
  sync ();
#endif

  printf ("successfully decompressed:      %s\n"
          "create ucmp file:               %s\n"
          "compressed file size:           %d\n"
          "decompressed file size:         %d\n",
          filename, new_filename, filesize (input_fd), filesize (output_fd));

  close (input_fd);
  close (output_fd);
}

static void
array_init (int *array, int size)
{
  for (int i = 0; i < size; i++)
    array[i] = 0;
}

static void
charmap_init (struct Codeword *char_mapping, int size)
{
  for (int i = 0; i < size; i++)
    {
      char_mapping[i].ch = (char)i;
      char_mapping[i].length = 0;
      code_clean (char_mapping[i].code);
    }
}

static void
build_char_map (struct Codeword *char_mapping, struct TreeNode *root,
                uint8_t length)
{
  if (root->ch != '\0')
    {
      uint8_t index = root->ch;
      char_mapping[index].length = length;
      return;
    }
  if (root->left)
    build_char_map (char_mapping, root->left, length + 1);
  if (root->right)
    build_char_map (char_mapping, root->right, length + 1);
}

static bool
tree_heap_less (void *a, void *b)
{
  struct TreeNode *node1 = a;
  struct TreeNode *node2 = b;
  return node1->count > node2->count;
}

static bool
codeword_heap_less (void *a, void *b)
{
  struct Codeword *node1 = a;
  struct Codeword *node2 = b;
  if (node1->length > node2->length)
    return true;
  else if (node1->length < node2->length)
    return false;
  else
    return node1->ch > node2->ch;
}