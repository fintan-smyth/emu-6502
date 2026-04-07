//      ___________ ___ __       ____   ___
//     / ____/ ___// (_) /_     / __ \ <  /
//    / /_   \__ \/ / / __ \   / / / / / /
//   / __/  ___/ / / / /_/ /  / /_/ / / /
//  /_/    /____/_/_/_.___/   \____(_)_/

#ifndef _FSLIB_H
# define _FSLIB_H

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

// ### Generic Dynamic array ###
//

#define SAFE_CMP(X, Y) ((X > Y) - (X < Y))

#define FS_VEC_INITSIZE 32

#define FS_DECLARE_VEC(T, tag)                                                         \
                                                                                       \
typedef struct Vec_##tag                                                               \
{                                                                                      \
    T		*data;                                                                     \
    size_t	size;                                                                      \
    size_t	capacity;                                                                  \
} Vec_##tag;                                                                           \
                                                                                       \
static inline Vec_##tag *vec_##tag##_init()                                            \
{                                                                                      \
    Vec_##tag *vec = (Vec_##tag *)calloc(1, sizeof(Vec_##tag));                        \
                                                                                       \
	vec->capacity = FS_VEC_INITSIZE;                                                   \
    vec->data = (T *)calloc(FS_VEC_INITSIZE, sizeof(T));                               \
	return vec;                                                                        \
}                                                                                      \
                                                                                       \
static inline void vec_##tag##_push(Vec_##tag *v, T value)                             \
{                                                                                      \
    if (v->size >= v->capacity)                                                        \
	{                                                                                  \
        v->capacity *= 2;                                                              \
        v->data = (T *)realloc(v->data, v->capacity * sizeof(T));                      \
    }                                                                                  \
    v->data[v->size++] = value;                                                        \
}                                                                                      \
                                                                                       \
static inline void vec_##tag##_free(Vec_##tag *v, void (*del)(T))                      \
{                                                                                      \
    if (del != NULL)                                                                   \
	{                                                                                  \
		for (size_t i = 0; i < v->size; i++)                                           \
			del(v->data[i]);                                                           \
	}                                                                                  \
    free(v->data);                                                                     \
	free(v);                                                                           \
}                                                                                      \
                                                                                       \
static inline T vec_##tag##_pop(Vec_##tag *v)                                          \
{                                                                                      \
	return v->data[--v->size];                                                         \
}                                                                                      \

FS_DECLARE_VEC(char *, str);

#ifdef DEBUG
	#define ASSERT_QUEUE_POP() assert(q->size > 0 && "Popping empty queue!")
#else
	#define ASSERT_QUEUE_POP() do {} while (0)
#endif

// ### Generic Double Ended Queue (Linked list implementation) ###
//
#define FS_DECLARE_QUEUE(T, tag)                                                       \
                                                                                       \
typedef struct Queue_##tag##_node                                                      \
{                                                                                      \
	struct Queue_##tag##_node	*next;                                                 \
	struct Queue_##tag##_node	*prev;                                                 \
	T							val;                                                   \
}	Queue_##tag##_node;                                                                \
                                                                                       \
typedef struct Queue_##tag                                                             \
{                                                                                      \
	Queue_##tag##_node 	*head;                                                         \
	size_t				size;                                                          \
}	Queue_##tag;                                                                       \
                                                                                       \
static inline Queue_##tag *queue_##tag##_init(void)                                    \
{                                                                                      \
	return calloc(1, sizeof(Queue_##tag));                                             \
}                                                                                      \
                                                                                       \
static inline void queue_##tag##_push_head(Queue_##tag *q, T val)                      \
{                                                                                      \
	Queue_##tag##_node *new = calloc(1, sizeof(*new));                                 \
                                                                                       \
	new->val = val;                                                                    \
	if (q->head == NULL)                                                               \
	{                                                                                  \
		new->next = new;                                                               \
		new->prev = new;                                                               \
		q->head = new;                                                                 \
		q->size++;                                                                     \
		return ;                                                                       \
	}                                                                                  \
	Queue_##tag##_node *tmp = q->head->prev;                                           \
	new->next = q->head;                                                               \
	q->head->prev = new;                                                               \
	new->prev = tmp;                                                                   \
	tmp->next = new;                                                                   \
	q->head = new;                                                                     \
	q->size++;                                                                         \
}                                                                                      \
                                                                                       \
static inline void queue_##tag##_push_tail(Queue_##tag *q, T val)                      \
{                                                                                      \
	Queue_##tag##_node *new = calloc(1, sizeof(*new));                                 \
                                                                                       \
	new->val = val;                                                                    \
	if (q->head == NULL)                                                               \
	{                                                                                  \
		new->next = new;                                                               \
		new->prev = new;                                                               \
		q->head = new;                                                                 \
		q->size++;                                                                     \
		return ;                                                                       \
	}                                                                                  \
	Queue_##tag##_node *tmp = q->head->prev;                                           \
	new->next = q->head;                                                               \
	q->head->prev = new;                                                               \
	new->prev = tmp;                                                                   \
	tmp->next = new;                                                                   \
	q->size++;                                                                         \
}                                                                                      \
                                                                                       \
static inline T queue_##tag##_pop_head(Queue_##tag *q)                                 \
{                                                                                      \
	ASSERT_QUEUE_POP();                                                                \
	T out = q->head->val;                                                              \
                                                                                       \
	if (q->size <= 1)                                                                  \
	{                                                                                  \
		free(q->head);                                                                 \
		q->head = NULL;                                                                \
		q->size = 0;                                                                   \
		return out;                                                                    \
	}                                                                                  \
                                                                                       \
	Queue_##tag##_node *head = q->head;                                                \
	head->prev->next = head->next;                                                     \
	head->next->prev = head->prev;                                                     \
	q->head = head->next;                                                              \
	free(head);                                                                        \
	q->size--;                                                                         \
	return out;                                                                        \
}                                                                                      \
                                                                                       \
static inline T queue_##tag##_pop_tail(Queue_##tag *q)                                 \
{                                                                                      \
	ASSERT_QUEUE_POP();                                                                \
	T out = q->head->prev->val;                                                        \
                                                                                       \
	if (q->size <= 1)                                                                  \
	{                                                                                  \
		free(q->head);                                                                 \
		q->head = NULL;                                                                \
		q->size = 0;                                                                   \
		return out;                                                                    \
	}                                                                                  \
                                                                                       \
	Queue_##tag##_node *tail = q->head->prev;                                          \
	tail->next->prev = tail->prev;                                                     \
	tail->prev->next = tail->next;                                                     \
	free(tail);                                                                        \
	q->size--;                                                                         \
	return out;                                                                        \
}                                                                                      \
                                                                                       \
static inline void queue_##tag##_clear(Queue_##tag *q, void (*del)(T))                 \
{                                                                                      \
	if (del != NULL)                                                                   \
	{                                                                                  \
		while (q->size > 0)                                                            \
			del(queue_##tag##_pop_head(q));                                            \
	}                                                                                  \
	else                                                                               \
	{                                                                                  \
		while (q->size > 0)                                                            \
			queue_##tag##_pop_head(q);                                                 \
	}                                                                                  \
}                                                                                      \



// ### Generic Double Ended Queue (Array implementation) ###
//

#define FS_ARRQUEUE_INITSIZE 32
#define FS_DECLARE_ARRQUEUE(T, tag)                                                    \
                                                                                       \
typedef struct ArrQueue_##tag                                                          \
{                                                                                      \
	T		*data;                                                                     \
	ssize_t	head;                                                                      \
	ssize_t	tail;                                                                      \
	size_t	size;                                                                      \
	size_t	capacity;                                                                  \
}	ArrQueue_##tag;                                                                    \
                                                                                       \
static inline ArrQueue_##tag *arrqueue_##tag##_init()                                  \
{                                                                                      \
	ArrQueue_##tag *q = calloc(1, sizeof(*q));                                         \
                                                                                       \
	q->capacity = FS_ARRQUEUE_INITSIZE;                                                \
	q->data = calloc(FS_ARRQUEUE_INITSIZE, sizeof(T));                                 \
	q->head = 1;                                                                       \
	q->tail = 0;                                                                       \
	return q;                                                                          \
}                                                                                      \
                                                                                       \
static inline void _arrqueue_##tag##_grow(ArrQueue_##tag *q)                           \
{                                                                                      \
	T *new_data = malloc(q->capacity * 2 * sizeof(T));                                 \
                                                                                       \
	size_t chunk_size = q->capacity - q->head;                                         \
	memcpy(new_data, &q->data[q->head], chunk_size * sizeof(T));                       \
	memcpy(&new_data[chunk_size], q->data, (q->capacity - chunk_size) * sizeof(T));    \
	free(q->data);                                                                     \
	q->head = 0;                                                                       \
	q->tail = q->capacity - 1;                                                         \
	q->capacity *= 2;                                                                  \
	q->data = new_data;                                                                \
}                                                                                      \
                                                                                       \
static inline void arrqueue_##tag##_push_tail(ArrQueue_##tag *q, T val)                \
{                                                                                      \
	if (q->size >= q->capacity)                                                        \
		_arrqueue_##tag##_grow(q);                                                     \
                                                                                       \
    q->tail = (q->tail + 1) % q->capacity;                                             \
	q->data[q->tail] = val;                                                            \
	q->size++;                                                                         \
}                                                                                      \
                                                                                       \
static inline void arrqueue_##tag##_push_head(ArrQueue_##tag *q, T val)                \
{                                                                                      \
	if (q->size >= q->capacity)                                                        \
		_arrqueue_##tag##_grow(q);                                                     \
                                                                                       \
    q->head = (q->head + q->capacity - 1) % q->capacity;                               \
	q->data[q->head] = val;                                                            \
	q->size++;                                                                         \
}                                                                                      \
                                                                                       \
static inline T arrqueue_##tag##_pop_head(ArrQueue_##tag *q)                           \
{                                                                                      \
	ASSERT_QUEUE_POP();                                                                \
	T out = q->data[q->head];                                                          \
	q->head = (q->head + 1) % q->capacity;                                             \
	q->size--;                                                                         \
	return out;                                                                        \
}                                                                                      \
                                                                                       \
static inline T arrqueue_##tag##_pop_tail(ArrQueue_##tag *q)                           \
{                                                                                      \
	ASSERT_QUEUE_POP();                                                                \
	T out = q->data[q->tail];                                                          \
	q->tail = (q->tail + q->capacity - 1) % q->capacity;                               \
	q->size--;                                                                         \
	return out;                                                                        \
}                                                                                      \
                                                                                       \
static inline void arrqueue_##tag##_free(ArrQueue_##tag *q, void (*del)(T))            \
{                                                                                      \
	if (del != NULL && q->size > 0)                                                    \
	{                                                                                  \
		for (size_t i = q->head; q->size-- > 0; i = (i + 1) % q->capacity)             \
			del(q->data[i]);                                                           \
	}                                                                                  \
	free(q->data);                                                                     \
	free(q);                                                                           \
}                                                                                      \


// ### Generic BSTSet ###
//

typedef enum { RBT_RED = 0, RBT_BLACK } RBTColor;

#define FS_DECLARE_BSTSET(T, tag, cmp)                                                 \
                                                                                       \
                                                                                       \
typedef struct BSTSet_##tag##_node                                                     \
{                                                                                      \
	T							val;                                                   \
	RBTColor					col;                                                   \
	struct BSTSet_##tag##_node	*left;                                                 \
	struct BSTSet_##tag##_node	*right;                                                \
	struct BSTSet_##tag##_node	*parent;                                               \
}	BSTSet_##tag##_node;                                                               \
                                                                                       \
typedef struct BSTSet_##tag                                                            \
{                                                                                      \
	BSTSet_##tag##_node	*tree;                                                         \
	size_t				size;                                                          \
}	BSTSet_##tag;                                                                      \
                                                                                       \
static void _bstset_##tag##_depth_r(BSTSet_##tag##_node *node,                         \
									int depth, int *max)                               \
{                                                                                      \
	if (node == NULL)                                                                  \
		return ;                                                                       \
	if (depth > *max)                                                                  \
		*max = depth;                                                                  \
	_bstset_##tag##_depth_r(node->left, depth + 1, max);                               \
	_bstset_##tag##_depth_r(node->right, depth + 1, max);                              \
}                                                                                      \
                                                                                       \
static inline int _bstset_##tag##_depth(BSTSet_##tag *set)                             \
{                                                                                      \
	int depth = 0;                                                                     \
                                                                                       \
	_bstset_##tag##_depth_r(set->tree, 0, &depth);                                     \
	return depth;                                                                      \
}                                                                                      \
                                                                                       \
static void	draw_bstset_##tag##_r(BSTSet_##tag##_node *node, int inv_depth, int x, int y, Vec_str *canvas)\
{                                                                                      \
	if (node == NULL)                                                                  \
		return ;                                                                       \
	canvas->data[y][x] = (node->col == RBT_BLACK) ? 'B' : 'R';                         \
	int	x_diff = 2;                                                                    \
	int	y_diff = 1;                                                                    \
	for (int i = 1; i < inv_depth; i++)                                                \
	{                                                                                  \
		x_diff *= 2;                                                                   \
		y_diff *= 2;                                                                   \
	}                                                                                  \
	if (node->right != NULL)                                                           \
	{                                                                                  \
		for (int i = 1; i < x_diff; i++)                                               \
			canvas->data[y][x + i] = '-';                                              \
		draw_bstset_##tag##_r(node->right, inv_depth - 1, x + x_diff, y, canvas);      \
	}                                                                                  \
	if (node->left != NULL)                                                            \
	{                                                                                  \
		for (int i = 1; i < y_diff; i++)                                               \
			canvas->data[y + i][x] = '|';                                              \
		draw_bstset_##tag##_r(node->left, inv_depth - 1, x, y + y_diff, canvas);       \
	}                                                                                  \
}                                                                                      \
                                                                                       \
Vec_str	*draw_bstset_##tag(BSTSet_##tag *set)                                          \
{                                                                                      \
	Vec_str *canvas = vec_str_init();                                                  \
                                                                                       \
	int max_depth = _bstset_##tag##_depth(set);                                        \
	int width = 1;                                                                     \
	int height = 1;                                                                    \
	int scale_x = 2;                                                                   \
	int scale_y = 1;                                                                   \
                                                                                       \
	for (int i = 1; i <= max_depth; i++)                                               \
	{                                                                                  \
		width += scale_x;                                                              \
		height += scale_y;                                                             \
		scale_x *= 2;                                                                  \
		scale_y *= 2;                                                                  \
	}                                                                                  \
	                                                                                   \
	for (int i = 0; i < height; i++)                                                   \
	{                                                                                  \
		vec_str_push(canvas, calloc(width + 1, sizeof(char)));                         \
		memset(canvas->data[i], ' ', width);                                           \
	}                                                                                  \
	                                                                                   \
	draw_bstset_##tag##_r(set->tree, max_depth, 0, 0, canvas);                         \
                                                                                       \
	size_t i = 0;                                                                      \
	for (; i < canvas->size; i++)                                                      \
	{                                                                                  \
		int j = width - 1;                                                             \
		for (; j >= 0 && canvas->data[i][j] == ' '; j--)                               \
			canvas->data[i][j] = '\0';                                                 \
		if (j == -1)                                                                   \
		{                                                                              \
			size_t size = canvas->size;                                                \
			while (i < size)                                                           \
			{                                                                          \
				free(vec_str_pop(canvas));                                             \
				i++;                                                                   \
			}                                                                          \
			break ;                                                                    \
		}                                                                              \
	}                                                                                  \
                                                                                       \
	return canvas;                                                                     \
}                                                                                      \
                                                                                       \
static int _bstset_##tag##_validate_r(BSTSet_##tag##_node *node)                       \
{                                                                                      \
	if (node == NULL)                                                                  \
		return 1;                                                                      \
                                                                                       \
	if (node->col == RBT_RED)                                                          \
	{                                                                                  \
		if ((node->left != NULL && node->left->col == RBT_RED)                         \
			|| (node->right != NULL && node->right->col == RBT_RED))                   \
		{                                                                              \
			dprintf(2, "RBT violation: multiple red nodes in a row\n");                \
			return -1;                                                                 \
		}                                                                              \
	}                                                                                  \
                                                                                       \
	int left_height = _bstset_##tag##_validate_r(node->left);                          \
	int right_height = _bstset_##tag##_validate_r(node->right);                        \
                                                                                       \
	if (left_height == -1 || right_height == -1                                        \
		|| (left_height != right_height                                                \
		&& dprintf(2, "RBT violation: black height mismatch\n"))                       \
	)                                                                                  \
		return -1;                                                                     \
                                                                                       \
	return (node->col == RBT_BLACK) ? left_height + 1 : left_height;                   \
}                                                                                      \
                                                                                       \
static inline bool bstset_##tag##_validate(BSTSet_##tag *set)                          \
{                                                                                      \
	if (set->tree == NULL)                                                             \
		return true;                                                                   \
                                                                                       \
	if (set->tree->col != RBT_BLACK)                                                   \
	{                                                                                  \
		dprintf(2, "RBT violation: root node is not black\n");                         \
		return false;                                                                  \
	}                                                                                  \
	                                                                                   \
	return (_bstset_##tag##_validate_r(set->tree) != -1);                              \
}                                                                                      \
                                                                                       \
static void _bstset_##tag##_print(BSTSet_##tag##_node *node, int space)                \
{                                                                                      \
    if (node == NULL) return;                                                          \
    space += 8;                                                                        \
    _bstset_##tag##_print(node->right, space);                                         \
	int i;                                                                             \
    for (i = 8; i < space - 6; i++) printf(" ");                                       \
    for (; i < space; i++) printf("-");                                                \
    printf("%s%04X\e[m\n", node->col == RBT_RED ? "\e[41;30m" : "\e[47;30m", node->val);\
    _bstset_##tag##_print(node->left, space);                                          \
}                                                                                      \
                                                                                       \
void	bstset_##tag##_print_rb(BSTSet_##tag *set)                                     \
{                                                                                      \
	Vec_str *canvas = draw_bstset_##tag(set);                                          \
                                                                                       \
	for (size_t i = 0; i < canvas->size; i++)                                          \
	{                                                                                  \
		for (int j = 0; canvas->data[i][j]; j++)                                       \
		{                                                                              \
			char c = canvas->data[i][j];                                               \
			if (c == 'R')                                                              \
				printf("\e[41;30m%c\e[m", c);                                          \
			else if (c == 'B')                                                         \
				printf("\e[47;30m%c\e[m", c);                                          \
			else                                                                       \
				printf("%c", c);                                                       \
		}                                                                              \
		printf("\n");                                                                  \
	}                                                                                  \
	vec_str_free(canvas, (void (*)(char *))free);                                      \
}                                                                                      \
                                                                                       \
static inline BSTSet_##tag *bstset_##tag##_init()                                      \
{                                                                                      \
	BSTSet_##tag *set = calloc(1, sizeof(*set));                                       \
                                                                                       \
	return set;                                                                        \
}                                                                                      \
                                                                                       \
static inline BSTSet_##tag##_node *_bstset_##tag##_min(BSTSet_##tag##_node *node)      \
{                                                                                      \
	while (node->left != NULL)                                                         \
		node = node->left;                                                             \
	return node;                                                                       \
}                                                                                      \
                                                                                       \
static inline void _bstset_##tag##_transplant(BSTSet_##tag *set,                       \
												BSTSet_##tag##_node *dead,             \
												BSTSet_##tag##_node *node)             \
{                                                                                      \
	if (dead->parent == NULL)                                                          \
		set->tree = node;                                                              \
	else if (dead == dead->parent->left)                                               \
		dead->parent->left = node;                                                     \
	else                                                                               \
		dead->parent->right = node;                                                    \
	if (node != NULL)                                                                  \
		node->parent = dead->parent;                                                   \
}                                                                                      \
                                                                                       \
static inline void _bstset_##tag##_rotate_left(BSTSet_##tag *set,                      \
												BSTSet_##tag##_node *node)             \
{                                                                                      \
	BSTSet_##tag##_node *right = node->right;                                          \
                                                                                       \
	node->right = right->left;                                                         \
	if (right->left != NULL)                                                           \
		right->left->parent = node;                                                    \
                                                                                       \
	right->parent = node->parent;                                                      \
	if (node->parent == NULL)                                                          \
		set->tree = right;                                                             \
	else if (node == node->parent->left)                                               \
		node->parent->left = right;                                                    \
	else                                                                               \
		node->parent->right = right;                                                   \
                                                                                       \
	right->left = node;                                                                \
	node->parent = right;                                                              \
}                                                                                      \
                                                                                       \
static inline void _bstset_##tag##_rotate_right(BSTSet_##tag *set,                     \
												BSTSet_##tag##_node *node)             \
{                                                                                      \
	BSTSet_##tag##_node *left = node->left;                                            \
                                                                                       \
	node->left = left->right;                                                          \
	if (left->right != NULL)                                                           \
		left->right->parent = node;                                                    \
                                                                                       \
	left->parent = node->parent;                                                       \
	if (node->parent == NULL)                                                          \
		set->tree = left;                                                              \
	else if (node == node->parent->left)                                               \
		node->parent->left = left;                                                     \
	else                                                                               \
		node->parent->right = left;                                                    \
                                                                                       \
	left->right = node;                                                                \
	node->parent = left;                                                               \
}                                                                                      \
                                                                                       \
/* static inline void print_bstset_##tag(BSTSet_##tag *set) */                         \
                                                                                       \
static inline void _bstset_##tag##_balance_insert(BSTSet_##tag *set,                   \
													BSTSet_##tag##_node *node)         \
{                                                                                      \
	while (node->parent != NULL && node->parent->col == RBT_RED)                       \
	{                                                                                  \
		if (node->parent == node->parent->parent->left)                                \
		{                                                                              \
			BSTSet_##tag##_node *uncle = node->parent->parent->right;                  \
			if (uncle != NULL && uncle->col == RBT_RED)                                \
			{                                                                          \
				node->parent->col = RBT_BLACK;                                         \
				uncle->col = RBT_BLACK;                                                \
				node->parent->parent->col = RBT_RED;                                   \
				node = node->parent->parent;                                           \
			}                                                                          \
			else                                                                       \
			{                                                                          \
				if (node == node->parent->right)                                       \
				{                                                                      \
					node = node->parent;                                               \
					_bstset_##tag##_rotate_left(set, node);                            \
				}                                                                      \
				node->parent->col = RBT_BLACK;                                         \
				node->parent->parent->col = RBT_RED;                                   \
				_bstset_##tag##_rotate_right(set, node->parent->parent);               \
			}                                                                          \
		}                                                                              \
		else                                                                           \
		{                                                                              \
			BSTSet_##tag##_node *uncle = node->parent->parent->left;                   \
			if (uncle != NULL && uncle->col == RBT_RED)                                \
			{                                                                          \
				node->parent->col = RBT_BLACK;                                         \
				uncle->col = RBT_BLACK;                                                \
				node->parent->parent->col = RBT_RED;                                   \
				node = node->parent->parent;                                           \
			}                                                                          \
			else                                                                       \
			{                                                                          \
				if (node == node->parent->left)                                        \
				{                                                                      \
					node = node->parent;                                               \
					_bstset_##tag##_rotate_right(set, node);                           \
				}                                                                      \
				node->parent->col = RBT_BLACK;                                         \
				node->parent->parent->col = RBT_RED;                                   \
				_bstset_##tag##_rotate_left(set, node->parent->parent);                \
			}                                                                          \
		}                                                                              \
	}                                                                                  \
	set->tree->col = RBT_BLACK;                                                        \
}                                                                                      \
                                                                                       \
static inline int bstset_##tag##_insert(BSTSet_##tag *set, T val)                      \
{                                                                                      \
	BSTSet_##tag##_node *current = set->tree;                                          \
	BSTSet_##tag##_node *parent = NULL;                                                \
	int (*cmpf)(T, T) = (int (*)(T, T))cmp;                                            \
	int	diff = 0;                                                                      \
                                                                                       \
	while (current != NULL)                                                            \
	{                                                                                  \
		parent = current;                                                              \
		diff = (cmpf == NULL)                                                          \
			? SAFE_CMP(val, current->val)                                              \
			: cmpf(val, current->val);                                                 \
		if (diff > 0)                                                                  \
			current = current->right;                                                  \
		else if (diff < 0)                                                             \
			current = current->left;                                                   \
		else                                                                           \
			return 1;                                                                  \
	}                                                                                  \
                                                                                       \
	BSTSet_##tag##_node *new = calloc(1, sizeof(*new));                                \
	new->val = val;                                                                    \
	new->parent = parent;                                                              \
	if (parent == NULL)                                                                \
		set->tree = new;                                                               \
	else if (diff > 0)                                                                 \
		parent->right = new;                                                           \
	else                                                                               \
		parent->left = new;                                                            \
	set->size++;                                                                       \
                                                                                       \
	/* printf("\e[?25l");                                                                 \
	printf("\e[2J\e[H");                                                               \
	bstset_##tag##_print_rb(set);                                                      \
	getchar(); */                                                                         \
	_bstset_##tag##_balance_insert(set, new);                                          \
	/* printf("\e[2J\e[H");                                                               \
	bstset_##tag##_print_rb(set);                                                      \
	getchar();                                                                         \
	printf("\e[?25h"); */                                                                \
	                                                                                   \
	return 0;                                                                          \
}                                                                                      \
                                                                                       \
static inline void _bstset_##tag##_remove_fix(BSTSet_##tag *set,                       \
												BSTSet_##tag##_node *node,             \
												BSTSet_##tag##_node *parent)           \
{                                                                                      \
	while (node != set->tree && (node == NULL || node->col == RBT_BLACK))              \
	{                                                                                  \
		if (node == parent->left)                                                      \
		{                                                                              \
			BSTSet_##tag##_node *sibling = parent->right;                              \
			if (sibling == NULL)                                                       \
			{                                                                          \
				node = parent;                                                         \
				parent = node->parent;                                                 \
				continue ;                                                             \
			}                                                                          \
			if (sibling->col == RBT_RED)                                               \
			{                                                                          \
				sibling->col = RBT_BLACK;                                              \
				parent->col = RBT_RED;                                                 \
				_bstset_##tag##_rotate_left(set, parent);                              \
				sibling = parent->right;                                               \
				if (sibling == NULL)                                                   \
				{                                                                      \
					node = parent;                                                     \
					parent = node->parent;                                             \
					continue ;                                                         \
				}                                                                      \
			}                                                                          \
                                                                                       \
			bool left_black = sibling->left == NULL || sibling->left->col == RBT_BLACK;\
			bool right_black = sibling->right == NULL || sibling->right->col == RBT_BLACK;\
                                                                                       \
			if (left_black && right_black)                                             \
			{                                                                          \
				sibling->col = RBT_RED;                                                \
				node = parent;                                                         \
				parent = node->parent;                                                 \
			}                                                                          \
			else                                                                       \
			{                                                                          \
				if (right_black)                                                       \
				{                                                                      \
					if (sibling->left != NULL)                                         \
						sibling->left->col = RBT_BLACK;                                \
					sibling->col = RBT_RED;                                            \
					_bstset_##tag##_rotate_right(set, sibling);                        \
					sibling = parent->right;                                           \
				}                                                                      \
				sibling->col = parent->col;                                            \
				parent->col = RBT_BLACK;                                               \
				if (sibling->right != NULL)                                            \
					sibling->right->col = RBT_BLACK;                                   \
				_bstset_##tag##_rotate_left(set, parent);                              \
				node = set->tree;                                                      \
			}                                                                          \
		}                                                                              \
		else                                                                           \
		{                                                                              \
			BSTSet_##tag##_node *sibling = parent->left;                               \
			if (sibling == NULL)                                                       \
			{                                                                          \
				node = parent;                                                         \
				parent = node->parent;                                                 \
				continue ;                                                             \
			}                                                                          \
			if (sibling->col == RBT_RED)                                               \
			{                                                                          \
				sibling->col = RBT_BLACK;                                              \
				parent->col = RBT_RED;                                                 \
				_bstset_##tag##_rotate_right(set, parent);                             \
				sibling = parent->left;                                                \
				if (sibling == NULL)                                                   \
				{                                                                      \
					node = parent;                                                     \
					parent = node->parent;                                             \
					continue ;                                                         \
				}                                                                      \
			}                                                                          \
                                                                                       \
			bool left_black = sibling->left == NULL || sibling->left->col == RBT_BLACK;\
			bool right_black = sibling->right == NULL || sibling->right->col == RBT_BLACK;\
                                                                                       \
			if (left_black && right_black)                                             \
			{                                                                          \
				sibling->col = RBT_RED;                                                \
				node = parent;                                                         \
				parent = node->parent;                                                 \
			}                                                                          \
			else                                                                       \
			{                                                                          \
				if (left_black)                                                        \
				{                                                                      \
					if (sibling->right != NULL)                                        \
						sibling->right->col = RBT_BLACK;                               \
					sibling->col = RBT_RED;                                            \
					_bstset_##tag##_rotate_left(set, sibling);                         \
					sibling = parent->left;                                            \
				}                                                                      \
				sibling->col = parent->col;                                            \
				parent->col = RBT_BLACK;                                               \
				if (sibling->left != NULL)                                             \
					sibling->left->col = RBT_BLACK;                                    \
				_bstset_##tag##_rotate_right(set, parent);                             \
				node = set->tree;                                                      \
			}                                                                          \
		}                                                                              \
	}                                                                                  \
	if (node != NULL)                                                                  \
		node->col = RBT_BLACK;                                                         \
}                                                                                      \
                                                                                       \
static inline int bstset_##tag##_remove(BSTSet_##tag *set, T val, void (*del)(T))      \
{                                                                                      \
	BSTSet_##tag##_node *target = set->tree;                                           \
	int (*cmpf)(T, T) = (int (*)(T, T))cmp;                                            \
	while (target != NULL)                                                             \
	{                                                                                  \
		int diff = (cmpf == NULL)                                                      \
			? SAFE_CMP(val, target->val)                                               \
			: cmpf(val, target->val);                                                  \
		if (diff == 0)                                                                 \
			break ;                                                                    \
		target = diff < 0 ? target->left : target->right;                              \
	}                                                                                  \
	if (target == NULL)                                                                \
		return 1;                                                                      \
                                                                                       \
	BSTSet_##tag##_node *replacement = NULL;                                           \
	BSTSet_##tag##_node *to_fix = NULL;                                                \
	BSTSet_##tag##_node *fix_parent;                                                   \
	RBTColor orig_col = target->col;                                                   \
                                                                                       \
	if (target->left == NULL)                                                          \
	{                                                                                  \
		to_fix = target->right;                                                        \
		fix_parent = target->parent;                                                   \
		_bstset_##tag##_transplant(set, target, target->right);                        \
	}                                                                                  \
	else if (target->right == NULL)                                                    \
	{                                                                                  \
		to_fix = target->left;                                                         \
		fix_parent = target->parent;                                                   \
		_bstset_##tag##_transplant(set, target, target->left);                         \
	}                                                                                  \
	else                                                                               \
	{                                                                                  \
		replacement = _bstset_##tag##_min(target->right);                              \
		orig_col = replacement->col;                                                   \
		to_fix = replacement->right;                                                   \
                                                                                       \
		if (replacement->parent == target)                                             \
		{                                                                              \
			fix_parent = replacement;                                                  \
		}                                                                              \
		else                                                                           \
		{                                                                              \
			fix_parent = replacement->parent;                                          \
			_bstset_##tag##_transplant(set, replacement, replacement->right);          \
			replacement->right = target->right;                                        \
			replacement->right->parent = replacement;                                  \
		}                                                                              \
		_bstset_##tag##_transplant(set, target, replacement);                          \
		replacement->left = target->left;                                              \
		replacement->left->parent = replacement;                                       \
		replacement->col = target->col;                                                \
	}                                                                                  \
                                                                                       \
	if (orig_col == RBT_BLACK)                                                         \
		_bstset_##tag##_remove_fix(set, to_fix, fix_parent);                           \
                                                                                       \
	if (set->tree != NULL)                                                             \
		set->tree->col = RBT_BLACK;                                                    \
                                                                                       \
    if (del != NULL)                                                                   \
		del(target->val);                                                              \
	free(target);                                                                      \
	set->size--;                                                                       \
	return 0;                                                                          \
}                                                                                      \
                                                                                       \
static inline int bstset_##tag##_contains(BSTSet_##tag *set, T val)                    \
{                                                                                      \
	BSTSet_##tag##_node *current = set->tree;                                          \
	int (*cmpf)(T, T) = (int (*)(T, T))cmp;                                            \
                                                                                       \
	while (current != NULL)                                                            \
	{                                                                                  \
		int diff = (cmpf == NULL)                                                      \
			? SAFE_CMP(val, current->val)                                              \
			: cmpf(val, current->val);                                                 \
		if (diff > 0)                                                                  \
			current = current->right;                                                  \
		else if (diff < 0)                                                             \
			current = current->left;                                                   \
		else                                                                           \
			return true;                                                               \
	}                                                                                  \
                                                                                       \
	return false;                                                                      \
}                                                                                      \
                                                                                       \
static void _bstset_##tag##_clear_r(BSTSet_##tag##_node *node, void (*del)(T))         \
{                                                                                      \
	if (node == NULL)                                                                  \
		return ;                                                                       \
	_bstset_##tag##_clear_r(node->left, del);                                          \
	_bstset_##tag##_clear_r(node->right, del);                                         \
	if (del != NULL)                                                                   \
		del(node->val);                                                                \
	free(node);                                                                        \
}                                                                                      \
                                                                                       \
static inline void bstset_##tag##_clear(BSTSet_##tag *set, void (*del)(T))             \
{                                                                                      \
	_bstset_##tag##_clear_r(set->tree, del);                                           \
	set->tree = NULL;                                                                  \
	set->size = 0;                                                                     \
}                                                                                      \


// ### Generic BSTMap ###
//

#define FS_DECLARE_BSTMAP(K, V, tag, cmp)                                              \
                                                                                       \
                                                                                       \
typedef struct BSTMap_##tag##_node                                                     \
{                                                                                      \
	K							key;                                                   \
	V							val;                                                   \
	RBTColor					col;                                                   \
	struct BSTMap_##tag##_node	*left;                                                 \
	struct BSTMap_##tag##_node	*right;                                                \
	struct BSTMap_##tag##_node	*parent;                                               \
}	BSTMap_##tag##_node;                                                               \
                                                                                       \
typedef struct BSTMap_##tag                                                            \
{                                                                                      \
	BSTMap_##tag##_node	*tree;                                                         \
	size_t				size;                                                          \
}	BSTMap_##tag;                                                                      \
                                                                                       \
static void _bstmap_##tag##_depth_r(BSTMap_##tag##_node *node,                         \
									int depth, int *max)                               \
{                                                                                      \
	if (node == NULL)                                                                  \
		return ;                                                                       \
	if (depth > *max)                                                                  \
		*max = depth;                                                                  \
	_bstmap_##tag##_depth_r(node->left, depth + 1, max);                               \
	_bstmap_##tag##_depth_r(node->right, depth + 1, max);                              \
}                                                                                      \
                                                                                       \
static inline int _bstmap_##tag##_depth(BSTMap_##tag *map)                             \
{                                                                                      \
	int depth = 0;                                                                     \
                                                                                       \
	_bstmap_##tag##_depth_r(map->tree, 0, &depth);                                     \
	return depth;                                                                      \
}                                                                                      \
                                                                                       \
static void	draw_bstmap_##tag##_r(BSTMap_##tag##_node *node, int inv_depth, int x, int y, Vec_str *canvas)\
{                                                                                      \
	if (node == NULL)                                                                  \
		return ;                                                                       \
	canvas->data[y][x] = (node->col == RBT_BLACK) ? 'B' : 'R';                         \
	int	x_diff = 2;                                                                    \
	int	y_diff = 1;                                                                    \
	for (int i = 1; i < inv_depth; i++)                                                \
	{                                                                                  \
		x_diff *= 2;                                                                   \
		y_diff *= 2;                                                                   \
	}                                                                                  \
	if (node->right != NULL)                                                           \
	{                                                                                  \
		for (int i = 1; i < x_diff; i++)                                               \
			canvas->data[y][x + i] = '-';                                              \
		draw_bstmap_##tag##_r(node->right, inv_depth - 1, x + x_diff, y, canvas);      \
	}                                                                                  \
	if (node->left != NULL)                                                            \
	{                                                                                  \
		for (int i = 1; i < y_diff; i++)                                               \
			canvas->data[y + i][x] = '|';                                              \
		draw_bstmap_##tag##_r(node->left, inv_depth - 1, x, y + y_diff, canvas);       \
	}                                                                                  \
}                                                                                      \
                                                                                       \
Vec_str	*draw_bstmap_##tag(BSTMap_##tag *map)                                          \
{                                                                                      \
	Vec_str *canvas = vec_str_init();                                                  \
                                                                                       \
	int max_depth = _bstmap_##tag##_depth(map);                                        \
	int width = 1;                                                                     \
	int height = 1;                                                                    \
	int scale_x = 2;                                                                   \
	int scale_y = 1;                                                                   \
                                                                                       \
	for (int i = 1; i <= max_depth; i++)                                               \
	{                                                                                  \
		width += scale_x;                                                              \
		height += scale_y;                                                             \
		scale_x *= 2;                                                                  \
		scale_y *= 2;                                                                  \
	}                                                                                  \
	                                                                                   \
	for (int i = 0; i < height; i++)                                                   \
	{                                                                                  \
		vec_str_push(canvas, calloc(width + 1, sizeof(char)));                         \
		memset(canvas->data[i], ' ', width);                                           \
	}                                                                                  \
	                                                                                   \
	draw_bstmap_##tag##_r(map->tree, max_depth, 0, 0, canvas);                         \
                                                                                       \
	size_t i = 0;                                                                      \
	for (; i < canvas->size; i++)                                                      \
	{                                                                                  \
		int j = width - 1;                                                             \
		for (; j >= 0 && canvas->data[i][j] == ' '; j--)                               \
			canvas->data[i][j] = '\0';                                                 \
		if (j == -1)                                                                   \
		{                                                                              \
			size_t size = canvas->size;                                                \
			while (i < size)                                                           \
			{                                                                          \
				free(vec_str_pop(canvas));                                             \
				i++;                                                                   \
			}                                                                          \
			break ;                                                                    \
		}                                                                              \
	}                                                                                  \
                                                                                       \
	return canvas;                                                                     \
}                                                                                      \
                                                                                       \
static int _bstmap_##tag##_validate_r(BSTMap_##tag##_node *node)                       \
{                                                                                      \
	if (node == NULL)                                                                  \
		return 1;                                                                      \
                                                                                       \
	if (node->col == RBT_RED)                                                          \
	{                                                                                  \
		if ((node->left != NULL && node->left->col == RBT_RED)                         \
			|| (node->right != NULL && node->right->col == RBT_RED))                   \
		{                                                                              \
			dprintf(2, "RBT violation: multiple red nodes in a row\n");                \
			return -1;                                                                 \
		}                                                                              \
	}                                                                                  \
                                                                                       \
	int left_height = _bstmap_##tag##_validate_r(node->left);                          \
	int right_height = _bstmap_##tag##_validate_r(node->right);                        \
                                                                                       \
	if (left_height == -1 || right_height == -1                                        \
		|| (left_height != right_height                                                \
		&& dprintf(2, "RBT violation: black height mismatch\n"))                       \
	)                                                                                  \
		return -1;                                                                     \
                                                                                       \
	return (node->col == RBT_BLACK) ? left_height + 1 : left_height;                   \
}                                                                                      \
                                                                                       \
static inline bool bstmap_##tag##_validate(BSTMap_##tag *map)                          \
{                                                                                      \
	if (map->tree == NULL)                                                             \
		return true;                                                                   \
                                                                                       \
	if (map->tree->col != RBT_BLACK)                                                   \
	{                                                                                  \
		dprintf(2, "RBT violation: root node is not black\n");                         \
		return false;                                                                  \
	}                                                                                  \
	                                                                                   \
	return (_bstmap_##tag##_validate_r(map->tree) != -1);                              \
}                                                                                      \
                                                                                       \
static void _bstmap_##tag##_print(BSTMap_##tag##_node *node, int space)                \
{                                                                                      \
    if (node == NULL) return;                                                          \
    space += 8;                                                                        \
    _bstmap_##tag##_print(node->right, space);                                         \
	int i;                                                                             \
    for (i = 8; i < space - 6; i++) printf(" ");                                       \
    for (; i < space; i++) printf("-");                                                \
    printf("%s%3d\e[m\n", node->col == RBT_RED ? "\e[41;30m" : "\e[47;30m", node->key);\
    _bstmap_##tag##_print(node->left, space);                                          \
}                                                                                      \
                                                                                       \
void	bstmap_##tag##_print_rb(BSTMap_##tag *map)                                     \
{                                                                                      \
	Vec_str *canvas = draw_bstmap_##tag(map);                                          \
                                                                                       \
	for (size_t i = 0; i < canvas->size; i++)                                          \
	{                                                                                  \
		for (int j = 0; canvas->data[i][j]; j++)                                       \
		{                                                                              \
			char c = canvas->data[i][j];                                               \
			if (c == 'R')                                                              \
				printf("\e[41;30m%c\e[m", c);                                          \
			else if (c == 'B')                                                         \
				printf("\e[47;30m%c\e[m", c);                                          \
			else                                                                       \
				printf("%c", c);                                                       \
		}                                                                              \
		printf("\n");                                                                  \
	}                                                                                  \
	vec_str_free(canvas, (void (*)(char *))free);                                      \
}                                                                                      \
                                                                                       \
static inline BSTMap_##tag *bstmap_##tag##_init()                                      \
{                                                                                      \
	BSTMap_##tag *map = calloc(1, sizeof(*map));                                       \
                                                                                       \
	return map;                                                                        \
}                                                                                      \
                                                                                       \
static inline BSTMap_##tag##_node *_bstmap_##tag##_min(BSTMap_##tag##_node *node)      \
{                                                                                      \
	while (node->left != NULL)                                                         \
		node = node->left;                                                             \
	return node;                                                                       \
}                                                                                      \
                                                                                       \
static inline void _bstmap_##tag##_transplant(BSTMap_##tag *map,                       \
												BSTMap_##tag##_node *dead,             \
												BSTMap_##tag##_node *node)             \
{                                                                                      \
	if (dead->parent == NULL)                                                          \
		map->tree = node;                                                              \
	else if (dead == dead->parent->left)                                               \
		dead->parent->left = node;                                                     \
	else                                                                               \
		dead->parent->right = node;                                                    \
	if (node != NULL)                                                                  \
		node->parent = dead->parent;                                                   \
}                                                                                      \
                                                                                       \
static inline void _bstmap_##tag##_rotate_left(BSTMap_##tag *map,                      \
												BSTMap_##tag##_node *node)             \
{                                                                                      \
	BSTMap_##tag##_node *right = node->right;                                          \
                                                                                       \
	node->right = right->left;                                                         \
	if (right->left != NULL)                                                           \
		right->left->parent = node;                                                    \
                                                                                       \
	right->parent = node->parent;                                                      \
	if (node->parent == NULL)                                                          \
		map->tree = right;                                                             \
	else if (node == node->parent->left)                                               \
		node->parent->left = right;                                                    \
	else                                                                               \
		node->parent->right = right;                                                   \
                                                                                       \
	right->left = node;                                                                \
	node->parent = right;                                                              \
}                                                                                      \
                                                                                       \
static inline void _bstmap_##tag##_rotate_right(BSTMap_##tag *map,                     \
												BSTMap_##tag##_node *node)             \
{                                                                                      \
	BSTMap_##tag##_node *left = node->left;                                            \
                                                                                       \
	node->left = left->right;                                                          \
	if (left->right != NULL)                                                           \
		left->right->parent = node;                                                    \
                                                                                       \
	left->parent = node->parent;                                                       \
	if (node->parent == NULL)                                                          \
		map->tree = left;                                                              \
	else if (node == node->parent->left)                                               \
		node->parent->left = left;                                                     \
	else                                                                               \
		node->parent->right = left;                                                    \
                                                                                       \
	left->right = node;                                                                \
	node->parent = left;                                                               \
}                                                                                      \
                                                                                       \
/* static inline void print_bstmap_##tag(BSTMap_##tag *map) */                         \
                                                                                       \
static inline void _bstmap_##tag##_balance_insert(BSTMap_##tag *map,                   \
													BSTMap_##tag##_node *node)         \
{                                                                                      \
	while (node->parent != NULL && node->parent->col == RBT_RED)                       \
	{                                                                                  \
		if (node->parent == node->parent->parent->left)                                \
		{                                                                              \
			BSTMap_##tag##_node *uncle = node->parent->parent->right;                  \
			if (uncle != NULL && uncle->col == RBT_RED)                                \
			{                                                                          \
				node->parent->col = RBT_BLACK;                                         \
				uncle->col = RBT_BLACK;                                                \
				node->parent->parent->col = RBT_RED;                                   \
				node = node->parent->parent;                                           \
			}                                                                          \
			else                                                                       \
			{                                                                          \
				if (node == node->parent->right)                                       \
				{                                                                      \
					node = node->parent;                                               \
					_bstmap_##tag##_rotate_left(map, node);                            \
				}                                                                      \
				node->parent->col = RBT_BLACK;                                         \
				node->parent->parent->col = RBT_RED;                                   \
				_bstmap_##tag##_rotate_right(map, node->parent->parent);               \
			}                                                                          \
		}                                                                              \
		else                                                                           \
		{                                                                              \
			BSTMap_##tag##_node *uncle = node->parent->parent->left;                   \
			if (uncle != NULL && uncle->col == RBT_RED)                                \
			{                                                                          \
				node->parent->col = RBT_BLACK;                                         \
				uncle->col = RBT_BLACK;                                                \
				node->parent->parent->col = RBT_RED;                                   \
				node = node->parent->parent;                                           \
			}                                                                          \
			else                                                                       \
			{                                                                          \
				if (node == node->parent->left)                                        \
				{                                                                      \
					node = node->parent;                                               \
					_bstmap_##tag##_rotate_right(map, node);                           \
				}                                                                      \
				node->parent->col = RBT_BLACK;                                         \
				node->parent->parent->col = RBT_RED;                                   \
				_bstmap_##tag##_rotate_left(map, node->parent->parent);                \
			}                                                                          \
		}                                                                              \
	}                                                                                  \
	map->tree->col = RBT_BLACK;                                                        \
}                                                                                      \
                                                                                       \
static inline int bstmap_##tag##_insert(BSTMap_##tag *map, K key, V val)               \
{                                                                                      \
	BSTMap_##tag##_node *current = map->tree;                                          \
	BSTMap_##tag##_node *parent = NULL;                                                \
	int (*cmpf)(K, K) = (int (*)(K, K))cmp;                                            \
	int	diff = 0;                                                                      \
                                                                                       \
	while (current != NULL)                                                            \
	{                                                                                  \
		parent = current;                                                              \
		diff = (cmpf == NULL)                                                          \
			? SAFE_CMP(key, current->key)                                              \
			: cmpf(key, current->key);                                                 \
		if (diff > 0)                                                                  \
			current = current->right;                                                  \
		else if (diff < 0)                                                             \
			current = current->left;                                                   \
		else                                                                           \
			return 1;                                                                  \
	}                                                                                  \
                                                                                       \
	BSTMap_##tag##_node *new = calloc(1, sizeof(*new));                                \
	new->key = key;                                                                    \
	new->val = val;                                                                    \
	new->parent = parent;                                                              \
	if (parent == NULL)                                                                \
		map->tree = new;                                                               \
	else if (diff > 0)                                                                 \
		parent->right = new;                                                           \
	else                                                                               \
		parent->left = new;                                                            \
	map->size++;                                                                       \
                                                                                       \
	/* printf("\e[?25l");                                                                 \
	printf("\e[2J\e[H");                                                               \
	bstmap_##tag##_print_rb(map);                                                      \
	getchar(); */                                                                         \
	_bstmap_##tag##_balance_insert(map, new);                                          \
	/* printf("\e[2J\e[H");                                                               \
	bstmap_##tag##_print_rb(map);                                                      \
	getchar();                                                                         \
	printf("\e[?25h"); */                                                                \
	                                                                                   \
	return 0;                                                                          \
}                                                                                      \
                                                                                       \
static inline void _bstmap_##tag##_remove_fix(BSTMap_##tag *map,                       \
												BSTMap_##tag##_node *node,             \
												BSTMap_##tag##_node *parent)           \
{                                                                                      \
	while (node != map->tree && (node == NULL || node->col == RBT_BLACK))              \
	{                                                                                  \
		if (node == parent->left)                                                      \
		{                                                                              \
			BSTMap_##tag##_node *sibling = parent->right;                              \
			if (sibling == NULL)                                                       \
			{                                                                          \
				node = parent;                                                         \
				parent = node->parent;                                                 \
				continue ;                                                             \
			}                                                                          \
			if (sibling->col == RBT_RED)                                               \
			{                                                                          \
				sibling->col = RBT_BLACK;                                              \
				parent->col = RBT_RED;                                                 \
				_bstmap_##tag##_rotate_left(map, parent);                              \
				sibling = parent->right;                                               \
				if (sibling == NULL)                                                   \
				{                                                                      \
					node = parent;                                                     \
					parent = node->parent;                                             \
					continue ;                                                         \
				}                                                                      \
			}                                                                          \
                                                                                       \
			bool left_black = sibling->left == NULL || sibling->left->col == RBT_BLACK;\
			bool right_black = sibling->right == NULL || sibling->right->col == RBT_BLACK;\
                                                                                       \
			if (left_black && right_black)                                             \
			{                                                                          \
				sibling->col = RBT_RED;                                                \
				node = parent;                                                         \
				parent = node->parent;                                                 \
			}                                                                          \
			else                                                                       \
			{                                                                          \
				if (right_black)                                                       \
				{                                                                      \
					if (sibling->left != NULL)                                         \
						sibling->left->col = RBT_BLACK;                                \
					sibling->col = RBT_RED;                                            \
					_bstmap_##tag##_rotate_right(map, sibling);                        \
					sibling = parent->right;                                           \
				}                                                                      \
				sibling->col = parent->col;                                            \
				parent->col = RBT_BLACK;                                               \
				if (sibling->right != NULL)                                            \
					sibling->right->col = RBT_BLACK;                                   \
				_bstmap_##tag##_rotate_left(map, parent);                              \
				node = map->tree;                                                      \
			}                                                                          \
		}                                                                              \
		else                                                                           \
		{                                                                              \
			BSTMap_##tag##_node *sibling = parent->left;                               \
			if (sibling == NULL)                                                       \
			{                                                                          \
				node = parent;                                                         \
				parent = node->parent;                                                 \
				continue ;                                                             \
			}                                                                          \
			if (sibling->col == RBT_RED)                                               \
			{                                                                          \
				sibling->col = RBT_BLACK;                                              \
				parent->col = RBT_RED;                                                 \
				_bstmap_##tag##_rotate_right(map, parent);                             \
				sibling = parent->left;                                                \
				if (sibling == NULL)                                                   \
				{                                                                      \
					node = parent;                                                     \
					parent = node->parent;                                             \
					continue ;                                                         \
				}                                                                      \
			}                                                                          \
                                                                                       \
			bool left_black = sibling->left == NULL || sibling->left->col == RBT_BLACK;\
			bool right_black = sibling->right == NULL || sibling->right->col == RBT_BLACK;\
                                                                                       \
			if (left_black && right_black)                                             \
			{                                                                          \
				sibling->col = RBT_RED;                                                \
				node = parent;                                                         \
				parent = node->parent;                                                 \
			}                                                                          \
			else                                                                       \
			{                                                                          \
				if (left_black)                                                        \
				{                                                                      \
					if (sibling->right != NULL)                                        \
						sibling->right->col = RBT_BLACK;                               \
					sibling->col = RBT_RED;                                            \
					_bstmap_##tag##_rotate_left(map, sibling);                         \
					sibling = parent->left;                                            \
				}                                                                      \
				sibling->col = parent->col;                                            \
				parent->col = RBT_BLACK;                                               \
				if (sibling->left != NULL)                                             \
					sibling->left->col = RBT_BLACK;                                    \
				_bstmap_##tag##_rotate_right(map, parent);                             \
				node = map->tree;                                                      \
			}                                                                          \
		}                                                                              \
	}                                                                                  \
	if (node != NULL)                                                                  \
		node->col = RBT_BLACK;                                                         \
}                                                                                      \
                                                                                       \
static inline int bstmap_##tag##_remove(BSTMap_##tag *map, K key, void (*del)(K))      \
{                                                                                      \
	BSTMap_##tag##_node *target = map->tree;                                           \
	int (*cmpf)(K, K) = (int (*)(K, K))cmp;                                            \
	while (target != NULL)                                                             \
	{                                                                                  \
		int diff = (cmpf == NULL)                                                      \
			? SAFE_CMP(key, target->key)                                               \
			: cmpf(key, target->key);                                                  \
		if (diff == 0)                                                                 \
			break ;                                                                    \
		target = diff < 0 ? target->left : target->right;                              \
	}                                                                                  \
	if (target == NULL)                                                                \
		return 1;                                                                      \
                                                                                       \
	BSTMap_##tag##_node *replacement = NULL;                                           \
	BSTMap_##tag##_node *to_fix = NULL;                                                \
	BSTMap_##tag##_node *fix_parent;                                                   \
	RBTColor orig_col = target->col;                                                   \
                                                                                       \
	if (target->left == NULL)                                                          \
	{                                                                                  \
		to_fix = target->right;                                                        \
		fix_parent = target->parent;                                                   \
		_bstmap_##tag##_transplant(map, target, target->right);                        \
	}                                                                                  \
	else if (target->right == NULL)                                                    \
	{                                                                                  \
		to_fix = target->left;                                                         \
		fix_parent = target->parent;                                                   \
		_bstmap_##tag##_transplant(map, target, target->left);                         \
	}                                                                                  \
	else                                                                               \
	{                                                                                  \
		replacement = _bstmap_##tag##_min(target->right);                              \
		orig_col = replacement->col;                                                   \
		to_fix = replacement->right;                                                   \
                                                                                       \
		if (replacement->parent == target)                                             \
		{                                                                              \
			fix_parent = replacement;                                                  \
		}                                                                              \
		else                                                                           \
		{                                                                              \
			fix_parent = replacement->parent;                                          \
			_bstmap_##tag##_transplant(map, replacement, replacement->right);          \
			replacement->right = target->right;                                        \
			replacement->right->parent = replacement;                                  \
		}                                                                              \
		_bstmap_##tag##_transplant(map, target, replacement);                          \
		replacement->left = target->left;                                              \
		replacement->left->parent = replacement;                                       \
		replacement->col = target->col;                                                \
	}                                                                                  \
                                                                                       \
	if (orig_col == RBT_BLACK)                                                         \
		_bstmap_##tag##_remove_fix(map, to_fix, fix_parent);                           \
                                                                                       \
	if (map->tree != NULL)                                                             \
		map->tree->col = RBT_BLACK;                                                    \
                                                                                       \
    if (del != NULL)                                                                   \
		del(target->key);                                                              \
	free(target);                                                                      \
	map->size--;                                                                       \
	return 0;                                                                          \
}                                                                                      \
                                                                                       \
static inline bool bstmap_##tag##_contains(BSTMap_##tag *map, K key)                   \
{                                                                                      \
	BSTMap_##tag##_node *current = map->tree;                                          \
	int (*cmpf)(K, K) = (int (*)(K, K))cmp;                                            \
                                                                                       \
	while (current != NULL)                                                            \
	{                                                                                  \
		int diff = (cmpf == NULL)                                                      \
			? SAFE_CMP(key, current->key)                                              \
			: cmpf(key, current->key);                                                 \
		if (diff > 0)                                                                  \
			current = current->right;                                                  \
		else if (diff < 0)                                                             \
			current = current->left;                                                   \
		else                                                                           \
			return true;                                                               \
	}                                                                                  \
                                                                                       \
	return false;                                                                      \
}                                                                                      \
                                                                                       \
static void _bstmap_##tag##_clear_r(BSTMap_##tag##_node *node, void (*del)(K))         \
{                                                                                      \
	if (node == NULL)                                                                  \
		return ;                                                                       \
	_bstmap_##tag##_clear_r(node->left, del);                                          \
	_bstmap_##tag##_clear_r(node->right, del);                                         \
	if (del != NULL)                                                                   \
		del(node->key);                                                                \
	free(node);                                                                        \
}                                                                                      \
                                                                                       \
static inline void bstmap_##tag##_clear(BSTMap_##tag *map, void (*del)(K))             \
{                                                                                      \
	_bstmap_##tag##_clear_r(map->tree, del);                                           \
	map->tree = NULL;                                                                  \
	map->size = 0;                                                                     \
}                                                                                      \


#endif // _FSLIB_H
