#ifndef _RBTREE_H_
#define _RBTREE_H_

#include <stddef.h>

typedef enum
{
  RBTREE_RED,
  RBTREE_BLACK
} color_t;

typedef int key_t;

typedef struct node_t
{
  color_t color;
  key_t key;
  struct node_t *parent, *left, *right;
} node_t;

typedef struct
{
  node_t *root;
  node_t *nil; // for sentinel
} rbtree;

rbtree *new_rbtree(void);
void delete_rbtree(rbtree *);
void free_subtree(rbtree *t, node_t *n);
void rebuild_after_delete(rbtree *t, node_t *n);
node_t *rbtree_insert(rbtree *, const key_t);
node_t *new_node(rbtree *t, key_t key);
void insert_node(rbtree *t, node_t *n);
void rebuild_after_insert(rbtree *t, node_t *n);
node_t *rbtree_find(const rbtree *, const key_t);
node_t *subtree_min(rbtree *t, node_t *start);
node_t *rbtree_min(const rbtree *);
node_t *rbtree_max(const rbtree *);
int rbtree_erase(rbtree *, node_t *);
void rbtree_transplant(rbtree *t, node_t *u, node_t *v);
void rotate_left(rbtree *t, node_t *parent);
void rotate_right(rbtree *t, node_t *parent);

int rbtree_to_array(const rbtree *, key_t *, const size_t);
int in_order_traversal(const rbtree *t, node_t *n, key_t *arr, const size_t len, int index);
void print_tree_structure(const rbtree *t, node_t *p, char *prefix, int is_last, int black_depth);
void print_tree(const rbtree *t);

#endif // _RBTREE_H_
