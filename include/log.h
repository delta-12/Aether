#ifndef AETHER_LOG_H
#define AETHER_LOG_H

#include <stddef.h>

typedef enum
{
    A_LOG_LEVEL_ERROR,
    A_LOG_LEVEL_WARNING,
    A_LOG_LEVEL_INFO,
    A_LOG_LEVEL_DEBUG,
    A_LOG_LEVEL_VERBOSE,
    A_LOG_LEVEL_MAX
} a_Log_Level_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

void a_Log_SetLogLevel(const a_Log_Level_t level);
void a_Log_RegisterCustomLogger(void (*log)(const char *const tag, const a_Log_Level_t level, char *const format, ...));
void a_Log(const char *const tag, const a_Log_Level_t level, char *const format, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#define A_LOG_ERROR(tag, format, ...)   a_Log(tag, A_LOG_LEVEL_ERROR, format, ## __VA_ARGS__)
#define A_LOG_WARNING(tag, format, ...) a_Log(tag, A_LOG_LEVEL_WARNING, format, ## __VA_ARGS__)
#define A_LOG_INFO(tag, format, ...)    a_Log(tag, A_LOG_LEVEL_INFO, format, ## __VA_ARGS__)
#define A_LOG_DEBUG(tag, format, ...)   a_Log(tag, A_LOG_LEVEL_DEBUG, format, ## __VA_ARGS__)
#define A_LOG_VERBOSE(tag, format, ...) a_Log(tag, A_LOG_LEVEL_VERBOSE, format, ## __VA_ARGS__)

#endif /* AETHER_LOG_H */
