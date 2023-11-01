#include "version.h"

#define STR(x) #x
#define XSTR(x) STR(x)

const char FileName[] = LIBNAME "." LIBTYPE;
const char IdString[] = LIBNAME " " XSTR(VERSION) "." XSTR(REVISION) " (" DATE ")\n\r";

