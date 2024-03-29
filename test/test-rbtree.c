#include <assert.h>
#include <rbtree.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// new_rbtree should return rbtree struct with null root node
void test_init(void)
{
  rbtree *t = new_rbtree();
  assert(t != NULL);
  assert(t->root == NULL);
  delete_rbtree(t);
}

// root node should have proper values and pointers
void test_insert_single(const key_t key)
{
  rbtree *t = new_rbtree();
  node_t *p = rbtree_insert(t, key);
  assert(p != NULL);
  assert(t->root == p);
  assert(p->key == key);
  // assert(p->color == RBTREE_BLACK);  // color of root node should be black
  assert(p->left == NULL);
  assert(p->right == NULL);
  assert(p->parent == NULL);
  delete_rbtree(t);
}

// find should return the node with the key or NULL if no such node exists
void test_find_single(const key_t key, const key_t wrong_key)
{
  rbtree *t = new_rbtree();
  node_t *p = rbtree_insert(t, key);

  node_t *q = rbtree_find(t, key);
  assert(q != NULL);
  assert(q->key == key);
  assert(q == p);

  q = rbtree_find(t, wrong_key);
  assert(q == NULL);

  delete_rbtree(t);
}

// erase should delete root node
void test_erase_root(const key_t key)
{
  rbtree *t = new_rbtree();
  node_t *p = rbtree_insert(t, key);
  assert(p != NULL);
  assert(t->root == p);
  assert(p->key == key);

  rbtree_erase(t, p);
  assert(t->root == NULL);

  delete_rbtree(t);
}

static void insert_arr(rbtree *t, const key_t *arr, const size_t n)
{
  for (size_t i = 0; i < n; i++)
  {
    rbtree_insert(t, arr[i]);
  }
}

static int comp(const void *p1, const void *p2)
{
  const key_t *e1 = (const key_t *)p1;
  const key_t *e2 = (const key_t *)p2;
  if (*e1 < *e2)
  {
    return -1;
  }
  else if (*e1 > *e2)
  {
    return 1;
  }
  else
  {
    return 0;
  }
};

// min/max should return the min/max value of the tree
void test_minmax(key_t *arr, const size_t n)
{
  // null array is not allowed
  assert(n > 0 && arr != NULL);

  rbtree *t = new_rbtree();
  assert(t != NULL);

  insert_arr(t, arr, n);
  assert(t->root != NULL);

  qsort((void *)arr, n, sizeof(key_t), comp);
  node_t *p = rbtree_min(t);
  assert(p != NULL);
  assert(p->key == arr[0]);

  node_t *q = rbtree_max(t);
  assert(q != NULL);
  assert(q->key == arr[n - 1]);

  rbtree_erase(t, p);
  p = rbtree_min(t);
  assert(p != NULL);
  assert(p->key == arr[1]);

  if (n >= 2)
  {
    rbtree_erase(t, q);
    q = rbtree_max(t);
    assert(q != NULL);
    assert(q->key == arr[n - 2]);
  }

  delete_rbtree(t);
}

void test_to_array(const key_t *arr, const size_t n)
{
  rbtree *t = new_rbtree();
  assert(t != NULL);

  insert_arr(t, arr, n);
  qsort((void *)arr, n, sizeof(key_t), comp);

  key_t *res = calloc(n, sizeof(key_t));
  rbtree_to_array(t, res, n);

  for (int i = 0; i < n; i++)
  {
    printf(" %d ", arr[i]);
    assert(arr[i] == res[i]);
  }

  delete_rbtree(t);
  printf("/*---- test_to_array completed ----*/ \n\n");
}

// Search tree constraint
// The values of left subtree should be less than or equal to the current node
// The values of right subtree should be greater than or equal to the current
// node

static bool search_traverse(const node_t *p, key_t *min, key_t *max)
{
  if (p == NULL)
  {
    return true;
  }

  *min = *max = p->key;

  key_t l_min, l_max, r_min, r_max;
  l_min = l_max = r_min = r_max = p->key;

  const bool lr = search_traverse(p->left, &l_min, &l_max);
  if (!lr || l_max > p->key)
  {
    return false;
  }
  const bool rr = search_traverse(p->right, &r_min, &r_max);
  if (!rr || r_min < p->key)
  {
    return false;
  }

  *min = l_min;
  *max = r_max;
  return true;
}

void test_search_constraint(const rbtree *t)
{
  assert(t != NULL);
  node_t *p = t->root;
  key_t min, max;
  assert(search_traverse(p, &min, &max));
}

// Color constraint
// 1. Each node is either red or black. (by definition)
// 2. All NIL nodes are considered black.
// 3. A red node does not have a red child.
// 4. Every path from a given node to any of its descendant NIL nodes goes
// through the same number of black nodes.

bool touch_nil = false;
int max_black_depth = 0;

static void init_color_traverse(void)
{
  touch_nil = false;
  max_black_depth = 0;
}

static bool color_traverse(const node_t *p, const color_t parent_color,
                           const int black_depth)
{
  if (p == NULL)
  {
    if (!touch_nil)
    {
      touch_nil = true;
      max_black_depth = black_depth;
    }
    else if (black_depth != max_black_depth)
    {
      return false;
    }
    return true;
  }
  if (parent_color == RBTREE_RED && p->color == RBTREE_RED)
  {
    return false;
  }
  int next_depth = ((p->color == RBTREE_BLACK) ? 1 : 0) + black_depth;
  return color_traverse(p->left, p->color, next_depth) &&
         color_traverse(p->right, p->color, next_depth);
}

void test_color_constraint(const rbtree *t)
{
  assert(t != NULL);
  node_t *p = t->root;
  assert(p == NULL || p->color == RBTREE_BLACK);

  init_color_traverse();
  assert(color_traverse(p, RBTREE_BLACK, 0));
}

// rbtree should keep search tree and color constraints
void test_rb_constraints(const key_t arr[], const size_t n)
{
  rbtree *t = new_rbtree();
  assert(t != NULL);

  insert_arr(t, arr, n);
  assert(t->root != NULL);

  test_color_constraint(t);
  test_search_constraint(t);

  delete_rbtree(t);
}

void test_distinct_values()
{
  const key_t entries[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12};
  const size_t n = sizeof(entries) / sizeof(entries[0]);
  test_rb_constraints(entries, n);
}

void test_duplicate_values()
{
  const key_t entries[] = {10, 5, 5, 34, 6, 23, 12, 12, 6, 12};
  const size_t n = sizeof(entries) / sizeof(entries[0]);
  test_rb_constraints(entries, n);
}

void test_minmax_suite()
{
  key_t entries[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12};
  const size_t n = sizeof(entries) / sizeof(entries[0]);
  test_minmax(entries, n);
}

//test_to_array를 실행하기위한 함수
void test_array_suite()
{
  key_t entries[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12};
  const size_t n = sizeof(entries) / sizeof(entries[0]);
  test_to_array(entries, n);
}

//추가적인 테스트를 위한 함수
void test_test_test(const key_t *arr, const size_t n)
{
  rbtree *t = new_rbtree();
  assert(t != NULL);

  insert_arr(t, arr, n);
  qsort((void *)arr, n, sizeof(key_t), comp);

  key_t *res = calloc(n, sizeof(key_t));
  rbtree_to_array(t, res, n);

  for (int i = 0; i < n; i++)
  {
    printf(" %d ", arr[i]);
    assert(arr[i] == res[i]);
  }
  printf("\n\n");
  rbtree_print(t, t->root);
  printf("\n\n");

  node_t *p = rbtree_find(t, 23);
  rbtree_erase(t, p);

  rbtree_print(t, t->root);
  printf("\n\n");

  node_t *d = rbtree_find(t, 1);
  rbtree_erase(t, d);

  rbtree_print(t, t->root);
  printf("\n\n");

  delete_rbtree(t);
  printf("/*---- test_test_test completed ----*/ \n\n");
}

void test_test()
{
  key_t entries[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12, 13, 100, 200, 35, 72, 125, 9, 1, 49};
  const size_t n = sizeof(entries) / sizeof(entries[0]);
  test_test_test(entries, n);
  test_minmax(entries, n);
  test_rb_constraints(entries, n);
}

int main(void)
{
  test_init();
  test_insert_single(1024);
  test_find_single(512, 1024);
  test_erase_root(128);
  test_minmax_suite();
  test_distinct_values();
  test_duplicate_values();
  test_array_suite();
  test_test();
  printf("/*---Passed all tests! Great job, master.---*/\n\n");
}