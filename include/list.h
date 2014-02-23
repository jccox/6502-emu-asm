#ifndef LIST_H_
#define LIST_H_

#include <stdio.h>

struct node_t {
	void *x;
	struct node_t *next;
};

struct List_T {

	int count; 		/* keep count of the number of nodes */
	
	struct node_t *head, *tail; 
};

enum direction{BEFORE, AFTER};
enum g_p{GET = 1, PUT = 2};
enum n_p{NEXT = 1, PREV = 2};
extern "C" {

	struct List_T * list_new(void);		      /* initialize a new list */

	void list_insert(struct List_T * list, int (*cmp)(void *node, void *), void *x, int direction); 

	void list_insertpos(struct List_T * list, void *x, int direction, int index);

	void *list_pop(struct List_T * list);

	void list_push(struct List_T * list, void *x); /* add element to the head of the list */

	void list_append(struct List_T * list, void *value);

	void list_free(struct List_T * *list, void (*f)(void *node)); /* free an existing list */

	void list_delnode(struct List_T * list, int (*cmp)(void *node, void *), void *find, void (*free_node)(void *node)); /* delete a specific node in a list */

	void print_list(struct List_T * list, void (*f)(void *node, FILE *out), FILE *out);

	int list_count(struct List_T * list);

	void list_copy(struct List_T * list, struct List_T * copy, void *(*f)(void *node)); /* copy the list */

	void list_reverse(struct List_T * list);

	int list_compare(struct List_T * list1, struct List_T * list2, int(*cmp)(void *,void *)); 

	void *list_findpos(struct List_T * list, int index);

	void *list_find(struct List_T * list, void *x, int (*cmp)(void *node, void *));

	struct List_T * list_split(struct List_T * list, void *x, int (*cmp)(void *node, void *));

	struct List_T * list_splitpos(struct List_T * list, int index);

	void list_join(struct List_T * receiver, struct List_T * giver);

	void list_next(void);
	void *list_get(void);
	void list_put(void *data);
	void list_reset(struct List_T*);
	void *list_iterate(struct List_T* list, void *data, int reset, int get_put, int next_prev);
}
#endif
