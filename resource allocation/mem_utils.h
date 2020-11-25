#ifndef mem_utils_h
#define mem_utils_h

#include <stdlib.h>
#include <string.h>

void* memdup(const void* ptr, size_t size) {
    void* buffer = malloc(size);
    memcpy(buffer, ptr, size);
    return buffer;
}

#endif /* mem_utils_h */
