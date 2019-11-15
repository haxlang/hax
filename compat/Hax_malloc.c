#include <stdlib.h>

void *
haxMalloc(size_t size)
{
    return malloc(size);
}

void *
haxCalloc(size_t number, size_t size)
{
    return calloc(number, size);
}

void *
haxRealloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

void
haxFree(void *ptr)
{
    free(ptr);
}
