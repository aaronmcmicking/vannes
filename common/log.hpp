#pragma once

#include <stdarg.h> 
#include <stdio.h> 
#include <stdlib.h> // exit
#include <string>
#include <sstream>

namespace VNES_LOG{

#define LOG(severity, format, ...) log(__FILE__, __LINE__, '\n', severity, format, ##__VA_ARGS__)

enum Severity{
    DEBUG = 1,
    INFO,
    WARN,
    ERROR,
    FATAL // fatals usually crash the program, but that is the responsibility of the caller 
};

// the min level to output log
Severity log_level = DEBUG;
bool file_out = false;
bool fatal_error_kills_program = false;
const std::string log_filename {"vannes.log"};

void init_log(){
    if(file_out){ fclose(fopen(log_filename.c_str(), "w")); }
}

void log(const char* __file__, int __line__, char endchar, Severity severity, const char* log_str, ...){
    if(severity < log_level && !file_out){ return; }

    std::string sev_label {};
    std::stringstream file_line {};
    switch(severity){
        case DEBUG:
            sev_label = "DEBUG ";
            break;
        case INFO:
            sev_label = "INFO  ";
            break;
        case WARN:
            sev_label = "WARN  ";
            break;
        case ERROR:
            sev_label = "ERROR ";
            break;
        case FATAL:
            sev_label = "FATAL ";
            break;
        default:
            printf("Unexpected severity '%d' in VNES_LOG::LOG", severity);
            sev_label = "UNKNOWN ";
            break;
    }

    file_line << __file__ << ':' << __line__ << ' ';
    //printf("%s:%d ", __file__, __line__);

    // stdout
    if(severity >= log_level){
        printf("%s", sev_label.c_str());
        printf("%s", file_line.str().c_str());
        va_list argptr;
        va_start(argptr, log_str);
        vfprintf(stdout, log_str, argptr);
        va_end(argptr);
        printf("%c", endchar);
    }

    // log file
    if(file_out){
        FILE* logfile = fopen(log_filename.c_str(), "a");
        fprintf(logfile, sev_label.c_str());
        fprintf(logfile, file_line.str().c_str());
        va_list argptr;
        va_start(argptr, log_str);
        vfprintf(logfile, log_str, argptr);
        va_end(argptr);
        fprintf(logfile, "%c", endchar);
        fclose(logfile);
    }

    // fatal kills
    if(severity == FATAL && fatal_error_kills_program){
        throw std::runtime_error("FATAL error encountered");
    }
}

}
