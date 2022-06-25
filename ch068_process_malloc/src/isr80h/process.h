#ifndef ISR80H_PROCESS_H
#define ISR80H_PROCESS_H

struct interrupt_frame;

void* system_call_6_start_load_process(struct interrupt_frame* interrupt_frame);

#endif