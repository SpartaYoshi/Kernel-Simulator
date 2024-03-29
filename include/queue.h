#include "commons.h"

extern process_queue_t idle_queue; // Queue of waiting processes
extern process_queue_t finished_queue; // Queue of finished processes

extern pcb_t nullp;

void init_queue(process_queue_t* q);
void enqueue(process_queue_t* q, pcb_t* data);
pcb_t* dequeue(process_queue_t* q);
int is_empty(process_queue_t* q);