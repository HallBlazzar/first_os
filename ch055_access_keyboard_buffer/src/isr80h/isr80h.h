#ifndef ISR80H_H
#define ISR80H_H

enum system_calls {
    SYSTEM_CALL_SUM,
    SYSTEM_CALL_PRINT,
    SYSTEM_CALL_GET_KEY,
};

void register_system_calls();


#endif