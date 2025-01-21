#ifndef __LOG_H__
#define __LOG_H__

#define LOG_LVL_DEBUG 1
#define LOG_LVL_INFO  2
#define LOG_LVL_WARN  3
#define LOG_LVL_ERROR 4

/*************************** USER DEFINE *******************************/

/***********************************************************************/
/** If don't use, comment it out.                                     **/
/***********************************************************************/
// file separator
#define _LOG_FILE_SEPARATOR '\\'

// global log level
// #define _LOG_GLOBAL_LVL          LOG_LVL_DEBUG

/***********************************************************************/
/** Define your print function here. For example:                     **/
/** #include <stdio.h>                                                **/
/** #define _LOG_USER_PRINT_FUNC(fmt, ...) printf(fmt, ##__VA_ARGS__) **/
/***********************************************************************/
#include "usart1.h"
#define _LOG_USER_PRINT_FUNC(fmt, ...) USART1_Printf(fmt, ##__VA_ARGS__)

/***********************************************************************/

#ifdef _DEBUG
#    ifdef _LOG_GLOBAL_LVL
#        undef LOG_LVL
#        define LOG_LVL _LOG_GLOBAL_LVL
#    endif

#    ifdef LOG_LVL
#        if (LOG_LVL == LOG_LVL_DEBUG) || (LOG_LVL == LOG_LVL_INFO) || (LOG_LVL == LOG_LVL_WARN) \
            || (LOG_LVL == LOG_LVL_ERROR)
#            define _LOG_ENABLE
#        else
#            error "The LOG_LVL value is incorrect"
#        endif
#    else
#        error "Please define LOG_LVL first"
#    endif
#endif


#if defined(_LOG_ENABLE)

#    define LOG_raw(fmt, ...) _LOG_USER_PRINT_FUNC(fmt, ##__VA_ARGS__)

#    include <string.h>

#    define _LOG_log_raw(level, fmt, ...)                                                                            \
        _LOG_USER_PRINT_FUNC("[%s] %s:%d (%s) >> " fmt, level, strrchr(__FILE__, _LOG_FILE_SEPARATOR) + 1, __LINE__, \
                             __FUNCTION__, ##__VA_ARGS__);
// debug
#    if LOG_LVL <= LOG_LVL_DEBUG
#        define LOG_D(fmt, ...) _LOG_log_raw("D", fmt, ##__VA_ARGS__)
#    else
#        define LOG_D(fmt, ...)
#    endif

// warning
#    if LOG_LVL <= LOG_LVL_WARN
#        define LOG_W(fmt, ...) _LOG_log_raw("W", fmt, ##__VA_ARGS__)
#    else
#        define LOG_W(fmt, ...)
#    endif

// info
#    if LOG_LVL <= LOG_LVL_INFO
#        define LOG_I(fmt, ...) _LOG_log_raw("I", fmt, ##__VA_ARGS__)
#    else
#        define LOG_I(fmt, ...)
#    endif

// error
#    if LOG_LVL <= LOG_LVL_ERROR
#        define LOG_E(fmt, ...) _LOG_log_raw("E", fmt, ##__VA_ARGS__)
#    else
#        define LOG_E(fmt, ...)
#    endif
#else
#    define LOG_raw(fmt, ...)
#    define LOG_E(fmt, ...)
#    define LOG_W(fmt, ...)
#    define LOG_I(fmt, ...)
#    define LOG_D(fmt, ...)
#endif

#endif