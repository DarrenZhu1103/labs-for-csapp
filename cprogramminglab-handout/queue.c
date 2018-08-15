/* 
 * Code for basic C skills diagnostic.
 * Developed for courses 15-213/18-213/15-513 by R. E. Bryant, 2017
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>

#include "harness.h"
#include "queue.h"

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
    queue_t *q =  malloc(sizeof(queue_t));
    /* What if malloc returned NULL? */
    if (!q)
      return NULL;
    q->head = NULL;
    q->size = 0;
    q->tail = NULL;
    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
	if (!q)
		return;
    /* How about freeing the list elements? */
    list_ele_t *p = q -> head, *t;
    while (p) {
    	t = p;
    	p = p -> next;
    	free(t);
    }
    /* Free queue structure */
    free(q);
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool q_insert_head(queue_t *q, int v)
{
    list_ele_t *newh;
    /* What should you do if the q is NULL? */
    if (!q)
    	return false;
    newh = malloc(sizeof(list_ele_t));
    /* What if malloc returned NULL? */
    if (!newh)
    	return false;
    newh->value = v;
    newh->next = q->head;
    q->head = newh;
    if (q -> size == 0)
    	q -> tail = newh;
    q -> size++;
    return true;
}


/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
 */
bool q_insert_tail(queue_t *q, int v)
{
    /* You need to write the complete code for this function */
    /* Remember: It should operate in O(1) time */
    if (!q)
    	return false;
    if (q -> size == 0)
    	return q_insert_head(q, v);
    list_ele_t *newh = malloc(sizeof(list_ele_t));
    /* What if malloc returned NULL? */
    if (!newh)
    	return false;
    newh -> value = v;
    newh -> next = NULL;
    q -> tail -> next = newh;
    q -> tail = newh;
    q -> size++;
    return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If vp non-NULL and element removed, store removed value at *vp.
  Any unused storage should be freed
*/
bool q_remove_head(queue_t *q, int *vp)
{
	if (!q || !q -> head)
		return false;
    /* You need to fix up this code. */
    list_ele_t *p = q -> head;
    if (vp)
    	*vp = p -> value;
    q->head = q->head->next;
    if (q -> size == 1)
    	q -> tail = NULL;
    free(p);
    q -> size--;
    return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    /* You need to write the code for this function */
    /* Remember: It should operate in O(1) time */
    if (!q)
    	return 0;
    return q -> size;
}

/*
  Reverse elements in queue.

  Your implementation must not allocate or free any elements (e.g., by
  calling q_insert_head or q_remove_head).  Instead, it should modify
  the pointers in the existing data structure.
 */
void q_reverse(queue_t *q)
{
    /* You need to write the code for this function */
    if (!q || !q -> head)
    	return;
    list_ele_t *p = q -> head, *r = p -> next, *temp;
    q -> head = q -> tail;
    q -> tail = p;
    p -> next = NULL;
    while (r != NULL) {
    	temp = r -> next;
    	r -> next = p;
    	p = r;
    	r = temp;
    }
}
