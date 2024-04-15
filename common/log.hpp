#pragma once

#include <stdarg.h> 
#include <stdio.h> 
#include <stdlib.h> // exit

namespace VNES_LOG{

#define LOG(severity, format, ...) log(__FILE__, __LINE__, severity, format, ##__VA_ARGS__)

enum Severity{
    DEBUG = 1,
    INFO,
    WARN,
    ERROR,
    FATAL // fatals usually crash the program, but that is the responsibility of the caller 
};

// the min level to output log
Severity log_level = (Severity)1;

void log(const char* __file__, int __line__, Severity severity, const char* log_str, ...){
    if(severity < log_level) return;
    switch(severity){
        case DEBUG:
            printf("DEBUG ");
            break;
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
