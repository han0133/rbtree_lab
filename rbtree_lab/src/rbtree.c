#include "rbtree.h"

#include <stdio.h>
#include <stdlib.h>

/* Purpose: Create and initialize a new empty red-black tree. */
rbtree *new_rbtree(void)
{
  rbtree *t = (rbtree *)calloc(1, sizeof(rbtree));   // allocate tree struct
  node_t *nil = (node_t *)calloc(1, sizeof(node_t)); // allocate sentinel node
  nil->color = RBTREE_BLACK;                         // sentinel is black
  nil->parent = nil;                                 // sentinel parent points to itself
  nil->left = nil;                                   // sentinel left points to itself
  nil->right = nil;                                  // sentinel right points to itself
  t->nil = nil;                                      // attach sentinel to tree
  t->root = t->nil;                                  // empty tree: root == nil
  return t;                                          // return initialized tree
}

/* Purpose: Recursively free subtree nodes (post-order) and avoid freeing the sentinel node. */
static void free_subtree(rbtree *t, node_t *n)
{
  if (n == NULL || n == t->nil) // nothing to free for NULL/nil
    return;                     // return early
  free_subtree(t, n->left);     // free left subtree
  free_subtree(t, n->right);    // free right subtree
  free(n);                      // free this node
}

/* Purpose: Destroy the entire tree, freeing nodes, sentinel and tree struct. */
void delete_rbtree(rbtree *t)
{
  if (t == NULL)            // nothing to do if tree is NULL
    return;                 // early return
  free_subtree(t, t->root); // free all regular nodes
  free(t->nil);             // free sentinel node
  free(t);                  // free tree container
}

/* Purpose: Helper transplant: replace subtree rooted at u with subtree rooted at v. */
static void transplant(rbtree *t, node_t *u, node_t *v)
{
  if (u->parent == t->nil)       // if u is root
    t->root = v;                 // v becomes new root
  else if (u == u->parent->left) // if u is left child
    u->parent->left = v;         // set left child to v
  else
    u->parent->right = v; // set right child to v
                          // if (v != t->nil)         // if v is not sentinel

  v->parent = u->parent; // update v's parent
}

/* Purpose: Find the minimum node in subtree starting at `start`. */
node_t *subtree_min(rbtree *t, node_t *start)
{
  node_t *curr = start;                          // start from provided node
  while (curr != t->nil && curr->left != t->nil) // traverse left while possible
    curr = curr->left;                           // move left
  return curr;                                   // return min node (could be nil)
}

/* Purpose: Left-rotate the subtree rooted at x. */
static void rotate_left(rbtree *t, node_t *x)
{
  node_t *y = x->right;          // set y
  x->right = y->left;            // turn y's left subtree into x's right
  if (y->left != t->nil)         // if y's left exists
    y->left->parent = x;         // update parent
  y->parent = x->parent;         // link y's parent to x's parent
  if (x->parent == t->nil)       // if x was root
    t->root = y;                 // y becomes root
  else if (x == x->parent->left) // else if x was left child
    x->parent->left = y;         // set left child
  else
    x->parent->right = y; // set right child
  y->left = x;            // put x on y's left
  x->parent = y;          // update x's parent
}

/* Purpose: Right-rotate the subtree rooted at x. */
static void rotate_right(rbtree *t, node_t *x)
{
  node_t *y = x->left;            // set y
  x->left = y->right;             // turn y's right subtree into x's left
  if (y->right != t->nil)         // if y's right exists
    y->right->parent = x;         // update parent
  y->parent = x->parent;          // link y's parent to x's parent
  if (x->parent == t->nil)        // if x was root
    t->root = y;                  // y becomes root
  else if (x == x->parent->right) // else if x was right child
    x->parent->right = y;         // set right child
  else
    x->parent->left = y; // set left child
  y->right = x;          // put x on y's right
  x->parent = y;         // update x's parent
}

/* Purpose: Restore red-black properties after insertion of node z. */
static void rebuild_after_insert(rbtree *t, node_t *z)
{
  while (z->parent->color == RBTREE_RED) // while parent is red
  {
    if (z->parent == z->parent->parent->left) // if parent is left child
    {
      node_t *y = z->parent->parent->right; // uncle
      if (y->color == RBTREE_RED)           // case 1: uncle red
      {
        z->parent->color = RBTREE_BLACK;       // recolor parent black
        y->color = RBTREE_BLACK;               // recolor uncle black
        z->parent->parent->color = RBTREE_RED; // recolor grandparent red
        z = z->parent->parent;                 // move z up
      }
      else
      {
        if (z == z->parent->right) // case 2: z is right child
        {
          z = z->parent;     // move z up
          rotate_left(t, z); // rotate left
        }
        z->parent->color = RBTREE_BLACK;       // case 3: recolor parent
        z->parent->parent->color = RBTREE_RED; // recolor grandparent
        rotate_right(t, z->parent->parent);    // rotate right
      }
    }
    else // mirror
    {
      node_t *y = z->parent->parent->left; // uncle
      if (y->color == RBTREE_RED)          // case 1 mirror
      {
        z->parent->color = RBTREE_BLACK;       // recolor parent
        y->color = RBTREE_BLACK;               // recolor uncle
        z->parent->parent->color = RBTREE_RED; // recolor grandparent
        z = z->parent->parent;                 // move z up
      }
      else
      {
        if (z == z->parent->left) // case 2 mirror
        {
          z = z->parent;      // move z up
          rotate_right(t, z); // rotate right
        }
        z->parent->color = RBTREE_BLACK;       // recolor parent
        z->parent->parent->color = RBTREE_RED; // recolor grandparent
        rotate_left(t, z->parent->parent);     // rotate left
      }
    }
  }
  t->root->color = RBTREE_BLACK; // ensure root is black
}

/* Purpose: Insert a key into the tree and return the created node pointer. */
node_t *rbtree_insert(rbtree *t, const key_t key)
{
  node_t *z = (node_t *)calloc(1, sizeof(node_t)); // allocate new node
  z->key = key;                                    // set key
  z->color = RBTREE_RED;                           // new nodes are red
  z->left = t->nil;                                // children point to sentinel
  z->right = t->nil;                               // children point to sentinel

  node_t *y = t->nil;  // y will track parent
  node_t *x = t->root; // start from root
  while (x != t->nil)  // find insertion point
  {
    y = x;               // update parent
    if (z->key < x->key) // go left if key smaller
      x = x->left;       // move left
    else
      x = x->right; // move right (allow duplicates to right)
  }
  z->parent = y;            // set parent
  if (y == t->nil)          // if tree was empty
    t->root = z;            // new node is root
  else if (z->key < y->key) // attach as left child
    y->left = z;            // set left pointer
  else
    y->right = z; // set right pointer

  rebuild_after_insert(t, z); // fix red-black properties
  return z;                   // return new node
}

/* Purpose: Find a node by key. Returns pointer to node or NULL if not found. */
node_t *rbtree_find(const rbtree *t, const key_t key)
{
  node_t *curr = t->root; // start from root
  while (curr != t->nil)  // traverse until sentinel
  {
    if (key == curr->key) // found
      return curr;        // return node
    else if (key < curr->key)
      curr = curr->left; // move left
    else
      curr = curr->right; // move right
  }
  return NULL; // not found: tests expect NULL
}

/* Purpose: Return pointer to minimum element in tree (or t->nil if empty). */
node_t *rbtree_min(const rbtree *t)
{
  node_t *curr = t->root;                        // start from root
  while (curr != t->nil && curr->left != t->nil) // traverse left
    curr = curr->left;                           // move left
  return curr;                                   // return min or nil
}

/* Purpose: Return pointer to maximum element in tree (or t->nil if empty). */
node_t *rbtree_max(const rbtree *t)
{
  node_t *curr = t->root;                         // start from root
  while (curr != t->nil && curr->right != t->nil) // traverse right
    curr = curr->right;                           // move right
  return curr;                                    // return max or nil
}

/* Purpose: Restore red-black properties after deletion. */
static void rebuild_after_delete(rbtree *t, node_t *x)
{
  while (x != t->root && x->color == RBTREE_BLACK) // while x is double-black
  {
    if (x == x->parent->left) // if x is left child
    {
      node_t *w = x->parent->right; // sibling
      if (w->color == RBTREE_RED)   // case 1
      {
        w->color = RBTREE_BLACK;       // recolor sibling
        x->parent->color = RBTREE_RED; // recolor parent
        rotate_left(t, x->parent);     // rotate left
        w = x->parent->right;          // update sibling
      }
      if (w->left->color == RBTREE_BLACK && w->right->color == RBTREE_BLACK) // case 2
      {
        w->color = RBTREE_RED; // recolor sibling
        x = x->parent;         // move x up
      }
      else
      {
        if (w->right->color == RBTREE_BLACK) // case 3
        {
          w->left->color = RBTREE_BLACK; // recolor
          w->color = RBTREE_RED;         // recolor
          rotate_right(t, w);            // rotate right
          w = x->parent->right;          // update sibling
        }
        w->color = x->parent->color;     // case 4
        x->parent->color = RBTREE_BLACK; // recolor
        w->right->color = RBTREE_BLACK;  // recolor
        rotate_left(t, x->parent);       // rotate left
        x = t->root;                     // finish
      }
    }
    else // mirror cases when x is right child
    {
      node_t *w = x->parent->left; // sibling
      if (w->color == RBTREE_RED)  // case 1 mirror
      {
        w->color = RBTREE_BLACK;       // recolor
        x->parent->color = RBTREE_RED; // recolor
        rotate_right(t, x->parent);    // rotate right
        w = x->parent->left;           // update sibling
      }
      if (w->right->color == RBTREE_BLACK && w->left->color == RBTREE_BLACK) // case 2 mirror
      {
        w->color = RBTREE_RED; // recolor
        x = x->parent;         // move x up
      }
      else
      {
        if (w->left->color == RBTREE_BLACK) // case 3 mirror
        {
          w->right->color = RBTREE_BLACK; // recolor
          w->color = RBTREE_RED;          // recolor
          rotate_left(t, w);              // rotate left
          w = x->parent->left;            // update sibling
        }
        w->color = x->parent->color;     // case 4 mirror
        x->parent->color = RBTREE_BLACK; // recolor
        w->left->color = RBTREE_BLACK;   // recolor
        rotate_right(t, x->parent);      // rotate right
        x = t->root;                     // finish
      }
    }
  }
  x->color = RBTREE_BLACK; // ensure x is black
}

/* Purpose: Erase node p from the tree and free its memory. */
int rbtree_erase(rbtree *t, node_t *p)
{
  if (t == NULL || p == NULL || p == t->nil) // invalid input
    return 0;                                // nothing done

  node_t *z = p;                       // node to remove
  node_t *y = z;                       // y will point to node actually removed
  node_t *x = NULL;                    // x will point to child that replaces y
  color_t y_original_color = y->color; // save original color

  if (z->left == t->nil) // if left child is nil
  {
    x = z->right;               // right child will replace z
    transplant(t, z, z->right); // replace z with its right child
  }
  else if (z->right == t->nil) // if right child is nil
  {
    x = z->left;               // left child will replace z
    transplant(t, z, z->left); // replace z with its left child
  }
  else // both children exist
  {
    y = subtree_min(t, z->right); // successor is minimum in right subtree
    y_original_color = y->color;  // save successor color
    x = y->right;                 // x is successor's right child
    if (y->parent == z)           // if successor is direct child
    {
      x->parent = y; // set x's parent to successor (may set nil->parent)
    }
    else
    {
      transplant(t, y, y->right); // replace successor with its right child
      y->right = z->right;        // move z's right subtree under y
      y->right->parent = y;       // fix parent
    }
    transplant(t, z, y); // replace z with successor
    y->left = z->left;   // attach z's left subtree to y
    y->left->parent = y; // fix parent
    y->color = z->color; // copy color
  }

  if (y_original_color == RBTREE_BLACK) // if removed node was black
  {
    rebuild_after_delete(t, x); // restore red-black properties
  }

  free(z);  // free removed node
  return 1; // success
}

/* Purpose: In-order traversal copying up to n keys into arr. */
static int in_order_copy(const rbtree *t, node_t *n, key_t *arr, const size_t nslots, int idx)
{
  if (n == t->nil || idx >= (int)nslots)              // stop if nil or array full
    return idx;                                       // return current index
  idx = in_order_copy(t, n->left, arr, nslots, idx);  // traverse left
  if (idx < (int)nslots)                              // if room left
    arr[idx] = n->key;                                // store key
  idx++;                                              // increment index
  idx = in_order_copy(t, n->right, arr, nslots, idx); // traverse right
  return idx;                                         // return updated index
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n)
{
  if (t == NULL || arr == NULL || n == 0) // validate args
    return 0;                             // nothing copied
  in_order_copy(t, t->root, arr, n, 0);   // fill array
  return 0;                               // API returns int; keep 0
}
