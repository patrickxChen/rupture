#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "log.h"

#define ANSI_RESET  "\033[0m"
#define ANSI_GREEN  "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_RED    "\033[31m"

static void default_backend(log_level_t level, const char *msg)
{
    switch (level) {
    case LOG_OK:
        printf("%s%s%s", ANSI_GREEN, msg, ANSI_RESET);
        break;
    case LOG_WARN:
        printf("%s%s%s", ANSI_YELLOW, msg, ANSI_RESET);
        break;
    case LOG_ERR:
        printf("%s%s%s", ANSI_RED, msg, ANSI_RESET);
        break;
    default:
        printf("%s", msg);
        break;
    }
    fflush(stdout);
}

static void (*g_backend)(log_level_t, const char *) = default_backend;

void log_set_backend(void (*fn)(log_level_t, const char *msg))
{
    g_backend = fn;
}

void dbg_log(log_level_t level, const char *fmt, ...)
{
    char buf[4096];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    g_backend(level, buf);
}
