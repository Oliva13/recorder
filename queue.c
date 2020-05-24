//
// queue.c - generic queue management library
//

#include "queue.h"

//
// Insert element at tail of queue
//
void enqueue(queue_t head, queue_t item)
{
	item->next = head;
	item->prev = head->prev;
	item->prev->next = item;
	head->prev = item;
}

//
// Remove and return element of head of queue
//
queue_t dequeue(queue_t head)
{
	queue_t item;
	if (head->next == head)
		return ((queue_t)0);
	item = head->next;
	item->next->prev = head;
	head->next = item->next;
	return item;
}

//
// Insert element after specified element
//
void queue_insert(queue_t prev, queue_t item)
{
	item->prev = prev;
	item->next = prev->next;
	prev->next->prev = item;
	prev->next = item;
}

//
// Remove specified element from queue
//
void queue_remove(queue_t item)
{
	item->prev->next = item->next;
	item->next->prev = item->prev;
}
