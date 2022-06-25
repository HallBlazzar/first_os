#ifndef IO_H
#define IO_H

struct interrupt_frame;

void* system_call_1_print(struct interrupt_frame* interrupt_frame);
void* system_call_2_get_key(struct interrupt_frame* interrupt_frame);
void* system_call_3_putchar(struct interrupt_frame* interrupt_frame);

#endif
