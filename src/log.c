#include "log.h"

#ifdef AETHER_LOG_ENABLED
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#endif /* AETHER_LOG_ENABLED */

#include "err.h"

#ifdef AETHER_LOG_ENABLED
#define A_LOG_ANSI_RESET  "\x1B[0m"
#define A_LOG_ANSI_RED    "\x1B[0;31m"
#define A_LOG_ANSI_YELLOW "\x1B[0;33m"
#define A_LOG_ANSI_GREEN  "\x1B[0;32m"
#define A_LOG_ANSI_BLUE   "\x1B[0;34m"
#define A_LOG_ANSI_PURPLE "\x1B[0;35m"

#define A_LOG_LEVEL_TAG_ERROR   "ERROR"
#define A_LOG_LEVEL_TAG_WARNING "WARNING"
#define A_LOG_LEVEL_TAG_INFO    "INFO"
#define A_LOG_LEVEL_TAG_DEBUG   "DEBUG"
#define A_LOG_LEVEL_TAG_VERBOSE "VERBOSE"

#ifndef AETHER_LOG_LEVEL
#define AETHER_LOG_LEVEL A_LOG_LEVEL_VERBOSE
#endif /* AETHER_LOG_LEVEL */
#endif /* AETHER_LOG_ENABLED */

typedef enum
{
    A_LOG_MODE_PRINT,
    A_LOG_MODE_CUSTOM
} a_Log_Mode_t;

#ifdef AETHER_LOG_ENABLED
static const char *a_Logger_AnsiColor[A_LOG_LEVEL_MAX] = {
    [A_LOG_LEVEL_ERROR]   = A_LOG_ANSI_RED,
    [A_LOG_LEVEL_WARNING] = A_LOG_ANSI_YELLOW,
    [A_LOG_LEVEL_INFO]    = A_LOG_ANSI_GREEN,
    [A_LOG_LEVEL_DEBUG]   = A_LOG_ANSI_BLUE,
    [A_LOG_LEVEL_VERBOSE] = A_LOG_ANSI_PURPLE,
};

static const char *a_Logger_LogTag[A_LOG_LEVEL_MAX] = {
    [A_LOG_LEVEL_ERROR]   = A_LOG_LEVEL_TAG_ERROR,
    [A_LOG_LEVEL_WARNING] = A_LOG_LEVEL_TAG_WARNING,
    [A_LOG_LEVEL_INFO]    = A_LOG_LEVEL_TAG_INFO,
    [A_LOG_LEVEL_DEBUG]   = A_LOG_LEVEL_TAG_DEBUG,
    [A_LOG_LEVEL_VERBOSE] = A_LOG_LEVEL_TAG_VERBOSE,
};

static const char *a_Logger_PrefixFormat  = "%s[%s] [%s] ";
static const char *a_Logger_PostfixFormat = "%s\r\n";

_Static_assert(((AETHER_LOG_LEVEL >= 0) && (AETHER_LOG_LEVEL < A_LOG_LEVEL_MAX)), "Invalid log level");
static a_Log_Level_t a_Log_Level = AETHER_LOG_LEVEL;
static a_Log_Mode_t  a_Log_Mode  = A_LOG_MODE_PRINT;

static void (*a_Log_CustomLog)(const char *const tag, const a_Log_Level_t level, const char *const format, ...) = NULL;   /* Client-specified logging function, not be called directly */
#endif /* AETHER_LOG_ENABLED */

#ifdef AETHER_LOG_ENABLED
static void a_Log_Print(const char *const tag, const a_Log_Level_t level, const char *const format, va_list args);
#endif /* AETHER_LOG_ENABLED */

void a_Log_SetLogLevel(const a_Log_Level_t level)
{
#ifdef AETHER_LOG_ENABLED
    if (level < A_LOG_LEVEL_MAX)
    {
        a_Log_Level = level;
    }
#else
    A_UNUSED(level);
#endif /* AETHER_LOG_ENABLED */
}

void a_Log_RegisterCustomLogger(void (*log)(const char *const tag, const a_Log_Level_t level, const char *const format, ...))
{
#ifdef AETHER_LOG_ENABLED
    if (NULL != log)
    {
        a_Log_CustomLog = log;
        a_Log_Mode      = A_LOG_MODE_CUSTOM;
    }
#else
    A_UNUSED(log);
#endif /* AETHER_LOG_ENABLED */
}

void a_Log(const char *const tag, const a_Log_Level_t level, const char *const format, ...)
{
#ifdef AETHER_LOG_ENABLED
    if ((NULL != tag) && (NULL != format) && (level <= a_Log_Level))
    {
        va_list args;

        va_start(args, format);

        switch (a_Log_Mode)
        {
        case A_LOG_MODE_PRINT:
            a_Log_Print(tag, level, format, args);
            break;
        case A_LOG_MODE_CUSTOM:
            a_Log_CustomLog(tag, level, format, args);
            break;
        default:
            break;
        }

        va_end(args);
    }
#else
    A_UNUSED(tag);
    A_UNUSED(level);
    A_UNUSED(format);
#endif /* AETHER_LOG_ENABLED */
}

#ifdef AETHER_LOG_ENABLED
static void a_Log_Print(const char *const tag, const a_Log_Level_t level, const char *const format, va_list args)
{
    printf(a_Logger_PrefixFormat, a_Logger_AnsiColor[level], a_Logger_LogTag[level], tag);
    vprintf(format, args);
    printf(a_Logger_PostfixFormat, A_LOG_ANSI_RESET);
}
#endif /* AETHER_LOG_ENABLED */
