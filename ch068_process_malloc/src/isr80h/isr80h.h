#ifndef ISR80H_H
#define ISR80H_H

enum system_calls {
    SYSTEM_CALL_SUM,
    SYSTEM_CALL_PRINT,
    SYSTEM_CALL_GET_KEY,
    SYSTEM_CALL_PUT_CHAR,
    SYSTEM_CALL_MALLOC,
    SYSTEM_CALL_FREE,
    SYSTEM_CALL_START_LOAD_PROCESS,
};

void register_system_calls();


#endif