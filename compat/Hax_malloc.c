#ifndef SIZE_T_DEFINED
typedef __SIZE_TYPE__ Size_t;
#define SIZE_T_DEFINED
#endif

void *bsdmalloc(Size_t size);
void *bsdcalloc(Size_t number, Size_t size);
void *bsdrealloc(void *ptr, Size_t size);
void bsdfree(void *ptr);

void *
haxMalloc(Size_t size)
{
    return bsdmalloc(size);
}

void *
haxCalloc(Size_t number, Size_t size)
{
    return bsdcalloc(number, size);
}

void *
haxRealloc(void *ptr, Size_t size)
{
    return bsdrealloc(ptr, size);
}

void
haxFree(void *ptr)
{
    bsdfree(ptr);
}
