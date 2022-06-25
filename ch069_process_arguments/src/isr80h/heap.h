#ifndef SYSTEM_CALL_HEAP_H
#define SYSTEM_CALL_HEAP_H

struct interrupt_frame;

void* system_call_4_malloc(struct interrupt_frame* interrupt_frame);
void* system_call_5_free(struct interrupt_frame* interrupt_frame);

#endif
