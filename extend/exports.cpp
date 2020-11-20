#include <cstdio>

#ifdef _WIN32
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API
#endif

extern "C" EXPORT_API double putchard(double x) { putchar((char)x); return 0; }
extern "C" EXPORT_API double printd(double x) { printf("%f", x); return 0; }
extern "C" EXPORT_API double getchard() { return getchar(); }