#ifndef DB_DEBUG_H
#define DB_DEBUG_H

void debug(const char *fmt, ...) {
#ifdef DB_DEBUG
    va_list args;
    char buf[1000];
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printf(buf);
    printf("\n");
#endif
}

#endif //DB_DEBUG_H
