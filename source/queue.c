#include <stdlib.h>

#include "../include/commons.h"
#include "../include/queue.h"

process_queue_t idle_queue;
process_queue_t finished_queue;

pcb_t nullp = {NULL, -1, PRSTAT_NULL, 140, 0, 
			0, 0, 0, 0,0,0,
			NULL};

void init_queue(process_queue_t *q) {
  q->front = 0;
  q->rear = -1;
  q->size = 0;

  // Initialize idle process queue
  for (int i = 0; i < QUEUE_CAPACITY; i++)
	idle_queue.q[i] = (pcb_t *)&nullp;
}

void enqueue(process_queue_t *q, pcb_t *data) {

  if (q->size != QUEUE_CAPACITY) {

	if (q->rear == QUEUE_CAPACITY - 1) {
	  q->rear = -1;
	}

	q->q[++q->rear] = data;
	q->size++;
  }
}

pcb_t *dequeue(process_queue_t *q) {
  if (is_empty(q))
	return NULL;
  pcb_t *data = q->q[q->front++];

  if (q->front == QUEUE_CAPACITY) {
	q->front = 0;
  }

  q->size--;
  return data;
}

int is_empty(process_queue_t *q) { return (q->size == 0); }