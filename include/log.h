#pragma once

typedef enum {
    LOG_NORMAL,
    LOG_OK,
    LOG_WARN,
    LOG_ERR
} log_level_t;

void log_set_backend(void (*fn)(log_level_t, const char *msg));
void dbg_log(log_level_t level, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

#define log_msg(...) dbg_log(LOG_NORMAL, __VA_ARGS__)
#define log_ok(...)  dbg_log(LOG_OK,     __VA_ARGS__)
#define log_warn(...) dbg_log(LOG_WARN,  __VA_ARGS__)
#define log_err(...) dbg_log(LOG_ERR,    __VA_ARGS__)
