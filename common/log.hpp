#pragma once

#include <stdarg.h> 
#include <stdio.h> 
#include <stdlib.h> // exit

namespace VNES_LOG{

#define LOG(severity, format, ...) log(__FILE__, __LINE__, severity, format, ##__VA_ARGS__)

enum Severity{
    INFO = 1,
    WARN,
    ERROR,
    FATAL // fatal should crash the program immediately 
};

void log(const char* __file__, int __line__, Severity severity, const char* log_str, ...){
    switch(severity){
        case INFO:
            printf("INFO  ");
            break;
        case WARN:
            printf("WARN  ");
            break;
        case ERROR:
            printf("ERROR ");
            break;
        case FATAL:
            printf("FATAL ");
            break;
        default:
            printf("Unexpected severity '%d' in VNES_LOG::LOG", severity);
            break;
    }

    printf("%s:%d ", __file__, __line__);
    va_list argptr;
    va_start(argptr, log_str);
    vfprintf(stdout, log_str, argptr);
    va_end(argptr);
    printf("\n");
}

}
