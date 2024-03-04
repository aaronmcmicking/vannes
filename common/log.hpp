#pragma once

#include <stdarg.h> 
#include <stdio.h> 
#include <stdlib.h> // exit

namespace VNES_LOG{

enum Severity{
    INFO = 1,
    WARN,
    ERROR,
    FATAL // fatal should crash the program immediately 
};

void log(const char* _file, int _line, Severity severity, const char* log_str, ...){
    switch(severity){
        case INFO:
            printf("INFO  - ");
            break;
        case WARN:
            printf("WARN  - ");
            break;
        case ERROR:
            printf("ERROR - ");
            break;
        case FATAL:
            printf("FATAL - ");
            break;
        default:
            printf("Unexpected severity '%d' in VNES_LOG::LOG", severity);
            break;
    }

    printf("%s:%d: ", _file, _line);
    va_list argptr;
    va_start(argptr, log_str);
    vfprintf(stdout, log_str, argptr);
    va_end(argptr);
    printf("\n");

    if(severity == Severity::FATAL){ exit(1); }
}

}
