/**
 * @file list.c
 * 
 * @author Joseph Cox
 *
 * @date 11/18/13
 *
 * Assignment: HW 8
 *
 * @brief Here are a bunch of functions implementing the linked list.
 *                             
 * @todo Null                
 *
 * @bugs None.
 **/




#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "list.h"

/* save yourself some typing */
/* emphasize the abstract nature of the type */
#define T struct List_T *


/**
 * @description Make a new node.
 * 
 * @param void *x A pointer to insert in a node.
 * @return struct node_t *new_node A pointer to the new node.
 */

struct node_t *new_node(void *x)
{
	struct node_t *node = (struct node_t *)malloc(sizeof(struct node_t));
	node->x = x;
	node->next = NULL;
	
	return node;
}

/**
 * @description Make a new list.
 * 
 * @return List_T A pointer to the new list.
 */

T list_new(void)
{
	T list = (struct List_T *)malloc(sizeof(struct List_T));

	assert(list);

	list->count = 0;
	list->head = NULL;
	list->tail = NULL;
	return list;
}


/**
 * @description Insert into the list.
 * 
 * @param struct List_T* list The list to insert into.
 * @param int (*cmp)(void *node, void *) A comparison function. Compare data and
 * return an int.
 * @param void *x The data to insert.
 * @param int direction Insert before or after the finding?
 */

void list_insert(T list, int (*cmp)(void *node, void *), void *x, int direction)
{
	

	struct node_t *prev = NULL, *cur;

	
	assert(list); 		/* does list exist? */

	cur = list->head;

	/*If we have no head, make a head.*/
	if(cur == NULL){
		list_push(list, x);
		return;
	}

	while(cur) {
		/* comparison*/
		if ((*cmp)(cur->x, x) == 0) {

			/*This allows a shift from prefix
			  to postfix insertions.*/
			if(direction == AFTER){
				prev = cur;
				cur = cur->next;
			}				
			/* Insert a node at the head of the list */
			if (prev == NULL) {
				list_push(list, x);
				return;
			}
			/*Insert at the tail.  We check prev->next
			  first in case direction == AFTER.
			  Note that this direction shift only really affects
			  tail node insertions.
			*/
			else if (prev->next == NULL || cur->next == NULL) {		       	
				list->tail = new_node(x);
				/*If we are doing postfix insertions,
				  then prev will be pointing at
				  the previous tail.*/
				if(direction == AFTER)
					prev->next = list->tail;
				else
					cur->next = list->tail;

				++list->count;
				return;
			}
			else { 	/* Add to somewhere in the middle */				
				prev->next = new_node(x);
				prev->next->next = cur;

				++list->count;
				return;
			}			
		}
		prev = cur;
		cur = cur->next;
	}      
	/*If we haven't found a spot, insert at the tail.*/
	list_append(list, x);
}


/**
 * @description Insert into the list.
 * 
 * @param struct List_T* list The list to insert into.
 * @param void * x Data to insert.
 *
 * @param int direction Insert before or after the finding?
 * @param int index The index to return the data from.
 */

void list_insertpos(struct List_T* list, void *x, int direction, int index)
{

	struct node_t *prev = NULL, *cur;

	
	int current_node = 1;

	assert(list); 		/* does list exist? */

	cur = list->head;

	/*If we have no head, make a head.*/
	if(cur == NULL){
		list_push(list, x);
		return;
	}

	while(cur) {
		/* comparison*/
		if (current_node == index) {
			
			/*This allows a shift from prefix
			  to postfix insertions.*/
			if(direction == AFTER){
				prev = cur;
				cur = cur->next;
			}				
			/* Insert a node at the head of the list */
			if (prev == NULL) {
				list_push(list, x);
				return;
			}
			/*Insert at the tail.  We check prev->next
			  first in case direction == AFTER.
			  Note that this direction shift only really affects
			  tail node insertions.
			*/
			else if (prev->next == NULL || cur->next == NULL) {		       	
				list->tail = new_node(x);
				/*If we are doing postfix insertions,
				  then prev will be pointing at
				  the previous tail.*/
				if(direction == AFTER)
					prev->next = list->tail;
				else
					cur->next = list->tail;

				++list->count;
				return;
			}
			else { 	/* Add to somewhere in the middle */				
				prev->next = new_node(x);
				prev->next->next = cur;

				++list->count;
				return;
			}			
		}
		prev = cur;
		cur = cur->next;
		++current_node;
	}      

}

/**
 * @description Return the head of the list
 *
 * @param struct List_T* list The list to pop from.
 *
 * @return void * The data in the head node
 */

void *list_pop(T list)
{
	struct node_t *tmp = list->head;
	void *data = tmp->x;
	
	list->head = list->head->next;
	
	free(tmp);

	--list->count;
	return data;
}


/**
 * @description put onto the head of the list
 *
 * @param struct List_T* list The list to push onto.
 *
 * @param void *x The data to put into the head node
 */

void list_push(T list, void *x)
{
	
	/* allocate space for an element of the list */
	struct node_t *cur = (struct node_t *)malloc(sizeof(struct node_t));
	assert(cur); 		/* did malloc fail? */

	assert(list); 	/* does list exist? */

	cur->x = x;
	cur->next = list->head;
 
	if(list->tail == NULL)
		list->tail = cur;

	list->head = cur;
	list->count++;
}

/**
 * @description put onto the head of the list
 *
 * @param struct List_T* list The list to append to.
 *
 * @param void *x The data to put into the head node
 */

void list_append(struct List_T* list, void *value)
{
	struct node_t *cur = new_node(value);
	assert(cur);

	assert(list);
	/*If there is no head, then this is the new head.*/
	if(list->head == NULL){
		list->head = list->tail = cur;
		++list->count;
		return;
	}
	
	list->tail->next = cur;
	list->tail = cur;

	++list->count;
}

/**
 * @description Free the entire list
 * 
 * @param struct List_T* *list The list to insert into.
 *
 * @param void (*f)(void *x) A freeing function.
 */

void list_free(T *list, void (*f)(void *x))
{
	struct node_t *cur, *next;
	
	assert(*list);
	/* free elements of list */
	for (cur = (*list)->head; cur; cur = next) {
		next = cur->next;
		/* function pointer to determine how to delete node -- data dependent */
		(*f)(cur->x);
		free(cur);
	}
	
	free(*list);

	*list = NULL;
}

/**
 * delete a specific node based on a comparison
 * comparison works like strcmp 0 if they match, a positive or negative value if they don't
 * delete the first node it finds that matches the criteria
 * @param struct List_T* list, the list to find the node in
 * @param int (*cmp)(void *node, void *cmp), the user defined comparison function. Compares the node_to a user defined value.
 * @param void *find, the value to find (will pass as the 2nd void * in the comparison function)
 * @param void (*f)(void *node), user provided function to free void *x
 */
void list_delnode(struct List_T* list, int (*cmp)(void *node, void *), void *find, void (*f)(void *node))
{
	struct node_t *prev = NULL, *cur;
	
	assert(list); 		/* does list exist? */

	cur = list->head;

	while(cur) {
		/* comparison*/
		if ((*cmp)(cur->x, find) == 0) {
			/* delete node at the head of the list */
			if (prev == NULL) {
				list->head = cur->next;
				(*f)(cur->x); /* free the node */
				free(cur);    /* free the pointer to the node */
				list->count--;
				return;
			}
			else if (cur->next == NULL) { /* delete at tail of list */
				list->tail = prev;
				prev->next = NULL;
				(*f)(cur->x); /* free the node */
				free(cur);    /* free the pointer to the node */
				list->count--;
				return;
			}
			else { 	/* delete somewhere in the middle */
				prev->next = cur->next;
				(*f)(cur->x); /* free the node */
				free(cur);    /* free the pointer to the node */
				list->count--;
				return;
			}
			
		}
		prev = cur;
		cur = cur->next;
	}
}

/**
 * print list, using user supplied print node function 
 * @param T list the list to walk through
 * @param void (*f)(void *x), function pointer to a user defined function that prints the node
 * @param FILE *out The file to print to.
 */
void print_list(T list, void (*f)(void *node, FILE *out), FILE *out) 
{
	struct node_t *cur;
	
	assert(list); 		/* does list exist? */

	cur = list->head;

	while(cur) {
		/* function pointer to determine how to print a node -- data dependent */
		(*f)(cur->x, out); 	/* cur->x is the node_to print */
		cur = cur->next;
	}
}

/**
 * @description Return the size of the list
 *
 * @param struct List_T* list The list to return the size of.
 *
 * @return void * The data found in that node.
 */

int list_count(T list)
{
	return list->count;
}

/**
 * @description Copy a list with this.
 *
 * @param struct List_T* list The list to copy from.
 * @param struct List_T* copy The list to copy into.
 *
 * @return void * (*f)(void *node) Use this to specify how to copy data.
 */
void list_copy(struct List_T* list, struct List_T* copy, void* (*f)(void *node))
{
	assert(list);
	assert(copy);

	struct node_t *cur;
	void *tmp = NULL;

	cur = list->head;
	
	while(cur) {
		tmp = (*f)(cur->x);
		list_push(copy, tmp);
		cur = cur->next;
	}
}

/**
 * @description Reverse a list with this.
 *
 * @param struct List_T* list The list to reverse.
 */

void list_reverse(T list)
{
	struct node_t *tmp = list->head;
	struct node_t *next = list->head;
	struct node_t *prev = NULL;

	while(tmp){
		next = tmp->next;
		tmp->next = prev;
		prev = tmp;
		list->head = prev;
		tmp = next;
	}
}

/**
 * @description Compare two lists
 * 
 * @param struct List_T* list1 The first list to compare from.
 *
 * @param struct List_T* list2 The second list to compare from.
 *
 * @param int(*cmp)(void *,void *) The function to use to compare.
 */

int list_compare(T list1, struct List_T* list2, int(*cmp)(void *,void *))		
{
	struct node_t *tmp1 = list1->head;
	struct node_t *tmp2 = list2->head;
	
	int cmp_flag;

	while(!(tmp1 == NULL || tmp2 == NULL)){
		if((cmp_flag = cmp(tmp1->x, tmp2->x)))
			return cmp_flag;
				
		tmp1 = tmp1->next;
		tmp2 = tmp2->next;
	}

	/*Finish the truth table!*/
	if(tmp1 == NULL && tmp2 != NULL)
		return -1;
	else if(tmp2 == NULL && tmp1 != NULL)
		return 1;
	else
		return 0;
}

/**
 * @description Return the data at some index.
 * 
 * @param struct List_T* list The list to search.
 *
 * @param int index The position to search for.
 */

void *list_findpos(struct List_T* list, int index)
{
	struct node_t *cur = list->head;

	int current_location = 1;

	while(cur && current_location != index){
		cur = cur->next;
		++current_location;
	}
	if(cur == NULL)
		return NULL;

	return cur->x;
}

/**
 * @description Find some data in a list through comparing.
 * 
 * @param struct List_T* list The list to search from
 *
 * @param void *x The data to search for.
 *
 * @param int(*cmp)(void *,void *) The function to use to compare.
 */

void *list_find(struct List_T* list, void *x, int (*cmp)(void *node, void *))
{
	struct node_t *cur = list->head;
	
	
	while(cur && cmp(cur->x, x) != 0){
		cur = cur->next;
	}
	/*We don't want a segfault, so we check for NULL.*/
	if(cur == NULL)
		return NULL;
	return cur->x;
}

/**
 * @description Split a list at a point designated by a comparison.
 * 
 * @param struct List_T* list The list to search from
 *
 * @param void *x The data to search for.
 *
 * @param int(*cmp)(void *,void *) The function to use to compare.
 */

struct List_T* list_split(struct List_T* list, void *x, int (*cmp)(void *node, void *))
{
	struct List_T* copy = list_new();

	struct node_t *cur = list->head;
	struct node_t *prev = NULL;
	int index = 1;
	
	/*If we split at the head, we give away the entire list.*/
	if(cmp(cur->x, x) == 0){
		copy->head = list->head;
		copy->count = list->count;
		copy->tail = list->tail;
		
		list->head = NULL;
		list->count = 0;
		list->tail = NULL;
		return copy;
	}

	while(cur && cmp(cur->x, x) != 0){
		prev = cur;
		cur = cur->next;
		++index;
	}

	if(cur == NULL){
		list_free(&copy, free);
		return NULL;
	}
	/*Split the list with this.*/
	prev->next = NULL;

	copy->head = cur;
	copy->tail = list->tail;
	copy->count = list->count - index;

	list->count = index;
	list->tail = prev;
	
	return copy;
}

/**
 * @description Split a list at a point designated by an index.
 * 
 * @param struct List_T* list The list to search from
 *
 * @param void *x The data to search for.
 *
 * @param int(*cmp)(void *,void *) The function to use to compare.
 */

struct List_T* list_splitpos(struct List_T* list, int index)
{
	struct List_T* copy = list_new();

	struct node_t *cur = list->head;
	struct node_t *prev = NULL;

	int current_location = 1;

	if(index == 1){
		copy->head = list->head;
		copy->count = list->count;
		copy->tail = list->tail;
		
		list->head = NULL;
		list->count = 0;
		list->tail = NULL;
		return copy;
	}
		
		

	while(cur && current_location != index){
		prev = cur;
		cur = cur->next;
		++current_location;
	}

	if(cur == NULL){
		list_free(&copy, free);
		return NULL;
	}
	/*Split the list with this.*/
	prev->next = NULL;

	copy->head = cur;
	copy->tail = list->tail;
	copy->count = list->count - index;

	list->count = index;
	list->tail = prev;
       
	return copy;	
}

/**
 * @description Merge two lists with this.
 * 
 * @param struct List_T* receiver The list to merge into.
 *
 * @param struct List_T* giver The list to merge from.
 */

void list_join(struct List_T* receiver, struct List_T* giver)
{
	receiver->tail->next = giver->head;
	receiver->tail = giver->tail;
	receiver->count = giver->count;
	
	free(giver);		
}

void *list_iterate(struct List_T* list, void *data, int reset, int get_put, int next_prev)
{
	static struct List_T *mylist = NULL;
	static struct node_t *current = NULL;
	struct node_t *tmp;
	if(reset == 1){
		current = list->head;
		mylist = list;
	}

	else if(next_prev == NEXT){
		if(current == NULL)
			return NULL;
		if(current != mylist->head || current->next != NULL)
			current = current->next;
	}
	else if(get_put == GET){
		if(current != NULL)
		return current->x;
	}
	else if(get_put == PUT){
		if(current == NULL){
			list_push(mylist, data);
			current = mylist->head;
			return NULL;
		}
		tmp = current->next;
		current->next = new_node(data);
		current->next->next = tmp;
	}
	
	return NULL;
}

void list_next(void)
{
	list_iterate(NULL, NULL, 0, 0, NEXT);
}

void *list_get(void)
{
	return list_iterate(NULL, NULL, 0, GET, 0);
}

void list_put(void *data)
{
	list_iterate(NULL, data, 0, PUT, 0);
}
void list_reset(struct List_T *list)
{
	list_iterate(list, NULL, 1, 0, 0);
}
