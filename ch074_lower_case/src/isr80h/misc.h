#ifndef ISR80H_MISC_H
#define ISR80H_MISC_H

// misc stands for miscellaneous

struct interrupt_frame;

void* system_call_0_sum(struct interrupt_frame* interrupt_frame);

#endif
