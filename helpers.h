#ifndef HELPERS_H_
#define HELPERS_H_

#ifndef TRACE
#define TRACE 0
#endif

#ifndef DEBUG
#define DEBUG 0
#endif

#define DEBUG_PRINT(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

#define TRACE_PRINT(fmt, ...) \
        do { if (TRACE) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)


#endif // HELPERS_H_
