#include <iostream>

#ifndef NDEBUG
#define OKAY(MSG, ...) printf("[+] "          MSG "\n", ##__VA_ARGS__)
#define INFO(MSG, ...) printf("[*] "          MSG "\n", ##__VA_ARGS__)
#define PRINT_ERROR(FUNCTION_NAME, err)                              \
    do {                                                             \
        fprintf(stderr,                                              \
                "[!] [" FUNCTION_NAME "] failed, error: %lu\n"     \
                "[*] %s:%d\n", err, __FILE__, __LINE__);             \
    } while (0)
#else
#define OKAY(MSG, ...)
#define INFO(MSG, ...)
#define PRINT_ERROR(FUNCTION_NAME, err)
#endif
#define LOG(MSG, ...) printf("[*] "          MSG "\n", ##__VA_ARGS__)
#define WARN(MSG, ...) fprintf(stderr, "[-] " MSG "\n", ##__VA_ARGS__)