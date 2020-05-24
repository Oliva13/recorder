#ifndef _SYS_QUEUE_H
#define _SYS_QUEUE_H

struct queue {
	struct queue   *next;
	struct queue   *prev;
};

typedef struct queue *queue_t;

#define queue_init(head)	((head)->next = (head)->prev = (head))
#define queue_empty(head)	((head)->next == (head))
#define queue_next(q)		((q)->next)
#define queue_prev(q)		((q)->prev)
#define queue_first(head)	((head)->next)
#define queue_last(head)	((head)->prev)
#define queue_end(head,q)	((q) == (head))

// Get the struct for this entry
#define queue_entry(q, type, member) \
    ((type *)((char *)(q) - (unsigned long)(&((type *)0)->member)))

void	enqueue(queue_t, queue_t);
queue_t	dequeue(queue_t);
void	queue_insert(queue_t, queue_t);
void	queue_remove(queue_t);

#endif

