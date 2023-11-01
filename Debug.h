#ifdef __DEBUG__
void kprintf(char *fmt, ...);
#else
#define kprintf(...)
#endif
