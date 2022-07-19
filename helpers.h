#ifndef HELPERS_H_
#define HELPERS_H_

#ifndef TRACE_LEVEL
#define TRACE_LEVEL 0
#endif

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0
#endif

#define DEBUG_PRINT(fmt, ...) \
        do { if (DEBUG_LEVEL) fprintf(stdout, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

#define TRACE_PRINT(fmt, ...) \
        do { if (TRACE_LEVEL) fprintf(stdout, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)


#endif // HELPERS_H_
