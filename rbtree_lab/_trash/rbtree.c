#include "rbtree.h"

#include <stdio.h>
#include <stdlib.h>

rbtree *new_rbtree(void)
{
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  node_t *nil = (node_t *)calloc(1, sizeof(node_t));
  nil->color = RBTREE_BLACK;
  nil->parent = nil;
  nil->right = nil;
  nil->left = nil;
  p->nil = nil;
  p->root = p->nil;
  return p;
}

/* 트리를 완전 삭제(메모리 해제)합니다. */
// RB tree 구조체가 차지했던 메모리를 전부 반환
void delete_rbtree(rbtree *t)
{
  if (t == NULL)
    return;

  free_subtree(t, t->root);
  free(t->nil);
  free(t);
}

void free_subtree(rbtree *t, node_t *n)
{
  if (n == NULL || n == t->nil)
    return;
  free_subtree(t, n->left);
  free_subtree(t, n->right);
  free(n);
}

/* 타겟 노드를 삭제(메모리 해제)합니다. */
// target 노드를 트리에서 삭제하는 함수
int rbtree_erase(rbtree *t, node_t *target)
{
  if (target == NULL || target == t->nil)
    return 0;

  node_t *removed = target;                        // 실제로 삭제될 노드
  color_t removed_original_color = removed->color; // 삭제될 노드의 원래 색상
  node_t *fixup_child = NULL;                      // 균형 복구 시 기준 노드

  // 왼쪽 자식이 nil이면, 오른쪽 자식으로 교체
  if (target->left == t->nil)
  {
    fixup_child = target->right;
    rbtree_transplant(t, target, target->right);
  }
  // 오른쪽 자식이 nil이면, 왼쪽 자식으로 교체
  else if (target->right == t->nil)
  {
    fixup_child = target->left;
    rbtree_transplant(t, target, target->left);
  }
  // 자식 둘 다 있으면 후계자(successor) 찾아 교체
  else
  {
    removed = subtree_min(t, target->right); // 오른쪽에서 최소값 찾기
    removed_original_color = removed->color; // 후계자 색상 저장
    fixup_child = removed->right;

    if (removed->parent != target)
    {
      rbtree_transplant(t, removed, removed->right); // removed를 오른쪽 자식으로 교체
      removed->right = target->right;
      removed->right->parent = removed;
    }
    rbtree_transplant(t, target, removed); // target을 removed로 교체
    removed->left = target->left;
    removed->left->parent = removed;
    removed->color = target->color;
  }

  // 삭제된 노드가 검은색이면 레드-블랙 트리 속성을 복원
  if (removed_original_color == RBTREE_BLACK)
    rebuild_after_delete(t, fixup_child);

  // 삭제 노드 메모리 해제
  free(target);
  return 1;
}

// 트리에서 u 노드를 v로 치환(교체)하는 함수
void rbtree_transplant(rbtree *t, node_t *u, node_t *v)
{
  if (u->parent == t->nil)
    t->root = v;
  else if (u == u->parent->left)
    u->parent->left = v;
  else
    u->parent->right = v;
  v->parent = u->parent;
}

/* 서브트리의 최소값을 가진 노드를 반환합니다. */
node_t *subtree_min(rbtree *t, node_t *start)
{
  while (start->left != t->nil)
    start = start->left;
  return start;
}

/* 노드 삭제 후 규칙 위반된 부분을 고쳐주는 리밸런싱 함수 */
void rebuild_after_delete(rbtree *t, node_t *successor)
{
  fprintf(stderr, "rebuild_after_delete: start. successor=%p, parent=%p, color=%d\n", (void *)successor, (void *)successor->parent, successor->color);
  fflush(stderr);
  node_t *sibling = NULL;

  while (successor->parent != t->nil && successor->color == RBTREE_BLACK)
  {
    // 이중흑색노드가 부모노드의 왼쪽자식일 경우
    if (successor == successor->parent->left)
    {
      sibling = successor->parent->right;
      // CASE1. 형제가 빨간색인 경우
      if (sibling->color == RBTREE_RED)
      {
        sibling->color = RBTREE_BLACK;
        successor->parent->color = RBTREE_RED;
        rotate_left(t, successor->parent);
      }
      // CASE2. 형제가 검은색이며
      else
      {
        // CASE2-A. 양쪽 자식이 모두 검 black색인 경우
        if (sibling->left->color == RBTREE_BLACK &&
            sibling->right->color == RBTREE_BLACK)
        {
          sibling->color = RBTREE_RED;
          successor = successor->parent;
        }
        else
        {
          // CASE2-B. 왼쪽 자식이 빨간색인 경우
          if (sibling->left->color == RBTREE_RED)
          {
            sibling->left->color = RBTREE_BLACK;
            sibling->color = RBTREE_RED;

            rotate_right(t, sibling);
            sibling = successor->parent->right;
          }

          // CASE2-C. 오른쪽 자식이 빨간색인 경우
          sibling->color = successor->parent->color;
          successor->parent->color = RBTREE_BLACK;
          sibling->right->color = RBTREE_BLACK;
          rotate_left(t, successor->parent);
          successor = t->root;
        }
      }
    }
    else
    {
      // 이중 흑색 노드가 부모노드의 오른쪽 자식인 경우
      sibling = successor->parent->left;
      // CASE1. 형제가 빨간색인 경우
      if (sibling->color == RBTREE_RED)
      {
        // recolor and rotate right (mirror of left-case)
        sibling->color = RBTREE_BLACK;
        successor->parent->color = RBTREE_RED;
        rotate_right(t, successor->parent);
        // update sibling after rotation
        sibling = successor->parent->left;
      }
      // CASE2. 형제가 검은색이며
      else
      {
        // CASE2-A. 양쪽 자식이 모두 검 black색인 경우
        if (sibling->left->color == RBTREE_BLACK &&
            sibling->right->color == RBTREE_BLACK)
        {
          sibling->color = RBTREE_RED;
          successor = successor->parent;
        }
        else
        {
          // CASE2-B. 오른쪽 자식이 빨간색인 경우 (mirror)
          if (sibling->right->color == RBTREE_RED)
          {
            // transform to CASE2-C by rotating left on sibling
            sibling->right->color = RBTREE_BLACK;
            sibling->color = RBTREE_RED;

            rotate_left(t, sibling);
            sibling = successor->parent->left;
          }

          // CASE2-C. 왼쪽 자식이 빨간색인 경우 (mirror)
          sibling->color = successor->parent->color;
          successor->parent->color = RBTREE_BLACK;
          sibling->left->color = RBTREE_BLACK;
          rotate_right(t, successor->parent);
          successor = t->root;
        }
      }
    }
  }
  successor->color = RBTREE_BLACK;
}

/* 트리에 새 노드를 삽입합니다. */
node_t *rbtree_insert(rbtree *t, const key_t key)
{
  node_t *n = new_node(t, key); // 새 노드를 만든다.
  insert_node(t, n);            // 새 노드를 트리의 알맞은 위치에 삽입한다.
  rebuild_after_insert(t, n);

  return n;
}

/* 새로운 노드를 생성합니다. */
node_t *new_node(rbtree *t, key_t key)
{
  node_t *n = (node_t *)calloc(1, sizeof(node_t));
  n->color = RBTREE_RED;
  n->key = key;
  n->left = t->nil;
  n->right = t->nil;
  n->parent = NULL; // 임시 설정. 실제 포인터는 rbtree_insert에서 넣는다.

  return n;
}

/* 트리의 알맞은 위치에 노드를 삽입합니다. */
void insert_node(rbtree *t, node_t *n)
{
  // 빈 트리라면 root에 노드를 삽입한다.
  if (t->root == t->nil)
  {
    t->root = n;
    n->parent = t->nil;
    return;
  }

  node_t *curr = t->root;
  while (curr != t->nil)
  {
    if (n->key < curr->key) // 삽입할 노드의 키가 현재 노드의 키보다 작으면 왼쪽 서브트리로 내려간다
    {
      if (curr->left == t->nil) // 자식이 nil이면 그 위치에 새 노드를 삽입한다
      {
        n->parent = curr;
        curr->left = n;
        return;
      }
      curr = curr->left;
    }
    else // 삽입할 노드의 키가 현재 노드의 키보다 크거나 같으면 오른쪽 서브트리로 내려간다
    {
      if (curr->right == t->nil) // 자식이 nil이면 그 위치에 새 노드를 삽입한다
      {
        n->parent = curr;
        curr->right = n;
        return;
      }
      curr = curr->right;
    }
  }
}

/* 우회전시키는 함수 */
void rotate_right(rbtree *t, node_t *parent)
{
  // 왼쪽 자식 노드의 오른쪽 자식을 부모 노드의 왼쪽으로 등록
  node_t *leftChild = parent->left;
  parent->left = leftChild->right;

  if (leftChild->right != t->nil)
    leftChild->right->parent = parent;

  leftChild->parent = parent->parent;

  if (parent->parent == t->nil) // 할아버지가 Nil이면 이 노드는 root다.
    t->root = leftChild;
  else
  {
    if (parent == parent->parent->left) // 부모가 할아버지의 왼쪽자식이었다면 자식을 할아버지의 왼쪽으로 위치시킨다.
      parent->parent->left = leftChild;
    else
      parent->parent->right = leftChild; // 부모가 할아버지의 오른쪽자식이었다면 자식을 할아버지의 오른쪽으로 위치시킨다.
  }

  leftChild->right = parent;
  parent->parent = leftChild;
}

/* 좌회전전시키는 함수 */
void rotate_left(rbtree *t, node_t *parent)
{
  // 왼쪽 자식 노드의 오른쪽 자식을 부모 노드의 오른으로 등록
  node_t *rightChild = parent->right;
  parent->right = rightChild->left;

  if (rightChild->left != t->nil)
    rightChild->left->parent = parent;

  rightChild->parent = parent->parent;

  if (parent->parent == t->nil) // 할아버지가 Nil이면 이 노드는 root다.
    t->root = rightChild;
  else
  {
    if (parent == parent->parent->right) // 부모가 할아버지의 오른쪽자식이었다면 자식을 할아버지의 오른쪽으로 위치시킨다.
      parent->parent->right = rightChild;
    else
      parent->parent->left = rightChild; // 부모가 할아버지의 왼쪽자식이었다면 자식을 할아버지의 왼쪽으로 위치시킨다.
  }

  rightChild->left = parent;
  parent->parent = rightChild;
}

/* 새 노드 삽입 후 규칙 위반된 부분을 고쳐주는 리밸런싱 함수 */
void rebuild_after_insert(rbtree *t, node_t *n)
{
  // 규칙4 위반 여부 판단: 빨간 노드의 자식은 모두 검은색이다.
  while (t->root != n && n->parent->color == RBTREE_RED)
  {
    // 부모 노드가 할아버지 노드의 왼쪽 서브트리인 경우
    if (n->parent == n->parent->parent->left)
    {
      node_t *uncle = n->parent->parent->right;
      // CASE1. 삼촌이 빨간색인 경우
      if (uncle->color == RBTREE_RED)
      { // CASE1- SOLUTION. 부모와 삼촌을 검은 색으로 칠하고, 할아버지를 빨간 색으로 칠한다.
        n->parent->color = RBTREE_BLACK;
        uncle->color = RBTREE_BLACK;
        n->parent->parent->color = RBTREE_RED;

        // 부작용 확인. 할아버지를 새로 삽입한 노드로 간주하고 다시 규칙4 위반 여부를 판단한다.
        n = n->parent->parent;
      }
      else
      { // CASE2. 삼촌이 검정색이고, n이 오른쪽 자식인 경우
        if (n == n->parent->right)
        {
          // CASE2- SOLUTION. 부모 노드를 좌회전시켜서 CASE3으로 만든다.
          n = n->parent;
          rotate_left(t, n->parent);
        }
        // CASE3. 삼촌이 검정색이고, n이 왼쪽 자식인 경우
        // CASE3- SOLUTION. 부모를 검정, 할아버지를 빨간 색으로 칠하고 할아버지 노드를 우회전시킨다.
        n->parent->color = RBTREE_BLACK;
        n->parent->parent->color = RBTREE_BLACK;
        rotate_right(t, n->parent->parent);
      }
      // 부모 노드가 할아버지 노드의 오른쪽 서브트리인 경우
    }
    else if (n->parent == n->parent->parent->right)
    {
      node_t *uncle = n->parent->parent->left;
      // CASE1. 삼촌이 빨간색인 경우
      if (uncle->color == RBTREE_RED)
      { // CASE1- SOLUTION. 부모와 삼촌을 검은 색으로 칠하고, 할아버지를 빨간 색으로 칠한다.
        n->parent->color = RBTREE_BLACK;
        uncle->color = RBTREE_BLACK;
        n->parent->parent->color = RBTREE_RED;

        // 부작용 확인. 할아버지를 새로 삽입한 노드로 간주하고 다시 규칙4 위반 여부를 판단한다.
        n = n->parent->parent;
      }
      else
      { // CASE2. 삼촌이 검정색이고, n이 왼쪽 자식인 경우
        if (n == n->parent->left)
        {
          // CASE2- SOLUTION. 부모 노드를 우회전시켜서 CASE3으로 만든다.
          n = n->parent;
          rotate_right(t, n->parent);
        }
        // CASE3. 삼촌이 검정색이고, n이 오른쪽 자식인 경우
        // CASE3- SOLUTION. 부모를 검정, 할아버지를 빨간 색으로 칠하고 할아버지 노드를 좌회전시킨다.
        n->parent->color = RBTREE_BLACK;
        n->parent->parent->color = RBTREE_BLACK;
        rotate_left(t, n->parent->parent);
      }
    }
  }
  t->root->color = RBTREE_BLACK;
}

/* key값을 가진 노드를 찾아 노드 포인터를 반환합니다. */
node_t *rbtree_find(const rbtree *t, const key_t key)
{
  node_t *res = NULL;
  node_t *curr = t->root;
  while (curr != t->nil)
  {
    if (key == curr->key)
    {
      res = curr;
      break;
    }
    else if (key < curr->key)
      curr = curr->left;
    else if (key > curr->key)
      curr = curr->right;
  }

  return res;
}

/* RB tree 중 최소 값을 가진 노드 포인터를 반환합니다. */
node_t *rbtree_min(const rbtree *t)
{
  node_t *curr = t->root;
  if (curr == t->nil)
    return t->nil;
  while (curr->left != t->nil)
  {
    curr = curr->left;
  }

  return curr;
}

node_t *rbtree_max(const rbtree *t)
{
  node_t *curr = t->root;
  if (curr == t->nil)
    return t->nil;
  while (curr->right != t->nil)
  {
    curr = curr->right;
  }

  return curr;
}

// - RB tree의 내용을 *key 순서대로* 주어진 array로 변환
// - array의 크기는 n으로 주어지며 tree의 크기가 n 보다 큰 경우에는 순서대로 n개 까지만 변환
// - array의 메모리 공간은 이 함수를 부르는 쪽에서 준비하고 그 크기를 n으로 알려줍니다.
int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n)
{
  node_t *curr = t->root;
  in_order_traversal(t, curr, arr, n, 0);
  return 0;
}

/* rtb노드의 값을 오름차순으로 배열에 저장한다. */
int in_order_traversal(const rbtree *t, node_t *n, key_t *arr, const size_t len, int index)
{
  if (n == t->nil || index >= len) // 종료조건: 더 이상 방문할 노드가 없거나  배열이 다 찼으면 종료
    return index;
  index = in_order_traversal(t, n->left, arr, len, index); // 왼쪽 자식을 재귀적으로 방문해서 key를 배열에 저장한다
  if (index < len)
    arr[index] = n->key;
  index++;
  index = in_order_traversal(t, n->right, arr, len, index); // 오른쪽 자식을 재귀적으로 방문해서 key를 배열에 저장한다
  return index;
}

/* 트리를 출력한다 */
#ifndef _RBTREE_H_
#include "rbtree.h"
#endif

#include <stdio.h>
#include <string.h>

void print_tree(const rbtree *t)
{
  const char *path = "/Users/soyoungan/dev/rbtree_lab_docker/rbtree_lab/tree.log";
  FILE *out = fopen(path, "w");
  if (out == NULL)
  {
    // fprintf(stderr, "print_tree: fopen failed path=%s errno=%d\n", path, errno);
    // fflush(stderr);
    // fallback to stdout if file cannot be opened
    print_tree_structure(t, t->root, "", 1, 0);
    printf("\n");
    return;
  }
  else
  {
    fprintf(stderr, "print_tree: opened %s\n", path);
    fflush(stderr);
  }
  // helper that writes into FILE*
  // implement as a local lambda-style call via a static function below
  extern void print_tree_structure_to_file(const rbtree *t, node_t *p, char *prefix, int is_last, int black_depth, FILE *out);
  print_tree_structure_to_file(t, t->root, "", 1, 0, out);
  fprintf(out, "\n");
  fclose(out);
}

void print_tree_structure(const rbtree *t, node_t *p, char *prefix, int is_last, int black_depth)
{
  /* Append output into repo-root tree.log via the file-output variant */
  FILE *out = fopen("/Users/soyoungan/dev/rbtree_lab_docker/rbtree_lab/tree.log", "a");
  if (out == NULL)
    return;
  print_tree_structure_to_file(t, p, prefix, is_last, black_depth, out);
  fclose(out);
}

// file-output variant of print_tree_structure
void print_tree_structure_to_file(const rbtree *t, node_t *p, char *prefix, int is_last, int black_depth, FILE *out)
{
  int width = 3;

  if (p == NULL || p == t->nil)
    return;

  fprintf(out, "%s", prefix);

  char color_char = p->color == RBTREE_BLACK ? 'B' : 'R';
  const char *color_start = "";
  const char *color_end = "";

  int has_left = (p->left != NULL && p->left != t->nil);
  int has_right = (p->right != NULL && p->right != t->nil);
  int is_leaf_path = (!has_left || !has_right);
  black_depth += p->color;

  if (strlen(prefix) == 0)
  {
    fprintf(out, "key: %s%*d | color: %c | parent: %*d | left: %*d | right: %*d%s",
            color_start,
            width, p->key,
            color_char,
            width, (p->parent == NULL || p->parent == t->nil) ? 0 : p->parent->key,
            width, (p->left == NULL || p->left == t->nil) ? 0 : p->left->key,
            width, (p->right == NULL || p->right == t->nil) ? 0 : p->right->key,
            color_end);

    if (is_leaf_path)
    {
      fprintf(out, " | black depth: %3d", black_depth);
    }
    fprintf(out, "\n");
    fflush(out);
  }
  else
  {
    fprintf(out, "%s%skey: %*d | color: %c | parent: %*d | left: %*d | right: %*d",
            color_start,
            is_last ? "╚═ " : "╠═ ",
            width, p->key,
            color_char,
            width, (p->parent == NULL || p->parent == t->nil) ? 0 : p->parent->key,
            width, (p->left == NULL || p->left == t->nil) ? 0 : p->left->key,
            width, (p->right == NULL || p->right == t->nil) ? 0 : p->right->key);

    if (is_leaf_path)
    {
      fprintf(out, " | black depth: %3d", black_depth);
    }
    fprintf(out, "%s\n", color_end);
    fflush(out);
  }

  // recurse
  char prefix_for_left[256];
  char prefix_for_right[256];
  strcpy(prefix_for_left, prefix);
  strcat(prefix_for_left, is_last ? "    " : "║   ");
  strcpy(prefix_for_right, prefix);
  strcat(prefix_for_right, is_last ? "    " : "║   ");

  print_tree_structure_to_file(t, p->left, prefix_for_left, 0, black_depth, out);
  print_tree_structure_to_file(t, p->right, prefix_for_right, 1, black_depth, out);
  fflush(out);
}